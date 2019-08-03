//---------------------------------------
//!
//!	\file core\memman_ps3.cpp
//! The memory manager the only place to
//! uses OS level memory calls
//!
//---------------------------------------
#include <sys/memory.h>
#include "core/memman.h"

//---------------------------------------
// Defines
//---------------------------------------

// the memory system
namespace Mem
{
static Mem::BaseMemStats s_BaseMemStats;

//---------------------------------------
//!
//! Allocate all our ram from the OS
//!
//---------------------------------------
void InitOsMem(	uintptr_t* ppXddr, uint32_t* pXddrSize,
				uintptr_t* ppGddr, uint32_t* pGddrSize,
				uintptr_t* ppDebug, uint32_t* pDebugSize )
{
	// do something sneaky and allocate a chunk of system memory before the main heaps are setup.
	// the size of this should be the largest memory container we require in game.
	uint32_t fakeContainer = MemoryContainer_Create( MAX_CONTAINER_SIZE );

	int osRes;
	// get the user memory pool stats right away before
	// we start picking bits off
	sys_memory_info_t userMem;
	osRes = sys_memory_get_user_memory_size( &userMem );
	ntAssert( osRes == CELL_OK );

	// calculate the memory sizes
	// this is (roughly) the memory used by the time we started the program (elf and statics plus our memory container area)
	uint64_t iStartUsed = userMem.total_user_memory - userMem.available_user_memory;
	uint64_t iInitialXddr = (256-64) * Mb - iStartUsed;
	uint64_t iInitialGddr = (256-32)* Mb;
	int64_t iInitialDebug = userMem.available_user_memory - iInitialXddr - 1 * Mb; // fudge for SPURS currently
	iInitialDebug -= 2 * Mb; // for SN object loader
	if ( iInitialDebug < 0 )
	{
		// This is possible because of the "fudge for SPURS", above.
		iInitialDebug = 0;
	}

	ntError( iInitialDebug >= 0 );
	
	s_BaseMemStats.iInitialUsed = iStartUsed;
	s_BaseMemStats.iInitialDebugFree = iInitialDebug;
	s_BaseMemStats.iInitialGddrFree = iInitialGddr;
	s_BaseMemStats.iInitialXddrFree = iInitialXddr;

	uint64_t iFreeXddr = iInitialXddr & 0xFFF00000;		// megabyte align
	uint64_t iFreeDebug = (uint64_t)iInitialDebug & 0xFFF00000;	// megabye

	// lets start carving up the real ram
	// 1st thing we need is to actually allocates all the ram from the OS!
	sys_addr_t	pXddrMem;
	sys_addr_t	pDebugMem;
	osRes = sys_memory_allocate(	iFreeXddr, 
									SYS_MEMORY_PAGE_SIZE_1M, (sys_addr_t*)&pXddrMem );
	ntAssert( osRes == CELL_OK );
	if ( iFreeDebug > 0 )
	{
		osRes = sys_memory_allocate(	iFreeDebug, 
										SYS_MEMORY_PAGE_SIZE_1M, (sys_addr_t*)&pDebugMem );
		ntAssert( osRes == CELL_OK );
	}
	else
	{
		pDebugMem = 0;
	}

	*ppXddr = (uintptr_t)pXddrMem;
	*pXddrSize = iFreeXddr;
	*ppGddr = (uintptr_t)0; // not yet managed
	*pGddrSize = iInitialGddr;
	*ppDebug = (uintptr_t)pDebugMem;
	*pDebugSize = iFreeDebug;

	// free up our fake container.
	MemoryContainer_Destroy( fakeContainer );
}

//---------------------------------------
//!
//! FreeOsMem
//! NOTE Gddr not yet managed
//!
//---------------------------------------
void FreeOsMem( uintptr_t pXddr, uintptr_t pGddr, uintptr_t pDebug )
{
	UNUSED( pGddr );

	// give the debug mem back
	sys_memory_free( (sys_addr_t) pDebug );
	// give the xddr mem back
	sys_memory_free( (sys_addr_t) pXddr );
}

//---------------------------------------
//!
//! GetBaseMemStats 
//!
//---------------------------------------
const BaseMemStats& GetBaseMemStats()
{
	return s_BaseMemStats;
}

//---------------------------------------
//!
//! MemoryContainer_Create safety wrapper
//! round memory container usage.
//!
//---------------------------------------
uint32_t	MemoryContainer_Create( uint32_t size )
{
	ntError_p( size <= MAX_CONTAINER_SIZE, ("Cannot allocate memory containers larger than 0x%x", MAX_CONTAINER_SIZE ) );
	
	sys_memory_container_t id;
	int r = sys_memory_container_create(&id, size);
	
	switch (r)
	{
	case ENOMEM: ntError_p( 0, ("Not enough free system memory to allocate container of size 0x%x", size) ); break;
	case EAGAIN: ntError_p( 0, ("Kernel memory shortage in MemoryContainer_Create()") );		break;
	case EFAULT: ntError_p( 0, ("Invalid address passed to sys_memory_container_create()") );	break;
	default:
		break;
	}
	
	return id;
}

//---------------------------------------
//!
//! MemoryContainer_Create safety wrapper
//! round memory container usage
//!
//---------------------------------------
void	MemoryContainer_Destroy( uint32_t id )
{	
	int r = sys_memory_container_destroy(id);
	
	switch (r)
	{
	case ESRCH: ntError_p( 0, ("Memory container 0x%x not found", id) );								break;
	case EBUSY: ntError_p( 0, ("Memory container 0x%x is still in use and cannot be destroyed", id) );	break;
	default:
		break;
	}
}

//---------------------------------------
//!
//! MemoryContainer_GetSize safety wrapper
//! round memory container usage
//!
//---------------------------------------
void	MemoryContainer_GetSize( uint32_t id, uint32_t& total, uint32_t& avail  )
{	
	sys_memory_info_t	info;
	int r = sys_memory_container_get_size(&info,id);
	
	switch (r)
	{
	case ESRCH: ntError_p( 0, ("Memory container 0x%x not found", id) );								break;
	case EFAULT: ntError_p( 0, ("Invalid address passed to sys_memory_container_get_size()") );	break;
	default:
		break;
	}

	total = info.total_user_memory;
	avail = info.available_user_memory;
}

} // end namespace Mem
