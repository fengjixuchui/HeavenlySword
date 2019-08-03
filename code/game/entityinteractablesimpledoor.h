//--------------------------------------------------
//!
//!	\file game/entityinteractablesimpledoor.h
//!	Definition of the Interactable simple door entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_SIMPLE_DOOR_H
#define	_ENTITY_INTERACTABLE_SIMPLE_DOOR_H

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinteractableswitch.h"
#include "game/interactiontransitions.h"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Interactable_SimpleDoor.
//! Base interactable simple door entity type
//!
//--------------------------------------------------
class Interactable_SimpleDoor : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_SimpleDoor)

public:
	// Constructor
	Interactable_SimpleDoor();

	// Destructor
	~Interactable_SimpleDoor();

	// From CEntity
	virtual void OnLevelStart();

	// Post Construct
	void OnPostConstruct();
	void OnAwake( void );

	bool GlobalHandlers(Message& msg);
	bool AttachedState(Message& msg);
	bool DefaultState(Message& msg);

	float GetHoldTime() const  {return m_fHoldTime;}
	float GetSlamSpeed() const {return m_fSlamSpeed;}

	CEntity* m_pSyncEnt;
	//CAnimationPtr m_pobAnim;

	// The animations
	CHashedString	m_OpenAnimation;
	CHashedString	m_CloseAnimation;


	bool		m_bMovedOffStart;		// Ensure we've started to move before we checking for returns to start 
	bool		m_bAutoClose;		

	float		m_fPosition;			// Track position incase we need to close a partialy open door (ie with synced open)
	float		m_fHoldingTime;
	bool		m_bSlamming;

	CoordinationParams m_obCoordinationParams;

	// Quick fix till on awake properly implemented for entities
	float m_bOnAwakeSetupDone;

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CKeyString	m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;	
	
	bool		m_bOpenOnStart;
	bool		m_bAIAvoid;

	float       m_fHoldTime;
	float       m_fSlamSpeed;

	// Not sure of pointer type yet !!MB_ENT
	/*void* m_pSharedAttributesPtr;
	LuaAttributeTable* m_pSharedAttributes;*/
};


#endif // _ENTITY_INTERACTABLE_SIMPLE_DOOR_H
