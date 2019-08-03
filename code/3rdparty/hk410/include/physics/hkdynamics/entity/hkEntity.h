/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_DYNAMICS2_ENTITY_H
#define HK_DYNAMICS2_ENTITY_H

#include <hkmath/hkMath.h>
//#include <hkbase/htl/hkSmallArray.h>
#include <hkdynamics/world/hkWorldObject.h>
#include <hkdynamics/common/hkMaterial.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/motion/rigid/hkKeyframedRigidMotion.h>

class hkEntityListener;
class hkEntityActivationListener;
class hkCollisionListener;
class hkMotion;
class hkSimulationIsland;
class hkWorld;
class hkConstraintInstance;
class hkAction;
class hkEntityDeactivator;
class hkDynamicsContactMgr;

extern const hkClass hkEntityClass;

/// This class represents the core "physical object" elements in the dynamics system, such
/// as rigid bodies. 
class hkEntity: public hkWorldObject
{

	public:

		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ENTITY);

			//
			// Event Handling
			//

			/// Adds an entity listener to the entity.
		void addEntityListener( hkEntityListener* el);

			/// Removes an entity listener from the entity.
		void removeEntityListener( hkEntityListener* el);

			/// Adds an entity activation listener to the entity.
		void addEntityActivationListener( hkEntityActivationListener* el);

			/// Removes an entity activation listener from the entity.
		void removeEntityActivationListener( hkEntityActivationListener* el);

			/// Adds a collision listener to the entity.
		void addCollisionListener( hkCollisionListener* cl );

			/// Removes a collision listener from the entity.
		void removeCollisionListener( hkCollisionListener* cl);

			/// Get const access to the array of entity listeners.
		inline const hkArray<hkEntityListener*>& getEntityListeners() const;

			/// Get const access to the array of entity activation listeners.
		inline const hkArray<hkEntityActivationListener*>& getEntityActivationListeners() const;
		
			/// Get const access to the array of entity listeners.
		inline const hkArray<hkCollisionListener*>& getCollisionListeners() const;

			/// Gets the process contact callback delay.
		inline hkUint16 getProcessContactCallbackDelay() const;

			/// Sets the process contact callback delay.
			/// This value is used to determine how often a callback is raised for the
			/// "process contact" collision event.
			/// A value of 0 means the callback is called every step, whereas a value of 4 means 
			/// that a callback is raised every 5th step.
			/// Process contact callbacks can be used to change properties of contact points, such
			/// as the friction of a sliding contact.
		inline void setProcessContactCallbackDelay( hkUint16 delay );





			//
			// Utility functions
			//


			/// Gets the material used by this entity.
			/// If the entity has no collision detection representation, 
			/// the material is not used.
		inline hkMaterial& getMaterial();

			/// Gets the material used by this entity.
			/// If the entity has no collision detection representation, 
			/// the material is not used.
		inline const hkMaterial& getMaterial() const;

			/// A utility function to determine if the entity is fixed.
		inline hkBool isFixed() const;

			/// Checks whether the body's velocity cannot be influenced by physics directly.
			/// Uses a cached variable to avoid referencing hkMotion object.
		inline hkBool isFixedOrKeyframed() const;

			/// Get's the entity's unique id. The uid is assigned in the entity's constructor and 
			/// is also updated when your deserialize objects.
		inline hkUint32 getUid() const;

			/// Find the contact manager between 'this' and the supplied entity.
			///
			/// Returns HK_NULL if no contact manager exists between 'this' and the supplied entity.
		hkDynamicsContactMgr* findContactMgrTo(const hkEntity* entity);


			//
			// Deactivation
			//

			/// Activates the specified entity and its island.
		void activate();

			/// Deactivates the specified entity and its island.
		void deactivate();

			/// Activates the specified entity and its island. Uses postponed operations queue if the world is locked for critical operations.
		void activateAsCriticalOperation();

			/// Deactivates the specified entity and its island. Uses postponed operations queue if the world is locked for critical operations.
		void deactivateAsCriticalOperation();

			/// Returns whether the entity is active. This method returns false if the entity
			/// has not yet been added to a hkWorld object.
		hkBool isActive() const;


			/// Gets the deactivator for the entity. The setDeactivator calls are in the 
			/// hkEntity derived classes, to ensure type safety.
		inline hkEntityDeactivator* getDeactivator();

			/// Gets the deactivator for the entity. The setDeactivator calls are in the 
			/// hkEntity derived classes, to ensure type safety.
		inline const hkEntityDeactivator* getDeactivator() const;


			//
			// Attached action and constraint accessors
			//


			/// Get the number of actions added to the world which reference this entity
		inline int getNumActions() const;

			/// Get the ith action added to the world which references this entity
		inline hkAction* getAction(int i);

			///	Returns the number of constraints attached to this entity
		int getNumConstraints() const;

			/// Returns the ith constraint attached to this entity
		hkConstraintInstance* getConstraint( int i );

			/// Returns the ith constraint attached to this entity (const version)
		const hkConstraintInstance* getConstraint( int i ) const;

		/// Returns read only access to the internal constraint master list
		inline const hkArray<struct hkConstraintInternal>&  getConstraintMasters() const;

		/// Returns read write access to the internal constraint master list
		inline hkArray<struct hkConstraintInternal>&  getConstraintMastersRw();

		/// Returns read only access to the internal constraint master list
		inline const hkArray<class hkConstraintInstance*>&  getConstraintSlaves() const;

			// Calculate the memory usage of this entity
		void calcStatistics( hkStatisticsCollector* collector) const;

			// Destructor.
		virtual ~hkEntity();

	protected:
		const hkArray<struct hkConstraintInternal>&  getConstraintMastersImpl() const;
		hkArray<struct hkConstraintInternal>&  getConstraintMastersRwImpl();
		const hkArray<class hkConstraintInstance*>&  getConstraintSlavesImpl() const;

		hkEntity( const hkShape* shape );

		void setDeactivator( hkEntityDeactivator* deactivator );


	protected:

			// The entity's simulation island.
		hkSimulationIsland* m_simulationIsland; //+nosave


			// The entity's material, only used if the collision detection is enabled.
		class hkMaterial   m_material;

			// The entity's deactivator.
		hkEntityDeactivator* m_deactivator;

			// ------------------ 2nd CacheLine64 (rarely accessed data ) -------------------------

	protected:
		friend class hkWorldConstraintUtil;

		// the next three elements store constraint information (note: they are owned by the simulation island
		hkArray<struct hkConstraintInternal> m_constraintsMaster; //+nosave
		hkArray<hkConstraintInstance*>		 m_constraintsSlave; //+nosave
		hkArray<hkUint8>					 m_constraintRuntime;

	public:
		hkObjectIndex m_storageIndex; //+overridetype(hkUint16)

	protected:

		hkUint16 m_processContactCallbackDelay;

	public:
			/// See: hkRigidBodyCinfo::m_autoRemoveLevel
		hkInt8  m_autoRemoveLevel;

		// offset into the accumulators
		hkUint32 m_solverData;

		// hkWorld-unique Id
		hkUint32 m_uid; //+default(0xffffffff)

	public:
			// The motion of the object
		class hkMaxSizeMotion m_motion;

	protected:
			//
			//	Rarely used members
			//
		friend class hkEntityCallbackUtil;
		friend class hkWorldCallbackUtil;
		friend class hkWorld;
		friend class hkSimulationIsland;
		friend class hkWorldOperationUtil;
		
		hkArray<hkCollisionListener*>        m_collisionListeners; //+nosave
		hkArray<hkEntityActivationListener*> m_activationListeners; //+nosave

		hkArray<hkEntityListener*> m_entityListeners; //+nosave

		hkArray<hkAction*> m_actions; //+nosave


	
	public:

			//
			// INTERNAL FUNCTIONS
			//

			// Gets the hkLinkedCollidable
		inline hkLinkedCollidable* getLinkedCollidable();

			// Simulation units use this interface.
		inline hkMotion* getMotion();

			// Get the simulation island, is HK_NULL for entities not in simulation.
		inline hkSimulationIsland* getSimulationIsland() const;
			
			// Deallocates internal arrays if size 0.
			// Called internal by hkWorld::removeEntity. Over
		virtual void deallocateInternalArrays();

		virtual hkMotionState* getMotionState(){ return HK_NULL; }

	public:

		hkEntity( class hkFinishLoadedObjectFlag flag );
};

#include <hkdynamics/entity/hkEntity.inl>

#endif // HK_DYNAMICS2_ENTITY_H

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
