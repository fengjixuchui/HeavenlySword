/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_MOUSE_SPRING_ACTION_H
#define HK_MOUSE_SPRING_ACTION_H

#include <hkdynamics/action/hkUnaryAction.h>

class hkRigidBody;
extern const hkClass hkMouseSpringActionClass;

	/// An action that allows users to drag a rigid body using the mouse. The mouse spring applies
	/// an impulse to the rigid body to move it around. You can see this action being
	/// used in the visual debugger and the Havok 2 demo framework.
class hkMouseSpringAction: public hkUnaryAction
{
	public:

		HK_DECLARE_REFLECTION();

		hkMouseSpringAction( hkRigidBody* body = HK_NULL );

			/// Creates a new hkMouseSpringAction for the specified rigid body.
			/// The mouse spring is created between the specified point in the rigid body's space and the specified
			/// mouse position in world space.
		hkMouseSpringAction(
			const hkVector4& positionInRbLocal, const hkVector4& mousePositionInWorld,
			const hkReal springDamping, const hkReal springElasticity,
			const hkReal objectDamping, hkRigidBody* rb );

		void entityRemovedCallback(hkEntity* entity);

			///	Sets the mouse position in world space
		void setMousePosition( const hkVector4& mousePositionInWorld );

			///	Sets the maximum relative force applied to the body
		void setMaxRelativeForce(hkReal newMax);

	public:

			/// Mouse spring end-point in the rigid body's local space
		hkVector4 m_positionInRbLocal;

			/// Mouse position in world space
		hkVector4 m_mousePositionInWorld;

			/// Damping for the mouse spring
		hkReal	m_springDamping;

			/// Elasticity for the mouse spring
		hkReal	m_springElasticity;

			/// Allows you to clip the impulse applied to the body (this gets multiplied by the mass of the object)
		hkReal  m_maxRelativeForce;

			/// Damping applied to the linear and angular velocities of the picked body
		hkReal	m_objectDamping;

		void applyAction( const hkStepInfo& stepInfo );

			/// hkAction clone interface.
		virtual hkAction* clone( const hkArray<hkEntity*>& newEntities, const hkArray<hkPhantom*>& newPhantoms ) const;

	public:

		hkMouseSpringAction( class hkFinishLoadedObjectFlag flag ) {}
};

#endif // HK_MOUSE_SPRING_H


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
