/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



int hkRagdollInstance::getNumBones () const
{
	return m_skeleton->m_numBones;
}

hkRigidBody* hkRagdollInstance::getRigidBodyOfBone (int i) const
{
	return m_rigidBodies[i];
}

hkConstraintInstance* hkRagdollInstance::getConstraintOfBone (int i) const
{
	return (i==0) ? HK_NULL : m_constraints[i-1];
}

int hkRagdollInstance::getParentOfBone (int i) const
{
	return (m_skeleton->m_parentIndices[i]);
}

const hkSkeleton* hkRagdollInstance::getSkeleton() const
{
	return m_skeleton;
}

const hkArray<hkRigidBody*>& hkRagdollInstance::getRigidBodyArray() const
{
	return m_rigidBodies;
}

const hkArray<hkConstraintInstance*>& hkRagdollInstance::getConstraintArray() const
{
	return m_constraints;
}


hkWorld* hkRagdollInstance::getWorld() const
{
	hkRigidBody* root = getRigidBodyOfBone(0);

	if (root)
	{
		return root->getWorld();
	}
	else
	{
		return HK_NULL;
	}
}

void hkRagdollInstance::getBoneTransform (int rbId, hkQsTransform& rbTransformOut) const
{
	const hkRigidBody* rbody = getRigidBodyOfBone(rbId);
	rbTransformOut.setFromTransformNoScale(rbody->getTransform());
	// Take the scale from the reference pose
	// It'd be faster if hkSkeleton kept the reference pose in model space
	hkSkeletonUtils::getModelSpaceScale(*m_skeleton, m_skeleton->m_referencePose, rbId, rbTransformOut.m_scale);
}

void hkRagdollInstance::getApproxBoneTransformAt (int rbId, hkReal time, hkQsTransform& rbTransformOut) const
{
	hkTransform tOut;
	getRigidBodyOfBone(rbId)->approxTransformAt( time, tOut );
	rbTransformOut.setFromTransformNoScale( tOut );

	// Take the scale from the reference pose
	// It'd be faster if hkSkeleton kept the reference pose in model space
	hkSkeletonUtils::getModelSpaceScale(*m_skeleton, m_skeleton->m_referencePose, rbId, rbTransformOut.m_scale);
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
