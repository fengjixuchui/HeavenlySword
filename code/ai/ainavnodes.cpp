/***************************************************************************************************
*
*	DESCRIPTION		ainavnodes.cpp - implements volumes, points etc. that will form the basis of
*					the search space for the AI navigation component 
*
*	NOTES
*
***************************************************************************************************/

#include "Physics/world.h"
#include "Physics/havokincludes.h"

#include "ai/ainavedge.h"
#include "ai/ainavnodes.h"
#include "ai/ainavgraphmanager.h"
#include "ai/ainavpath.h"
#include "ai/ainavpreprocess.h"
#include "ai/ainavdata.h"
#include "ai/ainavnodes.h"
#include "ai/aiintersect.h"

#include "core/visualdebugger.h"
#include "game/keywords.h"					// attn! deano! library depentant on game




/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAINavNodeAABB::CAINavNodeAABB() : m_obWorldMin(CONSTRUCT_CLEAR), m_obWorldMax(CONSTRUCT_CLEAR)
{
}


/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAINavNodeAABB::PostConstruct()
{
	// convert the AABB into a region
	float y = (m_obWorldMin.Y() + m_obWorldMax.Y()) * 0.5f;

	// vert 1 = max, max
	CAIRegionVertex* vert1 = NT_NEW_CHUNK( Mem::MC_AI ) CAIRegionVertex;
	ntAssert( vert1 );
	vert1->m_obPos.X() = m_obWorldMax.X();
	vert1->m_obPos.Z() = m_obWorldMax.Z();
	vert1->m_obPos.Y() = y;

	// vert 2 = max, min
	CAIRegionVertex* vert2 = NT_NEW_CHUNK( Mem::MC_AI ) CAIRegionVertex;
	ntAssert( vert2 );
	vert2->m_obPos.X() = m_obWorldMax.X();
	vert2->m_obPos.Z() = m_obWorldMin.Z();
	vert2->m_obPos.Y() = y;

	// vert 3 = min, min
	CAIRegionVertex* vert3 = NT_NEW_CHUNK( Mem::MC_AI ) CAIRegionVertex;
	ntAssert( vert3 );
	vert3->m_obPos.X() = m_obWorldMin.X();
	vert3->m_obPos.Z() = m_obWorldMin.Z();
	vert3->m_obPos.Y() = y;

	// vert 4 = min, max
	CAIRegionVertex* vert4 = NT_NEW_CHUNK( Mem::MC_AI ) CAIRegionVertex;
	ntAssert( vert4 );
	vert4->m_obPos.X() = m_obWorldMin.X();
	vert4->m_obPos.Z() = m_obWorldMax.Z();
	vert4->m_obPos.Y() = y;

	// add the verts to the region
	CAINavNodeRegion* region = NT_NEW_CHUNK( Mem::MC_AI ) CAINavNodeRegion;
	ntAssert( region );
	region->m_obPoints.push_back( vert1 );
	region->m_obPoints.push_back( vert2 );
	region->m_obPoints.push_back( vert3 );
	region->m_obPoints.push_back( vert4 );

	region->m_fWorldMaxHeight = m_obWorldMax.Y();
	region->m_fWorldMinHeight = m_obWorldMin.Y();

	region->m_obParentNavSet = m_obParentNavSet;
	region->m_obNavSetName	 = m_obNavSetName;

	// call the postconstructors
	vert1->PostConstruct();
	vert2->PostConstruct();
	vert3->PostConstruct();
	vert4->PostConstruct();
	region->PostConstruct();

	// store this stuff so we can free it later
	m_pobRegion = region;
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAINavNodeAABB::~CAINavNodeAABB()
{
	RegionVertexList::iterator vertItr = m_pobRegion->m_obPoints.begin();
	for(; vertItr != m_pobRegion->m_obPoints.end(); ++vertItr)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*vertItr) );
	}

	NT_DELETE_CHUNK( Mem::MC_AI, m_pobRegion );
}




/***************************************************************************************************
*
*	CLASS			CAINavNodeRegion		
*
*	DESCRIPTION		
*
***************************************************************************************************/

#include "gfx/triangle.h"	// borrow Frank's triangle class for now


/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAINavNodeRegion::MakeBoundingBox()
{
	m_WorldMax.X() = -(FLT_MAX);
	m_WorldMax.Y() = m_fWorldMaxHeight;
	m_WorldMax.Z() = -(FLT_MAX);

	m_WorldMin.X() = FLT_MAX;
	m_WorldMin.Y() = m_fWorldMinHeight;
	m_WorldMin.Z() = FLT_MAX;


	RegionVertexList::iterator itr = m_obPoints.begin();
	for(; itr != m_obPoints.end(); ++itr)
	{
		if ((*itr)->m_obPos.X() > m_WorldMax.X())
		{
			m_WorldMax.X() = (*itr)->m_obPos.X();
		}
		if ((*itr)->m_obPos.Z() > m_WorldMax.Z())
		{
			m_WorldMax.Z() = (*itr)->m_obPos.Z();
		}

		if ((*itr)->m_obPos.X() < m_WorldMin.X())
		{
			m_WorldMin.X() = (*itr)->m_obPos.X();
		}
		if ((*itr)->m_obPos.Z() < m_WorldMin.Z())
		{
			m_WorldMin.Z() = (*itr)->m_obPos.Z();
		}
	}
}

void CAINavNodeRegion::GetBoundingBox( CPoint& min, CPoint& max )
{
	min = m_WorldMin;
	max = m_WorldMax;
}

void CAINavNodeRegion::PostConstruct()
{
	// add this region to the navgraphmanager
	CAINavGraphManager::Get().AddRegion( this );
	MakeBoundingBox();
}

//#define NAVNODE_PRINT

void CAINavNodeRegion::Preprocess( void )
{
	RegionVertexList::iterator clearItr = m_obPoints.begin();
	for(; clearItr != m_obPoints.end(); ++clearItr)
	{
		(*clearItr)->m_bRemoved = false;
		(*clearItr)->m_obPos.Y() = 	m_fWorldMaxHeight;
	}

	// copy the region's vertices into the data structures required for triangulation
	m_aVertices = NT_NEW_CHUNK( Mem::MC_AI ) double[m_obPoints.size() + 1][2];	// size +1 because triangulation algorithm starts from aVertices[1]

	RegionVertexList::iterator copyItr = m_obPoints.begin();
	int count = 1;
	for(; copyItr != m_obPoints.end(); ++copyItr)
	{
		m_aVertices[count][0] = (*copyItr)->m_obPos.X();
		m_aVertices[count][1] = (*copyItr)->m_obPos.Z();

		++count;
	}

	// allocate memory for results
	int iNumOutputTris = AINavPreprocess::GetNumOutputTris( m_obPoints.size(), 0 );
	m_aTriangles = NT_NEW_CHUNK( Mem::MC_AI ) int[iNumOutputTris][3];
	int numPoints[2] = { m_obPoints.size(), 0 };

	// triangulate
	bool reversed;
	int err = AINavPreprocess::TriangulatePolygon( 1, numPoints, m_aVertices, m_aTriangles, reversed );

	if (err)
	{
		// flag the region as broken, delete any allocations and jump out of the boat
		NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aVertices );
		m_aVertices = NULL;
		NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aTriangles );
		m_aTriangles = NULL;

		ntPrintf( "failed to triangulate region %x\n", this );

		return;
	}

	if (reversed)
	{
		AIPointList	tempList;

		RegionVertexList::iterator revItr = m_obPoints.begin();
		for (;revItr != m_obPoints.end(); ++revItr)
		{
			CPoint* newPoint = NT_NEW_CHUNK( Mem::MC_AI ) CPoint( (*revItr)->m_obPos );
			tempList.push_front( newPoint );
		}

		revItr = m_obPoints.begin();
		AIPointList::iterator tempListItr = tempList.begin();
		for (; revItr != m_obPoints.end(); ++revItr, ++tempListItr)
		{
			(*revItr)->m_obPos = *(*tempListItr);
			NT_DELETE_CHUNK( Mem::MC_AI, *tempListItr );
		}
	}

	// The triangulation algorithm requires the vertices to begin at [1] rather than [0].
	// This makes handling the vertex array in later code ntError-prone, so we'll rebase to 0
	double	(*aRebasedVertices)[2] = NT_NEW_ARRAY_CHUNK( Mem::MC_AI ) double[m_obPoints.size()][2];
	NT_MEMCPY( aRebasedVertices, &(m_aVertices[1]), m_obPoints.size() * 2 * sizeof( double ) );
	NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aVertices );
	m_aVertices = aRebasedVertices;

	RegionVertexList::iterator testItr = m_obPoints.begin();
	for (int i = 0; testItr != m_obPoints.end(); ++testItr, ++i)
	{
#ifdef NAVNODE_PRINT
		ntPrintf( "points.X == %f,  verts.X == %f\n", (*testItr)->m_obPos.X(), m_aVertices[i][0] );
		ntPrintf( "points.Y == %f,  verts.Z == %f\n", (*testItr)->m_obPos.Z(), m_aVertices[i][1] );
#endif
	}


	// now decrement all the triangle indices
	for (int i = 0; i < iNumOutputTris; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			--m_aTriangles[i][j];
			ntAssert( m_aTriangles[i][j] >= 0);
		}
	}

	// make a list of connections
	for (int i = 0; i < iNumOutputTris; ++i)
	{
		//for (int j = 0; j < iNumOutputTris; ++j)
		for (int j = i; j < iNumOutputTris; ++j)
		{
			if (i == j)
			{
				continue;
			}

			int numMatchingVerts = 0;
			int matchingVertIndices[2] = {-1, -1};

			for (int p = 0; p < 3; ++p)
			{
				for (int q = 0; q < 3; ++q)
				{
					if (m_aTriangles[i][p] == m_aTriangles[j][q])
					{
						matchingVertIndices[numMatchingVerts++] = m_aTriangles[i][p];
						break;
					}
				}
			}
			ntAssert( numMatchingVerts <= 2 );
			if (numMatchingVerts == 2)
			{
				// we have a connection between triangles i and j,
				// shared edge is from vert matchingVertIndices[0] to matchingVertIndices[1]
#ifdef NAVNODE_PRINT
				ntPrintf( "connection: tri[%d] -> tri[%d]   vert[%d] -> vert[%d]\n", i, j, matchingVertIndices[0], matchingVertIndices[1] );
#endif

				float x = (float)(m_aVertices[matchingVertIndices[0]][0] + m_aVertices[matchingVertIndices[1]][0]) * 0.5f;	
				float z = (float)(m_aVertices[matchingVertIndices[0]][1] + m_aVertices[matchingVertIndices[1]][1]) * 0.5f;

				CPoint* obConnection = NT_NEW_CHUNK( Mem::MC_AI ) CPoint( x, m_fWorldMaxHeight, z );
				m_obConnectionPositions.push_back( obConnection );

				AINavDataStructs::Connection* connection = NT_NEW AINavDataStructs::Connection;
				connection->tris[0] = i;
				connection->tris[1] = j;
				connection->verts[0] = matchingVertIndices[0];
				connection->verts[1] = matchingVertIndices[1];
				m_obConnections.push_back( connection );
			}
		}
	}

	// copy result triangles back
	for (int i = 0; i < iNumOutputTris; ++i)
	{
		CVector vecA( (float)m_aVertices[m_aTriangles[i][0]][0], m_fWorldMaxHeight, (float)m_aVertices[m_aTriangles[i][0]][1] , 0.0f ); 
		CVector vecB( (float)m_aVertices[m_aTriangles[i][1]][0], m_fWorldMaxHeight, (float)m_aVertices[m_aTriangles[i][1]][1] , 0.0f ); 
		CVector vecC( (float)m_aVertices[m_aTriangles[i][2]][0], m_fWorldMaxHeight, (float)m_aVertices[m_aTriangles[i][2]][1] , 0.0f ); 

		Triangle* obTri = NT_NEW_CHUNK( Mem::MC_AI ) Triangle( vecA, vecB, vecC );
		m_obTriangles.push_back( obTri );		
	}
}


void CAINavNodeRegion::PaulsDebugRender( void ) const
{
/*
	// render connections
	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	float	fRadius = 1.1f;

	// render triangles 
	AITriangleList::const_iterator triangleItr = m_obTriangles.begin();
	for(; triangleItr != m_obTriangles.end(); ++triangleItr)
	{
		(*triangleItr)->DebugDraw( 0xFFFFFFFF );
	}

	AIPointList::const_iterator connectionItr = m_obConnectionPositions.begin();
	for(; connectionItr != m_obConnectionPositions.end(); ++connectionItr)
	{
		g_VisualDebug->RenderSphere( obOrientation, *(*connectionItr), fRadius, 0xffffff0f );
	}

	// render external connections
	ntstd::List<CPoint*>::const_iterator externalConnectionPosItr = m_obExternalConnectionPositions.begin();
	for(; externalConnectionPosItr != m_obExternalConnectionPositions.end(); ++externalConnectionPosItr)
	{
		g_VisualDebug->RenderSphere( obOrientation, *(*externalConnectionPosItr), fRadius, 0xffff0fff );
	}
*/
}

static CPoint GetCentre( CPoint& min, CPoint& max )
{
	return 0.5f * ( max + min );
}

static CPoint GetExtents( CPoint& min, CPoint& max )
{
	return 0.5f * ( max - min );
}


static bool	Overlaps( CPoint& min1, CPoint& max1, CPoint& min2, CPoint& max2 )
{
	CPoint	extents1( GetExtents( min1, max1 ) );
	CPoint	extents2( GetExtents( min2, max2 ) );

	CPoint	centre1( GetCentre( min1, max1 ) );
	CPoint	centre2( GetCentre( min2, max2 ) );

	const CPoint diff = centre2 - centre1;	//vector from A to B
	return 
		fabs(diff.X()) <= (fabs(extents1.X()) + fabs(extents2.X()))
		&&
		fabs(diff.Y()) <= (fabs(extents1.Y()) + fabs(extents2.Y()))
		&&
		fabs(diff.Z()) <= (fabs(extents1.Z()) + fabs(extents2.Z()));
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

int CAINavNodeRegion::ContainingTriangle( CPoint&	pos )
{
	UNUSED(pos);

	AITriangleList::iterator itr =  m_obTriangles.begin();
	for (int i = 0; itr != m_obTriangles.end(); ++itr, ++i)
	{
		CPoint a((*itr)->m_vertices[0]);
		CPoint b((*itr)->m_vertices[1]);
		CPoint c((*itr)->m_vertices[2]);
		if (AIIntersect::PointInTriangle( pos, a, b, c ))
		{
			return i;
		}
	}

	return -1;
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAINavNodeRegion::MakeConnections( CAINavNodeRegion*	otherRegion )
{
	if (m_aVertices == NULL)
	{
		return;
	}

	// test for overlap in bounding boxes
	CPoint otherMin;
	CPoint otherMax;
	otherRegion->GetBoundingBox( otherMin, otherMax );


	if (Overlaps( m_WorldMin, m_WorldMax, otherMin, otherMax ))
	{
		typedef ntstd::List<int, Mem::MC_AI> EdgeList;
		EdgeList containedList;

		// test each vert against the triangles in the other region
		RegionVertexList::iterator itr = m_obPoints.begin();
		for (int i = 0; itr != m_obPoints.end(); ++itr, ++i)
		{
#ifdef NAVNODE_PRINT
			ntPrintf( "points.X == %f,  verts.X == %f\n", (*itr)->m_obPos.X(), m_aVertices[i][0] );
			ntPrintf( "points.Y == %f,  verts.Z == %f\n", (*itr)->m_obPos.Z(), m_aVertices[i][1] );
#endif
			if (otherRegion->ContainingTriangle( (*itr)->m_obPos ) != -1)
			{
				// add verts that are in to contained list
				containedList.push_back( i );
				CPoint testPoint( (float)m_aVertices[i][0], (m_fWorldMaxHeight+m_fWorldMinHeight)*0.5f, (float)m_aVertices[i][1] );
				ntAssert( otherRegion->ContainingTriangle( testPoint ) != -1 );
			}	
		}


		// check the contained list for edges
		EdgeList::iterator vertI = containedList.begin();
		for (; vertI != containedList.end(); ++vertI)
		{
			int b = (*vertI) + 1;
			int c = (*vertI) - 1;
			int edgeVert = -1;

			if (b > (int)m_obPoints.size())
			{
				b = 0;
			}
			if (c == -1)
			{
				c = m_obPoints.size() - 1;
			}

			EdgeList::iterator vertJ = vertI;
			for (; vertJ != containedList.end(); ++vertJ)
			{
				edgeVert = -1;
				if ((*vertJ) == b)
				{
					edgeVert = b;
				}
				else if ((*vertJ) == c)
				{
					edgeVert = c;
				}
				if (edgeVert != -1)
				{
#ifdef NAVNODE_PRINT
					ntPrintf( "contained edge %d -> %d\n", (*vertI), edgeVert );
#endif

					// add to list of external connections
					CPoint a( (float)m_aVertices[(*vertI)][0], (m_fWorldMaxHeight+m_fWorldMinHeight)*0.5f, (float)m_aVertices[(*vertI)][1] );
					CPoint b( (float)m_aVertices[edgeVert][0], (m_fWorldMaxHeight+m_fWorldMinHeight)*0.5f, (float)m_aVertices[edgeVert][1] );

					ntAssert( otherRegion->ContainingTriangle( a ) != -1 );
					ntAssert( otherRegion->ContainingTriangle( b ) != -1 );

					CPoint* midPoint = NT_NEW_CHUNK( Mem::MC_AI ) CPoint( (a + b) * 0.5f );
					ntAssert( midPoint );

					m_obExternalConnectionPositions.push_back( midPoint );

					// find the triangle in this region that owns the contained edge
					int thisRegionTriIdx = -1;
					for (int i = 0; i < (int)m_obTriangles.size(); ++i)
					{
						int numMatchingVerts = 0;
						for (int j = 0; j < 3; ++j)
						{
							if (m_aTriangles[i][j] == (*vertI) || m_aTriangles[i][j] == edgeVert)
							{
								++numMatchingVerts;
							}							
						}
						if (numMatchingVerts == 2)
						{
							thisRegionTriIdx = i;
							break;
						}
					}
					ntAssert( thisRegionTriIdx != -1);

					ExternalConnection* newConnection = NT_NEW_CHUNK( Mem::MC_AI ) ExternalConnection;
					newConnection->edgeIndices[0] = (*vertI);
					newConnection->edgeIndices[1] = edgeVert;
					newConnection->triangleIdx = thisRegionTriIdx;
					newConnection->otherRegion = otherRegion;
					newConnection->otherTriangleIdx = otherRegion->ContainingTriangle( *midPoint );
					if (newConnection->otherTriangleIdx == -1)
					{
						NT_DELETE_CHUNK( Mem::MC_AI, newConnection );
					}
					else
					{
						m_obExternalConnections.push_back( newConnection );
					}
				}
			}
		}

		// for each contained edge
		
			// get the triangle containing the centre of the edge

			// get the triangle owning that edge

			// check that the edge isn't shared with any other triangle

			// store the connection (triangle idx, vertex idxs, other region pointer, other region tri idx)
	}
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAINavNodeRegion::ReBaseInternal()
{
	// add m_iBaseVertIdx to all vertex indices held by triangles
	for (int i = 0; i < (int)m_obTriangles.size(); ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			m_aTriangles[i][j] += m_iBaseVertIdx;
		}
	}

	// add base values to indices stored in internal connection arrays
	AINavDataStructsConnectionList::iterator connItr = m_obConnections.begin();
	for (; connItr != m_obConnections.end(); ++connItr)
	{
		for (int i = 0; i < 2; ++i)
		{
			(*connItr)->verts[i] += m_iBaseVertIdx;
		}
		for (int i = 0; i < 2; ++i)
		{
			(*connItr)->tris[i] += m_iBaseTriangleIdx;
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAINavNodeRegion::ReBaseExternal()
{
	ExternalConnectionList::iterator externalConnItr = m_obExternalConnections.begin();
	for(; externalConnItr != m_obExternalConnections.end(); ++externalConnItr)
	{
		for (int i = 0; i < 2; ++i)
		{
			(*externalConnItr)->edgeIndices[i] += m_iBaseVertIdx;
		}
		(*externalConnItr)->triangleIdx += m_iBaseTriangleIdx;
		(*externalConnItr)->otherTriangleIdx += (*externalConnItr)->otherRegion->GetTriBaseIdx();
	}
}


/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAINavNodeRegion::~CAINavNodeRegion()
{
	AITriangleList::iterator triItr = m_obTriangles.begin();
	for(; triItr != m_obTriangles.end(); ++triItr)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*triItr) );
	}
	AINavDataStructsConnectionList::iterator connItr = m_obConnections.begin();
	for(; connItr != m_obConnections.end(); ++connItr)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*connItr) );
	}
	AIPointList::iterator connPosItr = m_obConnectionPositions.begin();
	for(; connPosItr != m_obConnectionPositions.end(); ++connPosItr)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*connPosItr) );
	}
	ExternalConnectionList::iterator externalConnItr = m_obExternalConnections.begin();
	for(; externalConnItr != m_obExternalConnections.end(); ++externalConnItr)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*externalConnItr) );
	}
	AIPointList::iterator externalConnPosItr = m_obExternalConnectionPositions.begin();
	for(; externalConnPosItr != m_obExternalConnectionPositions.end(); ++externalConnPosItr)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*externalConnPosItr) );
	}

	NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aTriangles );
	NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aVertices );
}


