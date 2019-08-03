//--------------------------------------------------
//!
//!	\file gfx/renderer_ps3.cpp
//! implementation of the PS3 renderer
//!
//--------------------------------------------------

#include "gfx/renderer.h"

#include "gfx/rendercontext.h"
#include "gfx/display.h"
#include "gfx/pictureinpicture.h"
#include "gfx/surfacemanager.h"
#include "gfx/texturemanager.h"
#include "gfx/renderer_debug_settings_ps3.h"
#include "gfx/updateskin_ppu_spu.h"
#include "gfx/batchrender_ppu_spu.h"
#include "gfx/renderer_ppu_spu.h"
#include "effect/screensprite.h"

#include "core/visualdebugger.h"
#include "core/gfxmem.h"
#include "core/memman.h"
#include "core/timer.h"
#include "heresy/heresy_capi.h"

#define ALLOW_AUTO_PRESENT

// small helper install the visual debugger its actually in the visualdebugger_ps3.cpp file 
// so externed here
#ifndef _GOLD_MASTER
extern void RegisterPS3VisualDebugger();
#endif

extern void DestroyHeresyBuffers();

#ifdef COLLECT_SHADER_STATS
#include "input/inputhardware.h"


namespace
{

    class ShaderStats
    {
        struct ShaderListItem
        {
            ShaderListItem(Shader const* shader) : m_shader(shader), m_num(0)
            {
            }

            bool operator < (ShaderListItem const& other) const
            {
                return m_shader < other.m_shader;
            }

            static bool SortByFreq(ShaderListItem const& lhs, ShaderListItem const& rhs)
            {
                return lhs.m_num < rhs.m_num;
            }

            Shader const* GetShader() const
            {
                return m_shader;
            }

            unsigned int GetCount() const
            {
                return m_num;
            }

            Shader const* m_shader;
            mutable unsigned int m_num;
        };

        typedef ntstd::Set<ShaderListItem>      ShaderList;
        typedef ntstd::vector<ShaderListItem>   ShaderVector;


    private:
        void AddShader(ShaderList& list, Shader const* shader)
{
            ntstd::pair< ShaderList::iterator, bool > insertionResult = list.insert(ShaderListItem(shader));
            insertionResult.first -> m_num ++;
}

        void PrintListData(ShaderList const& list) const
        {
            ShaderVector out_vec(list.begin(), list.end());
            ntstd::sort(out_vec.begin(), out_vec.end(), &ShaderListItem::SortByFreq);
            
            for (ShaderVector::const_iterator iter = out_vec.begin(); iter != out_vec.end(); ++ iter)
            {
                if (iter -> GetShader() -> GetName())
                {
                    ntPrintf("%s  Count: %i \n", iter -> GetShader() -> GetName(), iter -> GetCount());
                }
                else
                {
                    ntPrintf("%s Index: %i Slot: %i Count: %i\n", iter -> GetShader() -> m_material -> GetTemplateName(), iter -> GetShader() -> m_index, iter -> GetShader() -> m_slot, iter -> GetCount());
                }
            }
        }

    public:
        void AddVertexShader(Shader const* shader)
        {
            AddShader(m_vertexShaders, shader);
        }

        void AddPixelShader(Shader const* shader)
        {
            AddShader(m_pixelShaders, shader);
        }

        void Reset()
        {
            m_vertexShaders.clear();
            m_pixelShaders.clear();
        }

        void PrintData() const
        {
            ntPrintf("==Vertex shaders======================\n");
            PrintListData(m_vertexShaders);
            ntPrintf("==Pixel shaders======================\n");
            PrintListData(m_pixelShaders);

        }

    private:
        ShaderList  m_vertexShaders;
        ShaderList  m_pixelShaders;

    };

    ShaderStats g_shaderStats;
}

void ResetShaderStats()
{
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F7 ) )
	{
        g_shaderStats.PrintData();
    }

    g_shaderStats.Reset();
}

#endif

static void GPUHangCallback( void* pLastKickedStart, void* pLastKickedEnd )
{
	char buffer[2048];
	sprintf( buffer, "/app_home/content_ps3/gpuhang.bin" );
	Renderer::Get().m_Platform.DumpPushBuffer( pLastKickedStart, pLastKickedEnd, buffer );
}

//-----------------------------------------------------
//!
//! Vertical blank ICE callback. allows us to swap
//!	vsync modes without tearing
//!
//-----------------------------------------------------
enum INTERUPT_COMMAND
{
	START_FRAME = 0,
	END_FRAME,
};

void Renderer::InteruptCallback(uint32_t command)
{
	if (command == START_FRAME)
		Renderer::Get().m_lGPUFrameTimer = CTimer::GetHWTimer();
	else if (command == END_FRAME)
		Renderer::Get().m_fGPUFrameTime = CTimer::GetElapsedSystemTimeSinceHere( Renderer::Get().m_lGPUFrameTimer );
}

//-----------------------------------------------------
//!
//! Initialises the renderer.
//!
//-----------------------------------------------------
Renderer::Renderer()
{
	m_Platform.m_pThis = this;

	RenderingContext::ms_pHeresyGlobal = (Heresy_GlobalData*) NT_ALLOC_CHUNK( Mem::MC_MISC, sizeof( Heresy_GlobalData ) );
	// set up heresy global data
	Heresy_InitGlobalData( RenderingContext::ms_pHeresyGlobal );

	// used to validate that GC and use are in step...
	void* pGcPtr;
	void* pMyPtr;
	UNUSED( pGcPtr );
	UNUSED( pMyPtr );

	SpuUpdateSkin::Initialise();
	BatchRenderer::Initialise();
	SpuRenderer::Initialise();


#ifdef _PROFILING
	m_Platform.m_pGPUReport = NT_NEW_CHUNK( Mem::MC_GFX ) GPUReport( 32, 4, 30 );
	m_Platform.m_pGPUReport->StartReports();
#endif

	// memory manager for XDDR pixel shaders (double buffered) currently 3 MB.
	DoubleEnderFrameAllocator_Initialise(	&RenderingContext::ms_pHeresyGlobal->m_PushBufferAllocator, 
											NT_MEMALIGN_CHUNK(Mem::MC_RSX_MAIN_INTERNAL, Mem::Heresy::HERESY_PUSH_BUFFER_SPACE, 0x100), 
											Mem::Heresy::HERESY_PUSH_BUFFER_SPACE );
	// cos of the current silly Gc Host memory system, we need to tell Gc that this ram is ours... this only works
	// if Gc hasn't already allocated any of th;s stuff. Tracker on it 
	pGcPtr = GcKernel::AllocateHostMemory( Mem::Heresy::HERESY_PUSH_BUFFER_SPACE );
	pMyPtr = (void*) DoubleEnderFrameAllocator_GetBaseAddress( &RenderingContext::ms_pHeresyGlobal->m_PushBufferAllocator );
	ntError( pGcPtr == pMyPtr );

	// reserve some RAM for the actual shader dictionary fragment shaders (so we can use UserMemory in non heresy mode and stop
	// Ice doing lots of little allocs.) This is now on-demand so its much much smaller (25Mb down to 1Mb)
	m_Platform.m_PixelShaderMainSpace.Initialise( NT_MEMALIGN_CHUNK(Mem::MC_RSX_MAIN_INTERNAL, Mem::Heresy::HERESY_SHADERDICT_SPACE, 0x100), Mem::Heresy::HERESY_SHADERDICT_SPACE );
	pGcPtr = GcKernel::AllocateHostMemory( Mem::Heresy::HERESY_SHADERDICT_SPACE );
	pMyPtr = (void*) m_Platform.m_PixelShaderMainSpace.GetBaseAddress();
	ntError( pGcPtr == pMyPtr );

#ifndef _GOLD_MASTER
	// Gc will allocate host memory here... so the RSX_MAIN allocator can't be used any later than here <sigh>
	// install the visual debugger, so things appear on screen
	RegisterPS3VisualDebugger();
#endif

	// take into account Gay GC memory grabbing. Store the pointer so we can free the chunk later on. [ARV].
	m_Platform.m_GCMemSpace = NT_ALLOC_CHUNK( Mem::MC_RSX_MAIN_INTERNAL, 1024 * 4 * 2 );

	// create the picture in picture sub system
	m_pPIPManager = NT_NEW_CHUNK( Mem::MC_GFX ) PIPManager();

	// fin
	m_eStandardCullMode = GFX_CULLMODE_EXPLICIT_CCW;
	m_Platform.PostPresent();

#ifdef _PROFILING
    GC_SET_METRICS_PERSISTENCY("VramAllocated", true);
    GC_SET_METRICS_PERSISTENCY("HostMemAllocated", true);

	m_Platform.m_ProfilePSSuspended = false;
	m_Platform.m_suspendedPS = DebugShaderCache::Get().LoadShader( "black_fp.sho" );
#endif

	GcKernel::RegisterTimeoutCallback( GPUHangCallback );

	// handly shader and geometry for fullscreen passes on PS3
	m_Platform.m_fullscreenVS = DebugShaderCache::Get().LoadShader( "fullscreen_vp.sho" );
	
	GcStreamField simpleQuadDesc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ); 
	float const simpleQuad[] = { -1.0f,  1.0f, 1.0f,  1.0f, 1.0f, -1.0f, -1.0f, -1.0f, };
	
	m_Platform.m_fullscreenData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 2, 1, &simpleQuadDesc );
	m_Platform.m_fullscreenData->Write( simpleQuad );

	// register a VBLANK callback and set defaults
	Ice::Render::SetVBlankStartCallback( VBlankCallback );
	Ice::Render::SetPpuInterruptCallback( InteruptCallback );

	m_presentMode = PM_AUTO;
	m_reqPresentMode = PM_AUTO;
	m_autoPresentMode = PM_IMMEDIATE;
	
	m_lGPUFrameTimer = 0;
	m_lCPUFrameTimer = 0;

	m_fGPUFrameTime = 0.0f;
	m_fCPUFrameTime = 0.0f;
}

//-----------------------------------------------------
//!
//! shutdown the renderer
//!
//-----------------------------------------------------
Renderer::~Renderer()
{
	// unregister our callbacks
	Ice::Render::SetVBlankStartCallback( 0 );
	Ice::Render::SetPpuInterruptCallback( 0 );

	GcKernel::RegisterTimeoutCallback( 0 );

	SpuRenderer::Destroy();
	BatchRenderer::Destroy();
	SpuUpdateSkin::Destroy();

	NT_FREE_CHUNK( Mem::MC_RSX_MAIN_INTERNAL, DoubleEnderFrameAllocator_GetBaseAddress( &RenderingContext::ms_pHeresyGlobal->m_PushBufferAllocator ) );
	NT_DELETE_CHUNK( Mem::MC_GFX, m_pPIPManager );

#ifndef _GOLD_MASTER
	// destroy our visual debugger
	NT_DELETE_CHUNK( Mem::MC_GFX, g_VisualDebug );
#endif

	if (m_Platform.m_pNullTexture)
		SurfaceManager::Get().ReleaseTexture( m_Platform.m_pNullTexture );

	// after this GcKernel will effectively be hosed, as our memory maps will be out of whack...
	GcKernel::FreeHostMemory( (void*)Mem::GetBaseAddress( Mem::MC_RSX_MAIN_USER ) );
	GcKernel::FreeHostMemory( (void*)m_Platform.m_PixelShaderMainSpace.GetBaseAddress() );
	NT_FREE_CHUNK( Mem::MC_RSX_MAIN_INTERNAL, m_Platform.m_PixelShaderMainSpace.GetBaseAddress() );
	GcKernel::FreeHostMemory( (void*)DoubleEnderFrameAllocator_GetBaseAddress( &RenderingContext::ms_pHeresyGlobal->m_PushBufferAllocator ) );


#ifdef _PROFILING
	if ( m_Platform.m_pGPUReport != NULL )
	{
		m_Platform.m_pGPUReport->~GPUReport();
		NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) m_Platform.m_pGPUReport );	
	}
#endif

	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t) RenderingContext::ms_pHeresyGlobal );
	RenderingContext::ms_pHeresyGlobal = 0;

	NT_FREE_CHUNK( Mem::MC_RSX_MAIN_INTERNAL, m_Platform.m_GCMemSpace );

	DestroyHeresyBuffers();
}

//-----------------------------------------------------
//!
//! Requests a change in the current present mode
//!
//-----------------------------------------------------
void Renderer::RequestPresentMode( PRESENT_MODE mode )
{
	m_reqPresentMode = mode;
}

//-----------------------------------------------------
//!
//! Vertical blank ICE callback. allows us to swap
//!	vsync modes without tearing
//!
//-----------------------------------------------------
void Renderer::Present()
{
	// we use 95% of the frame interval to swap to immediate before its too late
	float fVBlankInterval = (1.0f / CTimer::s_fGameRefreshRate) * 0.95f;
	float fFrameTime = ntstd::Max( m_fGPUFrameTime, m_fCPUFrameTime );

	if ( fFrameTime > fVBlankInterval)
		m_autoPresentMode = PM_IMMEDIATE;
	else
		m_autoPresentMode = PM_VBLANK;

	// swap the surfaces
	PROFILER_PAUSE;

	Renderer::Get().m_fCPUFrameTime = CTimer::GetElapsedSystemTimeSinceHere( Renderer::Get().m_lCPUFrameTimer );
	GcKernel::GetContext().InsertPpuInterrupt( END_FRAME );

	GcKernel::Present();

	GcKernel::GetContext().InsertPpuInterrupt( START_FRAME );
	Renderer::Get().m_lCPUFrameTimer = CTimer::GetHWTimer();
	
	PROFILER_RESUME;

	m_Platform.PostPresent();
}




//-----------------------------------------------------
//!
//! Vertical blank ICE callback. allows us to swap
//!	vsync modes without tearing
//!
//-----------------------------------------------------
void Renderer::VBlankCallback(uint32_t)
{
	if ( Renderer::Get().m_presentMode != Renderer::Get().m_reqPresentMode )
	{
		Renderer::Get().m_presentMode = Renderer::Get().m_reqPresentMode;

		if ( Renderer::Get().m_presentMode == PM_IMMEDIATE )
			GcKernel::SetDisplaySwapMode(Gc::kDisplaySwapModeHSync);
		else if ( Renderer::Get().m_presentMode == PM_VBLANK )
			GcKernel::SetDisplaySwapMode(Gc::kDisplaySwapModeVSync);
	}

#ifdef ALLOW_AUTO_PRESENT
	if ( Renderer::Get().m_presentMode == PM_AUTO )
	{
		if ( Renderer::Get().m_autoPresentMode == PM_IMMEDIATE )
			GcKernel::SetDisplaySwapMode(Gc::kDisplaySwapModeHSync);
		else if ( Renderer::Get().m_autoPresentMode == PM_VBLANK )
			GcKernel::SetDisplaySwapMode(Gc::kDisplaySwapModeVSync);
	}
#else
	if ( Renderer::Get().m_presentMode == PM_AUTO )
		GcKernel::SetDisplaySwapMode(Gc::kDisplaySwapModeHSync);
#endif
}

//-----------------------------------------------------
//!
//! Clears the current viewport.
//! takes memory of floating point render targets
//! casts to ARGB8 and fast clears that to black
//!
//-----------------------------------------------------
void Renderer::FastFloatClear( uint32_t iFlags, float fZValue, uint32_t iStencil )
{
#ifndef _RELEASE
	if ( iFlags & Gc::kColourBufferBit )
	{
		ntError_p( m_targetCache.GetPrimaryColourTarget(), ("Must have a render target before clearing it") );
		ntError_p( RendererPlatform::IsFloatBufferFormat( m_targetCache.GetPrimaryColourTarget()->m_Platform.GetGCFormat() ) == true,
					("Must use FastFloatClear() for floating point render targets only") );
	}
#endif

	// if you need a specific initialisation value, you'll have to write a full screen pass shader to do it.
	// Casting to an ARGB8 and using hardware clears is MUCH faster than using a pixel shader

	RenderTarget::Ptr colour0 = m_targetCache.GetColourTarget(0);
	RenderTarget::Ptr colour1 = m_targetCache.GetColourTarget(1);
	RenderTarget::Ptr colour2 = m_targetCache.GetColourTarget(2);
	RenderTarget::Ptr colour3 = m_targetCache.GetColourTarget(3);
	ZBuffer::Ptr depth = m_targetCache.GetDepthTarget();		

	uint32_t iBytesPerPixel = Gc::GetBytesPerPixel( colour0->m_Platform.GetGCFormat() );
	uint32_t width = colour0->GetWidth();
	uint32_t height = colour0->GetHeight();

	// scale number of scanlines to clear by the change in bytes per pixel
	switch (iBytesPerPixel)
	{
	case 4: break;
	case 8: height *= 2; break;
	case 16: height *= 4; break;
	default:
		ntError_p(0,("Unrecognised pixel byte size"));
	}

	RenderTarget::Ptr fake0, fake1, fake2, fake3, fakeDepth;
	
	RenderTarget::CreationStruct createParams0(
		colour0->m_Platform.GetRenderBuffer()->GetDataAddress(),
		colour0->m_Platform.GetRenderBuffer()->GetPitch(), 
		width, height, GF_ARGB8 );	
	fake0 = SurfaceManager::Get().CreateRenderTarget( createParams0 );

	if (colour1)
	{
		RenderTarget::CreationStruct createParams1( 
			colour1->m_Platform.GetRenderBuffer()->GetDataAddress(), 
			colour1->m_Platform.GetRenderBuffer()->GetPitch(), 
			width, height, GF_ARGB8 );	
		fake1 = SurfaceManager::Get().CreateRenderTarget( createParams1 );
	}

	if (colour2)
	{
		RenderTarget::CreationStruct createParams2(
			colour2->m_Platform.GetRenderBuffer()->GetDataAddress(),
			colour2->m_Platform.GetRenderBuffer()->GetPitch(),
			width, height, GF_ARGB8 );	
		fake2 = SurfaceManager::Get().CreateRenderTarget( createParams2 );
	}

	if (colour3)
	{
		RenderTarget::CreationStruct createParams3( 
			colour3->m_Platform.GetRenderBuffer()->GetDataAddress(), 
			colour3->m_Platform.GetRenderBuffer()->GetPitch(), 
			width, height, GF_ARGB8 );	
		fake3 = SurfaceManager::Get().CreateRenderTarget( createParams3 );
	}

	if (depth)
	{
		RenderTarget::CreationStruct createParamsZ( 
			depth->m_Platform.GetRenderBuffer()->GetDataAddress(), 
			depth->m_Platform.GetRenderBuffer()->GetPitch(), 
			width, height, GF_D24S8 );
		fakeDepth = SurfaceManager::Get().CreateRenderTarget( createParamsZ );
	}

	// set our fake targets and clear them
	m_targetCache.SetMultipleRenderTargets( fake0, fake1, fake2, fake3, fakeDepth );

	GcKernel::SetClearColour( 0,0,0,0 );
	GcKernel::SetClearDepthStencil( fZValue, iStencil );
	GcKernel::Clear( iFlags );

	// restore our old targets
	m_targetCache.SetMultipleRenderTargets( colour0, colour1, colour2, colour3, depth );
}

//-----------------------------------------------------
//!
//! Clears the current viewport.
//!
//-----------------------------------------------------
void Renderer::Clear(uint32_t iFlags, uint32_t iColour, float fZValue, uint32_t iStencil)
{
#ifndef _RELEASE
	if ( iFlags & Gc::kColourBufferBit )
	{
		ntError_p( m_targetCache.GetPrimaryColourTarget(), ("Must have a render target before clearing it") );
		ntError_p( RendererPlatform::IsFloatBufferFormat( m_targetCache.GetPrimaryColourTarget()->m_Platform.GetGCFormat() ) == false,
					("Must use FastFloatClear() for floating point rendertargets, or write your own clear code") );
	}
#endif

	if( iFlags & Gc::kColourBufferBit )
	{
		float r,g,b,a;
		NTCOLOUR_EXTRACT_FLOATS( iColour, r, g, b, a );
		GcKernel::SetClearColour( r,g,b,a );
	}
	
	GcKernel::SetClearDepthStencil( fZValue, iStencil );
	GcKernel::Clear( iFlags );
}

//-----------------------------------------------------
//!
//! Returns a floating point value representing the fraction of the available RAM
//! used by the current Direct3D device. 1.0f represents the amount of RAM defined
//! by the VRAM_LIMIT preprocessor macro.
//!
//-----------------------------------------------------
float	RendererPlatform::GetMemoryUsage() const
{
	float fResult = 0.0f;
	uint32_t iMemory = 0;
    GC_GET_METRICS("VramAllocated", iMemory);
	fResult = ( float )( ( double )( iMemory )/ ( double )VRAM_LIMIT );
	
/*
#ifdef TRACK_GFX_MEM
	uint32_t iTotal =	Renderer::ms_iVBAllocs +
						Renderer::ms_iIBAllocs +
						Renderer::ms_iDiskTex +
						Renderer::ms_iProcTex + 
						Renderer::ms_iRTAllocs;

	fResult = ( float )( ( double )( iTotal )/ ( double )VRAM_LIMIT );
#endif
*/
	return fResult;
}

//-----------------------------------------------------
//!
//! RendererPlatform::PostPresent()
//! Cleanup after present
//!
//----------------------------------------------------
void RendererPlatform::PostPresent()
{
#ifdef _PROFILING
	m_pThis->m_lastCount = m_pThis->m_renderCount;
	m_pThis->m_renderCount.Reset();
#endif

	FlushCaches();

	GcKernel::Enable( Gc::kPrimitiveRestart );
	GcKernel::SetPrimitiveRestartIndex( 0xffff );

	// we default to solid fill mode
	m_eFillMode = GFX_FILL_SOLID;
	DoubleEnderFrameAllocator_SwapAndResetAllocationDirection( &RenderingContext::ms_pHeresyGlobal->m_PushBufferAllocator );

	m_bForcePSRefresh = false;
	m_bStreamSet = false;

	// reset the device states
	m_pThis->SetDefaultRenderStates();
	
	// re-initialize SPU Batch Renderer
	BatchRenderer::PostPresent();
	SpuRenderer::PostPresent();
}

//-----------------------------------------------------
//!
//! RendererPlatform::FlushCaches()
//! Cleanup after present
//!
//----------------------------------------------------
void RendererPlatform::FlushCaches()
{
	// reset the vertex and pixel shader
	m_pThis->m_pCachedVertexShader = 0;
	m_pThis->m_pCachedPixelShader = 0;

	// reset the texture stage states
	ResetTextureStates();
}

//-----------------------------------------------------
//!
//! Set the current vertex shader.
//!
//----------------------------------------------------
void Renderer::SetVertexShader( Shader* pShader, bool bForce )
{
	if (pShader)
	{
		if ((bForce) || (m_pCachedVertexShader != pShader))
		{
			m_pCachedVertexShader = pShader;
			GcKernel::SetVertexShader( pShader->GetVertexHandle() );

			m_Platform.m_bStreamSet = false;

#ifdef COLLECT_SHADER_STATS
            g_shaderStats.AddVertexShader(pShader);
#endif
		}
	}
	else
	{
		GcShaderHandle empty;
		GcKernel::SetVertexShader( empty );

		m_Platform.m_bStreamSet = false;
		m_pCachedVertexShader = 0;
	}
}

//-----------------------------------------------------
//!
//! Set the current pixel shader.
//!
//----------------------------------------------------
void Renderer::SetPixelShader(Shader* pShader, bool bForce)
{
	if (pShader)
	{
#ifdef _PROFILING
		// debug overide of pixel shader for profiling
		if (m_Platform.m_ProfilePSSuspended)
		{
			m_pCachedPixelShader = pShader;
			GcKernel::SetFragmentShader( m_Platform.m_suspendedPS->GetPixelHandle() );
		}
		else
#endif
		if ((bForce) || (m_pCachedPixelShader != pShader))
		{
			m_pCachedPixelShader = pShader;
			GcKernel::SetFragmentShader( pShader->GetPixelHandle() );
#ifdef COLLECT_SHADER_STATS
            g_shaderStats.AddPixelShader(pShader);
#endif
		}
		else if (m_Platform.m_bForcePSRefresh)
		{
			GcKernel::RefreshFragmentShader( pShader->GetPixelHandle() );
//#ifdef COLLECT_SHADER_STATS
//            g_shaderStats.AddPixelShader(pShader);
//#endif
		}
	}
	else
	{
		GcShaderHandle empty;
		GcKernel::SetFragmentShader( empty );

		m_pCachedPixelShader = 0;
	}

	m_Platform.m_bForcePSRefresh = false;
}

//-----------------------------------------------------
//!
//! Sets the texture on a given sampler stage.
//!
//----------------------------------------------------
void Renderer::SetTexture(int iStage, const Texture::Ptr& pobTexture, bool bForce )
{
	if(pobTexture)
		m_Platform.SetTexture( iStage, pobTexture->m_Platform.GetTexture(), bForce );
	else
		SetTexture( iStage, Texture::NONE );
}

//-----------------------------------------------------
//!
//! Returns our black texture, allocates if not present
//!
//----------------------------------------------------
const GcTextureHandle& RendererPlatform::GetNullTexure()
{
	// assign our custom black texture
	if (!m_pNullTexture)
		m_pNullTexture = TextureManager::CreateProcedural2DTexture( 2, 2, GF_ARGB8, CVector(0,0,0,1) );

	ntError_p( m_pNullTexture, ("Failed to create null texture"));

	const TexturePlatform& nullTex = m_pNullTexture->m_Platform;
	return nullTex.GetTexture();
}	

//-----------------------------------------------------
//!
//! Clearsthe texture on a given sampler stage
//!
//----------------------------------------------------
void Renderer::SetTexture(int iStage, Texture::NONE_ENUM)
{
	m_Platform.SetTexture( iStage, m_Platform.GetNullTexure() );
}

//-----------------------------------------------------
//!
//! Sets the texture on a given sampler stage.
//!
//----------------------------------------------------
void RendererPlatform::SetTexture(int iStage, GcTextureHandle hTexture, bool bForce )
{
	ntAssert( 0 <= iStage && iStage < (int)MAX_SAMPLERS );
	ntAssert_p( hTexture, ("MUST always have a valid texture handle here now") );

	if( hTexture != m_aTextures[iStage] || (bForce == true) )
	{
		// from gc 1.2.2 we silently defer our texture stage set untill the 
		// draw call, as they've put in a very gay restriction on texture
		// stage setting only happeningin when GcKernel::SetTexture() is
		// called, something that we will remove in the future.

		m_iTexturesToSubmit |= (1 << iStage);
		//GcKernel::SetTexture( iStage, hTexture );

		m_aTextures[iStage] = hTexture;

		// we always invalidate our addressing and filtering modes here.
		m_aTexAddrMode[iStage] = TEXTUREADDRESS_UNKNOWN;
		m_aTexFilterModes[iStage] = TEXTUREFILTER_UNKNOWN;

		// gamma correction does not work on floating point textures, so we want to disable it..
		// unfortunately there's no method to get the current gamma correction state
		// so I must set it to "no correction" every time we want to set a floating point texture (Marco)
		Gc::TexFormat textureFormat = hTexture->GetFormat();
		if (textureFormat == Gc::kTexFormatRGBA16F	||
			textureFormat == Gc::kTexFormatRGBA32F	||
			textureFormat == Gc::kTexFormatGR16F	||
			textureFormat == Gc::kTexFormatR32F	)
			hTexture->SetGammaCorrect( 0 );
	}
}

//-----------------------------------------------------
//!
//! Sets the texture address mode for the given sampler stage. Redundant calls to
//! this function have minimal performance penalty.
//!
//----------------------------------------------------
void Renderer::SetSamplerAddressMode(int iStage, int iAddressMode)
{
	ntAssert(0 <= iStage && iStage < (int)RendererPlatform::MAX_SAMPLERS);

	TEXTUREADDRESS_TYPE eMode = (TEXTUREADDRESS_TYPE)iAddressMode;

	if(eMode != m_Platform.m_aTexAddrMode[iStage])
	{
		ntAssert_p( m_Platform.m_aTextures[iStage], ("No bound texture on stage %d to set address mode on.", iStage) );

		switch (eMode)
		{
		case TEXTUREADDRESS_CLAMPALL:
			m_Platform.m_aTextures[iStage]->SetWrapS( Gc::kWrapModeClampToEdge );
			m_Platform.m_aTextures[iStage]->SetWrapT( Gc::kWrapModeClampToEdge );
			m_Platform.m_aTextures[iStage]->SetWrapR( Gc::kWrapModeClampToEdge );
			break;

		case TEXTUREADDRESS_WRAPALL:
			m_Platform.m_aTextures[iStage]->SetWrapS( Gc::kWrapModeRepeat );
			m_Platform.m_aTextures[iStage]->SetWrapT( Gc::kWrapModeRepeat );
			m_Platform.m_aTextures[iStage]->SetWrapR( Gc::kWrapModeRepeat );
			break;

		case TEXTUREADDRESS_BORDERALL:
			m_Platform.m_aTextures[iStage]->SetWrapS( Gc::kWrapModeClampToBorder );
			m_Platform.m_aTextures[iStage]->SetWrapT( Gc::kWrapModeClampToBorder );
			m_Platform.m_aTextures[iStage]->SetWrapR( Gc::kWrapModeClampToBorder );
			break;

		case TEXTUREADDRESS_WCC:
			m_Platform.m_aTextures[iStage]->SetWrapS( Gc::kWrapModeRepeat );
			m_Platform.m_aTextures[iStage]->SetWrapT( Gc::kWrapModeClampToEdge );
			m_Platform.m_aTextures[iStage]->SetWrapR( Gc::kWrapModeClampToEdge );
			break;

		default:
			ntAssert_p(0,("Unrecognised texture address mode: %s\n", eMode));
			break;
		}

		m_Platform.m_aTexAddrMode[iStage] = eMode;
	}
}



//-----------------------------------------------------
//!
//! Sets the texture filter mode for the given sampler stage. Redundant calls to
//! this function have minimal performance penalty.
//!
//----------------------------------------------------
void Renderer::SetSamplerFilterMode(int iStage, int iFilterMode)
{
	ntAssert(0 <= iStage && iStage < (int)RendererPlatform::MAX_SAMPLERS);

	TEXTUREFILTER_TYPE eMode = (TEXTUREFILTER_TYPE)iFilterMode;

	if(eMode != m_Platform.m_aTexFilterModes[iStage])
	{
		ntAssert_p( m_Platform.m_aTextures[iStage], ("No bound texture on stage %d to set filter mode on.", iStage) );

		switch (eMode)
		{
		case TEXTUREFILTER_POINT:
			m_Platform.m_aTextures[iStage]->SetFilter( Gc::kFilterNearest, Gc::kFilterNearest );
			break;

		case TEXTUREFILTER_BILINEAR:
			ntAssert_p( !RendererPlatform::Is32bitFloatFormat( m_Platform.m_aTextures[iStage]->GetFormat() ), ("Bilinear filtering not supported on float32 textures") );
			m_Platform.m_aTextures[iStage]->SetFilter( Gc::kFilterLinear, Gc::kFilterLinearMipMapNearest );
			break;

		case TEXTUREFILTER_TRILINEAR:
			ntAssert_p( !RendererPlatform::Is32bitFloatFormat( m_Platform.m_aTextures[iStage]->GetFormat() ), ("Trilinear filtering not supported on float32 textures") );
			m_Platform.m_aTextures[iStage]->SetFilter( Gc::kFilterLinear, Gc::kFilterLinearMipMapLinear );
			break;

		default:
			ntAssert_p(0,("Unrecognised texture filter mode: %s\n", eMode));
			break;
		}

		m_Platform.m_aTexFilterModes[iStage] = eMode;
	}
}

Gc::AnisotropyLevel AnisotropicFilterType( int iLevel )
{
	ntAssert( iLevel >= 0 );

	int tmp = 0;

	if ( int(CRendererSettings::eAnisotropicFilterQuality) > 0 && iLevel > 0 ) 
	{
		int offset = ( CRendererSettings::eAnisotropicFilterQuality - 1 ) * 4;
		tmp = iLevel + offset;
	}

	return  Gc::AnisotropyLevel( tmp );
}


void Renderer::SetAnisotropicFilterLevel( int iStage, int iLevel )
{
	ntAssert(0 <= iStage && iStage < (int)RendererPlatform::MAX_SAMPLERS);
	
	//! get the filter settings
	Gc::AnisotropyLevel eLevel = AnisotropicFilterType( iLevel );
	if ( eLevel != m_Platform.m_aTexAnisotropicFilterLevel[iStage] )
	{
		ntAssert_p( m_Platform.m_aTextures[iStage], ("No bound texture on stage %d to set address mode on.", iStage) );

		ntAssert_p( !RendererPlatform::Is32bitFloatFormat( m_Platform.m_aTextures[iStage]->GetFormat() ), ("Anisotropic filtering not supported on float32 textures") );
		m_Platform.m_aTextures[iStage]->SetMaxAnisotropy( eLevel );
		m_Platform.m_aTexAnisotropicFilterLevel[iStage] = eLevel;

		ntPrintf( "\n stage %i, set to %i \n", iStage, eLevel );
	}
}	

//-----------------------------------------------------
//!
//!	Send our custom blend mode to the GFX device
//!
//----------------------------------------------------
void Renderer::SetBlendMode( GFX_BLENDMODE_TYPE eMode )
{
	// commit the blend mode
	switch(eMode)
	{
	case GFX_BLENDMODE_OVERWRITE:
		// overwrite all channels
		GcKernel::SetColourMask(true, true, true, true);
		GcKernel::Disable( Gc::kBlend );
		break;

	case GFX_BLENDMODE_LERP:
		// lerp all channels (on source alpha)
		GcKernel::SetColourMask(true, true, true, false);
		GcKernel::Enable( Gc::kBlend );
		GcKernel::SetBlendEquation( Gc::kBlendEquationAdd );
		GcKernel::SetBlendFunc(	Gc::kBlendSrcAlpha, Gc::kBlendOneMinusSrcAlpha );
		break;

	case GFX_BLENDMODE_ADD:
		// add all channels
		GcKernel::SetColourMask(true, true, true, false);
		GcKernel::Enable( Gc::kBlend );
		GcKernel::SetBlendEquation( Gc::kBlendEquationAdd );
		GcKernel::SetBlendFunc(	Gc::kBlendOne, Gc::kBlendOne );
		break;

	case GFX_BLENDMODE_SUB:
		// add sub channels
		GcKernel::SetColourMask(true, true, true, false);
		GcKernel::Enable( Gc::kBlend );
		GcKernel::SetBlendEquation( Gc::kBlendEquationRevSub );
		GcKernel::SetBlendFunc(	Gc::kBlendOne, Gc::kBlendOne );
		break;

	case GFX_BLENDMODE_ADD_SRCALPHA:
		// modulate src by src alpha, add to dest
		GcKernel::SetColourMask(true, true, true, false);
		GcKernel::Enable( Gc::kBlend );
		GcKernel::SetBlendEquation( Gc::kBlendEquationAdd );
		GcKernel::SetBlendFunc(	Gc::kBlendSrcAlpha, Gc::kBlendOne );
		break;

	case GFX_BLENDMODE_SUB_SRCALPHA:
		// modulate src by src alpha, subract from dest
		GcKernel::SetColourMask(true, true, true, false);
		GcKernel::Enable( Gc::kBlend );
		GcKernel::SetBlendEquationSeparate( Gc::kBlendEquationAdd, Gc::kBlendEquationRevSub );
		GcKernel::SetBlendFuncSeparate(	Gc::kBlendSrcAlpha, Gc::kBlendOne,
										Gc::kBlendSrcAlpha, Gc::kBlendOne );
		break;

	case GFX_BLENDMODE_DISABLED:
		// writes nothing
		GcKernel::SetColourMask(false, false, false, false);
		GcKernel::Disable( Gc::kBlend );
		break;

	default:
		ntAssert_p( false, ( "Unknown blend mode %d set", eMode ) );
	}
}

//-----------------------------------------------------
//!
//!	Send our custom zbuffer mode to the GFX device
//!
//----------------------------------------------------
void Renderer::SetZBufferMode( GFX_ZMODE_TYPE eMode )
{
	switch(eMode)
	{
	case GFX_ZMODE_DISABLED:
		GcKernel::Disable( Gc::kDepthTest );
		GcKernel::SetDepthTest( Gc::kAlways );
		GcKernel::SetDepthMask( false );
		break;

	case GFX_ZMODE_LESSEQUAL:
		GcKernel::Enable( Gc::kDepthTest );
		GcKernel::SetDepthTest( Gc::kLEqual );
		GcKernel::SetDepthMask( true );
		break;

	case GFX_ZMODE_LESSEQUAL_READONLY:
		GcKernel::Enable( Gc::kDepthTest );
		GcKernel::SetDepthTest( Gc::kLEqual );
		GcKernel::SetDepthMask( false );
		break;

	default:
		ntAssert_p(false, ("Unknown z mode %d set", eMode));
	}

	m_cachedZMode = eMode;
}

//-----------------------------------------------------
//!
//!	Send our culling mode to the CPU
//!
//----------------------------------------------------
void Renderer::SetCullMode( GFX_CULLMODE_TYPE eMode )
{	
	// pick the cull mode based on the current standard cull mode
	if(eMode == GFX_CULLMODE_NORMAL)
		eMode = m_eStandardCullMode;
	else if(eMode == GFX_CULLMODE_REVERSED)
		eMode = (GFX_CULLMODE_TYPE)(1 - (int)m_eStandardCullMode);

	// commit the cullmode
	switch(eMode)
	{
	case GFX_CULLMODE_NONE:
		GcKernel::Disable( Gc::kCullFace );
	break; 

	case GFX_CULLMODE_EXPLICIT_CCW:
		GcKernel::Enable( Gc::kCullFace );
		GcKernel::SetCullFace( Gc::kCullFront );
		break;

	case GFX_CULLMODE_EXPLICIT_CW:
		GcKernel::Enable( Gc::kCullFace );
		GcKernel::SetCullFace( Gc::kCullBack );
		break;

	default:
		ntAssert_p(false, ("Unknown cull mode %d set", eMode));
	}

	m_cachedCullMode = eMode;
}

//-----------------------------------------------------
//!
//!	Send our culling mode to the CPU
//!
//----------------------------------------------------
void Renderer::SetAlphaTestModeN( GFX_ALPHA_TEST_MODE eMode, float fRef )
{
	switch( eMode )
	{
	case GFX_ALPHATEST_NONE:
		GcKernel::Disable(Gc::kAlphaTest);
		break;

	case GFX_ALPHATEST_EQUAL:
		GcKernel::Enable(Gc::kAlphaTest);
		GcKernel::SetAlphaFunc(Gc::kEqual, fRef);
		break;

	case GFX_ALPHATEST_NOTEQUAL:
		GcKernel::Enable(Gc::kAlphaTest);
		GcKernel::SetAlphaFunc(Gc::kNotEqual, fRef);
		break;

	case GFX_ALPHATEST_LESS:
		GcKernel::Enable(Gc::kAlphaTest);
		GcKernel::SetAlphaFunc(Gc::kLess, fRef);
		break;

	case GFX_ALPHATEST_LESSEQUAL:
		GcKernel::Enable(Gc::kAlphaTest);
		GcKernel::SetAlphaFunc(Gc::kLEqual, fRef);
		break;

	case GFX_ALPHATEST_GREATER:
		GcKernel::Enable(Gc::kAlphaTest);
		GcKernel::SetAlphaFunc(Gc::kGreater, fRef);
		break;

	case GFX_ALPHATEST_GREATEREQUAL:
		GcKernel::Enable(Gc::kAlphaTest);
		GcKernel::SetAlphaFunc(Gc::kGEqual, fRef);
		break;

	case GFX_ALPHATEST_ALWAYS:
		GcKernel::Enable(Gc::kAlphaTest);
		GcKernel::SetAlphaFunc(Gc::kAlways, fRef);
		break;

	default:
		ntAssert_p(false, ("Unknown alpha test mode %d set", eMode));
	}
}

void Renderer::SetAlphaTestMode( GFX_ALPHA_TEST_MODE eMode, int iRef )
{
	SetAlphaTestModeN( eMode, _R(iRef) / 255.0f );
}

//-----------------------------------------------------
//!
//!	Send our fill mode
//!
//----------------------------------------------------
void Renderer::SetFillMode( GFX_FILL_MODE eMode )
{
	ntAssert_p(	(eMode == GFX_FILL_SOLID) ||
				(eMode == GFX_FILL_POINT) ||
				(eMode == GFX_FILL_WIREFRAME), ("Unknown fill mode %d set", eMode));

	m_Platform.m_eFillMode = eMode;
}

//-----------------------------------------------------
//!
//!	Toggle pointsprite renderstate
//!
//----------------------------------------------------
void Renderer::SetPointSpriteEnable( bool bEnabled )
{
	if (bEnabled)
	{
		GcKernel::Enable( Gc::kVertexProgramPointSize );
		GcKernel::SetPointSpriteParameters( true, 0x1, Gc::kSpriteZero );
	}
	else
	{
		GcKernel::SetPointSpriteParameters( false, 0, Gc::kSpriteZero );
		GcKernel::Disable( Gc::kVertexProgramPointSize );
	}
}

//-----------------------------------------------------
//!
//! Sets the current scissoring region
//!
//----------------------------------------------------
void Renderer::SetScissorRegion( const ScissorRegion* pRegion )
{
	ntAssert(pRegion);

	// note, assumes usual negative Y space. This may be incorrect... oh, for some docs...
	GcKernel::SetScissor( 
		pRegion->left,
		pRegion->top,
		pRegion->right - pRegion->left, 
		pRegion->bottom - pRegion->top );
}

//-----------------------------------------------------
//!
//! Unsets all textures.
//!
//----------------------------------------------------
void RendererPlatform::ResetTextureStates()
{
	m_iTexturesToSubmit = 0;

	// reset all texture stage states
	GcTextureHandle nil;

	for(u_int iStage = 0; iStage < MAX_SAMPLERS; ++iStage)
	{
		m_aTextures[iStage] = nil;
		m_aTexAddrMode[iStage] = TEXTUREADDRESS_UNKNOWN;
		m_aTexFilterModes[iStage] = TEXTUREFILTER_UNKNOWN;
	}
}

//-----------------------------------------------------
//!
//! RendererPlatform::DrawPrimitives
//! Wrappered call to Gc::DrawArrays
//!
//----------------------------------------------------
void RendererPlatform::DrawPrimitives( Gc::PrimitiveType primType, u_int iStartVertex, u_int iVertexCount )
{
#ifdef _PROFILING
	// record draw calls made and poly count to date
	switch(primType)
	{
		// dont increment polys drawn at all
		case Gc::kPoints:
		case Gc::kLineLoop:
		case Gc::kLines:
		case Gc::kLineStrip:
			m_pThis->m_renderCount.AddVertList(iVertexCount);
			break;

		case Gc::kTriangles:
			m_pThis->m_renderCount.AddTriList(iVertexCount);
			break;

		// polys increment the same with these prims
		case Gc::kTriangleStrip:
		case Gc::kTriangleFan:
		case Gc::kQuadStrip:
			m_pThis->m_renderCount.AddTriStrip(iVertexCount);
			break;

		case Gc::kQuads:
			m_pThis->m_renderCount.AddQuadList(iVertexCount);
			break;
			
		case Gc::kPolygon:
			m_pThis->m_renderCount.AddPolygon(iVertexCount);
			break;
	}
#endif

	if (m_eFillMode != GFX_FILL_SOLID)
	{
		switch(primType)
		{
		case Gc::kPoints:
		case Gc::kLines:
		case Gc::kLineLoop:
		case Gc::kLineStrip:
			break;

		// all other primtypes, check for Renderstate or debug overide
		default:
			{
				if (m_eFillMode == GFX_FILL_WIREFRAME)
					primType = Gc::kLineStrip;

				if (m_eFillMode == GFX_FILL_POINT)
					primType = Gc::kPoints;
			}
		}
	}

	// defered submit of our textures
	SubmitTextures();

	ntAssert_p( m_bStreamSet, ("Must have streams bound before calling draw. SetVertexShader() unbinds the streams. Did you fail to call SetStream() again?") );
	ntAssert_p( m_pThis->m_pCachedVertexShader, ("Must have a valid vertex shader before calling DrawPrimitives()") );
	ntAssert_p( m_pThis->m_pCachedPixelShader, ("Must have a valid pixel shader before calling DrawPrimitives()") );

	GcKernel::DrawArrays( primType, iStartVertex, iVertexCount );
}

//-----------------------------------------------------
//!
//! RendererPlatform::DrawIndexedPrimitives
//! Wrappered call to Gc::DrawElements
//!
//----------------------------------------------------
void RendererPlatform::DrawIndexedPrimitives( Gc::PrimitiveType primType, u_int iStartIndex, u_int iIndexCount, const IBHandle& hIndices )
{
#ifdef _PROFILING
	// record draw calls made and poly count to date
	switch(primType)
	{
		// dont increment polys drawn at all
		case Gc::kPoints:
		case Gc::kLineLoop:
		case Gc::kLines:
		case Gc::kLineStrip:
			m_pThis->m_renderCount.AddVertList(iIndexCount);
			break;

		case Gc::kTriangles:
			m_pThis->m_renderCount.AddTriList(iIndexCount);
			break;

		// polys increment the same with these prims
		case Gc::kTriangleStrip:
		case Gc::kTriangleFan:
		case Gc::kQuadStrip:
			m_pThis->m_renderCount.AddTriStrip(iIndexCount);
			break;

		case Gc::kQuads:
			m_pThis->m_renderCount.AddQuadList(iIndexCount);
			break;
			
		case Gc::kPolygon:
			m_pThis->m_renderCount.AddPolygon(iIndexCount);
			break;
	}
#endif

	if (m_eFillMode != GFX_FILL_SOLID)
	{
		switch(primType)
		{
		case Gc::kPoints:
		case Gc::kLines:
		case Gc::kLineLoop:
		case Gc::kLineStrip:
			break;

		// all other primtypes, check for Renderstate or debug overide
		default:
			{
				if (m_eFillMode == GFX_FILL_WIREFRAME)
					primType = Gc::kLineStrip;

				if (m_eFillMode == GFX_FILL_POINT)
					primType = Gc::kPoints;
			}
		}
	}

	// defered submit of our textures
	SubmitTextures();

	ntAssert_p( m_bStreamSet, ("Must have streams bound before calling draw. SetVertexShader() unbinds the streams. Did you fail to call SetStream() again?") );
	ntAssert_p( m_pThis->m_pCachedVertexShader, ("Must have a valid vertex shader before calling DrawPrimitives()") );
	ntAssert_p( m_pThis->m_pCachedPixelShader, ("Must have a valid pixel shader before calling DrawPrimitives()") );

	GcKernel::DrawElements( primType, iStartIndex, iIndexCount, hIndices.GetHandle() );
}

//-----------------------------------------------------
//!
//!	GetHardwareBackBuffer
//! method that retrieves the device back buffer. Treat with care!
//!
//----------------------------------------------------
RenderTarget::Ptr Renderer::GetHardwareBackBuffer()
{
	return SurfaceManager::Get().CreateRenderTarget( RenderTarget::CreationStruct( GcKernel::GetBackBuffer() ) );
}

//-----------------------------------------------------
//!
//!	SetStream
//! Wrapper on GcKernel::SetStream. MUST be called after
//! Renderer::SetVertexShader
//! NOTE! bound streams are always cleared after each draw call.
//! This is to ensure streams are never left bound, incuring
//! the DMA cost of unused vertex attributes.
//!
//----------------------------------------------------
uint32_t RendererPlatform::SetStream( const VBHandle& hStream, uint offsetInBytes, uint divider )
{
	ntAssert_p( m_pThis->m_pCachedVertexShader, ("Must have a valid vertex shader before calling SetStream()") );
	m_bStreamSet = true;
	return GcKernel::SetStream( m_pThis->m_pCachedVertexShader->GetVertexHandle(), hStream.GetHandle(), offsetInBytes, divider );
}

//-----------------------------------------------------
//!
//!	ClearStreams
//! Unbind what streams are currently active
//!
//----------------------------------------------------
void RendererPlatform::ClearStreams()
{
	ntAssert_p( m_pThis->m_pCachedVertexShader, ("Must have a valid vertex shader before calling ClearStreams()") );
	m_bStreamSet = false;
	GcKernel::ClearStreams( m_pThis->m_pCachedVertexShader->GetVertexHandle() );
}

//-----------------------------------------------------
//!
//!	SubmitTextures
//! Deferred call to GcKernel::SetTexture, forced by
//! bad usage model in Gc 1.2.2 and later. Will disappear.
//!
//----------------------------------------------------
void RendererPlatform::SubmitTextures()
{
	if (!m_iTexturesToSubmit)
		return;

	for (uint32_t i = 0;  i < MAX_SAMPLERS; i++)
	{
		if (m_iTexturesToSubmit & (1<<i))
		{
			ntAssert_p(m_aTextures[i], ("Must have a bound texture on this stage"));
			GcKernel::SetTexture( i, m_aTextures[i] );
		}
	}

	m_iTexturesToSubmit = 0;
}

//-----------------------------------------------------
//!
//!	RendererPlatform::CreateVertexStream
//! Wrapperd call to GcStreamBuffer::CreateVertexStream, 
//! wo we can create our own VBHandle objects and track
//! allocation sizes
//!
//----------------------------------------------------
VBHandle RendererPlatform::CreateVertexStream(	int vertexCount,
												int vertexStride,
												int	fieldCount,
												const GcStreamField* pFieldArray,
												Gc::BufferType bufferType,
												void* pMem )
{
#ifdef TRACK_GFX_MEM
	uint32_t vramStart = GcKernel::GetFreeVram();
#endif

	GcStreamBufferHandle hHandle = GcStreamBuffer::CreateVertexStream( vertexCount, vertexStride, fieldCount, pFieldArray, bufferType, pMem );

#ifdef TRACK_GFX_MEM
	return VBHandle( hHandle, (uint32_t)vertexCount, vramStart );
#else
	return VBHandle( hHandle );
#endif
}

//-----------------------------------------------------
//!
//!	RendererPlatform::CreateIndexStream
//! Wrapperd call to GcStreamBuffer::CreateIndexStream, 
//! wo we can create our own IBHandle objects and track
//! allocation sizes
//!
//----------------------------------------------------
IBHandle	RendererPlatform::CreateIndexStream(	Gc::StreamIndexType indexType,
													uint count,
													Gc::BufferType bufferType,
													void* pMem )
{
#ifdef TRACK_GFX_MEM
	uint32_t vramStart = GcKernel::GetFreeVram();
#endif

	GcStreamBufferHandle hHandle = GcStreamBuffer::CreateIndexStream( indexType, count, bufferType, pMem );

#ifdef TRACK_GFX_MEM
	return IBHandle( hHandle, vramStart );
#else
	return IBHandle( hHandle );
#endif
}

//-----------------------------------------------------
//!
//!	Gets the beginning of the currently used push buffer
//!
//----------------------------------------------------
void* RendererPlatform::GetStartPushBufferAddr()
{
	return GcKernel::GetContext().GetBeginAddress();
}
//-----------------------------------------------------
//!
//!	Gets the current end of the currently used push buffer
//!
//----------------------------------------------------
void* RendererPlatform::GetCurrentPushBufferAddr()
{
	return GcKernel::GetContext().GetCurrentAddress();
}

//-----------------------------------------------------
//!
//! Dump the push buffer to a binary file for disassembly on a pc
//!
//----------------------------------------------------
void RendererPlatform::DumpPushBuffer( void* pStart, void* pEnd, const char *pName )
{
	const unsigned int iSize = (char*)pEnd - (char*)pStart;
	FILE* fhPB = fopen( pName, "wb" );

	fwrite( pStart, iSize, 1, fhPB );
	fclose( fhPB );
}

//-----------------------------------------------------
//!
//! RendererPlatform::DrawFullscreenQuad 
//! Cut down on duplicated fullscreen vertex shaders
//! and quad data
//!
//-----------------------------------------------------
void RendererPlatform::DrawFullscreenQuad()
{
	Renderer::Get().SetVertexShader( m_fullscreenVS );
	SetStream( m_fullscreenData );
	DrawPrimitives( Gc::kQuads, 0, 4 );
	ClearStreams();
}

size_t GetSizeOfElement( VERTEX_DECL_STREAM_TYPE ntType )
{
	switch(ntType)
	{
	case VD_STREAM_TYPE_FLOAT1:	return (sizeof(float) * 1);
	case VD_STREAM_TYPE_FLOAT2:	return (sizeof(float) * 2);
	case VD_STREAM_TYPE_FLOAT3:	return (sizeof(float) * 3);
	case VD_STREAM_TYPE_FLOAT4:	return (sizeof(float) * 4);
	case VD_STREAM_TYPE_PACKED:	return (sizeof(uint32_t) * 1);

	case VD_STREAM_TYPE_UBYTE4: return (sizeof(uint32_t) * 1);
	case VD_STREAM_TYPE_SHORT2: return (sizeof(short) * 2);
	case VD_STREAM_TYPE_SHORT4: return (sizeof(short) * 4);

	case VD_STREAM_TYPE_UBYTE4N: return (sizeof(uint32_t) * 1);
	case VD_STREAM_TYPE_SHORT2N: return (sizeof(short) * 2);
	case VD_STREAM_TYPE_SHORT4N: return (sizeof(short) * 4);

	case VD_STREAM_TYPE_USHORT2N: return (sizeof(short) * 2);
	case VD_STREAM_TYPE_USHORT4N: return (sizeof(short) * 4);

	case VD_STREAM_TYPE_HALF2: return (sizeof(short) * 2);
	case VD_STREAM_TYPE_HALF4: return (sizeof(short) * 4);
	}
	ntAssert_p(0,("Unrecognised stream type: %s\n", ntType));
	return 0;
};

#ifdef TRACK_GFX_MEM
//-----------------------------------------------------
//!
//!	VBHandle::ctor
//! only RendererPlatform can create us
//!
//----------------------------------------------------
VBHandle::VBHandle(GcStreamBufferHandle hResource, uint32_t vertexCount, uint32_t vramStart) :
	m_hResource(hResource)
{
	m_iNumVerts = vertexCount;
	m_iAllocSize = vramStart - GcKernel::GetFreeVram();

	if (m_hResource)
	{
		TRACK_GFX_ALLOC_VB(m_iAllocSize);
		TRACK_GFX_ADD_VBSTAT(m_iNumVerts);
	}
}

//-----------------------------------------------------
//!
//!	VBHandle::Release
//! Handle release of resources when tracking
//!
//----------------------------------------------------
void VBHandle::Release()
{
	if (m_hResource)
	{
		if (m_hResource.GetRefCount() == 1)
		{
			TRACK_GFX_FREE_VB(m_iAllocSize);
			TRACK_GFX_FREE_VBSTAT(m_iNumVerts);
		}
	}
}

//-----------------------------------------------------
//!
//!	IBHandle::ctor
//! only RendererPlatform can create us
//!
//----------------------------------------------------
IBHandle::IBHandle(GcStreamBufferHandle hResource, uint32_t vramStart) :
	m_hResource(hResource)
{
	m_iAllocSize = vramStart - GcKernel::GetFreeVram();
	if (m_hResource)
	{
		TRACK_GFX_ALLOC_IB( m_iAllocSize );
	}
}

//-----------------------------------------------------
//!
//!	IBHandle::Release
//! Handle release of resources when tracking
//!
//----------------------------------------------------
void IBHandle::Release()
{
	if (m_hResource)
	{
		if (m_hResource.GetRefCount() == 1)
		{
			TRACK_GFX_FREE_IB(m_iAllocSize);
		}
	}
}

#endif
