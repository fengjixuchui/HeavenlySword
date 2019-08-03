//! -------------------------------------------
//! aibehaviour_whackamole.h
//!
//! Shoot on Spot and Cover behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIWHACKAMOLEBEHAVIOUR_H
#define _AIWHACKAMOLEBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAINavigNode;

class CAIWhackAMoleBehaviour : public CAIStateMachine
{
public:

	CAIWhackAMoleBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt), m_fTimer(0.0f), m_fUserTimer(0.0f), m_fInitTime(0.0f), 
												m_pobFacingEntity(NULL), 
												m_obEntityPos(CONSTRUCT_CLEAR), m_obFacing(CONSTRUCT_CLEAR), 
												m_obHiddingDirection(CONSTRUCT_CLEAR), m_SnapPoint(CONSTRUCT_CLEAR),
												m_bEntityFound(false), 
												m_fTimeBetweenShots(0.0f), m_iVolleyCount(-1), m_fHiddingTime(0.0f),
												m_pTCNode(NULL) {};
	~CAIWhackAMoleBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::WHACKAMOLE_DARIO;}

	static CEntity* GetVolleyCommander(int iSquad);

protected:

	enum STATES
	{
		STATE_ZERO,
		STATE_INITIALISE,
		STATE_TURNTOFACE_AND_AIM,
		STATE_SHOOT,
		STATE_FINISH_SHOOTING_ANIM,
		STATE_TAKING_COVER,
		STATE_SNAP,
		STATE_CROUCH_DOWN,
		STATE_CROUCHING,
		STATE_STAND_UP,
		STATE_RELOADING,
		STATE_HIDDEN,
		STATE_BREAK_COVER,
		STATE_IDLE_PRESHOOTING_ACTION,
		STATE_RETURN_TO_COVER,
		STATE_NO_BREAK_COVER_ANIMS,
		STATE_NO_RETURN_TO_COVER_ANIMS,
		STATE_ENTITY_NOT_FOUND,
		STATE_WAIT_FOR_CS_STANDARD,
		STATE_WAIT_FOR_TIMER,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	bool			SnapAngle				( void );
	void			SetUserTimer			( float f )			{ m_fUserTimer = f<0.0f ? 0.0f : f; }
	float			m_fTimer;
	float			m_fUserTimer;
	float			m_fInitTime;
	const CEntity*	m_pobFacingEntity;
	CPoint			m_obEntityPos;
	CDirection		m_obFacing;
	CDirection		m_obHiddingDirection;
	CPoint			m_SnapPoint;
	bool			m_bEntityFound;
	float			m_fTimeBetweenShots;
	int				m_iVolleyCount;
	float			m_fHiddingTime;
	CAINavigNode*	m_pTCNode;
	float			m_fAimingTime;

};

#endif // _AIWHACKAMOLEBEHAVIOUR_H



