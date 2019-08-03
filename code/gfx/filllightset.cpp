//--------------------------------------------------
//!
//!	\file filllightset.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/filllightset.h"
#include "gfx/sphericalharmonics.h"
#include "objectdatabase/dataobject.h"
#include "core/timer.h"
#include "gfx/levellighting.h"

START_CHUNKED_INTERFACE( FillLightNode_TOD, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_fTOD,	TimeOfDay )
	PUBLISH_PTR_CONTAINER_AS( m_lights.m_SHLights, SHLights )
	PUBLISH_VAR_AS(	m_lights.m_reflectanceColour,		ReflectanceColour )
	PUBLISH_VAR_AS(	m_lights.m_reflectanceCubeTexture,	ReflectanceCubeMap )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( Spatial_FillLightNode_Simple, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_centerOfInfluence, CenterOfInfluence )
	PUBLISH_PTR_CONTAINER_AS( m_lights.m_SHLights, SHLights )
	PUBLISH_VAR_AS(	m_lights.m_reflectanceColour,		ReflectanceColour )
	PUBLISH_VAR_AS(	m_lights.m_reflectanceCubeTexture,	ReflectanceCubeMap )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( Spatial_FillLightNode_TOD, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_centerOfInfluence, CenterOfInfluence )
	PUBLISH_PTR_CONTAINER_AS( m_fillNodes.m_list, FillLightNodes )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 

END_STD_INTERFACE

START_CHUNKED_INTERFACE( FillLightSet, Mem::MC_GFX )

	PUBLISH_PTR_CONTAINER_AS( m_nodes, SpatialNodes )
	DECLARE_POSTCONSTRUCT_CALLBACK(	PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE





//--------------------------------------------------
//!
//!	FillLightNode::GetContribution
//! Sum our light list into a single SH environment
//!
//--------------------------------------------------
void FillLightNode::GetContribution( SHEnvironment& environment ) const
{
	// environment should be clear before we get it

	for (	SHContributorList::const_iterator it = m_SHLights.begin();
			it != m_SHLights.end(); ++it )
	{
		(*it)->AddContribution( environment );
	}
}




//--------------------------------------------------
//!
//!	Spatial_FillLightNode_TOD::PostConstruct 
//! Sort our list into an orderd list
//!
//--------------------------------------------------
void Spatial_FillLightNode_TOD::PostConstruct()
{
	m_fillNodes.SortListAscending();
}

//--------------------------------------------------
//!
//!	Spatial_FillLightNode_TOD::SanityCheck 
//! makes sure our nodes are still ordered (in case 
//! of editing)
//!
//--------------------------------------------------
void Spatial_FillLightNode_TOD::SanityCheck() const
{
#ifndef _RELEASE

	if (m_fillNodes.IsUnsorted())
		m_fillNodes.SortListAscending();

#endif
}

//--------------------------------------------------
//!
//!	Spatial_FillLightNode_TOD::GetContribution 
//! retrive anew set of SH coeffs for this node
//!
//--------------------------------------------------
void Spatial_FillLightNode_TOD::GetContribution( SHEnvironment& environment ) const
{
	if( m_fillNodes.m_list.empty() )
		return;

	// make sure we're still sorted
	SanityCheck();

	// retrieve bounding nodes
	float fTOD = LevelLighting::Get().GetTimeOfDay();
	const FillLightNode_TOD *pPrev, *pNext;

	float fLerpVal = m_fillNodes.GetBoundingNodes( fTOD, &pPrev, &pNext, 24.0f );

	// environment should be clear before we get it
	if (pPrev == pNext)
	{
		// simple
		pPrev->m_lights.GetContribution( environment );
	}
	else
	{
		// lerp
		pPrev->m_lights.GetContribution( environment );
		environment *= (1.0f - fLerpVal);

		SHEnvironment next;
		pNext->m_lights.GetContribution( next );
		next *= fLerpVal;

		environment += next;
	}
}

//--------------------------------------------------
//!
//!	Spatial_FillLightNode_TOD::GetReflectanceColour 
//! not lerped, just most significant
//!
//--------------------------------------------------
CDirection Spatial_FillLightNode_TOD::GetReflectanceColour() const
{
	// we shouldnt be called if we have no valid nodes
	if( m_fillNodes.m_list.empty() )
		return CDirection(1.0f, 1.0f, 1.0f);

	// make sure we're still sorted
	SanityCheck();

	// retrieve bounding nodes
	float fTOD = LevelLighting::Get().GetTimeOfDay();
	const FillLightNode_TOD *pPrev, *pNext;

	float fLerpVal = m_fillNodes.GetBoundingNodes( fTOD, &pPrev, &pNext, 24.0f );

	// just return the most significant light
	if (pPrev == pNext)
	{
		// simple
		return pPrev->m_lights.GetReflectanceColour();
	}
	else
	{
		// lerp
		return CDirection::Lerp(	pPrev->m_lights.GetReflectanceColour(),
									pNext->m_lights.GetReflectanceColour(),
									fLerpVal );
	}
}

//--------------------------------------------------
//!
//!	Spatial_FillLightNode_TOD::GetReflectanceTexture 
//! not lerped, just most significant
//!
//--------------------------------------------------
const char* Spatial_FillLightNode_TOD::GetReflectanceTexture() const
{
	// we shouldnt be called if we have no valid nodes
	if (m_fillNodes.m_list.empty())
		return 0;

	// make sure we're still sorted
	SanityCheck();

	// retrieve bounding nodes
	float fTOD = LevelLighting::Get().GetTimeOfDay();
	const FillLightNode_TOD *pPrev, *pNext;

	float fLerpVal = m_fillNodes.GetBoundingNodes( fTOD, &pPrev, &pNext, 24.0f );

	// just return the most significant light
	if	(
		( pPrev == pNext ) ||
		( fLerpVal < 0.5f )
		)
	{
		return ntStr::GetString(pPrev->m_lights.m_reflectanceCubeTexture);
	}
	else
	{
		return ntStr::GetString(pNext->m_lights.m_reflectanceCubeTexture);
	}
}




//--------------------------------------------------
//!
//!	FillLightSet::ctor 
//!
//--------------------------------------------------
FillLightSet::FillLightSet()
{
	m_pSpatialInterpolator = 0;

	for (u_int i = 0; i < iMAX_CACHE_ENTRIES; i++)
	{
		m_apMostSignificant[i] = 0;
		m_afMostSignificant[i] = 0.0f;
	}
};

//--------------------------------------------------
//!
//!	FillLightSet::dtor 
//!
//--------------------------------------------------
FillLightSet::~FillLightSet()
{
	NT_DELETE_CHUNK( Mem::MC_GFX, m_pSpatialInterpolator );
};

//--------------------------------------------------
//!
//!	FillLightSet::PostConstruct 
//! construct our spatial interpolator
//!
//--------------------------------------------------
void FillLightSet::PostConstruct()
{
	if (m_pSpatialInterpolator)
		NT_DELETE_CHUNK( Mem::MC_GFX, m_pSpatialInterpolator );
	
	// pull out the centers of influence of our nodes
	u_int iNumNodes = m_nodes.size();
	CPoint* pTemp = NT_NEW_ARRAY_CHUNK(Mem::MC_GFX) CPoint [iNumNodes];

	int iCount = 0;
	for (	SpatialFillLightNodeList::iterator it = m_nodes.begin();
			it != m_nodes.end(); ++it, iCount++ )
	{
		pTemp[iCount] = (*it)->m_centerOfInfluence;
	}

	// construct our interpolator
//	m_pSpatialInterpolator = NT_NEW SI_DelaunayTriangulation( pTemp, iNumNodes );
	m_pSpatialInterpolator = NT_NEW_CHUNK( Mem::MC_GFX ) SI_DistanceWeight( pTemp, iNumNodes );

	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pTemp );
}

bool FillLightSet::EditorChangeValue(CallBackParameter,CallBackParameter)
{
	PostConstruct();
	return false;
}

//--------------------------------------------------
//!
//!	FillLightSet::CalcMostSignificantNodes
//! grab copies of significant results
//!
//--------------------------------------------------
void FillLightSet::CalcMostSignificantNodes( const CPoint& position )
{
	for (u_int i = 0; i < iMAX_CACHE_ENTRIES; i++)
	{
		m_apMostSignificant[i] = 0;
		m_afMostSignificant[i] = 0.0f;
	}

	if (m_nodes.empty())
		return;

	// retrive our results, stick them in a stack and sort by importances
	ntAssert( m_pSpatialInterpolator );
	m_pSpatialInterpolator->CalcNewResult( position );
	
	const SIresultList& results = m_pSpatialInterpolator->GetResultList();
	SIsortVector tempResults( results.size() );

	int iIndex = 0;
	for ( SIresultList::const_iterator it = results.begin(); it != results.end(); ++it )
		tempResults[iIndex++] = *it;

	ntstd::sort( &tempResults[0], &tempResults[0] + tempResults.size(), comparatorMoreThan() );

	// most significant ones should now be at the front of the stack, stick in the cache
	unsigned int iCurrItem = 0;
	while ( (iCurrItem < tempResults.size()) && (iCurrItem < iMAX_CACHE_ENTRIES) )
	{
		SpatialFillLightNodeList::const_iterator nodeIt( m_nodes.begin() );
		ntstd::advance( nodeIt, tempResults[iCurrItem]->ID );

		m_apMostSignificant[iCurrItem] = *nodeIt;
		m_afMostSignificant[iCurrItem] = tempResults[iCurrItem]->fraction;

		iCurrItem++;
	}
}

//--------------------------------------------------
//!
//!	FillLightSet::GetContribution
//! sum our most significant fill light nodes into
//! a single SH environment
//!
//--------------------------------------------------
void FillLightSet::GetContribution( SHEnvironment& environment ) const
{
	for ( u_int i = 0; i < iMAX_CACHE_ENTRIES; i++ )
	{
		if ( m_apMostSignificant[i] == 0 )
			return;

		SHEnvironment local;
		m_apMostSignificant[i]->GetContribution( local );
		local *= m_afMostSignificant[i];
		environment += local;
	}
}

//--------------------------------------------------
//!
//!	FillLightSet::GetReflectanceColour
//! Get the reflectance colour from the most important
//! fill light node in the set.
//!
//--------------------------------------------------
CDirection FillLightSet::GetReflectanceColour() const
{
	CDirection result( CONSTRUCT_CLEAR );

	if ( m_apMostSignificant[0] )
		result = m_apMostSignificant[0]->GetReflectanceColour();

	return result;
}

//--------------------------------------------------
//!
//!	FillLightSet::GetReflectanceTexture
//! Get the reflectance texture from the most important
//! fill light node in the set.
//!
//--------------------------------------------------
const char* FillLightSet::GetReflectanceTexture() const
{
	if ( m_apMostSignificant[0] )
		return m_apMostSignificant[0]->GetReflectanceTexture();

	return 0;
}

