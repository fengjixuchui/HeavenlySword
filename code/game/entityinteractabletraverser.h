//--------------------------------------------------
//!
//!	\file game/entityinteractabletraverser.h
//!	Definition of the Interactable Traverser entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_TRAVERSER_H
#define	_ENTITY_INTERACTABLE_TRAVERSER_H

#include "game/entityinteractable.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Interactable_Traverser.
//! Base interactable traverser type
//!
//--------------------------------------------------
class Interactable_Traverser : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Traverser)

public:
	// Constructor
	Interactable_Traverser();

	// Destructor
	~Interactable_Traverser();

	// From CEntity
	virtual void OnLevelStart();

	// Post Construct
	void OnPostConstruct();

	bool GlobalHandlers(Message& msg);
	bool AttachedState(Message& msg);
	bool DefaultState(Message& msg);

	Character* m_pOther;

	// The animations
	CHashedString	m_TraverserAnimation;
	CHashedString	m_UserAnimation;
	CHashedString	m_ResetAnimation;
	CHashedString	m_ActivateAnimation;
	CHashedString	m_DeactivateAnimation;
	CHashedString	m_InteruptAnimation;

	CHashedString	m_InteruptAttack;
	
	bool		m_bPairUsePoints;

	ntstd::List<CEntity*> m_obUsers;

	// Helpers
	bool IsNamedUsePoint(const CPoint& pobPos, const CHashedString obUseName);

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


#endif // _ENTITY_INTERACTABLE_TRAVERSER_H
