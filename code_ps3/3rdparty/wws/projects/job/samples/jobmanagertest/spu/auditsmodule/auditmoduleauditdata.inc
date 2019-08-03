/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Define the enums and text strings for audits used by the jobmanagertest/auditsmodule
**/
//--------------------------------------------------------------------------------------------------

#ifndef AUDIT_DATA
#error This is not a standard header file for user inclusion
#endif

//--------------------------------------------------------------------------------------------------


AUDIT_DATA( kAuditsModule_Job_Begin,					 	 	"Job_Begin:" )
AUDIT_DATA( kAuditsModule_Job_GotBufferTag,				 	 	"Job_GotBufferTag:" )
AUDIT_DATA( kAuditsModule_Job_CopiedParameters,			 	 	"Job_CopiedParameters:" )
AUDIT_DATA( kAuditsModule_Job_StartedNextJobLoading,	 	 	"Job_StartedNextJobLoading:" )
AUDIT_DATA( kAuditsModule_Job_ProcessedData,			 	 	"Job_ProcessedData:" )
AUDIT_DATA( kAuditsModule_Job_End,				 				"Job_End:" )
AUDIT_DATA( kAuditsModule_testHword,			 				"Job_testHword: hword=U16" )
AUDIT_DATA( kAuditsModule_test1Dword,			 				"Job_test1Dword: hword=U16, dw0=U64" )
AUDIT_DATA( kAuditsModule_test2Dword,			 				"Job_test2Dword: hword=U16, dw0=U64, dw1=U64" )
AUDIT_DATA( kAuditsModule_test3Dword,			 				"Job_test3Dword: hword=U16, dw0=U64, dw1=U64, dw2=U64" )
AUDIT_DATA( kAuditsModule_test4Dword,			 				"Job_test4Dword: hword=U16, dw0=U64, dw1=U64, dw2=U64 \n\t    dw3=U64" )
AUDIT_DATA( kAuditsModule_test5Dword,			 				"Job_test5Dword: hword=U16, dw0=U64, dw1=U64, dw2=U64 \n\t    dw3=U64, dw4=U64" )
AUDIT_DATA( kAuditsModule_test6Dword,			 				"Job_test6Dword: hword=U16, dw0=U64, dw1=U64, dw2=U64 \n\t    dw3=U64, dw4=U64, dw5=U64" )
AUDIT_DATA( kAuditsModule_testNDword,			 				"Job_testNDword: data=dump" )

