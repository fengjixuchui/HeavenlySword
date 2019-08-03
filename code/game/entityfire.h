//--------------------------------------------------
//!
//!	\file game/entityfire.h
//!	Definition of the fire entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_FIRE_H
#define	_ENTITY_FIRE_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"


//--------------------------------------------------
//!
//! Class Object_Fire.
//! A fire object type - particle system with some collision checks
//!
//--------------------------------------------------
class Object_Fire : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Fire)

public:
	// Constructor
	Object_Fire();

	// Destructor
	~Object_Fire();

	// Post Construct
	void OnPostConstruct();

	// Public variables so the FSM can get access to them
	CPoint		m_obParticleFXPosition;

	// Position and radius
	CPoint		m_obPosition;
	float		m_fRadius;

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	bool		m_bActive;

	// Particle effect for the fire
	CKeyString	m_FireParticleFX;
};


#endif // _ENTITY_FIRE_H
