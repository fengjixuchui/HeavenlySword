//------------------------------------------------------------------------------------------
//!
//!	\file aibehaviour_lockon.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIBEHAVIOURLOCKON_INC
#define _AIBEHAVIOURLOCKON_INC

#include "ai/aistatemachine.h"

class CEntity;

//------------------------------------------------------------------------------------------
//!
//!	AIBehaviour_LockedOn
//!
//------------------------------------------------------------------------------------------


class AIBehaviour_LockOn : public CAIStateMachine
{
public:

	AIBehaviour_LockOn(AI* pobEnt, CEntity& target) 
		: CAIStateMachine(pobEnt), m_target(target) {m_fRangeSqrd = 25.0f;}
	~AIBehaviour_LockOn() {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::LOCKON;}


// States
private:
	enum STATES
	{
		LOCKON_INITIAL
	};

	virtual bool	States(const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeDelta);

// Members
private:
	CEntity& m_target;
	float    m_fRangeSqrd;

};

#endif //_AIBEHAVIOURLOCKON_INC
