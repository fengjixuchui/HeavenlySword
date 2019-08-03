//--------------------------------------------------
//!
//!	\file postprocessing.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/spatialinterpolator.h"
#include "gfx/postprocessing.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/levellighting.h"
#include "core/visualdebugger.h"

START_CHUNKED_INTERFACE( ColourTransformMatrix, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_col1, DestinationRed )
	PUBLISH_VAR_AS(	m_col2, DestinationGreen )
	PUBLISH_VAR_AS(	m_col3, DestinationBlue )
	PUBLISH_VAR_AS(	m_row4, Transform )
			
END_STD_INTERFACE

START_CHUNKED_INTERFACE( ColourTransformNode, Mem::MC_GFX )

	PUBLISH_PTR_CONTAINER_AS( m_colourMats, ColourMatrices )
	PUBLISH_VAR_AS(	m_fTOD, TimeOfDay )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( Spatial_PostProcessingNode_Simple, Mem::MC_GFX )

	PUBLISH_PTR_CONTAINER_AS( m_colourMats, ColourMatrices )
	PUBLISH_VAR_AS(	m_centerOfInfluence, CenterOfInfluence )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( Spatial_PostProcessingNode_TOD, Mem::MC_GFX )

	PUBLISH_PTR_CONTAINER_AS( m_colourNodes.m_list, ColourNodes )
	PUBLISH_VAR_AS(	m_centerOfInfluence, CenterOfInfluence )
	DECLARE_POSTCONSTRUCT_CALLBACK(	PostConstruct )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( PostProcessingSet, Mem::MC_GFX )

	PUBLISH_PTR_CONTAINER_AS( m_nodes, SpatialNodes )
	DECLARE_POSTCONSTRUCT_CALLBACK(	PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE

//--------------------------------------------------
//!
//!	PostProcessingSet::ctor 
//!
//--------------------------------------------------
PostProcessingSet::PostProcessingSet()
{
	m_pSpatialInterpolator = 0;
};

//--------------------------------------------------
//!
//!	PostProcessingSet::dtor 
//!
//--------------------------------------------------
PostProcessingSet::~PostProcessingSet()
{
	NT_DELETE_CHUNK( Mem::MC_GFX, m_pSpatialInterpolator );
};

//--------------------------------------------------
//!
//!	PostProcessingSet::PostConstruct 
//! construct our spatial interpolator
//!
//--------------------------------------------------
void PostProcessingSet::PostConstruct()
{
	if (m_pSpatialInterpolator)
		NT_DELETE_CHUNK( Mem::MC_GFX, m_pSpatialInterpolator );
	
	// pull out the centers of influence of our nodes
	u_int iNumNodes = m_nodes.size();
	CPoint* pTemp = NT_NEW_CHUNK( Mem::MC_GFX ) CPoint [iNumNodes];

	int iCount = 0;
	for (	SpatialPostProcessingNodeList::iterator it = m_nodes.begin();
			it != m_nodes.end(); ++it, iCount++ )
	{
		pTemp[iCount] = (*it)->m_centerOfInfluence;
	}

	// construct our interpolator
//	m_pSpatialInterpolator = NT_NEW SI_DelaunayTriangulation( pTemp, iNumNodes );
	m_pSpatialInterpolator = NT_NEW_CHUNK( Mem::MC_GFX ) SI_DistanceWeight( pTemp, iNumNodes );

	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pTemp );
}

bool PostProcessingSet::EditorChangeValue(CallBackParameter,CallBackParameter)
{
	PostConstruct();
	return false;
}

//--------------------------------------------------
//!
//!	PostProcessingSet::GetMatrix
//! Calc our current colour transform based on a position
//!
//--------------------------------------------------
void PostProcessingSet::GetMatrix( const CPoint& position, CMatrix& result ) const
{
	if (m_nodes.empty())
	{
		result.SetIdentity();
		return;
	}

	ntAssert( m_pSpatialInterpolator );

	// calculate which nodes contribute to our result
	m_pSpatialInterpolator->CalcNewResult( position );

	// sum them up
	result.Clear();
	for (	SIresultList::const_iterator it = m_pSpatialInterpolator->GetResultList().begin();
			it != m_pSpatialInterpolator->GetResultList().end(); ++it )
	{
		// get this node
		SpatialPostProcessingNodeList::const_iterator nodeIt( m_nodes.begin() );
		ntstd::advance( nodeIt, (*it)->ID );

		// get this node's colour matrix
		CMatrix temp;
		(*nodeIt)->GetMatrix( temp );

		// accumulate it according to its contribution
		result[0] += temp[0] * (*it)->fraction;
		result[1] += temp[1] * (*it)->fraction;
		result[2] += temp[2] * (*it)->fraction;
		result[3] += temp[3] * (*it)->fraction;
	}
	
	// make sure we have valid W
	result[0].W() = 0.0f;
	result[1].W() = 0.0f;
	result[2].W() = 0.0f;
	result[3].W() = 1.0f;
}




//--------------------------------------------------
//!
//!	Spatial_PostProcessingNode::DebugRender 
//! Display what our final matrix is because we're selected in welder
//!
//--------------------------------------------------
void	Spatial_PostProcessingNode::DebugRender()
{
#ifndef _GOLD_MASTER
	CMatrix curr;
	GetMatrix( curr );

	// The columns
	g_VisualDebug->Printf2D( 10.0f, 200.0f, 0xffffffff, 0, "DestinationRed   - R: %.2f G: %.2f B: %.2f", curr.Row( 0 ).X(), curr.Row( 1 ).X(), curr.Row( 2 ).X() );
	g_VisualDebug->Printf2D( 10.0f, 212.0f, 0xffffffff, 0, "DestinationGreen - R: %.2f G: %.2f B: %.2f", curr.Row( 0 ).Y(), curr.Row( 1 ).Y(), curr.Row( 2 ).Y() );
	g_VisualDebug->Printf2D( 10.0f, 224.0f, 0xffffffff, 0, "DestinationBlue  - R: %.2f G: %.2f B: %.2f", curr.Row( 0 ).Z(), curr.Row( 1 ).Z(), curr.Row( 2 ).Z() );

	// Transformation
	g_VisualDebug->Printf2D( 10.0f, 236.0f, 0xffffffff, 0, "Transform        - R: %.2f G: %.2f B: %.2f", curr.Row( 3 ).X(), curr.Row( 3 ).Y(), curr.Row( 3 ).Z() );
#endif
}

//--------------------------------------------------
//!
//!	Spatial_PostProcessingNode_TOD::PostConstruct 
//! Sort our list into an orderd list
//!
//--------------------------------------------------
void Spatial_PostProcessingNode_TOD::PostConstruct()
{
	m_colourNodes.SortListAscending();
}

//--------------------------------------------------
//!
//!	Spatial_PostProcessingNode_TOD::SanityCheck 
//! makes sure our nodes are still ordered (in case 
//! of editing)
//!
//--------------------------------------------------
void Spatial_PostProcessingNode_TOD::SanityCheck()
{
#ifndef _RELEASE

	if (m_colourNodes.IsUnsorted())
		m_colourNodes.SortListAscending();

#endif
}

//--------------------------------------------------
//!
//!	Spatial_PostProcessingNode_TOD::GetMatrix 
//! Retrive a new matrix based on all our sub matrices
//!
//--------------------------------------------------
void	Spatial_PostProcessingNode_TOD::GetMatrix( CMatrix& result )
{
	// If we have an empty list return the identity matrix
	if ( m_colourNodes.m_list.empty() )
	{
		result.SetIdentity();
		return;
	}

	// make sure our list is still ordered
	SanityCheck();

	// find the nodes that bound our time of day
	float fTimeOfDay = LevelLighting::Get().GetTimeOfDay();

	const ColourTransformNode *pPrev, *pNext;
	float fLerpVal = m_colourNodes.GetBoundingNodes( fTimeOfDay, &pPrev, &pNext, 24.0f );

	// lerp matrixes to get result
	CMatrix prevMat, nextMat;
	pPrev->GetMatrix(prevMat);
	pNext->GetMatrix(nextMat);

	result = CMatrix::Lerp( prevMat, nextMat, fLerpVal );
}

//--------------------------------------------------
//!
//!	Spatial_PostProcessingNode_TOD::DebugRender 
//! Display what our final matrix is because we're selected in welder
//!
//--------------------------------------------------
void	Spatial_PostProcessingNode_TOD::DebugRender()
{
#ifndef _GOLD_MASTER
	Spatial_PostProcessingNode::DebugRender();

	// tack on the time of day render
	float fTimeOfDay = LevelLighting::Get().GetTimeOfDay();
	g_VisualDebug->Printf2D( 10.0f, 248.0f, 0xffffffff, 0, "The time is currently %02.0f%02.0f hundred hours", fTimeOfDay, fmodf( fTimeOfDay, 1.0f ) * 60.0f  );
#endif
}
