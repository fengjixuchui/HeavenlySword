/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_CHARACTER_STATE
#define HK_CHARACTER_STATE

#include <hkbase/hkBase.h>

class hkCharacterContext;
struct hkCharacterInput;
struct hkCharacterOutput;

/// The character state types.
enum hkCharacterStateType
{
	// default states
	HK_CHARACTER_ON_GROUND = 0,
	HK_CHARACTER_JUMPING,
	HK_CHARACTER_IN_AIR,
	HK_CHARACTER_CLIMBING,
	HK_CHARACTER_FLYING,

	// user states
	HK_CHARACTER_USER_STATE_0,
	HK_CHARACTER_USER_STATE_1,
	HK_CHARACTER_USER_STATE_2,
	HK_CHARACTER_USER_STATE_3,
	HK_CHARACTER_USER_STATE_4,
	HK_CHARACTER_USER_STATE_5,

	HK_CHARACTER_MAX_STATE_ID
};


/// This class represents the behaviour for a typical character state.
/// The state machine is built implicitly from a collection of states.
/// Most of the real work is done in the update call where the state performs actions
/// and transitions to other states through a given character context.
class hkCharacterState : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CHARACTER);

			/// Return the state type
		virtual hkCharacterStateType getType() const = 0;

			/// Do something before we transition to this state
		virtual void enterState( hkCharacterContext& context, hkCharacterStateType prevState, const hkCharacterInput& input, hkCharacterOutput& output );

			/// Do something before we leave this state
		virtual void leaveState( hkCharacterContext& context, hkCharacterStateType nextState, const hkCharacterInput& input, hkCharacterOutput& output );

			/// Process the input - causes state transitions and actions
		virtual void update( hkCharacterContext& context, const hkCharacterInput& input, hkCharacterOutput& output ) = 0;

};

#endif // HK_CHARACTER_STATE

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
