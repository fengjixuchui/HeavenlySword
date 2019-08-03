//! -------------------------------------------
//! aibehaviour_investigate.h
//!
//! INVESTIGATE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _INVESTIGATEBEHAVIOUR_H
#define _INVESTIGATEBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIInvestigateBehaviour : public CAIStateMachine
{
public:

	CAIInvestigateBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIInvestigateBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::INVESTIGATE_DARIO;}

protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_MOVING_TO_POINT,
		STATE_IDLE,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;

};

#endif // _INVESTIGATEBEHAVIOUR_H




