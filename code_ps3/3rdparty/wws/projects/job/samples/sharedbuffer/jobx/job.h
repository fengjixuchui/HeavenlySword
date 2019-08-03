/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between the PPU and the SPU
				for the jobx module.
				These are used for communicating data between the two.

	@note		The data layout must be consistent for PPU and SPU compiling.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOBX_H
#define WWS_JOB_JOBX_H

//--------------------------------------------------------------------------------------------------

#include "../job3/job.h"

namespace AuditId	// 13 bits of a 16 bit bitfield
{
	#define AUDIT_DATA( kEnumName, kString )        kEnumName , 
	enum
	{
		//Job audits start at kWwsJobManager_end
		kSharedBufferJobx_auditsBegin = kSharedBufferJob3_auditsEnd + 1,

		#include "jobdata.inc" 

		//This marks the end of the job audits
		kSharedBufferJobx_auditsEnd,
	};
	#undef AUDIT_DATA 
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOBX_H */
