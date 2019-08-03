/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <cell/spurs/types.h>
#include "jobapi/jobdefinition.h"
#include "jobapi/joblist.h"
#include "jobapi/spumodule.h"

#include "icegraphics.h"
#include "icemesh.h"
#include "icelights.h"
#include "iceobjectbatch.h"
#include "iceeffects.h"
#include "icetextures.h"
#include "icedebugdraw.h"
#include "icequicktimer.h"
#include "icetimingbar.h"
#include "icepad.h"
#include "icerenderprocs.h"
#include "icedisasm.h"

#include <fstream>
#include <float.h>
#include <string.h>

#define PRINT_F32(a) printf(#a " = %f\n", a);
#define PRINT_I32(a) printf(#a " = %i\n", a);
#define PRINT_U32(a) printf(#a " = %d\n", a);

//! Insert some push buffer debug strings so we know whats going on.
#define PUSHBUFFER_DEBUGSTRING(a) Ice::Render::InsertDebugString(a)
//#define PUSHBUFFER_DEBUGSTRING(a)

#define NUM_MESH_JOBS 2048   // This must be a multiple of 16.

#define COMMAND_BUFFER_SIZE (4 * 1048576)     // Size of each command buffer in bytes.

#define SINGLE_STEP_GPU 0
#define VALIDATE_PUSH_BUFFER 1

using namespace SMath;
using namespace Ice;
using namespace Ice::Bucketer;
using namespace Ice::Graphics;
using namespace Ice::Render;
using namespace Ice::Mesh;
using namespace Ice::MeshProc;

namespace Ice
{
	namespace Graphics
	{
		// Memory space for job headers to live.
		static ICE_ALIGN(128) JobHeader s_meshJobListHeaders[2][NUM_MESH_JOBS];

		static SingleThreadJobList s_meshJobList0(s_meshJobListHeaders[0], NUM_MESH_JOBS * sizeof(JobHeader));
		static SingleThreadJobList s_meshJobList1(s_meshJobListHeaders[1], NUM_MESH_JOBS * sizeof(JobHeader));

		static SingleThreadJobList* s_meshJobList[2] = {
			&s_meshJobList0, &s_meshJobList1
		};

		// SPU module handle for the mesh processing SPU module.
		static SpuModuleHandle  s_meshJobModule;

		static CellSpurs        *s_pSpurs;
		static AuditManager     *s_pAuditManager;

		static RenderCamera     *s_renderCamera;

		static MultisampleMode  s_multiSampleMode;

		//! Frame statistics -- reset inside BeginRendering()
		static U64              s_renderedVertexCount;
		U64                     g_toGpuIndexCount;
		U64                     g_toSpuIndexCount;
		U64                     g_throughSpuIndexCount;

		//! Pointer to the mesh output buffer.
		//! This is split into two buffers for a double buffered scheme and multiple buffers for a ring buffer scheme.
		static void             *s_meshOutputBuffer;

		//! The mesh processing output buffer gets mapped into the GPU memory map, so we need the GPU offset.
		static U32              s_meshOutputBufferGpuOffset;

		//! The total size of each mesh processing output buffer (ring buffers are allocated from this).
		static U64              s_meshOutputBufferSize;

		//! Stores the difference between the address of the mesh processing output buffer and its GPU offset.
		//! This is needed in order to calculate the GPU offset of vertex data that is referenced in the command
		//! buffer and that has been stored in the mesh processing output buffer.
		//! This address will be subtracted from the actual output addresses.
		static U32              s_meshOutputBufferOffset;

		//! The command buffer is produced by the PPU on one frame and then comsumed by the GPU on the next frame,
		//! so it needs to be double buffered.  Mesh processing will patch the command buffer on either of those
		//! frames depending upon the buffering scheme being used.
		//! Memory for the command buffer gets allocated during initialization.
		static U8               *s_commandBuffer[2];

		//! Parameters for how to perform mesh processing.
		struct MeshParameters
		{
			U64  m_firstSpu;
			U64  m_numSpus;
			U64  m_outputBufferType;
			U64  m_outputBufferSize;
			U64  m_firstGpuSemaphoreIndex;
			bool m_doOnSpu;
		};

		//! Current set of mesh processing parameters (used for setting semaphores).
		static MeshParameters   s_meshParams;

		//! Previous set of mesh processing parameters (used for kicking mesh processing jobs).
		static MeshParameters   s_prevMeshParams;

		//! Reference value to use for testing whether drawing has finished.
		I32                     g_referenceValue = 1;

		//! This mesh output mutex is used to allocate memory from a single combined buffer for mesh processing output.
		//! This mutex is read and written to atomically by the SPU mesh processing jobs.
		static SingleBufferMutex ICE_ALIGN(128) s_meshOutputBufferMutex;

		//! The mesh output ring buffer alloctors store the information each SPU needs to allocate memory in each of
		//! their ring buffers.
		static RingBufferAllocators ICE_ALIGN(16) s_meshOutputRingBufferAllocators;

		//! Mutex used to sync mesh processing job completion with the GPU's put pointer.
		static GpuSyncMutex ICE_ALIGN(128) s_gpuSyncMutex;

		//! There is one primary render target. If multisampling is used, then the primary render target is
		//! allocated separately and rendered to, a texture is created pointing to the same memory, then a
		//! downsample is performed to the current display render target. Otherwise, the primary render target's
		//! color buffer just points to the current display render target's color buffer and no multisample
		//! resolve is performed.
		static Render::RenderTarget    s_primaryRenderTarget;
		static U32                     s_currentDisplayRenderTargetIndex;
		static Render::RenderTarget    s_displayRenderTarget[2];
		static Render::Texture         s_primaryRenderTargetTexture;

		//! These programs are used for downsampling the multisample buffer
		static Render::VertexProgram   *s_texture2dVertexProgram = NULL;
		static Render::FragmentProgram *s_textureFragmentProgram = NULL;

		//! This is a VRAM vertex buffer set up for drawing a full-screen textured quad (used for downsampling,
		//! possibly in future for other post-processing effects). It contains four vertexes, each in float
		//! xyzuv format.
		static void *                       s_fullScreenTexturedQuadVertexBuffer = NULL;

		void *g_commandBufferNext;
		Render::CommandContext g_commandContext;
		Render::CommandContext *g_commandContextStack[8];
		U32	g_commandContextStackIndex = 0;

		// Viewport information.
		static U32 s_viewportLeft;
		static U32 s_viewportTop;
		static U32 s_viewportWidth;
		static U32 s_viewportHeight;
		static Vector s_viewportScale;
		static Vector s_viewportTranslate;
		ICE_ALIGN(16) ViewportInfo g_viewportInfo;

		// Camera globals.
		U32 g_cameraLeft;
		U32 g_cameraTop;
		U32 g_cameraWidth;
		U32 g_cameraHeight;
		F32 g_cameraFocalLength;
		Point g_cameraWorldPosition;
		Transform g_cameraTransform;

		// The camera frustum.
		Vec4 g_frustumPlaneLeft;
		Vec4 g_frustumPlaneRight;
		Vec4 g_frustumPlaneTop;
		Vec4 g_frustumPlaneBottom;
		Vec4 g_frustumPlaneNear;
		Vec4 g_frustumPlaneFar;

		// Projection matrices.
		Mat44 g_projectionMatrix;
		Mat44 g_viewProjectionMatrix;
		Mat44 g_transposeViewProjectionMatrix;

		// Lights.
		U32 g_numPointLights = 0;
		U32 g_numDirectionalLights = 0;
		ShadowVolumeCastingPointLight **g_pPointLights = NULL;
		ShadowVolumeCastingDirectionalLight **g_pDirectionalLights = NULL;

		// Structure for storing info needed to kick the mesh processing jobs.
		static struct
		{
			SingleThreadJobList *m_pJobList;
			U32                 m_numJobs;
			U32                 m_outputBufferPtr;
			U32                 m_lastPutPtrValue;
		} s_meshKickInfo;

		// If a job list gets kicked then this flag will get set in order to wait for completion later.
		static bool s_waitForJobListEnd = false;
	}
}

// These are for the included binary of the icemesh SPU job program.
extern const U8   icemesh_spu_bin[];
extern const I32  icemesh_spu_bin_size;
extern U32        icemesh_spu_bin_entry;

// Insert inital semaphore values into the push buffer for ring buffer allocation.
// This needs to be done at a time when mesh processing is not being run.
static void SetSemaphoreForRingBuffer(CommandContext *commandContext, U32 outputBufferPtr)
{
	U64 numSpus = s_meshParams.m_numSpus;
	U64 firstGpuSemaphoreIndex = s_meshParams.m_firstGpuSemaphoreIndex;
	U64 ringBufferSize = (s_meshParams.m_outputBufferSize / numSpus) & ~0x7F;

	for (U32F iSpu = 0; iSpu < numSpus; iSpu++)
	{
		commandContext->SetFlushPipeSemaphoreIndex(iSpu + firstGpuSemaphoreIndex);
		commandContext->FlushTexturePipeAndWriteSemaphore(outputBufferPtr + iSpu * ringBufferSize);
	}
}

static void KickMeshJobList()
{
	// Get the data from the mesh kick info structure.
	SingleThreadJobList *pJobList = s_meshKickInfo.m_pJobList;
	U64 numJobs = s_meshKickInfo.m_numJobs;
	U32 outputBufferPtr = s_meshKickInfo.m_outputBufferPtr;
	U32 lastPutPtrValue = s_meshKickInfo.m_lastPutPtrValue;

	// Load the previous set of mesh processing parameters.
	U64 firstSpu = s_prevMeshParams.m_firstSpu;
	U64 numSpus = s_prevMeshParams.m_numSpus;
	bool useSync = s_prevMeshParams.m_outputBufferType != kMeshOutputDoubleBuffer;
	bool useRing = s_prevMeshParams.m_outputBufferType == kMeshOutputRingBuffersWithSync;
	U64 outputBufferSize = s_prevMeshParams.m_outputBufferSize;
	U64 firstGpuSemaphoreIndex = s_prevMeshParams.m_firstGpuSemaphoreIndex;
	bool doOnSpu = s_prevMeshParams.m_doOnSpu;

	// Setup the GPU sync mutex if using sync.
	if (useSync)
	{
		memset(&s_gpuSyncMutex, 0, sizeof(GpuSyncMutex));
		s_gpuSyncMutex.m_putPtrPtr = (U32)&g_gpuControl->m_putOffset;
		s_gpuSyncMutex.m_totalJobCount = numJobs;
		s_gpuSyncMutex.m_lastJobPutPtrValue = lastPutPtrValue;
	}

	// Generate ring buffer allocators if using ring buffers, otherwise generate the single buffer mutex.
	if (useRing)
	{
		// Setup the mesh output ring buffer allocators.
		memset(s_meshOutputRingBufferAllocators, 0xFF, 96);
		U32 semaphorePtr = (U32)GetSemaphoreAddress(firstGpuSemaphoreIndex);
		U64 ringBufferSize = (outputBufferSize / numSpus) & ~0x7F;
		for (U32F iSpu = 0; iSpu < numSpus; iSpu++)
		{
			s_meshOutputRingBufferAllocators[iSpu + firstSpu].m_startPtr = outputBufferPtr + iSpu * ringBufferSize;
			s_meshOutputRingBufferAllocators[iSpu + firstSpu].m_endPtr = outputBufferPtr + (iSpu + 1) * ringBufferSize;
			s_meshOutputRingBufferAllocators[iSpu + firstSpu].m_freeStartPtr = outputBufferPtr + iSpu * ringBufferSize;
			s_meshOutputRingBufferAllocators[iSpu + firstSpu].m_gpuSemaphorePtr = semaphorePtr + iSpu * 0x10;
		}
	}
	else if (useSync)
	{
		// Setup the mesh output buffer mutex for use with a single combined buffer.
		s_meshOutputBufferMutex.m_freeStartPtr = outputBufferPtr;
		s_meshOutputBufferMutex.m_endPtr = outputBufferPtr + outputBufferSize;
		s_meshOutputBufferMutex.m_sizeOfFailedAllocations = 0;
	}
	else
	{
		// Setup the mesh output buffer mutex for use with a single combined buffer.
		// Since this is a double buffered scheme, only half of the total space is available.
		s_meshOutputBufferMutex.m_freeStartPtr = outputBufferPtr;
		s_meshOutputBufferMutex.m_endPtr = outputBufferPtr + ((outputBufferSize >> 1) & ~0x7F);
		s_meshOutputBufferMutex.m_sizeOfFailedAllocations = 0;
	}

	if (doOnSpu)
	{
		// Attach the job list to Spurs.
		U8 priorities[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		for (U32F iSpu = firstSpu; iSpu < firstSpu + numSpus; iSpu++)
		{
			priorities[iSpu] = 4;
		}
		pJobList->AttachToSpurs(s_pSpurs, priorities, numSpus, s_pAuditManager);

		// Kick the job list.
		pJobList->SetReadyCount(numSpus);
		s_waitForJobListEnd = true;
	}
	else
	{
		// Get a pointer to the job headers.  NOTE: The first 32 bytes are taken up by the job list header, so skip them.
		JobHeader const *pJobListHeaders = ((JobHeader const *)pJobList->GetWQAddr()) + 4;

		for (U32F iJob = 0; iJob < numJobs; iJob++)
		{
			// Mix 'em up some.
			U32 jobNum;
			if (iJob < (numJobs / 21) * 21)
				jobNum = (iJob / 21) * 21 + 20 - (iJob % 21);
			else
				jobNum = iJob;

			UseBufferCommand const *pUseInputBufferCmd =
				(UseBufferCommand const *)(pJobListHeaders[jobNum].m_mmaLoadCommands) + 3;
			MeshProcessing((U32 *)(pUseInputBufferCmd->m_mmAddressInQwords << 4),
				pUseInputBufferCmd->m_flags.m_mmLengthInQwords << 4, (iJob % numSpus) + firstSpu, jobNum);
		}

		// Clear out the job list if we are running on the PPU:
		pJobList->ForcedFinish();

		//PrintMeshProcessingStats();
		ClearMeshProcessingStats();
	}

	// The current mesh processing parameters now become the previous ones.
	s_prevMeshParams = s_meshParams;
}

#if 0
// There seems to be a bug with the call to SwapCompleteCallback/
// In rare occasions, the callback does not occur, so it is disabled until this gets fixed.
volatile bool g_swapHasOccured = false;
volatile bool g_performedMeshKick = false;

void SwapCompleteCallback(U32)
{
	while (g_swapHasOccured) { /* Do nothing */ }

	if (s_meshKickInfo.m_lastPutPtrValue == 0xFFFFFFFF)
	{
		g_performedMeshKick = false;
		g_swapHasOccured = true;
		return;
	}

	// Start execution of the mesh processing jobs.
	KickMeshJobList();
	g_performedMeshKick = true;
	g_swapHasOccured = true;
}
#endif

// Allocate a 1k chunk of push buffer space from the current buffer
void *Ice::Graphics::AllocNextPushBuffer()
{
	void *chunk = g_commandBufferNext;
	g_commandBufferNext = (U8*)g_commandBufferNext + kPushBufferChunk + 4;
	ICE_ASSERT((U32)g_commandBufferNext < (U32)s_commandBuffer[s_currentDisplayRenderTargetIndex] + COMMAND_BUFFER_SIZE);
	return chunk;
}

bool Ice::Graphics::ReserveFailedCallback(Render::CommandContextData *context, U32 numWords)
{
	// Allocate the next chunk
	void *next = AllocNextPushBuffer();
	// Jump to it
	SetJumpAt(context->m_cmdptr, TranslateAddressToIoOffset(next));

	// Fix-up the context
	context->m_cmdptr = (U32*)next;
	context->m_endptr = context->m_cmdptr + kPushBufferChunk/sizeof(context->m_cmdptr[0]);

	ICE_ASSERT(numWords < (U32)(context->m_endptr - context->m_cmdptr));

	return (numWords < (U32)(context->m_endptr - context->m_cmdptr));
}

void Ice::Graphics::Initialize(Render::DisplayMode mode, U32 pitch, Render::ColorBufferFormat format,
		Render::MultisampleMode multisample, U64 meshOutputBufferSize, CellSpurs *pSpurs, AuditManager *pAuditManager)
{
	U32 defaultSize = 16*1024*1024;
	void *defaultBase = memalign(1024*1024, defaultSize);
	U32 usedDefaultSize = Initialize( mode, kOutModeArgb8888, pitch, defaultBase, defaultSize, g_referenceValue );
	InitializeMemoryAllocators((U8*)defaultBase+usedDefaultSize, defaultSize - usedDefaultSize, 
		g_gpuHardwareConfig.m_videoMemoryBaseAddr, g_gpuHardwareConfig.m_usableVideoMemorySize, 
		(Ice::Render::Region*)malloc(sizeof(Ice::Render::Region)*0x1000), 0x1000);

	// Allocate the buffers for Mesh Processing in video memory.
	ICE_ASSERT((meshOutputBufferSize & 0xFFFFF) == 0);
	s_meshOutputBuffer = memalign(1048576, meshOutputBufferSize);
	s_meshOutputBufferGpuOffset = Render::MapMainMemoryToIoOffset(s_meshOutputBuffer, meshOutputBufferSize);
	s_meshOutputBufferSize = meshOutputBufferSize;

	// Calculate the difference between the address of the mesh processing output buffer and its GPU offset.
	// This is needed in order to calculate the GPU offset of vertex data that is referenced in the command buffer and
	// that has been stored in the mesh processing output buffer.
	// NOTE: The high order bit is xored to indicate that the data is located in the main memory context.
	// This address will be subtracted from the actual output addresses.
	s_meshOutputBufferOffset = ((U32)s_meshOutputBuffer - s_meshOutputBufferGpuOffset) ^ kLocatorMainMemory;

	// Allocate space in video memory to store the command buffers.
	s_commandBuffer[0] = (U8 *)AllocateIoMemory(COMMAND_BUFFER_SIZE);
	s_commandBuffer[1] = (U8 *)AllocateIoMemory(COMMAND_BUFFER_SIZE);

	// Initialize command context
	g_commandBufferNext = s_commandBuffer[0];
	g_commandContext.Init(AllocNextPushBuffer(), kPushBufferChunk, &ReserveFailedCallback);
	PushContext(&g_commandContext);

	U32 width = 1280;
	U32 height = 720;

	// Initialize display render target 0 and register it for rendering.
	s_displayRenderTarget[0].Init(width, height, format, kBufferD24S8, kMultisampleNone, kTargetLinear);
	U32 displayColorBuffer0Pitch;
	void *displayColorBuffer0Address = Render::AllocateColorSurfaceVideoMemory(width, height, format, kMultisampleNone, &displayColorBuffer0Pitch);
	s_displayRenderTarget[0].SetColorNoDepthBuffers(TranslateAddressToOffset(displayColorBuffer0Address), kRenderTargetVideoMemory, displayColorBuffer0Pitch);
	RegisterRenderTargetForDisplay(&s_displayRenderTarget[0], 0);

	// Initialize display render target 1 and register it for rendering.
	s_displayRenderTarget[1].Init(width, height, format, kBufferD24S8, kMultisampleNone, kTargetLinear);
	U32 displayColorBuffer1Pitch;
	void *displayColorBuffer1Address = Render::AllocateColorSurfaceVideoMemory(width, height, format, kMultisampleNone, &displayColorBuffer1Pitch);
	s_displayRenderTarget[1].SetColorNoDepthBuffers(TranslateAddressToOffset(displayColorBuffer1Address), kRenderTargetVideoMemory, displayColorBuffer1Pitch);
	RegisterRenderTargetForDisplay(&s_displayRenderTarget[1], 1);

	// Initialize primary render target (this is the one the scene is rendered to). If we are
	// multisampling then we will down-sample from this to one of the display render targets.
	// Otherwise the color buffer pointer and pitch for this render target will later alternate
	// between those of the two display render targets.
	s_primaryRenderTarget.Init(width, height, format, kBufferD24S8, multisample, kTargetLinear);
	if (multisample != kMultisampleNone)
	{
		// If multisampling, need to set and allocate the primary render target's color buffer
		// since it will be distinct from those of the display render targets.
		U32 primaryColorBufferPitch, primaryDepthBufferPitch;
		void *primaryColorBufferAddress = Render::AllocateColorSurfaceVideoMemory(width, height, format, multisample, &primaryColorBufferPitch);
		void *primaryDepthBufferAddress = Render::AllocateDepthStencilSurfaceVideoMemory(width, height, kBufferD24S8, multisample, &primaryDepthBufferPitch);
		s_primaryRenderTarget.SetColorAndDepthBuffers(TranslateAddressToOffset(primaryColorBufferAddress), kRenderTargetVideoMemory, primaryColorBufferPitch,
													  TranslateAddressToOffset(primaryDepthBufferAddress), kRenderTargetVideoMemory, primaryDepthBufferPitch);
	}
	else
	{
		// If not multisampling, just allocate and set the primary render target's depth buffer.
		// Its color buffer will each frame be set to point to the color buffer of one of the
		// display render targets.
		U32 primaryDepthBufferPitch;
		void *primaryDepthBufferAddress = Render::AllocateDepthStencilSurfaceVideoMemory(width, height, kBufferD24S8, multisample, &primaryDepthBufferPitch);
		s_primaryRenderTarget.SetDepthNoColorBuffers(TranslateAddressToOffset(primaryDepthBufferAddress), kRenderTargetVideoMemory, primaryDepthBufferPitch);
	}

	// If we are multisampling, the primary render target texture is set to the color buffer
	// address and format of the primary render target. Otherwise it is left as an 'empty
	// texture' which should never be used.
	s_primaryRenderTargetTexture.Init();
	if (multisample != kMultisampleNone)
	{
		SetTextureFromRenderTargetColorBuffer(&s_primaryRenderTargetTexture, &s_primaryRenderTarget);
	}

	// These programs are used for downsampling the multisample buffer
	s_texture2dVertexProgram = LoadVertexProgram("shaders/texture2d.vbin");
	s_textureFragmentProgram = LoadFragmentProgram("shaders/texture.fbin");

	// Set up VRAM vertex buffer for drawing full-screen textured quads (for downsampling, etc.).
	s_fullScreenTexturedQuadVertexBuffer = AllocateLinearVideoMemory(256); // Memory allocated & copied padded to 256 bytes
	static float data[64];
	data[0]  = -1.0f; // x0
	data[1]  = -1.0f; // y0
	data[2]  = -1.0f; // z0
	data[3]  =  0.0f; // u0
	data[4]  =  1.0f; // v0
	data[5]  =  1.0f; // x1
	data[6]  = -1.0f; // y1
	data[7]  = -1.0F; // z1
	data[8]  =  1.0f; // u1
	data[9]  =  1.0f; // v1
	data[10] =  1.0f; // x2
	data[11] =  1.0f; // y2
	data[12] = -1.0F; // z2
	data[13] =  1.0f; // u2
	data[14] =  0.0f; // v2
	data[15] = -1.0f; // x3
	data[16] =  1.0f; // y3
	data[17] = -1.0F; // z3
	data[18] =  0.0f; // u3
	data[19] =  0.0f; // v3
	memcpy(s_fullScreenTexturedQuadVertexBuffer, data, 64*4);

	// Set primary render target
	s_currentDisplayRenderTargetIndex = 0;
	if (multisample == kMultisampleNone)
	{
		s_primaryRenderTarget.SetColorAndDepthBuffers(
			s_displayRenderTarget[0].m_colorOffset[0],
			kRenderTargetVideoMemory,
			s_displayRenderTarget[0].m_colorPitch[0],
			s_primaryRenderTarget.m_depthOffset,
			kRenderTargetVideoMemory,
			s_primaryRenderTarget.m_depthPitch);
	}

	s_multiSampleMode = multisample;
	s_viewportLeft = 0;
	s_viewportTop = 0;
	s_viewportWidth = s_primaryRenderTarget.GetWidth();
	s_viewportHeight = s_primaryRenderTarget.GetHeight();
	g_viewportInfo.m_4xRgmsFlag = (multisample == Render::kMultisample4XRotated);

	s_renderCamera = nullptr;

	// Initialize the point & directional lights
	g_numPointLights = 0;
	g_numDirectionalLights = 0;

	// Save the pointer to Spurs and the audit manager.
	s_pSpurs = pSpurs;
	s_pAuditManager = pAuditManager;

	// Initialize the mesh processing SPU module.
	s_meshJobModule = SpuModuleHandle(icemesh_spu_bin, (U32)icemesh_spu_bin_size, "icemesh");

	// Register end of swap callback in order to start mesh processing after vsync.
//	Render::SetSwapCompleteCallback(&SwapCompleteCallback);

	// Name the releavent timing bars (to be printed).
	NameTimingBar(kTimingBarGraphics, "Graphics");
}

void Ice::Graphics::Terminate(void)
{
	Render::Terminate();
}

void Ice::Graphics::BeginRendering(bool doMeshOnSpu, U64 firstSpuForMesh, U64 numSpusForMesh,
	U64 outputBufferTypeForMesh, U64 outputBufferSizeForMesh, U64 firstGpuSemaphoreIndexForMesh)
{
	static bool doneOnce = false;

	ICE_ASSERT(firstSpuForMesh + numSpusForMesh <= 6);
	ICE_ASSERT(outputBufferSizeForMesh <= s_meshOutputBufferSize);
	ICE_ASSERT(firstGpuSemaphoreIndexForMesh >= kNumReservedSemaphores);    // Some semaphores are reseverd by the OSD.
	ICE_ASSERT(firstGpuSemaphoreIndexForMesh + numSpusForMesh <= 256);

	Render::SetRenderTarget(&s_primaryRenderTarget);

	// Set up rendering state every frame
	Render::SetDepthCullControl(Render::kCullLess, Render::kCullMsb);
	Render::SetStencilCullHint(kFuncEqual, 0, 0xFF);

	if (s_multiSampleMode != kMultisampleNone)
	{
		Render::SetMultisampleParameters(true, false, false, 0xFFFF);
	}

	Render::EnableRenderState(kRenderCullFace);
	Render::SetCullFace(Render::kCullFaceBack);
	Render::SetFrontFace(Render::kFrontFaceCcw);
	Render::EnableRenderState(kRenderDepthTest);
	Render::SetDepthFunc(kFuncLess);
	Render::DisableRenderState(kRenderBlend);
	Render::DisableRenderState(kRenderAlphaTest);

	// Get the index used to determine which buffer we should use for the many things which are double buffered.
	U32 index = s_currentDisplayRenderTargetIndex;

	// Reset vertex and index counts for this frame.
	s_renderedVertexCount = 0;
	g_toGpuIndexCount = 0;
	g_toSpuIndexCount = 0;
	g_throughSpuIndexCount = 0;

	// Save the parameters to be used for the rest of the frame.
	s_meshParams.m_firstSpu = firstSpuForMesh;
	s_meshParams.m_numSpus = numSpusForMesh;
	s_meshParams.m_outputBufferType = outputBufferTypeForMesh;
	s_meshParams.m_outputBufferSize = outputBufferSizeForMesh;
	s_meshParams.m_firstGpuSemaphoreIndex = firstGpuSemaphoreIndexForMesh;
	s_meshParams.m_doOnSpu = doMeshOnSpu;

	// If this is the fisrt time through then the GPU semaphores must be initialized.
	// Normally these semaphores are written to by the execution of commands in the command buffer.  However.
	// these commands are added to the end of a command buffer for use by the next frame's kick of the mesh
	// processing jobs.  This is done because mesh processing can start before its related command buffer and
	// the semaphore must be set properly.  Thus, the semaphores must be set properly before the first kick of
	// mesh processing jobs and this is done here by writing directly to them.
	if (!doneOnce)
	{
		// Make sure the block of semaphores is 4KB aligned.
		ICE_ASSERT((U32(GetSemaphoreAddress(0)) & 0xFFF) == 0);

		// If using ring buffers, then setup the semaphores that are used with them.
		if (outputBufferTypeForMesh == kMeshOutputRingBuffersWithSync)
		{
			// Get the address of the block of GPU semaphores to use.
			U32 volatile * __restrict pSemaphore = GetSemaphoreAddress(firstGpuSemaphoreIndexForMesh);

			// Calculate the size of each ring buffer.
			U64 ringBufferSize = (outputBufferSizeForMesh / numSpusForMesh) & ~0x7F;

			// Write to the semaphores directly with the pointer to the start of each ring buffer.
			for (U32F iSpu = 0; iSpu < numSpusForMesh; iSpu++)
			{
				pSemaphore[iSpu * 4] = U32(s_meshOutputBuffer) + iSpu * ringBufferSize;
			}
		}

		// Set the previous mesh processing parameters to the current ones the first time through.
		s_prevMeshParams = s_meshParams;

		doneOnce = true;
	}

	// Reset all of the job data.
	bool useRing = s_prevMeshParams.m_outputBufferType == kMeshOutputRingBuffersWithSync;
	bool useSync = s_prevMeshParams.m_outputBufferType != kMeshOutputDoubleBuffer;
	if (useRing)
	{
		s_meshJobList[index]->ResetList();
		Bucketer::ResetJobData(&s_meshJobModule, s_meshJobList[index], kMeshOutputRingBuffers,
				s_meshOutputRingBufferAllocators, &s_gpuSyncMutex, s_meshOutputBufferOffset);
	}
	else if (useSync)
	{
		s_meshJobList[index]->ResetList();
		Bucketer::ResetJobData(&s_meshJobModule, s_meshJobList[index], kMeshOutputSingleBuffer,
				&s_meshOutputBufferMutex, &s_gpuSyncMutex, s_meshOutputBufferOffset);
	}
	else
	{
		s_meshJobList[index]->ResetList();
		Bucketer::ResetJobData(&s_meshJobModule, s_meshJobList[index], kMeshOutputSingleBuffer,
				&s_meshOutputBufferMutex, NULL, s_meshOutputBufferOffset);
	}

}

// Helper function to insert commands into the push buffer to downsample the current multisample buffer.
void Downsample(void)
{
	if (s_multiSampleMode == kMultisampleNone) return;

	// Set render state for downsampling
	RenderTarget *target = &s_displayRenderTarget[s_currentDisplayRenderTargetIndex];
	SetRenderTarget(target);
	SetViewport(0, 0, target->GetWidth(), target->GetHeight());
	SetScissor(0, 0, target->GetWidth(), target->GetHeight());
	SetMultisampleParameters(false, false, false, 0xFFFF);
	SetDepthMask(false);
	SetStencilMask(0x00);
	SetColorMask(true, true, true, true);
	DisableRenderState(kRenderAlphaTest);
	DisableRenderState(kRenderDepthTest);
	DisableRenderState(kRenderBlend);
	DisableRenderState(kRenderCullFace);
	DisableRenderState(kRenderStencilTestBack);
	DisableRenderState(kRenderStencilTestFront);
	SetVertexProgram(s_texture2dVertexProgram);
	SetFragmentProgram(s_textureFragmentProgram);

	// Set a texture with the same memory address and layout as the primary render target's color buffer.
	SetTexture(0, &s_primaryRenderTargetTexture);

	// Disable all unused vertex attributes.
	for (U32F iAttrib = 2; iAttrib < 16; iAttrib++)
	{
		DisableVertexAttribArray(iAttrib);
	}

	// Draw full-screen quad
	SetVertexAttribFormat(0, kAttribFloat, kAttribCount3, 20);
	SetVertexAttribPointer(0, TranslateAddressToOffset(s_fullScreenTexturedQuadVertexBuffer), kAttribVideoMemory);
	SetVertexAttribFormat(1, kAttribFloat, kAttribCount2, 20);
	SetVertexAttribPointer(1, TranslateAddressToOffset((char *) s_fullScreenTexturedQuadVertexBuffer + 12), kAttribVideoMemory);
	DrawArrays(kDrawQuads, 0, 4);

	// Restore default render states set in Initialize().
	Render::EnableRenderState(kRenderCullFace);
	Render::EnableRenderState(kRenderDepthTest);
}

#if 0
// base == Video memory base or Io Memory Base if RSX
static unsigned long InstrumentPushBuffer(void *start, void *base, unsigned long end)
{
	static unsigned int g_pushData[4*1024*1024];
	memset(g_pushData, 0, sizeof(g_pushData));

	unsigned int returnTo = 0;

	unsigned int *ncmd = (unsigned int*)g_pushData;
	unsigned int *cmd = (unsigned int*)start;
	unsigned long offset = (unsigned long)start - (unsigned long)base;
	unsigned int reference = 0;

	unsigned int putNext = 0;
	while(putNext != end)
	{
		unsigned int getValue = cmd[putNext/4];
		if(getValue & 0x20000000) // Jump Short
		{
			putNext = (getValue & ~0x20000000) - offset;
		}
		else if(getValue & 0x00000001) // Jump Long
		{
			putNext = (getValue & ~0x00000001) - offset;
		}
		else if(getValue & 0x00000002) // Call
		{
			returnTo = putNext + 4;
			putNext = (getValue & ~0x00000002) - offset;
		}
		else if(getValue & 0x00020000) // Return
		{
			putNext = returnTo;
			returnTo = 0;
		}
		else
		{
			bool noinc = getValue & 0x40000000;
			unsigned int num = (getValue >> 18) & 0x7FF;
			putNext += 4;

			if(getValue == 0)
			{
				ncmd[0] = 0;
				ncmd += 1;
			}

			unsigned long reg = getValue & 0xFFFC;
			for(U32F i = 0; i < num; ++i, putNext += 4)
			{
				unsigned int value = cmd[putNext/4];
				if(reg == 0x0050)
					reference = value;
				ncmd[0x00] = 0x00040000 | reg;
				ncmd[0x01] = value;
				ncmd[0x02] = 0x00041808;
				ncmd[0x03] = 0x00000005;
				ncmd[0x04] = 0x000C1500;
				ncmd[0x05] = 0x00000000;
				ncmd[0x06] = 0x00000000;
				ncmd[0x07] = 0x00000000;
				ncmd[0x08] = 0x000C1500;
				ncmd[0x09] = 0x00000000;
				ncmd[0x0A] = 0x00000000;
				ncmd[0x0B] = 0x00000000;
				ncmd[0x0C] = 0x000C1500;
				ncmd[0x0D] = 0x00000000;
				ncmd[0x0E] = 0x00000000;
				ncmd[0x0F] = 0x00000000;
				ncmd[0x10] = 0x00041808;
				ncmd[0x11] = 0x00000000;
				ncmd[0x12] = 0x00040050;
				ncmd[0x13] = reference++;
				ncmd += 0x14;
				reg += noinc ? 0 : 4;
			}
		}
	}

	unsigned long ncmdsize = (unsigned long)ncmd - (unsigned long)g_pushData;
	memcpy(start, g_pushData, ncmdsize);
	return ncmdsize;
}
#endif

#define EVAL2(x) #x
#define EVAL(x) EVAL2(x)

static void DumpPushBuffer(U32 *start, U32 size)
{
	//static int cnt = 0;
	if((     (g_padInfo[0].m_keyPressed & kPadL2)
	    &&  (g_padInfo[0].m_keyPressed & kPadStart)
	    )
		//|| (++cnt == 1)
		)
	{
		static int dumpNum = 0;
		char name[4096];
		sprintf(name, "/app_home/" EVAL(USERSTRING) "/%i_cmdlog.html", dumpNum);
		FILE *testFile = fopen(name, "w");
		if (testFile == NULL)
		{
			printf("Can't dump command log to %s: permission denied\n", name);
		}
		else
		{
			fclose(testFile);
			std::ofstream file(name);
			DumpCommandBuffer(file, start, size);
			printf("Dumped command log to %s\n", name);
			++dumpNum;
		}
	}
}

// The render sequence is as follows:
// NOTE: This is just one example of how to construct a render loop.
// 1st frame:
//   Build animation jobs on PPU.
//   Execute animation jobs on SPU (PPU stalls for this).
//   Build mesh processing jobs and command buffer on PPU.
//   If not using SPU/GPU synchronization, then run mesh processing jobs on SPU:
//     This patches the command buffer and outputs the results of mesh processing.
// 2nd frame:
//   If using SPU/GPU sunchrnoization, then run mesh processing jobs on SPU:
//     This patches the command buffer and outputs the results of mesh processing.
//   Execute command buffer on GPU.
// 3rd frame:
//   Display
//
// Two GPU command buffers are always required.
// If SPU/GPU synchronization is performed, then one mesh processing output buffer is required,
// otherwise two are required.
void Ice::Graphics::EndRendering()
{
	// Insert commands into the end of the push buffer to downsample the multisampled frame buffer.
	Downsample();

	static bool doneOnce = false;
	static U32 lastPutEnd = 0;
	static U32 lastCommandBufferSize = 0;
	static U32 lastCommandBufferEnd = 0;

	// Get the index used to determine which buffer we should use for the many things which are double buffered.
	U32 index = s_currentDisplayRenderTargetIndex;
	U32 otherIndex = index ^ 1;

	bool useSync = s_prevMeshParams.m_outputBufferType != kMeshOutputDoubleBuffer;

	LapTimingBar(kTimingBarGraphics);

	// Add the commands to the command context to setup the correct values in the semaphores for the next frame,
	// if ring buffers are being used.
	if (s_meshParams.m_outputBufferType == kMeshOutputRingBuffersWithSync)
		SetSemaphoreForRingBuffer(&g_commandContext, U32(s_meshOutputBuffer));

	// Save mesh job data for kicking during the swap complete callback (the callback is not used currently).
	s_meshKickInfo.m_pJobList = s_meshJobList[index];
	s_meshKickInfo.m_numJobs = s_meshJobList[index]->GetCurrentNumJobs();

	if (!useSync)
	{
		s_meshKickInfo.m_outputBufferPtr = U32(s_meshOutputBuffer) + index * ((s_meshOutputBufferSize >> 1) & ~0x7F);
		s_meshKickInfo.m_lastPutPtrValue = 0;

		// Start execution of the mesh processing jobs.
		KickMeshJobList();

		// Wait for the mesh processing jobs to finish.
		if (s_waitForJobListEnd)
		{
			s_meshJobList[index]->WaitForJobListEnd();
			s_meshJobList[index]->Shutdown();
			s_waitForJobListEnd = false;
		}
	}
	else
	{
		s_meshKickInfo.m_outputBufferPtr = U32(s_meshOutputBuffer);
	}

	U32 commandBufferEnd = (U32)g_commandContext.m_cmdptr - (U32)s_commandBuffer[index];
	U32 commandBufferSize = (U32)g_commandBufferNext - (U32)s_commandBuffer[index];

	if (doneOnce)
	{
		// Don't wait here...

		if (useSync)
		{
			// Wait for the mesh processing jobs to finish.
			if (s_waitForJobListEnd)
			{
				s_meshJobList[otherIndex]->WaitForJobListEnd();
				s_meshJobList[otherIndex]->Shutdown();
				s_waitForJobListEnd = false;
			}
		}

		LapTimingBar(kTimingBarGraphics);

		// Last command buffer.
		DumpPushBuffer((U32*)s_commandBuffer[otherIndex], lastCommandBufferSize);
#if SINGLE_STEP_GPU
		SingleStepPusher(lastPutEnd);
#endif

#if VALIDATE_PUSH_BUFFER
		U32 addr;
		U32 reg;
		U32 value;
		ValidationError error = ValidatePushBuffer(s_commandBuffer[otherIndex],
			s_commandBuffer[otherIndex] + lastCommandBufferEnd, &addr, &reg, &value);
		ICE_ASSERTF(error == kErrorNone, ("error=%i addr=%x reg=%x value=%x", error, addr, reg, value));
#endif

		g_gpuControl->m_putOffset = lastPutEnd;
	}
	lastCommandBufferEnd = commandBufferEnd;
	lastCommandBufferSize = commandBufferSize;

	// Wait for the push buffer to be consumed.
	LapTimingBar(kTimingBarGraphics);
	while(!Render::TestReference(g_referenceValue)) { /* Do nothing */ }

	//StopTimingBar(kTimingBarGraphics);
	ResetTimingBar(kTimingBarGraphics);
	StartTimingBar(kTimingBarGraphics);

	++g_referenceValue;

	// Kick the command buffer which was patched by Mesh Processing during the last frame.
	U32 commandBufferStartOffset = TranslateAddressToIoOffset(s_commandBuffer[index]);
#if SINGLE_STEP_GPU
	U32 putEnd = KickAndDisplayRenderTarget(commandBufferStartOffset,
		(U32 *)(U32(s_commandBuffer[index]) + commandBufferEnd), &s_displayRenderTarget[index], g_referenceValue, true);
#else
	U32 putEnd = KickAndDisplayRenderTarget(commandBufferStartOffset,
		(U32 *)(U32(s_commandBuffer[index]) + commandBufferEnd), &s_displayRenderTarget[index], g_referenceValue, useSync);
#endif

	// Save the new final put pointer value.
	s_meshKickInfo.m_lastPutPtrValue = putEnd;

	// Wait until the whole push buffer has been consumed.
//	while (g_gpuControl->m_putOffset != g_gpuControl->m_getOffset) { /* Do nothing */ }

	if (useSync)
		{
		// Start the GPU
		g_gpuControl->m_putOffset = commandBufferStartOffset;
	}

#if 0
	if (!doneOnce)
		SwapCompleteCallback(0);

	// Wait until the callback has occured.
	while (!g_swapHasOccured) { /* Do nothing */ }

	// If the callback did not start mesh processing, then do so here.
	if (!g_performedMeshKick)
	{
		KickMeshJobList();
	}

	s_meshKickInfo.m_lastPutPtrValue = 0xFFFFFFFF;
	g_swapHasOccured = false;
#endif
	if (useSync)
	{
		// Start mesh processing.
		KickMeshJobList();
	}

	LapTimingBar(kTimingBarGraphics);

	// Set statics for the next iteration.
	doneOnce = true;
	lastPutEnd = putEnd;

	// Flip buffers.
	s_currentDisplayRenderTargetIndex = otherIndex;

	// Initialize the command context and bind it with the default context.
	g_commandBufferNext = s_commandBuffer[otherIndex];
	g_commandContext.Init(AllocNextPushBuffer(), kPushBufferChunk, &ReserveFailedCallback);

	// Set the primary render target (if not multisampling its color buffer needs to be set to the next display buffer)
	if (s_multiSampleMode == kMultisampleNone)
	{
		s_primaryRenderTarget.SetColorAndDepthBuffers(
			s_displayRenderTarget[otherIndex].m_colorOffset[0], kRenderTargetVideoMemory, s_displayRenderTarget[otherIndex].m_colorPitch[0],
			s_primaryRenderTarget.m_depthOffset, kRenderTargetVideoMemory, s_primaryRenderTarget.m_depthPitch);
	}
	Render::SetRenderTarget(&s_primaryRenderTarget);

	// If on the current frame we are using a sync technique, but next frame we are not, then we need to wait for the
	// current frame to finish rendering, otherwise we may overwrite the mesh processing output buffer.
	// This is a special case that only occurs because the buffering scheme has been changed.
	if (useSync && s_meshParams.m_outputBufferType == kMeshOutputDoubleBuffer)
	{
		if (s_waitForJobListEnd)
		{
			s_meshJobList[index]->WaitForJobListEnd();
			s_meshJobList[index]->Shutdown();
			s_waitForJobListEnd = false;
		}
		while(!Render::TestReference(g_referenceValue)) { /* Do nothing */ }
	}
}



RenderCamera *Ice::Graphics::GetRenderCamera(void)
{
	return s_renderCamera;
}

// Calculate Frustum culling information
static void CalculateFrustumInformation()
{
	// Calculate the planes
	g_frustumPlaneLeft = Vec4(g_transposeViewProjectionMatrix.GetRow(3) + g_transposeViewProjectionMatrix.GetRow(0));
	g_frustumPlaneRight = Vec4(g_transposeViewProjectionMatrix.GetRow(3) - g_transposeViewProjectionMatrix.GetRow(0));
	g_frustumPlaneTop = Vec4(g_transposeViewProjectionMatrix.GetRow(3) - g_transposeViewProjectionMatrix.GetRow(1));
	g_frustumPlaneBottom = Vec4(g_transposeViewProjectionMatrix.GetRow(3) + g_transposeViewProjectionMatrix.GetRow(1));
	g_frustumPlaneNear = Vec4(g_transposeViewProjectionMatrix.GetRow(3) + g_transposeViewProjectionMatrix.GetRow(2));
	g_frustumPlaneFar = Vec4(g_transposeViewProjectionMatrix.GetRow(3) - g_transposeViewProjectionMatrix.GetRow(2));

	// Normalize the planes
	g_frustumPlaneLeft *= (F32)LengthRcp(Vector(g_frustumPlaneLeft));
	g_frustumPlaneRight *= (F32)LengthRcp(Vector(g_frustumPlaneRight));
	g_frustumPlaneTop *= (F32)LengthRcp(Vector(g_frustumPlaneTop));
	g_frustumPlaneBottom *= (F32)LengthRcp(Vector(g_frustumPlaneBottom));
	g_frustumPlaneNear *= (F32)LengthRcp(Vector(g_frustumPlaneNear));
	g_frustumPlaneFar *= (F32)LengthRcp(Vector(g_frustumPlaneFar));
}

// Sphere frustum culling
bool Ice::Graphics::IsSphereInsideFrustum(const Vec4 &sphere)
{
	Scalar const radius = sphere.W();

	// Do thorough plane tests
	Scalar const leftDist = Dot3(sphere, g_frustumPlaneLeft) + g_frustumPlaneLeft.W();
	if(leftDist < -radius)
		return false;

	Scalar const rightDist = Dot3(sphere, g_frustumPlaneRight) + g_frustumPlaneRight.W();
	if(rightDist < -radius)
		return false;

	Scalar const topDist = Dot3(sphere, g_frustumPlaneTop) + g_frustumPlaneTop.W();
	if(topDist < -radius)
		return false;

	Scalar const bottomDist = Dot3(sphere, g_frustumPlaneBottom) + g_frustumPlaneBottom.W();
	if(bottomDist < -radius)
		return false;

	Scalar const nearDist = Dot3(sphere, g_frustumPlaneNear) + g_frustumPlaneNear.W();
	if(nearDist < -radius)
		return false;

	/*
	  if((g_camera->GetCameraFlags() & kCameraInfinite) == 0)
	  {
	  Scalar const farDist = F32(Dot3(Vector(sphere), Vector(g_frustumPlaneFar)) + g_frustumPlaneFar.W());
	  if(farDist < -radius)
	  return false;
	  }
	*/

	return true;
}

void Ice::Graphics::SetRenderToTextureCamera(RenderToTextureCamera *camera)
{
	s_renderCamera = NULL;
	g_cameraWorldPosition = camera->m_transform.GetTranslation();

	const Transform& worldTransform = camera->m_transform;

	const Vector rightDirection = worldTransform.GetXAxis();
	const Vector downDirection = worldTransform.GetYAxis();
	const Vector viewDirection = worldTransform.GetZAxis();

	g_cameraTransform.SetXAxis(rightDirection);
	g_cameraTransform.SetYAxis(-downDirection);
	g_cameraTransform.SetZAxis(-viewDirection);

	g_cameraTransform.SetTranslation(worldTransform.GetTranslation());

	g_cameraTransform = Inverse(g_cameraTransform);

	// Set up the standard OpenGL projection matrix
	F32 element22, element32;
	const F32 eps = 0.001;
	if (camera->m_farDepth == POSITIVE_INFINITY)
	{
		// Set up an infinite projection matrix
		element22 = eps-1.0F;
		element32 = (eps-2.0F) * camera->m_nearDepth;
	}
	else
	{
		// Set up a standard view frustum with a finite far plane
		const F32 farDepth = camera->m_farDepth;
		const F32 nearDepth = camera->m_nearDepth;
		const F32 d = -1.0F / (farDepth - nearDepth);
		const F32 k = 2.0F * farDepth * nearDepth;
		element22 = (farDepth + nearDepth) * d;
		element32 = k * d;
	}
	const F32 focalLength = camera->m_focalLength;
	const F32 aspectRatio = camera->m_aspectRatio;
	g_projectionMatrix.SetRow(0, Vec4(focalLength, 0.0f, 0.0f, 0.0f));
	g_projectionMatrix.SetRow(1, Vec4(0.0F, focalLength / aspectRatio, 0.0F, 0.0F));
	g_projectionMatrix.SetRow(2, Vec4(0.0F, 0.0F, element22, -1.0F));
	g_projectionMatrix.SetRow(3, Vec4(0.0F, 0.0F, element32, 0.0F));

	g_viewProjectionMatrix = g_cameraTransform.GetMat44() * g_projectionMatrix;
	g_transposeViewProjectionMatrix = Transpose(g_viewProjectionMatrix);

	// Set the viewport and scissor to the whole viewport
	g_cameraLeft = camera->m_left;
	g_cameraTop = camera->m_top;
	g_cameraWidth = camera->m_width;
	g_cameraHeight = camera->m_height;
	g_cameraFocalLength = focalLength;

	const F32 w = F32(g_cameraWidth) * 0.5F;
	const F32 h = F32(g_cameraHeight) * 0.5F;

	s_viewportScale = Vector(w, -h, 0.5f);
	s_viewportTranslate = Vector(F32(g_cameraLeft) + w, F32(g_cameraTop) + h, 0.5f);

	Render::SetRenderTarget(&camera->m_renderTarget);
	Render::SetViewport(g_cameraLeft, g_cameraTop, g_cameraWidth, g_cameraHeight);
	Render::SetScissor(g_cameraLeft, g_cameraTop, g_cameraWidth, g_cameraHeight);

	// Calculate the frustum information necessary to cull
	CalculateFrustumInformation();

	// Viewport Scissor Information.
	g_viewportInfo.m_scissorArea[0] = U16(g_cameraLeft);
	g_viewportInfo.m_scissorArea[1] = U16(g_cameraLeft + g_cameraWidth);
	g_viewportInfo.m_scissorArea[2] = U16(g_cameraTop);
	g_viewportInfo.m_scissorArea[3] = U16(g_cameraTop + g_cameraHeight);
	g_viewportInfo.m_hdcToScreen[0] = s_viewportScale.X();
	g_viewportInfo.m_hdcToScreen[1] = s_viewportTranslate.X();
	g_viewportInfo.m_hdcToScreen[2] = s_viewportScale.Y();
	g_viewportInfo.m_hdcToScreen[3] = s_viewportTranslate.Y();

	// Depth Bounds (NV extension)
	g_viewportInfo.m_depthBounds[0] = 0.0F;
	g_viewportInfo.m_depthBounds[1] = 1.0F;

	g_rigidObjectTransform = NULL;
	Bucketer::OnRigidObjectTransformChange();
}

void Ice::Graphics::SetRenderCamera(RenderCamera *camera)
{
	s_renderCamera = camera;
	g_cameraWorldPosition = camera->m_transform.GetTranslation();

	const Transform& worldTransform = camera->m_transform;

	const Vector rightDirection = worldTransform.GetXAxis();
	const Vector downDirection = worldTransform.GetYAxis();
	const Vector viewDirection = worldTransform.GetZAxis();

	g_cameraTransform.SetXAxis(rightDirection);
	g_cameraTransform.SetYAxis(-downDirection);
	g_cameraTransform.SetZAxis(-viewDirection);

	g_cameraTransform.SetTranslation(worldTransform.GetTranslation());

	g_cameraTransform = Inverse(g_cameraTransform);

	// Set up the standard OpenGL projection matrix
	F32 element22, element32;
	const F32 eps = 0.001;
	if (camera->m_farDepth == POSITIVE_INFINITY)
	{
		// Set up an infinite projection matrix
		element22 = eps-1.0F;
		element32 = (eps-2.0F) * camera->m_nearDepth;
	}
	else
	{
		// Set up a standard view frustum with a finite far plane
		const F32 farDepth = camera->m_farDepth;
		const F32 nearDepth = camera->m_nearDepth;
		const F32 d = -1.0F / (farDepth - nearDepth);
		const F32 k = 2.0F * farDepth * nearDepth;
		element22 = (farDepth + nearDepth) * d;
		element32 = k * d;
	}
	const F32 focalLength = camera->m_focalLength;
	const F32 aspectRatio = camera->m_aspectRatio;
	g_projectionMatrix.SetRow(0, Vec4(focalLength, 0.0f, 0.0f, 0.0f));
	g_projectionMatrix.SetRow(1, Vec4(0.0F, focalLength / aspectRatio, 0.0F, 0.0F));
	g_projectionMatrix.SetRow(2, Vec4(0.0F, 0.0F, element22, -1.0F));
	g_projectionMatrix.SetRow(3, Vec4(0.0F, 0.0F, element32, 0.0F));

	g_viewProjectionMatrix = g_cameraTransform.GetMat44() * g_projectionMatrix;
	g_transposeViewProjectionMatrix = Transpose(g_viewProjectionMatrix);

	// Set the viewport and scissor to the whole viewport
	g_cameraLeft = s_viewportLeft;
	g_cameraTop = s_viewportTop;
	g_cameraWidth = s_viewportWidth;
	g_cameraHeight = s_viewportHeight;
	g_cameraFocalLength = focalLength;

	const F32 w = F32(g_cameraWidth) * 0.5F;
	const F32 h = F32(g_cameraHeight) * 0.5F;

	s_viewportScale = Vector(w, -h, 0.5f);
	s_viewportTranslate = Vector(F32(g_cameraLeft) + w, F32(g_cameraTop) + h, 0.5f);

	Render::SetRenderTarget(&s_primaryRenderTarget);
	Render::SetViewport(g_cameraLeft, g_cameraTop, g_cameraWidth, g_cameraHeight);
	Render::SetScissor(g_cameraLeft, g_cameraTop, g_cameraWidth, g_cameraHeight);

	// Calculate the frustum information necessary to cull
	CalculateFrustumInformation();

	// Viewport Scissor Information.
	g_viewportInfo.m_scissorArea[0] = (U16) s_viewportLeft;
	g_viewportInfo.m_scissorArea[1] = (U16) s_viewportLeft + s_viewportWidth;
	g_viewportInfo.m_scissorArea[2] = (U16) s_viewportTop;
	g_viewportInfo.m_scissorArea[3] = (U16) s_viewportTop + s_viewportHeight;
	g_viewportInfo.m_hdcToScreen[0] = s_viewportScale.X();
	g_viewportInfo.m_hdcToScreen[1] = s_viewportTranslate.X();
	g_viewportInfo.m_hdcToScreen[2] = s_viewportScale.Y();
	g_viewportInfo.m_hdcToScreen[3] = s_viewportTranslate.Y();

	// Depth Bounds (NV extension)
	g_viewportInfo.m_depthBounds[0] = 0.0F;
	g_viewportInfo.m_depthBounds[1] = 1.0F;

	g_rigidObjectTransform = NULL;
	Bucketer::OnRigidObjectTransformChange();
}

// Set the point lights that will be in use
void Ice::Graphics::SetShadowVolumeCastingPointLights(Bucketer::ShadowVolumeCastingPointLight **lights, const U32 count)
{
	g_pPointLights = lights;
	g_numPointLights = count;
	ICE_ASSERT(count <= 4);
	Bucketer::OnRigidObjectTransformChange();
}

// Set the directional lights that will be in use
void Ice::Graphics::SetShadowVolumeCastingDirectionalLights(Bucketer::ShadowVolumeCastingDirectionalLight **lights, const U32 count)
{
	g_pDirectionalLights = lights;
	g_numDirectionalLights = count;
	ICE_ASSERT(count <= 4);
	Bucketer::OnRigidObjectTransformChange();
}

// For debugging.  Grabs some per-frame statistics.
void Graphics::GetFrameStats(U64 *vertexCount, U64 *toGpuIndexCount, U64 *toSpuIndexCount, U64 *throughSpuIndexCount)
{
	*vertexCount = s_renderedVertexCount;
	*toGpuIndexCount = g_toGpuIndexCount;
	*toSpuIndexCount = g_toSpuIndexCount;
	*throughSpuIndexCount = g_throughSpuIndexCount;
}

