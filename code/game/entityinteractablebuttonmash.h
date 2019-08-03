//--------------------------------------------------
//!
//!	\file game/entityinteractablebuttonmash.h
//!	Definition of the Interactable Button-Mash entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_BUTTONMASH_H
#define	_ENTITY_INTERACTABLE_BUTTONMASH_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"
#include "game/entityinteractableswitch.h"
#include "game/interactiontransitions.h"
#include "game/interactableparams.h"

class Att_ButtonMash : public Attr_Interactable
{
};

//--------------------------------------------------
//!
//! Class Interactable_ButtonMash.
//! Base interactable button-mash entity type
//!
//--------------------------------------------------
class Interactable_ButtonMash : public Interactable_Switch_Trigger
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_ButtonMash)

public:
	// Constructor
	Interactable_ButtonMash();

	// Destructor
	~Interactable_ButtonMash();

	// Post Construct
	void OnPostConstruct();

	// Animation names
	CHashedString	m_AnimStart;
	CHashedString	m_AnimButtonMash;
	CHashedString	m_AnimEnd;
	CHashedString	m_AnimFail;
	CHashedString	m_AnimOperatorStart;
	CHashedString	m_AnimOperatorButtonMash;
	CHashedString	m_AnimOperatorEnd;
	CHashedString	m_AnimOperatorFail;

	CPoint		m_CharacterPositionOffset;
	CPoint		m_CharacterRotationOffset;

	int			m_iButtonMashRepeats;

	// Mashing parameters
	float		m_fRequiredMPSStart;	// How fast you have to press it at the start (to keep it stationary)
	float		m_fRequiredMPSEnd;		// How fast you have to press it at the end (to keep it stationary)
	float		m_fMPSPveRateVariation;	// Percentage variation range to mash in.
	float		m_fMPSNveRateVariation;	// Percentage variation range to mash in.

	// Shared Params
	ButtonMashSharedParams	m_SharedParams;


	// Public Variables (once part of attribute table in the script)
	Character*	m_pOther;

	bool		m_bMovedOffStart;
	bool		m_bExitOnFail;

	virtual const CUsePointAttr* GetUsePointAttributes()	const {return m_pobSharedAttributes ? m_pobSharedAttributes->m_pobUsePointAttrs : 0;}
protected:
	
	Att_ButtonMash	*m_pobSharedAttributes;	// the *real* shared attributes here ;-)

	// The animation container
	CHashedString	m_AnimationContainer;

	// Not sure of pointer type yet !!MB_ENT
	//void* m_pSharedAttributesPtr;
	//LuaAttributeTable* m_pSharedAttributes;
};

#endif // _ENTITY_INTERACTABLE_BUTTONMASH_H
