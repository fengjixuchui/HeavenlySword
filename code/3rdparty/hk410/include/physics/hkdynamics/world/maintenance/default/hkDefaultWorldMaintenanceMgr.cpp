/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkdynamics/hkDynamics.h>
#include <hkbase/monitor/hkMonitorStream.h>


#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkcollide/agent/hkProcessCollisionInput.h>

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>
#include <hkdynamics/world/maintenance/default/hkDefaultWorldMaintenanceMgr.h>
#include <hkinternal/dynamics/world/simulation/continuous/hkContinuousSimulation.h>

#include <hkdynamics/entity/hkRigidBody.h>


hkDefaultWorldMaintenanceMgr::hkDefaultWorldMaintenanceMgr()
{
	m_minAllowedTimeValue = 32.0f + 1.0f;
	m_maxAllowedTimeValue = 64.0f - 1.0f;
}

void hkDefaultWorldMaintenanceMgr::init( hkWorld* world )
{
	world->m_simulation->setCurrentTime( hkTime(m_minAllowedTimeValue) );
	world->m_simulation->setCurrentPsiTime( hkTime(m_minAllowedTimeValue) );
}



void hkDefaultWorldMaintenanceMgr::resetWorldTime( hkWorld* world, hkStepInfo& stepInfo)
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );

	//
	// Reset time information for the entire world
	//
	hkReal stepDeltaTime  = stepInfo.m_endTime  - stepInfo.m_startTime;

	// reset stepInfo
	hkStepInfo& newStepInfo = world->m_dynamicsStepInfo.m_stepInfo;

	HK_ASSERT(0, newStepInfo.m_startTime == world->m_simulation->getCurrentPsiTime());

	{
		newStepInfo.m_startTime = hkTime(m_minAllowedTimeValue);
		newStepInfo.m_endTime   = hkTime(m_minAllowedTimeValue + stepDeltaTime);
		newStepInfo.m_deltaTime = stepDeltaTime;
		newStepInfo.m_invDeltaTime = 1.0f / stepDeltaTime;
	}

	hkReal warpDeltaTime = newStepInfo.m_startTime - stepInfo.m_startTime;

		// reset time variables in hkWorld
	{
		world->m_simulation->setCurrentTime( world->m_simulation->getCurrentTime() + warpDeltaTime );
		world->m_simulation->setCurrentPsiTime( newStepInfo.m_startTime );
	}

	if (world->m_simulation->getSimulateUntilTime() != -1)
	{
		world->m_simulation->setSimulateUntilTime( world->m_simulation->getSimulateUntilTime() + warpDeltaTime );
	}
		

		// reset time in all swept transforms and agents
	const hkArray<hkSimulationIsland*>& islands = world->getActiveSimulationIslands();
	{
		for (int i = 0; i < islands.getSize(); i++)
		{
			hkSimulationIsland* island = islands[i];

			for (int e = 0; e < island->m_entities.getSize(); e++)
			{
				hkRigidBody* body = static_cast<hkRigidBody*>(island->m_entities[e]);
				hkMotionState* ms = body->getRigidMotion()->getMotionState();
				ms->getSweptTransform().m_centerOfMass0(3) += warpDeltaTime;
			}

			// reset time in all agents
			hkWorldAgentUtil::warpTime(island, stepInfo.m_endTime, newStepInfo.m_endTime, *world->m_collisionInput);
		}
	}

	//
	// Call hkSimluation::warpTime() to update whatever variables needed.
	//
	world->m_simulation->warpTime( warpDeltaTime );


	stepInfo = newStepInfo;
	world->m_collisionInput->m_stepInfo = newStepInfo;
}

	// checks deactivators and sets m_active (status_to_be) status for those islands
void hkDefaultWorldMaintenanceMgr::markIslandsForDeactivationDeprecated( hkWorld* world, hkStepInfo& stepInfo)
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );

	if ( world->m_minDesiredIslandSize != 0 )
	{
		HK_WARN( 0xf0323454, "Requesting old style deactivation, this will also disable the world->m_minDesiredIslandSize optimization."
			" As a result the engine will run slower in multithreaded mode if the physics scene contains lots of small unconnected objects");
	}
	world->m_minDesiredIslandSize = 0;

	const hkArray<hkSimulationIsland*>& islands = world->getActiveSimulationIslands();

	{
		for (int i = islands.getSize()-1; i>=0; i--)
		{
			hkSimulationIsland* activeIsland = islands[i];
			HK_ASSERT(0x3b3ca726,  activeIsland->m_storageIndex == i );
			if ( activeIsland->shouldDeactivateDeprecated( stepInfo ) )
			{
				// the island has requested deactivation
				hkWorldOperationUtil::markIslandInactive(world, activeIsland);
			}
		}
	}
}

void hkDefaultWorldMaintenanceMgr::performMaintenance( hkWorld* world, hkStepInfo& stepInfo )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );

	HK_TIMER_BEGIN_LIST("Maintenance", "Split");

	hkWorldOperationUtil::splitSimulationIslands(world);

	if (stepInfo.m_startTime >= m_maxAllowedTimeValue)
	{
		HK_TIMER_SPLIT_LIST("ResetTime");
		resetWorldTime(world, stepInfo);
	}

	if (world->m_wantOldStyleDeactivation)
	{
		HK_TIMER_SPLIT_LIST("CheckDeactOld");
		markIslandsForDeactivationDeprecated(world, stepInfo);
	}

	HK_TIMER_END_LIST();
}

void hkDefaultWorldMaintenanceMgr::performMaintenanceNoSplit( hkWorld* world, hkStepInfo& stepInfo )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );

	HK_TIMER_BEGIN_LIST("Maintenance", "ResetTime");

	if (stepInfo.m_startTime >= m_maxAllowedTimeValue)
	{
		resetWorldTime(world, stepInfo);
	}

	if (world->m_wantOldStyleDeactivation)
	{
		HK_TIMER_SPLIT_LIST("CheckDeact");
		markIslandsForDeactivationDeprecated(world, stepInfo);
	}

	HK_TIMER_END_LIST();
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
