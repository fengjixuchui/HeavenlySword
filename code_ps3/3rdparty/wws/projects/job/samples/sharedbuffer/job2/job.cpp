/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This is an SPU module to be used by the sharedbuffer sample
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>

#include "job.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

// note job2 parameters are copied by an api call, which is the 2nd of 3 ways to pass parameters
void JobMain( void )
{
	// enable this line if you want to single step every time this job starts
	//WWSJOB_BREAKPOINT();

	// copy the parameters (which we'll define as 8 U32's) into a local buffer
	U32 WWSJOB_ALIGNED(16) params[8];
	WwsJob_JobApiCopyParams( &params[0]/*16 aligned*/, sizeof(U32)*8/*16 aligned, non-zero*/ );

	// job begin audit, pass parameters
	WwsJob_JobApiStoreAudit( AuditId::kSharedBufferJob2_begin, (8 >> 1)/*#dwords*/, &params[0] );

	//Start the next job loading
	WwsJob_JobApiLoadNextJob();


	// get the bufferTag of the code buffer (containing this code)
	WwsJob_BufferTag codeBufferTag = WwsJob_JobApiGetBufferTag( 0/*code bufferSet#*/, 0/*logicalBuffer#*/,
			WwsJob_kDontAllocDmaTag );

	// use audit to print lsa of code buffer (to verify which of 2 buffers it went to)
	WwsJob_JobApiStoreAudit( AuditId::kSharedBufferJob2_codeBuffer, 0,
			(U64)codeBufferTag.GetAddr() );


	// get the bufferTag of the data buffer
	WwsJob_BufferTag dataBufferTag = WwsJob_JobApiGetBufferTag( 1/*data bufferSet#*/, 0/*logicalBuffer#*/,
			WwsJob_kDontAllocDmaTag );

	// use audit to print lsa of code buffer (to verify which of 2 buffers it went to)
	WwsJob_JobApiStoreAudit( AuditId::kSharedBufferJob2_dataBuffer, 0,
			(U64)dataBufferTag.GetAddr() );


	// add 1.0 to the data buffer
	// note the data is in a input persistant buffer so the job manager will handle loading & saving it
	F32* pData = (F32*) dataBufferTag.GetAddr();
	U32 numWords = dataBufferTag.GetLength() >> 2;
	for( U32 i = 0 ; i < numWords ; i++ )
	{
		pData[i] += 1.0;
	}

	// end job audit
	WwsJob_JobApiStoreAudit( AuditId::kSharedBufferJob2_end );
}

//--------------------------------------------------------------------------------------------------
