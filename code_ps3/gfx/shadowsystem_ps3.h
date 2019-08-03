/***************************************************************************************************
*
*	DESCRIPTION		The rendering pipeline for soft and stencil shadows.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef GFX_SHADOWSYSTEM_PS3_H
#define GFX_SHADOWSYSTEM_PS3_H

#include "gfx/vertexdeclaration.h"
#include "gfx/gaussian.h"
#include "core/boundingvolumes.h"
#include "gfx/rendertarget.h"
#include "gfx/surface.h"
#include "gfx/shader.h"

class CRenderable;
class CullingFrustum;


//#define _DEBUG_SHADOW

//--------------------------------------------------
//!
//!	Class responsible for generating shadow maps for a sector
//!
//--------------------------------------------------
class ShadowMapSystem

{
public:
	ShadowMapSystem( int iShadowMapSize );
	~ShadowMapSystem();

	void PrepareShadowMapForRendering();
	void FinaliseShadowMap();

	void DebugRender();
private:
	void BuildShadowMapProjection( unsigned int index, const CAABB& frustumAABB, const CAABB& casterAABB, bool bFullShadowMap = false );

	int m_iShadowMapSize;

	RenderTarget::Ptr	m_pShadowMap;	//!< The current shadow map.
	RenderTarget::Ptr	m_pAlias;		//!< A fake render target aliased to the shadow map

	CMatrix		m_properViewProjMatrix;	//!< back-up for my nasty hack..
};

//--------------------------------------------------
//!
//!	Class responsible for generating stencil shadows for a sector
//!
//--------------------------------------------------
class StencilShadowSystem
{
public:
	StencilShadowSystem();

	void RenderStencilToColour();
	void DebugRender( RenderTarget::Ptr stencilResult );

private:

	DebugShader *m_stencilToColourVS;	//!< Shader for moving stencil into colour.
	DebugShader *m_stencilToColourPS;
	
	VBHandle		m_hSimpleQuadData;
	NewGaussianBlur m_blurHelper;		//!< Blurs the stencil colour target.
};

/***************************************************************************************************
*
*	CLASS			CShadowSystemController
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

class CShadowSystemController : public Singleton<CShadowSystemController>
{
public:
	friend class ShadowMapSystem;

	CShadowSystemController() { m_bUseStencilling = false; } 
	void LevelReset() { m_bUseStencilling = false; } 

	void SetStencilActive( bool bActive );

	bool IsShadowMapActive() const;
	bool IsStencilActive() const;

	CAABB const& GetShadowMapBounds() const;

	void RenderCasters() const;
	void RenderRecievers() const;

private:
    bool m_bUseStencilling;						//!< True if we should be using stencilling too.	
};

#endif // ndef GFX_SHADOWSYSTEM_PS3_H
