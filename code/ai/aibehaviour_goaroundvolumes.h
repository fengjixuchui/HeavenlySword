//! -------------------------------------------
//! aibehaviour_goaroundvolumes.h
//!
//! GO AROUND VOLUMES behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIGOAROUNDVOLUMESBEHAVIOUR_H
#define _AIGOAROUNDVOLUMESBEHAVIOUR_H

#include "ai/aistatemachine.h"

class CAIGoAroundVolumesBehaviour : public CAIStateMachine
{
public:

	CAIGoAroundVolumesBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt), m_pEnemy(NULL), m_obLastEnemyPos(CONSTRUCT_CLEAR),
												m_fTimer(0.0f), m_fTimerEnemyPosCheck(0.0f), m_fInitTime(0.0f),
												m_fUserTimer (0.0f),
												m_uiInitialMaxSpeed(0.0f),m_uiInitialFlags(0), m_uiWalkCtrlId(0),
												m_bOneTimeWarning(false) {};
	~CAIGoAroundVolumesBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::GOAROUNDVOLUMES_DARIO;}
	

protected:

	enum STATES
	{
		STATE_ZERO,
		STATE_INITIALISE,
		STATE_MOVING,
		STATE_WAIT_UNTIL_RECOVERED,
		STATE_IDLE,
		STATE_PATH_FINISHED,
		STATE_WAIT_FOR_CS_STANDARD,
		STATE_WAIT_FOR_TIMER,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			SetUserTimer			( float f )			{ m_fUserTimer = f<0.0f ? 0.0f : f; }

	const CEntity*	m_pEnemy;
	CPoint			m_obLastEnemyPos;
	
	float			m_fTimer;
	float			m_fTimerEnemyPosCheck;
	float			m_fInitTime;
	float			m_fUserTimer;
	float			m_uiInitialMaxSpeed;
	unsigned int	m_uiInitialFlags;

	uint64_t		m_uiWalkCtrlId;

	bool			m_bOneTimeWarning;

};

#endif // _AIGOAROUNDVOLUMESBEHAVIOUR_H

