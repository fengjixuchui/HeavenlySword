/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_BATCHJOBLIST_H
#define ICE_BATCHJOBLIST_H

#include "icebatchjob.h"

#if ICE_TARGET_PS3_PPU
#  include "jobapi/joblist.h"
#  include "jobapi/spumodule.h"
#  include "jobapi/commandlistbuilder.h"
struct CellSpurs;
class AuditManager;
#endif

namespace Ice
{
    namespace BatchJob
    {
		//! SPUs 0-5 are available for use, currently
		const U32 kNumSpusAvailable = 6;

		//! A JobId is a unique identifier for a job within a job block
		typedef U32 JobId;
		const JobId kJobIdInvalid = (JobId)-1;

		//! A JobBlockState encodes the current state of a BatchJobList with respect to defining and running a block of jobs
		enum JobBlockState {
			kJobBlockUninitialized = -1,//!< This job list has not been initialized
			kJobBlockCompleted = 0,		//!< No tasks are currently running and no block has been started
			kJobBlockQueuing,			//!< between BeginSpuJobs and EndSpuJobs; tasks may be running, but no end has been defined
			kJobBlockRunning,			//!< EndSpuJobs has been called but tasks are still running
			kJobBlockFull,				//!< No room for any jobs to queue right now
		};

#if ICE_TARGET_PS3_PPU
		//! A BatchJobDesc contains a description of a batch job's constant properties
		struct BatchJobDesc {
			BatchJobBufferDesc bufferDesc;	//!< description of our buffer layout
			U32 maxOutstandingJobs;			//!< maximum number of jobs we'll allow to be outstanding at once before AddJob will block
			SpuModuleHandle jobModule;		//!< a handle to the job binary code
			U32 maxContention;				//!< max number of spus to run this batch job on at once
			U8 spuPriorities[8];			//!< priority to run on each SPU; 1-15 where 1 is highest or 0 to disable
		};

		/*!
		 *	A BatchJobList encapsulates the handling of a list of jobs for the job manager.
		 *
		 *	It provides an interface to define a block of jobs and to poll or wait for their
		 *  completion as a block or individually.
		 */
		class BatchJobList {
		public:
			BatchJobList(BatchJobDesc const& jobDesc);
			~BatchJobList();

			/*!
			 * Initialize batch job list on SPUs
			 */
			void Init(CellSpurs *pSpurs, AuditManager *pAuditManager = NULL);
			/*!
			 * Clean up  batch job list data for SPU jobs
			 */
			void Terminate();

			/*!
			 * Begin a block of batch jobs for the SPU.
			 *
			 * Only one block of jobs need be defined per frame, but multiple
			 * blocks may be defined for convenience.
			 *
			 * Blocks may not overlap; that is, you may not begin a new block
			 * until you have ended the previous block with EndJobBlock()
			 * and verified that it has completed with IsJobBlockCompleted()
			 * or WaitForJobBlock().
			 *
			 * Blocks are not limitted in size, but there is a maximum number
			 * that may be pending for execution at once and there is a maximum
			 * total size of initialization data that can be queued - should  
			 * you manage to submit tasks fast enough that there is no room for
			 * another to be added, AddJob() may block until room becomes 
			 * available.  To avoid blocking, you may check the return value of 
			 * IsRoomForJob() before calling AddJob().
			 */
			void BeginJobBlock();
			/*!
			 * End a block of jobs for the SPU.
			 */
			void EndJobBlock();

			/*!
			 * Wait for all tasks in the current _ended_ block of SPU jobs to 
			 * complete.  WaitForJobBlock() can not be called before 
			 * EndJobBlock() has been called to define the end of the block.
			 */
			void WaitForJobBlock();
			/*!
			 * Check if all jobs in the current _ended_ block of SPU jobs have
			 * completed.  Even if all tasks that have so far been submitted 
			 * have completed, IsJobBlockCompleted() always returns false 
			 * until EndJobBlock() has been used to end the block.
			 */
			bool IsJobBlockCompleted();

			/*!
			 * Wait for a particular job in the current block to complete.
			 * This may be called before or after EndJobBlock(), as it does
			 * not require the current block to be ended.
			 */
			void WaitForJob(JobId jobId);
			/*!
			 * Check if a particular job in the current block has completed.
			 * This may be called before or after EndJobBlock(), as it does
			 * not require the current block to be ended.
			 */
			bool IsJobCompleted(JobId jobId);

			/*!
			 * Check if the given task can now be added with AddJob(), and
			 * return false if there is no room available to add the job.
			 */
			bool IsRoomForJob(U32 initialDmaListSize);
			/*!
			 * Add a job to the current block of SPU jobs, to be primed with
			 * the given DMA list, and return the resulting JobId.
			 *
			 * Note that this operation may block until IsRoomForJob()
			 * returns true if there is not currently room to add the job.
			 */
			JobId AddJob(U32 const* pInitialDmaList, U32 initialDmaListSize);

			/*!
			 * Returns the current state of the batch job list.
			 */
			JobBlockState GetJobBlockState();
			/*!
			 * Updates the double buffered task lists by kicking the next list
			 * whenever the current running list completes.
			 * Note that most other functions call Poll(), and so calling it
			 * is generally unnecessary.  It may however be useful to call 
			 * directly if you plan on running unrelated code for a significant
			 * period of time in the middle of defining a job block.
			 */
			void Poll();

		private:
			static const U32F kJobIdBufferIndexShift = 8;
			static const U32F kNumCommandsPerJob = 	6;				// kCommandsPerJob * sizeof(WwsJob_Command) must be a multiple of 16 bytes

			SingleThreadJobList*	m_activeJobList[2];
			U8*						m_jobListBuffer[2];
			WwsJob_Command*			m_commandBuffer[2];
			JobListMarker*			m_activeJobMarker[2];

			BatchJobDesc			m_jobDesc;
			U32						m_jobDataBufferLSA;
	
			U32F					m_numTotalJobs;				//!< total number of jobs that have been added since BeginJobs
			U32F					m_runJobList;				//!< index of buffer that is currently running, which may fall behind s_addJobList by up to 1 buffer
			U32F					m_addJobList;				//!< running index of buffers started since BeginJobs; (m_addJobList & 1) is the current set of buffers that are in use
			U32F					m_numJobsInAddJobList;		//!< how many jobs have been added to the active buffer
			JobBlockState			m_eJobBlockState;
		};
#endif	// if ICE_TARGET_PS3_PPU
	}
};

#endif //ICE_BATCHJOBLIST_H
