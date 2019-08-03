#if !defined(CORE_HEAP_H)
#define CORE_HEAP_H

//---------------------------------------
//!
//! \file core\heap.h
//! this is a version of Doug Lea's malloc
//! modified to work from a fixed block of memory
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//---------------------------------------

#if !defined(CORE_SYNCPRIMS_H)
#include "syncprims.h"
#endif

//---------------------------------------
//!
//! A heap allocator 
//!
//! This using a 'proper' heap allocator
//! from Doug Lea (its used for the Linux
//! std C++ heap!. Its been modified to 
//! work within a fixed bit of memory.
//! Essentially it allows lots of indepdent
//! bits of memory to each have new/malloc
//!
//! Note: This heap will probably work for GDDR
//! but will likely be quite slow and bad
//! it uses in place pointer (far from the CPU)
//! and the alignment rules are more complex...
//! 
//! Best only use for XDDR pools
//!
//---------------------------------------
class Heap
{
public:
	//! default ctor does nothing.
	Heap(){};

	//! for you real c++ people.
	Heap( uintptr_t pAddr, uint32_t iSize );

	//! as we use this in some places where ctor are not called 
	//! this is the real initialiser
	void Initialise( uintptr_t pAddr, uint32_t iSize );

	//! instead of a dtor
	void Kill();

	//! just a helper to get the base address back out.
	uintptr_t GetBaseAddress();

	//! Returns a pointer to a newly allocated chunk of at least n bytes, or 0
	//! if no space is available. Guarenteed 16 byte alignment
	void*	Malloc(size_t iSize);

	//!  Releases the chunk of memory pointed to by pMem, that had been previously
	//!  allocated using malloc .
	void	Free( void* pMem );

	//! Shrinks the size of an existing allocation
	void*	Shrink( void* pMem, uint32_t iSize );	

	//! Returns a pointer to a newly allocated chunk of n bytes, aligned
	//! in accord with the alignment argument.
	void*  Memalign(size_t iAlignment, size_t iSize);

	//! the total size this allocator controls
	uint32_t GetTotalSpace();

	//! how much space have we used
	uint32_t GetUsedSpace();

	//! how much space do we have left TODO
	uint32_t GetCurrentFreeSpace(){ return 0; }

	//---------------------------------
	// Policy functions follow that 
	// make it work as a general memory allocator
	//---------------------------------

	//! For a general allocation system we treat all 
	//! 'normal' alloc as bottom up allocations.
	uintptr_t	Alloc( uint32_t iSize );

	//! Shrinks the size of an existing allocation
	uintptr_t	Shrink( uintptr_t pMem, uint32_t iSize );	

	//!  Releases the chunk of memory pointed to by pMem, that had been previously
	//!  allocated using malloc .
	void		Free( uintptr_t pMem );

	//! Returns a pointer to a newly allocated chunk of n bytes, aligned
	//! in accord with the alignment argument.
	uintptr_t	MemAlign(uint32_t iSize, uint32_t iAlignment);

	//! Is Ours, returns if this pointer belong to this allocator
	bool IsOurs( uintptr_t pMem );

private:
	//! this a 1K internal header that is used to keep track of everthing
	struct malloc_state* m_pMallocState;

	//! consolatidate chunk to reduce fragmentation
	void malloc_consolidate(malloc_state* av);

	void* Heap::sys_malloc(uint32_t nb, malloc_state* av);

	char* m_pCurSysHigh;
	char* m_pMemTop;

	CriticalSection m_HeapCritical;
};

//---------------------
// Inlines
//---------------------

//---------------------------------------
//!
//! for you real c++ people
//!
//---------------------------------------
inline 	Heap::Heap( uintptr_t pAddr, uint32_t iSize )
{
	Initialise( pAddr, iSize );
}

//---------------------------------
//!
//!
//---------------------------------
inline 	uintptr_t Heap::GetBaseAddress()
{
	return (uintptr_t) m_pMallocState;
}

//---------------------------------
//!
//!
//---------------------------------
inline uintptr_t Heap::Alloc( uint32_t iSize )
{
	return (uintptr_t)Malloc( iSize );
}

inline uintptr_t Heap::Shrink( uintptr_t pMem, uint32_t iSize )
{
	return (uintptr_t)Shrink( (void*)pMem, iSize );
}

//---------------------------------
//!
//!
//---------------------------------
inline void Heap::Free( uintptr_t pMem )
{
	Free( (void*)pMem );
}

//---------------------------------
//!
//!
//---------------------------------
inline uintptr_t Heap::MemAlign( uint32_t iSize, uint32_t iAlignment )
{
	return (uintptr_t)Memalign( iAlignment, iSize );
}

//---------------------------------
//!
//!
//---------------------------------
inline bool Heap::IsOurs( uintptr_t pMem )
{
	// all can have free 0
	if( pMem == 0 )
		return true;
	return( pMem >= GetBaseAddress() && pMem <=  (uintptr_t) m_pMemTop );
}

#endif // end CORE_HEAP_H
