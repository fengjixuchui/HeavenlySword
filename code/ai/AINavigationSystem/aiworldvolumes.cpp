//! -------------------------------------------
//! aiworldvolumes.cpp
//!
//! Volumes used to describe the AI's world
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aiworldvolumes.h"
#include "ainavigsystemman.h"
#include "objectdatabase/dataobject.h"
#include "core/visualdebugger.h"

// Macros

#define CROSS2D(P,Q) ( (P.X()*Q.Z()) - (P.Z()*Q.X()) )

// XML Interface

START_CHUNKED_INTERFACE(SVaultingParams, Mem::MC_AI)
	PUBLISH_VAR_AS				(fVaultingDistance			, VaultingDistance)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(CAIWorldVolume, Mem::MC_AI)
	PUBLISH_CONTAINER_AS		(m_vectorpVertex	, Vertices)
	PUBLISH_VAR_AS				(m_fHeight			, Height)
	PUBLISH_VAR_AS				(m_fRadius			, Radius)
	PUBLISH_VAR_AS				(m_obCentre			, Position)
	PUBLISH_VAR_AS				(m_bRelaxedIsect	, RelaxChecks)
	PUBLISH_VAR_AS				(m_bContRegion		, Containment)
	PUBLISH_VAR_AS				(m_bTransparent		, Transparent)
	PUBLISH_VAR_AS				(m_bGoAroundVolume	, GoAroundVolume)
	PUBLISH_VAR_AS				(m_fGoAroundMinDst	, GoAroundMinDistance)
	PUBLISH_VAR_AS				(m_fGoAroundNodeRadius	, GoAroundNodeRadius)
	PUBLISH_VAR_AS				(m_fGoAroundDistFactor	, GoAroundDistFactor)
	PUBLISH_VAR_AS				(m_bVaultingVolume	, VaultingVolume)
	PUBLISH_VAR_AS				(m_bDebugMe			, DebugMe)
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_iMappedAreaInfo, 0, SectorBits )
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE

//!--------------------------------------------
//! Destructor
//!--------------------------------------------
CAIWorldVolume::~CAIWorldVolume()
{
	// Remove this volume from world manager
	CAINavigationSystemMan::Get().GetAIWorldManager()->RemoveVolume(this);

	// Deallocating dynamic data (Segments)
	m_vpSegments.clear();
	
	// Deallocating Dynamic GoAroundMatrix
	NT_DELETE_CHUNK(Mem::MC_AI, m_iPathFindMatrix);

	// Deallocating dynamic data (Go Around Nodes)
	m_vpGoAroundNodes.clear();

}

////!--------------------------------------------
////! GetPerpendicular
////!--------------------------------------------
//CDirection CAIWorldVolume::GetPerpendicular(const CDirection& obDir)
//{
//	// The calculated perpendicular is:
//	//			Perpendicular
//	//           |
//	//           |
//	//  A|---------------->B
//
//	CDirection obResult( CONSTRUCT_CLEAR );
//	obResult.Z() = -obDir.X();
//	obResult.X() = obDir.Z();
//	obResult.Y() = obDir.Y();
//
//	return obResult;
//}
//!--------------------------------------------
//! PostConstruct
//!--------------------------------------------

void CAIWorldVolume::PostConstruct ( void )
{
	// Sanity check
	ntAssert_p(m_fHeight>0.1,("CAIWorldVolume -> Bad Height provided in XML\n"));

	m_eVolumeType = m_fRadius<0.0f ? VTYPE_POLYHEDRON : VTYPE_CYLINDER;

	// Get the Name of the Volume
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	if(pDO)
	{
		strcpy(m_tmp_Name,ntStr::GetString(pDO->GetName()));
	}

	// Check the type of volume (CYLINDER or POLYHEDRON)
	if (m_eVolumeType == VTYPE_POLYHEDRON)
	{
		// Reset its Radius
		m_fRadiusPOLY = 0.0f;
		// Cache its number of vertices
		m_uiNumberOfVertex = m_vectorpVertex.size();

		// It is a POLYHEDRON -----------------------------------
		if (m_uiNumberOfVertex<2) 
		{
				ntAssert_p(0,("CAIWorldVolume::PostConstruct -> XML AI volume with less than 2 vertex!!!"));
				return;
		}
		if (m_uiNumberOfVertex==2)
		{
			// Segment
			return;
		}

		// The Volume has 3 or more segments

		m_vpSegments.reserve(m_uiNumberOfVertex);

		m_fRadius = 0.0f;
		m_obCentre = m_vectorpVertex[0];
		CDirection Normal(CONSTRUCT_CLEAR);
		CDirection Face(CONSTRUCT_CLEAR);

		for (unsigned int i = 1; i< m_uiNumberOfVertex; i++ )
		{
			// Update Face
			Face = CDirection (m_vectorpVertex[i] - m_vectorpVertex[i-1]);
			// Update Centre
			m_obCentre += m_vectorpVertex[i];
			// Update Normal
			Normal	= GetPerpendicular(Face);
			Normal.Normalise();
			if (m_bContRegion) Normal = -Normal;
			
			// Segment Generation

			SAIVolSegment obSeg(	m_vectorpVertex[i-1],
									m_vectorpVertex[i],
									Normal, 
									Face
									);
			m_vpSegments.push_back(obSeg);
		}

		// Close the perimeter
		Face = CDirection (m_vectorpVertex[0] - m_vectorpVertex[m_uiNumberOfVertex-1]);
		Normal	= GetPerpendicular(Face);
		Normal.Normalise();
		if (m_bContRegion) Normal = -Normal;

		// Segment Generation
		SAIVolSegment obSeg(	m_vectorpVertex[m_uiNumberOfVertex-1],
								m_vectorpVertex[0],
								Normal, 
								Face
								);
		m_vpSegments.push_back(obSeg);
	
		// Calculate Centre
		m_obCentre = m_obCentre / (float)m_uiNumberOfVertex;

		// Calculate Radius from the centre
		m_fRadiusPOLY = 0.0f;
		
		for (unsigned int i = 0; i< m_uiNumberOfVertex; i++ )
		{
			CDirection Line = CDirection(m_obCentre - m_vectorpVertex[i]);
			float fDist = Line.Length();
			if (fDist > m_fRadiusPOLY)
				m_fRadiusPOLY = fDist;
		}

		// ---------------------------
		// Calculate the virtual nodes
		// ---------------------------
		if (m_bGoAroundVolume)
		{
			//user_warn_p( m_vectorpVertex.size() <= PF_MATRIX_MAX_NODES ,("CAIWorldVolume -> The AI GoAroundVolume [%s] has more vertices that the maximum allowed (%d)\nIt will be ignored.", m_tmp_Name,PF_MATRIX_MAX_NODES));
			
			m_vpGoAroundNodes.reserve(m_vpSegments.size());

			for (unsigned int i = 0; i< m_vpSegments.size(); i++ )
			{
				CDirection CornerDir(CONSTRUCT_CLEAR);
				if (i>0)
				{
					CornerDir = m_vpSegments[i].Normal + m_vpSegments[i-1].Normal;
				}
				else
				{
					CornerDir = m_vpSegments[i].Normal + m_vpSegments[m_vpSegments.size()-1].Normal;
				}
				CornerDir.Normalise();
				// Go Around Node Generation
				SGoAroundNodes obGANode( m_vpSegments[i].P0, CornerDir );

				m_vpGoAroundNodes.push_back(obGANode);
			}
			if (m_fGoAroundNodeRadius<0.5f)
			{
				user_warn_p( 0 ,("CAIWorldVolume -> The AI GoAroundVolume [%s] very small node radius (%.3f).\nThis value has been changed to 1.0f", m_tmp_Name,m_fGoAroundNodeRadius));
				m_fGoAroundNodeRadius = 1.0f;
			}

			// Generate the Path Find table
			GeneratePathFindTable();

			// Add AI volume to the manager
			CAINavigationSystemMan::Get().GetAIWorldManager()->AddGoAroundVolume(this);
			ntPrintf(Debug::DCU_AI, "Adding GOAROUND Volume: %s [%s] - Vtx: %d - Node Radius: %.3f - Corner Dist: %.3f - Dbg: %s\n",m_tmp_Name, 
															(m_eVolumeType == VTYPE_POLYHEDRON)? "POLYHEDRON" : "CYLINDER", 
															m_uiNumberOfVertex,
															m_fGoAroundNodeRadius,
															m_fGoAroundMinDst,
															m_bDebugMe == true ? "DBG!" : "");
		}
	}
	else
	{
		// It is a CYLINDER
		m_fRadiusSQR = sqr(m_fRadius);
	}

	// Add AI volume to the manager
	
	if (IsVaultingVolume())
		CAINavigationSystemMan::Get().GetAIWorldManager()->AddVaultingVolume(this);
	else if (m_bContRegion)
		CAINavigationSystemMan::Get().GetAIWorldManager()->AddVolume(this);
	else
		CAINavigationSystemMan::Get().GetAIWorldManager()->AddVolume(this);

	// Delete the XML Vertices vector

	m_vectorpVertex.clear();

	// Check SectorBits

	if (HasInvalidSectorBits())
	{
		ntPrintf("(ERROR!) Loading volume: %s with BAD SectoBits (0)\n",m_tmp_Name );
//		user_warn_msg( ("(ERROR!) Loading volume: %s with BAD SectoBits (0)\n",m_tmp_Name ) );
	}

#ifndef _RELEASE
	// Get the Name of the Volume
	ntPrintf(Debug::DCU_AI, "Adding Volume: %s of type: %s with %d Vertices %s\n",m_tmp_Name, 
																(m_eVolumeType == VTYPE_POLYHEDRON)? "POLYHEDRON" : "CYLINDER", 
																m_uiNumberOfVertex,
																m_bDebugMe == true ? "DBG!" : "");
#endif

}
//!--------------------------------------------
//! GeneratePathFindTable
//!--------------------------------------------
void CAIWorldVolume::GeneratePathFindTable ( void )
{
	// generate the PF matrix structure
	unsigned int uiRowSize = m_vpGoAroundNodes.size();

	m_iPathFindMatrix = NT_NEW_CHUNK( Mem::MC_AI ) int[uiRowSize*uiRowSize];

	// Generate the PF Matrix
	for (unsigned int i = 0; i < uiRowSize; i++)
	{
		for (unsigned int j = 0; j < uiRowSize; j++)
		{
			if ( i == j )
			{
				m_iPathFindMatrix[i*uiRowSize+j] = j;
			}
			else if (!PreventsLineofSight(m_vpGoAroundNodes[i].FirstNodePos,m_vpGoAroundNodes[j].FirstNodePos))
			{
				 m_iPathFindMatrix[i*uiRowSize+j] = j;
			}
			else
			{
				m_iPathFindMatrix[i*uiRowSize+j] = GetFirstVertexFromShortestPath(i,j);
			}
			ntPrintf(Debug::DCU_AI, "AIVolume:[%s] - PathFind[%d][%d] = %d\n",m_tmp_Name,i,j,m_iPathFindMatrix[i*uiRowSize+j]);
		}
	}
	return;
}

//!--------------------------------------------
//! GetFirstVertexFromShortestPath
//!--------------------------------------------
int CAIWorldVolume::GetFirstVertexFromShortestPath ( unsigned int uiFrom, unsigned int uiTo )
{
	unsigned int index = uiFrom;
	unsigned int size = m_vectorpVertex.size();
	float		 fDist= 0.0f;
	float		 fTotalDistCW = 0.0f;
	float		 fTotalDistACW = 0.0f;

	if (uiFrom == uiTo)
		return uiFrom;

	// Clock-wise
	while ( index != uiTo)
	{
		if ( index < size-1 )
		{
			CDirection Dir = CDirection(m_vectorpVertex[index] - m_vectorpVertex[index+1]);
			fDist = Dir.Length();
			fTotalDistCW += fDist;
			index++;
		}
		else
		{
			CDirection Dir = CDirection(m_vectorpVertex[index] - m_vectorpVertex[0]);
			fDist = Dir.Length();
			fTotalDistCW += fDist;
			index = 0;
		}
	}

	// Reset variables
	fDist = 0.0f;
	index = uiFrom;

	// Anti-Clock-wise (this can be done in one loop, but no time for finnesse...)
	while ( index != uiTo)
	{
		if ( index > 0 )
		{
			CDirection Dir = CDirection(m_vectorpVertex[index] - m_vectorpVertex[index-1]);
			fDist = Dir.Length();
			fTotalDistACW += fDist;
			index--;
		}
		else
		{
			CDirection Dir = CDirection(m_vectorpVertex[index] - m_vectorpVertex[size - 1]);
			fDist = Dir.Length();
			fTotalDistACW += fDist;
			index = size - 1;
		}
	}

	// So...

	int ret = 0;
	if (fTotalDistCW < fTotalDistACW)
		ret = (uiFrom == size-1) ? 0 : (uiFrom+1);
	else
		ret = (uiFrom == 0) ? (size -1) : (uiFrom-1);
	return ret;
}

//!--------------------------------------------
//! GetPathAroundVolume
//!--------------------------------------------
bool CAIWorldVolume::GetPathAroundVolume ( const CPoint& obFrom, const CPoint&  obTo, CAINavigPath* pPath)
{
	// Is this a Go-Around-Volume?
	if (!m_bGoAroundVolume)
		return false;
	// is the supplied path container valid?
	if (!pPath)
		return false;
	pPath->clear();
	
	// =================================
	// Find the closest nodes to FROM,TO
	// =================================

	float fDist = 0.0f;
	float fMinDist = FLT_MAX; // A biiiiiggggg thing
	unsigned int uiMinIndexFrom = 0;
	unsigned int uiMinIndexTo = 0;
	bool bMinFoundFrom = false;
	bool bMinFoundTo = false;

	// Find the closest node to FROM
	for (unsigned int i = 0; i < m_vpGoAroundNodes.size(); i++)
	{
		CDirection Dir(m_vpGoAroundNodes[i].FirstNodePos - obFrom);
		fDist = Dir.LengthSquared();
		if (fMinDist>fDist)
		{
			if (!PreventsLineofSight(m_vpGoAroundNodes[i].FirstNodePos, obFrom))
			{
				fMinDist = fDist;
				uiMinIndexFrom = i;
				bMinFoundFrom = true;
			}
		}
	}

	fMinDist = FLT_MAX; // A biiiiiggggg thing

	// Find the closest node to TO
	for (unsigned int i = 0; i < m_vpGoAroundNodes.size(); i++)
	{
		CDirection Dir(m_vpGoAroundNodes[i].FirstNodePos - obTo);
		fDist = Dir.LengthSquared();
		if (fMinDist>fDist)
		{
			if (!PreventsLineofSight(m_vpGoAroundNodes[i].FirstNodePos, obTo))
			{
				fMinDist = fDist;
				uiMinIndexTo = i;
				bMinFoundTo = true;
			}
		}
	}

	// =================================
	// Fill up the Path container
	// =================================

#ifndef _RELEASE
	int iIterationWatchdog = 0;
#endif

	if (bMinFoundFrom && bMinFoundTo)
	{
		unsigned int uiIndex = uiMinIndexFrom;
		// Set From Node
		CAINavigNode* pNode = NT_NEW_CHUNK( Mem::MC_AI ) CAINavigNode();
		float fRand = grandf(m_fGoAroundMinDst)*m_fGoAroundDistFactor;
		CPoint p3d = CPoint(m_vpGoAroundNodes[uiIndex].FirstNodePos + fRand*m_vpGoAroundNodes[uiIndex].CornerDir);
		pNode->SetGoAroundNodeParams(p3d, m_fGoAroundNodeRadius);
		pPath->push_back(pNode);

		unsigned int uiRowSize = m_vpGoAroundNodes.size();

		while ( uiIndex != uiMinIndexTo)
		{
			uiIndex = m_iPathFindMatrix[uiIndex*uiRowSize+uiMinIndexTo];
			CAINavigNode* pNode = NT_NEW_CHUNK( Mem::MC_AI ) CAINavigNode();
			fRand = grandf(m_fGoAroundMinDst);
			p3d = CPoint(m_vpGoAroundNodes[uiIndex].FirstNodePos + fRand*m_vpGoAroundNodes[uiIndex].CornerDir);
			pNode->SetGoAroundNodeParams(p3d, m_fGoAroundNodeRadius);
			pPath->push_back(pNode);

			ntError_p(iIterationWatchdog++ < 100, ("Too many iterations building go around nodes")); 
		}
		pPath->SetDeallocateManually(true);
		pPath->PathReady();
		pPath->PointToFirstNode();
	}
														
	return true;
}

// =============================================================================================
//										VOLUME GEOMETRIC STUFF
// =============================================================================================

//!--------------------------------------------
//! IsInProcessingDistance
//!--------------------------------------------
bool CAIWorldVolume::IsInProcessingDistance ( const CPoint& obFrom, const CPoint& obTo ) const
{
	// Calculate Ckeck-line length
	CDirection LineFromTo = CDirection(obFrom-obTo);
	float fCheckRadius = LineFromTo.Length();
	
	// Calculate Distance from obFrom to Volume Centre
	CDirection LineFromCentre = CDirection(obFrom-m_obCentre);
	float fFromCentreDist = LineFromCentre.Length();

	float fVolumeRadius = (m_eVolumeType == VTYPE_POLYHEDRON) ? m_fRadiusPOLY: m_fRadius;

	if ( fFromCentreDist > (fCheckRadius+fVolumeRadius) )
		return false;
	else
		return true;
}

//!--------------------------------------------
//! IsInProcessingDistance
//!--------------------------------------------
bool CAIWorldVolume::IsInProcessingDistance ( const CPoint& obFrom, float fCheckRadius ) const
{
	// Calculate Distance from obFrom to Volume Centre
	CDirection LineFromCentre = CDirection(obFrom-m_obCentre);
	float fFromCentreDist = LineFromCentre.Length();

	float fVolumeRadius = (m_eVolumeType == VTYPE_POLYHEDRON) ? m_fRadiusPOLY: m_fRadius;

	if ( fFromCentreDist > (fCheckRadius+fVolumeRadius) )
		return false;
	else
		return true;
}

//!--------------------------------------------
//! IsInside
//!--------------------------------------------
bool CAIWorldVolume::IsInside ( const CPoint& p3d )	const
{
	if (m_eVolumeType == VTYPE_POLYHEDRON)
	{
		// Check first the Y axis because we don't allow different heights to be edited in MrEd.
		if ( (p3d.Y() < m_vpSegments[0].P0.Y()) || ( p3d.Y() > (m_vpSegments[0].P0.Y()+m_fHeight ) ) ) return false;

		// Check X, Z
		for ( unsigned int i = 0; i<m_vpSegments.size(); ++i )
		{
			// Check the dot product sign
			CDirection	Line2Point	(p3d - m_vpSegments[i].P0);
			
			if ( Line2Point.Dot(m_vpSegments[i].Normal)>0 )	return false;

			// As far as this vertex is concerned, the point is inside. Keep on looping.
		}
		return true;
	}
	else
	{
		// Check first the Y axis
		if ( (p3d.Y() < m_obCentre.Y()) || ( p3d.Y() > (m_obCentre.Y()+m_fHeight ) ) ) return false;

		// Equalise the Y component
		CPoint obPos(p3d);	// p3d is a const object ...

		obPos.Y() = m_obCentre.Y();

		CDirection	Line2Point	(m_obCentre - obPos);
		if ( Line2Point.LengthSquared()< m_fRadiusSQR ) 
		{
			return true;
		}
		else
		{
			return false;
		}

	}
}

//!--------------------------------------------
//! IntersectsAIVolume
//!--------------------------------------------
bool CAIWorldVolume::IntersectsAIVolume	( const CPoint& obFrom, const CPoint& obTo, CPoint* pPoint, CDirection*	pNormal ) const
{
	if ( m_bContRegion )
	{
		return (IntersectsAIContainmentVolume(obFrom,obTo,pPoint,pNormal));
	}
	else
	{
		return (IntersectsAIExclusionVolume(obFrom,obTo,pPoint,pNormal));
	}
}

//!--------------------------------------------
//! PreventsLineofSight
//!--------------------------------------------
bool CAIWorldVolume::PreventsLineofSight( const CPoint& obFrom, const CPoint& obTo ) const
{
	CPoint IPoint = CPoint(CONSTRUCT_CLEAR);

	if (m_eVolumeType == VTYPE_POLYHEDRON)
	{ 
		// Check the Y component first
		if ( (obFrom.Y() < m_vpSegments[0].P0.Y()) || ( obFrom.Y() > m_vpSegments[0].P0.Y()+m_fHeight ) ) return false;

#ifndef _GOLD_MASTER				
		// Check each segment
		if (m_bDebugMe == true)
		{
			static int a = 0;
			a++;
			if (a>1000) {
				a = 0;
				ntPrintf("Debug: %s",m_tmp_Name);
			}
		}
#endif

		for ( unsigned int i = 0; i<m_vpSegments.size(); ++i )
		{
			// Is there an Intersection with this segment?
			if ( IntersectsSegments(obFrom,obTo,&m_vpSegments[i],&IPoint) )
			{
				// DebugRender
#ifndef _GOLD_MASTER				
				static const CPoint DBG_VLINE_PT(0,0.2f,0);
				if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
				{
					IPoint.Y()	= obTo.Y();

					g_VisualDebug->RenderPoint(IPoint,10.0f,DC_RED);
					g_VisualDebug->RenderLine(IPoint, CPoint(IPoint + m_vpSegments[i].Normal) , DC_GREEN);
					g_VisualDebug->RenderLine(IPoint-DBG_VLINE_PT,IPoint+DBG_VLINE_PT, DC_RED);
				}
#endif
				return true;
			} 
		}
	}
	else
	{
		// CYLINDER
		if ( (obFrom.Y() < m_obCentre.Y()) || ( obFrom.Y() > m_obCentre.Y()+m_fHeight ) ) return false;

		if (!IntersectsCircle( obFrom, obTo, &IPoint )) 
		{
			return false;
		}


		//bool InsideTo	= IsInside(obTo);
		//bool InsideFrom = IsInside(obFrom);

		//if (InsideTo && InsideFrom) return false;
		//if (InsideTo ^ InsideFrom) return true;

		// Both points are outside the CYLINDER

		//CDirection Line(m_obCentre - obFrom);
		//CDirection Perp = GetPerpendicular(Line);
		//Perp.Normalise();

		//CPoint pt3D_R = m_obCentre - m_fRadius*Perp;
		//CPoint pt3D_L = m_obCentre + m_fRadius*Perp;

		//SAIVolSegment obSegDiameter(pt3D_R,pt3D_L,CDirection(CONSTRUCT_CLEAR),CDirection(pt3D_R-pt3D_L));
		//
		//CPoint pIsectPoint; 

		//if (!IntersectsSegments ( obFrom, obTo, &obSegDiameter, pIsectPoint )) 
		//{
		//	return false;
		//}
	
		// Debug render
#ifndef _GOLD_MASTER
		if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
		{
			CDirection Normal = CDirection(CONSTRUCT_CLEAR);

			CPoint Centre(m_obCentre.X(),obTo.Y(),m_obCentre.Z());
			Normal = CDirection(Centre - obFrom);
			Normal.Normalise();

			// Draw base and top

			CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
			ArcMatrix.SetTranslation(m_obCentre);
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_GREEN);
			ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight/2,0));
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_GREEN);
			ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight,0));
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_GREEN);

			// Draw the Centre

			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obCentre, 0.1f, DC_GREEN,0);

			// Draw the Normal
			g_VisualDebug->RenderLine(obFrom, CPoint(obFrom + Normal) , DC_BLUE);
		}
#endif
		return true;
	}
	return false;

}

//!--------------------------------------------
//! IntersectsAIContainmentVolume
//!--------------------------------------------
bool CAIWorldVolume::IntersectsAIContainmentVolume ( const CPoint& obFrom, const CPoint& obTo, CPoint* pPoint, CDirection* pNormal ) const
{
	// Check the Y component first
	if ( (obFrom.Y() < m_obCentre.Y()) || ( obFrom.Y() > m_obCentre.Y()+m_fHeight ) ) return false;

	CDirection		Normal				= CDirection(CONSTRUCT_CLEAR);
	CPoint			IPoint				= CPoint(CONSTRUCT_CLEAR);

	if (m_eVolumeType == VTYPE_POLYHEDRON)
	{ 
		// Check each segment
		
		for ( unsigned int i = 0; i<m_vpSegments.size(); ++i )
		{
			// Is there Intersection with this segment?
			if ( IntersectsSegments(obFrom,obTo,&m_vpSegments[i],&IPoint) )
			{
				// Only one intersection inside a convex volume
				
				*pNormal	= m_vpSegments[i].Normal;
				*pPoint= IPoint;
				pPoint->Y()	= obTo.Y();

				// DebugRender
#ifndef _GOLD_MASTER
				static const CPoint DBG_VLINE_PT(0,0.2f,0);
				if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
				{
					g_VisualDebug->RenderPoint(*pPoint,10.0f,DC_RED);
					g_VisualDebug->RenderLine(*pPoint, CPoint(*pPoint + *pNormal) , DC_BLUE);
					g_VisualDebug->RenderLine(*pPoint-DBG_VLINE_PT,*pPoint+DBG_VLINE_PT, DC_RED);
				}
#endif
				return true;
			} 
		}
	}
	else
	{
		// CYLINDER
		if (IsInside(obTo) && IsInside(obFrom) ) return false;

		CPoint Centre(m_obCentre.X(),obTo.Y(),m_obCentre.Z());
		*pNormal = CDirection(Centre - obFrom);
		pNormal->Normalise();

#ifndef _GOLD_MASTER
		if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
		{
			// Draw base and top

			CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
			ArcMatrix.SetTranslation(m_obCentre);
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_YELLOW);
			ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight/2,0));
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_YELLOW);
			ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight,0));
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_YELLOW);

			// Draw the Centre

			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obCentre, 0.1f,DC_YELLOW,0);

			// Draw the Normal
			g_VisualDebug->RenderLine(obFrom, CPoint(obFrom + *pNormal) , DC_BLUE);
		}
#endif

		return true;
	}
	return false;
}

//!--------------------------------------------
//! IntersectsAIVolume
//!--------------------------------------------
bool CAIWorldVolume::IntersectsAIExclusionVolume ( const CPoint& obFrom, const CPoint& obTo, CPoint* pPoint, CDirection* pNormal ) const
{
	static const CPoint DBG_VLINE_PT(0,0.2f,0);

	if (m_obCentre.Y()<0.0f)
		pNormal=pNormal;

	// Check the Y component first
	if ( (obFrom.Y() < m_obCentre.Y()) || ( obFrom.Y() > m_obCentre.Y()+m_fHeight ) ) return false;

	CDirection		Normal				= CDirection(CONSTRUCT_CLEAR);
	CPoint			IPoint				= CPoint(CONSTRUCT_CLEAR);

	if (m_eVolumeType == VTYPE_POLYHEDRON)
	{ 
		// Check each segment

		unsigned int	uiNumIntersections	= 0;
		float			fMinDist			= 99999.0f;	// !!! - (Dario) very big
		
		for ( unsigned int i = 0; i<m_vpSegments.size(); ++i )
		{
			// Is there Intersection with this segment?
			if ( IntersectsSegments(obFrom,obTo,&m_vpSegments[i],&IPoint) )
			{
				// Update counter
				uiNumIntersections++;
				
				// Is the right normal?
				CDirection LineFromTo = CDirection(obTo - obFrom);
				if (LineFromTo.Dot(m_vpSegments[i].Normal) > 0.0f)
				{
					// This is not the right segment
					continue;
				}

				// Calculate distance to raycast source
				CDirection Dir = CDirection(IPoint-obFrom);
				float fDist = Dir.LengthSquared();

				if (fDist < fMinDist ) 
				{
					fMinDist	=  fDist;
					*pNormal	= m_vpSegments[i].Normal;
					*pPoint		= IPoint;
				}

				if (uiNumIntersections==2)
				{
					// A segment will intersect a convex area in a max. of 2 points
					break;
				}
				
				// -------- If only contiguous segments are to be checked ---------
				if (m_bRelaxedIsect)
				{
					// Check previous segment
					unsigned int uiPrevIndex = (i == 0) ? m_vpSegments.size()-1 : i-1; 
					if ( !IntersectsSegments(obFrom,obTo,&m_vpSegments[uiPrevIndex],&IPoint) )
					{
						unsigned int uiNextIndex = (i == (m_vpSegments.size()-1) ) ? 0 : i+1; 
						if (!IntersectsSegments(obFrom,obTo,&m_vpSegments[uiNextIndex],&IPoint))
						{
							// No second intersection found
							break;
						}
					}
					uiNumIntersections++;
					// The second intersection has been found
					CDirection Dir = CDirection(IPoint-obFrom);
					float fDist = Dir.LengthSquared();

					if (fDist < fMinDist ) 
					{
						fMinDist	=  fDist;
						*pNormal	= m_vpSegments[i].Normal;
						*pPoint		= IPoint;
					}
					break;
				}
				// ---------------------------------------------------------------
			} 
		}

		if (uiNumIntersections==0) 
		{
			return false;
		}
		else
		{
			IPoint = CPoint(pPoint->X(),obTo.Y(),pPoint->Z());
			*pPoint= IPoint;

			// DebugRender
#ifndef _GOLD_MASTER
			if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
			{
				g_VisualDebug->RenderPoint(IPoint,10.0f,DC_RED);
				g_VisualDebug->RenderLine(IPoint, CPoint(IPoint + *pNormal) , DC_BLUE);
				g_VisualDebug->RenderLine(IPoint-DBG_VLINE_PT,IPoint+DBG_VLINE_PT, DC_RED);
			}
#endif
			return true;
		}
	}
	else
	{
		// CYLINDER

		//if (!IsInside(obTo) && !IsInside(obFrom) ) 
		//	return false;

		if (!IntersectsCircle(obFrom,obTo,&IPoint))
			return false;

		CPoint Centre(m_obCentre.X(),obTo.Y(),m_obCentre.Z());
		*pNormal = CDirection(obFrom - Centre);
		pNormal->Normalise();

		// Draw base and top
#ifndef _GOLD_MASTER
		if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
		{
			CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
			ArcMatrix.SetTranslation(m_obCentre);
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_YELLOW);
			ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight/2,0));
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_YELLOW);
			ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight,0));
			g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  DC_YELLOW);

			// Draw the Centre

			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obCentre, 0.1f,DC_YELLOW,0);

			// Draw the Normal
		//	g_VisualDebug->RenderLine(obFrom, CPoint(obFrom + *pNormal) , DC_BLUE);
			g_VisualDebug->RenderPoint(IPoint,10.0f,DC_RED);
			g_VisualDebug->RenderLine(IPoint, CPoint(IPoint + *pNormal) , DC_BLUE);
			g_VisualDebug->RenderLine(IPoint-DBG_VLINE_PT,IPoint+DBG_VLINE_PT, DC_RED);
		}
#endif

		return true;
	}
}

//!--------------------------------------------
//! IntersectsCircle
//!--------------------------------------------
bool CAIWorldVolume::IntersectsCircle ( const CPoint& obP, const CPoint& obQ, CPoint* pIsectPoint ) const
{

	// Usign the same Y component for fast cheking

	CPoint EyeCentre(m_obCentre.X(),obP.Y(),m_obCentre.Z());
	CPoint obEyeQ(obQ.X(),obP.Y(),obQ.Z());

	CDirection LinePQ		= CDirection(obEyeQ - obP);
	CDirection LinePC		= CDirection(EyeCentre - obP);

	// Check if the origin of the segment is inside the cylinder
	bool bIsPInside = ((EyeCentre - obP).LengthSquared() < m_fRadiusSQR);

	if ( bIsPInside )
	{
		LinePC = -LinePC;
	}

	float fLinePQSQR	= LinePQ.LengthSquared();
	float fLinePCSQR	= LinePC.LengthSquared();
	float fPQDotPC		= LinePQ.Dot(LinePC);

	if ( fLinePQSQR < EPSILON ) 
	{ 
		//ntAssert_p(0,("CAIWorldVolume::IntersectsCircle -> This two points are extremely close.... return false")); 
		ntPrintf("CAIWorldVolume::IntersectsCircle -> This two points are extremely close.... return false");
		return false;
	} // This two points are extremely close.... return false 

	float fDelta		= (fPQDotPC*fPQDotPC) - (fLinePQSQR)*(fLinePCSQR-m_fRadiusSQR);

	if (fDelta < 0.0f) 
	{
		return false;
	}


	// The Ray intersects with the circle
	
	float t = -(-fPQDotPC + sqrt(fDelta))/fLinePQSQR;

	if (bIsPInside)
	{
		t = -t;
	}

	if (t<0.0f || t>1.0f) 
	{
		return false;
	}

	// The segment intersects with the circle

	*pIsectPoint = obP + ( t*LinePQ);

#ifndef _GOLD_MASTER
	if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
	{
		g_VisualDebug->RenderPoint(*pIsectPoint,10.0f,DC_RED);
		g_VisualDebug->RenderLine(obP, *pIsectPoint , DC_YELLOW);
	}
#endif

	return true;
}

//!--------------------------------------------
//! IntersectsCircle
//!--------------------------------------------
//bool CAIWorldVolume::IntersectsCircle ( const CPoint& obP, const CPoint& obQ, CPoint* pIsectPoint ) const
//{
//
//	//CPoint EyeCentre(m_obCentre.X(),obP.Y(),m_obCentre.Z());
//
//	//CDirection LinePQ		= CDirection(obQ - obP);
//	//CDirection LinePC		= CDirection(EyeCentre - obP);
//	//CDirection NormalPQ		= PERPEND(LinePQ);
//	//float fDist2NormalPQ	= LinePC.Dot(LinePQ);
//	//float fDist2LinePQ		= LinePC.Dot(NormalPQ);
//
//	//g_VisualDebug->RenderPoint(obQ,10.0f,DC_BLUE);
//	//g_VisualDebug->RenderLine(EyeCentre, obP , DC_YELLOW);
//	//g_VisualDebug->RenderLine(obP, obP+NormalPQ , DC_YELLOW);
//
//	//// Check if line doesn't intersect
//	//if  ( (m_fRadius < fabs(fDist2LinePQ)) || (fDist2NormalPQ < 0.0f) ) return false;
//
//	//// Calculate the linest intersection point
//
//	//CDirection DirPQ	= LinePQ;
//	//DirPQ.Normalise();	
//
//	//float fPQIsectLenght	= fDist2NormalPQ - sqrt(m_fRadiusSQR - sqr(fDist2LinePQ));
//
//	//CPoint IPoint(obP+ DirPQ*fPQIsectLenght);
//
//	//// Check if IPoint belongs to the segment
//
//	//*pIsectPoint = IPoint;
//
//	//return true;
//
//	;
//
//}

//!--------------------------------------------
//! IntersectsLine (obP0,obP1 = from, to
//!					obSeg = segment
//!					pPoint = intersection point)
//!--------------------------------------------
bool CAIWorldVolume::IntersectsSegments ( const CPoint& obPA, const CPoint& obPB, const SAIVolSegment* obSeg1, CPoint* pIsectPoint ) const
{	
	SAIVolSegment obSeg0(obPA,obPB,CDirection(CONSTRUCT_CLEAR),CDirection(obPB-obPA));

	float fSeg0LengthSQR = obSeg0.Dir.LengthSquared();
	
	CPoint E = obSeg1->P0 - obSeg0.P0;
	float fKross	= CROSS2D(obSeg0.Dir,obSeg1->Dir);
	float fKrossSQR = fKross * fKross;

	if ( fKrossSQR > ( EPSILON * fSeg0LengthSQR * obSeg1->Dir.LengthSquared() ) )
	{
		// Segments are not parallel
	
		float s = CROSS2D(E,obSeg1->Dir)/fKross;

		if (s<0 || s>1)
		{
			// The intersection of lines is not a point of the segment  = obFrom0 + s*obTo0
			return false;
		}

		float t = CROSS2D(E,obSeg0.Dir)/fKross;

		if (t<0 || t>1)
		{
			// The intersection of lines is not a point of the segment  = obFrom1 + t*obTo1
			return false;
		}

		// The intersection is a point that belongs to both segments

		*pIsectPoint = obSeg1->P0 + ( t*obSeg1->Dir);

		return true;
	}
	else
	{
		// !!! (Dario) it returns false anyway.... (to be checked)
		return false;

		// Lines of the segments are parallel (or very, very ... almost)

		fKross		= CROSS2D(E,obSeg0.Dir);
		fKrossSQR	= fKrossSQR * fKrossSQR;
		
		if ( fKrossSQR > ( EPSILON * fSeg0LengthSQR * E.LengthSquared() ) )
		{
			// Lines of the segment are different (just parallel)
			return false;
		}
		else
		{
			// Lines are the same (segments overlap)
			// Just return false !!! - (Dario) To be checked

			return false;
		}
	}

}

//!--------------------------------------------
//! DebugRender
//!--------------------------------------------
void CAIWorldVolume::DebugRender ( void )
{
#ifndef _GOLD_MASTER

	float fHalfHeight = m_fHeight/2;
	static const CPoint DBG_VLINE_PT(0,0.2f,0);

	if (!this->IsInActiveSector())
		return;

	unsigned int color =	IsTransparent() ? DC_BLUE : 
							IsVaultingVolume() ? DC_GREEN : 
							m_bGoAroundVolume ? DC_WHITE : DC_PURPLE;

	if (m_bDebugMe) 
	{	
		color = DC_GREEN;
#if defined( PLATFORM_PS3 )
		g_VisualDebug->Printf3D(m_obCentre+CPoint(0,fHalfHeight,0),DC_YELLOW,0,"%s",m_tmp_Name);
#endif
	}

	if (m_eVolumeType == VTYPE_POLYHEDRON)
	{
		for ( unsigned int i = 0; i<m_vpSegments.size(); ++i )
		{
			// Draw Vertices
			g_VisualDebug->RenderLine(m_vpSegments[i].P0, m_vpSegments[i].P1, color);
			g_VisualDebug->RenderLine(m_vpSegments[i].P0+CPoint(0,m_fHeight,0), m_vpSegments[i].P1+CPoint(0,m_fHeight,0), color);
			g_VisualDebug->RenderLine(m_vpSegments[i].P0, m_vpSegments[i].P0+CPoint(0,m_fHeight,0), color);
			g_VisualDebug->RenderLine(m_vpSegments[i].P1, m_vpSegments[i].P1+CPoint(0,m_fHeight,0), color);

			// Draw Normals
			g_VisualDebug->RenderLine(m_vpSegments[i].Centre + CPoint(0,fHalfHeight,0), 
									CPoint(m_vpSegments[i].Centre + CPoint(0,fHalfHeight,0)+m_vpSegments[i].Normal), 
									DC_YELLOW);
			
			g_VisualDebug->RenderLine(	m_vpSegments[i].Centre+CPoint(0,fHalfHeight,0)-DBG_VLINE_PT,
										m_vpSegments[i].Centre+CPoint(0,fHalfHeight,0)+DBG_VLINE_PT, 
										DC_PURPLE);
#if defined( PLATFORM_PS3 )
			g_VisualDebug->Printf3D(m_vpSegments[i].Centre+CPoint(0,fHalfHeight,0),DC_WHITE,0,"Face:%d",i);		
#endif
		}

		// Draw the Centre and Radius

		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obCentre, 0.1f,color,0);
		//g_VisualDebug->Printf3D(m_obCentre,DC_WHITE,0,"AI_POLY_VOL");
	//	CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
	//	ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight/2,0));
	//	g_VisualDebug->RenderArc(ArcMatrix, m_fRadiusPOLY , TWO_PI,  DC_YELLOW);
		
		// Go Around Volume
		if (m_bGoAroundVolume)
		{
#if defined( PLATFORM_PS3 )
			g_VisualDebug->Printf3D(m_obCentre,DC_BLUE,0,"%s",m_tmp_Name);
#endif
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obCentre , m_fRadiusPOLY ,DC_YELLOW, 4096);
			for ( unsigned int i = 0; i < m_vpGoAroundNodes.size(); ++i )
			{

				g_VisualDebug->RenderLine(	m_vpGoAroundNodes[i].Vertex+ CPoint(0,fHalfHeight,0), 
											CPoint(m_vpGoAroundNodes[i].Vertex+m_vpGoAroundNodes[i].CornerDir)+ CPoint(0,fHalfHeight,0), DC_BLUE);
				g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_vpGoAroundNodes[i].FirstNodePos, m_fGoAroundNodeRadius,DC_BLUE);
				g_VisualDebug->RenderLine(	m_vpGoAroundNodes[i].Vertex+ CPoint(0,fHalfHeight,0), 
											CPoint(m_vpGoAroundNodes[i].FirstNodePos+m_fGoAroundMinDst*m_fGoAroundDistFactor*m_vpGoAroundNodes[i].CornerDir), DC_BLUE);
				g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), CPoint(m_vpGoAroundNodes[i].FirstNodePos+m_fGoAroundMinDst*m_fGoAroundDistFactor*m_vpGoAroundNodes[i].CornerDir), m_fGoAroundNodeRadius,DC_GREEN);
#if defined( PLATFORM_PS3 )			
				g_VisualDebug->Printf3D(m_vpGoAroundNodes[i].FirstNodePos,DC_WHITE,0,"GANode:%d",i);
#endif
			}
		}
	}
	else
	{
		// Draw base and top

		CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
		ArcMatrix.SetTranslation(m_obCentre);
		g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  color);
		ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight/2,0));
		g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  color);
		ArcMatrix.SetTranslation(m_obCentre+CPoint(0,m_fHeight,0));
		g_VisualDebug->RenderArc(ArcMatrix, m_fRadius , TWO_PI,  color);

		// Draw the Centre and Radius

		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obCentre, 0.1f,color,0);
		//g_VisualDebug->Printf3D(m_obCentre,DC_WHITE,0,"AI_CYLYNDRE_VOL");

		// Draw 4 vertical lines

	}
#endif
}
