/***************************************************************************************************
*
*	FILE			entitybindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/

#include "core/exportstruct_anim.h"

#include "anim/animation.h"
#include "anim/animator.h"
#include "anim/hierarchy.h"

#include "game/attacks.h"
#include "game/entitymanager.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/luaglobal.h"
#include "game/messagehandler.h"
#include "game/movement.h"
#include "game/renderablecomponent.h"
#include "game/interactioncomponent.h"
#include "game/awareness.h"
#include "game/query.h"

#include "gfx/shadowsystem.h"
#include "gfx/levelofdetail.h"

#include "editable/enumlist.h"

#include "aicomponent.h"
#include "entitybindings.h"
#include "special.h"

#include "JAMNET/netman.h"
#include "core/timer.h"

#include "hair/chaincore.h"
#include "hair/effectchain.h"
#include "hair/chain2render.h"
#include "effect/commanderchains.h"

#include "gfx/meshinstance.h"
#include "game/luaattrtable.h"
#include "objectdatabase/dataobject.h"
#include "camera/sceneelementcomponent.h"

#include "game/luahelp.h"
#include "gfx/renderersettings.h"
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
#include "game/aimcontroller.h" // aiming component

#include "Physics/config.h"
#include "Physics/advancedcharactercontroller.h"
#include "Physics/system.h"

// for CCamUtil::MatrixFromEuler_XYZ(). Maybe should break out math support fns into
// their own file  - camutils.h pulls in a lot (renderer.h, base.h etc...)
#include "camera/camutils.h"


#include "blendshapes/blendshapescomponent.h"
#include "blendshapes/xpushapeblending.h"
#include "physics/lookatcomponent.h"



//-------------------------------------------------------------------------------------------------
// BINDFUNC:  entity Targ()
// DESCRIPTION: // aiming component
// Returns the current target entity (nil if none)
// DEPREICATED! Just use the 'self' table instead!
//-------------------------------------------------------------------------------------------------
static CEntity* Targ()
{
	return CLuaGlobal::Get().GetTarg();
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  SetTarg( entity NewTarg )
// DESCRIPTION:
// Sets Targ to a new entity. DEPREICATED - use SetSelf() instead! 
//-------------------------------------------------------------------------------------------------
static void SetTarg(CEntity* pobEnt )
{
	CLuaGlobal::Get().SetTarg( pobEnt );
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  IsDirectionHeld()
// DESCRIPTION:
// Finds whether or not a direction is being held on the input pad
//-------------------------------------------------------------------------------------------------
static bool IsDirectionHeld ()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	//ntAssert( pobEnt );
	//ntAssert( pobEnt->GetInputComponent() );

	if ((pobEnt->GetInputComponent() && pobEnt->GetInputComponent()->IsDirectionHeld()) ||
		(pobEnt->IsAI() && ((AI*)pobEnt)->GetAIComponent()->GetMovementMagnitude() > 0.3f))
	{
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  SetSelf( entity NewSelf )
// DESCRIPTION:
// Switches the 'self' table to another entity.
// 'self' is the current entity and should _only_ be changed using this
// function. Never change the self variable directly!
//-------------------------------------------------------------------------------------------------
static int SetSelf( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args(pobState);
	
	if(args[1].IsLightUserData()) // Check to see if an entity pointer has been passed in
	{
		ntAssert( false );
		return 0;
	}
	else if( args[1].Is<CEntity*>() )
	{
		CLuaGlobal::Get().SetTarg(args[1].Get<CEntity*>() );
		return 0;
	}

	// Otherwise we are specifying the entity state table
	NinjaLua::LuaObject obNewSelf( args[1] );

	LuaAttributeTable* pAttrTable = LuaAttributeTable::GetFromLuaState( *obNewSelf.GetState() );
	//ntAssert_p( pAttrTable != 0, ("Not a Entity") );

	if (pAttrTable)
	{
		ntPrintf("SETSELF - Attribute table being passed instead of an Entity:\n%s\n", pobState->FileAndLine());
		//ntAssert(false); // Assert to temporarily catch attribute tables

		CLuaGlobal::Get().SetTarg( pAttrTable->GetEntity() );
		return 0;
	}

	ntPrintf("WARNING: Invalid argument in SetSelf\n");

	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: DoesEntityExist(string)
// DESCRIPTION: Find out if a particular entity exists in the world.
//-------------------------------------------------------------------------------------------------
static bool DoesEntityExist (CHashedString pcString)
{
	if (CEntityManager::Get().FindEntity(pcString))
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateEntityFromLua(  )
// DESCRIPTION:
// Sets Targ to a new entity
//-------------------------------------------------------------------------------------------------
#ifdef BYPASS_LPCD_PASS_BY_VALUE
int CreateEntityFromLua( LuaState* pobState )
{
	LuaStack args(pobState);

	ntAssert( args[1].IsTable() );
	LuaObject obPropertiesTable( args[1] );

#else
void CreateEntityFromLua(NinjaLua::LuaObject obPropertiesTable )
{
#endif

	// if we are just a normal lua table this will provide a LuaAttributeTable interface to it
	LuaAttributeTable temp( obPropertiesTable ); 
	// get a lua attribute table, if we were passed a normal lua table handle that as well
	LuaAttributeTable* pAttrTable = LuaAttributeTable::GetFromLuaState( *obPropertiesTable.GetState() );
	if( pAttrTable == 0 )
	{
		pAttrTable = &temp;
	}
	CreateEntityFromLuaAttributeTable( pAttrTable );
}

void CreateEntityFromLuaAttributeTable( LuaAttributeTable* pAttrTable )
{

	ntstd::String pClassName;
	ntstd::String pName;

	NinjaLua::LuaObject ClassNameObj = pAttrTable->GetAttribute("Class");
	if( ClassNameObj.IsNil() )
	{
		pClassName = "CEntity"; // default to CEntity
	} else
	{
		pClassName = ClassNameObj.GetString();
	}

	NinjaLua::LuaObject NameObj = pAttrTable->GetAttribute("Name");
	if( NameObj.IsNil() )
	{
		pName = DataObject::GetUniqueName(); // get a unique name if none provided
	} else
	{
		pName = NameObj.GetString();
	}

	DataObject* pDO = ObjectDatabase::Get().ConstructObject( pClassName.c_str(), pName.c_str(), GameGUID(), 0, true, false );

	CEntity* pobNewEntity  = (CEntity*) pDO->GetBasePtr();
	pobNewEntity->SetAttributeTable( LuaAttributeTable::Create() );
	pobNewEntity->GetAttributeTable()->SetDataObject( pDO );
	pAttrTable->DeepCopyTo( pobNewEntity->GetAttributeTable() );
	ObjectDatabase::Get().DoPostLoadDefaults( pDO );
	SetTarg(pobNewEntity);

#ifdef BYPASS_LPCD_PASS_BY_VALUE
	return 0;
#endif
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Anim_Play(name,speed,locomoting,looping)
// DESCRIPTION:	Play an anim on the target entity.
//-------------------------------------------------------------------------------------------------
static int Anim_Play (NinjaLua::LuaState* pobState)
{
	// Parameters:
	// Name - Short name for the anim
	// Speed - Speed multiplier for the anim, default is 1.0
	// Locomoting - Is the anim playing relative to its current world position? If nil is specified, the default for the anim is applied.
	// Looping - Does this anim loop? If nil is specified, the default for the anim is applied.

	NinjaLua::LuaStack args(pobState);
	ntAssert( args[1].IsString() );

	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	CAnimator* pobAnimator = pobEnt->GetAnimator();
	ntAssert( pobAnimator );

	CAnimationPtr obNewAnim = pobAnimator->CreateAnimation( args[1].GetString() );

	if (obNewAnim!=0)
	{
		if (args[2].IsNumber())
		{
			obNewAnim->SetSpeed(args[2].GetFloat());

			int iFlags=0;

			if (args[3].IsBoolean() && args[3].GetBoolean())
			{
				iFlags|=ANIMF_LOCOMOTING;
			}

			if (args[4].IsBoolean() && args[4].GetBoolean())
			{
				iFlags|=ANIMF_LOOPING;
			}
		
			obNewAnim->SetFlagBits( iFlags );
		}

		pobAnimator->AddAnimation( obNewAnim );
	}
	else
	{
		ntPrintf("Warning: Anim_Play - Entity %s has no anim called %s\n",pobEnt->GetName().c_str(),args[1].GetString());
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Anim_StopAll()
// DESCRIPTION:	Stop all anims playing on target entity.  Should only be called on simple entities
//				without movement controllers.  In characters the animations are managed by the 
//				movement controllers and should not be removed without their knowledge - GH
//-------------------------------------------------------------------------------------------------
static void Anim_StopAll ()
{
	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert( pobSelf );

	CAnimator* pobAnimator = pobSelf->GetAnimator();
	
	if (pobAnimator) // Safety check
	{
		pobAnimator->RemoveAllAnimations();
	}
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Anim_MessageOnCompletion(anim name)
// DESCRIPTION:	Generates 'msg_animdone' message when specified anim is completed.
//-------------------------------------------------------------------------------------------------
static int Anim_MessageOnCompletion (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);

	if (!args[1].IsString())
	{
		ntPrintf("Anim_MessageOnCompletion: Error, anim name string not specified\n");
		return 0;
	}

	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();

	if (!pobSelf->GetAnimator() || !pobSelf->GetMessageHandler())
	{
		ntPrintf("Anim_MessageOnCompletion: Entity %s does not have the right components\n",pobSelf->GetName().c_str());
		return 0;
	}

	pobSelf->GetAnimator()->GetAnimEventHandler().GenerateAnimDoneMessage(args[1].GetString());

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Anim_GetDuration(anim name)
// DESCRIPTION:	Get the duration of a specified anim on the target entity.
//-------------------------------------------------------------------------------------------------
static float Anim_GetDuration (CHashedString pcShortAnimName)
{
	CEntity* pEnt = CLuaGlobal::Get().GetTarg();
	ntAssert(pEnt);

	const CAnimationHeader* pAnimH = pEnt->FindAnimHeader( pcShortAnimName, false );

	ntAssert(pAnimH);
	
	return pAnimH->GetDuration();
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Anim_IsAnimating()
// DESCRIPTION:	Find out if the current target entity is playing an anim.
//-------------------------------------------------------------------------------------------------
static bool Anim_IsAnimating ()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	ntAssert( pobEnt->GetAnimator() );

	return pobEnt->GetAnimator()->IsPlayingAnimation();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: GetEntityYAxis(entity *pobEnt)
// DESCRIPTION: Get the 'up' vector of an entity.
//-------------------------------------------------------------------------------------------------
static NinjaLua::LuaObject GetEntityYAxis (CEntity* pobEnt)
{
	ntAssert(pobEnt);
	ntAssert(pobEnt->GetHierarchy());

	NinjaLua::LuaObject obTable;
	obTable.AssignNewTable(CLuaGlobal::Get().State());
	
	const CDirection& obZAxis=pobEnt->GetMatrix().GetYAxis();

	obTable.Set("x",obZAxis.X());
	obTable.Set("y",obZAxis.Y());
	obTable.Set("z",obZAxis.Z());

	return obTable;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: GetEntityZAxis(entity *pobEnt)
// DESCRIPTION:
//  Get the 'forward' vector of an entity.
//-------------------------------------------------------------------------------------------------
static NinjaLua::LuaObject GetEntityZAxis (CEntity* pobEnt)
{
	ntAssert(pobEnt);
	ntAssert(pobEnt->GetHierarchy());

	NinjaLua::LuaObject obTable;
	obTable.AssignNewTable(CLuaGlobal::Get().State());
	
	const CDirection& obZAxis=pobEnt->GetMatrix().GetZAxis();

	obTable.Set("x",obZAxis.X());
	obTable.Set("y",obZAxis.Y());
	obTable.Set("z",obZAxis.Z());

	return obTable;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: GetEntityZAxis(entity *pobEnt, string transform)
// DESCRIPTION:
//  Get the 'forward' vector of an entity.
//-------------------------------------------------------------------------------------------------
static NinjaLua::LuaObject GetEntityZAxisNamed(CEntity* pEnt, CHashedString pcTransform)
{
	ntAssert(pEnt);
	ntAssert(pEnt->GetHierarchy());
	ntAssert(!ntStr::IsNull(pcTransform));
	ntAssert(pEnt->GetHierarchy()->GetTransform(pcTransform));

	NinjaLua::LuaObject obTable;
	obTable.AssignNewTable(CLuaGlobal::Get().State());
	
	const CDirection& obZAxis = pEnt->GetHierarchy()->GetTransform(pcTransform)->GetWorldMatrix().GetZAxis();

	obTable.Set("x",obZAxis.X());
	obTable.Set("y",obZAxis.Y());
	obTable.Set("z",obZAxis.Z());

	return obTable;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: VectorByLocalMatrix(entity,vector)
// DESCRIPTION: Multiply a vector against the local matrix of the target entity.
//-------------------------------------------------------------------------------------------------
static NinjaLua::LuaObject VectorByLocalMatrix (CEntity* pobEnt, NinjaLua::LuaObject obVector)
{
	if (!pobEnt) // No entity specified so use target
	{
		pobEnt = CLuaGlobal::Get().GetTarg();
	}

	ntAssert( pobEnt );
	ntAssert( pobEnt->GetHierarchy() );

	CDirection obInput(obVector["x"].GetFloat(),obVector["y"].GetFloat(),obVector["z"].GetFloat());

	CDirection obOutput=obInput * pobEnt->GetHierarchy()->GetRootTransform()->GetLocalMatrix();

	NinjaLua::LuaObject obTable;
	obTable.AssignNewTable(CLuaGlobal::Get().State());
	
	obTable.Set("x",obOutput.X());
	obTable.Set("y",obOutput.Y());
	obTable.Set("z",obOutput.Z());

	return obTable;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: VectorByWorldMatrix(entity,vector)
// DESCRIPTION: Multiply a vector against the world matrix of the target entity.
//-------------------------------------------------------------------------------------------------
static NinjaLua::LuaObject VectorByWorldMatrix (CEntity* pobEnt, NinjaLua::LuaObject obVector)
{
	if (!pobEnt) // No entity specified so use target
	{
		pobEnt = CLuaGlobal::Get().GetTarg();
	}

	ntAssert( pobEnt );
	ntAssert( pobEnt->GetHierarchy() );

	CDirection obInput(obVector["x"].GetFloat(),obVector["y"].GetFloat(),obVector["z"].GetFloat());

	CDirection obOutput=obInput * pobEnt->GetMatrix();

	NinjaLua::LuaObject obTable;
	obTable.AssignNewTable(CLuaGlobal::Get().State());
	
	obTable.Set("x",obOutput.X());
	obTable.Set("y",obOutput.Y());
	obTable.Set("z",obOutput.Z());

	return obTable;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:		SetIdentity()
// DESCRIPTION:		Set local matrix of the root transform to identity.
//-------------------------------------------------------------------------------------------------
static void SetIdentity ()
{
	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert( pobSelf );

	Transform* pobTransform=pobSelf->GetHierarchy()->GetRootTransform();

	CMatrix obIdentity(CONSTRUCT_IDENTITY);

	pobTransform->SetLocalMatrix(obIdentity);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  EnableMovementComponent()
// DESCRIPTION: This enables the movement component
//-------------------------------------------------------------------------------------------------
static void EnableMovementComponent()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	ntAssert( pobEnt->GetMovement() );
	pobEnt->GetMovement()->SetEnabled( true );
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  DisableMovementComponent()
// DESCRIPTION: This disables the movement component, but it doesn't remove the anims
//				associated with the current controller or transition
//-------------------------------------------------------------------------------------------------
static void DisableMovementComponent()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );

	ntAssert( pobEnt->GetMovement() );
	pobEnt->GetMovement()->SetEnabled( false );
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  EnableAnimatorComponent()
// DESCRIPTION:
//-------------------------------------------------------------------------------------------------
static void EnableAnimatorComponent()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );

	ntAssert( pobEnt->GetAnimator() );
	pobEnt->GetAnimator()->Enable();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  DisableAnimatorComponent()
// DESCRIPTION:
//-------------------------------------------------------------------------------------------------
static void DisableAnimatorComponent()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );

	ntAssert( pobEnt->GetAnimator() );
	pobEnt->GetAnimator()->Disable();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Show( string transformname )
// DESCRIPTION:	Make all the meshes on transform name visible. If no transform name is specified,
//				it will make all transforms visible.
//-------------------------------------------------------------------------------------------------
static int Show( NinjaLua::LuaState* pobState )
{
	CEntity* pobTarg = CLuaGlobal::Get().GetTarg();
	ntError_p( pobTarg, ("Targ is nil!") );
	CRenderableComponent* pobRenderableComp = pobTarg->GetRenderableComponent();
	ntAssert( pobRenderableComp );
	CHierarchy* pobH = pobTarg->GetHierarchy();
	ntAssert( pobH );

	NinjaLua::LuaStack obArgs(pobState);

	if (obArgs[1].IsString())
	{
		int iIdx = pobH->GetTransformIndex( CHashedString( obArgs[1].GetString() ) );
		ntError_p( iIdx!=-1, ( "transform '%s' not found!", obArgs[1].GetString() ) );
		Transform* pobTransform = pobH->GetTransform( iIdx );

		pobRenderableComp->EnableAllByTransform( pobTransform, true );
	}
	else // we do this by adding all renderables to the level rendering list
	{
		pobRenderableComp->AddRemoveAll_Game( true );
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Hide( string transformname )
// DESCRIPTION:	Make all the meshes on transform name invisible. If no transform name is specified,
//				it will make all transforms invisible.
//-------------------------------------------------------------------------------------------------
static int Hide( NinjaLua::LuaState* pobState )
{
	CEntity* pobTarg = CLuaGlobal::Get().GetTarg();
	ntError_p( pobTarg, ("Targ is nil!") );
	CRenderableComponent* pobRenderableComp = pobTarg->GetRenderableComponent();
	ntAssert( pobRenderableComp );
	CHierarchy* pobH = pobTarg->GetHierarchy();
	ntAssert( pobH );

	NinjaLua::LuaStack obArgs(pobState);

	if (obArgs[1].IsString()) // We are referring to the transform we wish to hide by name
	{
		int iIdx = pobH->GetTransformIndex( CHashedString( obArgs[1].GetString() ) );
		ntError_p( iIdx!=-1, ( "transform '%s' not found!", obArgs[1].GetString() ) );
		Transform* pobTransform = pobH->GetTransform( iIdx );

		pobRenderableComp->EnableAllByTransform( pobTransform, false );
	}
	else if (obArgs[1].IsLightUserData()) // We have a pointer to the renderable itself
	{
		// HC: This is pretty evil and should be used with caution!
		// Originally added so that we could selectively turn off the chain renderable. See CreateChainmanChains()
		CRenderable* pobRenderable=(CRenderable*)obArgs[1].GetLightUserData();
		pobRenderable->DisableRendering(true);
		pobRenderable->DisableShadowCasting(true);
		pobRenderable->DisableShadowRecieving(true);
	}		
	else // We want to hide all transforms
	{
		// we do this by the renderable component suspending all
		// processing on its sub-renderables
		pobRenderableComp->AddRemoveAll_Game( false );
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  SetMeshVisibility( string, bool )
// DESCRIPTION: Toggle the rendering of a mesh on the target entity.
//-------------------------------------------------------------------------------------------------
static int SetMeshVisibility ( NinjaLua::LuaState* L )
{
	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert_p( pobSelf, ("Error: No valid target entity found!") );

	CRenderableComponent* pobRenderable = pobSelf->GetRenderableComponent();
	ntAssert_p( pobRenderable, ("Error: Entity %s does not have a renderable component",pobSelf->GetName().c_str()) );

	NinjaLua::LuaStack obArgs(L);

	ntAssert_p(obArgs[1].IsString(),("Error: First argument in SetMeshVisibility is not a string!\n"));
	ntAssert_p(obArgs[2].IsBoolean(),("Error: Second argument in SetMeshVisibility is not a bool!\n"));

	pobRenderable->EnableAllByMeshName(obArgs[1].GetString(),obArgs[2].GetBoolean());	

	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  bool Reparent( entity newparent, string newparenttransform )
// DESCRIPTION:
// Sets the parent of the targ entity.
//-------------------------------------------------------------------------------------------------
bool Reparent( CEntity* pobNewParent, CHashedString pcNewParentTransform )
{
	// Get the current target - if we can't find it then something is very wrong
	CEntity* pobTarg = CLuaGlobal::Get().GetTarg();
	ntError_p( pobTarg, ("Targ is nil!") );

	// Make sure that the target has a heirarchy
	if ( !pobTarg->GetHierarchy() )
	{
		user_warn_p( 0, ("The target for reparenting, '%s', has no hierarchy!", pobTarg->GetName().c_str() ) );
		return false;
	}

	// Make sure that we have something to reparent to
	if ( !pobNewParent )
	{
		user_warn_p( 0, ( "Trying to reparent to a non-existant entity.\n" ) );
		return false;
	}

	// Make sure that we are not doing anything really silly
	if ( pobTarg == pobNewParent )
	{
		user_warn_p( 0, ( "Trying to reparent '%s' to itself.\n", pobNewParent->GetName().c_str() ) );
		return false;
	}

	// Make sure the item we want to reparent to has a heirarchy
	if ( !pobNewParent->GetHierarchy() )
	{
		user_warn_p( 0, ( "The new parent entity, '%s', has no heirarchy.\n", pobNewParent->GetName().c_str() ) );
		return false;
	}

	// Try to find the requested transform on the new parent heirarchy
	int iIdx = pobNewParent->GetHierarchy()->GetTransformIndex( CHashedString( pcNewParentTransform ) );

	// Make sure that we found it
	if ( iIdx == -1 )
	{
		user_warn_p( 0, ( "Can't find transform '%s' on '%s' to reparent to.\n", ntStr::GetString(pcNewParentTransform), ntStr::GetString(pobNewParent->GetName()) ) );
		return false;
	}

	// We can now get the transforms that we require
	Transform* pobParentTransform = pobNewParent->GetHierarchy()->GetTransform( iIdx );
	Transform* pobTargTransform = pobTarg->GetHierarchy()->GetRootTransform();	

	// Set the parent pointer on the entity
	pobTarg->SetParentEntity( pobNewParent );

	// Now deal explicitly with the transforms
	pobTargTransform->RemoveFromParent();
	pobParentTransform->AddChild( pobTargTransform );

/*
	CMatrix obMatrix;
	CCamUtil::MatrixFromEuler_XYZ( obMatrix, 3.4f, 0.0f, 0.0f );
	obMatrix.SetTranslation( CPoint( 0.0f, 0.1f, -0.2f) );
	pobTargTransform->SetLocalMatrix(obMatrix);
*/
	// We have been successful
	return true;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  ReparentToRoot( entity newparent )
// DESCRIPTION: Reparent target entity to the root of the specified parent entity transform.
//-------------------------------------------------------------------------------------------------
static void ReparentToRoot( CEntity* pobNewParent )
{
	CEntity* pobTarg = CLuaGlobal::Get().GetTarg();
	ntError_p( pobTarg, ("Targ is nil!") );
	ntError_p( pobTarg->GetHierarchy(), ("Targ has no hierarchy!") );

	Transform* pobTargTransform = pobTarg->GetHierarchy()->GetRootTransform();

	Transform* pobParentTransform = pobNewParent->GetHierarchy()->GetRootTransform();

	pobTarg->SetParentEntity( pobNewParent );
	pobTargTransform->RemoveFromParent();
	pobParentTransform->AddChild( pobTargTransform );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  ReparentToWorld()
// DESCRIPTION:
// Reparent an entity to the world hierarchy.
//-------------------------------------------------------------------------------------------------
static void ReparentToWorld ()
{
	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntError_p( pobSelf, ("self is nil!") );
	ntError_p( pobSelf->GetHierarchy(), ("self has no hierarchy!") );

	Transform* pobThisTransform = pobSelf->GetHierarchy()->GetRootTransform();

	if (pobThisTransform->GetParentHierarchy()!=CHierarchy::GetWorld()) // Check to see if we are already parented to the world
	{
		CMatrix obWorldMatrix = pobThisTransform->GetWorldMatrix();

		pobThisTransform->RemoveFromParent();

		pobThisTransform->SetLocalMatrix(obWorldMatrix);	
				
		CHierarchy::GetWorld()->GetRootTransform()->AddChild(pobThisTransform);

		pobSelf->SetParentEntity( 0 );
	}
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  SetParentEntity( entity ent )
// DESCRIPTION: Set the parent of an entity.
//-------------------------------------------------------------------------------------------------
static int SetParentEntity( NinjaLua::LuaState* L )
{
	CEntity* pobSelf=CLuaGlobal::Get().GetTarg();
	ntAssert(pobSelf);

	NinjaLua::LuaStack obArgs( L );

	if (obArgs[1].IsLightUserData() || obArgs[1].IsUserData())
	{
		pobSelf->SetParentEntity(obArgs[1].Get<CEntity*>());
	}
	else
	{
		pobSelf->SetParentEntity( 0 ); // This entity is parentless!
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  SetLocalTransform( string transformname [optional], float x, float y, float z, float ax, float ay, float az )
// DESCRIPTION:
// Sets the root position and orientation of the targ entity, relative to parent.
// x,y,z is position.
// ax,ay,az are euler angles, in degrees (Order is XYZ)
//-------------------------------------------------------------------------------------------------
static int SetLocalTransform( NinjaLua::LuaState* pobState )
{
	CEntity* pobTarg = CLuaGlobal::Get().GetTarg();
	ntAssert( pobTarg );
	ntAssert( pobTarg->GetHierarchy() );

	int arg=1;

	NinjaLua::LuaStack obArgs( pobState );

	Transform* pobTransform;

	if (obArgs[1].IsNumber())
	{
		pobTransform = pobTarg->GetHierarchy()->GetRootTransform();
	}
	else if (obArgs[1].IsString())
	{
		const char* pcString=obArgs[1].GetString();

		CHashedString obHash(pcString);

		pobTransform = pobTarg->GetHierarchy()->GetTransform(obHash);

		++arg;
	}
	else
	{
		ntPrintf("Failed to SetLocalTransform, invalid arguments\n");
		return 0;
	}

	float x = obArgs[arg++].GetFloat();
	float y = obArgs[arg++].GetFloat();
	float z = obArgs[arg++].GetFloat();
	float ax = obArgs[arg++].GetFloat() * DEG_TO_RAD_VALUE;
	float ay = obArgs[arg++].GetFloat() * DEG_TO_RAD_VALUE;
	float az = obArgs[arg++].GetFloat() * DEG_TO_RAD_VALUE;

	// build the matrix
	CMatrix obMat;
	CCamUtil::MatrixFromEuler_XYZ( obMat, ax, ay, az );
	obMat.SetTranslation( CPoint(x,y,z) );

	// poke it in
	
	pobTransform->SetLocalMatrix( obMat );

	if( pobTarg->GetPhysicsSystem() )
	{
		pobTarg->GetPhysicsSystem()->EntityRootTransformHasChanged();
	}

	/*
	Transform* pobTestTransform = pobTarg->GetHierarchy()->GetRootTransform();
	CMatrix obTest = pobTestTransform->GetLocalMatrix();
	CPoint obPos = obTest.GetTranslation();
	ntPrintf("Entity %s\n", pobTarg->GetName().c_str() );
	ntPrintf("Local Translation X %f Y %f Z %f\n", obPos.X(), obPos.Y(), obPos.Z());
	ntPrintf("Requested Translation X %f Y %f Z %f\n", x, y, z);*/

	return 0;	// nothing to return
}

static int GetTransformLocalPosition ( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack obArgs( pobState );

	if (obArgs[1].IsString())
	{
		CEntity* pobTarg = CLuaGlobal::Get().GetTarg();
		ntAssert( pobTarg );
		ntAssert( pobTarg->GetHierarchy() );

		const char* pcString=obArgs[1].GetString();

		CHashedString obHash(pcString);

		Transform* pobTransform = pobTarg->GetHierarchy()->GetTransform(obHash);

		NinjaLua::LuaObject obTable;
		obTable.AssignNewTable(CLuaGlobal::Get().State());
		obTable.Set("x",pobTransform->GetLocalTranslation().X());
		obTable.Set("y",pobTransform->GetLocalTranslation().Y());
		obTable.Set("z",pobTransform->GetLocalTranslation().Z());

		obTable.Push();

		return 1;
	}

	return 0;
}

static int GetTransformLocalRotation ( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack obArgs( pobState );

	if (obArgs[1].IsString())
	{
		CEntity* pobTarg = CLuaGlobal::Get().GetTarg();
		ntAssert( pobTarg );
		ntAssert( pobTarg->GetHierarchy() );

		const char* pcString=obArgs[1].GetString();

		CHashedString obHash(pcString);

		Transform* pobTransform = pobTarg->GetHierarchy()->GetTransform(obHash);

		float ax,ay,az;
		
		CCamUtil::EulerFromMat_XYZ(pobTransform->GetLocalMatrix(),ax,ay,az);

		NinjaLua::LuaObject obTable;
		obTable.AssignNewTable(CLuaGlobal::Get().State());
		obTable.Set("x",ax);
		obTable.Set("y",ay);
		obTable.Set("z",az);

		obTable.Push();

		return 1;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  Send( Message msg, Entity dest )
// DESCRIPTION:
// Sends msg to dest.
//-------------------------------------------------------------------------------------------------
static int Send(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);

	Message  obMsg(eUndefinedMsgID);
	Message* pMsg = 0;

	if(args[1].Is<Message*>())
		pMsg = args[1].Get<Message*>();
	else if(args[1].IsTable())
	{
		NinjaLua::LuaObject tbl = args[1];
		obMsg.FillFromLuaTable(tbl);
		pMsg = &obMsg;
	}
	ntAssert(pMsg);

	CEntity* pobDest=NULL;

	if(args[2].IsUserData())
	{
		pobDest = NinjaLua::LuaValue::Get<CEntity*>( NinjaLua::GetState(*pobState), 2 );
	}
	else if(args[2].IsString())
	{
		pobDest = CEntityManager::Get().FindEntity(args[2].GetString());
	} 

	ntAssert_p(pobDest,("%s: Invalid entity passed into binding Send\n", pobState->FileAndLine()));

	if(pobDest && pobDest->GetMessageHandler())
	{
		pobDest->GetMessageHandler()->QueueMessage(*pMsg);
	}
	else if ( !pobDest )
	{
		// Added the message id string to the warning and sending through lua warning mechanism to make things more useful... - JML
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		const char* pcMsgText = ntStr::GetString(pMsg->GetHashedString("Msg").GetDebugString()); 
		NinjaLua::LogLuaWarning(*pobState, "Trying to send message '%s' to a non existent entity (%s). \n", pcMsgText, args[2].IsString()?args[2].GetString():"<Invalid>");
#else
		NinjaLua::LogLuaWarning(*pobState, "Trying to send message (hash:%i) to a non existent entity (%s). \n", pMsg->GetHashedString("Msg").GetHash(), args[2].IsString()?args[2].GetString():"<Invalid>");
#endif
	}
	else
	{
		// If we don't have a message handler we shall warn the world.
		// If may be that we want to ntAssert here in the future but at
		// the moment lots of changes are happening in the lua state 
		// stuff so i am trying to make sure that the game won't die - GH
		// Added the message id string to the warning and sending through lua warning mechanism to make things more useful... - JML
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		const char* pcMsgText = ntStr::GetString(pMsg->GetHashedString("Msg").GetDebugString());
		NinjaLua::LogLuaWarning(*pobState, "Trying to send message '%s' to an entity (%s) which has no message handler. \n", pcMsgText, pobDest->GetName().c_str());
#else
		NinjaLua::LogLuaWarning(*pobState, "Trying to send message (hash:%i) to an entity (%s) which has no message handler. \n", pMsg->GetHashedString("Msg").GetHash(), pobDest->GetName().c_str());
#endif
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  Send( Message msg, Entity dest )
// DESCRIPTION:
// Sends msg to dest.
//-------------------------------------------------------------------------------------------------
static int SendDelayed(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);

	Message  obMsg(eUndefinedMsgID);
	Message* pMsg = 0;
	float fDelay = 0.0f;

	if(args[1].Is<Message*>())
		pMsg = args[1].Get<Message*>();
	else if(args[1].IsTable())
	{
		NinjaLua::LuaObject tbl = args[1];
		obMsg.FillFromLuaTable(tbl);
		pMsg = &obMsg;
	}
	ntAssert(pMsg);

	CEntity* pobDest=NULL;

	if(args[2].IsUserData())
	{
		pobDest = NinjaLua::LuaValue::Get<CEntity*>( NinjaLua::GetState(*pobState), 2 );
	}
	else if(args[2].IsString())
	{
		pobDest = CEntityManager::Get().FindEntity(args[2].GetString());
	} 

	if(args[3].IsNumber())
	{
		fDelay = args[3].GetFloat();
	}

	ntAssert_p(pobDest,("%s: Invalid entity passed into binding Send\n", pobState->FileAndLine()));

	if(pobDest && pobDest->GetMessageHandler())
	{
		pobDest->GetMessageHandler()->QueueMessageDelayed(*pMsg, fDelay);
	}
	else if ( !pobDest )
	{
		// Added the message id string to the warning and sending through lua warning mechanism to make things more useful... - JML
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		const char* pcMsgText = ntStr::GetString(pMsg->GetHashedString("Msg").GetDebugString()); 
		NinjaLua::LogLuaWarning(*pobState, "Trying to send message '%s' to a non existent entity (%s). \n", pcMsgText, args[2].IsString()?args[2].GetString():"<Invalid>");
#else
		NinjaLua::LogLuaWarning(*pobState, "Trying to send message (hash:%i) to a non existent entity (%s). \n", pMsg->GetHashedString("Msg").GetHash(), args[2].IsString()?args[2].GetString():"<Invalid>");
#endif
	}
	else
	{
		// If we don't have a message handler we shall warn the world.
		// If may be that we want to ntAssert here in the future but at
		// the moment lots of changes are happening in the lua state 
		// stuff so i am trying to make sure that the game won't die - GH
		// Added the message id string to the warning and sending through lua warning mechanism to make things more useful... - JML
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		const char* pcMsgText = ntStr::GetString(pMsg->GetHashedString("Msg").GetDebugString());
		NinjaLua::LogLuaWarning(*pobState, "Trying to send message '%s' to an entity (%s) which has no message handler. \n", pcMsgText, pobDest->GetName().c_str());
#else
		NinjaLua::LogLuaWarning(*pobState, "Trying to send message (hash:%i) to an entity (%s) which has no message handler. \n", pMsg->GetHashedString("Msg").GetHash(), pobDest->GetName().c_str());
#endif
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: ConfigureRenderer
// DESCRIPTION:
// NO LONGER USED, here till we clean up the params
//-------------------------------------------------------------------------------------------------
static int ConfigureRenderer( NinjaLua::LuaState* pobState )
{
	UNUSED( pobState );
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  UseDepthHaze(bool b)
// DESCRIPTION:
//	Sets whether to force the cool camera on opponent death
//-------------------------------------------------------------------------------------------------
static void UseDepthHaze(bool b)
{
	CRendererSettings::bEnableDepthHaze = b;
}


//-------------------------------------------------------------------------------------------------
// Component creation function
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_Hierarchy()
// DESCRIPTION:
// Adds a hierarchy to the entity. Practically all entities will require this.
//-------------------------------------------------------------------------------------------------
static void CreateComponent_Hierarchy()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	pobEnt->InstallHierarchy();
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_MessageHandler()
// DESCRIPTION:
// Adds a message handler to the entity, so it can receive messages.
//-------------------------------------------------------------------------------------------------
static void CreateComponent_MessageHandler()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	pobEnt->InstallMessageHandler();
}




//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_Attack()
// DESCRIPTION:
//-------------------------------------------------------------------------------------------------
static void CreateComponent_Attack( CHashedString pcAttackDefinition )
{
	// Get the current target entity
	Character* pobEnt = CLuaGlobal::Get().GetTarg()->ToCharacter();

	// If we have found the target entity...
	if ( pobEnt )
	{
		// Get the attack definition from the object database
		CAttackDefinition* pobDefinition = ObjectDatabase::Get().GetPointerFromName< CAttackDefinition* >( pcAttackDefinition );
		
		// If we can find the attack definition...
		if ( pobDefinition )
		{
			// Stick the defined component onto the entity
			CAttackComponent* pobNewAttack = NT_NEW_CHUNK(Mem::MC_ENTITY) CAttackComponent( *pobEnt, pobDefinition );
			pobEnt->SetAttackComponent(pobNewAttack);
		}

		// Note that we couldn't find the attack definition
		else
		{
			ntPrintf( "Attack Definition %s not found in CreateComponent_Attack\n", ntStr::GetString(pcAttackDefinition) );
		}
	}

	// Note that we can't find the entity
	else
	{
		ntPrintf( "Current target entity could not be found in CreateComponentAttack\n" );
	}
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_Aware()
// DESCRIPTION:
//-------------------------------------------------------------------------------------------------
//static void CreateComponent_Aware( const char* pcAwareDefinition )
static void CreateComponent_Aware( CHashedString pcAwareDefinition )
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	CAttackTargetingData* pobDefinition = ObjectDatabase::Get().GetPointerFromName< CAttackTargetingData* >( pcAwareDefinition );
	ntAssert( pobDefinition );
	AwarenessComponent* pobNewAware = NT_NEW_CHUNK(Mem::MC_ENTITY) AwarenessComponent( pobEnt, pobDefinition );
	pobEnt->SetAwarenessComponent( pobNewAware );
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_Input()
// DESCRIPTION:
//-------------------------------------------------------------------------------------------------
static void CreateComponent_Input(int iPad)
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	if(pobEnt && pobEnt->IsPlayer())
	{
		// set the input component to be passive
		pobEnt->ToPlayer()->SetInputComponent(NT_NEW_CHUNK(Mem::MC_ENTITY) CInputComponent(pobEnt, (PAD_NUMBER)iPad) );
	}
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_Movement()
// DESCRIPTION: DEPRECATE NOW - JML
//-------------------------------------------------------------------------------------------------
static void CreateComponent_Movement(CHashedString pcDynamicsState)
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );

	UNUSED( pcDynamicsState );
	//Physics::CharacterLG* pobDynamicsState = (Physics::CharacterLG*)pobEnt->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::CHARACTER_CONTROLLER_LG );

	CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement( pobEnt,
											pobEnt->GetAnimator(),
											//pobDynamicsState);
											pobEnt->GetPhysicsSystem() );
	pobEnt->SetMovement(pobMovement);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_SceneElement()
// DESCRIPTION: A SceneElementComponent determines how the camera interacts with this entity.
//-------------------------------------------------------------------------------------------------
static void CreateComponent_SceneElement(CHashedString pcDef)
{
	CEntity* pEnt = CLuaGlobal::Get().GetTarg();
	ntAssert(pEnt);

	SceneElementComponentDef* pDef = ObjectDatabase::Get().GetPointerFromName<SceneElementComponentDef*>(pcDef);
	//ntAssert(pDef);

	if (pDef)
	{
		SceneElementComponent* pSEC = NT_NEW_CHUNK(Mem::MC_ENTITY) SceneElementComponent(pEnt, pDef);
		pEnt->SetSceneElementComponent(pSEC);
	}
}

static void CreateComponent_LookAt( CHashedString obDef )
{
	CEntity* pEnt = CLuaGlobal::Get().GetTarg();
	ntAssert(pEnt);
	
	pEnt->InstallLookAtComponent( obDef );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_BlendShapes()
// DESCRIPTION: Creates a blendshape controller for this entity and, if specified, installs
//			a given BSClump as the initial blendshape set
// NOTE: this component requires CRenderableComponent to be installed previously
// PARAMETERS: pcBSClumpName - the bsclump to install as the current bsset. Can be 0 if desired and 
//								it will install an empty blendshape component
//-------------------------------------------------------------------------------------------------
static void CreateComponent_BlendShapes( const char* pcBSClumpName )
{
	if ( XPUShapeBlending::Get().IsEnabled() ) 
	{
		CEntity* pEnt = CLuaGlobal::Get().GetTarg();
		ntAssert(pEnt);

		pEnt->InstallBlendShapesComponent( pcBSClumpName );
	}
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  BlendShapes_SetBSClump()
// DESCRIPTION: sets the the bsclump as the current blendshape set
// PARAMETERS: pcBSClumpName - the bsclump to install as the current bsset
//-------------------------------------------------------------------------------------------------
static void BlendShapes_SetBSClump( const char* pcBSClumpName )
{
	if ( XPUShapeBlending::Get().IsEnabled() )
	{
		CEntity* pEnt = CLuaGlobal::Get().GetTarg();
		ntAssert( pEnt && pEnt->GetBlendShapesComponent() );

		ntError( pcBSClumpName );
		pEnt->GetBlendShapesComponent()->PushBSSet( pcBSClumpName );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  BlendShapes_AddBSAnimsFromContainer()
// DESCRIPTION: adds bsnims to the blendshpe component (it does not play them)
// PARAMETERS: pcBSAnimContainerName - the container name
//-------------------------------------------------------------------------------------------------
static void BlendShapes_AddBSAnimsFromContainer( const char* pcBSAnimContainerName )
{
	if ( XPUShapeBlending::Get().IsEnabled() )
	{
		CEntity* pEnt = CLuaGlobal::Get().GetTarg();
		ntAssert( pEnt && pEnt->GetBlendShapesComponent() );
		
		ntError( pcBSAnimContainerName );
		pEnt->GetBlendShapesComponent()->AddBSAnimsFromContainer( pcBSAnimContainerName, false );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  BlendShapes_RemoveBSAnimsFromContainer()
// DESCRIPTION: removes bsnims to the blendshpe component (and stops them if playing)
// PARAMETERS: pcBSAnimContainerName - the container name
//-------------------------------------------------------------------------------------------------
static void BlendShapes_RemoveBSAnimsFromContainer( const char* pcBSAnimContainerName  )
{
	if ( XPUShapeBlending::Get().IsEnabled() )
	{
		CEntity* pEnt = CLuaGlobal::Get().GetTarg();
		ntAssert( pEnt && pEnt->GetBlendShapesComponent() );
		
		ntAssert( pcBSAnimContainerName );
		pEnt->GetBlendShapesComponent()->RemoveBSAnimsFromContainer( pcBSAnimContainerName );
	}
}



//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_Animator()
// DESCRIPTION:
// Adds an animator the entity, so it can play anims.
// (requires a hierarchy to animate)
//-------------------------------------------------------------------------------------------------
static void CreateComponent_Animator(CHashedString pcAnimContainer)
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );

	// requires a hierarchy
	if( !pobEnt->GetHierarchy() )
		pobEnt->InstallHierarchy();

	pobEnt->InstallAnimator(pcAnimContainer);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_Audio()
// DESCRIPTION:
// Adds audio channels to the entity so it can emit sounds.
//-------------------------------------------------------------------------------------------------
static void CreateComponent_Audio()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );

	pobEnt->InstallAudioChannel();
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_HairFromLua()
// DESCRIPTION: a hair to an entity
// the only reason why hair are an entity by their own is because they own a hierarchy
//-------------------------------------------------------------------------------------------------
static void CreateComponent_Hair(NinjaLua::LuaObject obPropertiesTable)
{
	// ignore if disbale
	if(!ChainRessource::Exists()) return;

	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	LuaAttributeTable* pAttrTable = pobEnt->GetAttributeTable();
	ntError_p(pAttrTable!=0, ("No Lua Attribuate table (null pointer)"));
	
	OneChain* pChain = ChainRessource::CreateComponent_Hair(pobEnt,pAttrTable);
	if(pChain && pChain->HasIronChain())
	{
		ntError_p( pobEnt->GetHierarchy(), ("Parent Entity has no hierarchy!") );

		// Parent transform is root, as we're a world space effect
		Transform* pParentTransform = pobEnt->GetHierarchy()->GetRootTransform();

		// construct chain renderable		
		// @todo -- chunking of this is tricky as it goes in a shared list
		CommanderChains* pChainsEffect = NT_NEW CommanderChains( pParentTransform, pChain->GetIronChain() );
		ntAssert( pChainsEffect );

		// Get the renderable component of the holding entity
		CRenderableComponent* pRenderable = pobEnt->GetRenderableComponent();
		ntAssert( pRenderable );

		// Give the ownership of this item to the parent renderable
		pRenderable->AddAddtionalRenderable( pChainsEffect );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  CreateComponent_Dynamics()
// DESCRIPTION:
// Add a dynamics component to the entity.
// (requires a hierarchy)
//-------------------------------------------------------------------------------------------------
static void CreateComponent_Dynamics()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );

	// requires a hierarchy
	if( !pobEnt->GetHierarchy() )
		pobEnt->InstallHierarchy();

	pobEnt->InstallDynamics();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  ResetAimingComponent()
// DESCRIPTION:
//-------------------------------------------------------------------------------------------------
static void ResetAimingComponent ()
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();

	AimingComponent* pobComponent = pobEnt->GetAimingComponent();
	
	if (pobComponent)
		pobComponent->SetFromParent();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  NS_MatchEntity()
// DESCRIPTION:
// Matches NS Entity A to Game Entity B
//-------------------------------------------------------------------------------------------------
static void NS_MatchEntity( const char* pcNSEntityName, const char* pcGameEntityName )
{
	ntstd::String NSName( pcNSEntityName );
	NSName += "NSEnt";	// standard extension for NSEntity container names (entity name + "NSEnt")

	// replace the data in m_sName with the new name
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(NSName) );
	ntError_p( pDO, ("NS_MatchEntity can't find %s NSEnt container\n", pcNSEntityName) );
	StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
	ntAssert( pInterface->GetFieldByName("m_sName") );
	pInterface->SetData( pDO, "m_sName", ntstd::String( pcGameEntityName ) );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:		UnpackMsg()
// DESCRIPTION:		Unleash all the unnamed arguments on to the stack.
//-------------------------------------------------------------------------------------------------
static int UnpackMsg(NinjaLua::LuaState* pobState)
{
	int iTop1 = pobState->GetTop();
	iTop1 = iTop1;
	NinjaLua::LuaArgs args(*pobState);

	if(!args.Check<Message*>(1))
		return 0;

	Message* pMsg =0;
	args.Get(pMsg, 1);

	int iTop2 = pobState->GetTop();
	iTop2 = iTop2;

	if(!pMsg->HasUnnamedParams())
		return 0;

	int iParamCount = 0;
	for( int i = 2; i < pMsg->GetParamCount(); ++i)
	{
		iParamCount += pMsg->GetValue(i);
	}
	return iParamCount;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:		CreateRangedWeaponObject()
// DESCRIPTION:		Creates a particular ranged weapon and returns a pointer to it.
//					This is a temporary solution to creating C++ weapons to attach to AI and will
//					likely be replaced once the AI states are in C++.
//-------------------------------------------------------------------------------------------------
CEntity* CreateRangedWeaponObject(NinjaLua::LuaObject obPropertiesTable)
{
	//The pointer to entity that we'll be returning.
	CEntity* pWeapon = NULL;

	//If we are just a normal lua table this will provide a LuaAttributeTable interface to it.
	LuaAttributeTable temp(obPropertiesTable);

	//Get a lua attribute table, if we were passed a normal lua table handle that as well.
	LuaAttributeTable* pAttrTable = LuaAttributeTable::GetFromLuaState(*obPropertiesTable.GetState());
	if(pAttrTable == 0)
	{
		pAttrTable = &temp;
	}

	ntstd::String pClassName;
	ntstd::String pName;

	//Always default classname to Object_Ranged_Weapon.
	pClassName = "Object_Ranged_Weapon";

	NinjaLua::LuaObject NameObj = pAttrTable->GetAttribute("Name");
	if(NameObj.IsNil())
	{
		pName = DataObject::GetUniqueName();	//Get a unique name if none provided.
	}
	else
	{
		pName = NameObj.GetString();
	}

	DataObject* pDO = ObjectDatabase::Get().ConstructObject( pClassName.c_str(), pName.c_str(), GameGUID(), 0, true, false );

	pWeapon  = (CEntity*)pDO->GetBasePtr();
	pWeapon->SetAttributeTable( LuaAttributeTable::Create() );
	pWeapon->GetAttributeTable()->SetDataObject( pDO );
	pAttrTable->DeepCopyTo( pWeapon->GetAttributeTable() );
	ObjectDatabase::Get().DoPostLoadDefaults( pDO );
//	SetTarg(pWeapon);	//TODO: This shouldn't be needed, returning pointer directly to lua, (not via 'this').

	//If the type passed isn't recognized, return NULL.
	ntAssert(pWeapon != NULL);	//We shouldn't get here unless someone requested an invalid weapon to be created.
	return pWeapon;
}


/***************************************************************************************************
*
*	FUNCTION		CEntityBindings::Register
*
*	DESCRIPTION		Registers the static functions with the scripting environment
*
***************************************************************************************************/
void CEntityBindings::Register()
{
	NinjaLua::LuaObject obLuaGlobals = CLuaGlobal::Get().State().GetGlobals();

	obLuaGlobals.Register( "Targ", Targ );
	obLuaGlobals.Register( "SetTarg", SetTarg );

	obLuaGlobals.RegisterRaw( "SetSelf", SetSelf );

#ifdef BYPASS_LPCD_PASS_BY_VALUE
#error
#else	
	obLuaGlobals.Register( "CreateEntityFromLua", CreateEntityFromLua );
#endif

	obLuaGlobals.RegisterRaw( "Send", Send );
	obLuaGlobals.RegisterRaw( "SendDelayed", SendDelayed );

	obLuaGlobals.Register( "DoesEntityExist", DoesEntityExist );
	
	// Animation binds
	obLuaGlobals.RegisterRaw("Anim_Play",					Anim_Play );
	obLuaGlobals.Register(	"Anim_StopAll",					Anim_StopAll );
	obLuaGlobals.RegisterRaw(			"Anim_MessageOnCompletion",		Anim_MessageOnCompletion );
	obLuaGlobals.Register(	"Anim_GetDuration",				Anim_GetDuration );
	obLuaGlobals.Register(	"Anim_IsAnimating",				Anim_IsAnimating );

	obLuaGlobals.Register( "GetEntityYAxis", GetEntityYAxis );
	obLuaGlobals.Register( "GetEntityZAxis", GetEntityZAxis );
	//obLuaGlobals.Register( "GetEntityInputDirection", GetEntityInputDirection );
	//obLuaGlobals.Register( "GetDirectionByEntityDiff", GetDirectionByEntityDiff );
	obLuaGlobals.Register( "GetEntityZAxisNamed", GetEntityZAxisNamed );
	obLuaGlobals.Register( "VectorByLocalMatrix", VectorByLocalMatrix );
	obLuaGlobals.Register( "VectorByWorldMatrix", VectorByWorldMatrix );

	obLuaGlobals.Register( "SetIdentity", SetIdentity );
	obLuaGlobals.RegisterRaw( "SetLocalTransform", SetLocalTransform );
	obLuaGlobals.RegisterRaw( "GetTransformLocalPosition", GetTransformLocalPosition );
	obLuaGlobals.RegisterRaw( "GetTransformLocalRotation", GetTransformLocalRotation );

	obLuaGlobals.Register( "DisableMovementComponent", DisableMovementComponent );
	obLuaGlobals.Register( "EnableMovementComponent", EnableMovementComponent );
	obLuaGlobals.Register( "DisableAnimatorComponent", DisableAnimatorComponent );
	obLuaGlobals.Register( "EnableAnimatorComponent", EnableAnimatorComponent );
	obLuaGlobals.Register( "Reparent", Reparent );
	obLuaGlobals.Register( "ReparentToRoot", ReparentToRoot );
	obLuaGlobals.Register( "ReparentToWorld", ReparentToWorld );
	obLuaGlobals.RegisterRaw( "SetParentEntity", SetParentEntity );

	obLuaGlobals.RegisterRaw( "Show", Show );
	obLuaGlobals.RegisterRaw( "Hide", Hide );
	obLuaGlobals.RegisterRaw( "SetMeshVisibility", SetMeshVisibility );

	// Components
	obLuaGlobals.Register( "CreateComponent_Hierarchy", CreateComponent_Hierarchy );
	obLuaGlobals.Register( "CreateComponent_MessageHandler", CreateComponent_MessageHandler );
	obLuaGlobals.Register( "CreateComponent_Animator", CreateComponent_Animator );
	obLuaGlobals.Register( "CreateComponent_Audio", CreateComponent_Audio );
	obLuaGlobals.Register( "CreateComponent_Dynamics", CreateComponent_Dynamics );
	obLuaGlobals.Register( "CreateComponent_Attack", CreateComponent_Attack );
	obLuaGlobals.Register( "CreateComponent_Aware", CreateComponent_Aware );
	obLuaGlobals.Register( "CreateComponent_Movement", CreateComponent_Movement );
	obLuaGlobals.Register( "CreateComponent_Input", CreateComponent_Input );
	obLuaGlobals.Register( "CreateComponent_SceneElement", CreateComponent_SceneElement);
	obLuaGlobals.Register( "CreateComponent_Hair", CreateComponent_Hair ); // hair
	obLuaGlobals.Register( "CreateComponent_BlendShapes", CreateComponent_BlendShapes );
	obLuaGlobals.Register( "CreateComponent_LookAt", CreateComponent_LookAt );

	obLuaGlobals.Register( "BlendShapes_SetBSClump", BlendShapes_SetBSClump );
	obLuaGlobals.Register( "BlendShapes_AddBSAnimsFromContainer", BlendShapes_AddBSAnimsFromContainer );
	obLuaGlobals.Register( "BlendShapes_RemoveBSAnimsFromContainer", BlendShapes_RemoveBSAnimsFromContainer );

	obLuaGlobals.RegisterRaw( "ConfigureRenderer", ConfigureRenderer );
	obLuaGlobals.Register( "UseDepthHaze", UseDepthHaze );

	// Object interaction stuff
	obLuaGlobals.Register( "IsDirectionHeld", IsDirectionHeld );

	obLuaGlobals.Register( "ResetAimingComponent", ResetAimingComponent );

	obLuaGlobals.Register( "NS_MatchEntity", NS_MatchEntity );

	obLuaGlobals.RegisterRaw( "UnpackMsg", UnpackMsg);
	obLuaGlobals.Register( "CreateRangedWeaponObject", CreateRangedWeaponObject );
}

