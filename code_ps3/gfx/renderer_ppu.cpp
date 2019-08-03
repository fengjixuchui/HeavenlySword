//! \file renderer_ppu.cpp
//! SUPERC
//! This is a SuperC implementation, it should be
//! treated as C code using C++ style syntax and not
//! true C++ code. In particular, ctors must not be
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs

#include "gfx/depthhazeconsts.h"
#include "gfx/rendercontext.h"
#include "gfx/renderer_ppu_spu.h"
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

#define RENDERER_JOBS_MEM (2 * Mem::Mb)

void SpuRenderer::Initialise(void)
{
	if ( !m_bInitialized && CRendererSettings::bUseHeresy && CRendererSettings::bEnableSPURenderer )
	{
		ElfManager::Get().Load( RENDERER_SPU_ELF );
		m_pRendererTask = NT_PLACEMENT_NEW  ( (void*)(NT_MEMALIGN_CHUNK(Mem::MC_GFX, sizeof( SPUTask ), 0x10 )) ) SPUTask( ElfManager::Get().GetProgram( RENDERER_SPU_ELF ) );
			
		m_uiDependencyCounter = 0;

		m_pJobsMemory = NT_MEMALIGN_CHUNK(Mem::MC_GFX, RENDERER_JOBS_MEM, 0x100);
		DoubleEnderFrameAllocator_Initialise( &m_pDoubleBufferedJobsMem, m_pJobsMemory, RENDERER_JOBS_MEM );

		m_bInitialized = true;
	}
}
void SpuRenderer::Destroy(void)
{
	if ( m_bInitialized )
	{
		NT_FREE_CHUNK(Mem::MC_GFX, (uintptr_t)m_pJobsMemory);

		m_pRendererTask->~SPUTask();
		NT_FREE_CHUNK(Mem::MC_GFX, (uintptr_t)m_pRendererTask);

        ElfManager::Get().Unload( RENDERER_SPU_ELF );
		m_bInitialized = false;
	}
}

void SpuRenderer::PrePresent(void)
{
	if ( m_bInitialized && m_uiDependencyCounter != 0)
		m_pRendererTask->StallForJobToFinish();
}


void SpuRenderer::PostPresent(void)
{
	if ( m_bInitialized )
	{
		DoubleEnderFrameAllocator_SwapAndResetAllocationDirection( &SpuRenderer::m_pDoubleBufferedJobsMem );	
		// reset dependency counter
		m_uiDependencyCounter = 0;
	}
}


//extern bool g_bAnotherToggle;

void SpuRenderer::Render( void* vector, eSpuBatchRendPasses renderingPass )
{
	// define our vector type
	typedef ntstd::Vector<CRenderable*, Mem::MC_GFX> RenderableVector;	
	RenderableVector* pPPURenderableVector;
	RenderableVector  ppuRenderableVector;
	ppuRenderableVector.empty();
	RenderableVector* renderableVector = (RenderableVector*)vector;

	// get renderable count
	uint32_t renderableCount = (*renderableVector).size();

	// if there are no renderables exit
	if ( m_bInitialized && renderableCount != 0 && CRendererSettings::bEnableSPURenderer )
	{
		// Get main Push Buffer
		Heresy_PushBuffer* pMainPB = (Heresy_PushBuffer*)&GcKernel::GetContext();
		// align main push buffer
		Heresy_AlignPushBuffer( pMainPB, 0x10 );

		BatchRenderRenderContext* pContext = (BatchRenderRenderContext*)DoubleEnderFrameAllocator_MemAlign( &SpuRenderer::m_pDoubleBufferedJobsMem, sizeof(BatchRenderRenderContext), 16 );
		// make a copy of the current rendering context;
		pContext->InitContext();
		// Prepare DMA Render Context
		DMABuffer pDMABatchRenderContext( pContext, DMABuffer::DMAAllocSize( sizeof(BatchRenderRenderContext) ) );

		uint32_t index = 0;
		Heresy_PushBuffer currentPushBuffer;
		while ( index < renderableCount )
		{
			uint32_t maxIterations = Min( renderableCount - index, MAX_MESHES_PER_JOB );
			RendererMaterialInstanceData* pMeshesData = (RendererMaterialInstanceData*)DoubleEnderFrameAllocator_MemAlign( &SpuRenderer::m_pDoubleBufferedJobsMem, maxIterations * sizeof(RendererMaterialInstanceData), 16 );
			DMABuffer pDMAPerJobMeshesData( pMeshesData, DMABuffer::DMAAllocSize( maxIterations * sizeof(RendererMaterialInstanceData)));
		
			RendererData* pGlobalData = (RendererData*)DoubleEnderFrameAllocator_MemAlign( &SpuRenderer::m_pDoubleBufferedJobsMem, sizeof(RendererData), 16 );
			DMABuffer pDMAGlobalData( pGlobalData, DMABuffer::DMAAllocSize( sizeof(RendererData) ) );

			uint32_t addedMeshCounter = 0;
			uint32_t totalPixelShaderSize = 0;			

			currentPushBuffer = *((Heresy_PushBuffer*)&GcKernel::GetContext());
			for (uint32_t counter = 0; counter < maxIterations; counter++)
			{
				CRenderable* renderable = (*renderableVector)[index];
				if ( renderable->GetRenderableType() == CRenderable::RT_MESH_INSTANCE  )
				{
					CMeshInstance* meshInstance = (CMeshInstance*)renderable;

					const Heresy_PushBuffer* pPBHeader = NULL;

					// Get Mesh Push Buffer
					switch ( renderingPass )
					{
						case kRendPassShadowMap:
							pPBHeader = meshInstance->GetShadowMapPushBuffer();
						break;

						case kRendPassColorOpaque:
						case kRendPassColorTransparent:
							pPBHeader = meshInstance->GetRenderPushBuffer();
						break;

						case kRendPassPreZ:
							pPBHeader = meshInstance->GetDepthPushBuffer();
						break;

						default:
							ntAssert( "Renderer: Unknown Rendering Pass" );
						break;
					}
					
					if ( pPBHeader != NULL )
					{
						const CMeshHeader* meshHeader = meshInstance->GetMeshHeader();
						const Transform* meshTransform = meshInstance->GetTransform();
						meshTransform->GetParentHierarchy()->UpdateSkinMatrices();
									
						if ( !meshHeader->IsPositionCompressed() || ( meshInstance->GetMaterialInstance()->GetBoundType() == VSTT_SKINNED) ) 
							pMeshesData[addedMeshCounter].bApplyPosReconstMatrix = false;
						else
							pMeshesData[addedMeshCounter].bApplyPosReconstMatrix  = true;

						// get reconstruction matrix for this instance
						const void *rec = meshInstance->GetReconstructionMatrix();
						if (rec != NULL)
							NT_MEMCPY(&pMeshesData[addedMeshCounter].obReconstructionMatrix, rec, sizeof(CMatrix));
						else
							pMeshesData[addedMeshCounter].obReconstructionMatrix.SetIdentity();

						// this flag let me know if we have to set one or two matrices in the depth vertex shader
						bool bSetReconstMatrix = ( meshHeader->IsPositionCompressed() && ( meshInstance->GetMaterialInstance()->GetBoundType() == VSTT_SKINNED) ) ;
							
						// this stuff is not costant
						if ((!bSetReconstMatrix ) && (renderingPass == kRendPassShadowMap))
							pMeshesData[addedMeshCounter].obObjectToWorld = pMeshesData[addedMeshCounter].obReconstructionMatrix * meshTransform->GetWorldMatrixFast();
						else
							pMeshesData[addedMeshCounter].obObjectToWorld = meshTransform->GetWorldMatrixFast();

						// do the pushbuffer fixup (if path properties was done inline this could be done there...)
						restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pPBHeader + 1);
						uint16_t i = 0;
						uint32_t pixelShaderSize = 0;
						while ( (i < pPBHeader->m_iNumPatches) && (!(pPatch->m_iData & HPBP_FIXUP_TYPE_MASK))  )
						{
							pPatch++;
							i++;
						}

						if (i != pPBHeader->m_iNumPatches)
							pixelShaderSize = (pPatch->m_Semantic & HPBP_PIXEL_SHADER_SIZE_MASK)<<2;

						ntAssert( pixelShaderSize != 0);
						pixelShaderSize = (pixelShaderSize + (0x40 - 1)) & (~(0x40 - 1));
						totalPixelShaderSize += pixelShaderSize;
			
						uint32_t headerSize = ((uint32_t)pPBHeader->m_pStart) - ((uint32_t)pPBHeader);
						if ( headerSize > 32768 )
							headerSize = 32768;
			
						// left some empty space in the main push buffer so that a SPU can fill it
						unsigned int pushBufferSize = (pPBHeader->m_pCurrent - pPBHeader->m_pStart)<<2;
						unsigned int alignedPushBufferSize = ((pushBufferSize + 0xF) & (~(0xF)));
						Heresy_IncPB( pMainPB, alignedPushBufferSize>>2 );

						pMeshesData[addedMeshCounter].bCollapseReconstMatrix = ((!bSetReconstMatrix ) && (renderingPass == kRendPassShadowMap));
						pMeshesData[addedMeshCounter].BoneCount =  meshHeader->m_iNumberOfBonesUsed;
						pMeshesData[addedMeshCounter].pBoneIndices = (void*)meshHeader->m_pucBoneIndices;
						pMeshesData[addedMeshCounter].ui32FrameFlag = meshInstance->m_FrameFlags;
						pMeshesData[addedMeshCounter].pSkinMatrixArray = (void*)meshInstance->GetTransform()->GetParentHierarchy()->GetSkinMatrixArray();
						pMeshesData[addedMeshCounter].pPushBufferHeader = pPBHeader;
						pMeshesData[addedMeshCounter].ui32PushBufferHeaderSize = headerSize;
						pMeshesData[addedMeshCounter].pPushBuffer = pPBHeader->m_pStart;
						pMeshesData[addedMeshCounter].ui32PushBufferSize = pushBufferSize;

						addedMeshCounter++;

#if defined( _PROFILING )
						Renderer::Get().m_renderCount.AddTriStrip( meshInstance->GetIndexCount() );
#endif	
					}
					else
					{
						ppuRenderableVector.push_back( renderable );
					}
				}
				else
				{
					ppuRenderableVector.push_back( renderable );
				}
				index++;
			}
			
			if (addedMeshCounter != 0 && totalPixelShaderSize != 0)
			{
				// allocate pixel shaders memory for this batch
				pGlobalData->pDestPixelShaders = (uint32_t*)Heresy_AllocatePixelShaderSpaceInXDDR( RenderingContext::ms_pHeresyGlobal, totalPixelShaderSize );
				pGlobalData->meshCount = addedMeshCounter;
				pGlobalData->renderingPass = renderingPass;

				SPUPushBufferManagement* pSpuPBManagement = (SPUPushBufferManagement*)DoubleEnderFrameAllocator_MemAlign( &SpuRenderer::m_pDoubleBufferedJobsMem, sizeof(SPUPushBufferManagement), 16 );
				pSpuPBManagement->m_HeresyGlobal.m_IceVramOffset = RenderingContext::ms_pHeresyGlobal->m_IceVramOffset;
				pSpuPBManagement->m_HeresyGlobal.m_RSXMainBaseAdjust = RenderingContext::ms_pHeresyGlobal->m_RSXMainBaseAdjust;
				pSpuPBManagement->m_pOutPushBuffer = currentPushBuffer;
				DMABuffer pDMASpuPBManagement( pSpuPBManagement, DMABuffer::DMAAllocSize( sizeof(SPUPushBufferManagement) ) );

				// set all its arguments
				m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMABatchRenderContext ), kRendContext );
				m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMASpuPBManagement ), kRendPushBufManagement );
				m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMAGlobalData ), kRendGlobalData );
				m_pRendererTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pDMAPerJobMeshesData ), kRendMeshesData );
				
				Exec::RunTask( m_pRendererTask ); 
		
				m_uiDependencyCounter++;
			}
		}

		pPPURenderableVector = (RenderableVector*)&ppuRenderableVector;
	}
	else
	{
		pPPURenderableVector = (RenderableVector*)vector;
	}

	
	renderableCount = (*pPPURenderableVector).size();
	for ( uint32_t i = 0; i < renderableCount; i++ )
	{
		CRenderable *pRenderable = (*pPPURenderableVector)[i];
		switch ( renderingPass )
		{
			case kRendPassShadowMap:
				pRenderable->RenderShadowMap();
			break;
			case kRendPassColorOpaque:
			case kRendPassColorTransparent:
				pRenderable->RenderMaterial();
			break;
			case kRendPassPreZ:
				pRenderable->RenderDepth();
			break;
			default:
				ntAssert( "Renderer: Unknown Rendering Pass" );
			break;
		}	
	}
}



//////////////////////////////////////////////////////////////////////////
// static initialisations
//////////////////////////////////////////////////////////////////////////
uintptr_t					SpuRenderer::m_pJobsMemory				= NULL;
SPUTask*					SpuRenderer::m_pRendererTask			= NULL;
bool						SpuRenderer::m_bInitialized				= false;
uint32_t					SpuRenderer::m_uiDependencyCounter		= 0;
DoubleEnderFrameAllocatorC	SpuRenderer::m_pDoubleBufferedJobsMem;
