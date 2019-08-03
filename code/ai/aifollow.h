//------------------------------------------------------------------------------------------
//!
//!	aifollow.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFOLLOW_H
#define _AIFOLLOW_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIIdle
//!
//------------------------------------------------------------------------------------------


class AIFollow : public CAIStateMachine
{
public:

	AIFollow( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIFollow( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::FOLLOW;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_GO,
		STATE_NEWDEST,
		STATE_WAIT
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;
	CEntity*		m_pobTarget;
	CPoint			m_obDest;

};

#endif
