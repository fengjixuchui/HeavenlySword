//--------------------------------------------------
//!
//!	\file bs_spu.h
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

#include <basetypes_spu.h>
#include <debug_spu.h>

#include "ntlib_spu/util_spu.h"

#ifndef ntDMA_ID
typedef uint32_t ntDMA_ID;
#endif


//
//	dma lists stopped working in debug mode for some reason so I had to use two separete dmas for
//	indices and deltas. 
//	TODO_OZZ: look into it or put indices and deltas into a nice little struct and dma'em together
//
#define BS_NO_DMA_LIST


class	CMatrix;
struct	BSTarget;
struct	BSDeltasDmaBuffer;


class BSVertexDmaBuffer
{
public:
	void Init( uint32_t numOfVertices, uint32_t vertexStride );
	
	void FreeDmaResources( void );

	void DmaIn( uint32_t ea, uint32_t n );
	
	void DmaOut( void );

	uint8_t*	 GetPtr( void );

private:
	uint8_t*	m_pMem;
	uint32_t	m_iVertexStride;
	uint32_t	m_iVertexCount;
	uint32_t	m_iMaxNumOfVertices;
	uint32_t	m_ea;

	ntDMA_ID	m_dmaId;
};


//--------------------------------------------------
//!
//!	Blendshapes Dma info
//!	all the necessary info for multi-buffering
//! dma transfers and vertex blending
//!
//--------------------------------------------------
class BSDeltasDmaBuffer
{
public:

	// gets local storage, and sets the matrices
	void Init( const CMatrix* pReconMatrix, const CMatrix* pReconMatrixInv, int vertexStride, uint32_t maxDeltas );
	
	// frees the dma_id
	void FreeDmaResources( void );
	
	// starts the dma in of a given target
	void DmaIn( const BSTarget* pTarget, float weight );

	// apply this blend info the the given vertex buffer
	void Apply( uint8_t* pVerts, uint32_t numOfVerts, size_t vertexStride, uint32_t totalIndexOffset );


private:
	//
	//	target specific
	//
	ntDMA_ID			m_id;						//! the ID for checking this DMA is done
	uint8_t*			m_pMem;						//! ptr to the starting point of this dma local storage
	uint32_t			m_iDeltasLSOffset;			//! deltas start at m_pMem + this offset

	const BSTarget*		m_pTarget;					//! the associated target 
	float				m_fWeight;					//! the current target weight		

	// 
	// set only once per run
	//
	int					m_iVertexStride;
	const CMatrix*		m_pReconMat;			//! reconstruction matrix
	const CMatrix*		m_pReconMatInv;			//! reconstruction matrix inverse
	int					m_iIndexOffset;
	uint32_t			m_iMaxNumOfDeltas;

	//
	// TMP	
	//
	bool				m_bCompressedPos;

#ifdef BS_NO_DMA_LIST
	ntDMA_ID			m_id2;
	uint8_t*			m_pMem2;
#endif
};

#endif // end of _BS_SPU_DMA_H_



