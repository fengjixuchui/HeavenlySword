/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#if defined (HK_PLATFORM_PS3SPU) || defined (HK_SIMULATE_SPU_DMA_ON_CPU)

#include <hkbase/thread/hkSpuDmaManager.h>
#include <hkbase/memory/dmacache/hkSpuDmaCache.h>

#include <hkdynamics/world/simulation/multithreaded/spu/utilities/hkSpuDmaWriter.h>
#include <hkbase/thread/hkSpuStack.h>
#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuConstraint.h>

#include <hkbase/thread/job/hkJobQueue.h>
#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobs.h>
#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobQueueUtils.h>

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/constraint/setup/hkConstraintSolverSetup.h>

#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>
#include <hkconstraintsolver/constraint/atom/hkBuildJacobianFromAtoms.h>
#include <hkconstraintsolver/jacobian/hkJacobianHeaderSchema.h>

#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuTest.h>
#include <hkcollide/agent/hkProcessCollisionData.h>

#include <hkbase/monitor/hkMonitorStream.h>

#include <hkdynamics/motion/util/hkRigidMotionUtil.h>


HK_COMPILE_TIME_ASSERT( HK_MAX_CONTACT_POINT == HK_MAX_NUM_CONTACT_POINTS_IN_CONSTRAINT_ON_SPU);

static int g_hkSpuId;


#if defined(HK_PLATFORM_HAS_SPU)
HK_COMPILE_TIME_ASSERT( HK_MAX_CONTACT_POINT == HK_MAX_NUM_CONTACT_POINTS_IN_CONSTRAINT_ON_SPU );
HK_COMPILE_TIME_ASSERT( HK_SPU_JACOBIAN_WRITER_OVERFLOW_BUFFER_SIZE >= 128 + HK_MAX_CONTACT_POINT * HK_SIZE_OF_JACOBIAN_LAA + sizeof(hkSimpleContactConstraintAtom));
HK_COMPILE_TIME_ASSERT( HK_SPU_SCHEMA_WRITER_OVERFLOW_BUFFER_SIZE   >= 128 + HK_MAX_CONTACT_POINT * HK_SIZE_OF_JACOBIAN_SINGLE_CONTACT_SCHEMA + HK_SIZE_OF_JACOBIAN_3D_FRICTION_SCHEMA + HK_SIZE_OF_JACOBIAN_HEADER_SCHEMA);
#endif

namespace {

	struct hkBuildAccumulatorsDataSet
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ROOT, hkBuildAccumulatorsDataSet );

		void init(int readMotionsDmaGroup, int writeAccumulatorsDmaGroup)
		{
			for (int i = 0; i < HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE; i++)
			{
				m_motionsArray[i] = reinterpret_cast<hkMotion*>(&m_motionsBuffer[i]);
			}

			m_readMotionsDmaGroup       = readMotionsDmaGroup;
			m_writeAccumulatorsDmaGroup = writeAccumulatorsDmaGroup;
		}

		hkPadSpu<int> m_numMotions;

		HK_ALIGN16( hkMaxSizeMotion       m_motionsBuffer[HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE] );
		HK_ALIGN16( hkMotion*             m_motionsArray [HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE] );
		HK_ALIGN16( hkVelocityAccumulator m_accumulators [HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE] );

		int m_readMotionsDmaGroup;
		int m_writeAccumulatorsDmaGroup;
	};

	struct hkBuildAccumulatorsDataSetTool
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ROOT, hkBuildAccumulatorsDataSetTool );

		void init(int readMotionsBaseDmaGroup, int writeAccumulatorsBaseDmaGroup)
		{
			m_dataSet[0].init(readMotionsBaseDmaGroup+0, writeAccumulatorsBaseDmaGroup+0);
			m_dataSet[1].init(readMotionsBaseDmaGroup+1, writeAccumulatorsBaseDmaGroup+1);
		}

		hkBuildAccumulatorsDataSet m_dataSet[2];
	};



	// this is nearly a duplication of hkBuildJacobianTask
	struct hkBuildJacobiansFromTaskInput
	{
		hkPadSpu<hkBuildJacobianTask::AtomInfo*> m_atomInfosOnSpu;
		hkPadSpu<int> m_numAtomInfos;

		hkPadSpuLong<HK_CPU_PTR(hkVelocityAccumulator*)> m_accumulatorsInMainMemory;

		hkPadSpuLong<HK_CPU_PTR(hkJacobianSchema*)> m_schemasOfNextTask;
		hkPadSpuLong<HK_CPU_PTR(void*)>             m_nextTask;

		//output:
		hkPadSpu<hkSpuDmaWriter*> m_jacobianWriter;
		hkPadSpu<hkSpuDmaWriter*> m_schemaWriter;
	};



	struct hkBuildJacobiansFromTaskDataSet
	{
		hkPadSpu<const hkVelocityAccumulator*>	m_accumulatorA;
		hkPadSpu<const hkVelocityAccumulator*>	m_accumulatorB;
		hkPadSpu<const hkTransform*>			m_transformA;
		hkPadSpu<const hkTransform*>			m_transformB;
		hkPadSpu<void*>							m_runtimeBuffer;
		HK_ALIGN16( hkUint8						m_constraintRuntimeBuffer[HK_SPU_CONSTRAINT_RUNTIME_BUFFER_SIZE]);
		HK_ALIGN16( hkUint8						m_constraintAtomsBuffer  [HK_SPU_CONSTRAINT_ATOM_BUFFER_SIZE]);

		hkPadSpu<int>							m_readRuntimeAndAtomsDmaGroup;
	};

	struct hkBuildJacobiansFromTaskDataSetTool
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ROOT, hkBuildJacobiansFromTaskDataSetTool );

		void init(int firstCacheDmaGroup, int numCacheDmaGroups, int readRuntimeAndAtomsBaseDmaGroup)
		{
			m_accumulatorsCache.init(firstCacheDmaGroup, numCacheDmaGroups);
			m_transformsCache.  init(firstCacheDmaGroup, numCacheDmaGroups);

			m_dataSet[0].m_readRuntimeAndAtomsDmaGroup = readRuntimeAndAtomsBaseDmaGroup+0;
			m_dataSet[1].m_readRuntimeAndAtomsDmaGroup = readRuntimeAndAtomsBaseDmaGroup+1;
		}

		void exit()
		{
			m_accumulatorsCache.exit();
			m_transformsCache.  exit();
		}

		hkSpuDmaCache<hkVelocityAccumulator, HK_SPU_ACCUMULATOR_CACHE_NUM_CACHE_ROWS, HK_SPU_ACCUMULATOR_CACHE_NUM_OVERFLOW_LINES> m_accumulatorsCache;
		hkSpuDmaCache<hkTransform,           HK_SPU_ACCUMULATOR_CACHE_NUM_CACHE_ROWS, HK_SPU_ACCUMULATOR_CACHE_NUM_OVERFLOW_LINES> m_transformsCache;

		hkBuildJacobiansFromTaskDataSet m_dataSet[2];
	};



}


	// returns the number of inactive frames
int HK_CALL hkSpuIntegrateMotionImpl(	const hkSolverInfo&			solverInfo,
										const hkStepInfo&			stepInfo, 
										const hkEntity*const*		entitiesBatchInMainMemory,
										int							numEntities,
										hkVelocityAccumulator*		accumulators)
{
	//
	// get the job-specific subarray of the island's entity list from main memory
	// note: this will NOT get the entities themselves from main memory, only their addresses (as one contiguous block)!
	//
	hkEntity** entities;
	HK_CPU_PTR(hkEntity**) entitiesArrayInMainMemory;
	int entitiesArraySize;
	{
		entitiesArrayInMainMemory	= (HK_CPU_PTR(hkEntity**))( hkUlong(entitiesBatchInMainMemory));
		entitiesArraySize			= int( HK_NEXT_MULTIPLE_OF(16, (numEntities * sizeof(HK_CPU_PTR(hkEntity*))) ) );

		entities = static_cast<hkEntity**>(hkSpuStack::getInstance().allocateStack( entitiesArraySize, "hkSpuBuildAccumulatorsJob::entity array" ));

		hkSpuDmaManager::getFromMainMemory( entities, entitiesArrayInMainMemory, entitiesArraySize, hkSpuDmaManager::READ_ONLY, hkSpuIntegrateMotionJobDmaGroups::GET_ENTITY_BATCH_DMA_GROUP );
	}

	//
	// setup triple-buffering
	//
	hkSpuIntegrateDataSet* prefetchSet;
	hkSpuIntegrateDataSet* workingSet;
	hkSpuIntegrateDataSet* writeSet;
	{
		hkSpuIntegrateDataSetTool* tool = reinterpret_cast<hkSpuIntegrateDataSetTool*>(hkSpuStack::getInstance().allocateStack( sizeof(hkSpuIntegrateDataSetTool), "hkSpuIntegrateMotionImpl::buffers" ));
		tool->init(hkSpuIntegrateMotionJobDmaGroups::READ_WRITE_MOTIONS_BASE_DMA_GROUP);
		prefetchSet = &tool->m_dataSet[0];
		workingSet  = &tool->m_dataSet[1];
		writeSet    = &tool->m_dataSet[2];
	}

	// wait until island's entity list has arrived from main memory
	hkSpuDmaManager::waitForDmaCompletion(hkSpuIntegrateMotionJobDmaGroups::GET_ENTITY_BATCH_DMA_GROUP );

	// total number of motions to process
	int numMotionsLeftToPrefetch = numEntities;

	//
	// Prefetch motions for the first iteration block
	//
	{
		// calculate block size: either 'full blocksize' or 'remaining # of entities'
		workingSet->m_numMotions = hkMath::min2(HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE, numMotionsLeftToPrefetch);

		// get all motions (for FIRST iteration block) from main memory, one by one
		{
			for (int i = 0; i < workingSet->m_numMotions; i++)
			{
				HK_CPU_PTR(hkMotion*) motionInMainMemory = hkAddByteOffsetCpuPtr( reinterpret_cast<HK_CPU_PTR(hkMotion*)>(entities[i]), HK_OFFSET_OF(hkEntity, m_motion) );
				hkSpuDmaManager::getFromMainMemory(&workingSet->m_motionsBuffer[i], motionInMainMemory, sizeof(hkMaxSizeMotion), hkSpuDmaManager::READ_WRITE, workingSet->m_readWriteMotionsDmaGroup);
			}
		}
		numMotionsLeftToPrefetch -= workingSet->m_numMotions;
	}

	//
	// Iterate over all motions
	//
	int minNumInactiveFrames = 0x7fffff;
	{
		for ( int blockEntityIdx = 0; blockEntityIdx < numEntities; blockEntityIdx += HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE )
		{

			//
			// Prefetch motions for the next iteration block
			//
			if ( numMotionsLeftToPrefetch > 0 )
			{
				// calculate block size: either 'full blocksize' or 'remaining # of motions'
				prefetchSet->m_numMotions = hkMath::min2(HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE, numMotionsLeftToPrefetch);

				const int idxNextBlockToPrefetch = blockEntityIdx + HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE;

				// wait until all write operations on this (previous write) set have finished before we start loading data into it
				hkSpuDmaManager::waitForDmaCompletion(prefetchSet->m_readWriteMotionsDmaGroup);

				// get all motions (for NEXT iteration block) from main memory, one by one
				{
					for (int i = 0; i < prefetchSet->m_numMotions; i++)
					{
						HK_CPU_PTR(hkMotion*) motionInMainMemory = hkAddByteOffsetCpuPtr( reinterpret_cast<HK_CPU_PTR(hkMotion*)>(entities[idxNextBlockToPrefetch+i]), HK_OFFSET_OF(hkEntity, m_motion) );
						hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &prefetchSet->m_motionsBuffer[i], sizeof(hkMaxSizeMotion));
						hkSpuDmaManager::getFromMainMemory(&prefetchSet->m_motionsBuffer[i], motionInMainMemory, sizeof(hkMaxSizeMotion), hkSpuDmaManager::READ_WRITE, prefetchSet->m_readWriteMotionsDmaGroup);
					}
				}
				numMotionsLeftToPrefetch -= prefetchSet->m_numMotions;
			}

			// wait until all motions (for THIS iteration block) have arrived
			hkSpuDmaManager::waitForDmaCompletion(workingSet->m_readWriteMotionsDmaGroup);

			// integrate motions
			{
				hkMotion*const*	firstMotion	= &workingSet->m_motionsArray[0];
				const int		numMotions	=  workingSet->m_numMotions;

				int numInactiveFrames = hkRigidMotionUtilApplyAccumulators(solverInfo, stepInfo, accumulators, firstMotion, numMotions, 0);
				minNumInactiveFrames = hkMath::min2( minNumInactiveFrames, numInactiveFrames );
			}

			// put all motions back to main memory
			{
				for (int i = 0; i < workingSet->m_numMotions; i++)
				{
					HK_CPU_PTR(hkMotion*) motionInMainMemory = hkAddByteOffsetCpuPtr( reinterpret_cast<HK_CPU_PTR(hkMotion*)>(entities[blockEntityIdx+i]), HK_OFFSET_OF(hkEntity, m_motion) );
					hkSpuDmaManager::putToMainMemory(motionInMainMemory, &workingSet->m_motionsBuffer[i], sizeof(hkMaxSizeMotion), hkSpuDmaManager::WRITE_BACK, workingSet->m_readWriteMotionsDmaGroup);
				}
			}

			// advance accumulators pointer accordingly
			accumulators += workingSet->m_numMotions;

			// rotate the three buffers
			{
				hkSpuIntegrateDataSet* buffer = writeSet;
				writeSet    = workingSet;
				workingSet  = prefetchSet;
				prefetchSet = buffer;
			}
		}
	}

	// wait until all motions have been written back to main memory
	//@@@PS3SF: <todo: replace this with a combined call to both dma groups
	hkSpuDmaManager::waitForDmaCompletion( prefetchSet->m_readWriteMotionsDmaGroup );
	hkSpuDmaManager::waitForDmaCompletion( workingSet ->m_readWriteMotionsDmaGroup );
	hkSpuDmaManager::waitForDmaCompletion( writeSet   ->m_readWriteMotionsDmaGroup );

#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	{
		for (int i =0; i < HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE; i++)
		{
			hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &prefetchSet->m_motionsBuffer[i], sizeof(hkMaxSizeMotion));
			hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &workingSet ->m_motionsBuffer[i], sizeof(hkMaxSizeMotion));
			hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &writeSet   ->m_motionsBuffer[i], sizeof(hkMaxSizeMotion));
		}
	}

	// perform final checks on READ_ONLY and READ_COPY data
	hkSpuDmaManager::performFinalChecks( entitiesArrayInMainMemory, entities, entitiesArraySize );
#endif

	hkSpuStack::getInstance().deallocateStack( sizeof(hkSpuIntegrateDataSetTool) );
	hkSpuStack::getInstance().deallocateStack( entities                       );

	return minNumInactiveFrames;
}



static HK_FORCE_INLINE hkJobQueue::JobStatus hkSpuIntegrateMotionJob(	const hkSolverInfo&					info,
																		const hkStepInfo&					stepInfo,
																			  hkJobQueue&					jobQueue,
																			  hkJobQueue::JobQueueEntry&	jobInOut,
																			  hkJobQueue::WaitStatus		waitStatus)
{
	HK_TIMER_BEGIN_LIST("Simulate", "Integrate");
	HK_TIMER_BEGIN("IntMotion", HK_NULL);

	hkIntegrateMotionJob& job = const_cast<hkIntegrateMotionJob&>( reinterpret_cast<const hkIntegrateMotionJob&>(jobInOut) );

	// get the taskHeader from main memory
	// note: the mode needs to be READ_COPY, as the m_exportFinished member of hkBuildJacobianTaskHeader is modified in between
	HK_ALIGN16(char taskHeaderBuffer[sizeof(hkBuildJacobianTaskHeader)]);
	hkBuildJacobianTaskHeader& taskHeader = (hkBuildJacobianTaskHeader&)taskHeaderBuffer;
	hkSpuDmaManager::getFromMainMemory( &taskHeader, job.m_taskHeader, sizeof(taskHeader), hkSpuDmaManager::READ_COPY, hkSpuIntegrateMotionJobDmaGroups::GET_TASK_HEADER_DMA_GROUP );

	int firstEntityIdx = job.m_firstEntityIdx;
	int numEntities    = job.m_numEntities;

	int accumulatorsBufferSize = numEntities * sizeof(hkVelocityAccumulator);
	char* accumulatorsBuffer = (char*)hkSpuStack::getInstance().allocateStack(accumulatorsBufferSize, "IntegrateMotionJob");

	// wait until taskHeader has arrived from main memory
	hkSpuDmaManager::waitForDmaCompletion( hkSpuIntegrateMotionJobDmaGroups::GET_TASK_HEADER_DMA_GROUP );

	// get accumulator subarray from main memory
 	hkVelocityAccumulator* accumulators;
	HK_CPU_PTR(void*) accumulatorStartInMainMemory;
	{
		int accumulatorOffset = (1+firstEntityIdx) * sizeof(hkVelocityAccumulator); // +1 to skip the first (fixed) accumulator
		accumulatorStartInMainMemory = hkAddByteOffsetCpuPtr(taskHeader.m_accumulatorsBase, accumulatorOffset);
		hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion(accumulatorsBuffer, accumulatorStartInMainMemory, accumulatorsBufferSize, hkSpuDmaManager::READ_ONLY);
		accumulators = reinterpret_cast<hkVelocityAccumulator*>( accumulatorsBuffer );
	}

	{
		int entitiesOffset = firstEntityIdx * sizeof(HK_CPU_PTR(hkEntity*));
		hkEntity*const* firstEntityInMainMemory = (HK_CPU_PTR(hkEntity**))( hkUlong(taskHeader.m_allEntities) + entitiesOffset );
		job.m_numInactiveFrames = hkSpuIntegrateMotionImpl(info, stepInfo, firstEntityInMainMemory, numEntities, accumulators);

		hkSpuDmaManager::performFinalChecks( accumulatorStartInMainMemory, accumulators, accumulatorsBufferSize);
	}

	hkSpuStack::getInstance().deallocateStack(accumulatorsBufferSize);

	hkSpuDmaManager::performFinalChecks( job.m_taskHeader, &taskHeader, sizeof(hkBuildJacobianTaskHeader));

	HK_TIMER_END();
	HK_TIMER_END_LIST();

	return jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_SPU, &jobInOut, jobInOut, 0, waitStatus );
}



static HK_FORCE_INLINE hkJobQueue::JobStatus hkSpuSolveConstraintsJob(	const hkSolverInfo&					info,
																		const hkStepInfo&					stepInfo,
																		      hkJobQueue&					jobQueue,
																		      hkJobQueue::JobQueueEntry&	jobInOut,
																		      hkJobQueue::WaitStatus		waitStatus)
{
	const hkSolveConstraintsJob& job = reinterpret_cast<hkSolveConstraintsJob&>(jobInOut);

	HK_TIMER_BEGIN_LIST("Simulate", "Integrate");
	HK_TIMER_BEGIN("Solve", HK_NULL);

	// save some of the data in the job (as job is modified below when transforming to broadphase job)
	HK_CPU_PTR(void*)                      solverBufferInMainMemory	= job.m_buffer;
	int                                    solverBufferSize			= job.m_bufferSize;
	HK_CPU_PTR(hkBuildJacobianTaskHeader*) taskHeaderInMainMemory	= job.m_taskHeader;

	// get the taskHeader from main memory
	// taskHeader is only needed in hkSpuIntegrateMotionImpl() but we start the DMA as early as possible
	// note: the mode needs to be READ_COPY, as the m_exportFinished member of hkBuildJacobianTaskHeader is modified in between
	HK_ALIGN16(char taskHeaderBuffer[sizeof(hkBuildJacobianTaskHeader)]);
	hkBuildJacobianTaskHeader& taskHeader = (hkBuildJacobianTaskHeader&)taskHeaderBuffer;
	hkSpuDmaManager::getFromMainMemory( &taskHeader, taskHeaderInMainMemory, sizeof(taskHeader), hkSpuDmaManager::READ_COPY, hkSpuSolveJobDmaGroups::GET_TASK_HEADER_DMA_GROUP );

	//printf("SPU solve job");
	//HK_SPU_DEBUG_PRINTF(("buffer size = %d\n", solverBufferSize));

	//HK_SPU_DEBUG_PRINTF(("[%d] Solve constraints job. Island: %d\n", g_hkSpuId, job.m_islandIndex));

		// check for free size. synchronize with assert 0xf023edff and 0xf023fdff
	HK_ASSERT( 0xf023edfe, hkSpuStack::getInstance().getStackFreeSize() == HK_SPU_TOTAL_BUFFER_SIZE);

	char* solverBuffer = (char*)hkSpuStack::getInstance().allocateStack(solverBufferSize, "SolverBuffer");

	hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion(solverBuffer, solverBufferInMainMemory, solverBufferSize, hkSpuDmaManager::READ_COPY);

	hkVelocityAccumulator* accumulators = reinterpret_cast<hkVelocityAccumulator*>( hkAddByteOffset(solverBuffer, job.m_accumulatorsOffset) );
	hkJacobianElement*	   jacobians    = reinterpret_cast<hkJacobianElement*>    ( hkAddByteOffset(solverBuffer, job.m_jacobiansOffset)    );
	hkSolverElemTemp*	   solverTemp   = reinterpret_cast<hkSolverElemTemp*>     ( hkAddByteOffset(solverBuffer, job.m_solverTempOffset)   );
	hkJacobianSchema*	   schemas      = reinterpret_cast<hkJacobianSchema*>     ( hkAddByteOffset(solverBuffer, job.m_schemasOffset)      );

	//HK_TIMER_BEGIN_LIST("Integrate", "ZeroSolverRes" );

	//
	// zero solver results
	//
	{
	
		const int numQuadSolverResults = int( HK_NEXT_MULTIPLE_OF(16, job.m_numSolverResults * sizeof(float)) ) >> 4;
		hkVector4* quadSolverTemps = reinterpret_cast<hkVector4*>(solverTemp);
		for (int i = 0; i < numQuadSolverResults; i++)
		{
			quadSolverTemps->setZero4();
			quadSolverTemps++;
		}
	}
	{
		//HK_TIMER_SPLIT_LIST("Solve");

		hkSolveConstraints( info, schemas, accumulators, jacobians, solverTemp );

		//HK_MONITOR_ADD_VALUE( "NumJacobians", float(island->m_numSolverResults), HK_MONITOR_TYPE_INT );
		//HK_MONITOR_ADD_VALUE( "NumEntities",  float(island->getEntities().getSize()), HK_MONITOR_TYPE_INT );
	}
	//HK_TIMER_END_LIST();

	// wait until taskHeader has arrived from main memory
	hkSpuDmaManager::waitForDmaCompletion( hkSpuSolveJobDmaGroups::GET_TASK_HEADER_DMA_GROUP );

	HK_TIMER_SPLIT_LIST("IntMotion");

	// integrate motions
	int numInactiveFrames = hkSpuIntegrateMotionImpl(info, stepInfo, taskHeader.m_allEntities, taskHeader.m_numAllEntities, &accumulators[1]);

	// start hkIntegrateMotionJob in parallel to solver export
	// note: we won't actually start this job, just pretend to so that the job queue will execute the job's 'finish' code section;
	//       we therefore do not have to initialize all the parameters
	{
		hkIntegrateMotionJob motionJob( job, taskHeaderInMainMemory, 0, taskHeader.m_numAllEntities, HK_NULL );
		motionJob.m_numInactiveFrames = numInactiveFrames;
		hkJobQueue::JobQueueEntry jobOut;
		jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_INVALID, (hkJobQueue::JobQueueEntry*)&motionJob, jobOut );
	}

	HK_TIMER_SPLIT_LIST("Export");

	// export solver data
	hkExportImpulsesAndRhs(info, solverTemp, schemas, accumulators, jacobians);

	// free local resources
	{
		hkSpuDmaManager::performFinalChecks( solverBufferInMainMemory, solverBuffer, solverBufferSize );
		hkSpuStack::getInstance().deallocateStack(solverBufferSize);
	}

	hkSpuDmaManager::performFinalChecks( taskHeaderInMainMemory, &taskHeader, sizeof(hkBuildJacobianTaskHeader));

	HK_TIMER_END();
	HK_TIMER_END_LIST();

	return jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_SPU, &jobInOut, jobInOut, 0, waitStatus );
}



static HK_LOCAL_INLINE void HK_CALL hkSpuBuildJacobiansFromTask(const hkBuildJacobiansFromTaskInput& input, hkConstraintQueryIn &queryIn)
{
	const hkBuildJacobianTask::AtomInfo* atomInfos = input.m_atomInfosOnSpu;
	int numAtomInfos = input.m_numAtomInfos;

	hkBuildJacobiansFromTaskDataSetTool* dataSetTool = reinterpret_cast<hkBuildJacobiansFromTaskDataSetTool*>(hkSpuStack::getInstance().allocateStack( sizeof(hkBuildJacobiansFromTaskDataSetTool), "hkBuildJacobiansFromTaskDataSetTool" ));
	dataSetTool->init(hkSpuBuildJacobiansJobDmaGroups::ACCUMULATORS_CACHE_BASE_DMA_GROUP, hkSpuBuildJacobiansJobDmaGroups::NUM_ACCUMULATORS_DMA_GROUPS, hkSpuBuildJacobiansJobDmaGroups::READ_RUNTIME_AND_ATOMS_DMA_GROUP);

	hkSpuDmaCache<hkVelocityAccumulator, HK_SPU_ACCUMULATOR_CACHE_NUM_CACHE_ROWS, HK_SPU_ACCUMULATOR_CACHE_NUM_OVERFLOW_LINES> *accumulatorsCache = &dataSetTool->m_accumulatorsCache;
 	hkSpuDmaCache<hkTransform,           HK_SPU_ACCUMULATOR_CACHE_NUM_CACHE_ROWS, HK_SPU_ACCUMULATOR_CACHE_NUM_OVERFLOW_LINES> *transformsCache   = &dataSetTool->m_transformsCache;

	hkBuildJacobiansFromTaskDataSet* prefetchSet = &dataSetTool->m_dataSet[0];
	hkBuildJacobiansFromTaskDataSet* workingSet  = &dataSetTool->m_dataSet[1];

	//
	// Prefetch accumulators, runtime and instance for the first constraint
	//
	{
		HK_ASSERT( 0xf0f02334, numAtomInfos > 0 );
		const hkBuildJacobianTask::AtomInfo& atomInfo = atomInfos[0];

		workingSet->m_accumulatorA = accumulatorsCache->getFromMainMemory( hkAddByteOffsetCpuPtr( input.m_accumulatorsInMainMemory.val(), atomInfo.m_accumulatorOffsetA) );
		workingSet->m_accumulatorB = accumulatorsCache->getFromMainMemory( hkAddByteOffsetCpuPtr( input.m_accumulatorsInMainMemory.val(), atomInfo.m_accumulatorOffsetB) );

 		workingSet->m_transformA = transformsCache->getFromMainMemory( atomInfo.m_transformA );
 		workingSet->m_transformB = transformsCache->getFromMainMemory( atomInfo.m_transformB );

		if ( atomInfo.m_runtime )
		{
			const int runtimeSize = HK_NEXT_MULTIPLE_OF(16, atomInfo.m_runtimeSize);
			HK_ASSERT2(0x589d577a, runtimeSize <= HK_SPU_CONSTRAINT_RUNTIME_BUFFER_SIZE, "Runtime doesn't fit into static buffer on spu.");
			hkSpuDmaManager::getFromMainMemory(&workingSet->m_constraintRuntimeBuffer[0], atomInfo.m_runtime, runtimeSize, hkSpuDmaManager::READ_WRITE, workingSet->m_readRuntimeAndAtomsDmaGroup );
			workingSet->m_runtimeBuffer = &workingSet->m_constraintRuntimeBuffer[0];
		}
		else
		{
			workingSet->m_runtimeBuffer = HK_NULL;
		}
		const int atomsSize = HK_NEXT_MULTIPLE_OF(16, atomInfo.m_atomsSize);
		HK_ASSERT2(0x5b29c273, atomsSize <= HK_SPU_CONSTRAINT_ATOM_BUFFER_SIZE, "Atoms don't fit into static buffer on spu.");
		hkSpuDmaManager::getFromMainMemory(&workingSet->m_constraintAtomsBuffer[0], atomInfo.m_atoms, atomsSize, hkSpuDmaManager::READ_ONLY, workingSet->m_readRuntimeAndAtomsDmaGroup );
	}

	//
	// Iterate over all atom infos
	//
	for (int a = 0; a < numAtomInfos; a++)
	{
		const hkBuildJacobianTask::AtomInfo& atomInfo = atomInfos[a];

		//
		// Prefetch accumulators, runtime and atoms for the next constraint 
		//
		if (a < numAtomInfos - 1)
		{
			const hkBuildJacobianTask::AtomInfo& nextAtomInfo = atomInfos[a+1];

			prefetchSet->m_accumulatorA = accumulatorsCache->getFromMainMemory( hkAddByteOffsetCpuPtr( input.m_accumulatorsInMainMemory.val(), nextAtomInfo.m_accumulatorOffsetA) );
			prefetchSet->m_accumulatorB = accumulatorsCache->getFromMainMemory( hkAddByteOffsetCpuPtr( input.m_accumulatorsInMainMemory.val(), nextAtomInfo.m_accumulatorOffsetB) );

			prefetchSet->m_transformA = transformsCache->getFromMainMemory( nextAtomInfo.m_transformA );
			prefetchSet->m_transformB = transformsCache->getFromMainMemory( nextAtomInfo.m_transformB );

			// wait for all atom writebacks of the previous iteration to be finished; this could also be done as double-buffered, but is not for now :)
			//@@@PS3SF: <todo: replace this with a combined call to both dma groups
			hkSpuDmaManager::waitForDmaCompletion( hkSpuBuildJacobiansJobDmaGroups::WRITE_BACK_CONTACT_CONSTRAINT_ATOM_DMA_GROUP );
			hkSpuDmaManager::waitForDmaCompletion( hkSpuBuildJacobiansJobDmaGroups::WRITE_BACK_RUNTIME_DMA_GROUP                 );
			hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &prefetchSet->m_constraintAtomsBuffer[0], 0);

			if ( nextAtomInfo.m_runtime )
			{
				const int runtimeSize = HK_NEXT_MULTIPLE_OF(16, nextAtomInfo.m_runtimeSize);
				HK_ASSERT2(0x48479a4c, runtimeSize <= HK_SPU_CONSTRAINT_RUNTIME_BUFFER_SIZE, "Runtime doesn't fit into static buffer on spu.");

				hkSpuDmaManager::waitForDmaCompletion( prefetchSet->m_readRuntimeAndAtomsDmaGroup ); //@@@PS3SF: <todo: is this wait actually necessary? we are never writing back using this dma group and the correct check should be the one a few lines above
				hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &prefetchSet->m_constraintRuntimeBuffer[0], 0 );

				hkSpuDmaManager::getFromMainMemory(&prefetchSet->m_constraintRuntimeBuffer[0], nextAtomInfo.m_runtime, runtimeSize, hkSpuDmaManager::READ_WRITE, prefetchSet->m_readRuntimeAndAtomsDmaGroup );
				prefetchSet->m_runtimeBuffer = &prefetchSet->m_constraintRuntimeBuffer[0];
			}
			else
			{
				prefetchSet->m_runtimeBuffer = HK_NULL;
			}
			const int atomsSize = HK_NEXT_MULTIPLE_OF(16, nextAtomInfo.m_atomsSize);
			HK_ASSERT2(0x7bfa658b, atomsSize <= HK_SPU_CONSTRAINT_ATOM_BUFFER_SIZE, "Atoms don't fit into static buffer on spu.");
			hkSpuDmaManager::getFromMainMemory(&prefetchSet->m_constraintAtomsBuffer[0], nextAtomInfo.m_atoms, atomsSize, hkSpuDmaManager::READ_ONLY, prefetchSet->m_readRuntimeAndAtomsDmaGroup );
		}

		// prepare queryIn
		{
			queryIn.m_transformA                    = workingSet->m_transformA;
			queryIn.m_transformB                    = workingSet->m_transformB;

			queryIn.m_bodyA                         = const_cast<hkVelocityAccumulator*>(workingSet->m_accumulatorA.val());
			queryIn.m_bodyB                         = const_cast<hkVelocityAccumulator*>(workingSet->m_accumulatorB.val());
			queryIn.m_bodyAOffset                   = atomInfo.m_accumulatorOffsetA;
			queryIn.m_bodyBOffset                   = atomInfo.m_accumulatorOffsetB;

			queryIn.m_constraintInstance            = HK_NULL;
			queryIn.m_constraintRuntime             = workingSet->m_runtimeBuffer;

			queryIn.m_constraintRuntimeInMainMemory = (HK_CPU_PTR(void*))(atomInfo.m_runtime);
#		if defined (HK_PLATFORM_HAS_SPU)
			queryIn.m_atomInMainMemory              = atomInfo.m_atoms;
#		endif
		}

		hkConstraintQueryOut queryOut;
		{
			queryOut.m_jacobians            = static_cast<hkJacobianElement*>(input.m_jacobianWriter->requestBuffer());
			queryOut.m_jacobiansVirtualBase = static_cast<hkJacobianElement*>(input.m_jacobianWriter->getVirtualBase());
			queryOut.m_jacobianSchemas      = static_cast<hkJacobianSchema*> (input.m_schemaWriter  ->requestBuffer());
		}

		// wait for runtime, atoms and accumulators
		hkSpuDmaManager::  waitForDmaCompletion( workingSet->m_readRuntimeAndAtomsDmaGroup );
		accumulatorsCache->waitForDmaCompletion( workingSet->m_accumulatorA );
		accumulatorsCache->waitForDmaCompletion( workingSet->m_accumulatorB );
 		transformsCache  ->waitForDmaCompletion( workingSet->m_transformA );
 		transformsCache  ->waitForDmaCompletion( workingSet->m_transformB );

		{
			hkConstraintAtom* atoms = reinterpret_cast<hkConstraintAtom*>(&workingSet->m_constraintAtomsBuffer[0]);
			int atomsSize = atomInfo.m_atomsSize;
			hkSolverBuildJacobianFromAtoms( atoms, atomsSize, queryIn, queryOut );
		}

		accumulatorsCache->releaseObjectAt(workingSet->m_accumulatorA);
		accumulatorsCache->releaseObjectAt(workingSet->m_accumulatorB);
		transformsCache  ->releaseObjectAt(workingSet->m_transformA);
		transformsCache  ->releaseObjectAt(workingSet->m_transformB);

		input.m_jacobianWriter->finishWrite(queryOut.m_jacobians);
		input.m_schemaWriter  ->finishWrite(queryOut.m_jacobianSchemas);

		// switch prefetch buffers & dma group
		hkBuildJacobiansFromTaskDataSet* h = workingSet; workingSet = prefetchSet; prefetchSet = h;

	}

	//
	// Add "End" or "Goto" schema
	//
	{
		void* bufferEnd;
		if ( !input.m_nextTask )
		{
			// no 'next task' -> write end schema
			hkJacobianSchema* endSchema = static_cast<hkJacobianSchema*>(input.m_schemaWriter->requestBuffer());
			*(reinterpret_cast<hkInt32*>(endSchema)) = 0;
			bufferEnd = endSchema+1;
			goto FINISH_WRITE;
		}
		else
		{
			int branchOffset = hkGetByteOffsetInt( input.m_schemaWriter->getCurrentDestInMainMemory(), input.m_schemasOfNextTask);
			if ( branchOffset >= 256 )
			{
				// add Goto schema
				hkJacobianGotoSchema* gotoSchema = static_cast<hkJacobianGotoSchema*>(input.m_schemaWriter->requestBuffer());
				gotoSchema->initOffset(branchOffset);
				bufferEnd = gotoSchema+1;
				goto FINISH_WRITE;
			}
			else if ( branchOffset > 0 )
			{
				// add 8bit Goto schema
				hkJacobianGoto8Schema* goto8Schema = static_cast<hkJacobianGoto8Schema*>(input.m_schemaWriter->requestBuffer());
				goto8Schema->initOffset(branchOffset);
				bufferEnd = goto8Schema+1;
FINISH_WRITE:
				input.m_schemaWriter->finishWrite(bufferEnd);
			}
		}
#ifdef HK_DEBUG
		HK_CPU_PTR(void*) testEnd = input.m_schemaWriter->getCurrentDestInMainMemory();
		HK_ASSERT(0xaf6451ed, testEnd <= input.m_schemasOfNextTask);
#endif
	}

	input.m_jacobianWriter->flush();
	input.m_schemaWriter->flush();

	// wait for all remaining atom writebacks to be finished
	hkSpuDmaManager::waitForAllDmaCompletion( );

	hkSpuDmaManager::tryToPerformFinalChecks(0, &workingSet ->m_constraintRuntimeBuffer[0], 0 );
	hkSpuDmaManager::tryToPerformFinalChecks(0, &prefetchSet->m_constraintRuntimeBuffer[0], 0 );
	hkSpuDmaManager::tryToPerformFinalChecks(0, &workingSet ->m_constraintAtomsBuffer[0], 0 );
	hkSpuDmaManager::tryToPerformFinalChecks(0, &prefetchSet->m_constraintAtomsBuffer[0], 0 );

	dataSetTool->exit();
	hkSpuStack::getInstance().deallocateStack( sizeof(hkBuildJacobiansFromTaskDataSetTool) );

}

static hkJobQueue::JobStatus HK_CALL hkSpuBuildJacobiansJob(	hkJobQueue&					jobQueue, 
																hkJobQueue::JobQueueEntry&	jobInOut,
																hkJobQueue::WaitStatus		waitStatus)
{
	HK_TIMER_BEGIN_LIST("Simulate", "Integrate");
	HK_TIMER_BEGIN("BuildJac", HK_NULL)
	const hkBuildJacobiansJob& job = reinterpret_cast<hkBuildJacobiansJob&>(jobInOut);

	//HK_SPU_DEBUG_PRINTF(("Build jacobians job. Island: %d\n", job.m_islandIndex));

	HK_ALIGN16( hkConstraintQueryIn queryIn );
	hkSpuDmaManager::getFromMainMemory( &queryIn, job.m_constraintQueryIn, sizeof(hkConstraintQueryIn), hkSpuDmaManager::READ_COPY, hkSpuBuildJacobiansJobDmaGroups::GET_QUERYIN_DMA_GROUP );

	hkBuildJacobianTask* task = job.m_buildJacobianTask;

	{

		//
		// create dma writers for jacobians and schemas
		//
		hkSpuDmaWriter jacobiansWriter;
		hkSpuDmaWriter schemasWriter;
		void* jacobiansBuffer;
		void* schemasBuffer;
		{
			// We use double buffering so we need twice the amount of memory
			const int sizeJacobianBuffer = 2 * (HK_SPU_JACOBIAN_WRITER_BASE_BUFFER_SIZE + HK_SPU_JACOBIAN_WRITER_OVERFLOW_BUFFER_SIZE);
			const int sizeSchemaBuffer   = 2 * (HK_SPU_SCHEMA_WRITER_BASE_BUFFER_SIZE + HK_SPU_SCHEMA_WRITER_OVERFLOW_BUFFER_SIZE);

			jacobiansBuffer = hkSpuStack::getInstance().allocateStack(sizeJacobianBuffer, "sizeJacobianBuffer");
			schemasBuffer   = hkSpuStack::getInstance().allocateStack(sizeSchemaBuffer, "sizeSchemaBuffer");

			// size of destination buffer in main memory is only needed for debugging (to check against overflows in the destination buffer), so we simply set it to -1
			jacobiansWriter.init(task->m_jacobiansBase, task->m_jacobians,  -1, jacobiansBuffer, HK_SPU_JACOBIAN_WRITER_BASE_BUFFER_SIZE, HK_SPU_JACOBIAN_WRITER_OVERFLOW_BUFFER_SIZE, 0, 1);
			schemasWriter  .init(task->m_schemas,       task->m_schemas,    -1, schemasBuffer,   HK_SPU_SCHEMA_WRITER_BASE_BUFFER_SIZE,   HK_SPU_SCHEMA_WRITER_OVERFLOW_BUFFER_SIZE,   2, 3);
		}

		hkBuildJacobiansFromTaskInput input;
		{
			input.m_atomInfosOnSpu           = task->m_atomInfos;
			input.m_numAtomInfos             = task->m_numAtomInfos;

			input.m_accumulatorsInMainMemory = task->m_accumulators;

			input.m_jacobianWriter           = &jacobiansWriter;
			input.m_schemaWriter             = &schemasWriter;

			input.m_schemasOfNextTask        = task->m_schemasOfNextTask;
			input.m_nextTask                 = task->m_next;
		}

		hkSpuDmaManager::waitForDmaCompletion(hkSpuBuildJacobiansJobDmaGroups::GET_QUERYIN_DMA_GROUP );

		// hkSpuBuildJacobiansFromTask() writes back data to main memory immediately via the jacobian & schema writers
		hkSpuBuildJacobiansFromTask(input, queryIn);

		//
		// free stack memory used by writers
		//
		{
			hkSpuStack::getInstance().deallocateStack(schemasBuffer);
			hkSpuStack::getInstance().deallocateStack(jacobiansBuffer);
		}
	}

	hkSpuDmaManager::performFinalChecks( job.m_constraintQueryIn, &queryIn, sizeof(hkConstraintQueryIn));

	HK_TIMER_END();
	HK_TIMER_END_LIST();

	return jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_SPU, &jobInOut, jobInOut, 0, waitStatus );
}



hkJobQueue::JobStatus HK_CALL hkSpuBuildAccumulatorsJob(	const hkStepInfo&			stepInfo, 
															hkJobQueue&					jobQueue, 
															hkJobQueue::JobQueueEntry&	jobInOut,
															hkJobQueue::WaitStatus		waitStatus)
{
	const hkBuildAccumulatorsJob& job = reinterpret_cast<hkBuildAccumulatorsJob&>(jobInOut);

	HK_TIMER_BEGIN_LIST("Simulate", "Integrate");
	HK_TIMER_BEGIN("BuildAccum", HK_NULL);

	// we are dealing with a maximum size of entities per job here (ACCUMULATORS_PER_JOB), so we can simply use a regular array for storing the array of entity pointers
	HK_ALIGN16( hkEntity* entities[hkBuildAccumulatorsJob::ACCUMULATORS_PER_JOB] );

	//
	// get the job-specific subarray of the island's entity list from main memory
	// note: this will NOT get the entities themselves from main memory, only their addresses (as one contiguous block)!
	//
	HK_CPU_PTR(hkEntity**) entitiesArrayInMainMemory;
	int entitiesArraySize;
	{
		entitiesArrayInMainMemory	= (HK_CPU_PTR(hkEntity**))( hkUlong(job.m_islandEntitiesArray) + job.m_firstEntityIdx * sizeof(HK_CPU_PTR(hkEntity*)) );
		entitiesArraySize			= int( HK_NEXT_MULTIPLE_OF(16, (job.m_numEntities * sizeof(HK_CPU_PTR(hkEntity*))) ) );

		HK_ASSERT( 0xaffeea24, entitiesArraySize <= sizeof(entities) );

		hkSpuDmaManager::getFromMainMemory( &entities[0], entitiesArrayInMainMemory, entitiesArraySize, hkSpuDmaManager::READ_ONLY, hkSpuBuildAccumulatorsJobDmaGroups::GET_ENTITY_BATCH_DMA_GROUP );
	}

	// get the task-header from main memory
	// note: the mode needs to be READ_COPY, as the m_openJobs member of hkBuildJacobianTaskHeader might be modified in the meantime by other jobs
	HK_ALIGN16(char taskHeaderBuffer[sizeof(hkBuildJacobianTaskHeader)]);
	hkBuildJacobianTaskHeader& taskHeader = (hkBuildJacobianTaskHeader&)taskHeaderBuffer;
	hkSpuDmaManager::getFromMainMemory( &taskHeader, job.m_taskHeader, sizeof(taskHeader), hkSpuDmaManager::READ_COPY, hkSpuBuildAccumulatorsJobDmaGroups::GET_TASK_HEADER_DMA_GROUP );

	//
	// setup double-buffering
	//
	hkBuildAccumulatorsDataSet* prefetchSet;
	hkBuildAccumulatorsDataSet* workingSet;
	{
	    hkBuildAccumulatorsDataSetTool* tool = reinterpret_cast<hkBuildAccumulatorsDataSetTool*>(hkSpuStack::getInstance().allocateStack( sizeof(hkBuildAccumulatorsDataSetTool),"hkSpuBuildAccumulatorsJob::buffers" ));
		tool->init(hkSpuBuildAccumulatorsJobDmaGroups::READ_MOTIONS_BASE_DMA_GROUP, hkSpuBuildAccumulatorsJobDmaGroups::WRITE_ACCUMULATORS_BASE_DMA_GROUP);
	    prefetchSet = &tool->m_dataSet[0];
	    workingSet  = &tool->m_dataSet[1];
	}

	// wait until island's entity list and task-header have arrived from main memory
	//@@@PS3SF: <todo: replace this with a combined call to both dma groups
	hkSpuDmaManager::waitForDmaCompletion( hkSpuBuildAccumulatorsJobDmaGroups::GET_ENTITY_BATCH_DMA_GROUP );
	hkSpuDmaManager::waitForDmaCompletion( hkSpuBuildAccumulatorsJobDmaGroups::GET_TASK_HEADER_DMA_GROUP  );

	// total number of motions to process
	int numMotionsLeftToPrefetch = job.m_numEntities;

	//
	// Prefetch motions for the first iteration block
	//
	{
		// calculate block size: either 'full blocksize' or 'remaining # of entities'
		workingSet->m_numMotions = hkMath::min2(HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE, numMotionsLeftToPrefetch);

		// get all motions (for FIRST iteration block) from main memory, one by one
		{
			for (int i = 0; i < workingSet->m_numMotions; i++)
			{
				HK_CPU_PTR(hkMotion*) motionInMainMemory = hkAddByteOffsetCpuPtr( reinterpret_cast<HK_CPU_PTR(hkMotion*)>(entities[i]), HK_OFFSET_OF(hkEntity, m_motion) );
				// the following READ_COPY should actually be a READ_ONLY; change was done only to get dampening (which modifies the motions) working
				hkSpuDmaManager::getFromMainMemory(&workingSet->m_motionsBuffer[i], motionInMainMemory, sizeof(hkMaxSizeMotion), hkSpuDmaManager::READ_COPY, workingSet->m_readMotionsDmaGroup);
			}
		}
		numMotionsLeftToPrefetch -= workingSet->m_numMotions;
	}

	//
	// Iterate over all motions
	//
	{
		for ( int blockEntityIdx = 0; blockEntityIdx < job.m_numEntities; blockEntityIdx += HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE )
		{

			//
			// Prefetch motions for the next iteration block
			//
			if ( numMotionsLeftToPrefetch > 0 )
			{
				// calculate block size: either 'full blocksize' or 'remaining # of motions'
				prefetchSet->m_numMotions = hkMath::min2(HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE, numMotionsLeftToPrefetch);

				const int idxNextBlockToPrefetch = blockEntityIdx + HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE;

				// get all motions (for NEXT iteration block) from main memory, one by one
				{
					for (int i = 0; i < prefetchSet->m_numMotions; i++)
					{
						HK_CPU_PTR(hkMotion*) motionInMainMemory = hkAddByteOffsetCpuPtr( reinterpret_cast<HK_CPU_PTR(hkMotion*)>(entities[idxNextBlockToPrefetch+i]), HK_OFFSET_OF(hkEntity, m_motion) );
						// the following READ_COPY should actually be a READ_ONLY; change was done only to get dampening (which modifies the motions) working
						//@@@PS3SF: <todo: move dampening someplace else (OS suggested collision detection?) and make this transaction READ_ONLY again
						hkSpuDmaManager::getFromMainMemory(&prefetchSet->m_motionsBuffer[i], motionInMainMemory, sizeof(hkMaxSizeMotion), hkSpuDmaManager::READ_COPY, prefetchSet->m_readMotionsDmaGroup);
					}
				}
				numMotionsLeftToPrefetch -= prefetchSet->m_numMotions;
			}

			// wait until all motions (for THIS iteration block) have arrived
			hkSpuDmaManager::waitForDmaCompletion(workingSet->m_readMotionsDmaGroup);

			// wait for last accumulators write on this buffer to be finished, so we can safely reuse it again (without overwriting data that has not been completely transfered back to ppu)
			hkSpuDmaManager::waitForDmaCompletion(workingSet->m_writeAccumulatorsDmaGroup);
			hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &workingSet->m_accumulators[0], 0);

			//
			// this utility function fills the accumulators buffer from only a subpart of the motions array
			//
			{
				hkMotion*const*					firstMotion					= &workingSet->m_motionsArray[0];
				const int						numMotions					=  workingSet->m_numMotions;
				hkVelocityAccumulator*			accumulators				= &workingSet->m_accumulators[0];

				hkRigidMotionUtilApplyForcesAndBuildAccumulators(stepInfo, firstMotion, numMotions, 0, accumulators );
			}

#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
			// perform final checks on (this iteration block's) motions
			for ( int i = 0; i < workingSet->m_numMotions; i++ )
			{
				HK_CPU_PTR(hkMotion*) motionInMainMemory = hkAddByteOffsetCpuPtr( reinterpret_cast<HK_CPU_PTR(hkMotion*)>(entities[blockEntityIdx+i]), HK_OFFSET_OF(hkEntity, m_motion) );
				hkSpuDmaManager::performFinalChecks(motionInMainMemory, &workingSet->m_motionsBuffer[i], sizeof(hkMaxSizeMotion));
			}
#endif

			// move newly built accumulators back to ppu
			HK_CPU_PTR(hkVelocityAccumulator*) dstInMainMemory = hkAddByteOffsetCpuPtr(taskHeader.m_accumulatorsBase, (1 + job.m_firstEntityIdx + blockEntityIdx) * sizeof(hkVelocityAccumulator) );
			hkSpuDmaManager::putToMainMemory(dstInMainMemory, &workingSet->m_accumulators[0], workingSet->m_numMotions*sizeof(hkVelocityAccumulator), hkSpuDmaManager::WRITE_NEW, workingSet->m_writeAccumulatorsDmaGroup);

			// switch prefetch & working buffer and dma group
			hkBuildAccumulatorsDataSet* h = workingSet; workingSet = prefetchSet; prefetchSet = h;
		}
	}

	// wait for accumulator writes on both buffers to be finished
	hkSpuDmaManager::waitForDmaCompletion(workingSet ->m_writeAccumulatorsDmaGroup);
	hkSpuDmaManager::waitForDmaCompletion(prefetchSet->m_writeAccumulatorsDmaGroup);
	hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &prefetchSet->m_accumulators[0], 0);
	hkSpuDmaManager::tryToPerformFinalChecks(HK_NULL, &workingSet->m_accumulators[0], 0);

	// perform final checks on READ_ONLY and READ_COPY data
	hkSpuDmaManager::performFinalChecks( job.m_taskHeader, &taskHeader, sizeof(hkBuildJacobianTaskHeader));
	hkSpuDmaManager::performFinalChecks( entitiesArrayInMainMemory, &entities[0], entitiesArraySize );

	// free local memory
	hkSpuStack::getInstance().deallocateStack( sizeof(hkBuildAccumulatorsDataSetTool) );

	HK_TIMER_END();
	HK_TIMER_END_LIST();

	return jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_SPU, &jobInOut, jobInOut, 0, waitStatus );
}



hkJobQueue::JobStatus HK_CALL hkSpuProcessNextJob( hkJobQueue& queue, const HK_CPU_PTR(void*) worldDynamicsStepInfoAddress, hkJobQueue::WaitStatus waitStatus, int spuId )
{
	g_hkSpuId = spuId;

	HK_ALIGN( hkJobQueue::JobQueueEntry job, 128 );

	queue.m_jobAddFunc    = hkJobQueueUtils::addDynamicsJob;
	queue.m_jobPopFunc    = hkJobQueueUtils::popDynamicsJob;
	queue.m_finishJobFunc = hkJobQueueUtils::finishDynamicsJob;
	queue.m_userData      = HK_NULL;
	{
		hkJobQueue::JobStatus jobStatus = queue.getNextJob(hkJobQueue::THREAD_TYPE_SPU, job, waitStatus, spuId);
		if ( jobStatus != hkJobQueue::GOT_NEXT_JOB)
		{
			return jobStatus;
		}
	}

		// we can only fetch the info once we have our first job
	HK_ALIGN( hkWorldDynamicsStepInfo worldDynStepInfoBuf, 128 );
	hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &worldDynStepInfoBuf, worldDynamicsStepInfoAddress, sizeof(worldDynStepInfoBuf), hkSpuDmaManager::READ_COPY);
	hkStepInfo*   stepInfo   = &worldDynStepInfoBuf.m_stepInfo;
	hkSolverInfo* solverInfo = &worldDynStepInfoBuf.m_solverInfo;

	//HK_SPU_DEBUG_PRINTF(("**** NEW FRAME ****\n"));

	//
	// this loop exits once there are no jobs left on the local queue
	//
	hkJobQueue::JobStatus jobStatus;
	while ( 1 )
	{
		hkDynamicsJob& dynamicsJob = reinterpret_cast<hkDynamicsJob&>(job);

		switch ( dynamicsJob.m_jobType )
		{
			case DYNAMICS_JOB_BUILD_ACCUMULATORS:
				{
					hkSpuDmaManager::performFinalChecks( worldDynamicsStepInfoAddress, &worldDynStepInfoBuf, sizeof(worldDynStepInfoBuf));
					hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &worldDynStepInfoBuf, worldDynamicsStepInfoAddress, sizeof(worldDynStepInfoBuf), hkSpuDmaManager::READ_COPY);
					jobStatus = hkSpuBuildAccumulatorsJob( *stepInfo, queue, job, waitStatus );
					if ( jobStatus != hkJobQueue::GOT_NEXT_JOB )
					{
						goto END;
					}
					break;
				}

			case DYNAMICS_JOB_BUILD_JACOBIANS:
				{
					jobStatus = hkSpuBuildJacobiansJob( queue, job, waitStatus );
					if ( jobStatus != hkJobQueue::GOT_NEXT_JOB )
					{
						goto END;
					}
					break;
				}

			case DYNAMICS_JOB_SOLVE_CONSTRAINTS:
				{
					jobStatus = hkSpuSolveConstraintsJob( *solverInfo, *stepInfo, queue, job, waitStatus );
					if ( jobStatus != hkJobQueue::GOT_NEXT_JOB )
					{
						goto END;
					}
					break;
				}

			case DYNAMICS_JOB_INTEGRATE_MOTION:
				{
					jobStatus = hkSpuIntegrateMotionJob( *solverInfo, *stepInfo, queue, job, waitStatus );
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
	hkSpuDmaManager::performFinalChecks( worldDynamicsStepInfoAddress, &worldDynStepInfoBuf, sizeof(worldDynStepInfoBuf));
	return jobStatus;
}



#endif /*#if defined (HK_PLATFORM_PS3SPU) || defined (HK_SIMULATE_SPU_DMA_ON_CPU)*/

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20061017)
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
