//---------------------------------------
//!
//!	\file core\memman.cpp
//! The memory manager the only place to
//! uses OS level memory calls
//!
//---------------------------------------
#include "core/mem.h"
#include "core/memman.h"
#include "core/mempool.h"

#include <Fw/FwMem/FwMem.h>

#if defined( _HAVE_MEMORY_TRACKING )

void* operator new( size_t memSize, Mem::MEMORY_CHUNK chunk, const char* pTag, const char* pSubTag, int iLine )
{
	void* pPtr = (void*) Mem::Alloc( chunk, memSize, pTag, pSubTag, iLine );
	return pPtr;
}

void* operator new[]( size_t memSize, Mem::MEMORY_CHUNK chunk, const char* pTag, const char* pSubTag, int iLine )
{
	void* pPtr = (void*) Mem::Alloc( chunk, memSize, pTag, pSubTag, iLine );
	return pPtr;
}
#else
void* operator new( size_t memSize, Mem::MEMORY_CHUNK chunk )
{
	void* pPtr = (void*) Mem::Alloc( chunk, memSize );
	return pPtr;
}

void* operator new[]( size_t memSize, Mem::MEMORY_CHUNK chunk )
{
	void* pPtr = (void*) Mem::Alloc( chunk, memSize );
	return pPtr;
}

#endif

void operator delete( void* pBlock )
{
#if defined( _HAVE_MEMORY_TRACKING )
	// special marker tells free to use the chunk stack
	Mem::Free( Mem::MC_UNKNOWN, (uintptr_t) pBlock, 0, 0, 0  );
#else
	Mem::Free( Mem::MC_UNKNOWN, (uintptr_t) pBlock );
#endif
}

void operator delete[]( void* pBlock )
{
#if defined( _HAVE_MEMORY_TRACKING )
	// special marker tells free to use the chunk stack
	Mem::Free( Mem::MC_UNKNOWN, (uintptr_t) pBlock, 0, 0, 0  );
#else
	Mem::Free( Mem::MC_UNKNOWN, (uintptr_t) pBlock );
#endif
}

void operator delete( void* pBlock, const std::nothrow_t& )
{
#if defined( _HAVE_MEMORY_TRACKING )
	// special marker tells free to use the chunk stack
	Mem::Free( Mem::MC_UNKNOWN, (uintptr_t) pBlock, 0, 0, 0  );
#else
	Mem::Free( Mem::MC_UNKNOWN, (uintptr_t) pBlock );
#endif
}

void operator delete[]( void* pBlock, const std::nothrow_t& )
{
#if defined( _HAVE_MEMORY_TRACKING )
	// special marker tells free to use the chunk stack
	Mem::Free( Mem::MC_UNKNOWN, (uintptr_t) pBlock, 0, 0, 0  );
#else
	Mem::Free( Mem::MC_UNKNOWN, (uintptr_t) pBlock );
#endif
}


// overidden global new and delete operators
// this is to get round problems with ATG's memory tracking macros.
// note, these MUST NOT be inline
void* operator new( size_t memSize )
{
#if defined( _HAVE_MEMORY_TRACKING )
	return (void*) Mem::Alloc( Mem::MC_MISC, memSize, 0, 0, 0 );
#else
	return (void*) Mem::Alloc( Mem::MC_MISC, memSize );
#endif
}

void* operator new[]( size_t memSize )
{
#if defined( _HAVE_MEMORY_TRACKING )
	return (void*) Mem::Alloc( Mem::MC_MISC, memSize, 0, 0, 0 );
#else
	return (void*) Mem::Alloc( Mem::MC_MISC, memSize );
#endif
}

void* operator new( size_t memSize, const std::nothrow_t& )
{
#if defined( _HAVE_MEMORY_TRACKING )
	return (void*) Mem::Alloc( Mem::MC_MISC, memSize, 0, 0, 0 );
#else
	return (void*) Mem::Alloc( Mem::MC_MISC, memSize );
#endif
}

void* operator new[]( size_t memSize, const std::nothrow_t& )
{
#if defined( _HAVE_MEMORY_TRACKING )
	return (void*) Mem::Alloc( Mem::MC_MISC, memSize, 0, 0, 0 );
#else
	return (void*) Mem::Alloc( Mem::MC_MISC, memSize );
#endif
}

void* operator new( size_t memSize, size_t memAlign, const std::nothrow_t& )
{
#if defined( _HAVE_MEMORY_TRACKING )
	return (void*) Mem::MemAlign( Mem::MC_MISC, memSize, memAlign, 0, 0, 0 );
#else
	return (void*) Mem::MemAlign( Mem::MC_MISC, memSize, memAlign );
#endif
}

void* operator new[]( size_t memSize, size_t memAlign, const std::nothrow_t& )
{
#if defined( _HAVE_MEMORY_TRACKING )
	return (void*) Mem::MemAlign( Mem::MC_MISC, memSize, memAlign, 0, 0, 0 );
#else
	return (void*) Mem::MemAlign( Mem::MC_MISC, memSize, memAlign );
#endif
}

namespace Mem
{
void* ATG_AllocCallback( uint32_t size, uint32_t align, const char* pTag, short iLine)
{
#if !defined( _HAVE_MEMORY_TRACKING )
	UNUSED(pTag);
	UNUSED(iLine);
	return (void*) Mem::MemAlign( Mem::MC_GFX, size, align  );
#else
	return (void*) Mem::MemAlign( Mem::MC_GFX, size, align, "ATG", pTag, iLine );
#endif
}

void	ATG_FreeCallback( void* pAddress )
{
#if !defined( _HAVE_MEMORY_TRACKING )
	Mem::Free( Mem::MC_GFX, (uintptr_t) pAddress );
#else
	Mem::Free( Mem::MC_GFX, (uintptr_t) pAddress, __FILE__, NT_FUNC_NAME, __LINE__ );
#endif
}
} // end Mem namespace 

#ifdef PLATFORM_PS3
extern "C"
{
void* LegacyMiscMalloc( size_t size )
{
	return NT_MALLOC(size);
}

void LegacyMiscFree( void* ptr )
{
	return NT_FREE(ptr);
}

}
#endif




