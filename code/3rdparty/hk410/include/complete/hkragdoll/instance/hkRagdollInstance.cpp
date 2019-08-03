/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkragdoll/hkRagdoll.h>
#include <hkragdoll/instance/hkRagdollInstance.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkRagdollInstance);

hkRagdollInstance::hkRagdollInstance ( const hkArray<hkRigidBody*>& rigidBodies, const hkArray<hkConstraintInstance*>& constraints, const hkSkeleton* skeleton )
:
	m_skeleton (skeleton)
{
	m_rigidBodies = rigidBodies;
	m_constraints = constraints;

	HK_ASSERT2(0x54366cce, m_rigidBodies.getSize()-1 == m_constraints.getSize(), "Inconsistent number of rigid bodies and constraints");
	HK_ASSERT2(0x54366cce, m_rigidBodies.getSize() == m_skeleton->m_numBones, "Inconsistent number of rigid bodies and skeleton bones");

	for (int rb=0; rb<m_rigidBodies.getSize(); rb++)
	{
		m_rigidBodies[rb]->addReference();
	}
	for (int c=0;  c<m_constraints.getSize(); c++)
	{
		m_constraints[c]->addReference();
	}
}

hkRagdollInstance::~hkRagdollInstance ()
{

	for (int rb=0; rb<m_rigidBodies.getSize(); rb++)
	{
		m_rigidBodies[rb]->removeReference();
	}
	for (int c=0;  c<m_constraints.getSize(); c++)
	{
		m_constraints[c]->removeReference();
	}
}

void hkRagdollInstance::getPoseWorldSpace(hkQsTransform* poseOut ) const
{
	for (int b=0; b < m_skeleton->m_numBones; b++)
	{
		getBoneTransform(b, poseOut[b]);
	}
}

void hkRagdollInstance::getPoseModelSpace(hkQsTransform* poseOut, const hkQsTransform& worldFromModel) const
{
	getPoseWorldSpace(poseOut);

	const int numBones = m_skeleton->m_numBones;

	hkQsTransform modelFromWorld;
	modelFromWorld.setInverse( worldFromModel );

	for (int b=0; b < numBones; b++)
	{
		poseOut[b].setMul(modelFromWorld, poseOut[b]);
	}
}

void hkRagdollInstance::setPoseModelSpace(const hkQsTransform* poseIn, const hkQsTransform& worldFromModel)
{
	const int numBones = getNumBones();

	for (int b=0; b<numBones; b++)
	{
		hkRigidBody* rbody = getRigidBodyOfBone(b);

		hkQsTransform rbTransform; rbTransform.setMul(worldFromModel, poseIn[b]);
		rbTransform.m_rotation.normalize();

		rbody->setPositionAndRotation(rbTransform.getTranslation(), rbTransform.getRotation());
		rbody->setLinearVelocity(hkVector4::getZero());
		rbody->setAngularVelocity(hkVector4::getZero());
	}

}

void hkRagdollInstance::setPoseWorldSpace(const hkQsTransform* poseIn)
{
	const int numBones = getNumBones();

	for (int b=0; b<numBones; b++)
	{
		hkRigidBody* rbody = getRigidBodyOfBone(b);

		rbody->setPositionAndRotation(poseIn[b].getTranslation(), poseIn[b].getRotation());
		rbody->setLinearVelocity(hkVector4::getZero());
		rbody->setAngularVelocity(hkVector4::getZero());
	}
}

// Add to world
hkResult hkRagdollInstance::addToWorld (hkWorld* world, hkBool updateFilter) const
{
	if (getWorld())
	{
		HK_WARN(0x79faa2d6, "addToWorld: Ragdoll already part of a world");
		return HK_FAILURE;
	}

	world->addEntityBatch( reinterpret_cast<hkEntity*const*> ( m_rigidBodies.begin() ), m_rigidBodies.getSize() ); 

	// contraints 
	for (int c=0; c < m_constraints.getSize(); ++c)
		{
			world->addConstraint(m_constraints[c]);		
		}


	if (updateFilter)
	{
		const int numBones = m_skeleton->m_numBones;

		for (int rb=0; rb<numBones; ++rb)
		{
			hkRigidBody* rbody = m_rigidBodies[rb];
			world->updateCollisionFilterOnEntity(rbody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_IGNORE_SHAPE_COLLECTIONS);
		}
	}

	return HK_SUCCESS;
}

// Remove from world
hkResult hkRagdollInstance::removeFromWorld () const
{
	hkWorld* world = getWorld();

	if (!world)
	{
		HK_WARN(0x79faa2d6, "removeFromWorld: Ragdoll is not part of any world");
		return HK_FAILURE;
	}

	// contraints 
	for (int c=0; c < m_constraints.getSize(); ++c)
		{
			world->removeConstraint(m_constraints[c]);		
		}

	world->removeEntityBatch( reinterpret_cast<hkEntity*const*>(m_rigidBodies.begin()), m_rigidBodies.getSize() );


	return HK_SUCCESS;
}


hkRagdollInstance* hkRagdollInstance::clone() const
{
	const int numRigidBodies = m_rigidBodies.getSize();
	const int numConstraints = m_constraints.getSize();

	hkLocalArray<hkRigidBody*> clonedRigidBodies (numRigidBodies);
	clonedRigidBodies.setSize(numRigidBodies);

	hkLocalArray<hkConstraintInstance*> clonedConstraints (numConstraints);
	clonedConstraints.setSize(numConstraints);

	{
		for (int rb=0; rb<numRigidBodies; rb++)
		{
			clonedRigidBodies[rb] = m_rigidBodies[rb]->clone();
		}

		for (int c=0; c<numConstraints;c++)
		{
			hkRigidBody* clonedChild = clonedRigidBodies [c+1];
			hkRigidBody* clonedParent = clonedRigidBodies [ m_skeleton->m_parentIndices[c+1] ];
			clonedConstraints[c] = m_constraints[c]->clone(clonedChild, clonedParent);
		}
	}

	hkRagdollInstance* newInstance = new hkRagdollInstance(clonedRigidBodies, clonedConstraints, m_skeleton);

	{
		for (int rb=0; rb<numRigidBodies; rb++)
		{
			clonedRigidBodies[rb]->removeReference();
		}

		for (int c=0; c<numConstraints;c++)
		{
			clonedConstraints[c]->removeReference();
		}
	}

	return newInstance;
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
