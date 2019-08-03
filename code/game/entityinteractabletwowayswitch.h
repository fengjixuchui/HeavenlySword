//--------------------------------------------------
//!
//!	\file game/entityinteractabletwowayswitch.h
//!	Definition of the Interactable Two Way Switch object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_TWOWAY_SWITCH_H
#define	_ENTITY_INTERACTABLE_TWOWAY_SWITCH_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"
#include "game/entityinteractableswitch.h"

//--------------------------------------------------
//!
//! Class Interactable_TwoWaySwitch.
//! Base interactable two way switch entity type
//!
//--------------------------------------------------
class Interactable_TwoWaySwitch : public Interactable_Switch_Trigger
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_TwoWaySwitch)

public:
	// Constructor
	Interactable_TwoWaySwitch();

	// Destructor
	~Interactable_TwoWaySwitch();

	// From CEntity
	virtual void OnLevelStart();

	// Post Construct
	void OnPostConstruct();

	LuaAttributeTable* GetSharedAttributes(void) { return m_pSharedAttributes; }

	// Public Variables (once part of attribute table in the script)
	Character*	m_pOther;
	bool		m_bDeactivateOnFinish;

	// Position
	bool		m_bPositionA;

protected:
	
	// Active
	bool		m_bActive;

	// The animation container
	CHashedString	m_AnimationContainer;

	// Not sure of pointer type yet !!MB_ENT
	void* m_pSharedAttributesPtr;
	LuaAttributeTable* m_pSharedAttributes;
};


#endif // _ENTITY_INTERACTABLE_TWOWAY_SWITCH_H
