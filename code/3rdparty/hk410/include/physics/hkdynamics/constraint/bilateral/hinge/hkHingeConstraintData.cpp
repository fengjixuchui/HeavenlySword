/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkmath/linear/hkVector4Util.h>

#include <hkdynamics/constraint/bilateral/hinge/hkHingeConstraintData.h>
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkHingeConstraintData);

hkHingeConstraintData::hkHingeConstraintData()
{
	m_atoms.m_transforms.m_transformA.setIdentity();
	m_atoms.m_transforms.m_transformB.setIdentity();

	m_atoms.m_2dAng.m_freeRotationAxis = 0;
}

//
// hkHingeCinfo initialization methods
//

void hkHingeConstraintData::setInWorldSpace( const hkTransform& bodyATransform, const hkTransform& bodyBTransform,
										const hkVector4& pivot, const hkVector4& axis ) 
{
	hkVector4 perpToAxle1;
	hkVector4 perpToAxle2;
	hkVector4Util::calculatePerpendicularVector( axis, perpToAxle1 ); perpToAxle1.normalize3();
	perpToAxle2.setCross(axis, perpToAxle1);

	m_atoms.m_transforms.m_transformA.getColumn(0).setRotatedInverseDir(bodyATransform.getRotation(), axis);
	m_atoms.m_transforms.m_transformA.getColumn(1).setRotatedInverseDir(bodyATransform.getRotation(), perpToAxle1);
	m_atoms.m_transforms.m_transformA.getColumn(2).setRotatedInverseDir(bodyATransform.getRotation(), perpToAxle2);
	m_atoms.m_transforms.m_transformA.getTranslation().setTransformedInversePos(bodyATransform, pivot);

	m_atoms.m_transforms.m_transformB.getColumn(0).setRotatedInverseDir(bodyBTransform.getRotation(), axis);
	m_atoms.m_transforms.m_transformB.getColumn(1).setRotatedInverseDir(bodyBTransform.getRotation(), perpToAxle1);
	m_atoms.m_transforms.m_transformB.getColumn(2).setRotatedInverseDir(bodyBTransform.getRotation(), perpToAxle2);
	m_atoms.m_transforms.m_transformB.getTranslation().setTransformedInversePos(bodyBTransform, pivot);

	HK_ASSERT2(0x3a0a5292, isValid(), "Members of Hinge constraint inconsistent after World Space constructor.");
}


void hkHingeConstraintData::setInBodySpace( const hkVector4& pivotA,const hkVector4& pivotB,
									  const hkVector4& axisA,const hkVector4& axisB)
{
	m_atoms.m_transforms.m_transformA.getTranslation() = pivotA;
	m_atoms.m_transforms.m_transformB.getTranslation() = pivotB;

	hkVector4* baseA = &m_atoms.m_transforms.m_transformA.getColumn(0);
	baseA[0] = axisA; baseA[0].normalize3();
	hkVector4Util::calculatePerpendicularVector( baseA[0], baseA[1] ); baseA[1].normalize3();
	baseA[2].setCross( baseA[0], baseA[1] );

	hkVector4* baseB = &m_atoms.m_transforms.m_transformB.getColumn(0);
	baseB[0] = axisB; baseB[0].normalize3();
	hkVector4Util::calculatePerpendicularVector( baseB[0], baseB[1] ); baseB[1].normalize3();
	baseB[2].setCross( baseB[0], baseB[1] );

	HK_ASSERT2(0x3a0a5292, isValid(), "Members of Hinge constraint inconsistent after Body Space constructor..");
}

void hkHingeConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	if ( wantRuntime )
	{
		infoOut.m_numSolverResults = SOLVER_RESULT_MAX;
		infoOut.m_sizeOfExternalRuntime = sizeof( Runtime);
	}
	else
	{
		infoOut.m_numSolverResults = 0;
		infoOut.m_sizeOfExternalRuntime = 0;
	}
}

void hkHingeConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& infoOut ) const 
{
	getConstraintInfoUtil( m_atoms.getAtoms(), m_atoms.getSizeOfAllAtoms(), infoOut );
}



hkBool hkHingeConstraintData::isValid() const
{
	return m_atoms.m_transforms.m_transformA.getRotation().isOrthonormal() && m_atoms.m_transforms.m_transformB.getRotation().isOrthonormal();
}


int hkHingeConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_HINGE;
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
