/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifdef HK_DEBUG
	#define CHECK_INVARIANT()					checkPoseValidity();
	#define CHECK_INTERNAL_FLAG_IS_CLEAR(f)		checkInternalFlagIsClear(F_BONE_INTERNAL_FLAG##f);
#else
	#define CHECK_INVARIANT()					;
	#define CHECK_INTERNAL_FLAG_IS_CLEAR(f)		;
#endif


const hkSkeleton* hkPose::getSkeleton() const
{
	return m_skeleton;
}

void hkPose::syncAll() const
{
	syncLocalSpace();
	syncModelSpace();
}

int hkPose::isFlagOn (int boneIdx, hkInt8 flag) const
{
	return m_boneFlags[boneIdx] & flag;
}

void hkPose::setFlag (int boneIdx, hkInt8 flag) const
{
	m_boneFlags[boneIdx] |= flag;
}

void hkPose::clearFlag (int boneIdx, hkInt8 flag) const
{
	m_boneFlags[boneIdx] &= static_cast<hkInt8>(~flag);
}

void hkPose::clearInternalFlags() const
{
	const int numBones = m_boneFlags.getSize();
	for (int i=0; i<numBones; ++i)
	{
		m_boneFlags[i] &= (~M_BONE_INTERNAL_FLAGS);
	}
}

hkPose& hkPose::operator= ( const hkPose& other )
{
	m_skeleton = other.m_skeleton;
	m_localPose = other.m_localPose;
	m_modelPose = other.m_modelPose;
	m_boneFlags = other.m_boneFlags;
	m_modelInSync = other.m_modelInSync;
	m_localInSync = other.m_localInSync;

	return *this;
}


const hkQsTransform& hkPose::getBoneModelSpace (int boneIdx) const
{
	if (!isFlagOn (boneIdx, F_BONE_MODEL_DIRTY))
	{
		return m_modelPose[boneIdx];
	}
	else
	{
		return calculateBoneModelSpace(boneIdx);
	}
}


const hkQsTransform& hkPose::getBoneLocalSpace (int boneIdx) const
{
	if (! isFlagOn (boneIdx, F_BONE_LOCAL_DIRTY))
	{
		return m_localPose[boneIdx];
	}
	else
	{
		const hkQsTransform& modelFromBone = m_modelPose[boneIdx];
		HK_ASSERT2(0x5a3281f6, ! isFlagOn (boneIdx, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose" );

		const hkInt16 parentIdx = m_skeleton->m_parentIndices[boneIdx];

		if (parentIdx != -1)
		{
			// ask for parent model (may be dirty)
			const hkQsTransform& modelFromParent = getBoneModelSpace(parentIdx);

			// parentfrombone = inv(modelfromparent) * modelfrombone
			m_localPose[boneIdx].setMulInverseMul(modelFromParent, modelFromBone);

		}
		else
		{
			m_localPose[boneIdx] = modelFromBone;
		}

		clearFlag (boneIdx, F_BONE_LOCAL_DIRTY);

		CHECK_INVARIANT();

		return m_localPose[boneIdx];
	}
}

void hkPose::makeAllChildrenLocalSpace(int boneIdx) const
{
	CHECK_INTERNAL_FLAG_IS_CLEAR(1);

	const int numBones = m_skeleton->m_numBones;

	setFlag(boneIdx, F_BONE_INTERNAL_FLAG1);

	// Synchronize all descendants local representation
	{
		for (int i= boneIdx+1; i<numBones; ++i)
		{
			const hkInt16 parentId = m_skeleton->m_parentIndices[i];
			if (isFlagOn(parentId, F_BONE_INTERNAL_FLAG1))
			{
				getBoneLocalSpace( i ); // sync local
				setFlag( i, F_BONE_INTERNAL_FLAG1);
				m_modelInSync = false;
			}
		} 
	}

	// Dirty all descendants model representation
	{
		for (int i= boneIdx+1; i < numBones; ++i)
		{
			if (isFlagOn( i, F_BONE_INTERNAL_FLAG1))
			{
				setFlag( i, F_BONE_MODEL_DIRTY);
				clearFlag( i, F_BONE_INTERNAL_FLAG1);
			}
		}
	}

}

void hkPose::makeFirstChildrenModelSpace(int boneIdx) const
{
	const int numBones = m_skeleton->m_numBones;

	for (int i= boneIdx+1; i<numBones; ++i)
	{
		const hkInt16 parentId = m_skeleton->m_parentIndices[i];
		if (parentId == boneIdx)
		{
			getBoneModelSpace( i ); // sync model
			m_boneFlags[i] = F_BONE_LOCAL_DIRTY;
			m_localInSync = false;
		}
	}

}

void hkPose::setBoneLocalSpace (int boneIdx, const hkQsTransform& boneLocal)
{
	// The model transform will be invalidated for all descendants
	// Make also sure the local transform for those is in sync
	makeAllChildrenLocalSpace(boneIdx);

	m_localPose[boneIdx] = boneLocal;

	m_boneFlags[boneIdx] = F_BONE_MODEL_DIRTY;

	m_modelInSync = false;

	CHECK_INVARIANT();

}

void hkPose::setBoneModelSpace (int boneIdx, const hkQsTransform& boneModel, PropagateOrNot propagateOrNot)
{
	if (! propagateOrNot )
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

	m_modelPose[boneIdx] = boneModel;

	m_boneFlags[boneIdx] = F_BONE_LOCAL_DIRTY;

	m_localInSync = false;

	CHECK_INVARIANT();

}

hkPose::hkPose (const hkSkeleton* skeleton)
	: m_skeleton(skeleton), m_modelInSync(false), m_localInSync (false)
{
	const int numBones = m_skeleton->m_numBones;

	m_modelPose.setSize(numBones);
	m_localPose.setSize(numBones);
	m_boneFlags.setSize(numBones);

#ifdef HK_DEBUG
	setNonInitializedFlags();
#endif

}

int hkPose::getRequiredMemorySize (const hkSkeleton* skeleton)
{
	const int numBones = skeleton->m_numBones;

	const int totalSize =  numBones * ( 2 * hkSizeOf(hkQsTransform) + hkSizeOf(hkInt8) );

	return totalSize;
}

hkPose::hkPose(const hkSkeleton* skeleton, void* preallocatedMemory)
	:	m_skeleton  (skeleton), 
		m_localPose (reinterpret_cast<hkQsTransform*> (preallocatedMemory), skeleton->m_numBones, skeleton->m_numBones),
		m_modelPose (m_localPose.begin() + skeleton->m_numBones, skeleton->m_numBones, skeleton->m_numBones),
		m_boneFlags (reinterpret_cast<hkInt8*> (m_modelPose.begin() + skeleton->m_numBones), skeleton->m_numBones, skeleton->m_numBones),
		m_modelInSync (false),
		m_localInSync (false)
{
	HK_ASSERT2 (0x4643f989, (reinterpret_cast<hkUlong> (preallocatedMemory) & 0xf) == 0, "Preallocated memory must be 16-byte aligned");

#ifdef HK_DEBUG
	setNonInitializedFlags();
#endif
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
