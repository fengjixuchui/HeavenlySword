//--------------------------------------------------
//!
//!	\file keylightset.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "keylightset.h"
#include "objectdatabase/dataobject.h"
//#include "timer.h"
#include "levellighting.h"

START_CHUNKED_INTERFACE( KeyLightNode_TOD, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_fTOD,							TimeOfDay )
	PUBLISH_VAR_AS(	m_light.m_colour,				Colour )
	PUBLISH_VAR_AS(	m_light.m_skyColour,			SkyColour )
	PUBLISH_VAR_AS(	m_light.m_rayleighScattering,	RayleighScattering )
	PUBLISH_VAR_AS(	m_light.m_mieScattering,		MieScattering )
	PUBLISH_VAR_AS( m_light.m_fInscatterMultiplier,				InscatterMultiplier )
	PUBLISH_VAR_AS( m_light.m_fHenleyGreensteinEccentricity,	HenleyGreensteinEccentricity )
	PUBLISH_VAR_AS( m_light.m_fSunPower,						SunPower )
	PUBLISH_VAR_AS( m_light.m_fSunMultiplier, 					SunMultiplier )
	PUBLISH_VAR_AS( m_light.m_fWorldSunPower, 					WorldSunPower )
	PUBLISH_VAR_AS( m_light.m_fWorldSunMultiplier,				WorldSunMultiplier )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( Spatial_KeyLightNode_Simple, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_centerOfInfluence,			CenterOfInfluence )
	PUBLISH_VAR_AS(	m_light.m_colour,				Colour )
	PUBLISH_VAR_AS(	m_light.m_skyColour,			SkyColour )
	PUBLISH_VAR_AS(	m_light.m_rayleighScattering,	RayleighScattering )
	PUBLISH_VAR_AS(	m_light.m_mieScattering,		MieScattering )
	PUBLISH_VAR_AS( m_light.m_fInscatterMultiplier,				InscatterMultiplier )
	PUBLISH_VAR_AS( m_light.m_fHenleyGreensteinEccentricity,	HenleyGreensteinEccentricity )
	PUBLISH_VAR_AS( m_light.m_fSunPower,						SunPower )
	PUBLISH_VAR_AS( m_light.m_fSunMultiplier, 					SunMultiplier )
	PUBLISH_VAR_AS( m_light.m_fWorldSunPower, 					WorldSunPower )
	PUBLISH_VAR_AS( m_light.m_fWorldSunMultiplier,				WorldSunMultiplier )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( Spatial_KeyLightNode_TOD, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_centerOfInfluence, CenterOfInfluence )
	PUBLISH_PTR_CONTAINER_AS( m_keyNodes.m_list, KeyLightNodes )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 

END_STD_INTERFACE

START_CHUNKED_INTERFACE( KeyLightSet, Mem::MC_GFX )

	PUBLISH_PTR_CONTAINER_AS( m_nodes, SpatialNodes )
	DECLARE_POSTCONSTRUCT_CALLBACK(	PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE

// Rayleigh and Mie coefficients calculated at Blue=434nm, Green=498nm, Red=680nm 
#define PI_3 (PI*PI*PI)
const CVector KeyLightNode::s_RaylieghCoEff(1.8721e-7f*PI_3, 6.5078e-7f*PI_3,1.128e-6f*PI_3, 0);
const CVector KeyLightNode::s_MieCoEffatT2( 4.07509E-4f*PI_3, 3.09498E-4f*PI_3, 1.6600E-4f*PI_3, 0);
const CVector KeyLightNode::s_MieCoEffatT16( 0.00608f*PI_3, 0.00462f*PI_3, 0.002478f*PI_3, 0);

//--------------------------------------------------
//!
//!	KeyLightNode::ctor
//!
//--------------------------------------------------
KeyLightNode::KeyLightNode() :
	m_colour( 1.0f, 1.0f, 1.0f, 10.0f ) , 
	m_skyColour( 1.0f, 1.0f, 1.0f, 1.0f ),
	m_rayleighScattering( s_RaylieghCoEff ),
	m_mieScattering( CVector::Lerp( s_MieCoEffatT2, s_MieCoEffatT16, 0.0f) ),
	m_fInscatterMultiplier( 0.3f ),
	m_fHenleyGreensteinEccentricity( 0.9f ),
	m_fSunPower( 40.f ),
	m_fSunMultiplier( 1.f ),
	m_fWorldSunPower( 0.f ),
	m_fWorldSunMultiplier( 0.f )
{}




//--------------------------------------------------
//!
//!	Spatial_KeyLightNode_TOD::PostConstruct 
//! Sort our list into an orderd list
//!
//--------------------------------------------------
void Spatial_KeyLightNode_TOD::PostConstruct()
{
	m_keyNodes.SortListAscending();
}

//--------------------------------------------------
//!
//!	Spatial_KeyLightNode_TOD::SanityCheck 
//! makes sure our nodes are still ordered (in case 
//! of editing)
//!
//--------------------------------------------------
void Spatial_KeyLightNode_TOD::SanityCheck() const
{
#ifndef _RELEASE

	if (m_keyNodes.IsUnsorted())
		m_keyNodes.SortListAscending();

#endif
}

//--------------------------------------------------
//!
//!	Spatial_KeyLightNode_TOD::GetKeyLight 
//! get our key light at the current time of day
//!
//--------------------------------------------------
void Spatial_KeyLightNode_TOD::GetKeyLight( KeyLightNode& keyLight ) const
{
	if (m_keyNodes.m_list.empty())
		return;

	// make sure we're still sorted
	SanityCheck();

	// retrieve bounding nodes
	float fTOD = LevelLighting::Get().GetTimeOfDay();
	const KeyLightNode_TOD *pPrev, *pNext;

	float fLerpVal = m_keyNodes.GetBoundingNodes( fTOD, &pPrev, &pNext, 24.0f );

	// environment should be clear before we get it
	if (pPrev == pNext)
	{
		// simple
		keyLight = pPrev->m_light;
	}
	else
	{
		// lerp
		keyLight = pPrev->m_light;
		keyLight *= (1.0f - fLerpVal);

		KeyLightNode next;
		next = pNext->m_light;
		next *= fLerpVal;

		keyLight += next;
	}
}




//--------------------------------------------------
//!
//!	KeyLightSet::ctor 
//!
//--------------------------------------------------
KeyLightSet::KeyLightSet()
{
	m_pSpatialInterpolator = 0;
};


//--------------------------------------------------
//!
//!	KeyLightSet::dtor 
//!
//--------------------------------------------------
KeyLightSet::~KeyLightSet()
{
	NT_DELETE_CHUNK( Mem::MC_GFX, m_pSpatialInterpolator );
};

//--------------------------------------------------
//!
//!	KeyLightSet::PostConstruct 
//! construct our spatial interpolator
//!
//--------------------------------------------------
void KeyLightSet::PostConstruct()
{
	if (m_pSpatialInterpolator)
		NT_DELETE_CHUNK( Mem::MC_GFX, m_pSpatialInterpolator );
	
	// pull out the centers of influence of our nodes
	u_int iNumNodes = m_nodes.size();
	CPoint* pTemp = NT_NEW_ARRAY_CHUNK(Mem::MC_GFX) CPoint [iNumNodes];

	int iCount = 0;
	for (	SpatialKeyLightNodeList::iterator it = m_nodes.begin();
			it != m_nodes.end(); ++it, iCount++ )
	{
		pTemp[iCount] = (*it)->m_centerOfInfluence;
	}

	// construct our interpolator
//	m_pSpatialInterpolator = NT_NEW SI_DelaunayTriangulation( pTemp, iNumNodes );
	m_pSpatialInterpolator = NT_NEW_CHUNK( Mem::MC_GFX ) SI_DistanceWeight( pTemp, iNumNodes );

	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pTemp );
}

bool KeyLightSet::EditorChangeValue(CallBackParameter,CallBackParameter)
{
	PostConstruct();
	return false;
}

//--------------------------------------------------
//!
//!	KeyLightSet::GetKeyLight
//! get an interpolated value for our most significant keylights
//!
//--------------------------------------------------
void KeyLightSet::GetKeyLight( const CPoint& position, KeyLightNode& keyLight ) const
{
	if (m_nodes.empty())
		return;

	keyLight.Clear();

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
		SpatialKeyLightNodeList::const_iterator nodeIt( m_nodes.begin() );
		ntstd::advance( nodeIt, tempResults[iCurrItem]->ID );

		KeyLightNode next;
		(*nodeIt)->GetKeyLight( next );
		next *= tempResults[iCurrItem]->fraction;

		keyLight += next;
		iCurrItem++;
	}
}
