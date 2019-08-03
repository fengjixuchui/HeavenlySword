/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/visualdebugger/viewer/constraint/hkHingeDrawer.h>
#include <hkdynamics/constraint/bilateral/hinge/hkHingeConstraintData.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPrimitiveDrawer.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/hkDebugDisplayHandler.h>
#include <hkdynamics/entity/hkRigidBody.h>


////////////////////////////////////////////////////////////////////

void hkHingeDrawer::drawConstraint(hkConstraintInstance* constraint, hkDebugDisplayHandler* displayHandler, int tag)
{

	hkHingeConstraintData* hinge = static_cast<hkHingeConstraintData*>(constraint->getData());

	m_primitiveDrawer.setDisplayHandler(displayHandler);

	{
		hkTransform refLocalToWorld;
		hkTransform attLocalToWorld;
		updateCommonParameters(constraint, refLocalToWorld, attLocalToWorld);
		m_bodyBWPivot.setTransformedPos(refLocalToWorld, hinge->m_atoms.m_transforms.m_transformB.getTranslation());
		m_bodyAWPivot.setTransformedPos(attLocalToWorld, hinge->m_atoms.m_transforms.m_transformA.getTranslation());
	}

	drawBodyFrames(tag);

	drawPivots(tag);

	// not a complete representation of error
	// but some feedback
	// draw a red error line between the pivots
	displayHandler->displayLine(m_bodyAWPivot,m_bodyBWPivot,hkColor::RED, tag);

	// Draw the free axis of rotation
	{
		hkVector4 axisInWorld;
		axisInWorld.setRotatedDir(m_RB, hinge->m_atoms.m_transforms.m_transformB.getColumn(hkHingeConstraintData::Atoms::AXIS_AXLE));// m_basisB.m_axle);

		hkVector4 startAxis,endAxis;
		endAxis.setMul4(.5f * m_lineLength, axisInWorld);
		endAxis.add4(m_bodyBWPivot);
		startAxis.setMul4(-.5f * m_lineLength, axisInWorld);
		startAxis.add4(m_bodyBWPivot);
		displayHandler->displayLine(startAxis, endAxis, hkColor::rgbFromFloats(0.f, .5f, 1.f), tag);
	}
}

////////////////////////////////////////////////////////////////////

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
