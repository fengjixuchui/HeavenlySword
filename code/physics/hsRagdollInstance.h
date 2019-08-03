#ifndef INC_HSRAGDOLL_INSTANCE
#define INC_HSRAGDOLL_INSTANCE
#include "config.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include "Physics/havokincludes.h"
#include "physics/havokthreadutils.h"

#include <hkmath/hkMath.h>

#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/world/hkPhysicsSystem.h>

#include <hkanimation/rig/hkSkeleton.h>

extern const hkClass hsRagdollInstanceClass;

#include <hkragdoll/instance/hkRagdollInstance.h>
#include <hkdynamics/world/hkWorld.h>

	/// An hkRagdollInstance encapsulates the rigid bodies, constraints and associated skeleton for a physical ragdoll.
	/// It also provides utilities to get / set poses, add and remove from world, cloning, and handling rigid body transforms.
class hsRagdollInstance : public hkRagdollInstance
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
		hsRagdollInstance ( const hkArray<hkRigidBody*>& rigidBodies, const hkArray<hkConstraintInstance*>& constraints, const hkSkeleton* skeleton );

			/// Destructor.
		virtual ~hsRagdollInstance ();

			/// Cloning - it returns a new hsRagdollInstance cloned from this one.
		virtual hsRagdollInstance* clone() const;

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

			/// Gets the current pose of the rag doll
		void getPoseModelSpace (hkQsTransform* poseOut, const hkQsTransform& worldFromModel) const;

			/// Sets the current pose of the rag doll
		void setPoseModelSpace (const hkQsTransform* poseIn, const hkQsTransform& worldFromModel);
		void setPoseModelSpaceKeyframedOnly(const hkQsTransform* poseIn, const hkQsTransform& worldFromModel);

		//
		// World add remove methods
		//

			/// Add to world; returns failure if the ragdoll is already in the world.
			// <animreview: use different filters (e.g. hkGroupFilter::subid and remove updateFilter parameter
		hkResult addToWorld ( hkBool updateFilter, hkEntityActivation initialActivationState = HK_ENTITY_ACTIVATION_DO_ACTIVATE ) const;

			/// Removes from world; returns failure if the ragdoll wasn't part of a world.
		hkResult removeFromWorld () const;

			/// Returns the world where the ragdoll has been added to (HK_NULL if none).
		inline hkWorld* getWorld() const;

		//
		// Utility methods
		//

			/// Returns the current transform (local-to-world) of the i-th bone (rigid body).
		inline void getBoneTransform (int rbId, hkQsTransform& boneTransformOut) const;

	public:

		hsRagdollInstance( hkFinishLoadedObjectFlag f ) : hkRagdollInstance( f )  { } 

};

#include "hsRagdollInstance.inl"

#endif
#endif //INC_HSRAGDOLL_INSTANCE
