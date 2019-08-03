//--------------------------------------------------
//!
//!	\file game/entityragdoll.h
//!	Definition of the ragdoll entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_RAGDOLL_H
#define	_ENTITY_RAGDOLL_H

#include "game/entityinteractable.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Object_Ragdoll.
//! Ragdoll object.
//!
//--------------------------------------------------
class Object_Ragdoll : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Ragdoll)

public:
	// Constructor
	Object_Ragdoll();

	// Destructor
	~Object_Ragdoll();

	// Post Construct
	void OnPostConstruct();

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CKeyString	m_InitialState;

	// The Ragdoll Clump
	ntstd::String		m_RagdollClump;
};


#endif // _ENTITY_RAGDOLL_H
