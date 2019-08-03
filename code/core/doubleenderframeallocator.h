#if !defined(CORE_DOUBLEENDERFRAMEALLOCATOR_H)
#define CORE_DOUBLEENDERFRAMEALLOCATOR_H

//---------------------------------------
//!
//! \file core\doubleenderframeallocator.h
//! this is a frame AKA linear allocator with
//! two ends for a little flexibity.
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

// c version
typedef uintptr_t BottomCheckpoint;	//!< A frame marker for the bottom allocations
typedef uintptr_t TopCheckpoint;	//!< A frame marker for the top allocations
typedef struct DoubleEnderFrameAllocatorCT
{
	uintptr_t m_pCurBottomAddr;		//!< current address of the bottom free space
	uintptr_t m_pCurTopAddr;		//!< current address of the top free space

	BottomCheckpoint	m_BaseFrame;	//!< Initial base frame (start of the memory this manages)
	TopCheckpoint		m_EndFrame;		//!< Initial end frame (end of the memory this manages)

	bool				m_bAllocUp;
} DoubleEnderFrameAllocatorC;

//---------------------------------------
//!
//! as we use this in some places where ctor are not called 
//! this is the real initialiser
//!
//---------------------------------------
inline 	void DoubleEnderFrameAllocator_Initialise( DoubleEnderFrameAllocatorC* pAllocator, uintptr_t pAddr, uint32_t iSize )
{
	pAllocator->m_BaseFrame = (BottomCheckpoint) pAddr;
	pAllocator->m_EndFrame = (TopCheckpoint)pAddr + iSize;
	pAllocator->m_pCurBottomAddr = (uintptr_t)pAllocator->m_BaseFrame;
	pAllocator->m_pCurTopAddr = (uintptr_t)pAllocator->m_EndFrame;
	pAllocator->m_bAllocUp = true;
}

//---------------------------------------
//!
//! just a helper to get the base address back out
//!
//---------------------------------------
inline	uintptr_t DoubleEnderFrameAllocator_GetBaseAddress( const DoubleEnderFrameAllocatorC* pAllocator ) 
{
	return (uintptr_t)pAllocator->m_BaseFrame;
}

//---------------------------------------
//!
//! get a bottom marker at the currently used point
//!
//---------------------------------------
inline BottomCheckpoint DoubleEnderFrameAllocator_GetBottomCheckpoint( const DoubleEnderFrameAllocatorC* pAllocator )
{
	return (BottomCheckpoint) pAllocator->m_pCurBottomAddr;
}

//---------------------------------------
//!
//! get a top marker at the currently used point
//!
//---------------------------------------
inline TopCheckpoint DoubleEnderFrameAllocator_GetTopCheckpoint( const DoubleEnderFrameAllocatorC* pAllocator )
{
	return (TopCheckpoint) pAllocator->m_pCurTopAddr;
}

//---------------------------------------
//!
//! set the current bottom to a previous taken marker
//!
//---------------------------------------
inline void DoubleEnderFrameAllocator_SetBottomCheckpoint( DoubleEnderFrameAllocatorC* pAllocator, BottomCheckpoint frame)
{
	pAllocator->m_pCurBottomAddr = (uintptr_t)frame;
	ntAssert_p( pAllocator->m_pCurBottomAddr < pAllocator->m_pCurTopAddr, ("Incorrect Frame Allocation") );
}

//---------------------------------------
//!
//! set the current top to a previous taken marker
//!
//---------------------------------------
inline void DoubleEnderFrameAllocator_SetTopCheckpoint( DoubleEnderFrameAllocatorC* pAllocator,  TopCheckpoint frame )
{
	pAllocator->m_pCurTopAddr = (uintptr_t)frame;
	ntAssert_p( pAllocator->m_pCurBottomAddr < pAllocator->m_pCurTopAddr, ("Incorrect Frame Allocation") );
}

//---------------------------------------
//!
//! allocate some space of the bottom heap
//!
//---------------------------------------
inline uintptr_t DoubleEnderFrameAllocator_AllocBottom( DoubleEnderFrameAllocatorC* pAllocator, uint32_t iSize )
{
	ntError_p( (pAllocator->m_pCurBottomAddr + iSize) <= pAllocator->m_pCurTopAddr, ("Bottom Frame Allocation overflow") );
	uintptr_t pCurAddr = pAllocator->m_pCurBottomAddr;
	pAllocator->m_pCurBottomAddr += iSize;
	return pCurAddr;
}

//---------------------------------------
//!
//! allocate some space of the top heap
//!
//---------------------------------------
inline uintptr_t DoubleEnderFrameAllocator_AllocTop( DoubleEnderFrameAllocatorC* pAllocator, uint32_t iSize )
{
	ntError_p( (pAllocator->m_pCurTopAddr - iSize) >= pAllocator->m_pCurBottomAddr, ("Top Frame Allocation overflow") );
	pAllocator->m_pCurTopAddr -= iSize;
	uintptr_t pCurAddr = pAllocator->m_pCurTopAddr;
	return pCurAddr;
}

//---------------------------------------
//!
//! Reset the bottom heap to the base 
//!
//---------------------------------------
inline void DoubleEnderFrameAllocator_ResetBottom( DoubleEnderFrameAllocatorC* pAllocator )
{
	DoubleEnderFrameAllocator_SetBottomCheckpoint( pAllocator, pAllocator->m_BaseFrame );
}

//! Reset the top heap to the end
inline void DoubleEnderFrameAllocator_ResetTop( DoubleEnderFrameAllocatorC* pAllocator )
{
	DoubleEnderFrameAllocator_SetTopCheckpoint( pAllocator, pAllocator->m_EndFrame );
}

inline void DoubleEnderFrameAllocator_SetAllocationDirection( DoubleEnderFrameAllocatorC* pAllocator, bool bUp )
{
	pAllocator->m_bAllocUp = bUp;
}

inline void DoubleEnderFrameAllocator_SwapAndResetAllocationDirection( DoubleEnderFrameAllocatorC* pAllocator )
{
	if( pAllocator->m_bAllocUp )
	{
		DoubleEnderFrameAllocator_ResetTop( pAllocator );
		DoubleEnderFrameAllocator_SetAllocationDirection( pAllocator, false );
	} else
	{
		DoubleEnderFrameAllocator_ResetBottom( pAllocator );
		DoubleEnderFrameAllocator_SetAllocationDirection( pAllocator, true );
	}
}

//---------------------------------------
//!
//! the total size this allocator controls
//!
//---------------------------------------
inline uint32_t DoubleEnderFrameAllocator_GetTotalSpace( DoubleEnderFrameAllocatorC* pAllocator )
{
	return (uint32_t)( ((uintptr_t)pAllocator->m_EndFrame) - ((uintptr_t)pAllocator->m_BaseFrame) );
}

//---------------------------------------
//!
//! how much space have we used
//!
//---------------------------------------
inline uint32_t DoubleEnderFrameAllocator_GetUsedSpace( DoubleEnderFrameAllocatorC* pAllocator )
{
	uintptr_t iTopUsed = ((uintptr_t)pAllocator->m_EndFrame) - pAllocator->m_pCurTopAddr;
	uintptr_t iBottomUsed = pAllocator->m_pCurBottomAddr - ((uintptr_t)pAllocator->m_BaseFrame);
	return (uint32_t)( iTopUsed + iBottomUsed );
}

//---------------------------------------
//!
//! how much space do we have left
//!
//---------------------------------------
inline uint32_t DoubleEnderFrameAllocator_GetCurrentFreeSpace( DoubleEnderFrameAllocatorC* pAllocator )
{
	return DoubleEnderFrameAllocator_GetTotalSpace( pAllocator ) - DoubleEnderFrameAllocator_GetUsedSpace( pAllocator );
}

//---------------------------------
//!
//! For a general allocation system we treat all 
//! 'normal' alloc as bottom up allocations
//!
//---------------------------------
inline uintptr_t DoubleEnderFrameAllocator_Alloc( DoubleEnderFrameAllocatorC* pAllocator, uint32_t iSize )
{
	if( pAllocator->m_bAllocUp )
		return DoubleEnderFrameAllocator_AllocBottom( pAllocator, iSize );
	else
		return DoubleEnderFrameAllocator_AllocTop( pAllocator, iSize );
}


//---------------------------------
//!
//! For a general allocation system we treat all 
//! MemAlign as bottom up allocations
//!
//---------------------------------
inline uintptr_t DoubleEnderFrameAllocator_MemAlign( DoubleEnderFrameAllocatorC* pAllocator,uint32_t iSize, uint32_t iAlignment )
{
	if(  pAllocator->m_bAllocUp )
	{
		// align our bottom
		 pAllocator->m_pCurBottomAddr = ( pAllocator->m_pCurBottomAddr + (iAlignment-1)) & ~(iAlignment-1);
		return DoubleEnderFrameAllocator_AllocBottom( pAllocator, iSize );
	} else
	{
		// align our top
		DoubleEnderFrameAllocator_AllocTop( pAllocator, iSize );
		pAllocator->m_pCurTopAddr = ( pAllocator->m_pCurTopAddr  & ~(iAlignment-1));
		return  pAllocator->m_pCurTopAddr;
	}
}


#if defined( __cplusplus )

//---------------------------------------
//!
//! A double ended frame allocator 
//!
//! (similar to one discribed in Game Gems 1, 
//! though indepedent written i.e I haven't 
//! really read it...)
//!
//! A Frame allocator is a linear allocator
//! where you can take 'checkpoints' 
//!
//! a checkpoint is a marker which you can 
//! reset too.
//!
//! The double ender bit means you actually
//! have two linear allocator working from 
//! either end of the memory. This means you
//! can have a little flexibity in alloc order
//! i.e. You can alloc 3 things from the bottom
//!		 then 1 from the top and reset either one
//!		 without affecting the other. Useful for 
//!		 temp allocations but still with the speed
//!		 and benefits of a true linear allocator
//!
//!
//---------------------------------------
class DoubleEnderFrameAllocator : private DoubleEnderFrameAllocatorC
{
public:

	//! default ctor does nothing.
	DoubleEnderFrameAllocator(){};

	//! for you real c++ people.
	DoubleEnderFrameAllocator( uintptr_t pAddr, uint32_t iSize );

	//! as we use this in some places where ctor are not called 
	//! this is the real initialiser
	void Initialise( uintptr_t pAddr, uint32_t iSize )
	{
		DoubleEnderFrameAllocator_Initialise( this, pAddr, iSize );
	}

	//! just a helper to get the base address back out.
	uintptr_t GetBaseAddress() const
	{
		return DoubleEnderFrameAllocator_GetBaseAddress( this );
	}

	//! get a bottom marker at the currently used point
	BottomCheckpoint GetBottomCheckpoint()
	{
		return DoubleEnderFrameAllocator_GetBottomCheckpoint( this );
	}

	//! get a top marker at the currently used point
	TopCheckpoint GetTopCheckpoint()
	{
		return DoubleEnderFrameAllocator_GetTopCheckpoint( this );
	}

	//! set the current bottom to a previous taken marker
	void SetBottomCheckpoint(BottomCheckpoint frame)
	{
		DoubleEnderFrameAllocator_SetBottomCheckpoint( this, frame );
	}
	
	//! set the current top to a previous taken marker
	void SetTopCheckpoint(TopCheckpoint frame)
	{
		DoubleEnderFrameAllocator_SetTopCheckpoint( this, frame );
	}

	//! allocate some space of the bottom heap
	uintptr_t AllocBottom( uint32_t iSize )
	{
		return DoubleEnderFrameAllocator_AllocBottom( this, iSize );
	}

	//! allocate some space of the top heap
	uintptr_t AllocTop( uint32_t iSize )
	{
		return DoubleEnderFrameAllocator_AllocTop( this, iSize );
	}

	uintptr_t	Shrink( uintptr_t addr, uint32_t )
	{
		ntAssert_p(0, ("RSX allocator can not shrink!\n"));
		return addr;
	}

	//! Reset the bottom heap to the base 
	void ResetBottom()
	{
		DoubleEnderFrameAllocator_ResetBottom( this );
	}

	//! Reset the top heap to the end
	void ResetTop()
	{
		DoubleEnderFrameAllocator_ResetTop( this );
	}

	//! the total size this allocator controls
	uint32_t GetTotalSpace()
	{
		return DoubleEnderFrameAllocator_GetTotalSpace( this );
	}

	//! how much space have we used
	uint32_t GetUsedSpace()
	{
		return DoubleEnderFrameAllocator_GetUsedSpace( this );
	}

	//! how much space do we have left
	uint32_t GetCurrentFreeSpace()
	{
		return DoubleEnderFrameAllocator_GetCurrentFreeSpace( this );
	}

	//! Set Direction, if we you to use this as a normal allocator we can choose the direction (up or down)
	void SetAllocationDirection( bool bUp )
	{
		DoubleEnderFrameAllocator_SetAllocationDirection( this, bUp );
	}

	//! swap and reset the direction, useful if you using this a simple double buffering system
	void SwapAndResetAllocationDirection( void )
	{
		DoubleEnderFrameAllocator_SwapAndResetAllocationDirection( this );
	}

	//---------------------------------
	// Policy functions follow that 
	// make it work as a general memory allocator
	//---------------------------------

	//! For a general allocation system we treat all 
	//! 'normal' alloc as bottom up allocations.
	uintptr_t Alloc( uint32_t iSize )
	{
		return DoubleEnderFrameAllocator_Alloc( this, iSize );
	}

	//! Free is completely ignored.
	void Free( uintptr_t pAlloc );

	//! an aligned bottom allocation
	uintptr_t MemAlign( uint32_t iSize, uint32_t iAlignment )
	{
		return DoubleEnderFrameAllocator_MemAlign( this, iSize, iAlignment );
	}

	//! does this pointer belong to us (useful for debugging)
	bool IsOurs( uintptr_t pMem );

private:
};

//---------------------
// Inlines
//---------------------

//---------------------------------------
//!
//! for you real c++ people
//!
//---------------------------------------
inline 	DoubleEnderFrameAllocator::DoubleEnderFrameAllocator( uintptr_t pAddr, uint32_t iSize )
{
	Initialise( pAddr, iSize );
}



//---------------------------------
//!
//!
//---------------------------------
inline bool DoubleEnderFrameAllocator::IsOurs( uintptr_t pMem )
{
	// all can have free 0
	if( pMem == 0 )
		return true;

	return( pMem >= m_BaseFrame && pMem <=  m_EndFrame );
}

//---------------------------------------
//!
//! Free is completely ignored.
//!
//---------------------------------------
inline void DoubleEnderFrameAllocator::Free( uintptr_t pAlloc )
{
	// Normal free's are completely ignored!
	UNUSED( pAlloc );
}


#endif

#endif // end CORE_DOUBLEENDERFRAMEALLOCATOR_H
