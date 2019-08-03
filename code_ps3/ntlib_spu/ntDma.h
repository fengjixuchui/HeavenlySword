//---------------------------------------------------------------
//!
//! \file ntDMA.h
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

#ifndef	NTLIB_NTDMA_H_
#define	NTLIB_NTDMA_H_

#ifndef __SPU__
#	error This file can only be included in an SPU project.
#endif // !__SPU__

#include "jobapi/jobapi.h"
#include "jobapi/jobspudma.h"

#include "ntlib_spu/basetypes_spu.h"
#include "ntlib_spu/debug_spu.h"

// Set this to true to stall the spu for the completion of each DMA executed.
namespace ntDMA { namespace Debug { extern bool gWaitForCompletionAfterEachDMA; } }

// Remove the stall regardless of the above variable in release builds.
#ifndef _RELEASE
#	define DECLARE_DEBUG_VARS
#endif // !_RELEASE

#ifdef DECLARE_DEBUG_VARS
#	define WAIT_FOR_COMPLETION_AFTER_EACH_DMA( id )	do { if ( ntDMA::Debug::gWaitForCompletionAfterEachDMA ){ ntPrintf("Stall Start for %i\n", id); ntDMA::StallForCompletion( (id) ); ntPrintf("Stall Over for %i\n", id); } } while( 0 )
#else
#	define WAIT_FOR_COMPLETION_AFTER_EACH_DMA( id )	do {} while( 0 )
#endif // DECLARE_DEBUG_VARS

// Define this to assert if DmaToSPU or DmaToPPU is going to stall due to lack of MFC queue space.
//#define ASSERT_ON_DMA_QUEUE_SPACE_STALL

// Define this to enable checking of DMA transfer parameter blocks.
#ifndef _RELEASE
#	define NT_DEBUG_DMA_TRANSFERS
#endif // !_RELEASE

// Forward declare the debug checking function.
namespace ntDMA { struct Params; namespace Debug { inline void Check( const Params & ); } }

#ifdef NT_DEBUG_DMA_TRANSFERS
#	define DMA_PARAM_CHECK( param )	ntDMA::Debug::Check( param )
#else
#	define DMA_PARAM_CHECK( param ) do {} while ( 0 )
#endif // _DEBUG

// Type for an Id to identify a DMA transfer.
typedef uint32_t ntDMA_ID;

/***************************************************************************************************
*
*	DMA Transfers.
*	==============
*
*	-	*Always* use the 'Params' structure below to setup DMA transfers. It contains checking code
*		that will make it less prone to memory scribbling if you get something wrong. Having said
*		that, it isn't a magic bullet and you can still mess things up quite badly; so be careful!
*
*	-	The piece of hardware that handles SPU DMA requests is referred to as the MFC. The MFC
*		has a queue for DMA requests and simple works through this queue. Obviously the queue
*		is a finite size so if it is full and you call DMAToSPU/DMAToPPU then the SPU will stall
*		until a free slot becomes available in this queue. You can query the remaining number of 
*		free slots in this queue by calling 'NumberOfAvailableDMAQueueSlots()'.
*
*	-	Single DMA transfers are limited in size to copy <= 16KB.
*
*	-	How do you know when a DMA has completed? You can either stall the SPU and then carry on
*		in the knowledge that it has now completed, or you can use the non-blocking query function
*		'HasCompleted( dma_id )' to check.
*
*	-	When setting up a Params block for a DMA transfer, you must supply an id to be associated
*		with the transfer. The id must be between 0 and 31 inclusive (i.e. it must specify a
*		bit-position within a 32-bit word). The id does not have to be unique - if you are only
*		interested in querying whether multiple transfers have all completed then you can give them
*		all the same id and the completion query functions will tell you the 'id' has finished
*		when all of the transfers with that id have finished.
*
*	-	Completion checking caveat: If you check the completion status of a transfer using the
*		correct id, but the transfer has completed ages ago and a new transfer using the same id
*		has been queued then you'll be querying the status of the new transfer. This is fairly
*		obvious when stated but might become confusing in the actual code; watch out for it. If
*		you don't want your new transfer to have any relation to any previous transfer then use
*		a fresh id for it. You can use the function 'GetFreshID()' for this - it will cycle back
*		to an id of 0 after 31 though - so still be aware of this caveat.
*
*	-	You must be careful when using I/O-style buffers with DMA transfers (i.e. a buffer in 
*		local-store that acts as both an input and an output for the SPU).
*		Never do the following:
*				DmaToPPU( params );
*				params.m_EA[ 1 ] = new_ea;
*				DmaToSPU( params );
*		There is a chance that the two DMAs will execute simultaneously. This could lead to
*		unpredictable values being DMAd in or out of the SPU local-store. The correct way to do
*		this is:
*				StoreToEaThenFetchFromEa( params, new_ea );
*		This function uses a 'fenced-get' DMA transfer to ensure that the DMAToPPU has finished
*		before it starts the DMA transfer to local-store. The function will never stall as long
*		as there are at least two empty DMA slots in the MFC DMA queue.
*
*	-	Unit test code is commented out at the bottom of this file. It will need to be copied
*		and setup in a relevent place in a PPU project and an SPU project to test.
*
*	-	Lastly, and most importantly, if you don't understand all the above *completely*, then
*		read it again until you do or go and ask Andrew to explain it to you. If you don't
*		understand exactly what you're doing you'll bugger something up fairly badly.
*
*
*
*
***************************************************************************************************/
namespace ntDMA
{
	//
	//	Structure encapsulating parameters for a DMA transfer - allows us to do useful error checking.
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
		:	m_LSAddr		( NULL )
		,	m_EAlong		( 0 )
		,	m_TransferSize	( 0 )
		,	m_ID			( InvalidDMAParams )
		{}

		// Construct a set of params that will transfer "length_in_bytes" bytes from/to
		// "local_store_addr" to/from "effective_addr" and let this transfer be associated
		// with the id "dma_id". Use this with 64-bit effective addresses. All addresses
		// and the length_in_bytes parameter must be aligned to at least 16 bytes. Sony
		// say 128-bytes is a bit quicker.
		void Init64( void *local_store_addr, uint64_t effective_addr, uint32_t length_in_bytes, ntDMA_ID dma_id )
		{
			m_LSAddr = static_cast< int8_t * >( local_store_addr );
			m_EAlong = effective_addr;
			m_TransferSize = length_in_bytes;
			m_ID = dma_id;

			ntError_p( m_ID >= 0 && m_ID < 32, ("[DMA] DMA ids must be between 0 and 31 inclusive, you have requested an invalid id of %i\n", m_ID) );
			if ( m_ID < 0 || m_ID >= 32 )
			{
				m_ID = InvalidDMAParams;
			}

			DMA_PARAM_CHECK( *this );
		}

		// Construct a set of params that will transfer "length_in_bytes" bytes from/to
		// "local_store_addr" to/from "effective_addr" and let this transfer be associated
		// with the id "dma_id". Use this with 32-bit effective addresses.  All addresses
		// and the length_in_bytes parameter must be aligned to at least 16 bytes. Sony
		// say 128-bytes is a bit quicker.
		void Init32( void *local_store_addr, uint32_t effective_addr, uint32_t length_in_bytes, ntDMA_ID dma_id )
		{
			m_LSAddr = static_cast< int8_t * >( local_store_addr );
			m_TransferSize = length_in_bytes;
			m_ID = dma_id;

			m_EA[ 0 ] = 0;					// Set upper 32-bits to zero.
			m_EA[ 1 ] = effective_addr;		// EA address goes in lower 32-bits.

			ntError_p( m_ID >= 0 && m_ID < 32, ("[DMA] DMA ids must be between 0 and 31 inclusive, you have requested an invalid id of %i\n", m_ID) );
			if ( m_ID < 0 || m_ID >= 32 )
			{
				m_ID = InvalidDMAParams;
			}

			DMA_PARAM_CHECK( *this );
		}

		// The Local store address of the DMA transfer.
		int8_t *		m_LSAddr;

		// The effective address of the DMA transfer.
		union
		{
			uint64_t	m_EAlong;
			uint32_t	m_EA[ 2 ];
		};

		// The size (in bytes) of the dma transfer.
		uint32_t		m_TransferSize;

		// The id for this transfer - should be on [0,32).
		// An invalid Params block is indicated by m_ID being set to InvalidDMAParams.
		ntDMA_ID		m_ID;

		// The invalid Params block id.
		static const ntDMA_ID InvalidDMAParams = ntDMA_ID( -1 );
	};

	// Initiate a DMA transfer according to the given Params block.
	// Both these functions make sensible checks on the parameters.
	inline void DmaToSPU( Params params );
	inline void DmaToPPU( Params params );

	// Initiates a DMA using the given Params block that stores to main
	// memory then queues up another DMA that writes into SPU local-store
	// using the given params but reading from the effective address 'ea'
	// instead of the effective address specified in the Params block.
	// NOTE: This is different to doing the following sequence:
	//		DmaToPPU( params ):
	//		params.m_EA[ 1 ] = ea;
	//		DmaToSPU( params );
	// As in the above example the two DMAs could potentially be executed
	// at the same time - which would give unpredictable results.
	inline void StoreToEaThenFetchFromEa( const Params &params, int64_t ea );
	inline void StoreToEaThenFetchFromEa( const Params &params, int32_t ea );

	// Returns true if the DMA corresponding to the given id has completed.
	// NOTE: This function does NOT block.
	inline bool HasCompleted( ntDMA_ID dma_id );

	// Stalls the SPU until the DMA corresponding to the given id has completed.
	inline void StallForCompletion( ntDMA_ID dma_id );

	// Returns the number of free DMA queue slots.
	inline int32_t NumberOfAvailableDMAQueueSlots();

	// Returns the next consecutive dma id.
	inline ntDMA_ID GetFreshID();
	inline void		FreeID( ntDMA_ID id );

	static const uint32_t	MaxSingleDMATransferSize = 0x4000;	// Single DMA transfers are limited in size to <= 16KB.
}

/***************************************************************************************************
*
*	Implementation note.
*	====================
*
*	-	All of the spu_<blah> intrinsics are surrounded by "asm __volatile__("" : : : "memory");"
*		statements. This weirdness is here to tell GCC that the enclosed statement shouldn't be
*		moved during optimisation and that memory is indeterminately modified by the enclosed code.
*
*	-	NOTE: I'm only using the asm __volatile__ enclosure because the ATG SkDma code does. I'm
*		dubious whether we need it or not as technically it should only apply to assembler within
*		the first set of quotes in the brackets. Placing a statement before *and* after is also
*		dubious, IMHO. Better safe than sorry though.
*
*
***************************************************************************************************/
inline void ntDMA::DmaToSPU( ntDMA::Params params )
{
	DMA_PARAM_CHECK( params );

#	ifdef ASSERT_ON_DMA_QUEUE_SPACE_STALL
		ntError_p( NumberOfAvailableDMAQueueSlots() > 0, ("[DMA] MFC queue is full - about to stall requesting a DMA transfer.") );
#	endif

	JobDmaLargeGet( params.m_LSAddr, params.m_EA[ 1 ], params.m_TransferSize, params.m_ID );

	WAIT_FOR_COMPLETION_AFTER_EACH_DMA( params.m_ID );
}

inline void ntDMA::DmaToPPU( ntDMA::Params params )
{
	DMA_PARAM_CHECK( params );

#	ifdef ASSERT_ON_DMA_QUEUE_SPACE_STALL
		ntError_p( NumberOfAvailableDMAQueueSlots() > 0, ("[DMA] MFC queue is full - about to stall requesting a DMA transfer.") );
#	endif

	JobDmaLargePut( params.m_LSAddr, params.m_EA[ 1 ], params.m_TransferSize, params.m_ID );

	WAIT_FOR_COMPLETION_AFTER_EACH_DMA( params.m_ID );
}

inline bool ntDMA::HasCompleted( ntDMA_ID dma_id )
{
	ntError_p( dma_id >= 0 && dma_id < 32, ("[DMA] Invalid DMA id detected. ID = %i", dma_id) );

	return JobDmaWaitTagStatusImmediate( 1 << dma_id ) != 0;			// If it's zero, we haven't finished.
}

inline void ntDMA::StallForCompletion( ntDMA_ID dma_id )
{
	ntError_p( dma_id >= 0 && dma_id < 32, ("[DMA] Invalid DMA id detected. ID = %i", dma_id) );

	JobDmaWaitTagStatusAll( 1 << dma_id );
}

inline int32_t ntDMA::NumberOfAvailableDMAQueueSlots()
{
	uint32_t enabled = AreInterruptsEnabled();
	if ( enabled )
	{
		DisableInterrupts();
	}

	int32_t ret_val = mfc_stat_cmd_queue();

	if( enabled )
	{
		EnableInterrupts();
	}

	return ret_val;
}

inline ntDMA_ID ntDMA::GetFreshID()
{
	return ntDMA_ID( WwsJob_JobApiUseDmaTag() );
}

inline void ntDMA::FreeID( ntDMA_ID id )
{
	WwsJob_JobApiFreeDmaTagId( id );
}

inline void ntDMA::StoreToEaThenFetchFromEa( const ntDMA::Params &params, int32_t ea )
{
	union PtrConv_
	{
		int64_t ea64;
		int32_t ea32[ 2 ];
	};
	PtrConv_ ptr;
	ptr.ea32[ 0 ] = 0;
	ptr.ea32[ 1 ] = ea;

	StoreToEaThenFetchFromEa( params, ptr.ea64 );
}

inline void ntDMA::StoreToEaThenFetchFromEa( const ntDMA::Params &params, int64_t ea )
{
	DMA_PARAM_CHECK( params );
	ntError_p( params.m_TransferSize <= MaxSingleDMATransferSize, ("Single DMAs are limited to transfers of less than 16KB. m_TransferSize=%i", params.m_TransferSize) );
	ntError_p( ( ea % 0xf ) == 0, ("[DMA] Effective address not aligned to 16-byte boundary. EA = 0x%X", (uint32_t)ea) );

#	ifdef ASSERT_ON_DMA_QUEUE_SPACE_STALL
		ntError_p( NumberOfAvailableDMAQueueSlots() > 1, ("[DMA] MFC queue doesn't have enough slots free - about to stall requesting a DMA transfer.") );
#	endif

	JobDmaPut( params.m_LSAddr, params.m_EA[ 1 ], params.m_TransferSize, params.m_ID );
	JobDmaGetf( params.m_LSAddr, (int32_t)ea, params.m_TransferSize, params.m_ID );

	WAIT_FOR_COMPLETION_AFTER_EACH_DMA( params.m_ID );
}

inline void ntDMA::Debug::Check( const ntDMA::Params &params )
{
	ntError_p( params.m_ID >= 0 && params.m_ID < 32, ("[DMA] Invalid DMA id detected. ID = %i", params.m_ID) );
	ntError_p( ( reinterpret_cast< intptr_t >( params.m_LSAddr ) & 0xf ) == 0, ("[DMA] Local store address not aligned to 16-byte boundary. LS = 0x%X", (uint32_t)params.m_LSAddr) );
	ntError_p( ( params.m_EAlong & 0xf ) == 0, ("[DMA] Effective address not aligned to 16-byte boundary. EA = 0x%X", params.m_EA[ 1 ]) );
	ntError_p( ( params.m_TransferSize & 0xf ) == 0, ("[DMA] Transfer size is not a multiple of 16-bytes. Size = %i", params.m_TransferSize) );
	ntError_p( params.m_TransferSize > 0, ("[DMA] Transfer size must be greater than zero. Size = %i", params.m_TransferSize) );
}

#undef DMA_PARAM_CHECK

/***************************************************************************************************
*
*	Unit test code.
*	===============
*
*	-	The first function is PPU code.
*	-	The second function is SPU code.
*
*	-	This unit-test tests both ntDMA and ntDMAList functionality.
*
***************************************************************************************************/
/*
void ntDMA_PPU_UnitTest( const char * const spu_elf_filename )
{
	static SPUProgram spu_program( spu_elf_filename );

	int32_t *ea = static_cast< int32_t * >( DMABuffer::DMAAlloc( 128*sizeof( int32_t ) ) );
	for ( int32_t i=0;i<128;i++ )
	{
		ea[ i ] = i;
	}
	int32_t *ea2 = static_cast< int32_t * >( DMABuffer::DMAAlloc( 128*sizeof( int32_t ) ) );
	for ( int32_t i=0;i<128;i++ )
	{
		ea2[ i ] = i*2;
	}
	int32_t *ea3 = static_cast< int32_t * >( DMABuffer::DMAAlloc( 128*sizeof( int32_t ) ) );
	for ( int32_t i=0;i<128;i++ )
	{
		ea3[ i ] = 1;
	}


	SPUTask spu_task( &spu_program );
	spu_task.AddArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t)(intptr_t)ea ), 0 );
	spu_task.AddArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t)(intptr_t)ea2 ), 1 );
	spu_task.AddArgument( SPUArgument( SPUArgument::Mode_InputOnly, (uint32_t)(intptr_t)ea3 ), 2 );

	Exec::RunTask( &spu_task );

	for ( int32_t i=0;i<128;i++ )
	{
		ntError( ea[ i ] == 8128 );
		ntError( ea2[ i ] == 8128*2 );
		ntError( ea3[ i ] == 128 );
	}
}
*/
/*
extern "C" void SpuMain( const uint128_t* pParamsArray )
{
	GetU32Input( ea, 0 );
	GetU32Input( ea2, 1 );
	GetU32Input( ea3, 2 );

	static int32_t ls[ 128 ] __attribute__((aligned((128))));
	static int32_t ls2[ 256 ] __attribute__((aligned((128))));

	static const ntDMA_ID dma_tag = 0;
	static const ntDMA_ID dma_tag_list = 1;

	ntDMA::Params params;
	params.Init32( ls, ea, 128*sizeof( int32_t ), dma_tag );
	ntDMA::DmaToSPU( params );
	ntDMA::StallForCompletion( dma_tag );

	ntDMAList::Params list_params( ls2, dma_tag_list );
	ntDMAList::AddListElement32( list_params, 128*sizeof( int32_t ), ea2 );
	ntDMAList::AddListElement32( list_params, 128*sizeof( int32_t ), ea3 );

	ntDMAList::DmaToSPU( list_params );

	ntDMA::StallForCompletion( dma_tag_list );

	int32_t sum = 0, sum2 = 0, sum3 = 0;
	for ( int32_t i=0;i<128;i++ )
	{
		sum += ls[ i ];
		sum2 += ls2[ i ];
		sum3 += ls2[ i + 128 ];
	}

	for ( int32_t i=0;i<128;i++ )
	{
		ls[ i ] = sum;
		ls2[ i ] = sum2;
		ls2[ i + 128 ] = sum3;
	}

	ntDMAList::DmaToPPU( list_params );

	ntDMA::DmaToPPU( params );
}
*/

#endif // !NTLIB_NTDMA_H_

