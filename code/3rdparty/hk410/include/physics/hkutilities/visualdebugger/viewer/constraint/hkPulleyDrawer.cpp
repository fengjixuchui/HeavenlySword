/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/constraint/pulley/hkPulleyConstraintData.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPulleyDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPrimitiveDrawer.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/hkDebugDisplayHandler.h>

#include <hkmath/linear/hkVector4Util.h>



void hkPulleyDrawer::drawConstraint(hkConstraintInstance* constraint, hkDebugDisplayHandler* displayHandler, int tag)
{
	hkPulleyConstraintData* pulley = static_cast<hkPulleyConstraintData*>(constraint->getData());

	m_primitiveDrawer.setDisplayHandler(displayHandler);

	{
		hkTransform refLocalToWorld;
		hkTransform attLocalToWorld;
		updateCommonParameters(constraint, refLocalToWorld, attLocalToWorld);
		m_bodyBWPivot.setTransformedPos(refLocalToWorld, pulley->m_atoms.m_translations.m_translationB);
		m_bodyAWPivot.setTransformedPos(attLocalToWorld, pulley->m_atoms.m_translations.m_translationA);
	}

	drawPivots(tag);

	drawBodyFrames(tag);

	const hkPulleyConstraintAtom& pulleyAtom = pulley->m_atoms.m_pulley;

	displayHandler->displayLine(pulleyAtom.m_fixedPivotAinWorld, pulleyAtom.m_fixedPivotBinWorld, hkColor::GREY75, tag);
	displayHandler->displayLine(m_bodyAWPivot, pulleyAtom.m_fixedPivotAinWorld, hkColor::WHITE, tag);

	int numLines = int(hkMath::max2( pulleyAtom.m_leverageOnBodyB, 1.0f ));
	hkVector4 shift; shift.set(0.07f, 0.0f, 0.0f);
	shift.setRotatedDir(static_cast<hkRigidBody*>(constraint->getEntityB())->getTransform().getRotation(), shift);

	hkVector4 base; base.setMul4( (numLines-1) * -0.5f, shift );

	for (int i = 0; i < numLines; i++)
	{
		hkVector4 thisShift; thisShift.setAddMul4( base, shift, 1.0f * i );
		hkVector4 a; a.setAdd4(m_bodyBWPivot, thisShift);
		hkVector4 b; b.setAdd4(pulleyAtom.m_fixedPivotBinWorld, thisShift);
		displayHandler->displayLine(a, b, hkColor::WHITE, tag);
	}

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
