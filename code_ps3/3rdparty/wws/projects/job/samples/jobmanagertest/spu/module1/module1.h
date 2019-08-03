/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between both PPU and SPU
				for the module1 module.
				They are used for communicating data between the two.
**/
//--------------------------------------------------------------------------------------------------


#ifndef WWS_JOB_MODULE1_H
#define WWS_JOB_MODULE1_H

//--------------------------------------------------------------------------------------------------

struct Module1Params
{
	U32		m_eaOutputAddr;
	U32		m_pad0[3];

	U32		m_multiplier;
	U32		m_pad1[3];
} WWSJOB_ALIGNED(16);

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_MODULE1_H */
