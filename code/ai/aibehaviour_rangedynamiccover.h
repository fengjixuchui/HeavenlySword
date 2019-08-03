//! ------------------------------------------------------------------------------------
//! aibehaviour_rangedynamiccover.h
//!
//! KEEP WITHIN CERTAIN DISTANCE (USING WAY-POINTS) FROM THE ENEMY   
//! , TAKE COVER AND SHOOT behaviour for RANGE AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIRANGEDYNCOVERBEHAVIOUR_H
#define _AIRANGEDYNCOVERBEHAVIOUR_H

#include "ai/aistatemachine.h"
#include "game/attacks.h"

class CAIVision;

class CAIRangeDynamicCoverBehaviour : public CAIStateMachine
{
public:

	CAIRangeDynamicCoverBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt), m_fTimer(0.0f), m_fUserTimer(0.0f), 
													m_fTimerEnemyPosCheck(0.0f), fInitTime(0.0f),
													fTimeBeetweenShoots(0.0f), fTimeBetweenShootsMoving(0.0f),
													m_obLastEnemyPos(CONSTRUCT_CLEAR),
													m_bFarFromMaxRange(false), m_bCloserThanMinRange(false), m_pVis(NULL),
													m_obValidCoverDirection(CONSTRUCT_CLEAR), m_fTimeInCover(0.0f), m_fCoverAnimDuration(0.0f),
													m_iVolleyCount(-1), m_fVolleyTimeBtwShots(0.0f), m_bInPauseBtwShots(false),
													m_pCommander(NULL), m_iLostLOS(0), m_iRejections(0) {};
	~CAIRangeDynamicCoverBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::RANGEDYNAMICCOVER_DARIO;}


protected:

	enum STATES
	{
		STATE_ZERO,
		STATE_INITIALISE,
		STATE_MOVING,
		STATE_WAIT_UNTIL_RECOVERED,
		STATE_MOVING_TO_COVER_POINT,
		STATE_TAKING_COVER,
		STATE_COVERING,
		STATE_PLAYING_PEEKING_ANIM,
		STATE_VOLLEY_COMMANDER_AIM,
		STATE_VOLLEY_COMMANDER_FIRE,
		STATE_VOLLEY_READY,
		STATE_VOLLEY_AIM,
		STATE_VOLLEY_SHOOTING,
		STATE_SHOOTING,
		STATE_BREAKING_COVER,
		STATE_IDLE,
		STATE_PATH_FINISHED_FACING_ENEMY,
		STATE_SCALED_TAKING_COVER,
		STATE_ADJUSTING_COVER_ANGLE,
		STATE_WAIT_FOR_CS_STANDARD,
		STATE_WAIT_FOR_TIMER,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	bool			FaceEnemy ( void );
	void			SetUserTimer			( float f )			{ m_fUserTimer = f<0.0f ? 0.0f : f; }
	float			m_fTimer;
	float			m_fUserTimer;
	float			m_fTimerEnemyPosCheck;
	float			fInitTime;
	float			fTimeBeetweenShoots;
	float			fTimeBetweenShootsMoving;
	
	CPoint			m_obLastEnemyPos;

	bool			m_bFarFromMaxRange;
	bool			m_bCloserThanMinRange;

	CAIVision*		m_pVis;

	CDirection		m_obValidCoverDirection;
	float			m_fTimeInCover;
	float			m_fCoverAnimDuration;
	int				m_iVolleyCount;
	float			m_fVolleyTimeBtwShots;
	bool			m_bInPauseBtwShots;
	CEntity*		m_pCommander;
	int				m_iLostLOS;
	int				m_iRejections;

};

#endif // _AIRANGEDYNCOVERBEHAVIOUR_H






