//! -------------------------------------------
//! aibehaviour_patrol.h
//!
//! PATROL behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIPATROLBEHAVIOUR_H
#define _AIPATROLBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIPatrolBehaviour : public CAIStateMachine
{
public:

	CAIPatrolBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt), m_bHasInitialReaction(false) {};
	~CAIPatrolBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::PATROL_DARIO;}


protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_WALK,
		STATE_IDLE,
		STATE_SPOTTED,
		STATE_ALERTED,
		STATE_SEENSOMETHING,
		STATE_RETURN,
		STATE_TURNTOFACE,
		STATE_MOVE_TO_REPORTER,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;
	
	CPoint			m_obEntityPos;
	bool			m_bEntityFound;
	CDirection		m_obFacing;
	int				m_AfterTurnState;
	bool			m_bHasInitialReaction;

};

#endif // _AIPATROLBEHAVIOUR_H




