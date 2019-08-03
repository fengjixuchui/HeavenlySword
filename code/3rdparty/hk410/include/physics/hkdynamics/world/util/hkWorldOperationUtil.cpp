/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkmath/basetypes/hkAabb.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/htl/hkAlgorithm.h>
#include <hkbase/sort/hkUnionFind.h>
#include <hkbase/debugutil/hkCheckDeterminismUtil.h>

#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkinternal/collide/broadphase/hkBroadPhase.h>
#include <hkinternal/collide/broadphase/hkBroadPhaseHandle.h>

#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseDispatcher.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>
#include <hkcollide/filter/hkCollisionFilter.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>
#include <hkinternal/collide/agent3/machine/1n/hkAgent1nTrack.h>
#include <hkinternal/collide/agent3/machine/nn/hkAgentNnMachine.h>

#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkinternal/collide/agent3/machine/nn/hkLinkedCollidable.h>

#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkdynamics/world/util/hkWorldOperationQueue.h>
#include <hkdynamics/world/util/hkWorldConstraintUtil.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>
#include <hkdynamics/world/util/hkWorldCallbackUtil.h>

#include <hkdynamics/world/simulation/hkSimulation.h>
#include <hkinternal/dynamics/world/simulation/continuous/hkContinuousSimulation.h>

#include <hkdynamics/entity/hkRigidBody.h>

#include <hkdynamics/motion/rigid/hkFixedRigidMotion.h>
#include <hkdynamics/motion/rigid/hkKeyframedRigidMotion.h>
#include <hkdynamics/motion/util/hkRigidMotionUtil.h>

#include <hkdynamics/world/util/hkNullAction.h>
#include <hkdynamics/phantom/hkPhantom.h>




void HK_CALL hkWorldOperationUtil::sortBigIslandToFront( hkWorld* world, hkSimulationIsland* island )
{
	HK_ASSERT(0x3ada00c7,  island->m_isInActiveIslandsArray);
	int storageIndex = island->m_storageIndex;
	if ( storageIndex == 0)
	{
		return;
	}

	world->getActiveSimulationIslands(); // for access check
	hkArray<hkSimulationIsland*>& activeIslands = world->m_activeSimulationIslands;
	hkSimulationIsland* other = activeIslands[0];
	if ( island->getEntities().getSize() > other->getEntities().getSize())
	{
		island->m_storageIndex = 0;
		other->m_storageIndex = hkObjectIndex(storageIndex);
		activeIslands[0] = island;
		activeIslands[storageIndex] = other;
	}
}


void hkWorldOperationUtil::updateEntityBP( hkWorld* world, hkEntity* entity )
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, entity, HK_ACCESS_RW );
	HK_ASSERT(0, world == entity->m_world);
	// postponed operations

	if ( world->areCriticalOperationsLocked() )
	{
		hkWorldOperation::UpdateEntityBP op;
		op.m_entity = entity;
		world->queueOperation(op);
		return;
	}


	hkCollidable* c = entity->getCollidableRw();
	const hkShape* shape = c->getShape();

	world->lockCriticalOperations();

	HK_ON_DEBUG( world->m_simulation->assertThereIsNoCollisionInformationForEntities(&entity, 1, world) );

	if (shape)
	{
		// add the shape to the broad phase and merge islands as necessary
		hkAabb aabb;

		hkLocalArray< hkBroadPhaseHandlePair > newPairs( world->m_broadPhaseQuerySize );
		hkLocalArray< hkBroadPhaseHandlePair > delPairs( world->m_broadPhaseQuerySize );

		c->getShape()->getAabb( c->getTransform(), world->getCollisionInput()->getTolerance() * .5f, aabb );
		hkBroadPhaseHandle* bph = static_cast<hkBroadPhaseHandle*>(c->getBroadPhaseHandle());
		world->m_broadPhase->updateAabbs( &bph, &aabb, 1, newPairs, delPairs );

		if ( newPairs.getSize() + delPairs.getSize() > 0)
		{
			hkTypedBroadPhaseDispatcher::removeDuplicates(newPairs, delPairs);
			world->m_broadPhaseDispatcher->removePairs( static_cast<hkTypedBroadPhaseHandlePair*>(delPairs.begin()), delPairs.getSize() );
			world->m_broadPhaseDispatcher->addPairs(    static_cast<hkTypedBroadPhaseHandlePair*>(newPairs.begin()), newPairs.getSize(), world->getCollisionFilter() );
		}

	}

	world->unlockAndAttemptToExecutePendingOperations();
}




void hkWorldOperationUtil::addEntityBP( hkWorld* world, hkEntity* entity )
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, entity, HK_ACCESS_RW );
	hkCollidable* c = entity->getCollidableRw();
	const hkShape* shape = c->getShape();

	if (shape)
	{
		// add the shape to the broad phase and merge islands as necessary
		hkAabb aabb;

		hkLocalArray< hkBroadPhaseHandlePair > pairsOut( world->m_broadPhaseQuerySize );

		c->getShape()->getAabb( c->getTransform(), world->getCollisionInput()->getTolerance() * .5f, aabb );

		world->m_broadPhase->addObject( c->getBroadPhaseHandle(), aabb, pairsOut );

		if (pairsOut.getSize() > 0)
		{
			world->m_broadPhaseDispatcher->addPairs( static_cast<hkTypedBroadPhaseHandlePair*>(&pairsOut[0]), pairsOut.getSize(), world->getCollisionFilter() );
		}
	}
}


void hkWorldOperationUtil::addPhantomBP( hkWorld* world, hkPhantom* phantom)
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, phantom, HK_ACCESS_RW );
	// Broadphase
	hkLocalArray< hkBroadPhaseHandlePair > newPairs( world->m_broadPhaseQuerySize );
	
	hkAabb aabb;
	phantom->calcAabb( aabb );
	world->m_broadPhase->addObject( phantom->getCollidableRw()->getBroadPhaseHandle(), aabb, newPairs );

	// check for changes
	if ( newPairs.getSize() != 0 )
	{
		world->m_broadPhaseDispatcher->addPairs( static_cast<hkTypedBroadPhaseHandlePair*>(&newPairs[0]), newPairs.getSize(), world->getCollisionFilter() );
	}
}


void hkWorldOperationUtil::addEntitySI( hkWorld* world, hkEntity* entity, hkEntityActivation initialActivationState)
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, entity, HK_ACCESS_RW );
	entity->setWorld( world );

	if ( entity->isFixed() )
	{
		world->m_fixedIsland->internalAddEntity( entity );
	}
	else
	{
		hkSimulationIsland* newIsland;
		if (world->m_wantSimulationIslands)
		{

			bool wantActive = initialActivationState == HK_ENTITY_ACTIVATION_DO_ACTIVATE;
			hkArray<hkSimulationIsland*>& islands = (wantActive) ? world->m_activeSimulationIslands : world->m_inactiveSimulationIslands;

			// check whether we can append our new entity to the last island
			if ( islands.getSize() 
				&& wantActive
				&& canIslandBeSparse( world, estimateIslandSize( islands.back()->m_entities.getSize()+1, islands.back()->m_numConstraints+3 ) )
				)
			{
				newIsland = islands.back();
				newIsland->m_sparseEnabled = true;
			}
			else
			{
				newIsland = new hkSimulationIsland(world);
				newIsland->m_active = wantActive;
				newIsland->m_isInActiveIslandsArray = wantActive;
				newIsland->m_storageIndex = (hkObjectIndex)islands.getSize();
				newIsland->m_splitCheckFrameCounter = hkUchar(newIsland->m_storageIndex);
				islands.pushBack(newIsland);
			}

			if (initialActivationState == HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE && !world->m_wantDeactivation)
			{
				HK_WARN(0xad000500, "Adding inactive entities while world.m_wantDeactivation == false" );
			}
		}
		else
		{
			HK_ASSERT2(0x71c02c92, initialActivationState == HK_ENTITY_ACTIVATION_DO_ACTIVATE, "Error: Cannot add a deactivated entity when hkWorld::m_wantSimulationIslands == false." );
			const hkArray<hkSimulationIsland*>& activeIslands = world->getActiveSimulationIslands();
			newIsland = activeIslands[0];
		}
		newIsland->internalAddEntity(entity);
	}
}



void hkWorldOperationUtil::removeEntityBP( hkWorld* world, hkEntity* entity )
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, entity, HK_ACCESS_RW );

	hkCollidable* c = entity->getCollidableRw();
	if ( c->getShape() != HK_NULL )
	{
		// Remove all TOI contact points before calling entity-removed callbacks
		world->m_simulation->resetCollisionInformationForEntities(&entity, 1, world, true);

		hkLocalArray< hkBroadPhaseHandlePair > pairsOut( world->m_broadPhaseQuerySize );

		world->m_broadPhase->removeObject( c->getBroadPhaseHandle(), pairsOut );

		if (pairsOut.getSize() > 0)
		{
			world->m_broadPhaseDispatcher->removePairs( static_cast<hkTypedBroadPhaseHandlePair*>(&pairsOut[0]), pairsOut.getSize() );
		}
	}
}

void hkWorldOperationUtil::removePhantomBP( hkWorld* world, hkPhantom* phantom)
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, phantom, HK_ACCESS_RW );
	//
	//	remove pairs
	//
	{
		hkLocalArray< hkBroadPhaseHandlePair > removedPairs( world->m_broadPhaseQuerySize );
		world->m_broadPhase->removeObject( phantom->getCollidableRw()->getBroadPhaseHandle(), removedPairs );

		// check for changes
		if ( removedPairs.getSize() != 0 )
		{
			world->m_broadPhaseDispatcher->removePairs( static_cast<hkTypedBroadPhaseHandlePair*>(&removedPairs[0]), removedPairs.getSize() );
		}
	}
}

void hkWorldOperationUtil::removeEntitySI( hkWorld* world, hkEntity* entity )
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, entity, HK_ACCESS_RW );
		// Set world to NULL
	entity->setWorld( HK_NULL );

	hkSimulationIsland* simIsland = entity->getSimulationIsland();
	// fire the events while the entities simulation island is still valid

	simIsland->internalRemoveEntity( entity );

	// remove the simulation island if it is inactive and has no more entities
	if ( ( !simIsland->isFixed() ) && ( simIsland->m_entities.getSize() == 0 ) && world->m_wantSimulationIslands)
	{
		removeIsland( world, simIsland );
		delete simIsland;
	}
}


void hkWorldOperationUtil::removeAttachedActionsFromFixedIsland( hkWorld* world, hkEntity* entity, hkArray<hkAction*>& actionsToBeMoved )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	//
	// We are adding the entity to a new dynamic island from the fixed island.
	//

	//
	//	Moving actions from the fixed island to the moving island
	//
	{

		// Check if any of the entity's actions are on the fixed island's list
		for( int i = 0; i< entity->getNumActions(); i++ )
		{
			hkAction* action= entity->getAction( i );

			if (action->getSimulationIsland() == world->m_fixedIsland)
			{
				actionsToBeMoved.pushBack(action);
				action->addReference();
				world->m_fixedIsland->removeAction( action);
				world->m_fixedIsland->m_actionListCleanupNeeded = true;
				HK_ASSERT2(0, world->m_fixedIsland->m_active == world->m_fixedIsland->m_isInActiveIslandsArray, "Internal: just checking.");
				putIslandOnDirtyList(world, world->m_fixedIsland);
			}

		}
	}	
}


void hkWorldOperationUtil::removeAttachedActionsFromDynamicIsland( hkWorld* world, hkEntity* entity, hkArray<hkAction*>& actionsToBeMoved )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	//
	// We are removing the entity from a dynamic island and adding it to the fixed island.
	// Check constraints and actions to see if they also need to be moved to the fixed island.
	//

	hkSimulationIsland* island = entity->getSimulationIsland(); // HK_NULL;
	HK_ASSERT(0xf0ff0065, island);

	hkInplaceArray<hkEntity*,16> actionEntities;

	{
		// Check if any of the actions need to be moved to the fixed island.
		for(int i = 0; i < entity->getNumActions(); i++ )
		{
			hkAction* action = entity->getAction(i);

			action->getEntities( actionEntities );

			hkBool moveAction = true;

			for( int j = 0; j < actionEntities.getSize(); j++ )
			{
				// set moveAction to TRUE if: all other entities attached to the action (besides the one being moved to the fixed island) are fixed
				if( !actionEntities[j]->isFixed() && ( actionEntities[j] != entity ) )
				{
					moveAction = false;
					break;
				}
			}

			if( moveAction )
			{
				actionsToBeMoved.pushBack(action);
				action->addReference();

				HK_ASSERT(0XAD000350, action->getSimulationIsland() == island);

				// remove it from the dynamic island
				island->removeAction( action );
				island->m_actionListCleanupNeeded = true;
				putIslandOnDirtyList(world, island);

			}
		}
	}
}


void hkWorldOperationUtil::addActionsToEntitysIsland( hkWorld* world, hkEntity* entity, hkArray<hkAction*>& actionsToBeMoved )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	//
	// 	Check constraints and actions and move them and merge islands as necessary.
	//

	hkSimulationIsland* entitySI = entity->getSimulationIsland();

	hkInplaceArray<hkEntity*,16> attachedEntities;

	for (int i = 0; i < actionsToBeMoved.getSize(); i++)
	{
		hkAction* action = actionsToBeMoved[i];
		
		if (entitySI != world->m_fixedIsland)
		{
			// moving to dynamic island


			entitySI->addAction(action);
			action->removeReference();
			//action->setSimulationIsland(entitySI);

			// See if we can merge islands (due to actions)
			attachedEntities.clear();
			action->getEntities( attachedEntities );

			{
				for( int j = 0; j<attachedEntities.getSize(); j++ )
				{
					if( !attachedEntities[j]->isFixed() && ( attachedEntities[j] != entity ) )
					{
						hkWorldOperationUtil::mergeIslandsIfNeeded( attachedEntities[j], entity );
					}
				}
			}

		}
		else
		{
			// processing fixed island

			// Add the action to the fixed island
			world->m_fixedIsland->addAction( action );
			action->removeReference();
			// Set the fixed island to be the action's island
			//action->setSimulationIsland( world->m_fixedIsland ); done

		}
	}
}


void hkWorldOperationUtil::removeIsland( hkWorld* world, hkSimulationIsland* island )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	if (!island->m_isInActiveIslandsArray )
	{
		world->m_inactiveSimulationIslands[island->m_storageIndex] = world->m_inactiveSimulationIslands[world->m_inactiveSimulationIslands.getSize() - 1];
		world->m_inactiveSimulationIslands[island->m_storageIndex]->m_storageIndex = island->m_storageIndex;
		world->m_inactiveSimulationIslands.popBack();
	}
	else
	{
		world->getActiveSimulationIslands(); // for access checks
		hkArray<hkSimulationIsland*>& activeIslands = world->m_activeSimulationIslands;

		activeIslands[island->m_storageIndex] = activeIslands.back();
		activeIslands[island->m_storageIndex]->m_storageIndex = island->m_storageIndex;
		activeIslands.popBack();
	}

	removeIslandFromDirtyList(world, island);
	
}


void hkWorldOperationUtil::addConstraintToCriticalLockedIsland( hkWorld* world, hkConstraintInstance* constraint  )
{
	HK_ASSERT2(0xad000103, constraint->getOwner() == HK_NULL, "Error: you are trying to add a constraint, that has already been added to some world");
	HK_ASSERT2(0xf0ff0066, constraint->getEntityA()->getWorld() == world && constraint->getEntityB()->getWorld() == world, "One of the constraint's entities has been already removed from the world (or has not been added yet?).");


	// world locking is not thread safe.
	// so we check for existing locks and use them
	HK_ASSERT( 0xf0ee3234, world->areCriticalOperationsLocked() );
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RO, constraint->getEntityA()->getSimulationIsland(), HK_ACCESS_RW);
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RO, constraint->getEntityB()->getSimulationIsland(), HK_ACCESS_RW);

	hkWorldConstraintUtil::addConstraint(     world, constraint );
	hkWorldCallbackUtil::fireConstraintAdded( world, constraint );

#if defined(HK_ENABLE_EXTENSIVE_WORLD_CHECKING)
	constraint->getMasterEntity()->getSimulationIsland()->isValid();
#endif
}

void hkWorldOperationUtil::removeConstraintFromCriticalLockedIsland( hkWorld* world, hkConstraintInstance* constraint )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RO );
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RO, constraint->getEntityA()->getSimulationIsland(), HK_ACCESS_RW);
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RO, constraint->getEntityB()->getSimulationIsland(), HK_ACCESS_RW);

	HK_ASSERT2(0x6c6f226b, constraint->getOwner() && static_cast<hkSimulationIsland*>(constraint->getOwner())->getWorld() == world, "Trying to remove a constraint, that has not been added to the world or was added to a different world.");

	HK_ASSERT( 0xf0ee3234, world->areCriticalOperationsLockedUnchecked() );

	if (world->m_constraintListeners.getSize())
	{
		hkWorldCallbackUtil::fireConstraintRemoved( world, constraint );
	}
	hkWorldConstraintUtil::removeConstraint( constraint );

#if defined(HK_ENABLE_EXTENSIVE_WORLD_CHECKING)
	constraint->getEntityA()->getSimulationIsland()->isValid();
	constraint->getEntityB()->getSimulationIsland()->isValid();
#endif

}

hkConstraintInstance* hkWorldOperationUtil::addConstraintImmediately( hkWorld* world, hkConstraintInstance* constraint, FireCallbacks fireCallbacks  )
{
	HK_ASSERT2(0xad000103, constraint->getOwner() == HK_NULL, "Error: you are trying to add a constraint, that has already been added to some world");
	HK_ASSERT2(0xf0ff0066, constraint->getEntityA()->getWorld() == world && constraint->getEntityB()->getWorld() == world, "One of the constraint's entities has been already removed from the world (or has not been added yet?).");

#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	hkSimulationIsland* island = static_cast<hkSimulationIsland*>(constraint->getOwner());
#	endif

	HK_ASSERT( 0xf0ee3234, !world->areCriticalOperationsLocked() );

// 		// world locking is not thread safe.
// 		// so we check for existing locks and use them
// 	if ( world->areCriticalOperationsLocked() )
// 	{
// 		HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RO, constraint->getEntityA()->getSimulationIsland(), HK_ACCESS_RW);
// 		HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RO, constraint->getEntityB()->getSimulationIsland(), HK_ACCESS_RW);
// 
// 		hkWorldConstraintUtil::addConstraint( world, constraint );
// 		if (fireCallbacks)
// 		{
// 			hkWorldCallbackUtil::fireConstraintAdded( world, constraint );
// 		}
// #	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
// 		island->isValid();
// #	endif
// 	}
// 	else
	{
		HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
		world->lockCriticalOperations();
		hkWorldConstraintUtil::addConstraint( world, constraint );
		if (fireCallbacks)
		{
			hkWorldCallbackUtil::fireConstraintAdded( world, constraint );
		}
#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
		island->isValid();
#	endif
		world->unlockAndAttemptToExecutePendingOperations();
	}



	return constraint;
}


void hkWorldOperationUtil::removeConstraintImmediately( hkWorld* world, hkConstraintInstance* constraint, FireCallbacks fireCallbacks )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RO );
	HK_ASSERT2(0x6c6f226b, constraint->getOwner() && static_cast<hkSimulationIsland*>(constraint->getOwner())->getWorld() == world, "Trying to remove a constraint, that has not been added to the world or was added to a different world.");

	hkSimulationIsland* island = static_cast<hkSimulationIsland*>(constraint->getOwner());
	island->m_splitCheckRequested = true;

		// world locking is not thread safe.
		// so we check for existing locks and use them
	//HK_ASSERT( 0xf0ee3234, !world->areCriticalOperationsLocked() );
	if ( world->areCriticalOperationsLockedUnchecked() )
	{
		if (fireCallbacks && world->m_constraintListeners.getSize())
		{
			hkWorldCallbackUtil::fireConstraintRemoved( world, constraint );
		}
		hkWorldConstraintUtil::removeConstraint( constraint );
#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
		island->isValid();
#	endif
	}
	else
	{
		HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
		world->lockCriticalOperations();
		if (fireCallbacks && world->m_constraintListeners.getSize())
		{
			hkWorldCallbackUtil::fireConstraintRemoved( world, constraint );
		}
		hkWorldConstraintUtil::removeConstraint( constraint );
#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
		island->isValid();
#	endif
		world->unlockAndAttemptToExecutePendingOperations();
	}
	
	return;
}

//
//	Find all groups which are inactive
//
static HK_FORCE_INLINE void findInactiveGroups( const hkUnionFind& checker, const hkArray<hkEntity*>& entities, int numGroups, hkFixedArray<int>& isActive )
{
	{ for (int i =0 ; i < numGroups; i++){ isActive[i] = 0; }	}
	{
		for (int i =0 ; i < entities.getSize(); i++ )
		{
			hkEntity* entity = entities[i];
			int inactiveFrames = hkMath::max2( entity->m_motion.m_deactivationNumInactiveFrames[0], entity->m_motion.m_deactivationNumInactiveFrames[1] );
			if (inactiveFrames <= hkMotion::NUM_INACTIVE_FRAMES_TO_DEACTIVATE)
			{
				int group = checker.m_parents[i];
				isActive[group] = 1;
			}
		}
	}
}

// reindex all groups using the following criteria:
// group 0: all active subgroups merged (if any)
//       1..n: all deactive subgroups not merged 
static HK_FORCE_INLINE void identifyDeactivatedSubgroups( hkUnionFind& checker, const hkArray<hkEntity*>& entities, hkArray<int>& groupsSizes, hkFixedArray<hkChar>& groupSparse )
{
	HK_ASSERT( 0xf0323f89, entities.getSize() == checker.m_numNodes );
	HK_ASSERT( 0xf04df223, groupSparse.getSizeDebug() >= groupsSizes.getSize()+1 );	// we need an extra element at the end to keep the algorithm simple and fast
	int numOriginalGroups = groupsSizes.getSize();

	hkLocalBuffer<int> isActive( numOriginalGroups );
	findInactiveGroups( checker, entities, numOriginalGroups, isActive );
	//
	//	Regroup data. Group 0 is the active group, the other groups are inactive
	//
	int numOutputGroups = 1;	// the first group is the active group

	//
	//	build reindex array so that we get the desired group layout
	//
	int* reindex = isActive.begin();
	groupSparse[0] = true;
	{
		for (int i = 0; i < numOriginalGroups; i++)
		{
			if ( isActive[i] )
			{
				reindex[i] = 0;
			}
			else
			{
				reindex[i] = numOutputGroups;
				groupSparse[numOutputGroups] = false;
				numOutputGroups++;
			}
		}
	}
	{
		// check whether we have only deactivated groups
		if ( numOutputGroups > numOriginalGroups )
		{
			groupSparse[0] = false;
			return;
		}
	}
	checker.reindex( isActive, numOutputGroups, groupsSizes );
}


//
//	Calculate whether a group is active and estimate the size for each constraint for the solver
//
static HK_FORCE_INLINE void findInactiveGroupsAndEstimateSolverSize( const hkUnionFind& checker, const hkArray<hkEntity*>& entities, int numGroups, hkFixedArray<int>& isActive, hkFixedArray<int>& sizes )
{
	{ for (int i =0 ; i < numGroups; i++){ isActive[i] = 0; sizes[i] = 0; }	}
	{
		for (int i =0 ; i < entities.getSize(); i++ )
		{
			hkEntity* entity = entities[i];
			int group = checker.m_parents[i];

			int inactiveFrames = hkMath::max2( entity->m_motion.m_deactivationNumInactiveFrames[0], entity->m_motion.m_deactivationNumInactiveFrames[1] );
			if (inactiveFrames <= hkMotion::NUM_INACTIVE_FRAMES_TO_DEACTIVATE)
			{
				isActive[group] = 1;
			}
			int size = hkWorldOperationUtil::estimateIslandSize( 1, entity->getConstraintMasters().getSize() );
			sizes[group] += size;
		}
	}
}


	// try to group the potential children into reasonable chunks  
	// basic idea: deactivated subgroups do not get merged
	//             merge active groups as long as the combined size still allows for a sparse island
static HK_FORCE_INLINE void mergeSmallSubgroups( hkWorld* world, hkUnionFind& checker, const hkArray<hkEntity*>& entities, hkArray<int>& groupsSizes, hkFixedArray<hkChar>& groupSparse )
{
	HK_ASSERT( 0xf0323f89, entities.getSize() == checker.m_numNodes );
	HK_ASSERT( 0xf0323f89, entities.getSize() == checker.m_numNodes );
	int numOriginalGroups = groupsSizes.getSize();

		// the constraint size for each group (-1 if group is inactive and -2 if group is already processed)
	hkLocalBuffer<int> sizes   ( numOriginalGroups );
	hkLocalBuffer<int> isActive( numOriginalGroups );

	findInactiveGroupsAndEstimateSolverSize( checker, entities, numOriginalGroups, isActive,sizes );

		// we only want to use sizes for the final decision.
		// Therefor set the sizes of deactivated groups to -1
	{
		for (int i=0; i < numOriginalGroups; i++)
		{
			if ( !isActive[i]  )
			{
				sizes[i] = -1;
			}
		}
	}


	// try to merge groups. groups taken get a size of -2
	int numOutputGroups = 0;	// the first group is the active group
	int* reindex = isActive.begin();

	// we can merge groups if none of the partners can deactivate and the combined size is still ok
	{
		int extraSize = hkWorldOperationUtil::estimateIslandSize( 10, 10 );

		for (int i=0; i < numOriginalGroups; i++)
		{
			int groupSize = sizes[i];
			if ( groupSize == -2){ continue; } // group taken already

			int groupId = numOutputGroups++;
			groupSparse[groupId] = false;
			reindex[i] = groupId;

			if ( groupSize < 0)	{ continue; }  // do not merge inactive groups

				// look for other groups we can merge into this one
			for (int j = i+1; j < numOriginalGroups; j++)
			{
				if ( !hkWorldOperationUtil::canIslandBeSparse(world, groupSize + extraSize) )
				{
					// our current groups is already close to the size limit. Just stop.
					// If we do not do this we might end up wasting lots of CPU 
					// ( Imagine 1000 single objects. Every group being full will still scan the remaining objects resulting in a full n*n algorithm)
					break;
				}
				{
					int otherSize = sizes[j];
					if ( otherSize < 0 )
					{
						// group taken or inactive (->cannot be merged)
						continue;
					}
					if ( hkWorldOperationUtil::canIslandBeSparse( world, groupSize + otherSize) )
					{
						// merge groups
						groupSparse[groupId] = true;	// now it's no longer fully connected
						reindex[j] = groupId;
						groupSize += otherSize;
						sizes[j] = -2;
					}
				}
			}
		}
	}
	checker.reindex( isActive, numOutputGroups, groupsSizes );
}

void hkWorldOperationUtil::splitSimulationIsland( hkSimulationIsland* currentIsland, hkWorld* world, hkArray<hkSimulationIsland*>& newIslandsOut, hkArray<hkEntity*>* oldEntitiesOut )
{



	//
	// Do a union find on the edges (created by actions, constraints, and collision pairs) of the island
	// if there is more than connected group, create new islands accordingly, and add the entities to the
	// islands.
	//
	hkLocalBuffer<int> entityInfo(currentIsland->m_entities.getSize());

	hkFixedArray<int>* fixedArray = &entityInfo;

	hkUnionFind checker( *fixedArray, currentIsland->m_entities.getSize() );

		// this tries to find independent subgroups within the island
	hkBool isConnected = currentIsland->isFullyConnected( checker );

	if ( isConnected )
	{
		currentIsland->m_sparseEnabled = false;
		return;
	}

		// this will hold the size of each independent subgroup
	hkInplaceArray<int,32> groupsSizes; checker.assignGroups( groupsSizes );

		// this will hold the information whether a final subgroup will be fully connected
	hkLocalBuffer<hkChar> groupSparse( groupsSizes.getSize()+1 );


	//
	//	now based on different requirements regroup the existing subgroups
	//  synchronize the lower code with hkEntity::deactivate
	//
	if ( currentIsland->m_sparseEnabled )
	{
		// resort groups: group 0 will be active (if active objects exist), the other groups will hold inactive groups
		identifyDeactivatedSubgroups( checker, currentIsland->m_entities, groupsSizes, groupSparse );
		if ( groupsSizes.getSize() == 1 )
		{
			// no need to split a single group. This can only happen if all subgroups are active, otherwise the isConnected check a few lines above will have returned
			return;
		}
	}
	else
	{
		if ( world->m_minDesiredIslandSize != 0 )
		{
			// only allow for sparseEnabled islands if the world says this is allowed 
			mergeSmallSubgroups( world, checker, currentIsland->m_entities, groupsSizes, groupSparse );
		}
	}

		//
		// resort so that biggest group is at index zero
		//
	{
		int index = checker.moveBiggestGroupToIndexZero( groupsSizes );
		hkChar h = groupSparse[0];
		groupSparse[0] = groupSparse[index];
		groupSparse[index] = h;

	}

	int biggestGroupSize = groupsSizes[0];
	int numAllEntities   = currentIsland->getEntities().getSize();

	//
	//	Create all our islands
	//
	int numNewIslands = groupsSizes.getSize();
	hkLocalBuffer<hkSimulationIsland*> islands( numNewIslands );
	{
		currentIsland->m_sparseEnabled = groupSparse[0] != 0;
		hkBool active = currentIsland->isActive();
		islands[0] = currentIsland;
		for ( int j = 1; j < numNewIslands; j++)
		{
			hkSimulationIsland* newIsland = new hkSimulationIsland( world );
			newIsland->m_active = true;
			newIsland->m_isInActiveIslandsArray = true;
			newIsland->m_sparseEnabled = groupSparse[j] != 0;
			newIsland->m_storageIndex = hkObjectIndex(newIslandsOut.getSize());
			newIslandsOut.pushBack( newIsland );
			islands[j] = newIsland;
			int numEntitiesCapacity = groupsSizes[j];
#if defined(HK_PLATFORM_HAS_SPU)
				// make sure we allocate at least 16 bytes to force the memory to be aligned on a 16 byte boundary.
			numEntitiesCapacity = hkMath::max2( 4, numEntitiesCapacity );
#endif
			newIsland->m_entities.reserveExactly( numEntitiesCapacity );
#ifdef HK_DEBUG_MULTI_THREADING
			newIsland->m_inIntegrateJob = currentIsland->m_inIntegrateJob;
			if ( currentIsland->m_multiThreadLock.isMarkedForWrite() )
			{
				newIsland->markForWrite();
			}
#endif
		}
	}

	//
	//	Now iterate through our entities and redistribute them
	//
	{	
		// we do a small trick, we simply remember the old entities array and clear it,
		// however we can still access the old members as long as we do not override them
		hkEntity** oldEntities = currentIsland->m_entities.begin();
		int oldSize = currentIsland->getEntities().getSize();

			// if requested export our old entity array 
		if ( oldEntitiesOut )
		{
			currentIsland->m_entities.swap(*oldEntitiesOut);
			currentIsland->m_entities.clear();
			currentIsland->m_entities.reserveExactly(groupsSizes[0]);
		}
		else
		{
			currentIsland->m_entities.clear();
		}

		for ( int e = 0; e < oldSize; e++ )
		{
			hkSimulationIsland* newIsland = islands[entityInfo[e]];

			hkEntity* entity = oldEntities[e];

			entity->m_simulationIsland = newIsland;
			entity->m_storageIndex = (hkObjectIndex)newIsland->m_entities.getSize();
			newIsland->m_entities.pushBackUnchecked(entity);

			//
			//	Update the constraint info
			//
			if ( currentIsland != newIsland )
			{
				hkConstraintInternal* ci = entity->m_constraintsMaster.begin();
				for (int j = 0; j < entity->m_constraintsMaster.getSize(); ci++, j++)
				{
					hkConstraintInfo info;
#ifdef 	HK_STORE_CONSTRAINT_INFO_IN_INTERNAL
					ci->getConstraintInfo(info);
#else
					HK_ASSERT2( 0xf0e12335, 0, "HK_STORE_CONSTRAINT_INFO_IN_INTERNAL must be defined" );
					ci->m_constraint->getData()->getConstraintInfo( info );
#endif	
					info.m_maxSizeOfJacobians = currentIsland->m_constraintInfo.m_maxSizeOfJacobians;
					currentIsland->m_numConstraints--;
					currentIsland->subConstraintInfo( ci->m_constraint, info );

					newIsland->m_numConstraints++;
					ci->m_constraint->setOwner( newIsland );
					newIsland    ->addConstraintInfo( ci->m_constraint, info );
				}
			}
		}
	}



	//
	// If new islands have been created, go through the actions, constraints and collision pairs
	// of the current island, and move them to the correct new islands.
	// Also remove any entities from the current island which have been added to a new island above.
	//
	{
		// update actions
		const hkSimulationIsland* fixedIsland = world->getFixedIsland();

		hkAction** oldActions = currentIsland->m_actions.begin();
		int oldActionSize = currentIsland->m_actions.getSize();
		currentIsland->m_actions.clear();

		for ( int j = 0; j < oldActionSize ; j++)
		{
			hkAction* action = oldActions[j];

			if (action != hkNullAction::getNullAction())
			{
				//
				// First update the island pointer in the action with the first moving entity's island
				//
				hkSimulationIsland* movingIsland = HK_NULL;
				{
					hkInplaceArray<hkEntity*, 16> entities;
					action->getEntities(entities);

					for ( int k = 0; k < entities.getSize(); ++k )
					{
						movingIsland = entities[k]->getSimulationIsland();
						if ( movingIsland != fixedIsland )
						{
							break;
						}
					}
					HK_ASSERT(0x4287f3f7,  movingIsland != HK_NULL );
				}
				action->setSimulationIsland( movingIsland );
				movingIsland->m_actions.pushBack(action);
			}
		}
	}

	//
	// Now redistribute the collision detection information
	//
	int entitiesInSmallerGroups = numAllEntities - biggestGroupSize;
	if ( entitiesInSmallerGroups * 8 < numAllEntities )
	{
		// just copy each individual agent entry if the number of objects to move is small
		hkAgentNnTrack& track = currentIsland->m_agentTrack;
		for ( int sectorIndex = 0; sectorIndex < track.m_sectors.getSize(); sectorIndex++ )		
		{																						
			hkAgentNnSector* currentSector = track.m_sectors[sectorIndex];			
			hkAgentNnEntry* entry = currentSector->getBegin();							

			for( ; entry < hkAddByteOffset( currentSector->getBegin(), track.getSectorSize( sectorIndex ));  )			
			{
				hkSimulationIsland* newIsland = static_cast<hkEntity*>(entry->getCollidableA()->getOwner())->getSimulationIsland();
				if ( newIsland->isFixed() )
				{
					newIsland = static_cast<hkEntity*>( entry->getCollidableB()->getOwner() )->getSimulationIsland();
				}
				if ( newIsland == currentIsland )
				{
					entry = hkAddByteOffset( entry, entry->m_size );
				}
				else
				{
					hkAgentNnMachine_CopyAndRelinkAgentEntry( newIsland->m_agentTrack, entry);
					hkAgentNnMachine_InternalDeallocateEntry( track, entry );
					if ( sectorIndex >= track.m_sectors.getSize() )
					{
						break;
					}
				}
			}
		}
	}
	else
		// do a complete distribution of agents
	{
		hkAgentNnTrack track(world->m_collisionDispatcher->m_agent3AgentSize, world->m_collisionDispatcher->m_agent3SectorSize);

		if (currentIsland->m_agentTrack.m_sectors.getSize() == 1)
		{
			track.m_sectors.pushBackUnchecked(currentIsland->m_agentTrack.m_sectors[0]);
			currentIsland->m_agentTrack.m_sectors.clear();
		}
		else if (currentIsland->m_agentTrack.m_sectors.getSize() > 1)
		{
			track.m_sectors.reserveExactly(2);	// make sure we do not have an inplace array
			track.m_sectors.swap( currentIsland->m_agentTrack.m_sectors );
		}
		hkAlgorithm::swap( track.m_bytesUsedInLastSector, currentIsland->m_agentTrack.m_bytesUsedInLastSector);

		{																							
			for ( int sectorIndex = 0; sectorIndex < track.m_sectors.getSize(); sectorIndex++ )		
			{																						
				hkAgentNnSector* currentSector = track.m_sectors[sectorIndex];			
				hkAgentNnEntry* entry = currentSector->getBegin();							
				hkAgentNnEntry* endEntry =  hkAddByteOffset( entry, track.getSectorSize( sectorIndex ) );

				for( ; entry < endEntry; entry = hkAddByteOffset( entry, entry->m_size ) )			
				{
					hkSimulationIsland* newIsland = static_cast<hkEntity*>(entry->getCollidableA()->getOwner())->getSimulationIsland();
					if ( newIsland->isFixed() )
					{
						newIsland = static_cast<hkEntity*>( entry->getCollidableB()->getOwner() )->getSimulationIsland();
					}
					hkAgentNnMachine_CopyAndRelinkAgentEntry(newIsland->m_agentTrack, entry);
				}

				hkThreadMemory::getInstance().deallocateChunk( currentSector, track.m_sectorSize, HK_MEMORY_CLASS_CDINFO );
			}
			track.m_sectors.clear();
		}
	}
	currentIsland->m_agentTrack.m_sectors.optimizeCapacity(0);

#ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	currentIsland->isValid();
#endif

}

void hkWorldOperationUtil::splitSimulationIsland( hkWorld* world, hkSimulationIsland* currentIsland )
{
#ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	currentIsland->isValid();
#endif
	bool useActiveIslands = currentIsland->m_isInActiveIslandsArray;
	hkArray<hkSimulationIsland*>& simulationIslands = useActiveIslands
		? const_cast<hkArray<hkSimulationIsland*>&>(world->getActiveSimulationIslands())
		: world->m_inactiveSimulationIslands;


	// The island could be empty (the user could have removed all its entities) so delete it if it is.
	HK_ASSERT2(0xad000192, currentIsland->getEntities().getSize(), "Internal error: any empty islands should have been removed in removeEntitySI.");

	currentIsland->m_splitCheckRequested = false;
	{
		int oldSize = simulationIslands.getSize();

		splitSimulationIsland(currentIsland, world, simulationIslands );

		hkBool active = currentIsland->isActive();

		// update deactivation status for new islands
		for ( int j = oldSize; j < simulationIslands.getSize(); j++)
		{
			hkSimulationIsland* newIsland = simulationIslands[j];
			newIsland->m_active					= useActiveIslands;
			newIsland->m_isInActiveIslandsArray = useActiveIslands;
			if (useActiveIslands && !active)
			{
				// the current island is already marked for deactivation
				// mark new islands for deactivation accordingly
				hkWorldOperationUtil::markIslandInactive( world, newIsland );
			}
		}
	}
}

void hkWorldOperationUtil::splitSimulationIslands( hkWorld* world )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	// make sure all merges are processed earlier
	HK_ASSERT(0XAD000083, !world->m_pendingOperationsCount);

	// only split islands if we want simulation islands
	if (!world->m_wantSimulationIslands)
	{
		return;
	}

	hkArray<hkSimulationIsland*>& simulationIslands = const_cast<hkArray<hkSimulationIsland*>&>(world->getActiveSimulationIslands());
	int originalSize = simulationIslands.getSize();

	for (int i = originalSize - 1; i >= 0; --i)
	{
		hkSimulationIsland* currentIsland = simulationIslands[i];
		if (!currentIsland->m_splitCheckRequested)
		{
			continue;
		}
		splitSimulationIsland( world, currentIsland );
	}

	// check all the new simulation islands are valid
#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	{
		for (int r = originalSize; r < simulationIslands.getSize(); ++r)
		{
			simulationIslands[r]->isValid();
		}
	}
#	endif
}




void hkWorldOperationUtil::mergeIslands(hkWorld* world, hkEntity* entityA, hkEntity* entityB)
{
	hkSimulationIsland* islandA = entityA->getSimulationIsland();
	hkSimulationIsland* islandB = entityB->getSimulationIsland();

	HK_ASSERT(0xf0ff0067, islandA != islandB);

	if ( world->areCriticalOperationsLocked() )
	{
		hkWorldOperation::MergeIslands op;
		op.m_entities[0] = entityA;
		op.m_entities[1] = entityB;

		world->queueOperation(op);
		return;
	}
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	internalMergeTwoIslands( world, islandA, islandB );
}


hkSimulationIsland* hkWorldOperationUtil::internalMergeTwoIslands( hkWorld* world, hkSimulationIsland* islandA, hkSimulationIsland* islandB )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	HK_ASSERT(0x21dd55c1, world->m_wantSimulationIslands);
	HK_INTERNAL_TIMER_BEGIN("MergeIsle", this);

	HK_ASSERT(0xf0ff0068, islandA != islandB);

	HK_ASSERT2(0x63082c41,  (islandA != world->m_fixedIsland) && (islandB != world->m_fixedIsland), "Internal error: an island-merge-request executed for which at least one of the entities is fixed");
	if ( islandA->m_entities.getSize() <= islandB->m_entities.getSize() )
	{
		// we need this check so that our islands are deterministic
		if ( islandA->m_entities.getSize() < islandB->m_entities.getSize() || islandA->m_storageIndex > islandB->m_storageIndex)
		{
			hkAlgorithm::swap( islandA, islandB );
		}
	}
	{
		hkCheckDeterminismUtil::checkSt(islandA->m_storageIndex);
		hkCheckDeterminismUtil::checkSt(islandB->m_storageIndex);
		hkCheckDeterminismUtil::checkSt(islandA->getEntities().getSize());
		hkCheckDeterminismUtil::checkSt(islandB->getEntities().getSize());
		{	for (int i=0; i < islandA->getEntities().getSize(); i++ ){ hkCheckDeterminismUtil::checkSt( islandA->getEntities()[i]->m_uid); } }
		{	for (int i=0; i < islandB->getEntities().getSize(); i++ ){ hkCheckDeterminismUtil::checkSt( islandB->getEntities()[i]->m_uid); } }
	}


	world->lockCriticalOperations();

	//hkBool activate = islandA->m_active || islandB->m_active;
	hkBool activate = islandA->m_isInActiveIslandsArray || islandB->m_isInActiveIslandsArray;
	hkBool finalActiveState = islandA->isActive() || islandB->isActive();
	if (activate)
	{
		if (!islandA->m_isInActiveIslandsArray)
		{
			HK_ASSERT(0xf0ff0069, islandB->m_isInActiveIslandsArray);
			islandA->m_active = true;
			internalActivateIsland(world, islandA);

			islandA->m_highFrequencyDeactivationCounter = islandB->m_highFrequencyDeactivationCounter;
			islandA->m_lowFrequencyDeactivationCounter  = islandB->m_lowFrequencyDeactivationCounter;
		}
		else if (!islandB->m_isInActiveIslandsArray)
		{
			HK_ASSERT(0xf0ff0069, islandA->m_isInActiveIslandsArray);
			islandB->m_active = true;
			internalActivateIsland(world, islandB);

			// islandA has correct high/low-frequency deactivation counters
		}
		else
		{
			islandA->m_highFrequencyDeactivationCounter = hkMath::min2(islandA->m_highFrequencyDeactivationCounter, islandB->m_highFrequencyDeactivationCounter);
			islandA->m_lowFrequencyDeactivationCounter  = hkMath::min2(islandA->m_lowFrequencyDeactivationCounter,  islandB->m_lowFrequencyDeactivationCounter );
		}
	}
	islandA->m_sparseEnabled |= islandB->m_sparseEnabled;

	// Called first so that we can remove any duplicate collision pairs and delete their mgr/agent
	hkAgentNnMachine_AppendTrack(islandA->m_agentTrack, islandB->m_agentTrack);

	// copy entities, and update entity owners
	{
		hkObjectIndex insertIndex = (hkObjectIndex)islandA->m_entities.getSize();

		islandA->m_entities.setSize(islandA->m_entities.getSize() + islandB->m_entities.getSize());

		for (int i = 0; i < islandB->m_entities.getSize(); ++i, insertIndex++)
		{
			islandA->m_entities[insertIndex] = islandB->m_entities[i];
			islandB->m_entities[i]->m_simulationIsland = islandA;
			islandB->m_entities[i]->m_storageIndex = insertIndex;
		}
	}

	// copy actions
	{
		int insertIndex = islandA->m_actions.getSize();

		islandA->m_actions.setSize(islandA->m_actions.getSize() + islandB->m_actions.getSize());

		for (int i = 0; i < islandB->m_actions.getSize(); ++i)
		{
			if (islandB->m_actions[i] != hkNullAction::getNullAction())
			{
				islandA->m_actions[insertIndex] = islandB->m_actions[i];
				islandA->m_actions[insertIndex]->setSimulationIsland(islandA);
				insertIndex++;
			}
		}
		islandA->m_actions.setSize(insertIndex);
	}

	// update constraint owners of constraints B (masters only)
	{
		for (int e = 0; e < islandB->m_entities.getSize(); ++e)
		{
			hkEntity* entity = islandB->m_entities[e];
			hkConstraintInternal* ci = entity->m_constraintsMaster.begin();
			for ( int c = entity->m_constraintsMaster.getSize()-1; c>= 0; ci++, c--)
			{
				hkConstraintInstance* constraint = ci->m_constraint;
				constraint->setOwner( islandA );
			}
		}
		islandA->mergeConstraintInfo( *islandB );
		islandA->m_numConstraints += islandB->m_numConstraints;
		islandB->m_numConstraints = 0;
	}

	{
		hkArray<hkSimulationIsland*>* arrayToRemoveFrom;
		hkObjectIndex indexToRemoveAt;
		HK_ASSERT2(0xf0ff00a7, !(activate && !islandA->m_isInActiveIslandsArray), "Internal error");

		{
			if (islandB->m_isInActiveIslandsArray)
			{
				// remove islandB from active
				arrayToRemoveFrom = &world->m_activeSimulationIslands;
				indexToRemoveAt = (hkObjectIndex)islandB->getStorageIndex();

			}
			else
			{
				// remove islandB from inactive 
				arrayToRemoveFrom = &world->m_inactiveSimulationIslands;
				indexToRemoveAt = (hkObjectIndex)islandB->getStorageIndex();
			}
		}

		{
			// removing island from islandsArray
			// input; array to remove from
			//        island to remove / index to remove at
			HK_ASSERT2(0x69b9e470, (*arrayToRemoveFrom)[indexToRemoveAt] == islandA ||
								   (*arrayToRemoveFrom)[indexToRemoveAt] == islandB,   "internal error");

			if (indexToRemoveAt < arrayToRemoveFrom->getSize() - 1)
			{
				// conditional, as we don't want to overwrite the newly assigned m_storageIndex of islandA (see: case (1))
				(*arrayToRemoveFrom)[indexToRemoveAt] = (*arrayToRemoveFrom)[arrayToRemoveFrom->getSize() - 1];
				(*arrayToRemoveFrom)[indexToRemoveAt]->m_storageIndex = indexToRemoveAt;
			}
			(*arrayToRemoveFrom).popBack();
		}
	}

	// Properly copy/update state flags of the merged island
	islandA->m_splitCheckRequested = islandA->m_splitCheckRequested || islandB->m_splitCheckRequested;
	islandA->m_actionListCleanupNeeded = islandA->m_actionListCleanupNeeded || islandB->m_actionListCleanupNeeded;
	islandA->m_active = finalActiveState;

	if (islandB->m_dirtyListIndex != HK_INVALID_OBJECT_INDEX) // done in putIslandOnDirtyIsland: && islandA->m_dirtyListIndex == HK_INVALID_OBJECT_INDEX)
	{
		putIslandOnDirtyList(world, islandA);
	}


	HK_ASSERT(0xf0ff0071, islandA->m_active == islandA->m_isInActiveIslandsArray || islandA->m_dirtyListIndex != HK_INVALID_OBJECT_INDEX);


#ifdef HK_DEBUG
	islandB->m_entities.clear();
	islandB->m_actions.clear();
#endif

	removeIslandFromDirtyList(world, islandB);
	delete islandB;

	if ( islandA->m_isInActiveIslandsArray )
	{
		sortBigIslandToFront( world, islandA );
	}

#	ifdef HK_ENABLE_EXTENSIVE_WORLD_CHECKING
	islandA->isValid();
#	endif
	HK_INTERNAL_TIMER_END();

	world->unlockAndAttemptToExecutePendingOperations();

	return islandA;
}



void hkWorldOperationUtil::validateWorld( hkWorld* world )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RO );
	{
		const hkArray<hkSimulationIsland*>& activeIslands = world->getActiveSimulationIslands();

		for (int i = activeIslands.getSize()-1; i>=0; i--)
		{
			activeIslands[i]->isValid();
		}
	}
	{
		for (int i = world->m_inactiveSimulationIslands.getSize()-1; i>=0; i--)
		{
			world->m_inactiveSimulationIslands[i]->isValid();
		}
	}
	{
		world->m_fixedIsland->isValid();
	}
}

void HK_CALL hkWorldOperationUtil::setRigidBodyMotionType( hkRigidBody* body, hkMotion::MotionType newState, hkEntityActivation initialActivationState, hkUpdateCollisionFilterOnEntityMode collisionFilterUpdateMode)
{ 
	HK_ACCESS_CHECK_OBJECT( body->getWorld(), HK_ACCESS_RW );
	// Info on the assert: if unchecked, this causes an error later on, when collision filter is updated on the body,
	// and that causes agent-creation function to crash, when it is given a collidable which doesn't have a shape assigned (as it is
	// with the _FixedBody_).
	HK_ASSERT2(0xad4848b3, !body->getWorld() || body->getWorld()->getFixedRigidBody() != body, "Cannot call setMotionType for hkWorld::getFixedRigidBody.");

	hkMotion::MotionType oldState = body->getMotionType();

	// "do nothing" conditions, e.g. can't set original keyframed bodies to dynamic 
	if ( oldState == newState ) 
	{
		return;
	}

	bool newStateNeedsInertia = ( newState != hkMotion::MOTION_FIXED ) && (newState != hkMotion::MOTION_KEYFRAMED );
	bool oldStateNeedsInertia = ( oldState != hkMotion::MOTION_FIXED ) && (oldState != hkMotion::MOTION_KEYFRAMED );

#ifdef HK_DEBUG
	{
		bool qualityTypeIsFixedOrKeyframedOrKeyframedReporting = (body->getCollidable()->getQualityType() == HK_COLLIDABLE_QUALITY_FIXED || body->getCollidable()->getQualityType() == HK_COLLIDABLE_QUALITY_KEYFRAMED || body->getCollidable()->getQualityType() == HK_COLLIDABLE_QUALITY_KEYFRAMED_REPORTING);
		if (oldStateNeedsInertia == qualityTypeIsFixedOrKeyframedOrKeyframedReporting)
		{
			HK_WARN(0xad4bb4de, "Old quality type doesn't correspond to the old motionType. DO NOT call entity->getCollidable()->setQualityType(HK_COLLIDABLE_QUALITY_chosen_type) before the call to hkRigidBody::setMotionType(). Quality type changes are now handled by setMotionType internally. ");
			HK_WARN(0xad4bb4de, "This is important as the default collision filtering relies on qualityTypes of bodies (and filters out fixed-fixed, fixed-keyframed, keyframed-keyframed interactions). Also further asserts may be fired when processing TOI events for bodies with such inconsistent motion-quality types settings.");
		}
	}
#endif // HK_DEBUG

	if ( (newStateNeedsInertia && (!oldStateNeedsInertia)) && static_cast<hkKeyframedRigidMotion*>(body->getMotion())->m_savedMotion == HK_NULL)
	{
		HK_ASSERT2(0x7585f7ab, false, "Attempting to change hkRigidBody's hkMotionType to a dynamic type. This cannot be performed for bodies which were not initially constructed as dynamic." );
		// do nothing.
		return;
	}

	//adds an extra reference for the scope of this function
	body->addReference();

	hkWorld* world = body->getWorld();

	bool changeBetweenFixedAndNonFixed = ( oldState == hkMotion::MOTION_FIXED ) != ( newState == hkMotion::MOTION_FIXED );
	if( world && changeBetweenFixedAndNonFixed )
	{

			// An array for any constraints that need to be moved to and from the fixed island.
		hkInplaceArray<hkConstraintInstance*,16> constraintsToBeMoved;
		hkInplaceArray<hkAction*,16>             actionsToBeMoved;
		hkAgentNnTrack                           agentsToBeMoved(world->getCollisionDispatcher()->m_agent3AgentSize, world->getCollisionDispatcher()->m_agent3SectorSize);

		// need to block critical operations executed from within filter update and merge islands
		world->blockExecutingPendingOperations(true);

		world->allowCriticalOperations(false);

		// to be simplified
		hkWorldOperationUtil::removeAttachedConstraints( body, constraintsToBeMoved );

   		if ( newState != hkMotion::MOTION_FIXED )
		{
			hkWorldOperationUtil::removeAttachedActionsFromFixedIsland( world, body, actionsToBeMoved ); //ToDynamicIsland
		}
		else
		{
			hkWorldOperationUtil::removeAttachedActionsFromDynamicIsland( world, body, actionsToBeMoved ); //ToFixedIsland
		}

		// remove all the agents .
		removeAttachedAgentsConnectingTheEntityAndAFixedPartnerEntityPlus( body->getSimulationIsland()->m_agentTrack, body, agentsToBeMoved, newState);

		if (oldState != hkMotion::MOTION_FIXED && body->m_simulationIsland->m_entities.getSize() > 2)
		{
			body->m_simulationIsland->m_splitCheckRequested = true;
		}
		
		hkWorldOperationUtil::removeEntitySI(world, body);
	

		// ^-- above: removing stuff from old island
		{
			// Replace rigid motion object in the body.
			hkWorldOperationUtil::replaceMotionObject(body, newState, newStateNeedsInertia, oldStateNeedsInertia, world);
		}
		// ,-- below: adding stuff to new island


		hkWorldOperationUtil::addEntitySI(world, body, initialActivationState);
		hkWorldOperationUtil::addActionsToEntitysIsland(world, body, actionsToBeMoved );

		world->allowCriticalOperations(true);

		for ( int i = 0; i < constraintsToBeMoved.getSize();i++)
		{
			hkConstraintInstance* constraint = constraintsToBeMoved[i];

			// merge islands performed here:
			hkWorldOperationUtil::addConstraintImmediately(world, constraint, hkWorldOperationUtil::DO_NOT_FIRE_CALLBACKS_AND_SUPPRESS_EXECUTION_OF_PENDING_OPERATIONS );

			constraint->removeReference();
		}

		// Append all moved agents to the track of the new island
		{

			// And do not forget to merge islands (if motion type changed from fixed to dynamic)
			if (newState != hkMotion::MOTION_FIXED)
			{
				// We have to go through the whole list of collisionPartners of the entity and check for necessary merges.
				
				hkArray<hkLinkedCollidable::CollisionEntry>& collisionEntries = body->getLinkedCollidable()->m_collisionEntries;
				for (int i = 0; i < collisionEntries.getSize(); i++)
				{
					hkRigidBody* otherBody = static_cast<hkRigidBody*>(collisionEntries[i].m_partner->getOwner());
					hkWorldOperationUtil::mergeIslandsIfNeeded(body, otherBody);
				}

			}

			hkAgentNnMachine_AppendTrack(body->getSimulationIsland()->m_agentTrack, agentsToBeMoved);
		}

		// update sweptTransform
		if (newState == hkMotion::MOTION_FIXED)
		{
			hkMotionState* ms = body->getRigidMotion()->getMotionState();
				// have event time here...........
			hkSweptTransformUtil::freezeMotionState(world->m_dynamicsStepInfo.m_stepInfo.m_startTime, *ms);

			world->lockCriticalOperations();
			// invalidate TIMs + remove TOI events
			// <todo> rename this function:
			world->m_simulation->resetCollisionInformationForEntities(reinterpret_cast<hkEntity**>(&body), 1, world );
			// create discrete Aabb
			hkSimulation::collideEntitiesBroadPhaseDiscrete(reinterpret_cast<hkEntity**>(&body), 1, world);
			// update graphics !
			hkWorldCallbackUtil::fireInactiveEntityMoved(body->m_world, body);
			world->unlockCriticalOperations();
		}

		// Destroy or create agents (according to new quality type)
		world->updateCollisionFilterOnEntity(body, collisionFilterUpdateMode, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);

		// ?? ignore agents -- don't warptime

		world->blockExecutingPendingOperations(false);
		world->attemptToExecutePendingOperations();
	}
	else // 	if( world && changeBetweenFixedAndNonFixed )
	{
		// Replace rigid motion object in the body.
		hkWorldOperationUtil::replaceMotionObject(body, newState, newStateNeedsInertia, oldStateNeedsInertia, world);

		// When changing from dynamic to key-framed motion, we remove TOI events (because keyframed-keyframed & keyframed-fixed TOI's are not allowed).
		// We don't do that when changing from keyframed to dynamic motion.
		// When changing from/to fixed motion, TOI events are removed by calls to entityRemoveSI().
		if (world)
		{
			// Destroy or create agents (according to new quality type). This also removes Toi events.
			world->updateCollisionFilterOnEntity(body, collisionFilterUpdateMode, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
		}
	}

	body->removeReference();
}


	// this is meant to be only used with hkRigidBody::setMotionType
void HK_CALL hkWorldOperationUtil::removeAttachedConstraints( hkEntity* entity, hkArray<hkConstraintInstance*>& removedConstraints )
{
	hkWorld*  world = entity->getWorld();
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );

	// master constraints
	{
		hkArray<hkConstraintInternal>& constraints = entity->m_constraintsMaster;

		for (int i = constraints.getSize()-1; i >= 0; i--)
		{
			hkConstraintInstance* constraint = constraints[i].m_constraint;
			constraint->addReference();
			hkWorldOperationUtil::removeConstraintImmediately( world, constraint, hkWorldOperationUtil::DO_NOT_FIRE_CALLBACKS_AND_SUPPRESS_EXECUTION_OF_PENDING_OPERATIONS );
			removedConstraints.pushBack( constraint );
		}
	}

	// slave constraints
	{
		hkArray<hkConstraintInstance*>& constraints = entity->m_constraintsSlave;

		for (int i = constraints.getSize()-1; i >= 0; i--)
		{
			hkConstraintInstance* constraint = constraints[i];
			constraint->addReference();
			hkWorldOperationUtil::removeConstraintImmediately( world, constraint, hkWorldOperationUtil::DO_NOT_FIRE_CALLBACKS_AND_SUPPRESS_EXECUTION_OF_PENDING_OPERATIONS );
			removedConstraints.pushBack( constraint );
		}
	}
}


void HK_CALL hkWorldOperationUtil::removeAttachedAgentsConnectingTheEntityAndAFixedPartnerEntityPlus( hkAgentNnTrack& trackToScan, hkEntity* entity, hkAgentNnTrack& agentsRemoved, hkMotion::MotionType newMotionType)
{
	HK_ACCESS_CHECK_OBJECT( entity->getWorld(), HK_ACCESS_RW );
	HK_ASSERT(0xf0ff0072, trackToScan.m_agentSize == agentsRemoved.m_agentSize);
	// remove those agents which link to another fixed body

	HK_ASSERT(0, &trackToScan == &entity->m_simulationIsland->m_agentTrack); // that's just what it's ment for.

	// also assert that any non-fixed island is presently in the same simulation island
	for (int i = 0; i < entity->getLinkedCollidable()->m_collisionEntries.getSize(); i++)
	{
		hkLinkedCollidable::CollisionEntry& entry = entity->getLinkedCollidable()->m_collisionEntries[i];

		hkRigidBody* otherBody = static_cast<hkRigidBody*>(entry.m_partner->getOwner());

		if (otherBody->isFixed())
		{
			// If the other entity is fixed, then we move the agentEntry together with the entity -- easy.

			hkAgentNnEntry* oldAgentEntry = entry.m_agentEntry;

			hkAgentNnMachine_CopyAndRelinkAgentEntry(agentsRemoved, oldAgentEntry);
			//or
			// steal, and do no relinking -- just yet
			//hkAgentNnEntry* newEntry = hkAgentNnMachine_AllocateEntry(agentsRemoved);
			//hkString::memCpy(newEntry, entry.m_agentEntry, agentsRemoved.m_agentSize);

			// must tidy up the trackToScan
			hkAgentNnMachine_InternalDeallocateEntry(trackToScan, oldAgentEntry);
		}
		else
		{
			// Info: During broadphase we have the world locked. We still create agents 'asynchronously' (in relation to all
			// hkWorldOperations) there for agents may, and do connect different islands here.

			// Now, if we're just changing to fixed state then we need to make sure this agent stays in the partner entiti's island.
			if (newMotionType == hkMotion::MOTION_FIXED)
			{
				hkSimulationIsland* islandOfAgent = hkWorldAgentUtil::getIslandFromAgentEntry(entry.m_agentEntry, entity->m_simulationIsland, otherBody->m_simulationIsland);
				if (islandOfAgent == entity->m_simulationIsland)
				{
					// Need to move the agent to the other island now
					hkAgentNnEntry* oldAgentEntry = entry.m_agentEntry;

					hkAgentNnMachine_CopyAndRelinkAgentEntry(otherBody->m_simulationIsland->m_agentTrack, entry.m_agentEntry);
					
					hkAgentNnMachine_InternalDeallocateEntry(trackToScan, oldAgentEntry);
				}
			}

			// If we're changing from fixed -- we don't care -- it's in dynamic already, and we'll get a merge request soon.
		}
	}
}

void HK_CALL hkWorldOperationUtil::cleanupDirtyIslands( hkWorld* world )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	// Info:
	// Dirty islands list is processed by popping the last element. This way it may freely expand and shrink
	// (due to operations performed in activation/deactivation callbacks) and no locking is required.
	while (world->m_dirtySimulationIslands.getSize())
	{
		hkSimulationIsland* island = world->m_dirtySimulationIslands.back();
		world->m_dirtySimulationIslands.popBack();

		// Process if island was not deleted.
		if (island)
		{
			island->m_dirtyListIndex = HK_INVALID_OBJECT_INDEX;

#			if defined HK_ENABLE_EXTENSIVE_WORLD_CHECKING
				island->isValid();
#			endif 

			// Info:
			// Actions are claned-up first, as that fires no callbacks.
			// Activation/Deactivation is performed at the end, as this fires callbacks which may delete the island.
			// It may also modify the m_dirtySimulationIslands list.

			//
			// Clean up actions if requested
			//
			if (island->m_actionListCleanupNeeded)
			{
				hkArray<hkAction*>& actions = island->m_actions;

				int freeSlot = -1;
				int a = 0;
				for ( ; a < actions.getSize(); a++)
				{
					if (actions[a] == hkNullAction::getNullAction())
					{
						freeSlot = a;
						a++;
						break;
					}
				}

				for ( ; a < actions.getSize(); a++)
				{
					if (actions[a] != hkNullAction::getNullAction())
					{
						actions[freeSlot++] = actions[a];
					}
				}

				if (freeSlot != -1)
				{
					actions.setSize(freeSlot);
				}
				island->m_actionListCleanupNeeded = false;
			}

			//
			// Activate or Deactivate islands
			//
			if (island->m_active != island->m_isInActiveIslandsArray)
			{
				// After the next line the island pointer is unsafe.
				if ( island->m_active )
				{
					internalActivateIsland(world, island);
				}
				else
				{
					internalDeactivateIsland(world, island);
				}
			}

		}
	}
}


void hkWorldOperationUtil::internalActivateIsland( hkWorld* world, hkSimulationIsland* island)
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	HK_ASSERT(0x3ada00c7,  island->m_active && !island->m_isInActiveIslandsArray);
	HK_ASSERT(0xf0381288, world->m_wantSimulationIslands );
	HK_ASSERT(0xf0ff0073, !island->isFixed());

	world->m_inactiveSimulationIslands[island->m_storageIndex] = world->m_inactiveSimulationIslands[world->m_inactiveSimulationIslands.getSize() - 1];
	world->m_inactiveSimulationIslands[island->m_storageIndex]->m_storageIndex = island->m_storageIndex;
	world->m_inactiveSimulationIslands.popBack();

	world->m_activeSimulationIslands.pushBack( island );
	island->m_storageIndex = hkObjectIndex(world->m_activeSimulationIslands.getSize() - 1);

	island->m_isInActiveIslandsArray = true;

	// To ensure that a entity is never deactivated immediately after
	// calling forceActivate(...) we 'reset' the time stamps for the island.
	island->m_timeSinceLastHighFrequencyCheck = 0.f;
	island->m_timeSinceLastLowFrequencyCheck = 0.f;

	//for all entities: update sweptTransform
	{
		for (int e = 0; e < island->m_entities.getSize(); e++)
		{
			hkRigidBody* body = static_cast<hkRigidBody*>(island->m_entities[e]);
			hkMotionState* ms = body->getRigidMotion()->getMotionState();
			hkSweptTransformUtil::setTimeInformation(hkTime(0.0f), 0.0f, *ms);
		}
	}

	//for all agents: warpTime
	hkWorldAgentUtil::warpTime(island, island->m_timeOfDeactivation, world->m_dynamicsStepInfo.m_stepInfo.m_startTime, *world->m_collisionInput);

	sortBigIslandToFront( world, island );

	// removing from dirty array done outside
	hkWorldCallbackUtil::fireIslandActivated( world, island );
}

void hkWorldOperationUtil::internalDeactivateIsland( hkWorld* world, hkSimulationIsland* island)
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	HK_ASSERT( 0xf0381287, world->m_wantSimulationIslands );
	HK_ASSERT(0x6f1641eb,  !island->m_active && island->m_isInActiveIslandsArray);

	// recheck motions for fast velocities
	{
		hkEntity*const* bodies = island->getEntities().begin();
		int numBodies = island->getEntities().getSize();
		bool canDeactivate = hkRigidMotionUtilCanDeactivateFinal( world->m_dynamicsStepInfo.m_stepInfo, (hkMotion*const*)bodies, numBodies, HK_OFFSET_OF(hkEntity, m_motion));
		if ( !canDeactivate )
		{
			island->m_active = true;
			return;
		}
	}


	world->m_inactiveSimulationIslands.pushBack( island );

	world->m_activeSimulationIslands[island->m_storageIndex] = world->m_activeSimulationIslands[world->m_activeSimulationIslands.getSize() - 1];
	world->m_activeSimulationIslands[island->m_storageIndex]->m_storageIndex = island->m_storageIndex;
	world->m_activeSimulationIslands.popBack();

	island->m_storageIndex = hkObjectIndex(world->m_inactiveSimulationIslands.getSize() - 1);
	island->m_isInActiveIslandsArray = false;

	// store time of deactivation to validate TIM information in agents upon island reactivation
	// note: this is world->m_dynamicsStepInfo.m_stepInfo.m_startTime before integration, it therefore refers to t1 of all
	//       hkSweptTransforms
	island->m_timeOfDeactivation = world->m_dynamicsStepInfo.m_stepInfo.m_startTime;

	// Backstep all bodies to the time of deactivation 
	// and  copy t0 := t1
	{
		for (int e = 0; e < island->m_entities.getSize(); e++)
		{
			hkRigidBody* body = static_cast<hkRigidBody*>(island->m_entities[e]);
			hkMotionState* ms = body->getRigidMotion()->getMotionState();
			hkSweptTransformUtil::freezeMotionState(island->m_timeOfDeactivation, *ms);

		
			//hkWorldOperationUtil::updateEntityBP(world, body);

			// Reference rigidMotion directly to avoid re-activation.
			body->getRigidMotion()->setLinearVelocity ( hkVector4::getZero() );
			body->getRigidMotion()->setAngularVelocity( hkVector4::getZero() );
		}
	}

	// removing from dirty array done outside
	hkWorldCallbackUtil::fireIslandDeactivated( world, island );
}

void hkWorldOperationUtil::markIslandInactive( hkWorld* world, hkSimulationIsland* island )
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	island->m_active = false;	
	putIslandOnDirtyList(world, island);
}

void hkWorldOperationUtil::markIslandInactiveMt( hkWorld* world, hkSimulationIsland* island )
{
	HK_ASSERT2( 0xf02184ed, !world->m_multiThreadLock.isMarkedForWrite(), "You can only call this function when the engine is in multithreaded mode");
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RO );

	island->m_active = false;
	world->m_islandDirtyListCriticalSection->enter();
		// the next code is identical to putIslandOnDirtyList with the checks removed
	if (island->m_dirtyListIndex == HK_INVALID_OBJECT_INDEX)
	{
		island->m_dirtyListIndex = hkObjectIndex(world->m_dirtySimulationIslands.getSize());
		world->m_dirtySimulationIslands.pushBack(island);
	}
	world->m_islandDirtyListCriticalSection->leave();
}


void hkWorldOperationUtil::markIslandActive( hkWorld* world, hkSimulationIsland* island )
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RO, island, HK_ACCESS_RW );
	island->m_active = true;
	island->m_lowFrequencyDeactivationCounter = 0;
	island->m_highFrequencyDeactivationCounter = 0;
	putIslandOnDirtyList(world, island);
}

void hkWorldOperationUtil::removeIslandFromDirtyList( hkWorld* world, hkSimulationIsland* island)
{
	HK_ACCESS_CHECK_OBJECT( world, HK_ACCESS_RW );
	HK_ASSERT(0xf0ff0076, world == island->getWorld());
	if (island->m_dirtyListIndex != HK_INVALID_OBJECT_INDEX)
	{
		// Referenced to deleted islands are replaced with HK_NULLs.
		world->m_dirtySimulationIslands[island->m_dirtyListIndex] = HK_NULL;
		island->m_dirtyListIndex = HK_INVALID_OBJECT_INDEX;
	}
}

	// Only used in setMotionType
void hkWorldOperationUtil::replaceMotionObject(hkRigidBody* body, hkMotion::MotionType newState, hkBool newStateNeedsInertia, hkBool oldStateNeedsInertia, hkWorld* world )
{
	HK_ACCESS_CHECK_OBJECT( body->getWorld(), HK_ACCESS_RW );
	if ( newStateNeedsInertia )
	{
		// Restore a previously backed-up copy of dynamic motion
		if (!oldStateNeedsInertia)
		{
			hkKeyframedRigidMotion* keyframedMotion = static_cast<hkKeyframedRigidMotion*> (body->getMotion());
			hkMaxSizeMotion* savedMotion = keyframedMotion->m_savedMotion;

			HK_ASSERT2(0xad675ddd, savedMotion, "Cannot change a fixed or keyframed body into a dynamic body if it has not been initially created as dynamic.");

			// copy selected data from current (keyframed) motion to saved motion
			keyframedMotion->getMotionStateAndVelocitiesAndDeactivationType( savedMotion );
			savedMotion->m_deactivationNumInactiveFrames[0] = 0;
			savedMotion->m_deactivationNumInactiveFrames[1] = 0;

			body->setQualityType( hkCollidableQualityType(keyframedMotion->m_savedQualityTypeIndex) );

			// copy saved motion (incl. above data) back to current motion
			hkString::memCpy16(&body->m_motion, savedMotion, sizeof(hkMaxSizeMotion)>>4);

			delete savedMotion;
		}

		// Replace the current general dynamic motion with a more 'specialized' motion
		if ( body->m_motion.getType() != newState && newState != hkMotion::MOTION_DYNAMIC )
		{
				// copy everything, including vtable pointer
			hkMaxSizeMotion oldMotion;	hkString::memCpy16(&oldMotion, &body->m_motion, sizeof(hkMaxSizeMotion)>>4);
			hkMatrix3 inertiaLocal;
			static_cast<hkMotion&>(body->m_motion).getInertiaLocal( inertiaLocal );

			hkMotionState* ms = oldMotion.getMotionState();
			hkRigidBody::createDynamicRigidMotion( newState, oldMotion.getPosition(), oldMotion.getRotation(), oldMotion.getMass(), inertiaLocal, oldMotion.getCenterOfMassLocal(), ms->m_maxLinearVelocity, ms->m_maxAngularVelocity, &body->m_motion );
			hkMotion* newMotion = &body->m_motion;

			// newMotion->m_motionState->m_objectRadius is assigned below. (Just like ..->m_maxLinearVelocity and ..->m_maxAngularVelocity)
			oldMotion.getMotionStateAndVelocitiesAndDeactivationType( newMotion );

			newMotion->setLinearDamping(  oldMotion.getLinearDamping() );
			newMotion->setAngularDamping( oldMotion.getAngularDamping() );
		}
	}
	else // not newStateNeedsInertia
	{
			// copy everything, including vtable pointer
		hkMaxSizeMotion oldMotion;	hkString::memCpy16( &oldMotion, &body->m_motion, sizeof(hkMaxSizeMotion)>>4);

		hkKeyframedRigidMotion* newMotion;
		if ( newState == hkMotion::MOTION_FIXED )
		{
			newMotion = new (&body->m_motion) hkFixedRigidMotion( body->getPosition(), body->getRotation() );

			// Copy motion state 
			newMotion->m_motionState = oldMotion.m_motionState;

			// copy deactivation counter (holds deactivation type)
			// (velocities and m_deactivationNumInactiveFrames are zeroed by the constructor)
			newMotion->m_deactivationIntegrateCounter = oldMotion.m_deactivationIntegrateCounter;

			// freeze swept transform
			if (oldMotion.m_motionState.getSweptTransform().getInvDeltaTime() != 0.0f)
			{
				// This is a fixed state so we reset hkMotionState transform and both transforms stored in the hkSweptTransform to the same value.
				hkReal freezeTime = world ? world->getCurrentTime() : oldMotion.m_motionState.getSweptTransform().getBaseTime() + 1.0f / oldMotion.m_motionState.getSweptTransform().getInvDeltaTime();
				hkSweptTransformUtil::freezeMotionState(freezeTime, newMotion->m_motionState );
			}
		}
		else
		{
			newMotion = new (&body->m_motion) hkKeyframedRigidMotion( body->getPosition(), body->getRotation() );

			// Copy motion state information from the old motion
			oldMotion.getMotionStateAndVelocitiesAndDeactivationType(newMotion);
		}

		// Manage the backed-up, stored dynamic motion. And assign the newMotion to the body.
		if (oldStateNeedsInertia)
		{
			// Store current/old dynamic motion in the new keyframed/fixed motion
			hkMaxSizeMotion* savedOldMotion = new hkMaxSizeMotion();

			// we need to backup the motion container's 'hkReferencedObject' members so that we can restore them after copying the old motion into the container
			hkUint16 memSizeAndFlagsBackup = savedOldMotion->m_memSizeAndFlags;

				// override all members including vtable (sadly this includes the reference count)
			hkString::memCpy16( savedOldMotion, &oldMotion, sizeof(hkMaxSizeMotion)>>4);

			savedOldMotion->m_memSizeAndFlags = memSizeAndFlagsBackup;
			// manually set the reference counter to 1!
			savedOldMotion->m_referenceCount  = 1;

			newMotion->m_savedMotion = savedOldMotion;
			newMotion->m_savedQualityTypeIndex = int(body->getCollidable()->getQualityType());
		}
		else
		{
			hkKeyframedRigidMotion* oldKeyframedMotion = static_cast<hkKeyframedRigidMotion*> (&oldMotion);

			// Preserve saved dynamic motion of old motion in the new keyframed/fixed motion
			newMotion->setStoredMotion(oldKeyframedMotion->m_savedMotion);
			newMotion->m_savedQualityTypeIndex = oldKeyframedMotion->m_savedQualityTypeIndex;
		}
		body->m_solverData = 0;
		body->setQualityType( newState == hkMotion::MOTION_FIXED ? HK_COLLIDABLE_QUALITY_FIXED : HK_COLLIDABLE_QUALITY_KEYFRAMED );
	}

	body->m_collidable.setMotionState( body->getRigidMotion()->getMotionState()); 

	HK_ASSERT2(0xad56bccd, body->m_collidable.m_allowedPenetrationDepth > 0.0f, "Incorrect motion after a call to setMotionType." );
	HK_ASSERT2(0xad56bcce, body->getRigidMotion()->m_motionState.m_objectRadius > 0.0f, "Incorrect motion after a call to setMotionType." );

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
