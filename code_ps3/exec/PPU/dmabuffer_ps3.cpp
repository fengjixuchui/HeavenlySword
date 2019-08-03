//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\dmabuffer_ps3.cpp
//! 
//----------------------------------------------------------------------------------------

#include <Fw/FwMem/FwMem.h>
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec_ps3.h"


//----------------------------------------------------------------------------------------
//! 
//! Allocate DMA memory.
//! 
//----------------------------------------------------------------------------------------
void *DMABuffer::DMAAlloc( size_t num_bytes, size_t *actual_allocated_size, Mem::MEMORY_CHUNK chunk, uint32_t iAlignment )
{
	num_bytes = DMAAllocSize( num_bytes, iAlignment );
	if ( actual_allocated_size != NULL )
	{
		*actual_allocated_size = num_bytes;
	}

	return (void*)NT_MEMALIGN_CHUNK( chunk, num_bytes, iAlignment );
}


//----------------------------------------------------------------------------------------
//! 
//! Free DMA memory.
//! 
//----------------------------------------------------------------------------------------
void DMABuffer::DMAFree( const void *mem_ptr, Mem::MEMORY_CHUNK chunk )
{
	ntError_p( Util::IsAligned( reinterpret_cast< uintptr_t >( mem_ptr ), DefaultDMAAlignment ), ("mem_ptr is not correctly aligned - are you sure this is a pointer returned from DMAAlloc?") );

	// Slightly dirty const_cast here... it is correct C++ to expect to be
	// able to delete a pointer to const - but FwMem doesn't support this
	// hence the const_const.
	return NT_FREE_CHUNK( chunk, (uintptr_t)mem_ptr );
}








