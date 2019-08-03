/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Helpers for building a dmalist
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_DMA_LIST_H
#define WWS_JOB_DMA_LIST_H

//--------------------------------------------------------------------------------------------------

struct DmaListElement
{
	U32		m_size;
	U32		m_eaLo;

	void SetDmaListElement( const void* ea, U32 size )
	{
		m_size	= size;
		m_eaLo	= (U32) ea;
	}

	void SetDmaListElementStallAndNotify( const void* ea, U32 size )
	{
		m_size	= size | (0x80000000U);
		m_eaLo	= (U32) ea;
	}
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_DMA_LIST_H */
