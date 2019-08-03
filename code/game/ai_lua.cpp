//------------------------------------------------------------------------------------------
//!
//!	\file ai_lua.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/ai_lua.h"
#include "game/aicomponent.h"
#include "game/luaglobal.h"
#include "objectdatabase/dataobject.h"
#include "ai/aiformationcomponent.h"
#include "game/query.h"
#include "game/entitymanager.h"

//------------------------------------------------------------------------------------------
//  CAIComponent - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CAIComponent)
	// Base AI Methods
	
	LUA_EXPOSED_METHOD(ActivateSingleAnimDef,	ActivateSingleAnimDef, "", "", "")
	LUA_EXPOSED_METHOD(ActivateSingleAnim,	ActivateSingleAnim, "", "", "")
	LUA_EXPOSED_METHOD(SetDisabled,			SetDisabled, "", "", "")
	LUA_EXPOSED_METHOD_GET(CtrlState,		GetLuaState, "")
	LUA_EXPOSED_METHOD_SET(CtrlState,		SetLuaState, "")
	LUA_EXPOSED_METHOD_GET(Combat,			GetCombatComponentP, "")
	LUA_EXPOSED_METHOD_GET(Formation,		GetAIFormationComponent, "")
	LUA_EXPOSED_METHOD(SetRecovering,		SetRecovering, "Set whether this entity is recovering", "bool recovering", "true or false")
	LUA_EXPOSED_METHOD(CanAlwaysSeePlayer,	CanAlwaysSeePlayer, "Turn on this AI's x-ray specs.", "bool CanSee", "true or false")
	LUA_EXPOSED_METHOD(RefreshMovement,		RefreshController, "Put back on the correct movement controller, used to recover from attacks", "", "")
	LUA_EXPOSED_METHOD(SetAnim,				SetScriptAnimName, "Sets the name of an anim to play in the play anim state", "string anim", "")
	LUA_EXPOSED_METHOD(OverrideAnim,		CompleteSimpleAction, "Overrides the currently playing animation, allowing new actions to begin", "", "")
	LUA_EXPOSED_METHOD(SetLoopAnim,			SetScriptAnimLooping, "tells the PlayAnim behaviour to loop the anim indefinitely", "bool loop", "true to loop, false to not")
	LUA_EXPOSED_METHOD(SetLocator,			SetScriptLocatorName, "Sets the name of a locator to navigate to in the walktolocator behaviour", "string locator", "")
	LUA_EXPOSED_METHOD(SetObject,			SetScriptObjectName, "Sets the name of an object to use in the useobject behaviour", "string object", "")
	LUA_EXPOSED_METHOD(SetRun,				SetScriptRun, "Set whether to walk or run", "bool run", "true to run, false to walk")
	LUA_EXPOSED_METHOD(SetFacingLocator,	SetScriptFacingLocatorName, "Sets the name of a locator to face at the end of the walktolocator behaviour", "string locator", "")
	LUA_EXPOSED_METHOD(SetLocatorThreshold,	SetScriptLocatorThreshold, "Sets the distance threshold at which an AI will stop when moving to a locator", "float threshold", "distance")
	LUA_EXPOSED_METHOD(SetFacingEntity,		SetScriptFacingEntityName, "Sets the name of an entity to face in the FaceEntity behaviour", "string entity", "")
	LUA_EXPOSED_METHOD(ApplyNewAIDef,		ApplyNewAIDef, "Changes the AI def used by the entity - only using if you know what you're doing", "string new def", "")
	LUA_EXPOSED_METHOD(SetCoverPoint,		SetScriptCoverPointName, "Sets the name of a cover point to use in the FindCover behaviour", "string coverpoint", "")
	LUA_EXPOSED_METHOD(IsInCover,			InCover, "is the AI currently in cover", "", "")
	LUA_EXPOSED_METHOD(SetBallistaTarget,	SetScriptBallistaTarget, "Sets the name of a target entity for AIs using the ballista", "string entityname", "")
	LUA_EXPOSED_METHOD(SetBallistaTargetOffset,	SetScriptBallistaTargetOffset, "Sets an offset vector from the target's root position", "", "")
	LUA_EXPOSED_METHOD(FireBallista,		ScriptFireBallista, "Queues a request to fire the  AIs using the ballista", "string entityname", "")
	LUA_EXPOSED_METHOD(FindAttackTarget,	FindAttackTarget, "Returns an entity to attack", "", "")

	// AI Navigation
	LUA_EXPOSED_METHOD(SetIntention,		SetIntention, "Define the Navigation Intention of an AI", "", "")
	LUA_EXPOSED_METHOD(SetEntityToAttack,	SetEntityToAttack, "", "", "")
	LUA_EXPOSED_METHOD(SetEntityDescriptionsToAttack,SetEntityDescriptionsToAttack, "", "", "")
	LUA_EXPOSED_METHOD(SetEntityToFollow,	SetEntityToFollow, "", "", "")
	LUA_EXPOSED_METHOD(SetEntityToGoTo,		SetEntityToGoTo, "", "", "")
	LUA_EXPOSED_METHOD(SetStartEndNodes,	SetStartEndNodes, "", "", "")
	LUA_EXPOSED_METHOD(SetDestinationNode,	SetDestinationNode, "", "", "")
	LUA_EXPOSED_METHOD(SetPatrolling,		SetPatrolling, "", "", "")
	LUA_EXPOSED_METHOD(SetVision,			SetVision, "", "", "")
	LUA_EXPOSED_METHOD(SetVisionParam,		SetVisionParam, "", "", "")
	LUA_EXPOSED_METHOD(SetMovementParam,	SetMovementParam, "", "", "")
	LUA_EXPOSED_METHOD(SetRangedParam,		SetRangedParam, "", "", "")
	LUA_EXPOSED_METHOD(SetRangedTargetingParam, SetRangedTargetingParam, "", "", "")
	LUA_EXPOSED_METHOD(SetDestinationRadius,SetDestinationRadius, "", "", "")
	LUA_EXPOSED_METHOD(SetAttackRange,		SetAttackRange, "", "", "")
	LUA_EXPOSED_METHOD(SetIdleFlags,		SetIdleFlags, "", "", "")
	LUA_EXPOSED_METHOD(SetIdleClearsIntention,SetIdleClearsIntention, "", "", "")
	LUA_EXPOSED_METHOD(SetExternalControlState,SetExternalControlState, "", "", "")
	LUA_EXPOSED_METHOD(SetCoverAttitude,	SetCoverAttitude, "", "", "")
	LUA_EXPOSED_METHOD(SetReuseCoverPoints,	SetReuseCoverPoints, "", "", "")
	LUA_EXPOSED_METHOD(SetMinWallDetRadius,	SetMinWallDetRadius, "", "", "")
	LUA_EXPOSED_METHOD(SetMinMaxRadii,		SetMinMaxRadii, "", "", "")
	LUA_EXPOSED_METHOD(SetTimeBetweenShots,	SetTimeBetweenShoots, "", "", "")
	LUA_EXPOSED_METHOD(SetNumberOfConsecShots,	SetNumberOfConsecShots, "", "", "")
	LUA_EXPOSED_METHOD(SetHearingParam,		SetHearingParam, "", "", "")
	LUA_EXPOSED_METHOD(GetEntityToAttack,	GetEntityToAttack, "Returns the entity to attack", "", "")
	LUA_EXPOSED_METHOD(SetCannonTarget,		SetCannonTarget, "", "", "")
//	LUA_EXPOSED_METHOD(SetCannonTargetLocatorPos,		SetCannonTargetLocatorPos, "", "", "")
	LUA_EXPOSED_METHOD(ShootTheCannon,		ShootTheCannon, "", "", "")
	LUA_EXPOSED_METHOD(SetAIBoolParam,		SetAIBoolParam, "", "", "")
	
	LUA_EXPOSED_METHOD(SetOffsetRadius,		SetOffsetRadius, "", "", "")
	LUA_EXPOSED_METHOD(SetShootingAccuracy,		SetShootingAccuracy, "", "", "")
	LUA_EXPOSED_METHOD(SetAlwaysMiss,		SetAlwaysMiss, "", "", "")
	LUA_EXPOSED_METHOD(SetWalkRunMovementController,		SetWalkRunMovementController, "", "", "")

	LUA_EXPOSED_METHOD(SetNavigGraphActiveLUA, SetNavigGraphActiveLUA, "", "", "")
	LUA_EXPOSED_METHOD(SetWhackAMoleNode, SetWhackAMoleNode, "", "", "")
	LUA_EXPOSED_METHOD(SetVolleyShots, SetVolleyShots, "", "", "")
	LUA_EXPOSED_METHOD(SetVolleySquad, SetVolleySquad, "", "", "")
	LUA_EXPOSED_METHOD(SetVolleyReloadPause, SetVolleyReloadPause, "", "", "")
	LUA_EXPOSED_METHOD(SetVolleyAimPause, SetVolleyAimPause, "", "", "")
	LUA_EXPOSED_METHOD(SetVolleyPauseBetweenShots, SetVolleyPauseBetweenShots, "", "", "")

	LUA_EXPOSED_METHOD(SetGoToLastKnownPlayerPosInAttack,		SetGoToLastKnownPlayerPosInAttack, "", "", "")
	LUA_EXPOSED_METHOD(GetRangedParameter,	GetRangedParameter, "", "", "")

	// Extended Methods
	LUA_EXPOSED_METHOD(MakeConscious,		Lua_MakeConscious, "Removes the recovering flag", "", "")
	LUA_EXPOSED_METHOD(UsingWeapon,		Lua_SetActionUsing, "", "", "" )
	LUA_EXPOSED_METHOD(SetControllerModifier,	Lua_SetControllerModifier, "Changes the movement controller based on what you're carrying", "", "AMCM enum")
	LUA_EXPOSED_METHOD(GetControllerModifier, Lua_GetControllerModifier, "Retrieves the current movement controller modifier", "", "")
	LUA_EXPOSED_METHOD(FindNearestWeapon,	Lua_FindNearestWeapon, "Finds the nearest specified weapon", "", "string for weapon-type (e.g crossbow), float for max-distance")
LUA_EXPOSED_END(CAIComponent)

//------------------------------------------------------------------------------------------
//!
//!	AI_Lua::Lua_MakeConscious
//! Removes the recovering flag
//!
//------------------------------------------------------------------------------------------
void AI_Lua::Lua_MakeConscious()
{
	((CAIComponent*)this)->GetCombatComponent().RegainConsciousness();
}

//------------------------------------------------------------------------------------------
//!
//!	AI_Lua::Lua_SetActionUsing
//! Set the item the entity is currently using.
//!
//------------------------------------------------------------------------------------------
void AI_Lua::Lua_SetActionUsing(int iFlag )
{
	((CAIComponent*)this)->SetActionUsing( (AI_ACTION_USING) iFlag );
}

//------------------------------------------------------------------------------------------
//!
//!	AI_Lua::Lua_SetControllerModifier
//! Set the current movement-controller modifier. That value is then used to choose between
//! standard walk/run, crossbow walk/run, bazooka walk/run, and so on.
//!
//------------------------------------------------------------------------------------------
void AI_Lua::Lua_SetControllerModifier(int iModifier)
{
	//Do nothing if we're already using this modifier.
	if(iModifier == ((CAIComponent*)this)->GetControllerModifier())
	{
		return;
	}

	//Otherwise check for each modifier type and set the appropriate controller if it exists.
	CAIComponent* pobAIComponent = ((CAIComponent*)this);
	const CAIComponentDef* pobAIComponentDef = ((CAIComponent*)this)->GetDefinition();

	//No modifier, default walk/run and strafe animations.
	if(iModifier == AMCM_NONE)
	{
		if(pobAIComponentDef->m_pobMovementSet->m_pobWalkingController)
		{
			pobAIComponent->SetWalkController(pobAIComponentDef->m_pobMovementSet->m_pobWalkingController);
		}
		else
		{
			ntPrintf("WARNING: m_pobWalkingController not set on this entity, leaving walk-controller as-is\n");
		}

		if(pobAIComponentDef->m_pobMovementSet->m_pobStrafingController)
		{
			pobAIComponent->SetStrafeController(pobAIComponentDef->m_pobMovementSet->m_pobStrafingController);
		}
		else
		{
			ntPrintf("WARNING: m_pobStrafingController not set on this entity, leaving strafe-controller as-is\n");
		}

		if(pobAIComponentDef->m_pobMovementSet->m_pobCloseStrafingController)
		{
			pobAIComponent->SetCloseStrafeController(pobAIComponentDef->m_pobMovementSet->m_pobCloseStrafingController);
		}
		else
		{
			ntPrintf("WARNING: m_pobCloseStrafingController not set on this entity, leaving closestrafe-controller as-is\n");
		}
	}
	else if((iModifier == AMCM_CROSSBOW) || (iModifier == AMCM_BAZOOKA))
	{
		if(pobAIComponentDef->m_pobMovementSet->m_pobCrossbowWalkingController)
		{
			pobAIComponent->SetWalkController(pobAIComponentDef->m_pobMovementSet->m_pobCrossbowWalkingController);
		}
		else
		{
			ntPrintf("WARNING: m_pobCrossbowWalkingController not set on this entity, leaving walk-controller as-is\n");
		}

		if(pobAIComponentDef->m_pobMovementSet->m_pobCrossbowStrafeController)
		{
			pobAIComponent->SetStrafeController(pobAIComponentDef->m_pobMovementSet->m_pobCrossbowStrafeController);
		}
		else
		{
			ntPrintf("WARNING: m_pobCrossbowStrafeController not set on this entity, leaving strafe-controller as-is\n");
		}

		if(pobAIComponentDef->m_pobMovementSet->m_pobCrossbowCloseStrafingController)
		{
			pobAIComponent->SetCloseStrafeController(pobAIComponentDef->m_pobMovementSet->m_pobCrossbowCloseStrafingController);
		}
		else
		{
			ntPrintf("WARNING: m_pobCrossbowCloseStrafingController was not set on this entity, leaving closestrafe-controller as-is\n");
		}
	}
	else
	{
		ntPrintf("WARNING: Invalid ai movement controller modifier enum-value passed to Lua_SetControllerModifier - %d\n", iModifier);
		return;	//So that we don't change CAIComponent::m_eControllerModifier below.
	}

	//Finally, update the stored modifier for retrieval/comparison later.
	((CAIComponent*)this)->SetControllerModifier((AI_MOVEMENT_CONTROLLER_MODIFIER)iModifier);
}

//------------------------------------------------------------------------------------------
//!
//!	AI_Lua::Lua_GetControllerModifier()
//! Retrieves the current controller-modifier of this AI so that we can decide to skip
//! dropping weapons if we're already down to our swords etc.
//!
//------------------------------------------------------------------------------------------
int AI_Lua::Lua_GetControllerModifier()
{
//NOTE: MOST OF THIS FUNCTION (all except the last line) is for debug-output.
//To use this, be sure to set the member-variables m_MyWalkController etc to be 'public' instead of private... but then remember to put it back after!
/*
	//Check for each modifier type and print the appropriate controller to double-check it corresponds to the Controller-Modifier value.
	CAIComponent* pobAIComponent = ((CAIComponent*)this);
	const CAIComponentDef* pobAIComponentDef = ((CAIComponent*)this)->GetDefinition();

	if(pobAIComponent->m_MyWalkController == pobAIComponentDef->m_pobMovementSet->m_pobWalkingController)
	{
		ntPrintf("Standard walking controller\n");
	}
	else if(pobAIComponent->m_MyWalkController == pobAIComponentDef->m_pobMovementSet->m_pobCrossbowWalkingController)
	{
		ntPrintf("Crossbow walking controller\n");
	}
	else
	{
		ntPrintf("Unknown walking controller\n");
	}

	if(pobAIComponent->m_MyStrafeController == pobAIComponentDef->m_pobMovementSet->m_pobStrafingController)
	{
		ntPrintf("Standard strafing controller\n");
	}
	else if(pobAIComponent->m_MyStrafeController == pobAIComponentDef->m_pobMovementSet->m_pobCrossbowStrafeController)
	{
		ntPrintf("Crossbow strafing controller\n");
	}
	else
	{
		ntPrintf("Unknown strafing controller\n");
	}

	if(pobAIComponent->m_MyCloseStrafeController == pobAIComponentDef->m_pobMovementSet->m_pobCloseStrafingController)
	{
		ntPrintf("Standard close-strafing controller\n");
	}
	else if(pobAIComponent->m_MyCloseStrafeController == pobAIComponentDef->m_pobMovementSet->m_pobCrossbowCloseStrafingController)
	{
		ntPrintf("Crossbow close-strafing controller\n");
	}
	else
	{
		ntPrintf("Unknown close-strafing controller\n");
	}

	int iControllerModifier = (int)(((CAIComponent*)this)->GetControllerModifier());
	if(iControllerModifier == AMCM_NONE)
	{
		ntPrintf("C++ NONE\n");
	}
	else if(iControllerModifier == AMCM_CROSSBOW)
	{
		ntPrintf("C++ CROSSBOW\n");
	}
	else if(iControllerModifier == AMCM_BAZOOKA)
	{
		ntPrintf("C++ BAZOOKA\n");
	}
	else
	{
		ntPrintf("C++ UNKNOWN\n");
	}
*/
	//This is the only line that needs to be here if we don't require debug-output.
	return (int)(((CAIComponent*)this)->GetControllerModifier());
}

//------------------------------------------------------------------------------------------
//!
//!	AI_Lua::Lua_FindNearestCrossbow()
//! Find the nearest weapon so that we can choose to run over and pick it up.
//! Pass in a sub-string that will be in the weapon you want (e.g. "crossbow" or "bazooka").
//!
//------------------------------------------------------------------------------------------
CEntity* AI_Lua::Lua_FindNearestWeapon(const char* pcWeaponType, float fMaxDistance)
{
	//Decide which type of weapon we want to find (still send from lua as string for now, change later).
	RANGED_WEAPON_TYPE eFindWeaponType = RWT_NONE;
	//For now we'll only want our AI to be picking up crossbows anyway.
	if(strcmp(pcWeaponType, "crossbow") == 0)
	{
		eFindWeaponType = RWT_CROSSBOW;
	}
	//If there wasn't a specified type, then just return NULL early.
	if(eFindWeaponType == RWT_NONE)
	{
		return NULL;
	}

	//Setup an entity query to match the requested weapon type.
	CEntityQuery obWeaponQuery;
	CEQCIsRangedWeaponWithType obRangedWeaponType(eFindWeaponType);
	obWeaponQuery.AddClause(obRangedWeaponType);
	CEntityManager::Get().FindEntitiesByType(obWeaponQuery, CEntity::EntType_Weapon);

	//Pick the closest weapon that matches the criteria and return it.
	CEntity* pWeaponEntity = NULL;
	//Start fClosest at our maximum-distance, so any outside that distance will not be returned.
	//If all are outside of this distance, then, NULL will be returned as set-up above.
	float fClosest = fMaxDistance * fMaxDistance;	//We keep it squared here to avoid potentially a lot of square-root calculations.

	//Get a pointer to ourselves so we can query the object's distance from us.
	CAIComponent* pobAIComponent = ((CAIComponent*)this);
	//Get our position here and store it!
	AI* pAI = pobAIComponent->GetParent();
	CPoint obMyPos = pAI->GetPosition();

	QueryResultsContainerType::iterator obEnd = obWeaponQuery.GetResults().end();
	for(QueryResultsContainerType::iterator obIt = obWeaponQuery.GetResults().begin() ; obIt != obEnd ; ++obIt)
	{
		//Skip weapons that are already being held.
		const CEntity* pParent = (*obIt)->GetParentEntity();
		if(pParent != NULL)	//Weapons parented only to the world should have a NULL parent entity.
		{
			continue;
		}

		//Otherwise, this weapon is parented only to the world, so see if it's closer.
		CPoint obWeaponLocation = (*obIt)->GetPosition();
		float fX = obWeaponLocation.X() - obMyPos.X();
		float fY = obWeaponLocation.Y() - obMyPos.Y();
		float fZ = obWeaponLocation.Z() - obMyPos.Z();
		float fSquaredDistance = (float)((fX*fX) + (fY*fY) + (fZ*fZ));
		//If this is the closest weapon so far, update our to-be-returned entity pointer.
		if(fSquaredDistance < fClosest)
		{
			fClosest = fSquaredDistance;
			pWeaponEntity = *obIt;
		}
	}

	return pWeaponEntity;
}
