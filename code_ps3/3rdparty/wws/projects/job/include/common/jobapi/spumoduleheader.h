/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This is the format of the module header that is on all modules or plugins
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_SPU_MODULE_HEADER_H
#define WWS_JOB_SPU_MODULE_HEADER_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>

//--------------------------------------------------------------------------------------------------

struct SpuModuleHeader
{
	//The first qword contains "ila" instructions that ProDG spots in order to find the debug info for this module
	U32		m_ila0;					//This ila instruction contains a 16 bit random number to help with identifying code
	U32		m_ila1;					//This ila instruction contains a 16 bit random number to help with identifying code
	U32		m_ila2;					//This ila instruction contains a 16 bit random number to help with identifying code
	U32		m_ila3;					//This ila instruction contains a 16 bit random number to help with identifying code

	U32		m_entryOffset;			// offset to code entry (qword aligned)
	U32		m_bssOffset;			// offset to bss (qword aligned)
	U32		m_bssSize;				// bss size (qword aligned)
	U32		m_dataSize;

	U32		m_codeMarker;			//Should be the fixed value 0xCODECODE
	U32		m_ctorDtroListSize;
	U32		m_uploadAddr;
	U32		m_linkerScriptVersion;	//If the top bit is set, this is a plugin
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

enum
{
	kSpuModuleMarker = 0xC0DEC0DE,
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_SPU_MODULE_HEADER_H */
