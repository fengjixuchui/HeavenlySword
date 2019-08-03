/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Functions for PPU interaction with DependencyCounters
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_DEPENDENCY_H
#define WWS_JOB_DEPENDENCY_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>

//--------------------------------------------------------------------------------------------------

struct DependencyCounter;
struct CellSpurs;

//--------------------------------------------------------------------------------------------------

U16 WwsJob_JobApiDecrementDependencyImmediate( CellSpurs* pSpurs, DependencyCounter* pDependencyCounter, U32 decrementVal );

//--------------------------------------------------------------------------------------------------

bool WwsJob_JobApiIsDependencySatisfied( DependencyCounter* pDependencyCounter );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_DEPENDENCY_H */
