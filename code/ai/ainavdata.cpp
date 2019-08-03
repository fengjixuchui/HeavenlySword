//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ainavdata.c
//!                                                                                         
//! Looks after the C++ behaviour instances created by LUA
//!                                                                                         
//------------------------------------------------------------------------------------------

#include "ai/ainavgraphmanager.h"
#include "ainavnodes.h"
#include "aiintersect.h"
#include "camera/camgeometry.h"
#include "game/keywords.h"


#include "core/visualdebugger.h"

#include "ainavdata.h"

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::AINavData
//!                                                                                         
//! Build final navdata from the intermediate
//!                                                                                         
//------------------------------------------------------------------------------------------

AINavData::AINavData( AIIntermediateNavData*	pobIntData )
{
	ntAssert( pobIntData->m_bPreprocessed );

	// allocate memory for the vertex, triangle and connection arrays
	m_aVertices	= NT_NEW_ARRAY_CHUNK( Mem::MC_AI ) CPoint[pobIntData->m_iNumVerts];
	m_aTriangles = NT_NEW_ARRAY_CHUNK( Mem::MC_AI ) Triangle[pobIntData->m_iNumTris];
	m_aConnections =  NT_NEW_ARRAY_CHUNK( Mem::MC_AI ) AINavDataStructs::Connection[pobIntData->m_iNumConnections];
	m_aRegions =  NT_NEW_ARRAY_CHUNK( Mem::MC_AI ) Region[pobIntData->m_obRegionList.size()];
	ntAssert( m_aVertices );
	ntAssert( m_aTriangles );
	ntAssert( m_aConnections );
	ntAssert( m_aRegions );

	m_iNumVerts			= 0;
	m_iNumTriangles		= 0;
	m_iNumConnections	= 0;
	m_iNumRegions		= 0;

	int maxConnectionIdx = 0;

	AINavNodeRegionList::iterator itr = pobIntData->m_obRegionList.begin();
	for (; itr != pobIntData->m_obRegionList.end(); ++itr)
	{
		m_aRegions[m_iNumRegions].debugRenderTriangleList = NULL;

		if ((*itr)->m_aVertices == NULL)
		{
			continue;
		}

		(*itr)->GetBoundingBox( m_aRegions[m_iNumRegions].obWorldMin, m_aRegions[m_iNumRegions].obWorldMax );

		// make region centre and extents
		m_aRegions[m_iNumRegions].obCentre = 0.5f * ( m_aRegions[m_iNumRegions].obWorldMax + m_aRegions[m_iNumRegions].obWorldMin );
		m_aRegions[m_iNumRegions].obExtents = 0.5f * ( m_aRegions[m_iNumRegions].obWorldMax - m_aRegions[m_iNumRegions].obWorldMin );		

		// fill in minimum indices for region (must be done before processing triangles, verts etc.)
		m_aRegions[m_iNumRegions].minTriIndex = m_iNumTriangles;
		m_aRegions[m_iNumRegions].minVertIndex = m_iNumVerts;

		float y = ((*itr)->m_fWorldMaxHeight + (*itr)->m_fWorldMinHeight) * 0.5f;
		for (int i = 0; i < (*itr)->GetNumVertices(); ++i)
		{
			// copy vertex data
			m_aVertices[m_iNumVerts].X() = (float)(*itr)->m_aVertices[i][0];
			m_aVertices[m_iNumVerts].Z() = (float)(*itr)->m_aVertices[i][1];
			m_aVertices[m_iNumVerts].Y() = y;
			++m_iNumVerts;
		}

		for (int i = 0; i < (*itr)->GetNumTriangles(); ++i)
		{
			// copy triangle data
			m_aTriangles[m_iNumTriangles].a = (*itr)->m_aTriangles[i][0];
			m_aTriangles[m_iNumTriangles].b = (*itr)->m_aTriangles[i][1];
			m_aTriangles[m_iNumTriangles].c = (*itr)->m_aTriangles[i][2];
			
			// clear connections
			for (int j = 0 ; j < AI_CONNECTIONS_PER_TRIANGLE; ++j)
			{
				m_aTriangles[m_iNumTriangles].connections[j] = -1;
			}

			++m_iNumTriangles;
		}
		
		AINavDataStructsConnectionList::iterator connectItr = (*itr)->m_obConnections.begin();
		for (; connectItr != (*itr)->m_obConnections.end(); ++connectItr)
		{
			// copy internal connection data
			m_aConnections[m_iNumConnections] = *(*connectItr);
			++m_iNumConnections;
		}
		CAINavNodeRegion::ExternalConnectionList::iterator externalConnectItr = (*itr)->m_obExternalConnections.begin();
		for (; externalConnectItr != (*itr)->m_obExternalConnections.end(); ++externalConnectItr)
		{
			// copy external connection data
			m_aConnections[m_iNumConnections].tris[0] = (*externalConnectItr)->triangleIdx;
			m_aConnections[m_iNumConnections].tris[1] = (*externalConnectItr)->otherTriangleIdx;
			m_aConnections[m_iNumConnections].verts[0] = (*externalConnectItr)->edgeIndices[0];
			m_aConnections[m_iNumConnections].verts[1] = (*externalConnectItr)->edgeIndices[1];
			++m_iNumConnections;
		}

		// fill in maximum indices for region (must be done after processing triangles, verts etc.)
		m_aRegions[m_iNumRegions].maxTriIndex = m_iNumTriangles - 1;
		m_aRegions[m_iNumRegions].maxVertIndex = m_iNumVerts - 1;

		ntAssert( m_aRegions[m_iNumRegions].maxTriIndex > m_aRegions[m_iNumRegions].minTriIndex );
		ntAssert( m_aRegions[m_iNumRegions].maxVertIndex > m_aRegions[m_iNumRegions].minVertIndex );

		// Default the keywords to zero
		m_aRegions[m_iNumRegions].keywords = 0;

		// If given, assign the Parent Nav set string as a keyword
		if( (*itr)->m_obNavSetName.length() )
		{
			m_aRegions[m_iNumRegions].keywords = NT_NEW_CHUNK( Mem::MC_AI ) CKeywords( (*itr)->m_obNavSetName.c_str() ) ;
		}

		++m_iNumRegions;
	}

	// add connection indices to triangles
	// for every connection, read the triangle indices and copy the connection index to that triangle's connection array
	for (int i = 0; i < m_iNumConnections; ++i)
	{
		//ntPrintf( "connection ( %d -> %d )\n", m_aConnections[i].tris[0], m_aConnections[i].tris[1] );
		for (int j = 0 ; j < 2; ++j)		// each connection corresponds to 2 triangles
		{
			bool connectionWritten = false;
			for (int k = 0 ; k < AI_CONNECTIONS_PER_TRIANGLE; ++k)
			{
				if ( m_aTriangles[m_aConnections[i].tris[j]].connections[k] == -1)
				{
					m_aTriangles[m_aConnections[i].tris[j]].connections[k] = i;
					//ntPrintf( "  - written to %d\n", m_aConnections[i].tris[j] );
					connectionWritten = true;
					if (k > maxConnectionIdx)
					{
						maxConnectionIdx = k;
					}
					break;
				}
			}
			// if this assert fires, we need to increase AI_CONNECTIONS_PER_TRIANGLE
			ntAssert( connectionWritten );
		}
	}

	ntPrintf( "AINAVDATA: highest num connections on a node = %d\n", maxConnectionIdx + 1 );

	ntAssert( m_iNumVerts == pobIntData->m_iNumVerts );
	ntAssert( m_iNumTriangles == pobIntData->m_iNumTris );
	ntAssert( m_iNumConnections == pobIntData->m_iNumConnections );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::~AINavData
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

AINavData::~AINavData()
{
	NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aVertices );
	NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aTriangles );
	NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aConnections );
	for (int i = 0; i < m_iNumRegions; ++i)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, m_aRegions[i].keywords );
		m_aRegions[i].keywords = 0;
	}

	NT_DELETE_ARRAY_CHUNK( Mem::MC_AI, m_aRegions );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::GetNodeCentre
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

CPoint AINavData::GetNodeCentre( int nodeNum )
{
	ntAssert( nodeNum >= 0 && nodeNum < m_iNumTriangles );
	return ((m_aVertices[m_aTriangles[nodeNum].a] + m_aVertices[m_aTriangles[nodeNum].b] + m_aVertices[m_aTriangles[nodeNum].c]) / 3.0f);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::IsConnectionExternal
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

bool AINavData::IsConnectionExternal( int connIdx )
{
	int regions[2];
	int foundRegions = 0;
	for (int i = 0; i < m_iNumRegions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{	
			if (m_aRegions[i].minTriIndex <= m_aConnections[connIdx].tris[j] && m_aRegions[i].maxTriIndex >= m_aConnections[connIdx].tris[j])
			{
				regions[foundRegions] = i;
				++foundRegions;
			}
		}
		if (foundRegions == 2)
		{
			break;
		}
	}
	ntAssert( foundRegions == 2 );
	return regions[0] != regions[1];
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::IsPointInRegionBB
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

bool AINavData::IsPointInRegionBB( const CPoint& pos, int regionIdx )
{
	// TODO: ntAssert on regionIdx in range
	return AIIntersect::PointInAABB( pos, m_aRegions[regionIdx].obCentre, m_aRegions[regionIdx].obExtents );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::IsPointInRegionBB
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

bool AINavData::IsPointInTriangle( const CPoint& pos, int triangleIdx )
{
	return AIIntersect::PointInTriangle( pos,
		m_aVertices[m_aTriangles[triangleIdx].a],
		m_aVertices[m_aTriangles[triangleIdx].b],
		m_aVertices[m_aTriangles[triangleIdx].c] ); 
}

void AINavData::JoinPointsRender( int a, int b, int region, uint32_t colour )
{
#ifndef _GOLD_MASTER
	CPoint aUpper( m_aVertices[a].X(), m_aRegions[region].obWorldMax.Y(), m_aVertices[a].Z() );
	CPoint aLower( m_aVertices[a].X(), m_aRegions[region].obWorldMin.Y(), m_aVertices[a].Z() );
	CPoint bUpper( m_aVertices[b].X(), m_aRegions[region].obWorldMax.Y(), m_aVertices[b].Z() );
	CPoint bLower( m_aVertices[b].X(), m_aRegions[region].obWorldMin.Y(), m_aVertices[b].Z() );

	g_VisualDebug->RenderLine( aUpper, bUpper, colour );
	g_VisualDebug->RenderLine( aLower, bLower, colour );
	g_VisualDebug->RenderLine( aUpper, aLower, colour );
#endif
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::DebugRender
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

void AINavData::DebugRender()
{
#ifndef _GOLD_MASTER

	//uint32_t triangleColour = 0xFFFFFFFF;
	uint32_t internalConnectionColour = 0xFFFFFF0F;
	uint32_t externalConnectionColour = 0xFFFF0FFF;
	int region = 0;

/*
	for (int i = 0; i < m_iNumTriangles; ++i)
	{
		if (i > m_aRegions[region].maxTriIndex)
		{
			region++;
		}
		ntAssert( i <= m_aRegions[region].maxTriIndex );
		ntAssert( i >= m_aRegions[region].minTriIndex );

		CPoint aUpper( m_aVertices[m_aTriangles[i].a].X(), m_aRegions[region].obWorldMax.Y(), m_aVertices[m_aTriangles[i].a].Z() );
		CPoint bUpper( m_aVertices[m_aTriangles[i].b].X(), m_aRegions[region].obWorldMax.Y(), m_aVertices[m_aTriangles[i].b].Z() );
		CPoint cUpper( m_aVertices[m_aTriangles[i].c].X(), m_aRegions[region].obWorldMax.Y(), m_aVertices[m_aTriangles[i].c].Z() );

		CPoint aLower( m_aVertices[m_aTriangles[i].a].X(), m_aRegions[region].obWorldMin.Y(), m_aVertices[m_aTriangles[i].a].Z() );
		CPoint bLower( m_aVertices[m_aTriangles[i].b].X(), m_aRegions[region].obWorldMin.Y(), m_aVertices[m_aTriangles[i].b].Z() );
		CPoint cLower( m_aVertices[m_aTriangles[i].c].X(), m_aRegions[region].obWorldMin.Y(), m_aVertices[m_aTriangles[i].c].Z() );

		g_VisualDebug->RenderLine( aUpper, bUpper, triangleColour );
		g_VisualDebug->RenderLine( bUpper, cUpper, triangleColour );
		g_VisualDebug->RenderLine( cUpper, aUpper, triangleColour );

		g_VisualDebug->RenderLine( aLower, bLower, triangleColour );
		g_VisualDebug->RenderLine( bLower, cLower, triangleColour );
		g_VisualDebug->RenderLine( cLower, aLower, triangleColour );

		// now connect the top and bottom layers
		g_VisualDebug->RenderLine( aUpper, aLower, triangleColour );
		g_VisualDebug->RenderLine( bUpper, bLower, triangleColour );
		g_VisualDebug->RenderLine( cUpper, cLower, triangleColour );

		CPoint pt3D = GetNodeCentre( i );

		//g_VisualDebug->Printf3D( pt3D, 0xffffffff, 0, "%d", i );
	}
*/

	uint32_t lineColour = 0xA5FFFF0F;
	uint32_t polyColour = 0x45FFFF0F;

	static bool printed = false;

	for (region = 0; region < m_iNumRegions; ++region)
	{
		/*
		bool render = false;

		// only render regions containing points we're interested in
		if (	(m_aRegions[region].maxTriIndex >= 329 && m_aRegions[region].minTriIndex <= 329)
			||	(m_aRegions[region].maxTriIndex >= 367 && m_aRegions[region].minTriIndex <= 367) )
		{
			render = true;
		}
		if (!render)
		{
			continue;
		}
		*/

		for (int i = m_aRegions[region].minVertIndex; i < m_aRegions[region].maxVertIndex; ++i)
		{
			JoinPointsRender( i, i + 1, region, lineColour );
		}
		JoinPointsRender( m_aRegions[region].maxVertIndex, m_aRegions[region].minVertIndex, region, lineColour );

		// only render the polys themselves if our buffer is big enough
		int numTriangles = ((m_aRegions[region].maxTriIndex - m_aRegions[region].minTriIndex) + 1) * 2;
		if (m_aRegions[region].debugRenderTriangleList == NULL)
		{
			// allocate a buffer for the triangles we'll be rendering in this region
			m_aRegions[region].debugRenderTriangleList = NT_NEW_CHUNK( Mem::MC_AI ) CPoint[numTriangles * 3];

			int bufferIdx = 0;
			for (int i = m_aRegions[region].minTriIndex; i <= m_aRegions[region].maxTriIndex; ++i)
			{
				// build tris for top and bottom layer
				for (int j = 0; j < 2; ++j)
				{
					float y = j == 0 ? m_aRegions[region].obWorldMax.Y() : m_aRegions[region].obWorldMin.Y();

					m_aRegions[region].debugRenderTriangleList[bufferIdx] = m_aVertices[m_aTriangles[i].a];
					m_aRegions[region].debugRenderTriangleList[bufferIdx++].Y() = y;
					m_aRegions[region].debugRenderTriangleList[bufferIdx] = m_aVertices[m_aTriangles[i].b];
					m_aRegions[region].debugRenderTriangleList[bufferIdx++].Y() = y;
					m_aRegions[region].debugRenderTriangleList[bufferIdx] = m_aVertices[m_aTriangles[i].c];
					m_aRegions[region].debugRenderTriangleList[bufferIdx++].Y() = y;

				}

				//if (!printed) { ntPrintf( "region: %d  tri: %d\n", region, i ); }
				//if (!printed) { ntPrintf( "  - vert: %d\n", m_aTriangles[i].a ); }
				//if (!printed) { ntPrintf( "  - vert: %d\n", m_aTriangles[i].b ); }
				//if (!printed) { ntPrintf( "  - vert: %d\n", m_aTriangles[i].c ); }
			}
			ntAssert( bufferIdx == numTriangles * 3 );
		}

		// Show the triangle numbers
		for (int i = 0; i < m_iNumTriangles; ++i)
		{
			g_VisualDebug->Printf3D( ((m_aVertices[m_aTriangles[i].a] + m_aVertices[m_aTriangles[i].b] + m_aVertices[m_aTriangles[i].c]) / 3.0f) + CPoint(0.0f, 1.0f, 0.0f), 
									  DC_RED, 0, "%d", i );
		}

		if( m_aRegions[region].keywords )
			g_VisualDebug->Printf3D( m_aRegions[region].obCentre, polyColour, 0, "Set Name: %s", m_aRegions[region].keywords->GetKeywordString() );	
		g_VisualDebug->RenderPrimitive( m_aRegions[region].debugRenderTriangleList, numTriangles * 3, CMatrix( CONSTRUCT_IDENTITY ), polyColour, DPF_TRIANGLELIST | DPF_NOCULLING ); 
	}
	
	printed = true;

	// render connections
	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	float	fRadius = 0.1f;

	for (int i = 0; i < m_iNumConnections; ++i)
	{
		uint32_t colour = IsConnectionExternal( i ) ? externalConnectionColour : internalConnectionColour;
		CPoint connectionPoint( (m_aVertices[m_aConnections[i].verts[0]] + m_aVertices[m_aConnections[i].verts[1]]) * 0.5f );
		connectionPoint.Y() = m_aRegions[GetRegionForTriangle( m_aConnections[i].tris[0] )].obWorldMax.Y();
		g_VisualDebug->RenderSphere( obOrientation, connectionPoint, fRadius, colour );
	}

#endif
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::GetRegionForTriangle
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

int AINavData::GetRegionForTriangle( int triangleIdx )
{
	for (int i = 0; i < m_iNumRegions; i++)
	{
		if (m_aRegions[i].minTriIndex <= triangleIdx && m_aRegions[i].maxTriIndex >= triangleIdx)
		{
            return i;
		}
	}
	return -1;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::GetContainingNode
//!                                                                                         
//! Region keys, const char*, example: "nav set 1, nav set 2", this would allow the entity
//!																in those areas only. 
//! 
//------------------------------------------------------------------------------------------

int	AINavData::GetContainingNode( const CPoint& obPos, AINavHint* pHint, const CKeywords* pobKeys )
{
	int iIndex = 0;

	if( pHint )
	{
		// Check that the hint is still valid. 
		if( (pHint->m_Region >= 0 && pHint->m_Region < m_iNumRegions) && IsPointInRegionBB( obPos, pHint->m_Region ) )
		{
			if( (m_aRegions[pHint->m_Region].minTriIndex <= pHint->m_Tri && m_aRegions[pHint->m_Region].maxTriIndex >= pHint->m_Tri) &&
				IsPointInTriangle( obPos, pHint->m_Tri ) )
			{
				return pHint->m_Tri;
			}
			else
			{
				iIndex = pHint->m_Region;
			}
		}
	}


	for (; iIndex < m_iNumRegions; ++iIndex )
	{
		// If a region was requested but the data region hasn't been set then assume no
		if( !m_aRegions[ iIndex ].keywords && pobKeys )
			continue;

		// If there is a region and a region key request check that there is a match
		if( m_aRegions[ iIndex ].keywords && pobKeys && !m_aRegions[ iIndex ].keywords->ContainsAny( *pobKeys ) )
			continue;

		if (IsPointInRegionBB( obPos, iIndex ))
		{
			// check the triangles in the region
			for (int j = m_aRegions[iIndex].minTriIndex; j <= m_aRegions[iIndex].maxTriIndex; ++j )
			{
                if (IsPointInTriangle( obPos, j ))
				{
					if( pHint )
					{
						pHint->m_Region = (short) iIndex;
						pHint->m_Tri	= (short) j;
					}

					return j;
				}
			}
		}
	}
	return -1;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::GetIntersectionEdge 
//!                                                                                         
//!	Little experiment to allow entities to query the navgraph along a line segment and find 
//!	any endpoint along the way.
//! 
//! Resulting matrix:
//!		matrix.vec[0] = point 1 of the edge on the navgraph where the collision was
//!		matrix.vec[1] = point 2 of the edge
//!		matrix.vec[2] = the edge normal
//!		matrix.vec[3] = the intersect point
//! 
//------------------------------------------------------------------------------------------

bool AINavData::GetIntersectionEdge( u_int uiTri, const CPoint& obPIn, const CPoint& obPOut, CMatrix* pResult )
{
	// 
	enum HIT_EDGE { EDGE_NIL, EDGE_A, EDGE_B, EDGE_C };

	// Check the bounds of the tri
	if( uiTri >= (u_int) m_iNumTriangles )
		return false;

	CPoint obPCopyIn  = obPIn;
	CPoint obPCopyOut = obPOut;

	UNUSED( obPCopyIn );
	UNUSED( obPCopyOut );

	// Line segment
	C2DLineSegment obLine1 = C2DLineSegment( obPIn.X(), obPIn.Z(), obPOut.X(), obPOut.Z() );

	// Closest point saver
	CPoint		ptClosestIntersectPoint = CPoint( FLT_MAX, FLT_MAX, FLT_MAX );
	HIT_EDGE	eHitEdge			= EDGE_NIL;
	float		fBestDist = FLT_MAX;


	// Find the Closest tri to the OutPoint
	for (int iRegion = 0 ; iRegion < m_iNumRegions; ++iRegion )
	{
		CPoint ptClosestPoint = obLine1.ClosestPoint( m_aRegions[iRegion].obCentre.X(), m_aRegions[iRegion].obCentre.Z() );
		ptClosestPoint = CPoint( ptClosestPoint.X(), obPIn.Y(), ptClosestPoint.Z() );

		if (IsPointInRegionBB( ptClosestPoint, iRegion ))
		{
			// Storage for the new tri
			int iNewTri = -1;

			for (int iTestTri = m_aRegions[iRegion].minTriIndex; iTestTri <= m_aRegions[iRegion].maxTriIndex; ++iTestTri )
			{
				// Test that the line intersects to the tri
				CPoint ptA = m_aVertices[m_aTriangles[iTestTri].a]; 
				CPoint ptB = m_aVertices[m_aTriangles[iTestTri].b];
				CPoint ptC = m_aVertices[m_aTriangles[iTestTri].c];

				HIT_EDGE eEdge = EDGE_NIL;
				CPoint ptIntersectPoint;
				CPoint ptIntersectPointTemp;

				// Look for an intersetion on the first edge
				if( obLine1.GetIntersection( C2DLineSegment( ptA.X(), ptA.Z(), ptB.X(), ptB.Z() ), ptIntersectPoint ) == C2DLineSegment::INTERSECTS )
				{
					ptIntersectPoint = CPoint( ptIntersectPoint.X(), obPOut.Y(), ptIntersectPoint.Y() );
					float fDist = (ptIntersectPoint - obPOut).LengthSquared();
					if( fDist < fBestDist )
					{
						eEdge = EDGE_A;
						fBestDist = fDist;
					}
				}
				
				// Try the second edge
				if( obLine1.GetIntersection( C2DLineSegment( ptB.X(), ptB.Z(), ptC.X(), ptC.Z() ), ptIntersectPointTemp ) == C2DLineSegment::INTERSECTS )
				{
					ptIntersectPointTemp = CPoint( ptIntersectPointTemp.X(), obPOut.Y(), ptIntersectPointTemp.Y() );
					float fDist = (ptIntersectPointTemp - obPOut).LengthSquared();
					// Only accept the intersection if it is closer to the dest point
					if( fDist < fBestDist )
					{
						ptIntersectPoint = ptIntersectPointTemp;
						fBestDist = fDist;
						eEdge = EDGE_B;
					}
				}

				// Only accept the intersection if it is closer to the dest point
				if( obLine1.GetIntersection( C2DLineSegment( ptC.X(), ptC.Z(), ptA.X(), ptA.Z() ), ptIntersectPointTemp ) == C2DLineSegment::INTERSECTS )
				{
					ptIntersectPointTemp = CPoint( ptIntersectPointTemp.X(), obPOut.Y(), ptIntersectPointTemp.Y() );
					float fDist = (ptIntersectPointTemp - obPOut).LengthSquared();
					if( fDist < fBestDist )
					{
						ptIntersectPoint = ptIntersectPointTemp;
						fBestDist = fDist;
						eEdge = EDGE_C;
					}
				}

				// Test whether the line intersects the edge of the tri. (Note limited to XZ)
				if(	EDGE_NIL != eEdge )
				{
					ptClosestIntersectPoint	= ptIntersectPoint;
					eHitEdge				= eEdge;
					iNewTri					= iTestTri;
				}
			}

			// Is there a new closer tri?
			if( iNewTri != -1 )
			{
				uiTri = (u_int) iNewTri;
			}
		}
	}

	// Could find a hitting edge? There must be one, calculate the edges for the initial tri
	if( eHitEdge == EDGE_NIL )
	{
		// Test that the line intersects to the tri
		CPoint ptA = m_aVertices[m_aTriangles[uiTri].a]; 
		CPoint ptB = m_aVertices[m_aTriangles[uiTri].b];
		CPoint ptC = m_aVertices[m_aTriangles[uiTri].c];

		// Rebuild the line segment
		obLine1 = C2DLineSegment( obPIn.X(), obPIn.Z(), obPOut.X(), obPOut.Z() );
		CPoint ptIntersectPointTemp;
		float fBestDist = FLT_MAX;

		// Look for an intersetion on the first edge
		if( obLine1.GetIntersection( C2DLineSegment( ptA.X(), ptA.Z(), ptB.X(), ptB.Z() ), ptClosestIntersectPoint ) == C2DLineSegment::INTERSECTS )
		{
			ptClosestIntersectPoint = CPoint( ptClosestIntersectPoint.X(), obPOut.Y(), ptClosestIntersectPoint.Y() );
			fBestDist = ptClosestIntersectPoint.LengthSquared();
			eHitEdge = EDGE_A;
		}
		
		// Try the second edge
		if( obLine1.GetIntersection( C2DLineSegment( ptB.X(), ptB.Z(), ptC.X(), ptC.Z() ), ptIntersectPointTemp ) == C2DLineSegment::INTERSECTS )
		{
			ptIntersectPointTemp = CPoint( ptIntersectPointTemp.X(), obPOut.Y(), ptIntersectPointTemp.Y() );
			float fDist = (ptIntersectPointTemp - obPOut).LengthSquared();
			// Only accept the intersection if it is closer to the dest point
			if( fDist < fBestDist )
			{
				ptClosestIntersectPoint = ptIntersectPointTemp;
				fBestDist = fDist;
				eHitEdge = EDGE_B;
			}
		}

		// Only accept the intersection if it is closer to the dest point
		if( obLine1.GetIntersection( C2DLineSegment( ptC.X(), ptC.Z(), ptA.X(), ptA.Z() ), ptIntersectPointTemp ) == C2DLineSegment::INTERSECTS )
		{
			ptIntersectPointTemp = CPoint( ptIntersectPointTemp.X(), obPOut.Y(), ptIntersectPointTemp.Y() );
			float fDist = (ptIntersectPointTemp - obPOut).LengthSquared();
			if( fDist < fBestDist )
			{
				ptClosestIntersectPoint = ptIntersectPointTemp;
				fBestDist = fDist;
				eHitEdge = EDGE_C;
			}
		}
	}

	// Get the points that built the tri
	CPoint ptA = m_aVertices[m_aTriangles[uiTri].a];
	CPoint ptB = m_aVertices[m_aTriangles[uiTri].b];
	CPoint ptC = m_aVertices[m_aTriangles[uiTri].c];

	// Testing edge order...
	CPoint ptP1 = ptA;
	CPoint ptP2 = ptB;
	CPoint ptP3 = ptC;
	
	// Based on the edge that was hit, cache points that build the edge line segment and
	// point to build a normal
	switch( eHitEdge )
	{
		// Already built on construction of obEdge
		case EDGE_A:
			break;

		case EDGE_B:
			ptP1 = ptB;
			ptP2 = ptC;
			ptP3 = ptA;
			break;

		case EDGE_C:
			ptP1 = ptC;
			ptP2 = ptA;
			ptP3 = ptB;
			break;

		default:
			return false;
	}

	if( pResult )
	{
		// Create a normal, there must be a better way to perform this. 
		CDirection dirTemp = CDirection(ptP2 - ptP1);
		CDirection dirVector = CDirection(ptP3 - ptP1);

		dirTemp.Normalise();
		dirTemp = dirTemp * dirTemp.Dot( dirVector );

		CDirection dirNormal = (dirVector - dirTemp);
		dirNormal.Normalise();

		pResult->SetXAxis( CDirection( ptP1 ) );	// Point 1
		pResult->SetYAxis( CDirection( ptP2 ) );	// Point 2 (Making an edge)
		pResult->SetZAxis( dirNormal );				// Edge Normal
		pResult->SetTranslation( ptClosestIntersectPoint ); // Intersection
	}

	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::AddKeyWordToRegion
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

void AINavData::AddKeywordToRegion( const char* keyword, const int idx )
{
	ntAssert( idx >= 0 && idx < m_iNumRegions );
	if (!m_aRegions[idx].keywords)
	{
		m_aRegions[idx].keywords = NT_NEW_CHUNK( Mem::MC_AI ) CKeywords( keyword );
		ntAssert( m_aRegions[idx].keywords );
	}
	else
	{
		m_aRegions[idx].keywords->AddKeyword( keyword );
	}
}

//--------------------------------------------------
//!
//!	AINavData::GetRandomPosInNode
//!
//--------------------------------------------------

CKeywords* AINavData::GetNodeKeywords( int nodeNum ) const
{
	for (int i = 0; i < m_iNumRegions; ++i)
	{
		if (nodeNum >= m_aRegions[i].minTriIndex && nodeNum <= m_aRegions[i].maxTriIndex)
		{
			return m_aRegions[i].keywords;
		}
	}

	return NULL;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIIntermediateNavData::AddRegion
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

AIIntermediateNavData::~AIIntermediateNavData()
{
	m_obRegionList.clear();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIIntermediateNavData::AddRegion
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIIntermediateNavData::AddRegion( CAINavNodeRegion*	pobRegion )
{
	m_obRegionList.push_back( pobRegion );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIIntermediateNavData::Preprocess
//!                                                                                         
//!	Takes the input navigation data (a list of regions) and prepares them for adding to the
//! more compact, optimised for runtime data structures of AINavData. This includes an
//! individual preprocess step on each region for triangulation, and determination of
//! connectivity between regions
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIIntermediateNavData::Preprocess()
{
	AINavNodeRegionList::iterator itr = m_obRegionList.begin();
	for (; itr != m_obRegionList.end(); ++itr)
	{
		(*itr)->Preprocess();
	}

	// build links between regions
	itr = m_obRegionList.begin();
	for (; itr != m_obRegionList.end(); ++itr)
	{
		AINavNodeRegionList::iterator otherItr = m_obRegionList.begin();
		for (; otherItr != m_obRegionList.end(); ++otherItr)
		{
			if (itr == otherItr)
			{
				continue;
			}

			(*itr)->MakeConnections( (*otherItr) );
		}
	}



	// AINavData requires the following data for construction
	//	- total num verts
	//	- total num tris
	//	- total num connections
	//
	//	Per region data:
	//	- num verts
	//	- num tris
	//	- connections to other regions
	//	- worldmin and worldmax (AABB)
	m_iNumVerts = 0;
	m_iNumTris = 0;
	m_iNumConnections = 0;

	itr = m_obRegionList.begin();
	for (; itr != m_obRegionList.end(); ++itr)
	{
		// this region's verts and tris will be appended to the existing set,
		// so we set offset values in the region allowing the indices to be rebased
		// before adding to the main pools
		(*itr)->SetBaseValues( m_iNumVerts, m_iNumTris );
		(*itr)->ReBaseInternal();
        
		m_iNumTris	+= (*itr)->GetNumTriangles();
		m_iNumVerts	+= (*itr)->GetNumVertices();
		m_iNumConnections += (*itr)->GetNumConnections();
	}

	itr = m_obRegionList.begin();
	for (; itr != m_obRegionList.end(); ++itr)
	{
		// rebase all external links
		(*itr)->ReBaseExternal();        
	}

	m_bPreprocessed = true;
}

