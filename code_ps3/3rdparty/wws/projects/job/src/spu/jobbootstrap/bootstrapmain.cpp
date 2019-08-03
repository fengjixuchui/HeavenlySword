/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		The bootstrapper is required for loading the job manager if it is over 16K
				This is not needed for the downcoded version
**/
//--------------------------------------------------------------------------------------------------

#include <cell/dma.h>

#include <jobbootstrap/bootstrapmain.h>
#include <jobapi/jobdefinition.h>
#include <jobmanager/jobmanagerdmatags.h>

//--------------------------------------------------------------------------------------------------

JobListHeader gJobListHeader WWSJOB_ALIGNED( 128 );

Bool32 g_bFirstRunOfBootStrap = true;

extern U32 _end[];

typedef void (*JobManagerEntryFunction)( uintptr_t spursContext, uint64_t eaWork ) __attribute__((noreturn));

//--------------------------------------------------------------------------------------------------

void LoadJobManager( uint64_t eaWork, void* pLoadToAddress )
{
	if ( g_bFirstRunOfBootStrap )
	{
		DMA_READ( &gJobListHeader, eaWork, sizeof(JobListHeader), DmaTagId::kBlockingLoad );
		cellDmaWaitTagStatusAll( 1 << DmaTagId::kBlockingLoad );

		//JobPrintf( "LoadJobManager: First bootstrap, so load from 0x%X (size = 0x%X)\n", gJobListHeader.m_mmaJobManager, gJobListHeader.m_jobManagerSize );
		//cellDmaLargeGet( pLoadToAddress, gJobListHeader.m_mmaJobManager, gJobListHeader.m_jobManagerSize, DmaTagId::kBlockingLoad, 0, 0 );
		U32 lsAddress	= (U32) pLoadToAddress;
		U32 mmAddress	= gJobListHeader.m_mmaJobManager;
		U32 mmLength	= gJobListHeader.m_jobManagerSize;
		do
		{
			// max of 16K per dma
			U32 length = (mmLength > 0x4000) ? 0x4000 : mmLength;

			// start dma of main memory into miscData.m_buffer
			DMA_READ( lsAddress, mmAddress, length, DmaTagId::kBlockingLoad )
			lsAddress += length;
			mmAddress += length;
			mmLength  -= length;
		} while( mmLength > 0 );

		cellDmaWaitTagStatusAll( 1 << DmaTagId::kBlockingLoad );

		g_bFirstRunOfBootStrap = false;
	}
	else
	{
		//JobManager has been bootstrapped before without bootstrapper being reloaded, therefore, don't need to reload it
		//JobPrintf( "LoadJobManager: Not reloading JobManager (should already be present)\n" );
	}
}

//--------------------------------------------------------------------------------------------------

void BootStrapMain( uintptr_t spursContext, uint64_t eaWork )
{
	//Comment this back in to get a breakpoint at the start of the boot loader
	//WWSJOB_BREAKPOINT();

	//JobPrintf( "******** BootStrapMain *********\n" );
	//JobPrintf( "BootStrapMain: eaWork = 0x%llX\n", eaWork );

	void* pLoadToAddress	= (void*)0x1000;	//This is the addr JobManager.pm is built at
	WWSJOB_ASSERT( pLoadToAddress >= _end );			//If this fires, change addr JobManager.pm is built at

	LoadJobManager( eaWork, pLoadToAddress );

	JobManagerEntryFunction entryFunction = (JobManagerEntryFunction) pLoadToAddress;
	entryFunction( spursContext, eaWork );
}
//--------------------------------------------------------------------------------------------------
