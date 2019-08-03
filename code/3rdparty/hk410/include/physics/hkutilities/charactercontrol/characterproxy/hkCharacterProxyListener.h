/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_CHARACTER_PROXY_LISTENER_H
#define HK_CHARACTER_PROXY_LISTENER_H

#include <hkbase/hkBase.h>

class hkContactPoint;
class hkRigidBody;
class hkCharacterProxy;
struct hkRootCdPoint;
struct hkSimplexSolverInput;

/// A character interaction event is passed to a listeners objectInteractionCallback() when the character proxy hits another object
/// This even contains information that allows the user to recalculate or override the impulse that will be applied to the object and character
struct hkCharacterObjectInteractionEvent
{
		/// The position of the contact with the object in world space
	hkVector4	m_position;

		/// The normal at the point of contact. Note that the .w value is the distance between the surfaces.
	hkVector4	m_normal;

		/// The magnitude of the impulse that will be applied if not overridden
		/// This can be useful when playing sound or calculating damage
	hkReal		m_objectImpulse;

		/// Time slice information - passed on from hkCharacterProxy::integrate
	hkReal		m_timestep;

		/// The magnitude of the relative velocity along the normal
	hkReal		m_projectedVelocity;

		/// Mass information for the object (projected along the normal)
	hkReal		m_objectMassInv;

		// A pointer to the body that was hit
		/// If the character did not hit a body this will be HK_NULL
	hkRigidBody* m_body;
};

/// The result structure is initialized and then passed to a listeners objectInteractionCallback() the user can choose to change these
/// values and effect how the character will interact with objects
struct hkCharacterObjectInteractionResult
{
		/// The impulse that will be applied to object
	hkVector4	m_objectImpulse;

		/// The point in world space where the object impulse will be applied
	hkVector4	m_impulsePosition;

};

/// Instances of this listener can be registered with a hkCharacterProxy, and are used for catching contact points, 
/// for updating the manifold before it is passed to the simplex solver and for handling how the character interacts with 
/// dynamic objects in the scene.
class hkCharacterProxyListener
{
	public:

			///
		virtual ~hkCharacterProxyListener() { }

			/// Called before the simple solver is called.
			/// The manifold is passed so the user can retrieve body information if necessary
			/// This allows the user to override of add any information stored in the plane equations passed to the simplex solver.
		virtual void processConstraintsCallback( const hkArray<hkRootCdPoint>& manifold, hkSimplexSolverInput& input ) {}

			/// Called when a new contact point is taken from the results of the linear cast and added to the current manifold
		virtual void contactPointAddedCallback(const hkRootCdPoint& point) {}

			/// Called when a new contact point is discarded from the current manifold
		virtual void contactPointRemovedCallback(const hkRootCdPoint& point) {}

			/// Called when the character interacts with another character
		virtual void characterInteractionCallback(hkCharacterProxy* proxy, hkCharacterProxy* otherProxy, const hkContactPoint& contact) {}

			/// Called when the character interacts with another (non fixed or keyframed) rigid body.
		virtual void objectInteractionCallback(hkCharacterProxy* proxy, const hkCharacterObjectInteractionEvent& input, hkCharacterObjectInteractionResult& output ) {}

};

#endif //HK_CHARACTER_PROXY_LISTENER_H

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
