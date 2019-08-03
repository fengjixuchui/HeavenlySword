//------------------------------------------------------------------------------------------
//!
//!	\file aibehaviour_combat.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIBEHAVIOURCOMBAT_INC
#define _AIBEHAVIOURCOMBAT_INC

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIBehaviour_LockedOn
//!
//------------------------------------------------------------------------------------------


class AIBehaviour_Combat : public CAIStateMachine
{
public:

	AIBehaviour_Combat(AI* ent) : CAIStateMachine(ent) {};
	~AIBehaviour_Combat() {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::COMBAT;}


private:

	enum STATES
	{
		COMBAT_CHOOSE_POSITION,
		COMBAT_MOVING_TO_POSITION,
		COMBAT_READY,
	};

	// Delay counter
	float	m_fAngle;

	// Movement speed
	float	m_fMovementSpeed;

	// Delay counter
	float	m_fCombatReadyDelay;

	// The next location for the entity
	CPoint	m_obDestPoint;

	virtual bool	States(const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange);
};

#endif //_AIBEHAVIOURCOMBAT_INC
