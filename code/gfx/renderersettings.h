/***************************************************************************************************
*
*	DESCRIPTION		Renderer runtime switches.
*
*	NOTES			Debug settings and such like for the rendering bits and bobs. This is likely
*					to be a bit hacky until the end of the prototype.
*
***************************************************************************************************/

#ifndef GFX_RENDERERSETTINGS_H
#define GFX_RENDERERSETTINGS_H

#ifndef GFX_DISPLAY_H
#include "gfx/display.h"
#endif

enum AAMode
{
	AAM_NONE = 0,
	AAM_SUPERSAMPLE_4X = 1,
	AAM_MULTISAMPLE_4X = 2,
};

enum AAResolveMode
{
	AARES_BILINEAR_FILTER = 0,
	AARES_GAUSSIAN_FILTER = 1,
};

#ifdef PLATFORM_PS3
enum ANISOTROPIC_FILTERING_QUALITY
{
	AFQ_Disabled = 0,
	AFQ_Low = 1,
	AFQ_High = 2,
};
#endif


class CRendererSettings
{
public:
	// global badness
	// ---------------
#ifdef PLATFORM_PS3
	static DisplayManager::INTERNAL_MODE eDisplayMode;		//!< can be overidden by game.config
#endif

	static bool bUseTextures;					//!< Do we load textures or just use plain white ones?
	static bool bUseGTFTextures;				//!< Do we use GTF textures?
	static bool bUseParallaxMapping;			//!< Indicates we use parallax mapping techinique on normal mapped materials

	static bool bLogTextureErrors;				//!< Pipe out any problems with textures to a log file
	static bool bLogTextureInfo;				//!< Indicates we refuse to load invalid textures

//	static bool bLogTextureErrors;				//!< Pipe out any problems with textures to a log file
//	static bool bLogTextureInfo;				//!< Indicates we refuse to load invalid textures




	static bool bCheckTextureNameValid;			//!< Flag incorrectly named textures as a problem
	static bool bReplaceInvalidTextures;		//!< Indicates we refuse to load invalid textures
	static bool bGetDebugDOF;					//!< Debug display of the depth of field

	static bool bForceShaderModel2;				//!< Disable higher level rendering
	static bool bUseExteriorShadowSettings;		//!< exterior shadow settings should be use
	static bool bUseCustomShadowSettings;		//!< A completely custom set of shadows settings should be used
	static float fCustomShadowSettings[5];		//!< if we have custom shadow settings they are stored here...
	static bool bUseCustomSlopeAndBiasSettings;	//!< Custom polygon slope and bias settings..
	static float fCustomSlopeAndBias[2];		//!< custom slope and bias are stored here
	static int	iShadowQuality;					//! 0 to 4 (0 = off, 4 being best)
	static AAResolveMode eAAResolveMode;		//!< RSX doesn't automatically handle AAed RTs, so we can use custom filters

#ifdef PLATFORM_PS3
	static bool	bProfileLowFillrate;			//!< Set rendertarget allocations to be tiny
	static bool	bUseLowRezMode;					//!< Set rendertarget allocations to be half normal
	static bool	bUseZPrepass;					//!< Toggle z-prepassing
	static bool	bUseFastShadowRecieve;			//!< 
	static bool bUseHeresy;						//!< Use our heretical but fast graphics engine/driver
	static bool bLiveMaterialParamsEditing;		//!< toggle live material editing. Just bsskin for now
	static bool bEnableBatchRenderer;			//!< toggle SPU Batch Rendering
	static bool bEnableSPURenderer; 			//!< toggle SPU (non batched) Rendering
	static bool bEnableBeyondDebugging;			//!< toggle beyond debugging support
	static int	iDebugLayer;					//!< cycle debug texture layer!
#ifdef _PROFILING
	// flags used purely for profiling- no other function
	static bool	bProfileDisable_ShadowRecievePS;	//!< Targeted removoval of the shadow generate pass pixel shader
	static bool	bProfileDisable_OpaqueRenderPS;		//!< Targeted removoval of the opaque render pass pixel shader
#endif

#endif

	// debug info
	// ---------------

	static bool bShowWireframe;					//!< When true renders a low quality wireframe view.
	static bool bShowBoundingBoxes;				//!< Shows bounding boxes around all renderables.
	static bool bShowExposureSamplingArea;		//!< Shows the exposure sampling area.
	static bool bShowLightingInfo;				//!< Show debug lighting info.
	static bool bShowLevelsGraph;				//!< Show debug levels graph.

	// feature toggles
	// -----------------

	static bool bEnableDebugPrimitives;			//!< Turns debug primitives on/off.
	static bool bEnableCloth;					//!< Turns cloth on/off.
	static bool bEnableCulling;					//!< Turns on/off view frustum culling.
	static bool bEnableSky;						//!< Turns sky effects on/off.
	static bool bEnableLensEffects;				//!< Turns lens effects on/off.
	static bool bEnableBackfaceCulling;			//!< Turns on/off backface culling.
	static bool bEnableShadows;					//!< Turns shadows on/off.
	static bool bEnableMaterials;				//!< Reflects the actual state of materials.
	static bool bEnableGammaCorrection;			//!< Turns on/off gamma correction.
	static bool bEnableDepthHaze;				//!< Turns on/off depth haze
	static bool bEnableLensGhost;
	static bool bEnableSkinning;				//!< Turns on/off skinning.
	static bool bEnableSphericalHarmonics;		//!< Turns on/off SH lighting (unwise to turn off).
	static bool bEnableExposure;				//!< Turns on/off exposure sampling
	static bool bUseGPUExposure;				//!< Switch exposure generation method
	static bool bUseAlphaBlending;				//!< Turns on/off alpha blendind
	static bool bEnableOptimiseMeshesOnLoad;

	//! see if we can optimise our meshes on startup Legacy function REMOVE
#ifdef PLATFORM_PC
	inline static bool OptimiseMeshesOnLoad() { return bEnableOptimiseMeshesOnLoad; }
#elif defined (PLATFORM_PS3)
	inline static bool OptimiseMeshesOnLoad() { return false; };
#endif
	static AAMode GetAAMode();

	// this is only public so shell can set it, please dont fiddle with it anywhere!
	static AAMode eAAMode;						//!< AntiAliasing mode (super or multisampling)

#ifdef PLATFORM_PS3
	static ANISOTROPIC_FILTERING_QUALITY eAnisotropicFilterQuality;
#endif

};

#endif // ndef GFX_RENDERERSETTINGS_H
