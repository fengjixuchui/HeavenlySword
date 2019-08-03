/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkragdoll/hkRagdoll.h>
#include <hkragdoll/controller/rigidbody/hkRagdollRigidBodyController.h>
#include <hkragdoll/instance/hkRagdollInstance.h>

void hkRagdollRigidBodyController::initialize( int numBodies, hkRigidBody*const* bodies, hkInt16* parentIndices )
{
	m_bodyData.m_numRigidBodies = numBodies;
	m_bodyData.m_rigidBodies = bodies;
	m_bodyData.m_parentIndices = parentIndices;

	for (int i=0 ;i < m_bodyData.m_numRigidBodies; i++)
	{
		m_bodyData.m_rigidBodies[i]->addReference();
	}

	m_internalData.setSize( numBodies );
	hkKeyFrameHierarchyUtility::initialize( m_bodyData, m_internalData.begin() );
}

hkRagdollRigidBodyController::hkRagdollRigidBodyController ( int numBodies, hkRigidBody*const* bodies, hkInt16* parentIndices )
{
	m_controlDataPalette.setSize( 1 );
	m_controlDataPalette[0] = hkKeyFrameHierarchyUtility::ControlData();
	initialize( numBodies, bodies, parentIndices );
}

hkRagdollRigidBodyController::hkRagdollRigidBodyController ( hkRagdollInstance* ragdoll )
{
	m_controlDataPalette.setSize( 1 );
	m_controlDataPalette[0] = hkKeyFrameHierarchyUtility::ControlData();
	initialize( ragdoll->getRigidBodyArray().getSize(), ragdoll->getRigidBodyArray().begin(), ragdoll->getSkeleton()->m_parentIndices );
}

hkRagdollRigidBodyController::~hkRagdollRigidBodyController()
{
	for (int i=0 ;i < m_bodyData.m_numRigidBodies; i++)
	{
		m_bodyData.m_rigidBodies[i]->removeReference();
	}
}

void hkRagdollRigidBodyController::reinitialize()
{
	hkKeyFrameHierarchyUtility::initialize( m_bodyData, m_internalData.begin() );
}

void hkRagdollRigidBodyController::driveToPose ( hkReal deltaTime, const hkQsTransform* poseLocal, const hkQsTransform& worldFromModel, hkKeyFrameHierarchyUtility::Output* stressOut )
{
	hkKeyFrameHierarchyUtility::KeyFrameData kData;
	kData.m_internalReferencePose = m_internalData.begin();
	kData.m_worldFromRoot = worldFromModel;
	kData.m_desiredPoseLocalSpace = poseLocal;

	m_bodyData.m_controlDataIndices = ( m_bodyIndexToPaletteIndex.getSize() == m_bodyData.m_numRigidBodies ) ? m_bodyIndexToPaletteIndex.begin() : HK_NULL;

	hkKeyFrameHierarchyUtility::applyKeyFrame( deltaTime, kData, m_bodyData, m_controlDataPalette.begin(), stressOut );
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
