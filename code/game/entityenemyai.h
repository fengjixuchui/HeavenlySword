//--------------------------------------------------
//!
//!	\file game/entityenemyai.h
//!	Definition of the Enemy AI entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_ENEMY_AI_H
#define	_ENTITY_ENEMY_AI_H

#include "game/entityai.h"

//--------------------------------------------------
//!
//! Class EnemyAI.
//! Enemy AI entity type
//!
//--------------------------------------------------
class EnemyAI : public AI
{
	// Declare dataobject interface
	HAS_INTERFACE(EnemyAI)

public:
	EnemyAI();
	~EnemyAI();

	void OnPostConstruct();

protected:
};

LV_DECLARE_USERDATA(EnemyAI);

#endif //_ENTITY_ENEMY_AI_H
