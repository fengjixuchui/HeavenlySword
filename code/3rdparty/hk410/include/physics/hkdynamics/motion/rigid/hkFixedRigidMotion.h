/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_FIXED_RIGID_MOTION
#define HK_DYNAMICS2_FIXED_RIGID_MOTION

#include <hkdynamics/motion/rigid/hkKeyframedRigidMotion.h>

extern const hkClass hkFixedRigidMotionClass;

/// This class is contained by the hkRigidBody class.
/// It should not be accessed directly by the user.
class hkFixedRigidMotion : public hkKeyframedRigidMotion
{
	public:

		HK_DECLARE_REFLECTION();

			/// Construct a keyframed rigid body motion object, given position and rotation.
			/// Fixed rigid bodies have infinite mass and so will not move
			/// during simulation.  They can never be repositioned as their
			/// bounding volume information will not be properly updated.
		hkFixedRigidMotion( const hkVector4& position, const hkQuaternion& rotation );

	public:

			// This method updates the saved motion with the dynamic properties of the current keyframed motion, namely
			// m_linearVelocity, m_angularVelocity, m_rotation, m_oldCenterOfMassInWorld, m_centerOfMassInWorld and m_localToWorld.
			// This should always be called before a "keyframed" dynamic body is "unkeyframed" again.
		virtual void getPositionAndVelocities( hkMotion* motionOut );

		virtual void setStepPosition( hkReal position, hkReal timestep );

	public:

		hkFixedRigidMotion( class hkFinishLoadedObjectFlag flag ) : hkKeyframedRigidMotion( flag ) {}
};

#endif //HK_DYNAMICS2_FIXED_RIGID_MOTION

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
