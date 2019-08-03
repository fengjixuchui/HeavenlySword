/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Define the enums and text strings for audits used by the timing/job3 sample
**/
//--------------------------------------------------------------------------------------------------

#ifndef AUDIT_DATA
#error This is not a standard header file for user inclusion
#endif

//--------------------------------------------------------------------------------------------------


AUDIT_DATA( kTimingJob3_begin,	 	 	"Job3_begin: params=dump" )
AUDIT_DATA( kTimingJob3_codeBuffer,	 	"Job3_codeBuffer: pad pad32 codeLsa=U32" )
AUDIT_DATA( kTimingJob3_dataBuffer,	 	"Job3_dataBuffer: pad pad32 dataLsa=U32" )
AUDIT_DATA( kTimingJob3_end, 				"Job3_end:" )

