/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Manage a joblist from the PPU side.
				SingleThreadJobList may only be used if you are sure only a single PPU thread
				will be adding at a time.
				MultiThreadSafeJobList is for use if a joblist may be added to by multiple PPU
				threads at once or by SPUs.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_EXCEPTION_HANDLER_H
#define WWS_JOB_EXCEPTION_HANDLER_H

//--------------------------------------------------------------------------------------------------

struct CellSpurs;
struct CellSpursExceptionInfo;

//--------------------------------------------------------------------------------------------------

typedef void (*WwsJobExceptionEventHandler) ( CellSpurs*, const CellSpursExceptionInfo*, void* );

//--------------------------------------------------------------------------------------------------

extern WwsJobExceptionEventHandler gpWwsJobExceptionEventHandlerCallback;

//--------------------------------------------------------------------------------------------------

extern void WwsJobExceptionHandlerCallback( CellSpurs* pSpurs, const CellSpursExceptionInfo* pExceptionInfo, void* pMyData );

//--------------------------------------------------------------------------------------------------

inline void WwsJobSetExceptionHandlerCallbackFunction( WwsJobExceptionEventHandler funcPtr )
{
	WWSJOB_ASSERT( funcPtr );
	gpWwsJobExceptionEventHandlerCallback = funcPtr;
}

//--------------------------------------------------------------------------------------------------

inline WwsJobExceptionEventHandler WwsJobGSetExceptionHandlerCallbackFunction( void )
{
	return gpWwsJobExceptionEventHandlerCallback;
}
//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_EXCEPTION_HANDLER_H */
