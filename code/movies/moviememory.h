//------------------------------------------------------------------------------------------
//!
//!	\file moviememory.h
//!
//------------------------------------------------------------------------------------------

#ifndef	MOVIEMEMORY_H_
#define	MOVIEMEMORY_H_

namespace Movie
{
	// Define our memory chunk.
	static const Mem::MEMORY_CHUNK MemChunk = Mem::MC_MISC;

	// Required alignment for the following memory functions (in bytes).
	static const uint32_t Alignment = 32;

	// Memory allocation/deallocation to replace default functions.
	void *		Alloc	( uint32_t num_bytes );
	void		Free	( void *mem_ptr );

	void		MemoryCreate();
	void *		GetFrameMemory();
	void		ReleaseFrameMemory();
	uint32_t	GetFrameMemorySize();
	uint32_t	GetMaxNumFrames();
	void		MemoryDestroy();
	bool		MovieMemoryInUse();
}

#endif // !MOVIEMEMORY_H_


