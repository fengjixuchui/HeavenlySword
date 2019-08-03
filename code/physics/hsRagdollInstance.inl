int hsRagdollInstance::getNumBones () const
{
	return m_skeleton->m_numBones;
}

hkRigidBody* hsRagdollInstance::getRigidBodyOfBone (int i) const
{
	return m_rigidBodies[i];
}

hkConstraintInstance* hsRagdollInstance::getConstraintOfBone (int i) const
{
	return (i==0) ? HK_NULL : m_constraints[i-1];
}

int hsRagdollInstance::getParentOfBone (int i) const
{
	return (m_skeleton->m_parentIndices[i]);
}

const hkSkeleton* hsRagdollInstance::getSkeleton() const
{
	return m_skeleton;
}

const hkArray<hkRigidBody*>& hsRagdollInstance::getRigidBodyArray() const
{
	return m_rigidBodies;
}

const hkArray<hkConstraintInstance*>& hsRagdollInstance::getConstraintArray() const
{
	return m_constraints;
}


hkWorld* hsRagdollInstance::getWorld() const
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

void hsRagdollInstance::getBoneTransform (int rbId, hkQsTransform& rbTransformOut) const
{
	hkRigidBody *rbody = getRigidBodyOfBone( rbId );

	if ( rbody->getWorld() )
	{
		Physics::ReadAccess mutex;

		hkTransform transformOut;
		rbody->approxTransformAt( rbody->getWorld()->getCurrentTime(), transformOut );
		rbTransformOut.setFromTransformNoScale(transformOut);
	}
	else
	{
		rbTransformOut.setFromTransformNoScale( rbody->getTransform() );
	}
}

