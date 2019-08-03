//--------------------------------------------------
//!
//!	\file game/entityfriendlyai.cpp
//!	Definition of the Friendly AI entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"

#include "game/entityfriendlyai.h"

void ForceLinkFunction22()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction22() !ATTN!\n");
}

START_CHUNKED_INTERFACE(FriendlyAI, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(AI)
	COPY_INTERFACE_FROM(AI)

	//PUBLISH_VAR_AS(m_bIsEnemy, IsEnemy)
	OVERRIDE_DEFAULT(IsEnemy, "false")

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//!	FriendlyAI::FriendlyAI()
//!	Default constructor
//!
//--------------------------------------------------
FriendlyAI::FriendlyAI()

{

}

//--------------------------------------------------
//!
//!	FriendlyAI::~FriendlyAI()
//!	Default destructor
//!
//--------------------------------------------------
FriendlyAI::~FriendlyAI()
{
}


//--------------------------------------------------
//!
//!	FriendlyAI::OnPostConstruct
//!	Post construction
//!
//--------------------------------------------------
void FriendlyAI::OnPostConstruct()
{
	AI::OnPostConstruct();
}

