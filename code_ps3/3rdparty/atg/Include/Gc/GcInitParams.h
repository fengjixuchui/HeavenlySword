//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Graphics Core Initialisation Parameters		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_INIT_PARAMS_H
#define GC_INIT_PARAMS_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>
#ifdef ATG_PC_PLATFORM
#include <windows.h>
#endif

//--------------------------------------------------------------------------------------------------
//	FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GcKernel;

//--------------------------------------------------------------------------------------------------

#ifdef ATG_PC_PLATFORM

namespace Gc
{
	extern void GetDisplayModeDimensions( const GcInitParams& initParams, u32& dispWidth, u32& dispHeight );
}

#endif // ATG_PC_PLATFORM
	
//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class GcInitParams
{
public:                               

	// Construction & Destruction
	
	GcInitParams();
	
	// Display Mode Accessors
	void					SetDisplayMode( Gc::DisplayMode mode );
	Gc::DisplayMode			GetDisplayMode( void ) const;

    // Display Swap Mode Accessors
	//
	// Use to set display swaps at the next V-Sync (default) or at the next H-Sync - in other words,
	// V-Sync disabled.
	
	void					SetDisplaySwapMode(Gc::DisplaySwapMode mode);
	Gc::DisplaySwapMode		GetDisplaySwapMode( void ) const;
	
	// Gc managed Host Memory includes (in order)
	//
	//		* Gc Driver Memory				~16K
	//		* Report Buffer Memory			Optional - Set via SetReportBufferSize()
	//		* Push Buffer Memory			Set via SetPushBufferSize()
	//		
	//		* Managed Host Memory Heap		All remaining managed Host Memory
	//
	// Memory from the managed Host Memory heap can be allocated and freed using GcKernel::AllocateHostMemory()
	// and GcKernel::FreeHostMemory() respectively.
	//
	// Gc Scratch Memory is also allocated from the managed Host Memory heap - this allocated using
	// GcKernel::SetTotalScratchMemory().
	//
	// The size of Gc managed Host Memory - set using GcInitParams::SetManagedHostMemSize() - must be big
	// enough to hold all of the above, plus all the Gc resources you wish to allocate in host memory.
	
	// Accessors to set and query the size of Gc managed Host Memory.
	//
	// @note	hostMemSize must be 1MB aligned.
	
	void					SetManagedHostMemSize( int hostMemSize );
	int						GetManagedHostMemSize( void ) const;

	// Host Memory required by the underlying graphics driver.
	//
	// :NOTE: This value may need updating as Ice::Render changes.
	
	static const int		kDriverHostMemSize = ( 80*1024 + 128 ) + 4*sizeof( Ice::Render::Report );

	// Set Managed Host Memory - if you wish to provide your own block of XDR memory. The default
	// behaviour is for GcKernel to allocate its own.
	//
	// :NOTE: The block of XDR memory provided must be 1MB aligned and equal in size to the value
	// returned by GetManagedHostMemSize().
	
	void					SetManagedHostMem( void* pXdr );
	void*					GetManagedHostMem( void ) const;

	// Set allocation management memory that is required by the managed Host and Video Memory allocator.
	// Use to override defaults.
	//
	//	@param	pRegions	Ptr to array of region structs used for allocation management.
	//						Must be big enough to hold 'numRegions' entries or NULL implying
	//						GcKernel will make the allocation.
	//
	//	@param	numRegions	Maximum number of regions handled by the managed memory allocator.
	//
	//	@note	If supplying your own memory via 'pRegions' you are responsible for freeing it *after*
	//			GcKernel::Shutdown() has been executed.

	void					SetAllocatorRegionMem(Ice::Render::Region* pRegions, u32 numRegions);

	// Default allocation management memory size (768K). Reduce using SetAllocatorRegionMem().

	static const u32		kDefaultAllocatorNumRegions = 0x10000;
	static const u32		kDefaultAllocatorRegionMemSize = sizeof(Ice::Render::Region) * kDefaultAllocatorNumRegions;
	
	
	// Report Buffer Accessors
	//
	// RSX reports must reside in the first 16MB of Host Memory. Using SetReportBufferSize() will inform
	// GcKernel to automatically make a Host Memory allocation for you 'before' any others are made.
	// That way, we can guarantee that first 16MB of Host Memory is available for reports should you need it.
	//
	// You can then use GcKernel::GetReportBufferMemoryInfo() to obtain the allocated report buffer for you
	// to manage as you wish. The default Report Buffer size is zero.
	
	void					SetReportBufferSize( int reportBufferSize );
	int						GetReportBufferSize( void ) const;
	
	// Global Push Buffer Accessors
	//
	// :NOTE: The total Host Memory consumption is 2 * GetPushBufferSize() as the global Push Buffer
	// owned by GcKernel is double buffered.

	void					SetPushBufferSize( int pushBufferSize );
	int						GetPushBufferSize( void ) const;

#ifdef ATG_PC_PLATFORM

	// PC Platform Window Handle Accessors

	// Set Custom Display Dimensions (for non HD resolutions)

	void					SetCustomDisplayDimensions(u32 width, u32 height);
	
#endif

private:

	// Friends
	
	friend class GcKernel;
    
	// Constants

	static const int		kDefaultPushBufferSize = 2 * 1024 * 1024;	///< 2MB

	// Attributes

	Gc::DisplayMode			m_displayMode;			///< Display mode
	Gc::DisplaySwapMode		m_displaySwapMode;		///< Mode for controlling display swaps

	Ice::Render::Region*	m_pAllocatorRegionMem;	///< Ice::Render allocation management memory.
	u32						m_numAllocatorRegions;	///< Max number of Ice::Render allocation regions.
	
	int						m_reportBufferSize;		///< The size of the report buffer (if any)
	int						m_pushBufferSize;		///< The size of the internal pushbuffer.
	
	void*					m_pManagedHostMem;		///< Ptr to user-allocated XDR (if any) for managed host memory.
	int						m_managedHostMemSize;	///< Host memory (i.e. xdr) managed by Gc.

	// Operations

	void					GetAllocatorRegionMem(Ice::Render::Region** ppRegions, u32* pNumRegions) const;

#ifdef ATG_PC_PLATFORM

	u32						m_customDisplayWidth;	///< Custom display dimensions (for Gc::kDisplayCustom)
	u32						m_customDisplayHeight;

	// Operations

	friend void		Gc::GetDisplayModeDimensions( const GcInitParams& initParams, u32& dispWidth, u32& dispHeight );
	
	void			GetCustomDisplayDimensions(u32* pWidth, u32* pHeight) const;

#endif
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gc/GcInitParams.inl>

//--------------------------------------------------------------------------------------------------

#endif // GC_INIT_PARAMS_H
