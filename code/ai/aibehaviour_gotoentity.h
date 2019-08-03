//! -------------------------------------------
//! aibehaviour_gotoentity.h
//!
//! STEER TO LOCATOR NODE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIGOTOENTITYBEHAVIOUR_H
#define _AIGOTOENTITYBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIGoToEntityBehaviour : public CAIStateMachine
{
public:

	CAIGoToEntityBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIGoToEntityBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::GOTOENTITY_DARIO;}


protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_MOVING,
		STATE_IDLE,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;

};

#endif // _AIGOTOENTITYBEHAVIOUR_H




