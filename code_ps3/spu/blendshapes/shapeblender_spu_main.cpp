//--------------------------------------------------
//!
//!	\file shapeblender_spu_main.h
//! SUPERC
//! This is a SuperC implementation, it should be
//! treated as C code using C++ style syntax and not
//! true C++ code. In particular, ctors must not be
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//!
//--------------------------------------------------

#include "bs_spu.h"
#include "blendshapes/blendshapes_export.h"
#include "blendshapes/blendshapes_constants.h"
#include "blendshapes/blendshapesbatchinfo_spu_ppu.h"
#include "core/wrapindex.h"

#include <basetypes_spu.h>
#include <debug_spu.h>
#include "ntlib_spu/fixups_spu.h"
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/syncprims_spu.h"

#include "core/perfanalyzer_spu_ps3.h"

SPU_MODULE_FIXUP();


static const uint32_t NumOfDeltasDmaSlots	= 2;
static const uint32_t NumOfVertexDmaSlots	= 2;

static const uint32_t MaxVerticesPerBatch	= 1024;
static const uint32_t MaxDeltasPerBatch		= blendshapes::MAX_DELTAS;

using namespace blendshapes;

extern "C" void SpuMain( SPUArgumentList &params )
{	
	INSERT_PA_BOOKMARK ( PABM_SPU_BLENDSHAPES );

	GetU32Input(								vertices_ea,			BS_SPU_PARAM_VERTICES );	
	GetArrayInput(	BSTarget* ,					pTargetArray,			BS_SPU_PARAM_TARGETS );		
	GetArrayInput(	float*,						pTargetWeights,			BS_SPU_PARAM_WEIGHTS );
	GetArrayInput( CMatrix*,					reconstMatrices,		BS_SPU_PARAM_MATRICES );
	GetArrayInput( BSSpuAdditionalInfo*,		pAdditionalInfo,		BS_SPU_PARAM_ADDITIONAL_INFO );
	GetU32Input(								counter_ea,				BS_SPU_PARAM_TASKCOUNTER );

	BSDeltasDmaBuffer deltasDmaBuffers[ NumOfDeltasDmaSlots ];
	for(uint32_t iSlot = 0 ; iSlot < NumOfDeltasDmaSlots ; ++iSlot )
	{
		deltasDmaBuffers[iSlot].Init( reconstMatrices, reconstMatrices + 1, pAdditionalInfo->m_iVertexStride, MaxDeltasPerBatch );
	}

	BSVertexDmaBuffer vertexDmaBuffers[ NumOfVertexDmaSlots ];
	for(uint32_t iSlot = 0 ; iSlot < NumOfVertexDmaSlots ; ++iSlot )
	{
		vertexDmaBuffers[iSlot].Init( MaxVerticesPerBatch, pAdditionalInfo->m_iVertexStride );
	}

	WrapIndex<NumOfDeltasDmaSlots> deltasSlotIndex;
	WrapIndex<NumOfVertexDmaSlots> vertexSlotIndex;

	uint32_t vertexCount = pAdditionalInfo->m_iNumOfVertices;
	uint32_t vertexBatchCount = 0;
	uint32_t totalIndexOffset = 0;
	
	while( vertexCount > 0 )
	{
		vertexBatchCount = ntstd::Min( vertexCount, MaxVerticesPerBatch );
		vertexDmaBuffers[vertexSlotIndex].DmaIn( vertices_ea + totalIndexOffset * pAdditionalInfo->m_iVertexStride, vertexBatchCount );

		for ( uint32_t iTarget = 0; iTarget < pAdditionalInfo->m_iNumOfTargets; ++iTarget )
		{
			const float weight = pTargetWeights[ iTarget ];
			
			// if target weights is above our given threshold we can process it in
			if ( fabs(weight) > EPSILON )
			{
				deltasDmaBuffers[deltasSlotIndex].DmaIn( pTargetArray + iTarget, weight );
				deltasDmaBuffers[++deltasSlotIndex].Apply( vertexDmaBuffers[vertexSlotIndex].GetPtr(), vertexBatchCount, pAdditionalInfo->m_iVertexStride, totalIndexOffset );
			}
		}
		// don't forget about the last one!
		deltasDmaBuffers[++deltasSlotIndex].Apply( vertexDmaBuffers[vertexSlotIndex].GetPtr(), vertexBatchCount, pAdditionalInfo->m_iVertexStride, totalIndexOffset );
		
		vertexCount -= vertexBatchCount;
		totalIndexOffset += vertexBatchCount;

		vertexDmaBuffers[++vertexSlotIndex].DmaOut();
	}
	vertexDmaBuffers[++vertexSlotIndex].DmaOut();


	//ntPrintf("\n releasing resources.... " );

	for(uint32_t iSlot = 0 ; iSlot < NumOfDeltasDmaSlots ; ++iSlot )
	{
		deltasDmaBuffers[iSlot].FreeDmaResources();
	}

	for(uint32_t iSlot = 0 ; iSlot < NumOfVertexDmaSlots ; ++iSlot )
	{
		vertexDmaBuffers[iSlot].FreeDmaResources();
	}

	// let me manager know we're done (sorta)
	AtomicDecrement( counter_ea );
	//ntPrintf(" done! \n" );
}



//eof


