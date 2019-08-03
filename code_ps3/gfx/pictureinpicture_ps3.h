//--------------------------------------------------
//!
//!	\file pictureinpicture_ps3.h
//!	objects managing final back buffer compositing
//! of multiple viewports.
//!
//--------------------------------------------------

#ifndef GFX_PIP_PS3_H
#define GFX_PIP_PS3_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

#ifndef GFX_TEXTURE_H
#include "gfx/texture.h"
#endif

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

#ifndef GFX_TARGET_CACHE_H
#include "gfx/targetcache.h"
#endif

#ifndef GFX_SHADER_H
#include "gfx/shader.h"
#endif

#ifndef GFX_DEPTHOFFIELD_H
#include "gfx/depthoffield.h"
#endif

#ifndef CORE_PROFILING_H
#include "core/profiling.h"
#endif

template<typename T, int C> class SortableList;
class Display;
class CCamera;

//-----------------------------------------------------
//!
//! PIPView
//! Sumarises information associated with a viewport
//!
//-----------------------------------------------------
class PIPView
{
public:
	PIPView();

	void SetActive( bool bActive );
	bool GetActive() const { return m_bActive; }

	void SetCamera( const CCamera* pCamera ) { m_pCamera = pCamera; }
	const CCamera* GetCamera() const { return m_pCamera; }

	// rendering priority (lower the number, earlier the render)
	void SetDebugID( int iID ) { m_iDebugID = iID; }
	int GetDebugID() const { return m_iDebugID; }

	// rendering priority (lower the number, earlier the render)
	void SetViewPriority( int iPriority ) { m_iPriority = iPriority; }
	int GetViewPriority() const { return m_iPriority; }

	// member used to make us compatable with SortableList
	float GetSortingValue() const { return _R(m_iPriority); }

	// these dimensions / positions are normalised
	void SetViewDim( float width, float height ) { m_fWidth = width; m_fHeight = height; }
	void GetViewDim( float& width, float& height ) const { width = m_fWidth; height = m_fHeight; }
	
	// this is the top left hand position of the view. 0, 0 is top left.
	void SetViewPos( float x, float y ) { m_fX = x; m_fY = y; }
	void GetViewPos( float& x, float& y ) const { x = m_fX; y = m_fY; }

	// alloc and free VRAM
	void SetupView( uint32_t iBBWidth, uint32_t iBBHeight );
	void FreeView();

	// get view render result
	Texture::Ptr GetTexture() const;

	// control fade colour of this view
	void SetFadeColour( uint32_t iCol )		{ m_iFadeCol = iCol; }
	uint32_t GetFadeColour() { return m_iFadeCol; }

	// control fade colour of this view
	void SetFadeFraction( float fFraction )	{ m_fFadeFraction = fFraction; }
	float GetFadeFraction()	{ return m_fFadeFraction; }

	// access the views vertex data
	VBHandle& GetVB() { return m_hFinalQuad; } 

	// access the views collection of buffers
	RenderTarget::Ptr	GetMSAAColour()		{ return m_pHDRBackBuffer; }
	ZBuffer::Ptr		GetMSAADepth()		{ return m_pHDRZBuffer; }

	RenderTarget::Ptr	GetBackBuffer()		{ return m_pBackBuffer; }
	ZBuffer::Ptr		GetZBuffer()		{ return m_pZBuffer; }
	RenderTarget::Ptr	GetFloatZBuffer()	{ return m_pFloatZBuffer; }
	RenderTarget::Ptr	GetFloatColour()	{ return m_pFloatColour; }
	Texture::Ptr		GetIrradianceCache(){ return m_pIrradianceCache; }
	RenderTarget::Ptr*	GetIrradianceCacheFaces()	{ return m_pIrradianceCacheFace; }

	// local storage for various rendering techniques that
	// have persistent temporal data, such as exposure or DOF
	Texture::Ptr GetKeyLuminance() { return m_exposureOveridden ? m_exposureOveride : m_exposureLastVal->GetTexture(); }

	bool				m_exposureOveridden;
	Texture::Ptr		m_exposureOveride;
	RenderTarget::Ptr	m_exposureLastVal;
	RenderTarget::Ptr	m_exposureLastValTemp;
	void*				m_pExposureXDRTextures;

	DepthOfFieldSettings	m_DOFSettings;

#ifdef _PROFILING
	void ResetCounters();
	void FinaliseCounters();
	void PrintCounters(float fStartX, float fStartY);

	RenderCounter		m_startCount;
	RenderCounter		m_SMapCount;
	RenderCounter		m_ZPrepassCount;
	RenderCounter		m_RecieveShadowCount;
	RenderCounter		m_RenderOpaqueCount;
	RenderCounter		m_RenderAlphaCount;
	RenderCounter		m_MiscCount;
	RenderCounter		m_TotalCount;
#else
	void ResetCounters(){}
	void FinaliseCounters(){};
	void PrintCounters(float, float){};
#endif

private:
	const CCamera* m_pCamera;
	bool m_bActive;
	bool m_bAllocated;
	int	m_iPriority;
	int m_iDebugID;

	float m_fWidth, m_fHeight;
	float m_fX, m_fY;

	uint32_t	m_iFadeCol;
	float		m_fFadeFraction;

	RenderTarget::Ptr	m_pBackBuffer;
	ZBuffer::Ptr		m_pZBuffer;
	RenderTarget::Ptr	m_pFloatZBuffer;
	RenderTarget::Ptr	m_pFloatColour;

	// these buffers are multisample antialised
	RenderTarget::Ptr	m_pHDRBackBuffer;
	ZBuffer::Ptr		m_pHDRZBuffer;
	Texture::Ptr		m_pIrradianceCache;
	RenderTarget::Ptr	m_pIrradianceCacheFace[6];

	VBHandle	m_hFinalQuad;
};

//-----------------------------------------------------
//!
//! PIPManager
//! Public object embedded within the renderer class
//! that manages multiple view ports and their compositing
//! into the final frame buffer.
//! Note: takes the place of the old back buffer system
//!
//-----------------------------------------------------
class PIPManager
{
public:
	static const uint32_t MAX_VIEWS = 10;
	static const uint32_t IRRAD_CACHE_SIZE = 64;

	PIPManager();
	~PIPManager();

	PIPView&	GetView(uint32_t iIndex)
	{
		ntAssert( iIndex < MAX_VIEWS );
		return m_views[iIndex];
	}

	void GetValidViews(SortableList<PIPView, Mem::MC_MISC>& views);

	void	RenderBasic();
	void	PresentBasic();

	void	RenderLevel();
	void	PresentLevel();

	struct ViewVertex
	{
		ViewVertex(){};
		ViewVertex( float X, float Y, float U, float V ) { x = X; y = Y; u = U; v = V; };
		float x,y;
		float u,v;
	};

	static void CalcViewspaceVerts( float fLeft, float fTop, float fRight, float fBottom,
									ViewVertex* pVerts );

	void	SetCompositeClearCol( uint32_t iCol ) { m_iCompositeClearCol = iCol; }
	void	SetFadeColour( uint32_t iCol )		{ m_iFadeCol = iCol; }
	void	SetFadeFraction( float fFraction )	{ m_fFadeFraction = fFraction; }

	PIPView&	GetCurrentView()
	{
		ntError_p( m_bViewsRendering, ("Must be within RenderLevel() for this call to be valid") );
		ntAssert_p( m_pCurrentView, ("Current view pointer invalid") );
		return *m_pCurrentView;
	};

	enum BUFFER_TYPE
	{
		BACKBUFFER_LDR = 0,
		DEPTHBUFFER_LDR,
		BACKBUFFER_FLOAT,
		DEPTHBUFFER_FLOAT,
		BACKBUFFER_HDR,
		DEPTHBUFFER_HDR,
		IRRADCACHE_HDR,
	};

	RenderTarget::Ptr GetAndLockSharedRB(
		BUFFER_TYPE eType,
		int32_t width, uint32_t height,
		GFXFORMAT eformat, GFXAAMODE aamode = GAA_MULTISAMPLE_NONE );

	Texture::Ptr GetAndLockSharedTexture(
		BUFFER_TYPE eType,
		int32_t width, uint32_t height,
		GFXFORMAT eformat, GFXAAMODE aamode = GAA_MULTISAMPLE_NONE );

	void ReleaseSharedRB( BUFFER_TYPE eType );
	void ReleaseSharedTexture( BUFFER_TYPE eType );

	uint32_t	GetBBWidth() const { return m_iBBWidth; }
	uint32_t	GetBBHeight() const { return m_iBBHeight; }
	uint32_t	GetIrradCacheSize() const { return IRRAD_CACHE_SIZE; }

	static void DisplayFullscreenTexture( Texture::Ptr pTexture );

private:
	PIPView					m_views[MAX_VIEWS];
	bool					m_bViewsRendering;
	PIPView*				m_pCurrentView;

	uint32_t				m_iBBWidth;
	uint32_t				m_iBBHeight;

	DebugShader	*			m_compositeVS;
	DebugShader	*			m_compositePS;

	uint32_t				m_iCompositeClearCol;
	uint32_t				m_iFadeCol;
	float					m_fFadeFraction;

	// shared render targets go here.
	uint32_t				m_iAllocFlags;
	
	RenderTarget::Ptr		m_pBackBuffer;
	ZBuffer::Ptr			m_pZBuffer;
	RenderTarget::Ptr		m_pFloatZBuffer;
	RenderTarget::Ptr		m_pFloatColour;
	RenderTarget::Ptr		m_pHDRBackBuffer;
	ZBuffer::Ptr			m_pHDRZBuffer;
	Texture::Ptr			m_pIrradianceCache;
};

#endif // GFX_PIP_PS3_H
