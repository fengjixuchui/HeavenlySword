/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between the PPU and the SPU
				for the ppucallbackjob module.
				These are used for communicating data between the two.

	@note		The data layout must be consistent for PPU and SPU compiling.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_PPU_CALLBACK_JOB_H
#define WWS_JOB_PPU_CALLBACK_JOB_H

//--------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//The PPU will write out a params structure in this format for this job
//The SPU job code will expect to read in a params structure in this format.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct PpuCallbackModuleParams
{
	U32		m_port;
	U32		m_pad0[3];
} WWSJOB_ALIGNED(16);


//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_PPU_CALLBACK_JOB_H */
