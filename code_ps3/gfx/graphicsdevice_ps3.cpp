//--------------------------------------------------
//!
//!	\file gfx/graphicsdevice_ps3.cpp
//! This is the PS3 version of the graphics device
//!
//--------------------------------------------------

#include "gfx/graphicsdevice.h"
#include <Gc/GcKernel.h>
#include <Gc/GcInitParams.h>
#include "gfx/hardwarecaps.h"
#include "gfx/renderer.h"
#include "gfx/gpureports_ps3.h"
#include "core/profiling.h"
#include "core/timer_ps3.h"
#include "core/gfxmem.h"
#include "core/memman.h"
#include "gfx/rendercontext.h"
#include "gfx/display.h"
#include "heresy/heresy_capi.h"
#include "3rdparty/wws/projects/beyonddebugging/include/ppu/captureapi.h"

//----------------------------------------------------
//!
//! This is a small callback function we need in order 
//! to grab a copy of our push buffer with BD Capture
//!
//----------------------------------------------------


#if (defined(_RELEASE) || defined(_DEVELOPMENT)) && defined(_PROFILING)
void IceLibCaptureGpuFinishCallback(void* pLastKickedStart, void* pLastKickedEnd)
{
    // Call the BD lib capture heartbeat with the start and end offsets of our push buffer!
    BeyondDebugging::Capture::Heartbeat(Ice::Render::TranslateAddressToIoOffset(pLastKickedStart),
                                        Ice::Render::TranslateAddressToIoOffset(pLastKickedEnd));

	if ( Renderer::Get().m_Platform.m_pGPUReport )
	{
		Renderer::Get().m_Platform.m_pGPUReport->CloseReports();		
		Renderer::Get().m_Platform.m_pGPUReport->CollectReports();
		Renderer::Get().m_Platform.m_pGPUReport->StartReports();
	}
}

void GpuFinishCallback(void* pLastKickedStart, void* pLastKickedEnd)
{
	if ( Renderer::Get().m_Platform.m_pGPUReport )
	{
		Renderer::Get().m_Platform.m_pGPUReport->CloseReports();		
		Renderer::Get().m_Platform.m_pGPUReport->CollectReports();
		Renderer::Get().m_Platform.m_pGPUReport->StartReports();
}
}

#endif

//--------------------------------------------------
//!
//! PC specific construction of graphics device.
//! shouldn't actually do very much
//!
//--------------------------------------------------
GraphicsDevice::GraphicsDevice()
{
	m_Platform.pThis = this;
	NT_NEW_CHUNK( Mem::MC_GFX ) HardwareCapabilities();
}

//--------------------------------------------------
//!
//! dtor
//!
//--------------------------------------------------
GraphicsDevice::~GraphicsDevice()
{
#if defined(_RELEASE) || defined(_DEVELOPMENT)
	if (CRendererSettings::bEnableBeyondDebugging)
	{
		BeyondDebugging::Capture::Done();
		BeyondDebugging::Network::Done(); // :NOTE: Only required if you have no other code which shutdown the PS3 network libraries!
	}
#endif

	GcKernel::Shutdown();

	if (HardwareCapabilities::Exists())
		HardwareCapabilities::Kill();
}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------

bool GraphicsDevicePlatform::Init()
{
	static GcInitParams params;

	// we start with HSYNC, knowing we will swap to correct one in callback
	params.SetDisplaySwapMode( Gc::kDisplaySwapModeHSync );

	// Gc can 'see' all our our RSX XDDR ram
	params.SetManagedHostMem( (void*)Mem::GetBaseAddress( Mem::MC_RSX_MAIN_INTERNAL ) );
	params.SetManagedHostMemSize( Mem::RSX_ADDRESSABLE_SIZE );
	params.SetPushBufferSize( Mem::GcMem::GC_PUSH_BUFFER_SIZE );
	params.SetReportBufferSize( Mem::GcMem::GC_GPUREPORT_RAM_SIZE );
	GcKernel::Initialise( params );

#if (defined(_RELEASE) || defined(_DEVELOPMENT)) && defined(_PROFILING)
	if (CRendererSettings::bEnableBeyondDebugging)
	{
		// Initialise the PS3 network interface and libraries.
		BeyondDebugging::Network::Init();
		// Initalise the capture API.
		BeyondDebugging::Capture::Init();
		// Register a Gpu finish callback so we can execute the BD lib capture heartbeat function each frame. Only required if you're using GcKernel::Present().
		GcKernel::RegisterFinishCallback(IceLibCaptureGpuFinishCallback);
	}
	else
	{
		GcKernel::RegisterFinishCallback(GpuFinishCallback);
	}
#endif

	GcKernel::SetTotalScratchMemory( Mem::GcMem::GC_SCRATCH_RAM_SIZE );

	void* pGcPtr;
	void* pMyPtr;
	UNUSED( pGcPtr );
	UNUSED( pMyPtr );
	pGcPtr = GcKernel::AllocateHostMemory( Mem::RSX_MAIN_USER_SIZE ); // account for our XDDR texture manager
	pMyPtr = (void*)Mem::GetBaseAddress( Mem::MC_RSX_MAIN_USER );
	ntError( pGcPtr == pMyPtr );

	// note we have already reserved space in our RSX_MAIN allocation for all this stuff

	// now that Gc is initialised, setup our display manager
	DisplayManager::Get().ConfigureDisplay();

	// PS3 only puts out 50Hz on PAL SD, all else runs at 59.94, game 'tick' is half that.
	if	(
		(DisplayManager::Get().GetOutputMode() == DisplayManager::PAL_576_4_3) ||
		(DisplayManager::Get().GetOutputMode() == DisplayManager::PAL_576_16_9)
		)
	{
		pThis->m_fGameRefreshRate = 25.f;
		CTimer::s_fGameRefreshRate = 25.f;
	}	
	else
	{
		pThis->m_fGameRefreshRate = 29.97f;
		CTimer::s_fGameRefreshRate = 29.97f;
	}

#ifdef TRACK_GFX_MEM
	// we account for front and back buffers only
	uint32_t iSize = GFXFormat::CalculateVRAMFootprint( GF_ARGB8, (int)DisplayManager::Get().GetDeviceWidth(), (int)DisplayManager::Get().GetDeviceHeight() );
	Renderer::ms_iRTAllocs += iSize * 2;
#endif

	return true;
}
//--------------------------------------------------
//!
//! 
//! 
//--------------------------------------------------
