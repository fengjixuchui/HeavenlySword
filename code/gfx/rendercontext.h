//--------------------------------------------------
//!
//!	\file rendercontext.h
//!	Everything required to render a given context
//!
//--------------------------------------------------
#ifndef GFX_RENDERCONTEXT_H
#define GFX_RENDERCONTEXT_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

#ifndef GFX_CAMERA_H
#include "gfx/camera.h"
#endif

#ifndef GFX_SPHERICAL_HARMONICS_H
#include "gfx/sphericalharmonics.h"
#endif

#ifndef GFX_KEYLIGHT_SET_H
#include "gfx/keylightset.h"
#endif

#ifndef GFX_EXPOSURE_SET_H
#include "gfx/exposureset.h"
#endif

#ifndef _LEVEL_LIGHTING_H
#include "gfx/levellighting.h"
#endif

#ifndef CORE_BOUNDINGVOLUMES_H
#include "core/boundingvolumes.h"
#endif

class CRenderable;
class CMeshInstance;

struct ScissorRegion
{
    int    left;
    int    top;
    int    right;
    int    bottom;
};

struct Heresy_GlobalDataT;

//--------------------------------------------------
//!
//!	RenderingContext
//!
//--------------------------------------------------
class RenderingContext
{
public:
	//! Allocates storage for the contexts.
	static void Initialise();

	//! Destroys context storage.
	static void Destroy();
	
	//! Pushes the rendering context.
	static void PushContext();

	//! Pops the current rendering context.
	static void PopContext();

	//! The data structure to hold/cache information for a given rendering context.
	struct CONTEXT_DATA
	{
		const CCamera* m_pCullCamera;		//!< The culling camera.
		const CCamera* m_pViewCamera;		//!< The viewing camera.

		CMatrix m_worldToView;				//!< The transform from world space to view space.
		CMatrix m_worldToScreen;			//!< The transform from world space to screen space.
		CMatrix m_viewToScreen;				//!< The transform from view space to screen.
		float	m_fScreenAspectRatio;		//!< The aspect ratio of this context when its's created

		Texture::Ptr m_pStencilTarget;		//!< The stencil map currently in use.
		Texture::Ptr m_pShadowMap;			//!< The shadow map currently in use.
		Texture::Ptr m_pIrradianceCache;	//!< The irradiance cube map currently in use

		CVector	m_ExposureAndToneMapConsts;	//!< Constants that are used in our tone mapping pixel shader code			

		CMatrix m_shadowMapProjection[4];	//!< The transform's from world space to shadow map space.
        CMatrix m_viewToLightTransform[4];  //!< The transform's from view space to shadow map space.
		CVector	m_shadowPlanes[5];			//!< the shadow clipping planes
		CVector m_shadowRadii[4];			//!< used by the shadow anti-aliasing
		ScissorRegion m_shadowScissor[4];	//!< used to render 4 maps to a single texture

		CAABB m_shadowCasterAABB[4];		//!< AABB that fits the shadow caster
		CAABB m_shadowFrustumAABB[4];		//!< AABB that fits the shadow frustum

		ShadowMapParamsDef m_shadowMapDef;	//! the level shadows parameters

		// fill light params
		SHChannelMatrices	m_SHMatrices;	//!< SH coeffs in upload ready form
		Texture::Ptr		m_reflectanceMap;	//!< Reflectance map
		CDirection			m_reflectanceCol;	//!< Reflectance colour

		// key light params
		CDirection			m_keyDirection;	//!< Direction of keylight
		CDirection			m_toKeyLight;	//!< Direction to keylight src
		KeyLightNode		m_keyLight;		//!< Misc keylight params
		CDirection			m_shadowDirection;	//!< Direction of the shadow
		CDirection			m_sunDirection;	//!< Direction of the sun for atmospheric haze

		CDirection			m_keyColour;	//!< Colour of the keylight
		CDirection			m_skyColour;		//!< Colour of the sky

		// post processing
		CMatrix				m_postProcessingMatrix;	//!< The current post-processing matrix
		ExposureSettings	m_exposureSettings;		//!< The current values for the exposure function

		typedef ntstd::Vector<CRenderable*, Mem::MC_GFX>	RenderableVector;
		typedef ntstd::Vector<CMeshInstance*, Mem::MC_GFX>	BatchableVector;
		RenderableVector	m_aVisibleRenderables;				//!< List of visible renderables.
		BatchableVector		m_aBatchableVisibleRenderables;		//!< List of batch-able visible renderables. (they MIGHT be batched)

		RenderableVector	m_aShadowCastingRenderables;		//!< List of shadow casting renderables.
		BatchableVector		m_aBatchableShadowCastingRenderables;	//!< List of batch-able shadow casting renderables. (they MIGHT be batched)
		RenderableVector	m_aBatchedShadowCastingRenderables;	//!< List of batched visible opaque renderables.

		RenderableVector	m_aVisibleOpaqueRenderables;		//!< List of visible opaque renderables.
		RenderableVector	m_aBatchedVisibleOpaqueRenderables;	//!< List of batched visible opaque renderables.

		RenderableVector	m_aVisibleAlphaRenderables;			//!< List of visible alpha renderables.
		RenderableVector	m_aBatchedVisibleAlphaRenderables;	//!< List of visible alpha renderables.

		RenderableVector	m_aShadowCastingOpaqueRenderables;		
		RenderableVector	m_aShadowCastingAlphaRenderables;		

		// Get the current view pos
		CPoint GetEyePos() const { return m_pViewCamera->GetEyePos(); }

		// depth of field
		CVector				m_depthOfFieldParams;				
	};
	
	//! Gets the current context index.
	static int GetIndex() { return ms_iCurrentContext; }
	
	//! Gets the current context.
	static CONTEXT_DATA* Get() { return ms_pstCurrentContext; }

	static unsigned int	ms_uiRenderContextTick;		//!< a simple counter that gets incremented every push (for caches etc.)

	static Heresy_GlobalDataT* ms_pHeresyGlobal;

private:
	static int ms_iCurrentContext;						//!< The current rendering context index.
	static CScopedArray<CONTEXT_DATA> ms_astContexts;	//!< The contexts.
	static CONTEXT_DATA* ms_pstCurrentContext;			//!< The current context.
};

#endif // ndef GFX_RENDERCONTEXT_H
