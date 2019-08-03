/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Basic test of an SPU plugin
**/
//--------------------------------------------------------------------------------------------------

#include <jobsystem/helpers.h>
#include <jobapi/jobprintf.h>

#include "plugin1.h"

//--------------------------------------------------------------------------------------------------

//The parameters and return value for "PluginStart" can be whatever you like
extern "C" const char* PluginStart( const char* stringFromModule );

//--------------------------------------------------------------------------------------------------

const char* PluginStart( const char* stringFromModule )
{
	WWSJOB_UNUSED( stringFromModule );
	JobPrintf( "***Plugin1: PluginEntryPoint Begin\n" );

	JobPrintf( "***Plugin1: string parameter passed to plugin 1: \"%s\"\n", stringFromModule );

	const char* pReturnString = "Hello from plugin 1 to module";

	JobPrintf( "***Plugin1: Done\n" );

	return pReturnString;
}

//--------------------------------------------------------------------------------------------------
