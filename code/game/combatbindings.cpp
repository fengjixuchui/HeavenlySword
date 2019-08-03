/***************************************************************************************************
*
*	FILE			combatbindings.cpp
*
*	DESCRIPTION		Exposes combat related functionality to the scripting environment
*
***************************************************************************************************/

// Necessary includes
#include "game/luaglobal.h"
#include "game/combatbindings.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/messagehandler.h"
#include "game/attacks.h"
#include "game/strike.h"
#include "game/syncdcombat.h"
#include "ai/aiformationcomponent.h"
#include "physics/world.h"
#include "objectdatabase/dataobject.h"

#include "game/superstylesafety.h"
#include "hud/hudmanager.h"
#include "game/combatstyle.h"

//-------------------------------------------------------------------------------------------------
// BINDFUNC:		HUD.IncrementLifeClock()
// DESCRIPTION:		Add some life clock to the player
//-------------------------------------------------------------------------------------------------
static void Combat_IncrementLifeClock( float fSeconds )
{
	if ( StyleManager::Exists() )
	{
		ntAssert( StyleManager::Get().GetLifeClock() );
		LifeClock* pLifeClock = StyleManager::Get().GetLifeClock();

		pLifeClock->IncrementLifeClock(fSeconds);
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:		HUD.DecrementLifeClock()
// DESCRIPTION:		Take some life clock away from the player
//-------------------------------------------------------------------------------------------------
static void Combat_DecrementLifeClock( float fSeconds )
{
	if ( StyleManager::Exists() )
	{
		ntAssert( StyleManager::Get().GetLifeClock() );
		LifeClock* pLifeClock = StyleManager::Get().GetLifeClock();

		pLifeClock->DecrementLifeClock(fSeconds);
	}
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:		bool Combat_GenerateStrike(entity,entity,string)
// DESCRIPTION:		Generate a strike - used for ranged weapons and thrown objects to damage
//					opponents through the combat system.
//-------------------------------------------------------------------------------------------------
static bool Combat_GenerateStrike ( CEntity* pobAttacker, CEntity* pobTarget, CHashedString pcAttackDef)
{
	// I want to be sure that only projectiles use this function - DGF
	ntError( pobAttacker->IsProjectile() );

	// Make sure we have an object that was thrown
	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert( pobSelf );
	if ( !pobSelf ) return false;

	// Make sure that we have a person who has been hit
	ntAssert( pobTarget );
	if ( !pobTarget ) return false;

	// Make sure that someone is not trying to hit themself
	if ( pobAttacker == pobTarget )
		return false;

	// If the target has an attack component we can issue them with a strike
	if ( pobTarget->GetAttackComponent() )
	{
		// Find the attack data that wwe are going to hit the opponent with
		CAttackData* pobAttackData = ObjectDatabase::Get().GetPointerFromName< CAttackData* >( pcAttackDef );
		if ( pobAttackData )
		{
			// Sort out an originating position - use an attacker if we have it
			CPoint obOriginatorPos = ( pobAttacker ) ? pobAttacker->GetPosition() : pobSelf->GetPosition();

			// Issue a strike - using the thrown entity as the originator - but the reaction position 
			// relative to the character that issued the attack
			CStrike* pobStrike = NT_NEW CStrike(	0, 
												pobTarget, 
												pobAttackData, 
												1.0f, 
												1.0f, 
												false, 
												false, 
												false,
												false,
												false,
												false,
												pobAttacker, // Pass in pointer to projectile
												obOriginatorPos);

			// Post the strike
			SyncdCombat::Get().PostStrike( pobStrike );

			// We were successful
			return true;
		}

		// We couldn't find any atta
		user_warn_p( 0, ( "WARNING: Unable to find CAttackData %s\n", ntStr::GetString(pcAttackDef) ) );
	}

	// Otherwise we can still send the entity a message that it may be able to handle
	else if ( pobTarget->GetMessageHandler() )
	{
		// Send the message
		CMessageSender::SendEmptyMessage( "msg_object_strike", pobTarget->GetMessageHandler() );

		// We didn't manage to send a strike
		return false;
	}

	// We didn't manage to send a strike
	return false;
}

//static bool Combat_GenerateStrike_str(CEntity* pobAttacker, CEntity* pobTarget, const char* pcAttackDef)
//{
//	return 	Combat_GenerateStrike(pobAttacker, pobTarget, CHashedString(pcAttackDef));
//}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:		Install_FormationComponent()
// DESCRIPTION:		Creates and installs a formation component on the entity;
// NOTE:			This shouldn't live in the file, but is currently required
//-------------------------------------------------------------------------------------------------
static void Install_FormationComponent (void)
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt != NULL );

	FormationComponent* pComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) FormationComponent( pobEnt );
	ntAssert( pComponent != NULL );

	// Place the new component on the entity
	pobEnt->SetFormationComponent( pComponent );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:		SuperStyleSafety_Create()
// DESCRIPTION:		Creates a SuperStyleSafetyManager Singleton
//-------------------------------------------------------------------------------------------------
static bool SuperStyleSafety_Create()
{
	if ( !SuperStyleSafetyManager::Exists() )
	{
		NT_NEW SuperStyleSafetyManager();
		return true;
	}
	else
	{
		ntPrintf("SuperStyleSafety: SuperStyleSafetyManager already exists!\n");
		return false;
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:		SuperStyleSafety_Create()
// DESCRIPTION:		Sets the currently active set of safety volumes in the super style safety manager
//-------------------------------------------------------------------------------------------------
static bool SuperStyleSafety_SetSuperStyleSafetyVolumeCollection( CHashedString obCollectionName )
{
	SuperStyleSafetyVolumeCollection* pobColl = ObjectDatabase::Get().GetPointerFromName< SuperStyleSafetyVolumeCollection* >( obCollectionName );

	if ( SuperStyleSafetyManager::Exists() && pobColl )
	{
		SuperStyleSafetyManager::Get().SetSuperStyleSafetyVolumeCollection(pobColl);
		return true;
	}
	else if ( !SuperStyleSafetyManager::Exists() )
	{
		ntPrintf("SuperStyleSafety: No SuperStyleSafetyManager, did you call SuperStyleSafety.Create()?\n");
	}
	else if (!pobColl)
	{
		ntPrintf("SuperStyleSafety: %s doesn't exist.\n", obCollectionName.GetDebugString());
		SuperStyleSafetyManager::Get().SetSuperStyleSafetyVolumeCollection(NULL);
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CCombatBindings::Register
*
*	DESCRIPTION		Registers the static combat related functions with the scripting environment
*
***************************************************************************************************/
void CCombatBindings::Register()
{
	// Get the bit where globally available functions are registered
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	// These bindings should be moved onto the combat component - or a combat helper class
	// Register our static functions
	obGlobals.Register( "Combat_GenerateStrike",			Combat_GenerateStrike );

	// This is placed here only in the short term - perhaps a month (17-11-2005)
	obGlobals.Register( "Install_FormationComponent",		Install_FormationComponent );

	// Put combat bindings in the Combat Context
	obGlobals.Set("HUD", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["HUD"].Register("IncrementLifeClock",		Combat_IncrementLifeClock);
	obGlobals["HUD"].Register("DecrementLifeClock",		Combat_DecrementLifeClock);

	// Put safety bindings in the safety context
	obGlobals.Set("SuperStyleSafety", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["SuperStyleSafety"].Register("Create", SuperStyleSafety_Create);
	obGlobals["SuperStyleSafety"].Register("SetSuperStyleSafetyVolumeCollection", SuperStyleSafety_SetSuperStyleSafetyVolumeCollection);
}

