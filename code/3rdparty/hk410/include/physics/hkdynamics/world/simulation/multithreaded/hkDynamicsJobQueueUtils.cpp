/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/hkWorld.h>

#include <hkbase/thread/hkSpuDmaManager.h>

#include <hkdynamics/constraint/setup/hkConstraintSolverSetup.h>
#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobs.h>
#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobQueueUtils.h>
#include <hkbase/thread/hkSpuStack.h>

#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuConstraint.h>
#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuConfig.h>
#include <hkdynamics/world/commandqueue/hkPhysicsCommandQueue.h>



HK_COMPILE_TIME_ASSERT( sizeof( hkIntegrateJob )         <= sizeof( hkJobQueue::JobQueueEntry ) );
HK_COMPILE_TIME_ASSERT( sizeof( hkBuildAccumulatorsJob ) <= sizeof( hkJobQueue::JobQueueEntry ) );
HK_COMPILE_TIME_ASSERT( sizeof( hkBuildJacobiansJob )    <= sizeof( hkJobQueue::JobQueueEntry ) );
HK_COMPILE_TIME_ASSERT( sizeof( hkSolveConstraintsJob )  <= sizeof( hkJobQueue::JobQueueEntry ) );
HK_COMPILE_TIME_ASSERT( sizeof( hkBroadPhaseJob )        <= sizeof( hkJobQueue::JobQueueEntry ) );
HK_COMPILE_TIME_ASSERT( sizeof( hkAgentSectorJob )       <= sizeof( hkJobQueue::JobQueueEntry ) );
HK_COMPILE_TIME_ASSERT( sizeof( hkBuildJacobiansTaskJob )<= sizeof( hkJobQueue::JobQueueEntry ) );



#if !defined (HK_PLATFORM_PS3SPU)
hkAgentSectorHeader* hkAgentSectorHeader::allocate(int numTasks)
{
	int size = hkAgentSectorHeader::getAllocatedSize( numTasks );
	hkAgentSectorHeader* sh = static_cast<hkAgentSectorHeader*>(hkThreadMemory::getInstance().allocateChunk(size, HK_MEMORY_CLASS_DYNAMICS));

	sh->m_openJobs = numTasks;

	JobInfo** jobInfos = reinterpret_cast<JobInfo**>(sh+1);
	for ( int i =0; i < numTasks; i++)
	{
		jobInfos[i] = new JobInfo;
	}
	return sh;
}

void hkAgentSectorHeader::deallocate(int numTasks)
{
	JobInfo** jobInfos = reinterpret_cast<JobInfo**>(this+1);
	for ( int i =0; i < numTasks; i++)
	{
		delete jobInfos[i];
	}

	int size = hkAgentSectorHeader::getAllocatedSize( numTasks );
	hkThreadMemory::getInstance().deallocateChunk( this, size, HK_MEMORY_CLASS_DYNAMICS );
}
#endif



////////////////////////////////////////////////////////////////////////
//
// Callbacks from job queue
//
////////////////////////////////////////////////////////////////////////
void HK_CALL hkJobQueueUtils::addDynamicsJob( hkJobQueue& queue, hkJobQueue::DynamicData* data, const hkJobQueue::JobQueueEntry& jobIn )
{
	const hkDynamicsJob& job = reinterpret_cast<const hkDynamicsJob&>(jobIn);
	switch ( job.m_jobType )
	{
		case DYNAMICS_JOB_INTEGRATE:
		{
			const hkIntegrateJob& s = static_cast<const hkIntegrateJob&>(job);
			data->m_moreSpuJobsPossible = s.m_numIslands;
			break;
		}
		default:
			break;
	}
}

hkJobQueue::JobPopFuncResult HK_CALL hkJobQueueUtils::popDynamicsJob( hkJobQueue& queue, hkJobQueue::DynamicData* data, hkJobQueue::JobQueueEntry& jobIn, hkJobQueue::JobQueueEntry& jobOut )
{
	// onQueue is the job that stays on the queue. It is modified so that it points to the next task after the one being extracted.
	// out     is the job that points to the task being released. This is the image/clone to be used locally by the thread.

	hkDynamicsJob& job = reinterpret_cast<hkDynamicsJob&>(jobIn);

	hkArray<hkSimulationIsland*>* islands;
	if ( !HK_PLATFORM_IS_SPU )
	{
		hkWorld* world = static_cast<hkWorld*>(queue.m_userData);
		islands = &world->m_activeSimulationIslands;
	}
	else
	{
		islands = HK_NULL;
	}

	switch ( job.m_jobType )
	{ 
		case DYNAMICS_JOB_INTEGRATE:
			{
				hkIntegrateJob& onQueue = static_cast<hkIntegrateJob&>(job);
				hkIntegrateJob& out = reinterpret_cast<hkIntegrateJob&>(jobOut);
				return onQueue.popJobTask( *islands, out );
			}

		case DYNAMICS_JOB_BUILD_ACCUMULATORS:
			{
				hkBuildAccumulatorsJob& onQueue = static_cast<hkBuildAccumulatorsJob&>(job);
				hkBuildAccumulatorsJob& out     = reinterpret_cast<hkBuildAccumulatorsJob&>(jobOut);
				return onQueue.popJobTask( *islands, out );
			}

		case DYNAMICS_JOB_BUILD_JACOBIANS:
			{
				hkBuildJacobiansJob& onQueue = reinterpret_cast<hkBuildJacobiansJob&>(job);
				hkBuildJacobiansJob& out = reinterpret_cast<hkBuildJacobiansJob&>(jobOut);
				return onQueue.popJobTask( *islands, out );
			}

#if !defined (HK_PLATFORM_PS3SPU)
		case DYNAMICS_JOB_BROADPHASE:
			{
				hkBroadPhaseJob& onQueue = reinterpret_cast<hkBroadPhaseJob&>(job);
				hkBroadPhaseJob& out = reinterpret_cast<hkBroadPhaseJob&>(jobOut);
				return onQueue.popJobTask( *islands, out );
			}
#endif

		case DYNAMICS_JOB_SPLIT_ISLAND:
			{
				hkSplitSimulationIslandJob& onQueue = reinterpret_cast<hkSplitSimulationIslandJob&>(job);
				hkSplitSimulationIslandJob& out = reinterpret_cast<hkSplitSimulationIslandJob&>(jobOut);
				return onQueue.popJobTask( *islands, out );
			}

		case DYNAMICS_JOB_INTEGRATE_MOTION:
			{
				hkIntegrateMotionJob& onQueue = static_cast<hkIntegrateMotionJob&>(job);
				hkIntegrateMotionJob& out     = reinterpret_cast<hkIntegrateMotionJob&>(jobOut);
				return onQueue.popJobTask( *islands, out );
			}

		case DYNAMICS_JOB_FIRE_JACOBIAN_SETUP_CALLBACK_AND_BUILD_PPU_JACOBIANS:
		case DYNAMICS_JOB_SOLVE_CONSTRAINTS:
		case DYNAMICS_JOB_POST_COLLIDE:
			{
				hkDynamicsJob& onQueue = static_cast<hkDynamicsJob&>(job);
				hkDynamicsJob& out     = reinterpret_cast<hkDynamicsJob&>(jobOut);
				return onQueue.popDynamicsJobTask( *islands, out );
			}

		case DYNAMICS_JOB_AGENT_SECTOR:
			{
				hkAgentSectorJob& onQueue = static_cast<hkAgentSectorJob&>(job);
				hkAgentSectorJob& out = reinterpret_cast<hkAgentSectorJob&>(jobOut);
				return onQueue.popJobTask( *islands, out );
			}

		default:
			{
				HK_ASSERT2(0xad789ddd, false, "Won't handle this job!!!");
				break;
			}

	}
	return hkJobQueue::POP_QUEUE_ENTRY;
}

static HK_LOCAL_INLINE const hkBuildJacobianTaskHeader* downloadTaskHeader(const hkBuildJacobianTaskHeader* taskHeaderInMainMemory, hkBuildJacobianTaskHeader* buffer)
{
	const hkBuildJacobianTaskHeader* localTaskHeader;
	if ( HK_PLATFORM_IS_SPU )
	{
		localTaskHeader = buffer;
		hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( buffer, taskHeaderInMainMemory, sizeof(hkBuildJacobianTaskHeader), hkSpuDmaManager::READ_COPY );
		hkSpuDmaManager::performFinalChecks                   ( taskHeaderInMainMemory, buffer, sizeof(hkBuildJacobianTaskHeader) );
	}
	else
	{
		localTaskHeader = taskHeaderInMainMemory;
	}
	return localTaskHeader;
}

static HK_LOCAL_INLINE void uploadOpenJobsVariable( const hkBuildJacobianTaskHeader* localTaskHeader, const hkBuildJacobianTaskHeader* taskHeaderInMainMemory )
{
	if ( HK_PLATFORM_IS_SPU )
	{
		HK_CPU_PTR(hkBuildJacobianTaskHeader*) dest = const_cast<HK_CPU_PTR(hkBuildJacobianTaskHeader*)>(taskHeaderInMainMemory);
		hkSpuDmaManager::putToMainMemorySmallAndWaitForCompletion( &dest->m_openJobs, &localTaskHeader->m_openJobs, sizeof(int), hkSpuDmaManager::WRITE_NEW );
		hkSpuDmaManager::performFinalChecks                      ( &dest->m_openJobs, &localTaskHeader->m_openJobs, sizeof(int) );
	}
}


static HK_LOCAL_INLINE hkJobQueue::JobType whereToRunSolveJob(const hkBuildJacobianTaskHeader& taskHeader)
{
#if defined(HK_PLATFORM_HAS_SPU)

	HK_ASSERT( 0xf023fdff, true );					// synchronize the next lines with assert 0xf023edfe

	// synchronize this with all allocateStack() calls within hkSpuSolveConstraintsJob()!
	int totalStackMem = taskHeader.m_bufferSize + sizeof(hkSpuIntegrateDataSetTool) + int( HK_NEXT_MULTIPLE_OF(16, (taskHeader.m_numAllEntities * sizeof(HK_CPU_PTR(hkEntity*))) ) );

	if ( totalStackMem < HK_SPU_TOTAL_BUFFER_SIZE )
	{
		return hkJobQueue::JOB_TYPE_SPU;
	}
#endif
	return hkJobQueue::JOB_TYPE_CPU;
}

static HK_LOCAL_INLINE void spawnSplitSimulationIslandJob( const hkBuildJacobianTaskHeader* localTaskHeader, const hkSolveConstraintsJob* scj, hkJobQueue& queue, hkJobQueue::DynamicData* data )
{
	if ( localTaskHeader->m_splitCheckRequested )
	{
		HK_ALIGN16( hkSplitSimulationIslandJob splitJob( *scj ) );
		data->m_jobQueue[hkJobQueue::JOB_TYPE_CPU].enqueueInFront( (const hkJobQueue::JobQueueEntry&)splitJob );
#if defined (HK_PLATFORM_SPU)
		queue.releaseOneWaitingThread( hkJobQueue::JOB_TYPE_CPU, data );
#endif
	}
}


hkJobQueue::JobCreationStatus HK_CALL hkJobQueueUtils::finishDynamicsJob( hkJobQueue& queue, hkJobQueue::DynamicData* data, const hkJobQueue::JobQueueEntry& jobIn, hkJobQueue::JobQueueEntryInput& newJobCreated )
{
	int numJobsToFinish = 1;

	const hkDynamicsJob& job = reinterpret_cast<const hkDynamicsJob&>(jobIn);
	switch( job.m_jobType )
	{
		case DYNAMICS_JOB_BUILD_ACCUMULATORS:
		case DYNAMICS_JOB_BUILD_JACOBIANS_TASK:
			{
				const hkBuildAccumulatorsJob& baj = reinterpret_cast<const hkBuildAccumulatorsJob&>(job);

				hkBuildJacobianTaskHeader taskHeaderBufferOnSpu;
				const hkBuildJacobianTaskHeader* localTaskHeader = downloadTaskHeader(baj.m_taskHeader, &taskHeaderBufferOnSpu);

				//
				// decrease m_openJobs in taskHeader and check for creating a new job
				//

				hkJobQueue::JobCreationStatus status = hkJobQueue::NO_JOB_CREATED;
				localTaskHeader->m_openJobs--;
				if ( localTaskHeader->m_openJobs == 0 )
				{
					if ( localTaskHeader->m_tasks.m_numCallbackConstraints != 0
#if defined(HK_PLATFORM_HAS_SPU)
						|| localTaskHeader->m_tasks.m_numPpuSetupOnlyConstraints > 0
#endif
						)
					{
						// last job finished && callback requests -> convert job
						new (&newJobCreated.m_job) hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob(baj);
						newJobCreated.m_jobPriority     = hkJobQueue::JOB_HIGH_PRIORITY;
						newJobCreated.m_jobThreadType   = hkJobQueue::JOB_TYPE_CPU;
					}
					else
					{
						// last job finished && !callback requests -> convert job
						new (&newJobCreated.m_job) hkBuildJacobiansJob( baj, localTaskHeader->m_tasks.m_buildJacobianTasks, localTaskHeader->m_constraintQueryIn );
						localTaskHeader->m_openJobs   = localTaskHeader->m_tasks.m_numBuildJacobianTasks;
						newJobCreated.m_jobPriority   = hkJobQueue::JOB_HIGH_PRIORITY;
						newJobCreated.m_jobThreadType = hkJobQueue::JOB_TYPE_HINT_SPU;
					}
					status = hkJobQueue::JOB_CREATED;
				}
				uploadOpenJobsVariable( localTaskHeader, baj.m_taskHeader );
				return status;
			}

#if !defined (HK_PLATFORM_PS3SPU)
		case DYNAMICS_JOB_FIRE_JACOBIAN_SETUP_CALLBACK_AND_BUILD_PPU_JACOBIANS:
			{
				const hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob& fjscb = reinterpret_cast<const hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob&>(job);

				if ( fjscb.m_taskHeader->m_tasks.m_buildJacobianTasks->m_numAtomInfos > 0 ) {
					//
					// there's still some setup left that can be done on spu -> convert job into a high priority build jacobians job
					//
					new (&newJobCreated.m_job) hkBuildJacobiansJob( fjscb );
					fjscb.m_taskHeader->m_openJobs = fjscb.m_taskHeader->m_tasks.m_numBuildJacobianTasks;
					newJobCreated.m_jobPriority   = hkJobQueue::JOB_HIGH_PRIORITY;
					newJobCreated.m_jobThreadType = hkJobQueue::JOB_TYPE_HINT_SPU;
				}
				else
				{
					//
					// no more setup to be done -> convert job into a high priority solve constraints job
					//
					hkSolveConstraintsJob* scj = new (&newJobCreated.m_job) hkSolveConstraintsJob( fjscb );
					newJobCreated.m_jobPriority   = hkJobQueue::JOB_HIGH_PRIORITY;
					newJobCreated.m_jobThreadType = whereToRunSolveJob(*fjscb.m_taskHeader);

					spawnSplitSimulationIslandJob( fjscb.m_taskHeader, scj, queue, data );
				}

				return hkJobQueue::JOB_CREATED;
			}
#endif

		case DYNAMICS_JOB_BUILD_JACOBIANS:
			{
				const hkBuildJacobiansJob& bjj = reinterpret_cast<const hkBuildJacobiansJob&>(job);
				HK_CPU_PTR(hkBuildJacobianTaskHeader*) taskHeaderInMainMemory = bjj.m_buildJacobianTask->m_taskHeader;

				hkBuildJacobianTaskHeader taskHeaderBufferOnSpu;
				const hkBuildJacobianTaskHeader* localTaskHeader = downloadTaskHeader(taskHeaderInMainMemory, &taskHeaderBufferOnSpu);
				localTaskHeader->m_openJobs--;
				uploadOpenJobsVariable( localTaskHeader, taskHeaderInMainMemory );

#if defined(HK_PLATFORM_POTENTIAL_SPU)
				if ( HK_PLATFORM_IS_SPU )
				{
					// deallocate job-specific memory
					hkSpuDmaManager::performFinalChecks( HK_NULL, bjj.m_buildJacobianTask, sizeof(hkBuildJacobianTask) );
					hkSpuStack::getInstance().deallocateStack(sizeof(hkBuildJacobianTask));
				}  
#endif
				if ( localTaskHeader->m_openJobs == 0 )
				{
					//
					// last job finished -> convert job into a high priority solve constraints job, also spawn off a split simulation island job
					//
					hkSolveConstraintsJob* scj = new (&newJobCreated.m_job) hkSolveConstraintsJob(bjj, *localTaskHeader, taskHeaderInMainMemory );
					newJobCreated.m_jobPriority   = hkJobQueue::JOB_HIGH_PRIORITY;
					newJobCreated.m_jobThreadType = whereToRunSolveJob(*localTaskHeader);

					spawnSplitSimulationIslandJob( localTaskHeader, scj, queue, data );

					return hkJobQueue::JOB_CREATED;
				}

				break;
			}

		case DYNAMICS_JOB_SOLVE_CONSTRAINTS:
			{
				// this means the integration of the island is finished.
				// currently SPUs only help with integrating an island, so once
				// the collision detector starts on an island, no more spu work is possible
				// for that island.
				data->m_moreSpuJobsPossible--;

				const hkSolveConstraintsJob& scj = reinterpret_cast<const hkSolveConstraintsJob&>(job);
				HK_CPU_PTR(hkBuildJacobianTaskHeader*) taskHeaderInMainMemory = scj.m_taskHeader;

				// allow broadphase to continue with stage 2 (remove & add pairs)
				// important: the last job to finish must be on the ppu (so that it can release the job queue); if there are no agent sector jobs, this last job
				// has to be the broadphase job; to assure that the broadphase job (which got started by this solve job) does not overtake the solve job we need
				// to wait until this solve job is finished (i.e. we get here) before letting the (currently blocking) broadphase to continue
				hkSpuDmaUtils::setChar8InMainMemory(&taskHeaderInMainMemory->m_exportFinished, hkChar(1));

				break;
			}

		case DYNAMICS_JOB_INTEGRATE_MOTION:
			{
				const hkIntegrateMotionJob& imj = reinterpret_cast<const hkIntegrateMotionJob&>(job);
				if ( imj.m_numInactiveFrames <= hkMotion::NUM_INACTIVE_FRAMES_TO_DEACTIVATE)
				{
					hkSpuDmaUtils::setInt32InMainMemory( &imj.m_taskHeader->m_islandShouldBeDeactivated, 0);
				}
				HK_ASSERT( 0xf0002123, imj.m_numEntities>0);
				numJobsToFinish = 1 + ( unsigned(imj.m_numEntities-1)/hkIntegrateMotionJob::ACCUMULATORS_PER_JOB);
				// no break here
			}
		case DYNAMICS_JOB_SPLIT_ISLAND:
			{
				const hkSplitSimulationIslandJob& sij = reinterpret_cast<const hkSplitSimulationIslandJob&>(job);

				int openJobs = hkSpuDmaUtils::incrementInt32InMainMemory( &sij.m_taskHeader->m_numUnfinishedJobsForBroadphase, -numJobsToFinish );
				HK_ASSERT( 0xf0343212, openJobs >= 0 );
				if ( openJobs == 0 )
				{
					new (&newJobCreated.m_job) hkBroadPhaseJob(sij);
					newJobCreated.m_jobPriority   = hkJobQueue::JOB_HIGH_PRIORITY;
					newJobCreated.m_jobThreadType = hkJobQueue::JOB_TYPE_CPU_PRIMARY;
					return hkJobQueue::JOB_CREATED;
				}
				return hkJobQueue::NO_JOB_CREATED;
			}

		case DYNAMICS_JOB_AGENT_SECTOR:
			{
				const hkAgentSectorJob& asj = reinterpret_cast<const hkAgentSectorJob&>(job);
				if ( !asj.m_header)
				{
					// without the header, there is only one agent sector job for this island, no need for the PostCollide Job
					return hkJobQueue::NO_JOB_CREATED;
				}

				int openJobs = hkSpuDmaUtils::incrementInt32InMainMemory( &asj.m_header->m_openJobs, -1 );
				if ( openJobs == 0 )
				{
					new (&newJobCreated.m_job) hkPostCollideJob(asj);
					newJobCreated.m_jobPriority   = hkJobQueue::JOB_HIGH_PRIORITY;
					newJobCreated.m_jobThreadType = hkJobQueue::JOB_TYPE_CPU;
					return hkJobQueue::JOB_CREATED;
				}
				return hkJobQueue::NO_JOB_CREATED;
			}

		case DYNAMICS_JOB_POST_COLLIDE:
			{
				//const hkPostCollideJob& pcj = reinterpret_cast<const hkPostCollideJob&>(job);
				return hkJobQueue::NO_JOB_CREATED;
			}

		default:
			{
				break;
			}
	}

	return hkJobQueue::NO_JOB_CREATED;
}



hkMtThreadStructure::hkMtThreadStructure( hkWorld* world, hkMultiThreadedSimulation* simulation, hkJobQueue::ThreadType threadType )
{
	m_world = world;
	m_simulation = simulation;
	m_threadType = threadType;
	m_collisionInput = *world->m_collisionInput;

	m_constraintQueryIn.set( world->m_dynamicsStepInfo.m_solverInfo, world->m_dynamicsStepInfo.m_stepInfo);
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
