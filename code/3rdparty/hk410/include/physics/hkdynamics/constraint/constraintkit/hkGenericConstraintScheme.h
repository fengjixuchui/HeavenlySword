/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_GENERIC_CONSTRAINT_SCHEME_H
#define HK_DYNAMICS2_GENERIC_CONSTRAINT_SCHEME_H

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/hkConstraintData.h>

class hkConstraintModifier;
class hkConstraintMotor;
class hkGenericConstraintDataCinfo;

extern const hkClass hkGenericConstraintDataSchemeClass;

class hkGenericConstraintDataScheme
{
	public:

		HK_DECLARE_REFLECTION();

		hkGenericConstraintDataScheme() {} 

		enum 
		{	
			e_endScheme = 0, 
			e_setPivotA, e_setPivotB, 
			e_setLinearDofA, e_setLinearDofB, e_setLinearDofW, e_constrainLinearW, e_constrainAllLinearW,
			e_setAngularBasisA, e_setAngularBasisB, e_setAngularBasisAidentity, e_setAngularBasisBidentity,
			e_constrainToAngularW, e_constrainAllAngularW,e_setAngularMotor, e_setLinearMotor,
			e_setLinearLimit, e_setAngularLimit, e_setConeLimit, e_setTwistLimit, e_setAngularFriction, e_setLinearFriction,
			e_setStrength, e_restoreStrengh, e_doConstraintModifier, e_doRhsModifier, 
			e_numCommands 
		};  

		struct hkConstraintInfo m_info;
	
		hkArray<hkVector4>				m_data;
		hkArray<int>					m_commands;
		hkArray<hkConstraintModifier*>	m_modifiers; //+nosave
		hkArray<hkConstraintMotor*>		m_motors;

	public:

		hkGenericConstraintDataScheme(hkFinishLoadedObjectFlag f) : m_data(f), m_commands(f), m_modifiers(f), m_motors(f) {}

};


#endif

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
