//--------------------------------------------------
//!
//!	\file game/entityinteractableladder.h
//!	Definition of the Interactable ladder object
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_LADDER_H
#define	_ENTITY_INTERACTABLE_LADDER_H

#include "game/entityinteractable.h"


//--------------------------------------------------
//!
//! Class Interactable_Ladder.
//! The ladder object
//!
//--------------------------------------------------
class Interactable_Ladder : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Ladder)

public:
	//Constructor
	Interactable_Ladder();
	//Destructor
	~Interactable_Ladder();

	// Post Construct
	void OnPostConstruct();

	LuaAttributeTable* GetSharedAttributes(void) { return m_pSharedAttributes; }

	bool IsNamedUsePoint(const CPoint& pobPos, const CHashedString obUseName);

	// Public variables so state machine can use them
	Character*		m_pOther;
	CHashedString   m_LadderParams;
	bool			m_bRunTo;
	bool			m_bFromTop;

protected:
	//Object description
	CKeyString m_Description;

	LuaAttributeTable* m_pSharedAttributes;
};


#endif // _ENTITY_INTERACTABLE_LADDER_H
