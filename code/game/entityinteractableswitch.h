//--------------------------------------------------
//!
//!	\file game/entityinteractableswitch.h
//!	Definition of the Interactable switch object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_SWITCH_H
#define	_ENTITY_INTERACTABLE_SWITCH_H

#include "game/entityinteractable.h"
#include "game/entityinteractable.h"

//--------------------------------------------------
//!
//! Class Interactable.
//! Base interactable entity type
//!	These won't go in level, only provide a unified interface for interactive objects
//--------------------------------------------------
class Interactable_Switch_Trigger : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Switch_Trigger)

public:
	//Constructor
	Interactable_Switch_Trigger();
	//Destructor
	~Interactable_Switch_Trigger();

	//Accessor (as on/off switch or normalized completion value).
	bool GetStateValBool() { return (m_fStateValue == 0.0f) ? false : true; }
	float GetStateValFloat() { if (m_fStateValue > 1.0f) return 1.0f; if (m_fStateValue < 0.0f) return 0.0f; return m_fStateValue; }

	// Have to be public or the FSM can't do anything with them
	float m_fStateValue;

protected:
	//Object description
	CKeyString m_Description;
};


#endif // _ENTITY_INTERACTABLE_SWITCH_H

