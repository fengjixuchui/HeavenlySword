/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_CHARACTER_STATE_FLYING
#define HK_CHARACTER_STATE_FLYING

#include <hkutilities/charactercontrol/statemachine/hkCharacterState.h>

/// A state to represent flying.
/// This state is currently unused - no other states transition to it.
class hkCharacterStateFlying : public hkCharacterState
{
	public:

		hkCharacterStateFlying() {}

			/// Return the state type
		virtual hkCharacterStateType getType() const;

			/// Process the user input - causes state transitions etc.
		virtual void update(hkCharacterContext& context, const hkCharacterInput& input, hkCharacterOutput& output);
};

#endif // HK_CHARACTER_STATE_FLYING

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
