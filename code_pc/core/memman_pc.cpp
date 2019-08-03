//---------------------------------------
//!
//!	\file core\memman_pc.cpp
//! The memory manager the only place to
//! uses OS level memory calls
//!
//---------------------------------------
#include "core/memman.h"

//---------------------------------------
// Defines
//---------------------------------------
//#define FUDGE_DEBUG_MEM_STATS

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
	// these numbers are nothing like the PS3 purely to get walkways running for MS12 once thats done with these come down...
	// way way down

	s_BaseMemStats.iInitialGddrFree = (256-32)* Mb;
	s_BaseMemStats.iInitialXddrFree = 512 * Mb;
	s_BaseMemStats.iInitialDebugFree = 400 * Mb;

	uint32_t iFreeXddr = s_BaseMemStats.iInitialXddrFree & 0xFF100000; // take our page off and mega align
	uint32_t iFreeDebug = s_BaseMemStats.iInitialDebugFree & 0xFF100000; // take our page off

	
	// lets start carving up the real ram
	// 1st thing we need is to actually allocates all the ram from the OS!
	uintptr_t	pXddrMem = (uintptr_t) VirtualAlloc( 0, iFreeXddr, MEM_COMMIT, PAGE_READWRITE );
	uintptr_t	pDebugMem = (uintptr_t) VirtualAlloc( 0, iFreeDebug, MEM_COMMIT, PAGE_READWRITE );

	ntError( pXddrMem );
	ntError( pDebugMem );

	*ppXddr = (uintptr_t)pXddrMem;
	*pXddrSize = iFreeXddr;
	*ppGddr = (uintptr_t)0; // not yet managed
	*pGddrSize = s_BaseMemStats.iInitialGddrFree;
	*ppDebug = (uintptr_t)pDebugMem;
	*pDebugSize = iFreeDebug;
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
	VirtualFree( (void*) pDebug, 0, MEM_RELEASE );
	// give the xddr mem back
	VirtualFree( (void*) pXddr, 0, MEM_RELEASE );
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

} // end namespace Mem
