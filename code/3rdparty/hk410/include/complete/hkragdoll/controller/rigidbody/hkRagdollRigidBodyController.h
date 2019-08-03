/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_RAGDOLL_RIGIDBODY_CONTROLLER_H
#define HK_RAGDOLL_RIGIDBODY_CONTROLLER_H

#include <hkmath/hkMath.h>
#include <hkragdoll/controller/rigidbody/hkKeyFrameHierarchyUtility.h>

class hkRagdollInstance;

	/// This controller is useful when driving a set of constrained rigid bodies to a set of keyframes.
	/// It is often used to drive a ragdoll to a given animation pose. It is designed to work in model 
	/// or world space and is particularly useful for effects like hit reaction or following animation precisely.
	/// Note that this class replaces the hkRagdollBoneController.
class hkRagdollRigidBodyController 
{

	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ANIMATION, hkRagdollRigidBodyController);

			/// Constructor defined bodies to operate on and their hierarchy
			/// The constructor adds reference to the bodies.
			/// \param numBodies the number of bodies in the system
			/// \param bodies a pointer to the array of bodies
			/// \param parentIndices usually derived by looking at how the bodies are constrained; see hkRagdollUtils::constructSkeletonForRagdoll
		hkRagdollRigidBodyController ( int numBodies, hkRigidBody*const* bodies, hkInt16* parentIndices );

			/// A convenient constructor using a ragdoll instance
			/// The constructor adds references to the bodies of the ragdoll.
		hkRagdollRigidBodyController ( hkRagdollInstance* ragdoll );

			/// The constructor removes the references from the bodies
		~hkRagdollRigidBodyController();

			/// Applies the required velocities in order to drive the rigid bodies of this ragdoll to the desired pose.
			/// \param output see hkKeyFrameHierarchyUtility for parameter details
		void driveToPose ( hkReal deltaTime, const hkQsTransform* poseLocalSpace, const hkQsTransform& worldFromModel, hkKeyFrameHierarchyUtility::Output* stressOut );

			/// The controller uses values from the current state and previous state of the bodies in its calculation
			/// The previous state is held in the m_internalData member.
			/// Call this to initialize the internal data with the current state of the bodies.
			/// e.g. If you are using this controller intermittently call reintialize() once before you start using the controller
		void reinitialize();

			/// The input data for the controller
			/// See hkKeyFrameHierarchyUtility for parameter details
			/// On initialisation only a single element is allocated. 
			/// Resize this and adjust the array below for per bone parameters.
		hkArray<hkKeyFrameHierarchyUtility::ControlData> m_controlDataPalette;

			/// An array of indices into the palette above
			/// This array should be the same length as the number of bodies in the ragdoll
			/// If it is not then all bodes use the first element in the palette above.
		hkArray<int> m_bodyIndexToPaletteIndex;

	private:

		void initialize( int numBodies, hkRigidBody*const* bodies, hkInt16* parentIndices );

			// Information about the rigid bodies and their hierarchy
		hkKeyFrameHierarchyUtility::BodyData m_bodyData;

		hkArray<hkKeyFrameHierarchyUtility::WorkElem> m_internalData;
};


#endif // HK_RAGDOLL_RIGIDBODY_CONTROLLER_H

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
