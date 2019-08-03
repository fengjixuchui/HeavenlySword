/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkdynamics/hkDynamics.h>
#include <hkbase/htl/hkAlgorithm.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/debugutil/hkCheckDeterminismUtil.h>

#include <hkmath/basetypes/hkAabb.h>
#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkinternal/collide/broadphase/hkBroadPhase.h>
#include <hkinternal/collide/broadphase/hkBroadPhaseHandle.h>

#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkcollide/dispatch/contactmgr/hkNullContactMgrFactory.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseDispatcher.h>

#include <hkcollide/filter/hkCollisionFilter.h>
#include <hkcollide/filter/null/hkNullCollisionFilter.h>
#include <hkcollide/filter/defaultconvexlist/hkDefaultConvexListFilter.h>

#include <hkcollide/agent/null/hkNullAgent.h>
#include <hkcollide/agent/hkCollisionInput.h>
#include <hkcollide/agent/hkLinearCastCollisionInput.h>
#include <hkcollide/agent/hkCollisionAgentConfig.h>
#include <hkcollide/agent/hkCdBodyPairCollector.h>

#include <hkcollide/castutil/hkSimpleWorldRayCaster.h>
#include <hkcollide/castutil/hkWorldRayCaster.h>
#include <hkcollide/castutil/hkWorldLinearCaster.h>

#include <hkinternal/collide/agent3/machine/1n/hkAgent1nTrack.h>
#include <hkinternal/collide/agent3/machine/nn/hkAgentNnMachine.h>

#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/world/util/hkNullAction.h>

#include <hkdynamics/phantom/hkPhantom.h>
#include <hkdynamics/phantom/hkPhantomBroadPhaseListener.h>
#include <hkdynamics/world/util/broadphase/hkEntityEntityBroadPhaseListener.h>
#include <hkdynamics/world/util/broadphase/hkBroadPhaseBorderListener.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkdynamics/world/broadphaseborder/hkBroadPhaseBorder.h>

#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>
#include <hkdynamics/constraint/breakable/hkBreakableConstraintData.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstance.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstanceAction.h>

#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>

#include <hkdynamics/collide/hkSimpleConstraintContactMgr.h>
#include <hkdynamics/collide/hkReportContactMgr.h>


#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/hkPhysicsSystem.h>
#include <hkdynamics/world/util/hkWorldCallbackUtil.h>

#include <hkdynamics/world/simulation/hkSimulation.h>
#include <hkinternal/dynamics/world/simulation/continuous/hkContinuousSimulation.h>

//#if defined(HK_PLATFORM_MULTI_THREAD) // does not compile with this guard
#	include <hkdynamics/world/simulation/multithreaded/hkMultithreadedSimulation.h>
//#endif

#include <hkdynamics/world/simulation/backstep/hkBackstepSimulation.h>

#include <hkdynamics/world/util/hkWorldConstraintUtil.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>
#include <hkdynamics/world/util/hkWorldOperationQueue.h>
#include <hkdynamics/world/maintenance/default/hkDefaultWorldMaintenanceMgr.h>
#include <hkdynamics/world/memory/default/hkDefaultWorldMemoryWatchDog.h>

#if defined HK_DEBUG
// Only used in hkWorld::constrainedDynamicBodiesCanCollide() below.
#include <hkcollide/collector/pointcollector/hkClosestCdPointCollector.h>
#endif // #if defined HK_DEBUG

hkBool hkWorld::m_forceMultithreadedSimulation = false;

hkMultithreadConfig::hkMultithreadConfig()
{
#ifdef HK_PLATFORM_HAS_SPU
	m_maxNumConstraintsSolvedSingleThreaded       = 4;
#else
	m_maxNumConstraintsSolvedSingleThreaded       = 70;
#endif

#if defined(HK_PLATFORM_HAS_SPU)
	m_canCpuTakeSpuTasks = hkMultithreadConfig::CPU_CAN_TAKE_SPU_TASKS;
#endif
}

hkWorld::~hkWorld()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	// avoid any spurious asserts, if the user has just added some entities
	HK_ASSERT(0xad000086, !areCriticalOperationsLocked() && !m_pendingOperationsCount);
	//unlockAndAttemptToExecutePendingOperations();

	m_maintenanceMgr->removeReference();
	m_maintenanceMgr = HK_NULL;
	if ( m_modifyConstraintCriticalSection )
	{
		delete m_modifyConstraintCriticalSection;
		m_modifyConstraintCriticalSection = HK_NULL;
	}

	if ( m_propertyMasterLock )
	{
		delete m_propertyMasterLock;
		m_propertyMasterLock = HK_NULL;
		for (int i =0; i < m_propertyLocks.getSize();i++)
		{
			delete m_propertyLocks[i].m_lock;
		}
		m_propertyLocks.clear();
	}

	if ( m_worldLock )
	{
		delete m_worldLock;
		m_worldLock = HK_NULL;
	}

	if ( m_islandDirtyListCriticalSection )
	{
		delete m_islandDirtyListCriticalSection;
		m_islandDirtyListCriticalSection = HK_NULL;
	}


	if (m_memoryWatchDog)
	{
		m_memoryWatchDog->removeReference();
		m_memoryWatchDog = HK_NULL;
	}


	//
	// Cleanup notes: Actions and constraints should be removed by deleting the entities,
	// so we don't explicitly remove them here.
	//
	//

	// NOTE: if collide has been called and integrate has not been called yet
	// by the hkHalfSteppingUtility for example then the
	// m_simulationState will be CAN_NOT_REMOVE_ENTITIES_AND_CONSTRAINTS...
	// This will prevent the world from being destroyed as the loops below will continuously
	// fail to remove any entities.  Here we will set the m_simulationState to
	// CAN_REMOVE_ENTITIES_AND_CONSTRAINTS.  This should be safe as the world is about to
	// be deleted so this state variable should no longer be critical.

	const hkArray<hkSimulationIsland*>& activeIslands = getActiveSimulationIslands();
	{
		while( m_phantoms.getSize() > 0 )
		{
			removePhantom( m_phantoms[0] );
		}


		while ( activeIslands.getSize() > 0 && activeIslands[0]->m_entities.getSize())
		{
			hkEntity* entity = activeIslands[0]->m_entities[0];
			removeEntity( entity );
		}

		while ( m_inactiveSimulationIslands.getSize() > 0 )
		{
			removeEntity( m_inactiveSimulationIslands[0]->m_entities[0] );
		}
	}

	if (!m_wantSimulationIslands)
	{
		// Remove the only active simulation island
		HK_ASSERT(0, activeIslands.getSize() == 1 && activeIslands[0]->getEntities().getSize() == 0);
		hkWorldOperationUtil::removeIslandFromDirtyList(this, activeIslands.back());
		delete m_activeSimulationIslands.back();
		m_activeSimulationIslands.popBack();
	}



	{
		// remove fixed stuff

		removeEntity( m_fixedRigidBody );
		m_fixedRigidBody = HK_NULL;

		while ( m_fixedIsland->m_entities.getSize() > 0 )
		{
			removeEntity( m_fixedIsland->m_entities[0] );
		}

		hkWorldOperationUtil::removeIslandFromDirtyList(this, m_fixedIsland);
		delete m_fixedIsland;
		m_fixedIsland = HK_NULL;
	}


	hkWorldCallbackUtil::fireWorldDeleted( this );

	m_broadPhase->removeReference();
	m_broadPhase = HK_NULL;

	m_collisionDispatcher->removeReference();
	m_collisionDispatcher = HK_NULL;

	delete m_broadPhaseDispatcher;
	if ( m_broadPhaseBorder )
	{
		m_broadPhaseBorder->removeReference();
	}

	delete m_phantomBroadPhaseListener;
	delete m_entityEntityBroadPhaseListener;
	delete m_broadPhaseBorderListener;

	m_collisionFilter->removeReference();
	m_convexListFilter->removeReference();

	delete m_collisionInput->m_config;
	delete m_collisionInput;

	m_simulation->removeReference();

	HK_ON_DEBUG( hkDebugInfoOnPendingOperationQueues::cleanup(this); );
	delete m_pendingOperations; 
}

static void hkWorld_setupContactMgrFactories( hkWorld* world, hkCollisionDispatcher* dis )
{
	hkContactMgrFactory* simple = new hkSimpleConstraintContactMgr::Factory( world );
	hkContactMgrFactory* rep    = new hkReportContactMgr::Factory( world );
	hkContactMgrFactory* none   = new hkNullContactMgrFactory();



		// simple
	dis->registerContactMgrFactoryWithAll( simple, hkMaterial::RESPONSE_SIMPLE_CONTACT );
	dis->registerContactMgrFactoryWithAll( rep, hkMaterial::RESPONSE_REPORTING );
	dis->registerContactMgrFactoryWithAll( none, hkMaterial::RESPONSE_NONE );

	simple->removeReference();
	rep->removeReference();
	none->removeReference();
}

// This method should be called if you have changed the collision filter for the world.
void hkWorld::updateCollisionFilterOnWorld( hkUpdateCollisionFilterOnWorldMode updateMode, hkUpdateCollectionFilterMode updateShapeCollectionFilter )
{
	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::UpdateFilterOnWorld op;
		op.m_collisionFilterUpdateMode = updateMode;
		op.m_updateShapeCollections = updateShapeCollectionFilter;
		queueOperation( op );
		return;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	
	//
	// Proceed whit the proper operation
	//

	//HK_ASSERT2(0x623c210c,  m_filterUpdateState == CAN_UPDATE_FILTERS, "You are trying to update collision filters during a collide() call. This can lead to the system destroying a collision agent with a function in the current call stack, and so is not allowed.");

	blockExecutingPendingOperations(true);

	HK_TIMER_BEGIN( "UpdateFilterOnWorld", HK_NULL);
	if ( updateMode == HK_UPDATE_FILTER_ON_WORLD_FULL_CHECK )
	{
	

		// This method should be called if you have changed the world collision filter
		{
			// Active Islands
			const hkArray<hkSimulationIsland*>& activeIslands = getActiveSimulationIslands();
			for ( int i = 0; i < activeIslands.getSize(); ++i )
			{
				const hkArray<hkEntity*>& entities = activeIslands[i]->getEntities();
				
				for( int j = 0; j < entities.getSize(); j++ )
				{
					hkEntity* entity = entities[j];
					updateCollisionFilterOnEntity( entity, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, updateShapeCollectionFilter );
				}
			}
		}

		{
			// Inactive Islands
			for ( int i = 0; i < m_inactiveSimulationIslands.getSize(); ++i )
			{
				const hkArray<hkEntity*>& entities = m_inactiveSimulationIslands[i]->getEntities();
				
				for( int j = 0; j < entities.getSize(); j++ )
				{
					hkEntity* entity = entities[j];
					updateCollisionFilterOnEntity( entity, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, updateShapeCollectionFilter);
				}		
			}
		}

	}
	else
	{
		// do not requery broadphase -- only check each agent once
		lockCriticalOperations();

		{
			const hkArray<hkSimulationIsland*>* arrays[2] = { &getActiveSimulationIslands(), &m_inactiveSimulationIslands };

			for ( int a = 0; a < 2; a++)
			{
				hkInplaceArray<hkAgentNnEntry*, 32> agentsToRemove;
				// Active + Inactive Islands 
				for ( int i = 0; i < arrays[a]->getSize(); ++i )
				{
					hkSimulationIsland* island = (*arrays[a])[i];
					hkAgentNnTrack& track = island->m_agentTrack;

					agentsToRemove.clear();
					HK_FOR_ALL_AGENT_ENTRIES_BEGIN(track, agentEntry)
					{
						// Verify existing collision agents

						if (!getCollisionFilter()->isCollisionEnabled( *agentEntry->m_collidable[0],  *agentEntry->m_collidable[1] ))
						{
							goto removeAgentLabel;
						}

						// check for disabled collisions, especially landscape = landscape ones
						{
							{
								hkCollidableQualityType qt0 = agentEntry->m_collidable[0]->getQualityType();
								hkCollidableQualityType qt1 = agentEntry->m_collidable[1]->getQualityType();
								int collisionQuality = getCollisionDispatcher()->getCollisionQualityIndex( qt0, qt1 );
								if ( collisionQuality == hkCollisionDispatcher::COLLISION_QUALITY_INVALID )
								{
									goto removeAgentLabel;
								}
							}
						}

						// check collections
						if ( updateShapeCollectionFilter == HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS )
						{
							hkAgentNnMachine_UpdateShapeCollectionFilter( agentEntry, *getCollisionInput(), *island );
						}
						continue;

						{
						removeAgentLabel:

							agentsToRemove.pushBack(agentEntry);

							// Request split check.
							island->m_splitCheckRequested = true;
						}
					}
					HK_FOR_ALL_AGENT_ENTRIES_END;

					while(agentsToRemove.getSize())
					{
						hkAgentNnEntry* agent = agentsToRemove.back();
						agentsToRemove.popBack();
						hkWorldAgentUtil::removeAgentAndItsToiEvents(agent);
					}
				}
			}
		}
		unlockCriticalOperations();

	}

	if (updateMode == HK_UPDATE_FILTER_ON_WORLD_FULL_CHECK )
	{
		for ( int i = 0; i < m_phantoms.getSize(); i++ )
		{
			hkPhantom* phantom = m_phantoms[i];
			updateCollisionFilterOnPhantom( phantom, updateShapeCollectionFilter );
		}
	}

	blockExecutingPendingOperations(false);
	attemptToExecutePendingOperations();


	HK_TIMER_END();
}

static void HK_CALL hkWorld_updateFilterOnSinglePhantom( hkPhantom* phantom, hkCollidable* collidable, hkCollisionFilter* filter  )
{
	hkBool oldOverlapping = phantom->isOverlappingCollidableAdded( collidable );
	hkCollidable* phantomCollidable = phantom->getCollidableRw();

	if( filter->isCollisionEnabled( *phantomCollidable, *collidable ) )
	{
		if ( !oldOverlapping )
		{
			phantom->addOverlappingCollidable( collidable );
		}

		if( collidable->getType() == hkWorldObject::BROAD_PHASE_PHANTOM )
		{
			hkBool otherOldOverlapping = static_cast<hkPhantom*>(collidable->getOwner())->isOverlappingCollidableAdded( phantomCollidable );
			if( !otherOldOverlapping )
			{
				static_cast<hkPhantom*>(collidable->getOwner())->addOverlappingCollidable( phantomCollidable );
			}
		}
	}
	else
	{
		if ( oldOverlapping )
		{
			phantom->removeOverlappingCollidable( collidable );
		}

		if( collidable->getType() == hkWorldObject::BROAD_PHASE_PHANTOM )
		{
			hkBool otherOldOverlapping = static_cast<hkPhantom*>(collidable->getOwner())->isOverlappingCollidableAdded( phantomCollidable );
			if( otherOldOverlapping )
			{
				static_cast<hkPhantom*>(collidable->getOwner())->removeOverlappingCollidable( phantomCollidable );
			}
		}
	}
} 

void hkWorld::updateCollisionFilterOnPhantom( hkPhantom* phantom, hkUpdateCollectionFilterMode updateShapeCollectionFilter )
{
	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::UpdateFilterOnPhantom op;
		op.m_phantom = phantom;
		op.m_updateShapeCollections = updateShapeCollectionFilter;
		queueOperation( op );
		return;
	}
	
	//
	// Proceed with the proper operation
	//
	HK_ASSERT2(0x6c6fbb7e,  phantom->getWorld() == this, "Trying to update a phantom that has not been added to the world");
	HK_ACCESS_CHECK_WITH_PARENT( this, HK_ACCESS_RO, phantom, HK_ACCESS_RW );

	lockCriticalOperations();
	HK_TIMER_BEGIN_LIST("UpdateFilterOnPhantom", "broadphase" );


	// Get the list of overlapping pairs and see which ones are to be removed and which are to be added
	hkCollidable* phantomCollidable = phantom->getCollidableRw();
	hkLocalArray<hkBroadPhaseHandlePair> pairsOut( m_broadPhaseQuerySize );
	m_broadPhase->reQuerySingleObject( phantomCollidable->getBroadPhaseHandle(), pairsOut );

	HK_TIMER_SPLIT_LIST("UpdateOverlaps");

	// Sort the pairsOut list
	for( int i = 0; i<pairsOut.getSize(); i++ )
	{
		// check for not having self overlaps
		HK_ASSERT2( 0xf043defd, pairsOut[i].m_b != phantomCollidable->getBroadPhaseHandle(), "Error in Broadphase: query object returned in query result" );
		if( pairsOut[i].m_b == phantomCollidable->getBroadPhaseHandle() )
		{
			// Ignore self overlaps 
			continue;
		}
		hkCollidable* collidable = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pairsOut[i].m_b)->getOwner() );
		hkWorld_updateFilterOnSinglePhantom( phantom, collidable, m_collisionFilter );
	}
	
	if ( updateShapeCollectionFilter )
	{
		HK_TIMER_SPLIT_LIST("collectionFilter");
		phantom->updateShapeCollectionFilter();	
	}

	unlockAndAttemptToExecutePendingOperations();
	HK_TIMER_END_LIST();
}

// This method should be called if you have altered the collision filtering information for this entity.
void hkWorld::updateCollisionFilterOnEntity( hkEntity* entity, hkUpdateCollisionFilterOnEntityMode updateMode, hkUpdateCollectionFilterMode updateShapeCollectionFilter )
{
	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::UpdateFilterOnEntity op;
		op.m_entity = entity;
		op.m_collisionFilterUpdateMode = updateMode;
		op.m_updateShapeCollections = updateShapeCollectionFilter;
		queueOperation( op );
		return;
	}
	
	//
	// Proceed with the proper operation
	//
	HK_ASSERT2(0XAD000103, entity->getWorld() == this, "Error: updatingCollisionFilter on a body not inserted into this world.");

	HK_ACCESS_CHECK_WITH_PARENT( this, HK_ACCESS_RW, entity, HK_ACCESS_RW );

	// If you don't want to lock the world than get an extra entity->addReference()
	HK_TIMER_BEGIN_LIST( "UpdateFilterOnEntity", "init");
	{
		lockCriticalOperations();
		
			// Recreate a list of present broad-phase pairs
		hkInplaceArray<hkBroadPhaseHandlePair,128> updatedPairs;

		if (updateMode == HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK)
		{
				// Get the list of overlapping pairs and see which ones are to be removed and which are to be added
			HK_TIMER_SPLIT_LIST("broadphase");

			m_broadPhase->reQuerySingleObject( entity->getCollidable()->getBroadPhaseHandle(), updatedPairs );

			//
			//	Do phantoms
			//
			{
				HK_TIMER_SPLIT_LIST("phantom");

				for (int i = 0; i < updatedPairs.getSize(); i++ )
				{
					hkCollidable* collidable = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(updatedPairs[i].m_b)->getOwner() );
					hkPhantom* phantom = hkGetPhantom(collidable);

					if (phantom)
					{
						hkWorld_updateFilterOnSinglePhantom( phantom, entity->getCollidableRw(), m_collisionFilter );
						if ( updateShapeCollectionFilter )
						{
							phantom->updateShapeCollectionFilter();	
						}

						// Remove entry from the list
						updatedPairs.removeAt(i--);
					}
				}
			}

			//
			// Do entities
			//

			hkArray<hkLinkedCollidable::CollisionEntry>& collisionEntries = entity->getLinkedCollidable()->m_collisionEntries;

			hkInplaceArray<hkBroadPhaseHandlePair,128> presentPairs;
			{
				for (int i = 0; i < collisionEntries.getSize(); i++)
				{
					hkBroadPhaseHandlePair& pair = presentPairs.expandOne();
					pair.m_a = entity->getLinkedCollidable()->m_collisionEntries[i].m_agentEntry->getCollidableA()->getBroadPhaseHandle();
					pair.m_b = entity->getLinkedCollidable()->m_collisionEntries[i].m_agentEntry->getCollidableB()->getBroadPhaseHandle();
				}
			}

				// Create a to-remove and to-add lists
			hkTypedBroadPhaseDispatcher::removeDuplicates(presentPairs, updatedPairs);

			// Add pairs executed after verifying existing agents
			HK_ASSERT(0xf0764312, presentPairs.getSize() == 0);
		}

		//
		// Verify existing collision agents
		//
		{
			HK_TIMER_SPLIT_LIST("checkAgts");
			hkCollidableQualityType qt0 = entity->getLinkedCollidable()->getQualityType();

			hkArray<hkLinkedCollidable::CollisionEntry>& collisionEntries = entity->getLinkedCollidable()->m_collisionEntries;
			for (int i = 0; i < collisionEntries.getSize(); i++)
			{
				hkLinkedCollidable::CollisionEntry& entry = collisionEntries[i];

				if (!getCollisionFilter()->isCollisionEnabled( *entity->getCollidable(),  *entry.m_partner ))
				{
					goto removeAgentLabel;
				}

				// check for disabled collisions, especially landscape = landscape ones
				{
					{
						hkCollidableQualityType qt1 = entry.m_partner->getQualityType();
						int collisionQuality = getCollisionDispatcher()->getCollisionQualityIndex( qt0, qt1 );
						if ( collisionQuality == hkCollisionDispatcher::COLLISION_QUALITY_INVALID )
						{
							goto removeAgentLabel;
						}
					}
				}

				// check collections
				if ( updateShapeCollectionFilter == HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS )
				{
					hkEntity* entityA = entity;
					hkEntity* entityB = static_cast<hkEntity*>(entry.m_partner->getOwner());
					hkSimulationIsland* island = (entityA->isFixed() )? entityB->getSimulationIsland(): entityA->getSimulationIsland();

					hkAgentNnMachine_UpdateShapeCollectionFilter( entry.m_agentEntry, *getCollisionInput(), *island );
				}
				continue;

				{
				removeAgentLabel:

					//remove agent
					HK_ON_DEBUG(int oldSize = collisionEntries.getSize());
					hkWorldAgentUtil::removeAgentAndItsToiEvents(entry.m_agentEntry);
					HK_ASSERT(0xf0ff002a, oldSize - 1 == collisionEntries.getSize());
					// the collision entries list just shrinked, so set the index to the first unchecked entry
					i--;

					// Request split check.
					entity->getSimulationIsland()->m_splitCheckRequested = true;
				}
			}
		}

		//
		// (Continuation of broadphase check) add new agents:
		//

		// only performed at FULL_BROAD_PHASE_CHECK
		if (updatedPairs.getSize() > 0)
		{
			HK_TIMER_SPLIT_LIST("addAgts");
				// filter and add pairsOut
				// this list includes entity-phantom pairs as well
			m_broadPhaseDispatcher->addPairs( static_cast<hkTypedBroadPhaseHandlePair*>(&updatedPairs[0]), updatedPairs.getSize(), getCollisionFilter() );
		}

		

#		ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
		{
			hkSimulationIsland* island = entity->getSimulationIsland();
			if ( !island->isFixed())
			{
				island->isValid();
			}
		}
#		endif

		unlockAndAttemptToExecutePendingOperations();
	}
	HK_TIMER_END_LIST();
}

hkEntity* hkWorld::addEntity( hkEntity* entity, enum hkEntityActivation initialActivationState)
{
	HK_ASSERT2( 0x7f090345, entity, "You can not add a null entity to a world.");

	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::AddEntity op;
		op.m_entity = entity;
		op.m_activation = initialActivationState;
		queueOperation( op );
		return HK_NULL;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	HK_INTERNAL_TIMER_BEGIN_LIST("AddEntity", "Island");
	
	//
	// Proceed with the proper operation
	//

	// history: m_simulationState = CAN_NOT_REMOVE_ENTITIES_AND_CONSTRAINTS;

	HK_ASSERT2(0x3f9eb209,  entity->getWorld() == HK_NULL, "You are trying to add an entity to a world, which has already been added to a world" );
	HK_ASSERT2(0xf0ff0030, entity->m_actions.isEmpty(), "Entity to add has already actions attached, this is wrong");

	// check if the collidable back ptr to the motion state is set
	// as it may not be due to packedfile serialization
    if (!entity->m_collidable.getMotionState())
	{
		hkMotionState*	motionState = entity->getMotionState();
		entity->m_collidable.setMotionState( motionState );
	}
	// The motions state as added may be 4 dimensional (have time, which is probably
	// nothing in relation to this world time)
	// so we make sure to set the invDeltaTime to 0 on it which 
	// makes it just a 3D placement (it will set itself up with a 
	// time quantum upon next step).
	hkMotionState* ms = static_cast<hkRigidBody*>( entity )->getRigidMotion()->getMotionState();
	hkSweptTransformUtil::setTimeInformation(hkTime(0.0f), 0.0f, *ms);
	
	// Simulation Island
	allowCriticalOperations(false);
	{
			// Assign world-unique id
		entity->m_uid = ++m_lastEntityUid;
			// Add a reference to the entity
		entity->addReference();
			// add island
		hkWorldOperationUtil::addEntitySI( this, entity, initialActivationState );

	}
	allowCriticalOperations(true);


	lockCriticalOperations();
	HK_INTERNAL_TIMER_SPLIT_LIST("Broadphase");
	{
			// Add the entity to BroadPhase
		hkWorldOperationUtil::addEntityBP( this, entity );
	}

		// Run callbacks before other pending operations as callbacks fire internal operations which ensure proper state of the world 
	//if ( DO_FIRE_CALLBACKS == fireCallbacks )
	HK_INTERNAL_TIMER_SPLIT_LIST("Callbacks");
	{
			// Fire the callbacks
			// notice: order
		hkWorldCallbackUtil::fireEntityAdded( this, entity );
		hkEntityCallbackUtil::fireEntityAdded( entity );
	}

	unlockCriticalOperations();

#ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	HK_INTERNAL_TIMER_SPLIT_LIST("Validate");
	{
		hkSimulationIsland* island = entity->getSimulationIsland();

		{
			if ( island ) { island->isValid(); }
		}

	}
#endif

	{
		attemptToExecutePendingOperations();
	}
	HK_INTERNAL_TIMER_END_LIST();
	return entity;
}


void hkWorld::addEntityBatch( hkEntity*const* entityBatch, int numEntities, hkEntityActivation initialActivationState )
{
	if( numEntities == 0 )
	{
		return;
	}

	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::AddEntityBatch op;
		op.m_entities = const_cast<hkEntity**>(entityBatch);
		HK_ASSERT(0xf0ff0040, numEntities < HK_INVALID_OBJECT_INDEX );
		op.m_numEntities = static_cast<hkObjectIndex>(numEntities);
		op.m_activation = initialActivationState;
		queueOperation( op );
		return;
	}
	
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	HK_TIMER_BEGIN_LIST("AddEntities", "init")

	//SimulationState savedState = static_cast<SimulationState>(m_simulationState);
	//m_simulationState = CAN_NOT_REMOVE_ENTITIES_AND_CONSTRAINTS;

	lockCriticalOperations();

	hkLocalArray< hkBroadPhaseHandle* > collList(numEntities);
	hkLocalArray< hkAabb > aabbList(numEntities);

	hkSimulationIsland* gabriolaIsland;

	// create an island
	bool usedGabriola = false;
	if (m_wantSimulationIslands)
	{
		gabriolaIsland = new hkSimulationIsland(this);
		gabriolaIsland->m_splitCheckRequested = true;

		if (initialActivationState == HK_ENTITY_ACTIVATION_DO_ACTIVATE)
		{
			gabriolaIsland->m_storageIndex = (hkObjectIndex)getActiveSimulationIslands().getSize();
			gabriolaIsland->m_isInActiveIslandsArray = true;
			gabriolaIsland->m_active = true;
			// will be added to active list if used.
		}
		else
		{
			gabriolaIsland->m_storageIndex = (hkObjectIndex)m_inactiveSimulationIslands.getSize();
			gabriolaIsland->m_isInActiveIslandsArray = false;
			gabriolaIsland->m_active = false;
			// will be added to inactive list if used.
		}
		gabriolaIsland->m_splitCheckFrameCounter = hkUchar(gabriolaIsland->m_storageIndex);
	}
	else
	{
		gabriolaIsland = getActiveSimulationIslands()[0];
		gabriolaIsland->m_entities.reserve( gabriolaIsland->m_entities.getSize() + numEntities );
	}
	
	{
		hkReal extraRadius = getCollisionInput()->getTolerance() * .5f;
		for( int i = 0; i < numEntities; i++ )
		{
			hkEntity* entity = entityBatch[i];
				
			HK_ASSERT2( 0xad5fbd63, entity, "You can not batch with a null entity to a world.");
			HK_ASSERT2( 0xad5fbd64, entity->getWorld() == HK_NULL, "You are trying to add an entity, which already belongs to an hkWorld.");

			// Assign world-unique id
			entity->m_uid = ++m_lastEntityUid;

			entity->addReference();
			if (!entity->getCollidable()->getMotionState()) // may be null due to packfile serialize
			{
				entity->m_collidable.setMotionState( entity->getMotionState() );
			}
			hkMotionState* ms = static_cast<hkRigidBody*>( entity )->getRigidMotion()->getMotionState();
			hkSweptTransformUtil::setTimeInformation(hkTime(0.0f), 0.0f, *ms); // set time to 0 with no invdelta (so not swept)

			entity->setWorld( this );

			if ( entity->isFixed() )
			{
				m_fixedIsland->internalAddEntity( entity );
			}
			else
			{
				usedGabriola = true;
				gabriolaIsland->internalAddEntity(entity);
			}


			hkCollidable* c = entity->getCollidableRw();
			const hkShape* shape = c->getShape();

			if (shape)
			{
				// add the shape to the broad phase and merge islands as necessary
				hkAabb& aabb = *aabbList.expandByUnchecked(1); 
				c->getShape()->getAabb( c->getTransform(), extraRadius, aabb );			
				collList.pushBackUnchecked( c->getBroadPhaseHandle() ); 
			}
		}
	}

	if (m_wantSimulationIslands)
	{
		if (usedGabriola)
		{
			hkArray<hkSimulationIsland*>& islandArray = initialActivationState == HK_ENTITY_ACTIVATION_DO_ACTIVATE
				? const_cast<hkArray<hkSimulationIsland*>&>(getActiveSimulationIslands())
				: m_inactiveSimulationIslands;
			islandArray.pushBack(gabriolaIsland);
		}
		else
		{
			delete gabriolaIsland;
		}
	}

	hkLocalArray< hkBroadPhaseHandlePair > pairsOut( m_broadPhaseQuerySize );

	HK_TIMER_SPLIT_LIST("Broadphase");
	
	m_broadPhase->addObjectBatch( collList, aabbList, pairsOut );

	HK_TIMER_SPLIT_LIST("CreateAgents");

	m_broadPhaseDispatcher->addPairs( static_cast<hkTypedBroadPhaseHandlePair*>(pairsOut.begin()), pairsOut.getSize(), getCollisionFilter() );

	HK_TIMER_SPLIT_LIST("AddedCb");

	{
		for( int i = 0; i < numEntities; i++ )
		{
			hkEntity* entity = entityBatch[i];
			hkWorldCallbackUtil::fireEntityAdded( this, entity );
			hkEntityCallbackUtil::fireEntityAdded( entity );
		}
	}

	unlockAndAttemptToExecutePendingOperations();

	HK_TIMER_END_LIST();
}


hkBool hkWorld::removeEntity( hkEntity* entity )
{
	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::RemoveEntity op;
		op.m_entity = entity;
		queueOperation( op );
		return false;
	}

	HK_ASSERT2(0x72576e5e,  entity->getWorld() == this, "You are trying to remove an entity from a world to which it is not added");

	//
	// Proceed with the proper operation
	//

	lockCriticalOperations();

	// Update the BroadPhase
	HK_INTERNAL_TIMER_BEGIN_LIST("RemEntity", "Broadphase");
	hkWorldOperationUtil::removeEntityBP( this, entity );
	HK_ASSERT(0xad000095, 0 == entity->getLinkedCollidable()->m_collisionEntries.getSize());

	// Fire the callbacks
	HK_INTERNAL_TIMER_SPLIT_LIST("Callbacks");
	// World callbacks are called first (to allow access to the entity's constraints and actions (as they're removed in the entity callback)
	hkWorldCallbackUtil::fireEntityRemoved( this, entity );
	hkEntityCallbackUtil::fireEntityRemoved( entity );

	// when should callbacks be called ? with all agents + constraints + etc. in place ?

	HK_ASSERT(0xad000210, entity->m_actions.isEmpty());
	HK_ASSERT(0xad000211, entity->m_constraintsMaster.isEmpty());
	HK_ASSERT(0xad000212, entity->m_constraintsSlave.isEmpty());

#	if defined HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	hkSimulationIsland* island = (entity->getSimulationIsland() && entity->getSimulationIsland()->m_entities.getSize() == 1) ? HK_NULL : entity->getSimulationIsland();
#	endif
		
		// do it here as you also need to allow for removal of constraints by the callbacks
	allowCriticalOperations(false);
	{
		HK_INTERNAL_TIMER_SPLIT_LIST("Island");
		HK_ASSERT(0xf0ff0041, entity->getWorld());

		hkWorldOperationUtil::removeEntitySI( this, entity );

		// If the entity has been loaded from a packfile try and deallocate any internal zero size arrays in the entity.
		// If the arrays have a non-zero size the user will be warned.
		if (entity->m_memSizeAndFlags == 0)
		{
			entity->deallocateInternalArrays();
		}
		entity->removeReference();
	}
	allowCriticalOperations(true);

#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	{
		HK_INTERNAL_TIMER_SPLIT_LIST("Validate");
		if ( island ) { island->isValid(); }
	}
#	endif

	unlockAndAttemptToExecutePendingOperations();

	HK_INTERNAL_TIMER_END_LIST();
	return true;
}




void hkWorld::removeEntityBatch( hkEntity*const* entityBatch, int numEntities )
{
	if( numEntities < 1 )
	{
		return;
	}

	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::RemoveEntityBatch op;
		op.m_entities = const_cast<hkEntity**>(entityBatch);
		HK_ASSERT(0xf0ff0043, numEntities < HK_INVALID_OBJECT_INDEX);
		op.m_numEntities = (hkObjectIndex)numEntities;
		queueOperation( op );
		return;
	}

	lockCriticalOperations();

	HK_TIMER_BEGIN_LIST("RemEntities", "Init+CallBck");

	// Remove all TOI contact points before calling entity-removed callbacks
	m_simulation->resetCollisionInformationForEntities(const_cast<hkEntity**>(entityBatch), numEntities, this, true);

	// Remove collision agents via broadphase
	{
		hkLocalArray< hkBroadPhaseHandle* > collList(numEntities);
		{
			hkEntity*const* entity = entityBatch;
			hkEntity*const* entityEnd = entityBatch + numEntities;

			while( entity != entityEnd )
			{
				HK_ASSERT2(0xadb7d62a, *entity, "An HK_NULL found in the entity list for hkWorld::removeEntityBatch");
				HK_ASSERT2(0xadb7d62b, (*entity)->getWorld() == this, "Trying to remove an entity which does not belong to this hkWorld.");

				hkCollidable* c = (*entity)->getCollidableRw();
				if ( c->getShape() != HK_NULL )
				{
					collList.pushBackUnchecked( c->getBroadPhaseHandle() );
				}
				entity++;
			}
		}

		HK_TIMER_SPLIT_LIST("BroadPhase");
		hkLocalArray< hkBroadPhaseHandlePair > pairsOut( m_broadPhaseQuerySize );

		m_broadPhase->removeObjectBatch( collList, pairsOut );

		HK_TIMER_SPLIT_LIST("DelAgents");

		m_broadPhaseDispatcher->removePairs( static_cast<hkTypedBroadPhaseHandlePair*>(pairsOut.begin()), pairsOut.getSize() );
	}

	HK_TIMER_SPLIT_LIST("RemoveCb");
	{
		hkEntity*const* entity = entityBatch;
		hkEntity*const* entityEnd = entityBatch + numEntities;

		while( entity != entityEnd )
		{
			// World callbacks are called first (to allow access to the entity's constraints and actions (as they're removed in the entity callback)
			hkWorldCallbackUtil::fireEntityRemoved( this, *entity );
			hkEntityCallbackUtil::fireEntityRemoved( *entity );

			hkWorldOperationUtil::removeEntitySI(this, *entity);

			// If the entity has been loaded from a packfile try and deallocate any internal zero size arrays in the entity.
            // If the arrays have a non-zero size the user will be warned.
			if ( (*entity)->m_memSizeAndFlags == 0 )
			{
				(*entity)->deallocateInternalArrays();
			}
			(*entity)->removeReference();
			entity++;
		}
	}
	HK_TIMER_END_LIST();

	unlockAndAttemptToExecutePendingOperations();
}


void hkWorld::activateRegion( const hkAabb& aabb )
{
	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::ActivateRegion op;
		op.m_aabb = hkAllocateChunk<hkAabb>(1, HK_MEMORY_CLASS_DYNAMICS);
		hkString::memCpy(op.m_aabb, &aabb, sizeof(hkAabb));
		queueOperation( op );
		return;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	hkArray<hkBroadPhaseHandlePair> pairs;
	m_broadPhase->querySingleAabb( aabb, pairs );
	for (int i = 0; i < pairs.getSize(); i++)
	{
		HK_ASSERT2(0xf0ff0098, pairs[i].m_a == HK_NULL, "Internal check.");
		hkCollidable* coll = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pairs[i].m_b)->getOwner() );
		hkRigidBody*  body = hkGetRigidBody(coll);
		if (body)
		{
			body->activate();
		}
	}
}


hkConstraintInstance* hkWorld::addConstraint( hkConstraintInstance* constraint )
{
#if defined HK_DEBUG
	// Check if bodies attached to constraint might collide (arising in unwanted artifacts later).
	hkBool bodiesCollisionEnabledBeforeConstraintAdded = constrainedDynamicBodiesCanCollide( constraint );
#endif // #if defined HK_DEBUG

	HK_ASSERT2(0xad675544, (constraint->getData()->getType() < hkConstraintData::BEGIN_CONSTRAINT_CHAIN_TYPES) ^ (constraint->getType() == hkConstraintInstance::TYPE_CHAIN), "You're adding an inconsistent constraint which uses hkConstraintChainData+hkConstraintInstance or vice versa.");

	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::AddConstraint op;
		op.m_constraint = constraint;
		queueOperation( op );
		return HK_NULL;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	// Proceed whit the proper operation
	HK_ASSERT2(0xad000103, constraint->getOwner() == HK_NULL, "Error: you are trying to add a constraint, that has already been added to some world");

	// This is to allow loading of pre-3.3 assets.
	//
	// In 3.3 we have two problematic members of the hkBreakableConstraintData: m_childRuntimeSize and m_childNumSolverResults.
	//
	// Initialization of those members depends on a call to a virtual method of a different object, 
	// and we cannot do that safely in our current serialization framework neither at the time 
	// of converting assets nor in the finish-up constructor.
	//
	// Therefore we're doing that here.
	//
	if (constraint->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_BREAKABLE)
	{
		hkBreakableConstraintData* data = static_cast<hkBreakableConstraintData*>(constraint->getData());

		if (data->m_childRuntimeSize == 0)
		{
			hkConstraintData::RuntimeInfo info;
			data->m_constraintData->getRuntimeInfo( true, info );
			data->m_childRuntimeSize      = hkUint16(info.m_sizeOfExternalRuntime);
			data->m_childNumSolverResults = hkUint16(info.m_numSolverResults);
		}

	}


	hkConstraintInstance* result;

	blockExecutingPendingOperations(true);
	{
		// info: locking done in the hkWorldOperationUtil function
		constraint->pointNullsToFixedRigidBody();
		result = hkWorldOperationUtil::addConstraintImmediately(this, constraint);

		if ( constraint->getType() == hkConstraintInstance::TYPE_CHAIN )
		{
			// Adding constraint chain's action
			hkConstraintChainInstance* chain = static_cast<hkConstraintChainInstance*>(constraint);
				// if the constraint chain instance is part of a physics system, than the action
				// might also be part of that system and already be added to the world. So
				// we have to check whether the action is added already
			if (chain->m_action->getWorld() == HK_NULL)
			{
				addAction(chain->m_action);
			}


			HK_ASSERT2(0xad7877dd, chain->m_chainedEntities.getSize() - 1 <= chain->getData()->getNumConstraintInfos(), "hkConstraintChainInstance requires more constraintInfos than it has in its hkConstraintChainData (it has too many hkEntities).");
			HK_ASSERT2(0xad7877de, chain->m_chainedEntities.getSize() >= 2, "hkConstraintChainInstance has less than 2 chained bodies.");
			if (chain->m_chainedEntities.getSize() - 1 < chain->getData()->getNumConstraintInfos())
			{
				HK_WARN(0xad7877de, "hkConstraintChainInstance does not use all ConstraintInfos supplied in its hkConstralintChainData.");
			}
		}
	}
	blockExecutingPendingOperations(false);
	attemptToExecutePendingOperations();

#if defined HK_DEBUG
	// Check if bodies attached to constraint will collide (arising in unwanted artifacts later).
	hkBool bodiesCollisionEnabledAfterConstraintAdded = constrainedDynamicBodiesCanCollide( constraint );
	warnIfConstrainedDynamicBodiesCanCollide( constraint, bodiesCollisionEnabledBeforeConstraintAdded, bodiesCollisionEnabledAfterConstraintAdded );
#endif // #if defined HK_DEBUG

	return result;
}


hkConstraintInstance* hkWorld::createAndAddConstraintInstance( hkRigidBody* bodyA, hkRigidBody* bodyB, hkConstraintData* constraintData)
{
	hkConstraintInstance* constraint;

	constraint = new hkConstraintInstance( bodyA, bodyB, constraintData, hkConstraintInstance::PRIORITY_PSI );

	constraint->setUserData( constraintData->getUserData() );
	this->addConstraint( constraint);
	return constraint;
}


hkBool hkWorld::removeConstraint( hkConstraintInstance* constraint)
{
	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::RemoveConstraint op;
		op.m_constraint = constraint;
		queueOperation( op );
		return false;
	}

	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	// Proceed whit the proper operation
	HK_ASSERT2(0x6c6f226b, constraint->getOwner() && static_cast<hkSimulationIsland*>(constraint->getOwner())->getWorld() == this, "Trying to remove a constraint, that has not been added to the world or was added to a different world.");
	HK_ASSERT2(0Xad000114, constraint->getData()->getType() != hkConstraintData::CONSTRAINT_TYPE_CONTACT, "Error: trying to remove a constactConstraint which is owned and managed by agents only");

	lockCriticalOperations();
	{
		if ( constraint->getType() == hkConstraintInstance::TYPE_CHAIN )
		{
			// Adding constraint chain's action
			hkConstraintChainInstance* chain = static_cast<hkConstraintChainInstance*>(constraint);
			if (chain->m_action->getWorld() == this)
			{
				removeActionImmediately(chain->m_action);
			}
		}

		// info: locking done in the hkWorldOperationUtil function
		hkWorldOperationUtil::removeConstraintImmediately(this, constraint);
	}
	unlockAndAttemptToExecutePendingOperations();

	return true;
}

hkAction* hkWorld::addAction( hkAction* action )
{
	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::AddAction op;
		op.m_action = action;
		queueOperation( op );
		return HK_NULL;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	HK_ASSERT2(0XAD000108, action->getWorld() == HK_NULL, "Error: trying to add an action, that already has been added to some world");

	// Proceed whit the proper operation
	action->addReference();

	lockCriticalOperations();

	hkInplaceArray< hkEntity*, 4 > entities;
	action->getEntities( entities );
	action->setWorld( this );

	hkEntity* firstMovableEntity = HK_NULL;

	for (int i = 0; i < entities.getSize(); ++i)
	{
		HK_ASSERT2(0x3a26883f,  entities[i]->m_world == this, "Error: You tried to add an action which depends on Entities which are not added to the physics" );

		entities[i]->m_actions.pushBack( action );

		hkSimulationIsland* island = entities[i]->getSimulationIsland();
		if ( !island->isFixed() )
		{
			if ( firstMovableEntity == HK_NULL )
			{
				firstMovableEntity = entities[i];
				island->addAction(action);
			}
			else
			{
				// check to see if islands need to be merged
				if ( firstMovableEntity->getSimulationIsland() != entities[i]->getSimulationIsland() )
				{
					hkWorldOperationUtil::mergeIslands(this, firstMovableEntity, entities[i]);
				}
			}
		}
	}

	// When all entities are fixed, add the action to the fixed island
	if (firstMovableEntity == HK_NULL)
	{
		HK_ASSERT2(0xad34fe33, entities.getSize(), "You tried to add an action which has no entities specified.");
		entities[0]->getSimulationIsland()->addAction(action);
	}

	// Run pending operations before firing callbacks to make sure that all merge-island-requests are processed before other operations.
	unlockAndAttemptToExecutePendingOperations();

	hkWorldCallbackUtil::fireActionAdded( this, action );

	// This action might have already been removed in the above callback. Still though we assume the user has keeps a reference to 
	// it outside of this call, so we can safely return the pointer.

	return action;
}



void hkWorld::removeAction( hkAction* action )
{
	// Check if operation may be performed now
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::RemoveAction op;
		op.m_action = action;
		queueOperation( op );
		return;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	HK_ASSERT2(0xad000107, action->getWorld() == this, "Error: removing an action that already has been removed from the world (note: it may still be hanging on the actionList of some entity, as it is only removed form it in actionRemovedCallbacks. And those callbacks are not ordered.)");

	// Proceed whit the proper operation
	removeActionImmediately(action);
}

void hkWorld::removeActionImmediately( hkAction* action )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0xad000600, this == action->getWorld(), "Removing an action which is not in this world.");
	// Add temporary reference
	action->addReference();

	lockCriticalOperations();
	// Fire callbacksk while the action is still in the this
	hkWorldCallbackUtil::fireActionRemoved( this, action );

	//  TODO clear comments:
	//  Not needed with locking: 
	//	// The action might already have been removed from withing the callbacks, therefore proceed only if it still is in the this.
	//	if (action->getWorld() == this)
	{
		hkInplaceArray< hkEntity*, 4 > entities;
		action->getEntities( entities );
		for (int i = 0; i < entities.getSize(); ++i)
		{
			HK_ASSERT2(0xad000220, entities[i]->getWorld() == this, "Error: action being removed is attached to an entity not insterted into its world.");
			//detachActionFromEntity(action, entities[i]);
			int idx = entities[i]->m_actions.indexOf(action);
			HK_ASSERT2(0xad000240, idx >= 0, "You tried to remove an action that was never added while removing an action" );
			entities[i]->m_actions.removeAt(idx);
		}

	 	hkSimulationIsland* island = action->getSimulationIsland();
		island->removeAction( action );
		action->setWorld( HK_NULL );
		action->removeReference();

		hkWorldOperationUtil::putIslandOnDirtyList(island->getWorld(), island);
	}
	
	unlockAndAttemptToExecutePendingOperations();

	// Remove temporary reference
	action->removeReference();
}

void hkWorld::attachActionToEntity(hkAction* action, hkEntity* entity)
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0xad000230, action->getWorld(), "Error: this function is only meant to be called from action, and only when they are inserted into the world");
	//if (action->getWorld())
	{
		HK_ASSERT2(0xad000221, entity->getWorld() == this, "Error: attaching an entity not inserted into the world");
		HK_ASSERT2(0xad000222, entity->m_actions.indexOf(action) < 0 , "Error: You tried to add the same action twice");
		entity->m_actions.pushBack( action );

		if (action->getSimulationIsland()->isFixed() && !entity->isFixed())
		{
			action->getSimulationIsland()->removeAction(action);
			entity->getSimulationIsland()->addAction(action);
		}
		else if ( entity->getSimulationIsland() != action->getSimulationIsland() && !entity->isFixed() )
		{
			// HACK: taking an arbitrary entity form action's island -- if it get's deleted before the merge is performed, the 
			//       islands may not get merged.
			hkWorldOperationUtil::mergeIslands(this, entity, action->getSimulationIsland()->m_entities[0]);
		}

	}

}

void hkWorld::detachActionFromEntity(hkAction* action, hkEntity* entity)
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0xad000230, action->getWorld(), "Error: this function is only meant to be called from action, and only when they are inserted into the world");
	//if (action->getWorld())
	{
		HK_ASSERT2(0xad000223, entity->getWorld() == this, "Error: detaching from entity not inserted to the world");
		
		int idx = entity->m_actions.indexOf( action );
		HK_ASSERT2(0x3ef53a57, idx >= 0, "You tried to remove an action that was never added" );
		entity->m_actions.removeAt(idx);

		entity->getSimulationIsland()->m_splitCheckRequested = true;

		//
		// And now find a valid island for the action
		//
		hkInplaceArray< hkEntity*, 4 > entities;
		action->getEntities( entities );
		hkSimulationIsland* newIsland = HK_NULL;
		for (int i = 0; i < entities.getSize(); ++i)
		{
			if (entities[i] != entity)
			{
				newIsland = entities[i]->getSimulationIsland();
				if (!newIsland->isFixed())
				{
					break;
				}
			}
		}

		if (newIsland != action->getSimulationIsland())
		{
			action->getSimulationIsland()->removeAction(action);
			newIsland->addAction(action);
		}

	}
}

hkPhantom* hkWorld::addPhantom( hkPhantom* phantom )
{
	HK_ASSERT2(0x13c74a8e,  phantom, "Cannot pass an HK_NULL as parameter to hkWorld::addPhantom");

	// Check if operation may be performed now
	if (areCriticalOperationsLockedForPhantoms())
	{
		hkWorldOperation::AddPhantom op;
		op.m_phantom = phantom;
		queueOperation( op );
		return HK_NULL;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	//
	// Proceed whit the proper operation
	//

	HK_ASSERT2(0x13c74a8e,  phantom->getWorld() == HK_NULL, "Trying to add a phantom to a hkWorld twice");

	lockCriticalOperations();
	
	// check if the collidable back ptr to the motion state is set
	// as it may not be due to packedfile serialization
	if (!phantom->m_collidable.getMotionState())
	{
		phantom->m_collidable.setMotionState( phantom->getMotionState() );
	}

	phantom->setWorld( this );
	phantom->addReference();
	m_phantoms.pushBack( phantom );

	hkWorldOperationUtil::addPhantomBP(this, phantom);

	//disable + execute here ?

	hkWorldCallbackUtil::firePhantomAdded( this, phantom );
	phantom->firePhantomAdded();

	unlockAndAttemptToExecutePendingOperations();

	return phantom;
}



void hkWorld::addPhantomBatch( hkPhantom*const* phantomBatch, int numPhantoms )
{
	// Check if operation may be performed now
	if (areCriticalOperationsLockedForPhantoms())
	{
		hkWorldOperation::AddPhantomBatch op;
		op.m_phantoms = const_cast<hkPhantom**>(phantomBatch);
		op.m_numPhantoms = hkObjectIndex(numPhantoms);
		queueOperation( op );
		return;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	lockCriticalOperations();

	hkLocalArray< hkBroadPhaseHandle* > collList(numPhantoms);
	hkLocalArray< hkAabb > aabbList(numPhantoms);

	aabbList.setSizeUnchecked( numPhantoms );
	collList.setSizeUnchecked( numPhantoms );

	for( int i = 0; i < numPhantoms; i++ )
	{
		hkPhantom* phantom = phantomBatch[i];

		HK_ASSERT2(0xad87bc8a, phantom, "An HK_NULL found in a phantom batch.");
		HK_ASSERT2(0xad87bc89, phantom->getWorld() == HK_NULL, "Trying to add a phantom which already belongs to an hkWorld.");

		if (!phantom->getCollidable()->getMotionState()) // may be null due to packfile serialize
		{
			phantom->m_collidable.setMotionState( phantom->getMotionState() );
		}

		phantom->setWorld( this );

		collList[i] = ( phantom->getCollidableRw()->getBroadPhaseHandle() );
		phantom->calcAabb( aabbList[i] );

		phantom->addReference();

		m_phantoms.pushBack( phantom );
		hkWorldCallbackUtil::firePhantomAdded( this, phantom );
		phantom->firePhantomAdded();
	}

	hkLocalArray< hkBroadPhaseHandlePair > newPairs( m_broadPhaseQuerySize );


	m_broadPhase->addObjectBatch( collList, aabbList, newPairs );

	// check for changes
	m_broadPhaseDispatcher->addPairs( static_cast<hkTypedBroadPhaseHandlePair*>(newPairs.begin()), newPairs.getSize(), getCollisionFilter() );

	unlockAndAttemptToExecutePendingOperations();

}



void hkWorld::removePhantom( hkPhantom* phantom )
{
	HK_ASSERT2(0x13c74a8f,  phantom, "Cannot pass an HK_NULL as parameter to hkWorld::removePhantom");

	// Check if operation may be performed now
	if (areCriticalOperationsLockedForPhantoms())
	{
		hkWorldOperation::RemovePhantom op;
		op.m_phantom = phantom;
		queueOperation( op );
		return;
	}

	//
	// Proceed whit the proper operation
	//

	HK_ASSERT2(0x627789e0,  phantom->getWorld() == this, "Trying to remove a phantom from a hkWorld to which it was not added");
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	lockCriticalOperations();

	//
	//	fire callbacks
	//
	hkWorldCallbackUtil::firePhantomRemoved( this, phantom );
	phantom->firePhantomRemoved();

	hkWorldOperationUtil::removePhantomBP(this, phantom);

	//
	//	remove phantom from list
	//
	m_phantoms.removeAt(m_phantoms.indexOf( phantom ) );
	phantom->setWorld( HK_NULL );
	
	// If the entity has been loaded from a packfile try and deallocate any internal zero size arrays in the phantom.
    // If the arrays have a non-zero size the user will be warned.
	if ( phantom->m_memSizeAndFlags == 0 )
	{
		phantom->deallocateInternalArrays();
	}
	phantom->removeReference();

	unlockAndAttemptToExecutePendingOperations();
}



void hkWorld::removePhantomBatch( hkPhantom*const* phantomBatch, int numPhantoms )
{
//	HK_ASSERT2(0xf0ff009c, !areCriticalOperationsLocked(), "Error: removing phantoms is not allowed when the world is locked (it might be safe, but also might cause problems if you change BroadPhase from an add/removeEntity, for example.)");

	if (areCriticalOperationsLockedForPhantoms())
	{
		hkWorldOperation::RemovePhantomBatch op;
		op.m_phantoms = const_cast<hkPhantom**>(phantomBatch);
		op.m_numPhantoms = hkObjectIndex(numPhantoms);
		queueOperation( op );
		return;
	}
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	lockCriticalOperations();

	//
	//	fire callbacks
	//
	hkLocalArray< hkBroadPhaseHandle* > collList(numPhantoms);
	{
		for( int i = 0; i < numPhantoms; i++ )
		{
			hkPhantom* phantom = phantomBatch[i];
			HK_ASSERT2(0xad87bc87, phantom, "An HK_NULL pointer found in phantom batch.");
			HK_ASSERT2(0xad87bc88, phantom->getWorld() == this, "Trying to remove a phantom which does not belong to this hkWorld.");
			collList.pushBackUnchecked( phantom->getCollidableRw()->getBroadPhaseHandle() );
			hkWorldCallbackUtil::firePhantomRemoved( this, phantom );
			phantom->firePhantomRemoved();
		}
	}
	
	//
	//	remove pairs
	//
	{
		hkLocalArray< hkBroadPhaseHandlePair > removedPairs( m_broadPhaseQuerySize );
		m_broadPhase->removeObjectBatch( collList, removedPairs );
		m_broadPhaseDispatcher->removePairs( static_cast<hkTypedBroadPhaseHandlePair*>(removedPairs.begin()), removedPairs.getSize() );
	}

	//
	// remove phantom from phantom list
	//
	{
		for( int i = 0; i < numPhantoms; i++ )
		{
			hkPhantom* phantom = phantomBatch[i];
			phantom->setWorld( HK_NULL );
			m_phantoms.removeAt(m_phantoms.indexOf( phantom ) );
			// If the entity has been loaded from a packfile try and deallocate any internal zero size arrays in the phantom
            // If the arrays have a non-zero size the user will be warned.
			if ( phantom->m_memSizeAndFlags == 0 )
			{
				phantom->deallocateInternalArrays();
			}
			phantom->removeReference();
		}
	}

	unlockAndAttemptToExecutePendingOperations();
}

void hkWorld::addPhysicsSystem( const hkPhysicsSystem* sys )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	// rbs & phantoms (batch added)
	if(sys->getRigidBodies().getSize() > 0)
	{
		addEntityBatch( (hkEntity*const*)( sys->getRigidBodies().begin() ), sys->getRigidBodies().getSize(),
				sys->isActive() ? HK_ENTITY_ACTIVATION_DO_ACTIVATE : HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE ); 
	}
	if(sys->getPhantoms().getSize() > 0)
	{
		addPhantomBatch( sys->getPhantoms().begin(), sys->getPhantoms().getSize() ); 
	}

	// actions & constraints -- in this order, as the constraint chains also add their 'instance actions' independently.
	for (int a=0; a < sys->getActions().getSize(); ++a)
	{
		// allow null actions for now
		if (sys->getActions()[a])		
		{
			addAction(sys->getActions()[a]);		
		}
	}
	for (int c=0; c < sys->getConstraints().getSize(); ++c)
	{
		addConstraint(sys->getConstraints()[c]);		
	}
}

void hkWorld::removePhysicsSystem( const hkPhysicsSystem* sys )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	// contraints & actions
	for (int c=0; c < sys->getConstraints().getSize(); ++c)
	{
		removeConstraint(sys->getConstraints()[c]);		
	}
	for (int a=0; a < sys->getActions().getSize(); ++a)
	{
		removeAction(sys->getActions()[a]);		
	}

	// Entities (rigid bodies)
	int threshold = 4;
	int numEntities = sys->getRigidBodies().getSize();
	if ( numEntities < threshold )
	{
		for (int e=0; e < sys->getRigidBodies().getSize(); ++e)
		{
			removeEntity(sys->getRigidBodies()[e]);
		}
	}
	else
	{
		removeEntityBatch( reinterpret_cast<hkEntity*const*>(sys->getRigidBodies().begin()), sys->getRigidBodies().getSize() );
	}

	// phantoms (batch added)
	if(sys->getPhantoms().getSize() > 0)
	{
		removePhantomBatch( sys->getPhantoms().begin(), sys->getPhantoms().getSize() ); 
	}
}

	//
	// Gravity (convenience)
	//

void hkWorld::setGravity( const hkVector4& gravity )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );

	// We are not waking up objects
	m_gravity = gravity;
}


//
// Listener registration
//

void hkWorld::addActionListener( hkActionListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x7d1e9387, m_actionListeners.indexOf(worldListener) < 0, "You tried to add a world action listener twice" );

	m_actionListeners.pushBack( worldListener );
}

void hkWorld::removeActionListener( hkActionListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_actionListeners.indexOf(worldListener);
	HK_ASSERT2(0x52e10e50, i >= 0, "You tried to remove a world action listener, which was never added" );
	m_actionListeners[i] = HK_NULL;
}


void hkWorld::addConstraintListener( hkConstraintListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x1a5b73b6, m_constraintListeners.indexOf( worldListener ) < 0, "You tried to add a world constraint listener twice" );

	m_constraintListeners.pushBack( worldListener );
}

void hkWorld::removeConstraintListener( hkConstraintListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_constraintListeners.indexOf( worldListener );
	HK_ASSERT2(0x14e7d731, i >= 0, "You tried to remove a world constraint listener, which was never added" );
	m_constraintListeners[i] = HK_NULL;
}

void hkWorld::addEntityListener( hkEntityListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x41fecf63, m_entityListeners.indexOf(worldListener) < 0, "You tried to add a world entity listener twice" );
	m_entityListeners.pushBack( worldListener );
}

void hkWorld::removeEntityListener( hkEntityListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_entityListeners.indexOf(worldListener);
	HK_ASSERT2(0x7e5dcf64, i >= 0, "You tried to remove a world entity listener, which was never added" );
	m_entityListeners[i] = HK_NULL;
}

void hkWorld::addPhantomListener( hkPhantomListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x4aa03aaf, m_phantomListeners.indexOf(worldListener) < 0, "You tried to add a world entity listener twice" );
	m_phantomListeners.pushBack( worldListener );
}

void hkWorld::removePhantomListener( hkPhantomListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_phantomListeners.indexOf(worldListener);
	HK_ASSERT2(0x25ce777c, i >= 0, "You tried to remove a hkPhantomListener, which was never added" );
	m_phantomListeners[i] = HK_NULL;
}

void hkWorld::addIslandActivationListener( hkIslandActivationListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x1a14bf93, m_islandActivationListeners.indexOf(worldListener) < 0, "You tried to add a world activation listener twice" );
	m_islandActivationListeners.pushBack( worldListener );
}

void hkWorld::removeIslandActivationListener( hkIslandActivationListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_islandActivationListeners.indexOf( worldListener );
	HK_ASSERT2(0x137408f4, i >= 0, "You tried to remove a world activation listener, which was never added" );
	m_islandActivationListeners[i] = HK_NULL;
}

void hkWorld::addWorldPostCollideListener( hkWorldPostCollideListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x63a352b1, m_worldPostCollideListeners.indexOf(worldListener) < 0, "You tried to add a world post detection listener twice" );
	m_worldPostCollideListeners.pushBack( worldListener );
}

void hkWorld::removeWorldPostCollideListener( hkWorldPostCollideListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_worldPostCollideListeners.indexOf(worldListener);
	HK_ASSERT2(0x67c333b0, i >= 0, "You tried to remove a world post detection listener, which was never added" );
	m_worldPostCollideListeners[i] = HK_NULL;
}


void hkWorld::addWorldPostSimulationListener( hkWorldPostSimulationListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x619dae7f, m_worldPostSimulationListeners.indexOf(worldListener) < 0, "You tried to add a world post simulation listener twice" );
	m_worldPostSimulationListeners.pushBack( worldListener );
}

void hkWorld::removeWorldPostSimulationListener( hkWorldPostSimulationListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_worldPostSimulationListeners.indexOf(worldListener);
	HK_ASSERT2(0x5eb3cb29, i >= 0, "You tried to remove a world post simulation listener, which was never added" );
	m_worldPostSimulationListeners[i] = HK_NULL;
}

void hkWorld::addWorldPostIntegrateListener( hkWorldPostIntegrateListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x619dae7f, m_worldPostIntegrateListeners.indexOf(worldListener) < 0, "You tried to add a world post simulation listener twice" );
	m_worldPostIntegrateListeners.pushBack( worldListener );
}

void hkWorld::removeWorldPostIntegrateListener( hkWorldPostIntegrateListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_worldPostIntegrateListeners.indexOf(worldListener);
	HK_ASSERT2(0x5eb3cb29, i >= 0, "You tried to remove a world post simulation listener, which was never added" );
	m_worldPostIntegrateListeners[i] = HK_NULL;
}


void hkWorld::addIslandPostCollideListener( hkIslandPostCollideListener* islandListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x5c983e76, m_islandPostCollideListeners.indexOf(islandListener) < 0, "You tried to add a island post detection listener twice" );
	m_islandPostCollideListeners.pushBack( islandListener );
}

void hkWorld::removeIslandPostCollideListener( hkIslandPostCollideListener* islandListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_islandPostCollideListeners.indexOf(islandListener);
	HK_ASSERT2(0x60701a8c, i >= 0, "You tried to remove a island post detection listener, which was never added" );
	m_islandPostCollideListeners[i] = HK_NULL;
}


void hkWorld::addIslandPostIntegrateListener( hkIslandPostIntegrateListener* islandListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x4654251a, m_islandPostIntegrateListeners.indexOf(islandListener) < 0, "You tried to add a island post simulation listener twice" );
	m_islandPostIntegrateListeners.pushBack( islandListener );
}

void hkWorld::removeIslandPostIntegrateListener( hkIslandPostIntegrateListener* islandListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_islandPostIntegrateListeners.indexOf(islandListener);
	HK_ASSERT2(0x143fdef2, i >= 0, "You tried to remove a island post simulation listener, which was never added" );
	m_islandPostIntegrateListeners[i] = HK_NULL;
}


void hkWorld::addCollisionListener( hkCollisionListener* collisionListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x1debcc37, m_collisionListeners.indexOf(collisionListener) < 0, "You tried to add a world collision listener twice" );
	m_collisionListeners.pushBack( collisionListener );
}


void hkWorld::removeCollisionListener( hkCollisionListener* collisionListener)
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_collisionListeners.indexOf(collisionListener);
	HK_ASSERT2(0x6c3fe017, i >= 0, "You tried to remove a world collision listener, which was never added" );
	m_collisionListeners[i] = HK_NULL;
}


void hkWorld::addWorldDeletionListener( hkWorldDeletionListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0x2ad1617f, m_worldDeletionListeners.indexOf( worldListener ) < 0, "You tried to add a world deletion listener twice" );
	m_worldDeletionListeners.pushBack( worldListener );
}

void hkWorld::removeWorldDeletionListener( hkWorldDeletionListener* worldListener )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	int i = m_worldDeletionListeners.indexOf( worldListener );
	HK_ASSERT2(0x12f005e2, i >= 0, "You tried to remove a world deletion listener, which was never added" );
	m_worldDeletionListeners[i] = HK_NULL;
}


void hkWorld::setHighFrequencyDeactivationPeriod( hkReal period )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	m_highFrequencyDeactivationPeriod = period;
}

void hkWorld::setLowFrequencyDeactivationPeriod( hkReal period )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	m_lowFrequencyDeactivationPeriod = period;
}

hkStepResult hkWorld::stepDeltaTime( hkReal physicsDeltaTime )
{
	hkStepResult result = m_simulation->stepDeltaTime( physicsDeltaTime );

	if ( ( result == HK_STEP_RESULT_SUCCESS ) && ( m_memoryWatchDog != HK_NULL ) )
	{
		m_memoryWatchDog->watchMemory( this );
	}

	return result;
}


hkStepResult hkWorld::integrate( hkReal physicsDeltaTime )
{
	return m_simulation->integrate( physicsDeltaTime );
}

hkStepResult hkWorld::collide()
{
	return m_simulation->collide();
}

hkStepResult hkWorld::advanceTime()
{
	hkStepResult result = m_simulation->advanceTime();

	if ( ( result == HK_STEP_RESULT_SUCCESS ) && ( m_memoryWatchDog != HK_NULL ) )
	{
		m_memoryWatchDog->watchMemory( this );
	}

	return result;
}


void hkWorld::setFrameTimeMarker( hkReal frameDeltaTime )
{
	m_simulation->setFrameTimeMarker( frameDeltaTime );
}

bool hkWorld::isSimulationAtMarker() const
{
	return m_simulation->isSimulationAtMarker();
}

bool hkWorld::isSimulationAtPsi() const
{
	return m_simulation->isSimulationAtPsi( );
}

#if defined(HK_PLATFORM_MULTI_THREAD)
void hkWorld::stepBeginSt( hkReal frameDeltaTime )
{
	HK_ASSERT2( 0x1298af35, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED, "You cannot call this function on the world if you are not using a multithreaded simulation.");
	static_cast<hkMultiThreadedSimulation*>(m_simulation)->stepBeginSt( frameDeltaTime );
}


void hkWorld::stepProcessMt( const hkThreadToken& token )
{
	HK_ASSERT2( 0x1298af35, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED, "You cannot call this function on the world if you are not using a multithreaded simulation.");
	static_cast<hkMultiThreadedSimulation*>(m_simulation)->stepProcessMt( token );
}

void hkWorld::stepEndSt()
{
	HK_ASSERT2( 0x1298af35, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED, "You cannot call this function on the world if you are not using a multithreaded simulation.");
	static_cast<hkMultiThreadedSimulation*>(m_simulation)->stepEndSt( );
}


hkThreadToken hkWorld::getThreadToken()
{
	HK_ASSERT2( 0x1298af35, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED, "You cannot call this function on the world if you are not using a multithreaded simulation.");
	return static_cast<hkMultiThreadedSimulation*>(m_simulation)->getThreadToken( );
}
void hkWorld::resetThreadTokens()
{
	HK_ASSERT2( 0x1298af35, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED, "You cannot call this function on the world if you are not using a multithreaded simulation.");
	return static_cast<hkMultiThreadedSimulation*>(m_simulation)->resetThreadTokens( );
}


hkJobQueue* hkWorld::getJobQueue()
{
	HK_ASSERT2(0x192fa846, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED, "This function can only be called for a multithreaded simulation");
	return static_cast<hkMultiThreadedSimulation*>(m_simulation)->getJobQueue();
}


void hkWorld::getMultithreadConfig( hkMultithreadConfig& config )
{
	HK_ASSERT2(0x192fa846, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED, "This function can only be called for a multithreaded simulation");
	static_cast<hkMultiThreadedSimulation*>(m_simulation)->getMultithreadConfig( config );
}

void hkWorld::setMultithreadConfig( const hkMultithreadConfig& config )
{
	HK_ASSERT2(0x192fa846, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED, "This function can only be called for a multithreaded simulation");
	static_cast<hkMultiThreadedSimulation*>(m_simulation)->setMultithreadConfig( config );
}
#else
void hkWorld::stepBeginSt( hkReal frameDeltaTime ){			HK_ASSERT2( 0xf032ed45, false, "Your plattform does not support multithreaded simulation");	}
void hkWorld::stepProcessMt( const hkThreadToken& token ){	HK_ASSERT2( 0xf032ed46, false, "Your plattform does not support multithreaded simulation");	}
void hkWorld::stepEndSt(){									HK_ASSERT2( 0xf032ed47, false, "Your plattform does not support multithreaded simulation");	}
hkThreadToken hkWorld::getThreadToken(){					HK_ASSERT2( 0xf032ed48, false, "Your plattform does not support multithreaded simulation");	return HK_THREAD_TOKEN_PRIMARY; }
void hkWorld::resetThreadTokens(){							HK_ASSERT2( 0xf032ed49, false, "Your plattform does not support multithreaded simulation");	}
hkJobQueue* hkWorld::getJobQueue(){							HK_ASSERT2( 0xf032ed4a, false, "Your plattform does not support multithreaded simulation");	return HK_NULL; }
void hkWorld::getMultithreadConfig( hkMultithreadConfig& config ){ HK_ASSERT2( 0xf032ed4b, false, "Your plattform does not support multithreaded simulation");	}
void hkWorld::setMultithreadConfig( const hkMultithreadConfig& config ){ HK_ASSERT2( 0xf032ed4c, false, "Your plattform does not support multithreaded simulation");	}
#endif

void hkWorld::calcStatistics( hkStatisticsCollector* collector ) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	collector->beginObject("hkWorld", collector->MEMORY_INSTANCE, this);

	{
		const hkArray<hkSimulationIsland*>& activeIslands = getActiveSimulationIslands();

		for (int i = activeIslands.getSize()-1; i>=0; i--)
		{
			hkSimulationIsland* island = activeIslands[i];
			collector->addChildObject( "Active", collector->MEMORY_RUNTIME, island );
		}
	}
	{
		for (int i = m_inactiveSimulationIslands.getSize()-1; i>=0; i--)
		{
			hkSimulationIsland* island = m_inactiveSimulationIslands[i];
			collector->addChildObject( "Inactive", collector->MEMORY_RUNTIME, island );
		}
	}
	collector->addChildObject( "Fixed", collector->MEMORY_RUNTIME, m_fixedIsland );

	// Phantoms
	{
		collector->addArray("PhantomPtrs", collector->MEMORY_ENGINE, m_phantoms);
		for (int i = 0; i < m_phantoms.getSize(); i++)
		{
			collector->addChildObject("Phantoms", collector->MEMORY_INSTANCE, m_phantoms[i]);
		}
	}



	// Listeners


	// world operation queue, maintenance ....

	collector->pushDir("Internal");
	{
		collector->addChildObject("Simulation", collector->MEMORY_ENGINE, m_simulation);
		collector->addArray("DirtyIslPtr", collector->MEMORY_RUNTIME, m_dirtySimulationIslands);
		collector->addChildObject("MaintnceMgr", collector->MEMORY_ENGINE, m_maintenanceMgr);

		collector->addChunk("OpQueue", collector->MEMORY_ENGINE, m_pendingOperations, sizeof(*m_pendingOperations));
		collector->pushDir("OpQueue");
			collector->addArray("Operations", collector->MEMORY_RUNTIME, m_pendingOperations->m_pending);
		collector->popDir();

		collector->pushDir("Collide");
		{
			collector->addChildObject("Broadphase", collector->MEMORY_ENGINE, getBroadPhase() );
			collector->addChunk("BpDispatch", collector->MEMORY_ENGINE, m_broadPhaseDispatcher, sizeof(*m_broadPhaseDispatcher));
			collector->addChunk("BpDispatch", collector->MEMORY_ENGINE, m_entityEntityBroadPhaseListener, sizeof(*m_entityEntityBroadPhaseListener));
			collector->addChunk("BpDispatch", collector->MEMORY_ENGINE, m_phantomBroadPhaseListener, sizeof(*m_phantomBroadPhaseListener));
			collector->addChunk("BpDispatch", collector->MEMORY_ENGINE, m_broadPhaseBorderListener, sizeof(*m_broadPhaseBorderListener));

			collector->addChunk("CollInput", collector->MEMORY_ENGINE, m_collisionInput, sizeof(*m_collisionInput));
			collector->addChildObject("Filter", collector->MEMORY_SHARED, m_collisionFilter);
			collector->addChildObject("Dispatcher", collector->MEMORY_ENGINE, m_collisionDispatcher);
		}
		collector->popDir();

	}
	collector->popDir();

	//collector->addChildObject("WldOpQueInf", collector->MEMORY_RUNTIME, m_pendingOperationQueues);

#define ADD_ARRAY_STATISTICS(arrayName, elemName, varName) \
		collector->addArray(arrayName, collector->MEMORY_ENGINE, varName);

/*		for (int i = 0; i < m_entityListeners.getSize(); i++)\
//		{\
//			collector->addChildObject(elemName, collector->MEMORY_SHARED, varName[i]);\
//		}\
//	}*/

	collector->pushDir("Listeners");
	ADD_ARRAY_STATISTICS("EntLisPtr",    "EntityLis",    m_entityListeners);
	ADD_ARRAY_STATISTICS("PhantLisPtr",  "PhantomLis",   m_phantomListeners);
	ADD_ARRAY_STATISTICS("ConstrLisPtr", "ConstrLis",    m_constraintListeners);

	ADD_ARRAY_STATISTICS("WldDelLisPtr", "WrldDelLis",   m_worldDeletionListeners);
	ADD_ARRAY_STATISTICS("IslActLisPtr", "IsldActLis",   m_islandActivationListeners);
	ADD_ARRAY_STATISTICS("PstSimLisPtr", "PostSimLis",   m_worldPostSimulationListeners);
	ADD_ARRAY_STATISTICS("PstIntLisPtr", "PostIntLis",   m_worldPostIntegrateListeners);
	ADD_ARRAY_STATISTICS("PstColLisPtr", "PostColLis",   m_worldPostCollideListeners);
	ADD_ARRAY_STATISTICS("PstIntLisPtr", "PostIntLis",   m_islandPostIntegrateListeners);

	ADD_ARRAY_STATISTICS("CollLisPtr",   "CollisionLis", m_collisionListeners);
	collector->popDir();

#undef ADD_ARRAY_STATISTICS
	collector->endObject();

}


void hkWorld::checkDeterminism()
{
#if HK_CHECK_DETERMINISM_ENABLE_MT_CHECKS || HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS
	hkArray<const hkSimulationIsland*> islands;
	{
		// for all motions: check with loaded data
		{
			islands.pushBack(this->getFixedIsland());
			const hkArray<hkSimulationIsland*>& activeIslands = getActiveSimulationIslands();
			for (int i = 0; i < activeIslands.getSize(); i++)
			{
				islands.pushBack(activeIslands[i]);
			}
			for (int j = 0; j < this->m_inactiveSimulationIslands.getSize(); j++)
			{
				islands.pushBack(this->m_inactiveSimulationIslands[j]);
			}
		}
	}

	//
	//	Check entity specific data
	//
	{
		for (int i = 0; i < islands.getSize(); i++)
		{
			const hkSimulationIsland& island = *islands[i];
			hkCheckDeterminismUtil::checkSt( island.getEntities().getSize() );

			for (int e = 0; e < island.m_entities.getSize(); e++)
			{
				hkRigidBody& body = static_cast<hkRigidBody&>(*island.m_entities[e]);
				{
					hkCheckDeterminismUtil::checkSt( body.getUid() );
				}

					//
					//	Motions
					//
				{
					hkMotion* motion = body.getRigidMotion();
					hkCheckDeterminismUtil::checkSt(&motion->getPosition(),1);
					hkCheckDeterminismUtil::checkSt(&motion->getRotation().m_vec,1);
					hkCheckDeterminismUtil::checkSt(&motion->getLinearVelocity(),1);
					hkCheckDeterminismUtil::checkSt(&motion->getAngularVelocity(),1);
				}
			}
		}
	}

	//
	//	Check collision information
	//
	if(0)
	{
		for (int i = 0; i < islands.getSize(); i++)
		{
			const hkSimulationIsland* island = islands[i];
			{
				hkCheckDeterminismUtil::checkSt(island->m_agentTrack.m_sectors.getSize());
				HK_ACCESS_CHECK_WITH_PARENT( island->m_world, HK_ACCESS_RO, island, HK_ACCESS_RW );
				HK_FOR_ALL_AGENT_ENTRIES_BEGIN(island->m_agentTrack, entry)
				{
					int uidA = hkGetRigidBody(entry->getCollidableA())->getUid();
					int uidB = hkGetRigidBody(entry->getCollidableB())->getUid();
					hkCheckDeterminismUtil::checkSt( uidA );
					hkCheckDeterminismUtil::checkSt( uidB );
					hkCheckDeterminismUtil::checkSt( &entry->m_agentIndexOnCollidable[0],2 );
					hkCheckDeterminismUtil::checkSt( entry->m_agentType );
					hkCheckDeterminismUtil::checkSt( entry->m_numContactPoints );
					hkSimpleConstraintContactMgr* mgr = (hkSimpleConstraintContactMgr*)entry->m_contactMgr;
					if ( mgr->getConstraintInstance() )
					{
						hkSimpleContactConstraintAtom* atoms = mgr->m_contactConstraintData.m_atom;
						hkCheckDeterminismUtil::checkSt( (char*)atoms, atoms->m_sizeOfAllAtoms );
					}
				}
				HK_FOR_ALL_AGENT_ENTRIES_END;
			}
		}
	}
#endif //HK_CHECK_DETERMINISM_ENABLE_MT_CHECKS || HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS

}



void hkWorld::getCinfo(hkWorldCinfo& info) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	// NOTE: The order of these variables matches the order in the world Cinfo. Please keep them in sync
	// if you are making changes.

	// Basic setup
	info.m_gravity = m_gravity;
	info.m_broadPhaseQuerySize = m_broadPhaseQuerySize;
	info.m_broadPhaseWorldAabb.m_min  = m_broadPhaseExtents[0];
	info.m_broadPhaseWorldAabb.m_max  = m_broadPhaseExtents[1];
	info.m_collisionTolerance = m_collisionInput->getTolerance();
	info.m_collisionFilter = m_collisionFilter;
	info.m_convexListFilter = m_convexListFilter;

	info.m_broadPhaseBorderBehaviour = (m_broadPhaseBorder)? m_broadPhaseBorder->m_type : hkWorldCinfo::BROADPHASE_BORDER_DO_NOTHING;

	info.m_expectedMaxLinearVelocity = m_collisionDispatcher->m_expectedMaxLinearVelocity;
	info.m_expectedMinPsiDeltaTime   = m_collisionDispatcher->m_expectedMinPsiDeltaTime;


	info.m_memoryWatchDog = m_memoryWatchDog;

	// Optimizations
	info.m_broadPhaseNumMarkers = m_broadPhaseNumMarkers;
	info.m_contactPointGeneration = m_contactPointGeneration;
	info.m_contactRestingVelocity = m_dynamicsStepInfo.m_solverInfo.m_contactRestingVelocity;

	// Solver Settings
	info.m_solverTau = m_dynamicsStepInfo.m_solverInfo.m_tau;
	info.m_solverDamp = m_dynamicsStepInfo.m_solverInfo.m_damping;
	info.m_solverIterations = m_dynamicsStepInfo.m_solverInfo.m_numSteps;
	info.m_solverMicrosteps = m_dynamicsStepInfo.m_solverInfo.m_numMicroSteps;

	
	// Internal algorithm settings
	info.m_iterativeLinearCastEarlyOutDistance = m_collisionInput->m_config->m_iterativeLinearCastEarlyOutDistance;
	info.m_iterativeLinearCastMaxIterations = m_collisionInput->m_config->m_iterativeLinearCastMaxIterations;
	info.m_highFrequencyDeactivationPeriod = m_highFrequencyDeactivationPeriod;
	info.m_lowFrequencyDeactivationPeriod = m_lowFrequencyDeactivationPeriod;
	info.m_shouldActivateOnRigidBodyTransformChange = m_shouldActivateOnRigidBodyTransformChange;
	info.m_wantOldStyleDeactivation            = m_wantOldStyleDeactivation;
	info.m_toiCollisionResponseRotateNormal    = m_toiCollisionResponseRotateNormal;
	info.m_deactivationReferenceDistance       = m_deactivationReferenceDistance;

	// Debugging flags
	info.m_enableDeactivation = m_wantDeactivation;
	info.m_simulationType = m_simulationType;
	info.m_frameMarkerPsiSnap = m_simulation->m_frameMarkerPsiSnap;

	info.m_enableSimulationIslands = m_wantSimulationIslands;
	info.m_processActionsInSingleThread = m_processActionsInSingleThread;
	info.m_minDesiredIslandSize = m_minDesiredIslandSize;

}


hkWorldMemoryWatchDog* hkWorld::getMemoryWatchDog( ) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_memoryWatchDog;
}

void hkWorld::setMemoryWatchDog( hkWorldMemoryWatchDog* watchDog )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	if (watchDog != HK_NULL )
	{
		watchDog->addReference();
	}
	if ( m_memoryWatchDog != HK_NULL	)
	{
		m_memoryWatchDog->removeReference();
	}
	m_memoryWatchDog = watchDog;
}


hkWorld::hkWorld( const hkWorldCinfo& info, unsigned int sdkversion )
{
	m_lastEntityUid = hkUint32(-1);

	if ( HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS || HK_CHECK_DETERMINISM_ENABLE_MT_CHECKS )
	{
		HK_WARN(0xf0233212, "The engine is compiled with special determinism checks, see hkbase/debugutil/hkCheckDeterminismUtil.h");
	}

	//
	//	Check consistency of stepSizes in info HVK-929
	//
#ifdef HK_DEBUG
	{
		hkReal estimatedDt = 0.016f; // or shall we use info.m_expectedMinPsiDeltaTime
		hkReal gravLen = info.m_gravity.length3();
		if ( gravLen > 0.0f )
		{
			hkReal distanceTravelledInOneFrameDueToGravitationalAcceleration = 0.5f * gravLen * estimatedDt * estimatedDt;
			if ( distanceTravelledInOneFrameDueToGravitationalAcceleration > info.m_collisionTolerance * 2.0f )
			{
				HK_WARN( 0xf0de4354, "Your m_collisionTolerance seems to be very small" );
			}
			hkReal distanceTravelledInTenFramesDueToGravitationalAcceleration = 0.5f * gravLen * 10.0f * estimatedDt * 10.0f * estimatedDt;
			if ( distanceTravelledInTenFramesDueToGravitationalAcceleration < info.m_collisionTolerance  )
			{
				HK_WARN( 0xf0de4355, "Your m_collisionTolerance seems to be very big" );
			}

			hkReal velocityInOneFrame = 1.0f * estimatedDt * gravLen;
			if ( velocityInOneFrame > info.m_contactRestingVelocity )
			{
				HK_WARN( 0xf0de4356, "Your m_contactRestingVelocity seems to be too small" );
			}
		}
	}
#endif


	//
	// Operation delaying manager
	//
	m_pendingOperations = new hkWorldOperationQueue(this);
	m_pendingOperationQueues = HK_NULL;
	HK_ON_DEBUG( hkDebugInfoOnPendingOperationQueues::init(this);	);
	m_pendingOperationsCount = 0;
	m_criticalOperationsLockCount = 0;
	m_criticalOperationsLockCountForPhantoms = 0;
	m_blockExecutingPendingOperations = false;
	m_criticalOperationsAllowed = true;
	m_modifyConstraintCriticalSection = HK_NULL;
	m_propertyMasterLock = HK_NULL;
	m_worldLock = HK_NULL;
	m_islandDirtyListCriticalSection = HK_NULL;
	m_pendingOperationQueueCount = 1;

	//
	// Broadphase
	//
	{
		m_broadPhaseExtents[0] = info.m_broadPhaseWorldAabb.m_min;
		m_broadPhaseExtents[1] = info.m_broadPhaseWorldAabb.m_max;

		HK_ASSERT2(0x465fe452, info.m_broadPhaseNumMarkers == 0, "There is currently an issue with markers that can cause a crash. You should disable markers until this is fixed");
		m_broadPhaseNumMarkers = info.m_broadPhaseNumMarkers;

		HK_ASSERT2(0x1570b067,  (m_broadPhaseExtents[0].compareLessThan4(m_broadPhaseExtents[1]) & HK_VECTOR3_COMPARE_MASK_XYZ) == HK_VECTOR3_COMPARE_MASK_XYZ, "Each axis of world size MUST be > 0.0 !");
		m_broadPhase = hkBroadPhase::m_defaultCreationFunction( m_broadPhaseExtents[0], m_broadPhaseExtents[1], info.m_broadPhaseNumMarkers );

		// this is used as a guess for how many overlapping objects will be found in the broadphase on the addition of an entity or phantom
		m_broadPhaseQuerySize = info.m_broadPhaseQuerySize; // 1024
		m_broadPhaseUpdateSize = m_broadPhaseQuerySize / 2;
	}

	markForWrite();


	if (sdkversion != HAVOK_SDK_VERSION_NUMBER)
	{
		HK_ERROR(0x53c94b42, "** Havok libs built with version [" << HAVOK_SDK_VERSION_NUMBER << "], used with code built with [" << sdkversion << "]. **");
	}

	// set sim type in order for a new one to be created 
	m_simulationType = hkWorldCinfo::SIMULATION_TYPE_INVALID;
	m_simulation     = HK_NULL;

	m_gravity = info.m_gravity;


	//
	// Used to be in hkWorld::updateFromCinfo
	//



	// activation upon hkRigidBody::set Position/Rotation/Transform
	m_shouldActivateOnRigidBodyTransformChange = info.m_shouldActivateOnRigidBodyTransformChange;
	m_wantOldStyleDeactivation = info.m_wantOldStyleDeactivation;


	m_toiCollisionResponseRotateNormal = info.m_toiCollisionResponseRotateNormal;
	m_highFrequencyDeactivationPeriod = info.m_highFrequencyDeactivationPeriod;
	m_lowFrequencyDeactivationPeriod  = info.m_lowFrequencyDeactivationPeriod;

	m_deactivationReferenceDistance = info.m_deactivationReferenceDistance;

	// Solver info initialization
	{
		hkSolverInfo& si = m_dynamicsStepInfo.m_solverInfo;
		si.m_one		 = 1.0f;

		si.setTauAndDamping( info.m_solverTau, info.m_solverDamp );

		// new values
		si.m_contactRestingVelocity = info.m_contactRestingVelocity;
		si.m_numSteps    = info.m_solverIterations;
		si.m_invNumSteps = 1.0f / info.m_solverIterations;

		si.m_numMicroSteps    = info.m_solverMicrosteps;
		si.m_invNumMicroSteps = 1.0f / info.m_solverMicrosteps;

		const hkReal expectedDeltaTime = 0.016f;
		hkReal gravity = info.m_gravity.length3();
		if ( gravity == 0.0f )
		{
			gravity = 9.81f;
		}

		const hkReal averageObjectSize = gravity * 0.1f;

		for ( int i = 0; i < hkSolverInfo::DEACTIVATION_CLASSES_END; i++)
		{
			hkReal relVelocityThres;  // relative to gravity*1sec
			hkReal relDeceleration;	  // factor of the gravity at relVelocityThres 
			hkReal timeToDeact;
			switch (i)
			{
			case hkSolverInfo::DEACTIVATION_CLASS_INVALID:
			case hkSolverInfo::DEACTIVATION_CLASS_OFF:
				relVelocityThres   = HK_REAL_EPSILON;
				relDeceleration    = 0.0f;
				timeToDeact        = 1000;
				break;
			case hkSolverInfo::DEACTIVATION_CLASS_LOW:
				relVelocityThres   = 0.01f;   // = 10cm/sec
				relDeceleration    = 0.08f;
				timeToDeact        = 0.1f;
				break;
			case hkSolverInfo::DEACTIVATION_CLASS_MEDIUM:
				relVelocityThres   = 0.017f;   // = 17cm/sec
				relDeceleration    = 0.2f;
				timeToDeact        = 0.1f;
				break;
			case hkSolverInfo::DEACTIVATION_CLASS_HIGH:
				relVelocityThres   = 0.02f;   // = 20cm/sec
				relDeceleration    = 0.3f;
				timeToDeact        = 0.10f;
				break;
			default:
			case hkSolverInfo::DEACTIVATION_CLASS_AGGRESSIVE:
				relVelocityThres   = 0.025f;   // = 25cm/sec
				relDeceleration    = 0.4f;
				timeToDeact        = 0.05f;
				break;
			}
			hkSolverInfo::DeactivationInfo& di = si.m_deactivationInfo[i];

			di.m_stepsToDeactivate = hkUint16(timeToDeact * si.m_numSteps / expectedDeltaTime);
			const hkReal velocityThres = gravity * relVelocityThres;

			hkReal deceleration = gravity * relDeceleration / velocityThres;

			di.m_slowObjectVelocityMultiplier = 1.0f - expectedDeltaTime * si.m_invNumSteps * deceleration;

			di.m_linearVelocityThresholdInv = 1.0f / velocityThres;
			di.m_angularVelocityThresholdInv = 1.0f / (averageObjectSize * velocityThres);
			
			if (relDeceleration > 0)
			{
				di.m_relativeSleepVelocityThreshold = expectedDeltaTime * si.m_invNumSteps / relDeceleration;
			}
			else
			{
				di.m_relativeSleepVelocityThreshold = HK_REAL_MAX / 16.0f;
			}
			hkReal q = info.m_deactivationReferenceDistance;
			di.m_maxDistSqrd[0] = q;
			di.m_maxDistSqrd[1] = q * 4;
			di.m_maxRotSqrd[0]  = q * 2;
			di.m_maxRotSqrd[1]  = q * 8;

			di.m_maxDistSqrd[0] *= di.m_maxDistSqrd[0];
			di.m_maxDistSqrd[1] *= di.m_maxDistSqrd[1];
			di.m_maxRotSqrd[0] = di.m_maxRotSqrd[0] * di.m_maxRotSqrd[0];
			di.m_maxRotSqrd[1] = di.m_maxRotSqrd[0] * di.m_maxRotSqrd[1];
		}
	}

	//
	// End of code that was in updateFromCinfo
	//


	m_memoryWatchDog = info.m_memoryWatchDog;
	if ( m_memoryWatchDog != HK_NULL )
	{
		m_memoryWatchDog->addReference();
	}

	// Simulation islands and deactivation
	m_wantSimulationIslands = info.m_enableSimulationIslands;
	m_wantDeactivation = info.m_enableDeactivation;
	if (!m_wantSimulationIslands && m_wantDeactivation)
	{
		m_wantDeactivation = false;
		HK_WARN(0xad678954, "Cannot use deactivation when not using simulation islands. Deactivation disabled.");
	}
	m_processActionsInSingleThread = info.m_processActionsInSingleThread;


	//
	// Collision detection bridge
	//
	{
		m_broadPhaseDispatcher    = new hkTypedBroadPhaseDispatcher();
		m_phantomBroadPhaseListener = new hkPhantomBroadPhaseListener();
		m_entityEntityBroadPhaseListener = new hkEntityEntityBroadPhaseListener(this);
		m_broadPhaseBorderListener = new hkBroadPhaseBorderListener();

		m_broadPhaseDispatcher->setBroadPhaseListener(m_phantomBroadPhaseListener, hkWorldObject::BROAD_PHASE_ENTITY,  hkWorldObject::BROAD_PHASE_PHANTOM);
		m_broadPhaseDispatcher->setBroadPhaseListener(m_phantomBroadPhaseListener, hkWorldObject::BROAD_PHASE_PHANTOM, hkWorldObject::BROAD_PHASE_ENTITY);
		m_broadPhaseDispatcher->setBroadPhaseListener(m_phantomBroadPhaseListener, hkWorldObject::BROAD_PHASE_PHANTOM, hkWorldObject::BROAD_PHASE_PHANTOM);

		m_broadPhaseDispatcher->setBroadPhaseListener(m_entityEntityBroadPhaseListener, hkWorldObject::BROAD_PHASE_ENTITY, hkWorldObject::BROAD_PHASE_ENTITY);

		// Extra five records for the Broad Phase Borders
		m_broadPhaseDispatcher->setBroadPhaseListener(m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_ENTITY, hkWorldObject::BROAD_PHASE_BORDER);
		m_broadPhaseDispatcher->setBroadPhaseListener(m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_BORDER, hkWorldObject::BROAD_PHASE_ENTITY);

		// Use boder listeners to handle border-phantom overlaps.
		m_broadPhaseDispatcher->setBroadPhaseListener(m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_PHANTOM, hkWorldObject::BROAD_PHASE_BORDER);
		m_broadPhaseDispatcher->setBroadPhaseListener(m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_BORDER, hkWorldObject::BROAD_PHASE_PHANTOM);
		m_broadPhaseDispatcher->setBroadPhaseListener(m_broadPhaseBorderListener, hkWorldObject::BROAD_PHASE_BORDER, hkWorldObject::BROAD_PHASE_BORDER);

		hkContactMgrFactory* defaultCmFactory = new hkSimpleConstraintContactMgr::Factory( this );

			// add a default collision filter (returns always true)
		m_collisionDispatcher = new hkCollisionDispatcher( hkNullAgent::createNullAgent, defaultCmFactory );

		defaultCmFactory->removeReference();

		if ( info.m_collisionFilter == HK_NULL )
		{
			m_collisionFilter = new hkNullCollisionFilter();
		}
		else
		{
			m_collisionFilter = info.m_collisionFilter;
			m_collisionFilter->addReference();
		}

		if ( info.m_convexListFilter == HK_NULL )
		{
			m_convexListFilter = new hkDefaultConvexListFilter();
		}
		else
		{
			m_convexListFilter = info.m_convexListFilter;
			m_convexListFilter->addReference();
		}


		m_collisionInput = new hkProcessCollisionInput;
		hkProcessCollisionInput& pci = * m_collisionInput;

		pci.m_dispatcher = m_collisionDispatcher;
		pci.m_tolerance = info.m_collisionTolerance;
		pci.m_filter = m_collisionFilter;
		pci.m_convexListFilter = m_convexListFilter;

		pci.m_config = new hkCollisionAgentConfig();
		m_contactPointGeneration = info.m_contactPointGeneration;
		pci.m_config->m_iterativeLinearCastEarlyOutDistance = info.m_iterativeLinearCastEarlyOutDistance;
		pci.m_config->m_iterativeLinearCastMaxIterations = info.m_iterativeLinearCastMaxIterations;

		pci.m_createPredictiveAgents = false;
		pci.m_collisionQualityInfo = pci.m_dispatcher->getCollisionQualityInfo( hkCollisionDispatcher::COLLISION_QUALITY_PSI );

		hkWorld_setupContactMgrFactories( this, getCollisionDispatcher() );
	}

	// override simulation type
	if ( m_forceMultithreadedSimulation )
	{
		hkWorldCinfo* worldInfo = const_cast<hkWorldCinfo*>( &info );
		worldInfo->m_simulationType = hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
	}

	m_minDesiredIslandSize = 0;
	//@@@PS3OS: <todo: why is the simulation deleted in here?
	if (info.m_simulationType != m_simulationType)
	{
		if (m_simulation)
		{
			delete m_simulation;
		}

		m_simulationType = info.m_simulationType;
		switch(m_simulationType)
		{
			case hkWorldCinfo::SIMULATION_TYPE_DISCRETE:                   m_simulation = new hkSimulation    ( this ); break;
			case hkWorldCinfo::SIMULATION_TYPE_CONTINUOUS:                 m_simulation = new hkContinuousSimulation( this ); break;
#if defined(HK_PLATFORM_MULTI_THREAD)
			case hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED:
			{
				m_modifyConstraintCriticalSection = new hkCriticalSection( 4000 );
				m_propertyMasterLock = new hkCriticalSection( 4000 );
				m_simulation = new hkMultiThreadedSimulation( this );
				m_minDesiredIslandSize = info.m_minDesiredIslandSize;
				break;
			}
#endif
			default: 
			{
				HK_ASSERT2( 0xf032ed54, 0, "Invalid simulation type. Please select a valid type." );
				break;
			}

		}

		m_worldLock = new hkCriticalSection( 4000 );
		m_islandDirtyListCriticalSection = new hkCriticalSection( 4000 );
		m_simulation->m_frameMarkerPsiSnap = info.m_frameMarkerPsiSnap;

	}

	//
	//	Surface qualities
	//
	{
		hkReal gravLen = m_gravity.length3();
		if ( gravLen == 0.0f )
		{
			gravLen = 9.81f;
		}

		hkCollisionDispatcher::InitCollisionQualityInfo input;
		input.m_gravityLength = gravLen;
		input.m_collisionTolerance = m_collisionInput->m_tolerance;
		input.m_minDeltaTime = info.m_expectedMinPsiDeltaTime; // 50 Hz
		input.m_maxLinearVelocity = info.m_expectedMaxLinearVelocity;
		input.m_wantContinuousCollisionDetection = info.m_simulationType >= hkWorldCinfo::SIMULATION_TYPE_CONTINUOUS ;
		input.m_enableNegativeManifoldTims = info.m_contactPointGeneration == hkWorldCinfo::CONTACT_POINT_REJECT_MANY;
		input.m_enableNegativeToleranceToCreateNon4dContacts = info.m_contactPointGeneration >= hkWorldCinfo::CONTACT_POINT_REJECT_DUBIOUS;
		input.m_defaultConstraintPriority = hkConstraintInstance::PRIORITY_PSI;
		input.m_toiConstraintPriority = hkConstraintInstance::PRIORITY_TOI;
		input.m_toiHigherConstraintPriority = hkConstraintInstance::PRIORITY_TOI_HIGHER;
		input.m_toiForcedConstraintPriority = hkConstraintInstance::PRIORITY_TOI_FORCED;

		m_collisionDispatcher->initCollisionQualityInfo( input );

		hkCollisionDispatcher* dis = m_collisionDispatcher;
		m_collisionInput->m_collisionQualityInfo = dis->getCollisionQualityInfo( dis->COLLISION_QUALITY_PSI );

	}


	//
	// Simulation Islands
	//
	{
		m_fixedIsland = new hkSimulationIsland(this);
		m_fixedIsland->m_storageIndex = HK_INVALID_OBJECT_INDEX;
		m_fixedIsland->m_active = false;
		m_fixedIsland->m_isInActiveIslandsArray = false;
#ifdef HK_DEBUG_MULTI_THREADING
			// we disable this flag for fixed islands
		m_fixedIsland->m_allowIslandLocking = true;
#endif

		if (!m_wantSimulationIslands)
		{
			hkSimulationIsland* activeIsland = new hkSimulationIsland(this);
			m_activeSimulationIslands.pushBack( activeIsland );
			activeIsland->m_storageIndex = 0;
		}
	}


	//
	// Add the fixed rigid body
	// NOTE: This rigid body has no shape, and we do not need it to,
	// so we temporarily disable the associated rigid body construction warning
	//
	{
		hkRigidBodyCinfo rbci;
		rbci.m_motionType = hkMotion::MOTION_FIXED;
		rbci.m_mass = 0;
		m_fixedRigidBody = new hkRigidBody( rbci );
		addEntity( m_fixedRigidBody );
		m_fixedRigidBody->removeReference();
	}

	{
		m_dynamicsStepInfo.m_stepInfo.m_deltaTime = 0;
		m_collisionInput->m_dynamicsInfo = &m_dynamicsStepInfo;
	}

		// note: do not manually unmark the broadphase as this is done in hkWorld::unmarkForWrite()!
	m_broadPhase->markForWrite();

	//
	//	Broadphase border
	//
	if ( info.m_broadPhaseBorderBehaviour != info.BROADPHASE_BORDER_DO_NOTHING )
	{
		m_broadPhaseBorder = new hkBroadPhaseBorder( this, info.m_broadPhaseBorderBehaviour );
	}
	else
	{
		m_broadPhaseBorder = HK_NULL;
	}

	m_maintenanceMgr = new hkDefaultWorldMaintenanceMgr();
	m_maintenanceMgr->init(this);

	unmarkForWrite();
	if ( info.m_simulationType != hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED)
	{
		m_multiThreadLock.disableChecks();
	}

}

void hkWorld::setCollisionFilter( hkCollisionFilter* filter,
 								  hkBool             runUpdateCollisionFilterOnWorld, 
								  hkUpdateCollisionFilterOnWorldMode          updateMode,			
								  hkUpdateCollectionFilterMode updateShapeCollectionFilter )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	if ( filter == HK_NULL )
	{
		filter = new hkNullCollisionFilter();
	}
	else
	{
		filter->addReference();
	}

	m_collisionFilter->removeReference();

	m_collisionFilter = filter;
	m_collisionInput->m_filter = m_collisionFilter;

	{
		if (runUpdateCollisionFilterOnWorld)
		{
			updateCollisionFilterOnWorld(updateMode, updateShapeCollectionFilter);
		}
		else
		{
#if defined(HK_DEBUG)
			if ( ( getActiveSimulationIslands().getSize() != 0) || ( m_inactiveSimulationIslands.getSize() != 0 ) )
			{
				HK_WARN(0x4a5454cb, "You are setting the collision filter after adding entities. Collisions between these entities will not have been filtered correctly."\
						" You can use hkWorld::updateCollisionFilter() to make sure the entities are filtered according to the new filter");
			}
#endif
		}
	}
		}

void hkWorld::checkAccessGetActiveSimulationIslands() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	if ( !(this->m_multiThreadLock.isMarkedForWrite() || this->m_multiThreadLock.isMarkedForReadRecursive()))
	{
		// if we are in multi threaded mode, we need the job queue to be locked
		HK_ASSERT( 0xf0232344, m_simulationType == hkWorldCinfo::SIMULATION_TYPE_MULTITHREADED);
		HK_ON_DEBUG(const hkMultiThreadedSimulation* mts = static_cast<const hkMultiThreadedSimulation*>(m_simulation));
		HK_ASSERT2( 0xf0232345, mts->m_jobQueue.m_criticalSection.isEntered(), "In the multithreaded section of hkWorld, you cannot use getActiveSimulationIslands()"  );
	}
}

void hkWorld::setBroadPhaseBorder( hkBroadPhaseBorder* b )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	if ( m_broadPhaseBorder )
	{
		m_broadPhaseBorder->deactivate();
		m_broadPhaseBorder->removeReference();
	}

	m_broadPhaseBorder = b;
	if ( b )
	{
		b->addReference();
	}
}

hkBroadPhaseBorder* hkWorld::getBroadPhaseBorder() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_broadPhaseBorder;
}



void hkWorld::castRay(const hkWorldRayCastInput& input, hkRayHitCollector& collector ) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	hkWorldRayCaster rayCaster;
	rayCaster.castRay( *getBroadPhase(), input, getCollisionFilter(), HK_NULL, collector );
}

void hkWorld::castRay(const hkWorldRayCastInput& input, hkWorldRayCastOutput& output ) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	hkSimpleWorldRayCaster rayCaster;
	rayCaster.castRay( *getBroadPhase(), input, getCollisionFilter(), HK_NULL, output );
}


void hkWorld::getClosestPoints( const hkCollidable* collA, const hkCollisionInput& input, hkCdPointCollector& collector) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	HK_TIMER_BEGIN_LIST("hkWorld::getClosestPoints", "broadphase" );

	hkAabb aabb;
	hkReal restTolerance = input.getTolerance() - getCollisionInput()->getTolerance() * 0.5f;
	collA->getShape()->getAabb( collA->getTransform(), restTolerance, aabb );

	//
	//	Goto the broadphase and get all overlapping objects
	//
	hkInplaceArray<hkBroadPhaseHandlePair,128> hits;
	m_broadPhase->querySingleAabb( aabb, hits );

	hkBroadPhaseHandlePair* p = hits.begin();
	hkShapeType typeA = collA->getShape()->getType();

	HK_TIMER_SPLIT_LIST("narrowphase")
	for (int i = hits.getSize() -1; i>=0; p++, i--)
	{
		const hkTypedBroadPhaseHandle* tp = static_cast<const hkTypedBroadPhaseHandle*>( p->m_b );
		const hkCollidable* collB = static_cast<hkCollidable*>(tp->getOwner());
		if ( collA == collB )
		{
			continue;
		}

		if ( !getCollisionFilter()->isCollisionEnabled( *collA, *collB ))
		{
			continue;
		}

		const hkShape* shapeB = collB->getShape();
		if ( !shapeB )
		{
			continue;
		}

		hkShapeType typeB = shapeB->getType();

		hkCollisionDispatcher::GetClosestPointsFunc getClosestPointFunc = input.m_dispatcher->getGetClosestPointsFunc( typeA, typeB );
		getClosestPointFunc( *collA, *collB, input, collector ); 
	}
	HK_TIMER_END_LIST();
}


void hkWorld::getPenetrations( const hkCollidable* collA, const hkCollisionInput& input, hkCdBodyPairCollector& collector ) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	HK_TIMER_BEGIN_LIST("hkWorld::getPenetrations", "broadphase" );

	hkAabb aabb;
	hkReal restTolerance = input.getTolerance() - getCollisionInput()->getTolerance() * 0.5f;
	collA->getShape()->getAabb( collA->getTransform(), restTolerance, aabb );

	//
	//	Goto the broadphase and get all overlapping objects
	//
	hkInplaceArray<hkBroadPhaseHandlePair,128> hits;
	m_broadPhase->querySingleAabb( aabb, hits );

	hkBroadPhaseHandlePair* p = hits.begin();
	hkShapeType typeA = collA->getShape()->getType();

	HK_TIMER_SPLIT_LIST("narrowphase")
	for (int i = hits.getSize() -1; i>=0; p++, i--)
	{
		const hkTypedBroadPhaseHandle* tp = static_cast<const hkTypedBroadPhaseHandle*>( p->m_b );
		const hkCollidable* collB = static_cast<hkCollidable*>(tp->getOwner());

		if ( collA == collB )
		{
			continue;
		}

		if ( !getCollisionFilter()->isCollisionEnabled( *collA, *collB ))
		{
			continue;
		}

		const hkShape* shapeB = collB->getShape();
		if ( !shapeB )
		{
			continue;
		}

		hkShapeType typeB = shapeB->getType();

		hkCollisionDispatcher::GetPenetrationsFunc getPenetrationsFunc = input.m_dispatcher->getGetPenetrationsFunc( typeA, typeB );
		getPenetrationsFunc( *collA, *collB, input, collector ); 
		if ( collector.getEarlyOut() )
		{
			break;
		}
	}
	HK_TIMER_END_LIST();
}

void hkWorld::linearCast( const hkCollidable* collA, const hkLinearCastInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startPointCollector ) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	hkWorldLinearCaster linearCaster;
	linearCaster.linearCast( *getBroadPhase(), collA, input, getCollisionFilter(), *getCollisionInput(), getCollisionInput()->m_config, castCollector, startPointCollector );
}

/////////////////////////////////////////////////////////////
//
// Serialization / systems support.
//
/////////////////////////////////////////////////////////////

static hkBool HK_CALL enumerateAllInactiveEntitiesInWorld(const hkWorld* world, hkPhysicsSystem* sys)
{
	hkBool hasFixedEntities = false;
	if (world->getFixedIsland()) 
	{
		const hkArray<hkEntity*>& e = world->getFixedIsland()->getEntities();
		for (int ei=0; ei < e.getSize(); ++ei)
		{
				// Sometimes we have a dummy rigid body in the island, so we ignore it
			if ( ei == 0 && e[ei]->getCollidable()->getShape() == HK_NULL )
			{
				continue;
			}
			sys->addRigidBody( static_cast<hkRigidBody*>( e[ei] ) );
		}
		hasFixedEntities = e.getSize() > 0;
	}

	hkArray<hkSimulationIsland*>::const_iterator si_it;
	const hkArray<hkSimulationIsland*>& inactive_sim_islands = world->getInactiveSimulationIslands();
	for ( si_it = inactive_sim_islands.begin(); si_it != inactive_sim_islands.end(); si_it++)
	{
		const hkArray<hkEntity*>& e = (*si_it)->getEntities();
		for (int ei=0; ei < e.getSize(); ++ei)
		{
			sys->addRigidBody( static_cast<hkRigidBody*>( e[ei] ) );
		}
	}
	return inactive_sim_islands.getSize() > 0 || hasFixedEntities;
}

static hkBool HK_CALL enumerateAllActiveEntitiesInWorld(const hkWorld* world, hkPhysicsSystem* sys)
{
	hkArray<hkSimulationIsland*>::const_iterator si_it;
	const hkArray<hkSimulationIsland*>& active_sim_islands = world->getActiveSimulationIslands();
	// set active state of the physics system if active rigid bodies were found
	for ( si_it = active_sim_islands.begin(); si_it != active_sim_islands.end(); si_it++)
	{
		const hkArray<hkEntity*>& e = (*si_it)->getEntities();
		for (int ei=0; ei < e.getSize(); ++ei)
		{
			sys->addRigidBody( static_cast<hkRigidBody*>( e[ei] ) );
		}
	}
	return active_sim_islands.getSize() > 0;
}

static void HK_CALL enumerateAllEntitiesInWorld(const hkWorld* world, hkPhysicsSystem* sys)
{
	enumerateAllInactiveEntitiesInWorld( world, sys );
	sys->setActive(enumerateAllActiveEntitiesInWorld( world, sys ));
}

static void enumerateAllConstraintsInIsland( hkSimulationIsland* island, hkPhysicsSystem* sys)
{
	for (int e = 0; e < island->getEntities().getSize(); ++e)
	{
		hkEntity* entity = island->getEntities()[e];

		const hkArray<struct hkConstraintInternal>&  constraintMasters = entity->getConstraintMasters();

		for ( int c = 0; c < constraintMasters.getSize(); c++)
		{
			const hkConstraintInternal* ci = &constraintMasters[c];
			hkConstraintAtom* atom = hkWorldConstraintUtil::getTerminalAtom(ci);
			hkConstraintAtom::AtomType type = atom->getType();
			if (type != hkConstraintAtom::TYPE_CONTACT )
			{
				sys->addConstraint( ci->m_constraint );
			}
		}
	}
}

static void enumerateAllConstraintsInWorld(const hkWorld* world, hkPhysicsSystem* sys)
{
	// Get the list from each island. 
	// A constraint can not exist in two islands so we can just add them all inti one big list without checking for duplicates.
	hkArray<hkSimulationIsland*>::const_iterator si_it;
	const hkArray<hkSimulationIsland*>& active_sim_islands = world->getActiveSimulationIslands();
	for ( si_it = active_sim_islands.begin(); si_it != active_sim_islands.end(); si_it++)
	{
		enumerateAllConstraintsInIsland( (*si_it), sys );
	}

	const hkArray<hkSimulationIsland*>& inactive_sim_islands = world->getInactiveSimulationIslands();
	for ( si_it = inactive_sim_islands.begin(); si_it != inactive_sim_islands.end(); si_it++)
	{
		enumerateAllConstraintsInIsland( (*si_it), sys );
	}
}

static void enumerateAllActionsInWorld(const hkWorld* world, hkPhysicsSystem* sys)
{
	// Get the list from each island. 
	// A constraint can not exist in two islands so we can just add them all inti one big list withut checking for duplicates.
	hkArray<hkSimulationIsland*>::const_iterator si_it;
	const hkArray<hkSimulationIsland*>& active_sim_islands = world->getActiveSimulationIslands();
	for ( si_it = active_sim_islands.begin(); si_it != active_sim_islands.end(); si_it++)
	{
		const hkArray<hkAction*>& a = (*si_it)->getActions();
		for (int ai=0; ai < a.getSize(); ++ai)
		{
			sys->addAction( a[ai] );
		}
	}

	const hkArray<hkSimulationIsland*>& inactive_sim_islands = world->getInactiveSimulationIslands();
	for ( si_it = inactive_sim_islands.begin(); si_it != inactive_sim_islands.end(); si_it++)
	{
		const hkArray<hkAction*>& a = (*si_it)->getActions();
		for (int ai=0; ai < a.getSize(); ++ai)
		{
			sys->addAction( a[ai] );
		}
	}
}

static void enumerateAllPhantomsInWorld(const hkWorld* world, hkPhysicsSystem* sys)
{
	hkBroadPhaseBorder* border = world->getBroadPhaseBorder();
	const hkArray<hkPhantom*>& phantoms = world->getPhantoms();
	if ( border )
	{
		for (int pi=0; pi < phantoms.getSize(); ++pi)
		{
			hkPhantom* p = phantoms[pi];
			if ( p == border->m_phantoms[0] ) continue;
			if ( p == border->m_phantoms[1] ) continue;
			if ( p == border->m_phantoms[2] ) continue;
			if ( p == border->m_phantoms[3] ) continue;
			if ( p == border->m_phantoms[4] ) continue;
			if ( p == border->m_phantoms[5] ) continue;
			sys->addPhantom(p);
		}
	}
	else
	{
		for (int pi=0; pi < phantoms.getSize(); ++pi)
		{
			sys->addPhantom(phantoms[pi]);
		}
	}
}



hkPhysicsSystem* hkWorld::getWorldAsOneSystem() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	hkPhysicsSystem* sys = new hkPhysicsSystem();
	
		//
		// rigid bodies
		//
	enumerateAllEntitiesInWorld(this, sys);
	enumerateAllPhantomsInWorld(this, sys);

	// Constraints and Actions
	enumerateAllConstraintsInWorld( this, sys);
	enumerateAllActionsInWorld( this, sys);

	return sys;
}

void hkWorld::getWorldAsSystems(hkArray<hkPhysicsSystem*>& systemsInOut) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	hkPhysicsSystem* sys = new hkPhysicsSystem();
	systemsInOut.pushBack(sys);

	//
	// active rigid bodies
	//
	enumerateAllActiveEntitiesInWorld( this, sys );
	enumerateAllPhantomsInWorld( this, sys );

	// Constraints and Actions
	enumerateAllConstraintsInWorld( this, sys );
	enumerateAllActionsInWorld( this, sys );
	
	//
	// fixed/inactive rigid bodies
	//
	sys = new hkPhysicsSystem();
	if (enumerateAllInactiveEntitiesInWorld( this, sys ))
	{
		sys->setActive(false);
		systemsInOut.pushBack(sys);
	}
	else
	{
		delete sys;
	}
}

/////////////////////////////////////////////////////////////
//
//  Locking the world, and delaying worldOperations
//
/////////////////////////////////////////////////////////////

void hkWorld::internal_executePendingOperations()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_INTERNAL_TIMER_BEGIN("PendingOps", this);
	m_pendingOperationsCount = 0;
	m_pendingOperations->executeAllPending();
	HK_INTERNAL_TIMER_END();
}

void hkWorld::queueOperation(const hkWorldOperation::BaseOperation& operation)
{
	m_pendingOperations->queueOperation(operation);
}

hkWorldOperation::UserCallback* hkWorld::queueCallback(hkWorldOperation::UserCallback* callback, void* userData)
{
	if (areCriticalOperationsLocked())
	{
		hkWorldOperation::UserCallbackOperation operation;
		operation.m_userCallback = callback;
		operation.m_userData = userData;
		queueOperation(operation);
		return callback;
	}

	callback->worldOperationUserCallback(userData);
	return callback;
}


void hkWorld::findInitialContactPoints( hkEntity** entities, int numEntities )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	
	HK_ON_DEBUG( m_simulation->assertThereIsNoCollisionInformationForEntities(entities, numEntities, this) );

	hkStepInfo stepInfo( m_simulation->getCurrentPsiTime(), m_simulation->getCurrentPsiTime() );
	m_simulation->collideEntitiesDiscrete( entities, numEntities, this, stepInfo, hkSimulation::FIND_CONTACTS_EXTRA );
}

void hkWorld::findInitialContactPointsOfAllEntities()
{
	const hkArray<hkSimulationIsland*>& activeIslands = getActiveSimulationIslands();
	for (int i = 0; i < activeIslands.getSize(); i++)
	{
		findInitialContactPoints( activeIslands[i]->m_entities.begin(), activeIslands[i]->m_entities.getSize());
	}
	for (int i = 0; i < m_inactiveSimulationIslands.getSize(); i++)
	{
		findInitialContactPoints( m_inactiveSimulationIslands[i]->m_entities.begin(), m_inactiveSimulationIslands[i]->m_entities.getSize());
	}
}


int hkWorld::getMemUsageForIntegration( )
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );

	int maxMemUse = 0;
	const hkArray<hkSimulationIsland*>& activeIslands = getActiveSimulationIslands();

	for (int i = activeIslands.getSize()-1; i>=0; i--)
	{
		hkSimulationIsland* activeIsland = activeIslands[i];
		HK_ASSERT(0x367e587f,  activeIsland->m_storageIndex == i );

		int memUsage = activeIsland->getMemUsageForIntegration();

		maxMemUse = hkMath::max2(maxMemUse, memUsage);
	}
	return maxMemUse;
}

#if defined HK_DEBUG

hkBool hkWorld::constrainedDynamicBodiesCanCollide( const hkConstraintInstance* constraint ) const
{
	const hkRigidBody* rigidBodyA = constraint->getRigidBodyA();
	const hkRigidBody* rigidBodyB = constraint->getRigidBodyB();

	//
	// Only check for collisions between dynamic bodies.
	//
	if ( rigidBodyA && rigidBodyB && (! rigidBodyA->isFixedOrKeyframed() ) && (! rigidBodyB->isFixedOrKeyframed() )  )
	{	
		const hkCollidable* collidableA = rigidBodyA->getCollidable();
		const hkCollidable* collidableB = rigidBodyB->getCollidable();

		//
		// Check if the hkWorld's filter thinks the objects should collide.
		// If so, check if bodies are close enough for collisions between them to be a likely problem.
		//
		if ( getCollisionFilter()->isCollisionEnabled( *collidableA, *collidableB ) )
		{
			// Use a hkClosestCdPointCollector class to gather the results of our query.
			hkClosestCdPointCollector collector;

			// Get the shape type of each shape (this is used to figure out the most appropriate
			// getClosestPoints(...) method to use).
			const hkShapeType shapeTypeA = collidableA->getShape()->getType();
			const hkShapeType shapeTypeB = collidableB->getShape()->getType();

			hkCollisionInput input = *getCollisionInput();

			// Ask the collision dispatcher to locate a suitable getClosestPoints(...) method.
			hkCollisionDispatcher::GetClosestPointsFunc getClosestPointsFunc = getCollisionDispatcher()->getGetClosestPointsFunc( shapeTypeA, shapeTypeB );
			getClosestPointsFunc( *collidableA, *collidableB, input, collector );

			return collector.hasHit();
		}
	}

	return false;
}

void hkWorld::warnIfConstrainedDynamicBodiesCanCollide( const hkConstraintInstance* constraint,
															hkBool bodiesCollisionEnabledBeforeConstraintAdded,
															hkBool bodiesCollisionEnabledAfterConstraintAdded ) const
{
	const hkRigidBody* rigidBodyA = constraint->getRigidBodyA();
	const hkRigidBody* rigidBodyB = constraint->getRigidBodyB();

	const hkCollidable* collidableA = rigidBodyA->getCollidable();
	const hkCollidable* collidableB = rigidBodyB->getCollidable();

	if ( bodiesCollisionEnabledBeforeConstraintAdded )
	{
		// Current HK_WARN max string length is 512 characters. Keep messages a little shorter to be sure they fit.
		char warnInfoString[510];

		// Print body and constraint names if they are available.
		if ( rigidBodyA->getName() && rigidBodyB->getName() && constraint->getName() )
		{
			hkString::snprintf( warnInfoString,
				509,
				"Colliding body and constraint info; hkRigidBody A name:'%s', pointer:0x%p, filter info:%d. hkRigidBody B name:'%s', pointer:0x%p, filter info:%d. Constraint name:'%s', pointer:0x%p.",
				rigidBodyA->getName(),
				rigidBodyA,
				collidableA->getCollisionFilterInfo(),
				rigidBodyB->getName(),
				rigidBodyB,
				collidableB->getCollisionFilterInfo(),
				constraint->getName(),
				constraint );
		}
		else
		{
			hkString::snprintf( warnInfoString,
				509,
				"Colliding body and constraint info; hkRigidBody A pointer:0x%p, filter info:%d. hkRigidBody B pointer:0x%p, filter info:%d. Constraint pointer:0x%p.",
				rigidBodyA,
				collidableA->getCollisionFilterInfo(),
				rigidBodyB,
				collidableB->getCollisionFilterInfo(),
				constraint );
		}

		//
		// Warnings ID pairs match to allow users to disable irrelevant warnings only.
		// Have 2 warning outputs as there is no way to know how long user hkRigidBody and hkConstraint names will be...
		//
		if ( bodiesCollisionEnabledAfterConstraintAdded )
		{
			HK_WARN( 0x2a1db936, "Constraint added between two *colliding* dynamic rigid bodies. Check your collision filter logic and setup. Collision between constrained bodies typically leads to unintended artifacts e.g. adjacent, constrained ragdoll limbs colliding leading to 'ragdoll jitter'." );
			HK_WARN( 0x2a1db936, warnInfoString );
		}
		else
		{
			HK_WARN( 0x68c4e1dc, "Constraint added between two *colliding* dynamic rigid bodies. The bodies will collide with one another unless one of the functions hkWorld::updateCollisionFilter...() is called. Collision between constrained bodies typically leads to unintended artifacts e.g. adjacent, constrained ragdoll limbs colliding leading to 'ragdoll jitter'." );
			HK_WARN( 0x68c4e1dc, warnInfoString );
		}
	}
}

#endif	// #if defined HK_DEBUG

void hkWorld::lock()
{
	m_worldLock->enter();
	markForWrite();
}

void hkWorld::unlock()
{
	unmarkForWrite();
	m_worldLock->leave();
}

bool hkWorld::isLocked()
{
	return m_worldLock->isEntered();
}



void hkWorld::lockIslandForConstraintUpdate( hkSimulationIsland* island )
{
	if ( !m_modifyConstraintCriticalSection )
	{
		return;
	}
	if (!m_multiThreadLock.isMarkedForWrite() )
	{
		HK_ASSERT2( 0xf02134ed, island->m_allowIslandLocking, "You can only call this function during collision callbacks or when the world is locked");
	}

	m_modifyConstraintCriticalSection->enter();
	HK_ON_DEBUG_MULTI_THREADING(island->markForWrite());
	HK_ON_DEBUG_MULTI_THREADING(m_fixedIsland->markForWrite());
}

void hkWorld::unlockIslandForConstraintUpdate( hkSimulationIsland* island )
{
	if ( !m_modifyConstraintCriticalSection )
	{
		return;
	}
	if (!m_multiThreadLock.isMarkedForWrite() )
	{
		HK_ASSERT2( 0xf02134fd, island->m_allowIslandLocking, "You can only call this function during collision callbacks or when the world is locked");
	}
	island->unmarkForWrite();
	m_fixedIsland->unmarkForWrite();
	m_modifyConstraintCriticalSection->leave();
}



//////////////////////////////////////////////////////////////////////
//
//  Extra large-block comments moved from hkWorld.h
//
//////////////////////////////////////////////////////////////////////

// "concept: blocking of execution of pending operations"

// Concept: Suppreses attempts of execution of pending operations when hkWordl::attemptToExecutePendingOperations()
//          is called. Allows you to execute a series of critical operations (without locking the hkWorld) and only execute 
//          pending operations at the end (as each critical operation locks the world itself, therefore potentially putting
//          operations on the pending queue).
// Info:    This is only a boolean flag, not a blockCount.
// Usage:   Block the world before executing the first operation, unblock after executing the last one.
//          Call attemptToExecutePendingOperations explicitly then.
// Example: see hkWorld::updateCollisionFilterOnWorld


// "concept: allowing critical operations"

// Debugging utility: monitoring of critical operations executions.
// When 'critical' operations are NOT allowed, areCriticalOperationsLocked() fires an assert whenever called. 
// We assume that hkWorld::isLocked is called by every 'critical' operation at its beginning, to check whether
// the operation should be performed immediately or put on a pending list.


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
