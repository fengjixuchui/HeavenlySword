/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Define the enums and text strings for audits used by the timing/job2 sample
**/
//--------------------------------------------------------------------------------------------------

#ifndef AUDIT_DATA
#error This is not a standard header file for user inclusion
#endif

//--------------------------------------------------------------------------------------------------


AUDIT_DATA( kTimingJob2_begin,	 	 	"Job2_begin:  params=dump" )
AUDIT_DATA( kTimingJob2_codeBuffer,	 	"Job2_codeBuffer: pad pad32 codsLsa=U32" )
AUDIT_DATA( kTimingJob2_dataBuffer,	 	"Job2_dataBuffer: pad pad32 dataLsa=U32" )
AUDIT_DATA( kTimingJob2_end, 				"Job2_end:" )

