//--------------------------------------------------
//!
//!	\file levellighting.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/levellighting.h"
#include "effect/effect_error.h"
#include "objectdatabase/dataobject.h"
#include "gfx/postprocessing.h"
#include "gfx/filllightset.h"
#include "gfx/texturemanager.h"
#include "gfx/rendercontext.h"
#include "gfx/lenseffects.h"
#include "core/gatso.h"
#include "gfx/depthoffield.h"
#include "gfx/renderer.h"
#include "gfx/pictureinpicture.h"
#include "game/luaglobal.h"

START_CHUNKED_INTERFACE( SolarArc, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_fHorizonAngle,	HorizonAngle )
	PUBLISH_VAR_AS(	m_fTiltAngle,		TiltAngle )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( ShadowMapParamsDef, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_fShadowSlope,		fShadowSlope )
	PUBLISH_VAR_AS(	m_fShadowBias,		fShadowBias )
	PUBLISH_VAR_AS(	m_fShadowPlane0,	fShadowPlane0 )
	PUBLISH_VAR_AS(	m_fShadowPlane1,	fShadowPlane1 )
	PUBLISH_VAR_AS(	m_fShadowPlane2,	fShadowPlane2 )
	PUBLISH_VAR_AS(	m_fShadowPlane3,	fShadowPlane3 )
	PUBLISH_VAR_AS(	m_fShadowPlane4,	fShadowPlane4 )
	PUBLISH_VAR_AS(	m_bUseKeyDirection,	UseKeyDirection )
	PUBLISH_VAR_AS(	m_vShadowDirection,	ShadowDirection )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( LevelLightingDef, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_fSunRise,	SunRise )
	PUBLISH_VAR_AS(	m_fSunSet,	SunSet )
	PUBLISH_VAR_AS(	m_fPauseExposureFactor, PauseExposureFactor )

	PUBLISH_PTR_AS(				 m_pMoonDef,			MoonDef )
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pSunArc,				SunArc,				SolarArc )
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pMoonArc,			MoonArc,			SolarArc )
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pGhostSettings,		GhostingDef,		CGhostLensEffectSettings )
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pPostProcessingSet,	PostProcessingSet,	PostProcessingSet )
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pFillLightSet,		FillLightSet,		FillLightSet )
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pKeyLightSet,		KeyLightSet,		KeyLightSet )
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pExposureSet,		ExposureSet,		ExposureSet )
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pShadowMapParams,	ShadowMapParamsDef,	ShadowMapParamsDef	 )
	PUBLISH_VAR_AS(				 m_bSeperateHazeDir,	HaveSeperateHazeDirection )
	PUBLISH_VAR_AS(				 m_vHazeDir,			HazeDirection )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 

END_STD_INTERFACE



//------------------------------------------------------------------------------------------
//!
//!	LevelLightingCtrl::LoadLevelLightingDef
//! Unload current Level Lighting Def and load a new one
//!
//------------------------------------------------------------------------------------------

int LevelLightingCtrl::LoadLevelLightingDef(const char* pcNewLeveLightingDef)
{
	static char aFileName[MAX_PATH];
	Util::GetFiosFilePath( pcNewLeveLightingDef, aFileName );

	if( File::Exists( aFileName ) )
	{
		// Tell people what we're up to
		ntPrintf("XML Lighting Definition Loading \'%s\'\n", pcNewLeveLightingDef);

		// Open the XML file in memory
		FileBuffer obFile( aFileName, true );

		if ( !ObjectDatabase::Get().LoadDataObject( &obFile, pcNewLeveLightingDef ) )
		{
			ntError_p( false, ( "Failed to parse XML file '%s'", pcNewLeveLightingDef ) );
			return -1;
		}
		ntError_p( LevelLighting::Exists(), ("We should have a level lighting singleton by now") );
		LevelLighting::Get().UseLightingFile( CHashedString(pcNewLeveLightingDef) );
		CStaticLensConfig::LevelReset();
		return 0;
	}

	return -1;
}

void LevelLightingCtrl::SwitchLightingFile(const char* pcNewLeveLightingDef)
{
	ntError_p( LevelLighting::Exists(), ("We should have a level lighting singleton by now") );
	LevelLighting::Get().UseLightingFile( CHashedString(pcNewLeveLightingDef) );

}

float LevelLightingDef::s_fHoursInDay = 24.0f;

//--------------------------------------------------
//!
//!	LevelLightingDef::PostConstruct 
//! Create our level lighting singleton
//!
//--------------------------------------------------
void LevelLightingDef::PostConstruct()
{
	// first lighting def of this level, if so create level lighting singleton
	if ( !LevelLighting::Exists() )
	{
		NT_NEW_CHUNK( Mem::MC_GFX ) LevelLighting();
	}
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	ObjectContainer* pCont = pDO->GetParent();

	LevelLighting::Get().RegisterLightingFile( CHashedString(pCont->m_FileName), this );
}

//--------------------------------------------------
//!
//!	LevelLightingDef::~LevelLightingDef 
//! clean the singleton
//!
//--------------------------------------------------
LevelLightingDef::~LevelLightingDef()
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	ObjectContainer* pCont = pDO->GetParent();

	LevelLighting::Get().UnRegisterLightingFile( CHashedString(pCont->m_FileName) );
	
	if( LevelLighting::Get().GetNumLightingFilesRegistered() == 0 )
	{
		LevelLighting::Kill();
	}
}


//--------------------------------------------------
//!
//!	LevelLighting::LevelLighting
//!
//--------------------------------------------------
LevelLighting::LevelLighting() :
		m_pDef( 0 ),
		m_fTimeOfDay( 12.0f )
{
	for( int i=0;i < MAX_LEVELLIGHT_FILES;i++)
	{
		m_LevelLightDefs[i].m_pLightDef = 0;
	}
}

//--------------------------------------------------
//!
//! scan for a space in our light file and stick in in
//! there if we find one
//!
//--------------------------------------------------
void LevelLighting::RegisterLightingFile( CHashedString pLevelLightName, LevelLightingDef* pDef )
{
	for( int i=0;i < MAX_LEVELLIGHT_FILES;i++)
	{
		if( m_LevelLightDefs[i].m_pLightDef == 0 )
		{
			m_LevelLightDefs[i].m_pName = pLevelLightName;
			m_LevelLightDefs[i].m_pLightDef = pDef;
			return;
		}
	}
	ntError_p( false, ("No more room (Max %i)for any more level lighting file this level\n", MAX_LEVELLIGHT_FILES) );
}

//--------------------------------------------------
//!
//! scan for a space in our light file and stick in in
//! there if we find one
//!
//--------------------------------------------------
void LevelLighting::UseLightingFile( CHashedString pLevelLightName )
{
	for( int i=0;i < MAX_LEVELLIGHT_FILES;i++)
	{
		if( m_LevelLightDefs[i].m_pName == pLevelLightName )
		{
			m_pDef = m_LevelLightDefs[i].m_pLightDef;
			CStaticLensConfig::LevelReset();
			return;
		}
	}

#if !defined(_RELEASE )
	user_warn_p( false, ("Level Lighting file %s not found", pLevelLightName.GetDebugString()) );

	ntPrintf( "Loaded level lighting files :\n" );
	for( int i=0;i < MAX_LEVELLIGHT_FILES;i++)
	{
		if( m_LevelLightDefs[i].m_pName != 0 )
		{
			ntPrintf( "			: %s \n", m_LevelLightDefs[i].m_pName.GetDebugString() );
		}
	}
#endif
}
//--------------------------------------------------
//!
//! scan for this lihgitng file and remove it
//!
//--------------------------------------------------
void LevelLighting::UnRegisterLightingFile( CHashedString pLevelLightName )
{
	for( int i=0;i < MAX_LEVELLIGHT_FILES;i++)
	{
		if( m_LevelLightDefs[i].m_pName == pLevelLightName )
		{
			if( m_pDef == m_LevelLightDefs[i].m_pLightDef )
			{
				// no lighting now, until we switch to another one!
				m_pDef = 0;
			}
			m_LevelLightDefs[i].m_pLightDef  = 0;
			return;
		}
	}
	user_warn_p( false, ("Level Lighting file not found") );
}

int LevelLighting::GetNumLightingFilesRegistered()
{
	int count = 0;
	for( int i=0;i < MAX_LEVELLIGHT_FILES;i++)
	{
		if( m_LevelLightDefs[i].m_pLightDef != 0 )
		{
			count++;
		}
	}
	return count;
}

//--------------------------------------------------
//!
//!	LevelLighting::InstallDefaults 
//! Load a default XML lighting file if we havent
//! got a proper one
//!
//--------------------------------------------------
/*
void LevelLighting::InstallDefaults()
{
	ntAssert_p( !LevelLighting::Exists(), ("We already have a valid level light setup") );

	static char aFileName[MAX_PATH];
	Util::GetFiosFilePath( "data\\default_lighting.xml", aFileName );

	// Tell people what we're up to
	ntPrintf("XML loading \'%s\'\n", aFileName);

	// Open the XML file in memory
	FileBuffer obFile( aFileName, true );

	if ( !ObjectDatabase::Get().LoadDataObject( &obFile, aFileName ) )
	{
		ntError_p( false, ( "Failed to parse XML file '%s'", aFileName ) );
	}
}*/

//--------------------------------------------------
//!
//!	LevelLighting::GetWorldRimLight 
//! fake light shennanigans
//!
//--------------------------------------------------
CDirection LevelLighting::GetWorldRimLight(float /*fCoef*/, const CDirection& worldRealLight, const CDirection& /*worldCameraDirection*/)
{
/*
	CDirection worldFakeLight = worldCameraDirection;
	worldFakeLight.Y() += 0.5f;
	worldFakeLight *= (1.0f / worldFakeLight.Length());

	CDirection worldLerpLight = SimpleFunction::Lerp(worldFakeLight,worldRealLight,fCoef);
	worldLerpLight *= (1.0f / worldLerpLight.Length());
	
	return worldLerpLight;
/*/
	CDirection rim( -worldRealLight );
	if ( rim.Y() < 0.0f )
	{
		rim.Y() = 0.0f;
	}

	if ( rim.Length() > 0.0f )
	{
		rim /= rim.Length();
	}
	else
	{
		ntAssert_p( false, ("We should probably avoid the main light source running directly overhead...") );
	}

	return rim;
//*/
}

//--------------------------------------------------
//!
//!	LevelLighting::RetrieveContextLighting 
//! Setup the lighting in the current render context
//!
//--------------------------------------------------
void LevelLighting::RetrieveContextLighting( const CPoint& eyePos )
{
	CGatso::Start( "LevelLighting::RetrieveContextLighting" );

	ntError_p( m_pDef, ("No Lighting file currently being used\n") );

	// retrieve our parent viewport
	PIPView& viewport = Renderer::Get().m_pPIPManager->GetCurrentView();

	// post processing colour transforms
	m_pDef->m_pPostProcessingSet->GetMatrix( eyePos, RenderingContext::Get()->m_postProcessingMatrix );

	// fill light SH coefficients and reflectance params
	m_pDef->m_pFillLightSet->CalcMostSignificantNodes( eyePos );

	SHEnvironment environment;
	m_pDef->m_pFillLightSet->GetContribution( environment );

	RenderingContext::Get()->m_SHMatrices = environment;

	const char* pReflectTex = m_pDef->m_pFillLightSet->GetReflectanceTexture();
	if((pReflectTex) && (strcmp(pReflectTex,"NULL")!=0))
		RenderingContext::Get()->m_reflectanceMap = TextureManager::Get().LoadTexture_Neutral( pReflectTex );

	RenderingContext::Get()->m_reflectanceCol = m_pDef->m_pFillLightSet->GetReflectanceColour();

	// setup keylight params
	m_pDef->m_pKeyLightSet->GetKeyLight( eyePos, RenderingContext::Get()->m_keyLight );

	RenderingContext::Get()->m_keyDirection = -m_pDef->GetDirToKeyLight( m_fTimeOfDay );
	RenderingContext::Get()->m_toKeyLight = m_pDef->GetDirToKeyLight( m_fTimeOfDay );

	RenderingContext::Get()->m_keyColour = RenderingContext::Get()->m_keyLight.GetColour();
	RenderingContext::Get()->m_skyColour = RenderingContext::Get()->m_keyLight.GetSkyColour();

	if( m_pDef->m_bSeperateHazeDir  )
	{
		RenderingContext::Get()->m_sunDirection =  m_pDef->m_vHazeDir;
		RenderingContext::Get()->m_sunDirection.Normalise();
	} else
	{
		RenderingContext::Get()->m_sunDirection =  RenderingContext::Get()->m_keyDirection;
	}

	RenderingContext::Get()->m_keyColour.W() = 0.0f;
	RenderingContext::Get()->m_skyColour.W() = 0.0f;

	// exposure settings
	m_pDef->m_pExposureSet->GetSettings( eyePos, RenderingContext::Get()->m_exposureSettings );
	
	CStaticLensConfig::SetParameters(	RenderingContext::Get()->m_exposureSettings.m_fBloomFilterMin,
										RenderingContext::Get()->m_exposureSettings.m_fBloomFilterMax,
										RenderingContext::Get()->m_exposureSettings.m_fBloomFilterEffectPower,
										(int)(RenderingContext::Get()->m_exposureSettings.m_fBloomGaussianLevels) );

	// depth of field settings
	RenderingContext::Get()->m_depthOfFieldParams = viewport.m_DOFSettings.GetVSParameters();

	// shadow direction settings
	if( m_pDef->m_pShadowMapParams->m_bUseKeyDirection )
	{
		RenderingContext::Get()->m_shadowDirection = RenderingContext::Get()->m_keyDirection;
	} else
	{
		RenderingContext::Get()->m_shadowDirection = m_pDef->m_pShadowMapParams->m_vShadowDirection;
	}
	// copy them over
	NT_MEMCPY( &RenderingContext::Get()->m_shadowMapDef, m_pDef->m_pShadowMapParams, sizeof( ShadowMapParamsDef ) );

	CGatso::Stop( "LevelLighting::RetrieveContextLighting" );
}


