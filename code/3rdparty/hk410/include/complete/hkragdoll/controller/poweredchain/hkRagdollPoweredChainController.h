/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_RAGDOLL_POWERED_CHAIN_CONTROLLER_H
#define INC_RAGDOLL_POWERED_CHAIN_CONTROLLER_H

class hkRagdollInstance;
class hkPoweredChainMapper;

/// These utility methods are useful when driving a ragdoll to a given pose.
/// They require you to have set up a ragdoll with powered constraints.
/// They are designed to work in local space and are particularly useful for effects like
/// driving muscles during death or driving the ragdoll to a specific pose to get up from.
/// If you require a controller wich works in model space for effects like hit reaction 
/// then you should use the hkRagdollRigidBodyController instead.
/// Note that this class replaces the hkRagdollJointController.
class hkRagdollPoweredChainController
{
	public:

			/// Calculates and applies the desired driving poses to the motors for the associated powered hinges / powered ragdoll constraints.
		static void HK_CALL driveToPose( hkRagdollInstance* ragdoll, hkPoweredChainMapper* mapper, const hkQsTransform* poseLocalSpace );

			/// Enables the motors in all the powered constraints associated with the joints of this ragdoll instance.
		static void HK_CALL startMotors( hkRagdollInstance* ragdoll, hkPoweredChainMapper* mapper );

			/// Disables the motors of the powered constraints associated with the joints of this ragdoll instance.
		static void HK_CALL stopMotors( hkRagdollInstance* ragdoll, hkPoweredChainMapper* mapper );

};

#endif //INC_RAGDOLL_POWERED_CHAIN_CONTROLLER_H

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
