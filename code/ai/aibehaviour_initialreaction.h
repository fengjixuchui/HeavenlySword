//! -------------------------------------------
//! aibehaviour_initialreaction.h
//!
//! Initial Reaction behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIINITIALREACTIONBEHAVIOUR_H
#define _AIINITIALREACTIONBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIInitialReactionBehaviour : public CAIStateMachine
{
public:

	CAIInitialReactionBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt), m_fUserTimer(0.0f) {};
	~CAIInitialReactionBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::INITIALREACTION_DARIO;}

protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_PLAYING_IDLE_ANIM,
		STATE_COMPLETED_IDLE_ANIM,
		STATE_SOUND_ALERT,
		STATE_SPOTTER,
		STATE_WAIT_FOR_FINISHED_SPOTTER,
		STATE_WAIT_FOR_RESPONSE_ANIM_FINISHED,
		STATE_WAIT_FOR_FINISHED_SOUND_SPOTTER,
		STATE_IDLE,
		STATE_ANIM_FAILED,
		STATE_WAIT_FOR_TIMER,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			SetUserTimer			( float f )			{ m_fUserTimer = f<0.0f ? 0.0f : f; }


	float			m_fTimer;
	float			m_fUserTimer;
};

#endif // _AIINITIALREACTIONBEHAVIOUR_H




