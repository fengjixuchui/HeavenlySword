/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between the PPU and the SPU
				for the simple module.
				These are used for communicating data between the two.

	@note		The data layout must be consistent for PPU and SPU compiling.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_SIMPLE_MODULE_H
#define WWS_JOB_SIMPLE_MODULE_H

//--------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//The PPU will write out a params structure in this format for this job
//The SPU job code will expect to read in a params structure in this format.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct SimpleModuleParams
{
	U32		m_eaOutputAddr;
	U32		m_pad0[3];

	U32		m_outputBufferSize;
	U32		m_pad1[3];

	U32		m_multiplier;
	U32		m_pad2[3];
} WWSJOB_ALIGNED(16);

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_SIMPLE_MODULE_H */
