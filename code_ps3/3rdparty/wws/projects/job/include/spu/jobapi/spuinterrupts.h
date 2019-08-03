/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Helper functions for disabling and enabling interrupts or querying interrupt state
				Note that the spu_ienable, spu_idisable intrinsics do not work at present with
				position independent code
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_SPU_INTERRUPTS_H
#define WWS_JOB_SPU_INTERRUPTS_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <spu_intrinsics.h>

//--------------------------------------------------------------------------------------------------

inline U32 AreInterruptsEnabled( void )
{
	U32 machStatus = spu_readch(SPU_RdMachStat);
	U32 interruptsEnabled = (machStatus & 1);
	return interruptsEnabled;
}

//--------------------------------------------------------------------------------------------------

inline void DisableInterrupts( void )
{
	__builtin_spu_idisable();
}

//--------------------------------------------------------------------------------------------------

inline void EnableInterrupts( void )
{
	__builtin_spu_ienable();
}

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_SPU_INTERRUPTS_H */
