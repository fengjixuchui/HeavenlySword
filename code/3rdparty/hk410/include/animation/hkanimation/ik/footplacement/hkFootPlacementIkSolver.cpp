/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/ik/footplacement/hkFootPlacementIkSolver.h>

#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>
#include <hkanimation/rig/hkPose.h>

// Debug graphics
#include <hkvisualize/hkDebugDisplay.h>

#include <hkanimation/ik/twojoints/hkTwoJointsIkSolver.h>

#define IN_RANGE(x,a,b) (((x)>=(a))&&((x)<=(b)))

hkFootPlacementIkSolver::hkFootPlacementIkSolver(const Setup& setup) 
:
	m_setup(setup),
	m_currentWeight(0.0f),
	m_previousGroundHeightWS(0.0f),
	m_previousGroundNormalWS(setup.m_worldUpDirectionWS),
	m_previousVerticalDisplacement(0.0f)
{
	HK_ASSERT2 (0x7d9171be, setup.m_skeleton!=HK_NULL,"Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(setup.m_hipIndex, 0 , setup.m_skeleton->m_numBones-1), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(setup.m_kneeIndex, 0 , setup.m_skeleton->m_numBones-1), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(setup.m_ankleIndex, 0 , setup.m_skeleton->m_numBones-1), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, setup.m_kneeAxisLS.isNormalized3(), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, setup.m_worldUpDirectionWS.isNormalized3(), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, setup.m_modelUpDirectionMS.isNormalized3(), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(setup.m_cosineMaxKneeAngle, -1.0f, 1.0f), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(setup.m_cosineMinKneeAngle, -1.0f, 1.0f), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, setup.m_cosineMaxKneeAngle < setup.m_cosineMinKneeAngle, "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, setup.m_raycastDistanceUp>=0.0f, "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, setup.m_raycastDistanceDown>=0.0f, "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(setup.m_footPlantedAnkleHeightMS, setup.m_minAnkleHeightMS, setup.m_maxAnkleHeightMS), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(setup.m_footRaisedAnkleHeightMS, setup.m_minAnkleHeightMS, setup.m_maxAnkleHeightMS), "Invalid setup for foot placement.");
	HK_ASSERT2 (0x7d9171be, setup.m_footPlantedAnkleHeightMS<=setup.m_footRaisedAnkleHeightMS, "Invalid setup for foot placement.");

}

hkFootPlacementIkSolver::Setup::Setup()
:
	m_skeleton(HK_NULL),
	m_hipIndex(-1),
	m_kneeIndex(-1),
	m_ankleIndex(-1),
	m_kneeAxisLS(hkVector4::getZero()),
	m_footEndLS(hkVector4::getZero()),
	m_worldUpDirectionWS(hkVector4::getZero()),
	m_modelUpDirectionMS(hkVector4::getZero()),
	m_originalGroundHeightMS(0.0f),
	m_footPlantedAnkleHeightMS(0.0f),
	m_footRaisedAnkleHeightMS(0.0f),
	m_maxAnkleHeightMS(0.7f),
	m_minAnkleHeightMS(-0.1f),
	m_cosineMaxKneeAngle(-1.0f),
	m_cosineMinKneeAngle(1.0f),
	m_raycastDistanceUp(0.5f),
	m_raycastDistanceDown(0.8f)
{
}

hkFootPlacementIkSolver::Input::Input()
:
	m_footPlacementOn (true),
	m_raycastInterface (HK_NULL),
	m_onOffGain (0.2f),
	m_groundAscendingGain (1.0f),
	m_groundDescendingGain (1.0f),
	m_footPlantedGain (1.0f),
	m_footRaisedGain (1.0f)
{
}

void hkFootPlacementIkSolver::doFootPlacement(const Input& input, Output& output, hkPose& poseInOut)
{
	HK_ASSERT2 (0x7d9171be, input.m_raycastInterface!=HK_NULL, "Invalid input for foot placement. You need to specify a ray cast interface");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(input.m_onOffGain, 0.0f, 1.0f), "Invalid input for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(input.m_groundAscendingGain, 0.0f, 1.0f), "Invalid input for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(input.m_groundDescendingGain, 0.0f, 1.0f), "Invalid input for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(input.m_footPlantedGain, 0.0f, 1.0f), "Invalid input for foot placement.");
	HK_ASSERT2 (0x7d9171be, IN_RANGE(input.m_footRaisedGain, 0.0f, 1.0f), "Invalid input for foot placement.");
	HK_ASSERT2 (0x7d9171be, input.m_footRaisedGain<=input.m_footPlantedGain, "Invalid input for foot placement.");


	// World-To-Model and Model-To-World transforms
	const hkQsTransform& worldFromModel = input.m_worldFromModel;
	hkQsTransform modelFromWorld; modelFromWorld.setInverse(worldFromModel);
	// World UP in model space (not to be confused with Model UP)
	hkVector4 worldUpMS; worldUpMS.setRotatedDir(modelFromWorld.getRotation(), m_setup.m_worldUpDirectionWS);

	output.m_hitSomething = true;

	// Take the original foot height and orientation from the original ankle transform
	const hkReal desiredFootHeightMS = input.m_originalAnkleTransformMS.getTranslation().dot3(m_setup.m_modelUpDirectionMS);
	const hkQuaternion& originalFootRotationMS = input.m_originalAnkleTransformMS.getRotation();

	const hkQuaternion inputAnkleOrientationMS = poseInOut.getBoneModelSpace(m_setup.m_ankleIndex).getRotation();

	const hkQsTransform footCurrentMS = poseInOut.getBoneModelSpace(m_setup.m_ankleIndex);
	const hkVector4 footCurrentPosMS = footCurrentMS.getTranslation();
	hkVector4 footCurrentPosWS; footCurrentPosWS.setTransformedPos(worldFromModel, footCurrentPosMS);

	// If we don't hit anything, we will act as if we were switching the foot placement off, no matter what the user wanted
	bool footPlacementOn = input.m_footPlacementOn;

	// Get the target position and normal using raycast. Add the desired vertical distance as coming from the animation.
	hkVector4 groundNormalWS;
	hkReal actualGroundHeightWS;
	hkReal blendedGroundHeightWS;
	hkVector4 footTargetPosMS;
	{
		if (! footPlacementOn)
		{
			output.m_hitSomething = false;

			// Use the same normal
			groundNormalWS = m_previousGroundNormalWS;
			// Keep the same vertical displacement
			footTargetPosMS.setAddMul4(footCurrentPosMS, worldUpMS, m_previousVerticalDisplacement);
			// Assume height moves with the character
			actualGroundHeightWS = static_cast<hkReal>(footCurrentPosWS.dot3(m_setup.m_worldUpDirectionWS)) + m_previousVerticalDisplacement - desiredFootHeightMS;
			blendedGroundHeightWS = actualGroundHeightWS;
			// do not change the pelvis
			output.m_verticalError = 0.0f;
		}
		else
		{
			// Don't use the toe bone (it's outside the foot), instead use a point inbetwen heel and toe
			hkVector4 footEndCurrentPosMS; footEndCurrentPosMS.setTransformedPos(footCurrentMS, m_setup.m_footEndLS);
			hkVector4 footEndCurrentPosWS; footEndCurrentPosWS.setTransformedPos(worldFromModel, footEndCurrentPosMS);

			hkVector4 projectedFootPosWS;
			bool hit = castFoot( input.m_raycastInterface, footCurrentPosWS, footEndCurrentPosWS, projectedFootPosWS, groundNormalWS );
		
			if (!hit)
			{
				output.m_hitSomething = false;
				footPlacementOn = false;
				// Use the same normal
				groundNormalWS = m_previousGroundNormalWS;
				// Keep the same vertical displacement
				footTargetPosMS.setAddMul4(footCurrentPosMS, worldUpMS, m_previousVerticalDisplacement);
				// Assume height moves with the character
				actualGroundHeightWS = static_cast<hkReal>(footCurrentPosWS.dot3(m_setup.m_worldUpDirectionWS)) + m_previousVerticalDisplacement - desiredFootHeightMS;
				blendedGroundHeightWS = actualGroundHeightWS;
				// do not change the pelvis
				output.m_verticalError = 0.0f;
			}
			else
			{
				output.m_hitSomething = true;

				actualGroundHeightWS = projectedFootPosWS.dot3(m_setup.m_worldUpDirectionWS);

				output.m_verticalError = actualGroundHeightWS - ( static_cast<hkReal>(worldFromModel.m_translation.dot3(m_setup.m_worldUpDirectionWS)) + m_setup.m_originalGroundHeightMS); 

				groundNormalWS(3)=0.0f;

				//HK_DISPLAY_PLANE(groundNormalWS, projectedFootPosWS , 0.5f, 0xffffffff);

				hkVector4 nonVerticalHitPointWS; nonVerticalHitPointWS.setAddMul4(projectedFootPosWS, m_setup.m_worldUpDirectionWS, -actualGroundHeightWS);

				// This value goes from 0 (foot in air) to 1 (foot planted)
				hkReal footPlantWeight = (1.0f - ((desiredFootHeightMS - m_setup.m_footPlantedAnkleHeightMS) / (m_setup.m_footRaisedAnkleHeightMS - m_setup.m_footPlantedAnkleHeightMS)));

				if (footPlantWeight<input.m_footRaisedGain) footPlantWeight = input.m_footRaisedGain;
				if (footPlantWeight>input.m_footPlantedGain) footPlantWeight = input.m_footPlantedGain;

				// Now, calculate the vertical component damping the one from the raycast with the previous one
				{
					// Reset the height on start of Ik
					if (m_currentWeight == 0.0f)
					{
						blendedGroundHeightWS = actualGroundHeightWS;
					}
					else
					{
						// Calculate gain based on distance, possibly weight negative differences (going down) different than going up
						hkReal gain;
						{
							const hkReal factor = (actualGroundHeightWS < m_previousGroundHeightWS) ? input.m_groundDescendingGain : input.m_groundAscendingGain;
								
							gain = footPlantWeight * factor;
						}

						blendedGroundHeightWS = m_previousGroundHeightWS + (actualGroundHeightWS - m_previousGroundHeightWS) * gain;
					}
				}


				hkVector4 dampedHitPointWS; dampedHitPointWS.setAddMul4(nonVerticalHitPointWS, m_setup.m_worldUpDirectionWS, blendedGroundHeightWS);

				hkVector4 dampedHitPointMS; dampedHitPointMS.setTransformedPos(modelFromWorld, dampedHitPointWS);
				
				//HK_DISPLAY_PLANE(groundNormalWS, dampedHitPointWS, 0.5f, 0xff0000ff);

				footTargetPosMS.setAddMul4(dampedHitPointMS, worldUpMS, desiredFootHeightMS - m_setup.m_originalGroundHeightMS);

				// Check that the difference is not too big, clamp otherwise
				{
					// Interpolate between the user value and half of that depending on how much is the foot planted
					const hkReal maxFootHeightMS = m_setup.m_maxAnkleHeightMS;
					const hkReal minFootHeightMS = m_setup.m_minAnkleHeightMS;

					const hkReal footHeightMS = footTargetPosMS.dot3(m_setup.m_modelUpDirectionMS);

					if (footHeightMS>maxFootHeightMS)
					{
						footTargetPosMS.addMul4(maxFootHeightMS-footHeightMS, m_setup.m_modelUpDirectionMS);
					}
					if (footHeightMS<minFootHeightMS)
					{
						output.m_hitSomething = false;
						footPlacementOn = false;
						// Use the same normal
						groundNormalWS = m_previousGroundNormalWS;
						// Keep the same vertical displacement
						footTargetPosMS.setAddMul4(footCurrentPosMS, worldUpMS, m_previousVerticalDisplacement);
						// Assume height moves with the character
						actualGroundHeightWS = static_cast<hkReal>(footCurrentPosWS.dot3(m_setup.m_worldUpDirectionWS)) + m_previousVerticalDisplacement - desiredFootHeightMS;
						blendedGroundHeightWS = actualGroundHeightWS;
						// do not change the pelvis
						output.m_verticalError = 0.0f;
					}

					hkVector4 diffMS; diffMS.setSub4(footTargetPosMS, footCurrentPosMS);
					hkReal verticalDisplacement = diffMS.dot3(m_setup.m_modelUpDirectionMS);
					m_previousVerticalDisplacement = verticalDisplacement;
				}
			}
		}
	}

	// Do gain on the weight
	const hkReal targetWeight = (footPlacementOn ? 1.0f: 0.0f);
	m_currentWeight += (targetWeight - m_currentWeight) * input.m_onOffGain;

	if (m_currentWeight<0.01f)
	{
		m_currentWeight=0.0f;
	}

	// IK FIX-UPS
	if (m_currentWeight>0.0f)
	{
		// NOTE : The IK solver changes the orientation of the bones outside (children of) the chain
		// We don't want that, so we cache the rotation of the heel

		// FIX POSITION
		{

			hkTwoJointsIkSolver::Setup ikSetup;
			ikSetup.m_firstJointIdx = m_setup.m_hipIndex;
			ikSetup.m_secondJointIdx = m_setup.m_kneeIndex;
			ikSetup.m_endBoneIdx = m_setup.m_ankleIndex;
			ikSetup.m_hingeAxisLS = m_setup.m_kneeAxisLS;

			ikSetup.m_endTargetMS = footTargetPosMS;

			ikSetup.m_cosineMinHingeAngle = m_setup.m_cosineMinKneeAngle;
			ikSetup.m_cosineMaxHingeAngle = m_setup.m_cosineMaxKneeAngle;

			ikSetup.m_firstJointIkGain = m_currentWeight;
			ikSetup.m_secondJointIkGain = m_currentWeight;

			hkTwoJointsIkSolver::solve(ikSetup, poseInOut);
		}


		// FIX ORIENTATION
		{
			const hkVector4& desiredUpWS = groundNormalWS;

			hkVector4 worldUpDirectionMS;
			worldUpDirectionMS.setRotatedDir( modelFromWorld.getRotation(), m_setup.m_worldUpDirectionWS );
			hkVector4 desiredUpMS;
			desiredUpMS.setRotatedDir( modelFromWorld.getRotation(), desiredUpWS );

			hkQuaternion extraRotation;
			extraRotation.setShortestRotation( worldUpDirectionMS, desiredUpMS );

			hkQuaternion newRotationMS; newRotationMS.setMul(extraRotation, originalFootRotationMS);
		
			// Keep the translation that came from the IK, set just the rotation (and keep children local)
			hkQuaternion blendedRotation;
			{
				blendedRotation.m_vec.setInterpolate4(originalFootRotationMS.m_vec, newRotationMS.m_vec, m_currentWeight);
				blendedRotation.normalize();
			}
			poseInOut.accessBoneModelSpace(m_setup.m_ankleIndex, hkPose::PROPAGATE).setRotation(blendedRotation);
		}
	}
	else
	{
		output.m_verticalError = 0.0f;
	}

	m_previousGroundNormalWS = groundNormalWS;
	m_previousGroundHeightWS = blendedGroundHeightWS;

}

/*
** Internal methods
*/

hkBool hkFootPlacementIkSolver::castFoot( hkRaycastInterface* raycastInterface, const hkVector4& footWS, const hkVector4& footEndWS, hkVector4& projectedFootWSOut, hkVector4& groundNormalWSOut ) const
{
	const hkReal rayUpDistance = m_setup.m_raycastDistanceUp;
	const hkReal rayDownDistance = m_setup.m_raycastDistanceDown;
	const hkReal rayLength = rayUpDistance + rayDownDistance;

	// First ray cast, from the foot
	hkBool firstRayHit = false;
	hkReal firstRayFraction = HK_REAL_MAX;
	hkVector4 firstRayNormalWS;
	{
		// First ray (foot)
		// setup the raycast endpoints
		hkVector4 footToWS;     footToWS.setAddMul4( footWS, m_setup.m_worldUpDirectionWS, -rayDownDistance );
		hkVector4 footFromWS; footFromWS.setAddMul4( footWS, m_setup.m_worldUpDirectionWS,  rayUpDistance   );
		
		firstRayHit = raycastInterface->castRay (footFromWS, footToWS, firstRayFraction, firstRayNormalWS);
	}

	// Do a second ray cast, from the footEnd and then from the foot
	hkBool secondRayHit = false;
	hkReal secondRayFraction = HK_REAL_MAX;
	hkVector4 secondRayNormalWS;


	if (!footEndWS.equals3(footWS)) // ignore if the second point is the same as the first point
	{
		// setup the raycast endpoints
		hkVector4 footEndToWS; footEndToWS.setAddMul4( footEndWS, m_setup.m_worldUpDirectionWS, -rayDownDistance );
		hkVector4 footEndFromWS; footEndFromWS.setAddMul4( footEndWS, m_setup.m_worldUpDirectionWS, rayUpDistance );
		
		// 
		hkReal footEndRayFraction;
		secondRayHit = raycastInterface->castRay(footEndFromWS, footEndToWS, footEndRayFraction, secondRayNormalWS);


		if (secondRayHit) 
		{
			// HKA-693 
			// Test if the second hit normal is horizontal or pointing downwards.
			// If this is the case, then ignore the hit. Find the angle between the
			// world up direction and the normal returned from the raycast.
			const hkReal cosAngle = hkReal(secondRayNormalWS.dot3(m_setup.m_worldUpDirectionWS));

			// If the angle is 90 degrees or more (cosine is less that or equal to zero)
			// then ignore the second raycast has returned a normal which is pointing
			// downwards or perpendicular to the raycast.
			if (cosAngle <= 0.0f)
			{
				secondRayHit = false;
			}
			else
			{
				// Construct a plane tangent to the hitpoint from the footEnd raycast
				// 
				hkVector4 ray;
				ray.setSub4( footEndToWS, footEndFromWS );
				hkVector4 footEndHitPointWS; footEndHitPointWS.setAddMul4(footEndFromWS, ray, footEndRayFraction);
				const hkReal d = -footEndHitPointWS.dot3(secondRayNormalWS);

				// And intersect the original ray (footFrom->footTo) with this plane
				hkVector4 footFromWS; footFromWS.setAddMul4( footWS, m_setup.m_worldUpDirectionWS, rayUpDistance );
				const hkReal Vd = - rayLength * cosAngle; // a minus because ray is -up
				const hkReal Vo = - ( hkReal(secondRayNormalWS.dot3(footFromWS)) + d );
				const hkReal t = Vo / Vd;
				secondRayFraction = t;
			}
		}
	}

	if (!firstRayHit && !secondRayHit)
	{
		return false;
	}
	else
	{
		hkReal shortestFraction;

		if (!firstRayHit || (firstRayFraction > secondRayFraction))
		{
			shortestFraction = secondRayFraction;
			groundNormalWSOut = secondRayNormalWS;
		}
		else
		{
			shortestFraction = firstRayFraction;
			groundNormalWSOut = firstRayNormalWS;
		}

		const hkReal verticalDistanceOut = shortestFraction * rayLength - rayUpDistance; // fraction is [0..1] and ray is [-rayUp, rayDown]

		// Projected point is the foot point + distance in the down direction (negative up)
		projectedFootWSOut.setAddMul4(footWS, m_setup.m_worldUpDirectionWS, - (verticalDistanceOut));

		return true;
	}
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
