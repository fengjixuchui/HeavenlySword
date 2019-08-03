//--------------------------------------------------
//!
//!	\file game/entitypickup.cpp
//!	Definition of the pickup entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "messagehandler.h"

#include "game/entityarmymessagehub.h"

#if defined( PLATFORM_PS3 )
#include "army/armymanager.h"
#include "army/armydef.h"
#endif

void ForceLinkFunctionArmyMessageHub()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionArmyMessageHub() !ATTN!\n");
}

START_CHUNKED_INTERFACE(ArmyMessageHub, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_PTR_AS( m_pBattlefieldDef, Battlefield )
	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE

// global config option
bool g_UseArmy = true;


//--------------------------------------------------
//!
//! Pickup State Machine
//!
//--------------------------------------------------
STATEMACHINE(ARMYMESSAGEHUB_FSM, ArmyMessageHub)
	ARMYMESSAGEHUB_FSM()
	{			
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			EVENT(msg_army_global_event)
			{
				ArmyManager::Get().GlobalEventMessage( msg.GetHashedString( CHashedString(HASH_STRING_DATA) ) );
			}
			END_EVENT(true)
			EVENT( Trigger )
			{
				ArmyManager::Get().GlobalEventMessage( CHashedString( "Trigger" ) );
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

END_STATEMACHINE //ARMYMESSAGEHUB_FSM


//--------------------------------------------------
//!
//!	ArmyMessageHub::ArmyMessageHub()
//!	Default constructor
//!
//--------------------------------------------------
ArmyMessageHub::ArmyMessageHub()
{
}

//--------------------------------------------------
//!
//!	ArmyMessageHub::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void ArmyMessageHub::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	InstallMessageHandler();

	// Create and attach the statemachine
	ARMYMESSAGEHUB_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) ARMYMESSAGEHUB_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	ArmyMessageHub::~ArmyMessageHub()
//!	Default destructor
//!
//--------------------------------------------------
ArmyMessageHub::~ArmyMessageHub()
{
}

//--------------------------------------------------
//!
//!	ArmyMessageHub::~ArmyMessageHub()
//!	Default destructor
//!
//--------------------------------------------------
void ArmyMessageHub::Pause( bool bPause, bool bFullPhysicsPause )
{
	UNUSED( bFullPhysicsPause );
	// do normal pause stuff
	CEntity::Pause( bPause );
#if defined( PLATFORM_PS3 )
	if( m_pBattlefieldDef == 0 || g_UseArmy == false )
		return;

	// + tell the army manager
	if( !bPause )
	{
		ntPrintf("Army Message Hub Activate\n" );
		ArmyManager::Get().Activate( m_pBattlefieldDef );
		m_pBattlefieldDef->m_pMessageHub = this;
	} else
	{
		m_pBattlefieldDef->m_pMessageHub = 0;
		ntPrintf("Army Message Hub Deactivate\n" );
		// de-activate the the army
		ArmyManager::Get().Deactivate();
	}
#endif

}
