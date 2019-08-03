/***************************************************************************************************
*
*	CLASS			CAIChase
*
*	DESCRIPTION		Chase behaviour FSM
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _AI_CHASE_H
#define _AI_CHASE_H

#include "ai/aistatemachine.h"

class CAIChase : public CAIStateMachine
{
public:

	CAIChase( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIChase( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::CHASE;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_ALERTING,
		STATE_ALERTED,
		STATE_CHASE_ENEMY,
		STATE_STAND_LOOKAT,
		STATE_STAND_TAUNT,
		STATE_WALK_FOLLOW,
		STATE_WALK_TAUNT,
		STATE_WAIT,
		STATE_WALK_TOWARDS_ENEMY,
		STATE_LOST_ENEMY,
		STATE_REACQUIRE_ENEMY,
		STATE_START_ATTACKING,
		STATE_MOVE_TO_TAUNT,
		STATE_TAUNT_ENEMY,
		STATE_TAUNT_LOOPBACK,
		STATE_IDLE,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	bool			AtLastPlayerPos() const;

	float			m_fTimer;
	bool			m_bPlayerLeftPsychicRadius;
	CPoint			m_obLastReachablePlayerPos;

};

#endif // _AI_CHASE_H
