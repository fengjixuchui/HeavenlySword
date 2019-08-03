//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Memory management

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_MEM_H
#define	FW_MEM_H

#ifdef	ATG_DEBUG_MODE
#define	ATG_MEMORY_DEBUG_ENABLED
#endif	// ATG_DEBUG_MODE

//--------------------------------------------------------------------------------------------------
// We need to include <new> to support placement new

#include	<new>

//--------------------------------------------------------------------------------------------------
/**
	@class			FwMemConfig
	
	@brief			Defines functions that are used by FwMem for allocation & free.

**/
//--------------------------------------------------------------------------------------------------

class	FwMemConfig
{
public:
	// Typedef's for our allocation/free callbacks, available for all to use
	typedef void*	( *AllocCallback )( u32 size, u32 alignment, const char* pTag, short lineNumber );
	typedef void	( *FreeCallback )( void* pAddress );

	AllocCallback	m_pAllocCallback;
	FreeCallback	m_pFreeCallback;
};

//--------------------------------------------------------------------------------------------------
/**
	@class			FwMem
	
	@brief			Primary interface for framework memory management system.

    The framework includes a replacement memory manager that will, in future, be tuned to the 
	requirements of the target platform. At the very least, allocations need to be 16-byte aligned,
	but there will be a need to handle fragmenting heaps in future versions of this system. 

	@param			pMemConfig		-	A pointer to a persistent FwMemConfig structure, which 
										contains callbacks that are invoked on allocation & free.	

	@note			On debug builds the 'FW_NEW' and 'FW_TAGGED_NEW' macros (with their array forms)
					are defined to enable tagging of memory allocations.
	
	@warning		The internal structure of this subsystem is likely to undergo extensive change
					in future. Please don't rely on undocumented internal behaviour.
**/
//--------------------------------------------------------------------------------------------------

class	FwMem
{
	friend	class	FwMemDebug;

public:
	// Initialisation
	static	void			Initialise( const FwMemConfig* pMemConfig = NULL);
	static	bool			IsInitialised( void );

	// Shutdown
	static	void			Shutdown( void );

	// Obtain allocation/free callbacks
	inline	static	FwMemConfig::AllocCallback	GetAllocCallback( void );
	inline	static	FwMemConfig::FreeCallback	GetFreeCallback( void );
	inline	static	FwMemConfig::AllocCallback	GetDefaultAllocCallback( void );
	inline	static	FwMemConfig::FreeCallback	GetDefaultFreeCallback( void );

	// Invoke allocation/free callbacks.
	static	void*			CallAllocCallback( u32 size, u32 alignment, const char* pTag, short lineNumber );
	static	void			CallFreeCallback( void* pAddress );

	// C-style aligned memory allocation
	inline	static	void*	Alloc( u32 size, u32 alignment = 16, const char* pTag = "FwMem::Alloc", short lineNumber = 0 );
	inline	static	void	Free( void* block );

	// Debug callbacks for allocation 
	static	void*			DefaultAllocCallback( u32 size, u32 alignment, const char* pTag, short lineNumber );
	static	void			DefaultFreeCallback( void* pAddress );

private:
	// Locking control (used by FwMem and FwMemDebug)
	static	void			Lock( void );
	static	void			Unlock( void );

	// We manage our own alignment now, rather than call OS functions.
	static const int			kAlignGapSize	= sizeof( void* );
	struct AlignMemBlockHdr
	{
		void*			pHead;
		unsigned char	gap[ kAlignGapSize ];
	};

	// Memory configuration
	static	const	FwMemConfig*	ms_pMemConfig;
	static	FwMemConfig				ms_defaultMemConfig;

	// Have we been initialised?
	static	bool					ms_isInitialised;
};


#ifdef	ATG_MEMORY_DEBUG_ENABLED
#include <Fw/FwMem/FwMemDebug.h>
#endif // ATG_MEMORY_DEBUG_ENABLED

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a pointer to the default allocation callback

	@note			Depending on build configuration, this will return the allocation callback from
					either FwMem or FwMemDebug.

	@return			Pointer to default allocation memory.
**/
//--------------------------------------------------------------------------------------------------

inline	FwMemConfig::AllocCallback	FwMem::GetDefaultAllocCallback( void )
{
#ifdef	ATG_MEMORY_DEBUG_ENABLED
	return	FwMemDebug::DefaultAllocCallback;
#else
	return	FwMem::DefaultAllocCallback;	
#endif // ATG_MEMORY_DEBUG_ENABLED
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a pointer to the default free callback

	@note			Depending on build configuration, this will return the free callback from either
					FwMem or FwMemDebug.

	@return			Pointer to default free callback
**/
//--------------------------------------------------------------------------------------------------

inline	FwMemConfig::FreeCallback		FwMem::GetDefaultFreeCallback( void )
{
#ifdef	ATG_MEMORY_DEBUG_ENABLED
	return	FwMemDebug::DefaultFreeCallback;
#else
	return	FwMem::DefaultFreeCallback;	
#endif // ATG_MEMORY_DEBUG_ENABLED
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve the currently active allocation callback

	@return			Pointer to currently active allocation callback
**/
//--------------------------------------------------------------------------------------------------

inline	FwMemConfig::AllocCallback	FwMem::GetAllocCallback( void )
{
	FW_ASSERT_MSG( ms_isInitialised, ( "FwMem::Initialise() must be called before using memory system\n" ) );
	return	ms_pMemConfig->m_pAllocCallback;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve the currently active free callback

	@return			Pointer to currently active free callback
**/
//--------------------------------------------------------------------------------------------------

inline	FwMemConfig::FreeCallback		FwMem::GetFreeCallback( void )
{
	FW_ASSERT_MSG( ms_isInitialised, ( "FwMem::Initialise() must be called before using memory system\n" ) );
	return	ms_pMemConfig->m_pFreeCallback;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Allocates aligned memory

	@param			memSize			Size of allocation (in bytes)
	@param			alignment		Power-of-two alignment
	@param			pTag			Tag to associate with this allocation
	@param			lineNumber		Line number to associate with this allocation

	@note			By default this function always tags allocations as being from FwMem::Alloc,
					line 0. 

	@warning		Memory allocated by this function *must* be freed using FwMem::Free.
**/
//--------------------------------------------------------------------------------------------------

inline void*	FwMem::Alloc( u32 memSize, u32 alignment, const char* pTag, short lineNumber )
{
#ifdef	ATG_MEMORY_DEBUG_ENABLED
	return FwMem::CallAllocCallback( ( u32 )memSize, alignment, pTag, lineNumber );
#else
	FW_UNUSED( pTag );
	FW_UNUSED( lineNumber );
	return FwMem::CallAllocCallback( ( u32 )memSize, alignment, NULL, 0 );
#endif // ATG_MEMORY_DEBUG_ENABLED
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Releases aligned memory allocated with FwMem::Alloc

	@param			pMemory			Pointer to memory allocated by FwMem::Alloc

	@warning		Do not use this function to free memory other than that allocated by FwMem::Alloc.
**/
//--------------------------------------------------------------------------------------------------

inline void	FwMem::Free( void* pMemory )
{
	return FwMem::CallFreeCallback( pMemory );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Query whether the memory system has been initialised.
**/
//--------------------------------------------------------------------------------------------------

inline bool	FwMem::IsInitialised()
{
	return ms_isInitialised;
}

// Include our replacements for new/delete
#include	<Fw/FwMem/FwMemNew.h>

//--------------------------------------------------------------------------------------------------
/**
	@brief			Replacement for (_)_alloca
*/
//--------------------------------------------------------------------------------------------------

#ifdef _MSC_VER
	#define FW_ALLOCA _alloca
#else
	#define FW_ALLOCA __builtin_alloca
#endif

#endif	// FW_MEM_H
