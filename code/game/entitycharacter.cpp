//--------------------------------------------------
//!
//!	\file game/entitycharacter.cpp
//!	Definition of the Character entity object
//!
//--------------------------------------------------

#include "game/entitycharacter.h"
#include "game/entity.inl"

#include "ai/aiformationmanager.h"
#include "game/messagehandler.h"
#include "game/movement.h"
#include "game/attacks.h"
#include "game/awareness.h"
#include "game/weapons.h"
#include "game/entityai.h"
#include "gfx/clump.h"
#include "anim/animator.h"
#include "Physics/system.h"
#include "Physics/advancedcharactercontroller.h"
#include "core/exportstruct_anim.h"
#include "core/visualdebugger.h"
#include "input/inputhardware.h"
#include "game/aicomponent.h"
#include "game/interactioncomponent.h"
#include "hair/haircollision.h"
#include "hair/forcefielditem.h"
#include "objectdatabase/dataobject.h"
#include "blendshapes/xpushapeblending.h"
#include "blendshapes/anim/eyeblinker.h"
#include "blendshapes/blendshapescomponent.h"

void ForceLinkFunction20()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction20() !ATTN!\n");
}


//------------------------------------------------------------------------------------------
//  CEntity - XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(Character, Mem::MC_ENTITY)
	COPY_INTERFACE_FROM(CEntity)
	DEFINE_INTERFACE_INHERITANCE(CEntity)

	PUBLISH_VAR_AS(m_obDescription, Description)
	PUBLISH_VAR_AS(m_obIgnoredInteractions, IgnoredInteractions)

	OVERRIDE_DEFAULT( ConstructionScript, "Character_Construct" )

	PUBLISH_VAR_WITH_DEFAULT_AS(m_RagdollClump, "default_ragdoll", RagdollClump)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obHeldPosition, CPoint(0.0f, 0.0f, 0.0f), HeldPosition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obHeldOrientation, CQuat(0.0f, 0.0f, 0.0f, 1.0f), HeldOrientation)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obHoldingPosition, CPoint(0.0f, 0.0f, 0.0f), HoldingPosition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obHoldingOrientation, CQuat(0.0f, 0.0f, 0.0f, 1.0f), HoldingOrientation)
	PUBLISH_VAR_AS(m_obBSClump, BSClump)
	PUBLISH_VAR_AS(m_obBSAnimContainer, BSAnimContainer)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAIControlled, false, AIControlled)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fCollisionHeight, 0.9f, CollisionHeight)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fCollisionRadius, 0.35f, CollisionRadius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iHealth, 100000, Health)
	PUBLISH_VAR_AS(m_obReactionMatrix, ReactionMatrix)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fLifeClockWorth, 0.0f, LifeClockWorth)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fAudioRadius, 0.0f, AudioRadius)


	// Component Definitions
	PUBLISH_VAR_WITH_DEFAULT_AS(m_sAnimationContainer, "HeroAnimContainer",		AnimationContainer)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_sAttackDefinition, "AttackDefinitionHero",			CombatDefinition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_sAwareDefinition,	"AttackTargetingDataHero",		AwarenessDefinition)
	PUBLISH_VAR_AS(m_sSceneElementDefinition,	SceneElementDef)
	PUBLISH_VAR_AS(m_sLookAtComponentDefinition, LookAtComponentDef)
	PUBLISH_VAR_AS(m_sFootstepDefinition, FootstepDef)
	PUBLISH_VAR_AS(m_sEyeBlinkerDefinition, EyeBlinker)

	// Construction Scripts
	PUBLISH_VAR_WITH_DEFAULT_AS(m_sHairConstruction, "HeroHair", HairConstruction)

	PUBLISH_PTR_AS(m_pWeaponsDef, WeaponConstruction);

	// Importance
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bImportant, false, Important)

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bCanUseCheapCC, false, CanUseCheapCC )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//  CEntity - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START_INHERITED(Character, CEntity)
	// Methods that were in CEntityInfo
	LUA_EXPOSED_METHOD_GET(isDead,				IsDead,					"Is this entity dead?")
	LUA_EXPOSED_METHOD(RemoveKeyword,			RemoveKeyword,			"Removed a mapped keyword", "string (remove)", "")
	LUA_EXPOSED_METHOD(AddKeyword,				AddKeyword,				"Add a keyword", "string (add)", "")
	LUA_EXPOSED_METHOD(ChangeHealth,			ChangeHealth,			"Change this entities health by a given delta", "float (delta), const char* (reason)", "")
	LUA_EXPOSED_METHOD(SetHealth,				SetHealthPerc,			"Set the health", "Amount of health as % of original, reason for increase", "")
	LUA_EXPOSED_METHOD(SetInvulnerable,			SetInvulnerable,		"Make the entity a super hero", "Super hero? true / false", "")
	LUA_EXPOSED_METHOD(IsInvulnerable,			IsInvulnerable,			"Is the entity a super hero", "", "")

	// Weapon Accessors
	LUA_EXPOSED_METHOD(GetLeftWeapon,			GetLeftWeapon,			"Get Characters Left Weapon", "", "")
	LUA_EXPOSED_METHOD(GetRightWeapon,			GetRightWeapon,			"Get Characters Right Weapon", "", "")
	LUA_EXPOSED_METHOD(GetRangedWeapon,			GetRangedWeapon,		"Get Characters Ranged Weapon", "", "")
	LUA_EXPOSED_METHOD(GetInteractionTarget,	GetInteractionTarget,	"Get Characters Interaction Target", "", "")
	LUA_EXPOSED_METHOD(SetInteractionTarget,	SetInteractionTargetP,	"Set Characters Interaction Target", "entity target", "new interaction target")
	LUA_EXPOSED_METHOD(DropRangedWeapon,		DropRangedWeapon,		"Drop this character's ranged-weapon", "", "")
	LUA_EXPOSED_METHOD(ShowWeapons,				ShowWeapons,			"Show this characters weapons", "", "")
	LUA_EXPOSED_METHOD(HideWeapons,				HideWeapons,			"Hide this characters weapons", "", "")
	LUA_EXPOSED_METHOD(SheathWeapons,			SheathWeapons,			"Sheath this characters weapons", "", "")
	LUA_EXPOSED_METHOD(DrawWeapons,				DrawWeapons,			"Draw this characters weapons", "", "")
LUA_EXPOSED_END(Character)


//------------------------------------------------------------------------------------------
//!
//!	Character::Character()
//!	Default constructor
//!
//------------------------------------------------------------------------------------------
Character::Character()
:	m_pobRagdollClumpHeader(0),
	m_pobRagdollHierarchy(0),
    m_pLeftWeapon(0),
	m_pRightWeapon(0),
	m_pRangedWeapon(0),
	m_pChainmanRenderable(0),
	m_bWeaponsHidden(false),
	m_bKillInNextFrame(0),
	m_pobCollisionSphereSet1(0),
	m_pobCollisionSphereSet2(0),	
	m_pobArtificialWind(0),
	m_pobCollisionSword(0),
	m_pobCollisionFloor(0),
	m_pobEyeBlinker(0),
	m_iDamageDisplayCycle(0),	
	m_bIsInvulnerable( false )
{
	m_obInteractionTarget.m_pobInteractingEnt = 0;

	m_eCharacterType = CT_Invalid;
	m_eType = EntType_Character;

	bIsInExternalControlState = false;

	ATTACH_LUA_INTERFACE(Character);
	// ---- Setup originally from CEntityInfo ----
}

//------------------------------------------------------------------------------------------
//!
//!	Character::~Character()
//!	Default destructor
//!
//------------------------------------------------------------------------------------------
Character::~Character()
{
	if ( m_pobRagdollHierarchy != NULL )
	{
		if ( m_pobRagdollHierarchy->GetRootTransform()->GetParent() != NULL )
		{
			m_pobRagdollHierarchy->GetRootTransform()->RemoveFromParent();
		}

		CHierarchy::Destroy( m_pobRagdollHierarchy );
		m_pobRagdollHierarchy = NULL;
	}

	if ( m_pobRagdollClumpHeader != NULL )
	{
		CClumpLoader::Get().UnloadClump( m_pobRagdollClumpHeader );
	}

	if ( m_pobEyeBlinker != NULL )
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobEyeBlinker);
	}
	
	// [scee_st] all the allocations below are only ever made in hair/chaincore.cpp
	// hence we assume MC_PROCEDURAL (although it is not used in the delete call yet).
	//NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pobCollisionSphereSet1);
	m_pobCollisionSphereSet1=0;
	//NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pobCollisionSphereSet2);
	m_pobCollisionSphereSet2=0;
	NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pobArtificialWind);
	m_pobArtificialWind=0;
	NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pobCollisionSword);
	m_pobCollisionSword=0;
	NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pobCollisionFloor);
	m_pobCollisionFloor=0;
}


//------------------------------------------------------------------------------------------
//!
//!	Character::OnPostConstruct
//!	Post construction
//!
//------------------------------------------------------------------------------------------
void Character::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	// Set the health from our table
	m_fLastHealth = m_fCurrHealth = m_fStartHealth = m_obAttributeTable->GetNumber( "Health" );
	
	// Only handle ragdoll on characters
	if (!m_RagdollClump.empty() && m_RagdollClump != "NULL")
	{
		// Patch up a platform path to the data based on the platform independent xml label.
		// Use the shieldman setup as a default.
#if defined( PLATFORM_PC )
		static const char *sDefaultRagdollClumpFilename = "data/shieldman.ragdoll.clump";
#elif defined( PLATFORM_PS3 )
		static const char *sDefaultRagdollClumpFilename = "data/shieldman.ragdoll.clump_ps3";
#endif
		const char *pcString 	= m_RagdollClump.c_str();
		const int iStringLength = strlen( pcString ) - 8;
		const char *pcSuffix	= &pcString[iStringLength];
		static char g_acFullPath[64];
		
		// Is this filename of the form '<name>_ragdoll'?
		if ( !strcmp( pcSuffix, "_ragdoll" ) )
		{
			// Check for any legacy 'default' ragdolls
			if ( !strncmp( pcString, "default", 7 ) )
			{
				m_RagdollClump = sDefaultRagdollClumpFilename;
			}
			
			// Else form up the filename based on the name in the string.
			else
			{
				char *pacTemp = NT_NEW char[ iStringLength+1 ];
				strncpy( pacTemp, pcString, iStringLength ); 
				pacTemp[iStringLength] = 0;			
#if defined( PLATFORM_PC )  
				sprintf( g_acFullPath, "data/%s.ragdoll.clump", pacTemp );
#elif defined( PLATFORM_PS3 )
				sprintf( g_acFullPath, "data/%s.ragdoll.clump_ps3", pacTemp );
#endif
				NT_DELETE(pacTemp);

				m_RagdollClump = ntstd::String(g_acFullPath);
			}
		}

		// Load the clump header from this filename. (Do this here, in case the full clump path was specified in the xml.)
		m_pobRagdollClumpHeader = CClumpLoader::Get().LoadClump_Neutral(m_RagdollClump.c_str(), true);
		
		// If this couldn't be found, check for versions using _ragdoll instead of .ragdoll.
		user_error_p(m_pobRagdollClumpHeader != NULL, ("Could not find ragdoll clump %s, trying underscore version.../n", m_RagdollClump.c_str()));

		if ( m_pobRagdollClumpHeader == NULL )
		{
			sprintf( g_acFullPath, "%s", m_RagdollClump.c_str() );
			char *pobInsert = strstr( g_acFullPath, ".ragdoll" );
			if ( pobInsert )
			{
				*pobInsert = '_';
				m_RagdollClump = ntstd::String(g_acFullPath);
				m_pobRagdollClumpHeader = CClumpLoader::Get().LoadClump_Neutral(ntStr::GetString(m_RagdollClump), true);
			}
		}
	}

	if (m_pobRagdollClumpHeader)
	{
		m_pobRagdollHierarchy = CHierarchy::Create(m_pobRagdollClumpHeader);
		GetRagdollHierarchy()->GetRootTransform()->SetLocalMatrix(CVecMath::GetIdentity());
	}

	InstallAnimator( CHashedString(m_sAnimationContainer) );
	InstallMessageHandler();
	InstallAudioChannel();

	GetEntityAudioChannel()->RegisterFootstepDefinition(m_sFootstepDefinition); // Register the footstep definition, if defined
	GetEntityAudioChannel()->SetAudioRadius(m_fAudioRadius);

	// Create the Attack Component
	CAttackDefinition* pAttackDef = ObjectDatabase::Get().GetPointerFromName< CAttackDefinition* >(m_sAttackDefinition);
	if(pAttackDef)
		SetAttackComponent(NT_NEW_CHUNK(Mem::MC_ENTITY) CAttackComponent(*this, pAttackDef));
	else
		ntPrintf( "Attack Definition %s not found in CreateComponent_Attack\n", ntStr::GetString(m_sAttackDefinition) );

	// And the Awareness Component
	CAttackTargetingData* pAwareDef = ObjectDatabase::Get().GetPointerFromName< CAttackTargetingData* >(m_sAwareDefinition);
	if(pAwareDef)
		SetAwarenessComponent(NT_NEW_CHUNK(Mem::MC_ENTITY) AwarenessComponent(this, pAwareDef));
	else
		ntPrintf( "Aware Definition %s not found for entity %s\n", ntStr::GetString(m_sAwareDefinition), ntStr::GetString(GetName()) );

	// And the Scene Element Component
	SceneElementComponentDef* pSceneDef = ObjectDatabase::Get().GetPointerFromName<SceneElementComponentDef*>(m_sSceneElementDefinition);
	if(pSceneDef)
		SetSceneElementComponent(NT_NEW_CHUNK(Mem::MC_ENTITY) SceneElementComponent(this, pSceneDef));

	// And the Dynamics Component
	ConstructDynamicsState();
	
	// And the Movement Component
	CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement( this, GetAnimator(),	GetPhysicsSystem() );
	SetMovement(pobMovement);

	// installs the look-at component (if any)
	InstallLookAtComponent( m_sLookAtComponentDefinition );


	// TODO: Tidy these up...
	// Setup the weapons using the specified setup function
	if(m_pWeaponsDef && (!IsAI() || !ToAI()->IsSpawned()))	// New Way - If this is a spawned AI then it's weapons will be created separately
	{
		m_pWeaponsDef->CreateWeapons(this);
	}

	// Setup the hair using the specified setup function
	if(m_sHairConstruction.size() > 0)
	{
		HelperGetLuaFunc(m_sHairConstruction.c_str())();
	}

	//CreateComponent_Dynamics


	// the blendshape stuff that got left behind
	if ( XPUShapeBlending::Get().IsEnabled() && !m_obBSClump.empty() && m_obBSClump != "NULL")
	{
		InstallBlendShapesComponent( m_obBSClump.c_str() );

//		if ( obPropertiesTable.BSAnimContainer ~= nil and obPropertiesTable.BSAnimContainer ~= "" and obPropertiesTable.BSAnimContainer ~= "NULL" ) then	
//			for loopContainer in string.gmatch( obPropertiesTable.BSAnimContainer, "%w+" ) do
//				BlendShapes_AddBSAnimsFromContainer( loopContainer )
//			end
//		end

		if (!m_obBSAnimContainer.empty() && GetBlendShapesComponent())
		{
			GetBlendShapesComponent()->AddBSAnimsFromContainer(CHashedString(m_obBSAnimContainer), false);
		}
	}

	EyeBlinkerDef* pBlinkerDef = ObjectDatabase::Get().GetPointerFromName<EyeBlinkerDef*>(m_sEyeBlinkerDefinition);
	if ( pBlinkerDef )
	{
		m_pobEyeBlinker = NT_NEW_CHUNK(Mem::MC_ENTITY) EyeBlinker( this, pBlinkerDef );
	}
}

//--------------------------------------------------
//!
//!	Character::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Character::OnLevelStart()
{
}

//------------------------------------------------------------------------------------------
//!
//!	Character::Show
//!	Show this character and weapons as necessary
//!
//------------------------------------------------------------------------------------------
void Character::Show()
{
	CEntity::Show();

	if(m_bWeaponsHidden)
	{
		HideWeapons();
	}
}



//------------------------------------------------------------------------------------------
//!
//!	Character::ConstructDynamicsState
//!	Helper method to set up dynamics for a character
//!
//------------------------------------------------------------------------------------------
void Character::ConstructDynamicsState()
{
	// Install the dynamics component
	InstallDynamics();

	// Go through the volumes on the clump and find one with a character volume reference
	// TEMP MAKE A DEBUG SHAPE FOR THE CHARACTER VOLUME
	CColprimDesc obDebugVolume;
	//obDebugVolume.m_eType = CV_TYPE_OBB;
	obDebugVolume.m_eType = CV_TYPE_CAPSULE;

#ifdef PLATFORM_PS3
	// XXX: Not sure if this is the right thing to do. Problem is that m_pcType is an ntDiskPointer< const char > on PS3
	//		so you can't actually just assign "" to it. Buggerations.
	//obDebugVolume.m_pcType = "";
	const char *pcType = static_cast< const char * >( obDebugVolume.m_pcType );
	pcType = "";
	// XXX END
#else
	obDebugVolume.m_pcType = "";
#endif // PLATFORM_PS3


	obDebugVolume.m_iTransform = ROOT_TRANSFORM;
	obDebugVolume.m_obRotation.SetIdentity();
	obDebugVolume.m_obTranslation.Clear();

	// Right - i am not sure about these collision shapes - i am using an ntError of 0.09 to 
	// make things look correct but i have no idea where this ntError comes from - GH
	obDebugVolume.m_obTranslation.Y() = ( ( m_fCollisionHeight + ( m_fCollisionRadius * 2.0f ) ) / 2.0f ) + 0.09f;

	obDebugVolume.m_obCapsuleData.fLength = m_fCollisionHeight;
	obDebugVolume.m_obCapsuleData.fRadius = m_fCollisionRadius;

	Physics::AdvancedCharacterController* lg = NT_NEW Physics::AdvancedCharacterController(this, &obDebugVolume, m_RagdollClump);

	if(GetPhysicsSystem() == 0)
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System(this, GetName());
		SetPhysicsSystem( system );
	}

	GetPhysicsSystem()->AddGroup(lg);

	GetPhysicsSystem()->Lua_ActivateState("CharacterState");

}


//------------------------------------------------------------------------------------------
//!	Character::RemoveWeapon
//!	Remove a specied weapon
//------------------------------------------------------------------------------------------
void Character::RemoveWeapon(CEntity* pWeapon, bool bPush)
{
	if(!pWeapon)
		return;

	pWeapon->SetParentEntity( 0 );

	Message msgDetatch(msg_detach);	
	msgDetatch.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)this);
	pWeapon->GetMessageHandler()->QueueMessage(msgDetatch);

	if(bPush)	// FIX ME always just player or character too?
		pWeapon->GetPhysicsSystem()->Lua_Rigid_PushFromPlayer();  
}

//------------------------------------------------------------------------------------------
//!
//!	Character::DropWeapons
//!	Drop this characters weapons
//!
//------------------------------------------------------------------------------------------
void Character::DropWeapons()
{
#if defined( PLATFORM_PS3 )
	if(IsAI() && ToAI()->GetArmyRenderable() )
	{
		return;
	}
#endif

	RemoveWeapon(m_pLeftWeapon,		true);
	m_pLeftWeapon = 0;

	RemoveWeapon(m_pRightWeapon,	true);
	m_pRightWeapon = 0;

	RemoveWeapon(m_pRangedWeapon,	true);
	m_pRangedWeapon = 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Character::DropRangedWeapon
//!	Drop this characters ranged weapon specifically.
//!
//------------------------------------------------------------------------------------------
void Character::DropRangedWeapon()
{
#if defined( PLATFORM_PS3 )
	if(IsAI() && ToAI()->GetArmyRenderable())
	{
		return;
	}
#endif
	//If the thing we're interacting with is our ranged-weapon (it's not holstered or anything) then clear
	//our interaction target upon dropping the weapon.
	if(GetInteractionTarget() == m_pRangedWeapon)
	{
		SetInteractionTarget(NULL);
	}

	RemoveWeapon(m_pRangedWeapon, true);
	m_pRangedWeapon = 0;
}


//----------------------------------------------------------------------------------------------------
//!
//!	Character::HideWeapons
//!	Hide this characters weapons
//!
//----------------------------------------------------------------------------------------------------
void Character::HideWeapons()
{
	if(m_pLeftWeapon)
	{
		m_pLeftWeapon->Hide();	
	}

	if(m_pRightWeapon)
	{
		m_pRightWeapon->Hide();	
	}

	if(m_pRangedWeapon)
	{
		m_pRangedWeapon->Hide();	
	}

	m_bWeaponsHidden = true;
}



//----------------------------------------------------------------------------------------------------
//!
//!	Character::ShowWeapons
//!	Show this characters weapons
//!
//----------------------------------------------------------------------------------------------------
void Character::ShowWeapons()
{
	if(m_pLeftWeapon)
	{
		m_pLeftWeapon->Show();	
	}

	if(m_pRightWeapon)
	{
		m_pRightWeapon->Show();	
	}

	if(m_pRangedWeapon)
	{
		m_pRangedWeapon->Show();	
	}

	m_bWeaponsHidden = false;
}


//----------------------------------------------------------------------------------------------------
//!
//!	Character::SheathWeapons
//!	Sheath this characters weapons
//!
//----------------------------------------------------------------------------------------------------
void Character::SheathWeapons()
{
	if(!m_pWeaponsDef || !m_pWeaponsDef->IsBasicWeaponSet())
		return;

	BasicWeaponSetDef* pWeaponDef   = (BasicWeaponSetDef*)m_pWeaponsDef;
	WeaponDef*         pLeftWeapon  = pWeaponDef->m_pLeftWeapon;
	WeaponDef*         pRightWeapon = pWeaponDef->m_pRightWeapon;
	
	if(m_pLeftWeapon && pLeftWeapon && pLeftWeapon->m_sSheathTransform.compare(""))
	{
		m_pLeftWeapon->Lua_Reparent(this, CHashedString(pLeftWeapon->m_sSheathTransform));
		m_pLeftWeapon->Lua_SetLocalTransform(pLeftWeapon->m_ptSheathedPosition.X(), 
											 pLeftWeapon->m_ptSheathedPosition.Y(), 
											 pLeftWeapon->m_ptSheathedPosition.Z(),
											 pLeftWeapon->m_ptSheathedYPR.X(), 
											 pLeftWeapon->m_ptSheathedYPR.Y(), 
											 pLeftWeapon->m_ptSheathedYPR.Z());
	}

	if(m_pRightWeapon && pRightWeapon && pRightWeapon->m_sSheathTransform.compare(""))
	{
		m_pRightWeapon->Lua_Reparent(this, CHashedString(pRightWeapon->m_sSheathTransform));
		m_pRightWeapon->Lua_SetLocalTransform(pRightWeapon->m_ptSheathedPosition.X(), 
											  pRightWeapon->m_ptSheathedPosition.Y(), 
											  pRightWeapon->m_ptSheathedPosition.Z(),
											  pRightWeapon->m_ptSheathedYPR.X(), 
											  pRightWeapon->m_ptSheathedYPR.Y(), 
											  pRightWeapon->m_ptSheathedYPR.Z());
	}
}


//----------------------------------------------------------------------------------------------------
//!
//!	Character::DrawWeapons
//!	Draw this characters weapons
//!
//----------------------------------------------------------------------------------------------------
void Character::DrawWeapons()
{
	if(!m_pWeaponsDef || !m_pWeaponsDef->IsBasicWeaponSet())
		return;

	BasicWeaponSetDef* pWeaponDef   = (BasicWeaponSetDef*)m_pWeaponsDef;
	WeaponDef*         pLeftWeapon  = pWeaponDef->m_pLeftWeapon;
	WeaponDef*         pRightWeapon = pWeaponDef->m_pRightWeapon;
	
	if(m_pLeftWeapon && pLeftWeapon)
	{
		m_pLeftWeapon->Lua_Reparent(this, CHashedString(pLeftWeapon->m_sParentTransform));
		m_pLeftWeapon->Lua_SetLocalTransform(pLeftWeapon->m_ptPosition.X(), 
											 pLeftWeapon->m_ptPosition.Y(), 
											 pLeftWeapon->m_ptPosition.Z(),
											 pLeftWeapon->m_ptYPR.X(), 
											 pLeftWeapon->m_ptYPR.Y(), 
											 pLeftWeapon->m_ptYPR.Z());
	}

	if(m_pRightWeapon && pRightWeapon)
	{
		m_pRightWeapon->Lua_Reparent(this, CHashedString(pRightWeapon->m_sParentTransform));
		m_pRightWeapon->Lua_SetLocalTransform(pRightWeapon->m_ptPosition.X(), 
											  pRightWeapon->m_ptPosition.Y(), 
											  pRightWeapon->m_ptPosition.Z(),
											  pRightWeapon->m_ptYPR.X(), 
											  pRightWeapon->m_ptYPR.Y(), 
											  pRightWeapon->m_ptYPR.Z());
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Character::HelperCallLuaFunc
//!	Helper function to call a LUA function
//!
//------------------------------------------------------------------------------------------
NinjaLua::LuaFunction Character::HelperGetLuaFunc(CHashedString obFunctionName)
{
	return CLuaGlobal::GetLuaFunc(obFunctionName, this); 
}

//------------------------------------------------------------------------------------------
//!
//!	Character::HelperCallLuaFunc()
//!	Helper function to call a LUA function
//!
//------------------------------------------------------------------------------------------
NinjaLua::LuaFunction Character::HelperGetLuaFunc(CHashedString obTableName, CHashedString obFunctionName)
{
	return CLuaGlobal::GetLuaFunc(obTableName, obFunctionName, this); 
}


/***************************************************************************************************
*
*	FUNCTION		Character::DebugDisplayHealthHistory
*
*	DESCRIPTION		If requested draw the health history of this entity
*
***************************************************************************************************/
#ifndef _RELEASE
// Do we want to display the damage history
static bool gbDamageDisplay = false;

void Character::DebugDisplayHealthHistory( float fTimeStep )
{
	// Do we want to display a health bar
	static bool	bDebugDisplay = false;

	// Used for de-bouncing the debug button presses
	static u_int uiLastFrame = 0;

	// We only want to show any of this if the current context is game
	if ( CInputHardware::Get().GetContext() != INPUT_CONTEXT_GAME )
		return;

	// If we didn't try to change the settings last frame...
	if ( uiLastFrame != CTimer::Get().GetSystemTicks() )
	{
		uiLastFrame = CTimer::Get().GetSystemTicks();

		// Turn the health bar on/off
		if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_H ) )
			bDebugDisplay = !bDebugDisplay;

		// Turn the damage display on/off
		if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_KPAD_ENTER ) )
			gbDamageDisplay = !gbDamageDisplay;
	}

	// If we want to displpay the health bar...
	if ( bDebugDisplay )
	{
		float fYOffset = 1.0f;

		CPoint obPos = GetPosition() + CPoint( 0.0f, fYOffset, 0.0f );

		if ( IsDead() )
			g_VisualDebug->Printf3D( obPos, DC_RED, 0, "%s ( DECEASED )", GetName().c_str() );
		else
		{
			g_VisualDebug->Printf3D( obPos, DC_WHITE, 0, "%s (%.2f)", GetName().c_str(), GetCurrHealth());

			CPoint obBarBase = GetPosition();
			CPoint obLastTop = GetPosition() + CPoint(0.0f, fYOffset * (m_fLastHealth / GetStartHealth()), 0.0f);
			CPoint obCurrTop = GetPosition() + CPoint(0.0f, fYOffset * (GetCurrHealth() / GetStartHealth()), 0.0f);

			g_VisualDebug->Printf3D( obPos, DC_WHITE, 0, "%s (%.2f)", GetName().c_str(), GetCurrHealth());

			g_VisualDebug->WorldToScreen( obBarBase, obBarBase );
			g_VisualDebug->WorldToScreen( obLastTop, obLastTop );
			g_VisualDebug->WorldToScreen( obCurrTop, obCurrTop );

			obBarBase.Z() = obLastTop.Z() = obCurrTop.Z() = 0.0f;
			obBarBase.X() = obLastTop.X() = obCurrTop.X() = obPos.X() - 5.0f;

			g_VisualDebug->RenderLine( obBarBase, obLastTop, DC_RED, DPF_VIEWPORTSPACE );
			g_VisualDebug->RenderLine( obBarBase, obCurrTop, DC_GREEN, DPF_VIEWPORTSPACE );
		}
	}

	// If we want to display the damage history...
	if ( gbDamageDisplay )
	{
		for(  int iC = 0; iC < 3; iC++ )
		{
			if ( m_fDamageDisplayTimer[iC] <= 0.0f )
				continue;

			float fYOffset = 4.0f - m_fDamageDisplayTimer[iC];
			m_fDamageDisplayTimer[iC] -= fTimeStep;

			CPoint obPos = GetPosition() + CPoint(0.0f, fYOffset, 0.0f);
			
			if( IsPlayer() )
				g_VisualDebug->Printf3D( obPos, DC_RED, 0, "%.2f", m_fDamage[iC]);
			else
				g_VisualDebug->Printf3D( obPos, DC_WHITE, 0, "%.2f", m_fDamage[iC]);
		}
	}
}
#endif // _RELEASE

/***************************************************************************************************
*
*	FUNCTION		Character::DebugUpdateHealthHistory
*
*	DESCRIPTION		Store the history of this entity's health
*
***************************************************************************************************/
#ifndef _RELEASE
void Character::DebugUpdateHealthHistory( float fHealthChange, const char* pcReason )
{
	m_fDamage[m_iDamageDisplayCycle] = fHealthChange; 
	m_apcDamageReason[m_iDamageDisplayCycle] = pcReason; 
	m_fDamageDisplayTimer[m_iDamageDisplayCycle] = 4.0f; 
	m_iDamageDisplayCycle = ( m_iDamageDisplayCycle + 1 ) % 3;

	if( gbDamageDisplay )
	{
		ntPrintf("Health Change: %s, %f, (%s)\n", GetName().c_str(), fHealthChange, pcReason );
	}
}
#endif

/***************************************************************************************************
*
*	FUNCTION		Character::SetDead
*
*	DESCRIPTION		
*
***************************************************************************************************/
void Character::SetDead( bool bDead ) 
{
	if ( bDead )
	{
		// Super hero mode?
		if( m_bIsInvulnerable )
			return;

		// Shouldn't we actually call Kill() here?! - JML
		m_fCurrHealth = 0.0f;
		m_fLastHealth = 0.0f;

	}
	else
	{
		SetDeadMessageSent(false);
		m_fLastHealth = GetStartHealth();
		m_fCurrHealth = GetStartHealth();
	}
}

/***************************************************************************************************
*
*	FUNCTION		Character::SetHealth
*
*	DESCRIPTION		
*
***************************************************************************************************/
void Character::SetHealth( float fHealth, const char* pcReason )
{ 
	// Super hero mode?
	if( m_bIsInvulnerable )
		return;

	UNUSED( pcReason );

	m_fLastHealth = m_fCurrHealth; 
	m_fCurrHealth = fHealth; 
 
	if(fHealth != 0.0f && IsAI())
	{
		((AI*)this)->GetAIComponent()->HealthChanged(m_fCurrHealth, m_fStartHealth);
	}

#ifndef _RELEASE
	DebugUpdateHealthHistory( fHealth, pcReason );
#endif // _RELEASE
}

/***************************************************************************************************
*
*	FUNCTION		Character::SetHealthPerc
*
*	DESCRIPTION		Health util called from script where the param is passed as a percentage 
*					of the original health of the ent.
*
***************************************************************************************************/
void Character::SetHealthPerc( float fHealthPercent )
{
	m_fLastHealth = m_fCurrHealth; 
	m_fCurrHealth = (m_fStartHealth * fHealthPercent); 
 
	if( m_fCurrHealth != 0.0f && IsAI() )
		((AI*)this)->GetAIComponent()->HealthChanged( m_fCurrHealth, m_fStartHealth );
}

/***************************************************************************************************
*
*	FUNCTION		CEntityInfo::ChangeHealth
*
*	DESCRIPTION		
*
***************************************************************************************************/
void Character::ChangeHealth( float fDelta, const char* pcReason )
{ 
	// Super hero mode?
	if( m_bIsInvulnerable )
		return;

	UNUSED( pcReason );

	m_fLastHealth = m_fCurrHealth; 
	m_fCurrHealth += fDelta; 

	// 
	if( fDelta != 0.0f && IsAI() )
		((AI*)this)->GetAIComponent()->HealthChanged( m_fCurrHealth, m_fStartHealth );

#ifndef _RELEASE
	DebugUpdateHealthHistory( fDelta, pcReason );
#endif // _RELEASE
}

/***************************************************************************************************
*
*	FUNCTION		Character::IsDead
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool	Character::IsDead( void ) const
{ 
	return m_fCurrHealth <= 0.0f; 
}

/***************************************************************************************************
*
*	FUNCTION		Character::Kill
*
*	DESCRIPTION		
*
***************************************************************************************************/
void Character::Kill() 
{
	if(!IsDeadMessageSent())
	{
		m_pobMessageHandler->ReceiveMsg<msg_combat_killed>();

		// Find a suitable commander to inform about the entities removal from the living world
		const CEntity* pSuitableCommander = AIFormationManager::Get().GetCommander( this );

		if( pSuitableCommander && pSuitableCommander->GetMessageHandler() )
		{
			Message obMessage(msg_combat_killed);
			obMessage.SetEnt( "Sender", this );
			pSuitableCommander->GetMessageHandler()->Receive(obMessage);
		}

		SetDeadMessageSent(true);
	}
	m_fCurrHealth = 0.0f;
};

/***************************************************************************************************
*
*	FUNCTION		Character::KillEnt
*
*	DESCRIPTION		
*
***************************************************************************************************/
void Character::KillEnt(CEntity* pEnt)
{
	// Downcast to Character class
	Character* pobChar = pEnt->ToCharacter();
	// Check that it is actually a Character
	ntError_p(pobChar, ("KillEnt called on non Character"));
	pobChar->Kill();
}
 
void	Character::UpdateDerivedComponents(float fTimeStep)
{
	if (m_pobCollisionSphereSet1)
		m_pobCollisionSphereSet1->Update(fTimeStep);
	if (m_pobCollisionSphereSet2)
		m_pobCollisionSphereSet2->Update(fTimeStep);
	if (m_pobArtificialWind)
		m_pobArtificialWind->Update(fTimeStep);
	if (m_pobCollisionSword)
		m_pobCollisionSword->Update(fTimeStep);
	if (m_pobCollisionFloor)
		m_pobCollisionFloor->Update(fTimeStep);
	if (m_pobEyeBlinker)
		m_pobEyeBlinker->Update(fTimeStep);
}

//	scee.sbashow : Current simple routine which starts the nav anim off for navigating to a use point.
//
bool Character::SetOffNavigationToUsePoint(CharacterType eType, bool bWantsRun)
{
	// now deal with target...assumes a use point!
	const CUsePoint* const pobUsePnt = GetInteractionTargetUsePoint();
	CPoint obPosLS	= pobUsePnt->GetLocalPosition() + CDirection(pobUsePnt->GetMoveToOffset());
	
	const CHashedString obUseAnim(pobUsePnt->GetUseAnim(eType,bWantsRun? CUsePoint::UP_Run : CUsePoint::UP_Walk));


	if (obUseAnim==CHashedString(HASH_STRING_NULL))
	{
		// shouldn't get this - should assert, but given the transitional nature of this new move-to stuff,
		// safer not to for the moment.
		return false;
	}
	
	// Disable collision between object and user
	GetInteractionTarget()->GetInteractionComponent()->ExcludeCollisionWith( this );
	
	if (!pobUsePnt->HasUseAngle() || !pobUsePnt->HasFacingRequirements())
	{
		CMatrix obMatrix =GetInteractionTarget()->GetMatrix();	
		obMatrix.SetTranslation(CPoint(0.0f,0.0f,0.0f));
		CPoint obOffsetPosWS = obPosLS*obMatrix;
		
		// the offset for this transition is from the centre of the UP, but in WS coordinates!
		GetMovement()->Lua_AltStartMoveToTransition( obUseAnim, 
															GetInteractionTarget(), 
															0.0f, 
															1.0f, 
															&obOffsetPosWS);
	}
	else
	{
		// the hero must face opposite the use point normal.
		CDirection obDirFacingNormalLS =-CDirection(pobUsePnt->GetLocalUseFacing());
	
		GetMovement()->Lua_StartFacingMoveToTransition( obUseAnim, 
															GetInteractionTarget(), 
															0.0f, 
															1.0f, 
															&obPosLS, 
															0,
															&obDirFacingNormalLS );
	
	}
	

	return true;
}


//------------------------------------------------------------------------------------------
//!  public virtual  CanRemoveFromWorld
//!
//!  @return bool <TODO: insert return value description here>
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 19/09/2006
//------------------------------------------------------------------------------------------
bool Character::CanRemoveFromWorld(void)
{
	// Only allow the entity to be removed from the world
	if ( GetAttackComponent() )
	{
		COMBAT_STATE eCombatState = GetAttackComponent()->AI_Access_GetState();

		// Extra safe about the state of the combat system
		if( !(eCombatState == CS_DEAD || eCombatState == CS_STANDARD)  ||
			GetAttackComponent()->AI_Access_GetStrikeStackSize() || GetAttackComponent()->IsInSuperStyleSafetyTransition() )
		{
			return false;
		}
	}

	//if ragdoll is simulated entities cannot be removed from world
	Physics::AdvancedCharacterController* pobCC = (Physics::AdvancedCharacterController*)GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
	if (pobCC)
	{		
		Physics::AdvancedRagdoll * rag = pobCC->GetAdvancedRagdoll();
		if (rag && rag->GetState() != Physics::DEACTIVATED && rag->IsMoving())					
		{
			return false;
		}
	}

	return CEntity::CanRemoveFromWorld();
}


//------------------------------------------------------------------------------------------
//!  public  RemoveFromWorld
//!
//!  @param [in]		bCallerIsEntityBase bool  [=false]    
//!
//!  @return bool 
//!
//!  @remarks			Remove the entity from the world
//!
//!  @author GavB @date 19/09/2006
//------------------------------------------------------------------------------------------
bool Character::RemoveFromWorld( bool bSafeRemove )
{
	return CEntity::RemoveFromWorld( bSafeRemove );
}



