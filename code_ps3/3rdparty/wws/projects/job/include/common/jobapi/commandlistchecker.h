/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Utility function for disassembling a command list and checking its validity
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_COMMAND_LIST_CHECKER_H
#define WWS_JOB_COMMAND_LIST_CHECKER_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>

//--------------------------------------------------------------------------------------------------

union JobHeader;
struct WwsJob_Command;

//--------------------------------------------------------------------------------------------------

enum WwsJob_CheckCommandListOptions
{
	//These options can be ORed together for passing in as the options field

	kNothing			= 0,
	kPrintCommands		= 1,
	kPrintWarnings		= 2,
	kPrintHints			= 4,
	kErrorCheckJob		= 8,	//If this isn't set, only run-command style errors are checked
};

//--------------------------------------------------------------------------------------------------

//Currently just prints the commands
void CheckJob( JobHeader jobHeader, U32 options, U32 numSpus = 0xFFFFFFFF );
void CheckCommandList( const WwsJob_Command* pCommandList, U32 numU64s, U32 options, U32 numSpus = 0xFFFFFFFF );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_COMMAND_LIST_CHECKER_H */
