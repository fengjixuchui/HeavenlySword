//! -------------------------------------------
//! aibehavior_followleader.h
//!
//! CHASE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIBEHAVIOURFOLLOWLEADER_H
#define _AIBEHAVIOURFOLLOWLEADER_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIBehaviour_Attack
//!
//------------------------------------------------------------------------------------------


class CAIFollowLeaderBehaviour : public CAIStateMachine
{
public:

	CAIFollowLeaderBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIFollowLeaderBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::FOLLOWLEADER_DARIO;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_FOLLOWING_ENTITY,
		STATE_IDLE,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;

};

#endif //_AIBEHAVIOURFOLLOWLEADER_H
