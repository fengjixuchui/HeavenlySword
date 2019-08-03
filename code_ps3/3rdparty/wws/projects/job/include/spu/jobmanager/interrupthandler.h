/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Install an interrupt handler at LS0 that will be called on stall and notify events
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_INTERRUPT_HANDLER_H
#define WWS_JOB_INTERRUPT_HANDLER_H

//--------------------------------------------------------------------------------------------------

extern "C" void InstallInterruptHandler( void );
extern "C" void ShutdownInterruptHandler( void );

//--------------------------------------------------------------------------------------------------

extern "C" void WwsJob_StartTagSpecificBarrieredNullListWithInterrupt( U32 dmaTagId );
extern "C" void WwsJob_StartTagAgnosticBarrieredNullListWithInterrupt( U32 dmaTagId );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_INTERRUPT_HANDLER_H */
