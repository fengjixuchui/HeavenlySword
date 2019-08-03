//--------------------------------------------------
//!
//!	\file bs_spu.cpp
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

#include <basetypes_spu.h>
#include <debug_spu.h>
#include <string.h>
#include "ntlib_spu/fixups_spu.h"
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/ntDmaList.h"
#include <float.h>
#include <limits.h>

using namespace blendshapes;


static const float SHRT_MAX_INV = 1.0f / SHRT_MAX;
static const float FLT_MAX_INV = 1.0f / FLT_MAX;

static const float SHRT_RANGE = SHRT_MAX - SHRT_MIN;
static const float SHRT_RANGE_INV = 1.0f / SHRT_RANGE;


v128 unpack_position( const CMatrix& mat, const short* pos );
void repack_position( const CMatrix& mat, v128 src, short* dst );

bool is_identity( const CMatrix& mat );


void blend_compressed_pos( 
					const CMatrix& reconMat,
					const CMatrix& reconMatInv,
					uint8_t* pVerts, 
					uint16_t* pIndices, 
					delta_t* pDeltas, 
					float deltaScale, 
					float weight, 
					int nDeltas, 
					int nVerts,
					int vertStride, 
					int offset );


void blend_pos2( 
					const CMatrix& reconMat,
					const CMatrix& reconMatInv,
					uint8_t* pVerts, 
					uint16_t* pIndices , 
					delta_t* pDeltas, 
					float deltaScale, 
					float weight, 
					int nDeltas, 
					int nVerts,
					int vertStride, 
					int offset );


void BSDeltasDmaBuffer::Init( const CMatrix* pReconMatrix, const CMatrix* pReconMatrixInv, int vertexStride, uint32_t maxDeltas ) 
{
	m_pReconMat = pReconMatrix;
	m_pReconMatInv = pReconMatrixInv;
	m_iVertexStride = vertexStride;
	m_iMaxNumOfDeltas = maxDeltas;

	m_id = ntDMA::GetFreshID();
	m_iDeltasLSOffset = 0;
	m_pTarget = 0;
	m_fWeight = 0;	

	m_bCompressedPos = !is_identity( *m_pReconMat );

	uint32_t indicesBufferSize = m_iMaxNumOfDeltas * sizeof(uint16_t);
	uint32_t deltasBufferSize = m_iMaxNumOfDeltas * 3 * sizeof(delta_t);
#ifdef BS_NO_DMA_LIST
	m_pMem = static_cast<uint8_t*>( Allocate( indicesBufferSize ) );
	ntError( m_pMem != NULL );

	m_id2 = ntDMA::GetFreshID();
	m_pMem2 = static_cast<uint8_t*>( Allocate( deltasBufferSize ) );
	ntError( m_pMem2 != NULL );
#else
	m_pMem = static_cast<uint8_t*>( Allocate( indicesBufferSize + deltasBufferSize ) );
	ntError( m_pMem != NULL );
#endif

}

void BSDeltasDmaBuffer::FreeDmaResources( void )
{
	ntDMA::StallForCompletion( m_id );
	ntDMA::FreeID( m_id );

#ifdef BS_NO_DMA_LIST
	ntDMA::StallForCompletion( m_id2 );
	ntDMA::FreeID( m_id2 );
#endif
}

void BSDeltasDmaBuffer::DmaIn( const BSTarget* pTarget, float weight )
{
	//ntError(pTarget);

	m_pTarget = pTarget;

	if ( m_pTarget->m_numOfDeltas )
	{
//		ntPrintf("DMA_In(%i) 0x%x and 0x%x...\n", m_id, (uintptr_t)m_pTarget->m_pIndices, (uintptr_t)m_pTarget->m_pDeltas  );

		m_fWeight = weight;

		uint32_t dmaNumOfDeltas = ntstd::Min( m_pTarget->m_numOfDeltas, m_iMaxNumOfDeltas );
		uint32_t indicesDMASize = Util::Align( dmaNumOfDeltas*sizeof(uint16_t), 16 );
		uint32_t deltasDMASize = Util::Align( dmaNumOfDeltas*3*sizeof(delta_t), 16 );


		//
		//  HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! 
		//
		// this should NEVER happen. However, it is! oh well... this will do for now... 
		// TODO_OZZ: please look into it asap
		if ( !(Util::IsAligned((uintptr_t)m_pTarget->m_pIndices,16) && Util::IsAligned((uintptr_t)m_pTarget->m_pDeltas,16) ) )
		{
			ntPrintf("BS_SPU - Oh, oh... ptrs are not aligned. This is weird... and bad! I'll just set weights to zero and ignore the whole thing for now...\n");
			m_fWeight = 0.0f;
			return;
		}
		//	HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT! HACK ALERT!

		//ntAssert( Util::IsAligned((uintptr_t)m_pTarget->m_pIndices,16 ) );
		//ntAssert( Util::IsAligned((uintptr_t)m_pTarget->m_pDeltas,16 ) );

		
#ifdef BS_NO_DMA_LIST
		//memset( m_pMem2, 0, DMA_INDEX_BUFFER_SIZE );
		//memset( m_pMem2, 0, DMA_DELTA_BUFFER_SIZE );

		ntDMA::Params dma_param;
		dma_param.Init32( m_pMem, (uintptr_t)m_pTarget->m_pIndices, indicesDMASize, m_id );
		ntDMA::DmaToSPU( dma_param );

		ntDMA::Params dma_param2;
		dma_param2.Init32( m_pMem2, (uintptr_t)m_pTarget->m_pDeltas, deltasDMASize, m_id2 );
		ntDMA::DmaToSPU( dma_param2 );
#else
		//memset( m_pMem0, 0, DMA_BUFFER_SIZE );
		// dma-in the indices and deltas in one go
		ntDMAList::Params dma_list_params( m_pMem, m_id );
		ntDMAList::AddListElement32( dma_list_params, indicesDMASize, (uintptr_t)m_pTarget->m_pIndices );
		ntDMAList::AddListElement32( dma_list_params, deltasDMASize, (uintptr_t)m_pTarget->m_pDeltas );
		ntDMAList::DmaToSPU( dma_list_params );
#endif

		// blend/dma info needed for each blend slot
		m_iDeltasLSOffset = indicesDMASize;
		
	}
}


void BSDeltasDmaBuffer::Apply( uint8_t* pVerts, uint32_t numOfVerts, size_t vertexStride, uint32_t totalIndexOffset )
{
	if ( m_fWeight != 0 )			//!< this needs profiling. Ok, the whole thing needs profiling but you know what I mean....
	{
		// now, where are my indices and deltas?!
		ntDMA::StallForCompletion( m_id );
#ifdef BS_NO_DMA_LIST
		ntDMA::StallForCompletion( m_id2 );
		delta_t* pDeltas = (delta_t*)m_pMem2;
#else
		delta_t* pDeltas = (delta_t*)(m_pMem + m_iDeltasLSOffset);
#endif

		if ( m_bCompressedPos )
		{
			blend_compressed_pos( *m_pReconMat, *m_pReconMatInv,
				pVerts, 
				(uint16_t*)m_pMem, pDeltas, 
				m_pTarget->m_deltaScale, m_fWeight,
				m_pTarget->m_numOfDeltas, numOfVerts,
				m_iVertexStride,
				totalIndexOffset );
		}
		else
		{
			blend_pos2( *m_pReconMat, *m_pReconMatInv,
				pVerts, 
				(uint16_t*)m_pMem, pDeltas, 
				m_pTarget->m_deltaScale, m_fWeight,
				m_pTarget->m_numOfDeltas, numOfVerts,
				m_iVertexStride,
				totalIndexOffset );
		}
	}

	m_fWeight = 0;
}



void blend_pos2( 
					const CMatrix& reconMat,
					const CMatrix& reconMatInv,
					uint8_t* pVerts, 
					uint16_t* pIndices , 
					delta_t* pDeltas, 
					float deltaScale, 
					float weight, 
					int nDeltas, 
					int nVerts,
					int vertStride, 
					int offset )
{
	float scaleAndWeight = deltaScale * weight;
	for(int iDelta = 0 ; iDelta < nDeltas ; ++iDelta )
	{
		// relative vertex index
		int relativeIndex = pIndices[ iDelta ] - offset;

		// since indices are sorted by index, branch prediction is coherent 
		if ( relativeIndex >= 0 && relativeIndex < nVerts )
		{
			float* pPos = (float*)( pVerts + relativeIndex * vertStride );
			delta_t* pDelta = pDeltas + iDelta * 3;
		
			pPos[0] += scaleAndWeight * (float)pDelta[0];
			pPos[1] += scaleAndWeight * (float)pDelta[1];
			pPos[2] += scaleAndWeight * (float)pDelta[2];
		}
	}
}


void blend_compressed_pos( 
					const CMatrix& reconMat,
					const CMatrix& reconMatInv,
					uint8_t* pVerts, 
					uint16_t* pIndices, 
					delta_t* pDeltas, 
					float deltaScale, 
					float weight, 
					int nDeltas, 
					int nVerts,
					int vertStride, 
					int offset )
{
	// scale factor should leave the 4th component alone. Note that this factor includes
	// the decompression factor for the deltas as well so we don't have to do it individually 
	v128 scale_factor = spu_splats( deltaScale * weight );
	//scale_factor = spu_insert( 0.0f, scale_factor, 3 );

	v128 delta = (v128){ 0.0f, 0.0f, 0.0f, 0.0f };
	v128 position = (v128){ 0.0f, 0.0f, 0.0f, 0.0f };
	short* pPos = 0;
	
	for( int iDelta = 0; iDelta < nDeltas ; ++iDelta )
	{
		// get the vertex index, relative to this batch start
		int relativeIndex = pIndices[ iDelta ] - offset;
		if ( relativeIndex >= 0 && relativeIndex < nVerts )
		{
			// build a pseudo-unpacked delta to be concatenated with the full scale factor later on
			delta = spu_insert( (float)pDeltas[ iDelta*3 + 0 ], delta, 0 );
			delta = spu_insert( (float)pDeltas[ iDelta*3 + 1 ], delta, 1 );
			delta = spu_insert( (float)pDeltas[ iDelta*3 + 2 ], delta, 2 );

			pPos = (short*)( pVerts + relativeIndex * vertStride );

			position = unpack_position( reconMat, pPos );
			position = spu_madd( scale_factor, delta, position );
			repack_position( reconMatInv, position, pPos );
		}
	}
}



inline bool ElemCmpAnyAbsoluteGreaterThan( v128 vec, float value )
{
	vector unsigned int compResult = spu_cmpabsgt( vec, spu_splats(value) );
    vector unsigned int resultVec = spu_gather(compResult);
	return ( spu_extract( resultVec, 0 ) > 0 );
}



inline v128 unpack_position( const CMatrix& mat, const short* pos )
{
	v128 result = (v128){ 0.0f, 0.0f, 0.0f, 0.0f };
	
	// [SHRT_MIN, SHRT_MAX] --> [-1.0f, 1.0f]
	result = spu_insert( ((float)pos[0]) , result, 0 );
	result = spu_insert( ((float)pos[1]) , result, 1 );
	result = spu_insert( ((float)pos[2]) , result, 2 );
	result = spu_mul( result, spu_splats((float)SHRT_MAX_INV) );

//#ifndef _RELEASE
//	if ( ElemCmpAnyAbsoluteGreaterThan( result, 1.0f ) )
//	{
//		float f0 = spu_extract( result, 0 );
//		float f1 = spu_extract( result, 1 );
//		float f2 = spu_extract( result, 2 );
//		ntPrintf( "unpack... %f, %f, %f\n", f0, f1, f2 );
//	}
//#endif

	// [-1.0f, 1.0f] --> [FLT_MIN, FLT_MAX]
	result = result * mat;

	return result;
}




inline void repack_position( const CMatrix& mat, v128 src, short* dst )
{
	// [FLT_MIN, FLT_MAX] --> [-1.0f, 1.0f]
	v128 result = src * mat;

//#ifndef _RELEASE
//	if ( ElemCmpAnyAbsoluteGreaterThan( result, 1.2f ) )
//	{
//		float f0 = spu_extract( result, 0 );
//		float f1 = spu_extract( result, 1 );
//		float f2 = spu_extract( result, 2 );
//		ntPrintf( "repack... %f, %f, %f\n", f0, f1, f2 );
//	}
//#endif

	// [-1.0f, 1.0f] --> [SHRT_MIN, SHRT_MAX] 
	result = spu_mul( result, spu_splats((float)SHRT_MAX) );
	
	dst[0] = (short)spu_extract( result, 0 );
	dst[1] = (short)spu_extract( result, 1 );
	dst[2] = (short)spu_extract( result, 2 );
}


bool is_identity( const CMatrix& mat )
{
	vector unsigned int comp_result = spu_cmpeq( mat[0], VECTORMATH_UNIT_1000 );
	comp_result = spu_and( comp_result, spu_cmpeq( mat[1], VECTORMATH_UNIT_0100 ) );
	comp_result = spu_and( comp_result, spu_cmpeq( mat[2], VECTORMATH_UNIT_0010 ) );
	comp_result = spu_and( comp_result, spu_cmpeq( mat[3], VECTORMATH_UNIT_0001 ) );

	vector unsigned int result_count = spu_gather(comp_result);

	return ( spu_extract(result_count, 0) == 15 );
}



void BSVertexDmaBuffer::Init( uint32_t numOfVertices, uint32_t vertexStride )
{
	m_iMaxNumOfVertices = numOfVertices;
	m_iVertexStride = vertexStride;
	m_iVertexCount = 0;
	
	m_dmaId = ntDMA::GetFreshID();

	uint32_t alignedSize = Util::Align( m_iMaxNumOfVertices * m_iVertexStride, 16 );
	m_pMem = reinterpret_cast<uint8_t*>( Allocate( alignedSize ) );
	ntError_p( m_pMem, ("BS - Unable to allocate vertex LS buffer\n") );
	
}
	
void BSVertexDmaBuffer::FreeDmaResources( void )
{
	ntDMA::StallForCompletion( m_dmaId );
	ntDMA::FreeID( m_dmaId );
}

void BSVertexDmaBuffer::DmaIn( uint32_t ea, uint32_t n )
{
	ntAssert_p( n >= 0 && n <= m_iMaxNumOfVertices, ("nVerts: %i, MaxVerts: %i\n", n, m_iMaxNumOfVertices) );

	ntDMA::StallForCompletion( m_dmaId );
	m_iVertexCount = n;
	m_ea = ea;

	uint32_t alignedSize = Util::Align( m_iVertexCount * m_iVertexStride, 16 );
	
	ntDMA::Params param;
	param.Init32( m_pMem, m_ea, alignedSize, m_dmaId );
	ntDMA::DmaToSPU( param );	
}

void BSVertexDmaBuffer::DmaOut( void )
{
	if ( m_iVertexCount > 0 )
	{
		ntDMA::StallForCompletion( m_dmaId );
		
		uint32_t alignedSize = Util::Align( m_iVertexCount * m_iVertexStride, 16 );

		ntDMA::Params param;
		param.Init32( m_pMem, m_ea, alignedSize, m_dmaId );
		ntDMA::DmaToPPU( param );		
	}

	m_iVertexCount = 0;
}

uint8_t*	 BSVertexDmaBuffer::GetPtr( void )
{
	ntDMA::StallForCompletion( m_dmaId );
	return m_pMem;
}

