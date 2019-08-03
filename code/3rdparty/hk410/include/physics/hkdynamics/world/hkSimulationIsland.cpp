/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkdynamics/hkDynamics.h>


#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>
#include <hkbase/sort/hkUnionFind.h>

#include <hkmath/basetypes/hkAabb.h>
#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkinternal/collide/broadphase/hkBroadPhase.h>
#include <hkinternal/collide/broadphase/hkBroadPhaseHandle.h>
#include <hkcollide/shape/hkShape.h>
#include <hkcollide/agent/hkProcessCollisionOutput.h>

#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseDispatcher.h>
#include <hkcollide/dispatch/contactmgr/hkContactMgrFactory.h>
#include <hkcollide/agent/hkCollisionAgent.h>
#include <hkcollide/agent/hkProcessCollisionOutput.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>

#include <hkcollide/filter/hkCollisionFilter.h>

#include <hkinternal/collide/agent3/machine/nn/hkAgentNnMachine.h>
#include <hkinternal/collide/agent3/machine/hkAgentMachineUtil.h>

#include <hkdynamics/collide/hkDynamicsContactMgr.h>

#include <hkdynamics/motion/hkMotion.h>
#include <hkdynamics/constraint/setup/hkConstraintSolverSetup.h>

#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>
#include <hkdynamics/entity/hkEntityDeactivator.h>

#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/hkWorld.h>

#include <hkdynamics/world/util/hkWorldOperationQueue.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>


// used for backstepping
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/action/hkAction.h>
#include <hkdynamics/world/util/hkNullAction.h>

#include <hkdynamics/world/util/hkWorldAgentUtil.h>
#if HK_POINTER_SIZE==4
HK_COMPILE_TIME_ASSERT( sizeof( hkSimulationIsland ) <= 180 );
#endif


hkSimulationIsland::hkSimulationIsland( hkWorld* world ) 
:	m_world( world ),
	m_dirtyListIndex(HK_INVALID_OBJECT_INDEX),
	m_highFrequencyDeactivationCounter(0),
	m_lowFrequencyDeactivationCounter(0),
	m_splitCheckRequested(false),
	m_actionListCleanupNeeded (false),
	m_isInActiveIslandsArray(true),
	m_active(true),
	m_inIntegrateJob(false),
	m_timeSinceLastHighFrequencyCheck(0),
	m_timeSinceLastLowFrequencyCheck(0),
	m_agentTrack(world->m_collisionDispatcher->m_agent3AgentSize, world->m_collisionDispatcher->m_agent3SectorSize),
	m_timeOfDeactivation(-10.0f) // !! MUST BE DIFFERENT THAN DEFAULT TIME OF SEPARATING NORMAL OF AGENTS
{
	m_allowIslandLocking = false;
	m_constraintInfo.clear();
	m_numConstraints = 0;
	m_sparseEnabled = false;
#ifdef HK_PLATFORM_HAS_SPU
	// make sure we allocate at least 16 bytes to force the memory to be aligned on a 16 byte boundard.
	m_entities.reserve(4);
#endif
}

hkSimulationIsland::~hkSimulationIsland()
{
	HK_ASSERT2(0, m_dirtyListIndex == HK_INVALID_OBJECT_INDEX, "Island was not properly removed from the hkWorld::m_dirtySimulationIsland list.");
	for (int i = 0; i < m_actions.getSize(); i++)
	{
		HK_ASSERT2(0xf0ff0093, m_actions[i] == hkNullAction::getNullAction(), "Internal error: actions present in a simulation island upon its destruction.");	
	}
	HK_ASSERT(0xf0ff0023, m_numConstraints == 0 );
	HK_ASSERT2(0xf0ff0094, !m_entities.getSize(), "Internal error: entities present in a simulation island upon its destruction.");
	HK_ASSERT2(0xf0ff0095, !m_agentTrack.m_sectors.getSize(), "Internal error: agents present in a simulation island upon its destruction.");
}

void hkSimulationIsland::internalAddEntity(hkEntity* entity)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RW );
	HK_ASSERT2(0x6cf66cf2,  entity->getSimulationIsland() == HK_NULL, "addEntity - entity already added to an island" );

	entity->m_simulationIsland = this;
	entity->m_storageIndex = (hkObjectIndex)m_entities.getSize();
	m_entities.pushBack(entity);
}

void hkSimulationIsland::internalRemoveEntity(hkEntity* entity)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RW );
	HK_ASSERT2(0x42cea5f3,  entity->getSimulationIsland() == this, "removeEntity - entity not added to this island" );

	// remove the entity
	{
		HK_ASSERT2(0x74438d73,  m_entities.indexOf( entity ) == entity->m_storageIndex, "Internal error" );

		m_entities[entity->m_storageIndex] = m_entities[m_entities.getSize() - 1];
		m_entities[entity->m_storageIndex]->m_storageIndex = entity->m_storageIndex;
		m_entities.popBack();
	}

	entity->m_simulationIsland = HK_NULL;
	entity->m_storageIndex = HK_INVALID_OBJECT_INDEX;

	m_splitCheckRequested = true;
}



void hkSimulationIsland::calcStatistics( hkStatisticsCollector* collector ) const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );
	collector->beginObject( HK_NULL, collector->MEMORY_RUNTIME,  this );
	
	//
	// entities
	//
	{
		collector->addArray( "EntityPtrs", collector->MEMORY_RUNTIME, m_entities );
		for ( int i = 0; i < m_entities.getSize(); ++i )
		{
			hkEntity* entity = m_entities[i];
			collector->addChildObject( "Entity", collector->MEMORY_INSTANCE, entity);

			//
			// constraints details
			//
			{
				collector->pushDir( "Constraints" );
				collector->addArray("ConMstPtr", collector->MEMORY_RUNTIME, entity->m_constraintsMaster);
				collector->addArray("ConSlvPtr", collector->MEMORY_RUNTIME, entity->m_constraintsSlave);
				for ( int j = 0; j < entity->m_constraintsMaster.getSize(); ++j )
				{
					collector->addChildObject( "ConInstance",	 collector->MEMORY_INSTANCE, entity->m_constraintsMaster[j].m_constraint );
					collector->addChildObject( "ConData", collector->MEMORY_SHARED, entity->m_constraintsMaster[j].m_constraint->getData() );
				}
				collector->addArray( "Runtime",	 collector->MEMORY_ENGINE, entity->m_constraintRuntime );
				collector->popDir(  );
			}
		}
	}

	
	//
	// actions
	//
	{
		collector->addArray( "ActionPtrs", collector->MEMORY_RUNTIME, m_actions );
		for ( int i = 0; i < m_actions.getSize(); ++i )
		{
			collector->addChildObject( "Actions", collector->MEMORY_INSTANCE, m_actions[i] );
		}
	}

	//
	// agents
	//
	hkAgentMachineUtil::calcNnStatisticsContactMgrsOnly(m_agentTrack, collector);

	collector->pushDir( "CollAgents" );
		hkAgentMachineUtil::calcNnStatistics(m_agentTrack, collector);
	collector->popDir();

	collector->endObject();
}





static inline hkReal getClosestDist( hkProcessCollisionOutput& result )
{
	hkReal closestDist = HK_REAL_MAX;

	for (int i = 0; i < result.getNumContactPoints(); ++i )
	{
		if ( result.m_contactPoints[i].m_contact.getDistance() < closestDist )
		{
			closestDist = result.m_contactPoints[i].m_contact.getDistance();
		}
	}

	return closestDist;

}



//
// Very simple backstepping - we backstep each entity as it collides. We do not recurse, or revisit each pair.
// Hence an object hitting a wall can hit objects the far side of the wall...
//


void hkSimulationIsland::addAction( hkAction* act )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RW );
	HK_ASSERT(0xf0ff0028, act->getSimulationIsland() == HK_NULL);

	m_actions.pushBack( act );
	act->setSimulationIsland(this);
}


void hkSimulationIsland::removeAction( hkAction* act )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RW );
	int actIdx = m_actions.indexOf( act );
	
	HK_ASSERT2(0x1aa2186f,  actIdx >= 0, "Action is unknown to the physics" );

	m_actions[actIdx] = hkNullAction::getNullAction();
	//m_actions.removeAtAndCopy( actIdx );
	act->setSimulationIsland(HK_NULL);

	m_splitCheckRequested = true;
	m_actionListCleanupNeeded = true;
}


hkBool hkSimulationIsland::isFullyConnected( hkUnionFind& checkConnectivityOut )
{	
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );

	HK_ON_DEBUG( int numberOfAgents = 0);
	HK_ON_DEBUG( int numberOfConstraints = 0 );

	//
	// Check edges for collision pairs
	// info: interate over entities and their agent/partner lists 
	//
	{
		for (int e = 0; e < m_entities.getSize(); e++)
		{
			hkLinkedCollidable* collidable = &m_entities[e]->m_collidable;
			for (int i = 0; i < collidable->m_collisionEntries.getSize(); i++)
			{
				HK_ON_DEBUG( numberOfAgents++);
				hkLinkedCollidable* partnerCollidable = collidable->m_collisionEntries[i].m_partner;

				hkEntity* partnerEntity = static_cast<hkEntity*>(partnerCollidable->getOwner());
				if (!partnerEntity->isFixed())
				{
					int idx = partnerEntity->m_storageIndex;

					checkConnectivityOut.addEdge( e, idx );

					if ( checkConnectivityOut.isOneGroup() )
					{
						return true;
					}

				}
			}
		}
	}
	
	//
	// Check edges for constraints
	//
	{
		for ( int e = 0; e < m_entities.getSize(); e++)
		{
			hkEntity* entity = m_entities[e];
			for ( int i = 0; i < entity->m_constraintsMaster.getSize(); ++i )
			{
				HK_ON_DEBUG( numberOfConstraints++);
				hkConstraintInternal* con = &entity->m_constraintsMaster[i];
				if ( !con->m_entities[0]->isFixed() && !con->m_entities[1]->isFixed())
				{
					int a = con->m_entities[0]->m_storageIndex;
					int b = con->m_entities[1]->m_storageIndex;
					checkConnectivityOut.addEdge( a, b );
					if ( checkConnectivityOut.isOneGroup() )
					{
						return true;
					}
				}
			}
		}
	}	

	
	//
	// Check edges for actions
	//
	{
		hkInplaceArray<hkEntity*,10> actionEntities;
		for ( int i = 0; i < m_actions.getSize(); ++i )
		{
			if (m_actions[i] != hkNullAction::getNullAction())
			{
				actionEntities.clear();
				m_actions[i]->getEntities( actionEntities );

				int j = 0;
				int firstUnfixed = -1;
				while ( (firstUnfixed == -1) && (j < actionEntities.getSize()) )
				{
					if (!actionEntities[j]->isFixed())
					{
						firstUnfixed = j;
					}
					j++;
				}
	
				for ( ; j < actionEntities.getSize(); ++j )
				{
					if (!actionEntities[j]->isFixed())
					{
						int a = actionEntities[firstUnfixed]->m_storageIndex;
						int b = actionEntities[j]->m_storageIndex;
	
						checkConnectivityOut.addEdge( a, b );
						if ( checkConnectivityOut.isOneGroup() )
						{
							return true;
						}
					}
				}
			}
		}
	}
	//HK_ON_DEBUG( HK_WARN_ALWAYS(0xf0323412, "Agents " << numberOfAgents << "  constraints:" <<numberOfConstraints ));

	return checkConnectivityOut.isOneGroup();
}

HK_FORCE_INLINE hkBool hkSimulationIsland_isSameIsland( hkSimulationIsland*islandA, hkSimulationIsland*islandB )
{
	if ( islandA == islandB )
	{
		return true;
	}

	HK_ASSERT( 0xf0458745, islandA->m_world == islandB->m_world );
	if ( islandA->isFixed())
	{
		return true;
	}
	if ( islandB->isFixed())
	{
		return true;
	}

	return false;
}

#if defined HK_DEBUG
hkBool hkSimulationIsland_isSameIslandOrToBeMerged( hkWorld* world, hkSimulationIsland*islandA, hkSimulationIsland*islandB )
{
	if ( islandA == islandB )
	{
		return true;
	}

	HK_ASSERT( 0xf0458745, islandA->m_world == islandB->m_world );
	if ( islandA->isFixed())
	{
		return true;
	}
	if ( islandB->isFixed())
	{
		return true;
	}

	const hkWorldOperation::BaseOperation* op = hkDebugInfoOnPendingOperationQueues::findFirstPendingIslandMerge(world, islandA, islandB);
	if (op)
	{
		return true;
	}
	return false;
}
#endif

void hkSimulationIsland::isValid()
{
#ifdef HK_DEBUG
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );
	// check if dirty index is ok.
	HK_ASSERT(0, m_dirtyListIndex == HK_INVALID_OBJECT_INDEX || m_world->m_dirtySimulationIslands[m_dirtyListIndex] == this);

	{
		if ( !isFixed() )
		{
			if ( m_isInActiveIslandsArray )
			{
				HK_ASSERT(0x2f4b5bff, m_world->getActiveSimulationIslands().indexOf(this) == m_storageIndex);
			}
			else
			{
				HK_ASSERT(0x6d89676a, m_world->getInactiveSimulationIslands().indexOf(this) == m_storageIndex);
			}
		}
	}

	// check the constraints
	{
		hkConstraintInfo sumInfo; sumInfo.clear();
		{
			for ( int e = 0; e < m_entities.getSize(); e++)
			{
				hkEntity* entity = m_entities[e];

				HK_ASSERT(0XAD000106, entity->getSimulationIsland() == this);

				{
					for ( int i = 0; i < entity->m_constraintsMaster.getSize(); ++i )
					{
						hkConstraintInternal* intern = &entity->m_constraintsMaster[i];

						hkEntity* masterEntity = intern->getMasterEntity();
						hkEntity* slaveEntity = intern->getSlaveEntity();

						HK_ASSERT(0x624a30ff, masterEntity->getSimulationIsland() == this);
						HK_ASSERT(0xad000700, intern->m_priority == intern->m_constraint->getPriority());

						if (slaveEntity->getWorld() == getWorld())
						{
							if ( (!slaveEntity->isFixed()) && slaveEntity->getSimulationIsland() != this )
							{
								HK_ASSERT2(0x23cdd060, hkSimulationIsland_isSameIslandOrToBeMerged( m_world, slaveEntity->getSimulationIsland(), this ), "Constraints connected to two islands, which are not going to be merged" );
							}
						}
						//else
						//{
						//	// todo correct the assert (need to make sure, that futher constraints that link removedEntites will be removed as well)
						//	// THIS ASSERT IS INVALID AND FIRES IN API/RIGIDBODYAPI/MOTIONCHANGE
						//	HK_ASSERT(0XAD000108, getWorld()->m_pendingOperations->findFirstPending(hkWorldOperation::CONSTRAINT_REMOVE, intern->m_constraint));
						//}
						//HK_ASSERT(0x3e2a6a83, intern->m_entities[1]->isFixed() || (intern->m_entities[1]->getSimulationIsland() == this));
						HK_ASSERT(0x3c719c12, slaveEntity->isFixed() || (masterEntity->getSimulationIsland() == this));

						{
							hkConstraintData::ConstraintInfo info;
							intern->m_constraint->getData()->getConstraintInfo( info );
							sumInfo.add( info );
						}

						// check hkConstraintInternal->ConstraintSlave->Constraint->hkConstraintInternal->Constraint inter-points
						HK_ASSERT(0x3bd0155e,  masterEntity == entity);
						HK_ASSERT2(0x285d5d6d, intern->m_constraint->getInternal() == intern, "intern points to a wrong constraint");
						HK_ASSERT2(0x5052cc14, slaveEntity->m_constraintsSlave[intern->m_slaveIndex] == intern->m_constraint, "Constraint slave does not point to the right constraint");

						// start checks from: masters, slaves, island's constraints
					}
				}

				{
					for (int i = 0; i < entity->m_constraintsSlave.getSize(); i++)
					{
						hkConstraintInstance* con = entity->m_constraintsSlave[i];
						hkConstraintInternal* intern = con->getInternal();

						hkEntity* masterEntity = intern->getMasterEntity();
						hkEntity* slaveEntity  = intern->getSlaveEntity();

						if (masterEntity->getWorld() == getWorld())
						{
							if ( (!masterEntity->isFixed()) && masterEntity->getSimulationIsland() != this )
							{
								HK_ASSERT2(0x23cdd060, hkSimulationIsland_isSameIslandOrToBeMerged( m_world, masterEntity->getSimulationIsland(), this ), "Constraints connected to two islands, which are not going to be merged" );
							}
						}
						//else
						//{
						//	// todo correct the assert (need to make sure, that futher constraints that link removedEntites will be removed as well)
						//	// THIS ASSERT IS INVALID AND MIGHT FIRE IN API/RIGIDBODYAPI/MOTIONCHANGE
						//	HK_ASSERT(0XAD000107, getWorld()->m_pendingOperations->findFirstPending(hkWorldOperation::CONSTRAINT_REMOVE, intern->m_constraint));
						//}
						
						HK_ASSERT(0x3bd0155e, slaveEntity == entity);
						HK_ASSERT2(0x49809ddf, intern->m_constraint->getInternal() == intern, "intern points to a wrong constraint");
						HK_ASSERT2(0x22bb9606, slaveEntity->m_constraintsSlave[intern->m_slaveIndex] == intern->m_constraint, "Constraint slave does not point to the right constraint");
					
					}
				}
			}
		}

		// this fires currently if modifiers are used. They do not implement getConstraintInfo yet
		HK_ASSERT(0x471403ec,  this->m_constraintInfo.m_maxSizeOfJacobians  >= sumInfo.m_maxSizeOfJacobians );
		HK_ASSERT(0x431874cd,  this->m_constraintInfo.m_sizeOfJacobians  == sumInfo.m_sizeOfJacobians );
		HK_ASSERT(0x6135edae,  this->m_constraintInfo.m_sizeOfSchemas    == sumInfo.m_sizeOfSchemas );
		HK_ASSERT(0x4a8cb6cf,  this->m_constraintInfo.m_numSolverResults == sumInfo.m_numSolverResults );
	}

	// Checks whether all entities connected via collisionAgnents belong to the same island, or are already pending on the to-be-merged list(, or are fixed).
	// Info: this doesn't work anymore with our implicit recursive pending list
//	{
//		HK_FOR_ALL_AGENT_ENTRIES_BEGIN(this->m_agentTrack, entry)
//		{
//			hkEntity* entityA = static_cast<hkEntity*>(entry->m_collidable[0]->getOwner());
//			hkEntity* entityB = static_cast<hkEntity*>(entry->m_collidable[1]->getOwner());
//
//
//			if ( !(entityA->isFixed() || entityA->getSimulationIsland() == this) ||
//				 !(entityB->isFixed() || entityB->getSimulationIsland() == this)  )
//			{
//				HK_ASSERT(0x23cdd060, m_world->m_pendingOperations->findFirstPendingIslandMerge(entityA->getSimulationIsland(), entityB->getSimulationIsland()));
//			}
//		
//		}
//		HK_FOR_ALL_AGENT_ENTRIES_END;
//	}



	// Verify that there is only one collisionEntry between any pair of entities 
	{
		for (int e = 0; e < this->m_entities.getSize(); e++)
		{
			hkLinkedCollidable* collidable = this->m_entities[e]->getLinkedCollidable();

			hkArray<hkLinkedCollidable::CollisionEntry>& entries = collidable->m_collisionEntries;

			for (int i = 0; i < entries.getSize(); i++)
			{
				hkLinkedCollidable* partner = entries[i].m_partner;

				for (int j = i+1; j < entries.getSize(); j++)
				{
					HK_ASSERT2(0xf0ff0028, entries[j].m_partner != partner, "There are two top level agents between one pair of entities");
				}
			}

		}
	}


	// check the actions
	{
		if (!isFixed())
		{
			for ( int i = 0; i < m_actions.getSize(); ++i )
			{
				if (m_actions[i] != hkNullAction::getNullAction())
				{
					hkArray<hkEntity*> actionEntities;
					m_actions[i]->getEntities( actionEntities );

					HK_ASSERT(0x49414a00,  m_actions[i]->getSimulationIsland() == this || m_actions[i] == hkNullAction::getNullAction());

					for ( int j = 0; j < actionEntities.getSize(); ++j )
					{
						HK_ASSERT(0x58f9423f, hkSimulationIsland_isSameIslandOrToBeMerged( m_world, actionEntities[j]->getSimulationIsland(), this ) );
					}
				}
			}
		}

		for (int e = 0; e < m_entities.getSize(); e++)
		{
			for (int a = 0; a < m_entities[e]->getNumActions(); a++)
			{
				hkAction* action = m_entities[e]->getAction(a);
					// the action either is null, or was just removed from the island/world but still hangs on an entiti's actionList, 
				    //	or it must be properly assigned to an island
				HK_ASSERT(0xf0ff0029, action == hkNullAction::getNullAction() || action->getWorld() != getWorld() || action->getSimulationIsland() == this);
				//HK_ASSERT(0xad000175, m_entities[e]->m_actions[a]);
			}
		}
	}


	hkAgentNnMachine_AssertTrackValidity(m_agentTrack);
#endif		
}




hkBool hkSimulationIsland::shouldDeactivateDeprecated( const hkStepInfo& stepInfo )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	// NOTE: we could remember the last entity that reported active and check that first as an optimization.

	//
	// Check for deactivation
	//
	m_timeSinceLastHighFrequencyCheck += stepInfo.m_deltaTime;
	m_timeSinceLastLowFrequencyCheck += stepInfo.m_deltaTime;

	const hkBool highFreqCheck = m_timeSinceLastHighFrequencyCheck > m_world->getHighFrequencyDeactivationPeriod();
	const hkBool lowFreqCheck = m_timeSinceLastLowFrequencyCheck > m_world->getLowFrequencyDeactivationPeriod();

	if ( highFreqCheck )
	{
		m_timeSinceLastHighFrequencyCheck -= m_world->getHighFrequencyDeactivationPeriod();
		hkBool deactivate  = true;
		for ( int i = 0; i < m_entities.getSize(); ++i )
		{
			hkEntity* entity = m_entities[i];
			if ( ( entity->getDeactivator() == HK_NULL ) ||	 ( !entity->getDeactivator()->shouldDeactivateHighFrequency( entity ) ) )
			{
				deactivate = false;
				continue;
			}
		}
		if ( deactivate )
		{
			if ( ++m_highFrequencyDeactivationCounter >= 5 )
			{
				m_highFrequencyDeactivationCounter = 0;
				return true;
			}
		}
		else
		{
			m_highFrequencyDeactivationCounter = 0;
		}
	}
	else if ( lowFreqCheck )
	{
		m_timeSinceLastLowFrequencyCheck -= m_world->getLowFrequencyDeactivationPeriod();
		hkBool deactivate = true;
		for ( int i = 0; i < m_entities.getSize(); ++i )
		{
			if ( ( m_entities[i]->getDeactivator() == HK_NULL ) || 
				 ( !m_entities[i]->getDeactivator()->shouldDeactivateLowFrequency( m_entities[i] ) ) )
			{
				deactivate = false;
				continue;
			}
		}
		if ( deactivate )
		{
			if ( ++m_lowFrequencyDeactivationCounter >= 5 )
			{
				m_lowFrequencyDeactivationCounter = 0;
				return true;
			}
		}
		else
		{
			m_lowFrequencyDeactivationCounter = 0;
		}
	}

	return false;
}

void hkSimulationIsland::addConstraintToCriticalLockedIsland( hkConstraintInstance* constraint )
{
	hkWorldOperationUtil::addConstraintToCriticalLockedIsland( constraint->getEntityA()->getWorld(), constraint );
}

void hkSimulationIsland::removeConstraintFromCriticalLockedIsland( hkConstraintInstance* constraint )
{
	hkWorldOperationUtil::removeConstraintFromCriticalLockedIsland( constraint->getEntityA()->getWorld(), constraint );
}

void hkSimulationIsland::addCallbackRequest( hkConstraintInstance* constraint, int request )
{
	HK_ACCESS_CHECK_OBJECT( constraint->getSimulationIsland()->getWorld(), HK_ACCESS_RW );
	constraint->m_internal->m_callbackRequest |= request;
}



void hkSimulationIsland::mergeConstraintInfo( hkSimulationIsland& other )
{
	m_constraintInfo.add( other.m_constraintInfo );
}

void hkSimulationIsland::markForWrite( )
{
#ifdef HK_DEBUG_MULTI_THREADING
	if ( !m_inIntegrateJob )
	{
		if ( m_world && m_world->m_modifyConstraintCriticalSection )
		{
			HK_ASSERT2( 0xf0213de, m_world->m_modifyConstraintCriticalSection->haveEntered(), "You cannot mark an island for write without having the m_world->m_modifyConstraintCriticalSection entered" );
		}
	}
	m_multiThreadLock.markForWrite();
#endif
}

#ifdef HK_DEBUG_MULTI_THREADING
void hkSimulationIsland::checkAccessRw()
{
	// If you get a crash here and you want to understand constraintOwner, you may want to read the reference manual for hkResponseModifier
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW);
}
#endif

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
