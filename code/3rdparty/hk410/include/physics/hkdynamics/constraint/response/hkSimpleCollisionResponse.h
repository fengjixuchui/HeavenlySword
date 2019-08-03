/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_SIMPLE_COLLISION_RESPONSE_H
#define HK_DYNAMICS2_SIMPLE_COLLISION_RESPONSE_H

class hkContactPoint;
class hkMotion;
class hkVelocityAccumulator;
struct hkSimpleConstraintUtilCollideParams;

class hkSimpleCollisionResponse
{
	public:
		struct SolveSingleOutput
		{
			hkReal m_impulse;
			hkReal m_velocityKeyframedA;
			hkReal m_velocityKeyframedB;
		};

		static void HK_CALL solveSingleContact( const hkContactPoint& cp, hkTime time, const hkSimpleConstraintUtilCollideParams& params,
												hkMotion* bodyA, hkMotion* bodyB,
												class hkDynamicsContactMgr* contactMgr, SolveSingleOutput& output );

		struct SolveSingleOutput2
		{
			hkReal m_impulse;
		};

		static void HK_CALL solveSingleContact2( class hkSimpleContactConstraintData* constraintData,
												 const hkContactPoint& cp, const hkSimpleConstraintUtilCollideParams& params,
												 hkRigidBody* rigidBodyA, hkRigidBody* rigidBodyB, 
												 hkVelocityAccumulator* bodyA, hkVelocityAccumulator* bodyB,
												 SolveSingleOutput2& output );

};


#endif // HK_DYNAMICS2_SIMPLE_COLLISION_RESPONSE_H

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
