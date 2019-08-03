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
#include <jobapi/jobspudma.h>

//--------------------------------------------------------------------------------------------------

JobHeader WWSJOB_ALIGNED(128) g_jobHeaderCache[16];
Audit WWSJOB_ALIGNED( AUDIT_BLOCK_SIZE << 1 )  	g_auditBlocks[2][NUM_AUDITS_PER_BLOCK];

U32	WWSJOB_ALIGNED(16) g_lsaAudits;	// lsa where to store next audit in block (room DOES exist for 1 audit)
U32	WWSJOB_ALIGNED(16) g_mmaAudits;	// mma where to store next block of audits
U32	WWSJOB_ALIGNED(16) g_mmaAuditsBase;
U32	WWSJOB_ALIGNED(16) g_mmaAuditsEndLessAuditBlockSizePlus1;	// mma of end(exclusive) of audit memory ...
											// 		- AUDIT_BLOCK_SIZE + 1
Bool32 WWSJOB_ALIGNED(16) g_bJobAuditsEnabled			= false;
Bool32 WWSJOB_ALIGNED(16) g_bJobManagerAuditsEnabled	= false;
U32 WWSJOB_ALIGNED(16) g_mmaAuditBufferOutputHeader;


/**	\brief	contains 4 bytes of data per bufferSet, for each of 3 jobs

	A bufferSet is a continuous group of buffers of the same size.
	Each bufferSet can have any number (>= 1) of buffers as long as there
	is room to hold the buffers.  There are WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB bufferSets
	per each of 3 jobs.  For each job, the bufferSets are stored as 0 : WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB-1.
	They are init'd to inactive, and you can reserve (declare) them in and order
	you want (e.g. 5, 11, 2 which will create a sparse storage).
**/
WwsJob_BufferSet WWSJOB_ALIGNED(16) g_WwsJob_bufferSetArray[3/*jobs*/ * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB];

/**	\brief	contains 8 bytes of data per physical buffer, for each of 3 jobs

	Each job(0,1,2) has WWSJOB_MAX_NUM_BUFFERS_PER_JOB physical buffers allocated.
	Within each job, these buffers are reserved starting at 0
	when you reserve a bufferSet.  E.g. if you reserve bufferSet 6 with 3 buffers,
	and then reserve bufferSet 4 with 2 buffers,  your physical buffers (starting from 0)
	would be (in bufferSet:physical buffer notation): (6:0), (6:1), (6:2), (4:0), (4:1), and
	the remaining buffers would be free for further reservations or else to pass on shared
	buffers from a previous job to a future job.
**/
WwsJob_Buffer WWSJOB_ALIGNED(16) g_WwsJob_bufferArray[3/*jobs*/ * WWSJOB_MAX_NUM_BUFFERS_PER_JOB];

/**	\brief	each byte decodes the logical bufferNum used by the job,
			to the physical buffer# used within the bufferSet.

	When a new job starts loading, this array in set to FF's, which indicate inactive data.
	If, as above, you reserved buffers: (6:0), (6:1), (6:2), (4:0), and (4:1), and then
	'used' bufferSet 6, logical buffer 1 (which in this example returns the unused physical buffer 0),
	then this array would be: FF 00 FF FF FF ...  If you then used bufferSet 4, logical buffer 0
	and it returned the unused physical buffer 1, then this array would be: FF 00 FF 01 FF
**/
U8 WWSJOB_ALIGNED(16) g_WwsJob_logicalToBufferNumArray[3/*jobs*/ * WWSJOB_MAX_NUM_BUFFERS_PER_JOB];

/**	\brief	single buffer to hold commands for the loadJob.

	The commands execute before the loadJob is run, for the functions:
	reserving bufferSets, using buffers (dma read etc. or sharing existing buffers),
	or indicating this job is a general dependancy of another job.
**/
WwsJob_Command WWSJOB_ALIGNED(16) g_WwsJob_loadCommands[MAX_LOAD_COMMANDS_SIZE >> 3];



/** \brief	A bit per 1K page for memory used by SpursKernal, wwsjob manager, & stack

	This mask is set when the job starts, and never changes.
	It is used when the job shuts down to ensure the memory allocation has been
	returned to the initial state, to detect any memory leaks.
**/
QuadWord WWSJOB_ALIGNED(16) g_WwsJob_initialPageMask[2];	// value at initialization and shut down

/**	\brief	A bit per 1K page for memory currently used
**/
QuadWord WWSJOB_ALIGNED(16) g_WwsJob_usedPageMask[2];		// master "used      pages" mask

/** \brief	A bit per 1K page for memory which can be shared with other jobs

	If memory is in use by a job and can be shared with subsequent jobs, then
	both the 'used' and 'shareable' page bits will be on.  If the memory is no
	longer used by the job, but the buffer is hanging around in case it can be
	used by a future job, then the 'used' page bit is 0 and the 'shareable' page bit is 1.
	Note: any pages used by SpursKernal, wwsjob, or the stack (set in g_WwsJob_initialPageMask)
	will have the 'used' bit of 1 and 'shareable' bit of 0.
**/	
QuadWord WWSJOB_ALIGNED(16) g_WwsJob_shareablePageMask[2];	// master "shareable pages" mask

/** \brief	A bit per 1K page for memory which can be reserved for future use
	This mask allows the job to reserve memory which it may use in the future.
**/	
QuadWord WWSJOB_ALIGNED(16) g_WwsJob_reservedPageMask[2];	// master "reserved pages" mask

/**	\brief	A bit per dmaTagId which is used

	DmaTagId's of 0:31 correspond to bits 31:0 of s_usedDmaTagMask.
	The first 8 dmaTagId's (0:7) are used by the job manager, so the initial
	value of this mask is 0x000000ff.  When the user wants to use a dmaTagId
	in this case he would be given dmaTagId 31, so the mask would be 0x800000ff.
**/
volatile U32 WWSJOB_ALIGNED(16) g_WwsJob_usedDmaTagMask;		// master used dmaTagId mask



/**	\brief	keep track of what state the loadJob is in
**/
volatile U32 WWSJOB_ALIGNED(16) g_WwsJob_loadJobState;

/**	\brief	keep track of what state the runJob is in
**/
U32 WWSJOB_ALIGNED(16) g_WwsJob_runJobState;

/**	\brief	next job number (0:2) to load.  If you can't load any more, then hi 7 digits set to F (low digit kept)
**/
I32 WWSJOB_ALIGNED(16) g_WwsJob_nextLoadJobNum;

/**	\brief	job number (0:2) being loaded, of -1 if none
**/
I32 WWSJOB_ALIGNED(16) g_WwsJob_loadJobNum;

/**	\brief	job number (0:2) being run, or -1 if none
**/
I32 WWSJOB_ALIGNED(16) g_WwsJob_runJobNum;

/**	\brief	job number (0:2) which is storing dmas, or -1 if none
**/
volatile I32 WWSJOB_ALIGNED(16) g_WwsJob_storeJobNum;

/**	\brief	last job number (0:2) which stored dmas.

	At initialization, set to 2 (previous to job 0) so the code can work
**/
I32 WWSJOB_ALIGNED(16) g_WwsJob_lastStoreJobNum;

/**	\brief	ptr to 4 byte WwsJob_BufferSet for buffer containing code to run
**/
WwsJob_BufferSet *g_WwsJob_pJobCodeBufferSet;

/**	\brief	ptr to 8 byte WwsJob_Buffer for buffer containing code to run
**/
WwsJob_Buffer WWSJOB_ALIGNED(16) *g_WwsJob_pJobCodeBuffer;

/**	\brief	local store adrs of buffer containing code to run
**/
U32 WWSJOB_ALIGNED(16) g_WwsJob_lsaJobCodeBuffer;

/**	\brief	sole structure passed to job code when it is started

	This contains misc things the job will need
**/
WwsJob_DataForJob WWSJOB_ALIGNED(16) g_WwsJob_dataForJob;

/**	\brief	count of # of dmaTagId's the job took (which were not
			assigned to a buffer via the jobApi GetLogicalBuffer(s)

	If a job forgets to free dmaTagId's that it took then this count
	will allow the job manager to assert so the job can be fixed.
**/
U32 WWSJOB_ALIGNED(16) g_WwsJob_numUnassignedDmaTagIds;

/**	\brief	next free buffer number (from the start of the *entire* array) to take

	There are (3 * WWSJOB_MAX_NUM_BUFFERS_PER_JOB) entries in g_WwsJob_bufferArray[] and
	in g_WwsJob_logicalToBufferNumArray[].  This is the index (from the start of the *entire* array)
	to the next buffer to use (by reserveBufferSet).
**/
U32 WWSJOB_ALIGNED(16) g_WwsJob_firstBufferNum;

/** \brief	Number of available buffers for job

	Number of available buffers in g_WwsJob_bufferArray[] and g_WwsJob_logicalToBufferNumArray[]
**/
U32 WWSJOB_ALIGNED(16) g_WwsJob_numFreeBuffers;
	
/**	\brief	load command can cause this to be set, which will breakpoint
			just before the job starts running
**/
Bool32 WWSJOB_ALIGNED(16) g_WwsJob_bBreakpointRequested;

/**	\brief	increments each time a job is loaded

	Used to determine how old buffers are in case of a tie in trying
	to take a shareable but unused buffer.
**/
U8 WWSJOB_ALIGNED(16) g_WwsJob_timeStamp;

// to interface with SpursKernel
U32			WWSJOB_ALIGNED(16) g_WwsJob_eaWorkLoad;
U32			WWSJOB_ALIGNED(16) g_WwsJob_spursWorkloadId;
U32			WWSJOB_ALIGNED(16) g_WwsJob_jobIndex;			// job index set if job was taken (ignored if not taken)
JobHeader  	WWSJOB_ALIGNED(16) g_WwsJob_jobHeader;
WwsJob_JobData		WWSJOB_ALIGNED(16) g_WwsJob_jobDataArray[3/*jobs*/];

//Hmm... Does the g_currentJobHeaderCacheEa need resetting to NULL at entry to the PM?
//If we're re-run twice in a row, then the fact the g_currentJobHeaderCacheEa is non-null
// should indeed correctly mean that the g_jobHeaderCache has appropriate data in.
//Therefore we don't need to reset g_currentJobHeaderCacheEa.

//NO! That comment above IS NOT THE WHOLE STORY.  Suppose the list has one job in, and we
//run it.  Then the PPU resets the list and we're rescheduled to work on it.  This cache may
//be wrong. Therefore, we *must* reset the cache at the start of each run of the job manager.

U32 WWSJOB_ALIGNED(16) g_currentJobHeaderCacheEa = 0;

DependencyCounter WWSJOB_ALIGNED(16) g_dependencyCache[4];
U32 WWSJOB_ALIGNED(16) g_currentDependencyCacheEa = 0;
VU32 WWSJOB_ALIGNED(16) g_sideStack[2];

const JobDmaListElement g_nullDmaInterrupt[2] WWSJOB_ALIGNED(16) =
{
	{	0x80000000, 0x00000000 },	{	0x00000000,	0x00000000 }
};

VU32 WWSJOB_ALIGNED(16) g_BisledRegisterStore;

U32 WWSJOB_ALIGNED(16) g_countToNextPoll;

extern "C" const U32 WWSJOB_ALIGNED(16) g_stackSetupValue[4] = { 0x3FFD0, 0x1FC0, 0, 0 };

//--------------------------------------------------------------------------------------------------
