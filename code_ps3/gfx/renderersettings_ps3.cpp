/***************************************************************************************************
*
*	DESCRIPTION		Renderer runtime switches.
*
*	NOTES			Debug settings and such like for the rendering bits and bobs.
*
***************************************************************************************************/

#include "gfx/renderersettings.h"
#include "core/profiling.h"
#include "gfx/hardwarecaps.h"


// global badness
// ---------------
DisplayManager::INTERNAL_MODE	CRendererSettings::eDisplayMode = DisplayManager::SIXTEEN_NINE_720;

int CRendererSettings::iDebugLayer					= 0;
bool CRendererSettings::bEnableBeyondDebugging		= false;
bool CRendererSettings::bEnableBatchRenderer		= true;
bool CRendererSettings::bEnableSPURenderer			= true;
bool CRendererSettings::bUseParallaxMapping			= true;
bool CRendererSettings::bUseExteriorShadowSettings	= false;
bool CRendererSettings::bUseCustomShadowSettings	= false;
float CRendererSettings::fCustomShadowSettings[5];
bool CRendererSettings::bUseCustomSlopeAndBiasSettings	= false;
float CRendererSettings::fCustomSlopeAndBias[2];
int CRendererSettings::iShadowQuality				=	3;			// best quality to start with
bool CRendererSettings::bEnableOptimiseMeshesOnLoad = false;
AAMode CRendererSettings::eAAMode					= AAM_MULTISAMPLE_4X;
//AAMode CRendererSettings::eAAMode					= AAM_SUPERSAMPLE_4X;
//AAMode CRendererSettings::eAAMode					= AAM_NONE;
AAResolveMode CRendererSettings::eAAResolveMode		= AARES_BILINEAR_FILTER;

bool CRendererSettings::bProfileLowFillrate			= false;
bool CRendererSettings::bUseLowRezMode				= false;
bool CRendererSettings::bUseZPrepass				= false;
bool CRendererSettings::bUseFastShadowRecieve		= true;
bool CRendererSettings::bUseHeresy					= true;

#ifdef _PROFILING
bool CRendererSettings::bProfileDisable_ShadowRecievePS	= false;
bool CRendererSettings::bProfileDisable_OpaqueRenderPS	= false;
#endif

// debug info
// ---------------

bool CRendererSettings::bShowWireframe				= false;
bool CRendererSettings::bShowBoundingBoxes			= false;
bool CRendererSettings::bShowExposureSamplingArea	= false;
bool CRendererSettings::bShowLightingInfo			= false;
bool CRendererSettings::bShowLevelsGraph			= false;
bool CRendererSettings::bGetDebugDOF				= false;

// feature toggles
// -----------------
bool CRendererSettings::bEnableDebugPrimitives		= true;

bool CRendererSettings::bUseTextures				= true;

#ifdef _MASTER
// coment back in when we have auto GTF generation in wads
//bool CRendererSettings::bUseGTFTextures				= true;
bool CRendererSettings::bUseGTFTextures				= false;
#else
bool CRendererSettings::bUseGTFTextures				= false;
#endif

bool CRendererSettings::bLogTextureErrors			= true;
bool CRendererSettings::bCheckTextureNameValid		= true;
bool CRendererSettings::bReplaceInvalidTextures		= false;
bool CRendererSettings::bLogTextureInfo				= true;

bool CRendererSettings::bForceShaderModel2			= false;
bool CRendererSettings::bEnableCloth				= false;
bool CRendererSettings::bEnableCulling				= true;
bool CRendererSettings::bEnableBackfaceCulling		= true;
bool CRendererSettings::bEnableSky					= false;

// start debug fast with the graphics options turned down
#if defined(_DEBUG_FAST)

bool CRendererSettings::bEnableLensEffects			= false;
bool CRendererSettings::bEnableMaterials			= false;
bool CRendererSettings::bEnableDepthHaze			= false;
bool CRendererSettings::bEnableShadows				= false;

#else
bool CRendererSettings::bEnableLensEffects			= true;
bool CRendererSettings::bEnableMaterials			= true;
bool CRendererSettings::bEnableDepthHaze			= true;
bool CRendererSettings::bEnableShadows				= true; // for now off cos we broke them
#endif

bool CRendererSettings::bEnableGammaCorrection		= true;
bool CRendererSettings::bEnableLensGhost			= true;
bool CRendererSettings::bEnableSphericalHarmonics	= true;
bool CRendererSettings::bEnableSkinning				= true;
bool CRendererSettings::bEnableExposure				= true;
bool CRendererSettings::bUseGPUExposure				= true;
bool CRendererSettings::bUseAlphaBlending			= true;
bool CRendererSettings::bLiveMaterialParamsEditing	= false;

ANISOTROPIC_FILTERING_QUALITY CRendererSettings::eAnisotropicFilterQuality = AFQ_High;

//--------------------------------------------------
//!
//!	CRenderSettings::GetAAMode()
//! Return a supported (settings aside) AA Mode (PS2.0 hw get no AA, no matter what)
//!
//--------------------------------------------------
AAMode CRendererSettings::GetAAMode()
{
	return CRendererSettings::eAAMode;
}

//--------------------------------------------------
//!
//!	CDepthHazeSetting::GetAConsts()
//! CONSTS_A = 1 < log2 *e, termMultiplier >
//!
//--------------------------------------------------




