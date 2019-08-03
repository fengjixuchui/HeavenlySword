//--------------------------------------------------
//!
//!	\file renderer.h
//!	The main renderer class
//!
//--------------------------------------------------

#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#ifndef GFX_RENDERSTATES_H
#include "gfx/renderstates.h"
#endif

#ifndef GFX_RENDERERSETTINGS_H
#include "gfx/renderersettings.h"
#endif

#ifndef GFX_TARGET_CACHE_H
#include "gfx/targetcache.h"
#endif

#if defined( PLATFORM_PC )
#include "gfx/renderer_pc.h"
#elif defined( PLATFORM_PS3 )
#include "gfx/renderer_ps3.h"
#endif

#ifndef CORE_PROFILING_H
#include "core/profiling.h"
#endif

struct	ScissorRegion;
class	PIPManager;
class	Shader;

//--------------------------------------------------
//!
//! Renderer
//!
//--------------------------------------------------
class Renderer : public Singleton<Renderer>
{
public:
	friend class RendererPlatform;

	//! Creates the renderer.
	Renderer();

	//! Destroys the renderer.
	~Renderer();

	//! public member allows users to set and query current render targets
	TargetCache m_targetCache;

	//! method that retrieves the device back buffer. Treat with care!
	static RenderTarget::Ptr GetHardwareBackBuffer();

	//! Clears the current viewport.
	void Clear( uint32_t dwFlags, uint32_t dwColour, float fZValue, uint32_t dwStencil );

#ifdef PLATFORM_PS3
	void FastFloatClear( uint32_t dwFlags, float fZValue, uint32_t dwStencil );
#endif
	
	//! Sets the texture on a given sampler stage. Heresy once again breaks the caching
	void SetTexture( int iStage, Texture::Ptr const& pobTexture, bool bForce = false  );

	//! Resets the texture
	void SetTexture( int iStage, Texture::NONE_ENUM );

	//! Sets the sampler address mode for a given stage.
	void SetSamplerAddressMode(int iStage, int iAddressMode);

	//! Sets the sampler filter mode for a given stage.
	void SetSamplerFilterMode(int iStage, int iFilterMode);


#ifdef PLATFORM_PS3
	//! Sets the sampler anisotropic filter to the level specified in renderersettings
	void EnableSamplerAnisotropicFilter( int iStage );
	void SetAnisotropicFilterLevel( int iStage, int iLevel );
#endif

	//! Sets combinations of render states
	void SetDefaultRenderStates();

	//! Sets alpha blending
	void SetBlendMode( GFX_BLENDMODE_TYPE eMode );
	//! Gets current alpha blending setting
	GFX_BLENDMODE_TYPE GetBlendMode() { return m_cachedBlendMode; }
	
	//! Access Zbuffer mode
	void SetZBufferMode( GFX_ZMODE_TYPE eMode );
	GFX_ZMODE_TYPE GetZBufferMode() { return m_cachedZMode; }

	//! Sets culling mode
	void SetCullMode( GFX_CULLMODE_TYPE eMode );

	//! Sets alpha testing mode
	void SetAlphaTestModeN( GFX_ALPHA_TEST_MODE eMode, float fRef );		// Normalised ref 0.0f -> 1.0f
	void SetAlphaTestMode( GFX_ALPHA_TEST_MODE eMode, int iRef = 0 );		// Integer ref 0 -> 255

	//! Sets fill mode
	void SetFillMode( GFX_FILL_MODE eMode );

	//! Sets the point sprite render state
	void SetPointSpriteEnable( bool bEnabled );

	//! Sets the current scissor Region
	void SetScissorRegion( const ScissorRegion* pRegion );

	// shader accessors

	//! NOTE! when using heresy, our caching is broken, hence the force
	// methods on set Pixel shader and set Vertex shader
	void SetVertexShader( Shader* pShader, bool bForce = false );
	void SetPixelShader( Shader* pShader, bool bForce = false );

	Shader* GetVertexShader()	{ return m_pCachedVertexShader; }
	Shader* GetPixelShader()	{ return m_pCachedPixelShader; }

#ifdef PLATFORM_PC // these dont exist on PS3
	//! Sets a vertex shader constant.
	void SetVertexShaderConstant( int iRegister, const void* pvValue, int iNumRegisters );
	void SetVertexShaderConstant( int iRegister, CVectorBase const& obValue )						{ SetVertexShaderConstant( iRegister, &obValue, 1 ); }
	void SetVertexShaderConstant( int iRegister, CMatrix const& obValue, int iNumRegisters = 4 )	{ SetVertexShaderConstant( iRegister, &obValue, iNumRegisters ); }

	//! Sets a pixel shader constant.
	void SetPixelShaderConstant( int iRegister, const void* pvValue, int iNumRegisters );
	void SetPixelShaderConstant( int iRegister, CVectorBase const& obValue )						{ SetPixelShaderConstant( iRegister, &obValue, 1 ); }
	void SetPixelShaderConstant( int iRegister, CMatrix const& obValue, int iNumRegisters = 4 )		{ SetPixelShaderConstant( iRegister, &obValue, iNumRegisters ); }
#endif

	RendererPlatform m_Platform;

	// !Get the texture format we can use for HDR rendring
	static GFXFORMAT GetHDRFormat( void );

	//! Enable some basic tracking our our resources
#ifdef TRACK_GFX_MEM
	static uint32_t	ms_iVBAllocs;
	static uint32_t	ms_iVBVertCount;
	static uint32_t	ms_iIBAllocs;
	static uint32_t	ms_iDiskTex;
	static uint32_t	ms_iProcTex;
	static uint32_t	ms_iRTAllocs;
#endif

	//! some debug rendering code.
	void DebugRender( float fStartX, float fStartY, float fSpacing );
	void DumpStats();

	//! the picture in picture manager
	PIPManager*			m_pPIPManager;

#ifdef _PROFILING
	RenderCounter		m_renderCount;
	RenderCounter		m_lastCount;
#endif

	//! Request a change in presentmode
	enum PRESENT_MODE
	{
		PM_IMMEDIATE,	// equivalent to HSYNC
		PM_VBLANK,		// wait till next vertical blank
		PM_AUTO,		// game quicker than 30 Hz, VYSYNC else HSYNC
	};

	void RequestPresentMode( PRESENT_MODE mode );
	void Present();

	// time in seconds it took to process last push buffer
	float GetGPUFrameTime() const { return m_fGPUFrameTime; }

	// time in seconds outiside of blocking present call
	float GetCPUFrameTime() const { return m_fCPUFrameTime; }

private:

#ifdef PLATFORM_PS3
	PRESENT_MODE	m_presentMode;
	PRESENT_MODE	m_reqPresentMode;
	PRESENT_MODE	m_autoPresentMode;
	
	int64_t			m_lGPUFrameTimer;
	int64_t			m_lCPUFrameTimer;

	static void		VBlankCallback( uint32_t );
	static void		InteruptCallback( uint32_t );
#endif

	float			m_fGPUFrameTime;
	float			m_fCPUFrameTime;

	GFX_CULLMODE_TYPE	m_eStandardCullMode;	//!< The standard cull mode, defaults to CCW.

	//! Internal cache for lazy render state setting
	GFX_ZMODE_TYPE		m_cachedZMode;

	//! Internal cache for blending mode setting
	GFX_BLENDMODE_TYPE	m_cachedBlendMode;
	
	//! Internal cache for blending mode setting (only used on PS3 to workaround the FP Clear bug)
	GFX_CULLMODE_TYPE	m_cachedCullMode;

	//! Internal cache for current pixel and vertex shaders
	Shader*				m_pCachedVertexShader;
	Shader*				m_pCachedPixelShader;


};

#ifdef TRACK_GFX_MEM

#define TRACK_GFX_ALLOC_VB( size )			{ Renderer::ms_iVBAllocs += size; }
#define TRACK_GFX_ADD_VBSTAT( count )		{ Renderer::ms_iVBVertCount += count; }
#define TRACK_GFX_ALLOC_IB( size )			{ Renderer::ms_iIBAllocs += size; }
#define TRACK_GFX_ALLOC_DISK_TEX( tex )		{ Renderer::ms_iDiskTex += tex->m_iDiskSize; }
#define TRACK_GFX_ALLOC_PROC_TEX( tex )		{ Renderer::ms_iProcTex += tex->CalculateVRAMFootprint(); tex->m_iDiskSize = 0; }
#define TRACK_GFX_ALLOC_RT( rt )			{ Renderer::ms_iRTAllocs += rt->CalculateVRAMFootprint(); }

#define TRACK_GFX_FREE_VB( size )			{ Renderer::ms_iVBAllocs -= size; }
#define TRACK_GFX_FREE_VBSTAT( count )		{ Renderer::ms_iVBVertCount -= count; }
#define TRACK_GFX_FREE_IB( size )			{ Renderer::ms_iIBAllocs -= size; }
#define TRACK_GFX_FREE_DISK_TEX( tex )		{ Renderer::ms_iDiskTex -= tex->m_iDiskSize; }
#define TRACK_GFX_FREE_PROC_TEX( tex )		{ Renderer::ms_iProcTex -= tex->CalculateVRAMFootprint(); tex->m_iDiskSize = 0; }
#define TRACK_GFX_FREE_RT( rt )				{ Renderer::ms_iRTAllocs -= rt->CalculateVRAMFootprint(); }

#else

#define TRACK_GFX_ALLOC_VB( x )
#define TRACK_GFX_ADD_VBSTAT( x )
#define TRACK_GFX_ALLOC_IB( x )
#define TRACK_GFX_ALLOC_DISK_TEX( x )
#define TRACK_GFX_ALLOC_PROC_TEX( x )
#define TRACK_GFX_ALLOC_RT( x )

#define TRACK_GFX_FREE_VB( x )
#define TRACK_GFX_FREE_VBSTAT( x )
#define TRACK_GFX_FREE_IB( x )
#define TRACK_GFX_FREE_DISK_TEX( x )
#define TRACK_GFX_FREE_PROC_TEX( x )
#define TRACK_GFX_FREE_RT( x )

#endif

#endif // end GFX_RENDERER_H
