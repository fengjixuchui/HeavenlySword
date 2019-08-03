/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between both PPU and SPU
				for the nonaccbuffermodule.
				They are used for communicating data between the two.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_NON_ACC_BUFFER_MODULE_H
#define WWS_JOB_NON_ACC_BUFFER_MODULE_H

//--------------------------------------------------------------------------------------------------

struct NonAccBufferModuleParams
{
	U32		m_clearVal;
	U32		m_pad[3];
} WWSJOB_ALIGNED(16);

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_NON_ACC_BUFFER_MODULE_H */
