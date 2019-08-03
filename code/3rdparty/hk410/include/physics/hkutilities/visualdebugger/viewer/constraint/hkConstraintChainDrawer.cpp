/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/constraint/chain/hkConstraintChainData.h>
#include <hkdynamics/constraint/chain/ballsocket/hkBallSocketChainData.h>
#include <hkdynamics/constraint/chain/stiffspring/hkStiffSpringChainData.h>
#include <hkdynamics/constraint/chain/powered/hkPoweredChainData.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstance.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkConstraintChainDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPrimitiveDrawer.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/hkDebugDisplayHandler.h>

#include <hkmath/linear/hkVector4Util.h>



void hkConstraintChainDrawer::drawConstraint(hkConstraintInstance* constraint, hkDebugDisplayHandler* displayHandler, int tag)
{
	hkConstraintChainInstance* instance = static_cast<hkConstraintChainInstance*>(constraint);
	hkConstraintChainData* data = instance->getData();

	m_primitiveDrawer.setDisplayHandler(displayHandler);

	int numConstraints = instance->m_chainedEntities.getSize() - 1;
	
	for (int i = 0; i < numConstraints; i++)
	{
		hkTransform refLocalToWorld;
		hkTransform attLocalToWorld;

		hkRigidBody *refBody = reinterpret_cast<hkRigidBody*>(instance->m_chainedEntities[i+1]);
		hkRigidBody *attBody = reinterpret_cast<hkRigidBody*>(instance->m_chainedEntities[i]);

		refLocalToWorld = refBody->getTransform();
		attLocalToWorld = attBody->getTransform();

		m_RB = refLocalToWorld.getRotation();
		m_RA = attLocalToWorld.getRotation();

		m_bodyBWPos = refLocalToWorld.getTranslation();
		m_bodyAWPos = attLocalToWorld.getTranslation();

		hkVector4 pivotInB;
		hkVector4 pivotInA;

		switch( data->getType() )
		{
		case hkConstraintData::CONSTRAINT_TYPE_STIFF_SPRING_CHAIN:
			pivotInB = static_cast<hkStiffSpringChainData*>(data)->m_infos[i].m_pivotInB;
			pivotInA = static_cast<hkStiffSpringChainData*>(data)->m_infos[i].m_pivotInA;
			break;
		case hkConstraintData::CONSTRAINT_TYPE_BALL_SOCKET_CHAIN:
			pivotInB = static_cast<hkBallSocketChainData*>(data)->m_infos[i].m_pivotInB;
			pivotInA = static_cast<hkBallSocketChainData*>(data)->m_infos[i].m_pivotInA;
			break;
		case hkConstraintData::CONSTRAINT_TYPE_POWERED_CHAIN:
			pivotInB = static_cast<hkPoweredChainData*>(data)->m_infos[i].m_pivotInB;
			pivotInA = static_cast<hkPoweredChainData*>(data)->m_infos[i].m_pivotInA;
			break;
		default:
			HK_ASSERT2(0xad6777dd, false, "Chain type not supproted by the drawer.");
		}

		m_bodyBWPivot.setTransformedPos(refLocalToWorld, pivotInB);
		m_bodyAWPivot.setTransformedPos(attLocalToWorld, pivotInA);

		// drawing
		
		drawPivots(tag);
		drawBodyFrames(tag);
		displayHandler->displayLine(m_bodyAWPivot, m_bodyBWPivot, hkColor::RED, tag);
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
