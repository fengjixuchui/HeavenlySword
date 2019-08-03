/***************************************************************************************************
*
*	$Header:: /game/renderer.h 32    11/08/03 10:09 Simonb                                         $
*
*	The Renderer 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#ifndef GFX_RENDERER_PC_H
#define GFX_RENDERER_PC_H

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

// useful defines
#define D3DCOLORWRITEENABLE_RGB ( D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE )
#define D3DCOLORWRITEENABLE_ARGB ( D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RGB )

/***************************************************************************************************
*
*	CLASS			Renderer
*
*	DESCRIPTION		The renderer.
*
***************************************************************************************************/

//! The renderer for all game objects.
class RendererPlatform
{
public:
	friend class Renderer;

	Renderer* m_pThis;

	//! Sets the current vertex declaration.
	void SetVertexDeclaration( const CVertexDeclaration& pDecl );

	// Constructs vertex and index buffers.
	//! allocates ram for static vertex data
	VBHandle CreateStaticVertexBuffer(int iSize) const;
	//! allocates ram for static index data
	IBHandle CreateStaticIndexBuffer(int iSize) const;
	//! allocates ram for dynamic vertex data
	VBHandle CreateDynamicVertexBuffer(int iSize) const;
	//! allocates ram for dynamic index data
	IBHandle CreateDynamicIndexBuffer(int iSize) const;
	//! allocates ram for system vertex data (not renderable directly)
	VBHandle CreateSystemVertexBuffer(int iSize) const;
	//! allocates ram for system index data (not renderable directly)
	IBHandle CreateSystemIndexBuffer(int iSize) const;

	//! Constructs vertex and pixel shaders from exported data.
	CComPtr<IDirect3DVertexShader9> CreateVertexShader(const uint32_t* pdwFunction) const;
	CComPtr<IDirect3DPixelShader9> CreatePixelShader(const uint32_t* pdwFunction) const;
	
	//! Debug stuff for compiling shaders on the fly.
	CComPtr<IDirect3DVertexShader9> AssembleVertexShader(const char* pcSource) const;
	CComPtr<IDirect3DPixelShader9> AssemblePixelShader(const char* pcSource) const;
	CComPtr<IDirect3DVertexShader9> CompileVertexShader(const char* pcSource, const char* pcFuncName, const char* pcProfile ) const;
	CComPtr<IDirect3DPixelShader9> CompilePixelShader(const char* pcSource, const char* pcFuncName, const char* pcProfile ) const;

	//! Sets the texture on a given sampler stage.
	void SetTexture( int iStage, IDirect3DBaseTexture9* pobTexture, bool bForce );

	//! Gets the amount of RAM currently being managed by the Direct3D graphics device (0.0f to 1.0f range)
	float GetMemoryUsage() const;

	void EvictManagedResources() const;

	//! Wrappers for D3D scene delimiters.
	void BeginScene();
	void EndScene();

	//! Presents the device.
	void PostPresent();

protected:
	//! True if we're in a scene.
	bool InScene() const { return m_bInScene; }

private:
	void DebugClearOverlay();

	//! Resets the texture stage state cache.
	void ResetTextureStates();

	bool m_bInScene;							//!< True if we're in a scene.
	
	// Platform specific texture and sampler state cache
	static const u_int	MAX_SAMPLERS = 16;

	IDirect3DBaseTexture9* m_apobTextures[MAX_SAMPLERS];	//!< The currently set textures.
	int m_aiTextureAddressModes[MAX_SAMPLERS];				//!< The current sampler address modes.
	int m_aiTextureFilterModes[MAX_SAMPLERS];				//!< The current sampler filter modes.
};

#endif // ndef _RENDERER_H
