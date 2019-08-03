/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkbase/memory/hkMemory.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/memory/hkLocalBuffer.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>
#include <hkbase/monitor/hkMonitorStream.h>

#include <hkbase/htl/hkAlgorithm.h>
#include <hkbase/debugutil/hkCheckDeterminismUtil.h>

#include <hkmath/basetypes/hkAabb.h>

#include <hkinternal/collide/broadphase/hkBroadPhase.h>
#include <hkinternal/collide/broadphase/hkBroadPhaseHandle.h>

#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseDispatcher.h>

#include <hkcollide/agent/hkContactMgr.h>
#include <hkcollide/agent/hkCollisionInput.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>
#include <hkcollide/agent/hkProcessCollisionOutput.h>

#include <hkinternal/collide/agent3/machine/nn/hkAgentNnMachine.h>

#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
	// needed for the hkConstraintAtomInput
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>

#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/action/hkAction.h>

#include <hkdynamics/collide/hkCollisionListener.h>
#include <hkdynamics/phantom/hkPhantom.h>
#include <hkdynamics/motion/util/hkRigidMotionUtil.h>

#include <hkdynamics/constraint/setup/hkConstraintSolverSetup.h>

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/util/hkWorldCallbackUtil.h>
#include <hkdynamics/world/util/hkWorldConstraintUtil.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>
#include <hkdynamics/world/util/hkWorldOperationQueue.h>
#include <hkdynamics/world/maintenance/hkWorldMaintenanceMgr.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkdynamics/world/commandqueue/hkPhysicsCommandQueue.h>


#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkinternal/dynamics/world/simulation/continuous/toiresourcemgr/hkToiResourceMgr.h>
#include <hkdynamics/world/simulation/multithreaded/hkMultithreadedSimulation.h>

#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobs.h>

#include <hkdynamics/world/simulation/multithreaded/cpu/hkCpuConstraint.h>



////////////////////////////////////////////////////////////////////////
//
// Collision Agent helper functions
//
////////////////////////////////////////////////////////////////////////


inline hkAgentNnEntry* addAgentHelperFunc( hkLinkedCollidable* collA, hkLinkedCollidable* collB, hkProcessCollisionInput* input )
{
	hkCollidableQualityType qt0 = collA->getQualityType();
	hkCollidableQualityType qt1 = collB->getQualityType();
	hkChar collisionQuality = input->m_dispatcher->getCollisionQualityIndex( qt0, qt1 );
	if ( collisionQuality == hkCollisionDispatcher::COLLISION_QUALITY_INVALID )
	{
		return HK_NULL;
	}
	hkCollisionQualityInfo* origInfo = input->m_dispatcher->getCollisionQualityInfo( collisionQuality );
	input->m_createPredictiveAgents = origInfo->m_useContinuousPhysics;

	return hkWorldAgentUtil::addAgent(collA, collB, *input);
}

inline void processAgentHelperFunc( hkAgentNnEntry* entry, 
									const hkProcessCollisionInput& input, 
									hkProcessCollisionOutput& processOutput, 
									hkMultiThreadedSimulation* simulation )
{
	hkCollidable* collA = entry->getCollidableA();
	hkCollidable* collB = entry->getCollidableB();

	{
		hkMath::prefetch128( entry->m_contactMgr );
		hkMath::prefetch128( hkAddByteOffset(entry, 128) );

		input.m_collisionQualityInfo = input.m_dispatcher->getCollisionQualityInfo( entry->m_collisionQualityIndex );
		input.m_createPredictiveAgents = input.m_collisionQualityInfo->m_useContinuousPhysics;

		processOutput.reset();
		hkAgentNnMachine_ProcessAgent( entry, input, processOutput );

		if (hkMemory::getInstance().getAvailableMemory() == 0)
		{
			hkMemory::getInstance().m_memoryState = hkMemory::MEMORY_STATE_OUT_OF_MEMORY;
		}

		if (hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OUT_OF_MEMORY )
		{
			return;
		}

		if ( !processOutput.isEmpty() )
		{
			entry->m_contactMgr->processContact( *collA, *collB, input, processOutput );
		}

		if ( processOutput.hasToi() )
		{
			HK_ASSERT( 0xf0324354, input.m_stepInfo.m_startTime <= processOutput.m_toi.m_time );
			HK_ASSERT2(0xad8765dd, processOutput.m_toi.m_time >= simulation->getCurrentTime(), "Generating a TOI event before hkWorld->m_currentTime.");

			hkCriticalSectionLock lock( &simulation->m_toiQueueCriticalSection );
			simulation->addToiEvent(processOutput, *entry );
		}
	}
}

////////////////////////////////////////////////////////////////////////
//
// Jobs
//
////////////////////////////////////////////////////////////////////////




namespace hkBuildConstraintBatches
{
	typedef hkUint8 BatchIndex;
#define INVALID_BODY       hkObjectIndex(-1)
#define FIXED_BODY_INDEX   hkObjectIndex(-1)
#define INVALID_BATCH      hkBuildConstraintBatches::BatchIndex(-1)


	struct Constraint
	{
		hkConstraintInternal* m_internal;
		hkObjectIndex	m_bodyA;
		hkObjectIndex	m_bodyB;
		BatchIndex		m_batchIndex;
	};

	struct Body
	{
		BatchIndex		m_batchIndex;
	};

	struct Header
	{
		Header(): m_numConstraints(0){}

		hkBool      m_startNewBatch;
		int         m_numConstraints;
		Constraint *m_firstConstraint;
		Constraint *m_currentConstraint;
	};
}





static /*HK_FORCE_INLINE*/ void HK_CALL createBuildJacobianTask( hkSimulationIsland* island, const hkBuildJacobianTaskHeader* taskHeader, hkBuildJacobianTaskCollection* out )
{
	hkEntity*const* bodies = island->getEntities().begin();
	int numBodies = island->getEntities().getSize();

	hkInplaceArray<const hkConstraintInternal*,1024> callbackConstraints;
#if defined(HK_PLATFORM_PS3) || defined(HK_SIMULATE_SPU_DMA_ON_CPU)
	hkInplaceArray<const hkConstraintInternal*,1024> ppuSetupOnlyConstraints;
#endif

	//
	//	Collect all constraints and count them by batch index
	//
	hkJacobianElement*	jacobians	= taskHeader->m_jacobiansBase;
	hkJacobianSchema*	schemas		= taskHeader->m_schemasBase;

	//
	// build first task and link it to header
	//
	hkBuildJacobianTask* task;
	{
		task = new hkBuildJacobianTask;
		{
			HK_ASSERT(0xaf214256, (hkUlong)schemas == HK_NEXT_MULTIPLE_OF(16, (hkUlong)schemas));
			task->m_taskHeader		= const_cast<hkBuildJacobianTaskHeader*>(taskHeader);
			task->m_accumulators	= taskHeader->m_accumulatorsBase;
			task->m_jacobians		= jacobians;
			task->m_schemas			= schemas;
			task->m_jacobiansBase	= taskHeader->m_jacobiansBase;
		}
		out->m_numBuildJacobianTasks = 1;
		out->m_buildJacobianTasks = task;
	}

	{
		hkEntity*const* e = bodies;
		hkEntity*const* eEnd = bodies + numBodies;
		hkInplaceArray<hkBuildJacobianTask::AtomInfo,256> highPriorityConstraint;

		// we need to initialize motions' solverData before it is used below
		for (int i = 0; i < numBodies; i++ )
		{
			hkUint32 newVal = (1+i)*sizeof(hkVelocityAccumulator);
				// try to avoid making the cacheline dirty
			if ( newVal != e[i]->m_solverData)
			{
				e[i]->m_solverData = newVal;
			}
		}

		for ( ; e < eEnd; e++ )
		{
			const hkConstraintInternal* c = (*e)->getConstraintMasters().begin();
			const hkConstraintInternal* cEnd = (*e)->getConstraintMasters().end();

			for ( ;c < cEnd; c++ )
			{
				// collect constraints that need to fire a callback
				if ( c->m_callbackRequest & hkConstraintAtom::CALLBACK_REQUEST_CONTACT_POINT )
				{
					callbackConstraints.pushBack(c);
				}

#if defined(HK_PLATFORM_HAS_SPU)
				// collect constraints that only can be setup on PPU in a separate list
				if ( c->m_callbackRequest & hkConstraintAtom::CALLBACK_REQUEST_SETUP_PPU_ONLY )
				{
					// we will deliberately ignore increasing the schemas and jacobians pointers;
					// this will be done in hkSetSchemasAndJacPtrsInTasks() once the ppu-only constraints have been processed
					ppuSetupOnlyConstraints.pushBack(c);
					continue;
				}
#endif

				hkBuildJacobianTask::AtomInfo* atomInfo;
				if (c->m_priority >= hkConstraintInstance::PRIORITY_TOI_HIGHER)
				{
					atomInfo = &highPriorityConstraint.expandOne();
				}
				else
				{
						// check whether we still have space in the current task
					if ( task->m_numAtomInfos >= hkBuildJacobianTask::MAX_NUM_ATOM_INFOS )
					{
						hkBuildJacobianTask* previousTask = task;
						task = new hkBuildJacobianTask;
						out->m_numBuildJacobianTasks++;

#if defined(HK_PLATFORM_HAS_SPU)
						schemas = reinterpret_cast<hkJacobianSchema*>(HK_NEXT_MULTIPLE_OF( 16, hkUlong(schemas)));
#endif
						previousTask->m_next	          = task;
						previousTask->m_schemasOfNextTask = schemas;
						{
							task->m_taskHeader		= const_cast<hkBuildJacobianTaskHeader*>(taskHeader);
							task->m_accumulators	= taskHeader->m_accumulatorsBase;
							task->m_jacobians		= jacobians;
							task->m_schemas			= schemas;
							task->m_jacobiansBase	= taskHeader->m_jacobiansBase;
						}
					}
					atomInfo = &task->m_atomInfos[task->m_numAtomInfos++];
					HK_ASSERT(0xaf7492ad, task->m_numAtomInfos <= hkBuildJacobianTask::MAX_NUM_ATOM_INFOS);

					//
					//	Increment pointers
					//
					{
						schemas   = hkAddByteOffset(schemas,   c->m_sizeOfSchemas   );
						jacobians = hkAddByteOffset(jacobians, c->m_sizeOfJacobians );
					}
				}

				//
				//	Fill atomInfo
				//
				{
					atomInfo->m_atoms				= c->getAtoms();
					atomInfo->m_atomsSize			= c->getAtomsSize();
					atomInfo->m_instance			= c->m_constraint;
					atomInfo->m_runtime				= c->m_runtime;
					atomInfo->m_runtimeSize			= c->m_runtimeSize;
					atomInfo->m_accumulatorOffsetA	= c->m_entities[0]->m_solverData;
					atomInfo->m_accumulatorOffsetB	= c->m_entities[1]->m_solverData;
					atomInfo->m_transformA			= &c->m_entities[0]->getMotion()->getTransform();
					atomInfo->m_transformB			= &c->m_entities[1]->getMotion()->getTransform();
				}
			}
		}

		//
		// append high priority constraints to end of list
		//
		{
			for (int c = 0; c < highPriorityConstraint.getSize(); c++)
			{
				hkBuildJacobianTask::AtomInfo& source = highPriorityConstraint[c];
				hkBuildJacobianTask::AtomInfo* dest;
				{
					if ( task->m_numAtomInfos >= hkBuildJacobianTask::MAX_NUM_ATOM_INFOS )
					{
						hkBuildJacobianTask* previousTask = task;
						task = new hkBuildJacobianTask;
						out->m_numBuildJacobianTasks++;

#if defined(HK_PLATFORM_HAS_SPU)
						schemas = reinterpret_cast<hkJacobianSchema*>(HK_NEXT_MULTIPLE_OF( 16, hkUlong(schemas)));
#endif
						previousTask->m_next	          = task;
						previousTask->m_schemasOfNextTask = schemas;
						{
							task->m_taskHeader		= const_cast<hkBuildJacobianTaskHeader*>(taskHeader);
							task->m_accumulators	= taskHeader->m_accumulatorsBase;
							task->m_jacobians		= jacobians;
							task->m_schemas			= schemas;
							task->m_jacobiansBase	= taskHeader->m_jacobiansBase;
						}
					}
					dest = &task->m_atomInfos[task->m_numAtomInfos++];
					HK_ASSERT(0xaf7494ad, task->m_numAtomInfos <= hkBuildJacobianTask::MAX_NUM_ATOM_INFOS);
				}

				//
				//	Increment pointers
				//
				{
					hkConstraintInternal* internal = source.m_instance->m_internal;
					schemas   = hkAddByteOffset(schemas,   internal->m_sizeOfSchemas   );
					jacobians = hkAddByteOffset(jacobians, internal->m_sizeOfJacobians );
				}

#if defined (HK_PLATFORM_PS3)
				hkString::memCpy4(dest, &source, sizeof(hkBuildJacobianTask::AtomInfo)/4);
#else
				*dest = source;
#endif
			}
		}
	}

	task->m_schemasOfNextTask = hkAddByteOffset( schemas, HK_SIZE_OF_JACOBIAN_END_SCHEMA);

	{
		HK_ON_DEBUG( void* bufferEnd = hkAddByteOffset(taskHeader->m_buffer, taskHeader->m_bufferSize ) );
		HK_ASSERT( 0xf032eddf, (void*)schemas <= bufferEnd );

		HK_ON_DEBUG( int sizeOfAllSchemas = hkGetByteOffsetInt( taskHeader->m_schemasBase, schemas));
		HK_ON_DEBUG( int allocatedSchemaBufferSize = island->m_constraintInfo.m_sizeOfSchemas + 12 * ( 2+ (island->m_numConstraints/hkBuildJacobianTask::MAX_NUM_ATOM_INFOS)) );
		HK_ASSERT( 0xf032edff, sizeOfAllSchemas <= allocatedSchemaBufferSize );
	}

	//
	// collect constraints that need to fire callbacks
	//
	{
		// default values in case no callbacks are fired
		out->m_numCallbackConstraints = 0;
		out->m_callbackConstraints = HK_NULL;

		int numCC = callbackConstraints.getSize();
		if ( numCC )
		{
			const hkConstraintInternal** cc = hkAllocateChunk<const hkConstraintInternal*>( numCC, HK_MEMORY_CLASS_CONSTRAINT_SOLVER );
			// if we keep a pointer to the callback constraints, we need to mark the island read only, so that nobody removes are adds constraints
			callbackConstraints[0]->getMasterEntity()->getSimulationIsland()->getMultiThreadLock().markForRead();
			out->m_numCallbackConstraints = numCC;
			out->m_callbackConstraints = cc;
			for (int i = 0; i < numCC; i++)
			{
				cc[i] = callbackConstraints[i];
			}
		}
	}

#if defined(HK_PLATFORM_PS3) || defined(HK_SIMULATE_SPU_DMA_ON_CPU)
	//
	// collect constraints that need to be setup on ppu
	//
	{
		out->m_numPpuSetupOnlyConstraints = 0;
		out->m_ppuSetupOnlyConstraints = HK_NULL;

		int numCC = ppuSetupOnlyConstraints.getSize();
		if ( numCC )
		{
			const hkConstraintInternal** cc = hkAllocateChunk<const hkConstraintInternal*>( numCC, HK_MEMORY_CLASS_CONSTRAINT_SOLVER );
			out->m_numPpuSetupOnlyConstraints = numCC;
			out->m_ppuSetupOnlyConstraints = cc;
			for (int i = 0; i < numCC; i++)
			{
				cc[i] = ppuSetupOnlyConstraints[i];
			}
		}
	}
#endif
}

static HK_LOCAL_INLINE void splitSimulationIslandJobImpl( hkSimulationIsland* island, hkArray<hkSimulationIsland*>& newSplitIslands )
{
	{
		island->m_splitCheckRequested = false;

		 // we create a new array here, as we want to bypass the destructor,
		// basically we want to keep the original array,
		// the old entities array will be freed in the hkBroadphaseJob::popJobTask
		int buffer[ sizeof(hkArray<hkEntity*>)/4];
		hkArray<hkEntity*>& oldEntities = *new(buffer) hkArray<hkEntity*>;

		island->markForWrite();
		hkWorldOperationUtil::splitSimulationIsland( island, island->getWorld(), newSplitIslands, &oldEntities );
#ifdef HK_DEBUG_MULTI_THREADING
		{ for (int i =0; i < newSplitIslands.getSize(); i++){ newSplitIslands[i]->unmarkForWrite(); } }
#endif
		island->unmarkForWrite();
	}
}

static HK_FORCE_INLINE hkJobQueue::JobStatus splitSimulationIslandJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& jobInOut )
{
	HK_TIMER_BEGIN("SplitIsle", HK_NULL)

	const hkSplitSimulationIslandJob& job = reinterpret_cast<hkSplitSimulationIslandJob&>(jobInOut);

	splitSimulationIslandJobImpl( job.m_island, job.m_taskHeader->m_newSplitIslands );

	HK_TIMER_END();
	return jobQueue.finishJobAndGetNextJob( tl.m_threadType, (const hkJobQueue::JobQueueEntry*)&job, jobInOut, &tl );
}



// For now, simply do all integration here.
HK_FORCE_INLINE static hkJobQueue::JobStatus HK_CALL integrateJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& nextJobOut )
{
	const hkIntegrateJob& job = reinterpret_cast<hkIntegrateJob&>(nextJobOut);
	HK_TIMER_BEGIN_LIST("Integrate", "Actions");

	hkSimulationIsland* island = job.m_island;
#ifdef HK_DEBUG_MULTI_THREADING
	island->m_inIntegrateJob = true;
#endif

	const hkWorldDynamicsStepInfo& stepInfo = tl.m_world->m_dynamicsStepInfo;

	// actions
	if (!tl.m_world->m_processActionsInSingleThread)
	{
		hkArray<hkAction*>& actions = island->m_actions;
		for (int j = 0; j < actions.getSize(); j++)
		{
			hkAction* action = actions[j];
			if (action)
			{
#				ifdef HK_DEBUG_MULTI_THREADING
					hkInplaceArray<hkEntity*, 16> entities;
					action->getEntities(entities);
					{ for (int e = 0; e < entities.getSize(); e++) { entities[e]->markForWrite();   } }
					action->applyAction( stepInfo.m_stepInfo );
					{ for (int e = 0; e < entities.getSize(); e++) { entities[e]->unmarkForWrite(); } }
#				else // not HK_DEBUG_MULTI_THREADING
					action->applyAction( stepInfo.m_stepInfo );
#				endif // HK_DEBUG_MULTI_THREADING
			}
		}
	}

	hkBuildJacobianTaskHeader* taskHeader = new hkBuildJacobianTaskHeader;

	hkEntity*const* entities = island->getEntities().begin();

#if defined( HK_PLATFORM_HAS_SPU)
	{
		// make sure entities are aligned
		if ( hkUlong(entities) & 0x0f )
		{
			island->m_entities.reserve(4);
			entities = island->getEntities().begin();
		}
		HK_ASSERT( 0xf053fd43, (hkUlong(entities)&0x0f) == 0 );
	}
#endif

	unsigned int numEntities = island->getEntities().getSize();

	taskHeader->m_numAllEntities      = hkObjectIndex(numEntities);
	taskHeader->m_allEntities         = entities;
	taskHeader->m_entitiesCapacity    = hkObjectIndex(island->getEntities().getCapacity());
	taskHeader->m_numUnfinishedJobsForBroadphase  = 0;
	taskHeader->m_islandShouldBeDeactivated 	  = 1;		// will be set by the integrateMotion jobs to 0
	taskHeader->m_buffer              = HK_NULL;
	taskHeader->m_bufferSize          = 0;
	taskHeader->m_tasks.m_buildJacobianTasks = 0;

	//
	// try to decide whether we want a split job
	//
	int splitRequested = 0;
	{
		island->m_splitCheckFrameCounter++;
		if ( !island->m_sparseEnabled )
		{
			// check for normal island splitting every 8th frame
			if ( island->m_splitCheckRequested && ((island->m_splitCheckFrameCounter & 0x7) == 0) && tl.m_world->m_wantSimulationIslands ) 
			{
				splitRequested = 1;
			}
		}
		else
		{
			// now we have a not fully connected island. Check whether it grew over the limit size. Do this check every frame
			int islandSize = hkWorldOperationUtil::estimateIslandSize( island->m_entities.getSize(), island->m_numConstraints );
			if ( !hkWorldOperationUtil::canIslandBeSparse( tl.m_world, islandSize ) )
			{
					// our island got too big, break it up with only 3 frames delay
				island->m_sparseEnabled = false;

				if ( (island->m_splitCheckFrameCounter & 0x3) == 0 )
				{
					splitRequested = 1;
				}
			}
			else
			{
				// try to split off deactivated sub islands
				if ( (island->m_splitCheckFrameCounter & 0x7) == 0 )
				{
					splitRequested = 1;
				}
			}
		}
	}

	if ( island->m_constraintInfo.m_sizeOfJacobians == 0  )
	{
		//
		//	Single object
		//
		HK_TIMER_SPLIT_LIST("SingleObj");
		{	 for ( unsigned int i=0; i < numEntities; i++ ){ entities[i]->markForWrite(); }	}
		int numInactiveFrames = hkRigidMotionUtilApplyForcesAndStep( stepInfo.m_solverInfo, stepInfo.m_stepInfo, stepInfo.m_solverInfo.m_globalAccelerationPerStep, (hkMotion*const*)island->m_entities.begin(), island->m_entities.getSize(), HK_OFFSET_OF(hkEntity,m_motion) );
		{	 for ( unsigned int i=0; i < numEntities; i++ ){ entities[i]->unmarkForWrite(); }	}
		if ( numInactiveFrames <= hkMotion::NUM_INACTIVE_FRAMES_TO_DEACTIVATE)
		{
			taskHeader->m_islandShouldBeDeactivated = 0;
		}
		goto BUILD_BROAD_PHASE_JOB_AND_RETURN;
	}



		// if we have chain constraints, the constraint number is just plain wrong, so use the number of entities instead
	hkUint32 estimatedNumConstraints; estimatedNumConstraints = hkMath::max2( island->m_numConstraints, island->getEntities().getSize() );
	if ( estimatedNumConstraints < tl.m_simulation->m_multithreadConfig.m_maxNumConstraintsSolvedSingleThreaded ) 
	{

		//
		// single threaded execution
		//
		HK_TIMER_SPLIT_LIST("Solver 1Cpu");
		{
			{	 for ( unsigned int i=0; i < numEntities; i++ ){ entities[i]->markForWrite(); }	}
			island->getMultiThreadLock().markForRead();

			int numInactiveFrames = hkConstraintSolverSetup::solve( stepInfo.m_stepInfo, stepInfo.m_solverInfo,	tl.m_constraintQueryIn, *island,	entities,    numEntities );

			island->getMultiThreadLock().unmarkForRead();
			{	 for ( unsigned int i=0; i < numEntities; i++ ){ entities[i]->unmarkForWrite(); }	}

			if ( numInactiveFrames <= hkMotion::NUM_INACTIVE_FRAMES_TO_DEACTIVATE)
			{
				taskHeader->m_islandShouldBeDeactivated = 0;
			}
		}
BUILD_BROAD_PHASE_JOB_AND_RETURN:
		if ( splitRequested )
		{
			splitSimulationIslandJobImpl( island, taskHeader->m_newSplitIslands );
		}

		//
		// XXX Island and world (if this is the last island) post integrate callbacks should be called here, but whats the point?
		//
		HK_TIMER_END_LIST();
		{
			taskHeader->m_exportFinished = 1;
			new (&nextJobOut) hkBroadPhaseJob( job, taskHeader );
			HK_ON_DEBUG(hkJobQueue::JobStatus status = )
				jobQueue.addJobAndGetNextJob( hkJobQueue::JOB_TYPE_CPU_PRIMARY, hkJobQueue::JOB_HIGH_PRIORITY, tl.m_threadType, nextJobOut );
			HK_ASSERT( 0xf0213445, status == hkJobQueue::GOT_NEXT_JOB);
		}
		return hkJobQueue::GOT_NEXT_JOB;
	}


	{
		HK_TIMER_SPLIT_LIST("BuildJobs");
		//
		// multi threaded solution
		//

		int bufferSize = hkConstraintSolverSetup::calcBufferSize( *island, JOB_MIN_JACOBIANS_PER_JOB );

		char* buffer = (char*)hkMemory::getInstance().allocateRuntimeBlock( bufferSize, HK_MEMORY_CLASS_CONSTRAINT );

		const hkBuildJacobianTaskHeader* header = taskHeader;
		{
			hkConstraintSolverSetup::calcBufferOffsetsForSolve( *island, buffer, bufferSize, *taskHeader );
			taskHeader->m_numSolverResults		= island->m_constraintInfo.m_numSolverResults;
			taskHeader->m_constraintQueryIn		= &tl.m_constraintQueryIn;
			taskHeader->m_exportFinished		= hkChar(0);

			int numIntegrateMotionJobs = 1 + ((numEntities-1)/ hkIntegrateMotionJob::ACCUMULATORS_PER_JOB);
			taskHeader->m_numUnfinishedJobsForBroadphase	= splitRequested + numIntegrateMotionJobs;	// split job and integrate motion job
			taskHeader->m_splitCheckRequested               = hkChar(splitRequested);

			//
			// create a new high-priority job for building the accumulators and add it to the queue
			//
			{
				hkBuildAccumulatorsJob& j = reinterpret_cast<hkBuildAccumulatorsJob&>(nextJobOut);
				{
					j.m_jobType				= DYNAMICS_JOB_BUILD_ACCUMULATORS;
					j.m_taskHeader			= taskHeader;
					j.m_islandEntitiesArray	= entities;
					j.m_firstEntityIdx		= 0;
					j.m_numEntities			= hkObjectIndex(numEntities);
				}

				taskHeader->m_openJobs = 1 + 1 + ((numEntities-1)/hkBuildAccumulatorsJob::ACCUMULATORS_PER_JOB); // 1 hkBuildAccumulatorsJob + 1 hkBuildJacobiansTaskJob (see below)
			}
			jobQueue.addJob( nextJobOut, hkJobQueue::JOB_HIGH_PRIORITY, hkJobQueue::JOB_TYPE_HINT_SPU );
		}

		//
		// immediately execute 'buildJacobianTask' job (without actually adding it to the queue).
		//
		hkBuildJacobianTaskCollection out;
		{
			island->markAllEntitiesReadOnly();
			island->getMultiThreadLock().markForRead();
			createBuildJacobianTask( island, header, &out );
			island->getMultiThreadLock().unmarkForRead();
			island->unmarkAllEntitiesReadOnly();
		}

		HK_TIMER_END_LIST();
		taskHeader->m_tasks = out;

		hkBuildJacobiansTaskJob fjob( job, header );

		// we have to tell the job queue that hkBuildJacobiansTaskJob has finished,
		// so that the next dependent job can be activated
		hkJobQueue::JobStatus status = jobQueue.finishJobAndGetNextJob( tl.m_threadType, (hkJobQueue::JobQueueEntry*)&fjob, nextJobOut, &tl );
		return status;
	}
}

HK_FORCE_INLINE hkJobQueue::JobStatus HK_CALL hkSingleThreadedJobsOnIsland::cpuBroadPhaseJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& nextJobOut )
{
	const hkBroadPhaseJob& job = reinterpret_cast<hkBroadPhaseJob&>(nextJobOut);

	// Just do the broadphase for one island here, for now
	hkSimulationIsland* island = job.m_island;

#ifdef HK_DEBUG_MULTI_THREADING
	HK_ASSERT( 0xf0323454, island->m_inIntegrateJob );
	island->m_inIntegrateJob = false;
	island->m_allowIslandLocking = true;
#endif
	HK_ASSERT(0xf04321e3, job.m_taskHeader->m_numUnfinishedJobsForBroadphase == 0);

	HK_TIMER_BEGIN("Broadphase", "Collide");

	// 
	//	Deactivation check
	//
	if ( job.m_taskHeader->m_islandShouldBeDeactivated )
	{
		if ( tl.m_world->m_wantDeactivation && island->m_active && !island->m_sparseEnabled)
		{
			hkWorldOperationUtil::markIslandInactiveMt( tl.m_world, island );
		}
	}

	hkChar* exportFinished = ( job.m_taskHeader ) ? &job.m_taskHeader->m_exportFinished : HK_NULL;
	tl.m_simulation->collideEntitiesBroadPhaseContinuous( island->m_entities.begin(), island->m_entities.getSize(), tl.m_world, exportFinished );

	HK_ON_DEBUG_MULTI_THREADING( island->m_allowIslandLocking = false );

	//
	//	free resources
	//
	{
		hkBuildJacobianTaskHeader* taskHeader = job.m_taskHeader;
		hkInt32 value = hkCriticalSection::atomicExchangeAdd( (hkUint32*)&taskHeader->m_referenceCount, -1);
		if ( value == 1)
		{
			
			while ( taskHeader->m_tasks.m_buildJacobianTasks )
			{
				hkBuildJacobianTask* task = taskHeader->m_tasks.m_buildJacobianTasks;
				taskHeader->m_tasks.m_buildJacobianTasks = task->m_next;
				delete task;
			}
			if ( taskHeader->m_buffer)
			{
				hkMemory::getInstance().deallocateRuntimeBlock(taskHeader->m_buffer, taskHeader->m_bufferSize, HK_MEMORY_CLASS_CONSTRAINT);
				taskHeader->m_buffer = HK_NULL;
			}
			delete taskHeader;
		}
	}

	int numSectors = island->m_agentTrack.m_sectors.getSize();
	if (numSectors > 0)
	{
		hkAgentSectorJob& asJob = *new(&nextJobOut) hkAgentSectorJob(job, tl.m_world->m_dynamicsStepInfo.m_stepInfo, numSectors);
		{
				// if we have several agentSectorTasks, we need to create an organizing header
			if ( numSectors > hkAgentSectorJob::SECTORS_PER_JOB)
			{
				int numTasks = ((numSectors-1)/hkAgentSectorJob::SECTORS_PER_JOB) + 1;
				asJob.m_header = hkAgentSectorHeader::allocate(numTasks);
				asJob.m_numTotalTasks = numTasks;
			}
		}
		HK_ON_DEBUG(hkJobQueue::JobStatus status = )
		jobQueue.addJobAndGetNextJob( hkJobQueue::JOB_TYPE_CPU, hkJobQueue::JOB_LOW_PRIORITY, tl.m_threadType, nextJobOut );
		HK_ASSERT( 0xf0213445, status == hkJobQueue::GOT_NEXT_JOB);

		HK_TIMER_END();
		return hkJobQueue::GOT_NEXT_JOB;
	}
	else
	{
		HK_TIMER_END();
		return jobQueue.finishJobAndGetNextJob( tl.m_threadType, (const hkJobQueue::JobQueueEntry*)&job, nextJobOut, &tl );
	}
}

class hkDeferredConstraintOwner: public hkConstraintOwner
{
public:
	hkDeferredConstraintOwner( )
	{
		m_constraintInfo.clear();
		m_constraintAddRemoveCounter = 0;
		callbackRequestForAddConstraint = 0;
	}

	virtual void addConstraintToCriticalLockedIsland( hkConstraintInstance* constraint )
	{
		m_constraintForCommand = constraint;
		m_constraintAddRemoveCounter++;
	}

	virtual void removeConstraintFromCriticalLockedIsland( hkConstraintInstance* constraint )
	{
		m_constraintForCommand = constraint;
		m_constraintAddRemoveCounter--;
	}

	void addCallbackRequest( hkConstraintInstance* constraint, int request )
	{
		if ( constraint->m_internal)
		{
			constraint->m_internal->m_callbackRequest |= request;
		}
		else
		{
			callbackRequestForAddConstraint |= request;
		}
	}


	int	m_constraintAddRemoveCounter;

	hkConstraintInstance* m_constraintForCommand;
	int callbackRequestForAddConstraint;

	hkFixedSizePhysicsCommandQueue<8000> m_commandQueue;
};


static HK_FORCE_INLINE hkJobQueue::JobStatus HK_CALL agentSectorJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& nextJobOut )
{
	hkAgentSectorJob& job = reinterpret_cast<hkAgentSectorJob&>(nextJobOut);

	tl.m_collisionInput.m_stepInfo = job.m_stepInfo;

	// Just do the narrow phase for one island here, for now
	hkSimulationIsland* island = job.m_island;

	HK_TIMER_BEGIN("NarrowPhase", HK_NULL);

	island->markAllEntitiesReadOnly();
	island->getMultiThreadLock().markForRead();
	{
		hkDeferredConstraintOwner constraintOwner;
		HK_ALIGN16( hkProcessCollisionOutput processOutput( &constraintOwner ) );

		hkAgentNnTrack& agentTrack = island->m_agentTrack;
		int sectorIndex = job.m_taskIndex * job.SECTORS_PER_JOB;
		

		for ( int numSectors = job.m_numSectors; --numSectors >= 0; sectorIndex++ )
		{
			hkAgentNnSector* sector = agentTrack.m_sectors[sectorIndex];
			hkAgentNnEntry* entry = sector->getBegin();
			hkAgentNnEntry* endEntry = (sectorIndex < agentTrack.m_sectors.getSize() - 1) ?
											sector->getEnd(agentTrack) :
											hkAddByteOffset(entry, agentTrack.m_bytesUsedInLastSector );
			for ( ; entry < endEntry; entry = hkAddByteOffset( entry, entry->m_size ) )
			{
				processAgentHelperFunc(entry, tl.m_collisionInput, processOutput, tl.m_simulation );

				if ( constraintOwner.m_constraintAddRemoveCounter == 0 )
				{
					constraintOwner.callbackRequestForAddConstraint = 0;
					continue;
				}
				if ( constraintOwner.m_constraintAddRemoveCounter > 0 )
				{
					constraintOwner.m_commandQueue.addCommand( hkAddConstraintToCriticalLockedIslandPhysicsCommand( constraintOwner.m_constraintForCommand, constraintOwner.callbackRequestForAddConstraint ) );
					constraintOwner.callbackRequestForAddConstraint = 0;
				}
				else
				{
					HK_ASSERT( 0xf0235456, constraintOwner.callbackRequestForAddConstraint == 0);
					constraintOwner.m_commandQueue.addCommand( hkRemoveConstraintFromCriticalLockedIslandPhysicsCommand( constraintOwner.m_constraintForCommand ) );
				}
				constraintOwner.m_constraintAddRemoveCounter = 0;
			}
		}

	
		if ( !job.m_header)
		{	// now we are the only job for this island, apply the changes immediately
			HK_ON_DEBUG_MULTI_THREADING( island->m_allowIslandLocking = true; );
			island->getMultiThreadLock().unmarkForRead();
			tl.m_world->lockIslandForConstraintUpdate( island );
			island->m_constraintInfo.add( constraintOwner.m_constraintInfo );
			if ( constraintOwner.m_commandQueue.m_start !=  constraintOwner.m_commandQueue.m_current )
			{
				hkPhysicsCommandMachineProcess( tl.m_world, constraintOwner.m_commandQueue.m_start, constraintOwner.m_commandQueue.m_current );
			}
			tl.m_world->unlockIslandForConstraintUpdate( island );
			island->getMultiThreadLock().markForRead();
			HK_ON_DEBUG_MULTI_THREADING( island->m_allowIslandLocking = false; );
		}
		else
		{
			// copy the command queue to the JobInfo, so it can be processed later
			hkAgentSectorHeader::JobInfo* sh = job.m_header->getJobInfo(job.m_taskIndex);
			sh->m_constraintInfo = constraintOwner.m_constraintInfo;
			int size = hkGetByteOffsetInt( constraintOwner.m_commandQueue.m_start, constraintOwner.m_commandQueue.m_current );
			if ( size )
			{
				hkString::memCpy16( sh->m_commandQueue.m_start, constraintOwner.m_commandQueue.m_start, size>>4);
				sh->m_commandQueue.m_current = hkAddByteOffset( sh->m_commandQueue.m_start, size );
			}
		}
	}
	island->getMultiThreadLock().unmarkForRead();
	island->unmarkAllEntitiesReadOnly();

	HK_TIMER_NAMED_END("NarrowPhase");

	return jobQueue.finishJobAndGetNextJob( tl.m_threadType, (const hkJobQueue::JobQueueEntry*)&job, nextJobOut, &tl );
}

static HK_FORCE_INLINE hkJobQueue::JobStatus HK_CALL postCollideJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& nextJobOut )
{
	hkPostCollideJob& job = reinterpret_cast<hkPostCollideJob&>(nextJobOut);

	HK_TIMER_BEGIN_LIST("NarrowPhase", "PostCollide");

	hkSimulationIsland* island = job.m_island;

	HK_ON_DEBUG_MULTI_THREADING( island->m_allowIslandLocking = true );
	tl.m_world->lockIslandForConstraintUpdate( island );
	{
		for (int task = 0 ; task < job.m_numTotalTasks; task++ )
		{
			hkAgentSectorHeader::JobInfo* jobInfo = job.m_header->getJobInfo(task);
			island->m_constraintInfo.add( jobInfo->m_constraintInfo );

			if ( jobInfo->m_commandQueue.m_start != jobInfo->m_commandQueue.m_current )
			{
				int numCommands = hkGetByteOffsetInt( jobInfo->m_commandQueue.m_start,jobInfo->m_commandQueue.m_current ) >> 4;
				HK_MONITOR_ADD_VALUE( "numCmds", float(numCommands), HK_MONITOR_TYPE_INT);
				hkPhysicsCommandMachineProcess( tl.m_world, jobInfo->m_commandQueue.m_start, jobInfo->m_commandQueue.m_current );
			}
		}
		job.m_header->deallocate(job.m_numTotalTasks);
	}
	tl.m_world->unlockIslandForConstraintUpdate( island );
	HK_ON_DEBUG_MULTI_THREADING( island->m_allowIslandLocking = false );
	HK_TIMER_END_LIST();
	return jobQueue.finishJobAndGetNextJob( tl.m_threadType, (const hkJobQueue::JobQueueEntry*)&job, nextJobOut, &tl );
}

////////////////////////////////////////////////////////////////////////
//
// Job Dispatch loop
//
////////////////////////////////////////////////////////////////////////

void hkMultiThreadedSimulation::processNextJob( hkJobQueue::ThreadType threadType )
{
	hkJobQueue::JobQueueEntry job;

	if (m_jobQueue.getNextJob(threadType, job) == hkJobQueue::GOT_NEXT_JOB)
	{
		hkJobQueue::JobStatus jobStatus = hkJobQueue::GOT_NEXT_JOB;

		// This needs to be allocated on the heap for spu DMA
		hkMtThreadStructure* tl = new hkMtThreadStructure( m_world, this, threadType );

		while ( jobStatus == hkJobQueue::GOT_NEXT_JOB )
		{
			hkDynamicsJob& dynamicsJob = reinterpret_cast<hkDynamicsJob&>(job);

			switch ( dynamicsJob.m_jobType )
			{
				// Note: Each job is responsible for getting the next job from the job queue.
				// The 'job' parameter passed into each job function gets overwritten inside
				// the function with the 'next job to be processed'.
				case DYNAMICS_JOB_INTEGRATE:
				{
					jobStatus = integrateJob( *tl, m_jobQueue, job );
					break;
				}

				case DYNAMICS_JOB_BUILD_ACCUMULATORS:
				{
					jobStatus = hkCpuBuildAccumulatorsJob( *tl, m_jobQueue, job );
					break;
				}

				case DYNAMICS_JOB_FIRE_JACOBIAN_SETUP_CALLBACK_AND_BUILD_PPU_JACOBIANS:
				{
					jobStatus = hkSingleThreadedJobsOnIsland::cpuFireJacobianSetupCallbackAndBuildPpuOnlyJacobiansJob( m_jobQueue, job, hkJobQueue::WAIT_FOR_NEXT_JOB );
					break;
				}

				case DYNAMICS_JOB_BUILD_JACOBIANS:
				{
					jobStatus =  hkCpuBuildJacobiansJob( *tl, m_jobQueue, job, hkJobQueue::WAIT_FOR_NEXT_JOB );
					break;
				}

				case DYNAMICS_JOB_SOLVE_CONSTRAINTS:
				{
					jobStatus = hkCpuSolveConstraintsJob( *tl, m_jobQueue, job, hkJobQueue::WAIT_FOR_NEXT_JOB );
					break;
				}

				case DYNAMICS_JOB_INTEGRATE_MOTION:
				{
					if ( hkJobQueue::GOT_NEXT_JOB == hkCpuIntegrateMotionJob( *tl, m_jobQueue, job, hkJobQueue::WAIT_FOR_NEXT_JOB ))
					{
						continue;
					}
					goto end;
				}

				case DYNAMICS_JOB_SPLIT_ISLAND:
					{
						if ( hkJobQueue::GOT_NEXT_JOB == splitSimulationIslandJob( *tl, m_jobQueue, job ))
						{
							continue;
						}
						goto end;
					}

				case DYNAMICS_JOB_BROADPHASE:
				{
					// Broad phase can add an agent sector OR a NOP job (if there's no narrow phase resulting
					// from the broad phase).
					jobStatus = hkSingleThreadedJobsOnIsland::cpuBroadPhaseJob( *tl, m_jobQueue, job );
					break;
				}

				case DYNAMICS_JOB_AGENT_SECTOR:
				{
					// Agent sector always adds a NOP job, however we don't actually add one;
					// we just fall through to the NOP case below.
					jobStatus = agentSectorJob( *tl, m_jobQueue, job );
					break;
				}

				case DYNAMICS_JOB_POST_COLLIDE:
					{
						// Agent sector always adds a NOP job, however we dont actually add one;
						// we just fall through to the NOP case below.
						if ( hkJobQueue::GOT_NEXT_JOB == postCollideJob( *tl, m_jobQueue, job ))
						{
							continue;
						}
						goto end;
					}
				default:
				{
					HK_ASSERT2(0xafe1a256, 0, "Internal error - unknown job type");
				}
			}
		}
end:
		delete tl;
	}
}

////////////////////////////////////////////////////////////////////////
//
// Special broad phase listener to lock phantoms
//
////////////////////////////////////////////////////////////////////////





void hkMultiThreadedSimulation::MtPhantomBroadPhaseListener::addCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	//hkCriticalSectionLock lock( m_criticalSection ); updateAabb+dispatching now is atomic
	if ( pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_PHANTOM )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collA->getOwner() );
		p->markForWrite();
		p->addOverlappingCollidable( collB );
		p->unmarkForWrite();
	}

	if ( pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_PHANTOM )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collB->getOwner() );
		p->markForWrite();
		p->addOverlappingCollidable( collA );
		p->unmarkForWrite();
	}
}

void hkMultiThreadedSimulation::MtPhantomBroadPhaseListener::removeCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	//hkCriticalSectionLock lock( m_criticalSection ); updateAabb+dispatching now is atomic
	if ( pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_PHANTOM )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collA->getOwner() );
		p->markForWrite();
		p->removeOverlappingCollidable( collB );
		p->unmarkForWrite();
	}

	if ( pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_PHANTOM )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collB->getOwner() );
		p->markForWrite();
		p->removeOverlappingCollidable( collA );
		p->unmarkForWrite();
	}
}




////////////////////////////////////////////////////////////////////////
//
// Special broad phase listener to delay adding of new pairs
//
////////////////////////////////////////////////////////////////////////




void hkMultiThreadedSimulation::MtEntityEntityBroadPhaseListener::addCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	hkLinkedCollidable* collA = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
	hkLinkedCollidable* collB = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );

	hkEntity* entityA = static_cast<hkEntity*>( collA->getOwner() );
	hkEntity* entityB = static_cast<hkEntity*>( collB->getOwner() );

	if ( m_simulation->m_crossIslandPairsCollectingActive && 
		!entityA->isFixed() && !entityB->isFixed() &&
		entityA->getSimulationIsland() != entityB->getSimulationIsland() )
	{
		//m_simulation->m_addCrossIslandPairCriticalSection.enter(); (whole broadphase+dispatch is locked)
		m_simulation->m_addedCrossIslandPairs.pushBack(pair);
		//m_simulation->m_addCrossIslandPairCriticalSection.leave();
	}
	else
	{
		addAgentHelperFunc(collA, collB, m_simulation->getWorld()->getCollisionInput() );
	}
}


void hkMultiThreadedSimulation::MtEntityEntityBroadPhaseListener::removeCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{	
	hkLinkedCollidable* collA = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
	hkLinkedCollidable* collB = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );

	hkEntity* entityA = static_cast<hkEntity*>( collA->getOwner() );
	hkEntity* entityB = static_cast<hkEntity*>( collB->getOwner() );

	if ( m_simulation->m_crossIslandPairsCollectingActive && 
		!entityA->isFixed() && !entityB->isFixed() &&
		entityA->getSimulationIsland() != entityB->getSimulationIsland() )
	{
		//m_simulation->m_removeCrossIslandPairCriticalSection.enter();
		m_simulation->m_removedCrossIslandPairs.pushBack(pair);
		//m_simulation->m_removeCrossIslandPairCriticalSection.leave();
	}
	else
	{
		hkAgentNnEntry* entry = hkAgentNnMachine_FindAgent(collA, collB);

		if (entry)
		{
			hkWorldAgentUtil::removeAgent(entry);
		}
	}
}

////////////////////////////////////////////////////////////////////////
//
// Special broad phase listener to lock broad phase border
//
////////////////////////////////////////////////////////////////////////



void hkMultiThreadedSimulation::MtBroadPhaseBorderListener::addCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	//hkCriticalSectionLock lock( m_criticalSection ); updateAabb+dispatching now is atomic
	if (   pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_BORDER
		&& pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		return;
	}
	if ( pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collA->getOwner() );
		p->markForWrite();
		p->addOverlappingCollidable( collB );
		p->unmarkForWrite();
	}

	if ( pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collB->getOwner() );
		p->markForWrite();
		p->addOverlappingCollidable( collA );
		p->unmarkForWrite();
	}
}


void hkMultiThreadedSimulation::MtBroadPhaseBorderListener::removeCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	//hkCriticalSectionLock lock( m_criticalSection ); updateAabb+dispatching now is atomic
	if (   pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_BORDER
		&& pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		return;
	}

	if ( pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collA->getOwner() );
		p->markForWrite();
		p->removeOverlappingCollidable( collB );
		p->unmarkForWrite();
	}

	if ( pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collB->getOwner() );
		p->markForWrite();
		p->removeOverlappingCollidable( collA );
		p->unmarkForWrite();
	}
}


////////////////////////////////////////////////////////////////////////
//
// Multi Threaded Simulation Implementation
//
////////////////////////////////////////////////////////////////////////

hkMultiThreadedSimulation::hkMultiThreadedSimulation( hkWorld* world )
:
	hkContinuousSimulation( world ),
	m_addCrossIslandPairCriticalSection( 4000), 
	m_removeCrossIslandPairCriticalSection( 4000 ),
	m_stepInitSemaphore(0, 10),
	m_threadTokenCriticalSection(4000),
	m_toiQueueCriticalSection(4000),
	m_phantomCriticalSection(4000)
{
	m_numActiveThreads = 0;
	m_crossIslandPairsCollectingActive = false;
	HK_ASSERT(0x1aa46876, m_world->m_broadPhaseDispatcher != HK_NULL );

	m_world->getBroadPhase()->enableMultiThreading(10000);

	m_entityEntityBroadPhaseListener.m_simulation = this;
	m_phantomBroadPhaseListener.m_criticalSection = & m_phantomCriticalSection;
	world->m_broadPhaseDispatcher->setBroadPhaseListener( &m_entityEntityBroadPhaseListener, hkWorldObject::BROAD_PHASE_ENTITY, hkWorldObject::BROAD_PHASE_ENTITY);

	world->m_broadPhaseDispatcher->setBroadPhaseListener( &m_phantomBroadPhaseListener, hkWorldObject::BROAD_PHASE_PHANTOM, hkWorldObject::BROAD_PHASE_ENTITY);
	world->m_broadPhaseDispatcher->setBroadPhaseListener( &m_phantomBroadPhaseListener, hkWorldObject::BROAD_PHASE_ENTITY,  hkWorldObject::BROAD_PHASE_PHANTOM);
	world->m_broadPhaseDispatcher->setBroadPhaseListener( &m_phantomBroadPhaseListener, hkWorldObject::BROAD_PHASE_PHANTOM, hkWorldObject::BROAD_PHASE_PHANTOM);

	m_broadPhaseBorderListener.m_criticalSection = & m_phantomCriticalSection;
	// Extra five records for the Broad Phase Borders
	world->m_broadPhaseDispatcher->setBroadPhaseListener(&m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_ENTITY, hkWorldObject::BROAD_PHASE_BORDER);
	world->m_broadPhaseDispatcher->setBroadPhaseListener(&m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_BORDER, hkWorldObject::BROAD_PHASE_ENTITY);

	// Use boder listeners to handle border-phantom overlaps.
	world->m_broadPhaseDispatcher->setBroadPhaseListener(&m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_PHANTOM, hkWorldObject::BROAD_PHASE_BORDER);
	world->m_broadPhaseDispatcher->setBroadPhaseListener(&m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_BORDER, hkWorldObject::BROAD_PHASE_PHANTOM);
	world->m_broadPhaseDispatcher->setBroadPhaseListener(&m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_BORDER, hkWorldObject::BROAD_PHASE_BORDER);


	m_jobQueue.m_jobAddFunc    = hkJobQueueUtils::addDynamicsJob;
	m_jobQueue.m_jobPopFunc    = hkJobQueueUtils::popDynamicsJob;
	m_jobQueue.m_finishJobFunc = hkJobQueueUtils::finishDynamicsJob;
	m_jobQueue.m_userData       = world;
}

hkMultiThreadedSimulation::~hkMultiThreadedSimulation()
{
}



bool pairLessThan( const hkTypedBroadPhaseHandlePair& A, const hkTypedBroadPhaseHandlePair& B )
{
	hkCollidable* collA1 = static_cast<hkCollidable*>( A.getElementA()->getOwner() );
	hkCollidable* collA2 = static_cast<hkCollidable*>( A.getElementB()->getOwner() );
	hkCollidable* collB1 = static_cast<hkCollidable*>( B.getElementA()->getOwner() );
	hkCollidable* collB2 = static_cast<hkCollidable*>( B.getElementB()->getOwner() );

	int A1 = hkGetRigidBody( collA1 )->getUid();
	int A2 = hkGetRigidBody( collA2 )->getUid();
	int B1 = hkGetRigidBody( collB1 )->getUid();
	int B2 = hkGetRigidBody( collB2 )->getUid();
	
	if ( A1 < B1 )
	{
		return true;
	}
	if ( A1 == B1)
	{
		return A2 < B2;
	}
	return false;
}

static void sortPairList( hkArray<hkTypedBroadPhaseHandlePair>& pairs )
{

	// sort lower uid as object 0
	{
		hkTypedBroadPhaseHandlePair* pair = pairs.begin();
		for (int i= pairs.getSize()-1; i>=0; i--)
		{
			hkCollidable* collA1 = static_cast<hkCollidable*>( pair->getElementA()->getOwner() );
			hkCollidable* collA2 = static_cast<hkCollidable*>( pair->getElementB()->getOwner() );
			int A1 = hkGetRigidBody( collA1 )->getUid();
			int A2 = hkGetRigidBody( collA2 )->getUid();
			if ( A1 > A2 )
			{
				hkAlgorithm::swap( pair->m_a, pair->m_b );
			}
		}
	}

	hkAlgorithm::quickSort(pairs.begin(), pairs.getSize(), pairLessThan);


	if ( HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS )
	{
		hkLocalBuffer<hkUint32> uids( pairs.getSize()*2);
		int d = 0;
		{
			for (int i=0; i < pairs.getSize();i++)
			{ 
				hkCollidable* collA1 = static_cast<hkCollidable*>( pairs[i].getElementA()->getOwner() );
				hkCollidable* collA2 = static_cast<hkCollidable*>( pairs[i].getElementB()->getOwner() );
				int A1 = hkGetRigidBody( collA1 )->getUid();
				int A2 = hkGetRigidBody( collA2 )->getUid();
				if ( A1 > A2 ){ hkAlgorithm::swap(A1,A2); }
				uids[d++] = A1;
				uids[d++] = A2;
			}
		}
		hkCheckDeterminismUtil::checkSt(uids.begin(), pairs.getSize()*2 );
	}

}

hkThreadToken hkMultiThreadedSimulation::getThreadToken()
{
	m_threadTokenCriticalSection.enter();
	int numThreads = ++m_numActiveThreads;
	HK_ASSERT2( 0xf032e445, numThreads<10, "If you use the multi threaded simulation, you have to call hkWorld::m_simulation->resetThreadTokens() from a single thread, before you call stepDeltaTime()" );

	if ( numThreads == 1 )
	{
		m_threadTokenCriticalSection.leave();
		return HK_THREAD_TOKEN_PRIMARY;
	}
	else
	{
		m_threadTokenCriticalSection.leave();
		return HK_THREAD_TOKEN_SECONDARY;
	}
}

void hkMultiThreadedSimulation::resetThreadTokens()
{
	m_numActiveThreads = 0;
}


hkStepResult hkMultiThreadedSimulation::stepDeltaTime( hkReal physicsDeltaTime )
{
	HK_TIMER_BEGIN( "Simulate", HK_NULL );

	hkThreadToken token = getThreadToken();
	if ( token == HK_THREAD_TOKEN_PRIMARY )
	{
		// One thread processes this
		stepBeginSt( physicsDeltaTime );
		m_stepInitSemaphore.release( m_numActiveThreads );
	}

	m_stepInitSemaphore.acquire();

	// All threads process
	stepProcessMt( token );

	if ( token == HK_THREAD_TOKEN_PRIMARY )
	{
		stepEndSt();
	}

	HK_TIMER_END();

	return HK_STEP_RESULT_SUCCESS;
}


void hkMultiThreadedSimulation::stepBeginSt( hkReal physicsDeltaTime )
{
	m_world->markForWrite();

	HK_ASSERT2(0xad000070, !m_world->areCriticalOperationsLocked(), "The world cannot be locked when calling stepWorldInitST()");
	HK_ASSERT2(0xadef876d, !m_world->m_pendingOperationsCount, "No operations may be pending on the hkWorld::m_pendingOperations queue when calling stepWorldInitST");
	HK_ASSERT2(0xadfefed7, hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OK, "All memory exceptions must be handled and the memory state flag must be set back to hkMemory::MEMORY_STATE_OK");
	HK_ASSERT2(0xa0750079, isSimulationAtPsi(), "You may only call stepBeginSt when the simulation is at a PSI. Use isSimulationAtPsi() to check for this.");

	m_crossIslandPairsCollectingActive = true;
	m_world->lockCriticalOperations();
	m_world->m_pendingOperations->m_storeIslandMergesOnSeparateList = true;

	HK_ON_DEBUG(checkDeltaTimeIsOk( physicsDeltaTime ));

	m_physicsDeltaTime = physicsDeltaTime;


	hkStepInfo physicsStepInfo( m_currentPsiTime, m_currentPsiTime + m_physicsDeltaTime );
	
	m_world->m_dynamicsStepInfo.m_stepInfo = physicsStepInfo;
	m_world->m_collisionInput->m_stepInfo = physicsStepInfo;

	// perform maintenance
	{
		m_world->m_maintenanceMgr->performMaintenanceNoSplit( m_world, physicsStepInfo );
		hkWorldOperationUtil::cleanupDirtyIslands(m_world);
	}
	
	hkSolveUpload();

	//
	// Initialize all parameters of the dynamics step into that depend on the stepInfo
	//
	{
		// Step Info
		m_world->m_dynamicsStepInfo.m_stepInfo = physicsStepInfo;

		// Solver Info
		hkSolverInfo& solverInfo  = m_world->m_dynamicsStepInfo.m_solverInfo;
		solverInfo.m_deltaTime	  = physicsStepInfo.m_deltaTime    * solverInfo.m_invNumSteps;
		solverInfo.m_invDeltaTime = physicsStepInfo.m_invDeltaTime * solverInfo.m_numSteps;
		solverInfo.m_globalAccelerationPerSubStep.setMul4( solverInfo.m_deltaTime, m_world->m_gravity );
		solverInfo.m_globalAccelerationPerStep.setMul4( physicsStepInfo.m_deltaTime, m_world->m_gravity );
	}

	int numIslands = m_world->getActiveSimulationIslands().getSize();
	if (numIslands > 0)
	{
		// Process actions
		if (m_world->m_processActionsInSingleThread)
		{
			m_world->unlockCriticalOperationsForPhantoms();

			applyActions();

			m_world->lockCriticalOperationsForPhantoms();
		}

		// Create the first job - this will spawn other jobs that will eventually complete the step
		hkIntegrateJob job(numIslands);
		m_jobQueue.setSpuQueueCapacity(numIslands);
		m_jobQueue.addJob((hkJobQueue::JobQueueEntry&)job, hkJobQueue::JOB_LOW_PRIORITY, hkJobQueue::JOB_TYPE_CPU);
	}
    m_world->unmarkForWrite();
	m_world->m_multiThreadLock.markForRead( hkMultiThreadLock::THIS_OBJECT_ONLY );
	m_world->getFixedIsland()->markAllEntitiesReadOnly();
}

void hkMultiThreadedSimulation::stepProcessMt( const hkThreadToken& token )
{
	if (token == HK_THREAD_TOKEN_PRIMARY)
	{
		processNextJob( hkJobQueue::THREAD_TYPE_CPU_PRIMARY );
	}
	else
	{
		processNextJob( hkJobQueue::THREAD_TYPE_CPU );
	}
}


void hkMultiThreadedSimulation::stepEndSt()
{
	m_world->getFixedIsland()->unmarkAllEntitiesReadOnly();
	m_world->m_multiThreadLock.unmarkForRead();
	m_world->markForWrite();

	m_world->checkDeterminism();

	if ( m_addedCrossIslandPairs.getSize() + m_removedCrossIslandPairs.getSize())
	{
		// duplicates can be introduced if 2 islands are chasing each other
 		HK_TIMER_BEGIN_LIST("InterIsland", "duplicates");
 		hkTypedBroadPhaseDispatcher::removeDuplicates(  reinterpret_cast<hkArray<hkBroadPhaseHandlePair>&>(m_addedCrossIslandPairs),
 			reinterpret_cast<hkArray<hkBroadPhaseHandlePair>&>(m_removedCrossIslandPairs) );

			// we need to sort to keep determinism
		HK_TIMER_SPLIT_LIST("sortPairs");
		sortPairList( m_addedCrossIslandPairs   );
		sortPairList( m_removedCrossIslandPairs );

		{
			HK_TIMER_SPLIT_LIST("addAgt");
			HK_ALIGN16( hkProcessCollisionOutput processOutput(HK_NULL) );
			for ( int i = 0; i < m_addedCrossIslandPairs.getSize(); ++i )
			{
				hkLinkedCollidable* collA = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(m_addedCrossIslandPairs[i].m_a)->getOwner() );
				hkLinkedCollidable* collB = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(m_addedCrossIslandPairs[i].m_b)->getOwner() );

					// merge the island immediately
				{
					hkEntity* entityA = static_cast<hkEntity*>(collA->getOwner());
					hkEntity* entityB = static_cast<hkEntity*>(collB->getOwner());

					hkSimulationIsland* islandA = entityA->getSimulationIsland();
					hkSimulationIsland* islandB = entityB->getSimulationIsland();

					if ( islandA != islandB)
					{
						islandA = hkWorldOperationUtil::internalMergeTwoIslands( m_world, islandA, islandB );
					}
					processOutput.m_constraintOwner = islandA;
				}
				hkAgentNnEntry* entry = addAgentHelperFunc( collA, collB, m_world->getCollisionInput() );

				if ( entry != HK_NULL )
				{

					processAgentHelperFunc( entry, *m_world->getCollisionInput(), processOutput, this );
				}
			}
			m_addedCrossIslandPairs.clear();
		}
		HK_TIMER_SPLIT_LIST("removeAgt");
		{
			// This (removedCrossIslandPairs) should happen very seldomly. 
			// For this reason we're just okay to call
			//   hkWorldAgentUtil::removeAgentAndToiEvents(entry);
			// Also no extra locking is necessary, as this code section is single threaded / critical-section protected already.
			HK_ACCESS_CHECK_OBJECT( m_world, HK_ACCESS_RW );
			for ( int i = 0; i < m_removedCrossIslandPairs.getSize(); ++i )
			{
				hkLinkedCollidable* collA = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(m_removedCrossIslandPairs[i].m_a)->getOwner() );
				hkLinkedCollidable* collB = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(m_removedCrossIslandPairs[i].m_b)->getOwner() );
				hkAgentNnEntry* entry = hkAgentNnMachine_FindAgent(collA, collB);
				if (entry)
				{
					// this is single threaded -- no locking
					hkWorldAgentUtil::removeAgentAndItsToiEvents(entry);
				}
			}
			m_removedCrossIslandPairs.clear();
		}
		HK_TIMER_END_LIST();
	}

	m_crossIslandPairsCollectingActive = false;
	m_world->m_pendingOperations->m_storeIslandMergesOnSeparateList = false;
	if (m_world->m_pendingOperations->m_pending.getSize())
	{
		HK_WARN_ONCE(0x20096aae, "Critical operations generated during simulation's step. They're not deterministically ordered, and therefore the world may behave nondeterministically from here on.");
	}
	m_world->unlockAndAttemptToExecutePendingOperations();

	m_currentPsiTime = m_currentPsiTime + m_physicsDeltaTime;

	if ( m_world->m_worldPostCollideListeners.getSize() )
	{
		HK_TIMER_BEGIN("PostCollideCB", HK_NULL);
		hkStepInfo physicsStepInfo( m_currentPsiTime - m_physicsDeltaTime, m_currentPsiTime );
		hkWorldCallbackUtil::firePostCollideCallback( m_world, physicsStepInfo );
		HK_TIMER_END();
	}

	m_world->unmarkForWrite();

	//
	//	Process TOIs
	//

	// Function used from hkContinuousSimulation
	advanceTime();

	return;
}


void hkMultiThreadedSimulation::getMultithreadConfig( hkMultithreadConfig& config )
{
	config = m_multithreadConfig;
}

void hkMultiThreadedSimulation::setMultithreadConfig( const hkMultithreadConfig& config )
{
	m_multithreadConfig = config;

#if defined(HK_PLATFORM_HAS_SPU)
	if (config.m_canCpuTakeSpuTasks == hkMultithreadConfig::CPU_CAN_NOT_TAKE_SPU_TASKS)
	{
		m_jobQueue.m_jobQueryRules[(int)hkJobQueue::THREAD_TYPE_CPU_PRIMARY].m_numJobTypesAvailable = hkJobQueue::JOB_TYPE_SPU;
		m_jobQueue.m_jobQueryRules[(int)hkJobQueue::THREAD_TYPE_CPU].m_numJobTypesAvailable         = hkJobQueue::JOB_TYPE_SPU;
	}
	else
#endif
	{
		m_jobQueue.m_jobQueryRules[(int)hkJobQueue::THREAD_TYPE_CPU_PRIMARY].m_numJobTypesAvailable = hkJobQueue::JOB_TYPE_MAX;
		m_jobQueue.m_jobQueryRules[(int)hkJobQueue::THREAD_TYPE_CPU].m_numJobTypesAvailable         = hkJobQueue::JOB_TYPE_MAX;
	}

}


void hkMultiThreadedSimulation::assertThereIsNoCollisionInformationForAgent( hkAgentNnEntry* agent )
{
#if defined HK_DEBUG
	hkCriticalSectionLock lock( &m_toiQueueCriticalSection );

	hkContinuousSimulation::assertThereIsNoCollisionInformationForAgent( agent );
#endif // if defined HK_DEBUG
}

void hkMultiThreadedSimulation::assertThereIsNoCollisionInformationForEntities( hkEntity** entities, int numEntities, hkWorld* world )
{
#if defined HK_DEBUG
	HK_ASSERT(0xad44bb3e, numEntities && entities[0]->getWorld() );

	hkCriticalSectionLock lock( &m_toiQueueCriticalSection );

	hkContinuousSimulation::assertThereIsNoCollisionInformationForEntities( entities, numEntities, world );
#endif // if defined HK_DEBUG
}

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
