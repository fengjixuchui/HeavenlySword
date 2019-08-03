/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkragdoll/hkRagdoll.h>
#include <hkragdoll/controller/rigidbody/hkKeyFrameHierarchyUtility.h>

#include <hkbase/memory/hkLocalBuffer.h>
#include <hkdynamics/entity/hkRigidBody.h>

struct MatrixCache
{
	hkQsTransform m_desiredPose;
	hkQsTransform m_zeroHierarchyGainPose;
};

void hkKeyFrameHierarchyUtility::initialize( const BodyData& bodies, WorkElem* internalReferencePose )
{
	for (int rbId=0; rbId<bodies.m_numRigidBodies; rbId++)
	{
		hkRigidBody* body = bodies.m_rigidBodies[rbId];
		WorkElem& old = internalReferencePose[ rbId ];
		old.m_prevLinearVelocity = body->getLinearVelocity();
		old.m_prevAngularVelocity = body->getAngularVelocity();
		old.m_prevPosition = body->getCenterOfMassInWorld();
		old.m_prevRotation = body->getRotation();
	}
}

static void HK_CALL calcInertiaFromCom(hkVector4& shift, hkReal mass, hkMatrix3& inertia)
{
	inertia(3,0) = 0.0f;
	inertia(3,1) = 0.0f;
	inertia(3,2) = 0.0f;
	inertia(0,0) = mass * (shift(1)*shift(1) + shift(2)*shift(2));
	inertia(1,1) = mass * (shift(2)*shift(2) + shift(0)*shift(0));
	inertia(2,2) = mass * (shift(0)*shift(0) + shift(1)*shift(1));
	inertia(0,1) = inertia(1,0) = -mass * shift(0) * shift(1); 
	inertia(1,2) = inertia(2,1) = -mass * shift(1) * shift(2); 
	inertia(2,0) = inertia(0,2) = -mass * shift(2) * shift(0); 
}


	// calc the pose in worldspace and in center of mass space of the rigid bodies
static void HK_CALL calcComPoseUsingHierarchy(  const hkQsTransform* desiredPoseLocalSpace, const hkKeyFrameHierarchyUtility::BodyData& bodydata, const hkKeyFrameHierarchyUtility::ControlData* controlPalette,
									 const hkQsTransform& worldFromPose, MatrixCache* mc )
{
	for (int rbId=0; rbId< bodydata.m_numRigidBodies; rbId++)
	{
		hkQsTransform& zeroHierarchyGainPose = mc[rbId].m_zeroHierarchyGainPose;

		int parent = bodydata.m_parentIndices[rbId];
		{
			const hkQsTransform& parentT = ( parent >= 0) ? mc[parent].m_zeroHierarchyGainPose : worldFromPose;
			zeroHierarchyGainPose.setMul( parentT, desiredPoseLocalSpace[rbId] );
		}

		hkQuaternion poseOrientation = zeroHierarchyGainPose.getRotation();
		hkVector4    posePosition    = zeroHierarchyGainPose.getTranslation();
		const hkKeyFrameHierarchyUtility::ControlData& control = (bodydata.m_controlDataIndices) ? controlPalette[ bodydata.m_controlDataIndices[rbId] ] : controlPalette[0];
		const hkReal hierarchyGain = control.m_hierarchyGain;

		if ( hierarchyGain > 0.0f )
		{
			if ( parent >= 0)
			{
				hkRigidBody* parentBody = bodydata.m_rigidBodies[parent];
				hkQsTransform parentT (parentBody->getPosition(), parentBody->getRotation() );

				hkQsTransform desiredBasedOnParent; desiredBasedOnParent.setMul( parentT, desiredPoseLocalSpace[rbId] );
				poseOrientation.setSlerp(  poseOrientation, desiredBasedOnParent.getRotation(), hierarchyGain );
				posePosition.setInterpolate4( posePosition, desiredBasedOnParent.getTranslation(), hierarchyGain );
			}
		}

			// update the center to the center of mass of the rigid bodies
		{
			hkRigidBody* body = bodydata.m_rigidBodies[rbId];
			hkVector4 newCenterOfMassPosition;
			
			newCenterOfMassPosition.setRotatedDir( poseOrientation, body->getCenterOfMassLocal() );
			posePosition.add4( newCenterOfMassPosition );
		}
		mc[rbId].m_desiredPose.set( posePosition, poseOrientation, hkVector4::getZero() );
	}
}


static inline void HK_CALL clipVector( hkReal maxLen, hkVector4& vec )
{
	hkReal len2 = vec.lengthSquared3();
	hkReal max2 = maxLen * maxLen;
	if ( len2 > max2 )
	{
		hkReal f = maxLen * hkMath::sqrtInverse( len2 );
		vec.mul4( f );
	}
}

static inline hkReal HK_CALL clipSnapVector( hkReal snapMaxVelocity, hkReal maxDist, hkVector4& vectorIn )
{
	hkReal len2 = vectorIn.lengthSquared3();
	hkReal max2 = snapMaxVelocity * snapMaxVelocity;

	hkReal stressSquared = len2/max2;

	if ( len2 > max2 )
	{
		hkReal maxDist2 = maxDist * maxDist;
		if ( len2 < maxDist2 )
		{
			hkReal f = hkMath::sqrt( max2/len2);
			vectorIn.mul4( f );
		}
		else
		{
			hkReal f = (snapMaxVelocity * maxDist) / (len2);
			vectorIn.mul4( f );
		}
	}
	return stressSquared;
}

void hkKeyFrameHierarchyUtility::applyKeyFrame( hkReal m_deltaTime, const KeyFrameData& pose, const BodyData& bodydata, const ControlData* controlPalette, Output* output )
{

	// Get an array of the desired pose in world space
	hkLocalBuffer<MatrixCache> matrixBuffer ( bodydata.m_numRigidBodies );

	{
		calcComPoseUsingHierarchy(  pose.m_desiredPoseLocalSpace, bodydata, controlPalette,						  
			                        pose.m_worldFromRoot, &matrixBuffer[0] );
	}

	hkVector4 sumAngularMomentumApplied; sumAngularMomentumApplied.setZero4();
	hkVector4 sumLinearMomentumApplied;  sumLinearMomentumApplied.setZero4();

	const hkReal invDeltaTime = 1.0f / m_deltaTime;
	const hkSimdReal invDeltaTimeSimd = invDeltaTime;

	for (int rbId=0; rbId< bodydata.m_numRigidBodies; rbId++)
	{
		const hkKeyFrameHierarchyUtility::ControlData& control = (bodydata.m_controlDataIndices) ? controlPalette[ bodydata.m_controlDataIndices[rbId] ] : controlPalette[0];

		const hkSimdReal oneMinusDamp = 1.0f - control.m_velocityDamping;

		hkRigidBody* body = bodydata.m_rigidBodies[rbId];
		MatrixCache& mc  = matrixBuffer[rbId];
		WorkElem&    old = pose.m_internalReferencePose[ rbId ];

		hkVector4 desiredLinearVelocity;
		hkVector4 poseLinearVelocity;
		hkVector4 poseLinearAcceleration;
		{
			const hkVector4&    posePosition    = mc.m_desiredPose.getTranslation();

			desiredLinearVelocity.setSub4( posePosition, body->getCenterOfMassInWorld() );
			desiredLinearVelocity.mul4( invDeltaTimeSimd );

			poseLinearVelocity.setSub4( posePosition, old.m_prevPosition );
			old.m_prevPosition = posePosition;
			poseLinearVelocity.mul4( invDeltaTimeSimd );

			poseLinearAcceleration.setSub4( poseLinearVelocity, old.m_prevLinearVelocity );
			old.m_prevLinearVelocity = poseLinearVelocity;
		}

		hkVector4 localLinearVelocity = body->getLinearVelocity();
		hkVector4 currentLinearVelocity; currentLinearVelocity.setMul4( oneMinusDamp, localLinearVelocity );

			// pose acceleration based
		{
			currentLinearVelocity.addMul4( control.m_accelerationGain, poseLinearAcceleration );
		}

			// pose velocity based
		{
			poseLinearVelocity.sub4( currentLinearVelocity );
			hkVector4 velocity; velocity.setMul4( control.m_velocityGain, poseLinearVelocity );
			currentLinearVelocity.add4( velocity );
		}

			// position based
		{
			desiredLinearVelocity.sub4( currentLinearVelocity );
			hkVector4 velocity; velocity.setMul4( control.m_positionGain, desiredLinearVelocity );
			clipVector( control.m_positionMaxLinearVelocity, velocity );
			currentLinearVelocity.add4( velocity );
		}

			// snap
		if ( control.m_snapGain )
		{
			hkVector4 optimalCenterOfMassPosition;
			optimalCenterOfMassPosition.setRotatedDir( mc.m_zeroHierarchyGainPose.getRotation(), body->getCenterOfMassLocal() );
			optimalCenterOfMassPosition.add4( mc.m_zeroHierarchyGainPose.getTranslation() );

			hkVector4 optimalVelocity; optimalVelocity.setSub4( optimalCenterOfMassPosition, body->getCenterOfMassInWorld() );
			optimalVelocity.mul4( invDeltaTimeSimd );
			optimalVelocity.sub4( currentLinearVelocity );
			optimalVelocity.mul4( control.m_snapGain );

			hkReal maxDist = control.m_snapMaxLinearDistance * invDeltaTime * control.m_snapGain;
			hkReal stressSquared = clipSnapVector( control.m_snapMaxLinearVelocity,  maxDist, optimalVelocity );
			if (output) output[rbId].m_stressSquared = stressSquared;
			currentLinearVelocity.add4( optimalVelocity );
		}



			// apply velocity
		{
			body->setLinearVelocity(currentLinearVelocity);
		}

		// Get ang vel required
		hkVector4 desiredAngularVelocity;
		hkVector4 poseAngularVelocity;
		hkVector4 poseAngularAcceleration;
		{
			const hkQuaternion& poseOrientation = mc.m_desiredPose.getRotation();

			body->getRotation().estimateAngleTo( poseOrientation, desiredAngularVelocity );
			desiredAngularVelocity.mul4( invDeltaTimeSimd );

			old.m_prevRotation.estimateAngleTo( poseOrientation, poseAngularVelocity );
			old.m_prevRotation = poseOrientation;

			poseAngularVelocity.mul4( invDeltaTimeSimd );
			
			poseAngularAcceleration.setSub4( poseAngularVelocity, old.m_prevAngularVelocity );
			old.m_prevAngularVelocity = poseAngularVelocity;
		}

		hkVector4 localAngularVelocity =  body->getAngularVelocity();
		hkVector4 currentAngularVelocity;   currentAngularVelocity.setMul4( oneMinusDamp, localAngularVelocity );

			// pose acceleration based
		{
			currentAngularVelocity.addMul4( control.m_accelerationGain, poseAngularAcceleration );
		}

			// input velocity based
		{
			poseAngularVelocity.sub4( currentAngularVelocity );
			hkVector4 velocity; velocity.setMul4( control.m_velocityGain, poseAngularVelocity );
			currentAngularVelocity.add4( velocity );
		}

			// position based
		{
			desiredAngularVelocity.sub4( currentAngularVelocity );
			hkVector4 velocity;velocity.setMul4( control.m_positionGain, desiredAngularVelocity );
			clipVector( control.m_positionMaxAngularVelocity, velocity );
			currentAngularVelocity.add4( velocity );
		}

			// snap
		if ( control.m_snapGain )
		{
			hkVector4 optimalVelocity;
			body->getRotation().estimateAngleTo( mc.m_zeroHierarchyGainPose.getRotation(), optimalVelocity );
			optimalVelocity.mul4( invDeltaTimeSimd );
			optimalVelocity.sub4( currentAngularVelocity );
			optimalVelocity.mul4( control.m_snapGain );

			hkReal maxDist = control.m_snapMaxAngularDistance * invDeltaTime * control.m_snapGain;
			hkReal stressSquared = clipSnapVector( control.m_snapMaxAngularVelocity,  maxDist, optimalVelocity );
			if (output) output[rbId].m_stressSquared += stressSquared;
			currentAngularVelocity.add4( optimalVelocity );
		}

			// apply velocity
		{
			body->setAngularVelocity(currentAngularVelocity);
		}
	}	// for all bodies
}


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
