//------------------------------------------------------------------------------------------
//!
//!	\file aibehaviour_attack.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIBEHAVIOURATTACK_H
#define _AIBEHAVIOURATTACK_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIBehaviour_Attack
//!
//------------------------------------------------------------------------------------------


class AIBehaviour_Attack : public CAIStateMachine
{
public:

	AIBehaviour_Attack( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIBehaviour_Attack( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::ATTACK;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_ATTACK,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	bool			TooFarFromPlayer( void ) const;

	float			m_fTimer;

};

#endif //_AIBEHAVIOURATTACK_H
