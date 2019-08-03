//! -------------------------------------------
//! aibehaviour_faceentity.h
//!
//! Face Entity behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIFACEENTITYBEHAVIOUR_H
#define _AIFACEENTITYBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIFaceEntityBehaviour : public CAIStateMachine
{
public:

	CAIFaceEntityBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt), m_fTimer(0.0f), m_pobFacingEntity(NULL),
												m_obEntityPos(CONSTRUCT_CLEAR), m_obFacing(CONSTRUCT_CLEAR), m_bEntityFound(false), 
												m_fTimeBetweenShots(0.0f), m_fShootingTimeThreshold(1.0f) {};
	~CAIFaceEntityBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::FACEENTITY_DARIO;}

protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_TURNTOFACE,
		STATE_ENTITY_NOT_FOUND,
		STATE_WAIT_FOR_CS_STANDARD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;
	const CEntity*	m_pobFacingEntity;
	CPoint			m_obEntityPos;
	CDirection		m_obFacing;
	bool			m_bEntityFound;
	float			m_fTimeBetweenShots;
	float			m_fShootingTimeThreshold;

};

#endif // _AIFACEENTITYBEHAVIOUR_H



