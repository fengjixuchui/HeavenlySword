//! \file batchrender_ppu_spu.cpp
//! SUPERC
//! This is a SuperC implementation, it should be
//! treated as C code using C++ style syntax and not
//! true C++ code. In particular, ctors must not be
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs

#include "exec/ppu/sputask_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "exec/ppu/dmabuffer_ps3.h"

#include "gfx/updateskin_ppu_spu.h"
#include "gfx/rendercontext.h"
#include "gfx/renderable.h"
#include "gfx/meshinstance.h"

#include "anim/hierarchy.h"


void SpuUpdateSkin::Initialise(void)
{
	if ( !m_bInitialized )
	{
		ElfManager::Get().Load( UPDATESKIN_SPU_ELF );
		m_pUpdateSkinTask = NT_PLACEMENT_NEW  ( (void*)(NT_MEMALIGN_CHUNK(Mem::MC_GFX, sizeof( SPUTask ), 0x10 )) ) SPUTask( ElfManager::Get().GetProgram( UPDATESKIN_SPU_ELF ) );

#define EFFECTIVE_PERFRAME_MEM 128
#define UPDATESKIN_JOBS_MEM ((EFFECTIVE_PERFRAME_MEM * 2 + 1) * Mem::Kb)

		m_pJobsMemory = NT_MEMALIGN_CHUNK(Mem::MC_GFX, UPDATESKIN_JOBS_MEM, 0x10);
		DoubleEnderFrameAllocator_Initialise( &m_pDoubleBufferedJobsMem, m_pJobsMemory, UPDATESKIN_JOBS_MEM );

		m_bInitialized = true;
	}
}
void SpuUpdateSkin::Destroy(void)
{
	if ( m_bInitialized )
	{
		NT_FREE_CHUNK(Mem::MC_GFX, (uintptr_t)m_pJobsMemory);

		m_pUpdateSkinTask->~SPUTask();
		NT_FREE_CHUNK(Mem::MC_GFX, (uintptr_t)m_pUpdateSkinTask);

        ElfManager::Get().Unload( UPDATESKIN_SPU_ELF );
		m_bInitialized = false;
	}
}

void SpuUpdateSkin::Sync(void)
{
	DoubleEnderFrameAllocator_SwapAndResetAllocationDirection( &m_pDoubleBufferedJobsMem );	

	if ( m_bInitialized && m_bSync )
	{
		m_pUpdateSkinTask->StallForJobToFinish();

		m_bSync = false;
	}
}

void SpuUpdateSkin::PrepareSPUData( void* _visibleRenderables )
{
	typedef ntstd::Vector<CRenderable*, Mem::MC_GFX> RenderableVector;
	RenderableVector* visibleRenderables = (RenderableVector*)_visibleRenderables;

	for (uint32_t i = 0; (i < (*visibleRenderables).size()) && (currentHierarchy < maxUpdateCount); i++ )
	{
		CRenderable* Renderable = (*visibleRenderables)[i];
		if ( Renderable->GetRenderableType() == CRenderable::RT_MESH_INSTANCE )
		{
			const CMeshInstance* Mesh = (CMeshInstance*)Renderable;
			if ( Mesh != NULL ) 
			{
				const Transform* transform = Mesh->GetTransform();
				if ( transform != NULL )
				{
					CHierarchy* Hierarchy =   transform->GetParentHierarchy();
					if ( Hierarchy != NULL )
					{
						ntAssert( !( Hierarchy->GetFlags() & HIERF_INVALIDATED_MAIN ) );

						if ( ( Hierarchy->GetFlags() & HIERF_MAY_REQUIRE_SKIN_MATRIX_UPDATE ) )
						{
							ntAssert( Hierarchy->GetSkinToBoneArray() );
							if ( Hierarchy->GetSkinToBoneArray() )
							{
								m_pHierarchyData[ currentHierarchy ].m_ui32TransformCount = Hierarchy->GetTransformCount();
								m_pHierarchyData[ currentHierarchy ].m_pSkinToBoneArray = Hierarchy->GetSkinToBoneArray();
								m_pHierarchyData[ currentHierarchy ].m_pTransformArray = Hierarchy->GetRootTransform();
								m_pHierarchyData[ currentHierarchy ].m_pSkinMatrixArray = Hierarchy->GetSkinMatrixArray();

								currentHierarchy++;
							}

							Hierarchy->ClearFlagBits( HIERF_MAY_REQUIRE_SKIN_MATRIX_UPDATE );
						}
					}
				}
			}
		}
	}
}


//extern bool g_bUpdateSkin;

void SpuUpdateSkin::UpdateSkin(void)
{
	
//	if (g_bUpdateSkin)
//		return;

	if ( m_bInitialized )
	{
		// we get all the memory we can get
		m_pHierarchyData = (HierarchyData*)DoubleEnderFrameAllocator_MemAlign( &m_pDoubleBufferedJobsMem, EFFECTIVE_PERFRAME_MEM * Mem::Kb, 16 );
		// reset max hierarchy count
		maxUpdateCount = ((EFFECTIVE_PERFRAME_MEM * Mem::Kb) / sizeof ( HierarchyData )) - 1;
		// reset hierarchies index
		currentHierarchy = 0;

		// prepare data for our SPU Job		
		PrepareSPUData( (void*)&RenderingContext::Get()->m_aShadowCastingRenderables );	
		PrepareSPUData( (void*)&RenderingContext::Get()->m_aVisibleRenderables );

#define MAX_HIERARCHIES_PER_JOB 32

		uint32_t offset = 0;
		while ( currentHierarchy > 0)
		{
			uint32_t batchSize = Min( MAX_HIERARCHIES_PER_JOB, currentHierarchy );

			DMABuffer pHierarchiesData( m_pHierarchyData + offset, DMABuffer::DMAAllocSize( sizeof(HierarchyData) * batchSize ) );

			m_pUpdateSkinTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t) batchSize ), 0 );
  			m_pUpdateSkinTask->SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, pHierarchiesData ), 1 );
	
			Exec::RunTask( m_pUpdateSkinTask ); 
			
			// flag sync
			m_bSync = true;
			// update counters
			currentHierarchy -= batchSize;
			offset += batchSize;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// static initializations
//////////////////////////////////////////////////////////////////////////

bool	SpuUpdateSkin::m_bInitialized = false;
bool	SpuUpdateSkin::m_bSync = false;

uint32_t SpuUpdateSkin::maxUpdateCount = 0;
uint32_t SpuUpdateSkin::currentHierarchy = 0;

SPUTask* SpuUpdateSkin::m_pUpdateSkinTask = NULL;
SpuUpdateSkin::HierarchyData* SpuUpdateSkin::m_pHierarchyData = NULL;

uintptr_t SpuUpdateSkin::m_pJobsMemory = NULL;
DoubleEnderFrameAllocatorC SpuUpdateSkin::m_pDoubleBufferedJobsMem;

