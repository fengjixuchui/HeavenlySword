/***************************************************************************************************
*
*	$Header:: /game/frustum.cpp 3     24/07/03 11:53 Simonb                                        $
*
*
*
*	CHANGES
*
*	16/6/2003	SimonB	Created
*
***************************************************************************************************/

#include "frustum.h"
//#include "renderer.h"
#include "gfx/camera.h"
#include "anim/transform.h"
#include "boundingvolumes.h"

#include "Physics/debugdraw.h"
#include "gatso.h"
#include "visualdebugger.h"

/***************************************************************************************************
*
*	FUNCTION		CPlane::CPlane
*
*	DESCRIPTION		Creates a plane from 3 points on a triangle.
*
***************************************************************************************************/

CPlane::CPlane(CPoint const& obA, CPoint const& obB, CPoint const& obC)
{
	m_obNormal = CDirection(obC - obA).Cross(CDirection(obB - obA));
	m_obNormal.Normalise();

	m_fDistance = m_obNormal.Dot(CDirection(obA));
}
CPlane::CPlane(float a, float b, float c, float d)
{
	m_obNormal = CDirection(a,b,c);
	m_fDistance = d;
}


/***************************************************************************************************
*
*	FUNCTION		CPlane::ContainsPoint
*
*	DESCRIPTION		Returns true if a plane contains the given point.
*
***************************************************************************************************/

bool CPlane::ContainsPoint(CPoint const& obPoint) const
{
	return CDirection(obPoint).Dot(m_obNormal) < m_fDistance;
}

/***************************************************************************************************
*
*	FUNCTION		CPlane::IntersectsOBB
*
*	DESCRIPTION		Returns true if a plane intersects the given OBB.
*
***************************************************************************************************/

bool CPlane::IntersectsOBB(CMatrix const& obTransform, CDirection const& obHalfLengths) const
{
	// get the height of the box centre above the plane
	float fHeight = CDirection(obTransform.GetTranslation()).Dot(m_obNormal) - m_fDistance;

	// early out if the centre is in the plane
	if(fHeight < 0.0f)
		return true;

	// otherwise move along each face towards the plane (finds the closest point 
	// on the box surface to the plane boundary)
	fHeight -= fabsf(obTransform.GetXAxis().Dot(m_obNormal))*obHalfLengths.X();
	fHeight -= fabsf(obTransform.GetYAxis().Dot(m_obNormal))*obHalfLengths.Y();
	fHeight -= fabsf(obTransform.GetZAxis().Dot(m_obNormal))*obHalfLengths.Z();

	// check this point
	return (fHeight < 0.0f);
}

/***************************************************************************************************
*
*	FUNCTION		CPlane::GetHeight
*
*	DESCRIPTION		Get the signed height of a point to the plane.
*
***************************************************************************************************/

float CPlane::GetHeight(CPoint const& obPoint) const
{
	// get the height of the box centre above the plane
	return CDirection(obPoint).Dot(m_obNormal) - m_fDistance;
};


/***************************************************************************************************
*
*	FUNCTION		CPlane::IntersectsLineSeg
*
*	DESCRIPTION		Returns true if the line segment intersects the plane
*
***************************************************************************************************/

bool CPlane::IntersectsLineSeg(const CPoint& obStart, const CPoint& obEnd, CPoint& obResult ) const
{
	CDirection obRay( obEnd ^ obStart );
	
	float fRayAngle = m_obNormal.Dot( obRay );
	
	if ( fabsf(fRayAngle) < EPSILON ) // ray is parallel to plane, ignore.
		return false;

	float fTime = (m_fDistance - m_obNormal.Dot( CDirection(obStart) )) / fRayAngle;

	if ((fTime <= 0.0f) || (fTime >= 1.0f))	// intersection is outside line segement
		return false;

	obResult = obStart + (fTime * obRay);
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CPlane::IntersectsSweptSphere
*
*	DESCRIPTION		Returns true if the swepth sphere intersects the plane
*
***************************************************************************************************/
bool CPlane::IntersectsSweptSphere( const CSphereBound& bound, const CDirection& sweptDir, float& fOutT0, float& fOutT1 ) const
{
	float b_dot_n = GetHeight( bound.GetPosition() );
	float d_dot_n = GetNormal().Dot( sweptDir );

	if( fabsf(d_dot_n) < EPSILON )
	{
		if( b_dot_n <= bound.GetRadius() )
		{
			// effectively infinity
			fOutT0 = 0.f;
			fOutT1 = 1e32f;
			return true;
		} else
		{
			return false;
		}
	} else
	{
		float t0 = (bound.GetRadius() - b_dot_n) / d_dot_n;
		float t1 = (-bound.GetRadius() - b_dot_n) / d_dot_n;
		fOutT0 = min(t0, t1);
		fOutT0 = max(t0, t1);
		return true;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CFrustum::CFrustum
*
*	DESCRIPTION		Creates an empty, invalid frustum.
*
***************************************************************************************************/

CFrustum::CFrustum() : m_pobCamera(0), m_bValid(false) {}

/***************************************************************************************************
*
*	FUNCTION		CFrustum::Create
*
*	DESCRIPTION		Creates a maximal frustum for the given camera.
*
***************************************************************************************************/

CFrustum::CFrustum(const CCamera* pobCamera,float fAspectRatio)
{
	// cache the camera
	m_pobCamera = pobCamera;
	m_fAspectRatio = fAspectRatio;

	// reset the bounds
	m_obBoundsMin = CDirection(-1.0f, -1.0f, 0.0f);
	m_obBoundsMax = CDirection( 1.0f,  1.0f, 1.0f);
	m_bValid = true;

    // create the clipping planes
	CreateClipPlanes();

	//DrawFrustumInHKVDB(*this, m_pobCamera->GetViewTransform()->GetWorldMatrix());
}


/***************************************************************************************************
*
*	FUNCTION		CFrustum::Create
*
*	DESCRIPTION		Creates a clipped frustum for the given parent frustum and portal. The clipped
*					frustum is always contained in the parent frustum, and will become invalid
*					if the portal clips it to zero size.
*
***************************************************************************************************/

/*CFrustum::CFrustum(const CFrustum* pobParent, const CPortal* pobPortal)
{
	// cache the camera
	m_pobCamera = pobParent->GetCamera();

	// reset the bounds
	m_obBoundsMin = CDirection( 1.0f,  1.0f, 1.0f);
	m_obBoundsMax = CDirection(-1.0f, -1.0f, 1.0f);
	
	// clip the bounds to the portal
	bool abPlaneFailedOnce[] = { false, false, false, false, false, false };
	bool abPlaneFailedAll[] = { true, true, true, true, true, true };
	const CMatrix obTransform = pobPortal->GetSourceTransform()->GetWorldMatrix()
		*m_pobCamera->GetViewTransform()->GetWorldMatrix().GetAffineInverse();
	const float fYScale = 1.0f/ftanf(m_pobCamera->GetFOVAngle()/2.0f);
	const float fXScale = fYScale/m_fAspectRatio;
	for(int iHullPoint = 0; iHullPoint < pobPortal->GetNumHullPoints(); ++iHullPoint)
	{
		CPoint obViewPoint = pobPortal->GetHullPoints()[iHullPoint]*obTransform;
		bool bClipped = false;

		// check against each frustum plane
		for(int iPlane = 0; iPlane < 6; ++iPlane)
		{
			if(pobParent->m_aobClipPlanes[iPlane].ContainsPoint(obViewPoint))
				abPlaneFailedAll[iPlane] = false;
			else
			{
				if(iPlane == 5)
					bClipped = true;
				abPlaneFailedOnce[iPlane] = true;
			}
		}

        // if this point is in the main frustum we update the bounds
		if(!bClipped)
		{
			// project the point
			const float fX = obViewPoint.X()*fXScale/obViewPoint.Z();
			const float fY = obViewPoint.Y()*fYScale/obViewPoint.Z();
			const float fZ = (obViewPoint.Z() - m_pobCamera->GetZNear())/(m_pobCamera->GetZFar() - m_pobCamera->GetZNear());

			// adjust the bounds
			m_obBoundsMin.X() = min(fX, m_obBoundsMin.X());
			m_obBoundsMax.X() = max(fX, m_obBoundsMax.X());
			m_obBoundsMin.Y() = min(fY, m_obBoundsMin.Y());
			m_obBoundsMax.Y() = max(fY, m_obBoundsMax.Y());
			m_obBoundsMin.Z() = min(fZ, m_obBoundsMin.Z());
		}
	}

	// clip to the parent bounds
	if(abPlaneFailedOnce[5])
	{
		m_obBoundsMin = pobParent->m_obBoundsMin;
		m_obBoundsMax = pobParent->m_obBoundsMax;
	}
	else
	{
		m_obBoundsMin.X() = max(pobParent->m_obBoundsMin.X(), m_obBoundsMin.X());
		m_obBoundsMax.X() = min(pobParent->m_obBoundsMax.X(), m_obBoundsMax.X());
		m_obBoundsMin.Y() = max(pobParent->m_obBoundsMin.Y(), m_obBoundsMin.Y());
		m_obBoundsMax.Y() = min(pobParent->m_obBoundsMax.Y(), m_obBoundsMax.Y());
		m_obBoundsMin.Z() = max(pobParent->m_obBoundsMin.Z(), m_obBoundsMin.Z());
	}

	// check for a valid volume
	m_bValid = (m_obBoundsMin.X() < m_obBoundsMax.X()) && (m_obBoundsMin.Y() <= m_obBoundsMax.Y());
	for(int iPlane = 0; iPlane < 5; ++iPlane)
		m_bValid = m_bValid && !abPlaneFailedAll[iPlane];

    // create the clipping planes if valid
	if(m_bValid)
		CreateClipPlanes();
}*/

/***************************************************************************************************
*
*	FUNCTION		CFrustum::CreateClipPlanes
*
*	DESCRIPTION		Creates view-space clip planes from the current frustum parameters.
*
***************************************************************************************************/

void CFrustum::CreateClipPlanes()
{
	// build the debug volume (in view space)
	CPoint aobPositions[8];
	GetExtremePoints(&aobPositions[0]);

	// build the bounding planes (in view space)
    m_aobClipPlanes[0] = CPlane(aobPositions[2], aobPositions[6], aobPositions[4]);
    m_aobClipPlanes[1] = CPlane(aobPositions[3], aobPositions[7], aobPositions[6]);
    m_aobClipPlanes[2] = CPlane(aobPositions[1], aobPositions[5], aobPositions[7]);
    m_aobClipPlanes[3] = CPlane(aobPositions[0], aobPositions[4], aobPositions[5]);
    m_aobClipPlanes[4] = CPlane(aobPositions[4], aobPositions[6], aobPositions[7]);
    m_aobClipPlanes[5] = CPlane(-m_aobClipPlanes[4].GetNormal(), -m_aobClipPlanes[4].GetNormal().Dot(CDirection(aobPositions[0])));

}
/***************************************************************************************************
*
*	FUNCTION		CFrustum::CreateClipPlanes
*
*	DESCRIPTION		Creates clip planes from the current frustum parameters in the given transform.
*
***************************************************************************************************/

void CFrustum::CreateClipPlanes( const CMatrix& matrix )
{
	// build the debug volume (in view space)
	CPoint aobPositions[8];
	GetExtremePoints(&aobPositions[0], matrix);

	// build the bounding planes (in view space)
    m_aobClipPlanes[0] = CPlane(aobPositions[2], aobPositions[6], aobPositions[4]);
    m_aobClipPlanes[1] = CPlane(aobPositions[3], aobPositions[7], aobPositions[6]);
    m_aobClipPlanes[2] = CPlane(aobPositions[1], aobPositions[5], aobPositions[7]);
    m_aobClipPlanes[3] = CPlane(aobPositions[0], aobPositions[4], aobPositions[5]);
    m_aobClipPlanes[4] = CPlane(aobPositions[4], aobPositions[6], aobPositions[7]);
    m_aobClipPlanes[5] = CPlane(-m_aobClipPlanes[4].GetNormal(), -m_aobClipPlanes[4].GetNormal().Dot(CDirection(aobPositions[0])));

}
/***************************************************************************************************
*
*	FUNCTION		CFrustum::IntersectsOBB
*
*	DESCRIPTION		Returns true if the frustum intersects the given OBB.
*
***************************************************************************************************/

bool CFrustum::IntersectsOBB(CMatrix const& obTransform, CDirection const& obHalfLengths) const
{
	ntAssert(m_bValid);
	for(int iPlane = 0; iPlane < 6; ++iPlane)
	{
		if(!m_aobClipPlanes[iPlane].IntersectsOBB(obTransform, obHalfLengths))
			return false;
	}
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CFrustum::GetExtremePoints
*
*	DESCRIPTION		Computes the 8 frustum extreme points (in the convex hull sense).
*
***************************************************************************************************/

void CFrustum::GetExtremePoints(CPoint* pobPositions) const
{
	// build the debug volume (in view space)
	ntAssert(m_bValid);
	const float fYScale = ftanf(m_pobCamera->GetFOVAngle()/2.0f);
	const float fXScale = m_fAspectRatio*fYScale;

	CPoint aobQuad[] = 
	{
		CPoint(fXScale*m_obBoundsMin.X(), fYScale*m_obBoundsMin.Y(), 1.0f), 
		CPoint(fXScale*m_obBoundsMax.X(), fYScale*m_obBoundsMin.Y(), 1.0f), 
		CPoint(fXScale*m_obBoundsMin.X(), fYScale*m_obBoundsMax.Y(), 1.0f), 
		CPoint(fXScale*m_obBoundsMax.X(), fYScale*m_obBoundsMax.Y(), 1.0f)
	};

	const float fZNear = m_pobCamera->GetZNear()*(1.0f-m_obBoundsMin.Z()) + m_pobCamera->GetZFar()*m_obBoundsMin.Z();
	const float fZFar = m_pobCamera->GetZNear()*(1.0f-m_obBoundsMax.Z()) + m_pobCamera->GetZFar()*m_obBoundsMax.Z();

	pobPositions[0] = fZNear*aobQuad[0];
	pobPositions[1] = fZNear*aobQuad[1];
	pobPositions[2] = fZNear*aobQuad[2];
	pobPositions[3] = fZNear*aobQuad[3];
	pobPositions[4] = fZFar*aobQuad[0];
	pobPositions[5] = fZFar*aobQuad[1];
	pobPositions[6] = fZFar*aobQuad[2];
	pobPositions[7] = fZFar*aobQuad[3];
}


/***************************************************************************************************
*
*	FUNCTION		CFrustum::GetClipPlanes
*
*	DESCRIPTION		Computes the 6 frustum clip planes (in the convex hull sense).
*
***************************************************************************************************/
void CFrustum::GetClipPlanes(CPlane* pobPlanes) const
{
	// build the debug volume (in view space)
	CPoint aobPositions[8];
	GetExtremePoints(&aobPositions[0]);

	// build the bounding planes (in view space)
    pobPlanes[0] = CPlane(aobPositions[2], aobPositions[6], aobPositions[4]);
    pobPlanes[1] = CPlane(aobPositions[3], aobPositions[7], aobPositions[6]);
    pobPlanes[2] = CPlane(aobPositions[1], aobPositions[5], aobPositions[7]);
    pobPlanes[3] = CPlane(aobPositions[0], aobPositions[4], aobPositions[5]);
    pobPlanes[4] = CPlane(aobPositions[4], aobPositions[6], aobPositions[7]);
    pobPlanes[5] = CPlane(-m_aobClipPlanes[4].GetNormal(), -m_aobClipPlanes[4].GetNormal().Dot(CDirection(aobPositions[0])));
};

/***************************************************************************************************
*
*	FUNCTION		CFrustum::GetExtremePoints
*
*	DESCRIPTION		Computes the 8 frustum extreme points (in the convex hull sense).
*
***************************************************************************************************/

void CFrustum::GetExtremePoints(CPoint* pobPositions, const CMatrix& matrix ) const
{
	// build the debug volume (in view space)
	ntAssert(m_bValid);
	const float fYScale = ftanf(m_pobCamera->GetFOVAngle()/2.0f);
	const float fXScale = m_fAspectRatio*fYScale;

	CPoint aobQuad[] = 
	{
		CPoint(fXScale*m_obBoundsMin.X(), fYScale*m_obBoundsMin.Y(), 1.0f), 
		CPoint(fXScale*m_obBoundsMax.X(), fYScale*m_obBoundsMin.Y(), 1.0f), 
		CPoint(fXScale*m_obBoundsMin.X(), fYScale*m_obBoundsMax.Y(), 1.0f), 
		CPoint(fXScale*m_obBoundsMax.X(), fYScale*m_obBoundsMax.Y(), 1.0f)
	};

	const float fZNear = m_pobCamera->GetZNear()*(1.0f-m_obBoundsMin.Z()) + m_pobCamera->GetZFar()*m_obBoundsMin.Z();
	const float fZFar = m_pobCamera->GetZNear()*(1.0f-m_obBoundsMax.Z()) + m_pobCamera->GetZFar()*m_obBoundsMax.Z();

	pobPositions[0] = (fZNear*aobQuad[0]) * matrix;
	pobPositions[1] = (fZNear*aobQuad[1]) * matrix;
	pobPositions[2] = (fZNear*aobQuad[2]) * matrix;
	pobPositions[3] = (fZNear*aobQuad[3]) * matrix;
	pobPositions[4] = (fZFar*aobQuad[0]) * matrix;
	pobPositions[5] = (fZFar*aobQuad[1]) * matrix;
	pobPositions[6] = (fZFar*aobQuad[2]) * matrix;
	pobPositions[7] = (fZFar*aobQuad[3]) * matrix;
}


/***************************************************************************************************
*
*	FUNCTION		CFrustum::GetClipPlanes
*
*	DESCRIPTION		Computes the 6 frustum clip planes (in the convex hull sense).
*
***************************************************************************************************/
void CFrustum::GetClipPlanes(CPlane* pobPlanes, const CMatrix& matrix ) const
{
	// build the debug volume (in view space)
	CPoint aobPositions[8];
	GetExtremePoints(&aobPositions[0], matrix);

	// build the bounding planes (in view space)
    pobPlanes[0] = CPlane(aobPositions[2], aobPositions[6], aobPositions[4]);
    pobPlanes[1] = CPlane(aobPositions[3], aobPositions[7], aobPositions[6]);
    pobPlanes[2] = CPlane(aobPositions[1], aobPositions[5], aobPositions[7]);
    pobPlanes[3] = CPlane(aobPositions[0], aobPositions[4], aobPositions[5]);
    pobPlanes[4] = CPlane(aobPositions[4], aobPositions[6], aobPositions[7]);
    pobPlanes[5] = CPlane(-m_aobClipPlanes[4].GetNormal(), -m_aobClipPlanes[4].GetNormal().Dot(CDirection(aobPositions[0])));
};


/***************************************************************************************************
*
*	FUNCTION		CFrustum::DebugRender
*
*	DESCRIPTION		Renders a wireframe frustum via debug primitives.
*
***************************************************************************************************/

void CFrustum::DebugRender(uint32_t dwColour) const
{
#ifndef _GOLD_MASTER
	// build the debug volume (in view space)
	CPoint aobPositions[8];
	GetExtremePoints(&aobPositions[0]);

	// convert to world space
	for(int iVertex = 0; iVertex < 8; ++iVertex)
        aobPositions[iVertex] = aobPositions[iVertex]*m_pobCamera->GetViewTransform()->GetWorldMatrix();

	// render in wireframe
	g_VisualDebug->RenderLine(aobPositions[0], aobPositions[1], dwColour);
	g_VisualDebug->RenderLine(aobPositions[1], aobPositions[3], dwColour);
	g_VisualDebug->RenderLine(aobPositions[3], aobPositions[2], dwColour);
	g_VisualDebug->RenderLine(aobPositions[2], aobPositions[0], dwColour);

	g_VisualDebug->RenderLine(aobPositions[4], aobPositions[5], dwColour);
	g_VisualDebug->RenderLine(aobPositions[5], aobPositions[7], dwColour);
	g_VisualDebug->RenderLine(aobPositions[7], aobPositions[6], dwColour);
	g_VisualDebug->RenderLine(aobPositions[6], aobPositions[4], dwColour);

	g_VisualDebug->RenderLine(aobPositions[0], aobPositions[4], dwColour);
	g_VisualDebug->RenderLine(aobPositions[1], aobPositions[5], dwColour);
	g_VisualDebug->RenderLine(aobPositions[2], aobPositions[6], dwColour);
	g_VisualDebug->RenderLine(aobPositions[3], aobPositions[7], dwColour);
#endif
}

#define DW_AS_FLT(DW) (*(FLOAT*)&(DW))
#define FLT_AS_DW(F) (*(uint32_t*)&(F))
#define FLT_SIGN(F) ((FLT_AS_DW(F) & 0x80000000L))
#define ALMOST_ZERO(F) ((FLT_AS_DW(F) & 0x7f800000L)==0)
#define IS_SPECIAL(F)  ((FLT_AS_DW(F) & 0x7f800000L)==0x7f800000L)

///////////////////////////////////////////////////////////////////////////
//  PlaneIntersection
//    computes the point where three planes intersect
//    returns whether or not the point exists.
static inline bool PlaneIntersection( CPoint* intersectPt, const CPlane* p0, const CPlane* p1, const CPlane* p2 )
{
    CDirection n1_n2, n2_n0, n0_n1;  
    
	n1_n2 = p1->GetNormal().Cross( p2->GetNormal() );
	n2_n0 = p2->GetNormal().Cross( p0->GetNormal() );
	n0_n1 = p0->GetNormal().Cross( p1->GetNormal() );

    float cosTheta = p0->GetNormal().Dot( n1_n2 );
    
    if ( ALMOST_ZERO(cosTheta) || IS_SPECIAL(cosTheta) )
        return false;

    float secTheta = 1.f / cosTheta;

    n1_n2 = n1_n2 * p0->GetDistance();
    n2_n0 = n2_n0 * p1->GetDistance();
    n0_n1 = n0_n1 * p2->GetDistance();

    *intersectPt = CPoint(-(n1_n2 + n2_n0 + n0_n1) * secTheta);
    return true;
}

//  this function tests if the projection of a bounding sphere along the light direction intersects
//  the view frustum 
static bool SweptSpherePlaneIntersect(float& t0, float& t1, const CPlane* plane, const CPoint* center, const float radius, const CDirection* sweepDir)
{
	float b_dot_n = plane->GetNormal().Dot( CDirection(*center) ) + plane->GetDistance();
    float d_dot_n = plane->GetNormal().Dot( *sweepDir );

    if (d_dot_n == 0.f)
    {
        if (b_dot_n <= radius)
        {
            //  effectively infinity
            t0 = 0.f;
            t1 = 1e32f;
            return true;
        }
        else
            return false;
    }
    else
    {
        float tmp0 = ( radius - b_dot_n) / d_dot_n;
        float tmp1 = (-radius - b_dot_n) / d_dot_n;
        t0 = min(tmp0, tmp1);
        t1 = max(tmp0, tmp1);
        return true;
    }
}
CullingFrustum::CullingFrustum( const CMatrix& matrix )
{
	m_frustumMatrix = matrix;

	m_bInverseOk = false;

    //  build a view frustum based on the current view & projection matrices...
    CVector column1( matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0] );
    CVector column2( matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1] );
    CVector column3( matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2] );
    CVector column4( matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3] );

    CVector planes[6];
    planes[0] = column4 - column1;  // left
    planes[1] = column4 + column1;  // right
    planes[2] = column4 - column2;  // bottom
    planes[3] = column4 + column2;  // top
    planes[4] = column4 - column3;  // near
    planes[5] = column4 + column3;  // far
    // ignore near & far plane
   
    int p;

    for (p=0; p<6; p++)  // normalize the planes
    {
        float dot = planes[p].X()*planes[p].X() + planes[p].Y()*planes[p].Y() + planes[p].Z()*planes[p].Z();
        dot = 1.f / sqrtf(dot);
        planes[p] = planes[p] * dot;
    }

    for (p=0; p<6; p++)
        m_CamPlanes[p] = CPlane( planes[p].X(), planes[p].Y(), planes[p].Z(), planes[p].W() );

    //  build a bit-field that will tell us the indices for the nearest and farthest vertices from each plane...
    for (int i=0; i<6; i++)
        m_nVertexLUT[i] =	((planes[i].X()<0.f)?1:0) | 
							((planes[i].Y()<0.f)?2:0) | 
							((planes[i].Z()<0.f)?4:0);

    for (int i=0; i<8; i++)  // compute extrema
    {
        const CPlane& p0 = (i&1)?m_CamPlanes[4] : m_CamPlanes[5];
        const CPlane& p1 = (i&2)?m_CamPlanes[3] : m_CamPlanes[2];
        const CPlane& p2 = (i&4)?m_CamPlanes[0] : m_CamPlanes[1];

		CPoint pnt;
        PlaneIntersection( &pnt, &p0, &p1, &p2 );
		m_pntList[i] = CPoint( pnt.X(), pnt.Y(), pnt.Z() );
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
}

//  test if a sphere is within the view frustum
CullingFrustum::TEST_STATUS CullingFrustum::TestSphere(const CSphereBound* sphere) const
{
    bool inside = true;
	CDirection center( sphere->GetPosition().X(), sphere->GetPosition().Y(), sphere->GetPosition().Z() ); 
	float radius = sphere->GetRadius();

    for (int i=0; (i<6) && inside; i++)
		inside &= ((m_CamPlanes[i].GetNormal().Dot(center) + m_CamPlanes[i].GetDistance() + radius) >= 0.f);

	return inside ? PARTIALLY_INSIDE : COMPLETELY_OUTSIDE;
}

//  Tests if an AABB is inside/intersecting the view frustum
CullingFrustum::TEST_STATUS CullingFrustum::TestBox( const CAABB* box ) const
{
    bool intersect = false;

    for (int i=0; i<6; i++)
    {
        int nV = m_nVertexLUT[i];
        // pVertex is diagonally opposed to nVertex
        CDirection nVertex(	(nV&1)?box->Min().X() : box->Max().X(), 
								(nV&2)?box->Min().Y() : box->Max().Y(), 
								(nV&4)?box->Min().Z() : box->Max().Z() );
        CDirection pVertex(	(nV&1)?box->Max().X() : box->Min().X(), 
								(nV&2)?box->Max().Y() : box->Min().Y(), 
								(nV&4)?box->Max().Z() : box->Min().Z() );

		if ( m_CamPlanes[i].GetNormal().Dot( nVertex ) + m_CamPlanes[i].GetDistance() < 0.f )
            return COMPLETELY_OUTSIDE;
        if ( m_CamPlanes[i].GetNormal().Dot( pVertex ) + m_CamPlanes[i].GetDistance() < 0.f )
            intersect = true;
    }

    return (intersect) ? PARTIALLY_INSIDE : COMPLETELY_INSIDE;
}

//  Tests if an AABB is inside or completely outside the view frustum
CullingFrustum::TEST_STATUS CullingFrustum::FastTestBox( const CAABB* box ) const
{
    for (int i=0; i<6; i++)
    {
        int nV = m_nVertexLUT[i];
        CDirection nVertex(	(nV&1)?box->Min().X() : box->Max().X(), 
								(nV&2)?box->Min().Y() : box->Max().Y(), 
								(nV&4)?box->Min().Z() : box->Max().Z() );

		if ( m_CamPlanes[i].GetNormal().Dot( nVertex ) + m_CamPlanes[i].GetDistance() < 0.f )
            return COMPLETELY_OUTSIDE;
    }

    return PARTIALLY_INSIDE;
}


CullingFrustum::TEST_STATUS CullingFrustum::TestSweptSphere(const CSphereBound *sphere, const CDirection *sweepDir) const
{
	ntAssert( sphere );
	ntAssert( sweepDir );

	CDirection dirSweep( sweepDir->X(), sweepDir->Y(), sweepDir->Z() );
	CPoint center( sphere->GetPosition().X(), sphere->GetPosition().Y(), sphere->GetPosition().Z() ); 
	float radius = sphere->GetRadius();

    //  algorithm -- get all 12 intersection points of the swept sphere with the view frustum
    //  for all points >0, displace sphere along the sweep driection.  if the displaced sphere
    //  is inside the frustum, return TRUE.  else, return FALSE
    float displacements[12];
    int cnt = 0;
    float a, b;
    bool inFrustum = false;

    for (int i=0; i<6; i++)
    {
        if (SweptSpherePlaneIntersect(a, b, &m_CamPlanes[i], &center, radius, &dirSweep))
        {
            if (a>=0.f)
                displacements[cnt++] = a;
            if (b>=0.f)
                displacements[cnt++] = b;
        }
    }

    for (int i=0; i<cnt; i++)
    {
		CSphereBound dispSphere;
		dispSphere.SetPosition( sphere->GetPosition() + displacements[i] * (*sweepDir) );
		dispSphere.SetRadius( sphere->GetRadius() * 1.1f ); // spot the fudge factor...
		inFrustum |= (TestSphere(&dispSphere) == COMPLETELY_OUTSIDE)? false : true;
    }
	return inFrustum ? PARTIALLY_INSIDE : COMPLETELY_OUTSIDE;
}

void CullingFrustum::GetExtremePoints( CPoint outPoints[8] )
{
	NT_MEMCPY( outPoints, m_pntList, sizeof(CPoint)*8 );
}

CAABB CullingFrustum::GetFrustumBox()
{
	return m_frustumBox;
}

void CullingFrustum::ClipPolyAgainstPlane( const CPlane& plane, int vertexCount, CPoint* inVertex, int& outVertexCount, CPoint* outVertex ) const
{
	outVertexCount = 0;
	CDirection s( inVertex[vertexCount-1].X(), inVertex[vertexCount-1].Y(), inVertex[vertexCount-1].Z() );
	float d1 = plane.GetNormal().Dot(s) + plane.GetDistance();

	for( int vertNum = 0; vertNum < vertexCount;vertNum++ )
	{			
		CDirection p( inVertex[vertNum].X(), inVertex[vertNum].Y(), inVertex[vertNum].Z() );
		float d0 = plane.GetNormal().Dot(p) + plane.GetDistance();

		if( d0 > 0 )
		{
			if( d1 > 0 )
			{
				outVertex[outVertexCount++] = CPoint(p.X(), p.Y(), p.Z());
			} else
			{
				CDirection dir = p - s;
				float theta = plane.GetNormal().Dot( dir );		
				float t = -d1 / theta;
//				ntAssert( t >= 0.f && t <= 1.f );

				CDirection i =  s + t * dir;
				outVertex[outVertexCount++] = CPoint(i.X(), i.Y(), i.Z());
				outVertex[outVertexCount++] = CPoint(p.X(), p.Y(), p.Z());
			}
		} else
		{
			if( d1 > 0 )
			{
				CDirection dir = p - s;
				float theta = plane.GetNormal().Dot( dir );		
				float t = -d1 / theta;
//					ntAssert( t >= 0.f && t <= 1.f );
				CDirection i =  s + t * dir;
				outVertex[outVertexCount++] = CPoint(i.X(), i.Y(), i.Z());
			}
		}

		s = p;
		d1 = d0;
	}
}
CPoint* CullingFrustum::ClipPoly( int vertexCount, CPoint* inVertex, int& outVertexCount, CPoint* outVertex ) const
{
   	// start with last vertex
	for( int planeNum = 0;planeNum < 6;planeNum++ )
	{
		ClipPolyAgainstPlane( m_CamPlanes[planeNum], vertexCount, inVertex, outVertexCount, outVertex );

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

namespace 
{
	CAABB SplitBox( const CPoint& minPoint, const CPoint& midPoint, const CPoint& maxPoint, const unsigned index )
	{

		switch( index )
		{
		case 0x0: 
			return CAABB(	CPoint( minPoint.X(), minPoint.Y(), minPoint.Z() ), 
							CPoint( midPoint.X(), midPoint.Y(), midPoint.Z() ) );
		case 0x1:
			return CAABB(	CPoint( midPoint.X(), minPoint.Y(), minPoint.Z() ), 
							CPoint( maxPoint.X(), midPoint.Y(), midPoint.Z() ) );
		case 0x2:
			return CAABB(	CPoint( minPoint.X(), midPoint.Y(), minPoint.Z() ), 
							CPoint( midPoint.X(), maxPoint.Y(), midPoint.Z() ) );
		case 0x3:
			return CAABB(	CPoint( midPoint.X(), midPoint.Y(), minPoint.Z() ), 
							CPoint( maxPoint.X(), maxPoint.Y(), midPoint.Z() ) );
		case 0x4:
			return CAABB(	CPoint( minPoint.X(), minPoint.Y(), midPoint.Z() ), 
							CPoint( midPoint.X(), midPoint.Y(), maxPoint.Z() ) );
		case 0x5:
			return CAABB(	CPoint( midPoint.X(), minPoint.Y(), midPoint.Z() ), 
							CPoint( maxPoint.X(), midPoint.Y(), maxPoint.Z() ) );
		case 0x6:
			return CAABB(	CPoint( minPoint.X(), midPoint.Y(), midPoint.Z() ), 
							CPoint( midPoint.X(), maxPoint.Y(), maxPoint.Z() ) );
		case 0x7:
			return CAABB(	CPoint( midPoint.X(), midPoint.Y(), midPoint.Z() ), 
							CPoint( maxPoint.X(), maxPoint.Y(), maxPoint.Z() ) );
		default:
			return CAABB();
		}
	}
}

void CullingFrustum::RecursiveIntersect( const CAABB& testBox, const float minSplitDistance, CAABB& resultBox, int recursionLevel ) const
{
	static const int MAX_INTERSECT_RECURSION = 11;

	if( recursionLevel > MAX_INTERSECT_RECURSION )
	{
		resultBox.Union( testBox );
		return;
	}
	CPoint minPoint = testBox.Min();
	CPoint midPoint = (testBox.Max()+testBox.Min()) * 0.5f;
	CPoint maxPoint = testBox.Max();

	CPoint boxExtents = testBox.Max() - testBox.Min();

	if( boxExtents.X() < 0.00001f )
		boxExtents.X() = 0.00001f;
	if( boxExtents.Y() < 0.00001f )
		boxExtents.Y() = 0.00001f;
	if( boxExtents.Z() < 0.00001f )
		boxExtents.Z() = 0.00001f;

	unsigned int splitMask = 0x0; // do all axises


	if( (boxExtents.X()*boxExtents.X()) < (minSplitDistance*minSplitDistance) )
	{
		splitMask |= 0x1; // no X
		midPoint.X() = maxPoint.X();
	}
	if( (boxExtents.Y()*boxExtents.Y()) < (minSplitDistance*minSplitDistance) )
	{
		splitMask |= 0x2; // no Y
		midPoint.Y() = maxPoint.Y();
	}
	if( (boxExtents.Z()*boxExtents.Z()) < (minSplitDistance*minSplitDistance) )
	{
		splitMask |= 0x4; // no Z
		midPoint.Z() = maxPoint.Z();
	}
	if( splitMask == 0x7 )
	{
		resultBox.Union( testBox );
		return;
	}

	// split the box 8 ways, test each one if its clipping, split and recurse
	for( int i =0;i < 8;i++)
	{
		if( (i & splitMask)==0 )
		{
			const CAABB splitBox = SplitBox( minPoint, midPoint, maxPoint, i );
			CullingFrustum::TEST_STATUS status = TestBox( &splitBox );
			switch( status )
			{
			case CullingFrustum::PARTIALLY_INSIDE:
					RecursiveIntersect( splitBox, minSplitDistance, resultBox, ++recursionLevel );
					break;
			case CullingFrustum::COMPLETELY_INSIDE:
					resultBox.Union( splitBox );
					break;
			case CullingFrustum::COMPLETELY_OUTSIDE:
			default:
				// do nothing
				break;
			}
		}
	}
}

#define BROKEN_CLIPPER

#if !defined(BROKEN_CLIPPER)

CAABB CullingFrustum::Intersect( const CAABB& inBox, const float minSplitDistance ) const
{
	CGatso::Start( "CullingFrustum::Intersect" );
	CAABB resultAABB( CONSTRUCT_INFINITE_NEGATIVE );

	CullingFrustum::TEST_STATUS status = TestBox( &inBox );

	switch( status )
	{
	case CullingFrustum::PARTIALLY_INSIDE:
			RecursiveIntersect( inBox, minSplitDistance, resultAABB, 0 );
			break;
	case CullingFrustum::COMPLETELY_INSIDE:
			resultAABB = inBox;
			break;
	case CullingFrustum::COMPLETELY_OUTSIDE:
	default:
		// do nothing
		break;
	}

	CGatso::Stop( "CullingFrustum::Intersect" );
	return resultAABB;
}

#endif

#if defined(BROKEN_CLIPPER)
CAABB CullingFrustum::Intersect( const CAABB& inBox, const float  ) const
{
	CGatso::Start( "CullingFrustum::Intersect" );
	CAABB resultAABB( CONSTRUCT_INFINITE_NEGATIVE );

	CAABB box = m_frustumBox;
	box.Intersection( inBox );
	if( !box.IsValid() )
	{
		CGatso::Stop( "CullingFrustum::Intersect" );
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
	outputArray = ClipPoly( 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB.Union( outputArray[i] );
	}
	// right
	inputArray[0] = pnts[2]; inputArray[1] = pnts[6]; inputArray[2] = pnts[5];	inputArray[3] = pnts[1];
	outputArray = ClipPoly( 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB.Union( outputArray[i] );
	}
	// back
	inputArray[0] = pnts[6]; inputArray[1] = pnts[7]; inputArray[2] = pnts[4];	inputArray[3] = pnts[5];
	outputArray = ClipPoly( 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB.Union( outputArray[i] );
	}
	// left
	inputArray[0] = pnts[7]; inputArray[1] = pnts[3]; inputArray[2] = pnts[0];	inputArray[3] = pnts[4];
	outputArray = ClipPoly( 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB.Union( outputArray[i] );
	}
	// top
	inputArray[0] = pnts[7]; inputArray[1] = pnts[6]; inputArray[2] = pnts[2];	inputArray[3] = pnts[3];
	outputArray = ClipPoly( 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB.Union( outputArray[i] );
	}
	// bottom
	inputArray[0] = pnts[0]; inputArray[1] = pnts[1]; inputArray[2] = pnts[5];	inputArray[3] = pnts[4];
	outputArray = ClipPoly( 4, inputArray, outCount, tmpArray );
	for(int i=0;i < outCount;i++)
	{
		resultAABB.Union( outputArray[i] );
	}

	CGatso::Stop( "CullingFrustum::Intersect" );

	return resultAABB;
}

#endif

