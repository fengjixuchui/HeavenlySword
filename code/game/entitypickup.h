//--------------------------------------------------
//!
//!	\file game/entitypickup.h
//!	Definition of the pickup entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_PICKUP_H
#define	_ENTITY_PICKUP_H

#include "game/entityinteractable.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Object_Pickup.
//! Ragdoll object.
//!
//--------------------------------------------------
class Object_Pickup : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Pickup)

public:
	// Constructor
	Object_Pickup();

	// Destructor
	~Object_Pickup();

	// Post Construct
	void OnPostConstruct();

	void PlayAnim(CHashedString anim);

	// Public variables.
	Character* m_pOther;

	//From xml
	CHashedString	m_AnimPlayerActivate;
	CHashedString	m_AnimObjectActivate;
	CVector		m_MoveToPosition;
	CQuat		m_MoveToRotation;
	bool		m_PickupFlag;


protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CKeyString	m_InitialState;

	// Animation container
	CHashedString	m_AnimationContainer;
};


#endif // _ENTITY_PICKUP_H
