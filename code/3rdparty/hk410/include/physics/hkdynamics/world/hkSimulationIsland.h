/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_SIMULATION_ISLAND_H
#define HK_DYNAMICS2_SIMULATION_ISLAND_H

#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkdynamics/entity/hkEntity.h>

#include <hkdynamics/constraint/hkConstraintOwner.h>

#include <hkinternal/collide/agent3/machine/nn/hkAgentNnTrack.h>
#include <hkcollide/dispatch/broadphase/hkBroadPhaseListener.h>

class hkUnionFind;

class hkAction;
class hkEntity;
class hkConstraintInstance;
class hkStepInfo;
class hkBroadPhase;
class hkWorld;
class hkCollisionDispatcher;
struct hkWorldDynamicsStepInfo;
class hkBroadPhaseHandlePair;
struct hkCollisionInput;
struct hkProcessCollisionOutput;

///	These objects are internally managed by hkWorld. They are created and destroyed continuously during simulation.
/// At any one time, you can view the contents of the simulation islands, by calling hkWorld::getActiveSimulationIslands
/// and hkWorld::getInactiveSimulationIslands. However you must NOT keep any handles to the simulation islands, and you
/// must not alter anything in the simulation islands.
class hkSimulationIsland : public hkConstraintOwner
{
	public:
			
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SIMISLAND);

			/// Get the list of entities in the simulation island
		inline const hkArray<hkEntity*>& getEntities() const;

			/// Get the list of actions in the simulation island
		inline const hkArray<hkAction*>& getActions() const;

			/// Returns true if the island is active. 
			/// Tech info: returns true if the island is active in the next integration step. 
			///            To see whether the island actually was integrated in the last PSI or TOI step,
			///            check hkSimulationIsland::m_isInActiveIslandsArray.
		inline hkBool isActive() const;


			/// Returns true is the island is fixed
		inline hkBool isFixed() const;

			/// Get the world
		inline hkWorld* getWorld();

			/// Gets the memory usage required to complete the next integration step on this island.
			/// This function is called internally to prevent the memory usage breaching the critical memory limit in
			/// hkMemory.
		inline int getMemUsageForIntegration();


		void calcStatistics( hkStatisticsCollector* collector) const;


		//
		// The remainder of the file is for Internal use only
		//

	protected:

		friend class hkWorld;
		friend class hkWorldOperationUtil;
		friend class hkWorldCallbackUtil;
		friend class hkSimulation;
		friend class hkContinuousSimulation;

		
		hkSimulationIsland( hkWorld* world );
		~hkSimulationIsland();
			
			// Properly sets storage indices and pointers on the entity. Updates island's data with entity's motion info.
		void internalAddEntity(hkEntity* entity);

			// Properly sets storage indices and pointers on the entity. Updates island's data with entity's motion info.
		void internalRemoveEntity(hkEntity* entity);

			// hkConstraintOwner interface implementation
		virtual void addConstraintToCriticalLockedIsland(      hkConstraintInstance* constraint );
		virtual void removeConstraintFromCriticalLockedIsland( hkConstraintInstance* constraint );
		virtual void addCallbackRequest( hkConstraintInstance* constraint, int request );

#ifdef HK_DEBUG_MULTI_THREADING
		virtual void checkAccessRw();
#endif

		void addAction( hkAction* act );
		void removeAction( hkAction* act );

	protected:

		hkBool isFullyConnected( hkUnionFind& checkConnectivityOut );


	public:
			// deprecated
			// Checks deactivators for all entities and says (return hkBool value) whether the island is OK for deactivation.
		hkBool shouldDeactivateDeprecated( const hkStepInfo& stepInfo );

		inline int getStorageIndex();

	public:

		void markForWrite( );

		inline void unmarkForWrite();

	public:

		void isValid();

		void mergeConstraintInfo( hkSimulationIsland& other );

			// helper functions for debugging multithreading
		inline void markAllEntitiesReadOnly() const;

			// helper functions for debugging multithreading
		inline void unmarkAllEntitiesReadOnly() const;

		hkMultiThreadLock& getMultiThreadLock() const { return m_multiThreadLock; }

	public:
		hkWorld*	m_world;

		int m_numConstraints;

			// Storage index of the island: either in hkWorld::m_activeSimulationIslands or hkWorld::m_inactiveSimulationIslands
			// depending on m_isInActiveIslandsArray
		hkObjectIndex m_storageIndex;

			// If the island is dirty, than the island is also stored in the hkWorld::m_dirtySimulationIslands
			// dirty islands are cleaned up in hkWorldOperationUtil::cleanupDirtyIslands
		hkObjectIndex m_dirtyListIndex;

			// A counter which is incremented each frame
			// only on every 7th frame we check for split or subisland deactivation
		hkUchar m_splitCheckFrameCounter;

			// deprecated
		hkUchar m_highFrequencyDeactivationCounter;
		hkUchar m_lowFrequencyDeactivationCounter;

			// set if a split is requested
		bool m_splitCheckRequested:2;

			// this is set if the island can be split into subislands, but it
			// is not done to avoid tiny islands.
			// See hkWorldCinfo::m_minDesiredIslandSize for details
		bool m_sparseEnabled:2;

			// set if the action array need cleanup
		bool m_actionListCleanupNeeded:2;

			// for debugging multithreading
		bool m_allowIslandLocking:2;

			// see m_storageIndex
		bool m_isInActiveIslandsArray:2;

			// desired Activation State. Can only be deactivated if m_sparseEnabled is set to false
		bool m_active:2;

			// if this is set to true, the island is integrated and cannot be modified, this is used for debugging multithreading only
		bool m_inIntegrateJob:2;

	protected:
		mutable hkMultiThreadLock m_multiThreadLock;

	public:
		hkReal m_timeSinceLastHighFrequencyCheck;
		hkReal m_timeSinceLastLowFrequencyCheck;

			// the entities (note: optimized for single entities)
#if defined(HK_PLATFORM_HAS_SPU)
			// we need to upload this array to SPU (note: the inplace array does not guarantee a 16 byte alignment)
		hkArray<hkEntity*> m_entities;
#else
		hkInplaceArray<hkEntity*,1> m_entities;
#endif

			// for narrow phase collision agents
		hkAgentNnTrack m_agentTrack;

		hkArray<hkAction*> m_actions;

		hkTime m_timeOfDeactivation;


};

#include <hkdynamics/world/hkSimulationIsland.inl>

#endif // HK_DYNAMICS2_SIMULATION_ISLAND_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
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
