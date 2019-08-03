//--------------------------------------------------
//!
//!	\file water_spu_main.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/fixups_spu.h"
#include "ntlib_spu/ntDma.h"
#include "ntlib_spu/exec_spu.h"

#include "water/WaterDmaData.h"
#include "spu/water/water.h"
#include "spu/water/water_utils.h"
#include "spu/water/water_spu_config.h"
#include "core/wrapindex.h"

#include <stdlib.h>
#include <string.h>

#include "core/perfanalyzer_spu_ps3.h"

SPU_MODULE_FIXUP()



#define HUGE_HEIGHT  99999.9f
#define TINY_HEIGHT  -99999.9f

#ifndef _RELEASE 
#	define CHECK_WAVE_DATA( _wave ) //CheckWaveData( _wave );
#else
#	define CHECK_WAVE_DATA( _wave )
#endif


void CheckWaveData( const WaveDma& wave );

void SortAndUpdateWaveDmaArray( WaveDma* pArray, uint32_t n, float dt );


// vertices per dma transfer should always be aligned to 4
const static uint32_t VerticesPerDmaTransfer = 1024; //Util::Align( ntDMA::MaxSingleDMATransferSize / sizeof(VertexDma), 4);
const static uint32_t NumOfVertexBuffers = 2;

 
WaterDmaBuffer	waterVertexBuffers[ NumOfVertexBuffers ];




extern "C" void SpuMain( SPUArgumentList &params )
{
	INSERT_PA_BOOKMARK ( PABM_SPU_WATER );

	GetArrayInput(	WaterInstanceDma*,		pWaterInstance,		WATER_SPU_PARAM_WINSTANCE );	
	GetArrayOutput(	WaveDma*,				pWaveArray,			WATER_SPU_PARAM_WAVEARRAY );
//	GetArrayOutput( BuoyDma*,				pBuoyArray,			WATER_SPU_PARAM_BUOYARRAY );
	GetU32Input(							Grid_EA,			WATER_SPU_PARAM_VERTARRAY );

	WrapIndex<NumOfVertexBuffers> bufferIndex;
	v128 max_height = spu_splats( TINY_HEIGHT );
	v128 min_height = spu_splats( HUGE_HEIGHT );

	//ntPrintf( "*** batch size: %i. Sizeof(VertexDma): %i ***\n", VerticesPerDmaTransfer, sizeof(VertexDma) );
	//ntPrintf( "t:%f, u:%i, v:%i\n",  pWaterInstance->m_fTime, pWaterInstance->m_iGridSizeU, pWaterInstance->m_iGridSizeV );

	//! t = 0 means water got sent to the spu before update
	ntAssert( pWaterInstance->m_fTime >= 0.0f );
	ntAssert( pWaterInstance->m_fTimeStep >= 0.0f );

	//! reserve our dma resources
	for ( uint32_t i = 0; i < NumOfVertexBuffers; ++i )
	{
		waterVertexBuffers[i].Init( pWaterInstance, pWaveArray, VerticesPerDmaTransfer, MAX_NUM_WAVES );
	}

	SortAndUpdateWaveDmaArray( pWaveArray, MAX_NUM_WAVES, pWaterInstance->m_fTimeStep );

	uint32_t batchCount;
	uint32_t vertexCount = pWaterInstance->m_iGridSize[0] * pWaterInstance->m_iGridSize[1];
	uint32_t ea = Grid_EA;
	uint32_t offset = 0;
	while (  vertexCount > 0 )
	{
		batchCount = ntstd::Min( vertexCount, VerticesPerDmaTransfer );
		ntAssert( Util::IsAligned(batchCount,4) );
		ntAssert( batchCount >= 0 );
		
		waterVertexBuffers[bufferIndex].DmaIn( ea, batchCount, offset );
		waterVertexBuffers[++bufferIndex].ProcessAndDmaOut( max_height, min_height );

		offset += batchCount;
		vertexCount -= batchCount;
		ea += (uint32_t)(batchCount * sizeof(VertexDma));
	}
	waterVertexBuffers[++bufferIndex].ProcessAndDmaOut( max_height, min_height );

	//! update aabb info on water data
	pWaterInstance->m_fMaxHeight = MaxElem( max_height );
	pWaterInstance->m_fMinHeight = MinElem( min_height );	

	//! stall for last dma completition and release ids
	for ( uint32_t i = 0; i < NumOfVertexBuffers; ++i )
	{
		waterVertexBuffers[i].Release();
	}
}

int wave_qsort_cmp( const void* a, const void* b )
{
	const WaveDma* bindA = static_cast<const WaveDma*>(a);
	const WaveDma* bindB = static_cast<const WaveDma*>(b);

	// 1st criteria: push invalid waves to the back
	int result = (bindA->m_iFlags & kWF_Control_Invalid) - (bindB->m_iFlags & kWF_Control_Invalid);

	// 2nd criteria: sort by type
	if ( !result )
	{
	//	result = (bindA->m_iFlags & kWF_Type_All) - (bindB->m_iFlags & kWF_Type_All);
	}


	result = ( bindA->IsValid() && !bindB->IsValid() ) ? -1 : 0;

	return result;
}


void SortAndUpdateWaveDmaArray( WaveDma* pArray, uint32_t n, float dt )
{
	//! wave update is now performed on the PPU
	//for ( uint32_t i = 0; i < n; ++i )
	//{
	//	pArray[i].Update( dt );
	//	CHECK_WAVE_DATA( pArray[i] )
	//}
#ifdef WATER_SORT_WAVE_ARRAY
	qsort( pArray, n, sizeof(WaveDma), wave_qsort_cmp );
#endif
}



//
// Debug stuff
//
void CheckWaveData( const WaveDma& wave )
{
	ntAssert( wave.m_fAge >= 0 );
	ntAssert( wave.m_fAmplitude >= 0 );
	ntAssert( wave.m_fFrequency >= 0 );
	ntAssert( wave.m_fPhase >= 0 );

	// if wave is valid, then its direction must be normalised
	ntAssert( wave.IsValid() || abs(wave.m_obDirection.Length() - 1.0f) < EPSILON );
}


