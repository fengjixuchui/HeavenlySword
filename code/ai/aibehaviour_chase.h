//! -------------------------------------------
//! aibehavior_chase.h
//!
//! CHASE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIBEHAVIOURCHASE_H
#define _AIBEHAVIOURCHASE_H

#include "ai/aistatemachine.h"

class CAIChaseBehaviour : public CAIStateMachine
{
public:

	CAIChaseBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIChaseBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::CHASEPLAYER_DARIO;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_CHASE_ENEMY,
		STATE_WALK_TOWARDS_ENEMY,
		STATE_LOST_ENEMY,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;

};



#endif // _AIBEHAVIOURCHASE_H


