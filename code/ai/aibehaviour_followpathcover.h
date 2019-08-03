//! -------------------------------------------------------------------------
//! aibehaviour_followpathcover.h
//!
//! MOVE TOWARDS THE ENEMY BY WAY-POINTS USING COVER behaviour for MELE AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------------------------------------

#ifndef _AIFOLLOWPATHBEHAVIOUR_H
#define _AIFOLLOWPATHBEHAVIOUR_H

#include "ai/aistatemachine.h"
#include "game/attacks.h"

class CAIFollowPathCoverBehaviour : public CAIStateMachine
{
public:

	CAIFollowPathCoverBehaviour( AI* pobEnt ) : CAIStateMachine(pobEnt), m_pEnemy(NULL), m_obLastEnemyPos(CONSTRUCT_CLEAR), 
												m_obValidCoverDirection(CONSTRUCT_CLEAR),
												m_fTimer(0.0f), m_fTimerEnemyPosCheck(0.0f), m_fInitTime(0.0f),
												m_fUserTimer (0.0f),
												m_uiInitialMaxSpeed(0.0f),m_uiInitialFlags(0), m_uiWalkCtrlId(0),
												m_bOneTimeWarning(false), m_bStrafingDueToMinRange(false), m_eDivingAction(0),
												m_fTimeInCover(0.0f), m_fCoverAnimDuration(0.0f) {};
	~CAIFollowPathCoverBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::FOLLOWPATHCOVER_DARIO;}


protected:

	enum STATES
	{
		STATE_ZERO,
		STATE_INITIALISE,
		STATE_MOVING,
		STATE_START_DIVING,
		STATE_WAIT_UNTIL_RECOVERED,
		STATE_MOVING_TO_COVER_POINT,
		STATE_TAKING_COVER,
		STATE_SCALED_TAKING_COVER,
		STATE_ADJUSTING_COVER_ANGLE,
		STATE_COVERING,
		STATE_PLAYING_PEEKING_ANIM,
		STATE_BREAKING_COVER,
		STATE_IDLE,
		STATE_PATH_FINISHED,
		STATE_WAIT_FOR_CS_STANDARD,
		STATE_WAIT_FOR_TIMER,
		STATE_QUEUING,
		STATE_MOVING_TO_UPDATED_QUEUE_POS,
		STATE_MOVING_TO_OBJECT_TO_USE,
		STATE_WAIT_FOR_OBJECT_AVAILABLE,
		STATE_USING_OBJECT,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			SetUserTimer			( float f )			{ m_fUserTimer = f<0.0f ? 0.0f : f; }
	bool			SnapAngle ( void );
	const CEntity*	m_pEnemy;
	CPoint			m_obLastEnemyPos;
	CDirection		m_obValidCoverDirection;

	float			m_fTimer;
	float			m_fTimerEnemyPosCheck;
	float			m_fInitTime;
	float			m_fUserTimer;
	float			m_uiInitialMaxSpeed;
	unsigned int	m_uiInitialFlags;

	uint64_t		m_uiWalkCtrlId;

	bool			m_bOneTimeWarning;
	bool			m_bStrafingDueToMinRange;
	unsigned int	m_eDivingAction;
	float			m_fTimeInCover;
	float			m_fCoverAnimDuration;
	

};

#endif // _AIFOLLOWPATHBEHAVIOUR_H






