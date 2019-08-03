//---------------------------------------------------------------
//!
//! \file ntDMAList.h
//!
//!	Helper functions for DMA transfers initiated from an SPU.
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//---------------------------------------------------------------


#ifndef	NTLIB_NTDMALIST_H_
#define	NTLIB_NTDMALIST_H_

#ifndef __SPU__
#	error This file can only be included in an SPU project.
#endif // !__SPU__

#include "ntlib_spu/ntDMA.h"

// Forward declare the debug checking function.
//template<int MaxNumElements = 32>
//struct ntDMAList { struct Params; struct Debug { void Check( const Params & ); }; };
#ifdef NT_DEBUG_DMA_TRANSFERS
#	define DMALIST_PARAM_CHECK( param )	ntDMAList::Debug::Check( param )
#else
#	define DMALIST_PARAM_CHECK( param ) do {} while ( 0 )
#endif // _DEBUG


/***************************************************************************************************
*
*	DMA List Transfers.
*	===================
*
*	-	DMA list transfers are a special type of DMA transfer; they allow the caller to perform
*		a number of DMAs from different effective addresses to/from a contiguous block of local
*		store memory.
*
*	-	A DMA List transfer only takes up one slot in the MFC DMA queue.
*
*	-	In this implementation you can have up to MaxNumListElements transfers per DMA list.
*
*	-	DMA lists use the same debug defines as normal DMAs; see ntDMA.h for details.
*
*
*
***************************************************************************************************/
template<int MaxNumElements = 32>
struct ntDMAList
{
	//
	//	Structure encapsulating parameters for a DMA list transfer - allows us to do useful
	//	error checking.
	//
	//	This is deliberately just a structure of simple variables combined with constructors for safe
	//	initialisation of the members. All operations are done through inline functions declared below
	//	this structure.
	//
	struct Params
	{
		//
		//	Ctors.
		//

		// Default Ctor - only here so we can have sensible and (fairly) safe
		// arrays of these objects without pissing all over memory.
		Params()
		:	m_LSAddr			( NULL )
		,	m_ID				( ntDMA::Params::InvalidDMAParams )
		,	m_NumListElements	( 0 )
		{}

		// Create a Params block for a DMA list. The DMA list will start off with no transfers.
		// To add a transfer, call 'AddListElement'. The transfers will go into LS starting at
		// the address given by 'local_store_start_addr' and will be placed contiguously, one
		// after the other.
		Params( void *local_store_start_addr, ntDMA_ID dma_id )
		:	m_LSAddr			( local_store_start_addr )
		,	m_ID				( dma_id )
		,	m_NumListElements	( 0 )
		{
			ntError_p( m_ID >= 0 && m_ID < MaxNumElements, ("[DMAList] DMA ids must be between 0 and % inclusive, you have requested an invalid id of %i\n", MaxNumListElements, m_ID) );
			if ( m_ID < 0 || m_ID >= MaxNumElements )
			{
				m_ID = ntDMA::Params::InvalidDMAParams;
			}

			DMALIST_PARAM_CHECK( *this );
		}

		// The Local store address of the DMA transfer.
		void *			m_LSAddr;

		// The id for this transfer - should be on [0,32).
		// An invalid Params block is indicated by m_ID being set to InvalidDMAParams.
		ntDMA_ID		m_ID;

		static const uint32_t MaxNumListElements = MaxNumElements;
		JobDmaListElement	m_ListElements[ MaxNumListElements ];
		uint32_t			m_NumListElements;
	};

	struct Debug
	{
		static void Check( const Params &params );
	};


	// Add a transfer to the Params block.
	static inline void AddListElement32( Params &params, uint32_t transfer_size, uint32_t ea /* low 32-bits of effective address */ );
	static inline void AddListElement64( Params &params, uint32_t transfer_size, uint64_t ea /* 64-bit effective address */ );

	// Kick off the transfers the given Params block describes.
	static inline void DmaToSPU( const Params &params );
	static inline void DmaToPPU( const Params &params );

	// Initiates a DMA list using the Params block 'to_ppu', that stores to main
	// memory, then queues up another DMA list using 'to_spu' that writes into SPU
	// local-store.
	// NOTE: This is different to doing the following sequence:
	//		DmaToPPU( to_ppu ):
	//		DmaToSPU( to_spu );
	// As in the above example the two DMA lists could potentially be executed
	// at the same time - which would give unpredictable results.
	// NOTE: The two Params blocks must have the same ntDMA_ID value.
	inline void StoreToEaThenFetchFromEa( const Params &to_ppu, const Params &to_spu );
};

template<int MaxNumElements>
inline void ntDMAList<MaxNumElements>::AddListElement32( Params &params, uint32_t transfer_size, uint32_t ea )
{
	ntError_p( params.m_NumListElements < ntDMAList::Params::MaxNumListElements, ("[DMAList] Attempting to add too many dma list elements.") );
	ntError_p( ( ea & 0xf ) == 0, ("[DMAList] Effective address must be aligned to a 16-byte boundary. EA = 0x%X", ea) );
	ntError_p( ( transfer_size & 0xf ) == 0, ("[DMAList] Transfer size must be a multiple of 16-bytes. Transfer Size = %i", transfer_size) );
	params.m_ListElements[ params.m_NumListElements ].m_size = transfer_size;
	params.m_ListElements[ params.m_NumListElements ].m_eaLo = ea;
	params.m_NumListElements++;

	DMALIST_PARAM_CHECK( params );
}

template<int MaxNumElements>
inline void ntDMAList<MaxNumElements>::AddListElement64( Params &params, uint32_t transfer_size, uint64_t ea )
{
	ntError_p( params.m_NumListElements < ntDMAList::Params::MaxNumListElements, ("[DMAList] Attempting to add too many dma list elements.") );
	ntError_p( ( ea & 0xf ) == 0, ("[DMAList] Effective address must be aligned to a 16-byte boundary. EA = 0x%X", (uint32_t)ea) );
	ntError_p( ( transfer_size & 0xf ) == 0, ("[DMAList] Transfer size must be a multiple of 16-bytes. Transfer Size = %i", transfer_size) );
	params.m_ListElements[ params.m_NumListElements ].m_size = transfer_size;
	params.m_ListElements[ params.m_NumListElements ].m_eaLo = (uint32_t)ea;
	params.m_NumListElements++;

	DMALIST_PARAM_CHECK( params );
}

template<int MaxNumElements>
inline void ntDMAList<MaxNumElements>::DmaToSPU( const Params &params )
{
	DMALIST_PARAM_CHECK( params );

#	ifdef ASSERT_ON_DMA_STALL
		ntError_p( ntDMA::NumberOfAvailableDMAQueueSlots() > 0, ("[DMAList] MFC queue is full - about to stall requesting a DMA transfer.") );
#	endif

	JobDmaListGet( params.m_LSAddr, params.m_ListElements, sizeof( JobDmaListElement ) * params.m_NumListElements, params.m_ID );

	WAIT_FOR_COMPLETION_AFTER_EACH_DMA( params.m_ID );
}

template<int MaxNumElements>
inline void ntDMAList<MaxNumElements>::DmaToPPU( const Params &params )
{
	DMALIST_PARAM_CHECK( params );

#	ifdef ASSERT_ON_DMA_STALL
		ntError_p( ntDMA::NumberOfAvailableDMAQueueSlots() > 0, ("[DMAList] MFC queue is full - about to stall requesting a DMA transfer.") );
#	endif

	JobDmaListPut( params.m_LSAddr, params.m_ListElements, sizeof( JobDmaListElement ) * params.m_NumListElements, params.m_ID );

	WAIT_FOR_COMPLETION_AFTER_EACH_DMA( params.m_ID );
}

template<int MaxNumElements>
inline void ntDMAList<MaxNumElements>::StoreToEaThenFetchFromEa( const ntDMAList::Params &to_ppu, const ntDMAList::Params &to_spu )
{
	DMALIST_PARAM_CHECK( to_ppu );
	DMALIST_PARAM_CHECK( to_spu );
	ntError_p( to_ppu.m_ID == to_spu.m_ID, ("[DMAList] The two Params blocks must have the same id (%i).", to_ppu.m_ID) );

#	ifdef ASSERT_ON_DMA_QUEUE_SPACE_STALL
		ntError_p( ntDMA::NumberOfAvailableDMAQueueSlots() > 1, ("[DMAList] MFC queue doesn't have enough slots free - about to stall requesting a DMA transfer.") );
#	endif

	JobDmaListPut( to_ppu.m_LSAddr, to_ppu.m_ListElements, sizeof( JobDmaListElement ) * to_ppu.m_NumListElements, to_ppu.m_ID );
	JobDmaListGet( to_spu.m_LSAddr, to_spu.m_ListElements, sizeof( JobDmaListElement ) * to_spu.m_NumListElements, to_spu.m_ID );

	WAIT_FOR_COMPLETION_AFTER_EACH_DMA( to_ppu.m_ID );		// Should be the same m_ID value as to_spu - so no point stalling on both.
}

template<int MaxNumElements>
inline void ntDMAList<MaxNumElements>::Debug::Check( const ntDMAList::Params &params )
{
	ntError_p( ( int32_t( params.m_ListElements ) & 0x7 ) == 0, ("The list element array must be aligned to at least 8-bytes. m_ListElements = 0x%X",int32_t( params.m_ListElements )) );
	ntError_p( params.m_ID >= 0 && params.m_ID < 32, ("[DMAList] Invalid DMA id detected. ID = %i", params.m_ID) );
	ntError_p( ( reinterpret_cast< intptr_t >( params.m_LSAddr ) & 0xf ) == 0, ("[DMAList] Local store address not aligned to 16-byte boundary. Address = 0x%X", reinterpret_cast< intptr_t >( params.m_LSAddr )) );
	for ( uint32_t i=0;i<params.m_NumListElements;i++ )
	{
		ntError_p( params.m_ListElements[ i ].m_size <= ntDMA::MaxSingleDMATransferSize, ("Single DMAs are limited to transfers of less than 16KB. m_TransferSize=%i", params.m_ListElements[ i ].m_size) );

		ntError_p( ( params.m_ListElements[ i ].m_eaLo & 0xf ) == 0, ("[DMAList] Effective address must be aligned to a 16-byte boundary. Low word EA[%i] = 0x%X", i, params.m_ListElements[ i ].m_eaLo) );
		ntError_p( ( params.m_ListElements[ i ].m_size & 0xf ) == 0, ("[DMAList] Transfer size must be a multiple of 16-bytes. Size[%i] = %i", i, params.m_ListElements[ i ].m_size) );

		ntError_p( !( params.m_ListElements[ i ].m_size < 0 ), ("[DMAList] Transfer size must be positive. Size[%i] = %i", i, params.m_ListElements[ i ].m_size) );
		ntError_p( params.m_ListElements[ i ].m_size > 0, ("[DMAList] Transfer size must be greater than zero. Size[%i] = %i", i, params.m_ListElements[ i ].m_size) );
	}
}

#undef DMALIST_PARAM_CHECK

#endif // !NTLIB_NTDMALIST_H_

