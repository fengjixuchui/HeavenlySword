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

extern "C" void JobMain( U32 param0, U32 param1, U32 param2, U32 param3, U32 param4, U32 param5, U32 param6, U32 param7  );

//--------------------------------------------------------------------------------------------------

void JobMain( U32 param0, U32 param1, U32 param2, U32 param3, U32 param4, U32 param5, U32 param6, U32 param7  )
{
	// enable this line if you want to single step every time this job starts
	//WWSJOB_BREAKPOINT();

	// job begin audit, pass parameters
	WwsJob_JobApiStoreAudit( AuditId::kSharedBufferJobx_begin, 0,
			( (U64)param0 << 32 ) | (U64)param1,
			( (U64)param2 << 32 ) | (U64)param3,
			( (U64)param4 << 32 ) | (U64)param5,
			( (U64)param6 << 32 ) | (U64)param7 );

	//Start the next job loading
	WwsJob_JobApiLoadNextJob();


	// get the bufferTag of the code buffer (containing this code)
	WwsJob_BufferTag codeBufferTag = WwsJob_JobApiGetBufferTag( 0/*code bufferSet#*/, 0/*logicalBuffer#*/,
			WwsJob_kDontAllocDmaTag );

	// use audit to print lsa of code buffer (to verify which of 2 buffers it went to)
	WwsJob_JobApiStoreAudit( AuditId::kSharedBufferJobx_codeBuffer, 0,
			(U64)codeBufferTag.GetAddr() );


	// get the bufferTag of the data buffer
	WwsJob_BufferTag dataBufferTag = WwsJob_JobApiGetBufferTag( 1/*data bufferSet#*/, 0/*logicalBuffer#*/,
			WwsJob_kDontAllocDmaTag );

	// use audit to print lsa of code buffer (to verify which of 2 buffers it went to)
	WwsJob_JobApiStoreAudit( AuditId::kSharedBufferJobx_dataBuffer, 0,
			(U64)dataBufferTag.GetAddr() );


	// fill the data buffer with 99.0
	// (If there was any job manager error then this would mangle the job2 persistent buffer)
	F32* pData = (F32*) dataBufferTag.GetAddr();
	U32 numWords = dataBufferTag.GetLength() >> 2;
	for( U32 i = 0 ; i < numWords ; i++ )
	{
		pData[i] = 99.0;
	}

									 
	// end job audit
	WwsJob_JobApiStoreAudit( AuditId::kSharedBufferJobx_end );
}

//--------------------------------------------------------------------------------------------------
