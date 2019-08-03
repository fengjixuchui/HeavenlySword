/***************************************************************************************************
*
*	DESCRIPTION		The rendering pipeline for soft and stencil shadows.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef GFX_SHADOWSYSTEM_PC_H
#define GFX_SHADOWSYSTEM_PC_H

#ifndef CORE_BOUNDINGVOLUMES_H
#include "core/boundingvolumes.h"
#endif

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

#ifndef GFX_GAUSSIAN_H
#include "gfx/gaussian.h"
#endif

class Shader;

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

	void BuildShadowMapProjectionByIndex( unsigned int index );

private:
	// pParam is passed as void* but is really integer between 0 and 3
	void BuildShadowMapProjectionAsyncFunc( void* pParam );
	void BuildShadowMapProjection( unsigned int index, const CAABB& frustumAABB, const CAABB& casterAABB );

	int					m_iShadowMapSize;	//!< shadow map dimension
	RenderTarget::Ptr	m_pShadowMap;		//!< The current shadow map.
	Surface::Ptr		m_pShadowMapDepths;	//!< The associated shadow map depth buffer.

	RenderTarget::Ptr	m_pDummy;			//!< Needed to keep shadow map hardware rendering happy (but not actually used)

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

	void AllocateTextures(int iWidth,int iHeight,bool bCacheable);
	void DeallocateTextures();

	void RenderStencilToColour();
	void FinaliseStencilMap();

	void DebugRender();

	RenderTarget::Ptr	m_pStencilResult;	//!< Stencil colour result.
private:

	Shader* m_pStencilToColourVS;	//!< Shader for moving stencil into colour.
	Shader* m_pStencilToColourPS;
	CVertexDeclaration m_pStencilToColourDecl;

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

	CShadowSystemController() : m_bUseStencilling( false ) {}
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

#endif // ndef GFX_SHADOWSYSTEM_PC_H
