/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Job API function for doing an immediate dependency decrement from a PPU thread
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobdefinition.h>

#include <cell/spurs/ready_count.h>
#include <cell/atomic.h>

//--------------------------------------------------------------------------------------------------

U16 WwsJob_JobApiDecrementDependencyImmediate( CellSpurs* pSpurs, DependencyCounter* pDependencyCounter, U32 decrementVal )
{
	//Note that the U32 has the counter in the upper 16 bits
	U32 oldU32 = cellAtomicSub32( (U32*)pDependencyCounter, decrementVal << 16 );
	U32 oldVal = oldU32 >> 16;

	WWSJOB_ASSERT( oldVal >= decrementVal );

	if ( oldVal == decrementVal )
	{
		int ret = cellSpursReadyCountStore(	pSpurs,
											pDependencyCounter->m_workloadId,
											pDependencyCounter->m_readyCount );
		//Actually I probably only really want this to set the workload higher, never lower?
		WWSJOB_ASSERT( CELL_OK == ret );
		WWSJOB_UNUSED( ret );
	}

	return oldVal;
}

//--------------------------------------------------------------------------------------------------

bool WwsJob_JobApiIsDependencySatisfied( DependencyCounter* pDependencyCounter )
{
	return (pDependencyCounter->m_counter == 0);
}

//--------------------------------------------------------------------------------------------------
