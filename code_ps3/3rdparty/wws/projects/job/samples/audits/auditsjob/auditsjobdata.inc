/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Define the enums and text strings for audits used by the audits/auditsjob sample
**/
//--------------------------------------------------------------------------------------------------

#ifndef AUDIT_DATA
#error This is not a standard header file for user inclusion
#endif

//--------------------------------------------------------------------------------------------------


AUDIT_DATA( kAuditsJob_test,				"test:" )
AUDIT_DATA( kAuditsJob_testHword,			"testHword: hword=U16" )
AUDIT_DATA( kAuditsJob_test1Dword,			"test1Dword: pad dw0=U64" )
AUDIT_DATA( kAuditsJob_testHword1Dword,		"test1HwordDword: hword=U16, dw0=U64" )
AUDIT_DATA( kAuditsJob_testHword2Dword,		"test2HwordDword: hword=U16, dw0=U64, dw1=U64" )
AUDIT_DATA( kAuditsJob_testHword3Dword,		"test3HwordDword: hword=U16, dw0=U64, dw1=U64, dw2=U64" )
AUDIT_DATA( kAuditsJob_testHword4Dword,		"test4HwordDword: hword=U16, dw0=U64, dw1=U64, dw2=U64 \n\t    dw3=U64" )
AUDIT_DATA( kAuditsJob_testHword5Dword,		"test5HwordDword: hword=U16, dw0=U64, dw1=U64, dw2=U64 \n\t    dw3=U64, dw4=U64" )
AUDIT_DATA( kAuditsJob_testHword6Dword,		"test6HwordDword: hword=U16, dw0=U64, dw1=U64, dw2=U64 \n\t    dw3=U64, dw4=U64, dw5=U64" )
AUDIT_DATA( kAuditsJob_testNDword,			"testNDword: data=dump" )
AUDIT_DATA( kAuditsJob_testSignedHword,		"testSignedHword: hword=I16" )
AUDIT_DATA( kAuditsJob_testHword1F32,		"testHword1F32: hword=U16, pad32 f32_0=F32" )
AUDIT_DATA( kAuditsJob_testHword2F32,		"testHword2F32: hword=U16, f32_0=F32, f32_1=F32" )
AUDIT_DATA( kAuditsJob_testPieces,			"testPieces: u1=U01, i2=I02, u3=U03, i4=I04, u5=U05 pad i6=I06, u7=U07, i8=I08, u9=U09, i10=I10" )

