//! -------------------------------------------
//! aibehaviour_lookatentity.h
//!
//! Look At Entity behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#if 0

#ifndef _AILOOKATENTITYBEHAVIOUR_H
#define _AILOOKATENTITYBEHAVIOUR_H

#include "ai/aistatemachine.h"
#include "physics/Lookatcomponent.h"

class CAILookAtEntityBehaviour : public CAIStateMachine
{
public:

	CAILookAtEntityBehaviour( CEntity* pobEnt ) : CAIStateMachine(pobEnt), m_fTimer(0.0f), m_pobLookAtEntity(NULL), m_pLAC(NULL), 
												m_obEntityPos(CONSTRUCT_CLEAR), m_obFacing(CONSTRUCT_CLEAR), m_bEntityFound(false), 
												m_fTimeBetweenShots(0.0f), m_fShootingTimeThreshold(1.0f) {};
	~CAILookAtEntityBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::LOOKATENTITY_DARIO;}

protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_TURNTOFACE,
		STATE_ENTITY_NOT_FOUND,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float				m_fTimer;
	const CEntity*		m_pobLookAtEntity;
	LookAtComponent*	m_pLAC;
	CPoint				m_obEntityPos;
	CDirection			m_obFacing;
	bool				m_bEntityFound;
	float				m_fTimeBetweenShots;
	float				m_fShootingTimeThreshold;

};

#endif // _AILOOKATENTITYBEHAVIOUR_H


#endif
