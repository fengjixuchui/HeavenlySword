#if !defined(CORE_MEM_H)
#define CORE_MEM_H
//---------------------------------------
//!
//!	\file core\mem.h
//! Macros and enums etc. that are required for our memory
//! management system.
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
/// \note
//! This is not multi-threaded safe! If you have any
//! multi-threaded requirements you MUST have you own
//! chunk and critical section it itself.
//! Also the stats are currently not multi-thread safe...
//! though the debug memory tracker is...
//---------------------------------------

// comment this in if you want to debug our memory changes for a masterbuild
// but you have 392 available
// #define _HAVE_DEBUG_MEMORY

//! whether we are tracking all memory allocations
#if !defined( _RELEASE ) && defined( _HAVE_DEBUG_MEMORY )
#	define _HAVE_MEMORY_TRACKING
#endif

#if defined( PLATFORM_PC )
#define NT_FUNC_NAME __FUNCTION__
#elif defined( PLATFORM_PS3 )
#define NT_FUNC_NAME __PRETTY_FUNCTION__
#endif

namespace Mem
{
	static const uint32_t Kb = 1024;		//!< 17*Kb is neater than 48128
	static const uint32_t Mb = 1024 * Kb;	//!< 13*Mb is neater than 13631488

	enum MEMORY_SCOPE
	{
		MS_GLOBAL	= 0x0,
		MS_LEVEL,
		MS_AREA
	};

	enum MEMORY_CHUNK
	{
		MC_RSX_MAIN_INTERNAL	= 0,//!< main ram that RSX can see, push buffers etc.
		MC_RSX_MAIN_USER,			//!< this is a sub allocation of above with a heap for game useage of RSX addressable ram

		// note to chunk adders... you must also update typeof.h and SoftObject.cpp in the odb with each new chunk
		// though remember, only expose what may be serialised.

		MC_GFX,						//!< ATG and our GFX engine
		MC_LOADER,					//!< Loader stuff goes here
		MC_ANIMATION,				//!< Animation allocations, eg loaded anims, anim event data
		MC_HAVOK,					//!< note HAVOK is multi-thread aware
		MC_AUDIO,					//!< Guess		

		MC_ODB,						//!< Object DataBase allocations from here

		MC_ARMY,					//!< Army specific memory
		MC_LUA,						//!< Lua memory stuff
		MC_ENTITY,					//!< Entity memory here
		MC_MISC,					//!< Misc alloc go here

		MC_AI,						//!< All AI goes here
		MC_CAMERA,					//!< Camera memory here
		MC_EFFECTS,					//!< Special FX e.g. trails, particles
		MC_PROCEDURAL,				//!< Speedtree, hair, blendshapes

		// nobody should use this explicitly, is where our overflowed allocs
		// go, which is the majority on a _MASTER build
		MC_OVERFLOW,

#ifdef _HAVE_DEBUG_MEMORY
		MC_DEBUG,					//!< total debug memory
#endif

#ifdef _HAVE_MEMORY_TRACKING
		MC_DEBUG_MEM_TRACKER,		//!< this is purely for the mem tracking, anything in here isn't tracked (to stop recursion)
#endif

		// must be the last real memory chunk
		MC_NUM_CHUNKS,
		MC_UNKNOWN = MC_NUM_CHUNKS
	};

	void*	ATG_AllocCallback( uint32_t size, uint32_t , const char*, short );
	void	ATG_FreeCallback( void* pAddress );

}; // end namespace Mem


#if !defined( CORE_MEMMAN_H )
#include "core/memman.h"
#endif

// no memory tracking
#if !defined( _HAVE_MEMORY_TRACKING )

#define NT_NEW_CHUNK( chunk )						new( chunk )
#define NT_NEW										NT_NEW_CHUNK(Mem::MC_MISC)
#define NT_NEW_ARRAY_CHUNK( chunk )					new( chunk )
#define NT_NEW_ARRAY								NT_NEW_ARRAY_CHUNK(Mem::MC_MISC)
#define NT_PLACEMENT_NEW( mem_loc )					new ( mem_loc )

#define NT_DELETE_CHUNK( chunk, pointer )			delete pointer
#define NT_DELETE_ARRAY_CHUNK( chunk, pointer )		delete[] pointer
#define NT_DELETE( pointer )						NT_DELETE_CHUNK( Mem::MC_MISC, pointer )
#define NT_DELETE_ARRAY( pointer )					NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, pointer )

#define NT_ALLOC_CHUNK( chunk, size ) Mem::Alloc( chunk, size )
#define NT_MEMALIGN_CHUNK( chunk, size, align ) Mem::MemAlign( chunk, size, align )
#define NT_MEMALIGN_CHUNK_RET_NULL( chunk, size, align ) Mem::MemAlign( chunk, size, align, false )
#define NT_FREE_CHUNK( chunk, ptr ) Mem::Free( chunk, ptr )
#define NT_SHRINK_CHUNK( chunk, ptr, size) Mem::Realloc( chunk, ptr, size )

void* operator new( size_t memSize, Mem::MEMORY_CHUNK chunk );
void* operator new[]( size_t memSize, Mem::MEMORY_CHUNK chunk );

#else // lots of memory tracking

#define NT_NEW_CHUNK( chunk )						new( chunk, __FILE__, NT_FUNC_NAME, __LINE__  )
#define NT_NEW										NT_NEW_CHUNK(Mem::MC_MISC)
#define NT_NEW_ARRAY_CHUNK( chunk )					new( chunk, __FILE__, NT_FUNC_NAME, __LINE__  )
#define NT_NEW_ARRAY								NT_NEW_ARRAY_CHUNK(Mem::MC_MISC)
#define NT_PLACEMENT_NEW( mem_loc )					new ( mem_loc )

#define NT_DELETE_CHUNK( chunk, pointer )			delete pointer
#define NT_DELETE_ARRAY_CHUNK( chunk, pointer )		delete[] pointer
#define NT_DELETE( pointer )						NT_DELETE_CHUNK( Mem::MC_MISC, pointer )
#define NT_DELETE_ARRAY( pointer )					NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, pointer )

#define NT_ALLOC_CHUNK( chunk, size ) Mem::Alloc( chunk, size, __FILE__, NT_FUNC_NAME, __LINE__ )
#define NT_MEMALIGN_CHUNK( chunk, size, align ) Mem::MemAlign( chunk, size, align, __FILE__, NT_FUNC_NAME, __LINE__, true )
#define NT_MEMALIGN_CHUNK_RET_NULL( chunk, size, align ) Mem::MemAlign( chunk, size, align, __FILE__, NT_FUNC_NAME, __LINE__, false )
#define NT_FREE_CHUNK( chunk, ptr ) Mem::Free( chunk, ptr, __FILE__, NT_FUNC_NAME, __LINE__ )
#define NT_SHRINK_CHUNK( chunk, ptr, size) Mem::Realloc( chunk, ptr, size, __FILE__, NT_FUNC_NAME, __LINE__  )

void* operator new( size_t memSize, Mem::MEMORY_CHUNK chunk, const char* pTag, const char* pSubTag, int iLine );
void* operator new[]( size_t memSize, Mem::MEMORY_CHUNK chunk, const char* pTag, const char* pSubTag, int iLine );


#endif // end memory tracking

// these are for use in legacy C code only!
#define NT_MALLOC( size )	(void*)(NT_ALLOC_CHUNK( Mem::MC_MISC, size ))
#define NT_FREE( ptr )		NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)ptr )

#endif //CORE_MEM_H
