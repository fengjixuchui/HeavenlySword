//--------------------------------------------------
//!
//!	\file sector_pc.h
//!	PC portion of the sector renderer
//!
//--------------------------------------------------

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


class SkyEffect;
class Clouds;

//--------------------------------------------------
//!
//!	Sector class
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

	void DoForceDump() { m_bForceDump = true; };

	//! Gets the renderable container.
	ListSpace& GetRenderables() { return m_obRenderableSpace; }

	//! Renders the scene.
	void Render( const CCamera* pobCamera );

private:
	
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

	void RenderHDRAlpha( bool bHDRAlphaSupport );
	void RenderLDRAlpha( bool bHDRAlphaSupport );

	ListSpace				m_obRenderableSpace;	//!< Storage for renderables.

	AAResolver				m_AAResolver;			//!< class that resolves AA render targets
	HDRResolver				m_HDRResolver;			//!< class that moves HDR buffer to LDR
	
	// both methods present untill we figure out whats breaking the NV40's
	EXPControllerGPU		m_EXPControllerGPU;		//!< class that measures log luminance of scene
	EXPControllerCPU		m_EXPControllerCPU;		//!< class that measures log luminance of scene

	LensArtifacts			m_lensArtifacts;		//!< introduces bloom and ghosting to the scene

	ShadowMapSystem			m_shadowMapSystem;		//!< handles shadow map generation
	StencilShadowSystem		m_stencilShadowSystem;	//!< handles stencil shadow generation
	DepthOfField			m_depthOfField;			//!< handles depth of field

	SkyEffect*				m_pSkyBox;				//!< Cheesy temporary sky effect.
	Clouds*					m_pCloudsEffect;		//!< clouds effect.

	LevelsGraph				m_levelsGraph;			//!< some debug info

	bool					m_bForceDump;
	bool					m_bDoDumpFrame;
};

#endif // ndef _SECTOR_H
