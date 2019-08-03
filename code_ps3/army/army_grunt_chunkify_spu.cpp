/***************************************************************************************************
*
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/fixups_spu.h"
#include "ntlib_spu/exec_spu.h"

#include "army/army_ppu_spu.h"
#include "army/battalion.h"
#include "army/battlefield.h"
#include "army/battlefield_chunk.h"
#include "army/grunt.h"

#include <stdlib.h> // for qsort
#include <string.h> // memset

//-----------------------------------------------------
//!
//!	QsortGruntComparator
//! sort grunts by battlefield chunk
//!
//-----------------------------------------------------
int QsortGruntComparator( const void* a, const void* b )
{
	const GruntGameState* bindA = static_cast<const GruntGameState*>(a);
	const GruntGameState* bindB = static_cast<const GruntGameState*>(b);

	// secondary sort creteria keep the grunt ID monotous
	if( bindA->m_ChunkIndex == bindB->m_ChunkIndex )
	{
		return (bindA->m_GruntID - bindB->m_GruntID);
	}
	return (bindA->m_ChunkIndex - bindB->m_ChunkIndex);
}
//------------------------------------------------------
//!
//! we need to grunt array sorting on battlefield chunk
//! ID (essentially performing a crude spatial search)
//! we can't use an incremental search algorithm, as this
//! grunt array is being changed by N processors simul
//! this sort is guarenteed single access, so can rearrange
//! to its hearts content.
//!
//! The entire grunt array is in SPU LS, so its blindly fast
//! to move 128 byes around so the sort should be fairly quick.
//! though the size of each item may well effect how fast...
//!
//------------------------------------------------------
void GruntChunkifyAndSort( SPUArgumentList &params )
{
	// in GRUNT_SORT mode this is an input/output dma buffer, input only for the other modes
	GetArrayInput( BattalionArray*,				pBattalionArray,			SRTA_BATTALION_ARRAY );
	GetArrayInput( BattlefieldHeader*,			pBattlefieldHeader,			SRTA_BATTLEFIELD_HEADER );
	GetArrayInput( GruntArray*,					pGruntArray,				SRTA_GRUNTS );
	GetArrayInput( BattlefieldChunksHeader*,	pChunksHeader,				SRTA_BATTLEFIELD_CHUNKS_HEADER );
	GetArrayInput( ExecSPUJobAdder*,			pArmyAdder,					SRTA_ARMY_JOB_ADDER );
	GetU32Input(								render_barrier_ea,			SRTA_ARMY_BARRIER_EA );

	GruntGameState* pGrunts = (GruntGameState*)(pGruntArray+1);

	// we want the first grunt index for each chunk, -1 indicates no grunt
	uint16_t* iChunkGruntIDs = (uint16_t*) Allocate( sizeof(uint16_t) * pChunksHeader->m_iNumChunks );
	memset( iChunkGruntIDs, 0xFFFF, sizeof(uint16_t) * pChunksHeader->m_iNumChunks );

	// for each grunt decide what it chunk id 
	for( uint16_t i=0;i < pGruntArray->m_iNumGrunts;i++ )
	{
		pGrunts[i].m_ChunkIndex = BF_GetChunkIndex( &pGrunts[i].m_Position, pBattlefieldHeader );
		ntAssert( pGrunts[i].m_ChunkIndex < pChunksHeader->m_iNumChunks );

		// this will give us the first grunt ID
		if( pGrunts[i].m_GruntID < iChunkGruntIDs[ pGrunts[i].m_ChunkIndex ] )
		{
			iChunkGruntIDs[ pGrunts[i].m_ChunkIndex ] = pGrunts[i].m_GruntID;
		}
	}


	// we sort the grunts list, so we can then do a bsearch and linear walk to find all the 
	// grunts in a particular battlefield chunk (which are spatially ordered)
	qsort( pGrunts, pGruntArray->m_iNumGrunts, sizeof(GruntGameState), QsortGruntComparator ); 

	// now build the id to index table...
	uint16_t* pIndexTable = (uint16_t*) (pGrunts + pGruntArray->m_iNumGrunts);
	for( uint16_t i=0;i < pGruntArray->m_iNumGrunts;i++ )
	{
		ntAssert( pGrunts[i].m_GruntID < pGruntArray->m_iNumGrunts );
		pIndexTable[ pGrunts[i].m_GruntID ] = i;
	}

	//-------------
	// We are now going to DMA the GruntArray back to main memory 
	// this way we get some overlap between the task list builder and the dma
	ntDMA::Params gruntsDmaParams;
	ntDMA_ID gruntsId = ntDMA::GetFreshID();
	gruntsDmaParams.Init32( pGruntArray, g_DMAEffectiveAddresses[ SRTA_GRUNTS ], pGruntArray->m_iSize , gruntsId );
	ntDMA::DmaToPPU( gruntsDmaParams );

	// now kick of battlefield chunk updates for each non-empty chunk
	// the dependency counter will ensure sychronisation

	
	
	
	
//	ntBreakpoint();
	uint16_t iNumTasks = 0;
	for( uint16_t i=0;i < pChunksHeader->m_iNumChunks;i++)
	{
		// skip empty chunks 
		if ( iChunkGruntIDs[i] != 0xFFFF )
		{
			iNumTasks++;
		}
	}
	Exec::InitDependency( render_barrier_ea, iNumTasks, pArmyAdder );

	
	
	
	
	// remember you we need the EA address which aren't in our passed in DMABuffer!!
	ntAssert( pGruntArray->m_iSize == params.Get( SRTA_GRUNTS )->GetBuffer()->GetSize() );

	DMABuffer pGruntsDMA( g_DMAEffectiveAddresses[ SRTA_GRUNTS ], params.Get( SRTA_GRUNTS )->GetBuffer()->GetSize() );
	DMABuffer pBattalionArrayDMA( g_DMAEffectiveAddresses[ SRTA_BATTALION_ARRAY ], params.Get( SRTA_BATTALION_ARRAY )->GetBuffer()->GetSize() );
	DMABuffer pBattlefieldHeaderDMA( g_DMAEffectiveAddresses[ SRTA_BATTLEFIELD_HEADER ], params.Get( SRTA_BATTLEFIELD_HEADER )->GetBuffer()->GetSize() );

	SPUArgumentList argList;
	argList.Set( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t) GRUNT_LOGIC_EXECUTE ),	SRTA_RUN_TYPE );
	argList.Set( SPUArgument( SPUArgument::Mode_InputOnly, pBattalionArrayDMA ),				SRTA_BATTALION_ARRAY );
	argList.Set( SPUArgument( SPUArgument::Mode_InputOnly, pBattlefieldHeaderDMA ),				SRTA_BATTLEFIELD_HEADER );
	argList.Set( SPUArgument( SPUArgument::Mode_InputOnly, pGruntsDMA ),						SRTA_GRUNTS );

	for( uint16_t i=0;i < pChunksHeader->m_iNumChunks;i++)
	{
		// skip empty chunks 
		if( __builtin_expect( (iChunkGruntIDs[i] != 0xFFFF), 1) )
		{
			uint32_t* pChunksEA = (uint32_t*)pChunksHeader->m_pChunks;
			DMABuffer pChunkDMA( pChunksEA[i], DMABuffer::DMAAllocSize( sizeof(BattlefieldChunk) ) );
			argList.Set( SPUArgument(SPUArgument::Mode_InputOnly, pChunkDMA ), SRTA_BATTLEFIELD_CHUNK );
			// send the first grunt index over for this chunk
			argList.Set( SPUArgument(SPUArgument::Mode_InputOnly, (uint32_t) pIndexTable[ iChunkGruntIDs[i] ] ), SRTA_FIRST_GRUNT_INDEX );
//			ntPrintf( "Kick task for chunk %i\n", i );
			Exec::RunTask( pArmyAdder, &argList, render_barrier_ea );
		}
	}



	// this will cause a stall so the unit render task is okay
	Exec::AddBarrierJob( pArmyAdder, render_barrier_ea );

	// now install the render task to should be stalled by the above barrier job
	DMABuffer pRenderDestDMA( pArmyAdder->m_data0[RJAP_DEST_GRUNTS_EA], sizeof( GruntRenderState ) * pGruntArray->m_iNumGrunts );
	DMABuffer pRenderAllocDMA( pBattlefieldHeader->m_eaGruntRenderAllocator, sizeof(ArmyGruntRenderAllocator) );
		
	// now send off the unit render task
	SPUArgumentList unitList;
	unitList.Set( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t) UNIT_RENDER ),	SRTA_RUN_TYPE );
	unitList.Set( SPUArgument( SPUArgument::Mode_InputOnly,  pBattalionArrayDMA ),		SRTA_BATTALION_ARRAY );
	unitList.Set( SPUArgument( SPUArgument::Mode_InputOnly,  pBattlefieldHeaderDMA ),	SRTA_BATTLEFIELD_HEADER );
	unitList.Set( SPUArgument( SPUArgument::Mode_InputOnly,  pGruntsDMA ),				SRTA_GRUNTS );
	unitList.Set( SPUArgument( SPUArgument::Mode_OutputOnly, pRenderAllocDMA ),			SRTA_GRUNT_RENDER_ALLOCATOR );
	unitList.Set( SPUArgument( SPUArgument::Mode_OutputOnly, pRenderDestDMA ),			SRTA_RENDER_GRUNTS );

	
	//	Exec::RunTask( pRenderAdder, &unitList );
	Exec::RunTask( pArmyAdder, &unitList );

	Exec::JobApiSetReadyCount( pArmyAdder->m_eaSpurs, pArmyAdder->m_workloadId, pArmyAdder->m_NumWWSJobManagerSPUs );
	ntDMA::StallForCompletion( gruntsId );
	ntDMA::FreeID( gruntsId );

}
