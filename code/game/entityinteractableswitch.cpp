//--------------------------------------------------
//!
//!	\file game/entityinteractableswitch.cpp
//!	Definition of the Interactable entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"

#include "game/entityinteractableswitch.h"

void ForceLinkFunctionSwitch()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionSwitch() !ATTN!\n");
}

//! Switch trigger interface
START_CHUNKED_INTERFACE(Interactable_Switch_Trigger, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_AS(m_Description, Description)
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	Interactable_Switch_Trigger::Interactable_Switch_Trigger()
//!	Default Constructor
//!
//--------------------------------------------------
Interactable_Switch_Trigger::Interactable_Switch_Trigger()
{
	m_fStateValue = 0.0f;
}


//--------------------------------------------------
//!
//!	Interactable_Switch_Trigger::Interactable_Switch_Trigger()
//!	Default Destructor
//!
//--------------------------------------------------
Interactable_Switch_Trigger::~Interactable_Switch_Trigger()
{
	//Nothing to be done.
}
