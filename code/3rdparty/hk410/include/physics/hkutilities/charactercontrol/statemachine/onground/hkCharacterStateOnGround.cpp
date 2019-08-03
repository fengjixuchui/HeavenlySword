/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/charactercontrol/statemachine/hkCharacterState.h>
#include <hkutilities/charactercontrol/statemachine/hkCharacterContext.h>
#include <hkutilities/charactercontrol/statemachine/onground/hkCharacterStateOnGround.h>
#include <hkutilities/charactercontrol/statemachine/util/hkCharacterMovementUtil.h>

#include <hkdynamics/world/hkWorld.h>

#include <hkvisualize/hkDebugDisplay.h>

hkCharacterStateOnGround::hkCharacterStateOnGround()
{
	m_gain = 1.f;

	m_walkSpeed = 10.0f;

	m_killVelocityOnLaunch = true;

	m_limitVerticalVelocity = false;

	m_disableHorizontalProjection = false;
}

	// Return the state type
hkCharacterStateType hkCharacterStateOnGround::getType() const 
{
	return HK_CHARACTER_ON_GROUND;
}

	// Process the user input - causes state transitions etc.
void hkCharacterStateOnGround::update(hkCharacterContext& context, const hkCharacterInput& input, hkCharacterOutput& output)
{

	//
	// Check if I should jump
	//
	{
		if (input.m_wantJump)
		{
			context.setState(HK_CHARACTER_JUMPING, input, output);
			return;
		}
	}

	//
	// Check if I should climb
	{
		if (input.m_atLadder)
		{
			context.setState(HK_CHARACTER_CLIMBING, input, output);
			return;
		}
	}

	//
	// Check that I'm supported
	//	
	if (!input.m_isSupported)
	{
		if (m_killVelocityOnLaunch)
		{
			// Suddenly the character is no longer supported.
			// Remove all vertical velocity if required.
			{
				// Remove velocity in vertical component 
				output.m_velocity.subMul4(input.m_velocity.dot3(input.m_up), input.m_up);
			}
		}
		context.setState(HK_CHARACTER_IN_AIR, input, output);
		return;
	}

	//
	// Move character relative to the surface we're standing on
	//
	{
		hkCharacterMovementUtil::hkMovementUtilInput muInput;
		muInput.m_gain = m_gain;
		muInput.m_forward = input.m_forward;
		muInput.m_up = input.m_up;
		muInput.m_surfaceNormal = input.m_surfaceNormal;
		muInput.m_currentVelocity = input.m_velocity;
		muInput.m_desiredVelocity.set( input.m_inputUD * m_walkSpeed, input.m_inputLR * m_walkSpeed, 0 ); 
		muInput.m_maxVelocityDelta = 100.0f;
		muInput.m_surfaceVelocity = input.m_surfaceVelocity;


		// This will stick the character to the surface
		hkCharacterMovementUtil::calculateMovement(muInput, output.m_velocity);

		// Do not let the character fall faster than gravity
		if ( m_limitVerticalVelocity )
		{
			hkReal maxGravityVelocityInc = static_cast<hkReal>(input.m_characterGravity.dot3(input.m_up)) * input.m_stepInfo.m_deltaTime;
			hkReal actualVelocityInc = output.m_velocity.dot3(input.m_up) - input.m_velocity.dot3(input.m_up);

			if ((output.m_velocity.dot3(input.m_up) < 0) && (actualVelocityInc < maxGravityVelocityInc ))
			{
				output.m_velocity.addMul4(maxGravityVelocityInc - actualVelocityInc, input.m_up);
			}
		}
	}

	if (!m_disableHorizontalProjection)
	{

		output.m_velocity.sub4(input.m_surfaceVelocity);
		if (output.m_velocity.dot3(input.m_up) > .001f)
		{
			HK_ASSERT2(0x3f96452a, input.m_surfaceNormal.dot3(input.m_up) > HK_REAL_EPSILON,
				"Surface is vertical and the character is in the walk state.");

			hkReal velLen = output.m_velocity.normalizeWithLength3();

				// Get the desired length in the horizontal direction
			hkReal horizLen = velLen / static_cast<hkReal>(input.m_surfaceNormal.dot3(input.m_up));
				
				// Re project the velocity onto the horizontal plane
			hkVector4 c; c.setCross(input.m_surfaceNormal, output.m_velocity);
			output.m_velocity.setCross(c, input.m_up);

				// Scale the velocity to maintain the speed on the slope
			output.m_velocity.mul4(horizLen);
		}
		output.m_velocity.add4(input.m_surfaceVelocity);
	}


	// HVK-1362 : The following code is also used in the inAir state.
	// Its purpose is to allow gravity to affect the character by stopping
	// the hkCharacterMovementUtil culling velocity in the direction of m_up.
	// However leaving this code in means that characters can start sliding
	// down gentle slopes. 
	// The more common use case is that a character should not slide at all 
	// on slopes that are shallow enough to support them. Therefore the 
	// following lines have been commented out.
	
	// Restore to original vertical component
	//output.m_velocity.addMul4(-output.m_velocity.dot3(input.m_up), input.m_up);
	//output.m_velocity.addMul4(  input.m_velocity.dot3(input.m_up) , input.m_up);
}

hkReal hkCharacterStateOnGround::getGain()
{
	return m_gain;
}

void hkCharacterStateOnGround::setGain(hkReal newGain)
{
	m_gain = newGain;
}

hkReal hkCharacterStateOnGround::getSpeed()
{
	return m_walkSpeed;
}

void hkCharacterStateOnGround::setSpeed(hkReal newSpeed)
{
	m_walkSpeed = newSpeed;
}

hkBool hkCharacterStateOnGround::getGroundHugging()
{
	return m_killVelocityOnLaunch;
}

void hkCharacterStateOnGround::setGroundHugging(hkBool newVal)
{
	m_killVelocityOnLaunch = newVal;
}

hkBool hkCharacterStateOnGround::getDisableHorizontalProjection() const
{
	return m_disableHorizontalProjection;
}

void hkCharacterStateOnGround::setDisableHorizontalProjection( hkBool newVal )
{
	m_disableHorizontalProjection = newVal;
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
