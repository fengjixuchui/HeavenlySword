/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkdynamics/constraint/bilateral/prismatic/hkPrismaticConstraintData.h>
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>
#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkdynamics/constraint/motor/hkConstraintMotor.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkPrismaticConstraintData);

hkPrismaticConstraintData::hkPrismaticConstraintData()
{
	m_atoms.m_transforms.m_transformA.setIdentity();
	m_atoms.m_transforms.m_transformB.setIdentity();

	m_atoms.m_linLimit.m_min = -HK_REAL_MAX;
	m_atoms.m_linLimit.m_max =  HK_REAL_MAX;
	m_atoms.m_linLimit.m_axisIndex = 0;

	m_atoms.m_friction.m_maxFrictionForce = 0.0f;
	m_atoms.m_friction.m_frictionAxis = 0;
	m_atoms.m_motor.m_motorAxis = 0;

	m_atoms.m_motor.m_isEnabled = false;
	m_atoms.m_motor.m_targetPosition = 0.0f;
	m_atoms.m_motor.m_motor = HK_NULL;

	m_atoms.m_motor.m_motorAxis = hkPrismaticConstraintData::Atoms::AXIS_SHAFT;

	m_atoms.m_motor.m_initializedOffset = HK_OFFSET_OF(Runtime, m_initialized);
	m_atoms.m_motor.m_previousTargetPositionOffset = HK_OFFSET_OF(Runtime, m_previousTargetPosition);

	m_atoms.m_ang.m_firstConstrainedAxis = 0;
	m_atoms.m_ang.m_numConstrainedAxes   = 3;
	m_atoms.m_lin0.m_axisIndex = 1;
	m_atoms.m_lin1.m_axisIndex = 2;
}

hkPrismaticConstraintData::~hkPrismaticConstraintData()
{
	if( m_atoms.m_motor.m_motor )
	{
		m_atoms.m_motor.m_motor->removeReference();
	}
}


void hkPrismaticConstraintData::setInWorldSpace( const hkTransform& bodyATransform, const hkTransform& bodyBTransform,
											 const hkVector4& pivot,const hkVector4& axis)
{
		// Set relative orientation
	{
		hkVector4 constraintBaseInW[3];

		constraintBaseInW[0] = axis; constraintBaseInW[0].normalize3();
		hkVector4Util::calculatePerpendicularVector( constraintBaseInW[0], constraintBaseInW[1] ); constraintBaseInW[1].normalize3();
		HK_ASSERT2(0xadbbc333, hkMath::equal( 1.0f, constraintBaseInW[1].lengthSquared3() ), "Vector perpendicualr to axis is not normalized");
		constraintBaseInW[2].setCross( constraintBaseInW[0], constraintBaseInW[1] );

		hkVector4Util::rotateInversePoints( bodyATransform.getRotation(), constraintBaseInW, 3, &m_atoms.m_transforms.m_transformA.getRotation().getColumn(0) );
		hkVector4Util::rotateInversePoints( bodyBTransform.getRotation(), constraintBaseInW, 3, &m_atoms.m_transforms.m_transformB.getRotation().getColumn(0) );
	}

		// Set pivot points
	m_atoms.m_transforms.m_transformA.getTranslation().setTransformedInversePos( bodyATransform, pivot );
	m_atoms.m_transforms.m_transformB.getTranslation().setTransformedInversePos( bodyBTransform, pivot );

	HK_ASSERT2(0x4b2bf185, isValid(), "Members of prismatic constraint inconsistent.");

}

void hkPrismaticConstraintData::setInBodySpace( const hkVector4& pivotA, const hkVector4& pivotB,
											  const hkVector4& axisA, const hkVector4& axisB,
											  const hkVector4& axisAPerp, const hkVector4& axisBPerp)
{
	m_atoms.m_transforms.m_transformA.getTranslation() = pivotA;
	m_atoms.m_transforms.m_transformB.getTranslation() = pivotB;

	hkVector4* baseA = &m_atoms.m_transforms.m_transformA.getColumn(0);
	baseA[0] = axisA; baseA[0].normalize3();
	baseA[1] = axisAPerp; baseA[1].normalize3();
	baseA[2].setCross( baseA[0], baseA[1] );

	hkVector4* baseB = &m_atoms.m_transforms.m_transformB.getColumn(0);
	baseB[0] = axisB; baseB[0].normalize3();
	baseB[1] = axisBPerp; baseB[1].normalize3();
	baseB[2].setCross( baseB[0], baseB[1] );

	HK_ASSERT2(0x4b2bf185, isValid(), "Members of prismatic constraint inconsistent.");
}

void hkPrismaticConstraintData::setMotor( hkConstraintMotor* motor )
{
	if( motor )
	{
		motor->addReference();
	}

	if( m_atoms.m_motor.m_motor )
	{
		m_atoms.m_motor.m_motor->removeReference();
	}

	m_atoms.m_motor.m_motor = motor;
}

void hkPrismaticConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& infoOut ) const 
{
	getConstraintInfoUtil( m_atoms.getAtoms(), m_atoms.getSizeOfAllAtoms(), infoOut );
}

void hkPrismaticConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	// always request a runtime 
	infoOut.m_numSolverResults = SOLVER_RESULT_MAX;
	infoOut.m_sizeOfExternalRuntime = sizeof( Runtime );
}


hkBool hkPrismaticConstraintData::isValid() const
{
	return m_atoms.m_transforms.m_transformA.getRotation().isOrthonormal() && m_atoms.m_transforms.m_transformB.getRotation().isOrthonormal() && m_atoms.m_linLimit.m_min <= m_atoms.m_linLimit.m_max;
}

int hkPrismaticConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_PRISMATIC;
}

void hkPrismaticConstraintData::setMotorActive( hkConstraintInstance* instance, hkBool toBeEnabled )
{
	m_atoms.m_motor.m_isEnabled = toBeEnabled;
	m_atoms.m_friction.m_isEnabled = !toBeEnabled;

	if ( instance && instance->getRuntime() )
	{
		Runtime* runtime = getRuntime( instance->getRuntime() );

		runtime->m_solverResults[SOLVER_RESULT_MOTOR   ].m_impulseApplied = 0.0f;
		runtime->m_solverResults[SOLVER_RESULT_MOTOR   ].m_internalSolverData = 0.0f;
		runtime->m_solverResults[SOLVER_RESULT_FRICTION].m_impulseApplied = 0.0f;
		runtime->m_solverResults[SOLVER_RESULT_FRICTION].m_internalSolverData = 0.0f;
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
