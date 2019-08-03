/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkbase/htl/hkAlgorithm.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>

#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkcollide/dispatch/contactmgr/hkContactMgrFactory.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>
#include <hkcollide/agent/hkCollisionAgent.h>
#include <hkcollide/agent/hkContactMgr.h>
#include <hkinternal/collide/agent3/machine/nn/hkAgentNnTrack.h>
#include <hkinternal/collide/agent3/machine/nn/hkAgentNnMachine.h>
#include <hkinternal/collide/agent3/machine/1n/hkAgent1nTrack.h>
#include <hkinternal/collide/agent3/machine/1n/hkAgent1nMachine.h>

#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkSimulationIsland.h>

#include <hkdynamics/world/util/hkWorldOperationQueue.h>


// If both pointers are the same & point to a fixed island -- return it.
static HK_FORCE_INLINE hkSimulationIsland* getAnyNonFixedIsland( hkSimulationIsland* islandA, hkSimulationIsland* islandB)
{
	if (!islandA->isFixed())
	{
		return islandA;
	}
	if (!islandB->isFixed())
	{
		return islandB;
	}
	HK_ASSERT2(0x48af8302, islandA == islandB, "Internal error: two different fixed islands.");
	return islandA;
}


hkAgentNnEntry* hkWorldAgentUtil::addAgent( hkLinkedCollidable* collA, hkLinkedCollidable* collB, const hkProcessCollisionInput& input )
{
	HK_ASSERT2(0Xad000710, !hkAgentNnMachine_FindAgent(collA, collB), "An agent already exists between the two collidables specified.");

	hkEntity* entityA = static_cast<hkEntity*>( collA->getOwner() );
	hkEntity* entityB = static_cast<hkEntity*>( collB->getOwner() );

	// Request island merge
	hkWorldOperationUtil::mergeIslandsIfNeeded( entityA, entityB );

	//   Choose the island to add new agent to 
	//   merge might have been delayed
	hkSimulationIsland* theIsland = getAnyNonFixedIsland(entityA->getSimulationIsland(), entityB->getSimulationIsland());
	HK_ACCESS_CHECK_WITH_PARENT( theIsland->m_world, HK_ACCESS_IGNORE, theIsland, HK_ACCESS_RW );


	//
	//	Get the agent type and flip information
	//
	int agentType;
	int isFlipped;
	hkAgentNnMachine_GetAgentType( collA, collB, input, agentType, isFlipped );
	if ( isFlipped )
	{
		hkAlgorithm::swap( collA, collB );
	}

	//
	// Attempt to create the mgr
	//
	hkContactMgr* mgr;
	{
		hkContactMgrFactory* factory = input.m_dispatcher->getContactMgrFactory( entityA->getMaterial().getResponseType(), entityB->getMaterial().getResponseType() );
		mgr = factory->createContactMgr( *collA, *collB, input );
	}

	//
	//	Create the final agent
	//
	hkAgentNnTrack& track = theIsland->m_agentTrack;
	hkAgentNnEntry* newAgent = hkAgentNnMachine_CreateAgent( track, collA, collB, agentType, input, mgr );



#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	theIsland->isValid();
#	endif


	return newAgent;



//	// suspend agent
//	if (createSuspended)
//	{
//		// info: if entityA and entityB belong to to active/inactive islands, than whether the agent should/shouln't be created
//		//       only depends on which island we initially assign it to.
//		hkWorldAgentUtil::suspendAgent(pair);
//	}
//
//	return pair;


}

void hkWorldAgentUtil::removeAgent( hkAgentNnEntry* agent )
{
	HK_ON_DEBUG( hkSimulation* simulation = static_cast<hkEntity*>( agent->m_collidable[0]->getOwner() )->getSimulationIsland()->getWorld()->m_simulation );
	HK_ON_DEBUG( simulation->assertThereIsNoCollisionInformationForAgent(agent) );

	// Remove hkCollisionPair / agent from hkSimulationIsland
	hkSimulationIsland* theIsland;
	hkEntity* entityA = static_cast<hkEntity*>( agent->m_collidable[0]->getOwner() );
	hkEntity* entityB = static_cast<hkEntity*>( agent->m_collidable[1]->getOwner() );
	hkSimulationIsland* islandA = entityA->getSimulationIsland();
	hkSimulationIsland* islandB = entityB->getSimulationIsland();

	if (islandA == islandB)
	{
		theIsland = islandA;
		theIsland->m_splitCheckRequested = true;
	}
	else if (entityA->isFixed())
	{
		// don't check whether the island is fixed, cause you'll get a cache miss on the fixed island :-/
		theIsland = islandB;
	}
	else if (entityB->isFixed())
	{
		theIsland = islandA;
	}
	else
	{
		// This should happen only when you add and remove an agent between entities moving one after another (and belonging to two different islands)
		// in a way that their aabbs overlap in-between collision detection run for each of the islands.

		theIsland = getIslandFromAgentEntry(agent, islandA, islandB);

		// we have those, because we may still have a merge request for those entities in the pendingOperation queue
		//  and this is faster than going through the pendingOperations list. And we are too lazy.
		entityA->getSimulationIsland()->m_splitCheckRequested = true;
		entityB->getSimulationIsland()->m_splitCheckRequested = true;
	}
	HK_ACCESS_CHECK_WITH_PARENT( theIsland->m_world, HK_ACCESS_IGNORE, theIsland, HK_ACCESS_RW );


	hkAgentNnTrack& track = theIsland->m_agentTrack;
	hkCollisionDispatcher* dispatch = theIsland->getWorld()->getCollisionDispatcher();

	hkContactMgr* mgr = agent->m_contactMgr;
	hkAgentNnMachine_DestroyAgent( track, agent, dispatch, *theIsland );
	mgr->cleanup();

#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	theIsland->isValid();
#	endif

	//HK_INTERNAL_TIMER_END_LIST();

}

void hkWorldAgentUtil::removeAgentAndItsToiEvents ( hkAgentNnEntry* agent )
{
	hkSimulation* simulation = static_cast<hkEntity*>( agent->m_collidable[0]->getOwner() )->getSimulationIsland()->getWorld()->m_simulation;
	simulation->removeCollisionInformationForAgent( agent );

	hkWorldAgentUtil::removeAgent( agent );
}

hkSimulationIsland* hkWorldAgentUtil::getIslandFromAgentEntry( hkAgentNnEntry* entry, hkSimulationIsland* candidateA, hkSimulationIsland* candidateB)
{
	// just iterate over sectors of the shorter track
	hkBool searchIsleA = candidateA->m_agentTrack.m_sectors.getSize() <= candidateB->m_agentTrack.m_sectors.getSize();
	hkSimulationIsland* isleToSearch = searchIsleA ? candidateA : candidateB;

	hkBool sectorFound = false;
	hkArray<hkAgentNnSector*>& sectors = isleToSearch->m_agentTrack.m_sectors;
	for (int i = 0; i < sectors.getSize(); i++)
	{
		hkAgentNnSector* sector = sectors[i];
		if (sector->getBegin() <= entry && entry < sector->getEnd(isleToSearch->m_agentTrack) )
		{
			sectorFound = true;
			break;
		}
	}

	// if the agent is not there, then it's in the other track -- just remove it with hkAgentNnMachine_
	return (searchIsleA ^ sectorFound) ? candidateB : candidateA;
}



HK_FORCE_INLINE static hkAgentData* getAgentData( hkAgentNnEntry* entry)
{
	hkAgent3::StreamCommand command = hkAgent3::StreamCommand(entry->m_streamCommand);
	if ( command == hkAgent3::STREAM_CALL_WITH_TIM)
	{
		return hkAddByteOffset<hkAgentData>( entry, hkSizeOf( hkAgentNnMachineTimEntry ) );
	}
	else
	{
		return hkAddByteOffset<hkAgentData>( entry, hkSizeOf( hkAgentNnMachinePaddedEntry ) );
	}
}

void hkWorldAgentUtil::updateEntityShapeCollectionFilter( hkEntity* entity, hkCollisionInput& collisionInput )
{
	HK_ACCESS_CHECK_OBJECT( entity->getWorld(), HK_ACCESS_RW );
	hkLinkedCollidable* collidable = entity->getLinkedCollidable();
	for (int i = 0; i < collidable->m_collisionEntries.getSize(); i++)
	{
		hkAgentNnEntry* entry = collidable->m_collisionEntries[i].m_agentEntry;

		hkAgent3::UpdateFilterFunc func = collisionInput.m_dispatcher->getAgent3UpdateFilterFunc(entry->m_agentType);
		if (func)
		{
				// this cast is allowed, as the nn-machine only works between entities
			hkEntity* entityA = static_cast<hkEntity*>(entry->getCollidableA()->getOwner());
			hkEntity* entityB = static_cast<hkEntity*>(entry->getCollidableB()->getOwner());
			hkSimulationIsland* island = (entityA->isFixed() )? entityB->getSimulationIsland(): entityA->getSimulationIsland();

			hkAgentData* agentData = getAgentData(entry);
			func(entry, agentData, *entry->getCollidableA(), *entry->getCollidableB(), collisionInput, *island);
		}
	}
}

void hkWorldAgentUtil::invalidateTim( hkEntity* entity, hkCollisionInput& collisionInput )
{
	hkLinkedCollidable* collidable = entity->getLinkedCollidable();
	for (int i = 0; i < collidable->m_collisionEntries.getSize(); i++)
	{
		hkAgentNnEntry* entry = collidable->m_collisionEntries[i].m_agentEntry;
		hkAgentNnMachine_InvalidateTimInAgent( entry, collisionInput );
	}
}

void hkWorldAgentUtil::warpTime( hkSimulationIsland* island, hkTime oldTime, hkTime newTime, hkCollisionInput& collisionInput )
{
	HK_ACCESS_CHECK_WITH_PARENT( island->m_world, HK_ACCESS_RO, island, HK_ACCESS_RW );
	HK_FOR_ALL_AGENT_ENTRIES_BEGIN(island->m_agentTrack, entry)
	{
		hkAgentNnMachine_WarpTimeInAgent(entry, oldTime, newTime, collisionInput );
	}
	HK_FOR_ALL_AGENT_ENTRIES_END;
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
