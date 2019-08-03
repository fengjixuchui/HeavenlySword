//--------------------------------------------------
//!
//!	\file game/entityinteractablehoiststatue.h
//!	Definition of the Interactable Hoist Statue entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_HOISTSTATUE_H
#define	_ENTITY_INTERACTABLE_HOISTSTATUE_H

#include "game/entityinteractable.h"

//--------------------------------------------------
//!
//! Class Interactable_HoistStatue.
//! Base Interactable Hoist Statue entity type
//!
//--------------------------------------------------
class Interactable_HoistStatue : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_HoistStatue)

public:
	// Constructor
	Interactable_HoistStatue();

	// Destructor
	~Interactable_HoistStatue();

	// Post Construct
	void OnPostConstruct();

	LuaAttributeTable* GetSharedAttributes(void) { return m_pSharedAttributes; }

	CKeyString GetNextHoist();

	// Public Variables (once part of attribute table in the script)
	float m_fAnimSpeed;

	CKeyString m_obCollisionAtk;

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CKeyString	m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;
	CKeyString	m_obAnims;

	ntstd::List<CKeyString>				m_obHoistAnims;
	ntstd::List<CKeyString>::iterator	m_pobCurrHoist;

	// Not sure of pointer type yet !!MB_ENT
	void* m_pSharedAttributesPtr;
	LuaAttributeTable* m_pSharedAttributes;
	
};


#endif // _ENTITY_INTERACTABLE_HOISTSTATUE_H
