/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Common header between PPU and SPU for defining audit communication structures
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_MANAGER_AUDIT_IDS_H
#define WWS_JOB_JOB_MANAGER_AUDIT_IDS_H

//--------------------------------------------------------------------------------------------------

namespace AuditId	// 13 bits of a 16 bit bitfield
{
	#define AUDIT_DATA( kEnumName, kString )        kEnumName , 
	enum
	{
		#include <jobapi/jobmanagerauditdata.inc>

		kWwsJobManager_end,					// end(exclusive) of JobManager auditId's
		// NOTE: JobManager reserved right to use more AuditId's in future.  There are 8192 available
	};
	#undef AUDIT_DATA 
}

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_MANAGER_AUDIT_IDS_H */
