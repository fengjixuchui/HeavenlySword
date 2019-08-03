//--------------------------------------------------
//!
//!	\file game/entityinteractablespear.h
//!	Definition of the Interactable Spear entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_SPEAR_H
#define	_ENTITY_INTERACTABLE_SPEAR_H

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinteractable.h"
#include "fsm.h"

//--------------------------------------------------
//!
//! Class class Interactable_Spear.
//! Base interactable spear entity type
//!
//--------------------------------------------------
class Interactable_Spear : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Spear)

public:
	// Constructor
	Interactable_Spear();

	// Destructor
	~Interactable_Spear();

	// Post Construct
	void OnPostConstruct();

	LuaAttributeTable* GetSharedAttributes(void) { return m_pSharedAttributes; }

	Character*	m_pOther;
	int			m_nThrownCameraHandle;
	bool		m_bAiming;
	bool		m_bDoQuickThrow;
	bool		m_bCheckQuickThrow;
	bool		m_bHitRagdoll;
	bool		m_bHitSolid;
	bool		m_bCollision;

protected:

	// Object description
	// scee.sbashow - since all Interactable types use this, really should have this member in Interactable, perhaps?
	CKeyString	m_Description;

	// The animation container
	CHashedString	m_AnimationContainer;

	// Not sure of pointer type yet !!MB_ENT
	void* m_pSharedAttributesPtr;
	LuaAttributeTable* m_pSharedAttributes;
};


#endif // _ENTITY_INTERACTABLE_SPEAR_H
