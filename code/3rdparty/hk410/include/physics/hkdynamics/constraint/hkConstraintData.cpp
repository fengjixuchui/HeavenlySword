/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/hkConstraintData.h>
#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>

void hkConstraintData::addInstance( hkConstraintInstance* constraint, hkConstraintRuntime* runtime, int sizeOfRuntime ) const 
{
	if ( runtime )
	{
		hkString::memSet( runtime, 0, sizeOfRuntime );
	}
}

hkSolverResults* hkConstraintData::getSolverResults( hkConstraintRuntime* runtime )
{
	return static_cast<hkSolverResults*>(runtime);
}


#define HK_SKIP_ATOM_BY_TYPE(atomType, atomClassName)\
{\
case hkConstraintAtom::atomType:\
{\
	const atomClassName* atom = static_cast<const atomClassName*>(currentAtom);\
	currentAtom = atom->next();\
}\
	break;\
}

#define HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE(atomType, atomClassName)\
{\
case hkConstraintAtom::atomType:\
	{\
		const atomClassName* atom = static_cast<const atomClassName*>(currentAtom);\
		atom->addToConstraintInfo(infoOut);\
		currentAtom = atom->next();\
	}\
	break;\
}

void hkConstraintData::getConstraintInfoUtil( const hkConstraintAtom* atoms, int sizeOfAllAtoms, hkConstraintData::ConstraintInfo& infoOut)
{
	infoOut.m_atoms = const_cast<hkConstraintAtom*>(atoms);
	infoOut.m_sizeOfAllAtoms = sizeOfAllAtoms;
	infoOut.clear();
	infoOut.addHeader();

	if (atoms->m_type == hkConstraintAtom::TYPE_CONTACT)
	{
		hkSimpleContactConstraintAtom* contactAtom = static_cast<hkSimpleContactConstraintAtom*>( const_cast<hkConstraintAtom*>(atoms) );
		contactAtom->addToConstraintInfo(infoOut);
	}
	else
	{
		const hkConstraintAtom* atomsEnd    = hkAddByteOffsetConst<const hkConstraintAtom>( atoms, sizeOfAllAtoms );
		for( const hkConstraintAtom* currentAtom = atoms; currentAtom < atomsEnd; )
		{
NEXT_SWITCH:
			switch(currentAtom->m_type)
			{
			case hkConstraintAtom::TYPE_INVALID: 
				{
					// If this is blank padding between atoms, then move to the next 16-byte aligned atom
					currentAtom = reinterpret_cast<hkConstraintAtom*>( HK_NEXT_MULTIPLE_OF(16, hkUlong(currentAtom)) );
					goto NEXT_SWITCH;
				}

				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_BALL_SOCKET,				hkBallSocketConstraintAtom      );
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_STIFF_SPRING,			hkStiffSpringConstraintAtom		);

				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_LIN,						hkLinConstraintAtom             );
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_LIN_SOFT,				hkLinSoftConstraintAtom         );
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_LIN_LIMIT,				hkLinLimitConstraintAtom        );
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_LIN_FRICTION,			hkLinFrictionConstraintAtom		);
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_LIN_MOTOR,				hkLinMotorConstraintAtom		);

				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_2D_ANG,					hk2dAngConstraintAtom			);

				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_ANG,						hkAngConstraintAtom				);
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_ANG_LIMIT,				hkAngLimitConstraintAtom        );
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_CONE_LIMIT,				hkConeLimitConstraintAtom		);
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_TWIST_LIMIT,				hkTwistLimitConstraintAtom		);
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_ANG_FRICTION,			hkAngFrictionConstraintAtom     );
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_ANG_MOTOR,				hkAngMotorConstraintAtom        );

				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_RAGDOLL_MOTOR,			hkRagdollMotorConstraintAtom	);
				HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_PULLEY,					hkPulleyConstraintAtom			);

				//
				//	modifiers
				//

				// : no next() method
				//HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_MODIFIER_SOFT_CONTACT,    hkSoftContactModifierConstraintAtom    );
				//HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_MODIFIER_MASS_CHANGER,    hkMassChangerModifierConstraintAtom    );
				//HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_MODIFIER_VISCOUS_SURFACE, hkViscousSurfaceModifierConstraintAtom );
				//HK_GET_CONSTRAINT_INFO_FROM_ATOM_BY_TYPE( TYPE_MODIFIER_MOVING_SURFACE,  hkMovingSurfaceModifierConstraintAtom  );

				HK_SKIP_ATOM_BY_TYPE( TYPE_SET_LOCAL_TRANSFORMS,	hkSetLocalTransformsConstraintAtom		);
				HK_SKIP_ATOM_BY_TYPE( TYPE_SET_LOCAL_ROTATIONS,		hkSetLocalRotationsConstraintAtom		);
				HK_SKIP_ATOM_BY_TYPE( TYPE_SET_LOCAL_TRANSLATIONS,	hkSetLocalTranslationsConstraintAtom	);

			case hkConstraintAtom::TYPE_BRIDGE:
			case hkConstraintAtom::TYPE_MODIFIER_SOFT_CONTACT:
			case hkConstraintAtom::TYPE_MODIFIER_MASS_CHANGER:
			case hkConstraintAtom::TYPE_MODIFIER_VISCOUS_SURFACE:
			case hkConstraintAtom::TYPE_MODIFIER_MOVING_SURFACE:
				{
					// this is assumed to be the last atom
					currentAtom = atomsEnd;
					HK_ASSERT2(0x74890f9d, false, "What do we do here ?");
					break;
				}

			default:
				HK_ASSERT2(0xad67de77,0,"Illegal atom.");
			}
		}
	}
}

#undef  HK_SKIP_ATOM_BY_TYPE
#undef  HK_GET_CONSTRAINT_INFOR_FROM_ATOM_BY_TYPE



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
