/***************************************************************************************************
*
*	$Header:: /game/renderer.cpp 41    24/07/03 11:53 Simonb                                       $
*
*	The Renderer 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"

// for debug render
#include "core/visualdebugger.h"
#include "core/timer.h"
#include "gfx/levellighting.h"
#include "gfx/levelofdetail.h"
#include "gfx/depthoffield.h"

#if defined(PLATFORM_PS3) && defined(_SPEEDTREE)
#include "speedtree/speedtreemanager_ps3.h"
#include "speedtree/speedgrass_ps3.h"
#endif

// static trackers for gross allocations
#ifdef TRACK_GFX_MEM
uint32_t Renderer::ms_iVBAllocs = 0;
uint32_t Renderer::ms_iVBVertCount = 0;
uint32_t Renderer::ms_iIBAllocs = 0;
uint32_t Renderer::ms_iDiskTex = 0;
uint32_t Renderer::ms_iProcTex = 0;
uint32_t Renderer::ms_iRTAllocs = 0;
#endif

//-----------------------------------------------------
//!
//! Retrieve the format to use for HDR rendering
//!
//-----------------------------------------------------
GFXFORMAT Renderer::GetHDRFormat( void )
{
	if (HardwareCapabilities::Get().IsValidRenderTargetFormat(GF_ABGR16F))
		return GF_ABGR16F;
	else if (HardwareCapabilities::Get().IsValidRenderTargetFormat(GF_ABGR32F))
		return GF_ABGR32F;
	else
		return GF_ARGB8;
}

//-----------------------------------------------------
//!
//! Set the device to its default state
//!
//-----------------------------------------------------
void Renderer::SetDefaultRenderStates( void )
{
	Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
	SetBlendMode( GFX_BLENDMODE_NORMAL );
	SetZBufferMode( GFX_ZMODE_NORMAL );
	SetCullMode( GFX_CULLMODE_NORMAL );
}

//-----------------------------------------------------
//!
//! Debug render our rendering info
//!
//----------------------------------------------------
void Renderer::DumpStats()
{
	ntPrintf( "\n\nRenderer Stats\n" );
	ntPrintf( "-------------------------------------\n" );

#ifdef _PROFILING
	ntPrintf( "CPU Render: %.2f%%\n", CProfiler::Get().GetCPURenderPercent() );
#endif

#ifdef PLATFORM_PS3
#ifdef _PROFILING
	ntPrintf( "GPU Render: %.2f%%\n", CProfiler::Get().GetGPURenderPercent() );
	ntPrintf( "Draw Calls: %d\n", m_lastCount.m_iDraws );
	ntPrintf( "Rendered Polycount: %d\n", m_lastCount.m_iPolys );
	ntPrintf( "Rendered Vertexcount: %d\n", m_lastCount.m_iVerts );
#endif
#endif

	ntPrintf( "Level of Detail Bias: %d\n", CLODManager::Get().GetForcedLODOffset() );

	float fHours, fMins;
	LevelLighting::Get().GetTimeOfDay( fHours, fMins );
	ntPrintf( "ToD: %02.0f:%02.0f\n", fHours, fMins );

#ifdef PLATFORM_PS3

	uint32_t iMetric = 0;
	UNUSED( iMetric );

    GC_GET_METRICS("VramAllocated", iMetric);
	ntPrintf( " Gc VRAM: %.2f Mb (%.1f%%)\n", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(VRAM_LIMIT)) * 100.0f );

    GC_GET_METRICS("HostMemAllocated", iMetric);
	ntPrintf( " Gc XDR: %.2f Mb (%.1f%%)\n", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(RAM_LIMIT)) * 100.0f );

    GC_GET_METRICS("NumSetVertexShaderCalls", iMetric);
	ntPrintf( " Gc Set Vertex Shader: %d\n", iMetric );

    GC_GET_METRICS("NumSetFragmentShaderCalls", iMetric);
	ntPrintf( " Gc Set Pixel Shader: %d\n", iMetric );

    GC_GET_METRICS("NumSetTextureCalls", iMetric);
	ntPrintf( " Gc Set Texture: %d\n", iMetric );

	uint32_t iMetric2 = 0;
	UNUSED( iMetric2 );

    GC_GET_METRICS("NumVertices", iMetric);
    GC_GET_METRICS("NumIndexedVertices", iMetric2);
	ntPrintf( " Gc Num Vertices: %d\n", iMetric+iMetric2 );

	GC_GET_METRICS("NumDrawArraysCalls", iMetric);
	GC_GET_METRICS("NumDrawElementsCalls", iMetric2);
	ntPrintf( " Gc Num Draw Calls: %d\n", iMetric+iMetric2 );

	GC_GET_METRICS("GpuPushBufferWritten", iMetric);
	ntPrintf( " Gc Push buffer written: %.2f Kb\n", _R(iMetric) / (1024.f) );

	GC_RESET_METRICS();

#endif

	// memory use
#ifdef TRACK_GFX_MEM

	uint32_t iTotalGFXMem = Renderer::ms_iVBAllocs + 
							Renderer::ms_iIBAllocs + 
							Renderer::ms_iDiskTex + 
							Renderer::ms_iProcTex + 
							Renderer::ms_iRTAllocs;

	ntPrintf( "GFX MEM:         %.2f Mb (%.1f%%)\n", _R(iTotalGFXMem) / (1024.f*1024.f), _R(iTotalGFXMem/_R(VRAM_LIMIT)) * 100.0f );
	ntPrintf( " Vertex buffers: %.2f Mb\n", _R(Renderer::ms_iVBAllocs) / (1024.f*1024.f) );
	UNUSED( iTotalGFXMem );

#ifdef PLATFORM_PS3
	ntPrintf( "   Vertex Count: %d\n", Renderer::ms_iVBVertCount );

	float fAverageVertSize = Renderer::ms_iVBAllocs / _R(Renderer::ms_iVBVertCount);
	ntPrintf( "   Vertex Size (Ave): %.0f\n", fAverageVertSize );
	UNUSED( fAverageVertSize );
#endif

	ntPrintf( " Index buffers:  %.2f Mb\n", _R(Renderer::ms_iIBAllocs) / (1024.f*1024.f) );
	ntPrintf( " Disk textures:   %.2f Mb\n", _R(Renderer::ms_iDiskTex) / (1024.f*1024.f) );
	ntPrintf( " Procedural tex: %.2f Mb\n", _R(Renderer::ms_iProcTex) / (1024.f*1024.f) );
	ntPrintf( " Render targets: %.2f Mb\n", _R(Renderer::ms_iRTAllocs) / (1024.f*1024.f) );

#endif
}

//-----------------------------------------------------
//!
//! Debug render our rendering info
//!
//----------------------------------------------------
void Renderer::DebugRender( float fStartX, float fStartY, float fSpacing )
{
#ifndef _GOLD_MASTER

	uint32_t iNormalCol = NTCOLOUR_ARGB( 0xff, 0, 0xe0, 0xe0 );

	// timers
#	ifdef _PROFILING
		uint32_t iTimeCol = (CProfiler::Get().GetCPURenderPercent() > 100.0f) ? NTCOLOUR_ARGB( 0xff, 0xff, 0, 0 ) : iNormalCol;
		g_VisualDebug->Printf2D( fStartX, fStartY, iTimeCol, 0, "CPU Render: %.2f%%", CProfiler::Get().GetCPURenderPercent() );
		fStartY += fSpacing;
#	endif

#ifdef PLATFORM_PS3
	float fVBlankInterval = 1.0f / CTimer::s_fGameRefreshRate;
	float fFrameTime = ntstd::Max( m_fGPUFrameTime, m_fCPUFrameTime );
	float fInterval = (fFrameTime / fVBlankInterval ) * 100.0f;

	uint32_t iPresentCol = (fInterval > 100.0f) ? NTCOLOUR_ARGB( 0xff, 0xff, 0, 0 ) : iNormalCol;

	switch (m_presentMode)
	{
	case PM_IMMEDIATE: 	g_VisualDebug->Printf2D( fStartX, fStartY, iPresentCol, 0, "HSYNC %.2f%%", fInterval ); break;
	case PM_VBLANK: 	g_VisualDebug->Printf2D( fStartX, fStartY, iPresentCol, 0, "VSYNC %.2f%%", fInterval ); break;
	case PM_AUTO: 
		{
			if ( m_autoPresentMode == PM_IMMEDIATE )
				g_VisualDebug->Printf2D( fStartX, fStartY, iPresentCol, 0, "AUTO HSYNC %.2f%%", fInterval );
			else
				g_VisualDebug->Printf2D( fStartX, fStartY, iPresentCol, 0, "AUTO VSYNC %.2f%%", fInterval );
		}
		break;
	}
	fStartY += fSpacing;

#ifdef _PROFILING
	iTimeCol = (CProfiler::Get().GetGPURenderPercent() > 100.0f) ? NTCOLOUR_ARGB( 0xff, 0xff, 0, 0 ) : iNormalCol;
	g_VisualDebug->Printf2D( fStartX, fStartY, iTimeCol, 0, "GPU Render: %.2f%%", CProfiler::Get().GetGPURenderPercent() );
	fStartY += fSpacing;

	// poly counts
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Draw Calls: %d", m_lastCount.m_iDraws );
	fStartY += fSpacing;

	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Rendered Polycount: %d", m_lastCount.m_iPolys );
	fStartY += fSpacing;

	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Rendered Vertexcount: %d", m_lastCount.m_iVerts );
	fStartY += fSpacing;
#endif

#endif

	// level of detail
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Level of Detail Bias: %d", CLODManager::Get().GetForcedLODOffset() );
	fStartY += fSpacing;

	// print time of day
	float fHours, fMins;
	LevelLighting::Get().GetTimeOfDay( fHours, fMins );
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "ToD: %02.0f:%02.0f", fHours, fMins );
	fStartY += fSpacing*2.0f;

#ifdef PLATFORM_PS3


	// Ensure persistency for some of our Gpu metrics.

	int64_t	iTimeNanosec = 0;
    GC_GET_METRICS("GpuTime", iTimeNanosec);

	float fFrameIntervalSecs = 1.0f / 30.0f;
	float fGPUTimeSeconds = float(double(iTimeNanosec) * double(1e-9));
	float fGPURenderPercent = (fGPUTimeSeconds / fFrameIntervalSecs) * 100.0f;

	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc GPU Render: %.1f%%", fGPURenderPercent );
	fStartY += fSpacing;

	uint32_t iMetric = 0;

    GC_GET_METRICS("GpuPixels", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Pixels: %.1f screens (%d)", iMetric / (1280.f * 720.f), iMetric );
	fStartY += 12.f;

    GC_GET_METRICS("VramTotal", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Total VRAM: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(VRAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

    GC_GET_METRICS("VramAllocated", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Used  VRAM: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(VRAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

    GC_GET_METRICS("VramFree", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Free  VRAM: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(VRAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

    GC_GET_METRICS("HostMemTotal", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Total XDR: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(RAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

    GC_GET_METRICS("HostMemAllocated", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Used  XDR: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(RAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

    GC_GET_METRICS("HostMemFree", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Free  XDR: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(RAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

    GC_GET_METRICS("ScratchTotal", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Total Scratch: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(RAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

    GC_GET_METRICS("ScratchAllocated", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Used Scratch: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(RAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

    GC_GET_METRICS("ScratchFree", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Free Scratch: %.2f Mb (%.1f%%)", _R(iMetric) / (1024.f*1024.f), _R(iMetric/_R(RAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;
    GC_GET_METRICS("NumSetVertexShaderCalls", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Set Vertex Shader: %d", iMetric );
	fStartY += fSpacing;

    GC_GET_METRICS("NumSetFragmentShaderCalls", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Set Pixel Shader: %d", iMetric );
	fStartY += fSpacing;

    GC_GET_METRICS("NumSetTextureCalls", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Set Texture: %d", iMetric );
	fStartY += fSpacing;

	uint32_t iMetric2 = 0;
    GC_GET_METRICS("NumVertices", iMetric);
    GC_GET_METRICS("NumIndexedVertices", iMetric2);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Num Vertices: %d", iMetric+iMetric2 );
	fStartY += fSpacing;

	GC_GET_METRICS("NumDrawArraysCalls", iMetric);
	GC_GET_METRICS("NumDrawElementsCalls", iMetric2);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Num Draw Calls: %d", iMetric+iMetric2 );
	fStartY += fSpacing;

	GC_GET_METRICS("GpuPushBufferWritten", iMetric);
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Gc Push buffer written: %.2f Kb", _R(iMetric) / (1024.f) );
	fStartY += fSpacing*2.0f;

	GC_RESET_METRICS();

#endif

	// memory use
#ifdef TRACK_GFX_MEM

	uint32_t iTotalGFXMem = Renderer::ms_iVBAllocs + 
							Renderer::ms_iIBAllocs + 
							Renderer::ms_iDiskTex + 
							Renderer::ms_iProcTex + 
							Renderer::ms_iRTAllocs;

	uint32_t iCol = (iTotalGFXMem > VRAM_LIMIT) ? NTCOLOUR_ARGB( 0xff, 0xff, 0, 0 ) : iNormalCol;
	g_VisualDebug->Printf2D( fStartX, fStartY, iCol, 0, "GFX MEM:         %.2f Mb (%.1f%%)", _R(iTotalGFXMem) / (1024.f*1024.f), _R(iTotalGFXMem/_R(VRAM_LIMIT)) * 100.0f );
	fStartY += fSpacing;

	g_VisualDebug->Printf2D( fStartX, fStartY, iCol, 0, " Vertex buffers: %.2f Mb", _R(Renderer::ms_iVBAllocs) / (1024.f*1024.f) );
	fStartY += fSpacing;

#ifdef PLATFORM_PS3
	g_VisualDebug->Printf2D( fStartX, fStartY, iCol, 0, "   Vertex Count: %d", Renderer::ms_iVBVertCount );
	fStartY += fSpacing;

	float fAverageVertSize = Renderer::ms_iVBAllocs / _R(Renderer::ms_iVBVertCount);
	g_VisualDebug->Printf2D( fStartX, fStartY, iCol, 0, "   Vertex Size (Ave): %.0f", fAverageVertSize );
	fStartY += fSpacing;
#endif

	g_VisualDebug->Printf2D( fStartX, fStartY, iCol, 0, " Index buffers:  %.2f Mb", _R(Renderer::ms_iIBAllocs) / (1024.f*1024.f) );
	fStartY += fSpacing;

	g_VisualDebug->Printf2D( fStartX, fStartY, iCol, 0, " Disk texures:   %.2f Mb", _R(Renderer::ms_iDiskTex) / (1024.f*1024.f) );
	fStartY += fSpacing;

	g_VisualDebug->Printf2D( fStartX, fStartY, iCol, 0, " Procedural tex: %.2f Mb", _R(Renderer::ms_iProcTex) / (1024.f*1024.f) );
	fStartY += fSpacing;

	g_VisualDebug->Printf2D( fStartX, fStartY, iCol, 0, " Render targets: %.2f Mb", _R(Renderer::ms_iRTAllocs) / (1024.f*1024.f) );
	fStartY += fSpacing;

#endif

#if defined(PLATFORM_PS3) && defined(_SPEEDTREE)
	if (SpeedTreeManager::Exists())
	{
		unsigned int speedtreeVertex(0);
		unsigned int speedtreeIndex(0);
		unsigned int speedtreeTexture(0);

		unsigned int speedtreeTotal = SpeedTreeManager::Get().GetVRAMFootprint(speedtreeVertex, speedtreeIndex, speedtreeTexture);

		g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Speedtree Vertex: %.2f Mb  Index: %.2fMb, Textures: %.2fMb", _R(speedtreeVertex) / (1024.f*1024.f), _R(speedtreeIndex) / (1024.f*1024.f), _R(speedtreeTexture) / (1024.f*1024.f) );
		fStartY += fSpacing;

		g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Speedtree Total: %.2f Mb", _R(speedtreeTotal) / (1024.f*1024.f) );
		fStartY += fSpacing;

		unsigned int speedGrassVertex(0);
		unsigned int speedGrassIndex(0);
		unsigned int speedGrassTexture(0);

		unsigned int speedGrassTotal = SpeedGrass::GetVRAMFootprint(speedGrassVertex, speedGrassIndex, speedGrassTexture);

		g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Speedgrass Vertex: %.2f Mb  Index: %.2fMb, Textures: %.2fMb", _R(speedGrassVertex) / (1024.f*1024.f), _R(speedGrassIndex) / (1024.f*1024.f), _R(speedGrassTexture) / (1024.f*1024.f) );
		fStartY += fSpacing;

		g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, " Speedgrass Total: %.2f Mb", _R(speedGrassTotal) / (1024.f*1024.f) );
		fStartY += fSpacing;

	}

#endif

#endif
}
