//--------------------------------------------------
//!
//!	\file game/entityarmymessagehub.h
//! entity that is mapped in mred to route messages
//! between the army code and the normal entity system
//!
//--------------------------------------------------

#ifndef	_ENTITY_ARMY_MESSAGE_HUB_H
#define	_ENTITY_ARMY_MESSAGE_HUB_H

#include "entity.h"
#include "entity.inl"
#include "fsm.h"

class ArmyBattlefield;

//--------------------------------------------------
//!
//! Army message hub entity
//!
//--------------------------------------------------
class ArmyMessageHub : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(ArmyMessageHub)

public:
	// Constructor
	ArmyMessageHub();

	// Destructor
	~ArmyMessageHub();

	// Post Construct
	void OnPostConstruct();

	// lets us create or destroy the army stuff based on section loader (TODO make it betterer)
	virtual void Pause( bool bPause, bool bFullPhysicsPause = false );

protected:
	ArmyBattlefield* m_pBattlefieldDef;
};


#endif // _ENTITY_ARMY_MESSAGE_HUB_H
