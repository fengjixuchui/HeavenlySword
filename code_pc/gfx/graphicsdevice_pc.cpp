//--------------------------------------------------
//!
//!	\file gfx/graphicsdevice_pc.cpp
//! This is the PC version of the graphics device
//!
//--------------------------------------------------

#include "gfx/graphicsdevice.h"
#include "gfx/renderer.h"
#include "core/profiling.h"
#include "input/mouse.h"		// attn deano! good or bad?
#include "gfx/hardwarecaps.h"
#include "gfx/dxerror_pc.h"

// turn off to remove the NVPerfHUD hook
//#define USE_NVPERF_HUD

//! Use this define to force the reference device
//#define USE_REFERENCE_DEVICE

//--------------------------------------------------
//!
//!The main window procedure. Doesn't do much except quit when it closes.
//!
//--------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_SETCURSOR:
		if ( GraphicsDevice::Get().m_Platform.GetFullScreen() )
		{
			SetCursor(0);
			return TRUE;
		}
		break;

	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE)
			PostQuitMessage(0);
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}

	if	(
		(MouseInput::Exists()) &&
		(MouseInput::Get().m_Platform.ProcMouseInput(hWnd, uMsg, wParam, lParam))
		)
		return S_OK;

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//--------------------------------------------------
//!
//! PC specific construction of graphics device.
//! shouldn't actually do very much
//!
//--------------------------------------------------
GraphicsDevice::GraphicsDevice()
{
	m_Platform.pThis = this;
	m_Platform.m_pDirect3D = 0;
	m_Platform.m_pDevice = 0;
}

//--------------------------------------------------
//!
//! dtor
//!
//--------------------------------------------------
GraphicsDevice::~GraphicsDevice()
{
	m_Platform.m_pDirect3D->Release();
	m_Platform.m_pDevice->Release();

	if( m_Platform.m_bOwnWindow == true )
	{
		DestroyWindow( (HWND) m_Platform.m_pFocusWindow );
	}

	if (HardwareCapabilities::Exists())
		HardwareCapabilities::Kill();
}

//--------------------------------------------------
//!
//! Initialises the main window and DirectX device.
//!
//--------------------------------------------------
bool GraphicsDevicePlatform::InitFullScreen(	int width, 
												int height, 
												const char * pTitle,
												void* hwnd )
{
	// create the Direct3D object 
	ntAssert_p( !m_pDirect3D, ("Direct3d already initialised") );
	m_pDirect3D = Direct3DCreate9(D3D_SDK_VERSION); 

	ntAssert_p( m_pDirect3D, ("failed to initialise direct3d"));

	pThis->m_iDisplayRefreshRate = 60;
	pThis->m_iScreenX = width;  
	pThis->m_iScreenY = height;

	if( hwnd == 0 )
	{
		m_bOwnWindow = true;
		// register our window class
		const char* pcClassName = pTitle;
		WNDCLASSEX stClass = 
		{ 
			sizeof(WNDCLASSEX), 
			CS_OWNDC, 
			&WindowProc, 
			0,
			0, 
			GetModuleHandle(0), 
			0, 
			LoadCursor(0, IDC_ARROW), 
			(HBRUSH)( ::GetStockObject( BLACK_BRUSH ) ), 
			0, 
			pcClassName, 
			0			
		};	
		RegisterClassEx(&stClass);

		// adjust the window size for a 640x480 canvas
		DWORD dwWindowStyle = WS_CAPTION | WS_SYSMENU;
		
		RECT stRect = { 0, 0, width, height };

		AdjustWindowRect(&stRect, dwWindowStyle, FALSE);

		// create the window
		m_pFocusWindow = (void*) CreateWindowEx(
			0, 
			pcClassName,
			pTitle,
			dwWindowStyle, 
			2, 
			2, 
			stRect.right - stRect.left, 
			stRect.bottom - stRect.top, 
			0, 
			0, 
			GetModuleHandle(0), 
			0
		);
		ShowWindow( (HWND) m_pFocusWindow, SW_SHOWDEFAULT);
		m_bOwnWindow = true;
	} else
	{
		//TODO user supplied window handle
	}
	
	// we are a fullscreen device
	m_bFullScreen = true;

	// reset the device
	ResetDevice();

	return true;
}

//--------------------------------------------------
//!
//! Initialises the main window and DirectX device.
//!
//--------------------------------------------------
bool GraphicsDevicePlatform::InitWindowed(	int width, 
											int height, 
											const char * pTitle,
											void* hwnd )
{
	// create the Direct3D object 
	ntAssert_p( !m_pDirect3D, ("Direct3d already initialised") );
	m_pDirect3D = Direct3DCreate9(D3D_SDK_VERSION); 

	ntAssert_p( m_pDirect3D, ("failed to initialise direct3d"));

	// windows mode uses the windows refresh rate
	D3DDISPLAYMODE stMode;
	dxerror_p( m_pDirect3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &stMode), 
											("failed to get the current display mode") );
	pThis->m_iDisplayRefreshRate = stMode.RefreshRate;
	pThis->m_iScreenX = width;
	pThis->m_iScreenY = height;

	if( hwnd == 0 )
	{
		// register our window class
		const char* pcClassName = pTitle;
		WNDCLASSEX stClass = 
		{ 
			sizeof(WNDCLASSEX), 
			CS_OWNDC, 
			&WindowProc, 
			0,
			0, 
			GetModuleHandle(0), 
			0, 
			LoadCursor(0, IDC_ARROW), 
			(HBRUSH)( ::GetStockObject( BLACK_BRUSH ) ), 
			0, 
			pcClassName, 
			0			
		};	
		RegisterClassEx(&stClass);

		// adjust the window size for a 640x480 canvas
		DWORD dwWindowStyle = WS_CAPTION | WS_SYSMENU;
		
		RECT stRect = { 0, 0, width, height };

		AdjustWindowRect(&stRect, dwWindowStyle, FALSE);

		// create the window
		m_pFocusWindow = (void*) CreateWindowEx(
			0, 
			pcClassName,
			pTitle,
			dwWindowStyle, 
			2, 
			2, 
			stRect.right - stRect.left, 
			stRect.bottom - stRect.top, 
			0, 
			0, 
			GetModuleHandle(0), 
			0
		);
		ShowWindow( (HWND) m_pFocusWindow, SW_SHOWDEFAULT);
		m_bOwnWindow = true;
	} else
	{
		//TODO user supplied window handle
	}
	
	// we are a windowed device
	m_bFullScreen = false;

	// setup our mouse input singleton
	NT_NEW_CHUNK(Mem::MC_GFX) MouseInput( width, height ); 

	// reset the device
	ResetDevice();

	return true;
}

//--------------------------------------------------
//!
//! Resets the D3D device, destroy and recreates so 
//! only call if you really mean it!
//!
//--------------------------------------------------

void GraphicsDevicePlatform::ResetDevice()
{
	int iWidth, iHeight;
	pThis->GetScreenResolution( iWidth, iHeight );

	if (m_pDevice)
		m_pDevice->Release();
	else
	{
#ifdef TRACK_GFX_MEM
		// only inc for the first time. we account for front and back, but not spurious Z buffer.
		uint32_t iSize = GFXFormat::CalculateVRAMFootprint(	GF_ARGB8, iWidth, iHeight );
		Renderer::ms_iRTAllocs += iSize * 2;
#endif
	}

	// set up the presentation parameters
	D3DPRESENT_PARAMETERS stPP;
	memset(&stPP, 0, sizeof(stPP));
	stPP.BackBufferFormat = D3DFMT_X8R8G8B8;
	stPP.BackBufferCount = 1;
	stPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	stPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	stPP.hDeviceWindow = 0;
	stPP.Windowed = FALSE;
	stPP.EnableAutoDepthStencil = TRUE;
	stPP.AutoDepthStencilFormat	= D3DFMT_D24S8;
	stPP.Flags = 0;
	stPP.BackBufferWidth = iWidth;
	stPP.BackBufferHeight = iHeight;
	
	if( m_bFullScreen )
	{
		stPP.FullScreen_RefreshRateInHz =  pThis->GetDisplayRefreshRate();
		stPP.Windowed = FALSE;
		stPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	else
	{
		stPP.FullScreen_RefreshRateInHz = 0;
		stPP.Windowed = TRUE;
		stPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	// create a new device
	//-----------------------------------------------
	
	UINT AdapterToUse = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;

#if defined(USE_REFERENCE_DEVICE)
	D3DDEVTYPE DeviceType = D3DDEVTYPE_REF;
#endif

#if defined( USE_NVPERF_HUD )
	for( unsigned int iAdaptor = 0; iAdaptor < m_pobDirect3D->GetAdapterCount(); iAdaptor++ )
	{
		D3DADAPTER_IDENTIFIER9 ident;
		m_pobDirect3D->GetAdapterIdentifier( iAdaptor, 0, &ident );
		if( strcmp(ident.Description, "NVIDIA NVPerfHUD") == 0 )
		{
			AdapterToUse = iAdaptor;
			DeviceType = D3DDEVTYPE_REF;
		}
	}
#endif

	DWORD behaviorFlags = D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;

#if defined( _ENABLE_FP_EXCEPTIONS )
	behaviorFlags |= D3DCREATE_FPU_PRESERVE;
#endif

	HRESULT hr = m_pDirect3D->CreateDevice(
								AdapterToUse, 
								DeviceType,
								(HWND) m_pFocusWindow,
								behaviorFlags,
								&stPP, 
								&m_pDevice );
	UNUSED(hr);
	user_error_p( SUCCEEDED( hr ), ("failed to reset/create the device. You probably have MrEd or Maya open at the moment: Reason %s : %s ", ::DXGetErrorString9(hr), ::DXGetErrorDescription9(hr) ) );

	// get the device caps and check for ps.2.0 support
	if (HardwareCapabilities::Exists())
		HardwareCapabilities::Kill();

	NT_NEW_CHUNK(Mem::MC_GFX) HardwareCapabilities();
	ntError_p( HardwareCapabilities::Get().SupportsPixelShader2(),
		( "pixel shader version 2.0 or higher support not found, bailing out...\n" ) );

	// set the device options
	m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

	CTimer::s_fGameRefreshRate = (float) pThis->GetDisplayRefreshRate();
}

//--------------------------------------------------
//!
//! Checks that the graphics card can use this d3dformat as a render target
//!
//--------------------------------------------------
bool GraphicsDevicePlatform::IsValidRenderTargetType( D3DFORMAT eType )
{
	HRESULT hr = m_pDirect3D->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 
													D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, eType );
	if ( hr == S_OK )
		return true;

	ntError_p( hr != D3DERR_INVALIDCALL, ( "Couldn't determine if texture support is present or not" ) );
	return false;
}

//--------------------------------------------------
//!
//! Checks that the graphics card can use this d3dformat as a texture
//!
//--------------------------------------------------
bool GraphicsDevicePlatform::IsValidTextureType( D3DFORMAT eType )
{
	HRESULT hr = m_pDirect3D->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 
													0, D3DRTYPE_TEXTURE, eType );

	if ( hr == S_OK )
		return true;

	ntError_p( hr != D3DERR_INVALIDCALL, ( "Couldn't determine if texture support is present or not" ) );
	return false;
}

//--------------------------------------------------
//!
//! Checks that the graphics card can use this d3dformat as a depth surface
//!
//--------------------------------------------------
bool GraphicsDevicePlatform::IsValidDepthFormat( D3DFORMAT eType )
{
	HRESULT hr = m_pDirect3D->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_D24S8, 
													D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, eType );

	if ( hr == S_OK )
		return true;

	ntError_p( hr != D3DERR_INVALIDCALL, ( "Couldn't determine if texture support is present or not" ) );
	return false;
}

//--------------------------------------------------
//!
//! Checks that the graphics card can use this d3dformat as an blendable rendertarget
//!
//--------------------------------------------------
bool GraphicsDevicePlatform::IsValidAlphaBlendRTFormat( D3DFORMAT eType )
{
	HRESULT hr = m_pDirect3D->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 
													D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, eType );

	if ( hr == S_OK )
		return true;

	ntError_p( hr != D3DERR_INVALIDCALL, ( "Couldn't determine if texture support is present or not" ) );
	return false;
}

//--------------------------------------------------
//!
//! Checks that the graphics card supports hardware shadow maps
//!
//--------------------------------------------------
bool GraphicsDevicePlatform::SupportsHardwareShadowMaps()
{
	HRESULT hr = m_pDirect3D->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 
													D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, D3DFMT_D24S8 );

	if ( hr == S_OK )
		return true;

	ntError_p( hr != D3DERR_INVALIDCALL, ( "Couldn't determine if texture support is present or not" ) );
	return false;
}