//------------------------------------------------------------------------------------------
//!
//!	\file movement_lua.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/movement_lua.h"

#include "physics/system.h"
#include "physics/world.h"
#include <hkdynamics\phantom\hkPhantom.h>
#include <hkdynamics\phantom\hkSimpleShapePhantom.h>
#include <hkdynamics\phantom\hkAabbPhantom.h>
#include <hkdynamics\entity\hkRigidBody.h>
#include <hkcollide\shape\sphere\hkSphereShape.h>
#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkcollide\castutil\hkWorldRayCastOutput.h>
#include <hkcollide\collector\bodypaircollector\hkAllCdBodyPairCollector.h>
#include "physics\maths_tools.h"
#include "physics\havokthreadutils.h"

#include "core\visualdebugger.h"
#include "core\boundingvolumes.h"

#include "game\inputcomponent.h"
#include "game\renderablecomponent.h"
#include "game\archerwalkruncontroller.h"
#include "game\entitymanager.h"
#include "game\staticentity.h"
#include "game\continuationtransition.h"
#include "game\simpletransition.h"
#include "game/movement.h"
#include "game/luaglobal.h"
#include "game/interactiontransitions.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityarcher.h"
#include "game/turretcontroller.h"
#include "game/simpletransition.h"
#include "game/relativetransitions.h"
#include "game/targetedtransition.h"
#include "game/randmanager.h"


#include "camera/camutils.h"

#include "anim/hierarchy.h"
#include "anim/animator.h"

#include "objectdatabase/dataobject.h"


//------------------------------------------------------------------------------------------
// CMovement - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CMovement)
	// Expose Base Movement Methods
	LUA_EXPOSED_METHOD(ClearControllers,				ClearControllers, "Clears up all the content on the movement controller", "", "")
	LUA_EXPOSED_METHOD(SetInputDisabled,				SetInputDisabled, "Disable/Enable Input", "bool bDisable", "true to disable, false to enable")
	LUA_EXPOSED_METHOD(SetCompletionPhysicsControl,		SetCompletionPhysicsControl, "Set this entity to full physics control at the end of the current movement chain", "", "")

	// Expose Movement_Lua Methods
	LUA_EXPOSED_METHOD_RAW(SetMovementCompleteMessage,	Lua_SetMovementCompleteMessage, "Set the message to send back when the current controllers and chained controllers complete", "string Message, entity Receiver [OPT]", "The message to send|The Entity to send to - default itself")
	LUA_EXPOSED_METHOD_RAW(SetMovementInteruptMessage,	Lua_SetMovementInteruptMessage, "Set the message to send back when the current controllers and chained controllers are interrupted", "string Message, entity Receiver [OPT]", "The message to send|The Entity to send to - default itself")
	LUA_EXPOSED_METHOD(StartEmptyMovement,				Lua_StartEmptyMovement, "Will push a controller onto the movement system that does nothing", "", "")
	LUA_EXPOSED_METHOD(StartMovementFromXMLDef,			Lua_StartMovementFromXMLDef, "Will push a controller onto the movement system that from the name of its XML Def.", "string Definition", "Name of the Definition")
	LUA_EXPOSED_METHOD_RAW(ClampBones,					Lua_ClampBones, "", "", "")
	LUA_EXPOSED_METHOD(ActivateVehicle,					Lua_ActivateVehicle, "Will start a vehicular controller.", "string Definition, Entity Vehicle, string DrivingSeat, bool AIControlled", "The XML Definition|The Vehicle Entity|Transform name of the vehicles seating point|Is the entity controlling the vehicle an AI?")
	LUA_EXPOSED_METHOD(ChainMovementFromXMLDef,			Lua_ChainMovementFromXMLDef, "", "string Definition", "Name of the definition to chain")
	LUA_EXPOSED_METHOD_RAW(StartSimpleMovement,				Lua_StartSimpleMovement, "Starts a simple movement", "string Anim, bool Loop [=false], bool RandomOffset [=false]", "Name of the Animation|Loop the Anim, default to false|Start the anim at a random offset, default to false|Apply gravity, default to true")
	LUA_EXPOSED_METHOD(StartRelativeMovement,			Lua_StartRelativeMovement, "", "Entity RelativeEntity, string Animation, bool Loop, bool SoftMovement", "")
	LUA_EXPOSED_METHOD_RAW(ChainSimpleMovement,			Lua_ChainSimpleMovement, "", "string Anim, bool Loop [=false], bool RandomOffset [=false]", "")
	LUA_EXPOSED_METHOD(StartMoveToTransition,			Lua_StartMoveToTransition, "", "string Anim, Entity Target, float fDist, float fSpeed", "")
	LUA_EXPOSED_METHOD_RAW(StartFacingMovement,			Lua_StartFacingMovement, "", "string Anim, float AngularSpeed, float AnimSpeed, float StartTurnControl, float EndTurnControl, float BlendTime [OPT]", "")
	LUA_EXPOSED_METHOD_RAW(StartTargetedFacingMovement,	Lua_StartTargetedFacingMovement, "", "string Anim, float AngularSpeed, float AnimSpeed, float StartTurnControl, float EndTurnControl, float BlendTime [OPT]", "")
	LUA_EXPOSED_METHOD_RAW(StartSnapToMovement,			Lua_StartSnapToMovement, "", "string animation, entity target, Vector position, Vector rotation, bool gravity [OPT], string transform [OPT])", "")
	LUA_EXPOSED_METHOD_RAW(StartLinkedMovement,			Lua_StartLinkedMovement, "", "string Anim, entity AlignWith, string Transform, Vector Position, Vector Rotation", "")
	LUA_EXPOSED_METHOD(StartInteractiveAnimMovement,	Lua_StartInteractiveAnimMovement, "", "string Anim, Entity ControllerEnt, float MaxSpeed, float SpeedAcc, float SpeedDec, float ButtonPressInterval", "")
	LUA_EXPOSED_METHOD(CreateCrankLevelController,		Lua_CreateCrankLevelController, "", "string CrankParams", "")
	LUA_EXPOSED_METHOD(CreateCrankDoorController,		Lua_CreateCrankDoorController, "", "string CrankParams", "")
	LUA_EXPOSED_METHOD(CreateCrankOperatorController,	Lua_CreateCrankOperatorController, "", "string CrankParams", "")
	LUA_EXPOSED_METHOD(CrankDoorLock,					Lua_CrankDoorLock, "", "string CrankParams, bool Locked", "");
	LUA_EXPOSED_METHOD(LadderController,				Lua_LadderController, "", "string LadderParams, Entity pLadder", "")
	LUA_EXPOSED_METHOD(StartFallAftertouch,				Lua_StartFallAftertouch, "Will start a fall aftertouch controller", "string AnimName, float TimeScalar", "")
	LUA_EXPOSED_METHOD(CreateCounterWeightLeverController,		Lua_CreateCounterWeightLeverController, "", "string LeverParams", "")
	LUA_EXPOSED_METHOD(CreateCounterWeightOperatorController,	Lua_CreateCounterWeightOperatorController, "", "string LeverParams", "")
LUA_EXPOSED_END(CMovement)


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_SetMovementCompleteMessage
//!	Hands the movement component a message string that will be sent back to the script
//!	environment when the current controllers and chained controllers are complete
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_SetMovementCompleteMessage(NinjaLua::LuaState& State)
{
	NinjaLua::LuaStack args(State);
	ntAssert(args[1].IsString());

	if(args[2].Is<CEntity*>())
		((CMovement*)this)->SetCompletionMessage(args[1].GetString(), args[2].Get<CEntity*>());
	else
		((CMovement*)this)->SetCompletionMessage(args[1].GetString());

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_AltSetMovementCompleteMessage() - UNREGISTERED
//!	Hands the movement component a message string that will be sent back to the script
//!	environment when the current controllers and chained controllers are complete
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_AltSetMovementCompleteMessage(const char* pcMessage, CEntity* pEnt)
{
	ntAssert(pcMessage);

	if(pEnt)
		((CMovement*)this)->SetCompletionMessage(pcMessage, pEnt);
	else
		((CMovement*)this)->SetCompletionMessage(pcMessage);
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_SetMovementInteruptMessage
//!	Hands the movement component a message string that will be sent back to the script
//!	environment when the current controllers and chained controllers are interrupted
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_SetMovementInteruptMessage(NinjaLua::LuaState& State)
{
	NinjaLua::LuaStack args(State);
	ntAssert(args[1].IsString());

	if(args[2].Is<CEntity*>())
		((CMovement*)this)->SetInterruptMessage(args[1].GetString(), args[2].Get<CEntity*>());
	else
		((CMovement*)this)->SetInterruptMessage(args[1].GetString());

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartEmptyMovement
//!	Will push a controller onto the movement system that does nothing.
//!
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_StartEmptyMovement()
{
	return ((CMovement*)this)->BringInNewController(EmptyTransitionDef(), CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartMovementFromXMLDef
//!	Will push a controller onto the movement system that from the name of its XML Def.
//!
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_StartMovementFromXMLDef(CHashedString pcDefName)
{
	// Get a transition that will do its best to put us in the best movement state
	MovementControllerDef* pobDefintion = ObjectDatabase::Get().GetPointerFromName<MovementControllerDef*>(pcDefName);
	user_error_p(pobDefintion, ("Error! Movement definition %s not found\n",ntStr::GetString(pcDefName)));

	// Push an instance of the controller on to our movement component
	return ((CMovement*)this)->BringInNewController(*pobDefintion, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_Movement_ClampBones
//!	Will set the local transforms (post-upadte) of the curr_ent_bone_names to be the
//!	same as the bones in equiv_parent_ent_bone_names of the parent entity.
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_ClampBones(NinjaLua::LuaState& State)
{
	NinjaLua::LuaStack args(State);
	CEntity *child =  const_cast<CEntity*>(((CMovement*)this)->GetParentEntity()); // NASTY!

	ntAssert_p( child != NULL, ("You must have a valid target entity.") );
	
	const CEntity *parent = child->GetParentEntity();
	ntAssert_p( parent != NULL, ("The target entity must have a valid parent entity.") );

	// We need our entities to have hierarchies - which they should have anyway.
	ntAssert_p( child->GetHierarchy() != NULL, ("This entity much have a valid hierarchy.") );
	ntAssert_p( parent->GetHierarchy() != NULL, ("This entity much have a valid hierarchy.") );

	// We require two tables as arguments.
	ntAssert( args[ 1 ].IsTable() );
	ntAssert( args[ 2 ].IsTable() );

	NinjaLua::LuaObject child_bone_names( args[ 1 ] );
	NinjaLua::LuaObject parent_bone_names( args[ 2 ] );

	typedef ntstd::Vector< ntstd::pair< int, int > > BonePairVector;
	BonePairVector child_parent_bone_clamp;

	for (	NinjaLua::LuaIterator child_it( child_bone_names ), parent_it( parent_bone_names );
			child_it && parent_it;
			++child_it, ++parent_it )
	{
		const char *child_bone_name = child_it.GetValue().GetString();
		const char *parent_bone_name = parent_it.GetValue().GetString();

		int child_bone_idx = child->GetHierarchy()->GetTransformIndex( child_bone_name );
		int parent_bone_idx = parent->GetHierarchy()->GetTransformIndex( parent_bone_name );

		child_parent_bone_clamp.push_back( BonePairVector::value_type( child_bone_idx, parent_bone_idx ) );
	}

	CAnimator *child_animator = child->GetAnimator();
	ntAssert( child_animator != NULL );

	const CAnimator *parent_animator = parent->GetAnimator();
	ntAssert( parent_animator != NULL );

	child_animator->SetBonesToClamp( child_parent_bone_clamp, parent_animator );

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_Movement_ActivateVehicle
//!	Will push a vehicular controller onto the movement system that from the name 
//!	of its XML defintion.
//!
//!	N.B.	Having to use this bAIControlled flag is a bit sucky, but unfortunately necessary.
//!			This is because the movement controller def is owned by the vehicle and not the
//!			character using it, and the standard human player controller is dependent on the
//!			camera.
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_ActivateVehicle(CHashedString pcDefName, CEntity* pVehicle, CHashedString pcDrivingSeat, bool bAIControlled )
{
	// Get a transition that will do its best to put us in the best movement state
	TurretControllerDef* pDef = ObjectDatabase::Get().GetPointerFromName<TurretControllerDef*>(pcDefName);
	ntError_p(pDef, ("Error! Movement definition %s not found\n", ntStr::GetString(pcDefName)));

	TurretControllerDef obTempDef = *pDef;

	obTempDef.m_pVehicle = pVehicle;
	obTempDef.m_sDrivingSeat = pcDrivingSeat;
	obTempDef.m_bAIControlled = bAIControlled;

	// Push an instance of the controller on to our movement component
	((CMovement*)this)->BringInNewController( obTempDef, CMovement::DMM_STANDARD, 0.0f );
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_ChainMovementFromXMLDef
//!	Will push a controller onto the movement system that from the name of its XML
//!	defintion.  This controller will be used when the controller on the current stack,
//!	or the controller before it on the chained stack, has completed.
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_ChainMovementFromXMLDef(CHashedString pcDefinitionName)
{
	// Get a transition that will do its best to put us in the best movement state
	MovementControllerDef* pobDefintion = ObjectDatabase::Get().GetPointerFromName< MovementControllerDef* >( pcDefinitionName );
	ntAssert( pobDefintion );

	// Push an instance of the controller on to our movement component
	((CMovement*)this)->AddChainedController( *pobDefintion, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartSimpleMovement
//! Starts a simple movement on the movement component.  This functionality will need 
//!	to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_StartSimpleMovement( NinjaLua::LuaState& State )
{
	// Parameters:
	// 1. Animation name
	// 2. Looping (default=false)
	// 3. Random offset (default=false, only valid for looping
	// 4. Apply gravity

	NinjaLua::LuaStack args( State );

	// Define our movement
	SimpleTransitionDef obDef;
	obDef.m_obAnimationName = args[1].GetString();

	// See if we have a parameter for looping
	if ( args[2].IsBoolean() )
		obDef.m_bLooping = args[2].GetBoolean();
	
	// See if we are looping and we have a true boolean for argument three set an offset (doesn't need to be deterministic)
	if ( ( obDef.m_bLooping ) && ( args[3].IsBoolean() ) && ( args[3].GetBoolean() ) )
		obDef.m_fTimeOffsetPercentage = erandf(1.0f);

	// See if we have a parameter for applying gravity
	if ( args[4].IsBoolean() )
		obDef.m_bApplyGravity = args[4].GetBoolean();

	// Push the controller onto our movement component
	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartRelativeMovement
//! Start a relative movement.  If an entity to animate relative to is not supplied 
//! then the animation will be animated relative to the world origin
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_StartRelativeMovement(CEntity* pobRelativeEntity, CHashedString pcAnimationName, bool bLooping, bool bSoftMovement)
{
	// Make sure we have some reasonable data
	ntAssert(!ntStr::IsNull(pcAnimationName));

	// Create a definition for the movement
	SimpleRelativeTransitionDef obDef;
	obDef.m_pobAnimation = (const_cast<CEntity*>(((CMovement*)this)->GetParentEntity()))->GetAnimator()->CreateAnimation(pcAnimationName);

	// We are taking copies of a transform here - which isn't great - need to sort out the const issues here.
	obDef.m_pobRelativeTransform = NT_NEW Transform();

	// If we have a relative entity then we do the movement relative to that
	if ( pobRelativeEntity )
		obDef.m_pobRelativeTransform->SetLocalMatrix( pobRelativeEntity->GetRootTransformP()->GetWorldMatrix() );	

	//...otherwise we shall animate about the world origin
	else
		obDef.m_pobRelativeTransform->SetLocalMatrix( CMatrix( CONSTRUCT_IDENTITY ) );

	CHierarchy::GetWorld()->GetRootTransform()->AddChild( obDef.m_pobRelativeTransform );
	obDef.m_bOwnsTransform = true;

	// Set up the looping parameter
	obDef.m_bLooping = bLooping;

	// Push the controller onto our movement component
	if (bSoftMovement)
		((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_SOFT_RELATIVE, CMovement::MOVEMENT_BLEND );
	else
		((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_HARD_RELATIVE, CMovement::MOVEMENT_BLEND );
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_ChainSimpleMovement
//! Adds a simple movement on the movement component to be used when the existing one,
//! or any existing 'chained' ones have completed.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_ChainSimpleMovement( NinjaLua::LuaState& State )
{
	// Parameters:
	// 1. Animation name
	// 3. Looping (default=false)
	// 4. Random offset (default=false, only valid for looping

	NinjaLua::LuaStack args( State );

	// Define our movement
	SimpleTransitionDef obDef;
	obDef.m_obAnimationName = args[1].GetString();

	// See if we have a parameter for looping
	if ( args[2].IsBoolean() )
		obDef.m_bLooping = args[2].GetBoolean();
	
	// See if we are looping and we have a true boolean for argument three set an offset (doesn't need to be deterministic)
	if ( ( obDef.m_bLooping ) && ( args[3].IsBoolean() ) && ( args[3].GetBoolean() ) )
		obDef.m_fTimeOffsetPercentage = erandf(1.0f);

	// Push the controller onto our movement component
	((CMovement*)this)->AddChainedController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
		
	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartMoveToTransition
//!
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_StartMoveToTransition(CHashedString pcAnim, CEntity* pTarget, float fDist, float fSpeed)
{
	// Define our movement
	MoveToTransitionDef obDef;
	obDef.m_obAnimationName = pcAnim;
	obDef.m_pobTargetEntity = pTarget;
	obDef.m_fDistance = fDist;
	obDef.m_fAnimSpeed = fSpeed;
	obDef.m_fMaximumRotationSpeed = 180.0f;
	
	// Push the controller onto our movement component
	((CMovement*)this)->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
	
	return 0;
}

int Movement_Lua::Lua_AltStartMoveToTransition(CHashedString pcAnim, CEntity* pTarget, float fDist, float fSpeed, CPoint* obOffset)
{
	// Define our movement
	MoveToTransitionDef obDef;
	obDef.m_obAnimationName = pcAnim;
	obDef.m_pobTargetEntity = pTarget;
	obDef.m_fDistance = fDist;
	obDef.m_fAnimSpeed = fSpeed;
	obDef.m_fMaximumRotationSpeed = 180.0f;
	obDef.SetDebugNames( ntStr::GetString( pTarget->GetName() ), "MoveToTransitionDef" );
	if ( obOffset )
		obDef.m_obOffset = *obOffset;
	
	// Push the controller onto our movement component
	((CMovement*)this)->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
	
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartFacingMoveToTransition
//!
//!
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_StartFacingMoveToTransition(CHashedString pcAnim, CEntity* pTarget, float fDist, float fSpeed, CPoint* obOffsetLS, CQuat* pobRotOffset, CDirection* pobFacingDirLS)
{
	// Define our movement
	FacingMoveToTransitionDef obDef;
	obDef.m_obAnimationName = pcAnim;
	obDef.m_pobTargetEntity = pTarget;
	obDef.m_fDistance = fDist;
	obDef.m_fAnimSpeed = fSpeed;
	obDef.m_fMaximumRotationSpeed = 180.0f;
	obDef.SetDebugNames( ntStr::GetString( pTarget->GetName() ), "FacingMoveToTransitionDef" );
	obDef.m_bApplyGravity = false;
	if ( obOffsetLS )
		obDef.m_obOffsetLS = *obOffsetLS;


	if (pobFacingDirLS)
	{
		obDef.m_obFacingDirLS = *pobFacingDirLS;
	}
	else
	{
		// this is for the old system, where rotation angles were specified.
		CMatrix obMatrix = obDef.m_pobTargetEntity->GetMatrix();	
		obMatrix.SetTranslation(CPoint(0.0f,0.0f,0.0f));
		CDirection obFacingVector (CPoint(0.0f,0.0f,1.0f));

		CQuat obRotOffset(CONSTRUCT_IDENTITY);

		if ( pobRotOffset )
		{
			obRotOffset = *pobRotOffset;
		}
	
		CMatrix obRotMatrix;
		obRotMatrix.SetFromQuat(obRotOffset);
		obDef.m_obFacingDirLS = obFacingVector * obRotMatrix;
	}


	// Push the controller onto our movement component
	return ((CMovement*)this)->BringInNewController(obDef, CMovement::DMM_HARD_RELATIVE, CMovement::MOVEMENT_BLEND);
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartFacingMovement
//! Starts a simple movement on the movement component.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_StartFacingMovement( NinjaLua::LuaState& State )
{
	// This function takes 5 arguments:
	// 1. Animation name
	// 2. Angular speed - the rotational speed the character can turn
	// 3. Anim speed
	// 4. Start control time - the time offset in the anim where the player can start turning
	// 5. End control time - the time offset in the anim where the player can no longer turn
	// 6. Blend time - optional

	NinjaLua::LuaStack args(State);

	// Define our movement
	FacingTransitionDef obDef;
	obDef.m_obAnimationName=args[1].GetString();
	obDef.m_fAngularSpeed=args[2].GetFloat();
	obDef.m_fAnimSpeed=args[3].GetFloat();
	obDef.m_fStartTurnControl=args[4].GetFloat();
	obDef.m_fEndTurnControl=args[5].GetFloat();
	obDef.SetDebugNames( "", "FacingTransitionDef" );
	

	// Push the controller onto our movement component
	if (args[6].IsNumber())
	{
		((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, args[6].GetFloat() ); // Use 
	}
	else
	{
		((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND ); // Use default blend time
	}

	return (0);
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_AltStartFacingMovement
//! Alternate StartFacingMovement function to get rid of LuaState parameter
//!
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_AltStartFacingMovement( CHashedString pcAnimName, float fAngularSpeed, float fAnimSpeed, float fStartTurnControl, float fEndTurnControl, float fBlendTime)
{
	// This function takes 5 arguments:
	// 1. Animation name
	// 2. Angular speed - the rotational speed the character can turn
	// 3. Anim speed
	// 4. Start control time - the time offset in the anim where the player can start turning
	// 5. End control time - the time offset in the anim where the player can no longer turn
	// 6. Blend time - optional

	// Define our movement
	FacingTransitionDef obDef;
	obDef.m_obAnimationName = pcAnimName;
	obDef.m_fAngularSpeed = fAngularSpeed;
	obDef.m_fAnimSpeed = fAnimSpeed;
	obDef.m_fStartTurnControl = fStartTurnControl;
	obDef.m_fEndTurnControl = fEndTurnControl;

	if (fBlendTime >= 0.0f)
	{
		return ((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, fBlendTime );
	}
	else
	{
		return ((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND ); // Use default blend time
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartTargetedFacingMovement
//! Starts a simple movement on the movement component.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_StartTargetedFacingMovement ( NinjaLua::LuaState& State )
{
	// This function takes 5 arguments:
	// 1. Animation name
	// 2. Angular speed - the rotational speed the character can turn
	// 3. Anim speed
	// 4. Start control time - the time offset in the anim where the player can start turning
	// 5. End control time - the time offset in the anim where the player can no longer turn
	// 6. Blend time - optional

	NinjaLua::LuaStack args(State);

	// Define our movement
	FacingTransitionDef obDef;
	obDef.m_obAnimationName=args[1].GetString();
	obDef.m_fAngularSpeed=args[2].GetFloat();
	obDef.m_fAnimSpeed=args[3].GetFloat();
	obDef.m_fStartTurnControl=args[4].GetFloat();
	obDef.m_fEndTurnControl=args[5].GetFloat();
	obDef.m_fAlignToThrowTarget=true;
	
	// Push the controller onto our movement component
	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_AltStartTargetedFacingMovement
//! Alternative version.
//!
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_AltStartTargetedFacingMovement( CHashedString pcAnimName, float fAngularSpeed, float fAnimSpeed, float fStartTurnControl, float fEndTurnControl, float fBlendTime )
{
	// This function takes 5 arguments:
	// 1. Animation name
	// 2. Angular speed - the rotational speed the character can turn
	// 3. Anim speed
	// 4. Start control time - the time offset in the anim where the player can start turning
	// 5. End control time - the time offset in the anim where the player can no longer turn
	// 6. Blend time - optional

	// Define our movement
	FacingTransitionDef obDef;
	obDef.m_obAnimationName = pcAnimName;
	obDef.m_fAngularSpeed = fAngularSpeed;
	obDef.m_fAnimSpeed = fAnimSpeed;
	obDef.m_fStartTurnControl = fStartTurnControl;
	obDef.m_fEndTurnControl = fEndTurnControl;
	obDef.m_fAlignToThrowTarget = true;
	
	// Push the controller onto our movement component
	if ( fBlendTime >= 0.0f )
	{
		return ((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, fBlendTime );
	}
	else
	{
		return ((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	}


	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartSnapToMovement
//! Params: (string animation, entity target, Vector position, Vector rotation, bool gravity [OPT], string transform [OPT])
//! Starts a simple movement on the movement component.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_StartSnapToMovement ( NinjaLua::LuaState& State )
{
	NinjaLua::LuaStack args(State);

	ntAssert( args[3].IsTable() );
	NinjaLua::LuaObject obTranslationOffset( args[3] );

	ntAssert( args[4].IsTable() );
	NinjaLua::LuaObject obRotationOffset( args[4] );

	SnapToTransitionDef obDef;

	if (args[1].IsString())
	{
		obDef.m_obAnimationName=args[1].GetString();
	}
	else
	{
		obDef.m_obAnimationName = CHashedString();
	}

	if (args[2].IsUserData())
	{
		obDef.m_pobTargetEntity=args[2].Get<CEntity*>();
	}
	else
	{
		obDef.m_pobTargetEntity=0;
	}

	if (args[5].IsBoolean())
	{
		obDef.m_bApplyGravity=args[5].GetBoolean();
	}

	if(args[6].IsString())
		obDef.m_sTransformName = args[6].GetString();

	obDef.m_obTranslationOffset.X()=obTranslationOffset["x"].GetFloat();
	obDef.m_obTranslationOffset.Y()=obTranslationOffset["y"].GetFloat();
	obDef.m_obTranslationOffset.Z()=obTranslationOffset["z"].GetFloat();
	obDef.m_obRotationOffset.X()=obRotationOffset["x"].GetFloat();
	obDef.m_obRotationOffset.Y()=obRotationOffset["y"].GetFloat();
	obDef.m_obRotationOffset.Z()=obRotationOffset["z"].GetFloat();
	obDef.m_obRotationOffset.W()=obRotationOffset["w"].GetFloat();

	// Push the controller onto our movement component
	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
	
	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_AltStartSnapToMovement
//! Params: (string animation, entity target, Vector position, Vector rotation, bool gravity [OPT], string transform [OPT])
//! Starts a simple movement on the movement component.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_AltStartSnapToMovement(CHashedString pcAnimName, CEntity* pobTarg, CPoint& obPosition, CQuat& obRotation, bool bGravity)
{
	SnapToTransitionDef obDef;
	obDef.m_obAnimationName=pcAnimName;
	obDef.m_pobTargetEntity=pobTarg;
	obDef.m_bApplyGravity=bGravity;
	obDef.m_obTranslationOffset = obPosition;
	obDef.m_obRotationOffset = obRotation;
	
	// Push the controller onto our movement component
	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
}

void Movement_Lua::Lua_AltStartSnapToMovement(CHashedString pcAnimName, CEntity* pobTarg, CPoint& obPosition, CQuat& obRotation, bool bGravity, CHashedString pcTransform)
{
	SnapToTransitionDef obDef;
	obDef.m_obAnimationName=pcAnimName;
	obDef.m_pobTargetEntity=pobTarg;
	obDef.m_bApplyGravity=bGravity;
	obDef.m_sTransformName = pcTransform;
	obDef.m_obTranslationOffset = obPosition;
	obDef.m_obRotationOffset = obRotation;
	
	// Push the controller onto our movement component
	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartLinkedMovement
//! Starts a simple movement on the movement component.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
int Movement_Lua::Lua_StartLinkedMovement(NinjaLua::LuaState& State)
{
	NinjaLua::LuaStack args(State);

	LinkedMovementTransitionDef obDef;

	if (args[1].IsString()) // Animation
		obDef.m_obAnimationName=args[1].GetString();

	if (args[2].IsUserData()) // Entity we are aligned with
		obDef.m_pobTargetEntity=args[2].Get<CEntity*>();

	if (args[3].IsString()) // Transform on that entity we are aligned with
		obDef.m_obTargetTransform=args[3].GetString();

	if (args[4].IsTable()) // Translation offset
	{
		NinjaLua::LuaObject obTranslationOffset( args[4] );
		obDef.m_obTranslationOffset.X()=obTranslationOffset["x"].GetFloat();
		obDef.m_obTranslationOffset.Y()=obTranslationOffset["y"].GetFloat();
		obDef.m_obTranslationOffset.Z()=obTranslationOffset["z"].GetFloat();
	}
	
	if (args[5].IsTable()) // Rotation offset (euler in degrees)
	{
		NinjaLua::LuaObject obRotationOffset( args[5] );
	
		obDef.m_obRotationOffset=CCamUtil::QuatFromEuler_XYZ(
									obRotationOffset["x"].GetFloat(),
									obRotationOffset["y"].GetFloat(),
									obRotationOffset["z"].GetFloat());
	}
	
	obDef.SetDebugNames( ntStr::GetString( obDef.m_pobTargetEntity->GetName() ), "LinkedMovementTransitionDef" );


	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );

	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_AltStartLinkedMovement
//! Starts a simple movement on the movement component.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//!	Not exposed to Lua, a duplicate to allow C++ to use it without needing NinjaLua::LuaState& State
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_AltStartLinkedMovement(CHashedString obAnim, CEntity* pobEnt, CHashedString obTransform, CPoint& obTranslation, CPoint& obRotation)
{
	LinkedMovementTransitionDef obDef;

	// Animation
	obDef.m_obAnimationName=obAnim;

	// Entity we are aligned with
	obDef.m_pobTargetEntity=pobEnt;

	// Transform on that entity we are aligned with
	obDef.m_obTargetTransform = obTransform;

	// Translation offset
	obDef.m_obTranslationOffset = obTranslation;
	obDef.m_obRotationOffset=CCamUtil::QuatFromEuler_XYZ( obRotation.X(), obRotation.Y(), obRotation.Z() );
	
	obDef.SetDebugNames( ntStr::GetString( obDef.m_pobTargetEntity->GetName() ), "LinkedMovementTransitionDef" );

	return ((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_AltStartFullyLinkedMovement
//! Starts a simple movement on the movement component.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//!	Not exposed to Lua, a duplicate to allow C++ to use it without needing NinjaLua::LuaState& State
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_AltStartFullyLinkedMovement(CHashedString obAnim, CEntity* pobEnt, CHashedString obTransform, CPoint& obTranslation, CPoint& obRotation)
{
	FullyLinkedMovementTransitionDef obDef;

	// Animation
	obDef.m_obAnimationName=obAnim;

	// Entity we are aligned with
	obDef.m_pobTargetEntity=pobEnt;

	// Transform on that entity we are aligned with
	obDef.m_obTargetTransform = obTransform;

	// Translation offset
	obDef.m_obTranslationOffset = obTranslation;
	obDef.m_obRotationOffset=CCamUtil::QuatFromEuler_XYZ( obRotation.X(), obRotation.Y(), obRotation.Z() );
	
	obDef.SetDebugNames( ntStr::GetString( obDef.m_pobTargetEntity->GetName() ), "FullyLinkedTransitionDef" );

	return ((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_AltStartCorrectiveMovement
//! Starts a simple movement on the movement component.  This functionality will need 
//! to be extended in the future so we can pass movement definition structures.
//!
//!	Not exposed to Lua, a duplicate to allow C++ to use it without needing NinjaLua::LuaState& State
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_AltStartCorrectiveMovement(CPoint& obTargetPos, CQuat& obTargetRotation)
{
	CorrectiveMovementTransitionDef obDef;

	obDef.SetDebugNames( "", "CorrectiveTransitionDef" );

	obDef.m_obTargetPostion = obTargetPos;
	obDef.m_obTargetRotation = obTargetRotation;

	return ((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartInteractiveAnimMovement
//! An anim whose speed is controlled by action button presses.
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_StartInteractiveAnimMovement(CHashedString pcAnim, CEntity* pControllerEnt, float fMaxSpeed, float fSpeedAcc, float fSpeedDec, float fButtonPressInterval)
{
	InteractiveAnimControllerDef obDef;

	obDef.m_obAnimationName=pcAnim;
	obDef.m_pobControllingEntity=pControllerEnt;
	obDef.m_fAnimMaxSpeed=fMaxSpeed;
	obDef.m_fAnimSpeedAcceleration=fSpeedAcc;
	obDef.m_fAnimSpeedDeacceleration=fSpeedDec;
	obDef.m_fButtonPressInterval=fButtonPressInterval;

	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_AltStartSimpleMovement()
//! Starts a simple movement on the movement component.  This functionality will need 
//!	to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_AltStartSimpleMovement(CHashedString& pcAnimName, bool bLooping, bool bRandomOffset, bool bGravity)
{
	//Define our movement.
	SimpleTransitionDef obDef;
	obDef.m_obAnimationName = pcAnimName;

	obDef.SetDebugNames(ntStr::GetString(obDef.m_obAnimationName), "SimpleTransitionDef");
	//Set looping parameter.
	obDef.m_bLooping = bLooping;

	//Set random offset (for looping).
	if((obDef.m_bLooping) && (bRandomOffset))
	{
		obDef.m_fTimeOffsetPercentage = erandf(1.0f);
	}

    //Apply gravity if necessary.
	obDef.m_bApplyGravity = bGravity;

	//Push the controller onto our movement component.
	return ((CMovement*)this)->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_StartCoordinatedMovement()
//! Starts a simple movement on the movement component.  This functionality will need 
//!	to be extended in the future so we can pass movement definition structures.
//!
//------------------------------------------------------------------------------------------
bool Movement_Lua::Lua_StartCoordinatedMovement(CHashedString& pcAnimName, bool bLooping, bool bGravity, CoordinationParams* pobCoordParams)
{
	//Define our movement.
	CoordinatedTransitionDef obDef;
	obDef.m_obAnimationName = pcAnimName;

	//Set looping parameter.
	//obDef.m_bLooping = bLooping;

    //Apply gravity if necessary.
	obDef.m_bApplyGravity = bGravity;

	obDef.m_pobCoordinationParams = pobCoordParams;

	//Push the controller onto our movement component.
	return ((CMovement*)this)->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_Movement_CreateCrankLevelController
//!
//!
//!-------------------------------------------------------------------------------------------------
void Movement_Lua::Lua_CreateCrankLevelController(const char* pcCrankParams)
{
	/*
	CrankLeverControllerDef obDef;
	obDef.m_obCrankParameters=pcCrankParams;

	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
	*/
	UNUSED (pcCrankParams);
#ifndef _RELEASE
	ntPrintf("Movement_Lua::Lua_CreateCrankLevelController: Please use button mash object for crank lever\n");
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_Movement_CreateCrankDoorController(string parameters)
//!
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_CreateCrankDoorController(const char* pcCrankParams)
{
	/*
	CrankDoorControllerDef obDef;
	obDef.m_obCrankParameters=pcCrankParams;

	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
	*/
	UNUSED (pcCrankParams);
#ifndef _RELEASE
	ntPrintf("Movement_Lua::Lua_CreateCrankDoorController: Please use button mash object for crank lever\n");
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_CreateCrankOperatorController(string parameters)
//!
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_CreateCrankOperatorController(const char* pcCrankParams)
{
	/*
	CrankOperatorControllerDef obDef;
	obDef.m_obCrankParameters=pcCrankParams;

	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
	*/
	UNUSED (pcCrankParams);
#ifndef _RELEASE
	ntPrintf("Movement_Lua::Lua_CreateCrankDoorController: Please use button mash object for crank lever\n");
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_CrankDoorLock(string parameters,bool lock)
//!
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_CrankDoorLock(const char* pcCrankParams, bool bLocked)
{
/*
	CrankSharedParameters* pobParameters = ObjectDatabase::Get().GetPointerFromName<CrankSharedParameters*>(pcCrankParams);

	if (pobParameters)
	{
		pobParameters->m_bDoorLocked=bLocked;
	}
#ifndef _RELEASE
	else
	{
		ntPrintf("Movement_CrankDoorLock: %s not found\n",pcCrankParams);
	}
#endif // RELEASE
*/
	UNUSED (pcCrankParams);
	UNUSED (bLocked);
#ifndef _RELEASE
	ntPrintf("Movement_CrankDoorLock: Cranks replaced by button mash objects\n");
#endif // RELEASE
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_Movement_LadderController(string parameters,entity ladder)
//!
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_LadderController(CHashedString LadderParams, CEntity* pLadder, bool bChained)
{
	LadderControllerDef obDef;
	obDef.m_pobLadderParameters=ObjectDatabase::Get().GetPointerFromName<LadderParameters*>(LadderParams);
	obDef.m_pobLadderEntity=pLadder;

	obDef.SetDebugNames( ntStr::GetString( pLadder->GetName() ), "LadderControllerDef" );

	if(obDef.m_pobLadderParameters)
	{
		if (bChained)
			((CMovement*)this)->AddChainedController(obDef, CMovement::DMM_HARD_RELATIVE,0.25f);	
		else
			((CMovement*)this)->BringInNewController(obDef, CMovement::DMM_HARD_RELATIVE,0.25f);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::StartFallAftertouch()
//!
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_StartFallAftertouch(			CEntity*		pobFallingEntity,
													CHashedString		psAnimName,
													float				fMaxVelocity,
													float				fAccelFactor,
													CEntity*		pobControllingEntity )
{

	ntAssert(pobFallingEntity);
	ntAssert(pobControllingEntity);
	// Create our movement definition - aftertouch controll of falling object
	FallAftertouchTransitionDef obFallAftertouchDefinition;
	obFallAftertouchDefinition.SetDebugNames( ntStr::GetString( pobFallingEntity->GetName() ), "FallAftertouchTransitionDef" );

	// Set up the parameters
	obFallAftertouchDefinition.m_obAnimationName = psAnimName;
	obFallAftertouchDefinition.m_pobParentEntity = pobFallingEntity;
	obFallAftertouchDefinition.m_pobControllingEntity = pobControllingEntity;
	obFallAftertouchDefinition.m_fMaxHorizontalVelocity = fMaxVelocity;
	obFallAftertouchDefinition.m_fHorizontalAccelFactor = fAccelFactor;
	obFallAftertouchDefinition.m_fMaxVerticalVelocity = 12.0f;
	obFallAftertouchDefinition.m_fVerticalAccel = -4.0f;
	
	// Push the controller on to the movement component - no blend
	obFallAftertouchDefinition.m_pobParentEntity->GetMovement()->BringInNewController( obFallAftertouchDefinition, CMovement::DMM_STANDARD, 0.0f );
}

//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_Movement_CreateCounterWeightLeverController
//!
//!
//!-------------------------------------------------------------------------------------------------
void Movement_Lua::Lua_CreateCounterWeightLeverController(const char* pcLeverParams)
{
	/*
	CounterWeightLeverControllerDef obDef;
	obDef.m_obLeverParameters=pcLeverParams;

	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
	ntPrintf("Movement_Lua::CreateCounterWeightLeverController\n");
	*/
	UNUSED (pcLeverParams);
#ifndef _RELEASE
	ntPrintf("Movement_Lua::CreateCounterWeightLeverController: Please use button mash object for counterweight lever\n");
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	Movement_Lua::Lua_CreateCrankOperatorController(string parameters)
//!
//!
//------------------------------------------------------------------------------------------
void Movement_Lua::Lua_CreateCounterWeightOperatorController(const char* pcLeverParams)
{
	/*
	CounterWeightLeverOperatorControllerDef obDef;
	obDef.m_obLeverParameters=pcLeverParams;

	((CMovement*)this)->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
	ntPrintf("Movement_Lua::CreateCounterWeightOperatorController\n");
	*/
	UNUSED (pcLeverParams);
#ifndef _RELEASE
	ntPrintf("Movement_Lua::CreateCounterWeightOperatorController: Please use button mash object for counterweight lever\n");
#endif
}


