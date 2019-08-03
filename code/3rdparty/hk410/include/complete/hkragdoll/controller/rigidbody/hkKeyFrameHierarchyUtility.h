/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_KEYFRAME_HIERARCHY_UTILITY_H
#define HK_KEYFRAME_HIERARCHY_UTILITY_H

#include <hkmath/hkMath.h>

class hkRigidBody;

	/// The hkKeyFrameHierarchyUtility implements a controller that applies linear and angular velocities to the 
	/// rigid bodies in a constraint hierarchy e.g. a rag doll.
	/// This controller is useful when you are attempting to drive the system to a desired pose / configuration.
class hkKeyFrameHierarchyUtility 
{

	public:

			/// A structure containing the control data for the utility
			/// The controller works by first matching acceleration, then velocity and finally position.
			/// At each stage the controller examines the difference between the keyframe and the body.
			/// This difference is scaled by the gain and applied it to the body by changing its velocity.
			/// Differences in position are clamped before being applied.
			/// When choosing values for the gains it is common to have higher values in the higher order controllers
			/// e.g. acceleration gain > velocity gain > position gain
			///
			/// Once the three controllers have been applied we perform a final pass which 
			/// snaps the bodies to the keyframes when they are very close. This controller called the snap controller
			/// works similarly to the position controller. It usually has a higher gain than the position controller.
			/// It differs slightly in that its strength is proportional to the error.
		struct ControlData
		{
				/// This parameter blends the desired target for a bone between model space (0.0) or local space(1.0)
				/// Usually the controller will be much stiffer and more stable when driving to model space.
				/// However local space targeting can look more natural. 
				/// It is similar to the deprecated bone controller hierarchyGain parameter
			hkReal m_hierarchyGain; 

				/// This gain dampens the velocities of the bodies. The current velocity of the body is 
				/// scaled by this parameter on every frame before the controller is applied. It is
				/// applied every step and is generally more aggressive than standard linear or angular damping.
				/// A value of 0 means no damping.
			hkReal m_velocityDamping;

				/// This parameter controls the proportion of the difference in acceleration that is 
				/// applied to the bodies. This parameter dampens the effects of the velocity control.
			hkReal m_accelerationGain;

				/// This parameter controls the proportion of the difference in velocity that is 
				/// applied to the bodies. This parameter dampens the effects of the position control.
			hkReal m_velocityGain;

				/// This parameter controls the proportion of the difference in position that is 
				/// applied to the bodies. This parameter has the most immediate effect. High gain 
				/// values make the controller very stiff. Once the controller is too stiff
				/// it will tend to overshoot. The velocity gain can help control this.
			hkReal m_positionGain;	

				/// The position difference is scaled by the inverse delta time to compute a 
				/// velocity to be applied to the rigid body. The velocity is first clamped to this 
				/// limit before it is applied.
			hkReal m_positionMaxLinearVelocity;

				/// The orientation difference is scaled by the inverse delta time to compute an angular 
				/// velocity to be applied to the rigid body. The velocity is first clamped to this 
				/// limit before it is applied.
			hkReal m_positionMaxAngularVelocity;

				// The snap gain works like position gain.
			hkReal m_snapGain; 

				/// This limit works like m_positionMaxLinearVelocity
			hkReal m_snapMaxLinearVelocity;

				/// This limit works like m_positionMaxAngularVelocity
			hkReal m_snapMaxAngularVelocity;

				/// These sets the max distance for the snap gain to work at full strength
				/// The strength of the controller peaks at this distance
			hkReal m_snapMaxLinearDistance;

				/// These sets the max distance for the snap gain to work at full strength
				/// The strength of the controller peaks at this distance
			hkReal m_snapMaxAngularDistance;

				// These useful defaults are appropriate in most circumstances
			ControlData() :
				m_hierarchyGain(0.17f),
				m_velocityDamping(0.0f),
				m_accelerationGain(1.0f),
				m_velocityGain(0.6f),
				m_positionGain(0.05f),
				m_positionMaxLinearVelocity(1.4f),
				m_positionMaxAngularVelocity(1.8f),
				m_snapGain(0.1f),
				m_snapMaxLinearVelocity(0.3f),
				m_snapMaxAngularVelocity(0.3f),
				m_snapMaxLinearDistance(0.03f),
				m_snapMaxAngularDistance(0.1f) {}

		};

		/// A structure holding data which used by this utility and has to be persistent
		struct WorkElem
		{
			hkVector4    m_prevPosition;
			hkQuaternion m_prevRotation;
			hkVector4    m_prevLinearVelocity;
			hkVector4    m_prevAngularVelocity;
		};

		/// A structure holding the data representing the pose to drive to and the persistent data
		struct KeyFrameData
		{
			/// The transformation from the ragdoll root to world space
			hkQsTransform			m_worldFromRoot;

			/// The desired pose in local space, one transform for each rigid body
			const hkQsTransform *	m_desiredPoseLocalSpace;

			/// Internal array of work data. Has to be persistent and should be initialized with the initialize() method 
			struct WorkElem*				m_internalReferencePose;
		};

		/// A structure holding the rigid bodies and information about how they are linked / parented
		/// This parent indices are usually derived by looking at how the bodies are constrained; see hkRagdollUtils::constructSkeletonForRagdoll
		struct BodyData
		{
			/// The number of rigid bodies
			int				     m_numRigidBodies;

			/// An array of rigid bodies. The velocities of these rigid bodies will be modified to match the input pose
			hkRigidBody*const*   m_rigidBodies;

			/// The hierarchy of rigid bodies. A rigid body which has no parent has an index of-1
			const hkInt16*       m_parentIndices;

			/// An index one per body indicating which control data to use for this body
			/// If this is HK_NULL all bodies use the first element of the control data array
			int*				 m_controlDataIndices;

			BodyData() : m_controlDataIndices(HK_NULL) {}
		};

			/// The output
		struct Output
		{
				/// This output parameter is related to velocity and is calculated
				/// as a byproduct of computing the snap gain. It is a reasonable
				/// measure of the error or stress in the controller
			hkReal m_stressSquared;
		};

			/// Initialize the internal WorkElem array
		static void HK_CALL initialize(const BodyData& bodies, WorkElem* internalReferencePose);

			/// applies the required velocities in order to drive the rigid bodies of this
			/// ragdoll to the desired pose. 
			/// If the output array can be HK_NULL if you wish to ignore the output
		static void HK_CALL applyKeyFrame( hkReal m_deltaTime, const KeyFrameData& pose, const BodyData& body, const ControlData* controlPalette, Output* outputArray = HK_NULL );
};


#endif // HK_KEYFRAME_HIERARCHY_UTILITY_H

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
