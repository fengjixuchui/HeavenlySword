//----------------------------------------------------------------------------------------
//! 
//! \filename ntlib_spu\fixups.cpp
//! 
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//----------------------------------------------------------------------------------------

#include "jobapi/jobdefinition.h"
#include "ntlib_spu/fixups_spu.h"
#include "ntlib_spu/ntDma.h"

//#define DETAIL_INPUTS_AND_OUTPUTS

namespace SPUAllocateData
{
	uintptr_t	CurrentBaseAddr;
	uintptr_t	EndOfBufferMemory;
}

void *Allocate( uint32_t length_in_bytes )
{
	ntError_p( SPUAllocateData::CurrentBaseAddr == ROUND_POW2( SPUAllocateData::CurrentBaseAddr, 16 ), ("The CurrentBaseAddr is not a multiple of 16!") );

	// Adjust requested size to a multiple of 16.
	length_in_bytes = ROUND_POW2( length_in_bytes, 16 );

	// Return NULL if we're out of memory.
	if ( SPUAllocateData::CurrentBaseAddr + length_in_bytes > SPUAllocateData::EndOfBufferMemory )
	{
		ntError_p( false, ("Too much memory requested - returning NULL.") );
		return NULL;
	}

	void *mem_ptr = (void *)SPUAllocateData::CurrentBaseAddr;
	SPUAllocateData::CurrentBaseAddr += length_in_bytes;

	return mem_ptr;
}

static void InitialiseAllocator()
{
	// Which bufferset to use for the linear allocator?
	static const uint32_t LinearAllocatorBufferSet = 15;

	WwsJob_BufferTag linear_allocator_mem = WwsJob_JobApiGetBufferTag( LinearAllocatorBufferSet, 0, WwsJob_kDontAllocDmaTag );

	ntError( linear_allocator_mem.GetAddr() != NULL );
	ntError( linear_allocator_mem.GetLength() > 0 );

	SPUAllocateData::CurrentBaseAddr = uintptr_t( linear_allocator_mem.GetAddr() );
	SPUAllocateData::EndOfBufferMemory = SPUAllocateData::CurrentBaseAddr + linear_allocator_mem.GetLength();

	ntError_p( SPUAllocateData::CurrentBaseAddr == ROUND_POW2( SPUAllocateData::CurrentBaseAddr, 16 ), ("Base memory address must be at least 16-byte aligned.") );
	ntError_p( SPUAllocateData::EndOfBufferMemory == ROUND_POW2( SPUAllocateData::EndOfBufferMemory, 16 ), ("End memory address must be at least 16-byte aligned.") );
}

static void FixupParameters( SPUArgumentList &params, uint32_t *dma_effective_addresses )
{
	for ( int32_t i=0;i<SPUArgumentList::MaxNumArguments;i++ )
	{
		dma_effective_addresses[ i ] = 0xffffffff;

		SPUArgument *arg = params.Get( i );
		if ( arg != NULL && arg->GetType() == SPUArgument::Type_DMABuffer )
		{
			ntAssert( arg->GetBuffer()->IsAddrEA() );
			dma_effective_addresses[ i ] = uint32_t( arg->GetBuffer()->GetEA() );

			WwsJob_BufferTag buffer_tag = WwsJob_JobApiGetBufferTag( arg->GetBufferSetNum(), 0, WwsJob_kAllocDmaTag );

			void *ls_addr = buffer_tag.GetAddr();
			ntError_p( buffer_tag.GetLength() >= arg->GetBuffer()->GetSize(), ("DMABuffer size mismatch. Arg %i", i) );

#			ifdef DETAIL_INPUTS_AND_OUTPUTS
				ntPrintf( "Input %i is a DMABuffer: ea=0x%X, ls=0x%X, length=0x%X, DMA_ID = %i\n", i, arg->GetBuffer()->GetEA(), (uintptr_t)ls_addr, (uint32_t)arg->GetBuffer()->GetSize(), buffer_tag.m_dmaTagId );
#			endif

			arg->GetBuffer()->Set( ls_addr, buffer_tag.m_dmaTagId );
		}
	}
}

static void StoreStage( const SPUArgumentList &params, uint32_t *dma_effective_addresses )
{
	for ( int32_t i=0;i<SPUArgumentList::MaxNumArguments;i++ )
	{
		const SPUArgument *arg = params.Get( i );
		if ( arg != NULL && ( arg->GetMode() == SPUArgument::Mode_InputAndOutput || arg->GetMode() == SPUArgument::Mode_OutputOnly ) )
		{
			ntError_p( arg->GetType() == SPUArgument::Type_DMABuffer, ("Only DMABuffers can be outputs.") );
			ntError_p( dma_effective_addresses[ i ] != 0xffffffff, ("Invalid effective address for this DMA transfer.") );

#			ifdef DETAIL_INPUTS_AND_OUTPUTS
			ntPrintf( "DMABuffer is ouput %i: ea=0x%X, ls=0x%X, length=0x%X, DMA_ID = %i\n", i, (uint32_t)dma_effective_addresses[ i ], (uint32_t)arg->GetBuffer()->GetLS(), (uint32_t)arg->GetBuffer()->GetSize(), arg->GetBuffer()->GetDmaID() );
#			endif

			ntDMA::Params dma;
			dma.Init32( arg->GetBuffer()->GetLS(), dma_effective_addresses[ i ], arg->GetBuffer()->GetSize(), arg->GetBuffer()->GetDmaID() );
			ntDMA::DmaToPPU( dma );
		}
	}
}

// it can be useful to retrieve the EA address of input buffers for kicking off other tasks..
// yes globals are evil but meh... its works and doesn't cause any alignment issues or changes to 
// our SPU API
uint32_t g_DMAEffectiveAddresses[ SPUArgumentList::MaxNumArguments ];

extern "C" void SpuMain( SPUArgumentList &params );
void SPUFixups::RunJob()
{
	static const int32_t ParamListBufferSet = 14;
	WwsJob_BufferTag param_set_tag = WwsJob_JobApiGetBufferTag( ParamListBufferSet, 0, WwsJob_kDontAllocDmaTag );
	ntError_p( param_set_tag.GetLength() == ROUND_POW2( sizeof( SPUArgumentList ), 1024 ), ("SPU_MODULE_FIXUP: Invalid param-list size") );
	const SPUArgumentList *temp_params = static_cast< SPUArgumentList * >( param_set_tag.GetAddr() );
	SPUArgumentList params = *temp_params;

	FixupParameters( params, &( g_DMAEffectiveAddresses[ 0 ] ) );

	InitialiseAllocator();

	WwsJob_JobApiLoadNextJob();
	SpuMain( params );

	StoreStage( params, &( g_DMAEffectiveAddresses[ 0 ] ) );
}




