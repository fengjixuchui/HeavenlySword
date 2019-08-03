//--------------------------------------------------
//!
//!	\file exposureset.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "exposureset.h"
#include "objectdatabase/dataobject.h"
#include "levellighting.h"

START_CHUNKED_INTERFACE( ExposureSettings_TOD, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_fTOD,								TimeOfDay )
	PUBLISH_VAR_AS(	m_settings.m_fSamplingArea,			SamplingArea )
	PUBLISH_VAR_AS(	m_settings.m_fKeyValueMapping,		KeyValueMapping )
	PUBLISH_VAR_AS(	m_settings.m_fLuminanceBurnout,		LuminanceBurnout )
	PUBLISH_VAR_AS(	m_settings.m_fErrorReduction,		ErrorReduction )
							   
	PUBLISH_VAR_AS( m_settings.m_fBloomFilterMin,			BloomFilterMin )
	PUBLISH_VAR_AS( m_settings.m_fBloomFilterMax,			BloomFilterMax )
	PUBLISH_VAR_AS( m_settings.m_fBloomFilterEffectPower,	BloomFilterEffectPower )
	PUBLISH_VAR_AS( m_settings.m_fBloomGaussianLevels,		BloomGaussianLevels )

	PUBLISH_VAR_AS( m_settings.m_fKeyLuminanceMin,			KeyLumimanceMin )
	PUBLISH_VAR_AS( m_settings.m_fKeyLuminanceMax,			KeyLumimanceMax )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( Spatial_ExposureSettings_Simple, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_centerOfInfluence,				CenterOfInfluence )
	PUBLISH_VAR_AS(	m_settings.m_fSamplingArea,			SamplingArea )
	PUBLISH_VAR_AS(	m_settings.m_fKeyValueMapping,		KeyValueMapping )
	PUBLISH_VAR_AS(	m_settings.m_fLuminanceBurnout,		LuminanceBurnout )
	PUBLISH_VAR_AS(	m_settings.m_fErrorReduction,		ErrorReduction )
							   
	PUBLISH_VAR_AS( m_settings.m_fBloomFilterMin,			BloomFilterMin )
	PUBLISH_VAR_AS( m_settings.m_fBloomFilterMax,			BloomFilterMax )
	PUBLISH_VAR_AS( m_settings.m_fBloomFilterEffectPower,	BloomFilterEffectPower )
	PUBLISH_VAR_AS( m_settings.m_fBloomGaussianLevels,		BloomGaussianLevels )

	PUBLISH_VAR_AS( m_settings.m_fKeyLuminanceMin,			KeyLumimanceMin )
	PUBLISH_VAR_AS( m_settings.m_fKeyLuminanceMax,			KeyLumimanceMax )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( Spatial_ExposureSettings_TOD, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_centerOfInfluence, CenterOfInfluence )
	PUBLISH_PTR_CONTAINER_AS( m_expSettings.m_list, ExposureSettings )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 

END_STD_INTERFACE

START_CHUNKED_INTERFACE( ExposureSet, Mem::MC_GFX )

	PUBLISH_PTR_CONTAINER_AS( m_nodes, SpatialNodes )
	DECLARE_POSTCONSTRUCT_CALLBACK(	PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE

//--------------------------------------------------
//!
//!	Spatial_ExposureSettings_TOD::PostConstruct 
//! Sort our list into an orderd list
//!
//--------------------------------------------------
void Spatial_ExposureSettings_TOD::PostConstruct()
{
	m_expSettings.SortListAscending();
}

//--------------------------------------------------
//!
//!	Spatial_ExposureSettings_TOD::SanityCheck 
//! makes sure our nodes are still ordered (in case 
//! of editing)
//!
//--------------------------------------------------
void Spatial_ExposureSettings_TOD::SanityCheck() const
{
#ifndef _RELEASE

	if (m_expSettings.IsUnsorted())
		m_expSettings.SortListAscending();

#endif
}

//--------------------------------------------------
//!
//!	Spatial_ExposureSettings_TOD::GetSettings 
//! Get exposure settings for this time of day
//!
//--------------------------------------------------
void Spatial_ExposureSettings_TOD::GetSettings( ExposureSettings& settings ) const
{
	if (m_expSettings.m_list.empty())
		return;

	// make sure we're still sorted
	SanityCheck();

	// retrieve bounding nodes
	float fTOD = LevelLighting::Get().GetTimeOfDay();
	const ExposureSettings_TOD *pPrev, *pNext;

	float fLerpVal = m_expSettings.GetBoundingNodes( fTOD, &pPrev, &pNext, 24.0f );

	// environment should be clear before we get it
	if (pPrev == pNext)
	{
		// simple
		settings = pPrev->m_settings;
	}
	else
	{
		// lerp
		settings = pPrev->m_settings;
		settings *= (1.0f - fLerpVal);

		ExposureSettings next;
		next = pNext->m_settings;
		next *= fLerpVal;

		settings += next;
	}
}




//--------------------------------------------------
//!
//!	ExposureSet::ctor 
//!
//--------------------------------------------------
ExposureSet::ExposureSet()
{
	m_pSpatialInterpolator = 0;
};


//--------------------------------------------------
//!
//!	ExposureSet::dtor 
//!
//--------------------------------------------------
ExposureSet::~ExposureSet()
{
	NT_DELETE_CHUNK( Mem::MC_GFX, m_pSpatialInterpolator );
};

//--------------------------------------------------
//!
//!	ExposureSet::PostConstruct 
//! construct our spatial interpolator
//!
//--------------------------------------------------
void ExposureSet::PostConstruct()
{
	if (m_pSpatialInterpolator)
		NT_DELETE_CHUNK( Mem::MC_GFX, m_pSpatialInterpolator );
	
	// pull out the centers of influence of our nodes
	u_int iNumNodes = m_nodes.size();
	CPoint* pTemp = NT_NEW_CHUNK( Mem::MC_GFX) CPoint [iNumNodes];

	int iCount = 0;
	for (	SpatialExposureSettingsList::iterator it = m_nodes.begin();
			it != m_nodes.end(); ++it, iCount++ )
	{
		pTemp[iCount] = (*it)->m_centerOfInfluence;
	}

	// construct our interpolator
//	m_pSpatialInterpolator = NT_NEW SI_DelaunayTriangulation( pTemp, iNumNodes );
	m_pSpatialInterpolator = NT_NEW_CHUNK(Mem::MC_GFX) SI_DistanceWeight( pTemp, iNumNodes );

	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pTemp );
}

bool ExposureSet::EditorChangeValue(CallBackParameter,CallBackParameter)
{
	PostConstruct();
	return false;
}

//--------------------------------------------------
//!
//!	ExposureSet::GetSettings
//! get an interpolated value for our most significant exposure nodes
//!
//--------------------------------------------------
void ExposureSet::GetSettings( const CPoint& position, ExposureSettings& settings ) const
{
	if (m_nodes.empty())
		return;

	settings.Clear();

	// retrive our results, stick them in a stack and sort by importances
	ntAssert( m_pSpatialInterpolator );
	m_pSpatialInterpolator->CalcNewResult( position );
	
	const SIresultList& results = m_pSpatialInterpolator->GetResultList();
	SIsortVector tempResults( results.size() );

	int iIndex = 0;
	for ( SIresultList::const_iterator it = results.begin(); it != results.end(); ++it )
		tempResults[iIndex++] = *it;

	ntstd::sort( &tempResults[0], &tempResults[0] + tempResults.size(), comparatorMoreThan() );

	// most significant ones should now be at the front of the stack, 
	// accumulate them into our result set

	static const u_int iMAX_TO_EVALUATE = 4;
	u_int iCurrItem = 0;
	while ( (iCurrItem < tempResults.size()) && (iCurrItem < iMAX_TO_EVALUATE) )
	{
		SpatialExposureSettingsList::const_iterator nodeIt( m_nodes.begin() );
		ntstd::advance( nodeIt, tempResults[iCurrItem]->ID );

		ExposureSettings next;
		(*nodeIt)->GetSettings( next );
		next *= tempResults[iCurrItem]->fraction;

		settings += next;
		iCurrItem++;
	}
}
