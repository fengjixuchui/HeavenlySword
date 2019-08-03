/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_HK_POSE_H
#define HK_HK_POSE_H

#include <hkanimation/rig/hkSkeleton.h>
#include <hkbase/memory/hkLocalArray.h>

	/// This class is used to store and manipulate poses.
	/// It uses a dual representation of the poses : LOCAL SPACE, where the transform of each bone is relative to the transform
	/// of its parent, and MODEL SPACE, where the transform of each bone is absolute (respect to a global model space)
	/// Both representations are kept in sync lazily.
	/// hkPose objects are associated with skeletons on construction and can therefore only hold poses based on that skeleton
class hkPose
{
public:
	

	/*
	** Construction
	*/

		/// This enum is used for the construction of an hkPose instance described either in model or in local space
	enum PoseSpace
	{
		MODEL_SPACE,
		LOCAL_SPACE
	};
	
		/// Constructor (Note : it doesn't initialize the pose).
	HK_FORCE_INLINE hkPose (const hkSkeleton* skeleton);

		/// Constructor; uses the specified preallocated memory for internal arrays (Note : it doesn't initialize the pose).
		/// Use the static method getRequiredMemorySize() to determine the required size of this buffer.
	HK_FORCE_INLINE hkPose (const hkSkeleton* skeleton, void* preallocatedMemory);

		/// Returns the memory size needed to hold an hkPose object based on the given skeleton. 
		/// You can preallocate a buffer (possibly local) of this size and then use the hkPose(skeleton, preallocatedMemory) constructor
		/// to create an hkPose instance that doesn't dynamically allocate memory.
	HK_FORCE_INLINE static int getRequiredMemorySize (const hkSkeleton* skeleton);

		/// Constructor; initializes the pose from an array of transforms in Local Space or Model Space (depending on the value of "space")
	hkPose (PoseSpace space, const hkSkeleton* skeleton, const hkArray<hkQsTransform>& pose);

	/*
	** Get/set/access methods
	*/

		/// This enum is passes to methods that change a bone in model space, specifying whether the change propagates or not,
		/// i.e., whether the children keep their local (propagate) or model (don't propagate) transform.
	enum PropagateOrNot
	{
		DONT_PROPAGATE = 0,
		PROPAGATE = 1
	};

		/// Returns the skeleton associated with this pose
	HK_FORCE_INLINE const hkSkeleton* getSkeleton() const;

		/// Returns a const-reference to an array representing the pose in Local Space
	const hkArray<hkQsTransform>& getPoseLocalSpace() const;

		/// Returns a const-reference to an array representing the pose in Model Space
	const hkArray<hkQsTransform>& getPoseModelSpace() const;

		/// Overrides this pose with a new one described as an array of transforms in Local Space
	void setPoseLocalSpace (const hkArray<hkQsTransform>& poseLocal);
	
		/// Overrides this pose with a new one described as an array of transforms in Model Space
	void setPoseModelSpace (const hkArray<hkQsTransform>& poseModel);
	
		/// Retrieves the transform associated with one bone in Local Space
	HK_FORCE_INLINE const hkQsTransform& getBoneLocalSpace (int boneIdx) const;

		/// Retrieves the transform associated with one bone in Model Space
	HK_FORCE_INLINE const hkQsTransform& getBoneModelSpace (int boneIdx) const;

		/// Sets the transform of a particular bone in Local Space. The transform in Local Space of the descendants is kept (therefore their 
		/// transform in Model Space is not).
	HK_FORCE_INLINE void setBoneLocalSpace (int boneIdx, const hkQsTransform& boneLocal);

		/// Sets the transform of a particular bone in Model Space. The transform in Model Space of the descendants is kept,
		/// unless propagateOrNot is set to PROPAGATE. In that case, the Local Space transform of the descendants is kept (and the Model Space transform is not)
	HK_FORCE_INLINE void setBoneModelSpace (int boneIdx, const hkQsTransform& boneModel, PropagateOrNot propagateOrNot);

		/// Ensures all Local Space transforms stored are in sync (since they are evaluated lazily, they may not always be in sync). It is useful if the
		/// applications is going to repeatly access most of these Local Space transforms (without modifying them in Model Space)
	void syncLocalSpace () const;

		/// Ensures all Local Space transforms stored are in sync (since they are evaluated lazily, they may not always be in sync). It is useful if the
		/// applications is going to repeatly access most of these Model Space transforms (without modifying them in Local Space)
	void syncModelSpace () const;

		/// Synchronizes both representations (calls both syncLocalSpace and syncModelSpace).
	inline void syncAll () const;


	/*
	** Access methods, non-const : they sync one representation and dirt the other one. Use with caution.
	*/

		/// Returns a non-const reference to the bone in local space. This reference should only be used for a limited scope (it shouldn't be used
		/// once the hkPose is further modified). Useful in order to modify individual components of the transform without creating a temporary object.
		/// Use this method with caution..
	hkQsTransform& accessBoneLocalSpace (int boneIdx);

		/// Returns a non-const reference to the bone in model space. This reference should only be used for a limited scope (it shouldn't be used
		/// once the hkPose is further modified). Useful in order to modify individual components of the transform without creating a temporary object.
		/// Usage of propagateOrNot is as in setBoneModelSpace(). Use this method with caution.
	hkQsTransform& accessBoneModelSpace (int boneIdx, PropagateOrNot propagateOrNot = DONT_PROPAGATE);

		/// Returns a non-const reference to the pose as an array of transforms in Local Space. It fully syncs the Local Space representation of the pose,
		/// and fully invalidates the Model Space representation. This is useful in order to pass the pose to methods that only modify this Local Space representation.
		/// No other methods in hkPose should be called until this reference goes out of scope or is no longer used.
	hkArray<hkQsTransform>& accessPoseLocalSpace();

		/// Returns a non-const reference to the pose as an array of transforms in Model Space. It fully syncs the Model Space representation of the pose,
		/// and fully invalidates the Local Space representation. This is useful in order to pass the pose to methods that only modify this Model Space representation.
		/// No other methods in hkPose should be called until this reference goes out of scope or is no longer used.
	hkArray<hkQsTransform>& accessPoseModelSpace();

	/*
	** Write access methods, non-const: they don't sync the representatio; the user (caller) is expected to ignore current values and set new values.
	*/

		/// Returns a non-const reference to the pose as an array of transforms in local space. The caller is expected to set these transforms. No other methods
		/// in hkPose should be called until this reference goes out of scope or is no longer used. Use this method with caution.
	hkArray<hkQsTransform>& writeAccessPoseLocalSpace();

		/// Returns a non-const reference to the pose as an array of transforms in model space. The caller is expected to set these transforms. No other methods
		/// in hkPose should be called until this reference goes out of scope or is no longer used. Use this method with caution.
	hkArray<hkQsTransform>& writeAccessPoseModelSpace();


	/*
	** Utility methods
	*/

		/// Set the pose to be the reference pose
	void setToReferencePose ();

		/// Ensure that any bone constraint in the skeleton is enforced. Currently the only bone constraint is "lockTranslation".
		/// This method operates in local space.
	void enforceSkeletonConstraintsLocalSpace ();

		/// Ensure that any bone constraint in the skeleton is enforced. Currently the only bone constraint is "lockTranslation".
		/// This method operates in model space.
	void enforceSkeletonConstraintsModelSpace ();

	/*
	** Operators
	*/

		/// Assignment operator
	inline hkPose& operator= ( const hkPose& other );


private:

	const hkSkeleton* m_skeleton;

	mutable hkArray<hkQsTransform> m_localPose;
	mutable hkArray<hkQsTransform> m_modelPose;
	mutable hkArray<hkInt8> m_boneFlags;

	mutable hkBool m_modelInSync;
	mutable hkBool m_localInSync;

	// Bone Flags
	enum 
	{
		F_BONE_LOCAL_DIRTY = 0x1,
		F_BONE_MODEL_DIRTY = 0x2,
		F_BONE_INTERNAL_FLAG1 = 0x4, // internally used (enforceConstraints, makeAllChildrenLocal)
		F_BONE_INTERNAL_FLAG2 = 0x8, // internally used (calculateBoneModelSpace)
		F_BONE_INTERNAL_FLAG3 = 0x10, // internally used
	};

	// Masks
	enum
	{
		M_BONE_INTERNAL_FLAGS = F_BONE_INTERNAL_FLAG1 | F_BONE_INTERNAL_FLAG2 | F_BONE_INTERNAL_FLAG3
	};

	HK_FORCE_INLINE int isFlagOn (int boneIdx, hkInt8 flag) const;
	HK_FORCE_INLINE void setFlag (int boneIdx, hkInt8 flag) const;
	HK_FORCE_INLINE void clearFlag (int boneIdx, hkInt8 flag) const;

	HK_FORCE_INLINE void clearInternalFlags() const;

	HK_FORCE_INLINE void makeAllChildrenLocalSpace   (int boneIdx) const;
	HK_FORCE_INLINE void makeFirstChildrenModelSpace (int boneIdx) const;
	
	// These methods are called when the value stored is dirty
	const hkQsTransform& calculateBoneModelSpace (int boneIdx) const;

	// DEBUG ONLY METHODS
	void setNonInitializedFlags();
	hkBool checkInternalFlagIsClear(hkInt8 flag) const;

public:

	// For unit testing
	hkBool checkPoseValidity () const;

};

#include <hkanimation/rig/hkPose.inl>

#endif

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
