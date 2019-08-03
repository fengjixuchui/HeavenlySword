//! -------------------------------------------
//! aibehavior_attacking.h
//!
//! CHASE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIBEHAVIOURATTACKING_H
#define _AIBEHAVIOURATTACKING_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIBehaviour_Attack
//!
//------------------------------------------------------------------------------------------


class CAIAttackBehaviour : public CAIStateMachine
{
public:

	CAIAttackBehaviour( AI* pobEnt ) : 
		CAIStateMachine(pobEnt), 
		m_obMoveTarget(CONSTRUCT_CLEAR), 
		m_obEnemyInitialPos(CONSTRUCT_CLEAR), 
		m_fTimer(0.0f),
		m_fUserTimer(0.0f),
		m_fNextTargetChangeTimer(0.0f),
		m_fTestFormationEntryTimer(0.0f),
		m_fRadius(0.0f),
		m_bDoIHoldARangedWeapon(false),
		m_bStrafingDueToMinRange(false),
		m_eDivingAction(0)
	{
		// This behaviour has code that requires updateing even when the entity is reacting to combat
		m_UpdateFlags |= FORCE_UPDATE_WHEN_COMBAT_RECOVERING_F;
	};

	~CAIAttackBehaviour( void );

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::ATTACK_DARIO;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_SELECT_ATACK_POINT,
		STATE_MOVING_TO_ATTACK_POINT,
		STATE_ATTACK_POINT_POINTREACHED,
		STATE_ATTACKING,
		STATE_IDLE,
		STATE_CHASING_TARGET,
		STATE_START_DIVING,
		STATE_WALK_TOWARDS_ENEMY,
		STATE_LOST_ENEMY,
		STATE_IN_SHOOTING_RANGE,
		STATE_IN_MELE_RANGE,
		STATE_WAIT_FOR_CS_STANDARD,
		STATE_WAIT_FOR_TIMER,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			SetUserTimer			( float f )			{ m_fUserTimer = f<0.0f ? 0.0f : f; }

	CPoint	m_obMoveTarget;
	CPoint	m_obEnemyInitialPos;
	float	m_fTimer;
	float	m_fUserTimer;

	// Test change target timer. 
	float	m_fNextTargetChangeTimer;

	// Timer that when expired will test to see whether this entity can enter formation
	float	m_fTestFormationEntryTimer;

	float	m_fRadius;
	bool	m_bDoIHoldARangedWeapon;
	bool	m_bStrafingDueToMinRange;
	unsigned int m_eDivingAction;

};

#endif //_AIBEHAVIOURATTACKING_H
