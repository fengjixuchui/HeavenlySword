/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/rig/hkPose.h>

#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>

// Check hkPose.inl for definitions of macros CHECK_INVARIANT and CHECK_INTERNAL_FLAG_IS_CLEAR

// Constructor
hkPose::hkPose (PoseSpace space, const hkSkeleton* skeleton, const hkArray<hkQsTransform>& pose)
	: m_skeleton(skeleton)
{
	const int numBones = m_skeleton->m_numBones;

	m_modelPose.setSize(numBones);
	m_localPose.setSize(numBones);
	m_boneFlags.setSize(numBones);

	if (space == LOCAL_SPACE)
	{
		setPoseLocalSpace(pose);
	}
	else
	{
		setPoseModelSpace(pose);
	}

	CHECK_INVARIANT();
}


void hkPose::setNonInitializedFlags()
{
	const int numBones = m_skeleton->m_numBones;
	for (int i=0; i<numBones; ++i)
	{
		m_boneFlags[i] = F_BONE_LOCAL_DIRTY | F_BONE_MODEL_DIRTY;
	}

	m_localInSync = false;
	m_modelInSync = false;
}

const hkQsTransform& hkPose::calculateBoneModelSpace (int theBoneIdx) const
{
	CHECK_INTERNAL_FLAG_IS_CLEAR (2);

	HK_ASSERT2(0, isFlagOn (theBoneIdx, F_BONE_MODEL_DIRTY), "Internal error: Method called at unexpected time");

	HK_ASSERT2(0x5a3281f6, ! isFlagOn (theBoneIdx, F_BONE_LOCAL_DIRTY), "Trying to access uninitialized bone in pose" );

	int firstBoneInChain = theBoneIdx;

	// Flag bones in the chain, backwards
	while (true)
	{
		if (!isFlagOn(firstBoneInChain, F_BONE_MODEL_DIRTY))
		{
			break;
		}

		const hkInt16 parentIdx = m_skeleton->m_parentIndices[firstBoneInChain];

		if (parentIdx == -1)
		{
			HK_ASSERT2(0x5a3281f6, ! isFlagOn (firstBoneInChain, F_BONE_LOCAL_DIRTY), "Trying to access uninitialized bone in pose" );

			m_modelPose[firstBoneInChain] = m_localPose[firstBoneInChain];
			clearFlag (firstBoneInChain, F_BONE_MODEL_DIRTY);

			break; // leave the loop
		}

		setFlag(firstBoneInChain, F_BONE_INTERNAL_FLAG2);
		firstBoneInChain = parentIdx;
	}

	// Accumulate the transform through the parents; the first one is in model space already
	HK_ASSERT2(0, !isFlagOn (firstBoneInChain, F_BONE_MODEL_DIRTY), "Internal error: Bad logic somewhere" );

	for (int aBoneIdx = firstBoneInChain+1; aBoneIdx <= theBoneIdx; aBoneIdx++)
	{
		// Use only marked bones (part of the chain)
		if (!isFlagOn(aBoneIdx, F_BONE_INTERNAL_FLAG2))
		{
			continue;
		}
		
		const hkInt16 aParentIdx = m_skeleton->m_parentIndices[aBoneIdx];

		HK_ASSERT2(0x5a3281f6, ! isFlagOn (aBoneIdx, F_BONE_LOCAL_DIRTY), "Trying to access uninitialized bone in pose" );
		HK_ASSERT2(0x5a3281f6, ! isFlagOn (aParentIdx, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose" );

		m_modelPose[aBoneIdx].setMul(m_modelPose[aParentIdx], m_localPose[aBoneIdx]);

		clearFlag (aBoneIdx, F_BONE_INTERNAL_FLAG2);
		clearFlag (aBoneIdx, F_BONE_MODEL_DIRTY);
	}

	CHECK_INVARIANT();

	HK_ASSERT2(0, !isFlagOn (theBoneIdx, F_BONE_MODEL_DIRTY), "Internal error: Bad logic somewhere" );

	return m_modelPose[theBoneIdx];

}


const hkArray<hkQsTransform>& hkPose::getPoseLocalSpace() const
{
	syncLocalSpace();

	CHECK_INVARIANT();

	return m_localPose;
}

void hkPose::syncLocalSpace() const
{
	if (m_localInSync) return;

	const int numBones = m_skeleton->m_numBones;

	for (int b=0; b<numBones; ++b)
	{
		if ( isFlagOn (b, F_BONE_LOCAL_DIRTY) )
		{
			const hkQsTransform& modelFromBone = m_modelPose[b];
			HK_ASSERT2(0x5a3281f6, ! isFlagOn(b, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose" );

			const hkInt16 parentId = m_skeleton->m_parentIndices[b];
			if (parentId != -1)
			{
				const hkQsTransform& modelFromParent = m_modelPose[parentId];
				HK_ASSERT2(0x5a3281f6, ! isFlagOn(parentId, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose" );

				// modelfromBone = modelfromparent * parentfrombone
				// parentfrombone = inv(modelfromparent) * modelfrombone
				m_localPose[b].setMulInverseMul(modelFromParent, modelFromBone);
			}
			else
			{
				m_localPose[b] = modelFromBone;
			}

			clearFlag (b, F_BONE_LOCAL_DIRTY);
		}
	}

	CHECK_INVARIANT();

	m_localInSync = true;
}

const hkArray<hkQsTransform>& hkPose::getPoseModelSpace() const
{
	syncModelSpace();

	CHECK_INVARIANT();

	return m_modelPose;
}

void hkPose::syncModelSpace() const
{
	if (m_modelInSync) return;

	const int numBones = m_skeleton->m_numBones;

	for (int b=0; b<numBones; ++b)
	{
		if (isFlagOn (b, F_BONE_MODEL_DIRTY))
		{
			const hkQsTransform& parentFromBone = m_localPose[b];
			HK_ASSERT2(0x5a3281f6, !isFlagOn(b, F_BONE_LOCAL_DIRTY), "Trying to access uninitialized bone in pose");

			const hkInt16 parentIdx = m_skeleton->m_parentIndices[b];

			if (parentIdx != -1)
			{
				const hkQsTransform& modelFromParent = m_modelPose[parentIdx];
				// note that we can assume safely that the parent in model is clean at this stage
				// but not in the general (random access) case
				HK_ASSERT2(0x5a3281f6, !isFlagOn (parentIdx, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose");

				// modelfrombone = modelfromparent * parentfrombone
				m_modelPose[b].setMul(modelFromParent, parentFromBone);
			}
			else
			{
				m_modelPose[b] = parentFromBone;
			}

			clearFlag (b, F_BONE_MODEL_DIRTY);

		}
	}

	m_modelInSync = true;

	CHECK_INVARIANT()
}

void hkPose::setPoseLocalSpace (const hkArray<hkQsTransform>& poseLocal)
{
	const int numBones = m_skeleton->m_numBones;

	HK_ASSERT(0, poseLocal.getSize() == numBones);

	// Copy local pose
	hkString::memCpy(m_localPose.begin(), poseLocal.begin(), numBones * sizeof (hkQsTransform));

	for (int b=0; b<numBones; ++b)
	{
		m_boneFlags[b] = F_BONE_MODEL_DIRTY;
	}

	m_localInSync = true;
	m_modelInSync = false;

	CHECK_INVARIANT()
}

void hkPose::setPoseModelSpace (const hkArray<hkQsTransform>& poseModel)
{
	const int numBones = m_skeleton->m_numBones;

	HK_ASSERT(0, poseModel.getSize() == numBones);

	// Copy local pose
	hkString::memCpy(m_modelPose.begin(), poseModel.begin(), numBones * sizeof (hkQsTransform));

	for (int b=0; b<numBones; ++b)
	{
		m_boneFlags[b] = F_BONE_LOCAL_DIRTY;
	}

	m_localInSync = false;
	m_modelInSync = true;

	CHECK_INVARIANT()
}


// Access methods, non-const : they fully sync one representation and fully dirt the other one
hkQsTransform& hkPose::accessBoneLocalSpace (int boneIdx)
{
	// The model transform will be invalidated for all descendants
	// Make also sure the local transform for those is in sync
	makeAllChildrenLocalSpace(boneIdx);

	getBoneLocalSpace(boneIdx); // sync

	m_boneFlags[boneIdx] = F_BONE_MODEL_DIRTY;
	m_modelInSync = false;

	CHECK_INVARIANT()

	return m_localPose[boneIdx];
}

hkQsTransform& hkPose::accessBoneModelSpace (int boneIdx, PropagateOrNot propagateOrNot)
{
	if (propagateOrNot == DONT_PROPAGATE)
	{
		// The local transform will be invalidated for the first generation of descendants
		// Make also sure the model transform for those is in sync
		makeFirstChildrenModelSpace(boneIdx);
	}
	else
	{
		// Act as if were actually setting the transform in local space
		makeAllChildrenLocalSpace(boneIdx);
	}

	getBoneModelSpace(boneIdx); // sync

	m_boneFlags[boneIdx] = F_BONE_LOCAL_DIRTY;
	m_localInSync = false;

	CHECK_INVARIANT()

	return m_modelPose[boneIdx];

}


hkArray<hkQsTransform>& hkPose::accessPoseLocalSpace()
{
	syncLocalSpace();

	return writeAccessPoseLocalSpace();

}

hkArray<hkQsTransform>& hkPose::accessPoseModelSpace()
{
	syncModelSpace();

	return writeAccessPoseModelSpace();

}

hkArray<hkQsTransform>& hkPose::writeAccessPoseLocalSpace ()
{
	const int numBones = m_skeleton->m_numBones;
	for (int i=0; i<numBones; ++i)
	{
		m_boneFlags[i] = F_BONE_MODEL_DIRTY;
	}
	m_modelInSync = false;
	m_localInSync = true;

	CHECK_INVARIANT()

	return m_localPose;

}

hkArray<hkQsTransform>& hkPose::writeAccessPoseModelSpace ()
{
	const int numBones = m_skeleton->m_numBones;
	for (int i=0; i<numBones; ++i)
	{
		m_boneFlags[i] = F_BONE_LOCAL_DIRTY;
	}
	m_localInSync = false;
	m_modelInSync = true;

	CHECK_INVARIANT()

	return m_modelPose;

}


// For unit testing
hkBool hkPose::checkPoseValidity () const
{
	const int numBones = m_skeleton->m_numBones;

	// Check that for each bone we have one clean representation at least
	{
		for (int i=0; i<numBones; i++)
		{
			if (isFlagOn (i, F_BONE_LOCAL_DIRTY) && isFlagOn (i, F_BONE_MODEL_DIRTY) )
			{
				// Trying to access uninitialized pose
				HK_ASSERT2(0x5a3281f6, 0, "Invariant failed : both local and model transforms dirty for bone "<<i);				
				return false;
			}
		}
	}

	// Check that the "all in sync" flags are right
	{
		if (m_localInSync)
		{
			for (int i=0; i<numBones; i++)
			{
				if (isFlagOn (i, F_BONE_LOCAL_DIRTY))
				{
					HK_ASSERT2(0x5a3281f4, 0, "Invariant failed : m_localInSync is true but some local transforms are dirty");
					return false;
				}
			}
		}
		if (m_modelInSync)
		{
			for (int i=0; i<numBones; i++)
			{
				if (isFlagOn (i, F_BONE_MODEL_DIRTY))
				{
					HK_ASSERT2(0x5a3281f5,0, "Invariant failed : m_modelInSync is true but some model transforms are dirty");
					return false;
				}
			}
		}
	}

	return true;
}

// UTILITY METHODS

void hkPose::setToReferencePose()
{
	hkString::memCpy( m_localPose.begin(), m_skeleton->m_referencePose, sizeof(hkQsTransform) * m_skeleton->m_numBones );

	for (int b=0; b<m_skeleton->m_numBones; ++b)
	{
		m_boneFlags[b] = F_BONE_MODEL_DIRTY;
	}

	m_modelInSync = false;
	m_localInSync = true;

	CHECK_INVARIANT();

}


void hkPose::enforceSkeletonConstraintsLocalSpace ()
{
	CHECK_INTERNAL_FLAG_IS_CLEAR (1);

	const int numBones = m_skeleton->m_numBones;

	syncLocalSpace();

	for (int boneIdx=0; boneIdx< numBones; ++boneIdx)
	{
		const hkBone* bone = m_skeleton->m_bones[boneIdx];

		if (bone->m_lockTranslation)
		{
			m_localPose[boneIdx].m_translation = m_skeleton->m_referencePose[boneIdx].m_translation;
			setFlag(boneIdx, F_BONE_MODEL_DIRTY);
			setFlag(boneIdx, F_BONE_INTERNAL_FLAG1);
		}
		else
		{
			const hkInt16 parentIdx = m_skeleton->m_parentIndices[boneIdx];
			// Use the internal flag to invalidate all descendants
			if (parentIdx!=-1 && isFlagOn(parentIdx, F_BONE_INTERNAL_FLAG1))
			{
				setFlag(boneIdx, F_BONE_MODEL_DIRTY);
				setFlag(boneIdx, F_BONE_INTERNAL_FLAG1);
			}

		}

	}

	clearInternalFlags();

	CHECK_INVARIANT();
}

void hkPose::enforceSkeletonConstraintsModelSpace ()
{
	CHECK_INTERNAL_FLAG_IS_CLEAR (1);

	const int numBones = m_skeleton->m_numBones;

	syncModelSpace();

	for (int boneIdx=0; boneIdx< numBones; ++boneIdx)
	{
		const hkBone* bone = m_skeleton->m_bones[boneIdx];
		const hkInt16 parentIdx = m_skeleton->m_parentIndices[boneIdx];
		if (bone->m_lockTranslation)
		{
			const hkVector4& localTranslation = m_skeleton->m_referencePose[boneIdx].m_translation;

			if (parentIdx != -1)
			{
				const hkQsTransform& parentModelSpace = m_modelPose[parentIdx];
				// like setTransformedPos(), but we don't want scale
				m_modelPose[boneIdx].m_translation.setRotatedDir(parentModelSpace.getRotation(), localTranslation);
				m_modelPose[boneIdx].m_translation.add4(parentModelSpace.getTranslation());
			}
			else
			{
				m_modelPose[boneIdx].m_translation = localTranslation;
			}

			setFlag(boneIdx, F_BONE_LOCAL_DIRTY);

			// Mark that we modified this guy
			setFlag(boneIdx, F_BONE_INTERNAL_FLAG1);


		}
		else
		{
			// Use the internal flag to invalidate first descendants
			if (parentIdx!=-1 && isFlagOn(parentIdx, F_BONE_INTERNAL_FLAG1))
			{
				setFlag(boneIdx, F_BONE_LOCAL_DIRTY);
			}
		}
	}

	// Clear the modified flag
	clearInternalFlags();

	CHECK_INVARIANT();

}


 
hkBool hkPose::checkInternalFlagIsClear(hkInt8 flag) const
{
#ifdef HK_DEBUG
	const int numBones = m_skeleton->m_numBones;

	// Check that for each bone we have one clean representation at least
	{
		for (int i=0; i<numBones; i++)
		{
			if (isFlagOn (i, flag))
			{
				HK_ASSERT2(0x66565434, 0, "Invariant failed : Flag "<<" should be clear for all bones, and it's not for bone "<<i);
				return false;
			}
		}
	}
#endif
	return true;
}


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
