//--------------------------------------------------
//!
//!	\file game/entityhittrigger.h
//!	Definition of the hit-trigger entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_HITTRIGGER_H
#define	_ENTITY_HITTRIGGER_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Object_HitTrigger.
//! Ragdoll object.
//!
//--------------------------------------------------
class Object_HitTrigger : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_HitTrigger)

public:
	// Constructor
	Object_HitTrigger();

	// Destructor
	~Object_HitTrigger();

	// Post Construct
	void OnPostConstruct();

	//Accessors.
	LuaAttributeTable* GetSharedAttributes(void) { return m_pSharedAttributes; }

	// Public variables for FSM to see
    // String to look for on the attribute table.
	CKeyString	m_bProjectileAttribForSpecificMessage;

	// Target entity to send the specific message to
	CEntity*	m_pobSpecificMessageTarget;

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CKeyString	m_InitialState;

	// Specific Message Target (if applicable)
	CHashedString m_obSpecificTargetName;

	// Attribute table.
	LuaAttributeTable* m_pSharedAttributes;
};


#endif // _ENTITY_HITTRIGGER_H
