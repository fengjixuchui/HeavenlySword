/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Manage a cache of job headers in LS that have been dma-ed in from main memory
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_HEADER_CACHE_H
#define WWS_JOB_JOB_HEADER_CACHE_H

//--------------------------------------------------------------------------------------------------

union JobHeader;

//--------------------------------------------------------------------------------------------------

extern "C" JobHeader GetJobHeaderFromCache( U32 eaJobHeaderData );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_HEADER_CACHE_H */
