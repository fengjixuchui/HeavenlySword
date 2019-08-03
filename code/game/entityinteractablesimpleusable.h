//--------------------------------------------------
//!
//!	\file game/entityiteractablesimpleusable.h
//!	Definition of the interactive "simple usable" entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_SIMPLE_USABLE_H
#define	_ENTITY_INTERACTABLE_SIMPLE_USABLE_H

#include "game/entityinteractable.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Interactable_Simple_Usable.
//! Simple usable object, can be bound to various events.
//!
//--------------------------------------------------
class Interactable_Simple_Usable : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Simple_Usable)

public:
	//Constructor
	Interactable_Simple_Usable();

	//Destructor
	~Interactable_Simple_Usable();

	//Post Construct
	void OnPostConstruct();

	void PlayAnim(CHashedString anim);

	//Public variables.
	Character* m_pOther;

	//From xml
	CHashedString	m_AnimPlayerActivate;
	CHashedString	m_AnimObjectActivate;
	CVector		m_MoveToPosition;
	CQuat		m_MoveToRotation;

protected:
	//Object description
	CKeyString	m_Description;

	//Animation container
	CHashedString	m_AnimationContainer;
};


#endif // _ENTITY_INTERACTABLE_SIMPLE_USABLE_H
