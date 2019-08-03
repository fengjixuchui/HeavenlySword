//! -------------------------------------------
//! aibehaviour_gotonode.h
//!
//! FOLLOW PATH TO NODE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIGOTONODEBEHAVIOUR_H
#define _AIGOTONODEBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIGoToNodeBehaviour : public CAIStateMachine
{
public:

	CAIGoToNodeBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIGoToNodeBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::GOTONODE_DARIO;}


protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_MOVING,
		STATE_WAIT_UNTIL_RECOVERED,
		STATE_IDLE,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;

};

#endif // _AIGOTONODEBEHAVIOUR_H

