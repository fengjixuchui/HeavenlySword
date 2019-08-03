/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSpuUtils.h>

#include <hkbase/thread/hkSpuDmaManager.h>

#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
#	include <hkbase/thread/util/hkSpuSimulator.h>
#endif

#include <hkbase/thread/job/hkJobQueue.h>
#include <hkbase/monitor/hkMonitorStream.h>


/*
#include <stdio.h>
void printData(hkJobQueue::DynamicData& data)
{
	printf("\nJOB QUEUE DATA:\n");
	printf("data.m_numActiveJobs = %d\n", data.m_numActiveJobs);
	printf("data.m_numCpuThreadsWaitingForJobs = %d\n" , data.m_numCpuThreadsWaitingForJobs);
	printf("data.m_numSpuThreadsWaitingForJobs = %d\n" , data.m_numSpuThreadsWaitingForJobs);
	printf("data.m_jobQueue[0].m_elementsInUse = %d\n" , data.m_jobQueue[0].getSize());
	printf("data.m_jobQueue[1].m_elementsInUse = %d\n" , data.m_jobQueue[1].getSize());
	printf("data.m_jobQueue[1].m_capacity = %d\n" , data.m_jobQueue[1].getCapacity());
	printf("data.m_jobQueue[2].m_elementsInUse = %d\n" , data.m_jobQueue[2].getSize());
}
*/

#if !defined(HK_PLATFORM_PS3SPU) 
hkJobQueue::hkJobQueue( ) : hkSynchronized(0), m_cpuWaitSemaphore(0,1000), m_spuWaitSemaphore(0,1000), m_taskCompletionSemaphore(0, 1000)
{
	m_isLocked = false;
	m_buffer = HK_NULL;
	m_data = new DynamicData();
	m_data->m_flags.m_flags = 0xffffffff;

	m_data->m_numCpuThreadsWaitingForJobs = 0;
	m_data->m_numSpuThreadsWaitingForJobs = 0;
	m_data->m_numActiveJobs = 0;
	m_data->m_moreSpuJobsPossible = 0;

	// Every time the queue is resized from the PPU, it will make sure enough free slots are available (which SPUs can then fill)
	// This capacity could be dynamically resized based on the number of elements in the SPU queue
	m_data->m_jobQueue[1].setInitialCapacity(128);

	// Fill out the job query rules
	m_jobQueryRules[(int)THREAD_TYPE_CPU_PRIMARY].m_numJobTypesAvailable = JOB_TYPE_MAX;
	m_jobQueryRules[(int)THREAD_TYPE_CPU_PRIMARY].m_jobTypeOrder[JOB_TYPE_HINT_SPU] = JOB_TYPE_HINT_SPU;
	m_jobQueryRules[(int)THREAD_TYPE_CPU_PRIMARY].m_jobTypeOrder[0]                 = JOB_TYPE_CPU_PRIMARY;
	m_jobQueryRules[(int)THREAD_TYPE_CPU_PRIMARY].m_jobTypeOrder[1]                 = JOB_TYPE_CPU;

	m_jobQueryRules[(int)THREAD_TYPE_CPU].m_numJobTypesAvailable = JOB_TYPE_MAX;
	m_jobQueryRules[(int)THREAD_TYPE_CPU].m_jobTypeOrder[JOB_TYPE_HINT_SPU] = JOB_TYPE_HINT_SPU;
	m_jobQueryRules[(int)THREAD_TYPE_CPU].m_jobTypeOrder[0]                 = JOB_TYPE_CPU;
	m_jobQueryRules[(int)THREAD_TYPE_CPU].m_jobTypeOrder[1]                 = JOB_TYPE_CPU_PRIMARY;

	#if defined (HK_PLATFORM_HAS_SPU)
		m_jobQueryRules[(int)THREAD_TYPE_SPU].m_numJobTypesAvailable = 1;
		m_jobQueryRules[(int)THREAD_TYPE_SPU].m_jobTypeOrder[0] = JOB_TYPE_SPU;
	#endif

}

hkJobQueue::~hkJobQueue()
{
	delete m_data;
}
#endif


static HK_ALIGN(char hkJobQueue_dataStorage[128], 128);

HK_COMPILE_TIME_ASSERT(128 >= sizeof( hkJobQueue::DynamicData ) );


hkJobQueue::DynamicData* hkJobQueue::lockQueue( )
{
	lockMt();

	// If this is called recursively, the DMA will overwrite data
	HK_ASSERT(0x64964964, m_isLocked == false);
	HK_ON_DEBUG(m_isLocked = true;)

	if ( HK_PLATFORM_IS_SPU )
	{
		hkSpuDmaManager::getFromMainMemory( hkJobQueue_dataStorage, m_data, sizeof(DynamicData), hkSpuDmaManager::READ_WRITE);
			// we cannot combine this call with the above function, as we are waiting for all dmas
		hkSpuDmaManager::waitForAllDmaCompletion();
		return (DynamicData*)hkJobQueue_dataStorage;
	}
	return  m_data;
}

void hkJobQueue::unlockQueue( )
{
	HK_ON_DEBUG(m_isLocked = false;)

	if (HK_PLATFORM_IS_SPU)
	{
		hkSpuDmaManager::putToMainMemoryAndWaitForCompletion( m_data, hkJobQueue_dataStorage, sizeof(DynamicData), hkSpuDmaManager::WRITE_BACK);
		hkSpuDmaManager::performFinalChecks( m_data, hkJobQueue_dataStorage, sizeof(DynamicData));
	}
	unlockMt();

}


hkJobQueue::JobStatus hkJobQueue::getNextJob( ThreadType threadType, JobQueueEntry& job, WaitStatus waitStatus, int spuId )
{ 
	return finishJobAndGetNextJob( threadType, HK_NULL, job, HK_NULL, waitStatus, spuId); 
}


// TODO - job status -> GetNextJobStatus
// JobType -> JobType
// Inl file
// remove hkDynamicsJob

void hkJobQueue::releaseOneWaitingThread( JobType jobThreadType, DynamicData* data )
{
	// WARNING: THIS FUNCTION MUST ALWAYS BE CALLED WHEN THE MT CRITICAL SECTION IS LOCKED
	// TODO - add isLocked to critical section and add assert
#if defined(HK_PLATFORM_HAS_SPU)
	if (jobThreadType == JOB_TYPE_SPU)
	{
		if ( data->m_numSpuThreadsWaitingForJobs  )
		{
			data->m_numSpuThreadsWaitingForJobs--;
			m_spuWaitSemaphore.release();
		}
	}
	else
#endif
	{
		if ( data->m_numCpuThreadsWaitingForJobs  )
		{
			int numCpuJobs = data->m_jobQueue[JOB_TYPE_CPU].getSize() + data->m_jobQueue[JOB_TYPE_CPU_PRIMARY].getSize();
			if ( numCpuJobs  )
			{
				data->m_numCpuThreadsWaitingForJobs--;
				m_cpuWaitSemaphore.release();
			}
		}
	}
}

void hkJobQueue::checkQueueAndReleaseOneWaitingThread( ThreadType threadType, DynamicData* data )
{
	// WARNING: THIS FUNCTION MUST ALWAYS BE CALLED WHEN THE MT CRITICAL SECTION IS LOCKED
	// TODO - add isLocked to critical section and add assert
#if defined(HK_PLATFORM_HAS_SPU)
	if (threadType == THREAD_TYPE_SPU)
	{
		if ( !data->m_jobQueue[JOB_TYPE_SPU].isEmpty() )
		{
			if ( data->m_numSpuThreadsWaitingForJobs  )
			{
				data->m_numSpuThreadsWaitingForJobs--;
				m_spuWaitSemaphore.release();
				return;
			}
				// check if ppu can take spu jobs, and if so release a ppu thread
			if ( data->m_numCpuThreadsWaitingForJobs && m_jobQueryRules[THREAD_TYPE_CPU].m_numJobTypesAvailable==JOB_TYPE_SPU )
			{
				data->m_numCpuThreadsWaitingForJobs--;
				m_cpuWaitSemaphore.release();
				return;
			}
		}
		return;
	}
#endif
	{
		if ( data->m_numCpuThreadsWaitingForJobs )
		{
			int numCpuJobs = data->m_jobQueue[JOB_TYPE_CPU].getSize() + data->m_jobQueue[JOB_TYPE_CPU_PRIMARY].getSize();
			if ( numCpuJobs  )
			{
				data->m_numCpuThreadsWaitingForJobs--;
				m_cpuWaitSemaphore.release();
			}
		}
	}
}


void hkJobQueue::addJob( JobQueueEntry& job, JobPriority priority, JobType jobThreadType )
{
	HK_ASSERT(0xf032e454, jobThreadType >= 0 && jobThreadType < 3 );

	DynamicData* data = lockQueue( );
	m_jobAddFunc(*this, data, job);
	{
		// Add the jobEntry to the queue
		if ( priority == JOB_HIGH_PRIORITY )
		{
			data->m_jobQueue[jobThreadType].enqueueInFront( job );
		}
		else
		{
			data->m_jobQueue[jobThreadType].enqueue( job );
		}

		releaseOneWaitingThread( jobThreadType, data );
	}
	unlockQueue( );
}

HK_FORCE_INLINE hkJobQueue::JobStatus hkJobQueue::findNextJob(ThreadType threadType, JobQueueEntry& jobOut, DynamicData* data)
{
	// WARNING: THIS FUNCTION MUST ALWAYS BE CALLED WHEN THE MT CRITICAL SECTION IS LOCKED
	// TODO - add isLocked to critical section and add assert

	// Check queues for a new job, starting with the queue matching the thread index
	JobQueryRule& myRules = m_jobQueryRules[threadType];

	for ( int i = 0; i < myRules.m_numJobTypesAvailable; i++)
	{
		HK_QUEUE< JobQueueEntry >* queue = &data->m_jobQueue[myRules.m_jobTypeOrder[i]];

#if defined (HK_PLATFORM_HAS_SPU) 
		if ( !HK_PLATFORM_IS_SPU )
		{
			HK_ASSERT( 0xf0323454, threadType != THREAD_TYPE_SPU);
			// If the job is an SPU job, and there is an SPU available, let the SPU have it
			if ( ( myRules.m_jobTypeOrder[i] == JOB_TYPE_SPU ) )
			{
				// We are assuming here that the jobs are "single" jobs
				if (data->m_numSpuThreadsWaitingForJobs >= queue->getSize())
				{
					return NO_JOBS_AVAILABLE;
				}
			}
		}
#endif

		if ( !queue->isEmpty() )
		{
			HK_ALIGN16(JobQueueEntry job);
			queue->dequeue(job);
			 
			if ( m_jobPopFunc(*this, data, job, jobOut) == DO_NOT_POP_QUEUE_ENTRY )
			{
				queue->enqueueInFront(job);
			}

			data->m_numActiveJobs++;
			return GOT_NEXT_JOB;
		}
	}
	return NO_JOBS_AVAILABLE;
}

HK_FORCE_INLINE hkBool hkJobQueue::allQueuesEmpty( hkJobQueue::DynamicData* data )
{
	int numJobs = data->m_jobQueue[JOB_TYPE_CPU].getSize() + data->m_jobQueue[JOB_TYPE_CPU_PRIMARY].getSize();
#if defined(HK_PLATFORM_HAS_SPU)
	numJobs += data->m_jobQueue[JOB_TYPE_SPU].getSize();
#endif
	return numJobs == 0;
}

hkJobQueue::JobStatus hkJobQueue::finishJobAndGetNextJob( ThreadType threadType, const JobQueueEntry* oldJob, JobQueueEntry& jobOut, void* params, WaitStatus waitStatus, int spuId)
{
#ifndef HK_PLATFORM_PS3SPU
	HK_TIME_CODE_BLOCK("GetNextJob", HK_NULL);
#endif
	HK_ASSERT(0xf032e454, threadType >= -1 && threadType < 3 );
	while(1)
	{
		DynamicData* data = lockQueue( );

		hkBool jobCreated = false;
		JobQueueEntryInput createdJob; 
		// If we have an old job, we need to check whether this old job just triggers a new job
		{
			if (oldJob)
			{
				if ( m_finishJobFunc( *this, data, *oldJob, createdJob ) == JOB_CREATED )
				{
					// Add the job to the queue
					if ( createdJob.m_jobPriority == JOB_HIGH_PRIORITY )
					{
						data->m_jobQueue[createdJob.m_jobThreadType].enqueueInFront( (const JobQueueEntry&)createdJob.m_job );
					}
					else
					{
						data->m_jobQueue[createdJob.m_jobThreadType].enqueue( (const JobQueueEntry&)createdJob.m_job );
					}
					jobCreated = true;
				}
					// if we finished a dummy job, simply return
				if ( threadType == THREAD_TYPE_INVALID)
				{
					checkQueueAndReleaseOneWaitingThread( hkJobQueue::ThreadType(createdJob.m_jobThreadType), data);
					unlockQueue( );
					return NO_JOBS_AVAILABLE;
				}
				data->m_numActiveJobs--;
				oldJob = 0;
			}
		}
		/*
		{
			ThreadFlags::Flag flag;
			data->m_flags.getThreadFlag( spuId, flag );
			if (flag == ThreadFlags::DO_NOT_PROCESS)
			{
				unlockQueue( );
				return NO_JOBS_AVAILABLE;
			}
		}
		*/

		// Try to find another job from available job queues 
		JobStatus result = findNextJob( threadType, jobOut, data );

#if defined(HK_PLATFORM_HAS_SPU)
		// this function might spawn 2 new jobs: the add job and the find next job
		// Example:
		//    - enqueue() puts an spu job on the queue -> so we need to release a spu thread
		//    - findNextJob() gets and splits a job on the ppu -> so we need to release a ppu thread
		//    we only need to release 2 threads if the created job is on a different platform than the found job
		if ( jobCreated && createdJob.m_jobThreadType != JobType(threadType) )
		{
			checkQueueAndReleaseOneWaitingThread( ThreadType(createdJob.m_jobThreadType), data);
			jobCreated = false;
		}
#endif

		if (result == GOT_NEXT_JOB)
		{
			checkQueueAndReleaseOneWaitingThread( threadType, data);
			unlockQueue( );
			return GOT_NEXT_JOB;
		}

		// If there are no active jobs and the queues are both empty we must be finished
		// NOTE: allQueuesEmpty() is currently necessary for CPUs waiting on SPUs to finish jobs
		if ( HK_PLATFORM_IS_SPU && !data->m_moreSpuJobsPossible )
		{
			// Don't release waiting threads if we are an SPU, because the SPU never currently takes the last job
			unlockQueue( );
			return ALL_JOBS_FINISHED;
		}

		if ( (data->m_numActiveJobs == 0) && allQueuesEmpty( data ) )
		{
			// Don't release waiting threads if we are an SPU, because the SPU never currently takes the last job
			//HK_ASSERT( 0xf02132ff, !HK_PLATFORM_IS_SPU);
			if ( !HK_PLATFORM_IS_SPU  )
			{
				releaseWaitingThreads(data);
			}
			unlockQueue( );
			return ALL_JOBS_FINISHED;
		}

		if ( waitStatus == DO_NOT_WAIT_FOR_NEXT_JOB )
		{
			unlockQueue( );
			return NO_JOBS_AVAILABLE;
		}

		if ( HK_PLATFORM_IS_SPU  )
		{
			// The queues are both empty, but there are active jobs, so wait to see if a new job appears
			data->m_numSpuThreadsWaitingForJobs++;
			unlockQueue( );
			HK_CPU_TIMER_BEGIN("WaitForSignal",HK_NULL);
			m_spuWaitSemaphore.acquire();
			HK_CPU_TIMER_END();
		}
		else
		{
			// The queues are both empty, but there are active jobs, so wait to see if a new job appears
			data->m_numCpuThreadsWaitingForJobs++;
			unlockQueue( );
			HK_CPU_TIMER_BEGIN("WaitForSignal",HK_NULL);
			m_cpuWaitSemaphore.acquire();
			HK_CPU_TIMER_END();
		}
	}
}

#if !defined(HK_PLATFORM_PS3SPU)
hkJobQueue::JobStatus hkJobQueue::addJobAndGetNextJob( JobType jobThreadType, JobPriority priority, ThreadType threadType, JobQueueEntry& jobInOut, WaitStatus waitStatus )
{
	HK_ASSERT(0xf032e454, jobThreadType >= 0 && jobThreadType < JOB_TYPE_MAX );
	HK_ASSERT(0xf032e454, threadType    >= 0 && threadType    < THREAD_TYPE_MAX );

	HK_CPU_TIMER_BEGIN("GetNextJob",HK_NULL);
	
	bool firstTime = true;
	while (1)
	{
		DynamicData* data = lockQueue();

		if (firstTime)
		{
			m_jobAddFunc(*this, data, jobInOut);

			// Add the job to the queue
			if ( priority == JOB_HIGH_PRIORITY )
			{
				data->m_jobQueue[jobThreadType].enqueueInFront( jobInOut );
			}
			else
			{
				data->m_jobQueue[jobThreadType].enqueue( jobInOut );
			}
			data->m_numActiveJobs--;
		}

		// Try to find another job from available job queues 
		JobStatus result = findNextJob(threadType, jobInOut, data );

#if defined(HK_PLATFORM_HAS_SPU)
		// this function might spawn 2 new jobs: the add job and the find next job
		if ( firstTime && threadType != ThreadType(jobThreadType) )
		{
			checkQueueAndReleaseOneWaitingThread( ThreadType(jobThreadType), data);
		}
#endif
		firstTime = false;

		if (result == GOT_NEXT_JOB)
		{
			checkQueueAndReleaseOneWaitingThread( threadType, data);
			unlockQueue();
			HK_CPU_TIMER_END();
			return GOT_NEXT_JOB;
		}

		// If there are no active jobs and the queues are both empty so we must be finished
		// NOTE: allQueuesEmpty() is currently necessary for CPUs waiting on SPUs to finish jobs
		if ( ( (data->m_numActiveJobs == 0) && allQueuesEmpty( data ) ) )
		{
			// Dont release waiting threads if we are an SPU, because the SPU never currently takes the last job
			if ( !HK_PLATFORM_IS_SPU )
			{
				releaseWaitingThreads(data);
			}
			unlockQueue();
			HK_CPU_TIMER_END();
			return ALL_JOBS_FINISHED;
		}

		if ( waitStatus == DO_NOT_WAIT_FOR_NEXT_JOB )
		{
			unlockQueue( );
			return NO_JOBS_AVAILABLE;
		}

		if ( HK_PLATFORM_IS_SPU  )
		{
			// The queues are both empty, but there are active jobs, so wait to see if a new job appears
			data->m_numSpuThreadsWaitingForJobs++;
			unlockQueue();
			HK_CPU_TIMER_BEGIN("WaitForSignal",HK_NULL);
			m_spuWaitSemaphore.acquire();
		}
		else
		{
			// The queues are both empty, but there are active jobs, so wait to see if a new job appears
			data->m_numCpuThreadsWaitingForJobs++;
			unlockQueue();
			HK_CPU_TIMER_BEGIN("WaitForSignal",HK_NULL);
			m_cpuWaitSemaphore.acquire();
		}
		HK_CPU_TIMER_END();
	}
}
#endif

void hkJobQueue::releaseWaitingThreads(DynamicData* data)
{
	//DynamicData* data = lockQueue();
	// Release cpu threads
	{
		int numCpuJobs = data->m_numCpuThreadsWaitingForJobs;
		data->m_numCpuThreadsWaitingForJobs = 0;

		HK_ASSERT2( 0xf032dea1, data->m_jobQueue[0].isEmpty(), "Queue 0 not empty" );
		HK_ASSERT2( 0xf032dea1, data->m_jobQueue[1].isEmpty(), "Queue 1 not empty" );
		HK_ASSERT2( 0, data->m_numActiveJobs == 0, "Num active jobs is not 0" );
		
		for ( ;numCpuJobs > 0; numCpuJobs--)
		{
			m_cpuWaitSemaphore.release();
		}
	}

#if defined(HK_PLATFORM_HAS_SPU)
	// Release spu threads
	{
		int numSpuJobs = data->m_numSpuThreadsWaitingForJobs;
		data->m_numSpuThreadsWaitingForJobs = 0;
		HK_ASSERT2( 0xf032dea1, data->m_jobQueue[hkJobQueue::JOB_TYPE_SPU].isEmpty(), "Queue JOB_TYPE_SPU not empty" );
		
		for ( ; numSpuJobs > 0; numSpuJobs--)
		{
			m_spuWaitSemaphore.release();
		}
	}
#endif
	//unlockQueue();
}


#if !defined(HK_PLATFORM_PS3SPU)
void hkJobQueue::setSpuQueueCapacity(int queueCapacity)
{
	HK_ASSERT( 0xf0323344, !HK_PLATFORM_IS_SPU );
	{
		lockQueue();
		m_data->m_jobQueue[THREAD_TYPE_CPU_PRIMARY].reserve(queueCapacity);
		m_data->m_jobQueue[THREAD_TYPE_CPU].reserve(queueCapacity);
#if defined (HK_PLATFORM_HAS_SPU)
		m_data->m_jobQueue[THREAD_TYPE_SPU].reserve(queueCapacity);
#endif
		unlockQueue();
	}
}
#endif


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
