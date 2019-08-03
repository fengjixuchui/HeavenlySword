/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HKRAGDOLL_INSTANCE
#define INC_HKRAGDOLL_INSTANCE

#include <hkmath/hkMath.h>

#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/world/hkPhysicsSystem.h>

#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>

extern const hkClass hkRagdollInstanceClass;

	/// An hkRagdollInstance encapsulates the rigid bodies, constraints and associated skeleton for a physical ragdoll.
	/// It also provides utilities to get / set poses, add and remove from world, cloning, and handling rigid body transforms.
class hkRagdollInstance : public hkReferencedObject
{

	public:

		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ANIMATION);

			/// Constructor, requires a properly ordered set of rigid bodies and constraints.
			/// The must represent a tree (n rigid bodies, n-1 constraints), and the local
			/// transformation of the rigid bodies must match the pivot of the bone they represent.
			/// You can use hkRagdollUtils::prepareSystemForRagdoll() in order to ensure this.
			/// The skeleton contains information about hierarchy, bone names, initial pose and bone constraints.
			/// You can generate an hkSkeleton from a physics system by using the utility hkRagdollUtils::constructSkeletonFromRagdoll().
		hkRagdollInstance ( const hkArray<hkRigidBody*>& rigidBodies, const hkArray<hkConstraintInstance*>& constraints, const hkSkeleton* skeleton );

			/// Destructor.
		virtual ~hkRagdollInstance ();

			/// Cloning - it returns a new hkRagdollInstance cloned from this one.
		virtual hkRagdollInstance* clone() const;

		//
		// Accessors
		//

			/// Number of bones.
		inline int getNumBones () const;

			/// Access the rigid body associated with the i-th bone.
		inline hkRigidBody* getRigidBodyOfBone (int i) const;

			/// Access the constraint associated with the i-th bone. The constraint associated with a bone is the constraint that links that bone with its parent.
			/// Notice, therefore, that getConstraint(0) returns HK_NULL. 
		inline hkConstraintInstance* getConstraintOfBone (int i) const;

			/// Access the associated skeleton
		inline const hkSkeleton* getSkeleton () const;

			/// Retrieves the ID of the parent of a bone. -1 for the root (bone 0).
		inline int getParentOfBone (int i) const;

			/// Returns a (const) array of rigid bodies representing the bones
		inline const hkArray<hkRigidBody*>& getRigidBodyArray() const;

			/// Returns a (const) array of constraint representing the joints. Notice that, since the root (bone 0) has no parent, 
			/// the size of this array is n-1, where n is the number of bones.
		inline const hkArray<hkConstraintInstance*>& getConstraintArray() const;

		//
		// Get/Set pose methods
		//
			/// Gets the current pose of the rag doll in world space
		void getPoseWorldSpace (hkQsTransform* poseWorldSpaceOut) const;

			/// Sets the current pose of the rag doll using a pose in world space
		void setPoseWorldSpace (const hkQsTransform* poseModelSpaceIn);

			/// Gets the current pose of the rag doll in model space
			/// This is similar to getPoseWorldSpace but it uses the worldFromModel transform to transform the result to model space
		void getPoseModelSpace (hkQsTransform* poseModelSpaceOut, const hkQsTransform& worldFromModel) const;

			/// Sets the current pose of the rag doll using a pose in model space
		void setPoseModelSpace (const hkQsTransform* poseModelSpaceIn, const hkQsTransform& worldFromModel);

		//
		// World add remove methods
		//

			/// Add to world; returns failure if the ragdoll is already in the world.
		hkResult addToWorld (hkWorld* world, hkBool updateFilter) const;

			/// Removes from world; returns failure if the ragdoll wasn't part of a world.
		hkResult removeFromWorld () const;

			/// Returns the world where the ragdoll has been added to (HK_NULL if none).
		inline hkWorld* getWorld() const;

		//
		// Utility methods
		//

			/// Returns the current transform (bone-to-world) of the i-th bone (rigid body).
			/// Position and orientation are taken from the rb, scale is taken from the reference pose of the skeleton
		inline void getBoneTransform (int rbId, hkQsTransform& worldFromBoneOut) const;

			/// Returns the current transform (bone-to-world) of the i-th bone (rigid body).
			/// Position and orientation are taken from the rb, scale is taken from the reference pose of the skeleton
			/// This method uses hkRigidBody::approxTransformAt and is useful if you are stepping animation and physics independently.
		inline void getApproxBoneTransformAt (int rbId, hkReal time, hkQsTransform& worldFromBoneOut) const;

	public:

			/// The rigid bodies associated with this rag doll.
		/*const*/ hkArray<hkRigidBody*> m_rigidBodies;
			/// The constraints associated with this rag doll 
		/*const*/ hkArray<hkConstraintInstance*> m_constraints;

			/// The skeleton associated with this rag doll.
		const hkSkeleton* m_skeleton;

	public:

		hkRagdollInstance( hkFinishLoadedObjectFlag f ) : m_rigidBodies(f), m_constraints(f) { } 

};

#include <hkragdoll/instance/hkRagdollInstance.inl>

#endif //INC_HKRAGDOLL_INSTANCE

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
