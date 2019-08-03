/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/charactercontrol/statemachine/hkCharacterState.h>
#include <hkutilities/charactercontrol/statemachine/hkCharacterStateManager.h>
#include <hkutilities/charactercontrol/statemachine/hkCharacterContext.h>

hkCharacterContext::hkCharacterContext(const hkCharacterStateManager* Manager, hkCharacterStateType initialState)
: m_stateManager(Manager) , m_currentState(initialState)
{
	m_stateManager->addReference();
}

hkCharacterContext::~hkCharacterContext()
{
	m_stateManager->removeReference();
}

hkCharacterStateType hkCharacterContext::getState() const
{
	return m_currentState;
}

void hkCharacterContext::setState(hkCharacterStateType state, const hkCharacterInput& input, hkCharacterOutput& output )
{
	hkCharacterStateType prevState = m_currentState;

	HK_ASSERT2(0x6d4abe44, m_stateManager->getState(state) != HK_NULL , "Bad state transition to " << state << ". Has this state been registered?");

	// Leave the old state
	m_stateManager->getState(m_currentState)->leaveState(*this, state, input, output);

	// Transition to the new state
	m_currentState = state;

	// Enter the new state
	m_stateManager->getState(m_currentState)->enterState(*this, prevState, input, output);

}

void hkCharacterContext::update(const hkCharacterInput& input, hkCharacterOutput& output)
{
	// Give sensible initialized values for output;
	output.m_velocity = input.m_velocity;

	m_stateManager->getState(m_currentState)->update(*this, input, output);
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
