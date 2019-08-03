/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the structures that are shared between both PPU and SPU
				for the fixedaddrmodule.
				They are used for communicating data between the two.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_FIXED_ADDR_MODULE_H
#define WWS_JOB_FIXED_ADDR_MODULE_H

//--------------------------------------------------------------------------------------------------

struct FixedAddrModuleParams
{
	U32		m_pad[4];
} WWSJOB_ALIGNED(16);

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_FIXED_ADDR_MODULE_H */
