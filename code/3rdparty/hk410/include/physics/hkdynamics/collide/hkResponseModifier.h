/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_RESPONSE_MODIFIER_H
#define HK_DYNAMICS2_RESPONSE_MODIFIER_H

class hkDynamicsContactMgr;
class hkRigidBody;

/// This class contains a number of functions to allow altering the behavior of collision response
/// This class has a number of restrictions:
///    - Each modifier can only exist once. E.g. setting the mass scaling overrides previous mass scaling modifiers
///    - These modifiers change the constraint hierarchy, so you need access to the hkConstraintOwner
///      You can get an hkConstraintOwner in two ways:
///             -  by getting write access to the world (hkWorld::lock()) and simply using the non fixed island of one of the entities
///					(a hkSimulationIsland inherits from hkConstraintOwner)
///				-  if called from a collision callback simply use event.m_output.m_constraintOwner or event.m_constraintOwner
///		- Modifiers will not be serialized
class hkResponseModifier
{
	public:

			/// Scales the inverse mass of each object by the relevant scaling factor.
			/// Note: setting a factor to 0.0f makes that object "fixed" for the purpose of resolving this contact constraint.
			/// The constraint owner typically is the island of the involved objects. However if you call this function from a collision
			/// callback, you need to use event.m_collisionOutput.m_constraintOwner
		static void HK_CALL setInvMassScalingForContact( hkDynamicsContactMgr* manager, hkRigidBody* bodyA, hkRigidBody* bodyB, hkConstraintOwner& constraintOwner, hkReal factorA, hkReal factorB );

			/// Scales and clips the force applied by a contact. 
			///
			/// usedImpulseFraction scales the forces, e.g. using values less than 0.2f yields noticeably softer contact response
			/// and allows object to penetrate. They do get a softer/springy behavior.
			/// The parameter maxAcceleration clips the maximum forced used. This parameter does not introduce springyness.
			/// Try different combinations of the parameters to get the desired effect.
			/// 
			/// The constraint owner typically is the island of the involved objects. However if you call this function from a collision
			/// callback, you need to use event.m_collisionOutput.m_constraintOwner
		static void HK_CALL setImpulseScalingForContact( hkDynamicsContactMgr* manager, hkRigidBody* bodyA, hkRigidBody* bodyB, hkConstraintOwner& constraintOwner, hkReal usedImpulseFraction, hkReal maxAcceleration );

			/// sets the surfaceVelocity
		static void HK_CALL setSurfaceVelocity( hkDynamicsContactMgr* manager, hkRigidBody* body, hkConstraintOwner& constraintOwner, const hkVector4& velWorld );

			/// removes the surfaceVelocity
			/// NOTE: ONLY USE THIS FUNCTION AFTER CALLING setSurfaceVelocity
		static void HK_CALL clearSurfaceVelocity( hkDynamicsContactMgr* manager, hkConstraintOwner& constraintOwner, hkRigidBody* body );

			/// sets the surface to have low viscosity. Effectively disables static friction.
		static void HK_CALL setLowSurfaceViscosity( hkDynamicsContactMgr* manager, hkConstraintOwner& constraintOwner );

};

#endif		// HK_DYNAMICS2_RESPONSE_MODIFIER_H

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
