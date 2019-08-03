/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkragdoll/hkRagdoll.h>

#include <hkragdoll/controller/poweredconstraint/hkRagdollPoweredConstraintController.h>
#include <hkragdoll/instance/hkRagdollInstance.h>
#include <hkragdoll/utils/hkRagdollUtils.h>

#include <hkdynamics/constraint/bilateral/limitedhinge/hkLimitedHingeConstraintData.h>
#include <hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h>
#include <hkdynamics/constraint/motor/position/hkPositionConstraintMotor.h>

void hkRagdollPoweredConstraintController::startMotors ( hkRagdollInstance* ragdoll )
{
	const int numBones = ragdoll->getNumBones();

		// We count the number of powered constraints
	int poweredConstraints = 0;

	// Activate all motors
	for (int b=0; b<numBones; b++)
	{
		hkConstraintInstance* constraint = ragdoll->getConstraintOfBone(b);
		if (!constraint)
		{
			continue;
		}

		hkConstraintData* constraintData = constraint->getData();

		if (constraintData)
		{
			switch (constraintData->getType())
			{
				case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
				{
					hkLimitedHingeConstraintData* lhc = static_cast<hkLimitedHingeConstraintData*> (constraintData);
					lhc->setMotorActive( constraint, true);
					//lhc->getMotor()->setMasterWeight(m_masterWeight);
					lhc->setMotorTargetAngle( 0.0f );

					++poweredConstraints;
					break;
				}

				case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
				{
					hkRagdollConstraintData* rc = static_cast<hkRagdollConstraintData*> (constraintData);
					rc->setMotorsActive( constraint, true);
					//rc->getConeMotor()->setMasterWeight(m_masterWeight);
					//rc->getTwistMotor()->setMasterWeight(m_masterWeight);
					//rc->getPlaneMotor()->setMasterWeight(m_masterWeight);
					rc->setTargetFromRelativeFrame(hkTransform::getIdentity().getRotation());

					++poweredConstraints;
					break;
				}
			}
		}
	}

	if ((numBones / 2) > poweredConstraints)
	{
		HK_WARN (0x36688ecd, "Only "<<poweredConstraints<<" joints out of "<<(numBones-1)<<" is powered. "<<
				"Have you called hkRagdollUtils::powerConstraints() ?");

	}
}

void hkRagdollPoweredConstraintController::stopMotors ( hkRagdollInstance* ragdoll )
{
	// Deactivate all motors
	const int numBones = ragdoll->getNumBones();

	for (int b=0; b<numBones; b++)
	{
		hkConstraintInstance* constraint = ragdoll->getConstraintOfBone(b);
		if (!constraint)
		{
			continue;
		}

		hkConstraintData* constraintData = constraint->getData();

		if (constraintData)
		{
			switch (constraintData->getType())
			{
				case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
				{
					hkLimitedHingeConstraintData* lhc = static_cast<hkLimitedHingeConstraintData*> (constraintData);

					lhc->setMotorActive( constraint, false);
					break;
				}

				case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
				{
					hkRagdollConstraintData* rc = static_cast<hkRagdollConstraintData*> (constraintData);

					rc->setMotorsActive(constraint, false);
					break;
				}
			}
		}
	}
}

void hkRagdollPoweredConstraintController::driveToPose ( hkRagdollInstance* ragdoll, const hkQsTransform* desiredPose )
{
	const int numBones = ragdoll->getNumBones();
	
	for (int b=0; b<numBones; b++)
	{
		hkConstraintInstance* constraint = ragdoll->getConstraintOfBone(b);

		if (!constraint)
		{
			continue;
		}

		const hkQsTransform& RBfromRA = desiredPose[b];

		hkConstraintData* constraintData = constraint->getData();

		switch (constraintData->getType())
		{
			case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
				{
					hkLimitedHingeConstraintData* phc = static_cast<hkLimitedHingeConstraintData*> (constraintData);

					// Constraint space A
					hkQuaternion RAFromCA; RAFromCA.set( phc->m_atoms.m_transforms.m_transformA.getRotation() );

					// Constraint space B
					hkQuaternion RBFromCB; RBFromCB.set( phc->m_atoms.m_transforms.m_transformB.getRotation() );

					hkQuaternion RBFromCA; RBFromCA.setMul( RBfromRA.m_rotation, RAFromCA );
					hkQuaternion CBFromCA; CBFromCA.setInverseMul( RBFromCB, RBFromCA );

					hkVector4 desiredRotAxis; desiredRotAxis.set(1,0,0);
					hkQuaternion rest; hkReal desiredAngle;
					CBFromCA.decomposeRestAxis(desiredRotAxis, rest, desiredAngle);
					phc->setMotorTargetAngle( desiredAngle );
					break;
				}

			case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
				{
					hkRagdollConstraintData* rc = static_cast<hkRagdollConstraintData*> (constraintData);

					hkRotation desired; desired.set (RBfromRA.getRotation());

					rc->setTargetFromRelativeFrame(desired);

					//rc->getConeMotor()->setMasterWeight(m_masterWeight);
					//rc->getTwistMotor()->setMasterWeight(m_masterWeight);
					//rc->getPlaneMotor()->setMasterWeight(m_masterWeight);

					break;
				}
		}
	
	}

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
