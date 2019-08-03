//------------------------------------------------------------------------------------------
//!
//!	\file aiplayanim.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIPLAYANIM_H
#define _AIPLAYANIM_H

#include "aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIPlayAnim
//!
//------------------------------------------------------------------------------------------


class AIPlayAnim : public CAIStateMachine
{
public:

	AIPlayAnim( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIPlayAnim( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::PLAYANIM;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_PLAYANIM,
		STATE_PLAYINGANIM,
		STATE_STOPANDNOTIFY,
		STATE_COMPLETED,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;

};

#endif //_AIGETWEAPON_H
