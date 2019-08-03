//! -------------------------------------------
//! aibehaviour_playanim.h
//!
//! Play Animation behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIPLAYANIMBEHAVIOUR_H
#define _AIPLAYANIMBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIPlayAnimBehaviour : public CAIStateMachine
{
public:

	CAIPlayAnimBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIPlayAnimBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::PLAYANIM_DARIO;}

protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_PLAYINGANIM,
		STATE_COMPLETED,
		STATE_ANIM_FAILED,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;

};

#endif // _AIPLAYANIMBEHAVIOUR_H




