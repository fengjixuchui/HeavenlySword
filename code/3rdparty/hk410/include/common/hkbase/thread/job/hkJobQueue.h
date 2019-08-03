/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_BASE_THREAD_JOB_QUEUE_H
#define HK_BASE_THREAD_JOB_QUEUE_H

#include <hkbase/thread/hkCriticalSection.h>
#include <hkbase/thread/hkSemaphoreBusyWait.h>
#include <hkbase/thread/hkSpuUtils.h>

#if defined(HK_PLATFORM_PS3SPU) 
#include <hkbase/htl/hkFixedSizeQueue.h>
# define HK_QUEUE hkFixedSizeQueue
# else
#include <hkbase/htl/hkQueue.h>
# define HK_QUEUE hkQueue
# endif

struct ThreadFlags
{
	enum Flag
	{
		DO_NOT_PROCESS,
		PROCESS,
	};

	void getThreadFlag( int threadId, Flag& flag )
	{
		if (threadId == -1)
		{
			flag = PROCESS;
			return;
		}
		if ( (m_flags & (1 << threadId)) == 0)
		{
			flag = DO_NOT_PROCESS;
		}
		else
		{
			flag = PROCESS;
		}
	}

	void setThreadFlag( int threadId, Flag flag )
	{
		if (flag == PROCESS)
		{
			m_flags = m_flags | (1 << threadId );
		}
		else
		{
			m_flags = m_flags & ~(1 << threadId );
		}
	}

	hkUint32 m_flags;
};
	

//
// Data to transfer per atomic call
//

	/// This class implements a job queue with all necessary locking and waiting
class hkJobQueue: public hkSynchronized
{
	public:

			/// The basic struct stored in the job queue
		struct JobQueueEntry
		{
			HK_ALIGN16(hkUchar m_data[128]);
		};

		struct DynamicData
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, DynamicData );

				// The number of jobs which are currently processed
			HK_ALIGN16(int m_numActiveJobs);

				// The number of threads waiting for jobs using the m_waitSemaphore
			int m_numCpuThreadsWaitingForJobs;
			int m_numSpuThreadsWaitingForJobs;

				// if this value is not 0, there could be another spu job appearing on the queue
			int m_moreSpuJobsPossible;

			ThreadFlags m_flags;

			HK_QUEUE< JobQueueEntry > m_jobQueue[3];
		};


		//
		// Enums to specify job types
		//

			/// Types of job that can be placed on and retrieved from the job queue
		enum JobType
		{
			JOB_TYPE_CPU_PRIMARY = 0,
			JOB_TYPE_CPU = 1,
#if defined (HK_PLATFORM_HAS_SPU)
			JOB_TYPE_SPU = 2,
			JOB_TYPE_HINT_SPU = 2,
			JOB_TYPE_MAX = 3,
#else
			JOB_TYPE_HINT_SPU = 1,
			JOB_TYPE_MAX = 2,
#endif

		};

			/// Types of threads that can place jobs on and retrieve jobs from the job queue
		enum ThreadType
		{
			THREAD_TYPE_CPU_PRIMARY = JOB_TYPE_CPU_PRIMARY,
			THREAD_TYPE_CPU = JOB_TYPE_CPU,
#if defined (HK_PLATFORM_HAS_SPU)
			THREAD_TYPE_SPU = JOB_TYPE_SPU,
#endif
			THREAD_TYPE_MAX,
			THREAD_TYPE_INVALID = -1,
		};

			/// Whether a job is placed at the front or the end of its appropriate queue
		enum JobPriority
		{
			JOB_HIGH_PRIORITY,
			JOB_LOW_PRIORITY,
		};


		//
		// Callback functions used from within critical sections
		//


				/// The result from JobPopFunc
			enum JobPopFuncResult
			{
					/// Set to this if the function is taking the element off the queue
				POP_QUEUE_ENTRY,
					/// Set to this if the function has modified the element, and left it on the queue
				DO_NOT_POP_QUEUE_ENTRY 
			};

			typedef void             (HK_CALL *JobAddFunc)( hkJobQueue& queue, hkJobQueue::DynamicData* data, const JobQueueEntry& jobIn );
			typedef JobPopFuncResult (HK_CALL *JobPopFunc)( hkJobQueue& queue, hkJobQueue::DynamicData* data, JobQueueEntry& jobIn, JobQueueEntry& jobOut );

			enum JobCreationStatus
			{
				JOB_CREATED,
				NO_JOB_CREATED
			};

			struct JobQueueEntryInput
			{
				JobType m_jobThreadType;
				JobPriority m_jobPriority;
				JobQueueEntry m_job;
			};

		typedef JobCreationStatus (HK_CALL *FinishJobFunc)( hkJobQueue& queue, DynamicData* data, const JobQueueEntry& jobIn, JobQueueEntryInput& newJobCreatedOut );


	public:

			/// Note: this class should only ever be constructed on a CPU. If running on an SPU it must be dma'd into an
			/// appropriately sized 128 byte aligned local store buffer.
		hkJobQueue();

			// empty constructor
		hkJobQueue( hkFinishLoadedObjectFlag f) : hkSynchronized(0), m_cpuWaitSemaphore(f), m_spuWaitSemaphore(f), m_taskCompletionSemaphore(f) {}

		~hkJobQueue();

		//
		// Runtime functions
		//


			/// Adds a new job to the queue for a given priority and job thread type.
		void addJob( JobQueueEntry& job, JobPriority priority, JobType type );

		
				/// A flag to tell getNextJob what to do if no job is immediately available
			enum WaitStatus
			{
				WAIT_FOR_NEXT_JOB,
				DO_NOT_WAIT_FOR_NEXT_JOB
			};

				/// Whether a getNextJob call got another job or not
			enum JobStatus
			{
				GOT_NEXT_JOB,
				NO_JOBS_AVAILABLE,
				ALL_JOBS_FINISHED
			};

			/// Gets a new job. Note: this function should only be called at the start of the main loop.
			/// If you can you should call addAndGetNextJob
		JobStatus getNextJob( ThreadType threadType, JobQueueEntry& job, WaitStatus waitStatus = WAIT_FOR_NEXT_JOB, int spuId = -1 );


		//
		// Compound functions
		//

			/// Call this when one job is finished and you are not calling addJobAndGetNextJob
		JobStatus finishJobAndGetNextJob( ThreadType threadType, const JobQueueEntry* oldJob, JobQueueEntry& jobOut, void* params = HK_NULL, WaitStatus waitStatus = WAIT_FOR_NEXT_JOB, int spuId = -1 );

			/// Finishes a job and gets a new one
			/// This is faster than calling addJob and getNextJob separately, because it can do the
			/// entire operation in one critical section, rather than two critical sections.
		JobStatus addJobAndGetNextJob( JobType jobThreadType, JobPriority priority, ThreadType threadType, JobQueueEntry& jobInOut, WaitStatus waitStatus = WAIT_FOR_NEXT_JOB );

		void setSpuQueueCapacity(int queueCapacity);

	protected:
			// Call this to release all waiting threads
		void releaseWaitingThreads(DynamicData* data);

	public:


		HK_CPU_PTR(DynamicData*) m_data;

		bool m_isLocked;

		//
		// Data to transfer once
		//
		
			// A semaphore used to signal waiting jobs
		hkSemaphoreBusyWait m_cpuWaitSemaphore;
		hkSemaphoreBusyWait m_spuWaitSemaphore;


		hkSemaphoreBusyWait m_taskCompletionSemaphore;


		//
		// Static, locally set data
		//

		struct JobQueryRule
		{
			hkUint8 m_numJobTypesAvailable;
			hkEnum<JobType, hkUint8> m_jobTypeOrder[JOB_TYPE_MAX];
		};

		JobQueryRule m_jobQueryRules[JOB_TYPE_MAX];

		JobAddFunc m_jobAddFunc;
		JobPopFunc m_jobPopFunc; 
		FinishJobFunc m_finishJobFunc;

			// this userdata is pointing to hkWorld if the queue is used by havok physics
		void* m_userData;

		void* m_buffer;

	public:


			// Check if a thread is waiting for a job on a semaphore.
			// If one thread is, release the semaphore
		void releaseOneWaitingThread( JobType jobThreadType, DynamicData* data  );

			// Check if a thread with the given threadType is waiting for a job on a semaphore.
			// If one thread is, release the semaphore
		HK_FORCE_INLINE void checkQueueAndReleaseOneWaitingThread( ThreadType threadType, DynamicData* data  );

		HK_FORCE_INLINE JobStatus findNextJob(ThreadType threadType, JobQueueEntry& jobOut, DynamicData* data );
		
		HK_FORCE_INLINE hkBool allQueuesEmpty( DynamicData* data );

		DynamicData* lockQueue( );
		void unlockQueue( );

};


#endif // HK_BASE_THREAD_JOB_QUEUE_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
