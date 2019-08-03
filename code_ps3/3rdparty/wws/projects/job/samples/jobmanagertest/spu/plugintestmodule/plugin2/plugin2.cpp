/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Test of an SPU plugin with a bit more processing going on and global variables.
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobprintf.h>

#include "plugin2.h"

//--------------------------------------------------------------------------------------------------

//The parameters and return value for "PluginStart" can be whatever you like
extern "C" const Plugin2FunctionPtrTable* PluginStart( void );

//--------------------------------------------------------------------------------------------------

U32 g_param1;
U32 g_param2;

//--------------------------------------------------------------------------------------------------

void Reset( void )
{
	g_param1 = 0;
	g_param2 = 0;
}

//--------------------------------------------------------------------------------------------------

void SetParam1( U32 param )
{
	g_param1 = param;
}

//--------------------------------------------------------------------------------------------------

void SetParam2( U32 param )
{
	g_param2 = param;
}

//--------------------------------------------------------------------------------------------------

U32 CalculateProduct( void )
{
	return (g_param1 * g_param2);
}

//--------------------------------------------------------------------------------------------------

Plugin2FunctionPtrTable gPlugin2FunctionPtrTable;

//--------------------------------------------------------------------------------------------------

const Plugin2FunctionPtrTable* PluginStart( void )
{
	JobPrintf( "***Plugin2: PluginStart Begin\n" );

	JobPrintf( "***Plugin2: Setup function ptr table for plugin2\n" );
	//Fill in the func pointers at run-time (because this is position independent code
	//so we don't know the addresses until run-time
	gPlugin2FunctionPtrTable.m_ResetFuncPtr				= Reset;
	gPlugin2FunctionPtrTable.m_SetParam1FuncPtr			= SetParam1;
	gPlugin2FunctionPtrTable.m_SetParam2FuncPtr			= SetParam2;
	gPlugin2FunctionPtrTable.m_CalculateProductFuncPtr	= CalculateProduct;

	JobPrintf( "***Plugin2: Done\n" );

	//Return the pointer to the function table for the module to use
	return &gPlugin2FunctionPtrTable;
}

//--------------------------------------------------------------------------------------------------
