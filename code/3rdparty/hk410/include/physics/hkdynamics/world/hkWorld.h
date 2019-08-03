/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_WORLD_H
#define HK_DYNAMICS2_WORLD_H

#include <hkmath/hkMath.h>

#include <hkbase/config/hkConfigVersion.h>
#include <hkbase/thread/util/hkMultiThreadLock.h>
#include <hkmath/basetypes/hkStepInfo.h>
#include <hkconstraintsolver/solve/hkSolverInfo.h>
#include <hkdynamics/world/hkWorldCinfo.h>
#include <hkdynamics/world/simulation/hkSimulation.h>
#include <hkinternal/collide/broadphase/hkBroadPhase.h>

class hkCollisionFilter;
class hkConvexListFilter;
class hkCollisionDispatcher;
class hkCollidable;
struct hkCollisionInput;
struct hkProcessCollisionInput;
class hkCollisionListener;
class hkTypedBroadPhaseDispatcher;

struct hkWorldRayCastInput;
struct hkWorldRayCastOutput;
struct hkLinearCastInput;
class hkRayHitCollector;
class hkCdPointCollector;
class hkCdBodyPairCollector;

class hkBroadPhase;
class hkAabb;

class hkEntity;
class hkEntityListener;

class hkRigidBody;

class hkPhantom;
class hkPhantomListener;
class hkPhantomBroadPhaseListener;
class hkEntityEntityBroadPhaseListener;
class hkBroadPhaseBorderListener;

class hkAction;
class hkActionListener;

class hkConstraintInstance;
class hkConstraintData;
class hkConstraintListener;

class hkPhysicsSystem;

class hkSimulationIsland;
class hkContactMgrFactory;

class hkWorldPostSimulationListener;
class hkWorldPostIntegrateListener;
class hkWorldPostCollideListener;
class hkWorldDeletionListener;
class hkIslandActivationListener;

class hkIslandPostIntegrateListener;
class hkIslandPostCollideListener;

class hkStepInfo;
class hkWorldPairwiseBackstepFilter;

class hkSimulation;
class hkWorldOperationQueue;
struct hkDebugInfoOnPendingOperationQueues;
class hkWorldMaintenanceMgr;

class hkWorldMemoryWatchDog;



namespace hkWorldOperation { struct BaseOperation; class UserCallback; }

struct hkOperationParameter;

#if defined HK_DEBUG
//# define HK_ENABLE_EXTENSIVE_WORLD_CHECKING
#endif


	///	Allows you optimize the  hkWorld::updateCollisionFilterOnWorld call
enum hkUpdateCollisionFilterOnWorldMode
{
		/// Full filter filter
	HK_UPDATE_FILTER_ON_WORLD_FULL_CHECK,

		/// Recheck filter but only check for disabled entity-entity collisions
		/// which have been enabled before
	HK_UPDATE_FILTER_ON_WORLD_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY,
};

	/// Allows you to optimize you hkWorld::updateCollisionFilterOnEntity call
enum hkUpdateCollisionFilterOnEntityMode	
{
		/// Do a full check
	HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK,

		/// Recheck filter but only check for disabled entity-entity collisions
		/// which have been enabled before
	HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY
};

	/// Activation hints when calling hkWorld::addEntity
enum hkEntityActivation
{
		/// Tries to add the body in an inactive state. However if the body overlaps with an
		/// active body, it gets activated
	HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE,

		/// Always activate this body and overlapping bodies
	HK_ENTITY_ACTIVATION_DO_ACTIVATE 
};

	/// The hkUpdateCollectionFilter specifies whether you want to reevaluate 
	/// all the sub-shape collisions with the world's shape collection filter. Typical example of this is
	/// destructible terrain. So if you just disabled the collision of a bullet and a car, you do not
	/// need to recheck existing landscape collisions and can set 
	/// updateShapeCollectionFilter to HK_UPDATE_COLLECTION_FILTER_IGNORE_SHAPE_COLLECTIONS
enum hkUpdateCollectionFilterMode 
{
		/// Assume that no single shapes in a shape collections changed their filter status
	HK_UPDATE_COLLECTION_FILTER_IGNORE_SHAPE_COLLECTIONS,

		/// Recheck all subshapes in a shape collection
	HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS
};



	/// A structure used by the hkWorld that groups together solver and step time information. 
struct hkWorldDynamicsStepInfo
{
	hkStepInfo   m_stepInfo;
	hkSolverInfo m_solverInfo;
};

	/// This structure is currently only used for multi threaded simulations. It controls how the multi threading is run.
struct hkMultithreadConfig
{
	hkMultithreadConfig();

		/// PLAYSTATION(R)3 only. A flag used to determine if the cpu can take SPU tasks.
	enum CanCpuTakeSpuTasks
	{
			/// If only SPU jobs are available, and no SPU is waiting, the
			/// CPU will take the task. This is the default setting.
		CPU_CAN_TAKE_SPU_TASKS,

			/// The CPU will never take SPU tasks, even if only SPU tasks are
			/// available.
		CPU_CAN_NOT_TAKE_SPU_TASKS
	};

#if defined(HK_PLATFORM_HAS_SPU)
		/// This variable is used to set whether the cpu can take spu tasks. It defaults to
		/// true. WARNING: If you set this to false, and do not have the correct SPU program
		/// running on at least one SPU during the step() call, the step function will stall indefinitely.
	CanCpuTakeSpuTasks m_canCpuTakeSpuTasks;
#endif
		/// This variable is used to control when the constraints of a simulation island are solved in
		/// single threaded or multithreade mode.  On PC and XBOX360 this value is 64, which means that
		/// an island must have greater than 64 constraints to be solved in multithreaded mode.  If it 
		/// is set to 0 then the constraints of all simulation islands will be solved in multithreaded mode.
		/// The reason it is not set to 0 by default is because there is a small overhead in setting up 
		/// multithreading constraints for a simulation island, which can be significant for small islands.
		/// On the PS3 this variable defaults to 8.  SPU constraint setup only happens if the island
		/// has more constraints than are specified by this variable.  
	hkUint32 m_maxNumConstraintsSolvedSingleThreaded;
};

	/// A token used to indicate whether a thread is the primary thread or a secondary thread.
enum hkThreadToken
{
		/// Primary thread
	HK_THREAD_TOKEN_PRIMARY,
		/// Secondary thread
	HK_THREAD_TOKEN_SECONDARY
};


	/// The hkWorld is a container for the simulation's physical objects.  It also steps the simulation forward in time.
	/// You add elements (including rigid bodies, actions, constraints, and listeners) to the simulation by adding them to the hkWorld.
class hkWorld : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_WORLD);

			/// Creates a world with the given construction information
		hkWorld(const hkWorldCinfo& info, unsigned int sdkversion = HAVOK_SDK_VERSION_NUMBER );

			/// Get construction info for this instance.
		void getCinfo(hkWorldCinfo& info) const;


			//
			//	hkWorld Operations:
			//
			//  Adding/Removing Instances/BatchesOfInstances of Entities/Constraints/Actions/Phantoms
			//

			/// Adds the specified entity to the world and returns its pointer
			/// or returns HK_NULL if adding the body is delayed.
			/// Lets you specify the desired activation state. 
			/// Usage info: Note that even if you add a body inactive, it may be immediately automatically activated 
			///             by the engine due to overlaps with other bodies.
			/// Note: When using HK_ENTITY_ACTIVATION_DO_ACTIVATE parameter, the entity is added in the active state,
			///       however no activationCallbacks are called.
		hkEntity* addEntity( hkEntity* entity, hkEntityActivation initialActivationState = HK_ENTITY_ACTIVATION_DO_ACTIVATE);

			/// Removes the specified entity from the world 
			/// During a simulation step the removal of the entity is delayed until it is safe to do so.
			/// returns true if removed immediately false if delayed.
		hkBool removeEntity( hkEntity* entity);


			/// Efficiently add a group of entities at the same time.  Will not be as efficient when
			/// adding a small number of entities, in that case use addEntity.
			/// Lets you specify the desired activation state. 
			/// Warning: When adding entities inactive: all entities are added in the same island, 
			///          therefore any overlap with an active entity (already inserted into the world)
			///          causes activation of the whole island.
			/// Note: When using HK_ENTITY_ACTIVATION_DO_ACTIVATE parameter, the entity is added in the active state,
			///       however no activationCallbacks are called.
		void addEntityBatch( hkEntity*const* entityBatch, int numEntities, hkEntityActivation initialActivationState = HK_ENTITY_ACTIVATION_DO_ACTIVATE);

			/// Efficiently remove a group of entities at the same time.  Will not be as efficient when
			/// removing a small number of entities, in that case use removeEntity.  
		void removeEntityBatch( hkEntity*const* entityBatch, int numEntities );


			//
			//	Constraint addition and removal
			//

			/// Adds the specified constraint to the world
		hkConstraintInstance* addConstraint( hkConstraintInstance* constraint);

			/// Removes the specified constraint from the world
			/// returns true if removed immediately false if delayed
		hkBool removeConstraint( hkConstraintInstance* constraint);

			/// Utility function for creating and adding a specified constraint instance to the world
		hkConstraintInstance* createAndAddConstraintInstance( hkRigidBody* bodyA, hkRigidBody* bodyB, hkConstraintData* constraintData );
		


			//
			// Actions
			//
			
			/// Adds the specified action to the world. A reference to the action is added to every entity on the action's list
			/// of entities. All those entities must already by placed in an hkWorld.
		hkAction* addAction( hkAction* action ); 

			/// Remove the specified action from the world and removes its reference from each entity that it was connected to.
		void removeAction( hkAction* action );	
			
			/// Remove the specified action from the world and removes its reference from each entity that it was connected to.
			/// It bypasses the operation delay framework, there fore you _MUST_BE_SURE_ that _IT_IS_SAFE_ to use it.
			/// Info: use this only in hkAction::entityRemovedCallback
			/// Non-symmetric function. Only needed to be used explicitly when removing an entity causes removal of an action in a callback. 
			/// addActionImmediately not needed.
		void removeActionImmediately( hkAction* action );	


			//
			// Phantoms
			//

			/// Add the phantom to the world
		hkPhantom* addPhantom( hkPhantom* phantom );

			/// Add multiple phantoms to the world
		void addPhantomBatch( hkPhantom*const* phantomBatch, int numPhantoms );


			/// Remove the phantom from the world
		void removePhantom( hkPhantom* phantom );

			/// Remove multiple phantoms from the world
		void removePhantomBatch( hkPhantom*const* phantomBatch, int numPhantoms );

			/// Activates all entities whose aabb overlaps with the specified region
		void activateRegion( const hkAabb& aabb );


			// 
			// Systems (bodies, actions and phantoms all together). For instance 
			// a vehicle or a ragdoll.

			/// Add a hkPhysicsSystem to the world. The world does not
			/// store or reference the system struct itself, just 
			/// uses the data held to batch add to the world.
		void addPhysicsSystem( const hkPhysicsSystem* sys );

			/// Remove a hkPhysicsSystem from the world. 
			/// The world will just look through the lists in the given system
			/// and remove all found. It does not alter the system struct itself.
		void removePhysicsSystem( const hkPhysicsSystem* sys );

			//
			// Fixed rigid body
			//


			/// Gets the world's fixed rigid body. This is a fixed body that can be used to constrain an object to
			/// a position in world space without needing to create an additional fixed object. The world's fixed
			/// body differs from 'normal' rigid bodies in a number of ways, including but not limited to the
			/// following:<br>
			/// It doesn't have a shape/collision representation and so it never collides with anything.<br>
			/// It doesn't have a broadphase entry and so it cannot go outside the broadphase, resulting in a
			/// potential performance hit as 'normal' rigid bodies can.
		inline hkRigidBody* getFixedRigidBody();

		/// Gets read access to the world's fixed rigid body.
		/// See the non-const version of this function for more information.
		inline const hkRigidBody* getFixedRigidBody() const;

			//
			//  Collision filter update methods
			//

			/// This method should be called if you have altered the collision filtering information for this entity.
			/// Note: this does not activate the entity.
			/// The collisionFilterUpdateMode allows you to do an optimization: Only previously enabled entity entity collisions are 
			/// checked. However entity-phantom or new entity-entity collision are ignored.
			/// The updateShapeCollectionFilter specifies whether you want to reevaluate 
			/// all the sub-shape collisions with the world's shape collection filter. Typical example of this is
			/// destructible terrain. So if you just disabled the collision of a bullet and a car, you do not
			/// need to recheck existing landscape collisions and can set 
			/// updateShapeCollectionFilter to HK_UPDATE_COLLECTION_FILTER_IGNORE_SHAPE_COLLECTIONS
		void updateCollisionFilterOnEntity( hkEntity* entity, 
			                                hkUpdateCollisionFilterOnEntityMode updateMode,
											hkUpdateCollectionFilterMode updateShapeCollectionFilter);

			/// This method should be called if you have altered the collision filtering information for this phantom. Read updateCollisionFilterOnEntity() for details
		void updateCollisionFilterOnPhantom( hkPhantom* phantom, 
											 hkUpdateCollectionFilterMode updateShapeCollectionFilter );	

			/// This method should be called if you have changed the collision filter for the world.
			///
			/// WARNING: This method is very slow. It involves re-evaluating all the broad phase aabbs in the system.
			/// You should only ever call this at startup time.
			/// updateMode allows you to use a shortcut if you only want to disable previously enabled entity entity
			/// collisions.
			/// The updateShapeCollectionFilter specifies whether you want to reevaluate 
			/// all the sub-shape collisions with the world's shape collection filter. Typical example of this is
			/// destructible terrain. So if you just disabled the collision of a bullet and a car, you do not
			/// need to recheck existing landscape collisions and can set 
			/// updateShapeCollectionFilter to HK_UPDATE_COLLECTION_FILTER_IGNORE_SHAPE_COLLECTIONS
		void updateCollisionFilterOnWorld( hkUpdateCollisionFilterOnWorldMode updateMode,
										   hkUpdateCollectionFilterMode updateShapeCollectionFilter );

			//
			// Phantoms
			//


			/// Get a list of all the phantoms which have been added to the world
		inline const hkArray<hkPhantom*>& getPhantoms() const;


			//
			// Gravity
			//

			/// Sets the gravity value for the world. This does not wake up objects
		void setGravity( const hkVector4& gravity );

			/// Gets the world's gravity value.
		inline const hkVector4& getGravity() const; 


			//
			// Simulation Control
			//


			/// This utility function may be called to initialize the contact points for a set of entities added to the world.
			/// By default contact points are evaluated conservatively, for simulation efficiency.
			/// However this can lead to a slight initial jitter when objects are added.  The same applies to objects for
			/// objects which have had their collision agents deleted upon deactivation when they reactivate. (See 
			/// the helper class hkSuspendInactiveAgentsUtil for details. ) This function helps to remove this jitter by
			/// trying to find all the initial relevant contact points for a set of entities.
		void findInitialContactPoints( hkEntity** entities, int numEntities );

			/// This simply calls initializeContactPoints for all objects in the world. It is a utility function 
			/// which can be called after the world has been initialized and all objects added to it.
		void findInitialContactPointsOfAllEntities( );



			/// Gets the memory usage required to complete the next integration step
			/// This function is called internally to prevent the memory usage breaching the critical memory limit in
			/// hkMemory.
		int getMemUsageForIntegration();

			/// Calculate the memory statistics for the world, and all objects owned by the world
		void calcStatistics( hkStatisticsCollector* collector ) const;

			/// checks the current state of the engine using the hkCheckDeterminismUtil
		void checkDeterminism();


			/// Get the memory "watch dog" used by the world. By default this is HK_NULL, i.e. the memory
			/// for the world is not checked. If this is set, hkWorldMemoryWatchDog::watchMemory will be
			/// called every step, which gives the hkWorldMemoryWatchDog a chance to prevent the memory
			/// exceeding a desired threshold.
		hkWorldMemoryWatchDog* getMemoryWatchDog( ) const;

			/// Set the memory "watch dog" used by the world. By default this is HK_NULL, i.e. the memory
			/// for the world is not checked. If this is set, hkWorldMemoryWatchDog::watchMemory will be
			/// called every step, which gives the hkWorldMemoryWatchDog a chance to prevent the memory
			/// exceeding a desired threshold.
		void setMemoryWatchDog( hkWorldMemoryWatchDog* watchDog );

			//
			// Multithreaded Access 
			//

			/// Locks the world in a multithreaded environment.
			/// This can only be called outside any step function and should
			/// only be used for asynchronous queries like raycast, etc.
			/// lock() and unlock() acquire and release critical sections, so they are quite slow.
			/// For this reason it is best to lock the world and do many operations on the world when you have
			/// acquired the lock before unlocking it.
			/// If your own synchronization mechanisms take care of preventing multiple threads trying to access
			/// the world simulatneously, you dont need to call lock / unlock. YOu can instead call markForRead
			/// and the associated functions to enable debug checks for your synchronization.
			/// Read the multithreading use guide about details
		void lock();

			/// Unlocks the world. Opposite of lock()
		void unlock();

			/// Returns true if the world was locked by any thread. Returns false in single threaded mode.
		bool isLocked(); 


		// Internal function: Locks all entities in the simulation island for constraint updates
		void lockIslandForConstraintUpdate( hkSimulationIsland* island );

		// Internal function: opposite of lockIslandForConstraintUpdate
		void unlockIslandForConstraintUpdate( hkSimulationIsland* island );


			//
			// Multithreaded Access checks
			//
		
		enum MtAccessChecking
		{
			MT_ACCESS_CHECKING_ENABLED = 0,
			MT_ACCESS_CHECKING_DISABLED
		};

			/// Enable or disable multithreaded access checking
			/// By default this is enabled if the world simulation type is SIMULATION_TYPE_MULTITHREADED
			/// and false is it is anything else.
		inline void setMultithreadedAccessChecking( MtAccessChecking accessCheckState );

			/// Get whether multithreaded access checking is enabled
			/// By default this is enabled if the world simulation type is SIMULATION_TYPE_MULTITHREADED
			/// and false is it is anything else.
		inline MtAccessChecking getMultithreadedAccessChecking() const;
		
			/// Mark this class and all child classes for read only access for this thread
			/// Note: This is only for debugging and does not wait to get exclusive access, 
			/// but simply assert if another thread marked the hkWorld. You must read the
			/// user guide about multithreading to use this.
		inline void markForRead( ) const;

			/// Mark this class and all child classes for read write access for this thread
			/// Note: This is only for debugging and does not wait to get exclusive access, 
			/// but simply assert if another thread marked the hkWorld. You must read the
			/// user guide about multithreading to use this.
		inline void markForWrite( );

			/// Undo lockForRead
			/// Note: This is only for debugging and does not wait to get exclusive access, 
			/// but simply assert if another thread locked the hkWorld. You must read the
			/// user guide about multithreading to use this.
		inline void unmarkForRead( ) const;

			/// Unlock For write
			/// Note: This is only for debugging and does not wait to get exclusive access, 
			/// but simply assert if another thread locked the hkWorld. You must read the
			/// user guide about multithreading to use this.
		inline void unmarkForWrite();


			//
			// Event Handling
			//

			/// Adds the specified listener to the world
		void addActionListener( hkActionListener* worldListener );

			/// Removes the specified listener from the world
		void removeActionListener( hkActionListener* worldListener );

			/// Adds the specified listener to the world
		void addConstraintListener( hkConstraintListener* worldListener );

			/// Removes the specified listener from the world
		void removeConstraintListener( hkConstraintListener* worldListener );

			/// Adds the specified listener to the world
		void addEntityListener( hkEntityListener* worldListener );

			/// Removes the specified listener from the world
		void removeEntityListener( hkEntityListener* worldListener );


			/// Adds the specified listener to the world
		void addPhantomListener( hkPhantomListener* worldListener );

			/// Removes the specified listener from the world
		void removePhantomListener( hkPhantomListener* worldListener );

		
			/// Adds a listener which listens to ALL entity activations.
		void addIslandActivationListener( hkIslandActivationListener* worldListener );

			/// Removes the specified listener from the world
		void removeIslandActivationListener( hkIslandActivationListener* worldListener );

			/// Adds the specified listener to the world
		void addWorldPostCollideListener( hkWorldPostCollideListener* worldListener );

			/// Removes the specified listener from the world
		void removeWorldPostCollideListener( hkWorldPostCollideListener* worldListener );

			/// Adds the specified listener to the world
		void addWorldPostSimulationListener( hkWorldPostSimulationListener* worldListener );

			/// Removes the specified listener from the world
		void removeWorldPostSimulationListener( hkWorldPostSimulationListener* worldListener );

			/// Adds the specified listener to the world
		void addWorldPostIntegrateListener( hkWorldPostIntegrateListener* worldListener );

			/// Removes the specified listener from the world
		void removeWorldPostIntegrateListener( hkWorldPostIntegrateListener* worldListener );

			/// Adds the specified listener to the world
		void addIslandPostCollideListener( hkIslandPostCollideListener* islandListener );

			/// Removes the specified listener from the world
		void removeIslandPostCollideListener( hkIslandPostCollideListener* islandListener );

			/// Adds the specified listener to the world
		void addIslandPostIntegrateListener( hkIslandPostIntegrateListener* islandListener );

			/// Removes the specified listener from the world
		void removeIslandPostIntegrateListener( hkIslandPostIntegrateListener* islandListener );

			/// Adds a listener which listens to ALL collisions.
		void addCollisionListener( hkCollisionListener* worldListener );

			/// Removes a world collision listener. 
		void removeCollisionListener( hkCollisionListener* worldListener);

			/// Add a listener which receives events when the world is deleted
		void addWorldDeletionListener( hkWorldDeletionListener* worldListener );

			/// Remove a listener which receives events when the world is deleted
		void removeWorldDeletionListener( hkWorldDeletionListener* worldListener );


			//
			// Collision detection
			//

			/// Gets write access the broad phase owned by the world
		inline hkBroadPhase* getBroadPhase();

			/// Gets read only access to the broad phase owned by the world
		inline const hkBroadPhase* getBroadPhase() const;

			/// Get read access to the collision input. This is needed to manually query collision agents.
		inline const hkProcessCollisionInput* getCollisionInput() const;

			/// Get read/write access to the collision input. This is needed to manually query collision agents.
		inline hkProcessCollisionInput* getCollisionInput();

		inline hkSolverInfo* getSolverInfo();

			/// Gets the collision dispatcher owned by the world.
		inline hkCollisionDispatcher* getCollisionDispatcher() const;

			/// Gets the collision filter.
		inline const hkCollisionFilter* getCollisionFilter() const;

			/// Set the collision filter. This is used by the world by default when it calls the collision detector.
			/// WARNING: This should only be done before any objects are added to this world.
		void setCollisionFilter( hkCollisionFilter* filter,
								 hkBool             runUpdateCollisionFilterOnWorld = true, 
								 hkUpdateCollisionFilterOnWorldMode checkBroadPhaseMode = HK_UPDATE_FILTER_ON_WORLD_FULL_CHECK,				
								 hkUpdateCollectionFilterMode       updateShapeCollectionFilter = HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );

			/// Cast a ray into the world and get the closest hit.
			/// Note: it uses the hkWorld::getCollisionFilter() for filtering
			/// This function uses a ray-cast function for traversing the broad phase, so for
			/// long raycasts it is more applicable than hkAabbPhantom::castRay
		void castRay(const hkWorldRayCastInput& input, hkWorldRayCastOutput& output ) const;

			/// Cast a ray into the world and do a callback for every hit.
			/// Note: it uses the hkWorld::getCollisionFilter() for filtering
			/// This function uses a ray-cast function for traversing the broad phase, so for
			/// long raycasts it is more applicable than hkAabbPhantom::castRay
		void castRay(const hkWorldRayCastInput& input, hkRayHitCollector& collector ) const;


			/// Cast a shape within the world.
			/// The castCollector collects all potential hits.<br>
			/// Note that the distance stored within the castCollector is a hitFraction (between 0 and 1.0) and
			/// not a distance.<br>
			/// This function uses an aabb-cast function for traversing the broad phase, so for
			/// long linear casts it this will perform better than hkShapePhantom::linearCast
			/// The [optional] startPointCollector returns all the closest points
			/// at the start point of the linear cast. If you do not want this functionality, pass HK_NULL as the
			/// "startCollector".
			/// Note that shape radius is considered - you may want to set the radius of the cast shape to zero.
		void linearCast( const hkCollidable* collA, const hkLinearCastInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startCollector = HK_NULL ) const;

			/// Get the closest points to a hkCollidable (= [shape,transform,filterInfo] ).
			/// Note: If you have call this function every step for a given object, use the hkShapePhantom::getClosestPoints()
			/// By default, you should pass a pointer to world->getCollisionInput() via hkCollisionInput* input. However you 
			/// can use this parameter to pass in your own collision input
			/// structure, and change the shape collection collision filter, and the collision tolerance used. 
			/// (Note: If using your own collision input, make sure you start by copying the world collision input, so that
			/// you use the default hkCollisionDispatcher).
		void getClosestPoints( const hkCollidable* collA, const hkCollisionInput& input, hkCdPointCollector& collector) const;

			/// Get all shapes which are penetrating the collidable.
			/// Note: If you have call this function every step for a given object, use the hkShapePhantom version
			/// By default, you should pass a pointer to world->getCollisionInput() via hkCollisionInput* input. However you 
			/// can use this parameter to pass in your own collision input
			/// structure, and change the shape collection collision filter, and the collision tolerance used. 
			/// (Note: If using your own collision input, make sure you start by copying the world collision input, so that
			/// you use the default hkCollisionDispatcher).
		void getPenetrations( const hkCollidable* collA, const hkCollisionInput& input, hkCdBodyPairCollector& collector) const;


			/// Gets the current broadphase border, could be null.
		class hkBroadPhaseBorder* getBroadPhaseBorder() const;


			/// Sets a custom broadphase border.
			/// By default the hkWorld constructor checks the
			/// hkWorldCinfo.m_broadPhaseBorderBehaviour field and will create a default
			///	broadphase border if needed. Real hkWorldCinfo.m_broadPhaseBorderBehaviour for details
		void setBroadPhaseBorder( hkBroadPhaseBorder* bpb );


			//
			// Simulation Island access
			//

			/// Get the active simulation islands. You can use this function to access all the active entities, constraints and actions.
			/// NOTE: This should not be used during simulation, as it will generally result in sub-optimal performance.
		inline const hkArray<hkSimulationIsland*>& getActiveSimulationIslands() const;

			/// Get the inactive simulation islands. You can use this function to access all the inactive entities, constraints and actions.
			/// NOTE: This should not be used during simulation, as it will generally result in sub-optimal performance.		
		inline const hkArray<hkSimulationIsland*>& getInactiveSimulationIslands() const;

			/// Get the fixed simulation island. You can use this function to access all the fixed entities.
			/// There is only one fixed island, and it contains no actions or constraints.
			/// NOTE: This should not be used during simulation, as it will generally result in sub-optimal performance.
		inline const hkSimulationIsland* getFixedIsland() const;


			//
			// Stepping the world (Default)
			//


			/// Advances the state of the simulation by the amount of time.
			/// This function may also be called for a multithreaded simulation.  If you are stepping
			/// the world multithreaded you must call resetThreadTokens() in a single thread, then for
			/// each child thread you call this function. You must implement a barrier so that no
			/// child thread calls stepDeltaTime twice before resetThreadTokens is called again.
			/// Normally this returns HK_STEP_RESULT_SUCCES, but may return a failure code on memory failure.
			/// In this case memory should be freed (by removing objects from the world) and the function should be called again.
			/// There is also finer grained interface to multithreading simulation below.			
		hkStepResult stepDeltaTime( hkReal physicsDeltaTime );


			//
			// "Half" stepping (Single threaded only)
			//

			/// Integrate and solve constraints between all bodies. 
			/// Using this call instead of stepDeltaTime() allows you to distribute your
			/// physics computation for a single physics step between multiple game steps. 
			/// Normally this returns HK_STEP_RESULT_SUCCES, but may return a failure code on memory failure.
			/// In this case memory should be freed (by removing objects from the world) and the function should be called again.
			///	Please consult the user guide for more details.
		hkStepResult integrate( hkReal physicsDeltaTime );

			/// Perform collision detection.
			/// Using this call instead of stepDeltaTime() allows you to distribute your
			/// physics computation for a single physics step between multiple game steps. 
			/// Normally this returns HK_STEP_RESULT_SUCCES, but may return a failure code on memory failure.
			/// In this case memory should be freed (by removing objects from the world) and the function should be called again.
			///	Please consult the user guide for more details.
		hkStepResult collide();

			/// Advance the current time of the world.
			/// For discrete simulation, this simply advances the current time value,
			/// For continuous simulations, this will also perform the continuous physics calculations.
			/// Using this call instead of stepDeltaTime() allows you to distribute your
			/// physics computation for a single physics step between multiple game steps. 		
			/// This call is also used for asynchronous simulation: 
			/// It will advance the time to the next PSI step, unless
			/// an earlier frame timer marker has been set (see setFrameTimeMarker).
			/// Normally this returns HK_STEP_RESULT_SUCCES, but may return a failure code on memory failure.
			/// In this case memory should be freed (by removing objects from the world) and the function should be called again.
			///	Please consult the user guide for more details.
		hkStepResult advanceTime();


			//
			// Asychronous Stepping
			// 

			/// This call can be used to step the world asynchronously.  If you set a marker, then
			/// when advanceTime is called, it will advance either to this marker, or the current
			/// psi time, which ever is earlier.  This is illustrated by 
			/// hkAsynchronousTimestepper::stepAsynchronously. It can also be used in conjunction with
			/// multithreading simulation.
			/// Once you start using setFrameTimeMarker, you must continue to use it for all subsequent
			/// stepping.
			///	Please consult the user guide for more details.
		void setFrameTimeMarker( hkReal frameDeltaTime );

			/// This call can be used to step the world asychronously. It returns true if the simulation
			/// time is equal to the frame marker time. If the frameTimeMarker has not been set this returns
			/// false.
			///	Please consult the user guide for more details.
		bool isSimulationAtMarker() const;

			/// This call returns true if the simulation current time is equal to the simulation currentPsiTime.
			/// It can be used as a debug check when stepping the world asychronously.
			///	Please consult the user guide for more details.
		bool isSimulationAtPsi() const;

			//
			// Multithreaded stepping
			//

			/// Single threaded setup for multithreaded simulation.  Only one thread must call this function. When it
			/// returns all threads should call stepProcessMt.  No thread should call stepProcessMt until this function 
			/// completes. This function leaves the world in a markForRead() state, for stepProcesMt().  This state is
			/// reset in stepEndSt().
		void stepBeginSt( hkReal physicsDeltaTime );

			/// Multithreaded simulation.  This processes the integration and collision detection for all simulation islands.
			/// It is should be callled by all threads, after one thread has called stepBeginSt. 
		void stepProcessMt( const hkThreadToken& token );

			/// Single threaded function to end a multithreaded step.  Only one thread must call this function. It must be
			/// called when all threads have returned from calling stepProcessMt.
		void stepEndSt();


			/// This call must be called single threaded prior to calling getThreadToken or stepDeltaTime(). If you
			/// are using the fine grained multithreaded simulation functions (stepBeginSt, stepProcessMt and stepEndSt)	
			/// and have identified your own primary thread, and are not calling getThreadToken, you do not need to call
			/// this function.
		void resetThreadTokens();

			/// This returns HK_THREAD_TOKEN_PRIMARY for the first thread to call it, then HK_THREAD_TOKEN_SECONDARY
			/// for all subsequent threads.  When resetThreadTokens is called, the next thread token this returns
			/// will be HK_THREAD_TOKEN_PRIMARY.  This function is a utility function which does not need to be called
			/// if you have identified your own primary thread. 
		hkThreadToken getThreadToken();

			//
			// Time accessors
			//


			/// This returns current time of the simulation. Use it to calculate current transforms of rigid bodies
			/// in between PSI steps (with hkRigidBody::approxTransformAt). Note that this is not absolute time; its value is reset every 30 seconds or so.
		inline hkTime getCurrentTime() const;

			/// This returns the current "PSI" time of the simulation.  The PSI time is always the same or greater than
			/// the current time.  See the user guide for details.
		inline hkTime getCurrentPsiTime() const;


			//
			// Multithreaded simulation details accessors
			//


			/// This function is only valid when using multithreaded simulation. If using other
			/// simulations, it returns HK_NULL.
		class hkJobQueue* getJobQueue();


			/// This function is only valid for multithreading simulation.  It allows you to access
			/// some settings used for PLAYSTATION(R)3 multithreading simulation.
		void getMultithreadConfig( hkMultithreadConfig& config );

			/// This function is only valid for multithreading simulation. It allows you to control
			/// some settings used for PLAYSTATION(R)3 multithreading simulation.
		void setMultithreadConfig( const hkMultithreadConfig& config );

		hkMultiThreadLock& getMultiThreadLock() const { return m_multiThreadLock; }

		void checkAccessGetActiveSimulationIslands() const;

		//
		// Serialization Utilities
		//

			/// Get a hkPhysicsSystem that represents the whole system 
			/// contained in this world. This can be used as a serialization utility
			/// for taking a snapshot of the world.
			/// Release the physics system when you are finished using it.
		hkPhysicsSystem* getWorldAsOneSystem() const;

			/// Get an array of hkPhysicsSystem(s) that represents the whole system 
			/// contained in this world. This can be used as a serialization utility
			/// for taking a snapshot of the world.
			/// Release the physics systems when you are finished using it.
		void getWorldAsSystems(hkArray<hkPhysicsSystem*>& systemsOut) const;

		/*
		**	Protected Functions
		*/

			// you should use removeReference() instead, however we allow you to force destroy the world
		~hkWorld();

		/*
		**	protected members: They are public so that people can get full control over the engine
		**  if they know what they are doing!!!
		*/
	public:

			// A class controlling the simulation
		hkSimulation* m_simulation;


		hkVector4			m_gravity;

		hkSimulationIsland*	m_fixedIsland;
			hkRigidBody*	m_fixedRigidBody;

		hkArray<hkSimulationIsland*> m_activeSimulationIslands;
		hkArray<hkSimulationIsland*> m_inactiveSimulationIslands;

			// Dirty islands are scheduled for cleanup, performed  by hkWorldOperationUtil::cleanupDirtyIslands( )
		hkArray<hkSimulationIsland*> m_dirtySimulationIslands;

		hkWorldMaintenanceMgr* m_maintenanceMgr;
		hkWorldMemoryWatchDog* m_memoryWatchDog;

		hkBroadPhase* m_broadPhase;

			// Broad phase handler - this is a class which links the broad phase to the simulation
		hkTypedBroadPhaseDispatcher*    m_broadPhaseDispatcher;
		hkPhantomBroadPhaseListener*    m_phantomBroadPhaseListener;
		hkEntityEntityBroadPhaseListener* m_entityEntityBroadPhaseListener;
		hkBroadPhaseBorderListener* m_broadPhaseBorderListener;


		hkProcessCollisionInput* m_collisionInput;
			hkCollisionFilter*       m_collisionFilter;
			hkCollisionDispatcher*   m_collisionDispatcher;
			hkConvexListFilter*		m_convexListFilter;

			/// For delayed operations. Operations get delayed if they cannot be performed as the world is hkWorld::locked()
		hkWorldOperationQueue* m_pendingOperations;
			int m_pendingOperationsCount;
			int m_criticalOperationsLockCount;
				// It just holds a value offset which should be added to m_criticalOperationsLockCount when querying areCriticalOperationsLockedForPhantoms().
			int m_criticalOperationsLockCountForPhantoms;
			hkBool m_blockExecutingPendingOperations;
			hkBool m_criticalOperationsAllowed;
				// This points to structures used to debug operation postponing. Not used in release build.
			hkDebugInfoOnPendingOperationQueues* m_pendingOperationQueues;
				// Holds the number of pending queues (equal to the number of recursive calls to executeAllPendingOperations).
			int m_pendingOperationQueueCount;
				
			/// The thread index of the locking thread
		mutable hkMultiThreadLock m_multiThreadLock;

			/// Internal. Forces actions to be processed in a single thread.
		hkBool m_processActionsInSingleThread;

			// This allows to keep the islands size reasonable large. See hkWorldCinfo::m_minDesiredIslandSize for details
		hkUint32 m_minDesiredIslandSize;

			// set if running a mt simulation
			// whenever a constraint is added, this critical section is used
		class hkCriticalSection* m_modifyConstraintCriticalSection;

		class hkCriticalSection* m_worldLock;

			// a critical section which may guard the island dirty list
		class hkCriticalSection* m_islandDirtyListCriticalSection;

		struct PropertyLock
		{
			hkUint32 m_key;
			mutable hkMultiThreadLock m_multiThreadLock;
			class hkCriticalSection*  m_lock;
		};

		class hkCriticalSection* m_propertyMasterLock;
		hkArray<PropertyLock>    m_propertyLocks;


		hkBool m_wantSimulationIslands;


			// Deactivation parameters
		hkBool m_wantDeactivation;

			// set this to true if you want the old style deactivation. 
			// This feature will be removed for all releases after 4.1
		hkBool m_wantOldStyleDeactivation;

		hkBool m_shouldActivateOnRigidBodyTransformChange;


			// deprecated
		hkReal m_highFrequencyDeactivationPeriod;
			// deprecated
		hkReal m_lowFrequencyDeactivationPeriod;

			// see hkWorldCinfo::m_deactivationReferenceDistance
		hkReal m_deactivationReferenceDistance;

		hkReal m_toiCollisionResponseRotateNormal;
		hkWorldCinfo::SimulationType  m_simulationType;

			// debug only and used by the havok demo framework
		static hkBool m_forceMultithreadedSimulation;

	protected:
			// World-unique ids are assigned to entities. They're used to sort constraints deterministically when usin hkMultithreadedSimulation.
		hkUint32 m_lastEntityUid;

	
	public:
		friend class hkWorldOperationUtil;
		friend class hkWorldCallbackUtil;
		friend class hkSimulation;
		friend class hkHalfstepSimulation;
		friend class hkContinuousSimulation;
		friend class hkMultiThreadedSimulation;
		friend class hkSimpleConstraintContactMgr;

		hkArray<hkPhantom*> m_phantoms;

		hkArray<hkActionListener*>				m_actionListeners;
		hkArray<hkEntityListener*>				m_entityListeners;
		hkArray<hkPhantomListener*>				m_phantomListeners;
		hkArray<hkConstraintListener*>			m_constraintListeners;
		hkArray<hkWorldDeletionListener*>		m_worldDeletionListeners;
		hkArray<hkIslandActivationListener*>	m_islandActivationListeners;
		hkArray<hkWorldPostSimulationListener*>	m_worldPostSimulationListeners;
		hkArray<hkWorldPostIntegrateListener*>	m_worldPostIntegrateListeners;
		hkArray<hkWorldPostCollideListener*>	m_worldPostCollideListeners;
		hkArray<hkIslandPostIntegrateListener*>	m_islandPostIntegrateListeners;
		hkArray<hkIslandPostCollideListener*>	m_islandPostCollideListeners;
		hkArray<hkCollisionListener*>			m_collisionListeners;
		
		class hkBroadPhaseBorder*				m_broadPhaseBorder;
	

	public:
			//
			// These members are for internal use only
			//
		hkWorldDynamicsStepInfo m_dynamicsStepInfo;


		// Min and Max extents of broadphase
		// Needed for when construction info is retrieved
		hkVector4 m_broadPhaseExtents[2];
		int			m_broadPhaseNumMarkers;
		hkInt32		m_broadPhaseQuerySize;
		hkInt32		m_broadPhaseUpdateSize;
		hkEnum<enum hkWorldCinfo::ContactPointGeneration, hkInt8> m_contactPointGeneration;

		//
		// Deactivation 
		//

			// Deprecated: Gets the high frequency deactivation period.
		inline hkReal getHighFrequencyDeactivationPeriod() const;

			// Deprecated: Sets the high frequency deactivation period.
		void setHighFrequencyDeactivationPeriod( hkReal period );

			// Deprecated: Gets the low frequency deactivation period.
		inline hkReal getLowFrequencyDeactivationPeriod() const;

			// Deprecated: Sets the low frequency deactivation period.
		void setLowFrequencyDeactivationPeriod( hkReal period );

		//
		// Extra action management
		//

			// Attaches an action to an entity -- this should be called for every entity being 
			// attached to the action, when the action is already added to the world
			// Note: this does not manage the action's entity list.
		void attachActionToEntity(hkAction* action, hkEntity* entity);

			// Detaches an action from an entity -- this should be called for every entity being 
			// detached from the action, when the action is already added to the world.
			// Note: this does not manage the action's entity list.
		void detachActionFromEntity(hkAction* action, hkEntity* entity);


		//
		// Locking and delaying of operations
		//
	public:
		
			// Locks the world, i.e. prevents all critical operations (like adding/removing entities or constraints) from being
			// executed immediately. The operations are put on an hkWorldOperationsQueue and should be executed at the earliest
			// safe time. See hkWorldOperationQueue for the list of 'critical' operations which may be postponed by the system.
			// Subsequent lockCriticalOperations() calls accumulate.
		HK_FORCE_INLINE void	lockCriticalOperations();

			// Allows critical operations to be executed immediately after unlockCriticalOperations() has been called or every corresponding lockCriticalOperations().
			// It is advised to call attemptToExecutePendingOperations right after unlockCriticalOperations() to keep the order of operations 
			// the same as the order in which they were called.
		HK_FORCE_INLINE void	unlockCriticalOperations();

			// Returns number of locks put on the hkWorld. Critical operation will be postponed if areCriticalOperationsLocked() > 0
			// We assume that this function is called at a beginning of a critical operation.
			// So this function asserts if critical operations are not allowed at all
		HK_FORCE_INLINE int		areCriticalOperationsLocked() const;

			// like areCriticalOperationsLocked without the extra check
		HK_FORCE_INLINE int		areCriticalOperationsLockedUnchecked() const;

			// Concept: Locking of Phantoms is an add-on work-around.
			//          It lets you execute phantom-related critical functions even when hkWorld::areCriticalOperationsLocked() is called.
			//          Note that recursive phantom operations will not be executed, and will be postponed just like other operations are.

			// Decreases lock count for phantom operations. The operations must query hkWorld::areCriticalOperationsLockedForPhantoms()
			// instead of hkWorld::areCriticalOperationsLocked().
		HK_FORCE_INLINE void	unlockCriticalOperationsForPhantoms();

			// Increases lock count for phantom operations..
		HK_FORCE_INLINE void	lockCriticalOperationsForPhantoms();

			// Queries lock count for phantom operations. hkWorld::m_criticalOperationsLockCountForPhantoms is an offset value added to the primary hkWorld::m_criticalOperationsLockCount.
		HK_FORCE_INLINE int		areCriticalOperationsLockedForPhantoms() const;



			// Concept: explained in hkWorld.cpp. Search for: "concept: blocking of execution of pending operations"
			// Blocks or unblocks attempts of execution of pending operations.
		HK_FORCE_INLINE void	blockExecutingPendingOperations(hkBool block);

			// See: hkWorld::unlock + hkWorld::attemptToExecutePendingOperations
		HK_FORCE_INLINE void	unlockAndAttemptToExecutePendingOperations();

			// If the world is locked or execution of pending operations is blocked, then it does nothing. 
			// Otherwise it processes the list of pending operations.
		HK_FORCE_INLINE void	attemptToExecutePendingOperations();

			// Appends information about an operation request onto the world's hkWorldOperationQueue.
						void	queueOperation(const hkWorldOperation::BaseOperation& operation);

			// Queues an user callback onto operation queue.
			// If the world is not locked, the callback is executed immediately.
						hkWorldOperation::UserCallback* queueCallback(hkWorldOperation::UserCallback* callback, void* userData = HK_NULL);
	private:
			// Passes execution call to hkWorldOperationQueue::attemptToExecuteAllPending()
		                void	internal_executePendingOperations();

	#if defined HK_DEBUG
			// Returns true if both dynamic bodies attached to the constraint can collide with one another, based on
			// the logic in the hkWorld's hkCollisionFilter.
		hkBool constrainedDynamicBodiesCanCollide( const hkConstraintInstance* constraint ) const;

			// Issues warnings that vary depending on whether the two dynamic, constrained bodies will definitely
			// collide or whether the user simply needs to manually call hkWorld::updateCollisionFilter...()
		void warnIfConstrainedDynamicBodiesCanCollide( const hkConstraintInstance* constraint,
															hkBool collidingBeforeConstraintAdded,
															hkBool collidingAfterConstraintAdded ) const;
	#endif	// #if defined HK_DEBUG


	public:
			// Debugging utility: see hkWorld.cpp. Search for: "concept: allowing critical operations"
		HK_FORCE_INLINE void	allowCriticalOperations(hkBool allow);
};




#include <hkdynamics/world/hkWorld.inl>

#endif // HK_DYNAMICS2_WORLD_H


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
