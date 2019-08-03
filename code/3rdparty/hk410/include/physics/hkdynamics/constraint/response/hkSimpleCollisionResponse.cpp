/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkbase/debugutil/hkTraceStream.h>

#include <hkmath/linear/hkVector4Util.h>
#include <hkmath/basetypes/hkContactPoint.h>
#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkconstraintsolver/simpleConstraints/hkSimpleConstraintUtil.h>
#include <hkconstraintsolver/constraint/contact/hkContactPointProperties.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>

#include <hkdynamics/constraint/response/hkSimpleCollisionResponse.h>
#include <hkdynamics/motion/hkMotion.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/collide/hkDynamicsContactMgr.h>
#include <hkdynamics/constraint/contact/hkSimpleContactConstraintData.h>

void hkSimpleCollisionResponse::solveSingleContact( const hkContactPoint& cp, hkTime time, const hkSimpleConstraintUtilCollideParams& params, 
												    hkMotion* bodyA, hkMotion* bodyB, hkDynamicsContactMgr* contactMgr, SolveSingleOutput& output )
{

	hkSimpleConstraintInfoInitInput inA;
	{
		inA.m_invMass = bodyA->getMassInv();
		hkVector4 massCenter; hkSweptTransformUtil::calcCenterOfMassAt( bodyA->m_motionState, time, massCenter );
		inA.m_massRelPos.setSub4( cp.getPosition(), massCenter );
		bodyA->getInertiaInvWorld( inA.m_invInertia );
	}

	hkSimpleConstraintInfoInitInput inB;
	{
		inB.m_invMass = bodyB->getMassInv();
		hkVector4 massCenter; hkSweptTransformUtil::calcCenterOfMassAt( bodyB->m_motionState, time, massCenter );
		inB.m_massRelPos.setSub4( cp.getPosition(), massCenter );
		bodyB->getInertiaInvWorld( inB.m_invInertia );
	}

	hkBodyVelocity velA;
	hkBodyVelocity velB;
	{
		velA.m_linear  = bodyA->getLinearVelocity();
		velA.m_angular = bodyA->getAngularVelocity();
		velB.m_linear  = bodyB->getLinearVelocity();
		velB.m_angular = bodyB->getAngularVelocity();
	}

	// use collision-related velocities for calculation of m_velocityKeyframedA/B
	
	
	hkBodyVelocity origVelA; hkSweptTransformUtil::getVelocity( bodyA->m_motionState, origVelA.m_linear, origVelA.m_angular);
	hkBodyVelocity origVelB; hkSweptTransformUtil::getVelocity( bodyB->m_motionState, origVelB.m_linear, origVelB.m_angular);

	hkSimpleConstraintInfo info;
	hkRotation directions;
	hkVector4Util::buildOrthonormal( cp.getNormal(), directions );
	
	// insert callback here
	contactMgr->toiCollisionResponseBeginCallback( cp, inA, velA, inB, velB );

	hkSimpleConstraintUtil_InitInfo( inA, inB, directions, info );


	// This takes the m_extarnalSeparatingVelocity param as the current collision-detection velocities 
	output.m_impulse = hkSimpleConstraintUtil_Collide( info, params, velA, velB );

	contactMgr->toiCollisionResponseEndCallback( cp, output.m_impulse, inA, velA, inB, velB );


	if ( hkDebugToi)
	{
		hkVector4 pV; hkSimpleConstraintUtil_getPointVelocity(info, velA, velB, pV );
		hkToiPrintf( "Post", "#         Post v:%2.4f  i:%4.4f\n", pV(0), output.m_impulse );
	}

	//
	//	Check for delaying one impulse
	//
	{
		hkVector4 pA; hkSimpleConstraintUtil_getPointVelocity(info, origVelA, velB, pA );
		hkVector4 pB; hkSimpleConstraintUtil_getPointVelocity(info, velA, origVelB, pB );
#ifdef HK_DEBUG
		hkVector4 finalVelocityWhenBothBodiesAreReintegrated; hkSimpleConstraintUtil_getPointVelocity(info, velA, velB, finalVelocityWhenBothBodiesAreReintegrated );
		if (finalVelocityWhenBothBodiesAreReintegrated(0) < -HK_REAL_EPSILON)
		{
			HK_WARN(0xad45d441, "Internal warning. SCR generated invalid velocities");
		}
#endif

		output.m_velocityKeyframedA = pA(0);
		output.m_velocityKeyframedB = pB(0);
	}

	//HK_ASSERT(0xAD000002, output.m_velocityKeyframedA < 0.0f || output.m_velocityKeyframedB < 0.0f);

	//
	//	Write back the results
	//
	{
		bodyA->setLinearVelocity( velA.m_linear );
		bodyA->setAngularVelocity( velA.m_angular );
		bodyB->setLinearVelocity( velB.m_linear );
		bodyB->setAngularVelocity( velB.m_angular );
	}
}

void hkSimpleCollisionResponse::solveSingleContact2( class hkSimpleContactConstraintData* constraintData,
													 const hkContactPoint& cp, const hkSimpleConstraintUtilCollideParams& params,
													 hkRigidBody* rigidBodyA, hkRigidBody* rigidBodyB,
													 hkVelocityAccumulator* bodyA, hkVelocityAccumulator* bodyB,
													 SolveSingleOutput2& output )
{
	hkBodyVelocity vela[2];
	hkSimpleConstraintInfoInitInput ina[2];
	{
		hkVelocityAccumulator* body = bodyA;
		for ( int i = 0; i< 2; i++ )
		{
			hkSimpleConstraintInfoInitInput& in = ina[i];
			hkBodyVelocity& vel = vela[i];
			{
				in.m_invMass = body->m_invMasses(3);
				const hkVector4& massCenter = body->getCenterOfMassInWorld();
				in.m_massRelPos.setSub4( cp.getPosition(), massCenter );

				const hkVector4& iD = body->m_invMasses;
				vel.m_linear = body->m_linearVel;
				if ( body->m_matrixIsIdentity )
				{
					vel.m_angular = body->m_angularVel;
					in.m_invInertia.setDiagonal( iD(0), iD(1), iD(2) );
				}
				else
				{
					const hkRotation& t = body->getCoreFromWorldMatrix();
					vel.m_angular._setRotatedInverseDir( t, body->m_angularVel );
					hkMatrix3 x;
					hkVector4 a; a.setBroadcast3clobberW( iD, 0);
					hkVector4 b; b.setBroadcast3clobberW( iD, 1);
					hkVector4 c; c.setBroadcast3clobberW( iD, 2);
					x.getColumn(0).setMul4( a, t.getColumn(0) );
					x.getColumn(1).setMul4( b, t.getColumn(1) );
					x.getColumn(2).setMul4( c, t.getColumn(2) );

					in.m_invInertia.setMulInverse( x, t );
					in.m_invInertia.transpose();
				}
			}
			body = bodyB;
		}
	}
	
	hkSimpleConstraintInfo info;
	hkRotation directions;
	hkVector4Util::buildOrthonormal( cp.getNormal(), directions );
	
	constraintData->collisionResponseBeginCallback( cp, ina[0], vela[0], ina[1], vela[1] );

	hkSimpleConstraintUtil_InitInfo( ina[0], ina[1], directions, info );
	output.m_impulse = hkSimpleConstraintUtil_Collide( info, params, vela[0], vela[1] );

	constraintData->collisionResponseEndCallback( cp, output.m_impulse, ina[0], vela[0], ina[1], vela[1] );

	//
	//	Write back the results
	//
	{
		bodyA->m_linearVel  = vela[0].m_linear;
		rigidBodyA->getRigidMotion()->m_linearVelocity  = vela[0].m_linear;
		rigidBodyA->getRigidMotion()->m_angularVelocity = vela[0].m_angular;

		bodyB->m_linearVel  = vela[1].m_linear;
		rigidBodyB->getRigidMotion()->m_linearVelocity  = vela[1].m_linear;
		rigidBodyB->getRigidMotion()->m_angularVelocity = vela[1].m_angular;

		hkVelocityAccumulator* body = bodyA;
		for (int j=0; j < 2; j++)
		{
			hkBodyVelocity& vel = vela[j];
			if ( body->m_matrixIsIdentity )
			{
				body->m_angularVel = vel.m_angular;
			}
			else
			{
				body->m_angularVel._setRotatedDir( body->getCoreFromWorldMatrix(), vel.m_angular );
			}
			body=bodyB;
		}
	}
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
