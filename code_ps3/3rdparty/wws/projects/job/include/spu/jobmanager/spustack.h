/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Check for overflowing the allocated amount of SPU stack space
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_SPU_STACK_H
#define WWS_JOB_SPU_STACK_H

//--------------------------------------------------------------------------------------------------

#include <jobapi/jobdefinition.h>

//--------------------------------------------------------------------------------------------------

namespace SpuStackCheck
{
	enum
	{
		kStackOverflowMarker	= 0x71077345,
	};
}

//====================================================================================================================
// stack checking functions
//====================================================================================================================

inline void* PageNumToLsAddress( U32 pageNum )
{
	return (void*)(pageNum * 1024);
}

//--------------------------------------------------------------------------------------------------

inline void SetStackOverflowMarker( void )
{
	U32* pStackBase = (U32*) PageNumToLsAddress( LsMemoryLimits::kJobAreaEndPageNum );
	*pStackBase = SpuStackCheck::kStackOverflowMarker;
}

//--------------------------------------------------------------------------------------------------

inline void CheckStackOverflowMarker( void )
{
	const U32* pStackBase = (const U32*) PageNumToLsAddress( LsMemoryLimits::kJobAreaEndPageNum );
	WWSJOB_ASSERT( *pStackBase == SpuStackCheck::kStackOverflowMarker );
	WWSJOB_UNUSED( pStackBase );
}

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_SPU_STACK_H */
