//------------------------------------------------------------------------------------------
//!
//!	\file combat_lua.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/combat_lua.h"
#include "game/attacks.h"
#include "game/luaglobal.h"
#include "physics/world.h"
#include "game/messagehandler.h"
#include "game/hitcounter.h"
#include "objectdatabase/dataobject.h"
#include "game/syncdcombat.h"
#include "game/combatstyle.h"

//------------------------------------------------------------------------------------------
// CAttackComponent - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CAttackComponent)
	// Expose Base Combat Methods
	LUA_EXPOSED_METHOD_GET(HitCounter,			GetHitCounter, "") 
	LUA_EXPOSED_METHOD(AerialTargetingNotify,	AerialTargetingNotify, "Tells a characters attack system to stop falling in their KO", "", "")
	LUA_EXPOSED_METHOD(StartAttack,				StartNewAttack, "Tells a characters attack system to start an attack", "", "")
	LUA_EXPOSED_METHOD(MakeDead,				MakeDead, "Kills off the combat component", "", "")
	LUA_EXPOSED_METHOD(DisallowSwitchStance,	DisallowSwitchStance, "Stop the combat changing stance", "", "")
	LUA_EXPOSED_METHOD(AllowSwitchStance,		AllowSwitchStance, "Allow the combat to change stance", "", "")
	LUA_EXPOSED_METHOD(EndSyncdReaction,		EndSyncdReaction, "Indicates that a syncd reaction (stagger) animation is complete", "", "")
	LUA_EXPOSED_METHOD(HitByMainPlayer,			Audio_Access_HitByMainCharacter, "Is this temporary?", "", "")
	LUA_EXPOSED_METHOD(HitByPowerAttack,		Audio_Access_HitByPowerAttack, "Is this temporary?", "", "")
	LUA_EXPOSED_METHOD(HitByRangeAtttack,		Audio_Access_HitByRangeAttack, "Is this temporary?", "", "")
	LUA_EXPOSED_METHOD(HitBySpeedAttack,		Audio_Access_HitBySpeedAttack, "Is this temporary?", "", "")
	LUA_EXPOSED_METHOD(HitByKick,				Audio_Access_GetHitByKick, "Is this temporary?", "", "")
	LUA_EXPOSED_METHOD(SelectNextAttack,		SelectNextAttack, "Tells the combat system to choose a secondary attack", "", "")
	LUA_EXPOSED_METHOD(CompleteRecovery,		CompleteRecovery, "Tells the combat that recovery movement has been completed", "", "")
	LUA_EXPOSED_METHOD(StartFloored,			StartFlooredState, "Tells the combat that the KO movement has hit the floor", "", "")
	LUA_EXPOSED_METHOD(MoveInPowerStance,		MoveInPowerStance, "Is the combat component in power stance?", "", "")
	LUA_EXPOSED_METHOD(MoveInRangeStance,		MoveInRangeStance, "Is the combat component in range stance?", "", "")
	LUA_EXPOSED_METHOD(MoveInSpeedStance,		MoveInSpeedStance, "Is the combat component in speed stance?", "", "")
	LUA_EXPOSED_METHOD(StartSpecial,			StartSpecial, "", "", "")
	LUA_EXPOSED_METHOD(SetDisabled,				SetDisabled, "Turn the combat component of this entity on or off", "", "")
	LUA_EXPOSED_METHOD(SetTargetingDisabled,	SetTargetingDisabled, "Don't allow the entity to become targetted", "", "")
	LUA_EXPOSED_METHOD(SetKOAftertouch,			SetKOAftertouch, "Enable aftertouch control on a KOed character from their attacker.", "", "")


	// Expose Combat_Lua Methods
	LUA_EXPOSED_METHOD(SendRecoilMessage,		Lua_SendRecoilMessage, "Send msg_recoilstrike message to relevant entities that are near this entity.", "", "")
	LUA_EXPOSED_METHOD(ResetStylePoints,		Lua_ResetStylePoints, "Reset the style points", "", "" )
	LUA_EXPOSED_METHOD(ChangeLeadClusterTo,		Lua_ChangeLeadClusterTo, "Switch lead cluster to something else", "", "" )
	LUA_EXPOSED_METHOD(SetDefaultLeadClusterTo,	Lua_SetDefaultLeadClusterTo, "Switch default lead cluster to something else", "", "" )
	LUA_EXPOSED_METHOD(ResetLeadCluster,		Lua_ResetLeadCluster, "Reset cluster to original", "", "" )
	LUA_EXPOSED_METHOD(SetEnableStrikeVolumeCreation,		Lua_SetEnableStrikeVolumeCreation, "Set whether strike volumes get created or not", "", "" )

	LUA_EXPOSED_METHOD(SetInvulnerableToNormalStrike,		Lua_SetInvulnerableToNormalStrike, "", "", "" )
	LUA_EXPOSED_METHOD(SetInvulnerableToSyncStrike,		Lua_SetInvulnerableToSyncStrike, "", "", "" )
	LUA_EXPOSED_METHOD(SetInvulnerableToProjectileStrike,		Lua_SetInvulnerableToProjectileStrike, "", "", "" )
	LUA_EXPOSED_METHOD(SetInvulnerableToCounterStrike,		Lua_SetInvulnerableToCounterStrike, "", "", "" )

	LUA_EXPOSED_METHOD(IsInvolvedInASynchronisedAttack,		Lua_IsInvolvedInASynchronisedAttack, "", "", "" )
//	LUA_EXPOSED_METHOD(,, "", "", "")
LUA_EXPOSED_END(CAttackComponent)

//------------------------------------------------------------------------------------------
//!
//!	Combat_Lua::Lua_SendRecoilMessage
//! Send msg_recoilstrike message to relevant entities that are near this entity.
//!
//------------------------------------------------------------------------------------------
void Combat_Lua::Lua_SendRecoilMessage()
{
	ntAssert(this);
	if(!this) return;

	// Get our entity.
	CEntity* pEnt = ((CAttackComponent*)this)->m_pobParentEntity;
	ntAssert(pEnt);

	// See what entities we are colliding with
	ntstd::List<CEntity*> obIntersecting;
	Physics::CPhysicsWorld::Get().FindIntersectingRigidEntities((pEnt->GetPosition() + CPoint(0.0f, 1.0f, 0.0f)), 1.0f, obIntersecting);

	// Loop through the retrieved entities and send them a collision message
	ntstd::List<CEntity*>::iterator obEnd = obIntersecting.end();
	for(ntstd::List<CEntity*>::iterator obIt = obIntersecting.begin(); obIt != obEnd; ++obIt)
	{
		// If we have a message handler...
		if((*obIt)->GetMessageHandler())
		{
			// ...then send the message
			CMessageSender::SendEmptyMessage("msg_recoilstrike", (*obIt)->GetMessageHandler());
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Combat_Lua::Lua_ResetStylePoints
//! 
//!
//------------------------------------------------------------------------------------------
void Combat_Lua::Lua_ResetStylePoints()
{
	if ( StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
	{
		StyleManager::Get().GetHitCounter()->Used();
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:		bool Lua_GenerateStrike(entity,entity,string)
// DESCRIPTION:		Generate a strike - used for ranged weapons and thrown objects to damage
//					opponents through the combat system.
//-------------------------------------------------------------------------------------------------
bool Combat_Lua::Lua_GenerateStrike ( CEntity* pobAttacker, CEntity* pobTarget, const char* pcAttackDef )
{
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
			CPoint obOriginatorPos = pobAttacker->GetPosition();

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
												0,
												obOriginatorPos );

			// Post the strike
			SyncdCombat::Get().PostStrike( pobStrike );

			// We were successful
			return true;
		}

		// We couldn't find any atta
		user_warn_p( 0, ( "WARNING: Unable to find CAttackData %s\n", pcAttackDef ) );
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

void Combat_Lua::Lua_ChangeLeadClusterTo ( const char* pcObjName )
{
	ntError( pcObjName );

	DataObject* pobDOB = ObjectDatabase::Get().GetDataObjectFromName(pcObjName);

	if (pobDOB && strcmp(pobDOB->GetClassName(),"CClusterStructure") == 0)
	{
		((CAttackComponent*)this)->ChangeLeadClusterTo((CClusterStructure*)pobDOB->GetBasePtr());
	}
	else
	{
		ntPrintf("Failed to change lead cluster! Was the name %s right?\n",pcObjName);
	}
}

void Combat_Lua::Lua_SetDefaultLeadClusterTo ( const char* pcObjName )
{
	ntError( pcObjName );

	DataObject* pobDOB = ObjectDatabase::Get().GetDataObjectFromName(pcObjName);

	if (pobDOB && strcmp(pobDOB->GetClassName(),"CClusterStructure") == 0)
	{
		((CAttackComponent*)this)->SetDefaultLeadClusterTo((CClusterStructure*)pobDOB->GetBasePtr());
	}
	else
	{
		ntPrintf("Failed to change default lead cluster! Was the name %s right?\n",pcObjName);
	}
}

void Combat_Lua::Lua_ResetLeadCluster()
{
	((CAttackComponent*)this)->ResetLeadCluster();
}

void Combat_Lua::Lua_SetEnableStrikeVolumeCreation( bool bEnable )
{
	((CAttackComponent*)this)->SetEnableStrikeVolumeCreation(bEnable);
}

void Combat_Lua::Lua_SetInvulnerableToNormalStrike( bool bInv )
{
	((CAttackComponent*)this)->SetInvulnerableToNormalStrike(bInv);
}
void Combat_Lua::Lua_SetInvulnerableToSyncStrike( bool bInv )
{
	((CAttackComponent*)this)->SetInvulnerableToSyncStrike(bInv);
}
void Combat_Lua::Lua_SetInvulnerableToProjectileStrike( bool bInv )
{
	((CAttackComponent*)this)->SetInvulnerableToProjectileStrike(bInv);
}
void Combat_Lua::Lua_SetInvulnerableToCounterStrike( bool bInv )
{
	((CAttackComponent*)this)->SetInvulnerableToCounterStrike(bInv);
}

bool Combat_Lua::Lua_IsInvolvedInASynchronisedAttack()
{
	CAttackComponent* pobAttack = (CAttackComponent*)this;

	// Only return true if we have a current strike and that strike is something we are synchronising in
	return pobAttack && pobAttack->GetCurrentStrike() && pobAttack->GetCurrentStrike()->ShouldSync();
}

void Combat_Lua::Lua_SetCanHeadshotThisEntity( bool bHead )
{
	CAttackComponent* pobAttack = (CAttackComponent*)this;

	pobAttack->SetCanHeadshotThisEntity( bHead );
}

