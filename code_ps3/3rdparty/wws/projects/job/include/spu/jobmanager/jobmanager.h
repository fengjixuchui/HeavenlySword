/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		The main core of the job manager SPU Policy Module
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_MANAGER_H
#define WWS_JOB_JOB_MANAGER_H

//--------------------------------------------------------------------------------------------------

// defines affecting size of JobManager =======================================================================
// defines & structures =======================================================================================

#define WWSJOB_TRUE ~0
#define WWSJOB_FALSE 0

//--------------------------------------------------------------------------------------------------

static const U32 WwsJob_kMaxDependenciesPerJob = 4;

//--------------------------------------------------------------------------------------------------

// ptr to job code
//Note that the return value tells us how long the job spent in init and shutdown in the jobcrt0.spu.s
typedef U32 (*WwsJob_JobCodePtr)( const WwsJob_DataForJob* pDataForJob, Bool32 bBreakpoint );

//--------------------------------------------------------------------------------------------------

union WwsJob_Buffer		// 8 bytes, 8 byte aligned.  Note all 0's stored in 1st word when inactive
{
	struct
	{
		unsigned int	m_reserved					:1;		// if 1 then all other bits are 0
		unsigned int	m_shareable					:1;		// this is set to 0 when sharedByLaterJob
		unsigned int	m_shareableWriteIfDiscarded	:1;		// may only be 1 if shareable is 1
		unsigned int	m_sharedToLaterJob			:1;		// if 1 then later job responsible for clearing used/shareable page bits
		unsigned int	m_used						:1;		// 1 if job must free used pages when done
		unsigned int	m_dmaTagId					:5;		// 0 means no unique dmaTagId alloc'd
		unsigned int	m_mmLengthInQwords			:14;
		unsigned int	m_timeStamp					:8;		// NOTE: code assumes this is 8 bits
			
		unsigned int	m_mmAddressInQwords			:28;
		unsigned int	m_bufferSetNum				:4;		// bufferSetNum
	};
	U64		m_u64;
	U32		m_u32[2];
	U16		m_u16[4];
	U8		m_u8 [8];
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

namespace WwsJob_LoadJobState
{
	enum
	{
		kNone = 0,			///< There is no loadJob	(this MUST be 0)
		kReadCommands,		///< The loadJob is reading the LoadCommands
		kExecuteCommands,	///< The loadJob is executing the LoadCommands
		kCommandsExecuted	///< The loadJob has finished executing the LoadCommands
	};
}

//--------------------------------------------------------------------------------------------------

namespace WwsJob_RunJobState
{
	enum
	{
		kNone = 0,			///< There is no runJob		(this MUST be 0)
		kLoadNotAllowed,	///< The runJob has not yet allowed the next loadJob
		kLoadAllowed,		///< The runJob has allowed the next loadJob
	};
}

//--------------------------------------------------------------------------------------------------

struct WwsJob_JobData	// 32 bytes, 16 byte aligned
{
	U32					m_eaWorkLoad;
	U16					m_firstFreeBufferNum; // first free buffer#, including job#
	U16					m_jobHasShareableBuffers;	// 0 after powerup, non-zero if a job might have shareable buffers
	U32					m_jobIndex;		///< job index in workload
	U32					m_numDependencies;
	U32					m_eaDependency[WwsJob_kMaxDependenciesPerJob];
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

// structure added for use by GetLogicalBuffer
// The previous vars which were passed as references are stuffed into this structure
struct WwsJob_MiscData
{
	WwsJob_BufferSet	WWSJOB_ALIGNED( 16 ) *m_pBufferSet;
	WwsJob_BufferSet	WWSJOB_ALIGNED( 16 ) m_bufferSet;
	U32 				WWSJOB_ALIGNED( 16 ) m_bufferSetFirstPageNum;
	U32 				WWSJOB_ALIGNED( 16 ) m_numPagesPerBuffer;
	U32 				WWSJOB_ALIGNED( 16 ) m_numBuffers;
    WwsJob_Buffer 		WWSJOB_ALIGNED( 16 ) *m_pBuffers;
	U32 				WWSJOB_ALIGNED( 16 ) m_bufferNum;
	WwsJob_Buffer 		WWSJOB_ALIGNED( 16 ) *m_pBuffer;
	WwsJob_Buffer 		WWSJOB_ALIGNED( 16 ) m_buffer;
	U32 				WWSJOB_ALIGNED( 16 ) m_bufferPageNum;
	U8 					WWSJOB_ALIGNED( 16 ) *m_pLogicalToBufferNums;
}WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_MANAGER_H */
