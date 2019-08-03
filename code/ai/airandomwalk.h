//------------------------------------------------------------------------------------------
//!
//!	airandomwalk.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIRANDOMWALK_H
#define _AIRANDOMWALK_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIIdle
//!
//------------------------------------------------------------------------------------------


class AIRandomWalk : public CAIStateMachine
{
public:

	AIRandomWalk( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIRandomWalk( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::RANDOMWALK;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_GO,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;
	CPoint			m_obDest;

};

#endif
