/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <cell/gcm.h>
#include <cell/gcm_pm.h>
#if ICERENDER_090
#include <cell/sysmodule.h>
#endif
#include <sysutil/sysutil_sysparam.h>
#include "icerender.h"
#include "icedisasm.h"

#if ICERENDER_090
using namespace cell::Gcm;
#endif

using namespace Ice::Render;

namespace Ice
{
	namespace Render
	{
		enum
		{
			// Allocate 64K to the default libgcm command context
			kLibgcmContextSize					= 65536,

			// Each glue command context has 4K of space
			kGlueContextSize					= 4096
		};

		// Command buffer data
		CommandContext		g_glueCommandContext[4];

		static U32 s_defaultCommandAddress;

		U32 static s_nullFragmentProgram[8] = 
			{0x3E8101C0, 0xC8051C9D, 0xC8050001, 0xC8050001,  // MOVB     H0, f[COL0];
			0x00010000, 0x00000000, 0x00000000, 0x00000000}; // NOP


		U32 g_glueContextIndex;

		volatile GpuControl *g_gpuControl = NULL;

		void *g_nullFragmentProgramAddress;

		// GPU's hardware configuration information.
		ICE_ALIGN(128) GpuHardwareConfig g_gpuHardwareConfig;
	}
}

U32 Ice::Render::MapMainMemoryToIoOffset(void const *addr, U32 size)
{
	ICE_ASSERT((size & 0xFFFFF) == 0);
	ICE_ASSERT((U32(addr) & 0xFFFFF) == 0);

	U32 offset;
	cellGcmMapMainMemory(addr, size, &offset);

	U32F startEa = (U32)addr >> 20;
	U32F startIo = offset >> 20;
	U32F count = size >> 20;
	U32F endEa = startEa + count;
	U32F endIo = startIo + count;
	
	U32F io = startIo;
	for(U32F i = startEa; i < endEa; ++i, ++io)
	{
		ICE_ASSERT(g_gpuHardwareConfig.m_ioTable[i] == 0xFFFF);
		g_gpuHardwareConfig.m_ioTable[i] = io;
	}
		
	U32F ea = startEa;
	for(U32F i = startIo; i < endIo; ++i, ++ea)
	{
		ICE_ASSERT(g_gpuHardwareConfig.m_eaTable[i] == 0xFFFF);
		g_gpuHardwareConfig.m_eaTable[i] = ea;
	}
	
	return offset;
}

#if ICERENDER_090
static int32_t gcmReserveFailed(CellGcmContextData */*context*/, uint32_t /*numWords*/)
{
	/*printf("space\n");*/
	return CELL_GCM_ERROR_FAILURE;
}
#else
static uint32_t * gcmFunc(uint32_t) { return cellGcmGetCurrentBuffer(); }
static void gcmFinish(void){}
static void gcmFlush(void){}
#endif

// Shutdown libgcm
extern "C" U32 cellGcmTerminate();

U32 Ice::Render::Initialize(DisplayMode mode, DisplayOutputMode outMode, U32 outputColorBufferPitch, void *defaultCommandAddress, U32 defaultCommandSize, I32 initialReferenceValue, bool enablePerformanceReports)
{
	ICE_ASSERTF((U32(defaultCommandAddress) & 0xFFFFF) == 0, ("The default command address must be 1 meg aligned."));
	ICE_ASSERTF((defaultCommandSize & 0xFFFFF) == 0, ("The default command size must be a multiple of 1 meg"));
	ICE_ASSERT(defaultCommandSize >= kLibgcmContextSize);
	
	s_defaultCommandAddress = U32(defaultCommandAddress);
	cellGcmInit(kLibgcmContextSize, defaultCommandSize, (U64*)defaultCommandAddress);

	// Enable performance reports? (This also disables gpu power management)
	if(enablePerformanceReports)
		cellGcmInitPerfMon();
	
	// Aquire the GPU's configuration.
	CellGcmConfig config;
	cellGcmGetConfiguration(&config);

	// Copy relevant configuration information to a global structure.
	g_gpuHardwareConfig.m_videoMemoryBaseAddr = config.localAddress;
	g_gpuHardwareConfig.m_videoMemoryFrequency = config.memoryFrequency;
	g_gpuHardwareConfig.m_videoMemorySize = config.localSize + 0x2000000; // 32 mb is reserved for the OS.
	g_gpuHardwareConfig.m_usableVideoMemorySize = config.localSize; 
	g_gpuHardwareConfig.m_coreFrequency = config.coreFrequency;
	g_gpuHardwareConfig.m_semaphoreBaseAddress = U32(cellGcmGetLabelAddress(0));
	g_gpuHardwareConfig.m_semaphoreStride = U32(cellGcmGetLabelAddress(1)) - g_gpuHardwareConfig.m_semaphoreBaseAddress;
	g_gpuHardwareConfig.m_displaySwapMode = kSwapModeVSync;

	// Initialize the memory maps
	memset(g_gpuHardwareConfig.m_ioTable, 0xFF, sizeof(g_gpuHardwareConfig.m_ioTable));
	memset(g_gpuHardwareConfig.m_eaTable, 0xFF, sizeof(g_gpuHardwareConfig.m_eaTable));
	U32F startEa = (U32)defaultCommandAddress >> 20;
	U32F defaultMegs = defaultCommandSize >> 20;
	U32F io = 0;
	for(U32F i = 0; i < defaultMegs; ++i, ++io)
		g_gpuHardwareConfig.m_ioTable[i+startEa] = io;
	U32F ea = startEa;
	for(U32F i = 0; i < defaultMegs; ++i, ++ea)
		g_gpuHardwareConfig.m_eaTable[i] = ea;

	// Aquire the gpu control structure
	g_gpuControl = (GpuControl *)cellGcmGetControlRegister();

#if ICERENDER_090
	CellVideoOutState videoState;
	CellVideoOutResolution resolution;
	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	cellVideoOutGetResolution(videoState.displayMode.resolutionId, &resolution);
#else
	cellSysutilInit();
	CellVideoOutState videoState;
	CellVideoResolution resolution;
	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	cellVideoOutGetResolution(videoState.displayMode.resolutionId, &resolution);
#endif

	// Setup Out Mode
	U32 pixelSize;
	U32 omode;
	switch(outMode)
	{
#if ICERENDER_090
	default: case kOutModeArgb8888: omode = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8; pixelSize=4; break;
	case kOutModeAbgr8888: omode = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8B8G8R8; pixelSize=4; break;
	case kOutModeRgba16f: omode = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_R16G16B16X16_FLOAT; pixelSize=8; break;
#else
	default: case kOutModeArgb8888: omode = CELL_VIDEO_BUFFER_COLOR_FORMAT_X8R8G8B8; pixelSize=4; break;
	case kOutModeAbgr8888: omode = CELL_VIDEO_BUFFER_COLOR_FORMAT_X8B8G8R8; pixelSize=4; break;
	case kOutModeRgba16f: omode = CELL_VIDEO_BUFFER_COLOR_FORMAT_R16G16B16X16_FLOAT; pixelSize=8; break;
#endif
	}

	// Setup display
	U32 pitch = outputColorBufferPitch;
	U32 width = resolution.width;
	U32 height = resolution.height;
#if ICERENDER_090
	CellVideoOutResolutionId resolutionId;
#else
	CellVideoResolutionId resolutionId;
#endif
	switch(mode)
	{
	default: 
#if ICERENDER_090
	case kDisplay480: resolutionId = CELL_VIDEO_OUT_RESOLUTION_480; width=720; height=480; break;
	case kDisplay576: resolutionId = CELL_VIDEO_OUT_RESOLUTION_576; width=720; height=576; break;
	case kDisplay720: resolutionId = CELL_VIDEO_OUT_RESOLUTION_720; width=1280; height=720; break;
	case kDisplay1080: resolutionId = CELL_VIDEO_OUT_RESOLUTION_1080; width=720; height=576; break;
	case kDisplayAutoLinear: resolutionId = (CellVideoOutResolutionId)videoState.displayMode.resolutionId; pitch=width*pixelSize; break;
	case kDisplayAutoTiled: resolutionId = (CellVideoOutResolutionId)videoState.displayMode.resolutionId; pitch=GetTiledPitch(width*pixelSize); break;
	case kDisplayAutoCustom: resolutionId = (CellVideoOutResolutionId)videoState.displayMode.resolutionId; break;
#else
	case kDisplay480: resolutionId = CELL_VIDEO_RESOLUTION_480; width=720; height=480; break;
	case kDisplay576: resolutionId = CELL_VIDEO_RESOLUTION_576; width=720; height=576; break;
	case kDisplay720: resolutionId = CELL_VIDEO_RESOLUTION_720; width=1280; height=720; break;
	case kDisplay1080: resolutionId = CELL_VIDEO_RESOLUTION_1080; width=720; height=576; break;
	case kDisplayAutoLinear: resolutionId = (CellVideoResolutionId)videoState.displayMode.resolutionId; pitch=width*pixelSize; break;
	case kDisplayAutoTiled: resolutionId = (CellVideoResolutionId)videoState.displayMode.resolutionId; pitch=GetTiledPitch(width*pixelSize); break;
	case kDisplayAutoCustom: resolutionId = (CellVideoResolutionId)videoState.displayMode.resolutionId; break;
#endif
	}

	// Set hardware config
	g_gpuHardwareConfig.m_displayWidth = width;
	g_gpuHardwareConfig.m_displayHeight = height;
	g_gpuHardwareConfig.m_displayPitch = pitch;

	CellVideoOutConfiguration videocfg;
	memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
	videocfg.resolutionId = resolutionId;
	videocfg.format = omode;
	videocfg.pitch = pitch;
	cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0);

	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	switch(videoState.displayMode.aspect)
	{
		case CELL_VIDEO_OUT_ASPECT_4_3:  g_gpuHardwareConfig.m_aspectRatio = kAspectStandard; break;
		case CELL_VIDEO_OUT_ASPECT_16_9: g_gpuHardwareConfig.m_aspectRatio = kAspectWidescreen; break;
		default: ICE_ASSERT(!"Invalid hardware display aspect ratio?"); break;
	}

	cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);
	cellGcmSetJumpCommand(kLibgcmContextSize);
#if ICERENDER_090
	cellGcmSetUserCallback(&gcmReserveFailed);
#else
	cellGcmSetUserCallback(&gcmFunc, &gcmFinish, &gcmFlush);
#endif

	// Allocate command context for inter-frame glue area
	char *glueMemory = (char *)defaultCommandAddress + kLibgcmContextSize;
	g_glueCommandContext[0].Init(glueMemory, kGlueContextSize);
	g_glueCommandContext[1].Init(glueMemory + kGlueContextSize, kGlueContextSize);
	g_glueCommandContext[2].Init(glueMemory + kGlueContextSize * 2, kGlueContextSize);
	g_glueCommandContext[3].Init(glueMemory + kGlueContextSize * 3, kGlueContextSize);
	g_glueContextIndex = 0;

	// Load the null fragment program.
	g_nullFragmentProgramAddress = (U32*)((U8*)defaultCommandAddress + kLibgcmContextSize + kGlueContextSize*4);
	memcpy(g_nullFragmentProgramAddress, s_nullFragmentProgram, sizeof(s_nullFragmentProgram));
	g_nullFragmentProgramOffset = kLibgcmContextSize + kGlueContextSize*4;

	// Setup some default hardware state
	g_glueCommandContext[0].SetDrawBufferMask(kDrawBuffer0);
	g_glueCommandContext[0].SetColorMask(true, true, true, true);
	g_glueCommandContext[0].SetFrontFace(kFrontFaceCcw);
	g_glueCommandContext[0].Reserve(6);
	g_glueCommandContext[0].m_cmdptr[0] = 0x00041FCC;
	g_glueCommandContext[0].m_cmdptr[1] = 0x0000006F;
	g_glueCommandContext[0].m_cmdptr[2] = 0x000401A8;
	g_glueCommandContext[0].m_cmdptr[3] = 0xFEED0001;
	g_glueCommandContext[0].m_cmdptr[4] = 0x00041450;
	g_glueCommandContext[0].m_cmdptr[5] = 0x00080004;
	g_glueCommandContext[0].m_cmdptr += 6;

	// Setup references
	g_glueCommandContext[0].InsertReference(initialReferenceValue);
	
	asm volatile ("sync");
	// Update the PUT pointer on the GPU.
	// This causes the GPU to start consuming commands if it had previously stopped.
	g_gpuControl->m_putOffset = U32(g_glueCommandContext[0].m_cmdptr) - s_defaultCommandAddress;

	// Glue contexts + Null Fragment Program
	return kLibgcmContextSize+kGlueContextSize*4+128;
}

void Ice::Render::Terminate()
{
	cellGcmTerminate();
}

U32 Ice::Render::KickAndDisplayRenderTarget(U32 startIoOffset, U32 *end, const RenderTarget *target, I32 reference, bool stallGpu)
{
	ICE_ASSERT(target != NULL);
	ICE_ASSERTF(((target->m_targetFormat >> 12) & 0x000F) == kMultisampleNone, ("The Gpu does not support multisampled render targets for display."));
	
	// Insert jump into previously kicked buffer that transfers execution to the new buffer
	CommandContext *prevContext = &g_glueCommandContext[g_glueContextIndex];
	prevContext->InsertJump(startIoOffset);

	// Setup the inter-frame glue
	U32 index = (g_glueContextIndex + 1) & 3;
	g_glueContextIndex = index;

	CommandContext *context = &g_glueCommandContext[index];
	context->ResetBuffer();

	// Insert jump into previously kicked buffer that transfers execution to the new buffer
	*end = (U32(context->m_cmdptr) - s_defaultCommandAddress) | kCmdJumpFlag;

	// Insert a reference so that we can tell when a frame swap is done.
	context->InsertReference(reference);

	// Insert the swap command into the glue context
	if(context->Reserve(0x100))
	{
		cellGcmSetCurrentBuffer(context->m_cmdptr, 0x100);

		if(g_gpuHardwareConfig.m_displaySwapMode == kSwapModeImmediate)
			cellGcmSetFlipImmediate(target->m_displayIndex);
		else
			cellGcmSetFlip(target->m_displayIndex);
		cellGcmSetWaitFlip();

		context->m_cmdptr = cellGcmGetCurrentBuffer();
	}

	// Setup some default hardware state
	context->SetDrawBufferMask(kDrawBuffer0);
	context->SetColorMask(true, true, true, true);
	context->SetFrontFace(kFrontFaceCcw);
	if(context->Reserve(6))
	{
		context->m_cmdptr[0] = 0x00041FCC;
		context->m_cmdptr[1] = 0x0000006F;
		context->m_cmdptr[2] = 0x000401A8;
		context->m_cmdptr[3] = 0xFEED0001;
		context->m_cmdptr[4] = 0x00041450;
		context->m_cmdptr[5] = 0x00080004;
		context->m_cmdptr += 6;
	}

	// Invalidate the various caches
	context->InvalidatePreTransformCache();
	context->InvalidatePostTransformCache();
	context->InvalidateTextureCache();

	U32 putOffset = U32(context->m_cmdptr) - s_defaultCommandAddress;

	if(!stallGpu)
	{
		// Tell the gpu to continue processing until the next last valid command.
		asm volatile ("sync");
		g_gpuControl->m_putOffset = putOffset;
	}

	// Setting this to the GpuControl's Put Offset causes the GPU to start
	// consuming commands if it had previously stopped.
	return putOffset;
}
