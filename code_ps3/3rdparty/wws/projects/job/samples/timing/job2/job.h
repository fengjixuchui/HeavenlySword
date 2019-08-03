/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between the PPU and the SPU
				for the job2 module.
				These are used for communicating data between the two.

	@note		The data layout must be consistent for PPU and SPU compiling.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB2_H
#define WWS_JOB_JOB2_H

//--------------------------------------------------------------------------------------------------

#include "../job1/job.h"

namespace AuditId	// 13 bits of a 16 bit bitfield
{
	#define AUDIT_DATA( kEnumName, kString )        kEnumName , 
	enum
	{
		//Job audits start at kWwsJobManager_end
		kTimingJob2_auditsBegin = kTimingJob1_auditsEnd + 1,

		#include "jobdata.inc" 

		//This marks the end of the job audits
		kTimingJob2_auditsEnd,
	};
	#undef AUDIT_DATA 
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB2_H */
