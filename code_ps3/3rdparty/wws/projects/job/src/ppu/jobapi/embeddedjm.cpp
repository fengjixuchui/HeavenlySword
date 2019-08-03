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

#include <jobapi/embeddedjm.h>
#include <jobapi/jobdefinition.h>

//--------------------------------------------------------------------------------------------------

extern char _binary_jobmanager_file_start[];
extern char _binary_jobmanager_file_size;

extern char _binary_jobbootstrap_file_start[];
extern char _binary_jobbootstrap_file_size;

//--------------------------------------------------------------------------------------------------

const void* EmbeddedJobManager::GetJmBase( void )
{
	return (const void*)_binary_jobmanager_file_start;
}

//--------------------------------------------------------------------------------------------------

U32 EmbeddedJobManager::GetJmSize( void )
{
	U32 pmSize = (U32) &_binary_jobmanager_file_size;
	pmSize = (pmSize+127) & ~127;
	
	U32 JobManagerEnd = NumPagesToNumBytes( LsMemoryLimits::kJobAreaBasePageNum );
#if WWS_JOB_USE_C_VERSION==0
	U32 jobManagerBase = 0xA00;
#else
	U32 jobManagerBase = 0x1000;
#endif
	U32 maxJobManagerSize = JobManagerEnd - jobManagerBase;
	WWSJOB_ASSERT( pmSize <= maxJobManagerSize );
	WWSJOB_UNUSED( maxJobManagerSize );
	return pmSize;
}

//--------------------------------------------------------------------------------------------------

const void* EmbeddedJobManager::GetBsBase( void )
{
	return (const void*)_binary_jobbootstrap_file_start;
}

//--------------------------------------------------------------------------------------------------

U32 EmbeddedJobManager::GetBsSize( void )
{
	U32 pmSize = (U32) &_binary_jobbootstrap_file_size;
	pmSize = (pmSize+127) & ~127;
	return pmSize;
}

//--------------------------------------------------------------------------------------------------
