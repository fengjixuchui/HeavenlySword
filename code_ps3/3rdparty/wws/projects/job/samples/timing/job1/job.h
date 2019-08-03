/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between the PPU and the SPU
				for the job1 module.
				These are used for communicating data between the two.

	@note		The data layout must be consistent for PPU and SPU compiling.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB1_H
#define WWS_JOB_JOB1_H

//--------------------------------------------------------------------------------------------------

#include <jobapi/jobmanagerauditids.h>

namespace AuditId	// 13 bits of a 16 bit bitfield
{
	#define AUDIT_DATA( kEnumName, kString )        kEnumName , 
	enum
	{
		//Job audits start at kWwsJobManager_end
		kTimingJob1_auditsBegin = kWwsJobManager_end,

		#include "jobdata.inc" 

		//This marks the end of the job audits
		kTimingJob1_auditsEnd,
	};
	#undef AUDIT_DATA 
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB1_H */
