/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This SPU module tests job audits
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "auditsmodule.h"

#include "jobaudits.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	// enable this line if you want to single step every time this job starts
	//WWSJOB_BREAKPOINT();

	JobPrintf( "***Audits Module: Begin***\n" );

	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_Job_Begin );

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( 1, 0, WwsJob_kAllocDmaTag );
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_Job_GotBufferTag );

	//Read out parameters before starting next job loading
	AuditsModuleParams auditsModuleParams;
	WwsJob_JobApiCopyParams( &auditsModuleParams, sizeof(auditsModuleParams) );
	U32 ea			= auditsModuleParams.m_eaOutputAddr;
	U8 multiplier	= auditsModuleParams.m_multiplier;
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_Job_CopiedParameters );


	//Start next job loading
	WwsJob_JobApiLoadNextJob();
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_Job_StartedNextJobLoading );

	//do all the actual work *after* we've started the next job loading

	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	//double the contents of the buffer
	for (U32 i = 0; i < size; ++i)
	{
		lsAddr[i] = lsAddr[i] * multiplier;
	}
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_Job_ProcessedData );

	//Send the buffer back to main memory
	JobDmaPut( lsAddr, ea, size, buffTag.GetDmaTag() );



	// begin test other forms of audits

	// audit with hword
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_testHword, 0x1234 );

	// audit with hword + 1 dword
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_test1Dword, 0x1234,
			0x1000000010001234ULL );

	// audit with hword + 2 dword
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_test2Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL );

	// audit with hword + 3 dword
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_test3Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL );

	// audit with hword + 4 dword
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_test4Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL, 0x4000000040001234ULL );

	// audit with hword + 5 dword
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_test5Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL, 0x4000000040001234ULL, 0x5000000050001234ULL );

	// audit with hword + 6 dword
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_test6Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL, 0x4000000040001234ULL, 0x5000000050001234ULL, 0x6000000060001234ULL );

	// for N audits below
	U64 data[] = { 0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL, 0x4000000040001234ULL,
				   0x5000000050001234ULL, 0x6000000060001234ULL, 0x7000000070001234ULL, 0x8000000080001234ULL };

	// audit with hword + N dword, N=1
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_testNDword, 1, &data[0] );

	// audit with hword + N dword, N=8
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_testNDword, 8, &data[0] );

	// end   test other forms of audits


	
	WwsJob_JobApiStoreAudit( AuditId::kAuditsModule_Job_End );
	JobPrintf( "***Audits Module: Done*\n" );
}

//--------------------------------------------------------------------------------------------------
