//! -------------------------------------------
//! aibehavior_idle.h
//!
//! IDLE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIBEHAVIOURIDLE_H
#define _AIBEHAVIOURIDLE_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIIdle
//!
//------------------------------------------------------------------------------------------


class CAIIdleBehaviour : public CAIStateMachine
{
public:

	CAIIdleBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIIdleBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::IDLE_DARIO;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_IDLE,
		STATE_START_DIVING,
		STATE_PLAY_ANIM,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;

};

#endif // _AIBEHAVIOURIDLE_H


