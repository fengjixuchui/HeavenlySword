//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Graphics Core Initialisation Parameters		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_INIT_PARAMS_INL_H
#define GC_INIT_PARAMS_INL_H

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline	GcInitParams::GcInitParams()
	:
	m_displayMode( Gc::kDisplayAuto ),
	m_displaySwapMode( Gc::kDisplaySwapModeVSync ),
	m_pAllocatorRegionMem( NULL ),
	m_numAllocatorRegions( kDefaultAllocatorNumRegions ),
	m_reportBufferSize( 0 ),
	m_pushBufferSize( kDefaultPushBufferSize ),
	m_pManagedHostMem( NULL )
{
	SetManagedHostMemSize( FwAlign( ( u32 )( 2*kDefaultPushBufferSize + kDriverHostMemSize ), ( u32 )( 1024 * 1024 ) ) );

#ifdef ATG_PC_PLATFORM
	m_customDisplayWidth = m_customDisplayHeight = 0;
#endif
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::SetDisplayMode(Gc::DisplayMode mode)
{
	m_displayMode = mode;
}

//--------------------------------------------------------------------------------------------------

inline Gc::DisplayMode GcInitParams::GetDisplayMode( void ) const
{
	return m_displayMode;
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::SetDisplaySwapMode(Gc::DisplaySwapMode mode)
{
	m_displaySwapMode = mode;
}

//--------------------------------------------------------------------------------------------------

inline Gc::DisplaySwapMode GcInitParams::GetDisplaySwapMode( void ) const
{
	return m_displaySwapMode;
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::SetReportBufferSize( int reportBufferSize )
{
	FW_ASSERT((reportBufferSize >= 0) && (reportBufferSize <= (16 * 1024 * 1024 - kDriverHostMemSize)));
	m_reportBufferSize = reportBufferSize;
}

//--------------------------------------------------------------------------------------------------

inline int GcInitParams::GetReportBufferSize( void ) const
{
	return m_reportBufferSize;
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::SetPushBufferSize( int pushBufferSize )
{
	FW_ASSERT(pushBufferSize > 0);
	m_pushBufferSize = pushBufferSize;
}

//--------------------------------------------------------------------------------------------------

inline int GcInitParams::GetPushBufferSize( void ) const
{
	return m_pushBufferSize;
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::SetManagedHostMemSize( int hostMemSize )
{
	FW_ASSERT( FwIsAligned( (u32)hostMemSize, 1024*1024 ) && (hostMemSize >= kDriverHostMemSize) );
	m_managedHostMemSize = hostMemSize;
}

//--------------------------------------------------------------------------------------------------

inline int GcInitParams::GetManagedHostMemSize( void ) const
{
	return m_managedHostMemSize;
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::SetAllocatorRegionMem(Ice::Render::Region* pRegions, u32 numRegions)
{
	FW_ASSERT(numRegions > 0);

	m_pAllocatorRegionMem = pRegions;
	m_numAllocatorRegions = numRegions;
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::GetAllocatorRegionMem(Ice::Render::Region** ppRegions, u32* pNumRegions) const
{
	FW_ASSERT(ppRegions && pNumRegions);

	*ppRegions = m_pAllocatorRegionMem;
	*pNumRegions = m_numAllocatorRegions;
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::SetManagedHostMem( void* pXdr )
{
	FW_ASSERT( FwIsAligned( pXdr, 1024*1024 ) );
	m_pManagedHostMem = pXdr;
}

//--------------------------------------------------------------------------------------------------

inline void* GcInitParams::GetManagedHostMem( void ) const
{
	return m_pManagedHostMem;
}

//--------------------------------------------------------------------------------------------------

#ifdef ATG_PC_PLATFORM

inline void GcInitParams::SetCustomDisplayDimensions(u32 width, u32 height)
{
	FW_ASSERT((width > 0) && (height > 0));

	m_customDisplayWidth = width;
    m_customDisplayHeight = height;
}

//--------------------------------------------------------------------------------------------------

inline void GcInitParams::GetCustomDisplayDimensions(u32* pWidth, u32* pHeight) const
{
	FW_ASSERT(pWidth && pHeight);

	*pWidth = m_customDisplayWidth;
    *pHeight = m_customDisplayHeight;
}


#endif // ATG_PC_PLATFORM

//--------------------------------------------------------------------------------------------------

#endif // GC_INIT_PARAMS_INL_H

