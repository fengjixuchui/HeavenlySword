//--------------------------------------------------
//!
//!	\file game/entityinteractableturretpoint.h
//!	Definition of the Interactable Turret Point entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_TURRET_POINT_H
#define	_ENTITY_INTERACTABLE_TURRET_POINT_H

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinteractable.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Interactable_Turret_Point.
//! Base interactable traverser type
//!
//--------------------------------------------------
class Interactable_Turret_Point : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Turret_Point)

public:
	// Constructor
	Interactable_Turret_Point();

	// Destructor
	~Interactable_Turret_Point();

	// Post Construct
	void OnPostConstruct();

	bool GlobalHandlers(Message& msg);
	bool AttachedState(Message& msg);
	bool DefaultState(Message& msg);

	Character* m_pOther;

	int m_iCamID;

	// The animations
	CHashedString	m_EnterAnimation;
	CHashedString	m_ExitAnimation;
	CHashedString	m_FallAnimation;
	CHashedString	m_CollapseAnimation;

	CPoint			m_obMountPoint;
	CPoint			m_obSpritePoint;

	float			m_fMinPitch;
	float			m_fMaxPitch;
	float			m_fMinYaw;
	float			m_fMaxYaw;

	CHashedString	m_AttachToParent;
	CHashedString	m_AttachToParentTransform;
	CPoint			m_ptAttachToParentOffset;

	bool			m_bRelativeAnimations;
	
	//ntstd::Vector<CEntity*> m_obUsers;

protected:
	// Object description
	CKeyString		m_Description;

	// Initial State
	CKeyString		m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;
};


#endif // _ENTITY_INTERACTABLE_TURRET_POINT_H
