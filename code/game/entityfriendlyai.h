//--------------------------------------------------
//!
//!	\file game/entityfriendlyai.h
//!	Definition of the Friendly AI entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_FRIENDLY_AI_H
#define	_ENTITY_FRIENDLY_AI_H

#include "game/entityai.h"

//--------------------------------------------------
//!
//! Class FriendlyAI.
//! Friendly AI entity type
//!
//--------------------------------------------------
class FriendlyAI : public AI
{
	// Declare dataobject interface
	HAS_INTERFACE(FriendlyAI)

public:
	// Constructor
	FriendlyAI();

	//Destructor
	~FriendlyAI();

	void OnPostConstruct();

protected:
};

#endif //_ENTITY_FRIENDLY_AI_H
