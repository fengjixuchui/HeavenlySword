
//------------------------------------------------------------------------------------------
//!
//!	\file entity_lua.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/entity.h"
#include "game/entity.inl"

#include "game/luaglobal.h"

#include "game/query.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "anim/transform.h"
#include "anim/hierarchy.h"
#include "game/aicomponent.h"
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
#include "physics/system.h"
#include "camera/camutils.h"
#include "anim/animator.h"
#include "game/renderablecomponent.h"
#include "game/inputcomponent.h"
#include "game/aimcontroller.h" // Aiming component
#include "game/attacks.h"
#include "game/messagehandler.h"
#include "game/movement.h"
#include "game/interactioncomponent.h"
#include "game/awareness.h"
#include "ai/aiformationcomponent.h"
#include "physics/world.h"
#include "physics/advancedcharactercontroller.h"
#include "physics/spearlg.h"
#include "physics/projectilelg.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"
#include "core/exportstruct_anim.h"
#include "blendshapes/blendshapescomponent.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "physics/lookatcomponent.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"

//temp
#include "game/message.h"

//------------------------------------------------------------------------------------------
//  CEntity - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CEntity)
	// Base Methods exposed
	//----------------------------------------------------------------------------------------
	LUA_EXPOSED_METHOD(IsType,		IsType,  "Check the type of the entity", "string type", "") 

	// Exposed components
	LUA_EXPOSED_METHOD_GET(Combat,		GetAttackComponent_nonconst,		"Attack Functionality") 
	LUA_EXPOSED_METHOD_GET(SceneElement,GetSceneElement_nonconst,			"How the entity affects the camera")
	LUA_EXPOSED_METHOD_GET(Message,		GetMessageHandler_nonconst,			"Send Messages")
	LUA_EXPOSED_METHOD_GET(Dynamics,	GetPhysicsSystem_nonconst,			"Dynamics Functionality")
	LUA_EXPOSED_METHOD_GET(Movement,	GetMovement_nonconst,				"Movement Functionality")
	LUA_EXPOSED_METHOD_GET(Awareness,	GetAwarenessComponent_nonconst,		"Awareness Functionality")
	LUA_EXPOSED_METHOD_GET(Input,		GetInputComponent_nonconst,			"Input Functionality")
	LUA_EXPOSED_METHOD_GET(Interaction,	GetInteractionComponent_nonconst,	"Interaction Functionality")
//	LUA_EXPOSED_METHOD_GET(Audio,		GetEntityAudioChannel_nonconst,		"Audio Functionality")
	// Special exposed component for a anonymous, there must be an automatic way to do this... 
	LUA_EXPOSED_METHOD_GET(Formation,	GetFormationComponent,				"Formation Functionality")
	LUA_EXPOSED_METHOD_GET(BlendShapes, GetBlendShapesComponent_nonconst,	"BlendShapes Functionality")
	LUA_EXPOSED_METHOD_GET(LookAt,		GetLookAtComponent_nonconst,		"LookAt Functionality")


	// For Harvey...
	LUA_EXPOSED_METHOD(IsPlayer,		IsPlayer,							"Is this entity a player?",	"",	"")
	LUA_EXPOSED_METHOD(SetPaused,       Lua_Pause,                          "Pause this entity.", "bool pause", "true to pause, false to unpause")
	LUA_EXPOSED_METHOD(Deactivate,		Lua_Deactivate,						"Deactivate/Activate this entity", "bool deactivate", "true to deactivate, false to activate")
	LUA_EXPOSED_METHOD(Hide,			Hide,								"Hide this entity", "", "")
	LUA_EXPOSED_METHOD(Show,			Show,								"Show this entity", "", "")

	// Exposed Members
	LUA_EXPOSED_METHOD_GET(attrib,	GetAttrib, "Attributes")
	LUA_EXPOSED_METHOD_GET(name,	GetName, "The Entity name from XML")

	// Extended Methods
	//----------------------------------------------------------------------------------------
	LUA_EXPOSED_METHOD(GetPosition,					Lua_GetPosition,					"Returns the Entities position", "", "") //name diff as already a GetPosition defined
	LUA_EXPOSED_METHOD(GetPositionNamed,				Lua_GetPositionNamed,				"Create a vector lua object containing information on the entity position.", "string transform", "")
	LUA_EXPOSED_METHOD(SetPosition,					Lua_SetPosition,					"Set the Entities position", "number x, number y, number z", ": x co-ord|: y co-ord|: z co-ord") //name diff as already a SetPosition defined
	LUA_EXPOSED_METHOD(SetPositionToLocator,			Lua_SetPositionToLocator,			"Set the Entities position to a locator", "string locator", "")
	LUA_EXPOSED_METHOD(SetTranslationRelativeToEntity,	Lua_SetTranslationRelativeToEntity,	"Set local translation relative to an entity.","","")
	LUA_EXPOSED_METHOD(SetRotationRelativeToEntity,	Lua_SetRotationRelativeToEntity,	"Set local rotation relative to an entity.", "", "")
	LUA_EXPOSED_METHOD(CheckIfCharacterEntity,			Lua_CheckIfCharacterEntity,			"Check whether this entity is a character or not", "", "")
	LUA_EXPOSED_METHOD(RemoveSelfFromWorld,			Lua_RemoveSelfFromWorld,			"Flags an entity for deletion", "", "")
	LUA_EXPOSED_METHOD(IsPowerHeld,						Lua_IsPowerHeld,					"Checks if power is held", "number x", ": x duration")
	LUA_EXPOSED_METHOD(IsAttackHeld,					Lua_IsAttackHeld,					"Checks if attack is held", "number x", ": x duration")
	LUA_EXPOSED_METHOD(IsActionHeld,					Lua_IsActionHeld,					"Checks if action is held", "number x", ": x duration")
	LUA_EXPOSED_METHOD(ResetAimingComponent,			Lua_ResetAimingComponent,			"Resets the aiming component", "", "")
	LUA_EXPOSED_METHOD(SetAimRange,						Lua_SetAimRange,					"Sets the aim range for turretpoints", "", "")
	LUA_EXPOSED_METHOD(ResetAimRange,					Lua_ResetAimRange,					"Sets the aim range for turretpoints", "", "")
	LUA_EXPOSED_METHOD(Reparent,						Lua_Reparent,						"Reparent this entity", "entity newParent, string transform", "")
	LUA_EXPOSED_METHOD(ReparentToWorld,					Lua_ReparentToWorld,				"Reparent this entity to the world", "", "")
	LUA_EXPOSED_METHOD(SetParentEntity,					Lua_SetParentEntity,				"Sets the parent", "entity", "")
	LUA_EXPOSED_METHOD(SetLocalTransform,				Lua_SetLocalTransform,				"Sets the local transform", "", "")
	//LUA_EXPOSED_METHOD(SetLocalNamedTransform,			Lua_SetLocalNamedTransform,			"Sets the local transform", "", "")
	LUA_EXPOSED_METHOD(MakeDead,						Lua_MakeDead,						"Sets the dead entity info flag, which is used by the camera and combat code to tell if the entity is important and can be attacked/should be on screen", "", "")
	LUA_EXPOSED_METHOD(PlayAnim,						Lua_AnimPlay,						"Play an animation on this entity",	"string animname, float speed, bool locomoting, bool looping", "")
	LUA_EXPOSED_METHOD(SpawnExplodingKegExplosion,		Lua_SpawnExplodingKegExplosion,		"Creates an exploding-keg explosion on the entity", "", "")

	//Hacky methods.
	LUA_EXPOSED_METHOD(AttachRoachShellToEntity,		Lua_AttachRoachShellToEntity,		"Attach a roach shell to an entity", "entity pointer", "Entity to attach shell to")

LUA_EXPOSED_END(CEntity)



//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_GetPosition
//! Returns the position of this entity as a table.
//!
//------------------------------------------------------------------------------------------
NinjaLua::LuaObject	CEntity::Lua_GetPosition()
{
	CPoint ptPos = GetPosition();

	//no CPoint stuff in NinjaLUa yet, so just using table
	NinjaLua::LuaObject tbl = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());
	tbl.Set("x", ptPos.X());
	tbl.Set("y", ptPos.Y());
	tbl.Set("z", ptPos.Z());

	return tbl;
}

//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_GetPositionNamed
//! Create a vector lua object containing information on the entity position.
//!
//------------------------------------------------------------------------------------------
NinjaLua::LuaObject	CEntity::Lua_GetPositionNamed(const char* pcTransform)
{
	ntAssert(GetHierarchy());
	ntAssert(pcTransform);
	ntAssert(GetHierarchy()->GetTransform(pcTransform));

	NinjaLua::LuaObject obTable;
	obTable.AssignNewTable(CLuaGlobal::Get().State());
	
	const CPoint& pos=GetHierarchy()->GetTransform(pcTransform)->GetWorldTranslation();

	obTable.Set("x", pos.X());
	obTable.Set("y", pos.Y());
	obTable.Set("z", pos.Z());

	return obTable;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Entity_Lua::Lua_AnimPlay(name,speed,locomoting,looping)
// DESCRIPTION:	Play an anim on the target entity.
//-------------------------------------------------------------------------------------------------
void CEntity::Lua_AnimPlay (CHashedString pcAnim, float fSpeed, bool bLocomoting, bool bLooping)
{
	// Parameters:
	// Name - Short name for the anim
	// Speed - Speed multiplier for the anim, default is 1.0
	// Locomoting - Is the anim playing relative to its current world position? If nil is specified, the default for the anim is applied.
	// Looping - Does this anim loop? If nil is specified, the default for the anim is applied.

	ntAssert( ! ntStr::IsNull(pcAnim) );

	CAnimator* pobAnimator = GetAnimator();
	ntAssert( pobAnimator );

	CAnimationPtr obNewAnim = pobAnimator->CreateAnimation( pcAnim );

	if (obNewAnim!=0)
	{
		obNewAnim->SetSpeed(fSpeed);

		int iFlags=0;

		if (bLocomoting)
		{
			iFlags|=ANIMF_LOCOMOTING;
		}

		if (bLooping)
		{
			iFlags|=ANIMF_LOOPING;
		}
	
		obNewAnim->SetFlagBits( iFlags );

		pobAnimator->AddAnimation( obNewAnim );
	}
	else
	{
		ntPrintf("Warning: Lua_AnimPlay - Entity %s has no anim called %s\n", ntStr::GetString(GetName()),ntStr::GetString(pcAnim));
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_AnimStopAll
//!	Stops all animations playing
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_AnimStopAll()
{
	CAnimator* pobAnimator = GetAnimator();
	
	if (pobAnimator) // Safety check
	{
		pobAnimator->RemoveAllAnimations();
	}
}



//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Anim_MessageOnCompletion(anim name)
// DESCRIPTION:	Generates 'msg_animdone' message when specified anim is completed.
//-------------------------------------------------------------------------------------------------
void CEntity::Lua_AnimMessageOnCompletion (const CHashedString& pcAnim)
{
	
	if (ntStr::IsNull(pcAnim))
	{
		ntPrintf("Anim_MessageOnCompletion: Error, anim name string not specified\n");
		return;
	}


	if (!GetAnimator() || !GetMessageHandler())
	{
		ntPrintf("Lua_AnimMessageOnCompletion: Entity %s does not have the right components\n", ntStr::GetString(GetName()));
		return;
	}

	GetAnimator()->GetAnimEventHandler().GenerateAnimDoneMessage(pcAnim);
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_SetMeshVisibility
//! NinjaLua binding. Sets the visibility of a mesg.
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetMeshVisibility(const char* pcMeshName, bool bVisibility)
{
	CRenderableComponent* pobRenderable = GetRenderableComponent();
	ntAssert_p( pobRenderable, ("Error: Entity %s does not have a renderable component", GetName().c_str()) );

	pobRenderable->EnableAllByMeshName(pcMeshName, bVisibility);	
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_SetPosition
//! NinjaLua binding. Sets the position of this entity.
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetPosition(float x, float y, float z)
{
	SetPosition(CPoint(x,y,z));
}

//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_SetPosition
//! NinjaLua binding. Sets the position of this entity.
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetPositionToLocator(const char* pcLocatorName)
{
	CPoint locatorPosition;

	// Get the locator specified
	if (strcmp( pcLocatorName, "" ) != 0)
	{
		CEntity* pobEntity = CEntityManager::Get().FindEntity(pcLocatorName);
		if (pobEntity)
		{
			// if it was a success, store the entity's location
			locatorPosition = pobEntity->GetPosition();
			SetPosition(locatorPosition);
			return;
		}
	}
	user_warn_msg( ("SetPositionToLocator: Locator not found, %s\n", pcLocatorName) );
}

//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_SetTranslationRelativeToEntity
//! NinjaLua binding. Sets local translation relative to it an entity.
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetTranslationRelativeToEntity (CEntity* pobTarget,float fX,float fY,float fZ)
{
	Transform* pobRootTransform=GetHierarchy()->GetRootTransform();

	CDirection obInput(fX,fY,fZ);

	CDirection obOffset=obInput * pobTarget->GetMatrix();

	CPoint obNewTranslation(pobTarget->GetMatrix().GetTranslation() + obOffset);

	pobRootTransform->SetLocalTranslation(obNewTranslation);
}

//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_SetRotationRelativeToEntity
//! NinjaLua binding. Sets local rotation relative to its an entity.
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetRotationRelativeToEntity (CEntity* pobTarget,float fR1,float fR2,float fR3,float fR4)
{
	Transform* pobRootTransform=GetHierarchy()->GetRootTransform();

	CMatrix obRotation( CQuat(fR1,fR2,fR3,fR4) );

	CMatrix obNewLocalMatrix=obRotation * pobTarget->GetMatrix();

	obNewLocalMatrix.SetTranslation(pobRootTransform->GetLocalTranslation());

	pobRootTransform->SetLocalMatrix(obNewLocalMatrix);
}

//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_CheckIfCharacterEntity
//! Check whether this entity is a character or not
//!
//------------------------------------------------------------------------------------------
bool CEntity::Lua_CheckIfCharacterEntity()
{
	return IsCharacter();
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_RemoveSelfFromWorld
//! Flags an entity for deletion.
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_RemoveSelfFromWorld()
{
	RemoveFromWorld();
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_SetIdentity
//! Sets the identity
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetIdentity()
{
	Transform* pobTransform = GetHierarchy()->GetRootTransform();

	CMatrix obIdentity(CONSTRUCT_IDENTITY);

	pobTransform->SetLocalMatrix(obIdentity);
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_SetParentEntity
//! Sets the parent entity
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetParentEntity(CEntity* pobParent)
{
	ntAssert(this);

	SetParentEntity(pobParent);
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_ResetAimingComponent
//! Reset aiming component
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_ResetAimingComponent()
{
	AimingComponent* pobComponent = GetAimingComponent();
	
	if (pobComponent)
		pobComponent->SetFromParent();
}

//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_ResetAimRange
//! Set aim range for archer turret state (w/o turret point)
//!
//! Simply rappers for the Aim component function - please remove these when 
//! archer_state.lua makes its way to C++, TMcK
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetAimRange(float fMinYaw, float fMaxYaw, float fMinPitch, float fMaxPitch)
{
	AimingComponent* pobComponent = GetAimingComponent();
	
	pobComponent->SetAimRange(fMinYaw, fMaxYaw, fMinPitch, fMaxPitch);
}

//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_ResetAimRange
//! Reset aim range for archer turret state (w/o turret point)
//!
//! Simply rappers for the Aim component function - please remove these when 
//! archer_state.lua makes its way to C++, TMcK
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_ResetAimRange()
{
	AimingComponent* pobComponent = GetAimingComponent();
	
	pobComponent->ResetAimRange();
}

//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_IsPowerHeld
//! Checks if power has been held for a specified duration
//!
//------------------------------------------------------------------------------------------
bool CEntity::Lua_IsPowerHeld(float fDuration)
{
	if(GetInputComponent()) // Note: AI characters don't have an input component
	{
		float fHeldTime = GetInputComponent()->GetVHeldTime(AB_PSTANCE);

		if (fHeldTime > fDuration)
		{
			return true;
		}
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_IsAttackHeld
//! Checks if attack has been held for a specified duration
//!
//------------------------------------------------------------------------------------------
bool CEntity::Lua_IsAttackHeld(float fDuration)
{
	if(GetInputComponent()) // Note: AI characters don't have an input component
	{
		float fAttackFastTime = GetInputComponent()->GetVHeldTime(AB_ATTACK_FAST);
		float fAttackMediumTime = GetInputComponent()->GetVHeldTime(AB_ATTACK_MEDIUM);

		float fHeldTime=( fAttackFastTime > fAttackMediumTime ? fAttackFastTime : fAttackMediumTime );

		if (fHeldTime > fDuration)
		{
			return true;
		}
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_IsActionHeld
//! Checks if action has been held for a specified duration
//!
//------------------------------------------------------------------------------------------
bool CEntity::Lua_IsActionHeld(float fDuration)
{
	if(GetInputComponent()) // Note: AI characters don't have an input component
	{
		float fHeldTime = GetInputComponent()->GetVHeldTime(AB_ACTION);

		if (fHeldTime > fDuration)
		{
			return true;
		}
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_CreatePhysicsSystem
//! Creates the physics system
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_CreatePhysicsSystem()
{
	if( GetPhysicsSystem() == 0 )
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( this, GetName() );
		SetPhysicsSystem( system );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_SetLocalTransform
//! 
//! BINDFUNC:  SetLocalTransform( float x, float y, float z, float ax, float ay, float az )
//!
//! Sets the root position and orientation of the targ entity, relative to parent.
//! x,y,z is position.
//! ax,ay,az are euler angles, in degrees (Order is XYZ)
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_SetLocalTransform( float x, float y, float z, float ax, float ay, float az )
{

	Transform* pobTransform;

	pobTransform = GetHierarchy()->GetRootTransform();
	
	// build the matrix
	CMatrix obMat( CONSTRUCT_IDENTITY );
	CCamUtil::MatrixFromEuler_XYZ( obMat, ax* DEG_TO_RAD_VALUE, ay* DEG_TO_RAD_VALUE, az* DEG_TO_RAD_VALUE );
	obMat.SetTranslation( CPoint(x,y,z) );

	// poke it in
	
	pobTransform->SetLocalMatrix( obMat );

	if( GetPhysicsSystem() )
	{
		GetPhysicsSystem()->EntityRootTransformHasChanged();
	}

	/*
	Transform* pobTestTransform = GetHierarchy()->GetRootTransform();
	CMatrix obTest = pobTestTransform->GetLocalMatrix();
	CPoint obPos = obTest.GetTranslation();
	ntPrintf("Entity %s\n", GetName().c_str() );
	ntPrintf("Local Translation X %f Y %f Z %f\n", obPos.X(), obPos.Y(), obPos.Z());
	ntPrintf("Requested Translation X %f Y %f Z %f\n", x, y, z);
	*/

	return;	// nothing to return
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CEntity::Lua_MakeDead()
// DESCRIPTION: Sets the dead entity info flag which is used by the camera and combat code to
//				tell if the entity is important and can be attacked/should be on screen
//-------------------------------------------------------------------------------------------------
void CEntity::Lua_MakeDead()
{
	ToCharacter()->SetDead(true);

	// If it's got a scene element component then set it's importance down to zero...
	if( GetSceneElement() )
	{
		GetSceneElement()->SetImportance(0.f);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_Reparent
//! Reparent this entity
//!
//------------------------------------------------------------------------------------------
bool CEntity::Lua_Reparent(CEntity* pNewParent, CHashedString pcTransform)
{
	if(!pNewParent)
	{
		user_warn_p(0, ("Trying to reparent to a non-existant entity.\n"));
		return false;
	}

	if(!pNewParent->GetHierarchy())
	{
		user_warn_p(0, ("The new parent entity, '%s', has no heirarchy.\n", pNewParent->GetName().c_str()));
		return false;
	}

	// Get our root transform
	Transform* pOurTransform = m_pobHierarchy->GetRootTransform();

	// And the new parent transform
	Transform* pParentTransform = 0;

	if(!ntStr::IsNull(pcTransform))
	{
		// Try to find the requested transform on the new parent heirarchy
		int iIdx = pNewParent->GetHierarchy()->GetTransformIndex(pcTransform);
		if ( iIdx == -1 )
		{
			user_warn_p(0, ("Can't find transform '%s' on '%s' to reparent to.\n", pcTransform.GetDebugString(), pNewParent->GetName().c_str()));
			return false;
		}
		pParentTransform = pNewParent->GetHierarchy()->GetTransform(iIdx);
	}
	else
	{
		pParentTransform = pNewParent->GetHierarchy()->GetRootTransform();
	}

	// Set the parent pointer on the entity
	SetParentEntity(pNewParent);

	// Now deal explicitly with the transforms
	pOurTransform->RemoveFromParent();
	pParentTransform->AddChild(pOurTransform);
	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_ReparentToWorld
//! 
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_ReparentToWorld()
{
	ntError_p(GetHierarchy(), ("Lua_ReparentToWorld: No hierarchy!"));

	Transform* pOurTransform = GetHierarchy()->GetRootTransform();

	if(pOurTransform->GetParentHierarchy()!=CHierarchy::GetWorld())
	{
		CMatrix matWorld = pOurTransform->GetWorldMatrix();

		pOurTransform->RemoveFromParent();

		pOurTransform->SetLocalMatrix(matWorld);	
				
		CHierarchy::GetWorld()->GetRootTransform()->AddChild(pOurTransform);

		SetParentEntity( 0 );
	}
}




//-------------------------------------------------------------------------------------------------
// BINDFUNC: CEntity::Lua_ConstructProjectile(LuaAttributeTable* pAttrTable)
// DESCRIPTION: Construct a projectile.
//-------------------------------------------------------------------------------------------------
void CEntity::Lua_ConstructProjectile(LuaAttributeTable* pAttrTable)
{
	CEntity* pobSelf = this;

	const char* pcProjectileProperties=pAttrTable->GetString("ProjectileProperties").c_str();

	ProjectileProperties* pobProperties=ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(pcProjectileProperties);
	ntAssert(pobProperties);
	
	CPoint obPosition(pAttrTable->GetVector("Position"));
	CDirection obDirection;

	if(pAttrTable->GetBool("FirstPersonAim") == true)
	{
		const CMatrix& obCameraMatrix=CamMan::Get().GetPrimaryView()->GetCurrMatrix();

		CDirection obForward=CDirection(0.0f,0.0f,50.0f) * obCameraMatrix;

		CPoint obEnd=obCameraMatrix.GetTranslation() + obForward;

		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = (	//Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
										Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
										Physics::RAGDOLL_BIT						|
										Physics::SMALL_INTERACTABLE_BIT				|
										Physics::LARGE_INTERACTABLE_BIT				);

		Physics::TRACE_LINE_QUERY stQuery;

		if (Physics::CPhysicsWorld::Get().TraceLine(obCameraMatrix.GetTranslation(),obEnd,pobSelf,stQuery,obFlag))
		{
			obDirection.X()=stQuery.obIntersect.X()-obPosition.X();
			obDirection.Y()=stQuery.obIntersect.Y()-obPosition.Y();
			obDirection.Z()=stQuery.obIntersect.Z()-obPosition.Z();
			obDirection.Normalise();
		}
		else
		{
			obDirection.X()=obEnd.X()-obPosition.X();
			obDirection.Y()=obEnd.Y()-obPosition.Y();
			obDirection.Z()=obEnd.Z()-obPosition.Z();
			obDirection.Normalise();
		}
	}
	else
	{
		obDirection = CDirection(pAttrTable->GetVector("Direction"));
	}

	//pobSelf->GetDynamics()->ConstructProjectile("Projectile",pobProperties,obPosition,obDirection);
	// 2 - Create a group
	Physics::ProjectileLG* lg = Physics::ProjectileLG::Construct( pobSelf, pobProperties, obPosition, obDirection );
	// 3 - Create a system
	if( pobSelf->GetPhysicsSystem() == 0 )
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System( pobSelf, pobSelf->GetName() );
		pobSelf->SetPhysicsSystem( system );
	}
	// 4 - Add the group
	pobSelf->GetPhysicsSystem()->AddGroup( lg );
	lg->Activate();
}


void CreateRoachShellPiece(CEntity* pobEntity, const char* pcName, const char* pcClump, const char* pcTransformName, bool bThrowable)
{
	LuaAttributeTable* pobTable = LuaAttributeTable::Create();
	DataObject* pDO;
	if(bThrowable)
	{
		pDO = ObjectDatabase::Get().ConstructObject( "Interactable_Thrown", pcName, GameGUID(), 0, true, false );
	}
	else
	{
		pDO = ObjectDatabase::Get().ConstructObject( "CEntity", pcName, GameGUID(), 0, true, false );
	}
	CEntity* pobNewEntity  = (CEntity*) pDO->GetBasePtr();
	pobNewEntity->SetAttributeTable( pobTable );
	pobNewEntity->GetAttributeTable()->SetDataObject( pDO );
	pobTable->SetAttribute("Name", pcName );
	pobTable->SetAttribute("Clump", pcClump );

	if(bThrowable)
	{
		pobTable->SetString("SharedAttributes", "Att_Thrown_Shield");	//TODO: Need our own one, Att_Thrown_GGShellPiece perhaps?
		pobTable->SetString("InitialState", "DefaultState");
		pobTable->SetString("ParentTransform", "ROOT");
		pobNewEntity->SetRotation(CQuat(0.0f, 0.0f, 0.0f, 1.0f));	//MUST be done or you'll get a very strange set of axis on the object + warp.
	}

	ObjectDatabase::Get().DoPostLoadDefaults( pDO );

	// Parent the shell piece
	Transform* pobParentTransform = pobEntity->GetHierarchy()->GetTransform( pcTransformName );
	Transform* pobTargTransform = pobNewEntity->GetHierarchy()->GetRootTransform();
	pobNewEntity->SetParentEntity( pobEntity );
	pobTargTransform->RemoveFromParent();
	pobParentTransform->AddChild( pobTargTransform );

	//For throwable pieces, set them to their attached state (so they can't be interacted with or picked up whilst still attached and will be keyframed).
	if(bThrowable)
	{
		if(pobNewEntity->GetMessageHandler())
		{
			Message gotoattached(msg_goto_attachedstate);
			pobNewEntity->GetMessageHandler()->QueueMessage(gotoattached);
		}
	}

	if(pobNewEntity->GetPhysicsSystem())
	{
		pobNewEntity->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
	}
}


void CEntity::Lua_AttachRoachShellToEntity()
{
	ntPrintf("Attaching roach shell to entity\n");
	CEntity* pobEntity = this;
	ntError_p(pobEntity != NULL, ("Lua_AttachRoachShellToEntity called with no entity pointer to attach it to"));
	if(!pobEntity)
	{
		return;
	}

	CreateRoachShellPiece(pobEntity, "GGen_shell_1", "entities/characters/gladiatorgeneral/accessories/shell_upper_lame.clump", "shell_upperJ04", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_2", "entities/characters/gladiatorgeneral/accessories/shell_upperj03_rlame.clump", "shell_upperJ03_Rrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_3", "entities/characters/gladiatorgeneral/accessories/shell_upperj03_llame.clump", "shell_upperJ03_Lrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_4", "entities/characters/gladiatorgeneral/accessories/shell_upperj02_rlame.clump", "shell_upperJ02_Rrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_5", "entities/characters/gladiatorgeneral/accessories/shell_upperj02_llame.clump", "shell_upperJ02_Lrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_6", "entities/characters/gladiatorgeneral/accessories/shell_upperj01_rlame.clump", "shell_upperJ01_Rrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_7", "entities/characters/gladiatorgeneral/accessories/shell_upperj01_llame.clump", "shell_upperJ01_Lrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_8", "entities/characters/gladiatorgeneral/accessories/shell_j00_rlame.clump", "shell_upperJ00_Rrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_9", "entities/characters/gladiatorgeneral/accessories/shell_j00_llame.clump", "shell_upperJ00_Lrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_10", "entities/characters/gladiatorgeneral/accessories/shell_lowerj03_rlame.clump", "shell_lowerJ03_Rrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_11", "entities/characters/gladiatorgeneral/accessories/shell_lowerj03_llame.clump", "shell_lowerJ03_Lrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_12", "entities/characters/gladiatorgeneral/accessories/shell_lowerj02_rlame.clump", "shell_lowerJ02_Rrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_13", "entities/characters/gladiatorgeneral/accessories/shell_lowerj02_llame.clump", "shell_lowerJ02_Lrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_14", "entities/characters/gladiatorgeneral/accessories/shell_lowerj01_rlame.clump", "shell_lowerJ01_Rrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_15", "entities/characters/gladiatorgeneral/accessories/shell_lowerj01_llame.clump", "shell_lowerJ01_Lrib", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_16", "entities/characters/gladiatorgeneral/accessories/shell_lower_lame.clump", "shell_lowerJ04", true);
	CreateRoachShellPiece(pobEntity, "GGen_shell_17", "entities/characters/gladiatorgeneral/accessories/shell_j00_spine.clump", "shell_J00", false);
	CreateRoachShellPiece(pobEntity, "GGen_shell_18", "entities/characters/gladiatorgeneral/accessories/shell_lowerj01_spine.clump", "shell_lowerJ01", false);
	CreateRoachShellPiece(pobEntity, "GGen_shell_19", "entities/characters/gladiatorgeneral/accessories/shell_lowerj02_spine.clump", "shell_lowerJ02", false);
	CreateRoachShellPiece(pobEntity, "GGen_shell_20", "entities/characters/gladiatorgeneral/accessories/shell_lowerj03_spine.clump", "shell_lowerJ03", false);
	CreateRoachShellPiece(pobEntity, "GGen_shell_21", "entities/characters/gladiatorgeneral/accessories/shell_upperj01_spine.clump", "shell_upperJ01", false);
	CreateRoachShellPiece(pobEntity, "GGen_shell_22", "entities/characters/gladiatorgeneral/accessories/shell_upperj02_spine.clump", "shell_upperJ02", false);
	CreateRoachShellPiece(pobEntity, "GGen_shell_23", "entities/characters/gladiatorgeneral/accessories/shell_upperj03_spine.clump", "shell_upperJ03", false);

	ntPrintf("Roach shell creation successful\n");
}


void CEntity::Lua_SpawnExplodingKegExplosion()
{
	CEntity* pobEntity = this;
	ntError_p(pobEntity != NULL, ("Lua_SpawnExplodingKegExplosion: Could not get pointer to keg-explosion entity"));
	if(!pobEntity)
	{
		return;
	}

	//Spawn actual physical explosion
	Physics::CExplosionParams obExplosionParams;
	obExplosionParams.m_fPush = 200.0f;
	obExplosionParams.m_fPushDropoff = 10.0f;
	obExplosionParams.m_fRadius = 8.0f;
	obExplosionParams.m_pobOriginator = pobEntity;
	obExplosionParams.m_obPosition = pobEntity->GetPosition();
	if(pobEntity->GetPhysicsSystem())
	{
		pobEntity->GetPhysicsSystem()->Lua_AltExplosion( obExplosionParams );
	}

	//Play world sound.
	AudioHelper::PlaySound("misc_sb", "rocket_explode", pobEntity);	//TODO: Needs a custom sound.

	//Spawn Visuals/Effects
	//TODO: This really needs a custom explosion, we're just hijacking the bazooka one here.
	FXHelper::Pfx_CreateStatic("RocketExplode_Def", pobEntity, "ROOT");
	FXHelper::Pfx_CreateStatic("RocketExplode_ComplexSeeder_Def", pobEntity, "ROOT");
	FXHelper::Pfx_CreateStatic("DirtWake_02", pobEntity, "ROOT");
	FXHelper::Pfx_CreateStatic("Initial_boom", pobEntity, "ROOT");
	FXHelper::Pfx_CreateStatic("frag_Def", pobEntity, "ROOT");

	//Camera shake?
	CPoint ptPos = pobEntity->GetPosition();
	CamMan::Get().Shake(2.f, 1.f, 3.f, &ptPos, 50.f*50.f);
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_Pause
//! 
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_Pause(bool bPause)
{
	Pause(bPause, true);
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_Deactivate
//! 
//!
//------------------------------------------------------------------------------------------
void CEntity::Lua_Deactivate(bool bDeactivate)
{
	Pause(bDeactivate, true);

	if(bDeactivate)
	{
		Hide();
	}
	else
	{
		Show();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CEntity::Lua_
//! 
//!
//------------------------------------------------------------------------------------------
//void CEntity::Lua_()
//{
//}

