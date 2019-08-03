//--------------------------------------------------
//!
//!	\file pictureinpicture_pc.h
//!	objects managing final back buffer compositing
//! of multiple viewports.
//!
//--------------------------------------------------

#ifndef GFX_PIP_PC_H
#define GFX_PIP_PC_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

#ifndef GFX_TARGET_CACHE_H
#include "gfx/targetcache.h"
#endif

#ifndef GFX_DEPTHOFFIELD_H
#include "gfx/depthoffield.h"
#endif

template<typename T, int C> class SortableList;
class Shader;
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
	PIPView() :
		m_pCamera(0),
		m_bActive(false),
		m_bAllocated(false),
		m_iPriority(0),
		m_fWidth(1), m_fHeight(1),
		m_fX(0), m_fY(0),
		m_iFadeCol( NTCOLOUR_ARGB(0xff,0,0,0) ),
		m_fFadeFraction(0.0f)
	{}

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

	// local storage for various rendering techniques that
	// have persistent temporal data, such as exposure or DOF
	Texture::Ptr GetKeyLuminance() { return m_exposureLastVal->GetTexture(); }

	float					m_fExposureLastVal;
	RenderTarget::Ptr		m_exposureLastVal;
	RenderTarget::Ptr		m_exposureLastValTemp;

	DepthOfFieldSettings	m_DOFSettings;

private:
	const CCamera* m_pCamera;
	bool m_bActive;
	bool m_bAllocated;
	int	m_iPriority;
	int	m_iDebugID;

	float m_fWidth, m_fHeight;
	float m_fX, m_fY;

	uint32_t	m_iFadeCol;
	float		m_fFadeFraction;

	RenderTarget::Ptr	m_pBackBuffer;
	ZBuffer::Ptr		m_pZBuffer;
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
	static const int MAX_VIEWS = 10;

	PIPManager(uint32_t iBBWidth, uint32_t iBBHeight);
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
	void	SetBorderClearCol( uint32_t iCol )	{ m_iBorderClearCol = iCol; }
	void	SetFadeColour( uint32_t iCol )		{ m_iFadeCol = iCol; }
	void	SetFadeFraction( float fFraction )	{ m_fFadeFraction = fFraction; }

	PIPView&	GetCurrentView()
	{
		ntError_p( m_bViewsRendering, ("Must be within RenderLevel() for this call to be valid") );
		ntAssert_p( m_pCurrentView, ("Current view pointer invalid") );
		return *m_pCurrentView;
	};

	uint32_t	GetBBWidth() const { return m_iBBWidth; }
	uint32_t	GetBBHeight() const { return m_iBBHeight; }

	static void DisplayFullscreenTexture( Texture::Ptr pTexture );

private:
	PIPView					m_views[MAX_VIEWS];
	bool					m_bViewsRendering;
	PIPView*				m_pCurrentView;

	uint32_t				m_iBBWidth;
	uint32_t				m_iBBHeight;

	// this is used only in fullscreen mode on PC
	RenderTarget::Ptr		m_pFakeBackBuffer;
	
	RenderTarget::Ptr		m_pSimpleBackBuffer;
	ZBuffer::Ptr			m_pSimpleZBuffer;

	Shader*					m_compositeVS;
	Shader*					m_compositePS;
	CVertexDeclaration		m_compositeDecl;

	uint32_t				m_iCompositeClearCol;
	uint32_t				m_iBorderClearCol;
	uint32_t				m_iFadeCol;
	float					m_fFadeFraction;
};

#endif // GFX_PIP_PC_H
