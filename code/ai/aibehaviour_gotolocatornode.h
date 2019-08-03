//! -------------------------------------------
//! aibehaviour_gotolocatornode.h
//!
//! STEER TO LOCATOR NODE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIGOTOLOCATORNODEBEHAVIOUR_H
#define _AIGOTOLOCATORNODEBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIGoToLocatorNodeBehaviour : public CAIStateMachine
{
public:

	CAIGoToLocatorNodeBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIGoToLocatorNodeBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::GOTOLOCATORNODE_DARIO;}


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

#endif // _AIGOTOLOCATORNODEBEHAVIOUR_H




