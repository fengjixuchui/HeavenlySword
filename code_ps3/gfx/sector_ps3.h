/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef GFX_SECTOR_PC_H
#define GFX_SECTOR_PC_H


#ifndef GFX_LISTSPACE_H
#include "gfx/listspace.h"
#endif

#ifndef GFX_HDR_RESOLVE_H
#include "gfx/HDRresolver.h"
#endif

#ifndef GFX_EXPOSURE_H
#include "gfx/exposure.h"
#endif

#ifndef GFX_LENSEFFECTS_H
#include "gfx/lenseffects.h"
#endif

#ifndef GFX_SHADOWSYSTEM_H
#include "gfx/shadowsystem.h"
#endif

#ifndef GFX_DEPTHOFFIELD_H
#include "gfx/depthoffield.h"
#endif

#ifndef GFX_AA_RESOLVER_H
#include "gfx/aaresolver.h"
#endif

#ifndef GFX_SHADOW_GATHERER_H
#include "gfx/shadowgatherer.h"
#endif

#ifndef NAO32_CONVERTER_H
#include "gfx/NAO32converter.h"
#endif

#ifndef GFX_IRRADIANCE_H
#include "gfx/irradiance_ps3.h"
#endif

#ifndef RADIAL_MOTION_BLUR_H
#include "gfx/radialmotionblur.h"
#endif



// for key binding
#include "game/commandresult.h"


// FIXME_WIL fake clouds object
class Clouds
{
public:
};

class SkyEffect;

//--------------------------------------------------
//!
//! Sector class
//!
//--------------------------------------------------
class CSector : public Singleton<CSector>
{
public:
	//! constructor
	CSector();
	~CSector();

	//! Creates the temporary sky effect.
	void CreateEffects();

	//! Set clouds
	void SetClouds(Clouds* pClouds)
	{
		if(pClouds)
		{ ntAssert(!m_pCloudsEffect); }
		m_pCloudsEffect = pClouds;
	}

	//! Gets the renderable container.
	ListSpace& GetRenderables() { return m_obRenderableSpace; }

	//! Renders the scene.
	void Render( const CCamera* pobCamera );

	// register key
	void RegisterKey();

	void DoForceDump() { m_bForceDump = true; };

	CRadialMotionBlur &GetMotionBlurObject(void)
	{
		return m_radialMotionBlur;
	}

private:

	COMMAND_RESULT CommandToggleFlags(const int& kind);
	typedef enum
	{
		F_PROFILEDISABLE_SHADOWRECIEVEPS = BITFLAG(0),
		F_PROFILEDISABLE_OPAQUERENDERPS = BITFLAG(1),
		F_DUMPVIEW = BITFLAG(2),
	} DebugState;
	typedef BitMask<DebugState,uint32_t> DebugMask;
	DebugMask m_debugMask;
	// register key end

	//! platform specific uber function
	void RenderContext();

	//! Rendering context preprocess
	void CalculatePlanePercents( float* fPlanePercents );
	void CalculateVisibility( const float fPlanePercents[5] );

	//! shadow generation
	void GenerateShadowMap();
	void RecieveShadows( const float fPlanePercents[5] );

	//! internal sub-render methods
	void RenderSky();
	void RenderClouds( bool bHDRAlphaSupport );

	void RenderHDRAlpha( bool bHDRAlphaSupport );
	void RenderLDRAlpha( bool bHDRAlphaSupport );

	ShadowMapSystem			m_shadowMapSystem;		//!< handles shadow map generation

	ListSpace				m_obRenderableSpace;	//!< Storage for renderables.
	
	ShadowGatherer			m_ShadowGatherer;		//!< class that gathers a per pixel occlusion term
	AAResolver				m_AAResolver;			//!< class that resolves AA render targets
	HDRResolver				m_HDRResolver;			//!< class that moves HDR buffer to LDR
	EXPControllerGPU		m_EXPController;		//!< class that measures log luminance of scene
	LensArtifacts			m_lensArtifacts;		//!< introduces bloom and ghosting to the scene
	StencilShadowSystem		m_stencilShadowSystem;	//!< handles stencil shadow generation
	DepthOfField			m_depthOfField;			//!< handles depth of field
	NAO32converter			m_NAO32converter;		//!< Converts from custom logYUV HDR to RGB HDR space
	IrradianceManager		m_IrradianceManager;	//!< It generates a per view/per frame cube map that cache our irradiance values
	CRadialMotionBlur		m_radialMotionBlur;

	SkyEffect*				m_pSkyBox;				//!< Sky box based on depth haze atmospherics
	Clouds*					m_pCloudsEffect;		//!< clouds effect.

	bool					m_bForceDump;
	bool					m_bDoDumpFrame;
};

#endif // ndef _SECTOR_H
