//--------------------------------------------------
//!
//!	\file game/entityneutralai.h
//!	Definition of the Neutral AI entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_NEUTRAL_AI_H
#define	_ENTITY_NEUTRAL_AI_H

#include "game/entityai.h"

//--------------------------------------------------
//!
//! Class NeutralAI.
//! Neutral AI entity type
//!
//--------------------------------------------------
class NeutralAI : public AI
{
	// Declare dataobject interface
	HAS_INTERFACE(NeutralAI)

public:
	// Constructor
	NeutralAI();

	//Destructor
	~NeutralAI();

protected:

};

#endif //_ENTITY_NEUTRAL_AI_H
