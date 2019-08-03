/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkbase/thread/hkSpuDmaManager.h>
#include <hkbase/memory/dmacache/hkSpuDmaCache.h>

#include <hkbase/thread/job/hkJobQueue.h>
#include <hkanimation/playback/multithreaded/spu/hkSpuAnimation.h>
#include <hkanimation/playback/multithreaded/hkAnimationJobs.h>

#include <hkanimation/playback/multithreaded/hkAnimationJobQueueUtils.h>
#include <hkbase/thread/hkSpuStack.h>

#include <hkbase/monitor/hkMonitorStream.h>

#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>

#include <hkanimation/animation/hkSkeletalAnimation.h>
#include <hkanimation/animation/interleaved/hkInterleavedSkeletalAnimation.h>
#include <hkanimation/animation/waveletcompressed/hkWaveletSkeletalAnimation.h>
#include <hkanimation/animation/deltacompressed/hkDeltaCompressedSkeletalAnimation.h>
#include <hkanimation/animation/hkAnimationBinding.h>
#include <hkanimation/motion/hkAnimatedReferenceFrame.h>

#include <hkanimation/playback/control/hkAnimationControl.h>
#include <hkanimation/playback/utilities/hkSampleAndCombineUtils.h>

#if defined (HK_PLATFORM_PS3SPU) || defined (HK_SIMULATE_SPU_DMA_ON_CPU)

//Todo: Remove these dependencies
#if defined(HK_PLATFORM_PS3SPU)
hkThreadMemory* hkThreadMemory::s_threadMemoryInstance;

void hkThreadMemory::deallocateChunk(void*, int, HK_MEMORY_CLASS)
{
	return;
}

#endif

enum hkSpuDmaGroups
{
	HK_SPU_DMA_GROUP_ANIMATION_DATA			=  0, // reserve 2 groups for double-buffering
	HK_SPU_DMA_GROUP_ANIMATED_SKELETON_DATA =  2,
	HK_SPU_DMA_GROUP_OUTPUT_POSE			=  3,
	HK_SPU_DMA_GROUP_ANIMATION_SAMPLE_DATA	=  4, // reserve 2 groups for double-buffering
	HK_SPU_DMA_GROUP_SEMAPHORE				=  6
};


namespace {

	const int MAX_COMPRESSED_ANIM_DATA = 75 * 1024 ; // 75k max space for animation block

	struct hkSampleAndCombineBuffer
	{
		hkAnimationSampleAndCombineJob::ControlData m_controlData;
		HK_ALIGN16(hkUint8 m_animBlockData[MAX_COMPRESSED_ANIM_DATA]); 
	};

	struct hkAnimationBuffer
	{
		hkSampleAnimationJob::AnimationData m_animData;
		HK_ALIGN16(hkUint8 m_animBlockData[MAX_COMPRESSED_ANIM_DATA]); 
	};
}

inline void  prefetchAnimData(hkAnimationSampleAndCombineJob& job, int controlIndex, hkSampleAndCombineBuffer* data, int dmaGroup)
{
	// ToDo : Pack data more efficiently on PPU and remove waits here - they reduce the effectiveness of the double buffering
	hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &data->m_controlData, job.m_animationControls + controlIndex , sizeof( hkAnimationSampleAndCombineJob::ControlData), hkSpuDmaManager::READ_COPY, dmaGroup);

	// Get the binding array
	const int bindingSize = HK_NEXT_MULTIPLE_OF(16, data->m_controlData.m_binding.m_numAnimationTrackToBoneIndices * sizeof(hkInt16) );
	hkSpuDmaManager::getFromMainMemory( &data->m_animBlockData, data->m_controlData.m_binding.m_animationTrackToBoneIndices, bindingSize , hkSpuDmaManager::READ_COPY, dmaGroup);

	// Get the track mask - if present.
	hkUint8* freeSpace = data->m_animBlockData + bindingSize;
	if ( data->m_controlData.m_numTrackWeights > 0)
	{
		const int trackMaskSize = HK_NEXT_MULTIPLE_OF(16, data->m_controlData.m_numTrackWeights * sizeof(hkUint8) );
		hkSpuDmaManager::getFromMainMemory( freeSpace, data->m_controlData.m_trackWeights, trackMaskSize , hkSpuDmaManager::READ_COPY, dmaGroup);
		freeSpace += trackMaskSize;
	}

	// Get the data chunks
	for (int i=0; i< data->m_controlData.m_numChunks; i++)
	{	
		hkSkeletalAnimation::DataChunk& chunk = data->m_controlData.m_chunks[i];
		HK_ASSERT2(0x547e39e3, freeSpace + chunk.m_size <  data->m_animBlockData + MAX_COMPRESSED_ANIM_DATA, "Out of animation scratch space on SPU");
		hkSpuDmaManager::getFromMainMemory( freeSpace , chunk.m_data, chunk.m_size , hkSpuDmaManager::READ_COPY, dmaGroup);
		freeSpace += chunk.m_size;
	}
};

inline void  waitForAnimData(hkAnimationSampleAndCombineJob& job, int controlIndex, hkSampleAndCombineBuffer* data, int dmaGroup)
{
	// Wait for the DMA to complete
	hkSpuDmaManager::waitForDmaCompletion(dmaGroup);

	//DG ToDo : Pack data more efficiently on PPU and remove waits here - they reduce the effectiveness of the double buffering
	hkSpuDmaManager::performFinalChecks( job.m_animationControls + controlIndex , &data->m_controlData, sizeof( hkAnimationSampleAndCombineJob::ControlData) );

	// Get the binding array
	const int bindingSize = HK_NEXT_MULTIPLE_OF(16, data->m_controlData.m_binding.m_numAnimationTrackToBoneIndices * sizeof(hkInt16) );
	hkSpuDmaManager::performFinalChecks( data->m_controlData.m_binding.m_animationTrackToBoneIndices, &data->m_animBlockData, bindingSize);
	data->m_controlData.m_binding.m_animationTrackToBoneIndices = reinterpret_cast<hkInt16*> ( data->m_animBlockData );

	hkUint8* freeSpace = data->m_animBlockData + bindingSize;

	// Get the track mask - if present.
	if ( data->m_controlData.m_numTrackWeights > 0)
	{
		const int trackMaskSize = HK_NEXT_MULTIPLE_OF(16, data->m_controlData.m_numTrackWeights * sizeof(hkUint8) );
		hkSpuDmaManager::performFinalChecks( data->m_controlData.m_trackWeights, freeSpace, trackMaskSize);
		data->m_controlData.m_trackWeights = reinterpret_cast<hkUint8*> ( freeSpace );
		freeSpace += trackMaskSize;
	}

	// Get the data chunks
	for (int i=0; i< data->m_controlData.m_numChunks; i++)
	{	
		hkSkeletalAnimation::DataChunk& chunk = data->m_controlData.m_chunks[i];
		hkSpuDmaManager::performFinalChecks( chunk.m_data, freeSpace, chunk.m_size );
		chunk.m_data = freeSpace;
		freeSpace += chunk.m_size;
	}
}

// Dispatch call based on type
inline void process(hkSkeletalAnimation::Type animationType, hkReal localTime, int maxTrack, hkSkeletalAnimation::DataChunk* chunks, int numChunks, hkQsTransform* output)
{
	// Sample the animation
	switch (animationType)
	{
		case hkSkeletalAnimation::HK_INTERLEAVED_ANIMATION :
		{
			hkInterleavedSkeletalAnimation::samplePartialWithDataChunks( localTime, maxTrack, output, chunks, numChunks );
			break;
		}
		case hkSkeletalAnimation::HK_WAVELET_COMPRESSED_ANIMATION :
		{	
			hkWaveletSkeletalAnimation::samplePartialWithDataChunks( localTime, maxTrack, output, chunks, numChunks );
			break;
		}
		case hkSkeletalAnimation::HK_DELTA_COMPRESSED_ANIMATION :
		{
			hkDeltaCompressedSkeletalAnimation::samplePartialWithDataChunks( localTime, maxTrack, output, chunks, numChunks );
			break;
		}
	}
}

hkJobQueue::JobStatus HK_CALL hkSpuSampleAndCombineJob(hkAnimationSampleAndCombineJob& job)
{
	// Simple early out - user should not have placed this job on the queue
	if ( job.m_numAnimationControls == 0 )
	{
		HK_WARN(0x54e43234, "Animated Skeleton has no controls");
		return hkJobQueue::NO_JOBS_AVAILABLE;
	}

	HK_TIMER_BEGIN_LIST("SpuAnim", "FetchAnimSkel");

	/*
	dmaToSpuNoWait( first animation control + anim data )
	initialize output pose
	for each animation control
	{
		dmaToSpuNoWait( next animation control + anim data )
		waitForDma( current control + anim data )
		decompress current animation and blend into output pose
	}
	dmaToMainMem( output pose to job.poseOutput )
	}
	*/

	// Create double buffers for animation data
	hkSampleAndCombineBuffer* doubleBuffer = static_cast<hkSampleAndCombineBuffer*>(hkSpuStack::getInstance().allocateStack( sizeof(hkSampleAndCombineBuffer) * 2, "AnimationDoubleBuffer" ));
	int workingDmaGroup = 0;
	hkSampleAndCombineBuffer* workingData =  &doubleBuffer[workingDmaGroup];
	hkSampleAndCombineBuffer* prefetchData = &doubleBuffer[1-workingDmaGroup];

	// Grab the reference pose if needed
	hkQsTransform* referencePose = HK_NULL;
	if ((job.m_referencePose) && (job.m_referencePoseWeightThreshold > 0.0f))
	{
		// Allocate space for animated skeleton and upload to LS
		int poseSize = sizeof(hkQsTransform) * job.m_maxBone;
		referencePose = static_cast<hkQsTransform*>(hkSpuStack::getInstance().allocateStack( poseSize, "ReferencePose" ));
		hkSpuDmaManager::getFromMainMemory( referencePose, job.m_referencePose, poseSize, hkSpuDmaManager::READ_COPY, HK_SPU_DMA_GROUP_ANIMATED_SKELETON_DATA);
	}

	HK_TIMER_SPLIT_LIST("Prefetch");
	// Prefetch the first animation control
	{
		int prefetchDmaGroup = 1 - workingDmaGroup;
		prefetchAnimData( job, 0, prefetchData, HK_SPU_DMA_GROUP_ANIMATION_DATA + prefetchDmaGroup);

		// Swap buffers
		hkSampleAndCombineBuffer* tmp = prefetchData; prefetchData = workingData; workingData = tmp;
		workingDmaGroup = prefetchDmaGroup;
	}

	HK_TIMER_SPLIT_LIST("AllocOutputPose");
	// Init the output pose and working space
	hkQsTransform* outputPose;
	hkQsTransform* workingSpace;
	hkReal* boneWeights;
	const int numBones = job.m_maxBone;
	{
		outputPose   = static_cast<hkQsTransform*>( hkSpuStack::getInstance().allocateStack( sizeof(hkQsTransform) * numBones, "OutputPose" ) );
		workingSpace = static_cast<hkQsTransform*>( hkSpuStack::getInstance().allocateStack( sizeof(hkQsTransform) * numBones, "WorkingSpace" ) );
		boneWeights= static_cast<hkReal*>( hkSpuStack::getInstance().allocateStack( HK_NEXT_MULTIPLE_OF(16, sizeof(hkReal) * numBones), "BoneWeights" ) );

		for (int boneIdx = 0; boneIdx < numBones; boneIdx++)
		{
			outputPose[boneIdx].setZero();
			boneWeights[boneIdx] = 0.0f;
		}	
	}
	
	// Iterate through the controls
	bool renormalized = false;
	for (hkUint32 ctrlIdx=0; ctrlIdx < job.m_numAnimationControls; ctrlIdx++)
	{
		HK_TIMER_SPLIT_LIST("Prefetch");

		// Prefetch the next animation control
		hkUint32 nextCtrlIdx = ctrlIdx + 1;
		if (nextCtrlIdx < job.m_numAnimationControls)
		{
			int prefetchDmaGroup = 1 - workingDmaGroup;
			prefetchAnimData( job, nextCtrlIdx, prefetchData, HK_SPU_DMA_GROUP_ANIMATION_DATA + prefetchDmaGroup);
		}

		HK_TIMER_SPLIT_LIST("WaitDma");
		// Wait for working data to arrive
		waitForAnimData( job, ctrlIdx, workingData, HK_SPU_DMA_GROUP_ANIMATION_DATA + workingDmaGroup );

		HK_TIMER_SPLIT_LIST("Process");

		// Early-out attempts. These probably don't buy us much since we have to wait for the next DMA anyway.
		if(workingData->m_controlData.m_weight < HK_REAL_EPSILON)
		{
			continue;
		}

		hkUint32 maxTrack = hkSampleAndCombineUtils::getMaxTrack(workingData->m_controlData.m_binding.m_animationTrackToBoneIndices, workingData->m_controlData.m_trackWeights, numBones, workingData->m_controlData.m_binding.m_numAnimationTrackToBoneIndices);
		
		// Process workingData and place in output buffer
		process( workingData->m_controlData.m_animationType, workingData->m_controlData.m_localTime, job.m_maxBone, workingData->m_controlData.m_chunks, workingData->m_controlData.m_numChunks, workingSpace );

		hkAnimationBinding::BlendHint blendHint = workingData->m_controlData.m_binding.m_blendHint;

		hkSampleAndCombineUtils::BlendParameters params;
		params.m_animationTransformsIn = workingSpace;
		params.m_masterWeight = workingData->m_controlData.m_weight;
		params.m_numAnimationTransforms = maxTrack;
		params.m_numBones = numBones;
		params.m_perBoneWeights = workingData->m_controlData.m_trackWeights;
		params.m_trackToBoneMapping = workingData->m_controlData.m_binding.m_animationTrackToBoneIndices;

		if ( blendHint == hkAnimationBinding::NORMAL )
		{
			hkSampleAndCombineUtils::blendNormal(params, outputPose, boneWeights);
		}


		HK_TIMER_SPLIT_LIST("Renormalize");
		// If we have reached the last normal control or additive animations then we need to normalize the pose 
		if ( ( ( nextCtrlIdx == job.m_numAnimationControls) && ( blendHint == hkAnimationBinding::NORMAL   ) ) ||
			 ( !renormalized                                && ( blendHint == hkAnimationBinding::ADDITIVE ) ) )
		{
			// Blend in reference pose
			if (referencePose)
			{
				hkSpuDmaManager::waitForDmaCompletion(HK_SPU_DMA_GROUP_ANIMATED_SKELETON_DATA);
				hkSpuDmaManager::performFinalChecks( job.m_referencePose, referencePose, sizeof(hkQsTransform) * job.m_maxBone );		

				// Fill in Bones below the reference pose threshold
				for (int i=0; i < numBones; i++)
				{
					if (boneWeights[i] <= job.m_referencePoseWeightThreshold)
					{
						// Fill in tPose to avoid snapping
						const hkReal weight = job.m_referencePoseWeightThreshold - boneWeights[i];
						outputPose[i].blendAddMul(referencePose[i], weight);
						boneWeights[i] += weight;
					}
				}
			}

			hkQsTransform::fastRenormalizeBatch(outputPose, boneWeights, numBones);
			renormalized = true;
		}

		if ( blendHint == hkAnimationBinding::ADDITIVE )
		{
			hkSampleAndCombineUtils::blendAdditive(params, outputPose);
		}

		// Swap buffers
		{
			hkSampleAndCombineBuffer* tmp = prefetchData; prefetchData = workingData; workingData = tmp;
			workingDmaGroup = 1 - workingDmaGroup;
		}
	}

	// Write data back pose data to PPU
	{
		hkSpuDmaManager::putToMainMemoryAndWaitForCompletion( job.m_poseOut, outputPose, sizeof(hkQsTransform) * job.m_maxBone, hkSpuDmaManager::WRITE_NEW, HK_SPU_DMA_GROUP_OUTPUT_POSE );
		hkSpuDmaManager::performFinalChecks( job.m_poseOut, outputPose, sizeof(hkQsTransform) * job.m_maxBone );

	}
	
	// perform final checks on READ_ONLY and READ_COPY data
	hkSpuStack::getInstance().deallocateStack( boneWeights );
	hkSpuStack::getInstance().deallocateStack( workingSpace );
	hkSpuStack::getInstance().deallocateStack( outputPose );
	if (referencePose)
	{
		hkSpuStack::getInstance().deallocateStack( referencePose );
	}
	hkSpuStack::getInstance().deallocateStack( doubleBuffer );

	HK_TIMER_END_LIST();
	return hkJobQueue::NO_JOBS_AVAILABLE;
}



inline void  prefetchAnimData(hkSampleAnimationJob& job, int controlIndex, hkAnimationBuffer* data, int dmaGroup)
{
	// ToDo : Pack data more efficiently on PPU and remove waits here - they reduce the effectiveness of the double buffering
	hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &data->m_animData, job.m_animData + controlIndex , sizeof( hkSampleAnimationJob::AnimationData), hkSpuDmaManager::READ_COPY, dmaGroup);

	// Get the track mask - if present.
	hkUint8* freeSpace = data->m_animBlockData;

	// Get the data chunks
	for (int i=0; i< data->m_animData.m_numChunks; i++)
	{	
		hkSkeletalAnimation::DataChunk& chunk = data->m_animData.m_chunks[i];
		HK_ASSERT2(0x547e39e3, freeSpace + chunk.m_size <  data->m_animBlockData + MAX_COMPRESSED_ANIM_DATA, "Out of animation scratch space on SPU");
		hkSpuDmaManager::getFromMainMemory( freeSpace, chunk.m_data, chunk.m_size , hkSpuDmaManager::READ_COPY, dmaGroup);
		freeSpace += chunk.m_size;
	}
};

inline void  waitForAnimData(hkSampleAnimationJob& job, int controlIndex, hkAnimationBuffer* data, int dmaGroup)
{
	// Wait for the DMA to complete
	hkSpuDmaManager::waitForDmaCompletion(dmaGroup);
	hkSpuDmaManager::performFinalChecks( job.m_animData + controlIndex , &data->m_animData, sizeof( hkSampleAnimationJob::AnimationData) );

	hkUint8* freeSpace = data->m_animBlockData;

	// Get the data chunks
	for (int i=0; i< data->m_animData.m_numChunks; i++)
	{	
		hkSkeletalAnimation::DataChunk& chunk = data->m_animData.m_chunks[i];
		hkSpuDmaManager::performFinalChecks( chunk.m_data, freeSpace, chunk.m_size );
		chunk.m_data = freeSpace;
		freeSpace += chunk.m_size;
	}
}

//ToDo: Improve this so the output is double buffered too
hkJobQueue::JobStatus HK_CALL hkSpuSampleAnimationJob(hkSampleAnimationJob& job)
{
	HK_TIMER_BEGIN_LIST("SpuAnim", "SampleAnimation");

	hkAnimationBuffer* doubleBuffer = static_cast<hkAnimationBuffer*>(hkSpuStack::getInstance().allocateStack( sizeof(hkAnimationBuffer) * 2, "SampleAnimationDoubleBuffer" ));
	int workingDmaGroup = 0;
	hkAnimationBuffer* workingData =  &doubleBuffer[workingDmaGroup];
	hkAnimationBuffer* prefetchData = &doubleBuffer[1-workingDmaGroup];

	HK_TIMER_SPLIT_LIST("Prefetch");
	// Prefetch the first animation 
	{
		int prefetchDmaGroup = 1 - workingDmaGroup;
		prefetchAnimData( job, 0, prefetchData, HK_SPU_DMA_GROUP_ANIMATION_SAMPLE_DATA + prefetchDmaGroup);

		// Swap buffers
		hkAnimationBuffer* tmp = prefetchData; prefetchData = workingData; workingData = tmp;
		workingDmaGroup = prefetchDmaGroup;
	}

	for (int ctrlIdx=0; ctrlIdx < job.m_numAnims; ctrlIdx++)
	{
		HK_TIMER_SPLIT_LIST("Prefetch");
		// Prefetch the next animation control 
		int nextCtrlIdx = ctrlIdx + 1;
		if (nextCtrlIdx < job.m_numAnims)
		{
			int prefetchDmaGroup = 1 - workingDmaGroup;
			prefetchAnimData( job, nextCtrlIdx, prefetchData, HK_SPU_DMA_GROUP_ANIMATION_SAMPLE_DATA + prefetchDmaGroup);
		}

		HK_TIMER_SPLIT_LIST("WaitDma");
		// Wait for working data to arrive
		waitForAnimData( job, ctrlIdx, workingData, HK_SPU_DMA_GROUP_ANIMATION_SAMPLE_DATA + workingDmaGroup );

		// Init the output pose and working space
		hkQsTransform* workingSpace;
		workingSpace = static_cast<hkQsTransform*>( hkSpuStack::getInstance().allocateStack( sizeof(hkQsTransform) * workingData->m_animData.m_maxTrack , "OutputPose" ) );

		HK_TIMER_SPLIT_LIST("Process");
		process( workingData->m_animData.m_animationType, workingData->m_animData.m_localTime, workingData->m_animData.m_maxTrack, workingData->m_animData.m_chunks, workingData->m_animData.m_numChunks, workingSpace );

		// Write data back pose data to PPU
		{
			hkSpuDmaManager::putToMainMemoryAndWaitForCompletion( workingData->m_animData.m_poseOut, workingSpace, sizeof(hkQsTransform) * workingData->m_animData.m_maxTrack, hkSpuDmaManager::WRITE_NEW, HK_SPU_DMA_GROUP_OUTPUT_POSE );
			hkSpuDmaManager::performFinalChecks( workingData->m_animData.m_poseOut, workingSpace, sizeof(hkQsTransform) * workingData->m_animData.m_maxTrack );
		}

		hkSpuStack::getInstance().deallocateStack( workingSpace );
 
		// Swap buffers
		{
			hkAnimationBuffer* tmp = prefetchData; prefetchData = workingData; workingData = tmp;
			workingDmaGroup = 1 - workingDmaGroup;
		}
	}

	hkSpuStack::getInstance().deallocateStack( doubleBuffer );

	HK_TIMER_END_LIST();

	return hkJobQueue::NO_JOBS_AVAILABLE;
}

hkJobQueue::JobStatus HK_CALL hkSpuProcessNextAnimationJob( hkJobQueue& queue, hkJobQueue::WaitStatus waitStatus, int spuId )
{

	HK_ALIGN( hkJobQueue::JobQueueEntry job, 128 );

	queue.m_jobAddFunc    = hkAnimationJobQueueUtils::addAnimationJob;
	queue.m_jobPopFunc    = hkAnimationJobQueueUtils::popAnimationJob;
	queue.m_finishJobFunc = hkAnimationJobQueueUtils::finishAnimationJob;
	queue.m_userData      = HK_NULL;
	{
		hkJobQueue::JobStatus jobStatus = queue.getNextJob(hkJobQueue::THREAD_TYPE_SPU, job, waitStatus, spuId);
		if ( jobStatus != hkJobQueue::GOT_NEXT_JOB)
		{
			return jobStatus;
		}
	}

	//HK_SPU_DEBUG_PRINTF(("**** NEW FRAME ****\n"));

	//
	// this loop exits once there are no jobs left on the local queue
	//
	hkJobQueue::JobStatus jobStatus;
	while ( 1 )
	{
		hkAnimationJob& animationJob = reinterpret_cast<hkAnimationJob&>(job);

		switch ( animationJob.m_jobType )
		{
			case ANIMATION_JOB_SAMPLE_AND_COMBINE:
				{
					hkAnimationSampleAndCombineJob& sacJob = reinterpret_cast<hkAnimationSampleAndCombineJob&>(job);

					// Get the completion semaphore from main memory
					const int semBufSize = HK_NEXT_MULTIPLE_OF(16,sizeof(hkSemaphoreBusyWait));
					HK_ALIGN( char semaBuf[semBufSize], 128 ); 
					hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( semaBuf, sacJob.m_jobDone, semBufSize, hkSpuDmaManager::READ_COPY, HK_SPU_DMA_GROUP_SEMAPHORE);

					// Do the animation sampling and blending
					jobStatus = hkSpuSampleAndCombineJob( sacJob );

					// Check the semaphore has arrived
					hkSpuDmaManager::waitForDmaCompletion( HK_SPU_DMA_GROUP_SEMAPHORE );
					hkSpuDmaManager::performFinalChecks(sacJob.m_jobDone, semaBuf, semBufSize );

					// Release the semaphore to indicate the job is complete
					hkSemaphoreBusyWait* semaphore = reinterpret_cast<hkSemaphoreBusyWait*>(semaBuf);
					semaphore->release();
					
					if ( jobStatus != hkJobQueue::GOT_NEXT_JOB )
					{
						goto END;
					}
					break;
				}
			case ANIMATION_JOB_SAMPLE_ANIMATION:
				{
					hkSampleAnimationJob& saJob = reinterpret_cast<hkSampleAnimationJob&>(job);

					// Get the completion semaphore from main memory
					const int semBufSize = HK_NEXT_MULTIPLE_OF(16,sizeof(hkSemaphoreBusyWait));
					HK_ALIGN( char semaBuf[semBufSize], 128 ); 
					hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( semaBuf, saJob.m_jobDone, semBufSize, hkSpuDmaManager::READ_COPY, HK_SPU_DMA_GROUP_SEMAPHORE);

					// Do the sampling 
					jobStatus = hkSpuSampleAnimationJob( saJob );

					// Check the semaphore has arrived
					hkSpuDmaManager::waitForDmaCompletion( HK_SPU_DMA_GROUP_SEMAPHORE );
					hkSpuDmaManager::performFinalChecks(saJob.m_jobDone, semaBuf, semBufSize );

					hkSemaphoreBusyWait* semaphore = reinterpret_cast<hkSemaphoreBusyWait*>(semaBuf);
					semaphore->release();

					if ( jobStatus != hkJobQueue::GOT_NEXT_JOB )
					{
						goto END;
					}
					break;
				}
			default:
				{
					HK_ASSERT2(0x12ad3046, 0, "Job found on Spu queue that cannot be processed");
					break;
				}
		}
	}
END:
	return jobStatus;
}

#endif /*#if defined (HK_PLATFORM_PS3SPU) || defined (HK_SIMULATE_SPU_DMA_ON_CPU)*/

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
