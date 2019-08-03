#include "blendshapes/xpushapeblending.h"
#include "blendshapes/BlendedMeshInstance.h"
#include "blendshapes/BlendShapes.h"
#include "blendshapes/blendshapes_export.h"
#include "blendshapes/blendshapesbatchinfo_spu_ppu.h"

#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/spuprogram_ps3.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "gfx/rendercontext.h"
#include "anim/transform.h"

//! sync behaviour flags. Use only one!
//! calls frame end/reset once update is done. 
//#define BS_IMMEDIATE_SYNC
//! calls frame end/reset when flush is called (just before rendering)
#define BS_FLUSH_SYNC

static const char* ELF_NAME = "bs_spu_ps3.mod";




XPUShapeBlendingImpl::XPUShapeBlendingImpl( uint32_t vertsPerBatch )
:	m_meshRegistry(),
	m_pSpuBlender( 0 ),
	m_bEnabled( false ),
	m_vertsPerBatch( Util::Align(vertsPerBatch,16) ),
	m_blendWeightThreshold( EPSILON )
{
	ElfManager::Get().Load( ELF_NAME );
	m_pSpuBlender =  ElfManager::Get().GetProgram( ELF_NAME );
	
	m_pSpuTaskCounter = (int*)NT_MEMALIGN_CHUNK( Mem::MC_GFX, sizeof(int), 128 );
	AtomicSet( m_pSpuTaskCounter, 0 );
}


XPUShapeBlendingImpl::~XPUShapeBlendingImpl( void )
{
	Reset();
	NT_FREE_CHUNK(Mem::MC_GFX,(uintptr_t)m_pSpuTaskCounter);
}

bool XPUShapeBlendingImpl::IsRegistered( BlendedMeshInstance* pMesh ) const
{
	return ( ntstd::find( m_meshRegistry.begin(), m_meshRegistry.end(), pMesh ) != m_meshRegistry.end() );
}


void XPUShapeBlendingImpl::Register( BlendedMeshInstance *pMesh )
{
	ntAssert( pMesh && pMesh->IsShapeBlended() );
	
	bool bAlreadyRegistered = IsRegistered( pMesh );
	ntAssert_p( !bAlreadyRegistered, ("BlendedMeshInstance is already registered with XPUShapeBlender") );
	
	if ( !bAlreadyRegistered )
	{
		m_meshRegistry.push_back( pMesh );
	}
}

void XPUShapeBlendingImpl::Unregister( BlendedMeshInstance *pMesh )
{
	if ( IsRegistered( pMesh ) )
	{
		m_meshRegistry.remove( pMesh );
	}
	else
		user_warn_msg(("Unregistered BlendedMeshInstance detected\n"));
}

void XPUShapeBlendingImpl::Reset( bool bDeleteRegisteredInstances )
{
	user_warn_p( m_meshRegistry.empty(), ("BSMeshInstance registry is not empty on singleton reset. There are dangling instances somewhere... \n") );

	Flush();

	if ( bDeleteRegisteredInstances )
	{
		while ( !m_meshRegistry.empty() )
		{
			// note that blendedmeshintance unregisters itself from this manager upon destruction
			NT_DELETE_CHUNK( Mem::MC_GFX, m_meshRegistry.back() );
		}
	}
	else
	{
		m_meshRegistry.clear();
	}	
}

void XPUShapeBlendingImpl::BeginUpdate( void )
{
	//! early out if disabled
	if ( !IsEnabled() )
	{	
		return ;
	}

	//! queue the necessary batch infos
	for ( BSMeshRegistry_t::iterator it = m_meshRegistry.begin(); it != m_meshRegistry.end(); it++)
	{
		//! buffer needs to be reset regardless of the blendshape state
		//(*it)->ResetVertexBuffer();

		if ( (*it)->NeedsUpdating() )
		{
			(*it)->ResetVertexBuffer();
			BatchAndSendToSPU_NEW( *it );
		}
	}

#	ifdef BS_IMMEDIATE_SYNC
	Exec::FrameEnd();
	Exec::FrameReset();
#	endif //BS_IMMEDIATE_SYNC
}

void XPUShapeBlendingImpl::Flush( void )
{
#	ifdef BS_FLUSH_SYNC
	user_warn_p( *m_pSpuTaskCounter >= 0, ("BS - SpuTaskCounter is < 0") );
	if ( *m_pSpuTaskCounter > 0 )
	{
		Exec::FrameEnd();
		Exec::FrameReset();
	}
#	endif // BS_FLUSH_SYNC
}



inline u_int alignedBufferSize( u_int numOfElems, size_t elemStride )
{
	ntAssert( numOfElems * elemStride > 0 );
	return ROUND_POW2( numOfElems * elemStride, 16 );
}




void SPUBlend(	const SPUProgram* pSPUBlender,
				const void* pTargets, 
				const float* pWeights, 
				u_int numOfTargets,
				uint8_t* pVertexBuffer,
				u_int indexOffset,
				const void*	pReconstructionMatrices, 
				u_int numOfVerts,
				u_int vertStride )
{
	using namespace blendshapes;

	ntError( pSPUBlender );
	ntError( pTargets );
	ntError( pWeights );
	ntError( pVertexBuffer );
	ntError( pReconstructionMatrices );

	DMABuffer targetsDMABuffer(	pTargets, alignedBufferSize(numOfTargets, sizeof(BSTarget)) ); 
	DMABuffer weightsDMABuffer(	pWeights, alignedBufferSize(numOfTargets, sizeof(float)) );
	DMABuffer verticesDMABuffer( pVertexBuffer, alignedBufferSize(numOfVerts, vertStride) );
	DMABuffer matricesDMABuffer( pReconstructionMatrices, sizeof(CMatrix)*2 );

	SPUTask spu_task( pSPUBlender );

	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputAndOutput,	verticesDMABuffer ),			BS_SPU_PARAM_VERTEX_BUFFER );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			targetsDMABuffer ),				BS_SPU_PARAM_TARGETS_BUFFER );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			weightsDMABuffer ),				BS_SPU_PARAM_WEIGHTS_BUFFER );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			numOfTargets ),					BS_SPU_PARAM_NUM_OF_TARGETS );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			indexOffset ),					BS_SPU_PARAM_INDEX_OFFSET );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			numOfVerts ),					BS_SPU_PARAM_NUM_OF_VERTS );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			vertStride ),					BS_SPU_PARAM_VERTEX_STRIDE );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			matricesDMABuffer ),			BS_SPU_PARAM_MATRICES_BUFFER );

	Exec::RunTask( &spu_task );
}


void SPUBlend_NEW(	const SPUProgram* pSPUBlender,
				const void* pTargets, 
				const float* pWeights, 
				const BSSpuAdditionalInfo* pAdditionalInfo,
				uint8_t* pVertexBuffer,
				const void*	pReconstructionMatrices, 
				int* pCounter )
{
	using namespace blendshapes;

	ntError( pSPUBlender );
	ntError( pTargets );
	ntError( pWeights );
	ntError( pVertexBuffer );
	ntError( pReconstructionMatrices );

	uint32_t counter_EA = (uint32_t)(uintptr_t)pCounter;
	uint32_t vertices_EA = (uint32_t)(uintptr_t)pVertexBuffer;
	uint32_t numOfTargets = pAdditionalInfo->m_iNumOfTargets;

	DMABuffer targetsDMABuffer(	pTargets, alignedBufferSize(numOfTargets, sizeof(BSTarget)) ); 
	DMABuffer weightsDMABuffer(	pWeights, alignedBufferSize(numOfTargets, sizeof(float)) );
	DMABuffer verticesDMABuffer( pVertexBuffer, alignedBufferSize(pAdditionalInfo->m_iNumOfVertices, pAdditionalInfo->m_iVertexStride) );
	DMABuffer matricesDMABuffer( pReconstructionMatrices, sizeof(CMatrix)*2 );
	DMABuffer additionalInfoDmaBuffer( pAdditionalInfo, alignedBufferSize(1,sizeof(BSSpuAdditionalInfo)) );

	SPUTask spu_task( pSPUBlender );

	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			vertices_EA ),					BS_SPU_PARAM_VERTICES );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			targetsDMABuffer ),				BS_SPU_PARAM_TARGETS );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			weightsDMABuffer ),				BS_SPU_PARAM_WEIGHTS );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			matricesDMABuffer ),			BS_SPU_PARAM_MATRICES );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			additionalInfoDmaBuffer ),		BS_SPU_PARAM_ADDITIONAL_INFO );
	spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly,			counter_EA ),					BS_SPU_PARAM_TASKCOUNTER );


	AtomicIncrement( pCounter );
	Exec::RunTask( &spu_task );
}

void XPUShapeBlendingImpl::BatchAndSendToSPU_NEW( BlendedMeshInstance* pMesh )
{
	MeshBSSetPtr_t pBlendShapes = pMesh->GetBlendShapes();

	uint8_t* pVBStart = (uint8_t*)pMesh->GetVertexBufferWriteAddress();

	SPUBlend_NEW( m_pSpuBlender, 
		pBlendShapes->GetBSTargetHeadersPtr(),
		pBlendShapes->GetTargetWeights(),
		pMesh->GetSpuAdditionalInfo(),
		pVBStart,
		pMesh->m_obStreamMatrices,
		m_pSpuTaskCounter );
}

void XPUShapeBlendingImpl::BatchAndSendToSPU( BlendedMeshInstance* pMesh )
{
	MeshBSSetPtr_t pBlendShapes = pMesh->GetBlendShapes();

	uint8_t* pVBStart = (uint8_t*)pMesh->GetVertexBufferWriteAddress();

	u_int vertCount = pMesh->m_hVertexBuffer[0]->GetCount();
	u_int vertStride =  pMesh->m_hVertexBuffer[0]->GetStride();
	u_int indexOffset = 0;
	
	while ( vertCount > 0 )
	{
		u_int batchVertCount = ntstd::Min( m_vertsPerBatch, vertCount );
		ntAssert( batchVertCount >= 0 );
	
		SPUBlend( m_pSpuBlender, 
			pBlendShapes->GetBSTargetHeadersPtr(),
			pBlendShapes->GetTargetWeights(),
			pBlendShapes->GetNumOfBSTargets(),
			pVBStart,
			indexOffset,
			pMesh->m_obStreamMatrices,
			batchVertCount,
			vertStride );

		vertCount -= batchVertCount;
		pVBStart += batchVertCount * vertStride; 
		indexOffset += batchVertCount;
	}
}
//eof

