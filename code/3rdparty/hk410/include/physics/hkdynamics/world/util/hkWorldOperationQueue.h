/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_WORLD_OPERATION_QUEUE_H
#define HK_DYNAMICS2_WORLD_OPERATION_QUEUE_H

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/motion/hkMotion.h>

#include <hkbase/thread/hkCriticalSection.h>

class hkAabb;
class hkRigidBody;
class hkWorldObject;
class hkConstraintInstance;
class hkShape;

namespace hkWorldOperation
{
	enum Type
	{
		OPERATION_ID_ANY     = 0,
		OPERATION_ID_INVALID = 0,

		ENTITY_ADD,
		ENTITY_REMOVE,

		ENTITY_UPDATE_BROAD_PHASE,

		RIGIDBODY_SET_MOTION_TYPE,

		WORLD_OBJECT_SET_SHAPE,

		ENTITY_BATCH_ADD,
		ENTITY_BATCH_REMOVE,

		CONSTRAINT_ADD,
		CONSTRAINT_REMOVE,

		ACTION_ADD,
		ACTION_REMOVE,

		ISLAND_MERGE,       

		PHANTOM_ADD,
		PHANTOM_REMOVE,
		PHANTOM_BATCH_ADD,
		PHANTOM_BATCH_REMOVE,

		PHANTOM_UPDATE_BROAD_PHASE,

		UPDATE_FILTER_ENTITY,
		UPDATE_FILTER_PHANTOM,
		UPDATE_FILTER_WORLD,

		UPDATE_MOVED_BODY_INFO,
	
		ENTITY_BATCH_REINTEGRATE_AND_RECOLLIDE,

		RIGIDBODY_SET_POSITION_AND_ROTATION,
		RIGIDBODY_SET_LINEAR_VELOCITY,
		RIGIDBODY_SET_ANGULAR_VELOCITY,
		RIGIDBODY_APPLY_LINEAR_IMPULSE,
		RIGIDBODY_APPLY_POINT_IMPULSE,
		RIGIDBODY_APPLY_ANGULAR_IMPULSE,

		WORLD_OBJECT_ADD_REFERENCE,
		WORLD_OBJECT_REMOVE_REFERENCE,

		ACTIVATE_REGION,

		ACTIVATE_ENTITY,
		DEACTIVATE_ENTITY,

		USER_CALLBACK

	};

	class UserCallback : public hkReferencedObject
	{
		public:
			virtual void worldOperationUserCallback(void* userData) = 0;
	};


		// !! This is defined in hkWorldObject.h
//	enum Result
//	{
//		DONE,
//		POSTPONED
//	};

	struct BaseOperation
	{
		BaseOperation(Type type) : m_type(type) {}
		hkEnum<Type, hkUint8> m_type;

	private:
		BaseOperation();
	};

	struct AddEntity : public BaseOperation
	{	
		AddEntity() : BaseOperation(ENTITY_ADD) {}
		hkEntity* m_entity;
		hkEntityActivation m_activation;
	};

	struct RemoveEntity : public BaseOperation
	{	
		RemoveEntity() : BaseOperation(ENTITY_REMOVE) {}
		hkEntity* m_entity;
	};

	struct SetRigidBodyMotionType : public BaseOperation
	{	
		SetRigidBodyMotionType() : BaseOperation(RIGIDBODY_SET_MOTION_TYPE) {}
		hkRigidBody* m_rigidBody;
		hkEnum<hkMotion::MotionType, hkUint8>       m_motionType;
		hkEnum<hkEntityActivation, hkUint8>         m_activation;
		hkEnum<hkUpdateCollisionFilterOnEntityMode, hkUint8> m_collisionFilterUpdateMode;
		hkEnum<hkUpdateCollectionFilterMode, hkUint8>	m_updateShapeCollections;
	};

	struct SetWorldObjectShape : public BaseOperation
	{
		SetWorldObjectShape() : BaseOperation(WORLD_OBJECT_SET_SHAPE) {}
		hkWorldObject* m_worldObject;
		hkShape* m_shape;
	};


	struct AddEntityBatch : public BaseOperation
	{	
		AddEntityBatch() : BaseOperation(ENTITY_BATCH_ADD) {}
		hkEntity** m_entities;
		hkObjectIndex m_numEntities;
		hkEnum<hkEntityActivation, hkUint8> m_activation;
	};

	struct RemoveEntityBatch : public BaseOperation
	{	
		RemoveEntityBatch() : BaseOperation(ENTITY_BATCH_REMOVE) {}
		hkEntity** m_entities;
		hkObjectIndex m_numEntities;
	};

	struct AddConstraint : public BaseOperation
	{	
		AddConstraint() : BaseOperation(CONSTRAINT_ADD) {}
		hkConstraintInstance* m_constraint;
	};

	struct RemoveConstraint : public BaseOperation
	{	
		RemoveConstraint() : BaseOperation(CONSTRAINT_REMOVE) {}
		hkConstraintInstance* m_constraint;
	};

	struct AddAction : public BaseOperation
	{	
		AddAction() : BaseOperation(ACTION_ADD) {}
		hkAction* m_action;
	};

	struct RemoveAction : public BaseOperation
	{	
		RemoveAction() : BaseOperation(ACTION_REMOVE) {}
		hkAction* m_action;
	};


	struct MergeIslands : public BaseOperation
	{	
		MergeIslands() : BaseOperation(ISLAND_MERGE) {}
		hkEntity* m_entities[2];
	};


	struct AddPhantom : public BaseOperation
	{
		AddPhantom() : BaseOperation(PHANTOM_ADD) {}
		hkPhantom* m_phantom;
	};

	struct RemovePhantom : public BaseOperation
	{
		RemovePhantom() : BaseOperation(PHANTOM_REMOVE) {}
		hkPhantom* m_phantom;
	};

	struct AddPhantomBatch : public BaseOperation
	{
		AddPhantomBatch() : BaseOperation(PHANTOM_BATCH_ADD) {}
		hkPhantom** m_phantoms;
		hkObjectIndex m_numPhantoms;
	};

	struct RemovePhantomBatch : public BaseOperation
	{
		RemovePhantomBatch() : BaseOperation(PHANTOM_BATCH_REMOVE) {}
		hkPhantom** m_phantoms;
		hkObjectIndex m_numPhantoms;
	};


	struct UpdateEntityBP : public BaseOperation
	{
		UpdateEntityBP() : BaseOperation(ENTITY_UPDATE_BROAD_PHASE) {}
		hkEntity* m_entity;
	};

	struct UpdatePhantomBP : public BaseOperation
	{
		UpdatePhantomBP() : BaseOperation(PHANTOM_UPDATE_BROAD_PHASE) {}
		hkPhantom* m_phantom;
		hkAabb* m_aabb;
	};


	struct UpdateFilterOnEntity : public BaseOperation
	{	
		UpdateFilterOnEntity() : BaseOperation(UPDATE_FILTER_ENTITY) {}
		hkEntity* m_entity;
		hkEnum<hkUpdateCollisionFilterOnEntityMode, hkUint8> m_collisionFilterUpdateMode;
		hkEnum<hkUpdateCollectionFilterMode, hkUint8> 	m_updateShapeCollections;
	};
	struct UpdateFilterOnPhantom : public BaseOperation
	{	
		UpdateFilterOnPhantom() : BaseOperation(UPDATE_FILTER_PHANTOM) {}
		hkPhantom* m_phantom;
		hkEnum<hkUpdateCollectionFilterMode, hkUint8> 	m_updateShapeCollections;
	};
	struct UpdateFilterOnWorld : public BaseOperation
	{	
		UpdateFilterOnWorld() : BaseOperation(UPDATE_FILTER_WORLD) {}
		hkEnum<hkUpdateCollisionFilterOnWorldMode, hkUint8>      m_collisionFilterUpdateMode;
		hkEnum<hkUpdateCollectionFilterMode, hkUint8> 	m_updateShapeCollections;
	};

	struct ReintegrateAndRecollideEntityBatch : public BaseOperation
	{	
		ReintegrateAndRecollideEntityBatch() : BaseOperation(ENTITY_BATCH_REINTEGRATE_AND_RECOLLIDE) {}
		hkEntity** m_entities;
		hkObjectIndex m_numEntities;
		hkEnum<hkEntityActivation, hkUint8> m_activation;
	};

	struct UpdateMovedBodyInfo : public BaseOperation
	{
		UpdateMovedBodyInfo() : BaseOperation(UPDATE_MOVED_BODY_INFO) {}
		hkEntity* m_entity;
	};


	struct SetRigidBodyPositionAndRotation : public BaseOperation
	{
		SetRigidBodyPositionAndRotation() : BaseOperation(RIGIDBODY_SET_POSITION_AND_ROTATION) {}
		hkRigidBody* m_rigidBody;
		hkVector4* m_positionAndRotation; // performs and external allocation
	};
	struct SetRigidBodyLinearVelocity : public BaseOperation
	{
		SetRigidBodyLinearVelocity() : BaseOperation(RIGIDBODY_SET_LINEAR_VELOCITY) {}
		hkRigidBody* m_rigidBody;
		hkReal m_linearVelocity[3];
	};
	struct SetRigidBodyAngularVelocity : public BaseOperation
	{
		SetRigidBodyAngularVelocity() : BaseOperation(RIGIDBODY_SET_ANGULAR_VELOCITY) {}
		hkRigidBody* m_rigidBody;
		hkReal m_angularVelocity[3];
	};
	struct ApplyRigidBodyLinearImpulse : public BaseOperation
	{
		ApplyRigidBodyLinearImpulse() : BaseOperation(RIGIDBODY_APPLY_LINEAR_IMPULSE) {}
		hkRigidBody* m_rigidBody;
		hkReal m_linearImpulse[3];
	};
	struct ApplyRigidBodyPointImpulse : public BaseOperation
	{
		ApplyRigidBodyPointImpulse() : BaseOperation(RIGIDBODY_APPLY_POINT_IMPULSE) {}
		hkRigidBody* m_rigidBody;
		hkVector4* m_pointAndImpulse; // performs and external allocation
	};
	struct ApplyRigidBodyAngularImpulse : public BaseOperation
	{
		ApplyRigidBodyAngularImpulse() : BaseOperation(RIGIDBODY_APPLY_ANGULAR_IMPULSE) {}
		hkRigidBody* m_rigidBody;
		hkReal m_angularImpulse[3];
	};

		// This is a 'special' operation which actually is not postponed, but merely uses operationQueue's critical section to guarantee inter-thread safety.
	struct AddReference : public BaseOperation
	{
		AddReference() : BaseOperation(WORLD_OBJECT_ADD_REFERENCE) {}
		hkWorldObject* m_worldObject;
	};

		// This is a 'special' operation which actually is not postponed, but merely uses operationQueue's critical section to guarantee inter-thread safety.
	struct RemoveReference : public BaseOperation
	{
		RemoveReference() : BaseOperation(WORLD_OBJECT_REMOVE_REFERENCE) {}
		hkWorldObject* m_worldObject;
	};

	struct ActivateRegion : public BaseOperation
	{
		ActivateRegion() : BaseOperation(ACTIVATE_REGION) {}
		hkAabb* m_aabb;
	};

	struct ActivateEntity : public BaseOperation 
	{
		ActivateEntity() : BaseOperation(ACTIVATE_ENTITY) {}
		hkEntity* m_entity;
	};

	struct DeactivateEntity : public BaseOperation 
	{
		DeactivateEntity() : BaseOperation(DEACTIVATE_ENTITY) {}
		hkEntity* m_entity;
	};

	struct UserCallbackOperation : public BaseOperation
	{
		UserCallbackOperation() : BaseOperation(USER_CALLBACK) {}
		UserCallback* m_userCallback;
		void* m_userData;
	};

	struct BiggestOperation : public BaseOperation
	{
		BiggestOperation() : BaseOperation(OPERATION_ID_INVALID) {}
		hkUlong dummy[3 + 1];

	};


}



class hkWorldOperationQueue
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_WORLD, hkWorldOperationQueue ); 

		hkWorldOperationQueue(hkWorld* world);

		~hkWorldOperationQueue();

		void queueOperation(const hkWorldOperation::BaseOperation& operation);
						void executeAllPending();

		HK_FORCE_INLINE void purgeAllPending() { m_pending.clear(); }

	public:
		hkArray<hkWorldOperation::BiggestOperation> m_pending;
		hkWorld* m_world;

		hkArray<hkWorldOperation::BiggestOperation> m_islandMerges;
			// Internal. Only to be enabled by hkMultithrededSimulaiton
		hkBool m_storeIslandMergesOnSeparateList;

	private:

		hkCriticalSection m_queueOperationCriticalSection;

		hkWorldOperationQueue(): m_queueOperationCriticalSection(4000) {}
};



struct hkDebugInfoOnPendingOperationQueues
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_WORLD, hkDebugInfoOnPendingOperationQueues ); 

	hkDebugInfoOnPendingOperationQueues(hkArray<hkWorldOperation::BiggestOperation>* pending) : 
		m_pending(pending), 
		m_nextPendingOperationIndex(0),
		m_nextQueue(HK_NULL),
		m_prevQueue(HK_NULL)
	{
	}

	hkArray<hkWorldOperation::BiggestOperation>* m_pending;
	int m_nextPendingOperationIndex;
	hkDebugInfoOnPendingOperationQueues* m_nextQueue;
	hkDebugInfoOnPendingOperationQueues* m_prevQueue;

	// This struct is only used by the debug code.
	HK_ON_DEBUG 
	(
		static void HK_CALL init(hkWorld* world);
		static void HK_CALL cleanup(hkWorld* world);

		static void HK_CALL updateNextPendingOperationIndex(hkWorld* world, int index);
		static void HK_CALL addNextPendingOperationQueue(hkWorld* world, hkArray<hkWorldOperation::BiggestOperation>* tmpOldPending);
		static void HK_CALL removeLastPendingOperationQueue(hkWorld* world, hkArray<hkWorldOperation::BiggestOperation>* tmpOldPending);
		static int  HK_CALL getNumPendingOperationQueues(hkWorld* world);

		static hkDebugInfoOnPendingOperationQueues*  HK_CALL getLastQueue(hkDebugInfoOnPendingOperationQueues* queue)
		{
			while(queue->m_nextQueue)
			{
				queue = queue->m_nextQueue;
			}
			return queue;
		}

		// Searching through pending lists:
		//   gotoTheLastOne
		//   start from the nextPendingOperationIndex and process till end
		//     goto previousQueue and repeat
		//   while previousQueue != HK_NULL

		static const hkWorldOperation::BaseOperation* findFirstPendingIslandMerge(hkWorld* world, hkSimulationIsland* isleA, hkSimulationIsland* isleB);

		static const hkBool areEmpty(hkWorld* world);
	)
};

#include <hkdynamics/world/util/hkWorldOperationQueue.inl>

#endif // HK_DYNAMICS2_WORLD_OPERATION_QUEUE_H


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
