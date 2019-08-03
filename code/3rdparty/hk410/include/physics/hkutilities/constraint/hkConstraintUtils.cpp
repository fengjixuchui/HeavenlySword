/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkutilities/constraint/hkConstraintUtils.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/constraint/bilateral/ballandsocket/hkBallAndSocketConstraintData.h>
#include <hkdynamics/constraint/bilateral/hinge/hkHingeConstraintData.h>
#include <hkdynamics/constraint/bilateral/limitedhinge/hkLimitedHingeConstraintData.h>

#include <hkdynamics/constraint/chain/ballsocket/hkBallSocketChainData.h>
#include <hkdynamics/constraint/chain/powered/hkPoweredChainData.h>

#include <hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h>

#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/constraint/motor/hkConstraintMotor.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstance.h>
#include <hkdynamics/constraint/chain/ballsocket/hkBallSocketChainData.h>

#include <hkdynamics/constraint/bilateral/ballandsocket/hkBallAndSocketConstraintData.h>
//#include <hkdynamics/constraint/motor/position/hkPositionConstraintMotor.h>

#include <hkdynamics/world/hkSimulationIsland.h>

#include <hkinternal/dynamics/constraint/chain/ragdolllimits/hkRagdollLimitsData.h>
#include <hkdynamics/constraint/chain/hingelimits/hkHingeLimitsData.h>

#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>

hkConstraintInstance* hkConstraintUtils::convertToPowered (const hkConstraintInstance* originalConstraint, hkConstraintMotor* constraintMotor)
{
	hkConstraintInstance* newConstraint = HK_NULL;
	hkConstraintData* constraintData = originalConstraint->getData();
	
	hkEntity* entityA = originalConstraint->getEntityA();
	hkEntity* entityB = originalConstraint->getEntityB();

	switch (constraintData->getType())
	{
		case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
			{
				hkLimitedHingeConstraintData* oldConstraintData = static_cast<hkLimitedHingeConstraintData*> (constraintData);
				hkLimitedHingeConstraintData* newConstraintData = new hkLimitedHingeConstraintData();

				newConstraintData->m_atoms = oldConstraintData->m_atoms;

				newConstraintData->setMotor( constraintMotor );
				newConstraintData->setMotorActive(HK_NULL, true);

				newConstraint = new hkConstraintInstance (entityA, entityB, newConstraintData, originalConstraint->getPriority());
				newConstraintData->removeReference();

				newConstraint->setName( originalConstraint->getName() );

				break;
			}

		case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
			{
				hkRagdollConstraintData* oldConstraintData = static_cast<hkRagdollConstraintData*> (constraintData);
				hkRagdollConstraintData* newConstraintData = new hkRagdollConstraintData();

				newConstraintData->m_atoms = oldConstraintData->m_atoms;

				newConstraintData->setTwistMotor(constraintMotor);
				newConstraintData->setPlaneMotor(constraintMotor);
				newConstraintData->setConeMotor(constraintMotor);
				newConstraintData->setMotorsActive(HK_NULL, true);
				
				newConstraint = new hkConstraintInstance (entityA, entityB, newConstraintData, originalConstraint->getPriority());
				newConstraintData->removeReference();

				newConstraint->setName( originalConstraint->getName() );

				break;

			}

		default:
			{
				HK_WARN_ALWAYS (0xabba1b34, "Cannot convert constraint \""<<originalConstraint->getName()<<"\" to a powered constraint.");
				HK_WARN_ALWAYS (0xabba1b34, "Only limited hinges and ragdoll constraints can be powered.");
				return HK_NULL;
			}
	}
	
	return newConstraint;
}

hkConstraintInstance* HK_CALL hkConstraintUtils::convertToLimits (hkConstraintInstance* originalConstraint)
{
	hkConstraintData* originalData = originalConstraint->getData();
	hkConstraintData* limitData = HK_NULL;

	switch(originalData->getType())
	{
	case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
		{
			hkRagdollLimitsData* n = new hkRagdollLimitsData();
			hkRagdollConstraintData* o = static_cast<hkRagdollConstraintData*>(originalData);
			limitData = n;

			n->m_atoms.m_rotations.m_rotationA = o->m_atoms.m_transforms.m_transformA.getRotation();
			n->m_atoms.m_rotations.m_rotationB = o->m_atoms.m_transforms.m_transformB.getRotation();
			n->m_atoms.m_twistLimit  = o->m_atoms.m_twistLimit;
			n->m_atoms.m_planesLimit = o->m_atoms.m_planesLimit;
			n->m_atoms.m_coneLimit   = o->m_atoms.m_coneLimit;

			break;
		}
	case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
		{
			hkHingeLimitsData* n = new hkHingeLimitsData();
			hkLimitedHingeConstraintData* o = static_cast<hkLimitedHingeConstraintData*>(originalData);
			limitData = n;

			n->m_atoms.m_rotations.m_rotationA = o->m_atoms.m_transforms.m_transformA.getRotation();
			n->m_atoms.m_rotations.m_rotationB = o->m_atoms.m_transforms.m_transformB.getRotation();
			n->m_atoms.m_angLimit = o->m_atoms.m_angLimit;
			n->m_atoms.m_2dAng     = o->m_atoms.m_2dAng;

			break;
		}
	case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL_LIMITS:
	case hkConstraintData::CONSTRAINT_TYPE_HINGE_LIMITS:
		{
			// Just return the original constraint.
			// Remember to add a reference.
			HK_WARN(0xad67d7db, "The original constraint already is of the 'limits' type.");
			originalConstraint->addReference();
			return originalConstraint;
		}

	default:
		{
			HK_ASSERT2(0xad67d7da, false, "Unsupported constraint type. Cannot generate limits constraints.");
			return HK_NULL;
		}
	}

	if (limitData)
	{
		hkConstraintInstance* limitInstance = new hkConstraintInstance( originalConstraint->getEntityA(), originalConstraint->getEntityB(), limitData, originalConstraint->getPriority() );
		limitData->removeReference();

		return limitInstance;
	}

	return HK_NULL;
}



hkResult hkConstraintUtils::getConstraintPivots (const hkConstraintInstance* constraint, hkVector4& pivotInAOut, hkVector4& pivotInBOut)
{
	hkConstraintData *constraintData = constraint->getData();

	switch (constraintData->getType())
	{
		case hkConstraintData::CONSTRAINT_TYPE_BALLANDSOCKET:
		{
			hkBallAndSocketConstraintData* bsConstraint = static_cast<hkBallAndSocketConstraintData*> (constraintData);
			pivotInAOut = bsConstraint->m_atoms.m_pivots.m_translationA;
			pivotInBOut = bsConstraint->m_atoms.m_pivots.m_translationB;
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_HINGE:
		{
			hkHingeConstraintData* hConstraint = static_cast<hkHingeConstraintData*> (constraintData);
			pivotInAOut = hConstraint->m_atoms.m_transforms.m_transformA.getTranslation();
			pivotInBOut = hConstraint->m_atoms.m_transforms.m_transformB.getTranslation();
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
		{
			hkLimitedHingeConstraintData* hConstraint = static_cast<hkLimitedHingeConstraintData*> (constraintData);
			pivotInAOut = hConstraint->m_atoms.m_transforms.m_transformA.getTranslation();
			pivotInBOut = hConstraint->m_atoms.m_transforms.m_transformB.getTranslation();
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
		{
			hkRagdollConstraintData* rConstraint = static_cast<hkRagdollConstraintData*> (constraintData);
			pivotInAOut = rConstraint->m_atoms.m_transforms.m_transformA.getTranslation();
			pivotInBOut = rConstraint->m_atoms.m_transforms.m_transformB.getTranslation();
		}
		break;

		default:
		{
			HK_WARN_ALWAYS (0xabbabf3b, "Unsupported type of constraint in prepareSystemForRagdoll()");
			return HK_FAILURE;
		}
	}

	return HK_SUCCESS;
}

hkResult hkConstraintUtils::getConstraintMotors(const hkConstraintData* constraintData, hkConstraintMotor*& motor0, hkConstraintMotor*& motor1, hkConstraintMotor*& motor2 )
{
	switch (constraintData->getType())
	{
	case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
		{
			const hkLimitedHingeConstraintData* hConstraint = static_cast<const hkLimitedHingeConstraintData*> (constraintData);
			motor0 = hConstraint->getMotor();
			motor1 = HK_NULL;
			motor2 = HK_NULL;
		}
		break;

	case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
		{
			const hkRagdollConstraintData* rConstraint = static_cast<const hkRagdollConstraintData*> (constraintData);
			// Possibly, those motors should be extracted in a different order.
			motor0 = rConstraint->getTwistMotor();
			motor1 = rConstraint->getConeMotor();
			motor2 = rConstraint->getPlaneMotor();
		}
		break;

	default:
		{
			motor0 = motor1 = motor2 = HK_NULL;
			HK_WARN_ALWAYS (0xabbae233, "This type of constraint does not have motors");
			return HK_FAILURE;
		}
	}

	return HK_SUCCESS;
}


hkBool hkConstraintUtils::checkAndFixConstraint (const hkConstraintInstance* constraint, hkReal maxAllowedError, hkReal relativeFixupOnError)
{
	hkVector4 childPivotInChild;
	hkVector4 parentPivotInParent;

	hkResult res = 	getConstraintPivots(constraint, childPivotInChild, parentPivotInParent);

	// Unsupported constraint type? return false
	if (res!=HK_SUCCESS)
	{
		return false;
	}

	// The pivotInA should be 0,0,0 if the pivots are properly aligned, warn otherwise
	if (childPivotInChild.lengthSquared3()>1e-6f)
	{
		HK_WARN_ALWAYS (0xabba5dff, "Pivot of child rigid body (A) is expected to be aligned with the constraint at setup time.");
	}

	hkRigidBody* parentRigidBody = constraint->getRigidBodyB();
	hkRigidBody* childRigidBody = constraint->getRigidBodyA();

	const hkTransform& worldFromParent = parentRigidBody->getTransform();
	const hkTransform& worldFromChild = childRigidBody->getTransform();

	hkVector4 parentPivotInWorld; parentPivotInWorld.setTransformedPos(worldFromParent, parentPivotInParent);
	hkVector4 childPivotInWorld; childPivotInWorld.setTransformedPos(worldFromChild, childPivotInChild);

	hkVector4 error; error.setSub4(parentPivotInWorld, childPivotInWorld);

	// Are they aligned ?
	if (error.lengthSquared3()>maxAllowedError*maxAllowedError)
	{
		// NO

		// Interpolate the new position between the desired position and the position of the parent
		const hkVector4& parentPositionInWorld = parentRigidBody->getPosition();
		hkVector4 newChildPositionInWorld; newChildPositionInWorld.setInterpolate4(parentPivotInWorld, parentPositionInWorld, relativeFixupOnError);

		childRigidBody->setPosition(newChildPositionInWorld);

		// Set the velocity to match the parent
		childRigidBody->setLinearVelocity(parentRigidBody->getLinearVelocity());
		childRigidBody->setAngularVelocity(parentRigidBody->getAngularVelocity());

		return true; // fix up done
	}
	else
	{
		// YES

		return false; // no fix up done
	}

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
