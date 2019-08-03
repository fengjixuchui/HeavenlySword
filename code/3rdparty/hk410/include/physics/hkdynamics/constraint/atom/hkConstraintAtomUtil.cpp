/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>

#include <hkdynamics/constraint/atom/hkConstraintAtomUtil.h>



hkSimpleContactConstraintAtom* hkSimpleContactConstraintAtomUtil::allocateAtom(int numReservedContactPoints)
{
	int size = HK_NEXT_MULTIPLE_OF(16,sizeof(hkSimpleContactConstraintAtom)) + numReservedContactPoints * ( sizeof(hkContactPoint) + sizeof(hkContactPointProperties) );
	size = HK_NEXT_MULTIPLE_OF(16, size);

	hkSimpleContactConstraintAtom* atom =  reinterpret_cast<hkSimpleContactConstraintAtom*>( hkThreadMemory::getInstance().allocateChunk(size, HK_MEMORY_CLASS_DYNAMICS) );

	atom->m_info.init();
	atom->m_type = hkConstraintAtom::TYPE_CONTACT;
	atom->m_sizeOfAllAtoms = hkUint16(size);
	atom->m_numContactPoints = 0;
	atom->m_numReservedContactPoints = hkUint16(numReservedContactPoints);

	return atom;
}



void hkSimpleContactConstraintAtomUtil::copyContents(hkSimpleContactConstraintAtom* dst, const hkSimpleContactConstraintAtom* src)
{
	HK_ASSERT2(0xad76d88a, dst->m_numReservedContactPoints >= src->m_numContactPoints, "Destination atom does not have enough space.");

	dst->m_info = src->m_info;
	dst->m_numContactPoints = src->m_numContactPoints;

	{
		hkContactPoint*           dstCp  = dst->getContactPoints();
		hkContactPointProperties* dstCpp = dst->getContactPointProperties();
		hkContactPoint*           srcCp  = src->getContactPoints();
		hkContactPointProperties* srcCpp = src->getContactPointProperties();
		for (int i = 0; i < src->m_numContactPoints; i++)
		{
			*(dstCp++) = *(srcCp++);
			*(dstCpp++) = *(srcCpp++);
		}
	}
}



void hkSimpleContactConstraintAtomUtil::deallocateAtom(hkSimpleContactConstraintAtom* atom)
{
	hkThreadMemory::getInstance().deallocateChunk(atom, atom->m_sizeOfAllAtoms, HK_MEMORY_CLASS_DYNAMICS );
}



hkSimpleContactConstraintAtom* hkSimpleContactConstraintAtomUtil::expandOne(hkSimpleContactConstraintAtom* oldAtom_mightGetDeallocated)
{
	hkSimpleContactConstraintAtom* atom = oldAtom_mightGetDeallocated;

	if ( atom->m_numContactPoints >= atom->m_numReservedContactPoints )
	{
		atom = hkSimpleContactConstraintAtomUtil::allocateAtom( (atom->m_numContactPoints + 1) * 2);
		hkSimpleContactConstraintAtomUtil::copyContents(atom, oldAtom_mightGetDeallocated);
		hkSimpleContactConstraintAtomUtil::deallocateAtom(oldAtom_mightGetDeallocated);	
	}

	atom->m_numContactPoints++;

	// clear new property
	hkContactPointProperties* properties = &atom->getContactPointProperties()[atom->m_numContactPoints-1];
	hkString::memSet(properties, 0x0, sizeof(hkContactPointProperties));

	return atom;
}



void hkSimpleContactConstraintAtomUtil::removeAtAndCopy(hkSimpleContactConstraintAtom* atom, int index)
{
	HK_ASSERT2(0x724b10b5, index >= 0 && index < atom->m_numContactPoints , "Out of bound error.");
	atom->m_numContactPoints--;

	hkContactPoint*           cp  = &atom->getContactPoints()[index];
	hkContactPointProperties* cpp = &atom->getContactPointProperties()[index];
	for (int i = index; i < atom->m_numContactPoints; i++)
	{
		*(cp) = *(cp+1);
		*(cpp) = *(cpp+1);
		cp++;
		cpp++;
	}
}



hkSimpleContactConstraintAtom* hkSimpleContactConstraintAtomUtil::optimizeCapacity(hkSimpleContactConstraintAtom* oldAtom_mightGetDeallocated, int numFreeElemsLeft)
{
	hkSimpleContactConstraintAtom* atom = oldAtom_mightGetDeallocated;
	const int size = atom->m_numContactPoints + numFreeElemsLeft;
	if ( size*2 <= atom->m_numReservedContactPoints )
	{
		const int newSize = atom->m_numReservedContactPoints >> 1;
		atom = hkSimpleContactConstraintAtomUtil::allocateAtom(newSize);
		hkSimpleContactConstraintAtomUtil::copyContents(atom, oldAtom_mightGetDeallocated);
		hkSimpleContactConstraintAtomUtil::deallocateAtom(oldAtom_mightGetDeallocated);
	}

	return atom;
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
