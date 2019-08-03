/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_AUDITS_H
#define WWS_JOB_JOB_AUDITS_H

//--------------------------------------------------------------------------------------------------

#include <jobapi/jobmanagerauditids.h>

//--------------------------------------------------------------------------------------------------

namespace AuditId	// 13 bits of a 16 bit bitfield
{
	#define AUDIT_DATA( kEnumName, kString )        kEnumName , 
	enum
	{
		//Job audits start at kWwsJobManager_end
		kAuditsModuleBase = kWwsJobManager_end,

		#include "auditmoduleauditdata.inc"

		//This marks the end of the job audits
		kAuditsModuleEnd,
	};
	#undef AUDIT_DATA 
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_AUDITS_H */
