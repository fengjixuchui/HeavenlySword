/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		The JobApi functions that a job may call on an SPU
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_API_H
#define WWS_JOB_JOB_API_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>

//--------------------------------------------------------------------------------------------------

#include <jobapi/spumoduleheader.h>
#include <jobapi/spuinterrupts.h>

//--------------------------------------------------------------------------------------------------

typedef unsigned CellSpursWorkloadId;
struct WwsJob_Command;
union JobHeader;

//--------------------------------------------------------------------------------------------------

//namespace Wws
//{
//namespace Job
//{
	struct WwsJob_DataForJob;
//}
//};

//--------------------------------------------------------------------------------------------------

//This typedef needs to be outside the namespace so that
//it can be used for declaring the extern "C" value g_jobContext
typedef const WwsJob_DataForJob* JobApiContext;

//--------------------------------------------------------------------------------------------------

extern "C"
{
	//void JobMain( /* The user may receive up to 8 U32 parameters here */ );
	//The user must prototype this function themselves (as extern "C") since only they know
	//how many parameters they're going to be taking.

	extern JobApiContext g_jobContext;
	extern Bool32 g_nextJobHasStartedLoading;
}

//--------------------------------------------------------------------------------------------------

//namespace Wws
//{
//namespace Job
//{
union WwsJob_BufferTag;
union WwsJob_BufferTagInputOutput;

enum WwsJob_RequestDmaTag
{
	WwsJob_kAllocDmaTag		= 1,
	WwsJob_kDontAllocDmaTag	= 0,
};

//--------------------------------------------------------------------------------------------------
//These are the valid commands for the job to call
//Below they are actually all inline functions as much as possible.
const void* WwsJob_JobApiGetUnsafePointerToParams( void );
void WwsJob_JobApiCopyParams( void *pUserQwords, U32 size );
const U32 WwsJob_JobApiGetSpuNum( void );
const U32 WwsJob_JobApiGetJobId( void );
void WwsJob_JobApiExecuteCommands( const WwsJob_Command* pCommands );
WwsJob_BufferTag WwsJob_JobApiGetBufferTag( U32 logicalBufferSetNum, U32 logicalBufferNum, WwsJob_RequestDmaTag dmaTagRequest );
void WwsJob_JobApiGetBufferTags( WwsJob_BufferTagInputOutput* pBufferTagInputOutput, U32 numBuffers );
U32 WwsJob_JobApiUseDmaTag( void );
void WwsJob_JobApiFreeDmaTagId( U32 dmaTagId );
void WwsJob_JobApiLoadNextJob( void );
void WwsJob_JobApiFreeLogicalBuffer( U32 logicalBufferSetNum, U32 logicalBufferNum );
void WwsJob_JobApiFreeLogicalBuffers( const U8* pLogicalBufferNumPairs, U32 numLogicalBufferNumPairs );
void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword = 0 );
void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0 );
void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1 );
void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1, U64 param2 );
void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1, U64 param2, U64 param3 );
void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1, U64 param2, U64 param3, U64 param4 );
void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1, U64 param2, U64 param3, U64 param4, U64 param5 );
void WwsJob_JobApiStoreAudit( U16 auditId, U16 numDwords, const void* pDwords );
void* WwsJob_JobApiInitPlugin( const void* pluginAddr );
void WwsJob_JobApiShutdownPlugin( const void* pluginAddr );
void WwsJob_JobApiSendEvent( U32 portNum, U32 data1, U32 data2 );
void WwsJob_JobApiSetReadyCount( CellSpursWorkloadId workloadId, U32 readyCount );
void WwsJob_JobApiSetReadyCount( U32 eaSpurs, CellSpursWorkloadId workloadId, U32 readyCount ) __attribute__((deprecated));
void WwsJob_JobApiAddJobToJobList( U32 eaJobList, JobHeader jobHeader );
U16 WwsJob_JobApiDecrementDependencyImmediate( U32 eaSpurs, U32 eaDependencyCounter, U32 decrementVal );

//--------------------------------------------------------------------------------------------------

namespace WwsJob_ApiCommand
{
	enum
	{
		kExecuteCommands,

		kGetBufferTag,
		kGetBufferTags,

		kUseDmaTagId,
		kFreeDmaTagId,

		kLoadNextJob,
		
		kFreeLogicalBuffer,
		kFreeLogicalBuffers,

		kStoreAudit,
	};
}

//--------------------------------------------------------------------------------------------------

// data passed from job to jobManager, to indicate which buffer the job wants
struct WwsJob_BufferTagInput
{
	U32		m_logicalBufferSetNum;
	U32		m_logicalBufferNum;
	U32		m_useDmaTagId;
	U32		m_pad;
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

// data passed back to job

union WwsJob_BufferTag	// 16 bytes, 16 byte aligned
{
	struct 
	{           
		U8		m_logicalBufferSetNum;	// 0:7
		U8		m_logicalBufferNum;		// 0:7
		U16		m_dmaTagId;				// 1:31, or 0 means no dmaTagId 
		U16		m_lsAddressInWords;		// page aligned
		U16		m_lsLengthInWords;		// length of buffer, page aligned
		U32		m_mmAddress;			// qword aligned
		U32		m_mmLength;				// qword aligned, <= m_lsLength
	};
	VU32		m_vu32;
	U64			m_u64[2];
	U32			m_u32[4];
	U16			m_u16[8];
	U8			m_u8[16];
	
	void*	GetAddr( void ) const		{ return (void*)(m_lsAddressInWords * 4); }
	U32		GetLength( void ) const		{ return (m_lsLengthInWords * 4); }
	U16		GetDmaTag( void ) const		{ return m_dmaTagId; }
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

union WwsJob_BufferTagInputOutput
{
	WwsJob_BufferTagInput	m_bufferTagInput;
	WwsJob_BufferTag		m_bufferTagOutput;
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

union WwsJob_ApiReturn
{
	WwsJob_BufferTag	m_bufferTag;
	VU32		m_vu32;
	U64			m_u64[2];
	U32			m_u32[4];
	U16			m_u16[8];
	U8			m_u8[16];
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

typedef WwsJob_ApiReturn (*JobApiPtr)( U32/*WwsJob_ApiCommand*/ apiCommand, U32 parameter0, U32 parameter1, U32 parameter2 );
typedef void (*ExecuteCommandsPtr)( const WwsJob_Command* pCommands );
typedef WwsJob_BufferTag (*GetBufferTagPtr)( U32 unused, U32 logicalBufferSetNum, U32 logicalBufferNum, WwsJob_RequestDmaTag dmaTagRequest );
typedef void (*GetBufferTagsPtr)( WwsJob_BufferTagInputOutput* pBufferTagInput, U32 numBuffers );
typedef U32 (*UseDmaTagIdPtr)( void );
typedef void (*FreeDmaTagIdPtr)( U32 dmaTagId );
typedef void (*LoadNextJobPtr)( void );
typedef void (*FreeLogicalBufferPtr)(  U32 unused, U32 logicalBufferSetNum, U32 logicalBufferNum );
typedef void (*FreeLogicalBuffersPtr)( const U8* pLogicalBufferNumPairs, U32 numLogicalBufferNumPairs );
typedef void (*StoreAuditPtr)( U32 parm1, U32 parm2, U32 parm3 );

//--------------------------------------------------------------------------------------------------

//Note that the offset to m_pParameters is now hardcoded into jobctr0.spu.s
struct WwsJob_DataForJob
{
	U32		WWSJOB_ALIGNED( 16 ) m_spuNum;		// 0:5
	U32		WWSJOB_ALIGNED( 16 ) m_jobId;		// hi 16 bits = workloadId, lo 16 bits = jobIndex
	const void*	WWSJOB_ALIGNED( 16 ) m_pParameters;	// ptr to parameters (which are qword aligned)
	union 
	{
		ExecuteCommandsPtr m_executeCommands;
		GetBufferTagPtr m_getBufferTag;
		GetBufferTagsPtr m_getBufferTags;
		UseDmaTagIdPtr m_useDmaTagId;
		FreeDmaTagIdPtr m_freeDmaTagId;
		LoadNextJobPtr m_loadNextJob;
		FreeLogicalBufferPtr m_freeLogicalBuffer;
		FreeLogicalBuffersPtr m_freeLogicalBuffers;
		StoreAuditPtr m_storeAudit;
	}		m_pJobApi[9] WWSJOB_ALIGNED( 16 );
	Bool32		WWSJOB_ALIGNED( 16 ) m_bJobAuditsEnabled;
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

inline const void* WwsJob_JobApiGetUnsafePointerToParams( void )
{
	WWSJOB_ASSERT( !g_nextJobHasStartedLoading );
	return g_jobContext->m_pParameters;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiCopyParams( void *pUserQwords, U32 size )
{
	//Check the user isn't requesting too much?
	//WWSJOB_ASSERT( size <= g_jobContext->m_parametersMaxSize );
	WWSJOB_ASSERT( !g_nextJobHasStartedLoading );

	WwsJob_MemcpyQword( pUserQwords, g_jobContext->m_pParameters, size );
}

//--------------------------------------------------------------------------------------------------

inline const U32 WwsJob_JobApiGetSpuNum( void )
{
	return g_jobContext->m_spuNum;
}

//--------------------------------------------------------------------------------------------------

inline const U32 WwsJob_JobApiGetJobId( void )
{
	return g_jobContext->m_jobId;
}

//--------------------------------------------------------------------------------------------------

inline Bool32 WwsJob_JobApiAreJobAuditsEnabled( void )
{
	return g_jobContext->m_bJobAuditsEnabled;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiExecuteCommands( const WwsJob_Command* pCommands )
{
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kExecuteCommands].m_executeCommands(pCommands);
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline WwsJob_BufferTag WwsJob_JobApiGetBufferTag( U32 logicalBufferSetNum, U32 logicalBufferNum, WwsJob_RequestDmaTag dmaTagRequest )
{
	DisableInterrupts();
	WwsJob_BufferTag tag = g_jobContext->m_pJobApi[WwsJob_ApiCommand::kGetBufferTag].m_getBufferTag(0, logicalBufferSetNum, logicalBufferNum, dmaTagRequest );
	EnableInterrupts();
	return tag;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiGetBufferTags( WwsJob_BufferTagInputOutput* pBufferTagInputOutput, U32 numBuffers )
{
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kGetBufferTags].m_getBufferTags(pBufferTagInputOutput, numBuffers);
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline U32 WwsJob_JobApiUseDmaTag( void )
{
	DisableInterrupts();
	U32 ret = g_jobContext->m_pJobApi[WwsJob_ApiCommand::kUseDmaTagId].m_useDmaTagId();
	EnableInterrupts();
	return ret;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiFreeDmaTagId( U32 dmaTagId )
{
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kFreeDmaTagId].m_freeDmaTagId(dmaTagId);
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiLoadNextJob( void )
{
	g_nextJobHasStartedLoading = true;
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kLoadNextJob].m_loadNextJob();
	EnableInterrupts();

	//It might be tempting to zero out the params here in debug builds to try and help catch invalid usages
	//WwsJob_MemsetQword( g_jobContext->m_pParameters, 0xDEADBEEFDEADBEEFDEADBEEFDEADBEEF, size );
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiFreeLogicalBuffer( U32 logicalBufferSetNum, U32 logicalBufferNum )
{
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kFreeLogicalBuffer].m_freeLogicalBuffer(0, logicalBufferSetNum, logicalBufferNum);
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiFreeLogicalBuffers( const U8* pLogicalBufferNumPairs, U32 numLogicalBufferNumPairs )
{
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kFreeLogicalBuffers].m_freeLogicalBuffers(pLogicalBufferNumPairs, numLogicalBufferNumPairs);
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword )
{
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kStoreAudit].m_storeAudit((auditId << 16) | hword, 0, 0);
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0 )
{
	U64 param[] = { param0 };
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kStoreAudit].m_storeAudit((auditId << 16) | hword, 0x80000001, (U32)&param[0]);
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1 )
{
	U64 param[] = { param0, param1 };
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kStoreAudit].m_storeAudit((auditId << 16) | hword, 0x80000002, (U32)&param[0]);
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1, U64 param2 )
{
	U64 param[] = { param0, param1, param2 };
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kStoreAudit].m_storeAudit((auditId << 16) | hword, 0x80000003, (U32)&param[0] );
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1, U64 param2, U64 param3 )
{
	U64 param[] = { param0, param1, param2, param3 };
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kStoreAudit].m_storeAudit((auditId << 16) | hword, 0x80000004, (U32)&param[0] );
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1, U64 param2, U64 param3, U64 param4 )
{
	U64 param[] = { param0, param1, param2, param3, param4 };
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kStoreAudit].m_storeAudit((auditId << 16) | hword, 0x80000005, (U32)&param[0] );
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiStoreAudit( U16 auditId, U16 hword, U64 param0, U64 param1, U64 param2, U64 param3, U64 param4, U64 param5 )
{
	U64 param[] = { param0, param1, param2, param3, param4, param5 };
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kStoreAudit].m_storeAudit((auditId << 16) | hword, 0x80000006, (U32)&param[0] );
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiStoreAudit( U16 auditId, U16 numDwords, const void* pDwords )
{
	DisableInterrupts();
	g_jobContext->m_pJobApi[WwsJob_ApiCommand::kStoreAudit].m_storeAudit((auditId << 16) | numDwords, numDwords, (U32)pDwords );
	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

inline void* WwsJob_JobApiInitPlugin( const void* pluginAddr )
{
	typedef const void* (*PluginEntryPoint)( JobApiContext jobContextPtr );

	//U32 pluginSize			= apiReturn.m_bufferTag.GetLength();
	const SpuModuleHeader* pModuleHeader = (const SpuModuleHeader*) pluginAddr;
	PluginEntryPoint pluginEntryPoint = (PluginEntryPoint)(pModuleHeader->m_entryOffset + (U32)pluginAddr);

	//The SpuModuleHeader tells us where the PluginEntryPoint is.
	//We call the PluginEntryPoint so that global constructors get called.
	const void* pluginStart = pluginEntryPoint( g_jobContext );

	//Should return const really, but I'm having problems with gcc's warnings :(
	return const_cast<void*>( pluginStart );
}

//--------------------------------------------------------------------------------------------------

inline void* WwsJob_JobApiInitPlugin( WwsJob_BufferTag pluginBufferTag )
{
	return WwsJob_JobApiInitPlugin( pluginBufferTag.GetAddr() );
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiShutdownPlugin( const void* pluginAddr )
{
	//At present doesn't do anything.  Maybe we should call global destructors though?
	WWSJOB_UNUSED( pluginAddr );
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiShutdownPlugin( WwsJob_BufferTag pluginBufferTag )
{
	WwsJob_JobApiShutdownPlugin( pluginBufferTag.GetAddr() );
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_JobApiSetReadyCount( U32 eaSpurs, CellSpursWorkloadId workloadId, U32 readyCount )
{
	WWSJOB_UNUSED( eaSpurs );
	WwsJob_JobApiSetReadyCount( workloadId, readyCount );
}

//--------------------------------------------------------------------------------------------------

#define WWSJOB_UNINITIALIZED()	__attribute__(( section(".uninitialized,\"w\",@nobits#") ))

//--------------------------------------------------------------------------------------------------

namespace DmaTagId
{
	enum
	{
		//If a library needs to use a fixed tag number for an internal blocking load,
		//rather than being able to correctly allocate itself a tag, then it may use this tag.
		//Note that it's not impossible that this tag could be allocated to a job,
		//which would mean that the stall on this dma tag could take slightly longer
		//than intended, but this is hopefully an unlikely situation, which is why we
		//haven't reserved this tag as special.
		kBlockingLoadDmaTag = 8,
	};
};

//--------------------------------------------------------------------------------------------------

//} // namespace Job
//}; // namespace Wws

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_API_H */
