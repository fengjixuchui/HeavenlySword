/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This is an SPU module to be used by the audits sample
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobprintf.h>

#include "auditsjob.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( U32 auditIdBase );

//--------------------------------------------------------------------------------------------------

void JobMain( U32 auditIdBase )
{
	// enable this line if you want to single step every time this job starts
	//WWSJOB_BREAKPOINT();

	// avoid printf's since they take so long they throw off values of audits	  
	//JobPrintf( "auditsJob begin\n" );

	//Start the next job loading
	WwsJob_JobApiLoadNextJob();

	// audit with no data
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_test );

	// audit with hword
	//	Note this uses U16 for unsigned 16 bits.
	//  U01 to U64 is legal, but you have to have at least that many bits in the dword (you can't cross dword boundaries)
	//	Note ALL values (except F32) are printed in hex, and with the smallest # of digits to represent
	//		the maxiumum possible hex value.  This is done to keep the print size from varying.
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword, 0x1234 );

	// audit ignoring hword (via "pad"), but with 1 dword
	//	Note pad will ignore remaining bits in current dword (or hword)
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_test1Dword, 0x1234/*will be ignored*/,
			0x1000000010001234ULL );

	// audit with hword + 1 dword
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword1Dword, 0x1234,
			0x1000000010001234ULL );

	// audit with hword + 2 dword
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword2Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL );

	// audit with hword + 3 dword
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword3Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL );

	// audit with hword + 4 dword
	//	Note for the 2nd line this uses 1 tab (8 spaces per tab) & 4 spaces to indent the text to where the first line data is
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword4Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL, 0x4000000040001234ULL );

	// audit with hword + 5 dword
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword5Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL, 0x4000000040001234ULL, 0x5000000050001234ULL );

	// audit with hword + 6 dword
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword6Dword, 0x1234,
			0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL, 0x4000000040001234ULL, 0x5000000050001234ULL, 0x6000000060001234ULL );

	// for N audits below
	U64 data[] = { 0x1000000010001234ULL, 0x2000000020001234ULL, 0x3000000030001234ULL, 0x4000000040001234ULL,
				   0x5000000050001234ULL, 0x6000000060001234ULL, 0x7000000070001234ULL, 0x8000000080001234ULL };

	// audit with N dword, N=1
	//	Note N must be >= 1, and there is no hword (the #dwords is stored there)
	//	Note this uses "dump" which dumps dwords, up to 2 per line.
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testNDword, 1, &data[0] );

	// audit with N dword, N=8
	//	Note this prints an offset for all additional lines.
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testNDword, 8, &data[0] );



	// audit with signed hword (which is positive)
	//	Note this uses I16 instead of U16
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testSignedHword, 0x1234 );

	// audit with signed hword (which is negative)
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testSignedHword, (U16) -0x1234 );



	// for f32 audit
	union F32Etc
	{	U32	m_u32;
		F32	m_f32;
	};

	// audit with hword and 1 f32.  Note this uses a pad32 which skips the first 32 bits of the dword
	//	You can use pad01 to pad64 to skip 1:64 bits within a dword, or just pad to skip the remainder of a dword.
	//	Note this uses F32 to indicate the float, with a fixed print size so your text length doesn't vary.
	F32Etc ftmp1;
	ftmp1.m_f32 = 12.34f;
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword1F32, 0x1234,
			ftmp1.m_u32 );

	// audit with hword and 2 f32s
	F32Etc ftmp2;
	ftmp2.m_f32 = -23.45f;
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testHword2F32, 0x1234,
			((U64)ftmp1.m_u32 << 32) | (U64)ftmp2.m_u32 );



	// audit with the hword cut up into U1(1), I2(-2), U3(4), I4(+7), U5(0x10), (ignore remainder)
	//		and with the dword cut up into I6(-0x20), U7(0x40), I8(+0x7f), U9(0x100), I10(-0x200) (ignore remainder)
	//	Note each field uses the smallest number of hex digits needed to represent the maximum possible print value.
	WwsJob_JobApiStoreAudit( auditIdBase + AuditId::kAuditsJob_testPieces, /*note hword is on next line...*/
			(0x1 << (16-1)) | (0x2/*-2*/ << (16-1-2)) | (0x4 << (16-1-2-3)) | (0x7 << (16-1-2-3-4)) | (0x10 << (16-1-2-3-4-5)),
			((U64)0x20/*-0x20*/ << (64-6)) | ((U64)0x40 << (64-6-7)) | ((U64)0x7f << (64-6-7-8)) | ((U64)0x100 << (64-6-7-8-9)) | ((U64)0x200/*-0x200*/ << (64-6-7-8-9-10)) );
	
	

	// avoid printf's since they take so long they throw off values of audits	  
	//JobPrintf( "auditsJob end\n" );
}

//--------------------------------------------------------------------------------------------------
