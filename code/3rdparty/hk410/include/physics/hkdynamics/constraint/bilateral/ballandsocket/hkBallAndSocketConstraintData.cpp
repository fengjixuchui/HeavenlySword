/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/bilateral/ballandsocket/hkBallAndSocketConstraintData.h>

#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkBallAndSocketConstraintData);

hkBallAndSocketConstraintData::hkBallAndSocketConstraintData()
{
	m_atoms.m_pivots.m_translationA.setZero4();
	m_atoms.m_pivots.m_translationB.setZero4();
}

void hkBallAndSocketConstraintData::setInWorldSpace(const hkTransform& bodyATransform, 
												const hkTransform& bodyBTransform, 
												const hkVector4& pivot)
{
	m_atoms.m_pivots.m_translationA.setTransformedInversePos( bodyATransform, pivot );
	m_atoms.m_pivots.m_translationB.setTransformedInversePos( bodyBTransform, pivot );
}

void hkBallAndSocketConstraintData::setInBodySpace( const hkVector4& pivotA,
												const hkVector4& pivotB)
{
	m_atoms.m_pivots.m_translationA = pivotA;
	m_atoms.m_pivots.m_translationB = pivotB;
}


void hkBallAndSocketConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& infoOut ) const 
{
	getConstraintInfoUtil( m_atoms.getAtoms(), m_atoms.getSizeOfAllAtoms(), infoOut );
}

void hkBallAndSocketConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	if ( wantRuntime )
	{
		infoOut.m_numSolverResults = SOLVER_RESULT_MAX;
		infoOut.m_sizeOfExternalRuntime = sizeof( Runtime );
	}
	else
	{
		infoOut.m_numSolverResults = 0;
		infoOut.m_sizeOfExternalRuntime = 0;
	}
}


hkBool hkBallAndSocketConstraintData::isValid() const
{
	// Nothing to validate
	return true;
}


int hkBallAndSocketConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_BALLANDSOCKET;
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
