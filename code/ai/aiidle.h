//------------------------------------------------------------------------------------------
//!
//!	aiidle.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIIDLE_H
#define _AIIDLE_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIIdle
//!
//------------------------------------------------------------------------------------------


class AIIdle : public CAIStateMachine
{
public:

	AIIdle( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIIdle( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::IDLE;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_IDLE,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			Walk( void );

	float			m_fTimer;

};

#endif
