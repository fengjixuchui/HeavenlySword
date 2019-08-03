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

bool CRendererSettings::bUseParallaxMapping			= true;
bool CRendererSettings::bUseExteriorShadowSettings	= false;
bool CRendererSettings::bUseCustomShadowSettings	= false;
float CRendererSettings::fCustomShadowSettings[5];
bool CRendererSettings::bUseCustomSlopeAndBiasSettings	= false;
float CRendererSettings::fCustomSlopeAndBias[2];
int CRendererSettings::iShadowQuality				=	4;			// best quality to start with
bool CRendererSettings::bEnableOptimiseMeshesOnLoad = true;
AAMode CRendererSettings::eAAMode					= AAM_NONE;		// AA modes will be used only with SM3.0 GPUs, no matter what value this setting has been set
AAResolveMode CRendererSettings::eAAResolveMode		= AARES_BILINEAR_FILTER;

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
bool CRendererSettings::bUseGTFTextures				= false;
bool CRendererSettings::bLogTextureErrors			= true;
bool CRendererSettings::bCheckTextureNameValid		= true;
bool CRendererSettings::bReplaceInvalidTextures		= false;
bool CRendererSettings::bLogTextureInfo				= true;

bool CRendererSettings::bForceShaderModel2			= false;
bool CRendererSettings::bEnableCloth				= false;
bool CRendererSettings::bEnableCulling				= true;
bool CRendererSettings::bEnableBackfaceCulling		= true;
bool CRendererSettings::bEnableSky					= true;

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
bool CRendererSettings::bEnableShadows				= true;
#endif

bool CRendererSettings::bEnableGammaCorrection		= true;
bool CRendererSettings::bEnableLensGhost			= true;
bool CRendererSettings::bEnableSphericalHarmonics	= true;
bool CRendererSettings::bEnableSkinning				= true;
bool CRendererSettings::bEnableExposure				= true;
bool CRendererSettings::bUseGPUExposure				= true;
bool CRendererSettings::bUseAlphaBlending			= true;

//--------------------------------------------------
//!
//!	CRenderSettings::GetAAMode()
//! Return a supported (settings aside) AA Mode (PS2.0 hw get no AA, no matter what)
//!
//--------------------------------------------------
AAMode CRendererSettings::GetAAMode()
{
	if (HardwareCapabilities::Get().SupportsVertexShader3() && HardwareCapabilities::Get().SupportsPixelShader3())
	{
		ntAssert_p((CRendererSettings::eAAMode != AAM_MULTISAMPLE_4X), ("Multisampling is not (yet) supported on this platform"));
		return CRendererSettings::eAAMode;
	}
	else return AAM_NONE;
}

//--------------------------------------------------
//!
//!	CDepthHazeSetting::GetAConsts()
//! CONSTS_A = 1 < log2 *e, termMultiplier >
//!
//--------------------------------------------------







