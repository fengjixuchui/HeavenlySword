/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkbase/memory/hkMemory.h>
#include <hkbase/htl/hkAlgorithm.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/htl/hkPointerMapBase.h>

#include <hkmath/basetypes/hkAabb.h>

#include <hkinternal/collide/broadphase/hkBroadPhase.h>
#include <hkinternal/collide/broadphase/hkBroadPhaseHandle.h>

#include <hkcollide/dispatch/hkCollisionDispatcher.h>

#include <hkcollide/filter/hkCollisionFilter.h>

#include <hkcollide/agent/hkContactMgr.h>

#include <hkcollide/agent/hkCollisionAgent.h>
#include <hkcollide/agent/hkCollisionInput.h>
#include <hkcollide/agent/hkLinearCastCollisionInput.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>
#include <hkcollide/agent/hkProcessCollisionOutput.h>

#include <hkinternal/collide/agent3/machine/nn/hkAgentNnMachine.h>

#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseDispatcher.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>

#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>

#include <hkdynamics/action/hkAction.h>
#include <hkdynamics/motion/util/hkRigidMotionUtil.h>
#include <hkdynamics/motion/hkMotion.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/constraint/setup/hkConstraintSolverSetup.h>

#include <hkdynamics/entity/hkRigidBody.h>

#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>
#include <hkdynamics/world/util/hkWorldCallbackUtil.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>

#include <hkdynamics/world/simulation/hkSimulation.h>

#include <hkdynamics/collide/hkDynamicsContactMgr.h>

#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkdynamics/world/util/hkWorldOperationQueue.h>

#include <hkdynamics/world/maintenance/hkWorldMaintenanceMgr.h>


hkSimulation::hkSimulation( hkWorld* world )
:	m_world(world),
	m_currentTime( 0.0f ),
	m_currentPsiTime( 0.0f ),
	m_simulateUntilTime(-1.f),
	m_frameMarkerPsiSnap( .0001f ),
	m_previousStepResult(HK_STEP_RESULT_SUCCESS)
{
	m_lastProcessingStep = COLLIDE;
}

hkSimulation::~hkSimulation()
{
}

void hkSimulation::setFrameTimeMarker( hkReal frameDeltaTime )
{
	// This function is also used in multithreading  asychronous stepping, so we call mark / unmark for write
	m_world->markForWrite();
	m_simulateUntilTime = m_currentTime + frameDeltaTime;
	m_world->unmarkForWrite();
}

bool hkSimulation::isSimulationAtMarker()
{
	return( m_currentTime == m_simulateUntilTime );
}

bool hkSimulation::isSimulationAtPsi() const
{
	return ( m_currentPsiTime == m_currentTime );
}


hkStepResult hkSimulation::integrate( hkReal physicsDeltaTime )
{
	HK_ASSERT2(0x9764ea25, m_lastProcessingStep == COLLIDE, "You must not call integrate twice without calling collide");
	HK_ASSERT2(0xadfefed7, hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OK, "All memory exceptions must be handled and the memory state flag must be set back to hkMemory::MEMORY_STATE_OK");
	HK_ASSERT2(0xad000070, !m_world->areCriticalOperationsLocked(), "The m_world cannot be locked when calling integrate()");
	HK_ASSERT2(0xadef876d, !m_world->m_pendingOperationsCount, "No operations may be pending on the hkWorld::m_pendingOperations queue when calling integrate");
	HK_ASSERT2(0xa0750079, isSimulationAtPsi(), "You may only call integrate when the simulation is at a PSI. Use isSimulationAtPsi() to check for this. Common error: advanceTime() not called.");

	HK_ASSERT2(0xcba47962, (m_previousStepResult == HK_STEP_RESULT_SUCCESS) || (physicsDeltaTime == m_physicsDeltaTime), "When recovering from a step failure, you must step with the same delta time" );

	#ifdef HK_DEBUG
		checkDeltaTimeIsOk( physicsDeltaTime );
	#endif

	m_physicsDeltaTime = physicsDeltaTime;

	int memNeeded =	m_world->getMemUsageForIntegration();
	if ( memNeeded > hkThreadMemory::getInstance().getStack().getFreeBytes() ) 
	{
		HK_WARN(0x01983bb7,"Severe performance and stability warning: The stack size in hkMemory is insufficient to solve the current current constraint system. Allocating from system memory. This can fragment your memory");
		if ( memNeeded > hkMemory::getInstance().getAvailableMemory())
		{
			hkMemory::getInstance().m_memoryState = hkMemory::MEMORY_STATE_OUT_OF_MEMORY;
			m_previousStepResult = HK_STEP_RESULT_MEMORY_FAILURE_BEFORE_INTEGRATION;
			return HK_STEP_RESULT_MEMORY_FAILURE_BEFORE_INTEGRATION;
		}
	}

	hkStepInfo physicsStepInfo( m_currentPsiTime, m_currentPsiTime + m_physicsDeltaTime );

	m_world->m_dynamicsStepInfo.m_stepInfo = physicsStepInfo;
	m_world->m_collisionInput->m_stepInfo = physicsStepInfo;


	m_world->m_maintenanceMgr->performMaintenance( m_world, physicsStepInfo );
	integrateInternal( physicsStepInfo );

	m_lastProcessingStep = INTEGRATE;
	m_previousStepResult = HK_STEP_RESULT_SUCCESS;
	return HK_STEP_RESULT_SUCCESS;
}




hkStepResult hkSimulation::collide()
{
	HK_ASSERT2( 0xadfefed7, hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OK, "All memory exceptions must be handled and the memory state flag must be set back to hkMemory::MEMORY_STATE_OK");
	HK_ASSERT2(0xad000070, !m_world->areCriticalOperationsLocked(), "The m_world cannot be locked when calling collide()");
	HK_ASSERT2(0xadef876d, !m_world->m_pendingOperationsCount, "No operations may be pending on the hkWorld::m_pendingOperations queue when calling collide()");

	if ( m_previousStepResult != HK_STEP_RESULT_SUCCESS )
	{
		return reCollideAfterStepFailure();
	}
	else
	{
		HK_ASSERT2(0x9764ea25, m_lastProcessingStep == INTEGRATE, "You must call call collideSt after integrateSt");
		HK_ASSERT2(0xa0750079, isSimulationAtPsi(), "You may only call collide when the simulation is at a PSI. Use isSimulationAtPsi() to check for this.");

		hkStepInfo stepInfo(  m_currentPsiTime, m_currentPsiTime + m_physicsDeltaTime );

		collideInternal( stepInfo );

		// Check memory
		if (hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OUT_OF_MEMORY )
		{
			m_previousStepResult = HK_STEP_RESULT_MEMORY_FAILURE_DURING_COLLIDE;
			return HK_STEP_RESULT_MEMORY_FAILURE_DURING_COLLIDE;
		}

		// Increment the current psi time by the delta time
		m_currentPsiTime += m_physicsDeltaTime;

		//
		// Fire post detection callbacks
		//
		if ( m_world->m_worldPostCollideListeners.getSize() )
		{
			HK_TIMER_BEGIN("PostCollideCB", HK_NULL);
			hkWorldCallbackUtil::firePostCollideCallback( m_world, stepInfo );
			HK_TIMER_END();
		}

		HK_ASSERT(0xad000070, hkDebugInfoOnPendingOperationQueues::areEmpty(m_world) );

		m_lastProcessingStep = COLLIDE;
		m_previousStepResult = HK_STEP_RESULT_SUCCESS;
		return HK_STEP_RESULT_SUCCESS;
	}
}



void hkSimulation::checkDeltaTimeIsOk( hkReal deltaTime )
{
	HK_ASSERT2(0x7486d67e, deltaTime > HK_REAL_EPSILON, 
		"You are trying to step the simulation with a 0 delta time - this will lead to numerical problems, and is not allowed.");

	const hkReal factor = 4.f;
	if(deltaTime <  m_world->m_dynamicsStepInfo.m_stepInfo.m_deltaTime / factor )
	{
		HK_WARN(0x2a2cde91, "Simulation may become unstable, the time step has decreased by more than a factor of " << factor << " from the previous step");
	}
}


// This function is largely the same as the collide() function, but re-written, because of some differences in updating filters,
// deciding when to update m_current time and fire collide callbacks, and what stepInfo to use
hkStepResult hkSimulation::reCollideAfterStepFailure()
{
	HK_ASSERT2( 0xadfefed7, hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OK, "All memory exceptions must be handled and the memory state flag must be set back to hkMemory::MEMORY_STATE_OK");
	HK_ASSERT2(0xad000070, !m_world->areCriticalOperationsLocked(), "The m_world cannot be locked when calling collide()");
	HK_ASSERT2(0xadef876d, !m_world->m_pendingOperationsCount, "No operations may be pending on the hkWorld::m_pendingOperations queue when calling collide()");

	HK_ASSERT2( 0xeee97545, m_previousStepResult != HK_STEP_RESULT_MEMORY_FAILURE_BEFORE_INTEGRATION, "If integrate() fails, you must call integrate() again");

	// Do a full re-evaluation of the broad phase and all shape collections. This is very slow.  
	// The last step may have failed after broadphase update but before
	// agent update, so we re-calc everything.  We could instead store the last broad phase results to avoid this.
	// Also, because the bvTree agent caches the query aabb, need to similarly re-do all the bvTree queries.
	m_world->updateCollisionFilterOnWorld(HK_UPDATE_FILTER_ON_WORLD_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);

	hkStepInfo stepInfo;
	if ( m_previousStepResult == HK_STEP_RESULT_MEMORY_FAILURE_DURING_TOI_SOLVE )
	{
		// If the previous step failed during TOI solve, collide is re-called, from the current time the TOI solve had reached.
		stepInfo.set( m_currentTime, m_currentPsiTime );
	}
	else
	{
		// For normal simulation the simulation must always be at the PSI.
		HK_ASSERT2(0xa0750079, isSimulationAtPsi(), "You may only call collide when the simulation is at a PSI. Use isSimulationAtPsi() to check for this.");
		stepInfo.set( m_currentPsiTime, m_currentPsiTime + m_physicsDeltaTime );
	}

	collideInternal( stepInfo );

	// Check memory
	if (hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OUT_OF_MEMORY )
	{
		// Note: do not change the previous step result here - could be COLLIDE or TOI
		HK_TIMER_END();
		return m_previousStepResult;

	}

	// If we failed in collide() then re-do the collide step
	if ( m_previousStepResult == HK_STEP_RESULT_MEMORY_FAILURE_DURING_COLLIDE )
	{
		// Increment the current psi time by the delta time
		m_currentPsiTime += m_physicsDeltaTime;

		//
		// Fire post detection callbacks
		//
		if ( m_world->m_worldPostCollideListeners.getSize() )
		{
			HK_TIMER_BEGIN("PostCollideCB", HK_NULL);
			hkWorldCallbackUtil::firePostCollideCallback( m_world, stepInfo );
			HK_TIMER_END();
		}


		HK_ASSERT(0xad000070, hkDebugInfoOnPendingOperationQueues::areEmpty(m_world) );

		m_lastProcessingStep = COLLIDE;
	}

	m_previousStepResult = HK_STEP_RESULT_SUCCESS;
	return m_previousStepResult;
}


hkReal hkSimulation::snapSimulateTimeAndGetTimeToAdvanceTo()
{
	m_world->markForWrite();
	// If we are doing asysnchronous stepping, then snap the simulate time to the current PSI time if close enough
	if ( m_simulateUntilTime != -1 )
	{
		if ( hkMath::fabs( m_simulateUntilTime - m_currentPsiTime ) < m_frameMarkerPsiSnap )
		{
			m_simulateUntilTime = m_currentPsiTime;
		}
	}
	m_world->unmarkForWrite();

	hkReal timeToAdvanceTo;

	// If the user has scheduled an asychronous step, then only advance the current time to the frame time
	// specified.

	if ( m_simulateUntilTime == -1.f)
	{
		timeToAdvanceTo = m_currentPsiTime;
	}
	else
	{
		HK_ASSERT2(0xaf687532, m_simulateUntilTime >= m_currentTime, "Once you start calling setStepMarkerSt you must continue to do so." );
		timeToAdvanceTo = hkMath::min2( m_currentPsiTime, m_simulateUntilTime);
	}

	return timeToAdvanceTo;
}

hkStepResult hkSimulation::advanceTime()
{
	m_currentTime = snapSimulateTimeAndGetTimeToAdvanceTo();

	if ( (m_currentTime >= m_simulateUntilTime) && ( m_world->m_worldPostSimulationListeners.getSize() ) )
	{
		//
		// Fire post simulate callbacks --- this must be fired here in order for visualization to be updated
		//
		HK_TIMER_BEGIN("PostSimulateCb", HK_NULL);
		hkWorldCallbackUtil::firePostSimulationCallback( m_world );
		HK_TIMER_END();
	}

	m_previousStepResult = HK_STEP_RESULT_SUCCESS;
	return m_previousStepResult;
}


hkStepResult hkSimulation::stepDeltaTime( hkReal physicsDeltaTime )
{
	HK_TIME_CODE_BLOCK("Simulate", HK_NULL);

	// Initially m_previousStepResult will be the value returned from the last call to stepDeltaTime.
	// Each of these functions sets m_previousStepResult to their return value.
		
	if (	( m_previousStepResult == HK_STEP_RESULT_SUCCESS ) || 
			( m_previousStepResult == HK_STEP_RESULT_MEMORY_FAILURE_BEFORE_INTEGRATION ))
	{
		integrate( physicsDeltaTime );
	}

	if (	( m_previousStepResult == HK_STEP_RESULT_SUCCESS ) || 
			( m_previousStepResult == HK_STEP_RESULT_MEMORY_FAILURE_DURING_COLLIDE ))
	{
		collide();
	}

	if (	( m_previousStepResult == HK_STEP_RESULT_SUCCESS ) || 
			( m_previousStepResult == HK_STEP_RESULT_MEMORY_FAILURE_DURING_TOI_SOLVE ))
	{
		advanceTime();
	}


	return m_previousStepResult;
}



void hkSimulation::collideEntitiesBroadPhaseDiscrete( hkEntity** entities, int numEntities, hkWorld* world )
{
	HK_TIMER_BEGIN_LIST("BroadPhase", "InitMem");

	HK_ASSERT2(0xad63ee38, numEntities > 0, "No entities?");
	HK_ASSERT(0xad63ee37, world->areCriticalOperationsLocked());

	HK_ON_DEBUG(world->m_simulation->assertThereIsNoCollisionInformationForEntities(entities, numEntities, world));

	hkLocalArray<hkBroadPhaseHandlePair> newPairs( world->m_broadPhaseUpdateSize );
	hkLocalArray<hkBroadPhaseHandlePair> delPairs( world->m_broadPhaseUpdateSize );
	{
		hkAabb* aabbs = hkAllocateStack<hkAabb>( numEntities );
		hkBroadPhaseHandle** broadPhaseHandles = hkAllocateStack<hkBroadPhaseHandle*>(numEntities);

		hkReal halfTolerance = world->getCollisionInput()->getTolerance() * .5f;
		HK_TIMER_SPLIT_LIST("CalcAabbs");
		int numAabbs = numEntities;
		{
			hkEntity **e = entities;

			hkAabb *aabb = aabbs;
			hkBroadPhaseHandle **handle = broadPhaseHandles;
			for( int i = numAabbs-1; i >=0 ; e++, i-- )
			{
				hkCollidable* c = e[0]->getCollidableRw();
				//const hkMotionState* ms = c->getMotionState();
				//if ( !ms->m_deactivationCounter )
				//{
				//	numAabbs--;
				//	continue;
				//}
				
				hkReal radius = halfTolerance;
				handle[0] = c->getBroadPhaseHandle();

				c->getShape()->getAabb( c->getTransform(), radius, *aabb );

				aabb++;
				handle++;
			}
		}

		HK_TIMER_SPLIT_LIST("3AxisSweep");

		world->getBroadPhase()->updateAabbs( broadPhaseHandles, aabbs, numAabbs, newPairs, delPairs );

		hkDeallocateStack( broadPhaseHandles );
		hkDeallocateStack( aabbs );
	}

	if ( newPairs.getSize() + delPairs.getSize() > 0)
	{
		HK_TIMER_SPLIT_LIST("RemoveDup");
		hkTypedBroadPhaseDispatcher::removeDuplicates(newPairs, delPairs);

		HK_TIMER_SPLIT_LIST("RemoveAgt");
		world->m_broadPhaseDispatcher->removePairs( static_cast<hkTypedBroadPhaseHandlePair*>(delPairs.begin()), delPairs.getSize() );

		// check the memory limit
		int availableMemory = hkMemory::getInstance().getAvailableMemory();

		// be careful about overflows here
		int memoryEstimatePerPair = 128 /*agent*/ + 192 /* contactMgr */ + 256 /*contact points*/ + 64 /* links */;
		if ( newPairs.getSize() * memoryEstimatePerPair > availableMemory )
		{
			hkMemory::getInstance().m_memoryState = hkMemory::MEMORY_STATE_OUT_OF_MEMORY;
			HK_TIMER_END_LIST();
			return;
		}

		HK_TIMER_SPLIT_LIST("AddAgt");
		world->m_broadPhaseDispatcher->addPairs(    static_cast<hkTypedBroadPhaseHandlePair*>(newPairs.begin()), newPairs.getSize(), world->getCollisionFilter() );
	}
	HK_TIMER_END_LIST();
}

void hkSimulation::collideIslandNarrowPhaseDiscrete( hkSimulationIsland* island, const hkProcessCollisionInput& input)
{
	HK_TIMER_BEGIN( "NarrowPhase", HK_NULL);

	input.m_collisionQualityInfo = input.m_dispatcher->getCollisionQualityInfo( hkCollisionDispatcher::COLLISION_QUALITY_PSI );
	input.m_createPredictiveAgents = false;

	hkAgentNnTrack& agentTrack = island->m_agentTrack;
	hkAgentNnMachine_ProcessTrack( island, agentTrack, input );

	HK_TIMER_END();
}


void hkSimulation::collideInternal( const hkStepInfo& stepInfoIn )
{
	HK_TIME_CODE_BLOCK( "Collide", HK_NULL );

	//
	// Initialize all parameters of the dynamics step into that depend on the stepInfo
	//
	{
		// Step Info
		m_world->m_dynamicsStepInfo.m_stepInfo = stepInfoIn;
		m_world->m_collisionInput->m_stepInfo   = stepInfoIn;

		// Update Solver Info
		hkSolverInfo& solverInfo = m_world->m_dynamicsStepInfo.m_solverInfo;
		solverInfo.m_deltaTime	 = stepInfoIn.m_deltaTime    * solverInfo.m_invNumSteps;
		solverInfo.m_invDeltaTime= stepInfoIn.m_invDeltaTime * solverInfo.m_numSteps;
	}

	// validateWorld();

	//
	//	Broadphase
	//
	{
		// enable delay operations, as we cannot allow for the islands to merge now.
		m_world->lockCriticalOperations();
		const hkArray<hkSimulationIsland*>& activeIslands = m_world->getActiveSimulationIslands();

		for ( int i = 0; i < activeIslands.getSize(); ++i )
		{
			hkSimulationIsland* island = activeIslands[i];
			collideEntitiesBroadPhaseDiscrete( &island->m_entities[0], island->m_entities.getSize(), m_world );

			if (hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OUT_OF_MEMORY )
			{
				m_world->unlockAndAttemptToExecutePendingOperations();
				return;
			}
		}
		m_world->unlockAndAttemptToExecutePendingOperations();
	}

	//
	//	Narrowphase
	//
	{
		m_world->lockCriticalOperations();
		const hkArray<hkSimulationIsland*>& activeIslands = m_world->getActiveSimulationIslands();
		for ( int i = 0; i < activeIslands.getSize(); ++i )
		{
			hkSimulationIsland* island = activeIslands[i];

            // Can remove constraints here but not entities
			collideIslandNarrowPhaseDiscrete( island, *m_world->m_collisionInput );

			if ( hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OUT_OF_MEMORY )
			{
				m_world->unlockAndAttemptToExecutePendingOperations();
				return;
			}

			if ( m_world->m_islandPostCollideListeners.getSize() )
			{
				HK_TIMER_BEGIN("IslandPostCollideCb", HK_NULL);
				hkWorldCallbackUtil::fireIslandPostCollideCallback( m_world, island, stepInfoIn );
				HK_TIMER_END();
			}
		}
		m_world->unlockAndAttemptToExecutePendingOperations();
	}
}

void hkSimulation::integrateIsland( hkSimulationIsland* island, const hkWorldDynamicsStepInfo& stepInfo, hkConstraintQueryIn& constraintQueryIn )
{										
	// ApplyIslandActions is now called explicitly from integrate
	int numInactiveFrames;
	if ( island->m_constraintInfo.m_sizeOfJacobians == 0 )
	{
		HK_TIMER_BEGIN("SingleObj", HK_NULL);
		numInactiveFrames = hkRigidMotionUtilApplyForcesAndStep( stepInfo.m_solverInfo, stepInfo.m_stepInfo, stepInfo.m_solverInfo.m_globalAccelerationPerStep, (hkMotion*const*)island->m_entities.begin(), island->m_entities.getSize(), HK_OFFSET_OF(hkEntity,m_motion) );
		HK_TIMER_END();
	}
	else
	{
		numInactiveFrames = hkConstraintSolverSetup::solve( stepInfo.m_stepInfo, stepInfo.m_solverInfo,
			constraintQueryIn, *island,
			&island->m_entities[0],    island->m_entities.getSize() );
	}
	if ( numInactiveFrames > hkMotion::NUM_INACTIVE_FRAMES_TO_DEACTIVATE )
	{
		if (island->m_active)
		{
			if ( island->m_world->m_wantDeactivation )
			{
				hkWorldOperationUtil::markIslandInactive( island->m_world, island );
			}
		}
	}

}


void hkSimulation::integrateInternal( const hkStepInfo& stepInfoIn )
{
	HK_ASSERT(0xf0ff0061, !m_world->m_pendingOperationsCount);

	HK_TIMER_BEGIN_LIST("Integrate", "Init");

	//
	// Initialize all parameters of the dynamics step into that depend on the stepInfo
	//
	{
		// Step Info
		m_world->m_dynamicsStepInfo.m_stepInfo = stepInfoIn;
		
		// Solver Info
		hkSolverInfo& solverInfo = m_world->m_dynamicsStepInfo.m_solverInfo;
		solverInfo.m_deltaTime	 = stepInfoIn.m_deltaTime    * solverInfo.m_invNumSteps;
		solverInfo.m_invDeltaTime= stepInfoIn.m_invDeltaTime * solverInfo.m_numSteps;

		solverInfo.m_globalAccelerationPerSubStep.setMul4( solverInfo.m_deltaTime, m_world->m_gravity );
		solverInfo.m_globalAccelerationPerStep.setMul4( stepInfoIn.m_deltaTime, m_world->m_gravity );
	}
	
	//
	// Upload solver
	//
	hkSolveUpload();

	//
	// Integrate islands
	//
	{
		// We allow whatever operations here (from activation/deactivation callbacks.
		hkWorldOperationUtil::cleanupDirtyIslands(m_world);

		// Execute all actions. Note, that we only allow operations on phantoms here (i.e. entities/actions/constrainsts are still blocked)
		// TODO consider allowing collisionFilterUpdate too.
		{
			m_world->lockCriticalOperations();
			m_world->unlockCriticalOperationsForPhantoms();

			HK_TIMER_SPLIT_LIST("Actions");
			applyActions( );

			m_world->lockCriticalOperationsForPhantoms();
			m_world->unlockAndAttemptToExecutePendingOperations();
		}


		m_world->lockCriticalOperations();

			// Constraint Query Info In
		hkConstraintQueryIn constraintQueryIn;	constraintQueryIn.set( m_world->m_dynamicsStepInfo.m_solverInfo, stepInfoIn );

		HK_TIMER_SPLIT_LIST("Integrate");
		const hkArray<hkSimulationIsland*>& activeIslands = m_world->getActiveSimulationIslands();
		for (int i = activeIslands.getSize()-1; i>=0; i--)
		{
			hkSimulationIsland* activeIsland = activeIslands[i];
			HK_ASSERT(0x3b3ca726,  activeIsland->m_storageIndex == i );

			integrateIsland( activeIsland, m_world->m_dynamicsStepInfo, constraintQueryIn );

			// 
			//	fire island post integrate listener
			//
			if ( m_world->m_islandPostIntegrateListeners.getSize() )
			{
				HK_TIMER_SPLIT_LIST("PostIntegrateCb");
				hkWorldCallbackUtil::fireIslandPostIntegrateCallback( m_world, activeIsland, stepInfoIn );
			}
		}

		// required as actions may change the m_world; we need to apply mouseAction and other such things
		m_world->unlockAndAttemptToExecutePendingOperations();
	}


	//
	// Fire post integrate callbacks
	//
	if ( m_world->m_worldPostIntegrateListeners.getSize() )
	{
		HK_TIMER_BEGIN("PostIntegrateCb", HK_NULL);
		hkWorldCallbackUtil::firePostIntegrateCallback( m_world, stepInfoIn );
		HK_TIMER_END();
	}


	//
	// End the integrate timer list
	//
	HK_TIMER_END_LIST(); // integrate
}


void hkSimulation::applyActions()
{
	const hkArray<hkSimulationIsland*>& activeIslands = m_world->getActiveSimulationIslands();
	for (int i = 0; i < activeIslands.getSize(); i++)
	{
		HK_ASSERT(0x3b3ca726,  activeIslands[i]->m_storageIndex == i );

		hkArray<hkAction*>& actions = activeIslands[i]->m_actions;
		for (int j = 0; j < actions.getSize(); j++)
		{
			actions[j]->applyAction( m_world->m_dynamicsStepInfo.m_stepInfo );
		}
	}
}

void hkSimulation::collideEntitiesDiscrete( hkEntity** entities, int numEntities, hkWorld* world, const hkStepInfo& stepInfo, FindContacts findExtraContacts )
{
	HK_ASSERT2(0xad45ee3b, numEntities, "Must pass at least one hkEntity when callling hkSimulation::collideEntitiesDiscrete.");

	hkStepInfo originalStepInfo = stepInfo;
	hkProcessCollisionInput& input = *world->getCollisionInput();
	input.m_stepInfo = stepInfo;

	world->lockCriticalOperations();
	{
		collideEntitiesBroadPhaseDiscrete(entities, numEntities, world);

		//if (hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OUT_OF_MEMORY )
		//{
		//	world->unlockAndAttemptToExecutePendingOperations();
		//	return;
		//}

		collideEntitiesNarrowPhaseDiscrete(entities, numEntities, input, findExtraContacts);
	}
	world->unlockAndAttemptToExecutePendingOperations();

	input.m_stepInfo = originalStepInfo;
}




void hkSimulation::collideEntitiesNarrowPhaseDiscrete( hkEntity** entities, int numEntities, const hkProcessCollisionInput& input, FindContacts findExtraContacts )
{
	HK_ASSERT2(0xadfe825d, numEntities, "Must call the function with a non-zero number of entities.");
	HK_ASSERT2(0xadfe825d, entities[0]->getWorld()->areCriticalOperationsLocked(), "The hkWorld must be locked when calling hkSimulation::collideEntitiesNarrowPhaseDiscrete.");

	processAgentsOfEntities(entities, numEntities, input, &hkSimulation::processAgentCollideDiscrete, findExtraContacts);
}

void hkSimulation::processAgentCollideDiscrete(hkAgentNnEntry* entry, const hkProcessCollisionInput& processInput, hkProcessCollisionOutput& processOutput)
{
	processOutput.reset();
	processInput.m_collisionQualityInfo   = processInput.m_dispatcher->getCollisionQualityInfo( processInput.m_dispatcher->COLLISION_QUALITY_PSI );
	processInput.m_createPredictiveAgents = processInput.m_dispatcher->getCollisionQualityInfo( entry->m_collisionQualityIndex                   )->m_useContinuousPhysics;

	hkAgentNnMachine_ProcessAgent( entry, processInput, processOutput );

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
		hkCollidable& collA = *entry->getCollidableA();
		hkCollidable& collB = *entry->getCollidableB();
		entry->m_contactMgr->processContact( collA, collB, processInput, processOutput );
	} 
}


void hkSimulation::resetCollisionInformationForEntities( hkEntity** entities, int numEntities, hkWorld* world, hkBool skipReinitializationOfAgents )
{
	HK_ASSERT2(0XAD4545DD, world->areCriticalOperationsLocked(), "This is an internal function. It requires the world to be locked.");

	if (!skipReinitializationOfAgents)
	{
		// world->getCollisionInput() is not used in hkSimulation::processAgentResetCollisionInformation.
		processAgentsOfEntities( entities, numEntities, *world->getCollisionInput(), &hkSimulation::processAgentResetCollisionInformation, FIND_CONTACTS_DEFAULT);
	}
}

	//	void processAgentClearManifoldsAndTimsAndToisOfEntities( hkEntity** entities, int numEntities, hkWorld* world );
void hkSimulation::processAgentResetCollisionInformation(hkAgentNnEntry* entry, const hkProcessCollisionInput& processInput, hkProcessCollisionOutput& processOutput)
{
	// Invalidate Tims
	hkAgentNnMachine_InvalidateTimInAgent(entry, const_cast<hkProcessCollisionInput&>(processInput));

	// Clear manifolds
//	hkInplaceArray<hkContactPointId, 10> contactPointIds;
//	hkDynamicsContactMgr* mgr = static_cast<hkDynamicsContactMgr*>(entry->m_contactMgr);
//	mgr->getAllContactPointIds(contactPointIds);
//	for (int i = 0; i < contactPointIds.getSize(); i++)
//	{
//		// We clear the manifold by invalidating all present contact points by setting their distance to 
//		// infinity.
//		// This solution does not guarantee determinism though, as the internal agent caches are not reset.
//		mgr->getContactPoint()->setDistance(HK_REAL_MAX);
//		// crashes:mgr->removeContactPoint(contactPointIds[i]);
//	}

	// Info: to guarantee determinism, agents must be recreated.
}


// NOTE: This function does not alter the swept transform; it only modifies the transform of the rigid body
// as that is what is used in collision detection
static HK_LOCAL_INLINE void setRotationAroundCentreOfMass( hkRigidBody* rb, hkQuaternion& newRotation )
{
	hkTransform& trans = rb->getRigidMotion()->getMotionState()->getTransform();
	trans.setRotation(newRotation);
	hkVector4 centerShift;
	centerShift._setRotatedDir( trans.getRotation(), rb->getRigidMotion()->getCenterOfMassLocal() );
	trans.getTranslation().setSub4( rb->getRigidMotion()->getCenterOfMassInWorld(), centerShift );
}

void hkSimulation::processAgentsOfEntities( hkEntity** entities, int numEntities, const hkProcessCollisionInput& processInput, AgentEntryProcessFunction processingFunction, FindContacts findExtraContacts)
{
	// Have a set of entities which have been processed. To avoid processing agents twice, we don't process agents
	// that link the entity currently being processed to another entity, which already has been processed.
	HK_COMPILE_TIME_ASSERT( sizeof(hkUlong) == sizeof(hkLinkedCollidable*) );

	hkPointerMapBase<hkUlong> processedEntities;
	processedEntities.reserve(numEntities);

	//const hkProcessCollisionInput& processInput = *world->getCollisionInput();
	HK_ALIGN16(typedef hkProcessCollisionOutput hkAlignedCollisionOutput);
	hkAlignedCollisionOutput processOutput(HK_NULL);

	// Process each entity from the 'entities' list.
	for (int i = 0; i < numEntities; i++)
	{
		hkEntity* entity = entities[i];
		HK_ASSERT2(0xade278fd, entity->getWorld() == entities[0]->getWorld(), "All entities must belong to the same hkWorld");

		// <todo> maybe auto-activate each inactive found island
		//HK_ASSERT2(0xadf256fe, entity->getSimulationIsland()->m_isInActiveIslandsArray, "The entity is _inactive_ and some of it's agents might be not present");

		hkLinkedCollidable* lColl = entity->getLinkedCollidable();

		// Check that every entity is only passed once
		HK_ON_DEBUG( hkPointerMapBase<hkUlong>::Iterator collIt = processedEntities.findKey(hkUlong(lColl)); )
		HK_ASSERT2(0xad45db3, !processedEntities.isValid(collIt), "Same entity passed more than once to hkSimulation::processAgentsOfEntities.");

		processedEntities.insert(hkUlong(lColl), 0);
		

		for (int c = 0; c < lColl->m_collisionEntries.getSize(); c++)
		{
			hkLinkedCollidable::CollisionEntry& cEntry = lColl->m_collisionEntries[c];

			hkLinkedCollidable* partner = cEntry.m_partner;
			HK_ASSERT2( 0xf0321245, hkGetRigidBody(cEntry.m_partner), "Internal error, entity expected, something else found");

			hkPointerMapBase<hkUlong>::Iterator it = processedEntities.findKey(hkUlong(partner));
			if (processedEntities.isValid(it))
			{
				continue;
			}

			{
				// Process agents not linking to processed entities.
				hkAgentNnEntry* entry = cEntry.m_agentEntry;
				{
					hkEntity* entityA = static_cast<hkEntity*>(entry->getCollidableA()->getOwner());
					hkEntity* entityB = static_cast<hkEntity*>(entry->getCollidableB()->getOwner());
					hkSimulationIsland* island = (entityA->isFixed() )? entityB->getSimulationIsland(): entityA->getSimulationIsland();
					processOutput.m_constraintOwner = island;
				}

				hkAgentNnMachine_InvalidateTimInAgent(entry, const_cast<hkProcessCollisionInput&>(processInput));
				(this->*processingFunction)(entry, processInput, processOutput);

				if (	(findExtraContacts == FIND_CONTACTS_EXTRA) && 
						(processOutput.m_firstFreeContactPoint != processOutput.m_contactPoints ) )
				{
					hkRigidBody* rb = (hkRigidBody*)entity;
					hkQuaternion origRot = rb->getRigidMotion()->getRotation();
					hkTransform origTrans = rb->getTransform();

					// Keep this as the "master" normal
					hkVector4 normal = processOutput.m_contactPoints[0].m_contact.getSeparatingNormal();

					if (cEntry.m_agentEntry->getCollidableA() != entity->getCollidable())
					{
						normal.setNeg4(normal);
					}

					// Calculate the axis of rotation based on the contact normal and the line from the contact point to the centre of mass
					hkVector4 diff; diff.setSub4(rb->getCenterOfMassInWorld(), processOutput.m_contactPoints[0].m_contact.getPosition());
					hkVector4 rotateDir; rotateDir.setCross(normal, diff);
					hkReal len = rotateDir.length3();

					hkReal l = normal.dot3(diff);
					hkReal d = diff.length3();
					hkReal rotateSinFrom = l / d;

					// Only rotate if the contact point is not below the centre of mass
					if ( (len > .00001f) && (rotateSinFrom < .9999f) )
					{
						const hkReal toleranceMult = 20.f;

						// Increase tolerances to 10 times the normal, and save old versions for reset
						hkReal savedTolerance = processInput.m_tolerance;
						hkReal searchTolerance = processInput.m_tolerance * toleranceMult;
						((hkProcessCollisionInput&)processInput).m_tolerance = searchTolerance;
						hkReal savedCreate4dContact = processInput.m_collisionQualityInfo->m_create4dContact;
						hkReal savedCreateContact = processInput.m_collisionQualityInfo->m_createContact;
						((hkProcessCollisionInput&)processInput).m_collisionQualityInfo->m_create4dContact = searchTolerance;
						((hkProcessCollisionInput&)processInput).m_collisionQualityInfo->m_createContact = searchTolerance;


						// Calculate angle for rotation = asin(l/d) - asin( (l-x) / d) where x is the max distance we want the closest point to separate by
						hkReal rotateSinTo = hkMath::max2((l - searchTolerance * (1.f / toleranceMult) ) / d, 0.f);
						hkReal rotateAngle = hkMath::asin( rotateSinFrom ) - hkMath::asin( rotateSinTo );

						// normalize the rotation direction
						rotateDir.mul4(1.f / len );

						// Rotate body by a small rotation to pick up the point at the opposite side to the closest point
						{
							hkQuaternion quat( rotateDir, rotateAngle );
							hkQuaternion rbRot;
							rbRot.setMul(quat, rb->getRigidMotion()->getRotation());

							setRotationAroundCentreOfMass( rb, rbRot );

							hkAgentNnMachine_InvalidateTimInAgent(entry, const_cast<hkProcessCollisionInput&>(processInput));

							(this->*processingFunction)(entry, processInput, processOutput);
						}

						// Rotate the body by a small rotation to pick up the points at 90 degree extremities to the "primary" axis
						{
							hkVector4 rotateCrossDir; rotateCrossDir.setCross( rotateDir, normal );
							rotateCrossDir.normalize3();

							hkQuaternion quat( rotateCrossDir, rotateAngle );
							hkQuaternion rbRot; rbRot.setMul(quat, origRot);
							setRotationAroundCentreOfMass( rb, rbRot );

							hkAgentNnMachine_InvalidateTimInAgent(entry, const_cast<hkProcessCollisionInput&>(processInput));

							(this->*processingFunction)(entry, processInput, processOutput);


							rbRot.setInverseMul(quat, origRot);
							setRotationAroundCentreOfMass( rb, rbRot );

							hkAgentNnMachine_InvalidateTimInAgent(entry, const_cast<hkProcessCollisionInput&>(processInput));

							(this->*processingFunction)(entry, processInput, processOutput);
						}

						// Reset the body and update the manifold
						{
							// Reset the tolerances to their original values
							((hkProcessCollisionInput&)processInput).m_tolerance = savedTolerance;
							((hkProcessCollisionInput&)processInput).m_collisionQualityInfo->m_create4dContact = savedCreate4dContact;
							((hkProcessCollisionInput&)processInput).m_collisionQualityInfo->m_createContact = savedCreateContact;

							rb->getRigidMotion()->setTransform(origTrans);
							hkAgentNnMachine_InvalidateTimInAgent(entry, const_cast<hkProcessCollisionInput&>(processInput));

							// Ideally we just want to update manifold here, not re-collide
							(this->*processingFunction)(entry, processInput, processOutput);

						}
					}
				}
				if (hkMemory::getInstance().m_memoryState == hkMemory::MEMORY_STATE_OUT_OF_MEMORY )
	            {
		            return;
	            }

			}
		}
	}
}

void hkSimulation::reintegrateAndRecollideEntities( hkEntity** entityBatch, int numEntities, hkWorld* world )
{
	// Check if operation may be performed now
	if (world->areCriticalOperationsLocked())
	{
		hkWorldOperation::ReintegrateAndRecollideEntityBatch op;
		op.m_entities = const_cast<hkEntity**>(entityBatch);
		op.m_numEntities = hkObjectIndex(numEntities);
		world->queueOperation( op );
		// <todo> consider returning done/postponed status value
		return;
	}

	world->lockCriticalOperations();

	// Prepare the proper stepInfo. 
	const hkStepInfo physicsStepInfo( world->getCurrentTime(), world->getCurrentPsiTime() );
	world->m_collisionInput->m_stepInfo = physicsStepInfo;


	// Re-integrate selected bodies.
	{
		for (int i = 0; i < numEntities; i++)
		{
			hkRigidBody* body = static_cast<hkRigidBody*>(entityBatch[i]);
			hkMotionState& ms = *body->getRigidMotion()->getMotionState();

			hkSweptTransformUtil::backStepMotionState(world->getCurrentTime(), ms);
		}
		hkRigidMotionUtilStep(physicsStepInfo, (hkMotion*const*)entityBatch, numEntities, HK_OFFSET_OF(hkEntity, m_motion));
	}

	// Re-collide selected bodies.
	collideEntitiesBroadPhaseDiscrete(entityBatch, numEntities, world);

	// Reintegrate TIMs for all agents involved
	for (int i = 0; i < numEntities; i++)
	{
		hkWorldAgentUtil::invalidateTim(entityBatch[i], *world->m_collisionInput);
	}

	collideEntitiesNarrowPhaseDiscrete(entityBatch, numEntities, *world->getCollisionInput(), FIND_CONTACTS_DEFAULT);

	world->unlockAndAttemptToExecutePendingOperations();
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
