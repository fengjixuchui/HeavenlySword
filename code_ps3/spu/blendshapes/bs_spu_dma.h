//--------------------------------------------------
//!
//!	\file bs_spu_dma.h
//! SUPERC
//! This is a SuperC implementation, it should be
//! treated as C code using C++ style syntax and not
//! true C++ code. In particular, ctors must not be
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//!
//--------------------------------------------------

#ifndef _BS_SPU_DMA_H_
#define _BS_SPU_DMA_H_


#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/ntDmaList.h"
#include "blendshapes/blendshapes_export.h"
#include "blendshapes/blendshapes_constants.h"


void blend_postions( uint8_t* pVerts, 
				  uint16_t* pIndices , 
				  delta_t* pDeltas, 
				  float deltaScale, 
				  float weight, 
				  int nDeltas, 
				  int nVerts,
				  int vertStride )
{
	float scaleAndWeight = deltaScale * weight;

	for(int iDelta = 0 ; iDelta < nDeltas ; ++iDelta )
	{
		// real vertex index
		uint16_t index = pIndices[ iDelta ];

		if ( __builtin_expect( index >= nVerts , 0 ) )
			return;

		float* pPos = (float*)( pVerts + index * vertStride );
		delta_t* pDelta = pDeltas + iDelta * 3;
	
		pPos[0] += scaleAndWeight * (float)pDelta[0];
		pPos[1] += scaleAndWeight * (float)pDelta[1];
		pPos[2] += scaleAndWeight * (float)pDelta[2];
	}
}


//--------------------------------------------------
//!
//!	Blendshapes Dma info
//!	all the necessary info for multi-buffering
//! dma transfers and vertex blending
//!
//--------------------------------------------------
struct BlendDmaInfo
{
	//
	// exposed members
	//
	ntDMA_ID	m_id;						//! the ID for checking this DMA is done
	uint32_t	m_indicesDmaSize;			//! the actual size of data. Note that this implicitly defines the size of deltas as well
	uint8_t*	m_pMem;						//! ptr to the starting point of this dma local storage
	uint32_t	m_numOfDeltas;				//! how many deltas?
	float		m_scale;					//! unpacking scale factor
	float		m_weight;					//! anim weight for this target
	uint32_t	m_numOfVerts;				//! number of vertices to blend
	size_t		m_vertexStride;				//! duh

	//
	// some nice helper functions
	//
	void DMA_In( const BSTarget* pTarget );

	void Update( uint8_t* pVerts, uint32_t numOfVerts, size_t vertexStride, float blendThreshold )
	{
		if ( m_weight > blendThreshold )
		{
			//! stall for completition of this dma if necessary
			ntDMA::StallForCompletion( m_id );

			//! Yeah! blend me baby!
			blend_postions( pVerts,
				(uint16_t*)m_pMem, 
				(delta_t*)(m_pMem + m_indicesDmaSize), 
				m_scale, 
				m_weight, 
				m_numOfDeltas, 
				numOfVerts,
				vertexStride );
		}
	}
};

#endif // end of _BS_SPU_DMA_H_
