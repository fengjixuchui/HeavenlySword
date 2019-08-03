//! -------------------------------------------
//! aibehaviour_useobject.h
//!
//! USE OBJECT behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIUSEOBJECTBEHAVIOUR_H
#define _AIUSEOBJECTBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CEntity;

class CAIUseObjectBehaviour : public CAIStateMachine
{
public:

	CAIUseObjectBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt), m_fTimer(0.0f), m_fUserTimer(0.0f), m_bUseACannon(false) {};
	~CAIUseObjectBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::USEOBJECT_DARIO;}


protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_WAIT_FOR_OBJECT_AVAILABLE,
		STATE_USING_OBJECT,
		STATE_USING_CANNON,
		STATE_IDLE,
		STATE_WAIT_FOR_CS_STANDARD,
		STATE_WAIT_FOR_TIMER,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			SetUserTimer			( float f )			{ m_fUserTimer = f<0.0f ? 0.0f : f; }

	CEntity*	m_pobEntityToUse;
	float		m_fTimer;
	float		m_fUserTimer;
	bool		m_bUseACannon;
};

#endif // _AIUSEOBJECTBEHAVIOUR_H




