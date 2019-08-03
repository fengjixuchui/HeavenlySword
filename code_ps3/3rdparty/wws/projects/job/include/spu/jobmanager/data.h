/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		The data used by the job manager SPU Policy Module
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_DATA_H
#define WWS_JOB_DATA_H

//--------------------------------------------------------------------------------------------------

#define NUM_AUDITS_PER_BLOCK	16
#define AUDIT_BLOCK_SIZE ( sizeof(Audit) * (NUM_AUDITS_PER_BLOCK) )

//--------------------------------------------------------------------------------------------------

extern "C" U32 WWSJOB_ALIGNED(16) g_lsaAudits;	// lsa where to store next audit in block (room DOES exist for 1 audit)
extern "C" U32 WWSJOB_ALIGNED(16) g_mmaAudits;	// mma where to store next block of audits
extern "C" U32 WWSJOB_ALIGNED(16) g_mmaAuditsBase;
extern "C" U32 WWSJOB_ALIGNED(16) g_mmaAuditsEndLessAuditBlockSizePlus1;	// mma of end(exclusive) of audit memory ...
																		// 		- AUDIT_BLOCK_SIZE + 1
extern "C" Bool32 g_bJobAuditsEnabled;
extern "C" Bool32 g_bJobManagerAuditsEnabled;
extern "C" U32 WWSJOB_ALIGNED(16) g_mmaAuditBufferOutputHeader;

extern "C" volatile U32 WWSJOB_ALIGNED(16) g_WwsJob_usedDmaTagMask;		// master used dmaTagId mask
extern "C" Audit WWSJOB_ALIGNED( AUDIT_BLOCK_SIZE << 1 ) g_auditBlocks[2][NUM_AUDITS_PER_BLOCK];
extern "C" WwsJob_BufferSet WWSJOB_ALIGNED(16) g_WwsJob_bufferSetArray[3/*jobs*/ * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB];
extern "C" WwsJob_Buffer WWSJOB_ALIGNED(16) g_WwsJob_bufferArray[3/*jobs*/ * WWSJOB_MAX_NUM_BUFFERS_PER_JOB];
extern "C" U8 WWSJOB_ALIGNED(16) g_WwsJob_logicalToBufferNumArray[3/*jobs*/ * WWSJOB_MAX_NUM_BUFFERS_PER_JOB];
extern "C" WwsJob_Command WWSJOB_ALIGNED(16) g_WwsJob_loadCommands[MAX_LOAD_COMMANDS_SIZE >> 3];
extern "C" QuadWord WWSJOB_ALIGNED(16) g_WwsJob_initialPageMask[2];	// value at initialization and shut down
extern "C" QuadWord WWSJOB_ALIGNED(16) g_WwsJob_usedPageMask[2];		// master "used      pages" mask
extern "C" QuadWord WWSJOB_ALIGNED(16) g_WwsJob_shareablePageMask[2];	// master "shareable pages" mask
extern "C" QuadWord WWSJOB_ALIGNED(16) g_WwsJob_reservedPageMask[2];	// master "reserved pages" mask
extern "C" volatile U32 WWSJOB_ALIGNED(16) g_WwsJob_usedDmaTagMask;		// master used dmaTagId mask
extern "C" volatile U32 WWSJOB_ALIGNED(16) g_WwsJob_loadJobState;
extern "C" U32 WWSJOB_ALIGNED(16) g_WwsJob_runJobState;
extern "C" I32 WWSJOB_ALIGNED(16) g_WwsJob_nextLoadJobNum;
extern "C" I32 WWSJOB_ALIGNED(16) g_WwsJob_loadJobNum;
extern "C" I32 WWSJOB_ALIGNED(16) g_WwsJob_runJobNum;
extern "C" volatile I32 WWSJOB_ALIGNED(16) g_WwsJob_storeJobNum;
extern "C" I32 WWSJOB_ALIGNED(16) g_WwsJob_lastStoreJobNum;
extern "C" WwsJob_BufferSet *g_WwsJob_pJobCodeBufferSet;
extern "C" WwsJob_Buffer WWSJOB_ALIGNED(16) *g_WwsJob_pJobCodeBuffer;
extern "C" U32 WWSJOB_ALIGNED(16) g_WwsJob_lsaJobCodeBuffer;
extern "C" WwsJob_DataForJob WWSJOB_ALIGNED(16) g_WwsJob_dataForJob;
extern "C" U32 WWSJOB_ALIGNED(16) g_WwsJob_numUnassignedDmaTagIds;
extern "C" U32 WWSJOB_ALIGNED(16) g_WwsJob_firstBufferNum;
extern "C" U32 WWSJOB_ALIGNED(16) g_WwsJob_numFreeBuffers;
extern "C" Bool32 WWSJOB_ALIGNED(16) g_WwsJob_bBreakpointRequested;
extern "C" U8 WWSJOB_ALIGNED(16) g_WwsJob_timeStamp;

// to interface with SpursKernel
extern "C" U32				WWSJOB_ALIGNED(16) g_WwsJob_eaWorkLoad;
extern "C" U32				WWSJOB_ALIGNED(16) g_WwsJob_spursWorkloadId;
extern "C" U32				WWSJOB_ALIGNED(16) g_WwsJob_jobIndex;			// job index set if job was taken (ignored if not taken)
extern "C" JobHeader  		WWSJOB_ALIGNED(16) g_WwsJob_jobHeader;
extern "C" WwsJob_JobData	WWSJOB_ALIGNED(16) g_WwsJob_jobDataArray[3/*jobs*/];

extern "C" JobHeader g_jobHeaderCache[16] WWSJOB_ALIGNED(128);
extern "C" U32 g_currentJobHeaderCacheEa WWSJOB_ALIGNED(16);

extern "C" const struct JobDmaListElement g_nullDmaInterrupt[2] WWSJOB_ALIGNED(16);

//May be shared by various systems for doing atomic transaction, but data cannot be left in the buffer
//This memory buffer is actually now declared in the linker script because we share the same atomic buffer as spurs uses at 0x80
extern "C" SharedTempAtomicBuffer g_tempUsageAtomicBuffer WWSJOB_ALIGNED(128);

extern "C" DependencyCounter g_dependencyCache[4] WWSJOB_ALIGNED(16);
extern "C" U32 g_currentDependencyCacheEa;
extern "C" VU32 WWSJOB_ALIGNED(16) g_sideStack[2];

extern "C" U32 WWSJOB_ALIGNED(16) g_countToNextPoll;

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_DATA_H */
