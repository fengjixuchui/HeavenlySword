//------------------------------------------------------------------------------------------
//!
//!	aiwalktopoint.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIWALKTOPOINT_H
#define _AIWALKTOPOINT_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIIdle
//!
//------------------------------------------------------------------------------------------


class AIWalkToPoint : public CAIStateMachine
{
public:

	AIWalkToPoint( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIWalkToPoint( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::RANDOMWALK;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_GO,
		STATE_AT_TARGET
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;
	CPoint			m_obDest;
	bool			m_bNeedToAcknowledge;

};

#endif
