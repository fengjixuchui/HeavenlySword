//------------------------------------------------------------------------------------------
//!
//!	\file combat_lua.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "physics/system.h"
#include "game/luaglobal.h"
#include "game/luahelp.h"
#include "game/shellconfig.h"

#include "Physics/world.h"
#include "Physics/rigidbody.h"
#include "Physics/singlerigidlg.h"
#include "Physics/collisionbitfield.h"
#include "Physics/animatedlg.h"
#include "Physics/staticlg.h"
#include "Physics/compoundlg.h"
#include "Physics/projectilelg.h"
#include "Physics/spearlg.h"
#include "Physics/maths_tools.h"
#include "Physics/havokthreadutils.h"

#include "objectdatabase/dataobject.h"
#include "core/exportstruct_anim.h"
#include "core/exportstruct_clump.h"
#include "game/awareness.h"
#include "game/movement.h"
#include "game/entitymanager.h"
#include "game/query.h"
#include "game/messagehandler.h"
#include "game/renderablecomponent.h"
#include "game/aftertouchcontroller.h"
#include "game/interactiontransitions.h"
#include "game/playeroptions.h"
#include "camera/camutils.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "anim/animator.h"
#include "game/randmanager.h"

// For explosions
#include "game/entityinfo.h"
#include "game/attacks.h"
#include "game/syncdcombat.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	#include "Physics/advancedcharactercontroller.h"
	#include <hkdynamics/entity/hkRigidBody.h>
	#include <hkcollide/collector/bodypaircollector/hkAllCdBodyPairCollector.h>
#endif


//------------------------------------------------------------------------------------------
// Physics::System - Lua Interface
// JML NOTE - NOW THAT AISTATE.LUA IS NO LONGER USED CAN WE REMOVE UNNECESSARY BINDINGS?
//------------------------------------------------------------------------------------------
namespace Physics
{
	LUA_EXPOSED_START(System)
		// Base Functions
		LUA_EXPOSED_METHOD(RegisterPhysicsSoundDef,					RegisterCollisionEffectFilterDef,			"Register a Physics Sound Definition with this entity.", "string defintion", "the definition to use")
		LUA_EXPOSED_METHOD(DeactivateAllStates,		Deactivate,				 "", "", "")

		// Extended Functions
		LUA_EXPOSED_METHOD(ActivateState,							Lua_ActivateState,							"", "string name", "")
		LUA_EXPOSED_METHOD(DeactivateState,							Lua_DeactivateState,						"", "string name", "")
		LUA_EXPOSED_METHOD(SetHoldingCapsule,						Lua_SetHoldingCapsule,						"", "bool size", "")
		LUA_EXPOSED_METHOD(ActivateParticleOnContact,				Lua_ActivateParticleOnContact,				"", "", "")
		LUA_EXPOSED_METHOD(SetLookAtEntity,							Lua_SetLookAtEntity,						"", "entity target", "")
		LUA_EXPOSED_METHOD(ResetLookAt,								Lua_ResetLookAt,							"", "", "")
		LUA_EXPOSED_METHOD(SetLinearVelocity,						Lua_SetLinearVelocity,						"", "vector velocity", "")
		LUA_EXPOSED_METHOD(SetAngularVelocity,						Lua_SetAngularVelocity,						"", "vector velocity", "")
		LUA_EXPOSED_METHOD(SetLinearVelocityFromTarget,				Lua_SetLinearVelocityFromTarget,			"", "entity ControllingEntity, vector inputVelocity", "")	// Currently Unused...
		LUA_EXPOSED_METHOD(SetLinearVelocityFromCamera,				Lua_SetLinearVelocityFromCamera,			"", "vector inputVelocity", "")							// Currently Unused...
		LUA_EXPOSED_METHOD(ApplyLinearImpulse,						Lua_ApplyLinearImpulse,						"", "vector impulse", "")									// Currently Unused...
		LUA_EXPOSED_METHOD(Animated_Collapse,						Lua_Animated_Collapse,						"", "", "")
		LUA_EXPOSED_METHOD(Ragdoll_Freeze,							Lua_Ragdoll_Freeze,							"", "", "")
		LUA_EXPOSED_METHOD(Ragdoll_SetMotionType,					Lua_Ragdoll_SetMotionType,					"", "number montionType", "")
		LUA_EXPOSED_METHOD(Ragdoll_Reparent,						Lua_Ragdoll_Reparent,						"", "entity parent", "")									// Currently Unused...
		LUA_EXPOSED_METHOD(Ragdoll_CheckAtRest,						Lua_Ragdoll_CheckAtRest,					"", "", "")
		LUA_EXPOSED_METHOD(Ragdoll_SetBoneTransformTrackingMapping,	Lua_Ragdoll_SetBoneTransformTrackingMapping,	"", "int boneflag, entity hero, char namedtransform", "")
		LUA_EXPOSED_METHOD(Ragdoll_SetAnimatedBones,				Lua_Ragdoll_SetAnimatedBones,	"", "int boneflags", "")
		LUA_EXPOSED_METHOD(Ragdoll_AddAnimatedBone,					Lua_Ragdoll_AddAnimatedBone,	"", "int boneflag", "")
		LUA_EXPOSED_METHOD(Ragdoll_SetTurnDynamicOnContact,			Lua_Ragdoll_SetTurnDynamicOnContact,	"", "bool turn", "")
		LUA_EXPOSED_METHOD(Ragdoll_SetExemptFromCleanup,			Lua_Ragdoll_SetExemptFromCleanup,	"", "bool exemption", "")
		LUA_EXPOSED_METHOD(Ragdoll_Twitch,							Lua_Ragdoll_Twitch,	"", "", "")
		LUA_EXPOSED_METHOD(Ragdoll_ApplyLinearImpulse,				Lua_Ragdoll_ApplyLinearImpulse,	"", "vector impulse", "")
		LUA_EXPOSED_METHOD(Ragdoll_ApplyAngularImpulse,				Lua_Ragdoll_ApplyAngularImpulse,	"", "vector impulse", "")
		LUA_EXPOSED_METHOD(Ragdoll_SetBeingThrown, 					Lua_Ragdoll_SetBeingThrown,					"", "float fDistanceMultiplier, float fAngleMultiplier, string target", "")
		LUA_EXPOSED_METHOD(Ragdoll_SetMinTumbleDistance, 			Lua_Ragdoll_SetMinTumbleDistance,			"", "float fDist", ""); 	
		LUA_EXPOSED_METHOD(CharacterController_SetRagdollCollidable,Lua_CharacterController_SetRagdollCollidable,	"", "bool collide", "")		
		LUA_EXPOSED_METHOD(AttachedFromLeftFoot,					Lua_AttachedFromLeftFoot,					"", "", "")												
		LUA_EXPOSED_METHOD(Rigid_CheckAtRest,						Lua_Rigid_CheckAtRest,						"", "", "")
		LUA_EXPOSED_METHOD(Rigid_ResetCollisionFilters,				Lua_Rigid_ResetCollisionFilters,			"", "", "")												// Currently Unused...
		LUA_EXPOSED_METHOD(Rigid_CollideWithPlayerOnly,				Lua_Rigid_CollideWithPlayerOnly,			"", "", "")
		LUA_EXPOSED_METHOD(Rigid_CollideWithEnemiesOnly,			Lua_Rigid_CollideWithEnemiesOnly,			"", "", "")												// Currently Unused...
		LUA_EXPOSED_METHOD(Rigid_CollideWithPlayerAndEnemiesOnly,	Lua_Rigid_CollideWithPlayerAndEnemiesOnly,	"", "", "")												// Currently Unused...
		LUA_EXPOSED_METHOD(Rigid_PushFromPlayer,					Lua_Rigid_PushFromPlayer,					"", "", "")
		LUA_EXPOSED_METHOD(Rigid_CheckIfMoving,						Lua_Rigid_CheckIfMoving,					"", "", "")												// Currently Unused...
		LUA_EXPOSED_METHOD(Rigid_SetMotionType,						Lua_Rigid_SetMotionType,					"", "", "")
		LUA_EXPOSED_METHOD(Rigid_SetKeyframedMotion,				Lua_Rigid_SetKeyframedMotion,				"", "", "")
		LUA_EXPOSED_METHOD(Rigid_SetBreakableKeyframedMotion,		Lua_Rigid_SetBreakableKeyframedMotion,		"", "", "")
		LUA_EXPOSED_METHOD(Rigid_IsInBreakableKeyframedMotion,		Lua_Rigid_IsInBreakableKeyframedMotion,		"", "", "")
		LUA_EXPOSED_METHOD(Rigid_SetRotationRelativeToEntitySafe,	Lua_Rigid_SetRotationRelativeToEntitySafe,		"", "", "")
		LUA_EXPOSED_METHOD(Rigid_MoveRelativeToEntitySafe,			Lua_Rigid_MoveRelativeToEntitySafe,		"", "", "")
		LUA_EXPOSED_METHOD(Rigid_AntiGravity,						Lua_Rigid_AntiGravity,						"", "", "")
		LUA_EXPOSED_METHOD(CompoundRigid_CheckAtRest,				Lua_CompoundRigid_CheckAtRest,				"", "", "")
		LUA_EXPOSED_METHOD(CompoundRigid_SetMotionType,				Lua_CompoundRigid_SetMotionType,			"", "number motionType", "")								// Currently Unused...
		LUA_EXPOSED_METHOD(CompoundRigid_Collapse,					Lua_CompoundRigid_Collapse,					"", "", "")
		LUA_EXPOSED_METHOD(RemoveChildEntities,						Lua_RemoveChildEntities,					"", "", "")
		LUA_EXPOSED_METHOD(Rigid_OrientateToVelocity,				Lua_Rigid_OrientateToVelocity,				"", "", "")
		LUA_EXPOSED_METHOD(Rigid_EnableVelocityReflection,			Lua_Rigid_EnableVelocityReflection,			"", "boolean enable", "")
		LUA_EXPOSED_METHOD(Rigid_DeflectionRender,					Lua_Rigid_DeflectionRender,					"", "boolean enable", "")
		LUA_EXPOSED_METHOD(DoHurtOnCollision,						Lua_DoHurtOnCollision,						"", "boolean enable", "")
		LUA_EXPOSED_METHOD(CollisionStrike,							Lua_CollisionStrike,						"", "boolean enable", "")
		LUA_EXPOSED_METHOD(Rigid_SetController,						Lua_SetController,							"", "entity controller, string paramDef", "")
		LUA_EXPOSED_METHOD(Rigid_EnableSteering,					Lua_Rigid_EnableSteering,					"", "entity controller", "")
		LUA_EXPOSED_METHOD(Rigid_DisableSteering,					Lua_Rigid_DisableSteering,					"", "", "")
		LUA_EXPOSED_METHOD(Spear_SetMotionType,						Lua_Spear_SetMotionType,					"", "number motionType", "")
		LUA_EXPOSED_METHOD(Spear_StartThrownBehaviour,				Lua_Spear_StartThrownBehaviour,				"", "", "")
		LUA_EXPOSED_METHOD(Spear_StopThrownBehaviour,				Lua_Spear_StopThrownBehaviour,				"", "", "")
		LUA_EXPOSED_METHOD(Spear_GenerateAtRestMessage,				Lua_Spear_GenerateAtRestMessage,			"", "", "")
		LUA_EXPOSED_METHOD_RAW(Projectile_Reparent,					Lua_Projectile_Reparent,					"", "entity controller, vector collisionPoint", "");
		LUA_EXPOSED_METHOD(Projectile_EnableEntityTracking,			Lua_Projectile_EnableEntityTracking,		"", "entity controller", "entity to track");
		//LUA_EXPOSED_METHOD(Projectile_EnableSteering,				Lua_Projectile_EnableMonoSteering,			"", "entity controller", "the controlling entity");
		//LUA_EXPOSED_METHOD(Projectile_EnableDualSteering,			Lua_Projectile_EnableDualSteering,			"", "entity controller", "the controlling entity");
		LUA_EXPOSED_METHOD(Projectile_DisableSteering,				Lua_Projectile_DisableSteering,				"", "", "")
		LUA_EXPOSED_METHOD(Projectile_SetSplineRadius,				Lua_Projectile_SetSplineRadius,				"", "number radius", "");
		LUA_EXPOSED_METHOD(Projectile_IsMoving,						Lua_Projectile_IsMoving,					"", "", "")
		LUA_EXPOSED_METHOD(Projectile_GetStateTime,					Lua_Projectile_GetStateTime,				"", "", "")
		LUA_EXPOSED_METHOD(Character_SetCollidable,					Lua_Character_SetCollidable,				"", "bool collidable", "")
		LUA_EXPOSED_METHOD(Character_SetSoftParent,					Lua_Character_SetSoftParent,				"", "entity parent", "")
		LUA_EXPOSED_METHOD(SendMessageOnCollision,					Lua_SendMessageOnCollision,					"", "bool (send msg)", "")
		LUA_EXPOSED_METHOD(SetCollisionCallback,					Lua_SetCollisionCallback,					"", "string selfMessage, string receiverMessage", "")
		LUA_EXPOSED_METHOD(ParamAttack,								Lua_ParamAttack,							"", "table parameters", "")
		LUA_EXPOSED_METHOD(SetCharacterControllerDoMovementAbsolutely,	Lua_SetCharacterControllerDoMovementAbsolutely,							"", "bool DoMovementAbsolutely", "")
		LUA_EXPOSED_METHOD(SetCollisionFilterInfo,					SetCollisionFilterInfo,							"", "", "")
		LUA_EXPOSED_METHOD(SetCollisionFilterInfoEx,				SetCollisionFilterInfoEx,							"", "", "") //Just for testing. Will be removed
		LUA_EXPOSED_METHOD(GetCollisionFilterInfo,					GetCollisionFilterInfo,							"", "", "")

		
	LUA_EXPOSED_END(System)
};

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetHoldingCapsule
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetHoldingCapsule(bool bSize)
{
	Physics::AdvancedCharacterController* pobCharacterState=(Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if(pobCharacterState)
		pobCharacterState->SetCharacterControllerHoldingCapsule(bSize);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_ActivateState
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_ActivateState(const char* pcName)
{
	Physics::System* check1 = ((Physics::System*)((Physics::System*)((void*)this)));
	UNUSED(check1);

	CHECK_STR(pcName); // ALEXEY_TODO!!!

	if(strcmp("Ragdoll", pcName) == 0)
	{
#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::AdvancedCharacterController* lg = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		if(lg)
		{
			lg->DeactivateCharacterController();		
			lg->SetRagdollDead();
		}
#endif
	}
		
	if(strcmp("CharacterState", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		if(lg)
			lg->Activate();
	}

	if(strcmp("Rigid", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
		if(lg)
			lg->Activate();
	}

	if(strcmp("Soft", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::SOFT_BODY_LG);
		if(lg)
			lg->Activate();
	}

	if(strcmp("ProjectileState", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		if(lg)
			lg->Activate();
	}

	if(strcmp("Spear", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::SPEAR_LG);
		if(lg)
			lg->Activate();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_DeactivateState
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_DeactivateState(const char* pcName)
{
	CHECK_STR(pcName);

	// "Ragdoll" | "CharacterState" | "Rigid" | "Soft" | "ProjectileState" | "Spear"
	if(strcmp("Ragdoll", pcName) == 0)
	{
		/*Physics::LogicGroup* lg = pobEnt->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::RAGDOLL_LG);
		if(lg)
			lg->Deactivate();*/
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::AdvancedCharacterController* lg = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		if(lg)
		{
			lg->ActivateCharacterController();
		}
#endif
	}
	
	if(strcmp("CharacterState", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		if(lg)
			lg->Deactivate();
	}

	if(strcmp("Rigid", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
		if(lg)
			lg->Deactivate();
	}

	if(strcmp("Soft", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::SOFT_BODY_LG);
		if(lg)
			lg->Deactivate();
	}

	if(strcmp("ProjectileState", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		if(lg)
			lg->Deactivate();
	}

	if(strcmp("Spear", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::SPEAR_LG);
		if(lg)
			lg->Deactivate();
	}

	if(strcmp("Animated", pcName) == 0)
	{
		Physics::LogicGroup* lg = GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
		if(lg)
			lg->Deactivate();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_ActivateParticleOnContact
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_ActivateParticleOnContact()
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	ntAssert(lg);
	lg->ActivateParticleOnContact();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetLookAtEntity
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetLookAtEntity(CEntity* pobToWatch)
{
	/*Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

	if(pobCC)
		pobCC->LookAt(pobToWatch);*/
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_ResetLookAt
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_ResetLookAt()
{
	/*Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

	if(pobCC)
		pobCC->ResetLookAt();*/
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetLinearVelocity
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetLinearVelocity(NinjaLua::LuaObject obTable)
{
	SetLinearVelocity(CLuaHelper::DirectionFromTable(obTable));
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetLinearVelocity
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_AltSetLinearVelocity(CDirection &vDirection)
{
	SetLinearVelocity(vDirection);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetAngularVelocity
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetAngularVelocity(NinjaLua::LuaObject obTable)
{
	ntAssert(GetEntity()->GetHierarchy());

	CDirection obNewVel = CLuaHelper::DirectionFromTable(obTable) * GetEntity()->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
	SetAngularVelocity(obNewVel);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_AltSetAngularVelocity
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_AltSetAngularVelocity(const CDirection& NewVelocity)
{
	ntAssert(GetEntity()->GetHierarchy());

	CDirection obNewVel = NewVelocity * GetEntity()->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
	SetAngularVelocity(obNewVel);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetLinearVelocityFromTarget
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetLinearVelocityFromTarget(CEntity* pobControllingEntity, NinjaLua::LuaObject obInputVelocity)
{
	CEntity* pobSelf = GetEntity();

	ntAssert(pobSelf);
	ntAssert(pobSelf->GetHierarchy());
	ntAssert(pobControllingEntity);
	ntAssert(pobControllingEntity->GetHierarchy());
	ntAssert(pobControllingEntity->GetAwarenessComponent());

	CDirection obVelocity = CLuaHelper::DirectionFromTable(obInputVelocity);

	// Get a target based on the direction the character is facing
	CEntity* pobTarget=pobControllingEntity->GetAwarenessComponent()->GetTarget(AT_TYPE_THROW, pobControllingEntity, true);
	CDirection obResultantVelocity;

	if(pobTarget) // We have a target
	{
		CMatrix obLookAtTarget;

		CPoint obFrom(pobSelf->GetPosition());
		obFrom.Y()=0.0f;

		CPoint obTo(pobTarget->GetPosition());
		obTo.Y()=0.0f;

		CCamUtil::CreateFromPoints(obLookAtTarget,obFrom,obTo);

		obResultantVelocity=obVelocity * obLookAtTarget;
	}
	else // No target, use the direction the entity is facing in
	{
		obResultantVelocity=obVelocity * pobControllingEntity->GetMatrix();
	}

	// Set a new velocity for this rigid body
	SetLinearVelocity(obResultantVelocity);

	//	ntPrintf("setlinearvelocity -> %f,%f,%f -> %s\n",
	//		obResultantVelocity.X(),obResultantVelocity.Y(),obResultantVelocity.Z(),pobSelf->GetName().c_str());
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_AltSetLinearVelocityFromTarget
//! Alternative version
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_AltSetLinearVelocityFromTarget(CEntity* pobControllingEntity, const CDirection& InputVelocity)
{
	CEntity* pobSelf = GetEntity();

	ntAssert(pobSelf);
	ntAssert(pobSelf->GetHierarchy());
	ntAssert(pobControllingEntity);
	ntAssert(pobControllingEntity->GetHierarchy());
	ntAssert(pobControllingEntity->GetAwarenessComponent());

	// Get a target based on the direction the character is facing
	CEntity* pobTarget = pobControllingEntity->GetAwarenessComponent()->GetTarget(AT_TYPE_THROW, pobControllingEntity, true);
	CDirection obResultantVelocity;

	if(pobTarget) // We have a target
	{
		CMatrix obLookAtTarget;

		CPoint obFrom(pobSelf->GetPosition());
		obFrom.Y()=0.0f;

		CPoint obTo(pobTarget->GetPosition());
		obTo.Y()=0.0f;

		CCamUtil::CreateFromPoints(obLookAtTarget,obFrom,obTo);

		obResultantVelocity = InputVelocity * obLookAtTarget;
	}
	else // No target, use the direction the entity is facing in
	{
		obResultantVelocity = InputVelocity * pobControllingEntity->GetMatrix();
	}

	// Set a new velocity for this rigid body
	SetLinearVelocity(obResultantVelocity);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetLinearVelocityFromCamera
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetLinearVelocityFromCamera(NinjaLua::LuaObject obInputVelocity)
{
	CEntity* pobSelf = GetEntity();

	ntAssert(pobSelf);
	ntAssert(pobSelf->GetHierarchy());

	CDirection obVelocity = CLuaHelper::DirectionFromTable(obInputVelocity);

	const CMatrix& obCameraMatrix=CamMan::GetPrimaryView()->GetCurrMatrix();

	CDirection obDelta(pobSelf->GetPosition() - obCameraMatrix.GetTranslation());
	float fDistanceBetweenObjectAndCamera=obDelta.Length();

	CDirection obOffset1=CDirection(0.0f,0.0f,fDistanceBetweenObjectAndCamera) * obCameraMatrix; // Prevent the raycast from hitting stuff thats behind the player, such as a wall
	CDirection obOffset2=CDirection(0.0f,0.0f,50.0f) * obCameraMatrix;

	CPoint obStart(obCameraMatrix.GetTranslation() + obOffset1);
	CPoint obEnd(obCameraMatrix.GetTranslation() + obOffset2);

	Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
	obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
	obFlag.flags.i_collide_with = (	//Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

	Physics::TRACE_LINE_QUERY stQuery;

	if(Physics::CPhysicsWorld::Get().TraceLine(obStart,obEnd,pobSelf,stQuery,obFlag))
	{
		CDirection obNewVelocity(stQuery.obIntersect - pobSelf->GetPosition());

		obNewVelocity.Normalise();

		obNewVelocity*=obVelocity.Z();

		SetLinearVelocity(obNewVelocity);

		//ntPrintf("setlinearvelocity -> %f,%f,%f -> %s\n",
		//	obNewVelocity.X(),obNewVelocity.Y(),obNewVelocity.Z(),pobSelf->GetName().c_str());

		//if(stQuery.pobEntity)
		//	ntPrintf("LOS check -> %s\n",stQuery.pobEntity->GetName().c_str());
		
	}
	else
	{
		CDirection obNewVelocity(obEnd - pobSelf->GetPosition());

		obNewVelocity.Normalise();

		obNewVelocity*=obVelocity.Z();

		SetLinearVelocity(obNewVelocity);

		//ntPrintf("setlinearvelocity -> %f,%f,%f -> %s\n",
		//	obNewVelocity.X(),obNewVelocity.Y(),obNewVelocity.Z(),pobSelf->GetName().c_str());
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_AltSetLinearVelocityFromCamera
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_AltSetLinearVelocityFromCamera(const CDirection& InputVelocity)
{
	CEntity* pobSelf = GetEntity();

	ntAssert(pobSelf);
	ntAssert(pobSelf->GetHierarchy());

	const CMatrix& obCameraMatrix=CamMan::GetPrimaryView()->GetCurrMatrix();

	CDirection obDelta(pobSelf->GetPosition() - obCameraMatrix.GetTranslation());
	float fDistanceBetweenObjectAndCamera=obDelta.Length();

	CDirection obOffset1=CDirection(0.0f,0.0f,fDistanceBetweenObjectAndCamera) * obCameraMatrix; // Prevent the raycast from hitting stuff thats behind the player, such as a wall
	CDirection obOffset2=CDirection(0.0f,0.0f,50.0f) * obCameraMatrix;

	CPoint obStart(obCameraMatrix.GetTranslation() + obOffset1);
	CPoint obEnd(obCameraMatrix.GetTranslation() + obOffset2);

	Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
	obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
	obFlag.flags.i_collide_with = (	//Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

	Physics::TRACE_LINE_QUERY stQuery;

	if(Physics::CPhysicsWorld::Get().TraceLine(obStart,obEnd,pobSelf,stQuery,obFlag))
	{
		CDirection obNewVelocity(stQuery.obIntersect - pobSelf->GetPosition());

		obNewVelocity.Normalise();

		obNewVelocity *= InputVelocity.Z();

		SetLinearVelocity(obNewVelocity);
	}
	else
	{
		CDirection obNewVelocity(obEnd - pobSelf->GetPosition());

		obNewVelocity.Normalise();

		obNewVelocity *= InputVelocity.Z();

		SetLinearVelocity(obNewVelocity);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_ApplyLinearImpulse
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_ApplyLinearImpulse(NinjaLua::LuaObject obVelocity)
{
	ApplyLinearImpulse(CLuaHelper::DirectionFromTable(obVelocity));
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_GetLinearVelocity
//! 
//!
//------------------------------------------------------------------------------------------
NinjaLua::LuaObject Physics::System::Lua_GetLinearVelocity()
{
	if(this)
	{
		const CDirection& obLinearVelocity = GetLinearVelocity();

		NinjaLua::LuaObject obTable = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());
		obTable.Set("x",obLinearVelocity.X());
		obTable.Set("y",obLinearVelocity.Y());
		obTable.Set("z",obLinearVelocity.Z());
		return obTable;
	}
	else
	{
		ntPrintf("Physics::System::Lua_GetLinearVelocity() - Binding called on entity with no Dynamics Component\n");
		NinjaLua::LuaObject obTable = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());
		obTable.Set("x",0.f);
		obTable.Set("y",0.f);
		obTable.Set("z",0.f);
		return obTable;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Animated_Collapse
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Animated_Collapse()
{
	Physics::AnimatedLG* lg = (Physics::AnimatedLG*)GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
	if(lg && lg->IsActive())
		lg->MakeDynamic();
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_AttachedFromLeftFoot()
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	// Use the movement controller to animate the entity
	/*Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if(pobCharacter)
	{
		pobCharacter->GetAdvancedRagdoll()->AddConstraintOnLeftFoot();
	}*/
#endif
}

void Physics::System::Lua_Ragdoll_SetExemptFromCleanup(bool bExemption)
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if(pobCharacter)
		pobCharacter->SetRagdollExemptFromCleanup(bExemption);
}

void Physics::System::Lua_CharacterController_SetRagdollCollidable(bool bCollide)
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if(pobCharacter)
		pobCharacter->SetCharacterControllerRagdollCollidable(bCollide);
}

void Physics::System::Lua_Ragdoll_SetBeingHeld(bool bHeld)
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if(pobCharacter && pobCharacter->GetAdvancedRagdoll())
	{
		pobCharacter->GetAdvancedRagdoll()->SetRagdollHeld( bHeld );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Ragdoll_Freeze
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Ragdoll_Freeze()
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if(pobCharacter && pobCharacter->GetAdvancedRagdoll())
		pobCharacter->GetAdvancedRagdoll()->ForceFreeze();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Ragdoll_SetMotionType
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Ragdoll_SetMotionType(int iMotionType)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	enum
	{
		RAGDOLL_DYNAMIC		= 0,
		RAGDOLL_ANIMATED_PHYSICALLY	= 1,
		RAGDOLL_ANIMATED_ABSOLUTELY = 2,
		RAGDOLL_TRANSFORM_TRACKING_PHYSICALLY = 3,
		RAGDOLL_TRANSFORM_TRACKING_ANIMATED = 4
	};

	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if(pobCharacter)
	{
		switch(iMotionType)
		{
			case RAGDOLL_DYNAMIC:
			{
				GetCollisionStrikeHandler()->Disable();
				pobCharacter->SetRagdollDead();
				break;
			}

			case RAGDOLL_ANIMATED_PHYSICALLY:
			{
				GetCollisionStrikeHandler()->Disable();
				pobCharacter->SetRagdollAnimated(Physics::AdvancedRagdoll::ALL_BONES);
				break;
			}

			case RAGDOLL_ANIMATED_ABSOLUTELY:
			{
				GetCollisionStrikeHandler()->Disable();
				pobCharacter->SetRagdollAnimatedAbsolute(Physics::AdvancedRagdoll::ALL_BONES);
				break;
			}

			case RAGDOLL_TRANSFORM_TRACKING_PHYSICALLY:
			{
				GetCollisionStrikeHandler()->Disable();
				pobCharacter->SetRagdollTransformTracking();
				break;
			}

			case RAGDOLL_TRANSFORM_TRACKING_ANIMATED:
			{
				GetCollisionStrikeHandler()->Disable();
				pobCharacter->SetRagdollTransformTrackingAnimated();
				break;
			}
		}
	}
#else
	UNUSED(iMotionType);
#endif // _PS3_RUN_WITHOUT_HAVOK_BUILD
}

void Physics::System::Lua_Ragdoll_SetBoneTransformTrackingMapping(int iBoneFlag, CEntity* pobEntity, const char* pcTransformName)
{
	// Try to find the requested transform on the new parent heirarchy
	int iIdx = pobEntity->GetHierarchy()->GetTransformIndex( CHashedString( pcTransformName ) );

	// Make sure that we found it
	if ( iIdx == -1 )
	{
		user_warn_p( 0, ( "Can't find transform '%s' on '%s' to set ragdoll to.\n", pcTransformName, pobEntity->GetName().c_str() ) );
	}
	else
	{
		// We can now get the transforms that we require
		Transform* pobParentTransform = pobEntity->GetHierarchy()->GetTransform( iIdx );

		Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		ntAssert(pobCharacter);
		pobCharacter->SetRagdollBoneTransformTrackingMapping(iBoneFlag, pobParentTransform);
	}
}

void Physics::System::Lua_Ragdoll_SetAnimatedBones(int iBoneFlags)
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	ntAssert(pobCharacter);
	pobCharacter->SetRagdollAnimatedBones(iBoneFlags);
}

void Physics::System::Lua_Ragdoll_AddAnimatedBone(int iBoneFlag)
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	ntAssert(pobCharacter);
	pobCharacter->AddRagdollAnimatedBone(iBoneFlag);
}

void Physics::System::Lua_Ragdoll_SetTurnDynamicOnContact(bool bTurn)
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	ntAssert(pobCharacter);
	pobCharacter->SetRagdollTurnDynamicOnContact(bTurn);
}

void Physics::System::Lua_Ragdoll_Twitch()
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	ntAssert(pobCharacter);
	pobCharacter->RagdollTwitch();
}

void Physics::System::Lua_Ragdoll_ApplyLinearImpulse(NinjaLua::LuaObject obImp)
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	ntAssert(pobCharacter);
	pobCharacter->RagdollApplyLinearImpulse(CLuaHelper::DirectionFromTable(obImp));
}

void Physics::System::Lua_Ragdoll_ApplyAngularImpulse(NinjaLua::LuaObject obImp)
{
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	ntAssert(pobCharacter);
	pobCharacter->RagdollApplyAngularImpulse(CLuaHelper::DirectionFromTable(obImp));
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Ragdoll_Reparent
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Ragdoll_Reparent(CEntity* pobParentEntity)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	CEntity* pobRagdollEntity = GetEntity();
	ntAssert(pobRagdollEntity);
	ntAssert(pobParentEntity);
	
	pobRagdollEntity->GetAnimator()->ClearAnimWeights();
	pobRagdollEntity->GetAnimator()->Disable();
	pobRagdollEntity->GetMovement()->SetEnabled(false);
	Deactivate();

	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	ntAssert(pobCharacter);
	pobCharacter->SetRagdollAnimated(Physics::AdvancedRagdoll::ALL_BONES);

	CMatrix obLocal(CONSTRUCT_IDENTITY);
	obLocal.SetTranslation(CPoint(0.0f,-1.2f,-0.6f));

	Transform* pobHeldTransform = pobRagdollEntity->GetHierarchy()->GetRootTransform();
	pobHeldTransform->SetLocalMatrix(obLocal);
	pobHeldTransform->RemoveFromParent();
	pobParentEntity->GetHierarchy()->GetRootTransform()->AddChild(pobHeldTransform);

	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*) pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	ntAssert(lg);

	//lg->SetRotation(lg->GetRotation()); ?? huh ??
	lg->SetLinearVelocity(CVecMath::GetZeroDirection());
	lg->SetAngularVelocity(CVecMath::GetZeroDirection());
#else
	UNUSED(pobRagdollEntity); UNUSED(pobParentEntity);
#endif

}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Ragdoll_CheckAtRest
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Ragdoll_CheckAtRest()
{
	//ntError_p(0,("If you're reading this, Duncan has forgotten to fix something he took out of ragdolls. Go hit him.\n"));
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if(pobCharacter)
		pobCharacter->SetSendRagdollMessageOnAtRest("msg_atrest");
}



//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Ragdoll_SetBeingThrown
//! (Added by SCEE_SWRIGHT)
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Ragdoll_SetBeingThrown( float fDistanceMultiplier, float fAngleMultiplier, const char* pcTargetType )
{
	// Change this entity's collision representation to only collide with static geometry
	// (not the guy holding/throwing him). And set the ragdoll to be animated physically,
	// so that it will inherit velocities when transitioning to full dynamic mode.
	ntAssert(GetEntity());
	GetEntity()->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
								Physics::System::RAGDOLL_ANIMATED_ABSOLUTELY, // RAGDOLL_ANIMATED_PHYSICALLY,
								Physics::System::CHARACTER_CONTROLLER_COLLIDE_WITH_STATIC_ONLY );

	// Specify who is being targetted by the ragdoll (player or enemy)
	int iTarget = ( pcTargetType && strcmp( pcTargetType, "player" ) == 0 )
		? Physics::CollisionStrikeHandler::PLAYER | Physics::CollisionStrikeHandler::FRIENDLY 
		: Physics::CollisionStrikeHandler::ENEMY  | Physics::CollisionStrikeHandler::FRIENDLY;
	GetCollisionStrikeHandler()->SetStrikeFilter(iTarget); 
	
	// Set this ragdoll's throw trajectory parameters.
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	ntAssert( pobCharacter );
	pobCharacter->SetRagdollTrajectory( fDistanceMultiplier, fAngleMultiplier );
}




//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Ragdoll_SetMinTumbleDistance
//! (Added by SCEE_SWRIGHT)
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Ragdoll_SetMinTumbleDistance( float fDist )
{
}




//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_CheckAtRest
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_CheckAtRest()
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	
	if(lg)
		lg->AddCheckAtRestBehavior();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_ResetCollisionFilters
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_ResetCollisionFilters()
{
	Physics::StaticLG* lg = (Physics::StaticLG*)GetFirstGroupByType(Physics::LogicGroup::STATIC_LG);
	ntAssert(lg);

	Physics::RigidBody* rb = (Physics::RigidBody*)(*lg->GetElementList().begin());

	if(rb)
	{
		if(rb->GetHkRigidBody()->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
		{
			rb->GetHkRigidBody()->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
		};

		Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
		exceptionFlag.flags.exception_set |= Physics::ALWAYS_RETURN_FALSE_BIT;
		hkPropertyValue val2((int)exceptionFlag.base);
		rb->GetHkRigidBody()->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

		if(rb->GetHkRigidBody()->getWorld())
			rb->GetHkRigidBody()->getWorld()->updateCollisionFilterOnEntity(rb->GetHkRigidBody(), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_CollideWithPlayerOnly
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_CollideWithPlayerOnly()
{
	Physics::StaticLG* lg = (Physics::StaticLG*)GetFirstGroupByType(Physics::LogicGroup::STATIC_LG);
	ntAssert(lg);

	Physics::RigidBody* rb = (Physics::RigidBody*)(*lg->GetElementList().begin());

	if(rb)
	{
		if(rb->GetHkRigidBody()->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
		{
			rb->GetHkRigidBody()->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
		};

		Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
		exceptionFlag.flags.exception_set |= Physics::COLLIDE_WITH_PLAYER_ONLY;
		hkPropertyValue val2((int)exceptionFlag.base);
		rb->GetHkRigidBody()->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

		if(rb->GetHkRigidBody()->getWorld())
			rb->GetHkRigidBody()->getWorld()->updateCollisionFilterOnEntity(rb->GetHkRigidBody(), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_CollideWithEnemiesOnly
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_CollideWithEnemiesOnly()
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	ntAssert(lg);

	if(lg->GetRigidBody()->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
	{
		lg->GetRigidBody()->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
	};

	Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
	exceptionFlag.flags.exception_set |= Physics::COLLIDE_WITH_NMEs_ONLY;
	hkPropertyValue val2((int)exceptionFlag.base);
	lg->GetRigidBody()->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

	if(lg->GetRigidBody()->getWorld())
		lg->GetRigidBody()->getWorld()->updateCollisionFilterOnEntity(lg->GetRigidBody(), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_CollideWithPlayerAndEnemiesOnly
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_CollideWithPlayerAndEnemiesOnly()
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	ntAssert(lg);

	if(lg->GetRigidBody()->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
	{
		lg->GetRigidBody()->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
	};

	Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
	exceptionFlag.flags.exception_set |= Physics::COLLIDE_WITH_CCs_ONLY;
	hkPropertyValue val2((int)exceptionFlag.base);
	lg->GetRigidBody()->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

	if(lg->GetRigidBody()->getWorld())
		lg->GetRigidBody()->getWorld()->updateCollisionFilterOnEntity(lg->GetRigidBody(), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_PushFromPlayer
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_PushFromPlayer()
{
	CEntity* pobToPush = GetEntity();

	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	ntAssert(pobPlayer);
	CDirection pushVector(pobToPush->GetPosition() - pobPlayer->GetPosition());
	pushVector.Normalise();

	// [MUS] - This is a hack !!!! {E3 HACKS BEWARE}
	if(pobToPush->GetPhysicsSystem()) 
	{
		LogicGroup* pLogicGroup = GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
		
		ntAssert( pLogicGroup && "This shouldn't be happening" );

		if(pLogicGroup)
		{
		pLogicGroup->Activate(true);
		}

		pushVector *= 30.f;
		//pobToPush->GetPhysicsSystem()->ApplyLinearImpulse(pushVector);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_CheckIfMoving
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_CheckIfMoving()
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	ntAssert(lg);

	lg->AddCheckMovingBehavior();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_SetKeyframedMotion
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_SetKeyframedMotion(bool bEnable)
{
	Physics::System* ps = reinterpret_cast<Physics::System*>(this); 
    UNUSED(ps);
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	
	if(lg)
	{
		if(bEnable)
			lg->SetMotionType(Physics::HS_MOTION_KEYFRAMED);
		else
			lg->SetMotionType(Physics::HS_MOTION_DYNAMIC);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_SetBreakableKeyframedMotion
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_SetBreakableKeyframedMotion(bool on)
{		
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);

	if(lg)
	{
		if(on)
			lg->SetMotionType(Physics::HS_MOTION_BREAKABLE_KEYFRAMED);			
		
		else
			lg->SetMotionType(Physics::HS_MOTION_DYNAMIC);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_IsInBreakableKeyframedMotion
//! 
//!
//------------------------------------------------------------------------------------------
bool Physics::System::Lua_Rigid_IsInBreakableKeyframedMotion() const
{		
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);		
	if (lg)
		return lg->GetMotionType() == Physics::HS_MOTION_BREAKABLE_KEYFRAMED;				
	else
		return false;
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_SetMotionType
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_SetMotionType(int iMotionType)
{
	Physics::System* ps = reinterpret_cast<Physics::System*>(this); 
    UNUSED(ps);
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	
	if(lg)
	{
		lg->SetMotionType((Physics::EMotionType)iMotionType);
	}
}



//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_MoveRelativeToEntitySafe
//! NinjaLua binding. Sets local translation relative to an entity, but only if the body will not collide on 
//! its path to new position... 
//!
//------------------------------------------------------------------------------------------

bool Physics::System::Lua_Rigid_MoveRelativeToEntitySafe (CEntity* pobTarget,float fX,float fY,float fZ)
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	if(lg)
	{		
		CDirection obInput(fX,fY,fZ);
		CDirection obOffset=obInput * pobTarget->GetMatrix();
		CPoint obNewTranslation(pobTarget->GetMatrix().GetTranslation() + obOffset);

		return lg->MoveToSafe(obNewTranslation);
		}
		return false;
}

//------------------------------------------------------------------------------------------
//!
//!	hysics::System::Lua_Rigid_SetRotationRelativeToEntitySafe
//! NinjaLua binding. Sets local rotation telative to an entity, but only if the body with new rotation
//! will not penetrate other bodies. 
//!
//------------------------------------------------------------------------------------------
bool Physics::System::Lua_Rigid_SetRotationRelativeToEntitySafe (CEntity* pobTarget,float fR1,float fR2,float fR3,float fR4)
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	
	if(lg)
	{
		Transform* pobRootTransform=m_pobParentEntity->GetHierarchy()->GetRootTransform();
		CMatrix backUpPosition = pobRootTransform->GetLocalMatrix();

		CMatrix obRotation( CQuat(fR1,fR2,fR3,fR4) );
		CMatrix obNewLocalMatrix=obRotation * pobTarget->GetMatrix();
		obNewLocalMatrix.SetTranslation(pobRootTransform->GetLocalTranslation());

		return lg->SetRotationSafe(CQuat(obNewLocalMatrix));			
	}
	else 
		return false;
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_AntiGravity
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_AntiGravity(float fMinVelocity,float fDuration)
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	ntAssert(lg);

	lg->AddAntiGravityBehavior(fMinVelocity, fDuration);
}




//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CompoundRigid_CheckAtRest
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_CompoundRigid_CheckAtRest()
{
	Physics::CompoundLG* lg = (Physics::CompoundLG*)GetFirstGroupByType(Physics::LogicGroup::COMPOUND_RIGID_LG);
	
	if(lg)
		lg->AddCheckAtRestBehavior();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CompoundRigid_SetMotionType
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_CompoundRigid_SetMotionType(int iMotionType)
{
	Physics::System* ps = reinterpret_cast<Physics::System*>(this); 
    UNUSED(ps);
	Physics::CompoundLG* lg = (Physics::CompoundLG*)GetFirstGroupByType(Physics::LogicGroup::COMPOUND_RIGID_LG);
	
	if(lg)
	{
		lg->SetMotionType((Physics::EMotionType)iMotionType);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CompoundRigid_SetKeyframedMotion
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_CompoundRigid_SetKeyframedMotion(bool bEnable)
{
	if(bEnable)
		Lua_CompoundRigid_SetMotionType(Physics::HS_MOTION_KEYFRAMED);
	else
		Lua_CompoundRigid_SetMotionType(Physics::HS_MOTION_DYNAMIC);
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CompoundRigid_SetBreakableKeyframedMotion
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_CompoundRigid_SetBreakableKeyframedMotion(bool on)
{		
	if(on)
		Lua_CompoundRigid_SetMotionType(Physics::HS_MOTION_BREAKABLE_KEYFRAMED);				
	else
		Lua_CompoundRigid_SetMotionType(Physics::HS_MOTION_DYNAMIC);
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CompoundRigid_IsInBreakableKeyframedMotion
//! 
//!
//------------------------------------------------------------------------------------------
bool Physics::System::Lua_CompoundRigid_IsInBreakableKeyframedMotion() const
{		
	Physics::CompoundLG* lg = (Physics::CompoundLG*)GetFirstGroupByType(Physics::LogicGroup::COMPOUND_RIGID_LG);		
	if (lg)
		return lg->GetMotionType() == Physics::HS_MOTION_BREAKABLE_KEYFRAMED;				
	else
		return false;
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_MoveRelativeToEntitySafe
//! NinjaLua binding. Sets local translation relative to an entity, but only if the body will not collide on 
//! its path to new position... 
//!
//------------------------------------------------------------------------------------------

bool Physics::System::Lua_CompoundRigid_MoveRelativeToEntitySafe (CEntity* pobTarget,float fX,float fY,float fZ)
{
	Physics::CompoundLG* lg = (Physics::CompoundLG*)GetFirstGroupByType(Physics::LogicGroup::COMPOUND_RIGID_LG);
	if(lg)
	{		
		CDirection obInput(fX,fY,fZ);
		CDirection obOffset=obInput * pobTarget->GetMatrix();
		CPoint obNewTranslation(pobTarget->GetMatrix().GetTranslation() + obOffset);

		return lg->MoveToSafe(obNewTranslation);
		}
		return false;
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CompoundRigid_SetRotationRelativeToEntitySafe
//! NinjaLua binding. Sets local rotation relative to an entity, but only if the body with new rotation
//! will not penetrate other bodies. 
//!
//------------------------------------------------------------------------------------------
bool Physics::System::Lua_CompoundRigid_SetRotationRelativeToEntitySafe (CEntity* pobTarget,float fR1,float fR2,float fR3,float fR4)
{
	Physics::CompoundLG* lg = (Physics::CompoundLG*)GetFirstGroupByType(Physics::LogicGroup::COMPOUND_RIGID_LG);
	
	if(lg)
	{
		Transform* pobRootTransform=m_pobParentEntity->GetHierarchy()->GetRootTransform();
		CMatrix backUpPosition = pobRootTransform->GetLocalMatrix();

		CMatrix obRotation( CQuat(fR1,fR2,fR3,fR4) );
		CMatrix obNewLocalMatrix=obRotation * pobTarget->GetMatrix();
		obNewLocalMatrix.SetTranslation(pobRootTransform->GetLocalTranslation());

		return lg->SetRotationSafe(CQuat(obNewLocalMatrix));			
	}
	else 
		return false;
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CompoundRigid_Collapse
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_CompoundRigid_Collapse()
{
	Physics::CompoundLG* lg = (Physics::CompoundLG*)GetFirstGroupByType(Physics::LogicGroup::COMPOUND_RIGID_LG);
	ntAssert(lg);

	if(lg->IsCollapsed() == false)
		lg->Collapse();
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CompoundRigid_AntiGravity
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_CompoundRigid_AntiGravity(float fMinVelocity,float fDuration)
{
	Physics::CompoundLG* lg = (Physics::CompoundLG*)GetFirstGroupByType(Physics::LogicGroup::COMPOUND_RIGID_LG);
	ntAssert(lg);

	lg->AddAntiGravityBehavior(fMinVelocity, fDuration);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Dynamics_RemoveChildEntities
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_RemoveChildEntities()
{
	CEntity* pobTarget = GetEntity();
	ntAssert(pobTarget);
	Physics::ProjectileManager::Get().RemoveChildProjectiles(pobTarget);

#if defined( PLATFORM_PS3 )
	// TGS HACK for spawnees who don't drop their weapons
	if(pobTarget->IsAI() && pobTarget->ToAI()->GetArmyRenderable())
	{
		return;
	}
#endif

	//if(!pobTarget->IsEnemy()) // Commented out because we need this to work on enemy characters
	{
		// Secondly, send removefromworld message to any other entities which are parented and reparent to world + hide them
		CEntityQuery obQuery;
		EQCIsChildEntity obClause(pobTarget);
		obQuery.AddClause(obClause);
		
		CEntityManager::Get().FindEntitiesByType(obQuery, CEntity::EntType_AllButStatic);

		for(QueryResultsContainerType::iterator obIt=obQuery.GetResults().begin(); obIt!=obQuery.GetResults().end(); ++obIt)
		{
			CEntity* pEnt = (*obIt);
			ntError( pEnt && "Making sure that things are all fine down on the farm" );

			if( pEnt->GetMessageHandler() && pEnt->CanRemoveFromWorld() )
			{
				// Hide the entire renderable
				pEnt->GetRenderableComponent()->AddRemoveAll_Game(false);

				// Reparent this entity to the world
				CMatrix obWorldMatrix = pEnt->GetMatrix();
				pEnt->GetHierarchy()->GetRootTransform()->RemoveFromParent();
				pEnt->GetHierarchy()->GetRootTransform()->SetLocalMatrix(obWorldMatrix);	
				CHierarchy::GetWorld()->GetRootTransform()->AddChild( pEnt->GetHierarchy()->GetRootTransform() );

				// Unassign parent entity
				pEnt->SetParentEntity(0);

				// Remove the entity from the world
				bool bNoError = pEnt->RemoveFromWorld();
				ntError( bNoError && "Woops - the entity didn't remove itself from the world for some strange reason" ); UNUSED( bNoError );

				//ntPrintf("Removing %s from %s\n",(*obIt)->GetName().c_str(),GetEntity()->GetName().c_str());
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_OrientateToVelocity
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_OrientateToVelocity(bool bEnable)
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	ntAssert(lg);

	lg->SetPiercingBehavior(bEnable);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_EnableVelocityReflection
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_EnableVelocityReflection(bool bEnable)
{
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	ntAssert(lg);

	lg->SetDeflectionBehavior(bEnable);
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_DeflectionRender
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_DeflectionRender(bool bEnable)
{
#ifdef _DEBUG
	Physics::SingleRigidLG* lg = (Physics::SingleRigidLG*)GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);

	if (lg)
	{
		lg->SetDeflectionRenderer(bEnable);
	}
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_DoHurtOnCollision
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_DoHurtOnCollision(bool bEnable)
{
	if(bEnable)
		GetCollisionStrikeHandler()->Enable();
	else
		GetCollisionStrikeHandler()->Disable();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_CollisionStrike
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_CollisionStrike(bool bEnable)
{
	if(this)
	{
		if(bEnable)
			GetCollisionStrikeHandler()->Enable();
		else
			GetCollisionStrikeHandler()->Disable();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetController
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetController(CEntity* pEntController, CHashedString pcParamDef)
{
	CHECK_STR(pcParamDef);
	AftertouchControlParameters* pobDef = ObjectDatabase::Get().GetPointerFromName<AftertouchControlParameters*>(pcParamDef);//"GenericAftertouchControlParameters");

	Lua_SetControllerDef(pEntController, pobDef);
}

void Physics::System::Lua_SetControllerDef(CEntity* pEntController, AftertouchControlParameters* pobControlDef)
{
	CEntity* pobSelf = GetEntity();
	ntAssert(pobSelf);

	if(pEntController != 0)
	{
		if(!pobSelf->GetMovement()) // Create a movement system if one doesn't already exist
		{
			CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(pobSelf, pobSelf->GetAnimator(), this);
			pobSelf->SetMovement(pobMovement);
		}

#ifdef PLATFORM_PS3	// Only PS3 has the motion sensor pad
		if ( CPlayerOptions::Get().GetUseMotionControl() )
		{
			MotionSensorAfterTouchControllerDef obDef;

			obDef.m_pobControllingEntity = pEntController;

			obDef.m_pobParameters = pobControlDef;

			// Push the controller onto our movement component
			pobSelf->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);

			return;
		}
#endif

		AfterTouchControllerDef obDef;

		obDef.m_pobControllingEntity = pEntController;
		obDef.m_pobParameters = pobControlDef;//"GenericAftertouchControlParameters");

		// Push the controller onto our movement component
		pobSelf->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
		GetCollisionStrikeHandler()->Enable();
	}
	else
	{
		if(pobSelf->GetMovement())
			pobSelf->GetMovement()->ClearControllers();

		GetCollisionStrikeHandler()->Disable();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_EnableSteering
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_EnableSteering(CEntity* pEntController)
{
	CEntity* pobEnt = GetEntity();
	ntAssert(pobEnt);

	// is there a movement controller ?
	if(!pobEnt->GetMovement())
	{
		CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(pobEnt,
												pobEnt->GetAnimator(),
												pobEnt->GetPhysicsSystem());
		pobEnt->SetMovement(pobMovement);
	}

#ifdef PLATFORM_PS3	// Only PS3 has the motion sensor pad
	if ( CPlayerOptions::Get().GetUseMotionControl() )
	{
		MotionSensorAfterTouchControllerDef obMotionDef;

		obMotionDef.m_pobControllingEntity = pEntController;
		obMotionDef.m_pobParameters = ObjectDatabase::Get().GetPointerFromName<AftertouchControlParameters*>("GenericAftertouchControlParameters");

		// Push the controller onto our movement component
		pobEnt->GetMovement()->BringInNewController(obMotionDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
		GetCollisionStrikeHandler()->Enable();

		return;
	}
#endif

	AfterTouchControllerDef obDef;

	obDef.m_pobControllingEntity = pEntController;
	obDef.m_pobParameters = ObjectDatabase::Get().GetPointerFromName<AftertouchControlParameters*>("GenericAftertouchControlParameters");

	// Push the controller onto our movement component
	pobEnt->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
	GetCollisionStrikeHandler()->Enable();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Rigid_DisableSteering
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Rigid_DisableSteering()
{
	// Use the movement controller to animate the entity
	CEntity* pobEnt = GetEntity();
	ntAssert(pobEnt);

	if(pobEnt->GetMovement())
		pobEnt->GetMovement()->ClearControllers();

	GetCollisionStrikeHandler()->Disable();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Spear_SetMotionType
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Spear_SetMotionType(int iMotionType)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	Physics::SpearLG* lg = (Physics::SpearLG*) GetFirstGroupByType(Physics::LogicGroup::SPEAR_LG);

	if(lg)
		lg->SetMotionType((Physics::EMotionType)iMotionType);
#else
	UNUSED(iMotionType);
#endif 
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Spear_SetController
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Spear_SetController(CEntity* pEntController, const char* pcParamDef)
{
	CHECK_STR(pcParamDef);

	CEntity* pobSelf = GetEntity();
	ntAssert(pobSelf);

	Physics::SpearLG* lg = (Physics::SpearLG*)GetFirstGroupByType(Physics::LogicGroup::SPEAR_LG);

	if(lg)
	{
		if(pEntController)
		{
			if(!pobSelf->GetMovement()) // Create a movement system if one doesn't already exist
			{
				CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(pobSelf, pobSelf->GetAnimator(), pobSelf->GetPhysicsSystem());
				pobSelf->SetMovement(pobMovement);
			}

			SpearAfterTouchControllerDef obDef;

			obDef.m_pobControllingEntity = pEntController;
			obDef.m_pobParameters = ObjectDatabase::Get().GetPointerFromName<AftertouchControlParameters*>(pcParamDef);

			// Push the controller onto our movement component
			pobSelf->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
		}
		else
		{
			if(pobSelf->GetMovement())
				pobSelf->GetMovement()->ClearControllers();
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Spear_StartThrownBehaviour
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Spear_StartThrownBehaviour()
{
	Physics::SpearLG* lg = (Physics::SpearLG*)GetFirstGroupByType(Physics::LogicGroup::SPEAR_LG);

		if(lg)
			lg->AddThrownBehaviour();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Spear_StopThrownBehaviour
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Spear_StopThrownBehaviour()
{
	Physics::SpearLG* lg = (Physics::SpearLG*)GetFirstGroupByType(Physics::LogicGroup::SPEAR_LG);

	if(lg)
		lg->RemoveThrownBehaviour();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Spear_GenerateAtRestMessage()
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Spear_GenerateAtRestMessage()
{
	Physics::SpearLG* lg = (Physics::SpearLG*)GetFirstGroupByType(Physics::LogicGroup::SPEAR_LG);

	if(lg)
		lg->AddCheckAtRestBehavior();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Projectile_Reparent
//! 
//!
//------------------------------------------------------------------------------------------
int Physics::System::Lua_Projectile_Reparent(NinjaLua::LuaState& pobState)
{
	UNUSED( pobState );

	ntError( false && "This should no longer be used - please grab gav if you get this" );

	/*
	NinjaLua::LuaStack obArgs(pobState);

	ntAssert(obArgs[1].IsUserData());
	ntAssert(obArgs[2].IsTable());
	
	CEntity* pobTarget=obArgs[1].Get<CEntity*>(); // Pointer to the entity we want to stick 'self' to
	ntAssert(pobTarget);
	ntAssert(pobTarget->GetHierarchy());
	NinjaLua::LuaObject obV1(obArgs[2]); // Collision point
	CPoint obCollisionPoint = CLuaHelper::PointFromTable(obV1);

	CEntity* pobSelf = GetEntity();
    ntAssert(pobSelf);
	ntAssert(pobSelf->GetHierarchy());
	ntAssert(pobSelf->GetPhysicsSystem());

	Transform* pobSelfTransform = pobSelf->GetHierarchy()->GetRootTransform();
	Transform* pobTargetTransform = NULL;

	// Apply a force

	if(pobTarget->GetPhysicsSystem())
	{
		Physics::ProjectileLG* lg = (Physics::ProjectileLG*)GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		ntAssert(lg);

		CDirection obForce(lg->GetLinearVelocity());
		obForce*=lg->GetMass();

		lg->ApplyLocalisedLinearImpulse(obForce, CVector(obCollisionPoint));
	}

	if(pobTarget->GetAttackComponent()) // Parenting to a character
	{
		// Clear movement controllers and disable collision strike handler
		if(pobSelf->GetMovement())
			pobSelf->GetMovement()->ClearControllers();
		pobSelf->GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable();

		pobTargetTransform = pobTarget->GetHierarchy()->GetCharacterBoneTransform(CHARACTER_BONE_PELVIS);

		float fBestDist=FLT_MAX;

		// JML - Only consider real bones, there are transforms on ragdolls which don't correspond to visible bones.
		//       This can lead to very strange issues of weird swords under some (rare) circumstances.
		CHARACTER_BONE_ID eBestBone=CHARACTER_BONE_PELVIS;
		for(CHARACTER_BONE_ID eBone=CHARACTER_BONE_PELVIS; eBone<CHARACTER_BONE_R_KNEE; eBone = CHARACTER_BONE_ID(int(eBone)+1))
		{
			Transform* pTrans = pobTarget->GetHierarchy()->GetCharacterBoneTransform(eBone);
			if(!pTrans)
				continue;

			CDirection dDiff = pTrans->GetWorldTranslation() ^ obCollisionPoint;
			float fDist = dDiff.LengthSquared();
			if(fDist < fBestDist)
		{
				eBestBone = eBone;
				fBestDist = fDist;
				pobTargetTransform = pTrans;
		}
		}								

		if (!pobTargetTransform) // If for some bizarre reason we don't get a valid transform, revert back to using the root transform
		{
			pobTargetTransform=pobTarget->GetHierarchy()->GetRootTransform();
			eBestBone = CHARACTER_BONE_PELVIS;
		}

		// Calculate the new translation								
		CPoint obNewTranslation(0.0f,0.0f,0.0f);
		CMatrix obLocalMatrix(pobSelfTransform->GetLocalMatrix());
		obLocalMatrix*=pobTargetTransform->GetWorldMatrix().GetAffineInverse(); // Combine with the affine inverse of the parent
		pobSelfTransform->SetLocalMatrix(obLocalMatrix);

		pobSelfTransform->SetLocalTranslation(obNewTranslation);

		// Debug info
		if (g_ShellOptions->m_bDebugArcher)
		{
			switch (eBestBone)
			{
				case CHARACTER_BONE_ROOT: ntPrintf("Projectile parent to root\n"); break; 
				case CHARACTER_BONE_PELVIS: ntPrintf("Projectile parent to pelvis\n"); break; 
				case CHARACTER_BONE_SPINE_00: ntPrintf("Projectile parent to spine_00\n"); break; 
				case CHARACTER_BONE_SPINE_01: ntPrintf("Projectile parent to spine_01\n"); break; 
				case CHARACTER_BONE_SPINE_02: ntPrintf("Projectile parent to spine_02\n"); break; 
				case CHARACTER_BONE_NECK: ntPrintf("Projectile parent to neck\n"); break; 
				case CHARACTER_BONE_HEAD: ntPrintf("Projectile parent to head\n"); break; 
				case CHARACTER_BONE_HIPS: ntPrintf("Projectile parent to hips\n"); break; 
				case CHARACTER_BONE_L_SHOULDER: ntPrintf("Projectile parent to l_shoulder\n"); break; 
				case CHARACTER_BONE_L_ARM: ntPrintf("Projectile parent to l_arm\n"); break; 
				case CHARACTER_BONE_L_ELBOW: ntPrintf("Projectile parent to l_elbow\n"); break; 
				case CHARACTER_BONE_L_WRIST: ntPrintf("Projectile parent to l_wrist\n"); break; 
				case CHARACTER_BONE_L_WEAPON: ntPrintf("Projectile parent to l_weapon\n"); break; 
				case CHARACTER_BONE_L_LEG: ntPrintf("Projectile parent to l_leg\n"); break; 
				case CHARACTER_BONE_L_KNEE: ntPrintf("Projectile parent to l_knee\n"); break; 
				case CHARACTER_BONE_R_SHOULDER: ntPrintf("Projectile parent to r_shoulder\n"); break; 
				case CHARACTER_BONE_R_ARM: ntPrintf("Projectile parent to r_arm\n"); break; 
				case CHARACTER_BONE_R_ELBOW: ntPrintf("Projectile parent to r_elbow\n"); break; 
				case CHARACTER_BONE_R_WRIST: ntPrintf("Projectile parent to r_wrist\n"); break; 
				case CHARACTER_BONE_R_WEAPON: ntPrintf("Projectile parent to r_weapon\n"); break; 
				case CHARACTER_BONE_R_LEG: ntPrintf("Projectile parent to r_leg\n"); break; 
				case CHARACTER_BONE_R_KNEE: ntPrintf("Projectile parent to r_knee\n"); break; 
				default: break;
			}
		}


		if ( CHARACTER_BONE_HEAD == eBestBone || CHARACTER_BONE_NECK == eBestBone )
		{
			// Send a notification
			CMessageSender::SendEmptyMessage( CHashedString("msg_headshot"), pobTarget->GetMessageHandler() );
		}
	}
	else // Parenting to a world object
	{
		pobTargetTransform = pobTarget->GetHierarchy()->GetRootTransform();

		pobSelf->GetHierarchy()->GetRootTransform()->SetLocalTranslation(obCollisionPoint); // Set the position of the projectile to the collision point

		CMatrix obLocalMatrix=pobSelfTransform->GetLocalMatrix();
		obLocalMatrix*=pobTargetTransform->GetWorldMatrix().GetAffineInverse(); // Combine with the affine inverse of the parent
		pobSelfTransform->SetLocalMatrix(obLocalMatrix);
	}

	// Reparent this entity to the root of the target entity

	pobSelf->SetParentEntity(pobTarget);
	pobSelfTransform->RemoveFromParent();
	pobTargetTransform->AddChild(pobSelfTransform);
	*/
	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Projectile_EnableEntityTracking
//! Pre-milestone test case
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Projectile_EnableEntityTracking(CEntity* pEntityToTrack, float fXOffset , float fYOffset, float fZOffset, float fTurnWeighting)
{
	CEntity* pEntProjectile = GetEntity();
	ntAssert(pEntProjectile);
	ntAssert(pEntityToTrack);

	// is there a movement controller ?
	if(!pEntProjectile->GetMovement())
	{
		CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(pEntProjectile, pEntProjectile->GetAnimator(), this);
		pEntProjectile->SetMovement(pobMovement);
	}

	ProjectileTrackEntityControllerDef obDef;
	obDef.m_pobTrackedEntity = pEntityToTrack;
	obDef.m_obPosOffset.X() = fXOffset;
	obDef.m_obPosOffset.Y() = fYOffset;
	obDef.m_obPosOffset.Z() = fZOffset;
	obDef.m_fTurnWeighting = fTurnWeighting;

	// Push the controller onto our movement component
	pEntProjectile->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Projectile_EnableSteering
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Projectile_EnableSteering(CEntity* pEntController, bool bDualController, AftertouchControlParameters* pobControlDef )
{
	CEntity* pEntProjectile = GetEntity();
	ntAssert(pEntProjectile);
	ntAssert(pEntController);

	// is there a movement controller ?
	if(!pEntProjectile->GetMovement())
	{
		CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(pEntProjectile, pEntProjectile->GetAnimator(), this);
		pEntProjectile->SetMovement(pobMovement);
	}

#ifdef PLATFORM_PS3	// Only PS3 has the motion sensor pad
	if ( CPlayerOptions::Get().GetUseMotionControl() )
	{
		MotionSensorProjectileAfterTouchControllerDef obDef;

		obDef.m_pobControllingEntity	= pEntController;

		obDef.m_pobParameters = pobControlDef;

		// Allow input from either analogue input 
		obDef.m_bDualController			= bDualController;

		// Push the controller onto our movement component
		pEntProjectile->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, 0.0f);

		return;
	}
#endif

	// If we reach this far we either aren't on PS3 or are not using the motion sensor
	ProjectileAfterTouchControllerDef obDef;

	obDef.m_pobControllingEntity	= pEntController;

	obDef.m_pobParameters = pobControlDef;

	// Allow input from either analogue input 
	obDef.m_bDualController			= bDualController;
	
	// Push the controller onto our movement component
	pEntProjectile->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, 0.0f);
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Projectile_DisableSteering
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Projectile_DisableSteering()
{
	CEntity* pEntProjectile = GetEntity();
	ntAssert(pEntProjectile);

	ntAssert(pEntProjectile->GetMovement() );
	pEntProjectile->GetMovement()->ClearControllers();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Projectile_SetSplineRadius
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Projectile_SetSplineRadius(float fRadius)
{
	Physics::ProjectileLG* lg = (Physics::ProjectileLG*)GetFirstGroupByType( Physics::LogicGroup::PROJECTILE_LG );
	ntAssert( lg );

	lg->SetSplineRadius(fRadius); // Set the current radius for this spline
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Projectile_IsMoving
//! 
//!
//------------------------------------------------------------------------------------------
bool Physics::System::Lua_Projectile_IsMoving()
{
	Physics::ProjectileLG* lg = (Physics::ProjectileLG*)GetFirstGroupByType( Physics::LogicGroup::PROJECTILE_LG );
	ntAssert( lg );

	return lg->IsMoving();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Projectile_GetStateTime
//! 
//!
//------------------------------------------------------------------------------------------
float Physics::System::Lua_Projectile_GetStateTime()
{
	Physics::ProjectileLG* lg = (Physics::ProjectileLG*)GetFirstGroupByType( Physics::LogicGroup::PROJECTILE_LG );
	ntAssert( lg );

	return lg->GetStateTime();
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Character_SetCollidable
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_Character_SetCollidable(bool bCollidable)
{
	Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

	if (pobCC)
	{
		pobCC->SetCharacterControllerCollidable(bCollidable);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Character_SetSoftParent
//! 
//!
//------------------------------------------------------------------------------------------

void Physics::System::Lua_Character_SetSoftParent(CEntity* pSoftEnt)
{
	Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

	if (pobCC)
	{
		pobCC->SetSoftParent(pSoftEnt);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_Character_SetCollidable
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SendMessageOnCollision(bool bState)
{
	SetMsgOnCollision( bState );
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetCollisionCallback
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetCollisionCallback(const char* pcSelfMessage,const char* pcReceiverMessage)
{
	CHECK_STR(pcSelfMessage);
	CHECK_STR(pcReceiverMessage);

	// Callback always sends "msg_collision" so removed obsolite msg params
	GetCollisionCallbackHandler()->SetCollisionCallback(5.0f);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Physics::System::Lua_ParamAttack( obParamTable )
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
int Physics::System::Lua_ParamAttack ( NinjaLua::LuaObject obParamTable )
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	CEntity* pobSelf = m_pobParentEntity;

	ntAssert( pobSelf );

	CMatrix obWorldMatrix		= pobSelf->GetMatrix();
	CPoint  obWorldTranslation	= obWorldMatrix.GetTranslation();

	float	fFORWARD_OFFSET		= obParamTable.GetOpt( "ForwardOffset", 3.0f ); // 3.0f;
	float	fUP_OFFSET			= obParamTable.GetOpt( "UpOffset", 0.5f ); // 0.5f;
	float	fBOX_X				= obParamTable.GetOpt( "BoxX", 2.0f ); // 2.0f;
	float	fBOX_Y				= obParamTable.GetOpt( "BoxY", 0.5f ); // 0.5f;
	float	fBOX_Z				= obParamTable.GetOpt( "BoxZ", 3.0f ); // 3.0f;
	float	fMIN_VELOCITY		= obParamTable.GetOpt( "MinVel", 10.0f ); // 10.0f;
	float	fMAX_VELOCITY		= obParamTable.GetOpt( "MaxVel", 40.5f ); // 40.0f;
	float	fMIN_Y_VELOCITY		= obParamTable.GetOpt( "MinYVel", 5.0f ); // 5.0f;
	float	fMAX_Y_VELOCITY		= obParamTable.GetOpt( "MaxYVel", 10.0f ); // 10.0f;
	float	fDurarion			= obParamTable.GetOpt( "Duration", 5.0f ); // 5.0f;

	CDirection obForward(obWorldMatrix.GetZAxis());
	obForward		*= fFORWARD_OFFSET;
	
	CPoint obCentre(obWorldTranslation);
	obCentre		+= obForward;
	obCentre.Y()	+= fUP_OFFSET;

	///- -------------------------- DEBUG -------------------------------
#ifdef _DEBUG
//	CMatrix obDebugMatrix(obWorldMatrix);
//	obDebugMatrix.SetTranslation(obCentre);
//	g_VisualDebug->RenderOBB(obDebugMatrix,CDirection(fBOX_X,fBOX_Y,fBOX_Z),0x77ffffff);
#endif // _DEBUG
	///- -------------------------- DEBUG -------------------------------


	CQuat			obQuatRotation(obWorldMatrix);
	hkVector4		obPosition(obCentre.X(),obCentre.Y(),obCentre.Z());
	hkQuaternion	obRotation(Physics::MathsTools::CQuatTohkQuaternion(obQuatRotation));
	hkTransform		obLocalTransform(obRotation,obPosition);
	hkVector4		obHalfExtents(fBOX_X,fBOX_Y,fBOX_Z);
	hkBoxShape		obBoxShape(obHalfExtents);
	hkCollidable	obCollidable(&obBoxShape,&obLocalTransform);

	//Get all shapes which are penetrating the collidable Note: If you have call this function every step for a given object, use the hkShapePhantom version.
	hkAllCdBodyPairCollector obCollector;

	Physics::CPhysicsWorld::Get().GetPenetrations(&obCollidable, (hkCollisionInput &)*Physics::CPhysicsWorld::Get().GetCollisionInput(), obCollector);
	
	const hkArray<hkRootCdBodyPair>& obHits = obCollector.getHits();

	for(int iHit=0;iHit<obHits.getSize();iHit++)
	{
		const hkRootCdBodyPair* pobCurrentPair	= &obHits[iHit];
		const hkCollidable* pobCollided			= pobCurrentPair->m_rootCollidableB;

		if(pobCollided->getCollisionFilterInfo())
		{
			hkRigidBody* obRB = hkGetRigidBody(pobCollided);

			if(obRB)
			{
				float fVelocity		=	fMIN_VELOCITY + grandf(fMAX_VELOCITY-fMIN_VELOCITY);
				float fVelocityY	=	fMIN_Y_VELOCITY + grandf(fMAX_Y_VELOCITY-fMIN_Y_VELOCITY);

				hkVector4 obImpulseVelocity(	obWorldMatrix.GetZAxis().X() * fVelocity,
												(obWorldMatrix.GetZAxis().Y() * fVelocity) + fVelocityY,
												obWorldMatrix.GetZAxis().Z() * fVelocity );

				hkVector4 obCurrentVelocity(obRB->getLinearVelocity());
				hkVector4 obNewVelocity(obImpulseVelocity);

				obNewVelocity.sub4(obCurrentVelocity);
				obNewVelocity.mul4(obRB->getMass());
				obRB->applyLinearImpulse(obNewVelocity);

				CEntity* rbEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
				if( rbEntity )
				{
					// Hurt for n seconds
					rbEntity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Enable( 1, fDurarion );
				}
			}
		}
	}
#else
	UNUSED( pobState );
#endif
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Lua_SetCharacterControllerDoMovementAbsolutely( bool bDoMovementAbsolutely )
//! 
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_SetCharacterControllerDoMovementAbsolutely( bool bDoMovementAbsolutely )
{
	CEntity* pobThis = m_pobParentEntity;
	ntAssert( pobThis );

	Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) pobThis->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if (pobAdvCC)
	{
		pobAdvCC->SetDoCharacterControllerMovementAbsolutely( bDoMovementAbsolutely );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Physics::System::Dynamics_Explosion
//! Alternate, non-LUA, version.  LUA version is in dynamicsbindings.
//!
//------------------------------------------------------------------------------------------
void Physics::System::Lua_AltExplosion( const CExplosionParams& params )
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	CEntity* pobSelf = m_pobParentEntity;

	ntAssert( pobSelf );
	
	// [MUS] - As we are interested only by two type of entities (characters and bodies), we should use Havok instead of the Entity Query system
	// PS: Havok is an order of magnitude faster thant the current Query system !
	hkSphereShape sphereShape( params.m_fRadius );
	hkMotionState motionState;
	motionState.getTransform() = hkTransform(hkQuaternion::getIdentity(),Physics::MathsTools::CPointTohkVector( params.m_obPosition ));
	hkCollidable sphereCollidable(&sphereShape, &motionState);

	Physics::EntityCollisionFlag obFlag; obFlag.base = 0;
	obFlag.flags.i_am			=			Physics::LARGE_INTERACTABLE_BIT;
	obFlag.flags.i_collide_with = (			Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
											Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
											Physics::RAGDOLL_BIT						|
											Physics::SMALL_INTERACTABLE_BIT				|
											Physics::LARGE_INTERACTABLE_BIT				);

	sphereCollidable.setCollisionFilterInfo(obFlag.base);

	hkAllCdBodyPairCollector obCollector; 
	Physics::CPhysicsWorld::Get().GetPenetrations(&sphereCollidable, (hkCollisionInput&)*Physics::CPhysicsWorld::Get().GetCollisionInput(), obCollector);

	// See if we can find an attack move which provides the strike data for an explosion
	CAttackData* pobAttackData = ObjectDatabase::Get().GetPointerFromName< CAttackData* >( "atk_explosion" );
	
	for(int i=0; i < obCollector.getHits().getSize(); ++i)
	{
		CEntity* pobEntity = 0;

		hkRigidBody* obRB = hkGetRigidBody(obCollector.getHits()[i].m_rootCollidableA);
		if(0 == obRB)
			obRB = hkGetRigidBody(obCollector.getHits()[i].m_rootCollidableB);

		if((obRB) && (!obRB->isFixed()))
		{
			pobEntity = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();	
		} else {

			hkPhantom* obPH = hkGetPhantom(obCollector.getHits()[i].m_rootCollidableA);
			if(0 == obPH)
				obPH = hkGetPhantom(obCollector.getHits()[i].m_rootCollidableB);

			if(obPH) 
				pobEntity = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();				
		};
			

		if ( pobEntity && pobEntity != pobSelf )
		{
			{
				if (pobEntity->GetAttackComponent() && !pobEntity->ToCharacter()->IsDead())// Entity should receive a combat strike
				{
					CDirection obDirection(pobEntity->GetPosition() - params.m_obPosition);
					if ( pobAttackData && pobEntity != params.m_pobOriginator )
					{
						// Find an originating position
						CPoint obOriginatingPosition = ( params.m_pobOriginator ) ? params.m_pobOriginator->GetPosition() : CPoint( 0.0f, 0.0f, 0.0f );

						CStrike* pobStrike = NT_NEW CStrike(	0,
															pobEntity,
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
															obOriginatingPosition );

						// pobEntity->GetAttackComponent()->ReceiveStrike(pobStrike);
						SyncdCombat::Get().PostStrike( pobStrike );

						if ( pobEntity->GetMessageHandler() )
						{
							pobEntity->GetMessageHandler()->ReceiveMsg<msg_combat_struck>();
						}
					}
				}
				else if (pobEntity->GetPhysicsSystem())
				{
					// NOTE: Need to check for line of sight

					if (obRB && !obRB->isFixedOrKeyframed())
					{

						CDirection obDirection(Physics::MathsTools::hkVectorToCDirection(obRB->getPosition()) - params.m_obPosition);						
						obDirection.Normalise();
						obDirection *= params.m_fPush;

						//TODO: angular impulse or parts flying also up... 

						Physics::WriteAccess wmutex;																			
						obRB->applyLinearImpulse( Physics::MathsTools::CDirectionTohkVector( obDirection ) );
						
						//ntPrintf("LUA: explosion hit %s, push is %f\n",(*obIt)->GetName(),fThisPush);

					};

					if ( pobEntity->GetMessageHandler() )
					{
						CMessageSender::SendEmptyMessage( "msg_blast_damage", pobEntity->GetMessageHandler() );
					}
				}
			}
		}
	}
#endif
}
