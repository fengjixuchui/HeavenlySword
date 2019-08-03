//--------------------------------------------------
//!
//!	\file game/entityinteractablesimpleswitch.h
//!	Definition of the Interactable Simple Switch object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_SIMPLE_SWITCH_H
#define	_ENTITY_INTERACTABLE_SIMPLE_SWITCH_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"
#include "game/entityinteractableswitch.h"

//--------------------------------------------------
//!
//! Class Interactable_SimpleSwitch.
//! Base interactable simple switch entity type
//!
//--------------------------------------------------
class Interactable_SimpleSwitch : public Interactable_Switch_Trigger
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_SimpleSwitch)

public:
	// Constructor
	Interactable_SimpleSwitch();

	// Destructor
	~Interactable_SimpleSwitch();

	// Post Construct
	void OnPostConstruct();

	LuaAttributeTable* GetSharedAttributes(void) { return m_pSharedAttributes; }

	// Public Variables (once part of attribute table in the script)
	Character*	m_pOther;
	int			m_nUseCount;
	bool		m_bDeactivateOnFinish;

protected:
	
	// Active
	bool		m_bActive;

	// The animation container
	CHashedString	m_AnimationContainer;

	// Not sure of pointer type yet !!MB_ENT
	void* m_pSharedAttributesPtr;
	LuaAttributeTable* m_pSharedAttributes;
};


#endif // _ENTITY_INTERACTABLE_SIMPLE_SWITCH_H
