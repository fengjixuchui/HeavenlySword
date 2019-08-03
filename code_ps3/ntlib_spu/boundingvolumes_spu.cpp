#include <basetypes_spu.h>
#include <spu_intrinsics.h>

#include <debug_spu.h>

#include "vecmath_spu_ps3.h"
#include "boundingvolumes_spu.h"
#include "util_spu.h"

#define DW_AS_FLT(DW) (*(FLOAT*)&(DW))
#define FLT_AS_DW(F) (*(uint32_t*)&(F))
#define FLT_SIGN(F) ((FLT_AS_DW(F) & 0x80000000L))
#define ALMOST_ZERO(F) ((FLT_AS_DW(F) & 0x7f800000L)==0)
#define IS_SPECIAL(F)  ((FLT_AS_DW(F) & 0x7f800000L)==0x7f800000L)


CAABB::CAABB( CPoint* points, unsigned int numPoints ) 
{ 
	AABB_ConstructInfiniteNegative(this);
	for(unsigned int i=0;i < numPoints;i++)
	{
		Union( points[i] );
	}
}


static bool PlaneIntersection( CPoint* intersectPt, const CPlane* p0, const CPlane* p1, const CPlane* p2 )
{
    CDirection n1_n2, n2_n0, n0_n1;  
    
	n1_n2 = p1->GetNormal().Cross( p2->GetNormal() );
	n2_n0 = p2->GetNormal().Cross( p0->GetNormal() );
	n0_n1 = p0->GetNormal().Cross( p1->GetNormal() );

    float cosTheta = p0->GetNormal().Dot( n1_n2 );
    
    if ( ALMOST_ZERO(cosTheta) || IS_SPECIAL(cosTheta) )
	{
		*intersectPt = CPoint( CONSTRUCT_CLEAR );
        return false;
	}

    float secTheta = 1.f / cosTheta;

    n1_n2 = n1_n2 * p0->GetDistance();
    n2_n0 = n2_n0 * p1->GetDistance();
    n0_n1 = n0_n1 * p2->GetDistance();

    *intersectPt = CPoint(-(n1_n2 + n2_n0 + n0_n1) * secTheta);
    return true;
}


CullingFrustum::CullingFrustum( const CMatrix& matrix )
{
    Init(matrix);
}

void CullingFrustum::Init( const CMatrix& matrix )
{
	m_frustumMatrix = matrix;

	v128 Columns[4] ALIGNTO_POSTFIX( 16 );

	Columns[0] = matrix.GetRow0();
	Columns[0] = spu_insert(spu_extract(matrix.GetRow3(), 0), spu_insert(spu_extract(matrix.GetRow2(), 0), spu_insert(spu_extract(matrix.GetRow1(), 0), Columns[0], 1), 2), 3);

	Columns[1] = matrix.GetRow1();
	Columns[1] = spu_insert(spu_extract(matrix.GetRow3(), 1), spu_insert(spu_extract(matrix.GetRow2(), 1), spu_insert(spu_extract(matrix.GetRow0(), 1), Columns[1], 0), 2), 3);

	Columns[2] = matrix.GetRow2();
	Columns[2] = spu_insert(spu_extract(matrix.GetRow3(), 2), spu_insert(spu_extract(matrix.GetRow1(), 2), spu_insert(spu_extract(matrix.GetRow0(), 2), Columns[2], 0), 1), 3);

	Columns[3] = matrix.GetRow3();
	Columns[3] = spu_insert(spu_extract(matrix.GetRow2(), 3), spu_insert(spu_extract(matrix.GetRow1(), 3), spu_insert(spu_extract(matrix.GetRow0(), 3), Columns[3], 0), 1), 2);

	CVector planes[6];

	planes[0] = CVector(spu_sub(Columns[3], Columns[0])); // left
	planes[1] = CVector(spu_add(Columns[3], Columns[0])); // right
	planes[2] = CVector(spu_sub(Columns[3], Columns[1])); // bottom
	planes[3] = CVector(spu_add(Columns[3], Columns[1])); // top
	planes[4] = CVector(spu_sub(Columns[3], Columns[2])); // near
	planes[5] = CVector(spu_add(Columns[3], Columns[2])); // far

	v128 zeroVector = (v128){0, 0, 0, 0};

    for (int p=0; p<6; p++)  // normalize the planes
    {
        float dot = planes[p][0]*planes[p][0] + planes[p][1]*planes[p][1] + planes[p][2] * planes[p][2];
        dot = 1.f / sqrtf(dot);
        m_CamPlanes[p] = CPlane(planes[p] * dot);

		m_nVertexLUT[p]  = spu_cmpgt( zeroVector, m_CamPlanes[p].GetNormal().QuadwordValue());
	
    }

    for (int i=0; i<8; i++)  // compute extrema
    {
        const CPlane& p0 = (i&1)?m_CamPlanes[4] : m_CamPlanes[5];
        const CPlane& p1 = (i&2)?m_CamPlanes[3] : m_CamPlanes[2];
        const CPlane& p2 = (i&4)?m_CamPlanes[0] : m_CamPlanes[1];

		PlaneIntersection( &m_pntList[i], &p0, &p1, &p2 );
    }

	m_frustumBox = CAABB( m_pntList, 8 );

	// make the box a little smaller than the actual frustum (by scaling in its own space)
	CPoint centroid = m_frustumBox.GetCentre();
    CPoint			m_tmpList[8];
    for (int i=0; i<8; i++)
    {
		m_tmpList[i] = ((m_pntList[i] - centroid) * 0.99999f) + centroid; // brind the box in a little
	}


	m_frustumBox = CAABB( m_tmpList, 8 );

    if (!m_frustumBox.IsValid())
    {
        ntPrintf("!Frustum invalid!\n");
		ntBreakpoint();
    }

}

CSphereBound::CSphereBound( const CAABB& box )
{
    m_obPosition = box.GetCentre();
    CDirection radiusVec = CDirection(box.Max() - m_obPosition);
    m_fRadius = radiusVec.Length();;
}

void CAABB::Transform( const CMatrix& transform )
{
	CPoint pnts[8];
	pnts[0] = CPoint( Min().X(), Min().Y(), Min().Z() );
	pnts[1] = CPoint( Max().X(), Min().Y(), Min().Z() );
	pnts[2] = CPoint( Min().X(), Max().Y(), Min().Z() );
	pnts[3] = CPoint( Max().X(), Max().Y(), Min().Z() );
	pnts[4] = CPoint( Min().X(), Min().Y(), Max().Z() );
	pnts[5] = CPoint( Max().X(), Min().Y(), Max().Z() );
	pnts[6] = CPoint( Min().X(), Max().Y(), Max().Z() );
	pnts[7] = CPoint( Max().X(), Max().Y(), Max().Z() );

	// construct a AABB from our transformed points 
	m_obMin = CPoint(FLT_MAX, FLT_MAX, FLT_MAX);
	m_obMax = CPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	Union( CPoint(pnts[0].QuadwordValue() * transform) );
	Union( CPoint(pnts[1].QuadwordValue() * transform) );
	Union( CPoint(pnts[2].QuadwordValue() * transform) );
	Union( CPoint(pnts[3].QuadwordValue() * transform) );
	Union( CPoint(pnts[4].QuadwordValue() * transform) );
	Union( CPoint(pnts[5].QuadwordValue() * transform) );
	Union( CPoint(pnts[6].QuadwordValue() * transform) );
	Union( CPoint(pnts[7].QuadwordValue() * transform) );

}

void ClipPolyAgainstPlane( const CPlane* plane, int vertexCount, CPoint* inVertex, int& outVertexCount, CPoint* outVertex )
{
	outVertexCount = 0;
	CDirection s( inVertex[vertexCount-1].X(), inVertex[vertexCount-1].Y(), inVertex[vertexCount-1].Z() );
	float d1 = plane -> GetNormal().Dot(s) + plane -> GetDistance();

	for( int vertNum = 0; vertNum < vertexCount;vertNum++ )
	{			
		CDirection p( inVertex[vertNum].X(), inVertex[vertNum].Y(), inVertex[vertNum].Z() );
		float d0 = plane -> GetNormal().Dot(p) + plane -> GetDistance();

		if( d0 > 0 )
		{
			if( d1 > 0 )
			{
				outVertex[outVertexCount++] = CPoint(p.X(), p.Y(), p.Z());
			} else
			{
				CDirection dir = p - s;
				float theta = plane -> GetNormal().Dot( dir );		
                if (fabs(theta) < 0.0001f)
                {
				    outVertex[outVertexCount++] = CPoint(p.X(), p.Y(), p.Z());
                }
                else
                {
				    float t = -d1 / theta;
    				//ntAssert( t >= 0.f && t <= 1.f );
                    //t = ( t > 1.f ) ? 1.f : t;
    
				    CDirection i =  s + t * dir;
				    outVertex[outVertexCount++] = CPoint(i.X(), i.Y(), i.Z());
				    outVertex[outVertexCount++] = CPoint(p.X(), p.Y(), p.Z());
                }
			}
		} else
		{
			if( d1 > 0 )
			{
				CDirection dir = p - s;
				float theta = plane->GetNormal().Dot( dir );		
                if (fabs(theta) > 0.0001f)
                {
				    float t = -d1 / theta;
    				//ntAssert( t >= 0.f && t <= 1.f );
                    //t = ( t > 1.f ) ? 1.f : t;
    
				    CDirection i =  s + t * dir;
				    outVertex[outVertexCount++] = CPoint(i.X(), i.Y(), i.Z());
                }
			}
		}

		s = p;
		d1 = d0;
	}
}

CPoint* ClipPoly( const CullingFrustum* frustum, int vertexCount, CPoint* inVertex, int& outVertexCount, CPoint* outVertex ) 
{
   	// start with last vertex
	for( int planeNum = 0;planeNum < 6;planeNum++ )
	{
		ClipPolyAgainstPlane( frustum -> GetPlane(planeNum), vertexCount, inVertex, outVertexCount, outVertex );

		vertexCount = outVertexCount;
		CPoint* tmp = inVertex;
		inVertex = outVertex;
		outVertex = tmp;

		// early out for completelly clipped
		if( vertexCount == 0 )
			return inVertex;
	}

	return inVertex;
}


CAABB* CullingFrustum::Intersect( const CAABB& inBox, CAABB* resultAABB) const
{
    AABB_ConstructInfiniteNegative(resultAABB);

	CAABB box = m_frustumBox;

    
	box.Intersection( inBox );

	if( !box.IsValid() )
	{
//        ntPrintf("##Outsider!##\n");
		return resultAABB;
	}

    
	//
	//         7-------6
	//        /|     /|
	//       / |    / |
	//     3-------2  |
	//      | 4-- | ---5
	//      | /   |  /
	//      |/    | /
	//     0-------1
	CPoint pnts[8];

	// create the original points of the box
	pnts[0] = CPoint( box.Min().X(), box.Min().Y(), box.Min().Z() );
    pnts[1] = CPoint( box.Max().X(), box.Min().Y(), box.Min().Z() );
    pnts[2] = CPoint( box.Max().X(), box.Max().Y(), box.Min().Z() );
    pnts[3] = CPoint( box.Min().X(), box.Max().Y(), box.Min().Z() );

	pnts[4] = CPoint( box.Min().X(), box.Min().Y(), box.Max().Z() );
    pnts[5] = CPoint( box.Max().X(), box.Min().Y(), box.Max().Z() );
    pnts[6] = CPoint( box.Max().X(), box.Max().Y(), box.Max().Z() );
    pnts[7] = CPoint( box.Min().X(), box.Max().Y(), box.Max().Z() );

	CPoint inputArray[4*6*2];
	CPoint tmpArray[4*6*2];
	CPoint* outputArray;
	int outCount;
	// front
	inputArray[0] = pnts[3]; inputArray[1] = pnts[2]; inputArray[2] = pnts[1];	inputArray[3] = pnts[0];
	outputArray = ClipPoly( this, 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB -> Union( outputArray[i] );
	}

   
	// right
	inputArray[0] = pnts[2]; inputArray[1] = pnts[6]; inputArray[2] = pnts[5];	inputArray[3] = pnts[1];
	outputArray = ClipPoly( this, 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB -> Union( outputArray[i] );
	}


	// back
	inputArray[0] = pnts[6]; inputArray[1] = pnts[7]; inputArray[2] = pnts[4];	inputArray[3] = pnts[5];
	outputArray = ClipPoly( this, 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB -> Union( outputArray[i] );
	}

	// left
	inputArray[0] = pnts[7]; inputArray[1] = pnts[3]; inputArray[2] = pnts[0];	inputArray[3] = pnts[4];
	outputArray = ClipPoly( this, 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB -> Union( outputArray[i] );
	}

	// top
	inputArray[0] = pnts[7]; inputArray[1] = pnts[6]; inputArray[2] = pnts[2];	inputArray[3] = pnts[3];
	outputArray = ClipPoly( this, 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB -> Union( outputArray[i] );
	}

	// bottom
	inputArray[0] = pnts[0]; inputArray[1] = pnts[1]; inputArray[2] = pnts[5];	inputArray[3] = pnts[4];
	outputArray = ClipPoly( this, 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB -> Union( outputArray[i] );
	}
    
	return resultAABB;
    
}


