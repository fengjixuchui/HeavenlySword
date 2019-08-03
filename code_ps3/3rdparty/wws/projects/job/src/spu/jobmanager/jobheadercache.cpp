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

#include <cell/dma.h>
#include <cell/spurs/types.h>
#include <cell/spurs/policy_module.h>
#include <cell/spurs/ready_count.h>
#include <spu_mfcio.h>
#include <stddef.h>

#include <jobapi/jobdefinition.h>
#include <jobapi/audittypes.h>
#include <jobapi/jobapi.h>
#include <jobapi/spumoduleheader.h>
#include <jobapi/jobdefinition.h>
#include <jobmanager/interrupthandler.h>
#include <jobmanager/allocatejob.h>
#include <jobmanager/auditwriter.h>
#include <jobmanager/jobmanagerdmatags.h>
#include <jobmanager/spustack.h>
#include <jobmanager/jobheadercache.h>
#include <jobmanager/jobmanager.h>
#include <jobmanager/data.h>

#if WWS_JOB_USE_C_VERSION!=0
JobHeader GetJobHeaderFromCache( U32 eaJobHeaderData )
{
	U32 eaRequiredJobHeaderCacheLine = (eaJobHeaderData & ~0x7F);

	if  ( eaRequiredJobHeaderCacheLine != g_currentJobHeaderCacheEa )
	{
		//I don't like having a blocking load here.  Maybe we should look to optimise this in the future
		DMA_READ( &g_jobHeaderCache, eaRequiredJobHeaderCacheLine, 128, DmaTagId::kBlockingLoad );
		g_currentJobHeaderCacheEa = eaRequiredJobHeaderCacheLine;
		cellDmaWaitTagStatusAll( 1 << DmaTagId::kBlockingLoad );
	}

	//Add the offest into our job header cache
	const JobHeader* pJobHeader = (const JobHeader*) (U32(g_jobHeaderCache) | (eaJobHeaderData & 0x7F));

	return *pJobHeader;
}
#endif

//--------------------------------------------------------------------------------------------------
