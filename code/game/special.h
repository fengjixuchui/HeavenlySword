/***************************************************************************************************
*
*	DESCRIPTION		Deals with the management of special moves
*
*	NOTES
*
***************************************************************************************************/

#ifndef _SPECIAL_H
#define _SPECIAL_H

// Necessary inlcudes
#include "game/entity.h"
#include "game/entity.inl"
#include "editable/enumlist.h"
#include "input/inputhardware.h"
#include "game/inputcomponent.h"
#include "effect/effect_special.h"
#include "game/entitymanager.h"
#include "game/query.h"

/***************************************************************************************************
*
*	CLASS			SpecialDef
*
*	DESCRIPTION		This is a global bit of data for tweaking the parameters of a special move.
*
***************************************************************************************************/
class SpecialDef
{
public:

	// Construction destruction
	SpecialDef( void );
	virtual ~SpecialDef( void );

	// Check the data after serialisation
	virtual void PostConstruct( void );

	// What is the minimum duration of a special
	float m_fSpecialDuration;

	// How long is the deceleration to slow speed
	float m_fStartingTime;

	// How long does it take for the speed to pick up after the special
	float m_fFinishingTime;

	// What is the special speed multiplier
	float m_fSpecialSpeedFactor;

	float m_fSpeedSpecialTargetDistance, m_fSpeedSpecialTargetAngle;

	float m_fPowerSpecialDamageMultiplier;

	float m_fRangeSpecialDamageMultiplier, m_fRangeSpecialAttackSpeedMultiplier;
};


/***************************************************************************************************
*
*	CLASS			AttackSpecial
*
*	DESCRIPTION		Deals with the implementation of the details associated with special moves
*
***************************************************************************************************/
class AttackSpecial
{
private:

	// What state are we currently in
	enum SPECIAL_STATES
	{
		SPECIAL_STATE_IDLE,
		SPECIAL_STATE_STARTING,
		SPECIAL_STATE_SPECIAL,
		SPECIAL_STATE_FINISHING
	};

public:

	// Construction destruction
	AttackSpecial( CEntity* pobParentEntity, const SpecialDef* pobSpecialDef );

	// This updates us
	void	Update( float fTimeStep );

	// Kick off a special - this is now a one shot thing - true if successful
	bool	StartSpecial( STANCE_TYPE eStance );

	// Is this special currently active
	bool	IsActive( void ) const { return m_eState != SPECIAL_STATE_IDLE; }

	float GetAttackDamageMultiplier();
	float GetAttackSpeedMultiplier();
	STANCE_TYPE GetSpecialStance() { return m_eSpecialStance; };
	void SetSpecialStance(STANCE_TYPE eSpecialStance);

	// Helpers to set the time multiplier of all other entities 
	// - also called from combat when doing 'extra' attacks
	void SetEntityTimeMultipliers( float fMultiplier );
	void ClearEntityTimeMultipliers( void );

private:

	// A helper to change between states
	void SetState( SPECIAL_STATES eNewState );

	// The definition of what we are up to 
	const SpecialDef* m_pobDef;

	// A pointer to our parent entity
	CEntity* m_pobParent;

	// what are we doing?
	SPECIAL_STATES	m_eState;

	// How long have we been in our current state?
	float m_fDurationInCurrentState;

	// List that'll hold all entites we've slowed down
	ntstd::List<CEntity*> m_obSlowedEntities;

	void FillTargetListForSlowDown();

	STANCE_TYPE m_eSpecialStance;
};

#endif // _SPECIAL_H
