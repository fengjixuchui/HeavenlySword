/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Define the enums and text strings for audits used by the timing/job1 sample
**/
//--------------------------------------------------------------------------------------------------

#ifndef AUDIT_DATA
#error This is not a standard header file for user inclusion
#endif

//--------------------------------------------------------------------------------------------------


AUDIT_DATA( kTimingJob1_begin,	 	 	"Job1_begin: pad params=dump" )
AUDIT_DATA( kTimingJob1_codeBuffer,	 	"Job1_codeBuffer: pad pad32 codeLsa=U32" )
AUDIT_DATA( kTimingJob1_dataBuffer,	 	"Job1_dataBuffer: pad pad32 dataLsa=U32" )
AUDIT_DATA( kTimingJob1_end, 				"Job1_end:" )

