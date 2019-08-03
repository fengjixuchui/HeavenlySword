//--------------------------------------------------
//!
//!	\file game/entitykite.h
//!	Definition of the kite entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_KITE_H
#define	_ENTITY_KITE_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"

class CAnimation;

//--------------------------------------------------
//!
//! Class Object_Kite.
//! Ninja's kite entity type
//!
//--------------------------------------------------
class Object_Kite : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Kite)

public:
	// Constructor
	Object_Kite();

	// Destructor
	~Object_Kite();

	// Post Construct
	void OnPostConstruct();

	bool GlobalHandlers(Message& msg);
	bool AttachedState(Message& msg);
	bool DefaultState(Message& msg);

	// Attributes accessable to the FSM
	Character*		m_pOther;

	CAnimation*		m_pobAnim;

	CHashedString	m_obAnimationKiteFlight;
	//CHashedString	m_obAnimationKiteCrash;
	//CHashedString	m_obAnimationKiteRelease;
	CHashedString	m_obAnimationKiteLeave;

	CHashedString	m_obAnimationNinjaFlight;
	//CHashedString	m_obAnimationNinjaCrash;
	CHashedString	m_obAnimationNinjaRelease;
	//CHashedString	m_obAnimationNinjaFall;
	//CHashedString	m_obAnimationNinjaLand;
	CHashedString	m_obUserTransform;

	CPoint			m_obUserPosition;
	CPoint			m_obUserRotation;

	CDirection		m_obAngularVel;
	CPoint			m_obSpiralCentre;
	float			m_obVerticalVel;

	float			m_fMinTurnRadius;

	// FIX ME - Localy calculating velocity as Velocity of animated LG incorrect from havok.  Remove this when we have Havok 4
	CPoint			m_obPrevPos;
	double			m_dPrevTime;

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CHashedString	m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;	
};


#endif // _ENTITY_KITE_H
