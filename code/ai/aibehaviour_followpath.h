//! -------------------------------------------
//! aibehaviour_followpath.h
//!
//! PATROL behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIFOLLOWPATHBEHAVIOUR_H
#define _AIFOLLOWPATHBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIMovement;

class CAIFollowPathAndCoverBehaviour : public CAIStateMachine
{
public:

	CAIFollowPathAndCoverBehaviour( CEntity* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIFollowPathAndCoverBehaviour( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::PATROL_DARIO;}


protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_MOVING,
		STATE_MOVING_TO_COVER_POINT,
		STATE_TAKING_COVER,
		STATE_COVERING,
		STATE_PLAYING_PEEKING_ANIM,
		STATE_BREAKING_COVER,
		STATE_IDLE,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;
	
	unsigned int	m_uiPathMovFlags;

};

#endif // _AIFOLLOWPATHBEHAVIOUR_H






