
#include "config.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include "hsRagdollInstance.h"
#include "physics/world.h"

#include <hkragdoll/hkRagdoll.h>

#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>
#include <hkanimation/rig/hkPose.h>
#include <hkdynamics/world/hkWorld.h>

#include <hkbase/class/hkTypeInfo.h>
HK_REFLECTION_DEFINE_VIRTUAL(hsRagdollInstance);

hsRagdollInstance::hsRagdollInstance ( const hkArray<hkRigidBody*>& rigidBodies, const hkArray<hkConstraintInstance*>& constraints, const hkSkeleton* skeleton )
:	hkRagdollInstance( rigidBodies, constraints, skeleton )
{
}

hsRagdollInstance::~hsRagdollInstance ()
{
}

void hsRagdollInstance::getPoseModelSpace(hkQsTransform* poseOut, const hkQsTransform& worldFromModel) const
{
	const int numBones = m_skeleton->m_numBones;

	hkQsTransform transformWS;
	for (int b=0; b < numBones; b++)
	{
		getBoneTransform(b, transformWS);
		poseOut[b].setMulInverseMul(worldFromModel, transformWS);
	}
}

void hsRagdollInstance::setPoseModelSpace(const hkQsTransform* poseIn, const hkQsTransform& worldFromModel)
{
	const int numBones = getNumBones();

	Physics::WriteAccess mutex;

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

void hsRagdollInstance::setPoseModelSpaceKeyframedOnly(const hkQsTransform* poseIn, const hkQsTransform& worldFromModel)
{
	const int numBones = getNumBones();

	Physics::WriteAccess mutex;

	for (int b=0; b<numBones; b++)
	{
		hkRigidBody* rbody = getRigidBodyOfBone(b);
		if(rbody->isFixedOrKeyframed())
		{
			hkQsTransform rbTransform; rbTransform.setMul(worldFromModel, poseIn[b]);
			rbTransform.m_rotation.normalize();

			rbody->setPositionAndRotation(rbTransform.getTranslation(), rbTransform.getRotation());
			rbody->setLinearVelocity(hkVector4::getZero());
			rbody->setAngularVelocity(hkVector4::getZero());
		}
	}

}

// Add to world
hkResult hsRagdollInstance::addToWorld ( hkBool updateFilter, hkEntityActivation initialActivationState ) const
{
	if (getWorld())
	{
		HK_WARN(0x79faa2d6, "addToWorld: Ragdoll already part of a world");
		return HK_FAILURE;
	}

	Physics::CPhysicsWorld::Get().AddEntityBatch( reinterpret_cast<hkEntity*const*> ( m_rigidBodies.begin() ), m_rigidBodies.getSize(), initialActivationState ); 

	// contraints 
	for (int c=0; c < m_constraints.getSize(); ++c)
	{
		Physics::CPhysicsWorld::Get().AddConstraint( m_constraints[ c ] );		
	}


	if (updateFilter)
	{
		const int numBones = m_skeleton->m_numBones;

		for (int rb=0; rb<numBones; ++rb)
		{
			hkRigidBody* rbody = m_rigidBodies[rb];
			Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity(rbody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_IGNORE_SHAPE_COLLECTIONS);
		}
	}

	return HK_SUCCESS;
}

// Remove from world
hkResult hsRagdollInstance::removeFromWorld () const
{
	hkWorld* world = getWorld();

	if (!world)
	{
		HK_WARN(0x79faa2d6, "removeFromWorld: Ragdoll is not part of any world");
		return HK_FAILURE;
	}

	world->markForWrite();

	// contraints 
	for (int c=0; c < m_constraints.getSize(); ++c)
	{
		world->removeConstraint(m_constraints[c]);		
	}

	world->removeEntityBatch( reinterpret_cast<hkEntity*const*>(m_rigidBodies.begin()), m_rigidBodies.getSize() );

	world->unmarkForWrite();

	return HK_SUCCESS;
}


/// Cloning

hsRagdollInstance* hsRagdollInstance::clone() const
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

	hsRagdollInstance* newInstance = HK_NEW hsRagdollInstance(clonedRigidBodies, clonedConstraints, m_skeleton);

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

#endif

