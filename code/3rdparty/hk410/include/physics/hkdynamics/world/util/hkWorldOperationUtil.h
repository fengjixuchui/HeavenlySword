/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_WORLD_UTIL_H
#define HK_DYNAMICS2_WORLD_UTIL_H

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/motion/hkMotion.h>

class hkConstraintInstance;
class hkEntity;
struct hkAgentNnTrack;
class hkContinuousSimulation;

class hkWorldOperationUtil
{
	public://private:

		enum FireCallbacks 
		{
				// this is only used in addEntity and removeEntity, when they're called from hkRigidBody::setMotionType()
			DO_NOT_FIRE_CALLBACKS_AND_SUPPRESS_EXECUTION_OF_PENDING_OPERATIONS,
			DO_FIRE_CALLBACKS
		};

	public:

			//
			// WARNING: Those are internal functions and should never be called directly.
			//
		static void HK_CALL    addEntityBP ( hkWorld* world, hkEntity* entity );
		static void HK_CALL removeEntityBP ( hkWorld* world, hkEntity* entity );
		static void HK_CALL updateEntityBP ( hkWorld* world, hkEntity* entity );

		static void HK_CALL    addPhantomBP( hkWorld* world, hkPhantom* phantom);
		static void HK_CALL removePhantomBP( hkWorld* world, hkPhantom* phantom);


			/// When islands are enabled, creates an activated/deactivated island for the entity.
		static void HK_CALL addEntitySI ( hkWorld* world, hkEntity* entity, hkEntityActivation initialActivationState );

		static void HK_CALL removeEntitySI ( hkWorld* world, hkEntity* entity );

		static void HK_CALL addConstraintToCriticalLockedIsland( hkWorld* world, hkConstraintInstance* constraint  );
		static void HK_CALL removeConstraintFromCriticalLockedIsland( hkWorld* world, hkConstraintInstance* constraint  );

		static hkConstraintInstance*
					HK_CALL addConstraintImmediately   ( hkWorld* world, hkConstraintInstance* constraint, FireCallbacks fireCallbacks = DO_FIRE_CALLBACKS );

		static void HK_CALL removeConstraintImmediately( hkWorld* world, hkConstraintInstance* constraint, FireCallbacks fireCallbacks = DO_FIRE_CALLBACKS );


			//
			// Island handling
			//

		static void HK_CALL internalActivateIsland  ( hkWorld* world, hkSimulationIsland* island );

		static void HK_CALL internalDeactivateIsland( hkWorld* world, hkSimulationIsland* island );

		static HK_FORCE_INLINE void
				HK_CALL mergeIslandsIfNeeded(                 hkEntity* entityA, hkEntity* entityB);

			// Ask for two simulation islands to be merged (merge will be delayed if necessary)
		static void HK_CALL mergeIslands    ( hkWorld* world, hkEntity* entityA, hkEntity* entityB);

		static hkSimulationIsland* 
			HK_CALL internalMergeTwoIslands ( hkWorld* world, hkSimulationIsland* islandA, hkSimulationIsland* islandB );

		static void HK_CALL removeIsland    ( hkWorld* world, hkSimulationIsland* island );

		// split a single island, internal function use splitSimulationIslands() instead,
		// if oldEntitiesOut is set, than the original entity array will be stored into oldEntitiesOut;
		static void HK_CALL splitSimulationIsland( hkSimulationIsland* currentIsland, hkWorld* world, hkArray<hkSimulationIsland*>& newIslandsOut, hkArray<hkEntity*>* oldEntitiesOut = HK_NULL );

		// split any possible active or inactive simulation islands
		static void HK_CALL splitSimulationIslands( hkWorld* world );

			// split a single island
		static void HK_CALL splitSimulationIsland( hkWorld* world, hkSimulationIsland* currentIsland );

		static void HK_CALL validateIsland  ( hkWorld* world, hkSimulationIsland* island );

		static void HK_CALL validateWorld   ( hkWorld* world );


			//
			// Internal, only used by hkRigidBody::setMotionType
			//
		static void HK_CALL removeAttachedActionsFromFixedIsland  ( hkWorld* world, hkEntity* entity, hkArray<hkAction*>& actionsToBeMoved );

		static void HK_CALL removeAttachedActionsFromDynamicIsland( hkWorld* world, hkEntity* entity, hkArray<hkAction*>& actionsToBeMoved );

		static void HK_CALL addActionsToEntitysIsland             ( hkWorld* world, hkEntity* entity, hkArray<hkAction*>& actionsToBeMoved );


			//
			// 
			//

		static void HK_CALL setRigidBodyMotionType( hkRigidBody* body, hkMotion::MotionType newState, hkEntityActivation initialActivationState, hkUpdateCollisionFilterOnEntityMode queryBroadPhaseForNewPairs);

	private:
		// used for setEntityMotionType

			// Used by setMotionType
			// This stores dynamic motion and collidable's quality type when switching to fixed or keyframed motion.
		static void HK_CALL replaceMotionObject(hkRigidBody* body, hkMotion::MotionType newState, hkBool newStateNeedsInertia, hkBool oldStateNeedsInertia, hkWorld* world );

		static HK_FORCE_INLINE void HK_CALL sortBigIslandToFront( hkWorld* world, hkSimulationIsland* island );

			// Used by setMotionType
			// Function name ends with Plus, because it also does sth else.
			//  - if the agent connects entities in two separeate dynamic islands it 
			//    moves the agentEntries to the dynamic island, which is not going to be changed to fixed. .. blah
		static void HK_CALL removeAttachedAgentsConnectingTheEntityAndAFixedPartnerEntityPlus( hkAgentNnTrack& trackToScan, hkEntity* entity, hkAgentNnTrack& agentsRemoved, hkMotion::MotionType newMotionType);

		static void HK_CALL removeAttachedConstraints( hkEntity* entity, hkArray<hkConstraintInstance*>& constraintsToBeMoved );


	public:
		static void HK_CALL cleanupDirtyIslands( hkWorld* world );


		// marks an island for deactivation. Only call this when you have single threaded rw access to the world
		static void HK_CALL markIslandInactive( hkWorld* world, hkSimulationIsland* island );

		// marks an island for deactivation. Only call this when you have multithreaded read only access to the world
		static void HK_CALL markIslandInactiveMt( hkWorld* world, hkSimulationIsland* island );

		static void HK_CALL markIslandActive( hkWorld* world, hkSimulationIsland* island );
		static void HK_CALL removeIslandFromDirtyList( hkWorld* world, hkSimulationIsland* island);


			// calculates a size information which can be used for canIslandBeSparse(world,size)
		HK_FORCE_INLINE static int  HK_CALL hkWorldOperationUtil::estimateIslandSize( int numEntities, int numConstraints );

			// returns true if the island defined by the given size (by estimateIslandSize) would be a candidate for a sparseEnabled island.
			// These are islands which hold lots of independent objects. See hkWorldCinfo::m_minDesiredIslandSize for details
		HK_FORCE_INLINE static bool HK_CALL	canIslandBeSparse( hkWorld* world, int islandSize );

		HK_FORCE_INLINE static void HK_CALL putIslandOnDirtyList( hkWorld* world, hkSimulationIsland* island);

	private:
		//void replaceMotionObject(hkRigidBody* body, hkMotion::MotionType newState, hkBool newStateNeedsInertia, hkBool oldStateNeedsInertia );

};




#include <hkdynamics/world/util/hkWorldOperationUtil.inl>

#endif // HK_DYNAMICS2_WORLD_UTIL_H

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
