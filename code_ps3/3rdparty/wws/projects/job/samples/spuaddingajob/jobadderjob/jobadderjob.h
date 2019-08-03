/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between the PPU and the SPU
				for the jobadderjob module.
				These are used for communicating data between the two.

	@note		The data layout must be consistent for PPU and SPU compiling.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_ADDER_JOB_H
#define WWS_JOB_JOB_ADDER_JOB_H

//--------------------------------------------------------------------------------------------------

#include <cell/spurs/types.h>

//--------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//The PPU will write out a params structure in this format for this job
//The SPU job code will expect to read in a params structure in this format.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct JobAdderJobModuleParams
{
	U32						m_eaNewJobBuffer;
	U32						m_pad0[3];

	U32						m_newJobBufferSize;
	U32						m_pad1[3];

	U32						m_eaSpuModule;
	U32						m_pad2[3];

	U32						m_spuModuleFileSize;
	U32						m_pad3[3];

	U32						m_spuModuleRequiredBufferSizeInPages;
	U32						m_pad4[3];

	U32						m_eaJobList;
	U32						m_pad5[3];

	CellSpursWorkloadId		m_workloadId;
	U32						m_pad6[3];
} WWSJOB_ALIGNED(16);

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_ADDER_JOB_H */
