//! \file batchrender_ppu_spu.cpp
//! SUPERC
//! This is a SuperC implementation, it should be
//! treated as C code using C++ style syntax and not
//! true C++ code. In particular, ctors must not be
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs

#include "gfx/depthhazeconsts.h"
#include "gfx/rendercontext.h"
#include "gfx/batchrender_ppu_spu.h"
#include "gfx/renderer.h"
#include "gfx/meshinstance.h"
#include "anim/hierarchy.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "exec/ppu/dmabuffer_ps3.h"
#include "core/timer.h"
#include "core/gfxmem.h"

#define BREN_JOBS_MEM (2 * Mem::Mb)
#define JOB_BATCH_MAX_SIZE 256

void BatchRenderer::Initialise(void)
{
	if ( !m_bInitialized && CRendererSettings::bUseHeresy && CRendererSettings::bEnableBatchRenderer)
	{
		ElfManager::Get().Load( BATCH_RENDER_SPU_ELF );
		m_pRendererTask = NT_PLACEMENT_NEW  ( (void*)(NT_MEMALIGN_CHUNK(Mem::MC_GFX, sizeof( SPUTask ), 0x10 )) ) SPUTask( ElfManager::Get().GetProgram( BATCH_RENDER_SPU_ELF ) );
			
		m_uiDependencyCounter = 0;


		m_pJobsMemory = NT_MEMALIGN_CHUNK(Mem::MC_GFX, BREN_JOBS_MEM, 0x100);
		DoubleEnderFrameAllocator_Initialise( &m_pDoubleBufferedJobsMem, m_pJobsMemory, BREN_JOBS_MEM );

		m_bInitialized = true;
	}
}
void BatchRenderer::Destroy(void)
{
	if ( m_bInitialized )
	{
		NT_FREE_CHUNK(Mem::MC_GFX, (uintptr_t)m_pJobsMemory);

		m_pRendererTask->~SPUTask();
		NT_FREE_CHUNK(Mem::MC_GFX, (uintptr_t)m_pRendererTask);

        ElfManager::Get().Unload( BATCH_RENDER_SPU_ELF );
		m_bInitialized = false;
	}
}

void BatchRenderer::PrePresent(void)
{
	if ( m_bInitialized && m_uiDependencyCounter != 0)
	{
		m_pRendererTask->StallForJobToFinish();
	}
}


void BatchRenderer::PostPresent(void)
{
	if ( m_bInitialized )
	{
		DoubleEnderFrameAllocator_SwapAndResetAllocationDirection( &BatchRenderer::m_pDoubleBufferedJobsMem );	
		// reset dependency counter
		m_uiDependencyCounter = 0;
	}
}

void BatchRenderRenderContext::InitContext( void )
{
		RenderingContext::CONTEXT_DATA *context = RenderingContext::Get();

		m_worldToView = context->m_worldToView;
		m_worldToScreen = context->m_worldToScreen;					//!< The transform from world space to screen space.
		m_viewToScreen = context->m_viewToScreen;					//!< The transform from view space to screen.
			
		NT_MEMCPY( m_shadowMapProjection, context->m_shadowMapProjection, sizeof(m_shadowMapProjection));

			
		NT_MEMCPY( m_SHMatrices, context->m_SHMatrices.m_aChannelMats, sizeof(m_SHMatrices) );

		m_reflectanceCol = context->m_reflectanceCol;				//!< Reflectance colour
		m_keyDirection = context->m_keyDirection;					//!< Direction of keylight
		m_toKeyLight = context->m_toKeyLight;							//!< Direction to keylight src

		m_shadowDirection = context->m_shadowDirection;				//!< Direction of the shadow
		m_sunDirection = CDepthHazeSetting::GetSunDir();					//!< Direction of the sun for atmospheric haze
		m_keyColour = context->m_keyColour;							//!< Colour of the keylight
		m_skyColour = context->m_skyColour;							//!< Colour of the sky

		m_eyePos = context->GetEyePos();								//!< eyepos 
		m_shadowMapPlanesW = CVector( context->m_shadowPlanes[1].W(), context->m_shadowPlanes[2].W(), context->m_shadowPlanes[3].W(), 0.0f );			//!< <m_shadowPlanes[1].W, m_shadowPlanes[2].W, m_shadowPlanes[3].W, 0>
		*m_shadowRadii = *context->m_shadowRadii;					//!< shadow radii for each section
		
		m_gameTime = CVector( CTimer::Get().GetGameTime(), 0.0f, 0.0f, 0.0f );		//!< <time, 0, 0, 0>
	
		m_depthHazeA = CDepthHazeSetting::GetAConsts();				//!< Depth Haze A
		m_depthHazeG = CDepthHazeSetting::GetGConsts();				//!< Depth Haze G
		m_depthHazeBeta1PlusBeta2 = CDepthHazeSetting::GetBeta1PlusBeta2();			//! Depth Haze b1+b2
		m_depthHazeOneOverBeta1PlusBeta2 = CDepthHazeSetting::GetOneOverBeta1PlusBeta2();	// Depth Haze 1/(b1+b2)
		m_depthHazeBetaDash1 = CDepthHazeSetting::GetBetaDash1();	// Depth Haze b'1
		m_depthHazeBetaDash2 = CDepthHazeSetting::GetBetaDash2();	// Depth Haze b'2
		m_depthHazeSunColour = CDepthHazeSetting::GetSunColour();	// Sun Colour

		m_ExposureAndToneMapConsts = context->m_ExposureAndToneMapConsts;

		m_fCameraFOVAngle = context->m_pViewCamera->GetFOVAngle();
		m_fScreenAspectRatio = context->m_fScreenAspectRatio;

		NT_MEMCPY( m_shadowScissor, context->m_shadowScissor, sizeof(m_shadowScissor));

		m_RenderTargetSize = CVector( Renderer::Get().m_targetCache.GetWidth(), Renderer::Get().m_targetCache.GetHeight(), 0.0f, 0.0f );

		if ( context->m_reflectanceMap )
			NT_MEMCPY( &m_reflectanceMap, &context->m_reflectanceMap->m_Platform.GetTexture()->GetIceTexture(), sizeof(m_reflectanceMap) );	//!< Reflectance map
		if ( context->m_pShadowMap )		
			NT_MEMCPY( &m_pShadowMap, &context->m_pShadowMap->m_Platform.GetTexture()->GetIceTexture(), sizeof(m_pShadowMap) );			// all the registers for the shadow map
		if ( context->m_pStencilTarget )
			NT_MEMCPY( &m_pStencilTarget, &context->m_pStencilTarget->m_Platform.GetTexture()->GetIceTexture(), sizeof(m_pStencilTarget) );	// all the registers for the stencil target */
		if ( context->m_pIrradianceCache )
			NT_MEMCPY( &m_pIrradianceCache, &context->m_pIrradianceCache->m_Platform.GetTexture()->GetIceTexture(), sizeof(m_pIrradianceCache) );	// all the registers for the irradiance cache */
}

//extern bool g_bAnotherToggle;
//bool bReUsePixelShader = false;
//bool bJumpPostDraw = true;
//bool bJumpPreDraw = false;
//extern uint32_t g_ui32WastedMemory;

void BatchRenderer::Render( void* meshVector, eSpuBatchRendPasses renderingPass )
{

	if ( m_bInitialized )
	{
		typedef ntstd::Vector<CMeshInstance*, Mem::MC_GFX> MeshInstanceVector;	
		MeshInstanceVector* batch = (MeshInstanceVector*)meshVector;
		uint32_t renderableCount = (*batch).size();

		if ( !renderableCount ) 
			return;

		uint32_t left = 0, right;
		const CMeshHeader* pMeshHeaderLeft;
		const CMeshHeader* pMeshHeaderRight;

		// Get main Push Buffer
		Heresy_PushBuffer* pMainPB = (Heresy_PushBuffer*)&GcKernel::GetContext();
		// align main push buffer
		Heresy_AlignPushBuffer( pMainPB, 0x10 );

		m_pContext = (BatchRenderRenderContext*)DoubleEnderFrameAllocator_MemAlign( &BatchRenderer::m_pDoubleBufferedJobsMem, sizeof(BatchRenderRenderContext), 16 );
		// make a copy of the current rendering context;
		m_pContext->InitContext();
		// Prepare DMA Render Context
		DMABuffer pDMABatchRenderContext( BatchRenderer::m_pContext, DMABuffer::DMAAllocSize( sizeof(BatchRenderRenderContext) ) );


		for ( right = 0; right < renderableCount; right++ )
		{
			bool exit = (right == (renderableCount - 1));

			pMeshHeaderLeft = ((const CMeshInstance*)(*batch)[ left ])->GetMeshHeader();
			pMeshHeaderRight = ((const CMeshInstance*)(*batch)[ right ])->GetMeshHeader();

			if (( pMeshHeaderLeft != pMeshHeaderRight ) || exit )
			{
				if  ( exit )
				right++;

			
				const CMeshHeader* MeshHeader = pMeshHeaderLeft;

				BatchData* perBatchData = (BatchData*)DoubleEnderFrameAllocator_MemAlign( &BatchRenderer::m_pDoubleBufferedJobsMem, sizeof(BatchData), 16 );
	
				CMeshInstance* instanceMesh = (*batch)[ left ];
				if ( !instanceMesh->GetMeshHeader()->IsPositionCompressed() || ( instanceMesh->GetMaterialInstance()->GetBoundType() == VSTT_SKINNED) ) 
					perBatchData->bApplyPosReconstMatrix  = false;
				else
					perBatchData->bApplyPosReconstMatrix  = true;

				// get reconstruction matrix for this instance
				const void *rec = instanceMesh->GetReconstructionMatrix();
				if (rec != NULL)
					NT_MEMCPY(&(perBatchData->obReconstructionMatrix), rec, sizeof(CMatrix));
				else
					perBatchData->obReconstructionMatrix.SetIdentity();

				// this flag let me know if we have to set one or two matrices in the depth vertex shader
				bool bSetReconstMatrix = ( instanceMesh->GetMeshHeader()->IsPositionCompressed() && ( instanceMesh->GetMaterialInstance()->GetBoundType() == VSTT_SKINNED) ) ;
				perBatchData->bCollapseReconstMatrix = ((!bSetReconstMatrix ) && (renderingPass == kRendPassShadowMap));
				perBatchData->BoneCount =  MeshHeader->m_iNumberOfBonesUsed;
				perBatchData->pBoneIndices = (void*)MeshHeader->m_pucBoneIndices;
				perBatchData->renderingPass = renderingPass;
				//perBatchData->bReUsePixelShader = bReUsePixelShader;
				//perBatchData->bJumpPostDraw = bJumpPostDraw;
				//perBatchData->bJumpPreDraw = bJumpPreDraw;

				perBatchData->bReUsePixelShader = true;
				perBatchData->bJumpPostDraw = true;
				perBatchData->bJumpPreDraw = true;

				DMABuffer pDMAPerBatchData( perBatchData, DMABuffer::DMAAllocSize( sizeof(BatchData) ) );

				// Get Mesh Push Buffer
				switch ( renderingPass )
				{
					case kRendPassShadowMap:
						m_pPushBufferHeader = instanceMesh->GetShadowMapPushBuffer();
					break;

					case kRendPassColorOpaque:
					case kRendPassColorTransparent:
						m_pPushBufferHeader = instanceMesh->GetRenderPushBuffer();
					break;

					case kRendPassPreZ:
						m_pPushBufferHeader = instanceMesh->GetDepthPushBuffer();
					break;

					default:
						ntAssert( "BatchRenderer: Unknown Rendering Pass" );
					break;
				}
				
				// do the pushbuffer fixup (if path properties was done inline this could be done there...)
				restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (BatchRenderer::m_pPushBufferHeader +1 );
				uint16_t i = 0;
				uint32_t pixelShaderSize = 0;
				//uint32_t mem = 0;
				while ( i < BatchRenderer::m_pPushBufferHeader->m_iNumPatches )
				{
					switch( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) )
					{
						case HPBP_PIXEL_CONSTANT_FIXUP:
//							bReUsePixelShader = false;
							break;
						case HPBP_PIXEL_SHADER_FIXUP:
							pixelShaderSize = (pPatch->m_Semantic & HPBP_PIXEL_SHADER_SIZE_MASK)<<2;
							break;
//						case HPBP_INSTANCE_JUMP_POST_DRAW_FIXUP:
//							mem +=  4 * ((pPatch->m_iData & HPBP_DATA_MASK)>>HPBP_DATA_MASK_LENGTH);
//							break;
//						case HPBP_INSTANCE_JUMP_PRE_DRAW_FIXUP:
//							mem +=  4 * ((pPatch->m_iData & HPBP_DATA_MASK)>>HPBP_DATA_MASK_LENGTH);
//							break;
						default:
							break;
					}
					pPatch++;
					i++;
				}


				while ( left != right )
				{

					// HACK!! LIMIT BATCH SIZE TO A SINGLE INSTANCE
					uint32_t instanceCount = Min(right - left, JOB_BATCH_MAX_SIZE);
//					g_ui32WastedMemory += instanceCount * mem;

					m_pInstanceData = (MaterialInstanceData*)DoubleEnderFrameAllocator_MemAlign( &BatchRenderer::m_pDoubleBufferedJobsMem, instanceCount * sizeof(MaterialInstanceData), 16 );

					// fill instances structures
					for (unsigned int i = 0; i < instanceCount; i++)
					{
						CMeshInstance* Mesh = (*batch)[ left + i ];
						Mesh->GetTransform()->GetParentHierarchy()->UpdateSkinMatrices();
						
						// this stuff is not costant
						if ((!bSetReconstMatrix ) && (renderingPass == kRendPassShadowMap))
							BatchRenderer::m_pInstanceData[i].obObjectToWorld = perBatchData->obReconstructionMatrix * Mesh->GetTransform()->GetWorldMatrixFast();
						else
							BatchRenderer::m_pInstanceData[i].obObjectToWorld = Mesh->GetTransform()->GetWorldMatrixFast();

						BatchRenderer::m_pInstanceData[i].pSkinMatrixArray = (void*)Mesh->GetTransform()->GetParentHierarchy()->GetSkinMatrixArray();
						BatchRenderer::m_pInstanceData[i].ui32FrameFlag = Mesh->m_FrameFlags;
					}
			

					
					if ( BatchRenderer::m_pPushBufferHeader )
					{
						ntAssert( pixelShaderSize != 0);
						pixelShaderSize = (pixelShaderSize + (0x40 - 1)) & (~(0x40 - 1));

						uint32_t pixelShaderInstances = 1;
						// if we can reuse this pixel shader for all the instances we allocate space only for one of them
						if ( !perBatchData->bReUsePixelShader )
							pixelShaderInstances = instanceCount;

						// allocate pixel shaders memory for this batch
						uint32_t* pDestShader = (uint32_t*)Heresy_AllocatePixelShaderSpaceInXDDR( RenderingContext::ms_pHeresyGlobal,
																								pixelShaderSize * pixelShaderInstances );
	
						// make a copy of our instanced push buffer
						unsigned int pushBufferSize = (BatchRenderer::m_pPushBufferHeader->m_pCurrent - BatchRenderer::m_pPushBufferHeader->m_pStart)<<2;
						m_pPushBuffer = (uint32_t*)DoubleEnderFrameAllocator_MemAlign( &BatchRenderer::m_pDoubleBufferedJobsMem, pushBufferSize, 16 );
						NT_MEMCPY( BatchRenderer::m_pPushBuffer, BatchRenderer::m_pPushBufferHeader->m_pStart, pushBufferSize );
						DMABuffer pDMAPushBuffer( BatchRenderer::m_pPushBuffer, DMABuffer::DMAAllocSize( pushBufferSize ) );

						DMABuffer pDMAInstanceData( BatchRenderer::m_pInstanceData,  instanceCount * DMABuffer::DMAAllocSize( sizeof(MaterialInstanceData) ) );

						uint32_t headerSize = ((uint32_t)BatchRenderer::m_pPushBufferHeader->m_pStart) - ((uint32_t)BatchRenderer::m_pPushBufferHeader);
						if ( headerSize > 32768 )
							headerSize = 32768;

						DMABuffer pDMAPushBufferHeader( BatchRenderer::m_pPushBufferHeader, DMABuffer::DMAAllocSize( headerSize ));

						// Spu Management Init
						m_pSpuPBManagement = (SPUPushBufferManagement*)DoubleEnderFrameAllocator_MemAlign( &BatchRenderer::m_pDoubleBufferedJobsMem, sizeof(SPUPushBufferManagement), 16 );
						BatchRenderer::m_pSpuPBManagement->m_HeresyGlobal.m_IceVramOffset = RenderingContext::ms_pHeresyGlobal->m_IceVramOffset;
						BatchRenderer::m_pSpuPBManagement->m_HeresyGlobal.m_RSXMainBaseAdjust = RenderingContext::ms_pHeresyGlobal->m_RSXMainBaseAdjust;
						BatchRenderer::m_pSpuPBManagement->m_pOutPushBuffer = *((Heresy_PushBuffer*)&GcKernel::GetContext());
						DMABuffer pDMASpuPBManagement( BatchRenderer::m_pSpuPBManagement, DMABuffer::DMAAllocSize( sizeof(SPUPushBufferManagement) ) );


						// left some empty space in the main push buffer so that a SPU can fill it
						pushBufferSize = instanceCount * ((pushBufferSize + 0xF) & (~(0xF)));
						Heresy_IncPB( pMainPB, pushBufferSize>>2 );

						// set all its arguments
						m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMAPerBatchData ), kBatchRendPerBatchData );
  						m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMABatchRenderContext ), kBatchRendContext );
						m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMASpuPBManagement ), kBatchRendPushBufManagement );
						m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t) instanceCount ), kBatchRendInstanceCount );		
						m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMAPushBufferHeader ), kBatchRendPushBufHeader );
						m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMAPushBuffer ), kBatchRendPushBufPointer );
						m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMAInstanceData ), kBatchRendMaterialInstanceData );  
						m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t) pDestShader ), kBatchRendPixelShadersMem );  

			
						Exec::RunTask( m_pRendererTask ); 
		

						m_uiDependencyCounter++;

#if defined( _PROFILING )
						Renderer::Get().m_renderCount.AddBatchTriStrip( instanceMesh->GetIndexCount(), instanceCount );
#endif							
					} 
					left += instanceCount;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// static initialisations
//////////////////////////////////////////////////////////////////////////
BatchRenderRenderContext* BatchRenderer::m_pContext = NULL;
SPUPushBufferManagement* BatchRenderer::m_pSpuPBManagement = NULL;
const Heresy_PushBuffer* BatchRenderer::m_pPushBufferHeader = NULL;
uint32_t* BatchRenderer::m_pPushBuffer = NULL;
MaterialInstanceData* BatchRenderer::m_pInstanceData = NULL;	
uintptr_t BatchRenderer::m_pJobsMemory = NULL;
bool BatchRenderer::m_bInitialized = false;
DependencyCounter* BatchRenderer::m_pBatchesDependency = NULL;
uint32_t BatchRenderer::m_uiDependencyCounter = 0;
SPUTask* BatchRenderer::m_pRendererTask = NULL;
DoubleEnderFrameAllocatorC	BatchRenderer::m_pDoubleBufferedJobsMem;
