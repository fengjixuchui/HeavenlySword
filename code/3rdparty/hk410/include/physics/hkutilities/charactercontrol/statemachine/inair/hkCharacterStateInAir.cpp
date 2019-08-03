/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/charactercontrol/statemachine/hkCharacterState.h>
#include <hkutilities/charactercontrol/statemachine/hkCharacterContext.h>
#include <hkutilities/charactercontrol/statemachine/inair/hkCharacterStateInAir.h>

#include <hkutilities/charactercontrol/statemachine/util/hkCharacterMovementUtil.h>

#include <hkdynamics/world/hkWorld.h>

hkCharacterStateInAir::hkCharacterStateInAir()
{
	m_gain = 0.05f; 

	m_airSpeed = 10.0f;
}

	// Return the state type
hkCharacterStateType hkCharacterStateInAir::getType() const 
{
	return HK_CHARACTER_IN_AIR;
}

	// Process the user input - causes state transitions etc.
void hkCharacterStateInAir::update(hkCharacterContext& context, const hkCharacterInput& input, hkCharacterOutput& output)
{
	//
	// Grab details about the surface I'm currently on
	//
	if (input.m_isSupported)
	{
		context.setState(HK_CHARACTER_ON_GROUND, input, output);
		return;
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
	// Move character relative to the surface we're standing on
	//
	{
		hkCharacterMovementUtil::hkMovementUtilInput muInput;
		muInput.m_gain = m_gain;
		muInput.m_forward = input.m_forward;
		muInput.m_up = input.m_up;
		muInput.m_surfaceNormal = input.m_up;
		muInput.m_currentVelocity = input.m_velocity;
		muInput.m_desiredVelocity.set( input.m_inputUD * m_airSpeed, input.m_inputLR * m_airSpeed, 0); 
		muInput.m_maxVelocityDelta = 100.0f;
		muInput.m_surfaceVelocity = input.m_surfaceVelocity;

		hkCharacterMovementUtil::calculateMovement(muInput, output.m_velocity);

		// Restore to original vertical component
		output.m_velocity.addMul4(-output.m_velocity.dot3(input.m_up), input.m_up);
		output.m_velocity.addMul4(  input.m_velocity.dot3(input.m_up) , input.m_up);

		output.m_velocity.addMul4(input.m_stepInfo.m_deltaTime, input.m_characterGravity );
	}
}

hkReal hkCharacterStateInAir::getGain()
{
	return m_gain;
}

void hkCharacterStateInAir::setGain(hkReal newGain)
{
	m_gain = newGain;
}

hkReal hkCharacterStateInAir::getSpeed()
{
	return m_airSpeed;
}

void hkCharacterStateInAir::setSpeed(hkReal newSpeed)
{
	m_airSpeed = newSpeed;
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
