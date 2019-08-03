/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/bilateral/limitedhinge/hkLimitedHingeConstraintData.h>
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkdynamics/constraint/motor/hkConstraintMotor.h>

//#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
//#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>
//#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
//#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>

#include <hkmath/linear/hkVector4Util.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkLimitedHingeConstraintData);


hkLimitedHingeConstraintData::hkLimitedHingeConstraintData()
{
	m_atoms.m_transforms.m_transformA.setIdentity();
	m_atoms.m_transforms.m_transformB.setIdentity();

	m_atoms.m_2dAng.m_freeRotationAxis = 0;

	m_atoms.m_angLimit.m_limitAxis = 0;
	m_atoms.m_angLimit.m_minAngle = -HK_REAL_PI;
	m_atoms.m_angLimit.m_maxAngle =  HK_REAL_PI;
	m_atoms.m_angLimit.m_angularLimitsTauFactor = 1.0f;

	m_atoms.m_angFriction.m_firstFrictionAxis = 0;
	m_atoms.m_angFriction.m_numFrictionAxes = 1;
	m_atoms.m_angFriction.m_maxFrictionTorque = 0.0f;


	{	// the initialized variable is placed after all the solver result
		m_atoms.m_angMotor.m_initializedOffset = HK_OFFSET_OF( Runtime, m_initialized );

		// the previous target angle is stored right after it

		m_atoms.m_angMotor.m_previousTargetAngleOffset = HK_OFFSET_OF( Runtime, m_previousTargetAngle ); 

		m_atoms.m_angMotor.m_correspondingAngLimitSolverResultOffset = 2 * sizeof(hkSolverResults);
	}

	m_atoms.m_angMotor.m_isEnabled = false;
	m_atoms.m_angMotor.m_targetAngle = 0.0f;
	m_atoms.m_angMotor.m_motor = HK_NULL;
	m_atoms.m_angMotor.m_motorAxis = 0;
}

hkLimitedHingeConstraintData::~hkLimitedHingeConstraintData()
{
	if( m_atoms.m_angMotor.m_motor )
	{
		m_atoms.m_angMotor.m_motor->removeReference();
	}
}


void hkLimitedHingeConstraintData::setInWorldSpace(const hkTransform& bodyATransform,
													const hkTransform& bodyBTransform,
													const hkVector4& pivot,
													const hkVector4& axis)
{
	hkVector4 perpToAxle1;
	hkVector4 perpToAxle2;
	hkVector4Util::calculatePerpendicularVector( axis, perpToAxle1 ); perpToAxle1.normalize3();
	perpToAxle2.setCross(axis, perpToAxle1);

	m_atoms.m_transforms.m_transformA.getColumn(0).setRotatedInverseDir(bodyATransform.getRotation(), axis);
	m_atoms.m_transforms.m_transformA.getColumn(1).setRotatedInverseDir(bodyATransform.getRotation(), perpToAxle1);
	m_atoms.m_transforms.m_transformA.getColumn(2).setRotatedInverseDir(bodyATransform.getRotation(), perpToAxle2);
	m_atoms.m_transforms.m_transformA.getColumn(3).setTransformedInversePos(bodyATransform, pivot);

	m_atoms.m_transforms.m_transformB.getColumn(0).setRotatedInverseDir(bodyBTransform.getRotation(), axis);
	m_atoms.m_transforms.m_transformB.getColumn(1).setRotatedInverseDir(bodyBTransform.getRotation(), perpToAxle1);
	m_atoms.m_transforms.m_transformB.getColumn(2).setRotatedInverseDir(bodyBTransform.getRotation(), perpToAxle2);
	m_atoms.m_transforms.m_transformB.getColumn(3).setTransformedInversePos(bodyBTransform, pivot);

	HK_ASSERT2(0x3a0a5294, isValid(), "Members of LimitedHinge constraint inconsistent after World Space constructor..");
}


void hkLimitedHingeConstraintData::setInBodySpace(const hkVector4& pivotA,
												   const hkVector4& pivotB,
												   const hkVector4& axisA,
												   const hkVector4& axisB,
												   const hkVector4& axisAPerp,
												   const hkVector4& axisBPerp)
{
	m_atoms.m_transforms.m_transformA.getColumn(0) = axisA;
	m_atoms.m_transforms.m_transformA.getColumn(0).normalize3();
	m_atoms.m_transforms.m_transformA.getColumn(1) = axisAPerp;
	m_atoms.m_transforms.m_transformA.getColumn(1).normalize3();
	m_atoms.m_transforms.m_transformA.getColumn(2).setCross(axisA, axisAPerp);
	m_atoms.m_transforms.m_transformA.getColumn(3) = pivotA;

	m_atoms.m_transforms.m_transformB.getColumn(0) = axisB;
	m_atoms.m_transforms.m_transformB.getColumn(0).normalize3();
	m_atoms.m_transforms.m_transformB.getColumn(1) = axisBPerp;
	m_atoms.m_transforms.m_transformB.getColumn(1).normalize3();
	m_atoms.m_transforms.m_transformB.getColumn(2).setCross(axisB, axisBPerp);
	m_atoms.m_transforms.m_transformB.getColumn(3) = pivotB;

	HK_ASSERT2(0x3a0a5394, isValid(), "Members of LimitedHinge constraint inconsistent after Body Space constructor..");
}

void hkLimitedHingeConstraintData::setMotorActive( hkConstraintInstance* instance, hkBool toBeEnabled )
{
	m_atoms.m_angMotor.m_isEnabled = toBeEnabled;
	m_atoms.m_angFriction.m_isEnabled = !toBeEnabled;

	if ( instance && instance->getRuntime() )
	{
		Runtime* runtime = getRuntime( instance->getRuntime() );

		runtime->m_solverResults[SOLVER_RESULT_MOTOR   ].m_impulseApplied = 0.0f;
		runtime->m_solverResults[SOLVER_RESULT_MOTOR   ].m_internalSolverData = 0.0f;
		runtime->m_solverResults[SOLVER_RESULT_FRICTION].m_impulseApplied = 0.0f;
		runtime->m_solverResults[SOLVER_RESULT_FRICTION].m_internalSolverData = 0.0f;
	}
}

void hkLimitedHingeConstraintData::setMotor( hkConstraintMotor* motor )
{
	if( motor )
	{
		motor->addReference();
	}
	
	if( m_atoms.m_angMotor.m_motor )
	{
		m_atoms.m_angMotor.m_motor->removeReference();
	}
	m_atoms.m_angMotor.m_motor = motor;
}

void hkLimitedHingeConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& infoOut ) const 
{
	getConstraintInfoUtil( m_atoms.getAtoms(), m_atoms.getSizeOfAllAtoms(), infoOut );
}

void hkLimitedHingeConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	// we need runtime data to be able to support lastAngle and friction
	infoOut.m_numSolverResults = SOLVER_RESULT_MAX;
	infoOut.m_sizeOfExternalRuntime = sizeof( Runtime );
}


hkBool hkLimitedHingeConstraintData::isValid() const
{
	return m_atoms.m_transforms.m_transformA.getRotation().isOrthonormal() && m_atoms.m_transforms.m_transformB.getRotation().isOrthonormal() 
		// limits solverResults are used as a reference for motors!
		&& (!m_atoms.m_angMotor.m_isEnabled || m_atoms.m_angLimit.m_isEnabled);
	// Hinge now allows for ranges > 2 * HK_REAL_PI
}


int hkLimitedHingeConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE;
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
