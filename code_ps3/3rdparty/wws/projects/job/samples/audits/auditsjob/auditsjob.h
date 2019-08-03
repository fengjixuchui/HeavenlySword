/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between the PPU and the SPU
				for the auditsjob module.
				These are used for communicating data between the two.

	@note		The data layout must be consistent for PPU and SPU compiling.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_AUDITS_JOB_H
#define WWS_JOB_AUDITS_JOB_H

//--------------------------------------------------------------------------------------------------

namespace AuditId	// 13 bits of a 16 bit bitfield
{
	#define AUDIT_DATA( kEnumName, kString )        kEnumName , 
	enum
	{
		#include "auditsjobdata.inc" 

		kAuditsJob_numAudits,
	};
	#undef AUDIT_DATA 
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_AUDITS_JOB_H */
