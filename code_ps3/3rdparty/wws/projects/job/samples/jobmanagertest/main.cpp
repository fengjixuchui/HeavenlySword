/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		The core of our test application for the job manager
**/
//--------------------------------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>

#include <sys/spu_initialize.h>
#include <sys/ppu_thread.h>

#include <cell/sysmodule.h>
#include <cell/spurs/types.h>
#include <cell/spurs/control.h>
#include <sys/timer.h>

#include <jobapi/joblist.h>
#include <jobapi/spumodule.h>
#include <jobapi/commandlistbuilder.h>
#include <jobapi/auditmanager.h>
#include <jobapi/eventhandler.h>
#include <jobapi/jobdependency.h>

#include "spu/module1/module1.h"
#include "spu/module2/module2.h"
#include "spu/dmalistmodule/dmalistmodule.h"
#include "spu/plugintestmodule/plugintestmodule.h"

#include "spu/accbuff/createaccbuff/createaccbuff.h"
#include "spu/accbuff/fillaccbuff/fillaccbuff.h"
#include "spu/accbuff/writeaccbuff/writeaccbuff.h"
#include "spu/accbuff/nonaccbuff/nonaccbuff.h"

#include "spu/auditsmodule/auditsmodule.h"
#include "spu/auditsmodule/jobaudits.h"

#include "spu/fixedaddrmodule/fixedaddrmodule.h"

#include "dmalist.h"

//--------------------------------------------------------------------------------------------------

//Some handy options to choose which jobs we want
#define ADD_THREE_MODULE1_JOBS
#define ADD_THREE_MODULE2_JOBS
#define ADD_AUDIT_TEST_JOB
#define ADD_DMA_LIST_JOB
#define ADD_PLUGIN_JOB
#define ADD_ACCUMULATE_JOBS
#define ADD_TWO_DEPENDENENT_JOBS
#define ADD_HIGH_PRIO_JOB
#define ADD_FIXED_ADDR_JOB

//--------------------------------------------------------------------------------------------------

enum
{
	kLogicalBuffer0 = 0,
	kLogicalBuffer1 = 1,
	kLogicalBuffer2 = 2,
	kLogicalBuffer3 = 3,
};

//--------------------------------------------------------------------------------------------------

#define AUDIT_DATA( kEnumName, kString )        kString ,
const char* const g_auditModuleAuditText[] =
{
	"kAuditsModuleBase",
	#include "spu/auditsmodule/auditmoduleauditdata.inc"
	"kAuditsModuleEnd",
};
#undef AUDIT_DATA

//--------------------------------------------------------------------------------------------------

CellSpurs g_spurs1;			//Spurs object 1
CellSpurs g_spurs2;			//Spurs object 2

//--------------------------------------------------------------------------------------------------

JobHeader CreateAccumulationBufferInitJob(	CommandListBuilder& rCommandListBuilder,
											const SpuModuleHandle& createAccBufferModule,
											const SpuModuleHandle& fillAccBufferModule,
											const SpuModuleHandle& writeAccBufferModule,
											const SpuModuleHandle& nonAccBufferModule,
											void* pAccBuffer,
											U32 bufferSize	);
JobHeader CreateAccumulationBufferCompatibleJob(	CommandListBuilder& rCommandListBuilder,
													const SpuModuleHandle& createAccBufferModule,
													const SpuModuleHandle& fillAccBufferModule,
													const SpuModuleHandle& writeAccBufferModule,
													const SpuModuleHandle& nonAccBufferModule,
													void* pAccBuffer,
													U32 bufferSize	);
JobHeader CreateAccumulationBufferIncompatibleJob(	CommandListBuilder& rCommandListBuilder,
													const SpuModuleHandle& createAccBufferModule,
													const SpuModuleHandle& fillAccBufferModule,
													const SpuModuleHandle& writeAccBufferModule,
													const SpuModuleHandle& nonAccBufferModule,
													void* pAccBuffer,
													U32 bufferSize	);
JobHeader CreateAccumulationBufferFillerJob(	CommandListBuilder& rCommandListBuilder,
												const SpuModuleHandle& createAccBufferModule,
												const SpuModuleHandle& fillAccBufferModule,
												const SpuModuleHandle& writeAccBufferModule,
												const SpuModuleHandle& nonAccBufferModule,
												void* pAccBuffer,
												U32 bufferSize	);
JobHeader CreateAccumulationBufferWriteOutJob(	CommandListBuilder& rCommandListBuilder,
												const SpuModuleHandle& createAccBufferModule,
												const SpuModuleHandle& fillAccBufferModule,
												const SpuModuleHandle& writeAccBufferModule,
												const SpuModuleHandle& nonAccBufferModule,
												void* pAccBuffer,
												U32 bufferSize	);

//--------------------------------------------------------------------------------------------------

JobHeader CreateModule1Job( CommandListBuilder& rCommandListBuilder, const SpuModuleHandle& spuModuleHandle, void* pInputBuffer, void* outputBuffer, U32 bufferSize, U32 multiplier )
{
	//one code buffer
	const U32 kCodeBufferSet			= 0;
	const U32 kNumCodeBuffers			= 2;
	const U32 kCodeBufferSizeInPages	= spuModuleHandle.GetRequiredBufferSizeInPages();
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by 4 4K data buffers
	const U32 kDataBufferSet			= 1;
	const U32 kNumDataBuffers			= 4;
	const U32 kDataBufferSizeInPages	= 4;
	const U32 kDataBufferSetBasePageNum	= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//The input parameters to the job (in the memory layout they'll be expected in)
	Module1Params module1Params;
	module1Params.m_eaOutputAddr		= (U32) outputBuffer;
	module1Params.m_multiplier			= multiplier;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuModuleHandle, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UnreserveBufferSets( 1 << kCodeBufferSet ); // moved this up since we won't unreserve data bufferSet here
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
  #if 0 // 0 normally.  1 to test writeIfDiscard bit
	rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pInputBuffer, bufferSize, WwsJob_Command::kReadWriteCached );
  #else
	rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pInputBuffer, bufferSize, WwsJob_Command::kNonCached );
  #endif
	// note the job will unreserve the dataBufferSet
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &module1Params, sizeof(module1Params) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateModule2Job( CommandListBuilder& rCommandListBuilder, const SpuModuleHandle& spuModuleHandle,
		const void* pInputBuffer, void* outputBuffer, U32 bufferSize, U32 multiplier )
{
	//one code buffer
	const U32 kCodeBufferSet			= 0;
	const U32 kNumCodeBuffers			= 2;
	const U32 kCodeBufferSizeInPages	= spuModuleHandle.GetRequiredBufferSizeInPages();
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by 4 4K data buffers
	const U32 kDataBufferSet			= 1;
	const U32 kNumDataBuffers			= 4;
	const U32 kDataBufferSizeInPages	= 4;
	const U32 kDataBufferSetBasePageNum	= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//The input parameters to the job (in the memory layout they'll be expected in)
	Module2Params module2Params;
	module2Params.m_eaOutputAddr		= (U32) outputBuffer;
	module2Params.m_multiplier			= multiplier;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuModuleHandle, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UnreserveBufferSets( 1 << kCodeBufferSet );
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pInputBuffer, bufferSize, WwsJob_Command::kNonCached );
	// note the job will unreserve the dataBufferSet
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &module2Params, sizeof(module2Params) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateAuditsModuleJob( CommandListBuilder& rCommandListBuilder, const SpuModuleHandle& spuModuleHandle, const void* pInputBuffer, void* outputBuffer, U32 bufferSize, U32 multiplier )
{
	//one code buffer
	const U32 kCodeBufferSet			= 0;
	const U32 kNumCodeBuffers			= 2;
	const U32 kCodeBufferSizeInPages	= spuModuleHandle.GetRequiredBufferSizeInPages();
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by 4 4K data buffers
	const U32 kDataBufferSet			= 1;
	const U32 kNumDataBuffers			= 4;
	const U32 kDataBufferSizeInPages	= 4;
	const U32 kDataBufferSetBasePageNum	= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//The input parameters to the job (in the memory layout they'll be expected in)
	AuditsModuleParams auditsModuleParams;
	auditsModuleParams.m_eaOutputAddr		= (U32) outputBuffer;
	auditsModuleParams.m_multiplier			= multiplier;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuModuleHandle, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pInputBuffer, bufferSize, WwsJob_Command::kNonCached );
	// to test code only, the bufferSets won't be unreserved, so that the job manager has to do it when the job is done
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &auditsModuleParams, sizeof(auditsModuleParams) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateDmaListModuleJob( CommandListBuilder& rCommandListBuilder, const SpuModuleHandle& spuModuleHandle, const void* dmaListEffAddr, U32 listSize, void* outputBuffer, U32 multiplier )
{
	//one code buffer
	const U32 kCodeBufferSet			= 0;
	const U32 kNumCodeBuffers			= 2;
	const U32 kCodeBufferSizeInPages	= spuModuleHandle.GetRequiredBufferSizeInPages();
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by 4 4K data buffers
	const U32 kDataBufferSet			= 1;
	const U32 kNumDataBuffers			= 4;
	const U32 kDataBufferSizeInPages	= 4;
	const U32 kDataBufferSetBasePageNum	= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//The input parameters to the job (in the memory layout they'll be expected in)
	DmaListModuleParams dmaListModuleParams;
	dmaListModuleParams.m_eaOutputAddr	= (U32) outputBuffer;
	dmaListModuleParams.m_multiplier	= multiplier;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuModuleHandle, WwsJob_Command::kReadOnlyCached );
	// note the dmalist is loaded at the *bottom* of the buffer and the data it fetches loads into the *same*
	// buffer so that the dma list is normally overwritten.  You may specify a seperation of 0(default):7 qwords between the
	// bottom of the listelements and the bottom of the buffer in case you want the source-destination addresses
	// to be better matched for speed of dma transfer of the dma list elements.  The optional value 0 below is
	// this seperation, which in this case (the dma list fills the entire buffer) must be 0.
	rCommandListBuilder.UseInputDmaListBuffer( kDataBufferSet, kLogicalBuffer0, dmaListEffAddr, listSize, WwsJob_Command::kNonCached, 0/*optional*/ );
	rCommandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kDataBufferSet) );
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &dmaListModuleParams, sizeof(dmaListModuleParams) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreatePluginTestJob( CommandListBuilder& rCommandListBuilder, const SpuModuleHandle& spuModuleHandle, const SpuPluginHandle& spuPlugin1Handle, const SpuPluginHandle& spuPlugin2Handle )
{
	//one code buffer
	const U32 kCodeBufferSet			= 0;
	const U32 kNumCodeBuffers			= 2;
	const U32 kCodeBufferSizeInPages	= spuModuleHandle.GetRequiredBufferSizeInPages();
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by 4 4K data buffers
	const U32 kPluginBufferSet			= 1;
	const U32 kNumPluginBuffers			= 4;
	const U32 kPluginBufferSizeInPages	= 4;
	const U32 kPluginBufferSetBasePageNum	= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//The input parameters to the job (in the memory layout they'll be expected in)
//	PluginTestModuleParams pluginTestModuleParams;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kPluginBufferSet, kNumPluginBuffers, kPluginBufferSetBasePageNum, kPluginBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuModuleHandle, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UseInputBuffer( kPluginBufferSet, kLogicalBuffer0, spuPlugin1Handle, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UseInputBuffer( kPluginBufferSet, kLogicalBuffer1, spuPlugin2Handle, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kPluginBufferSet) );
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
//	rCommandListBuilder.AddParams( &pluginTestModuleParams, sizeof(pluginTestModuleParams) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

//NOTE: if you enable this code, then you should add calls to UnreserveBufferSets accordingly !!!
/*JobListMarker CreateAndAddAccumulationBufferTestJobs(
											CommandListBuilder& rCommandListBuilder,
											SingleThreadJobList& rJobList,
											const SpuModuleHandle& createAccBufferModule,
											const SpuModuleHandle& fillAccBufferModule,
											const SpuModuleHandle& writeAccBufferModule,
											const SpuModuleHandle& nonAccBufferModule,
											void* pAccBuffer,
											U32 bufferSize
											)
{
	//Note that these jobs *MUST* be put on a list that is *ONLY* going to be accessed by *ONE* SPU

	const bool bAddCompatibleJobBetweenCreateAndFill	= false;
	const bool bAddIncompatibleJobBetweenCreateAndFill	= true;
	const bool bAddCompatibleJobBetweenFillAndWrite		= false;
	const bool bAddIncompatibleJobBetweenFillAndWrite	= true;

	//two code buffers
	const U32 kCodeBufferSet							= 0;
	const U32 kNumCodeBuffers							= 2;
	const U32 kCodeBufferSizeInPages					= WwsJob_max(	WwsJob_max( createAccBufferModule.GetRequiredBufferSizeInPages(),
																		fillAccBufferModule.GetRequiredBufferSizeInPages() ),
																	WwsJob_max( writeAccBufferModule.GetRequiredBufferSizeInPages(),
																		nonAccBufferModule.GetRequiredBufferSizeInPages() ) );
	const U32 kCodeBufferSetBasePageNum					= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by one 16K data buffers
	const U32 kDataBufferSet							= 1;
	const U32 kNumDataBuffers							= 1;
	const U32 kDataBufferSizeInPages					= (bufferSize / 1024);
	const U32 kDataBufferSetBasePageNum					= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//This memory map is used for a compatible buffer set
	//It sets up buffers after the accumulation buffer so shouldn't cause a flush
	const U32 kCompatibleDataBufferSet					= 2;
	const U32 kNumCompatibleDataBuffers					= 4;
	const U32 kCompatibleDataBufferSizeInPages			= 4;
	const U32 kCompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum + kNumDataBuffers*kDataBufferSizeInPages;

	//This memory map is used for an incompatible buffer set
	//It sets up buffers that clash with the accumulation buffer so cause it to be flush
	const U32 kIncompatibleDataBufferSet				= 2;
	const U32 kNumIncompatibleDataBuffers				= 1;
	const U32 kIncompatibleDataBufferSizeInPages		= 4;
	const U32 kIncompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//The first job creates the accumulation buffer in LS
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	{
		//The input parameters to the job (in the memory layout they'll be expected in)
		CreateAccBufferModuleParams createAccBufferModuleParams;
		createAccBufferModuleParams.m_initValue		= 2;

		//Create the job
		rCommandListBuilder.InitializeJob();
		rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
		rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
		rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, createAccBufferModule, WwsJob_Command::kReadOnlyCached );
		rCommandListBuilder.UseUninitializedBuffer( kDataBufferSet, kLogicalBuffer0, pAccBuffer, bufferSize, WwsJob_Command::kReadWriteCached );
		rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
		rCommandListBuilder.AddParams( &createAccBufferModuleParams, sizeof(createAccBufferModuleParams) );
		JobHeader jobHeader = rCommandListBuilder.FinalizeJob();

		//And add it to the job list
		rJobList.AddJob( jobHeader );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//This job is compatible with the accumulation buffer memory map so
	//doesn't cause a flush
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	if ( bAddCompatibleJobBetweenCreateAndFill )
	{
		//The input parameters to the job (in the memory layout they'll be expected in)
		NonAccBufferModuleParams nonAccBufferModuleParams;
		nonAccBufferModuleParams.m_clearVal		= 0xCC;

		//Create the job
		rCommandListBuilder.InitializeJob();
		rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
		rCommandListBuilder.ReserveBufferSet( kCompatibleDataBufferSet, kNumCompatibleDataBuffers, kCompatibleDataBufferSetBasePageNum, kCompatibleDataBufferSizeInPages );
		rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, nonAccBufferModule, WwsJob_Command::kReadOnlyCached );
		rCommandListBuilder.UseInputBuffer( kCompatibleDataBufferSet, kLogicalBuffer0, pAccBuffer, NumPagesToNumBytes( kCompatibleDataBufferSizeInPages ), WwsJob_Command::kNonCached );
		rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
		rCommandListBuilder.AddParams( &nonAccBufferModuleParams, sizeof(nonAccBufferModuleParams) );
		JobHeader jobHeader = rCommandListBuilder.FinalizeJob();

		//And add it to the job list
		rJobList.AddJob( jobHeader );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//This job is incompatible with the accumulation buffer memory map so
	//causes a flush
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	if ( bAddIncompatibleJobBetweenCreateAndFill )
	{
		//The input parameters to the job (in the memory layout they'll be expected in)
		NonAccBufferModuleParams nonAccBufferModuleParams;
		nonAccBufferModuleParams.m_clearVal		= 0xCC;

		//Create the job
		rCommandListBuilder.InitializeJob();
		rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
		rCommandListBuilder.ReserveBufferSet( kIncompatibleDataBufferSet, kNumIncompatibleDataBuffers, kIncompatibleDataBufferSetBasePageNum, kIncompatibleDataBufferSizeInPages );
		rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, nonAccBufferModule, WwsJob_Command::kReadOnlyCached );
		rCommandListBuilder.UseInputBuffer( kIncompatibleDataBufferSet, kLogicalBuffer0, pAccBuffer, NumPagesToNumBytes( kIncompatibleDataBufferSizeInPages ), WwsJob_Command::kNonCached );
		rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
		rCommandListBuilder.AddParams( &nonAccBufferModuleParams, sizeof(nonAccBufferModuleParams) );
		JobHeader jobHeader = rCommandListBuilder.FinalizeJob();

		//And add it to the job list
		rJobList.AddJob( jobHeader );
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//The following job(s) add to the accumulation buffer in LS
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	{
		//The input parameters to the job (in the memory layout they'll be expected in)
		FillAccBufferModuleParams fillAccBufferModuleParams;
		fillAccBufferModuleParams.m_multiplier		= 17;

		//Create the job
		rCommandListBuilder.InitializeJob();
		rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
		rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
		rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, fillAccBufferModule, WwsJob_Command::kReadOnlyCached );
		rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pAccBuffer, bufferSize, WwsJob_Command::kReadWriteCached );
		rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
		rCommandListBuilder.AddParams( &fillAccBufferModuleParams, sizeof(fillAccBufferModuleParams) );
		JobHeader jobHeader = rCommandListBuilder.FinalizeJob();

		//And add it to the job list
		rJobList.AddJob( jobHeader );
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//This job is compatible with the accumulation buffer memory map so
	//doesn't cause a flush
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	if ( bAddCompatibleJobBetweenFillAndWrite )
	{
		//The input parameters to the job (in the memory layout they'll be expected in)
		NonAccBufferModuleParams nonAccBufferModuleParams;
		nonAccBufferModuleParams.m_clearVal		= 0xCC;

		//Create the job
		rCommandListBuilder.InitializeJob();
		rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
		rCommandListBuilder.ReserveBufferSet( kCompatibleDataBufferSet, kNumCompatibleDataBuffers, kCompatibleDataBufferSetBasePageNum, kCompatibleDataBufferSizeInPages );
		rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, nonAccBufferModule, WwsJob_Command::kReadOnlyCached );
		rCommandListBuilder.UseInputBuffer( kCompatibleDataBufferSet, kLogicalBuffer0, pAccBuffer, NumPagesToNumBytes( kCompatibleDataBufferSizeInPages ), WwsJob_Command::kNonCached );
		rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
		rCommandListBuilder.AddParams( &nonAccBufferModuleParams, sizeof(nonAccBufferModuleParams) );
		JobHeader jobHeader = rCommandListBuilder.FinalizeJob();

		//And add it to the job list
		rJobList.AddJob( jobHeader );
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//This job is incompatible with the accumulation buffer memory map so
	//causes a flush
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	if ( bAddIncompatibleJobBetweenFillAndWrite )
	{
		//The input parameters to the job (in the memory layout they'll be expected in)
		NonAccBufferModuleParams nonAccBufferModuleParams;
		nonAccBufferModuleParams.m_clearVal		= 0xCC;

		//Create the job
		rCommandListBuilder.InitializeJob();
		rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
		rCommandListBuilder.ReserveBufferSet( kIncompatibleDataBufferSet, kNumIncompatibleDataBuffers, kIncompatibleDataBufferSetBasePageNum, kIncompatibleDataBufferSizeInPages );
		rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, nonAccBufferModule, WwsJob_Command::kReadOnlyCached );
		rCommandListBuilder.UseInputBuffer( kIncompatibleDataBufferSet, kLogicalBuffer0, pAccBuffer, NumPagesToNumBytes( kIncompatibleDataBufferSizeInPages ), WwsJob_Command::kNonCached );
		rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
		rCommandListBuilder.AddParams( &nonAccBufferModuleParams, sizeof(nonAccBufferModuleParams) );
		JobHeader jobHeader = rCommandListBuilder.FinalizeJob();

		//And add it to the job list
		rJobList.AddJob( jobHeader );
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//The final job finalises the accumulation buffer in LS and flushes it
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	{
		//The input parameters to the job (in the memory layout they'll be expected in)
		WriteAccBufferModuleParams writeAccBufferModuleParams;
		writeAccBufferModuleParams.m_addValue		= 3;

		//Create the job
		rCommandListBuilder.InitializeJob();
		rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
		rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
		rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, writeAccBufferModule, WwsJob_Command::kReadOnlyCached );
		rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pAccBuffer, bufferSize, WwsJob_Command::kReadWriteCached );
		rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
		rCommandListBuilder.AddParams( &writeAccBufferModuleParams, sizeof(writeAccBufferModuleParams) );
		JobHeader jobHeader = rCommandListBuilder.FinalizeJob();

		//And add it to the job list
		return rJobList.AddJob( jobHeader );
	}
}*/

//--------------------------------------------------------------------------------------------------

void CreateTwoDependentJobs( JobHeader* pDepDecJob, JobHeader* pPostDepJob, CommandListBuilder& rCommandListBuilder, const SpuModuleHandle& spuModuleHandle, void* pInputBuffer, void* pOutputBuffer, U32 bufferSize, U32 firstMultiplier, U32 secondMultiplier, DependencyCounter* pDependencyCounter )
{
	//one code buffer
	const U32 kCodeBufferSet			= 0;
	const U32 kNumCodeBuffers			= 2;
	const U32 kCodeBufferSizeInPages	= spuModuleHandle.GetRequiredBufferSizeInPages();
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by 4 4K data buffers
	const U32 kDataBufferSet			= 1;
	const U32 kNumDataBuffers			= 4;
	const U32 kDataBufferSizeInPages	= 4;
	const U32 kDataBufferSetBasePageNum	= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//The input parameters to the job (in the memory layout they'll be expected in)
	Module1Params module1ParamsFirstJob;
	module1ParamsFirstJob.m_eaOutputAddr		= (U32) pOutputBuffer;
	module1ParamsFirstJob.m_multiplier			= firstMultiplier;

	//Create the first job - This one sends an output buffer to main memory
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuModuleHandle, WwsJob_Command::kReadOnlyCached );
 	rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pInputBuffer, bufferSize, WwsJob_Command::kNonCached );
	rCommandListBuilder.RequestDependencyDecrement( pDependencyCounter );
	rCommandListBuilder.UnreserveBufferSets( 1 << kCodeBufferSet );
	// note the job will unreserve the dataBufferSet
 	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &module1ParamsFirstJob, sizeof(module1ParamsFirstJob) );
	*pDepDecJob = rCommandListBuilder.FinalizeJob();


	Module1Params module1ParamsSecondJob;
	module1ParamsSecondJob.m_eaOutputAddr		= (U32) pOutputBuffer;
	module1ParamsSecondJob.m_multiplier			= secondMultiplier;

	//Create the second job - This one reads in the output buffer that came form the first job, modifies it and sends it back
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuModuleHandle, WwsJob_Command::kReadOnlyCached );
 	rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pOutputBuffer, bufferSize, WwsJob_Command::kNonCached );
	rCommandListBuilder.UnreserveBufferSets( 1 << kCodeBufferSet );
	// note the job will unreserve the dataBufferSet
 	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &module1ParamsSecondJob, sizeof(module1ParamsSecondJob) );
	*pPostDepJob = rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateFixedAddrModuleJob( CommandListBuilder& rCommandListBuilder, const SpuModuleHandle& spuModuleHandle )
{
	//one code buffer
	const U32 kCodeBufferSet			= 0;
	const U32 kNumCodeBuffers			= 1;	//Note: This job has to go to a fixed address so must be in a single code buffer
	const U32 kCodeBufferSizeInPages	= spuModuleHandle.GetRequiredBufferSizeInPages();

	// Note: for this job only, I will force it to compile & run at a different adrs entirely				 
	// This equate of 64 (below, in 1K pages) *MUST* match the value of SPU_UPLOAD_ADDRESS in fixedaddrmodule/makefile 
	const U32 kCodeBufferSetBasePageNum	= 64;	//LsMemoryLimits::kJobAreaBasePageNum;	//The job has been compiled to require loading to this address

	//The input parameters to the job (in the memory layout they'll be expected in)
	//FixedAddrModuleParams fixedAddrModuleParams;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuModuleHandle, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UnreserveBufferSets( 1 << kCodeBufferSet );
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

void CheckResults( const char* outputName, U32 numElts, const U8* pOutput, U8 expectedValue )
{
	WWSJOB_UNUSED( outputName );
	WWSJOB_UNUSED( pOutput );
	WWSJOB_UNUSED( expectedValue );

	for (U32 i = 0; i < numElts; ++i)
	{
		//printf( "%s[%d] = 0x%X\t\t\t\tShould be 0x%X\n", outputName, i, pOutput[i], expectedValue );
		WWSJOB_ASSERT_MSG( pOutput[i] == expectedValue, ( "ERROR: %s[%d] != %d\t\t\t0x%X != 0x%X \n", outputName, i, expectedValue, pOutput[i], expectedValue ) );
	}
}

//--------------------------------------------------------------------------------------------------

static int GetPpuThreadPriority( void )
{
	sys_ppu_thread_t thisPpuThreadId;
	int ret;
	WWSJOB_UNUSED( ret );

	ret = sys_ppu_thread_get_id( &thisPpuThreadId );
	WWSJOB_ASSERT( CELL_OK == ret );

	int thisPpuThreadPriority;
	ret = sys_ppu_thread_get_priority( thisPpuThreadId, &thisPpuThreadPriority );
	WWSJOB_ASSERT( CELL_OK == ret );

	return thisPpuThreadPriority;
}

//--------------------------------------------------------------------------------------------------

int main()
{
	const U32 kNumSpus1		= 1;
	const U32 kNumSpus2		= 1;

	//Initialize the SPUs
	int ret = sys_spu_initialize( kNumSpus1+kNumSpus2, 0 );
	WWSJOB_ASSERT( CELL_OK == ret );

	//Initialize Spurs
	int ppuThreadPrio	= GetPpuThreadPriority() - 1;	//Higher priority than main thread
	int spuThgrpPrio	= 128;
	int isExit			= 0;	//*Must* be false

	if ( kNumSpus1 )
	{
		ret = cellSpursInitialize( &g_spurs1, kNumSpus1, spuThgrpPrio, ppuThreadPrio, isExit );
		WWSJOB_ASSERT( CELL_OK == ret );
		printf( "Spurs1 initialized (spurs=0x%p)\n", &g_spurs1 );
	}
	if ( kNumSpus2 )
	{
		ret = cellSpursInitialize( &g_spurs2, kNumSpus2, spuThgrpPrio, ppuThreadPrio, isExit );
		WWSJOB_ASSERT( CELL_OK == ret );
		printf( "Spurs2 initialized (spurs=0x%p)\n", &g_spurs2 );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Get the handles for our job modules and plugins
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern char _binary_module1_file_start[];
	extern char _binary_module1_file_size;
	SpuModuleHandle spuModule1Handle ( _binary_module1_file_start, (U32)&_binary_module1_file_size, "module1.spu.mod" );

	extern char _binary_module2_file_start[];
	extern char _binary_module2_file_size;
	SpuModuleHandle spuModule2Handle ( _binary_module2_file_start, (U32)&_binary_module2_file_size, "module2.spu.mod" );

	extern char _binary_dmalistmodule_file_start[];
	extern char _binary_dmalistmodule_file_size;
	SpuModuleHandle spuDmaListModuleHandle ( _binary_dmalistmodule_file_start, (U32)&_binary_dmalistmodule_file_size, "dmalistmodule.spu.mod" );

	extern char _binary_plugintestmodule_file_start[];
	extern char _binary_plugintestmodule_file_size;
	SpuModuleHandle spuPluginTestModuleHandle ( _binary_plugintestmodule_file_start, (U32)&_binary_plugintestmodule_file_size, "plugintestmodule.spu.mod" );

	//And get our plugins
	extern char _binary_plugin1_file_start[];
	extern char _binary_plugin1_file_size;
	SpuPluginHandle spuPlugin1Handle ( _binary_plugin1_file_start, (U32)&_binary_plugin1_file_size, "plugin1.spu.plugin" );
	extern char _binary_plugin2_file_start[];
	extern char _binary_plugin2_file_size;
	SpuPluginHandle spuPlugin2Handle ( _binary_plugin2_file_start, (U32)&_binary_plugin2_file_size, "plugin2.spu.plugin" );

	extern char _binary_createaccbuffermodule_file_start[];
	extern char _binary_createaccbuffermodule_file_size;
	SpuModuleHandle createAccBufferModule ( _binary_createaccbuffermodule_file_start, (U32)&_binary_createaccbuffermodule_file_size, "createaccbuffermodule.spu.mod" );

	extern char _binary_fillaccbuffermodule_file_start[];
	extern char _binary_fillaccbuffermodule_file_size;
	SpuModuleHandle fillAccBufferModule ( _binary_fillaccbuffermodule_file_start, (U32)&_binary_fillaccbuffermodule_file_size, "fillaccbuffermodule.spu.mod" );

	extern char _binary_writeaccbuffermodule_file_start[];
	extern char _binary_writeaccbuffermodule_file_size;
	SpuModuleHandle writeAccBufferModule ( _binary_writeaccbuffermodule_file_start, (U32)&_binary_writeaccbuffermodule_file_size, "writeaccbuffermodule.spu.mod" );

	extern char _binary_nonaccbuffermodule_file_start[];
	extern char _binary_nonaccbuffermodule_file_size;
	SpuModuleHandle nonAccBufferModule ( _binary_nonaccbuffermodule_file_start, (U32)&_binary_nonaccbuffermodule_file_size, "nonaccbuffermodule.spu.mod" );

	extern char _binary_auditsmodule_file_start[];
	extern char _binary_auditsmodule_file_size;
	SpuModuleHandle spuAuditsModuleHandle ( _binary_auditsmodule_file_start, (U32)&_binary_auditsmodule_file_size, "auditsmodule.spu.mod" );

	extern char _binary_fixedaddrmodule_file_start[];
	extern char _binary_fixedaddrmodule_file_size;
	SpuModuleHandle fixedAddrModuleHandle ( _binary_fixedaddrmodule_file_start, (U32)&_binary_fixedaddrmodule_file_size, "fixedaddrmodule.spu.mod" );


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Init our audit buffers
	//
	//Must be inited before creating joblists
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	const U32 kAuditBufferSize = 8024*1024;	//Shared between the SPUs

	static U8 g_buffersForAudits1[kAuditBufferSize] WWSJOB_ALIGNED(128);
	memset( g_buffersForAudits1, 0, sizeof(g_buffersForAudits1) );

	AuditManager auditManager1( g_buffersForAudits1, kAuditBufferSize, kNumSpus1, AuditManager::kSingleBuffered );

	auditManager1.SetJobManagerAuditsEnabled( true );
	auditManager1.SetJobAuditsEnabled( true );

	auditManager1.RegisterAuditData( AuditId::kAuditsModuleBase, AuditId::kAuditsModuleEnd, "AuditModule_", g_auditModuleAuditText );

	static U8 g_buffersForAudits2[kAuditBufferSize] WWSJOB_ALIGNED(128);
	memset( g_buffersForAudits2, 0, sizeof(g_buffersForAudits2) );

	AuditManager auditManager2( g_buffersForAudits2, kAuditBufferSize, kNumSpus2, AuditManager::kSingleBuffered );

	auditManager2.SetJobManagerAuditsEnabled( true );
	auditManager2.SetJobAuditsEnabled( true );

	auditManager2.RegisterAuditData( AuditId::kAuditsModuleBase, AuditId::kAuditsModuleEnd, "AuditModule_", g_auditModuleAuditText );


	//Install the event handler to handle JobPrintf
	EventHandler eventHandler1;
	EventHandler eventHandler2;
	if ( kNumSpus1 )
		eventHandler1.InstallEventHandler( &g_spurs1, ppuThreadPrio, &auditManager1 );
	if ( kNumSpus2 )
		eventHandler2.InstallEventHandler( &g_spurs2, ppuThreadPrio, &auditManager2 );


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Create some jobLists
	//Tells spurs about the workloads
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static U8 g_bufferForHighPriorityJobList[ 1024 ] WWSJOB_ALIGNED( 128 );		//Let the user pass the biffer in rather than allocating
	static U8 g_bufferForStandardJobList[ 1024 ] WWSJOB_ALIGNED( 128 );
	static U8 g_bufferForMultiThreadedJobList[ 1024 ] WWSJOB_ALIGNED( 128 );
	static U8 g_bufferForAccumulationJobList[ 1024 ] WWSJOB_ALIGNED( 128 );
	SingleThreadJobList highPrioJobList( g_bufferForHighPriorityJobList, sizeof(g_bufferForHighPriorityJobList), "HighProList" );
	SingleThreadJobList standardJobList( g_bufferForStandardJobList, sizeof(g_bufferForStandardJobList), "StandardJobList" );
	MultiThreadSafeJobList multithreadedJobList( g_bufferForMultiThreadedJobList, sizeof(g_bufferForMultiThreadedJobList), "MultiThreadList" );
	SingleThreadJobList accumulationJobList( g_bufferForAccumulationJobList, sizeof(g_bufferForAccumulationJobList), "AccumulateList" );

	const uint8_t highPrioWorkPrios[8]		= { 1, 1, 1, 1, 1, 1, 0, 0 };
	const uint8_t standardWorkPrios[8]		= { 8, 8, 8, 8, 8, 8, 0, 0 };
	const uint8_t multithreadedWorkPrios[8]	= { 8, 8, 8, 8, 8, 8, 0, 0 };
	const uint8_t accumulationWorkPrios[8]	= { 5, 5, 5, 5, 5, 5, 0, 0 };

	//Tell SPURS about the job list
	highPrioJobList.AttachToSpurs( &g_spurs1, highPrioWorkPrios, kNumSpus1, &auditManager1 );
	standardJobList.AttachToSpurs( &g_spurs1, standardWorkPrios, kNumSpus1, &auditManager1 );
	multithreadedJobList.AttachToSpurs( &g_spurs1, multithreadedWorkPrios, kNumSpus1, &auditManager1 );
	accumulationJobList.AttachToSpurs( &g_spurs2, accumulationWorkPrios, 1, &auditManager2 );			//Only ever on one SPU

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Create a commandListBuilder to help us start creating some jobs
	//(for the time being we can just share one commandListBuilder among all lists)
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static U32 s_commandListBuilderDataBuffer[ 2 * 1024 ] WWSJOB_ALIGNED( 128 );
	CommandListBuilder commandListBuilder( s_commandListBuilderDataBuffer, sizeof( s_commandListBuilderDataBuffer ) );



	const U32 kBufferSize = 4*1024; (void)kBufferSize;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Add some jobs for module1
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#ifdef ADD_THREE_MODULE1_JOBS
	static U8 s_module1InputBuffer1[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module1InputBuffer2[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module1InputBuffer3[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module1OutputBuffer1[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module1OutputBuffer2[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module1OutputBuffer3[kBufferSize] WWSJOB_ALIGNED(128);

	//Create three jobs
	JobHeader module1job1 = CreateModule1Job(	commandListBuilder,
												spuModule1Handle,
												s_module1InputBuffer1,
												s_module1OutputBuffer1,
												kBufferSize,
												5 );

	JobHeader module1job2 = CreateModule1Job(	commandListBuilder,
												spuModule1Handle,
												s_module1InputBuffer2,
												s_module1OutputBuffer2,
												kBufferSize,
												4 );

	JobHeader module1job3 = CreateModule1Job(	commandListBuilder,
												spuModule1Handle,
												s_module1InputBuffer3,
												s_module1OutputBuffer3,
												kBufferSize,
												3 );
#endif /* ADD_THREE_MODULE1_JOBS */


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Create a module1 job that will be added a high-prio job at run-time.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#ifdef ADD_HIGH_PRIO_JOB
	static U8 s_runTimeJobInputBuffer[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_runTimeJobOutputBuffer[kBufferSize] WWSJOB_ALIGNED(128);
	JobHeader runTimeJob = CreateModule1Job(	commandListBuilder,
												spuModule1Handle,
												s_runTimeJobInputBuffer,
												s_runTimeJobOutputBuffer,
												kBufferSize,
												2 );
#endif /* ADD_HIGH_PRIO_JOB */


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Add some jobs for module2
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

#ifdef ADD_THREE_MODULE2_JOBS
	static U8 s_module2InputBuffer1[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module2InputBuffer2[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module2InputBuffer3[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module2OutputBuffer1[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module2OutputBuffer2[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_module2OutputBuffer3[kBufferSize] WWSJOB_ALIGNED(128);
	memset( s_module2InputBuffer1, 0x12, kBufferSize );
	memset( s_module2InputBuffer2, 0x22, kBufferSize );
	memset( s_module2InputBuffer3, 0x32, kBufferSize );

	JobHeader module2job1 = CreateModule2Job(	commandListBuilder,
												spuModule2Handle,
												s_module2InputBuffer1,
												s_module2OutputBuffer1,
												kBufferSize,
												5 );

	JobHeader module2job2 = CreateModule2Job(	commandListBuilder,
												spuModule2Handle,
												s_module2InputBuffer2,
												s_module2OutputBuffer2,
												kBufferSize,
												4 );

	JobHeader module2job3 = CreateModule2Job(	commandListBuilder,
												spuModule2Handle,
												s_module2InputBuffer3,
												s_module2OutputBuffer3,
												kBufferSize,
												3 );
#endif /* ADD_THREE_MODULE2_JOBS */


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Add a job to test audits
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#ifdef ADD_AUDIT_TEST_JOB
	static U8 s_auditsModuleInputBuffer[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_auditsModuleOutputBuffer[kBufferSize] WWSJOB_ALIGNED(128);
	memset( s_auditsModuleInputBuffer, 0x72, kBufferSize );
	JobHeader auditsModuleJob = CreateAuditsModuleJob(	commandListBuilder,
														spuAuditsModuleHandle,
														s_auditsModuleInputBuffer,
														s_auditsModuleOutputBuffer,
														kBufferSize,
														2 );
#endif /* ADD_AUDIT_TEST_JOB */


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Add a job to test dmalists
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#ifdef ADD_DMA_LIST_JOB
	static U64 s_dmaListModuleInputDataBuffer[kBufferSize/sizeof(U64)] WWSJOB_ALIGNED(128);
	const U32 kNumDmaListElements = kBufferSize / sizeof(DmaListElement);
	static DmaListElement s_dmaListModuleDmaListBuffer[kNumDmaListElements] WWSJOB_ALIGNED(128);
	static U8 s_dmaListModuleOutputBuffer1[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_dmaListModuleOutputBuffer2[kBufferSize] WWSJOB_ALIGNED(128);
	memset( s_dmaListModuleInputDataBuffer, 0x22, kBufferSize );
	memset( s_dmaListModuleDmaListBuffer, 0x44, kBufferSize );
	//The input data buffer is not sent in directly,
	//but instead the dmalist is provided, and it is the dmalist
	//which pulls in the input data buffer

	// The first list element is null, the second transfer 16 bytes
	// Each second list element dma will *overwrite* the 2 list elements themselves
	// to ensure that this works.
	DmaListElement* pListElement	= &s_dmaListModuleDmaListBuffer[0];
	U64* pSrc = &s_dmaListModuleInputDataBuffer[0];

	for ( unsigned int i = 0; i < kNumDmaListElements; i += 2 )
	{
		pListElement[i].SetDmaListElement( 0, 0 );
		pListElement[i+1].SetDmaListElement( (const void*)&pSrc[i], 16 );
	}

	JobHeader dmaListJob1 = CreateDmaListModuleJob(	commandListBuilder,
													spuDmaListModuleHandle,
													s_dmaListModuleDmaListBuffer,
													sizeof(s_dmaListModuleDmaListBuffer),
													s_dmaListModuleOutputBuffer1,
													3 );
	JobHeader dmaListJob2 = CreateDmaListModuleJob(	commandListBuilder,
													spuDmaListModuleHandle,
													s_dmaListModuleDmaListBuffer,
													sizeof(s_dmaListModuleDmaListBuffer),
													s_dmaListModuleOutputBuffer2,
													2 );
#endif /* ADD_DMA_LIST_JOB */


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Add a job to test plugins
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#ifdef ADD_PLUGIN_JOB
	JobHeader pluginTestJob = CreatePluginTestJob(	commandListBuilder,
													spuPluginTestModuleHandle,
													spuPlugin1Handle,
													spuPlugin2Handle );
#endif /* ADD_PLUGIN_JOB */


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Add a series of jobs to test an accumulation buffer
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#ifdef ADD_ACCUMULATE_JOBS
	const U32 kAcumulationBufferSize = 16*1024;
	static U8 s_accumulationBuffer[kAcumulationBufferSize] WWSJOB_ALIGNED(128);

	//Note that these jobs *MUST* be put on a list that is *ONLY* going to be accessed by *ONE* SPU

	JobHeader accBufferInitJob = CreateAccumulationBufferInitJob(	commandListBuilder,
																	createAccBufferModule,
																	fillAccBufferModule,
																	writeAccBufferModule,
																	nonAccBufferModule,
																	s_accumulationBuffer,
																	kAcumulationBufferSize );
	JobHeader accBufferFillJob = CreateAccumulationBufferFillerJob(	commandListBuilder,
																	createAccBufferModule,
																	fillAccBufferModule,
																	writeAccBufferModule,
																	nonAccBufferModule,
																	s_accumulationBuffer,
																	kAcumulationBufferSize );
	JobHeader accBufferWriteOutJob = CreateAccumulationBufferWriteOutJob(	commandListBuilder,
																			createAccBufferModule,
																			fillAccBufferModule,
																			writeAccBufferModule,
																			nonAccBufferModule,
																			s_accumulationBuffer,
																			kAcumulationBufferSize );


	JobHeader accBufferCompatibleJob = CreateAccumulationBufferCompatibleJob(	commandListBuilder,
																				createAccBufferModule,
																				fillAccBufferModule,
																				writeAccBufferModule,
																				nonAccBufferModule,
																				s_accumulationBuffer,
																				kAcumulationBufferSize );
	JobHeader accBufferIncompatibleJob = CreateAccumulationBufferIncompatibleJob(	commandListBuilder,
																					createAccBufferModule,
																					fillAccBufferModule,
																					writeAccBufferModule,
																					nonAccBufferModule,
																					s_accumulationBuffer,
																					kAcumulationBufferSize );
#endif /* ADD_ACCUMULATE_JOBS */


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Add a job to test being located at a fixed address
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#ifdef ADD_FIXED_ADDR_JOB
	JobHeader fixedAddrJob = CreateFixedAddrModuleJob( commandListBuilder, fixedAddrModuleHandle );
#endif /* ADD_FIXED_ADDR_JOB */


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Add a pair of jobs to test an dependencies
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#ifdef ADD_TWO_DEPENDENENT_JOBS
	static U8 s_dependencyTestInputBuffer[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_dependencyTestOutputBuffer[kBufferSize] WWSJOB_ALIGNED(128);
	memset( s_dependencyTestInputBuffer, 0x21, kBufferSize );
	static DependencyCounter s_dependencyCounter[4];
	const U32 kImmediateDependencyDecrementValue = 3;
	const U32 kDepIndex = 1;
	s_dependencyCounter[kDepIndex].m_counter		= 1+kImmediateDependencyDecrementValue;	//Will be decremented to zero by first job
																						//Also PPU will do an immediate decrement call
	s_dependencyCounter[kDepIndex].m_readyCount	= 1;	//Only one more job on this list after barrier
	s_dependencyCounter[kDepIndex].m_workloadId	= standardJobList.GetWorkloadId();

	JobHeader depDecJob;
	JobHeader pPostDepJob;
	CreateTwoDependentJobs(	&depDecJob,
							&pPostDepJob,
							commandListBuilder,
							spuModule1Handle,
							s_dependencyTestInputBuffer,
							s_dependencyTestOutputBuffer,
							kBufferSize,
							2,
							3,
							&s_dependencyCounter[kDepIndex] );
#endif /* ADD_TWO_DEPENDENENT_JOBS */




	U32 frameNo = 0;
	while ( frameNo < 32 )
	{
		printf( "Start frame number %d\n", frameNo );
		++frameNo;

		highPrioJobList.ResetList();
		multithreadedJobList.ResetList();
		standardJobList.ResetList();
		accumulationJobList.ResetList();

#ifdef ADD_THREE_MODULE1_JOBS
		memset( s_module1InputBuffer1, 0x11, kBufferSize );
		memset( s_module1InputBuffer2, 0x12, kBufferSize );
		memset( s_module1InputBuffer3, 0x13, kBufferSize );
		memset( s_module1OutputBuffer1, 0xCC, kBufferSize );
		memset( s_module1OutputBuffer2, 0xCC, kBufferSize );
		memset( s_module1OutputBuffer3, 0xCC, kBufferSize );

		//And add the jobs to the job list
		JobListMarker markerAfterModule1Job1 = standardJobList.AddJob( module1job1 );
		JobListMarker markerAfterModule1Job2 = standardJobList.AddJob( module1job2 );
		JobListMarker markerAfterModule1Job3 = standardJobList.AddJob( module1job3 );
#endif /* ADD_THREE_MODULE1_JOBS */

#ifdef ADD_THREE_MODULE2_JOBS
		memset( s_module2OutputBuffer1, 0xCC, kBufferSize );
		memset( s_module2OutputBuffer2, 0xCC, kBufferSize );
		memset( s_module2OutputBuffer3, 0xCC, kBufferSize );

		//And add the jobs to the job list
		JobListMarker markerAfterModule2Job1 = multithreadedJobList.AddJob( module2job1 );
		JobListMarker markerAfterModule2Job2 = multithreadedJobList.AddJob( module2job2 );
		JobListMarker markerAfterModule2Job3 = multithreadedJobList.AddJob( module2job3 );
#endif /* ADD_THREE_MODULE2_JOBS */

#ifdef ADD_AUDIT_TEST_JOB
		memset( s_auditsModuleOutputBuffer, 0xCC, kBufferSize );

		//And add it to the job list
		JobListMarker markerAfterAuditsModuleJob = standardJobList.AddJob( auditsModuleJob );
#endif /* ADD_AUDIT_TEST_JOB */

#ifdef ADD_DMA_LIST_JOB
		memset( s_dmaListModuleOutputBuffer1, 0xCC, kBufferSize );
		memset( s_dmaListModuleOutputBuffer2, 0xCC, kBufferSize );

		//And add it to the job list
		JobListMarker markerAfterDmaListModuleJob1 = standardJobList.AddJob( dmaListJob1 );
		JobListMarker markerAfterDmaListModuleJob2 = standardJobList.AddJob( dmaListJob2 );
#endif /* ADD_DMA_LIST_JOB */

#ifdef ADD_PLUGIN_JOB
		//And add it to the job list
		JobListMarker markerAfterPluginTestModuleJob = standardJobList.AddJob( pluginTestJob );
#endif /* ADD_PLUGIN_JOB */

#ifdef ADD_ACCUMULATE_JOBS
		memset( s_accumulationBuffer, 0x44, kAcumulationBufferSize );

		JobListMarker markerAfterAccumulationBufferTestJobs;
		if ( frameNo & 1 )
		{
			//On odd frames test compatible then incompatible
			accumulationJobList.AddJob( accBufferInitJob );
			accumulationJobList.AddJob( accBufferCompatibleJob );
			accumulationJobList.AddJob( accBufferFillJob );
			accumulationJobList.AddJob( accBufferIncompatibleJob );
			markerAfterAccumulationBufferTestJobs = accumulationJobList.AddJob( accBufferWriteOutJob );
		}
		else
		{
			//On even frames test incompatible then compatible
			accumulationJobList.AddJob( accBufferInitJob );
			accumulationJobList.AddJob( accBufferIncompatibleJob );
			accumulationJobList.AddJob( accBufferFillJob );
			accumulationJobList.AddJob( accBufferCompatibleJob );
			markerAfterAccumulationBufferTestJobs = accumulationJobList.AddJob( accBufferWriteOutJob );
		}
#endif /* ADD_ACCUMULATE_JOBS */

#ifdef ADD_FIXED_ADDR_JOB
		//And add it to the job list
		JobListMarker markerAfterFixedAddrModuleJob = standardJobList.AddJob( fixedAddrJob );
#endif /* ADD_FIXED_ADDR_JOB */

#ifdef ADD_TWO_DEPENDENENT_JOBS
		//Reset the dependency counter back up to 1
		s_dependencyCounter[kDepIndex].m_counter = 1+kImmediateDependencyDecrementValue;
		memset( s_dependencyTestOutputBuffer, 0xCC, kBufferSize );

		//And add it to the job list
		standardJobList.AddJob( depDecJob );

		//This barrier can't be passed until the dependency counter above has been written
		standardJobList.AddGeneralBarrier( &s_dependencyCounter[kDepIndex] );

		//And add it to the job list
		JobListMarker markerAfterTwoDependentJobs = standardJobList.AddJob( pPostDepJob );
#endif /* ADD_TWO_DEPENDENENT_JOBS */

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//Now start the joblists running
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		standardJobList.SetReadyCount( SingleThreadJobList::kRequestAllSpus );

		multithreadedJobList.SetReadyCount( MultiThreadSafeJobList::kRequestAllSpus );

		accumulationJobList.SetReadyCount( SingleThreadJobList::kRequestOneSpu );	//Only want one SPU on this job list ever



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//Once the other jobs have started running, add a job at run-time
		//that should pre-empt one of the other lists
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

#ifdef ADD_HIGH_PRIO_JOB
		//Pause before adding the job to give the SPUs time to start doing work
		sys_timer_usleep( 100000 );

		memset( s_runTimeJobInputBuffer, 0x72, kBufferSize );
		memset( s_runTimeJobOutputBuffer, 0xCC, kBufferSize );

		//Add the job to the joblist
		JobListMarker markerAfterRunTimeJob = highPrioJobList.AddJob( runTimeJob );

		//And set the ready count to tell an SPU to come and work on it
		highPrioJobList.SetReadyCount( SingleThreadJobList::kRequestOneSpu );	//Only one job on this list so set ready count to one
#endif /* ADD_HIGH_PRIO_JOB */



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//Stall until each bit of work is done, then print the results
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

#ifdef ADD_HIGH_PRIO_JOB
		//high-prio run-time job
		markerAfterRunTimeJob.StallForJobMarker();
		CheckResults( "s_runTimeJobOutputBuffer", kBufferSize, s_runTimeJobOutputBuffer, 0xE4 );
#endif /* ADD_HIGH_PRIO_JOB */

#ifdef ADD_THREE_MODULE1_JOBS
		//Module1 jobs
		markerAfterModule1Job1.StallForJobMarker();
		CheckResults( "s_module1OutputBuffer1", kBufferSize, s_module1OutputBuffer1, 0x55 );

		markerAfterModule1Job2.StallForJobMarker();
		CheckResults( "s_module1OutputBuffer2", kBufferSize, s_module1OutputBuffer2, 0x48 );

		markerAfterModule1Job3.StallForJobMarker();
		CheckResults( "s_module1OutputBuffer3", kBufferSize, s_module1OutputBuffer3, 0x39 );
#endif /* ADD_THREE_MODULE1_JOBS */


#ifdef ADD_THREE_MODULE2_JOBS
		//Module2 jobs
		markerAfterModule2Job1.StallForJobMarker();
		CheckResults( "s_module2OutputBuffer1", kBufferSize, s_module2OutputBuffer1, 0x5A );

		markerAfterModule2Job2.StallForJobMarker();
		CheckResults( "s_module2OutputBuffer2", kBufferSize, s_module2OutputBuffer2, 0x88 );

		markerAfterModule2Job3.StallForJobMarker();
		CheckResults( "s_module2OutputBuffer3", kBufferSize, s_module2OutputBuffer3, 0x96 );
#endif /* ADD_THREE_MODULE2_JOBS */


#ifdef ADD_AUDIT_TEST_JOB
		//AuditsModule job
		markerAfterAuditsModuleJob.StallForJobMarker();
		CheckResults( "s_auditsModuleOutputBuffer", kBufferSize, s_auditsModuleOutputBuffer, 0xE4 );
#endif /* ADD_AUDIT_TEST_JOB */

#ifdef ADD_DMA_LIST_JOB
		//DmaListModule jobs
		markerAfterDmaListModuleJob1.StallForJobMarker();
		CheckResults( "s_dmaListModuleOutputBuffer1", kBufferSize, s_dmaListModuleOutputBuffer1, 0x66 );

		//By changing the dma list to be kReadOnlyCached we can test whether kReadOnlyCached works with dma lists.
		//Assuming it works, then the dma list will not be re-read in on the second job since it already in LS.
		//The job is in fact naughty and disobeys kReadOnlyCached by corrupting that buffer.  We should expect 0x22 * 3 * 2 if it is not re-read in.
		//If the buffer was re-read from main memory (eg. if kReadOnlyCached had a bug) we would get 0x22 * 2.
		//Note that we normally run with kNonCached and expect 0x44, because the test isn't actually good code unless we put some
		//effort into ensuring another job won't jump in the middle and force a re-load anyway, which could result in false negatives.
		markerAfterDmaListModuleJob2.StallForJobMarker();
		CheckResults( "s_dmaListModuleOutputBuffer2", kBufferSize, s_dmaListModuleOutputBuffer2, 0x44 );
		//CheckResults( "s_dmaListModuleOutputBuffer2", kBufferSize, s_dmaListModuleOutputBuffer2, 0xCC );
#endif /* ADD_DMA_LIST_JOB */


#ifdef ADD_PLUGIN_JOB
		//Plugin test module job
		markerAfterPluginTestModuleJob.StallForJobMarker();
		//printf( "Plugin test has finished\n" );
#endif /* ADD_PLUGIN_JOB */


#ifdef ADD_ACCUMULATE_JOBS
		//Accumulate test jobs
		markerAfterAccumulationBufferTestJobs.StallForJobMarker();
		CheckResults( "s_accumulationBuffer", kAcumulationBufferSize, s_accumulationBuffer, 37 );
#endif /* ADD_ACCUMULATE_JOBS */


#ifdef ADD_FIXED_ADDR_JOB
		markerAfterFixedAddrModuleJob.StallForJobMarker();
		//No results to check
#endif


#ifdef ADD_TWO_DEPENDENENT_JOBS
		bool isSatisfied = WwsJob_JobApiIsDependencySatisfied( &s_dependencyCounter[kDepIndex] );
		WWSJOB_ASSERT( isSatisfied == false );
		WWSJOB_UNUSED( isSatisfied );
		WwsJob_JobApiDecrementDependencyImmediate( standardJobList.GetSpursPtr(), &s_dependencyCounter[kDepIndex], kImmediateDependencyDecrementValue );

		//Dependency test jobs
		markerAfterTwoDependentJobs.StallForJobMarker();
		CheckResults( "s_dependencyTestOutputBuffer", kBufferSize, s_dependencyTestOutputBuffer, 0xC6 );
#endif /* ADD_TWO_DEPENDENENT_JOBS */



		//error check the joblists while we're waiting (optionally print them to TTY as well)
		highPrioJobList.CheckJobList( SingleThreadJobList::kPrintWarningsAndHints );
		multithreadedJobList.CheckJobList( MultiThreadSafeJobList::kNothing );	//Intentional warnings with module2 are ignored at present
		standardJobList.CheckJobList( SingleThreadJobList::kPrintWarningsAndHints );
		accumulationJobList.CheckJobList( SingleThreadJobList::kPrintWarningsAndHints );


		//Wait for the end of the lists
		highPrioJobList.WaitForJobListEnd();
//		printf( "highPrioJobList work is now finished\n" );

		multithreadedJobList.WaitForJobListEnd();
//		printf( "multithreadedJobList work is now finished\n" );

		standardJobList.WaitForJobListEnd();
//		printf( "standardJobList work is now finished\n" );

		accumulationJobList.WaitForJobListEnd();
//		printf( "accumulationJobList work is now finished\n" );



		//Print out the audit buffers and empty them.
//		auditManager1.ProcessAuditBuffersForAllSpus( 0, AuditManager::PrintAudit, AuditManager::kPrintHeaders, NULL );
//		auditManager2.ProcessAuditBuffersForAllSpus( 0, AuditManager::PrintAudit, AuditManager::kPrintHeaders, NULL );
		auditManager1.EmptyAuditBuffersForAllSpus( 0 );
		auditManager2.EmptyAuditBuffersForAllSpus( 0 );

	}



	//Shut down and remove the workloads from Spurs
	standardJobList.Shutdown();
	multithreadedJobList.Shutdown();
	highPrioJobList.Shutdown();
	accumulationJobList.Shutdown();
	printf( "Shutdown joblists OK\n" );

	if ( kNumSpus2 )
	{
		eventHandler2.RemoveEventHandler( &g_spurs2 );
		ret = cellSpursFinalize( &g_spurs2 );
		WWSJOB_ASSERT_MSG( CELL_OK == ret, ("cellSpursFinalize returned %d", ret) );
	}
	if ( kNumSpus1 )
	{
		eventHandler1.RemoveEventHandler( &g_spurs1 );
		ret = cellSpursFinalize( &g_spurs1 );
		WWSJOB_ASSERT_MSG( CELL_OK == ret, ("cellSpursFinalize returned %d", ret) );
	}

	printf( "End of Job Manager Test\n" );

	return 0;
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateAccumulationBufferInitJob(	CommandListBuilder& rCommandListBuilder,
											const SpuModuleHandle& createAccBufferModule,
											const SpuModuleHandle& fillAccBufferModule,
											const SpuModuleHandle& writeAccBufferModule,
											const SpuModuleHandle& nonAccBufferModule,
											void* pAccBuffer,
											U32 bufferSize	)
{
	//two code buffers
	const U32 kCodeBufferSet							= 0;
	const U32 kNumCodeBuffers							= 2;
	const U32 kCodeBufferSizeInPages					= WwsJob_max(	WwsJob_max( createAccBufferModule.GetRequiredBufferSizeInPages(),
																		fillAccBufferModule.GetRequiredBufferSizeInPages() ),
																	WwsJob_max( writeAccBufferModule.GetRequiredBufferSizeInPages(),
																		nonAccBufferModule.GetRequiredBufferSizeInPages() ) );
	const U32 kCodeBufferSetBasePageNum					= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by one 16K data buffers
	const U32 kDataBufferSet							= 1;
	const U32 kNumDataBuffers							= 1;
	const U32 kDataBufferSizeInPages					= (bufferSize / 1024);
	const U32 kDataBufferSetBasePageNum					= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//This memory map is used for a compatible buffer set
	//It sets up buffers after the accumulation buffer so shouldn't cause a flush
//	const U32 kCompatibleDataBufferSet					= 2;
//	const U32 kNumCompatibleDataBuffers					= 4;
//	const U32 kCompatibleDataBufferSizeInPages			= 4;
//	const U32 kCompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum + kNumDataBuffers*kDataBufferSizeInPages;

	//This memory map is used for an incompatible buffer set
	//It sets up buffers that clash with the accumulation buffer so cause it to be flush
//	const U32 kIncompatibleDataBufferSet				= 2;
//	const U32 kNumIncompatibleDataBuffers				= 1;
//	const U32 kIncompatibleDataBufferSizeInPages		= 4;
//	const U32 kIncompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//The first job creates the accumulation buffer in LS
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//The input parameters to the job (in the memory layout they'll be expected in)
	CreateAccBufferModuleParams createAccBufferModuleParams;
	createAccBufferModuleParams.m_initValue		= 2;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, createAccBufferModule, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UseUninitializedBuffer( kDataBufferSet, kLogicalBuffer0, pAccBuffer, bufferSize, WwsJob_Command::kReadWriteCached );
	rCommandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kDataBufferSet) );
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &createAccBufferModuleParams, sizeof(createAccBufferModuleParams) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateAccumulationBufferCompatibleJob(	CommandListBuilder& rCommandListBuilder,
													const SpuModuleHandle& createAccBufferModule,
													const SpuModuleHandle& fillAccBufferModule,
													const SpuModuleHandle& writeAccBufferModule,
													const SpuModuleHandle& nonAccBufferModule,
													void* pAccBuffer,
													U32 bufferSize	)
{
	const U32 kCodeBufferSet							= 0;
	const U32 kNumCodeBuffers							= 2;
	const U32 kCodeBufferSizeInPages					= WwsJob_max(	WwsJob_max( createAccBufferModule.GetRequiredBufferSizeInPages(),
																		fillAccBufferModule.GetRequiredBufferSizeInPages() ),
																	WwsJob_max( writeAccBufferModule.GetRequiredBufferSizeInPages(),
																		nonAccBufferModule.GetRequiredBufferSizeInPages() ) );
	const U32 kCodeBufferSetBasePageNum					= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by one 16K data buffers
//	const U32 kDataBufferSet							= 1;
	const U32 kNumDataBuffers							= 1;
	const U32 kDataBufferSizeInPages					= (bufferSize / 1024);
	const U32 kDataBufferSetBasePageNum					= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//This memory map is used for a compatible buffer set
	//It sets up buffers after the accumulation buffer so shouldn't cause a flush
	const U32 kCompatibleDataBufferSet					= 2;
	const U32 kNumCompatibleDataBuffers					= 4;
	const U32 kCompatibleDataBufferSizeInPages			= 4;
	const U32 kCompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum + kNumDataBuffers*kDataBufferSizeInPages;

	//This memory map is used for an incompatible buffer set
	//It sets up buffers that clash with the accumulation buffer so cause it to be flush
//	const U32 kIncompatibleDataBufferSet				= 2;
//	const U32 kNumIncompatibleDataBuffers				= 1;
//	const U32 kIncompatibleDataBufferSizeInPages		= 4;
//	const U32 kIncompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//This job is compatible with the accumulation buffer memory map so
	//doesn't cause a flush
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//The input parameters to the job (in the memory layout they'll be expected in)
	NonAccBufferModuleParams nonAccBufferModuleParams;
	nonAccBufferModuleParams.m_clearVal		= 0xCC;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kCompatibleDataBufferSet, kNumCompatibleDataBuffers, kCompatibleDataBufferSetBasePageNum, kCompatibleDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, nonAccBufferModule, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UseInputBuffer( kCompatibleDataBufferSet, kLogicalBuffer0, pAccBuffer, NumPagesToNumBytes( kCompatibleDataBufferSizeInPages ), WwsJob_Command::kNonCached );
	rCommandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kCompatibleDataBufferSet) );
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &nonAccBufferModuleParams, sizeof(nonAccBufferModuleParams) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateAccumulationBufferIncompatibleJob(	CommandListBuilder& rCommandListBuilder,
													const SpuModuleHandle& createAccBufferModule,
													const SpuModuleHandle& fillAccBufferModule,
													const SpuModuleHandle& writeAccBufferModule,
													const SpuModuleHandle& nonAccBufferModule,
													void* pAccBuffer,
													U32 bufferSize	)
{
	WWSJOB_UNUSED( bufferSize );
	const U32 kCodeBufferSet							= 0;
	const U32 kNumCodeBuffers							= 2;
	const U32 kCodeBufferSizeInPages					= WwsJob_max(	WwsJob_max( createAccBufferModule.GetRequiredBufferSizeInPages(),
																		fillAccBufferModule.GetRequiredBufferSizeInPages() ),
																	WwsJob_max( writeAccBufferModule.GetRequiredBufferSizeInPages(),
																		nonAccBufferModule.GetRequiredBufferSizeInPages() ) );
	const U32 kCodeBufferSetBasePageNum					= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by one 16K data buffers
//	const U32 kDataBufferSet							= 1;
//	const U32 kNumDataBuffers							= 1;
//	const U32 kDataBufferSizeInPages					= (bufferSize / 1024);
	const U32 kDataBufferSetBasePageNum					= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//This memory map is used for a compatible buffer set
	//It sets up buffers after the accumulation buffer so shouldn't cause a flush
//	const U32 kCompatibleDataBufferSet					= 2;
//	const U32 kNumCompatibleDataBuffers					= 4;
//	const U32 kCompatibleDataBufferSizeInPages			= 4;
//	const U32 kCompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum + kNumDataBuffers*kDataBufferSizeInPages;

	//This memory map is used for an incompatible buffer set
	//It sets up buffers that clash with the accumulation buffer so cause it to be flush
	const U32 kIncompatibleDataBufferSet				= 2;
	const U32 kNumIncompatibleDataBuffers				= 1;
	const U32 kIncompatibleDataBufferSizeInPages		= 4;
	const U32 kIncompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum;

	//The input parameters to the job (in the memory layout they'll be expected in)
	NonAccBufferModuleParams nonAccBufferModuleParams;
	nonAccBufferModuleParams.m_clearVal		= 0xCC;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kIncompatibleDataBufferSet, kNumIncompatibleDataBuffers, kIncompatibleDataBufferSetBasePageNum, kIncompatibleDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, nonAccBufferModule, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UseInputBuffer( kIncompatibleDataBufferSet, kLogicalBuffer0, pAccBuffer, NumPagesToNumBytes( kIncompatibleDataBufferSizeInPages ), WwsJob_Command::kNonCached );
	rCommandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kIncompatibleDataBufferSet) );
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &nonAccBufferModuleParams, sizeof(nonAccBufferModuleParams) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateAccumulationBufferFillerJob(	CommandListBuilder& rCommandListBuilder,
												const SpuModuleHandle& createAccBufferModule,
												const SpuModuleHandle& fillAccBufferModule,
												const SpuModuleHandle& writeAccBufferModule,
												const SpuModuleHandle& nonAccBufferModule,
												void* pAccBuffer,
												U32 bufferSize	)
{
	const U32 kCodeBufferSet							= 0;
	const U32 kNumCodeBuffers							= 2;
	const U32 kCodeBufferSizeInPages					= WwsJob_max(	WwsJob_max( createAccBufferModule.GetRequiredBufferSizeInPages(),
																		fillAccBufferModule.GetRequiredBufferSizeInPages() ),
																	WwsJob_max( writeAccBufferModule.GetRequiredBufferSizeInPages(),
																		nonAccBufferModule.GetRequiredBufferSizeInPages() ) );
	const U32 kCodeBufferSetBasePageNum					= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by one 16K data buffers
	const U32 kDataBufferSet							= 1;
	const U32 kNumDataBuffers							= 1;
	const U32 kDataBufferSizeInPages					= (bufferSize / 1024);
	const U32 kDataBufferSetBasePageNum					= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//This memory map is used for a compatible buffer set
	//It sets up buffers after the accumulation buffer so shouldn't cause a flush
//	const U32 kCompatibleDataBufferSet					= 2;
//	const U32 kNumCompatibleDataBuffers					= 4;
//	const U32 kCompatibleDataBufferSizeInPages			= 4;
//	const U32 kCompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum + kNumDataBuffers*kDataBufferSizeInPages;

	//This memory map is used for an incompatible buffer set
	//It sets up buffers that clash with the accumulation buffer so cause it to be flush
//	const U32 kIncompatibleDataBufferSet				= 2;
//	const U32 kNumIncompatibleDataBuffers				= 1;
//	const U32 kIncompatibleDataBufferSizeInPages		= 4;
//	const U32 kIncompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum;

	//The input parameters to the job (in the memory layout they'll be expected in)
	FillAccBufferModuleParams fillAccBufferModuleParams;
	fillAccBufferModuleParams.m_multiplier		= 17;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, fillAccBufferModule, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pAccBuffer, bufferSize, WwsJob_Command::kReadWriteCached );
	rCommandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kDataBufferSet) );
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &fillAccBufferModuleParams, sizeof(fillAccBufferModuleParams) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------

JobHeader CreateAccumulationBufferWriteOutJob(	CommandListBuilder& rCommandListBuilder,
												const SpuModuleHandle& createAccBufferModule,
												const SpuModuleHandle& fillAccBufferModule,
												const SpuModuleHandle& writeAccBufferModule,
												const SpuModuleHandle& nonAccBufferModule,
												void* pAccBuffer,
												U32 bufferSize	)
{
	const U32 kCodeBufferSet							= 0;
	const U32 kNumCodeBuffers							= 2;
	const U32 kCodeBufferSizeInPages					= WwsJob_max(	WwsJob_max( createAccBufferModule.GetRequiredBufferSizeInPages(),
																		fillAccBufferModule.GetRequiredBufferSizeInPages() ),
																	WwsJob_max( writeAccBufferModule.GetRequiredBufferSizeInPages(),
																		nonAccBufferModule.GetRequiredBufferSizeInPages() ) );
	const U32 kCodeBufferSetBasePageNum					= LsMemoryLimits::kJobAreaBasePageNum;

	//followed by one 16K data buffers
	const U32 kDataBufferSet							= 1;
	const U32 kNumDataBuffers							= 1;
	const U32 kDataBufferSizeInPages					= (bufferSize / 1024);
	const U32 kDataBufferSetBasePageNum					= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//This memory map is used for a compatible buffer set
	//It sets up buffers after the accumulation buffer so shouldn't cause a flush
//	const U32 kCompatibleDataBufferSet					= 2;
//	const U32 kNumCompatibleDataBuffers					= 4;
//	const U32 kCompatibleDataBufferSizeInPages			= 4;
//	const U32 kCompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum + kNumDataBuffers*kDataBufferSizeInPages;

	//This memory map is used for an incompatible buffer set
	//It sets up buffers that clash with the accumulation buffer so cause it to be flush
//	const U32 kIncompatibleDataBufferSet				= 2;
//	const U32 kNumIncompatibleDataBuffers				= 1;
//	const U32 kIncompatibleDataBufferSizeInPages		= 4;
//	const U32 kIncompatibleDataBufferSetBasePageNum		= kDataBufferSetBasePageNum;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//The final job finalises the accumulation buffer in LS and flushes it
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//The input parameters to the job (in the memory layout they'll be expected in)
	WriteAccBufferModuleParams writeAccBufferModuleParams;
	writeAccBufferModuleParams.m_addValue		= 3;

	//Create the job
	rCommandListBuilder.InitializeJob();
	rCommandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	rCommandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );
	rCommandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, writeAccBufferModule, WwsJob_Command::kReadOnlyCached );
	rCommandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, pAccBuffer, bufferSize, WwsJob_Command::kReadWriteCached );
	rCommandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kDataBufferSet) );
	rCommandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	rCommandListBuilder.AddParams( &writeAccBufferModuleParams, sizeof(writeAccBufferModuleParams) );
	return rCommandListBuilder.FinalizeJob();
}

//--------------------------------------------------------------------------------------------------
