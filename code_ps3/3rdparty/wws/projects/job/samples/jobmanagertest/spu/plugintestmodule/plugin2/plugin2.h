/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Prototype for plugin2, used by both the plugin and the job that spawns the plugin
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_PLUGIN2_H
#define WWS_JOB_PLUGIN2_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>

//--------------------------------------------------------------------------------------------------

typedef void (*ResetFuncPtr)( void );
typedef void (*SetParam1FuncPtr)( U32 param );
typedef void (*SetParam2FuncPtr)( U32 param );
typedef U32 (*CalculateProductFuncPtr)( void );

//--------------------------------------------------------------------------------------------------

struct Plugin2FunctionPtrTable
{
	ResetFuncPtr				m_ResetFuncPtr;
	SetParam1FuncPtr			m_SetParam1FuncPtr;
	SetParam2FuncPtr			m_SetParam2FuncPtr;
	CalculateProductFuncPtr		m_CalculateProductFuncPtr;
};

//--------------------------------------------------------------------------------------------------

typedef const Plugin2FunctionPtrTable* (*Plugin2EntryPointPtr)( void );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOBPLUGIN2_H */
