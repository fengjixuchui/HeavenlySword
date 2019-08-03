//--------------------------------------------------
//!
//!	\file pictureinpicture.cpp
//!	objects managing final back buffer compositing
//! of multiple viewports.
//!
//--------------------------------------------------

#include "game/capturesystem.h"
#include "gfx/pictureinpicture.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/surfacemanager.h"
#include "gfx/sector.h"
#include "gfx/display.h"
#include "gfx/graphicsdevice.h"
#include "gfx/lightingtemplates.h"
#include "gfx/exposure.h"
#include "core/visualdebugger.h"
#include "core/profiling.h"
#include "core/gatso.h"
#include "input/inputhardware.h"
#include "effect/screensprite.h"


//-----------------------------------------------------
//!
//! PIPView::SetupView
//! Allocate VRAM and set as render targets
//!
//-----------------------------------------------------
void PIPView::SetupView(uint32_t iBBWidth, uint32_t iBBHeight)
{
	ntAssert_p( !m_bAllocated, ("PIPView already setup.") );
	m_bAllocated = true;

	uint32_t iWidth = (uint32_t)(m_fWidth * iBBWidth);
	uint32_t iHeight = (uint32_t)(m_fHeight * iBBHeight);

	// if we're virtually 1, make sure we can use cachable 
	// render targets
	if (fabs((m_fWidth - 1.0f)) < EPSILON)
		iWidth = iBBWidth;

	if (fabs((m_fHeight - 1.0f)) < EPSILON)
		iHeight = iBBHeight;

	bool bCacheable = ((iWidth == iBBWidth) && (iHeight == iBBHeight)) ? true : false;

	if (CRendererSettings::GetAAMode() == AAM_SUPERSAMPLE_4X)
	{
		iWidth = iWidth<<1;
		iHeight = iHeight<<1;
	}

	// create back buffer
	RenderTarget::CreationStruct createParamsBB( iWidth, iHeight, D3DFMT_A8R8G8B8, false );
	createParamsBB.bCacheable = bCacheable;
	m_pBackBuffer = SurfaceManager::Get().CreateRenderTarget( createParamsBB );

	// create Z buffer
	Surface::CreationStruct	createParamsZ( iWidth, iHeight, D3DFMT_D24S8 );
	createParamsZ.bCacheable = bCacheable;
	m_pZBuffer = SurfaceManager::Get().CreateSurface( createParamsZ );

	Renderer::Get().m_targetCache.SetColourAndDepthTargets( m_pBackBuffer, m_pZBuffer );
}

//-----------------------------------------------------
//!
//! PIPView::SetActive
//! Do any other persistant allocations or init we require
//!
//-----------------------------------------------------
void PIPView::SetActive( bool bActive )
{
	if (m_bActive == bActive)
		return;

	if (m_bActive)
	{
		// we're decativating, free stuff
		m_bActive = false;

		SurfaceManager::Get().ReleaseRenderTarget(m_exposureLastVal);
		SurfaceManager::Get().ReleaseRenderTarget(m_exposureLastValTemp);
	}
	else
	{
		// we're activating for the first time, alloc stuff
		m_bActive = true;

		m_fExposureLastVal = 1.0f;
		m_exposureLastVal = SurfaceManager::Get().CreateRenderTarget( 1, 1, EXPController::GetInternalFormat() );
		m_exposureLastValTemp = SurfaceManager::Get().CreateRenderTarget( 1, 1, EXPController::GetInternalFormat() );
	}
}

//-----------------------------------------------------
//!
//! PIPView::FreeView
//! Free VRAM used by view
//!
//-----------------------------------------------------
void PIPView::FreeView()
{
	ntAssert_p( m_bAllocated, ("PIPView not setup.") );
	m_bAllocated = false;

	// we never cache these, as we dont know if theyre going to be fixed
	// or variable size.
	SurfaceManager::Get().ReleaseRenderTarget( m_pBackBuffer );
	SurfaceManager::Get().ReleaseSurface( m_pZBuffer );
}

//--------------------------------------------------
//!
//! PIPView::GetTexture
//! Retrieve our view render target as a texture
//!
//--------------------------------------------------
Texture::Ptr PIPView::GetTexture() const
{
	ntAssert_p( m_bAllocated, ("PIPView not setup.") );
	return m_pBackBuffer->GetTexture();
}




//-----------------------------------------------------
//!
//! PIPManager::ctor
//! Allocate shaders and VRAM
//!
//-----------------------------------------------------
PIPManager::PIPManager(uint32_t iWidth, uint32_t iHeight)
{
	for (int i = 0; i < MAX_VIEWS; i++)
		m_views[i].SetDebugID(i);

	m_bViewsRendering		= false;
	m_pCurrentView			= 0;
	m_iBorderClearCol		= NTCOLOUR_ARGB(0xff,0,0,0);
	m_iCompositeClearCol	= NTCOLOUR_ARGB(0xff,0x10,0x10,0x10);
	m_iFadeCol				= m_iCompositeClearCol;
	m_fFadeFraction			= 0.f;

	// create our compositing shaders
	DebugShader* pNewVS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewVS->SetHLSLFunction(
		SHADERTYPE_VERTEX,

		"void composite_vs(	float4 position : POSITION0, "
		"						float2 texcoord : TEXCOORD0, "
		"						out float4 outpos : POSITION, "
		"						out float2 outtex : TEXCOORD0, "
		"						uniform float2 global_texeloffset : register(c0) )"
		"{ "
		"	outpos = position; "
		"	outpos.xy += global_texeloffset*outpos.ww; "
		"	outtex = texcoord; "
		"}",
		"composite_vs",
		"vs.1.1"
	);
	m_compositeVS = pNewVS;

	DebugShader* pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewPS->SetHLSLFunction(
		SHADERTYPE_PIXEL,

		"float4 composite_ps(	float2 texcoord : TEXCOORD0, "
		"						uniform sampler2D source : register(s0), "
		"						uniform float4 fade_colour : register(c0), "
		"						uniform float fade_fraction : register(c1) ) : COLOR0 "
		"{ "
		"	return lerp( tex2D( source, texcoord ).xyzw, fade_colour, fade_fraction ); "
		"}",
		"composite_ps",
		"ps.1.1"
	);
	m_compositePS = pNewPS;

	// create our compositing vertex declaration
	D3DVERTEXELEMENT9 decl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 2*sizeof(float),	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		D3DDECL_END()
	};
	m_compositeDecl = CVertexDeclarationManager::Get().GetDeclaration( &decl[0] );

	m_iBBWidth = iWidth;
	m_iBBHeight = iHeight;

	m_views[0].SetActive(true);
}

//-----------------------------------------------------
//!
//! PIPManager::dtor
//! Free shaders and VRAM
//!
//-----------------------------------------------------
PIPManager::~PIPManager()
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_compositeVS );
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_compositePS );
}

void PIPManager::GetValidViews(SortableList<PIPView>& views)
{
	// now sort our views in priority order
	for (int i = 0; i < MAX_VIEWS; i++)
	{
		if (m_views[i].GetActive())
		{
			ntAssert_p( m_views[i].GetCamera(), ("Active view with no camera") );
			views.m_list.push_back( &m_views[i] );
		}
	}
}

//-----------------------------------------------------
//!
//! PIPManager::RenderBasic
//! Sets up simple back buffers and clears them
//!
//-----------------------------------------------------
void	PIPManager::RenderBasic()
{
	Renderer::Get().m_Platform.BeginScene();

	RenderTarget::CreationStruct createParamsBB( m_iBBWidth, m_iBBHeight, D3DFMT_A8R8G8B8, false );
	createParamsBB.bCacheable = true;
	m_pSimpleBackBuffer = SurfaceManager::Get().CreateRenderTarget( createParamsBB );

	Surface::CreationStruct	createParamsZ( m_iBBWidth, m_iBBHeight, D3DFMT_D24S8 );
	createParamsZ.bCacheable = true;
	m_pSimpleZBuffer = SurfaceManager::Get().CreateSurface( createParamsZ );

	Renderer::Get().m_targetCache.SetColourAndDepthTargets( m_pSimpleBackBuffer, m_pSimpleZBuffer );
	Renderer::Get().Clear( D3DCLEAR_TARGET, 0, 1.0f, 0 );
}

//-----------------------------------------------------
//!
//!	PIPManager::PresentBasic
//! We have finished drawing, so swap buffers
//!
//-----------------------------------------------------
void PIPManager::PresentBasic()
{
	// flush any cached debug primitives or text
	CGatso::Start( "PIPManager::DebugDraw2D" );
	g_VisualDebug->Draw2D();
	g_VisualDebug->Reset();
	CGatso::Stop( "PIPManager::DebugDraw2D" );

	Renderer::Get().m_targetCache.SetColourTarget( Renderer::GetHardwareBackBuffer() );

	// need to clear hardwared back buffer if we're bordering
	if ( DisplayManager::Get().IsFullScreen() )
		Renderer::Get().Clear( D3DCLEAR_TARGET, m_iBorderClearCol, 1.0f, 0 );

	DisplayFullscreenTexture( m_pSimpleBackBuffer->GetTexture() );

	SurfaceManager::Get().ReleaseRenderTarget( m_pSimpleBackBuffer );
	SurfaceManager::Get().ReleaseSurface( m_pSimpleZBuffer );

	Renderer::Get().m_Platform.EndScene();

	// dump final resolved LDR buffer
	if	(
		(CInputHardware::Exists()) &&
		(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_INSERT ))
		)
		Renderer::Get().m_targetCache.GetPrimaryColourTarget()->SaveToDisk( "X_FinalBuffer" );

	// swap the surfaces
	// NB, in window mode this is a blocking call, so we pause our profile

	// swap the surfaces
	Renderer::Get().Present();
}

//-----------------------------------------------------
//!
//! PIPManager::RenderLevel
//! Renders all the views currently required by the level
//!
//-----------------------------------------------------
void	PIPManager::RenderLevel()
{
	Renderer::Get().m_Platform.BeginScene();

	// right, first we're going clear our real back buffer.
	RenderTarget::Ptr pCompositingBuffer;

	// we have to fake up a back buffer in full screen pc mode
	if (DisplayManager::Get().IsFullScreen())
	{
		m_pFakeBackBuffer = SurfaceManager::Get().CreateRenderTarget( 
			RenderTarget::CreationStruct( m_iBBWidth, m_iBBHeight, D3DFMT_X8R8G8B8, false ) );

		pCompositingBuffer = m_pFakeBackBuffer;
	}
	else
	{
		pCompositingBuffer = Renderer::GetHardwareBackBuffer();
	}

	// now we clear it and loop over our views, rendering them one by one
	Renderer::Get().m_targetCache.SetColourTarget( pCompositingBuffer );
	Renderer::Get().Clear( D3DCLEAR_TARGET, m_iCompositeClearCol, 1.0f, 0 );

	// now sort our views in priority order
	SortableList<PIPView>	views;
	GetValidViews(views);
	//for (int i = 0; i < MAX_VIEWS; i++)
	//{
	//	if (m_views[i].GetActive())
	//	{
	//		ntAssert_p( m_views[i].GetCamera(), ("Active view with no camera") );
	//		views.m_list.push_back( &m_views[i] );
	//	}
	//}

	ntAssert_p( !views.m_list.empty(), ("Must have atleast one view here") );
	views.SortListAscending();
	
	// now render them
	m_bViewsRendering = true;
	for (	ntstd::List<PIPView*>::iterator it = views.m_list.begin();
			it != views.m_list.end(); ++it )
	{
		m_pCurrentView = (*it);

		// allocate views render targets
		m_pCurrentView->SetupView( m_iBBWidth, m_iBBHeight );

		// render the sector
		CSector::Get().Render( m_pCurrentView->GetCamera() );

		// swap to compositing buffer and draw view
		Renderer::Get().m_targetCache.SetColourTarget( pCompositingBuffer );

		// set render states
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

		// set the shaders
		Renderer::Get().SetVertexShader( m_compositeVS );
		Renderer::Get().SetPixelShader( m_compositePS );
		Renderer::Get().m_Platform.SetVertexDeclaration( m_compositeDecl );

		// set PS consts
		float r,g,b,a;
		NTCOLOUR_EXTRACT_FLOATS( m_pCurrentView->GetFadeColour(), r,g,b,a )
		CVector	fadeCol( r,g,b,a );
		CVector	fadeFrac( m_pCurrentView->GetFadeFraction(),0.f,0.f,0.f );

		Renderer::Get().SetPixelShaderConstant( 0, &fadeCol, 1 );
		Renderer::Get().SetPixelShaderConstant( 1, &fadeFrac, 1 );

		// set dimensions
		float fLeft, fTop, fWidth, fHeight;
		m_pCurrentView->GetViewPos( fLeft, fTop );
		m_pCurrentView->GetViewDim( fWidth, fHeight );

		Renderer::Get().SetTexture( 0, m_pCurrentView->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

		ViewVertex	verts[6];
		CalcViewspaceVerts( fLeft, fTop, fLeft + fWidth, fTop + fHeight, verts );
		GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, verts, 4*sizeof(float));

		// free the view's render targets
		m_pCurrentView->FreeView();

		// cleanup
		Renderer::Get().SetTexture( 0, Texture::NONE );
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_NORMAL );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
	}
	m_bViewsRendering = false;
	m_pCurrentView = 0;
}

//-----------------------------------------------------
//!
//! PIPManager::CalcViewspaceVerts
//! Generates two D3D viewspace triangles
//!
//-----------------------------------------------------
void PIPManager::CalcViewspaceVerts(	float fLeft, float fTop, float fRight, float fBottom,
										ViewVertex* pVerts )
{
	pVerts[0] = ViewVertex( 2.0f*fLeft - 1.0f,	1.0f - 2.0f*fTop,		0.0f,	0.0f );
	pVerts[1] = ViewVertex( 2.0f*fRight - 1.0f,	1.0f - 2.0f*fTop,		1.0f,	0.0f );
	pVerts[2] = ViewVertex( 2.0f*fRight - 1.0f,	1.0f - 2.0f*fBottom,	1.0f,	1.0f );

	pVerts[3] = ViewVertex( 2.0f*fRight - 1.0f,	1.0f - 2.0f*fBottom,	1.0f,	1.0f );
	pVerts[4] = ViewVertex( 2.0f*fLeft - 1.0f,	1.0f - 2.0f*fBottom,	0.0f,	1.0f );
	pVerts[5] = ViewVertex( 2.0f*fLeft - 1.0f,	1.0f - 2.0f*fTop,		0.0f,	0.0f );
}

//-----------------------------------------------------
//!
//!	PIPManager::DisplayFullscreenTexture
//! Displays a texture that is assumed to have square pixels
//! (thats important!) in a manner that fits our output
//! display mode. Used for placing internal backbuffer
//! onto the Gc back buffer, or for loading screens
//!
//-----------------------------------------------------
void PIPManager::DisplayFullscreenTexture( Texture::Ptr pTexture )
{
	ScreenSprite sprite;
	sprite.SetTexture( pTexture );
	sprite.SetPosition( CPoint( DisplayManager::Get().GetDeviceWidth() * 0.5f, DisplayManager::Get().GetDeviceHeight() * 0.5f, 0.0f ) );

	float fTextureAspect = _R(pTexture->GetWidth()) / _R(pTexture->GetHeight());
	if ( fTextureAspect > DisplayManager::Get().GetDeviceAspect() )
	{
		// our display is 'square-er' than our texture (say a 16:9 texture on a 4:3 display)
		// this means we're bordering on top and bottom
		sprite.SetWidth( DisplayManager::Get().GetDeviceWidth() );
		sprite.SetHeight( (DisplayManager::Get().GetDeviceAspect() / fTextureAspect) * DisplayManager::Get().GetDeviceHeight() );
	}
	else
	{
		// our texture is 'square-er' than our display (say a 4:3 texture on a 16:9 display)
		// this means we're bordering on left and right
		sprite.SetWidth( (fTextureAspect / DisplayManager::Get().GetDeviceAspect()) * DisplayManager::Get().GetDeviceWidth() );
		sprite.SetHeight( DisplayManager::Get().GetDeviceHeight() );
	}
			
	sprite.Render();
}

//-----------------------------------------------------
//!
//!	PIPManager::PresentLevel
//! We have finished drawing, so swap buffers
//!
//-----------------------------------------------------
void PIPManager::PresentLevel()
{
	// flush any cached debug primitives or text
	CGatso::Start( "PIPManager::DebugDraw2D" );
	g_VisualDebug->Draw2D();
	g_VisualDebug->Reset();
	CGatso::Stop( "PIPManager::DebugDraw2D" );

	// Update capture manager 
	CaptureSystem::Get().Update(0.0f);

	// on the pc in fullscreen mode we must draw to the real fullscreen buffer
	if ( DisplayManager::Get().IsFullScreen() )
	{
		// now set the real hardware back buffer up and clear it
		Renderer::Get().m_targetCache.SetColourTarget( Renderer::GetHardwareBackBuffer() );
		Renderer::Get().Clear( D3DCLEAR_TARGET, m_iBorderClearCol, 1.0f, 0 );

		DisplayFullscreenTexture( m_pFakeBackBuffer->GetTexture() );

		SurfaceManager::Get().ReleaseRenderTarget( m_pFakeBackBuffer );
	}

	Renderer::Get().m_Platform.EndScene();

	// dump final resolved LDR buffer
	if	(
		(CInputHardware::Exists()) &&
		(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_INSERT ))
		)
		Renderer::Get().m_targetCache.GetPrimaryColourTarget()->SaveToDisk( "X_FinalBuffer" );

	// swap the surfaces
	Renderer::Get().Present();
}
