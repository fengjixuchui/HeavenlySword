//--------------------------------------------------
//!
//!	\file game/entityneutralai.cpp
//!	Definition of the Neutral AI entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"

#include "game/entityneutralai.h"

void ForceLinkFunction23()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction23() !ATTN!\n");
}

START_STD_INTERFACE(NeutralAI)
	DEFINE_INTERFACE_INHERITANCE(AI)
	COPY_INTERFACE_FROM(AI)

	//DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
	//DECLARE_POSTPOSTCONSTRUCT_CALLBACK( OnPostPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//!	NeutralAI::NeutralAI()
//!	Default constructor
//!
//--------------------------------------------------
NeutralAI::NeutralAI()
{
}

//--------------------------------------------------
//!
//!	NeutralAI::~NeutralAI()
//!	Default destructor
//!
//--------------------------------------------------
NeutralAI::~NeutralAI()
{
}
