/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Manage usage of the job manager binary that is embedded into the PPU elf
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_EMBEDDED_JM_H
#define WWS_JOB_EMBEDDED_JM_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>

//--------------------------------------------------------------------------------------------------

class EmbeddedJobManager
{
public:
	static const void*			GetJmBase( void );
	static U32					GetJmSize( void );

	
	static const void*			GetBsBase( void );	//Only needed by the C version of the Job Manager
	static U32					GetBsSize( void );	//Only needed by the C version of the Job Manager
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_EMBEDDED_JM_H */
