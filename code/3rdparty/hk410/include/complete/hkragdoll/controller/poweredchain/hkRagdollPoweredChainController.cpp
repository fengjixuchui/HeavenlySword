/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkragdoll/hkRagdoll.h>

#include <hkragdoll/controller/poweredchain/hkRagdollPoweredChainController.h>
#include <hkragdoll/instance/hkRagdollInstance.h>
#include <hkragdoll/utils/hkRagdollUtils.h>

#include <hkdynamics/constraint/chain/powered/hkPoweredChainData.h>

#include <hkutilities/constraint/hkPoweredChainMapper.h>

void hkRagdollPoweredChainController::driveToPose ( hkRagdollInstance* ragdoll, hkPoweredChainMapper* mapper, const hkQsTransform* desiredPose )
{
	const int numBones = ragdoll->getNumBones();

	// We're using the implicit constraint ordering form hkRagdoll used for the mapper as well.
	for (int b = 1; b < numBones; b++)
	{
		const int linkIndex = b-1;
		const hkQsTransform& RBfromRA = desiredPose[b];
		mapper->setTargetOrientation(linkIndex, RBfromRA.getRotation());
	}
}


void hkRagdollPoweredChainController::startMotors ( hkRagdollInstance* ragdoll, hkPoweredChainMapper* mapper )
{
	//to be implemented
	HK_WARN_ONCE(0xad67888e, "Function not implemneted.");
}

void hkRagdollPoweredChainController::stopMotors ( hkRagdollInstance* ragdoll, hkPoweredChainMapper* mapper )
{
	//to be implemented
	HK_WARN_ONCE(0xad67888e, "Function not implemneted.");

}


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
