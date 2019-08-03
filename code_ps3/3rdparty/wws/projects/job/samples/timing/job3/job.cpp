/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This is an SPU module to be used by the timing sample
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>

#include "job.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

// note job3 parameters are retrieved via a pointer from an api call, which is the 3rd of 3 ways to pass parameters
void JobMain( void )
{
	// enable this line if you want to single step every time this job starts
	//WWSJOB_BREAKPOINT();

	// get a pointer to the parameters (which we'll define as 8 U32's)
	// Note this is called "unsafe" to make you aware that you can *NOT* fetch the parameter data
	//	after you have started the load of the next job
	const U32 *pParams = (const U32*)WwsJob_JobApiGetUnsafePointerToParams( );

	// job begin audit, pass parameters
	WwsJob_JobApiStoreAudit( AuditId::kTimingJob3_begin, (8 >> 1)/*#dwords*/, pParams );

	//Start the next job loading
	WwsJob_JobApiLoadNextJob();


	// get the bufferTag of the code buffer (containing this code)
	WwsJob_BufferTag codeBufferTag = WwsJob_JobApiGetBufferTag( 0/*code bufferSet#*/, 0/*logicalBuffer#*/,
			WwsJob_kDontAllocDmaTag );

	// use audit to print lsa of code buffer (to verify which of 2 buffers it went to)
	WwsJob_JobApiStoreAudit( AuditId::kTimingJob3_codeBuffer, 0,
			(U64)codeBufferTag.GetAddr() );


	// get the bufferTag of the data buffer
	// Note we will need a dmaTagId so we can do a dmaPut
	WwsJob_BufferTag dataBufferTag = WwsJob_JobApiGetBufferTag( 1/*data bufferSet#*/, 0/*logicalBuffer#*/,
			WwsJob_kAllocDmaTag );

	// use audit to print lsa of code buffer (to verify which of 2 buffers it went to)
	WwsJob_JobApiStoreAudit( AuditId::kTimingJob3_dataBuffer, 0,
			(U64)dataBufferTag.GetAddr() );


	// note the data is in a input buffer so the job manager will handle loading it (but not saving it)
	// store the buffer out
	JobDmaPut( (const volatile void*) dataBufferTag.GetAddr(), dataBufferTag.m_mmAddress,
			dataBufferTag.m_mmLength, dataBufferTag.GetDmaTag() );

	// note we don't have to wait till the dma is done since the job manager will ensure it is done
	// before it allows the buffer to be used (or the job to be considered "done").

	// end job audit
	WwsJob_JobApiStoreAudit( AuditId::kTimingJob3_end );
}

//--------------------------------------------------------------------------------------------------
