/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/chain/hingelimits/hkHingeLimitsData.h>
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>

#include <hkmath/linear/hkVector4Util.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkHingeLimitsData);


hkHingeLimitsData::hkHingeLimitsData()
{
	m_atoms.m_rotations.m_rotationA.setIdentity();
	m_atoms.m_rotations.m_rotationB.setIdentity();

	m_atoms.m_2dAng.m_freeRotationAxis = 0;

	m_atoms.m_angLimit.m_limitAxis = 0;
	m_atoms.m_angLimit.m_minAngle = -HK_REAL_PI;
	m_atoms.m_angLimit.m_maxAngle =  HK_REAL_PI;
	m_atoms.m_angLimit.m_angularLimitsTauFactor = 1.0f;
}

/// \param bodyA			The first rigid body transform.
/// \param bodyB			The second rigid body transform.
/// \param axis				The hinge axis, specified in world space.
void hkHingeLimitsData::setInWorldSpace(const hkTransform& bodyATransform,
													const hkTransform& bodyBTransform,
													const hkVector4& axis)
{
	hkVector4 perpToAxle1;
	hkVector4 perpToAxle2;
	hkVector4Util::calculatePerpendicularVector( axis, perpToAxle1 ); perpToAxle1.normalize3();
	perpToAxle2.setCross(axis, perpToAxle1);

	m_atoms.m_rotations.m_rotationA.getColumn(0).setRotatedInverseDir(bodyATransform.getRotation(), axis);
	m_atoms.m_rotations.m_rotationA.getColumn(1).setRotatedInverseDir(bodyATransform.getRotation(), perpToAxle1);
	m_atoms.m_rotations.m_rotationA.getColumn(2).setRotatedInverseDir(bodyATransform.getRotation(), perpToAxle2);

	m_atoms.m_rotations.m_rotationB.getColumn(0).setRotatedInverseDir(bodyBTransform.getRotation(), axis);
	m_atoms.m_rotations.m_rotationB.getColumn(1).setRotatedInverseDir(bodyBTransform.getRotation(), perpToAxle1);
	m_atoms.m_rotations.m_rotationB.getColumn(2).setRotatedInverseDir(bodyBTransform.getRotation(), perpToAxle2);

	HK_ASSERT2(0x3a0a5284, isValid(), "Members of HingeLimits constraint inconsistent after World Space constructor..");

}

/////////////////////////////////////////////////////////////////////

/// \param axisA			The hinge axis, specified in bodyA space.
/// \param axisB			The hinge axis, specified in bodyB space.
/// \param axisAPerp		Axis perpendicular to the hinge axis, specified in bodyA space.
/// \param axisBPerp		Axis perpendicular to the hinge axis, specified in bodyB space.
void hkHingeLimitsData::setInBodySpace(const hkVector4& axisA,
												   const hkVector4& axisB,
												   const hkVector4& axisAPerp,
												   const hkVector4& axisBPerp)
{
	m_atoms.m_rotations.m_rotationA.getColumn(0) = axisA;
	m_atoms.m_rotations.m_rotationA.getColumn(0).normalize3();
	m_atoms.m_rotations.m_rotationA.getColumn(1) = axisAPerp;
	m_atoms.m_rotations.m_rotationA.getColumn(1).normalize3();
	m_atoms.m_rotations.m_rotationA.getColumn(2).setCross(axisA, axisAPerp);

	m_atoms.m_rotations.m_rotationB.getColumn(0) = axisB;
	m_atoms.m_rotations.m_rotationB.getColumn(0).normalize3();
	m_atoms.m_rotations.m_rotationB.getColumn(1) = axisBPerp;
	m_atoms.m_rotations.m_rotationB.getColumn(1).normalize3();
	m_atoms.m_rotations.m_rotationB.getColumn(2).setCross(axisB, axisBPerp);

	HK_ASSERT2(0x3a0a5385, isValid(), "Members of LimitedHinge constraint inconsistent after Body Space constructor..");

}

//////////////////////////////////////////////////////////////////////


void hkHingeLimitsData::getConstraintInfo( hkConstraintData::ConstraintInfo& infoOut ) const 
{
	getConstraintInfoUtil( m_atoms.getAtoms(), m_atoms.getSizeOfAllAtoms(), infoOut );
}

void hkHingeLimitsData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	// we need runtime data to be able to support lastAngle and friction
	infoOut.m_numSolverResults = SOLVER_RESULT_MAX;
	infoOut.m_sizeOfExternalRuntime = sizeof( Runtime );
}

////////////////////////////////////////////////////////////////////////

hkBool hkHingeLimitsData::isValid() const
{
	return m_atoms.m_rotations.m_rotationA.isOrthonormal() && m_atoms.m_rotations.m_rotationB.isOrthonormal();
	// Hinge now allows for ranges > 2 * HK_REAL_PI
}



////////////////////////////////////////////////////////////////////////////

int hkHingeLimitsData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_HINGE_LIMITS;
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
