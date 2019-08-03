/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/malleable/hkMalleableConstraintData.h>
#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/constraint/atom/hkBuildJacobianFromAtoms.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkMalleableConstraintData);

hkMalleableConstraintData::hkMalleableConstraintData(hkConstraintData* constraintData) 
: m_constraintData(constraintData),
  m_strength(0.01f)
{
	m_constraintData->addReference();   
	m_atoms.m_bridgeAtom.init( this );
}
	

hkMalleableConstraintData::hkMalleableConstraintData(hkFinishLoadedObjectFlag f) : hkConstraintData(f), m_atoms(f)
{
	m_atoms.m_bridgeAtom.init( this );
}

hkMalleableConstraintData::~hkMalleableConstraintData()
{
	m_constraintData->removeReference(); 	
}


void hkMalleableConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& info ) const
{
	// this must be called first otherwise clear()
	// gets called inside and erases the effect of previous
	// additions
	m_constraintData->getConstraintInfo(info);

	info.m_atoms = const_cast<hkConstraintAtom*>(m_atoms.getAtoms());
	info.m_sizeOfAllAtoms = m_atoms.getSizeOfAllAtoms();
}


void hkMalleableConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	m_constraintData->getRuntimeInfo( wantRuntime, infoOut );
}


void hkMalleableConstraintData::buildJacobian( const hkConstraintQueryIn &in, hkConstraintQueryOut &out )
{
	hkConstraintQueryIn min = in;

	min.m_virtMassFactor = min.m_virtMassFactor * m_strength;

	hkConstraintData::ConstraintInfo info; m_constraintData->getConstraintInfo(info);
	hkSolverBuildJacobianFromAtoms(	info.m_atoms, info.m_sizeOfAllAtoms, min, out );
}


void hkMalleableConstraintData::setStrength(const hkReal tau)
{
	m_strength = tau;
}




hkReal hkMalleableConstraintData::getStrength() const
{
	return m_strength;
}




hkBool hkMalleableConstraintData::isValid() const
{
	return m_constraintData->isValid();
}


int hkMalleableConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_MALLEABLE;
}


hkConstraintData* hkMalleableConstraintData::getWrappedConstraintData()
{
	return m_constraintData;
}

const hkConstraintData* hkMalleableConstraintData::getWrappedConstraintData() const
{
	return m_constraintData;
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
