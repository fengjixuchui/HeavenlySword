/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
 
#ifndef HK_CONSTRAINTSOLVER2_BUILD_JACOBIAN_FROM_ATOM_H
#define HK_CONSTRAINTSOLVER2_BUILD_JACOBIAN_FROM_ATOM_H

#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>
#include <hkconstraintsolver/constraint/contact/hkSimpleContactConstraintInfo.h>

extern "C"
{
	void HK_CALL hkSolverBuildJacobianFromAtomsNotContact( const struct hkConstraintAtom* atoms, int sizeOfAllAtoms, const class hkConstraintQueryIn &in, class hkConstraintQueryOut &out);

	HK_FORCE_INLINE void HK_CALL hkSolverBuildJacobianFromAtoms( const struct hkConstraintAtom* atoms, int sizeOfAllAtoms, const class hkConstraintQueryIn &in, class hkConstraintQueryOut &out)
	{
		if (atoms->m_type == hkConstraintAtom::TYPE_CONTACT)
		{
			struct hkSimpleContactConstraintAtom* contactAtom = static_cast<hkSimpleContactConstraintAtom*>( const_cast<hkConstraintAtom*>(atoms) );
			hkSimpleContactConstraintDataBuildJacobian( contactAtom, in, out );
		}
		else
		{
			hkSolverBuildJacobianFromAtomsNotContact( atoms, sizeOfAllAtoms, in, out );
		}
	}
} 


#endif // HK_CONSTRAINTSOLVER2_BUILD_JACOBIAN_FROM_ATOM_H

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
