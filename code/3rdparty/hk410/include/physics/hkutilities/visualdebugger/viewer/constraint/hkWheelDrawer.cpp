/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkutilities/visualdebugger/viewer/constraint/hkWheelDrawer.h>
#include <hkdynamics/constraint/bilateral/wheel/hkWheelConstraintData.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPrimitiveDrawer.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/hkDebugDisplayHandler.h>
#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>

///////////////////////////////////////////////////////////////////

void hkWheelDrawer::drawConstraint(hkConstraintInstance* constraint, hkDebugDisplayHandler* displayHandler, int tag)
{

	hkWheelConstraintData* wheel = static_cast<hkWheelConstraintData*>(constraint->getData());

	m_primitiveDrawer.setDisplayHandler(displayHandler);

	{
		hkTransform refLocalToWorld;
		hkTransform attLocalToWorld;
		updateCommonParameters(constraint, refLocalToWorld, attLocalToWorld);
		m_bodyBWPivot.setTransformedPos(refLocalToWorld, wheel->m_atoms.m_suspensionBase.m_transformB.getTranslation());
		m_bodyAWPivot.setTransformedPos(attLocalToWorld, wheel->m_atoms.m_suspensionBase.m_transformA.getTranslation());
	}

	drawBodyFrames(tag);
	
	drawPivots(tag);

	// draws red line between pivot on A and B in world space
	displayHandler->displayLine(m_bodyAWPivot, m_bodyBWPivot, hkColor::RED, tag);


	hkVector4 suspensionAxis;
	suspensionAxis.setRotatedDir(m_RB, wheel->m_atoms.m_suspensionBase.m_transformB.getColumn(hkWheelConstraintData::Atoms::AXIS_SUSPENSION));// m_tempWheelAtom.m_basisB.m_suspensionAxis); 

	// draw suspension axis
	{
		hkVector4 p1;
		hkVector4 p2;
		p1.setMul4(-.5f * m_lineLength, suspensionAxis);
		p1.add4(m_bodyBWPivot);
		p2.setMul4(.5f * m_lineLength, suspensionAxis);
		p2.add4(m_bodyBWPivot);
		displayHandler->displayLine(p1, p2, hkColor::GREEN, tag);
	}

	// draw suspension limits
	{
		hkVector4 p1;
		hkVector4 start;
		hkVector4 end;
		
		hkReal suspensionMin = wheel->m_atoms.m_lin0Limit.m_min;
		hkReal suspensionMax = wheel->m_atoms.m_lin0Limit.m_max;
		
		hkVector4 perpSuspension;
		perpSuspension.setRotatedDir(m_RB, wheel->m_atoms.m_suspensionBase.m_transformB.getColumn(hkWheelConstraintData::Atoms::AXIS_PERP_SUSPENSION));// m_tempWheelAtom.m_basisB.m_perpToSuspensionAxis);

		p1.setMul4(suspensionMax, suspensionAxis);
		p1.add4(m_bodyBWPivot);

		start.setMul4(.1f * m_lineLength, perpSuspension);
		start.add4(p1);

		end.setMul4(-.1f * m_lineLength, perpSuspension);
		end.add4(p1);

		displayHandler->displayLine(start,end, hkColor::WHITE, tag);

		p1.setMul4(suspensionMin, suspensionAxis);
		p1.add4(m_bodyBWPivot);

		start.setMul4(.1f * m_lineLength, perpSuspension);
		start.add4(p1);

		end.setMul4(-.1f * m_lineLength, perpSuspension);
		end.add4(p1);

		displayHandler->displayLine(start, end, hkColor::WHITE, tag);
	}
	
	// draw steering axis
	{
		hkVector4 p1;
		hkVector4 p2;
		hkVector4 steeringAxis;
		steeringAxis.setRotatedDir(m_RB, wheel->m_atoms.m_steeringBase.m_rotationB.getColumn(hkWheelConstraintData::Atoms::AXIS_STEERING));//  m_tempWheelAtom.m_basisB.m_steeringAxis);
		p1.setMul4(-.5f * m_lineLength, steeringAxis);
		p1.add4(m_bodyBWPivot);
		p2.setMul4(.5f * m_lineLength, steeringAxis);
		p2.add4(m_bodyBWPivot);
		displayHandler->displayLine(p1, p2, hkColor::YELLOW, tag);
	}



	// draw axle
	{
		hkVector4 p1;
		hkVector4 p2;
		hkVector4 axle;
		axle.setRotatedDir(m_RA, wheel->m_atoms.m_steeringBase.m_rotationA.getColumn(hkWheelConstraintData::Atoms::AXIS_AXLE)); //m_atoms.m_steeringBase.m_rotationA.getColumn(hkWheelConstraintData::Atoms::AXIS_AXLE));

		p1.setMul4(-.5f * m_lineLength, axle);
		p1.add4(m_bodyAWPivot);

		p2.setMul4(.5f * m_lineLength, axle);
		p2.add4(m_bodyAWPivot);

		displayHandler->displayLine(p1, p2, hkColor::BLUE, tag);
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
