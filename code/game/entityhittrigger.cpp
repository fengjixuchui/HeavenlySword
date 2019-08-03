//--------------------------------------------------
//!
//!	\file game/entityhittrigger.cpp
//!	Definition of the hit-trigger entity object
//!
//--------------------------------------------------

/*
TODO LIST:
1. The entlib version of TriggerEntity needs to be updated with InitalState, and changed to use Object_HitTrigger and no construction-script.
2. There seems to be an assert caused on the message handler being created, but not here (very odd!). The construction all goes fine, but then later,
- there are ~9 asserts for (m_obMessageHandler == 0) as it's trying to be created elsewhere too... needs looking into.
3. Needs testing.

Aside from those it should be done.
*/
#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "messagehandler.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/renderablecomponent.h"
#include "game/entityprojectile.h"

#include "game/entityhittrigger.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunction39()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction39() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Object_HitTrigger, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_InitialState, InitialState)
	PUBLISH_VAR_AS(m_bProjectileAttribForSpecificMessage, ProjAttribForSpecificMessage)
	PUBLISH_VAR_AS(m_obSpecificTargetName, SpecificMessageTargetName)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Hit Trigger State Machine
//!
//--------------------------------------------------
STATEMACHINE(HITTRIGGER_FSM, Object_HitTrigger)
	HITTRIGGER_FSM(bool bActive)
	{
		if(bActive)
		{
			SET_INITIAL_STATE(ACTIVE);
		}
		else
		{
			SET_INITIAL_STATE(INACTIVE);
		}

	}

	STATE(ACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				//Nothing to do.
			}
			END_EVENT(true)

			EVENT(msg_projcol)
			{
				CEntity* pTarget = ME->m_pobSpecificMessageTarget;
				CKeyString obSpecificAttrib = ME->m_bProjectileAttribForSpecificMessage;
				// Check for a specific type of projectile hit (flaming arrow puzzle etc...)
				if ( pTarget && stricmp( obSpecificAttrib.GetString(), "" ) != 0 )
				{
					Object_Projectile* pProjectile = (Object_Projectile*)msg.GetEnt("Projectile");

					if ( pProjectile )
					{
						LuaAttributeTable* pAttrTable = pProjectile->GetAttributeTable();

						if ( pAttrTable->GetBool( CHashedString(obSpecificAttrib.GetString()) ) )
						{
							Message SpecificProjCol(msg_specific_projcol);
							//Attach the trigger and target entities to this message (so it can be hidden/exploded/whatever).
							SpecificProjCol.SetEnt("TriggerTargetEnt", pTarget);
							SpecificProjCol.SetEnt("HitTriggerEnt", (CEntity*)ME);
							
							if ( pTarget->GetMessageHandler() )
							{
								pTarget->GetMessageHandler()->QueueMessage(SpecificProjCol);
							}
						}
					}
				}

				// Standard OnHit Event Processing
				ntPrintf("############ msg_projcol recieved on trigger entity\n");
				ME->GetMessageHandler()->ProcessEvent("OnHit");	//Fire our OnAction event.
			}
			END_EVENT(true)

			EVENT(msg_deactivate)
			{
				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				//Nothing to do.
			}
			END_EVENT(true)

			EVENT(msg_activate)
			{
				SET_STATE(ACTIVE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE


END_STATEMACHINE //HITTRIGGER_FSM


//--------------------------------------------------
//!
//!	Object_HitTrigger::Object_HitTrigger()
//!	Default constructor
//!
//--------------------------------------------------
Object_HitTrigger::Object_HitTrigger()
{
	m_eType = EntType_Static; // Must be static! Otherwise projectiles will not collide with it. 
	m_pSharedAttributes = 0;
}

//--------------------------------------------------
//!
//!	Object_HitTrigger::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_HitTrigger::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	InstallMessageHandler();
	InstallDynamics();

	m_pobPhysicsSystem->Lua_Rigid_SetKeyframedMotion(true);

	bool bActive = false;

	// Find the target entity in the object database
	m_pobSpecificMessageTarget = ObjectDatabase::Get().GetPointerFromName<CEntity*>(m_obSpecificTargetName);

	if(m_InitialState == "Active")
	{
		ntPrintf("#### Hit trigger active status was true\n");
		bActive = true;
	}
	// Create and attach the statemachine
	HITTRIGGER_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) HITTRIGGER_FSM(bActive);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_HitTrigger::~Object_HitTrigger()
//!	Default destructor
//!
//--------------------------------------------------
Object_HitTrigger::~Object_HitTrigger()
{
	NT_DELETE(m_pSharedAttributes);
}
