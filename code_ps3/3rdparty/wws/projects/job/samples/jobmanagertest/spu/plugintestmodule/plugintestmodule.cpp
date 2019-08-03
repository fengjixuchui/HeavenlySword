/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Test plugin usage
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "plugintestmodule.h"

#include "plugin1/plugin1.h"
#include "plugin2/plugin2.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	JobPrintf( "***Plugin Test Module: Begin\n" );

	WwsJob_BufferTag buffTag0	= WwsJob_JobApiGetBufferTag( 1, 0, WwsJob_kDontAllocDmaTag );
	WwsJob_BufferTag buffTag1	= WwsJob_JobApiGetBufferTag( 1, 1, WwsJob_kDontAllocDmaTag );

	//Read out parameters before starting next job loading
//	PluginTestModuleParams params;
//	WwsJob_JobApiCopyParams( &params, sizeof(params) );

	//Start next job loading
	WwsJob_JobApiLoadNextJob();

	//do all the actual work *after* we've started the next job loading


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Test plugin 1
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	//Get the pointer to our plugin's entry point
	Plugin1EntryPointPtr pPlugin1EntryPoint = (Plugin1EntryPointPtr) WwsJob_JobApiInitPlugin( buffTag0 );

	//Call the plugin and pass a parameter
	const char* pStringFromPlugin1 = pPlugin1EntryPoint( "Hello from module to plugin 1" );

	//Use the return value of the plugin
	JobPrintf( "***Plugin Test Module: string returned from plugin 1: \"%s\"\n", pStringFromPlugin1 );
	WWSJOB_UNUSED( pStringFromPlugin1 );

	WwsJob_JobApiShutdownPlugin( buffTag0 );


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Test plugin 2
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	//Get the pointer to our plugin's entry point
	Plugin2EntryPointPtr pPlugin2EntryPoint = (Plugin2EntryPointPtr) WwsJob_JobApiInitPlugin( buffTag1 );

	//Call the plugin and get the pointer to the function table
	const Plugin2FunctionPtrTable* pPlugin2 = pPlugin2EntryPoint(  );

	const U32 param1 = 7;
	const U32 param2 = 9;

	//Use the return value of the plugin
	pPlugin2->m_ResetFuncPtr();
	pPlugin2->m_SetParam1FuncPtr( param1 );
	pPlugin2->m_SetParam2FuncPtr( param2 );
	U32 product = pPlugin2->m_CalculateProductFuncPtr();

	JobPrintf( "***Plugin Test Module: plugin2 calculated the product of %d and %d to be %d\n", param1, param2, product );

	WWSJOB_UNUSED( product );
	WWSJOB_ASSERT( product == (param1 * param2) );

	WwsJob_JobApiShutdownPlugin( buffTag1 );

	JobPrintf( "***Plugin Test Module: Done\n" );
}

//--------------------------------------------------------------------------------------------------
