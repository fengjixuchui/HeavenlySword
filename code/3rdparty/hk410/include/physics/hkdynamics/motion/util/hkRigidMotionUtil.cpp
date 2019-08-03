/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkmath/linear/hkVector4Util.h>

#include <hkdynamics/motion/util/hkRigidMotionUtil.h>
#include <hkdynamics/motion/rigid/hkBoxMotion.h>
#include <hkdynamics/motion/rigid/hkStabilizedSphereMotion.h>
#include <hkdynamics/motion/rigid/hkStabilizedBoxMotion.h>
#include <hkdynamics/motion/rigid/hkFixedRigidMotion.h>
#include <hkdynamics/motion/rigid/hkKeyframedRigidMotion.h>
#include <hkdynamics/motion/rigid/ThinBoxMotion/hkThinBoxMotion.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>

#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
#	include <hkbase/thread/util/hkSpuSimulator.h>
#endif

	// simply calculates the velocity of the object
static HK_FORCE_INLINE hkReal calcVelocityForDeactivation( const hkMotion& motion, hkReal objectRadius)
{
	hkReal linVelSqrd = motion.m_linearVelocity.lengthSquared3();
	hkReal angVelSqrd = motion.m_angularVelocity.lengthSquared3();
	hkReal vel = objectRadius * objectRadius * angVelSqrd + linVelSqrd;
	return vel;
}



	// returns the number of inactive frames
	// Every 4th/16th frame it checks the transform of the motion against the reference positions
	// See hkWorldCinfo::m_deactivationReferenceDistance
int hkRigidMotionUtilCheckDeactivation( const struct hkSolverInfo& si, hkMotion& motion )
{
	hkUint32 c = motion.m_deactivationIntegrateCounter;
	c++;
	motion.m_deactivationIntegrateCounter = hkUchar(c);
	if ( (c&3) == 0 )
	{
		// select high or low frequency check
		int select;
		if ( (c&15)!=0)
		{
			// high frequency checks
			select = 0;
		}
		else
		{
				// check for no deactivation
			if ( c == 0x100 )
			{
				motion.m_deactivationIntegrateCounter = 0xff;
				goto END_OF_FUNCTION;
			}
				// select low frequency checks
			motion.m_deactivationIntegrateCounter = 0;
			select = 1;
		}

		// we have to clip the radius. The reason is jitter because we store the quaternion in 32 bit
		// as a result we get 4% jitter. Without clipping the radius, the jitter could be enough to keep the object alive
		hkReal     radius          = hkMath::min2( 1.0f, motion.m_motionState.m_objectRadius);
		HK_ASSERT2(0x5c457c12,  radius > 0, "Radius was not set correctly for entity ");

		hkReal velSqrd = calcVelocityForDeactivation( motion, radius );
		{
			// we remember our maximum velocity. This is used to as a reference velocity for the final
			// deactivation
			hkReal& maxDeactVel = motion.m_deactivationRefPosition[select](3);	maxDeactVel = hkMath::max2( maxDeactVel, velSqrd );
		}


		hkVector4& refPosition    = motion.m_deactivationRefPosition[select];
		hkUint32&  refOrientation = motion.m_motionState.m_deactivationRefOrientation[select];


		const hkSweptTransform& sweptTransform = motion.m_motionState.getSweptTransform();

		const hkVector4&    currentPosition = sweptTransform.m_centerOfMass1;
		const hkQuaternion& currentRotation = sweptTransform.m_rotation1;

		while(1)	// dummy loop to improve code layout
		{
			const hkSolverInfo::DeactivationInfo& di = si.m_deactivationInfo[motion.m_motionState.m_deactivationClass];
				// distance check
			{
				hkReal maxSqrd = di.m_maxDistSqrd[select];

				hkVector4 transDist;	transDist.setSub4( refPosition, currentPosition );
				const hkReal distSqrd = transDist.lengthSquared3();
				if ( distSqrd > maxSqrd)
				{
					break;
				}
			}

				// orientation check
			{
				hkQuaternion refQ; hkVector4Util::unPackInt32IntoQuaternion( refOrientation, refQ);

				hkReal maxSqrd = di.m_maxRotSqrd[select];

				hkVector4 dist;	dist.setSub4( refQ.m_vec, currentRotation.m_vec );
				const hkReal distSqrd = dist.lengthSquared4();
				if ( distSqrd > maxSqrd)
				{
					break;
				}

			}

				// deactivate. Increment counter, but clip value at 64
			{
				int dc = motion.m_deactivationNumInactiveFrames[select];
				motion.m_deactivationNumInactiveFrames[select] = hkUint8(dc + 1 - (dc>>6));
				goto END_OF_FUNCTION;
			}
		}


		motion.m_deactivationNumInactiveFrames[select] = 0;

		// reset reference position and orientation and max velocity
		{
			hkVector4 pos = currentPosition;
			pos.zeroElement(3);	// thats the max velocity
			refPosition = pos;
			refOrientation = hkVector4Util::packQuaternionIntoInt32(currentRotation);
		}
	}
END_OF_FUNCTION:
	return hkMath::max2( motion.m_deactivationNumInactiveFrames[0], motion.m_deactivationNumInactiveFrames[1] );
}

#if !defined(HK_PLATFORM_PS3SPU)

bool hkRigidMotionUtilCanDeactivateFinal( const hkStepInfo& info, hkMotion*const* motions, int numMotions, int motionOffset )
{
	for (int i = numMotions-1; i>=0; motions++, i--)
	{
		hkMotion& motion = *hkAddByteOffset(motions[0], motionOffset);
		HK_ASSERT2( 0xf03245df, motion.m_type != hkMotion::MOTION_FIXED, "Internal error, checking fixed motion is not allowed" );

		// we have to clip the radius. The reason is jitter because we store the quaternion in 32 bit
		// as a result we get 4% jitter. Without clipping the radius, the jitter could be enough to keep the object alive
		hkReal     radius          = hkMath::min2( 1.0f, motion.m_motionState.m_objectRadius);

			// we allow 2 times the max velocity as well as an extra sleepVel
		const hkReal sleepVel = 0.1f;	//@@@ps3 make this external
		hkReal velSqrd = 0.25f * calcVelocityForDeactivation( motion, radius ) - sleepVel*sleepVel;

			// check the velocity where we have the higher counter; if counters are equal, use the low frequency check
		if ( motion.m_deactivationNumInactiveFrames[0] > motion.m_deactivationNumInactiveFrames[1])
		{
			if ( velSqrd > motion.m_deactivationRefPosition[0](3))
			{
				return false;
			}
		}
		else
		{
			if ( velSqrd > motion.m_deactivationRefPosition[1](3))
			{
				return false;
			}
		}
	}
	return true;
}


void hkRigidMotionUtilStep( const hkStepInfo& info, hkMotion*const* motions, int numMotions, int motionOffset )
{
	for (int i = numMotions-1; i>=0; motions++, i--)
	{
		hkMotion* motion = hkAddByteOffset(motions[0], motionOffset);
		HK_ASSERT2( 0xf032eddf, motion->m_type != hkMotion::MOTION_FIXED, "Internal error, stepping fixed motion is not allowed" );
		hkSweptTransformUtil::_stepMotionState( info, motion->m_linearVelocity, motion->m_angularVelocity, motion->m_motionState);
	}
}



int hkRigidMotionUtilApplyForcesAndStep( const struct hkSolverInfo& solverInfo, const hkStepInfo& info, const hkVector4& deltaVel, hkMotion*const* motions, int numMotions, int motionOffset )
{
	int numInactiveFrames = 0x7fffffff;
	for (int i = numMotions-1; i>=0; motions++, i--)
	{
		hkMotion* motion = hkAddByteOffset(motions[0], motionOffset);

		switch( motion->m_type )
		{
			case hkMotion::MOTION_FIXED:
				{
					continue;
				}

			case hkMotion::MOTION_THIN_BOX_INERTIA:
				{
					hkThinBoxMotion* tbm = reinterpret_cast<hkThinBoxMotion*>( motion );
					tbm->m_linearVelocity.add4( deltaVel );
					const hkReal one = 1.0f;
					const hkReal deltaTime = info.m_deltaTime;
					tbm->m_linearVelocity.mul4(  hkMath::max2( 0.0f, one - deltaTime * tbm->m_motionState.m_linearDamping ) );
					tbm->m_angularVelocity.mul4( hkMath::max2( 0.0f, one - deltaTime * tbm->m_motionState.m_angularDamping) );

					// get the angular momentum in world space
					hkVector4 angularMomentum;
					{
						hkVector4 h; h.setRotatedInverseDir( tbm->getTransform().getRotation(), tbm->m_angularVelocity );
						hkVector4 in; in.setReciprocal4( tbm->m_inertiaAndMassInv );
						h.mul4( in );
						angularMomentum.setRotatedDir( tbm->getTransform().getRotation(), h );
					}

					hkSweptTransformUtil::_stepMotionState( info, tbm->m_linearVelocity, tbm->m_angularVelocity, tbm->m_motionState);

					// get the new angular velocity from the world momentum
					{
						hkVector4 h; h.setRotatedInverseDir( tbm->getTransform().getRotation(), angularMomentum );
						h.mul4( tbm->m_inertiaAndMassInv );
						tbm->m_angularVelocity.setRotatedDir( tbm->getTransform().getRotation(), h );
					}
					goto CHECK_DEACTIVATION;
				}

			case hkMotion::MOTION_KEYFRAMED:
				{
					break;
				}

			default:
				{
					motion->m_linearVelocity.add4( deltaVel );
					const hkReal one = 1.0f;
					const hkReal deltaTime = info.m_deltaTime;
					motion->m_linearVelocity.mul4(  hkMath::max2( 0.0f, one - deltaTime * motion->m_motionState.m_linearDamping ) );
					motion->m_angularVelocity.mul4( hkMath::max2( 0.0f, one - deltaTime * motion->m_motionState.m_angularDamping) );
					break;
				}
		}
		hkSweptTransformUtil::_stepMotionState( info, motion->m_linearVelocity, motion->m_angularVelocity, motion->m_motionState);

CHECK_DEACTIVATION:
		numInactiveFrames = hkMath::min2( numInactiveFrames, hkRigidMotionUtilCheckDeactivation( solverInfo, *motion ));
	}
	return numInactiveFrames;
}
#endif

// Disable optimization here for flaky 64 bit compiler
#if defined(HK_PLATFORM_X64)
#pragma optimize("g", off)
#endif

hkVelocityAccumulator* hkRigidMotionUtilApplyForcesAndBuildAccumulators(const hkStepInfo& info, hkMotion*const* motions, int numMotions, int motionOffset, hkVelocityAccumulator* accumulatorsOut )
{
	for (int i = numMotions-1; i>=0; motions++, accumulatorsOut++, i--)
	{
		hkMotion* motion = hkAddByteOffset(motions[0], motionOffset);

		switch( motion->m_type )
		{
			case hkMotion::MOTION_INVALID:
				{
					HK_ASSERT2( 0xf03243ee, 0, "hkMotion::MOTION_INVALID detected");
				}

			case hkMotion::MOTION_THIN_BOX_INERTIA:
			case hkMotion::MOTION_BOX_INERTIA:
			case hkMotion::MOTION_STABILIZED_BOX_INERTIA:
				{
					const hkReal one = 1.0f;
					const hkReal deltaTime = info.m_deltaTime;
					motion->m_linearVelocity.mul4(  hkMath::max2( 0.0f, one - deltaTime * motion->m_motionState.m_linearDamping ) );
					motion->m_angularVelocity.mul4( hkMath::max2( 0.0f, one - deltaTime * motion->m_motionState.m_angularDamping) );

					hkVelocityAccumulator* accumulator = accumulatorsOut;

					accumulator->getCoreFromWorldMatrix().setTranspose( motion->getTransform().getRotation() );

					accumulator->m_type = hkVelocityAccumulator::HK_RIGID_BODY;		

					const hkBoxMotion* bm = reinterpret_cast<hkBoxMotion*>( motion );
					accumulator->m_invMasses = bm->m_inertiaAndMassInv;
					accumulator->m_linearVel = motion->m_linearVelocity;

					hkMotionState& ms = motion->m_motionState;
					hkSweptTransformUtil::calcCenterOfMassAt( ms, info.m_startTime, accumulator->getCenterOfMassInWorld() );

					accumulator->m_angularVel.           _setRotatedDir( accumulator->getCoreFromWorldMatrix(), motion->m_angularVelocity );
					accumulator->m_matrixIsIdentity    = false;

					accumulator->m_deactivationClass   = motion->getDeactivationClass();
					accumulator->m_deactivationCounter = motion->getDeactivationCounter();

					break;
				}

			case hkMotion::MOTION_SPHERE_INERTIA:
			case hkMotion::MOTION_STABILIZED_SPHERE_INERTIA:
				{
					const hkReal one = 1.0f;
					const hkReal deltaTime = info.m_deltaTime;
					motion->m_linearVelocity.mul4(  hkMath::max2( 0.0f, one - deltaTime * motion->m_motionState.m_linearDamping ) );
					motion->m_angularVelocity.mul4( hkMath::max2( 0.0f, one - deltaTime * motion->m_motionState.m_angularDamping) );

					hkVelocityAccumulator* accumulator = accumulatorsOut;

					accumulator->m_type = hkVelocityAccumulator::HK_RIGID_BODY;

					accumulator->m_invMasses = motion->m_inertiaAndMassInv;

					const hkMotionState& ms = motion->m_motionState;

					hkSweptTransformUtil::calcCenterOfMassAt( ms, info.m_startTime, accumulator->getCenterOfMassInWorld() );

					accumulator->m_angularVel           = motion->m_angularVelocity;
					accumulator->m_linearVel            = motion->m_linearVelocity;
					accumulator->getCoreFromWorldMatrix().setIdentity();
					accumulator->m_matrixIsIdentity     = true;

					accumulator->m_deactivationClass    = motion->getDeactivationClass();
					accumulator->m_deactivationCounter  = motion->getDeactivationCounter();

					break;
				}

			case hkMotion::MOTION_FIXED:
				{
					HK_ASSERT2(0x7a3053cc, 0, "fixed rigid bodies cannot go into the solver");

					hkVelocityAccumulator* accumulator = accumulatorsOut;
					accumulator->setFixed();

					break;
				}

			case hkMotion::MOTION_KEYFRAMED:
				{
					hkVelocityAccumulator* accumulator = accumulatorsOut;

					accumulator->m_type = hkVelocityAccumulator::HK_KEYFRAMED_RIGID_BODY;

					accumulator->m_invMasses.               setZero4();
					accumulator->getCenterOfMassInWorld() = motion->getCenterOfMassInWorld();
					accumulator->m_angularVel             = motion->m_angularVelocity;
					accumulator->m_linearVel              = motion->m_linearVelocity;
					accumulator->getCoreFromWorldMatrix().  setIdentity();
					accumulator->m_matrixIsIdentity       = true;

					break;
				}

			default:
				{
					HK_ASSERT2( 0xf0323456, 0, "Unknown motion" );
				}
		}
	}

	return accumulatorsOut;
}

#if defined(HK_PLATFORM_X64)
#pragma optimize("g", on)
#endif

int hkRigidMotionUtilApplyAccumulators(const struct hkSolverInfo& solverInfo, const hkStepInfo& info, const hkVelocityAccumulator* accumulators, hkMotion*const* motions, int numMotions, int motionOffset, hkBool processDeactivation )
{
	const hkVelocityAccumulator* accu = accumulators;

	int numInactiveFrames = 0x7fffffff;
	hkVector4 integrationLinearVelocity;
	hkVector4 integrationAngularVelocity;

	for (int i = numMotions-1; i>=0; motions++, accu++, i--)
	{
		hkMotion* motion = hkAddByteOffset(motions[0], motionOffset);

		switch( motion->m_type )
		{
			case hkMotion::MOTION_INVALID:
				{
					HK_ASSERT2( 0xf03243ed, 0, "hkMotion::MOTION_INVALID detected");
				}

			case hkMotion::MOTION_THIN_BOX_INERTIA:
				{
					hkThinBoxMotion* tbm = reinterpret_cast<hkThinBoxMotion*>( motion );

					tbm->m_linearVelocity = accu->getSumLinearVel();
					tbm->m_angularVelocity._setRotatedDir( tbm->getTransform().getRotation(), accu->getSumAngularVel() );

					tbm->setDeactivationCounter( accu->m_deactivationCounter );

					// get the angular momentum in world space
					hkVector4 angularMomentum;
					{
						hkVector4 h; h.setRotatedInverseDir( tbm->getTransform().getRotation(), tbm->m_angularVelocity );
						hkVector4 in; in.setReciprocal4( tbm->m_inertiaAndMassInv );
						h.mul4( in );
						angularMomentum.setRotatedDir( tbm->getTransform().getRotation(), h );
					}

					hkSweptTransformUtil::_stepMotionState( info, tbm->m_linearVelocity, tbm->m_angularVelocity, tbm->m_motionState);
					tbm->m_linearVelocity = accu->m_linearVel;
					// get the new angular velocity from the world momentum
					{
						hkVector4 h; h.setRotatedInverseDir( tbm->getTransform().getRotation(), angularMomentum );
						h.mul4( tbm->m_inertiaAndMassInv );
						tbm->m_angularVelocity.setRotatedDir( tbm->getTransform().getRotation(), h );
					}
					break;
				}

			case hkMotion::MOTION_STABILIZED_BOX_INERTIA:
				{
 					motion->m_linearVelocity	= accu->getSumLinearVel();
 					motion->m_angularVelocity	= accu->getSumAngularVel();
 					integrationLinearVelocity	= accu->getSumLinearVel();
					integrationAngularVelocity	. setRotatedDir( motion->getTransform().getRotation(), accu->getSumAngularVel() );
					goto sphereInertia;
				}

			case hkMotion::MOTION_STABILIZED_SPHERE_INERTIA:
				{
					motion->m_linearVelocity	= accu->getSumLinearVel();
 					motion->m_angularVelocity	= accu->getSumAngularVel();
 					integrationLinearVelocity	= accu->getSumLinearVel();
 					integrationAngularVelocity	= accu->getSumAngularVel();
					goto sphereInertia;
				}

			case hkMotion::MOTION_KEYFRAMED:
				{
					motion->m_linearVelocity	= motion->m_linearVelocity;
					motion->m_angularVelocity	= motion->m_angularVelocity;
					integrationLinearVelocity	= motion->m_linearVelocity;
					integrationAngularVelocity	= motion->m_angularVelocity;
					goto sphereInertiaNoDeactivationCount;
				}

			case hkMotion::MOTION_BOX_INERTIA:
				{
					motion->m_linearVelocity	= accu->m_linearVel;
 					motion->m_angularVelocity	. _setRotatedDir( motion->getTransform().getRotation(), accu->m_angularVel );
					integrationLinearVelocity	= accu->getSumLinearVel();
 					integrationAngularVelocity	. setRotatedDir( motion->getTransform().getRotation(), accu->getSumAngularVel() );
					goto sphereInertia;
				}

			case hkMotion::MOTION_SPHERE_INERTIA:
				{
					motion->m_linearVelocity	= accu->m_linearVel;
					motion->m_angularVelocity	= accu->m_angularVel;
					integrationLinearVelocity	= accu->getSumLinearVel();
					integrationAngularVelocity	= accu->getSumAngularVel();
sphereInertia:
					motion->setDeactivationCounter( accu->m_deactivationCounter );
sphereInertiaNoDeactivationCount:
					hkSweptTransformUtil::_stepMotionState( info, integrationLinearVelocity, integrationAngularVelocity, motion->m_motionState);
					break;
				}

			case hkMotion::MOTION_FIXED:
			default:
				{
					break;
				}
		}

		if (processDeactivation)
		{
			numInactiveFrames = hkMath::min2( numInactiveFrames, hkRigidMotionUtilCheckDeactivation( solverInfo, *motion ));
		}
	}

	return processDeactivation ? numInactiveFrames : -1;
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
