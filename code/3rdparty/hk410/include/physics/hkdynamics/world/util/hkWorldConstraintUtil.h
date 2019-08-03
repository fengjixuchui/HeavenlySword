/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_WORLD_CONSTRAINT_UTIL_H
#define HK_DYNAMICS2_WORLD_CONSTRAINT_UTIL_H

#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>

class hkConstraintInstance;
class hkEntity;
class hkWorld;
struct hkModifierConstraintAtom;

class hkWorldConstraintUtil
{
	public:
		static void                      HK_CALL addConstraint( hkWorld* world, hkConstraintInstance* constraint );

			/// removes the constraint and deletes all attached modifiers
		static void                      HK_CALL removeConstraint( hkConstraintInstance* constraint );
		static hkConstraintInstance*     HK_CALL getConstraint( const hkEntity* entityA, const hkEntity* entityB);

		static void                      HK_CALL addModifier             ( hkConstraintInstance* instance, hkConstraintOwner& constraintOwner, hkModifierConstraintAtom* s );
		static void                      HK_CALL removeModifier          ( hkConstraintInstance* instance, hkConstraintOwner& constraintOwner, hkConstraintAtom::AtomType type );
		static hkModifierConstraintAtom* HK_CALL findModifier            ( hkConstraintInstance* instance, hkConstraintAtom::AtomType type );
		static hkModifierConstraintAtom* HK_CALL findLastModifier        ( hkConstraintInstance* instance );
		static void                      HK_CALL updateFatherOfMovedAtom ( hkConstraintInstance* instance, const hkConstraintAtom* oldAtom, const hkConstraintAtom* updatedAtom, int updatedSizeOfAtom );
		HK_FORCE_INLINE static hkConstraintAtom* HK_CALL getTerminalAtom (const hkConstraintInternal* cInternal);
};

#include <hkdynamics/world/util/hkWorldConstraintUtil.inl>

#endif // HK_DYNAMICS2_WORLD_CONSTRAINT_UTIL_H

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
