/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Useful helper functionality
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_PPU_HELPERS_H
#define WWS_JOB_PPU_HELPERS_H

//--------------------------------------------------------------------------------------------------

#include <jobapi/jobprintf.h>
#include <wwsbase/types.h>
#include <stddef.h>

//--------------------------------------------------------------------------------------------------

//namespace Wws
//{
//namespace Job
//{

//--------------------------------------------------------------------------------------------------

#ifdef __SNC__
	#define WWSJOB_BREAKPOINT()						do { __builtin_stop(); } while ( false )
#else
	#define WWSJOB_BREAKPOINT()						do { __asm__("trap"); } while ( false )
#endif

//--------------------------------------------------------------------------------------------------

#define WWSJOB_PREDICT( cond, expected )	__builtin_expect( ( cond ), ( expected ) )

//--------------------------------------------------------------------------------------------------

#define WWSJOB_ALWAYS_ASSERT( condition )			do { if ( ! WWSJOB_PREDICT((long int)(condition), 1) ) { JobBasePrintf("Assertion failed at %s(%d): %s\n", __FILE__, __LINE__, #condition); WWSJOB_BREAKPOINT(); } } while (false)
#define WWSJOB_ALWAYS_ASSERT_MSG( condition, msg )	do { if ( ! WWSJOB_PREDICT((long int)(condition), 1) ) { JobBasePrintf("Assertion failed at %s(%d): %s\n", __FILE__, __LINE__, #condition); JobBasePrintf msg ; WWSJOB_BREAKPOINT(); } } while (false)

//--------------------------------------------------------------------------------------------------

#ifdef ASSERTS_ENABLED
	#define WWSJOB_ASSERT( condition )				WWSJOB_ALWAYS_ASSERT( condition )
	#define WWSJOB_ASSERT_MSG( condition, msg )		WWSJOB_ALWAYS_ASSERT_MSG( condition, msg )
#else
	#define WWSJOB_ASSERT( condition )				do {} while (false)
	#define WWSJOB_ASSERT_MSG( condition, msg )		do {} while (false)
#endif

//--------------------------------------------------------------------------------------------------

#define WWSJOB_ALIGNED(X) __attribute__((aligned(X)))

//--------------------------------------------------------------------------------------------------

#define WWSJOB_UNUSED(X) do { ((void)(X)); } while(0)

//--------------------------------------------------------------------------------------------------

inline bool WwsJob_IsPow2( U32 value )
{
	return ((value & (value - 1)) == 0);
}

//--------------------------------------------------------------------------------------------------

inline const void* WwsJob_AlignPtr( const void* pAddress, size_t alignment )
{
	WWSJOB_ASSERT(WwsJob_IsPow2(alignment));
	return (const void*)(((size_t(pAddress)) + (alignment - 1)) & ~(alignment - 1));
}

//--------------------------------------------------------------------------------------------------

inline void* WwsJob_AlignPtr( void* pAddress, size_t alignment )
{
	WWSJOB_ASSERT(WwsJob_IsPow2(alignment));
	return (void*)(((size_t(pAddress)) + (alignment - 1)) & ~(alignment - 1));
}

//--------------------------------------------------------------------------------------------------

inline U32 WwsJob_AlignU32( U32 val, size_t alignment )
{
	WWSJOB_ASSERT(WwsJob_IsPow2(alignment));
	return ((val + alignment - 1) & ~(alignment - 1));
}

//--------------------------------------------------------------------------------------------------

inline bool WwsJob_IsAligned( U32 value, U32 alignment )
{
	WWSJOB_ASSERT(WwsJob_IsPow2(alignment));
	
	return !(value & (alignment - 1));
}

//--------------------------------------------------------------------------------------------------

inline bool WwsJob_IsAligned( const void* pAddress, size_t alignment )
{
	WWSJOB_ASSERT( WwsJob_IsPow2(alignment) );
	
	return !(size_t(pAddress) & (alignment - 1));
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_MemcpyQword( void* dest, const void* src, U32 size )
{
	WWSJOB_ASSERT( WwsJob_IsAligned( dest, 16 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( src, 16 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( size, 16 ) );

	VU32* pDest128		= (VU32*) dest;
	const VU32* pSrc128	= (const VU32*) src;

	for ( U32 i = 0; i < size; i += 16 )
	{
		*pDest128 = *pSrc128;
		++pDest128;
		++pSrc128;
	}
}

//--------------------------------------------------------------------------------------------------

template<typename T> inline T WwsJob_min(T const& a, T const& b)					{ return (a < b) ? a : b; }
template<typename T> inline T WwsJob_max(T const& a, T const& b)					{ return (a > b) ? a : b; }

//--------------------------------------------------------------------------------------------------

//} // namespace Job
//}; //namespace Wws

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_PPU_HELPERS_H */
