//--------------------------------------------------
//!
//!	\file game/entityenemyai.cpp
//!	Definition of the Enemy AI entity object
//!
//--------------------------------------------------

#include "game/entityenemyai.h"

#include "objectdatabase/dataobject.h"

void ForceLinkFunction21()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction21() !ATTN!\n");
}


//------------------------------------------------------------------------------------------
// Enemy AI XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(EnemyAI, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(AI)
	COPY_INTERFACE_FROM(AI)

	OVERRIDE_DEFAULT(IsEnemy, "true")

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//!	EnemyAI::EnemyAI
//!	Default constructor
//!
//--------------------------------------------------
EnemyAI::EnemyAI()
{
	m_bIsEnemy = true;
}

//--------------------------------------------------
//!
//!	EnemyAI::~EnemyAI
//!	Default destructor
//!
//--------------------------------------------------
EnemyAI::~EnemyAI()
{
}


//--------------------------------------------------
//!
//!	EnemyAI::OnPostConstruct
//!	Post construction
//!
//--------------------------------------------------
void EnemyAI::OnPostConstruct()
{
	AI::OnPostConstruct();
}
