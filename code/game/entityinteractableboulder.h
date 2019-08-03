//--------------------------------------------------
//!
//!	\file game/entityinteractableboulder.h
//!	Definition of the Interactable Boulder entity object
//! - Button mashing to get boulder rolling, then
//!   under physics control/
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_BOULDER_H
#define	_ENTITY_INTERACTABLE_BOULDER_H

#include "game/entityinteractable.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Interactable_Boulder.
//! Base interactable boulder entity type
//!
//--------------------------------------------------
class Interactable_Boulder : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Boulder)

public:
	// Constructor
	Interactable_Boulder();

	// Destructor
	~Interactable_Boulder();

	// Post Construct
	void OnPostConstruct();

	LuaAttributeTable* GetSharedAttributes(void) { return m_pSharedAttributes; }

	// Public Variables (once part of attribute table in the script)
	Character*	m_pOther;

	// Animation names
	CHashedString	m_AnimBoulderStart;
	CHashedString	m_AnimBoulderButtonMash;
	CHashedString	m_AnimBoulderEnd;
	CHashedString	m_AnimOperatorStart;
	CHashedString	m_AnimOperatorButtonMash;
	CHashedString	m_AnimOperatorEnd;

	CPoint		m_CharacterPositionOffset;
	CPoint		m_CharacterRotationOffset;

	// Mashing parameters
	float		m_fRequiredMPSStart;	// How fast you have to press it at the start (to keep it stationary)
	float		m_fRequiredMPSEnd;		// How fast you have to press it at the end (to keep it stationary)
	float		m_fMPSRateVariation;	// Percentage variation range to mash in.

	// Shared Params
	ButtonMashSharedParams	m_SharedParams;

	// TEMP: Direction of the boulder at the start
	CDirection m_obFacingDirection;

protected:

	// Object description
	CKeyString	m_Description;

	// Initial State
	CKeyString	m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;

	// Not sure of pointer type yet !!MB_ENT
	void* m_pSharedAttributesPtr;
	LuaAttributeTable* m_pSharedAttributes;
};


#endif // _ENTITY_INTERACTABLE_BOULDER_H
