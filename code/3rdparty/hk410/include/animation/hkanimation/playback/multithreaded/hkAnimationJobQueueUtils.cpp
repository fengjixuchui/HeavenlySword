/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkbase/thread/hkSpuDmaManager.h>
#include <hkanimation/playback/multithreaded/hkAnimationJobs.h>
#include <hkanimation/playback/multithreaded/hkAnimationJobQueueUtils.h>

HK_COMPILE_TIME_ASSERT( sizeof( hkAnimationSampleAndCombineJob )         <= sizeof( hkJobQueue::JobQueueEntry ) );



void HK_CALL hkAnimationJobQueueUtils::addAnimationJob ( hkJobQueue& queue, hkJobQueue::DynamicData* data, const hkJobQueue::JobQueueEntry& jobIn )
{
	const hkAnimationJob& job = reinterpret_cast<const hkAnimationJob&>(jobIn);
	switch ( job.m_jobType )
	{
		case ANIMATION_JOB_SAMPLE_AND_COMBINE:
		{
			const hkAnimationSampleAndCombineJob& s = static_cast<const hkAnimationSampleAndCombineJob&>(job);
			// Initialize job.
			break;
		}
		default:
			break;
	}
}

hkJobQueue::JobPopFuncResult HK_CALL hkAnimationJobQueueUtils::popAnimationJob   ( hkJobQueue& queue, hkJobQueue::DynamicData* data,       hkJobQueue::JobQueueEntry& jobIn, hkJobQueue::JobQueueEntry& jobOut  )
{

	hkAnimationJob& job = reinterpret_cast<hkAnimationJob&>(jobIn);

	switch ( job.m_jobType )
	{ 
		case ANIMATION_JOB_SAMPLE_AND_COMBINE:
		case ANIMATION_JOB_SAMPLE_ANIMATION:
			{
				jobOut = jobIn;
				return hkJobQueue::POP_QUEUE_ENTRY;
			}

		break;
		default:
			{
				HK_ASSERT2(0xad789ddd, false, "Won't handle this job!!!");
				break;
			}

	}
	return hkJobQueue::POP_QUEUE_ENTRY;
}

hkJobQueue::JobCreationStatus HK_CALL hkAnimationJobQueueUtils::finishAnimationJob( hkJobQueue& queue, hkJobQueue::DynamicData* data, const hkJobQueue::JobQueueEntry& jobIn, hkJobQueue::JobQueueEntryInput& newJobCreated )
{
	const hkAnimationJob& job = reinterpret_cast<const hkAnimationJob&>(jobIn);
	switch( job.m_jobType )
	{
		case ANIMATION_JOB_SAMPLE_AND_COMBINE:
		case ANIMATION_JOB_SAMPLE_ANIMATION:
			{
				hkJobQueue::JobCreationStatus status = hkJobQueue::NO_JOB_CREATED;
				
				return status;
			}
		default:
			{
				break;
			}
	}

	return hkJobQueue::NO_JOB_CREATED;
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
