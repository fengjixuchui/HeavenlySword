//--------------------------------------------------
//!
//!	\file game/entityinteractablerigidbody.h
//!	Definition of the Interactable Rigid Body entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_RIGID_BODY_H
#define	_ENTITY_INTERACTABLE_RIGID_BODY_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Interactable_RigidBody.
//! Base interactable rigid body entity type
//!
//--------------------------------------------------
class Interactable_RigidBody : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_RigidBody)

public:
	// Constructor
	Interactable_RigidBody();

	// Destructor
	~Interactable_RigidBody();

	// Post Construct
	void OnPostConstruct();

	bool GlobalHandlers(Message& msg);
	bool AttachedState(Message& msg);
	bool DefaultState(Message& msg);

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CHashedString	m_InitialState;

	// The animation container
	CKeyString	m_AnimationContainer;

	// Not sure of pointer type yet !!MB_ENT
	/*void* m_pSharedAttributesPtr;
	LuaAttributeTable* m_pSharedAttributes;*/
};


#endif // _ENTITY_INTERACTABLE_RIGID_BODY_H
