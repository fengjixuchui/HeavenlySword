/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Define the enums and text strings for audits used by the timing/jobx sample
**/
//--------------------------------------------------------------------------------------------------

#ifndef AUDIT_DATA
#error This is not a standard header file for user inclusion
#endif

//--------------------------------------------------------------------------------------------------


AUDIT_DATA( kTimingJobx_begin,	 	 	"Jobx_begin: pad params=dump" )
AUDIT_DATA( kTimingJobx_codeBuffer,	 	"Jobx_codeBuffer: pad pad32 codeLsa=U32" )
AUDIT_DATA( kTimingJobx_dataBuffer,	 	"Jobx_dataBuffer: pad pad32 dataLsa=U32" )
AUDIT_DATA( kTimingJobx_end, 				"Jobx_end:" )

