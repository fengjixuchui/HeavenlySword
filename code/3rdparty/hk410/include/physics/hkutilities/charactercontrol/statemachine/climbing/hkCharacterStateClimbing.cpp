/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/charactercontrol/statemachine/hkCharacterState.h>
#include <hkutilities/charactercontrol/statemachine/hkCharacterContext.h>
#include <hkutilities/charactercontrol/statemachine/climbing/hkCharacterStateClimbing.h>

#include <hkdynamics/world/hkWorld.h>

#include <hkutilities/charactercontrol/statemachine/util/hkCharacterMovementUtil.h>

	// Return the state type
hkCharacterStateType hkCharacterStateClimbing::getType() const 
{
	return HK_CHARACTER_CLIMBING;
}
	// Process the user input - causes state transitions etc.
void hkCharacterStateClimbing::update(hkCharacterContext& context, const hkCharacterInput& input, hkCharacterOutput& output)
{
  	if (!input.m_atLadder)
	{
		context.setState(HK_CHARACTER_IN_AIR, input, output);
		return;
	}

	//
	// Check if I should jump
	//
	{
		if (input.m_wantJump)
		{
			// When climbing we'll simply push away from the ladder
			output.m_velocity.addMul4( input.m_characterGravity.length3(), input.m_surfaceNormal );
			context.setState( HK_CHARACTER_IN_AIR, input, output );
			return;
		}
	}

	//
	// Move character relative to the surface we're standing on
	//
	{
		hkCharacterMovementUtil::hkMovementUtilInput muInput;
		muInput.m_gain = 1.0f;
		muInput.m_forward = input.m_forward;
		muInput.m_up = input.m_surfaceNormal;
		muInput.m_surfaceNormal = input.m_surfaceNormal;
		muInput.m_currentVelocity = input.m_velocity;
		muInput.m_desiredVelocity.set( input.m_inputUD * 10, input.m_inputLR * 10, -0.1f ); //-0.1 attract toward the ladder
		muInput.m_maxVelocityDelta = 100.0f;
		muInput.m_surfaceVelocity = input.m_surfaceVelocity;

		hkCharacterMovementUtil::calculateMovement(muInput, output.m_velocity);

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
