//!---------------------------------------------------------------------------------------------
//!
//!	\file game/weapons.cpp
//!	
//!
//!---------------------------------------------------------------------------------------------

//!---------------------------------------------------------------------------------------------
//! Includes
//!---------------------------------------------------------------------------------------------
#include "weapons.h"

#include "game/entity.inl"
#include "game/entityhero.h"
#include "game/entityarcher.h"
#include "game/entityspawnpoint.h"
#include "game/renderablecomponent.h"
#include "game/entityinteractablethrown.h"
#include "game/entityrangedweapon.h"
#include "game/messagehandler.h"
#include "effect/chainmanchains.h"
#include "effect/rangestancechain.h"
#include "camera/camutils.h"
#include "physics/system.h"
#include "luaattrtable.h"
#include "objectdatabase/dataobject.h"


//!---------------------------------------------------------------------------------------------
//! XML Interfaces
//!---------------------------------------------------------------------------------------------

// Basic Weapon Set for Basic Enemies, Basic Hero and Archer
START_CHUNKED_INTERFACE(BasicWeaponSetDef, Mem::MC_ENTITY)
	PUBLISH_PTR_AS(m_pLeftWeapon,   LeftWeapon)
	PUBLISH_PTR_AS(m_pRightWeapon,  RightWeapon)
	PUBLISH_PTR_AS(m_pRangedWeapon, RangedWeapon)
END_STD_INTERFACE

// Hero Weapon Set for the Heavenly Hero
START_CHUNKED_INTERFACE(HeroWeaponSetDef, Mem::MC_ENTITY)
	PUBLISH_PTR_AS(m_pTechniqueLeft,  TechniqueLeft)
	PUBLISH_PTR_AS(m_pTechniqueRight, TechniqueRight)
	PUBLISH_PTR_AS(m_pPower,          Power)
	PUBLISH_PTR_AS(m_pRangeLeft,      RangeLeft)
	PUBLISH_PTR_AS(m_pRangeRight,     RangeRight)
	PUBLISH_PTR_AS(m_pBasic,          Basic)
END_STD_INTERFACE

// Thrown Weapon e.g. Enemy Swords, Shields, Axes...
START_CHUNKED_INTERFACE(ThrownWeaponDef, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(m_sClump,           Clump)
	PUBLISH_VAR_AS(m_sParentTransform, ParentTransform)
	PUBLISH_VAR_AS(m_ptPosition,       HeldPosition)
	PUBLISH_VAR_AS(m_ptYPR,            HeldYPR)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_sSheathTransform, "",	 SheathTransform)
	PUBLISH_VAR_AS(m_ptSheathedPosition, SheathedPosition)
	PUBLISH_VAR_AS(m_ptSheathedYPR,      SheathedYPR)

	PUBLISH_VAR_AS(m_sSharedAttributes, SharedAttributes)
END_STD_INTERFACE

// Ranged Weapon i.e. Enemy Crossbow and Bazooka
START_CHUNKED_INTERFACE(RangedWeaponDef, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(m_sClump,           Clump)
	PUBLISH_VAR_AS(m_sParentTransform, ParentTransform)
	PUBLISH_VAR_AS(m_ptPosition,       HeldPosition)
	PUBLISH_VAR_AS(m_ptYPR,            HeldYPR)

	PUBLISH_VAR_AS(m_sSharedAttributes, SharedAttributes)
END_STD_INTERFACE

// Standard Hero Weapon i.e. Technique and Power Swords
START_CHUNKED_INTERFACE(HeroStandardWeaponDef, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(m_sClump,           Clump)
	PUBLISH_VAR_AS(m_sParentTransform, ParentTransform)
	PUBLISH_VAR_AS(m_ptPosition,       HeldPosition)
	PUBLISH_VAR_AS(m_ptYPR,            HeldYPR)

	PUBLISH_VAR_AS(m_sClass,             Class)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_sSheathTransform, "",	 SheathTransform)
	PUBLISH_VAR_AS(m_ptSheathedPosition, SheathedPosition)
	PUBLISH_VAR_AS(m_ptSheathedYPR,      SheathedYPR)
END_STD_INTERFACE

// Ranged Hero Weapon
START_CHUNKED_INTERFACE(HeroRangedWeaponDef, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(m_sClump,           Clump)
	PUBLISH_VAR_AS(m_sParentTransform, ParentTransform)
	PUBLISH_VAR_AS(m_ptPosition,       HeldPosition)
	PUBLISH_VAR_AS(m_ptYPR,            HeldYPR)

	PUBLISH_VAR_AS(m_sClass,           Class)
	PUBLISH_VAR_AS(m_sHandleClump,     HandleClump)
END_STD_INTERFACE

// Archer Crossbow
START_CHUNKED_INTERFACE(ArcherWeaponDef, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(m_sClump,           Clump)
	PUBLISH_VAR_AS(m_sParentTransform, ParentTransform)
	PUBLISH_VAR_AS(m_ptPosition,       HeldPosition)
	PUBLISH_VAR_AS(m_ptYPR,            HeldYPR)

	PUBLISH_VAR_AS(m_sAnimContainer,    AnimContainer)
	PUBLISH_VAR_AS(m_sSharedAttributes, SharedAttributes)
END_STD_INTERFACE

//!---------------------------------------------------------------------------------------------
//!
//! BasicWeaponSetDef::CreateWeapons
//! Create a basic set of weapons left/right/ranged
//!
//!---------------------------------------------------------------------------------------------
ntstd::Vector<CEntity*> BasicWeaponSetDef::PreCreateWeapons(ntstd::String sName, const Spawnee_Def* pSpawnDef) const
{
	ntstd::Vector<CEntity*> retval;

	if(m_pRightWeapon)
	{
		ntstd::String sWeaponName = sName + "_r_weapon";
		CEntity* pWeapon = m_pRightWeapon->Create(sWeaponName, pSpawnDef);
		ntAssert(pWeapon);

		retval.push_back(pWeapon);
	}
	else
	{
		retval.push_back(0);
	}

	if(m_pLeftWeapon)
	{
		ntstd::String sWeaponName = sName + "_l_weapon";
		CEntity* pWeapon = m_pLeftWeapon->Create(sWeaponName, pSpawnDef);
		ntAssert(pWeapon);

		retval.push_back(pWeapon);
	}
	else
	{
		retval.push_back(0);
	}

	if(m_pRangedWeapon)
	{
		ntstd::String sWeaponName = sName + "_ranged_weapon";
		CEntity* pWeapon = m_pRangedWeapon->Create(sWeaponName, pSpawnDef);
		ntAssert(pWeapon);

		retval.push_back(pWeapon);
	}
	else
	{
		retval.push_back(0);
	}

	return retval;
}


//!---------------------------------------------------------------------------------------------
//!
//! BasicWeaponSetDef::CreateWeapons
//! Create a basic set of weapons left/right/ranged
//!
//!---------------------------------------------------------------------------------------------
bool BasicWeaponSetDef::CreateWeapons(Character* pCharacter) const
{
	if(m_pRightWeapon && !pCharacter->GetRightWeapon())
	{
		pCharacter->SetRightWeapon(m_pRightWeapon->Create(pCharacter, "_r_weapon"));
		ntError(pCharacter->GetRightWeapon());
	}

	if(m_pLeftWeapon && !pCharacter->GetLeftWeapon())
	{
		pCharacter->SetLeftWeapon(m_pLeftWeapon->Create(pCharacter, "_l_weapon"));
		ntError(pCharacter->GetLeftWeapon());
	}

	if(m_pRangedWeapon && !pCharacter->GetRangedWeapon())
	{
		pCharacter->SetRangedWeapon(m_pRangedWeapon->Create(pCharacter, "_ranged_weapon"));
		ntError(pCharacter->GetRangedWeapon());

		pCharacter->SetInteractionTarget(pCharacter->GetRangedWeapon());

		// Send object into their AI_EquipState
		Message msgEquip(msg_equip);
		msgEquip.SetEnt(CHashedString(HASH_STRING_OTHER),	pCharacter);
		pCharacter->GetRangedWeapon()->GetMessageHandler()->QueueMessage(msgEquip);
		
		// Send character straight into their interacting state
		Message msgInteract(msg_interact);
		msgInteract.SetEnt(CHashedString("target"), pCharacter->GetRangedWeapon());
		pCharacter->GetMessageHandler()->QueueMessage(msgInteract);

		// Hide any melee weapons
		if(pCharacter->GetRightWeapon())
			pCharacter->GetRightWeapon()->Hide();

		if(pCharacter->GetLeftWeapon())
			pCharacter->GetLeftWeapon()->Hide();
	}

	// Created Successfully
	return true;
}


//!---------------------------------------------------------------------------------------------
//!
//! BasicWeaponSetDef::CreateWeapons
//! Create a basic set of weapons left/right/ranged - Version for Spawned Characters
//!
//!---------------------------------------------------------------------------------------------
bool BasicWeaponSetDef::CreateWeapons(Character* pCharacter, const ntstd::Vector<CEntity*>& vecWeapons) const
{
	if((m_pRightWeapon && !vecWeapons[0]) || (m_pLeftWeapon && !vecWeapons[1]) || (m_pRangedWeapon && !vecWeapons[2]))
		return false;

	if(m_pRightWeapon)
	{
		pCharacter->SetRightWeapon(m_pRightWeapon->Create(pCharacter, vecWeapons[0]));
		ntError(pCharacter->GetRightWeapon());
	}

	if(m_pLeftWeapon)
	{
		pCharacter->SetLeftWeapon(m_pLeftWeapon->Create(pCharacter, vecWeapons[1]));
		ntError(pCharacter->GetLeftWeapon());
	}

	if(m_pRangedWeapon)
	{
		pCharacter->SetRangedWeapon(m_pRangedWeapon->Create(pCharacter, vecWeapons[2]));
		ntError(pCharacter->GetRangedWeapon());

		pCharacter->SetInteractionTarget(pCharacter->GetRangedWeapon());

		// Send object into their AI_EquipState
		Message msgEquip(msg_equip);
		msgEquip.SetEnt(CHashedString(HASH_STRING_OTHER),	pCharacter);
		pCharacter->GetRangedWeapon()->GetMessageHandler()->QueueMessage(msgEquip);
		
		// Send character straight into their interacting state
		Message msgInteract(msg_interact);
		msgInteract.SetEnt(CHashedString("target"), pCharacter->GetRangedWeapon());
		pCharacter->GetMessageHandler()->QueueMessage(msgInteract);

		// Hide any melee weapons
		if(pCharacter->GetRightWeapon())
			pCharacter->GetRightWeapon()->GetRenderableComponent()->AddRemoveAll_Game(false);

		if(pCharacter->GetLeftWeapon())
			pCharacter->GetLeftWeapon()->GetRenderableComponent()->AddRemoveAll_Game(false);

		// Activate the Weapon (Physics)
		pCharacter->GetRangedWeapon()->GetPhysicsSystem()->Activate();
	}

	// Created Successfully
	return true;
}


//!---------------------------------------------------------------------------------------------
//!
//! HeroWeaponSetDef::CreateWeapons
//! Create a basic set of weapons left/right/ranged
//!
//!---------------------------------------------------------------------------------------------
bool HeroWeaponSetDef::CreateWeapons(Character* pCharacter) const
{
	Hero* pHero = (Hero*)pCharacter;

	// Technique
	if(m_pTechniqueLeft)
	{
		pHero->m_pLeftWeapon = m_pTechniqueLeft->Create(pCharacter, "_l_sword");
		ntAssert(pHero->m_pLeftWeapon);

		// Set up the sheathed parameters
		pHero->m_sLeftSheathedTransform = m_pTechniqueLeft->GetSheathTransform();
		pHero->m_ptLeftSheathedPosition = m_pTechniqueLeft->GetSheathedPosition();
		pHero->m_ptLeftSheathedYPR      = m_pTechniqueLeft->GetSheathedYPR();
	}

	if(m_pTechniqueRight)
	{
		pHero->m_pRightWeapon = m_pTechniqueRight->Create(pCharacter, "_r_sword");
		ntAssert(pHero->m_pRightWeapon);

		// Set up the sheathed parameters
		pHero->m_sRightSheathedTransform = m_pTechniqueRight->GetSheathTransform();
		pHero->m_ptRightSheathedPosition = m_pTechniqueRight->GetSheathedPosition();
		pHero->m_ptRightSheathedYPR      = m_pTechniqueRight->GetSheathedYPR();
	}

	if(m_pPower)
	{
		pHero->m_pBigSword = m_pPower->Create(pCharacter, "_big_sword");
		ntAssert(pHero->m_pBigSword);
	}

	if(m_pRangeLeft)
	{
		pHero->m_pRanged_LHandle = m_pRangeLeft->Create(pCharacter, "_l_handle", true);
		ntAssert(pHero->m_pRanged_LHandle);

		pHero->m_pRanged_LBlade = m_pRangeLeft->Create(pCharacter, "_l_sword_blade", false);
		ntAssert(pHero->m_pRanged_LBlade);

		Create_SwordChainEffect(pHero->m_pRanged_LBlade, pHero, "l_pivot");
	}

	if(m_pRangeRight)
	{
		pHero->m_pRanged_RHandle = m_pRangeRight->Create(pCharacter, "_r_handle", true);
		ntAssert(pHero->m_pRanged_RHandle);

		pHero->m_pRanged_RBlade = m_pRangeRight->Create(pCharacter, "_r_sword_blade", false);
		ntAssert(pHero->m_pRanged_RBlade);

		Create_SwordChainEffect(pHero->m_pRanged_RBlade, pHero, "r_pivot");
	}

	if(m_pBasic)
	{
		pHero->m_pBasicSword = m_pBasic->Create(pCharacter, "_basic_sword");
		ntAssert(pHero->m_pBasicSword);

		// Set up the sheathed parameters
		pHero->m_sBasicSheathedTransform = m_pBasic->GetSheathTransform();
		pHero->m_ptBasicSheathedPosition = m_pBasic->GetSheathedPosition();
		pHero->m_ptBasicSheathedYPR      = m_pBasic->GetSheathedYPR();
	}

	// Created Successfully
	return true;
}


//!---------------------------------------------------------------------------------------------
//!
//! ThrownWeaponDef::Create
//! Create a Thrown Type Weapon
//!
//!---------------------------------------------------------------------------------------------
CEntity* ThrownWeaponDef::Create(ntstd::String sWeaponName, const Spawnee_Def* pSpawnDef)
{
	// Construct the Weapon
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Interactable_Thrown", sWeaponName.c_str(), GameGUID(), 0, true, false);
	CEntity* pWeapon = (CEntity*)pDO->GetBasePtr();

	// Set up Weapon Attribute Table
	LuaAttributeTable* pWeaponAttrs = LuaAttributeTable::Create();
	pWeapon->SetAttributeTable(pWeaponAttrs);
	pWeapon->GetAttributeTable()->SetDataObject(pDO);

	pWeaponAttrs->SetString("Name", sWeaponName.c_str());
	pWeaponAttrs->SetString("Clump", m_sClump.c_str());
	pWeaponAttrs->SetString("SharedAttributes", m_sSharedAttributes.c_str());
	pWeaponAttrs->SetBool("Attached", false);
	pWeaponAttrs->SetInteger("SectorBits", pSpawnDef->GetMappedAreaInfo());

	// Post Construct the Weapon
	ObjectDatabase::Get().DoPostLoadDefaults(pDO);

	// The weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! ThrownWeaponDef::Create
//! Create a Thrown Type Weapon
//!
//!---------------------------------------------------------------------------------------------
CEntity* ThrownWeaponDef::Create(Character* pCharacter, ntstd::String sAppend)
{
	ntError(pCharacter);

	// Wrap the character in a ninja lua object for use with attribute tables
	CLuaGlobal::Get().State().Push((CEntity*)pCharacter);
	NinjaLua::LuaObject obCharacter(-1, CLuaGlobal::Get().State(), false);

	// Generate a unique name for the weapon
	ntstd::String sWeaponName = pCharacter->GetName();
	sWeaponName.append(sAppend);

	// Construct the Weapon
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Interactable_Thrown", sWeaponName.c_str(), GameGUID(), 0, true, false);
	CEntity* pWeapon = (CEntity*)pDO->GetBasePtr();

	// Set up Weapon Attribute Table
	LuaAttributeTable* pWeaponAttrs = LuaAttributeTable::Create();
	pWeapon->SetAttributeTable(pWeaponAttrs);
	pWeapon->GetAttributeTable()->SetDataObject(pDO);

	pWeaponAttrs->SetString("Name", sWeaponName.c_str());
	pWeaponAttrs->SetString("Clump", m_sClump.c_str());
	pWeaponAttrs->SetString("SharedAttributes", m_sSharedAttributes.c_str());
	pWeaponAttrs->SetAttribute("ParentEntity", obCharacter);
	pWeaponAttrs->SetString("ParentTransform", m_sParentTransform.c_str());
	pWeaponAttrs->SetBool("Attached", true);
	pWeaponAttrs->SetInteger("SectorBits", pCharacter->GetMappedAreaInfo());

	// Post Construct the Weapon
	ObjectDatabase::Get().DoPostLoadDefaults(pDO);

	// Set the Weapons Position and Orientation
	ntAssert(pWeapon->GetHierarchy());
	CMatrix mat;
	CCamUtil::MatrixFromEuler_XYZ(mat, m_ptYPR.X()*DEG_TO_RAD_VALUE, m_ptYPR.Y()*DEG_TO_RAD_VALUE, m_ptYPR.Z()*DEG_TO_RAD_VALUE);
	mat.SetTranslation(m_ptPosition);
	pWeapon->GetHierarchy()->GetRootTransform()->SetLocalMatrix(mat);

	// The weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! HeroStandardWeaponDef::Create
//! Create a Hero Type Weapon
//!
//!---------------------------------------------------------------------------------------------
CEntity* HeroStandardWeaponDef::Create(Character* pCharacter, ntstd::String sAppend)
{
	ntError(pCharacter);

	// Wrap the character in a ninja lua object for use with attribute tables
	CLuaGlobal::Get().State().Push((CEntity*)pCharacter);
	NinjaLua::LuaObject obCharacter(-1, CLuaGlobal::Get().State(), false);

	// Generate a unique name for the weapon
	ntstd::String sWeaponName = pCharacter->GetName();
	sWeaponName.append(sAppend);

	// Construct the Weapon
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("CEntity", sWeaponName.c_str(), GameGUID(), 0, true, false);
	CEntity* pWeapon = (CEntity*)pDO->GetBasePtr();

	// Set up Weapon Attribute Table
	LuaAttributeTable* pWeaponAttrs = LuaAttributeTable::Create();
	pWeapon->SetAttributeTable(pWeaponAttrs);
	pWeapon->GetAttributeTable()->SetDataObject(pDO);

	pWeaponAttrs->SetString("Name", sWeaponName.c_str());
	pWeaponAttrs->SetString("Clump", m_sClump.c_str());
	pWeaponAttrs->SetString("WeaponClass", m_sClass.c_str());
	if(m_sClass.compare("basic"))
		pWeaponAttrs->SetString("ConstructionScript", "Weapon_Construct");
	pWeaponAttrs->SetInteger("SectorBits", pCharacter->GetMappedAreaInfo());

	// Post Construct the Weapon
	ObjectDatabase::Get().DoPostLoadDefaults(pDO);

	// Deactivate Physics to Start
	if(pWeapon->GetPhysicsSystem())
	{
		Physics::LogicGroup* lg = pWeapon->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
		if(lg)
			lg->Deactivate();
	}

	// The weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! HeroRangedWeaponDef::Create
//! Create a Hero Type Weapon
//!
//!---------------------------------------------------------------------------------------------
CEntity* HeroRangedWeaponDef::Create(Character* pCharacter, ntstd::String sAppend, bool bHandle)
{
	ntError(pCharacter);

	// Wrap the character in a ninja lua object for use with attribute tables
	CLuaGlobal::Get().State().Push((CEntity*)pCharacter);
	NinjaLua::LuaObject obCharacter(-1, CLuaGlobal::Get().State(), false);

	// Generate a unique name for the weapon
	ntstd::String sWeaponName = pCharacter->GetName();
	sWeaponName.append(sAppend);

	// Construct the Weapon
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("CEntity", sWeaponName.c_str(), GameGUID(), 0, true, false);
	CEntity* pWeapon = (CEntity*)pDO->GetBasePtr();

	// Set up Weapon Attribute Table
	LuaAttributeTable* pWeaponAttrs = LuaAttributeTable::Create();
	pWeapon->SetAttributeTable(pWeaponAttrs);
	pWeapon->GetAttributeTable()->SetDataObject(pDO);

	pWeaponAttrs->SetString("Name", sWeaponName.c_str());
	pWeaponAttrs->SetString("Clump", bHandle ? m_sHandleClump.c_str() : m_sClump.c_str());
	pWeaponAttrs->SetInteger("SectorBits", pCharacter->GetMappedAreaInfo());
	if(!bHandle)
	{
		pWeaponAttrs->SetString("WeaponClass", m_sClass.c_str());
		pWeaponAttrs->SetString("ConstructionScript", "Weapon_Construct");
	}

	// Post Construct the Weapon
	ObjectDatabase::Get().DoPostLoadDefaults(pDO);

	// Deactivate Physics to Start
	if(pWeapon->GetPhysicsSystem())
	{
		Physics::LogicGroup* lg = pWeapon->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
		if(lg)
			lg->Deactivate();
	}

	// The weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! ArcherWeaponDef::Create
//! Create a Archer Type Weapon
//!
//!---------------------------------------------------------------------------------------------
CEntity* ArcherWeaponDef::Create(Character* pCharacter, ntstd::String)
{
	ntError(pCharacter);

	// Wrap the character in a ninja lua object for use with attribute tables
	CLuaGlobal::Get().State().Push((CEntity*)pCharacter);
	NinjaLua::LuaObject obCharacter(-1, CLuaGlobal::Get().State(), false);

	// Generate a unique name for the weapon
	ntstd::String sWeaponName = pCharacter->GetName();
	sWeaponName.append("_Xbow");

	// Construct the Weapon
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Object_ArcherXBow", sWeaponName.c_str(), GameGUID(), 0, true, false);
	CEntity* pWeapon = (CEntity*)pDO->GetBasePtr();

	LuaAttributeTable* pWeaponAttrs = LuaAttributeTable::Create();
	pWeapon->SetAttributeTable(pWeaponAttrs);
	pWeapon->GetAttributeTable()->SetDataObject(pDO);
	pWeaponAttrs->SetString("Name", sWeaponName.c_str());
	pWeaponAttrs->SetString("Clump", "entities/weapons/archers_xbow/archer_xbow.clump");
	pWeaponAttrs->SetString("SharedAttributes", "Att_Archers_Xbow");
	pWeaponAttrs->SetString("AnimationContainer", m_sAnimContainer.c_str());
	pWeaponAttrs->SetAttribute("ParentEntity", obCharacter);
	pWeaponAttrs->SetString("ParentTransform", m_sParentTransform.c_str());
	pWeaponAttrs->SetAttribute("Other", obCharacter);
	pWeaponAttrs->SetAttribute("archer", obCharacter);
	pWeaponAttrs->SetInteger("SectorBits", pCharacter->GetMappedAreaInfo());

	ObjectDatabase::Get().DoPostLoadDefaults(pDO);

	// Set the Weapons Position and Orientation
	ntAssert(pWeapon->GetHierarchy());
	CMatrix mat;
	CCamUtil::MatrixFromEuler_XYZ(mat, m_ptYPR.X()*DEG_TO_RAD_VALUE, m_ptYPR.Y()*DEG_TO_RAD_VALUE, m_ptYPR.Z()*DEG_TO_RAD_VALUE);
	mat.SetTranslation(m_ptPosition);
	pWeapon->GetHierarchy()->GetRootTransform()->SetLocalMatrix(mat);

	// The Weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! ThrownWeaponDef::Create
//! Create a Thrown Type Weapon - Respawn Version
//!
//!---------------------------------------------------------------------------------------------
CEntity* ThrownWeaponDef::Create(Character* pCharacter, CEntity* pWeapon)
{
	// Set the Weapons Position and Orientation
	ntError(pWeapon);
	ntError(pWeapon->GetHierarchy());
	CMatrix mat;
	CCamUtil::MatrixFromEuler_XYZ(mat, m_ptYPR.X()*DEG_TO_RAD_VALUE, m_ptYPR.Y()*DEG_TO_RAD_VALUE, m_ptYPR.Z()*DEG_TO_RAD_VALUE);
	mat.SetTranslation(m_ptPosition);

	// Unpause it
	pWeapon->Pause(false);
	pWeapon->Show();

	// Parent it to the character
	pWeapon->SetParentEntity(pCharacter);
	Transform* pTransformParent = pCharacter->GetHierarchy()->GetTransform(CHashedString(m_sParentTransform));
	Transform* pTransformTarget = pWeapon->GetHierarchy()->GetRootTransform();
	pTransformTarget->RemoveFromParent();
	pTransformParent->AddChild(pTransformTarget);
	pWeapon->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
	pWeapon->GetHierarchy()->GetRootTransform()->SetLocalMatrix(mat);
	((Interactable_Thrown*)pWeapon)->Attach();

	// The weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! RangedWeaponDef::Create Interactable_Thrown
//! Create a Ranged Type Weapon
//!
//!---------------------------------------------------------------------------------------------
CEntity* RangedWeaponDef::Create(ntstd::String sWeaponName, const Spawnee_Def* pSpawnDef)
{
	// Construct the Weapon
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Object_Ranged_Weapon", sWeaponName.c_str(), GameGUID(), 0, true, false);
	CEntity* pWeapon = (CEntity*)pDO->GetBasePtr();

	// Set up Weapon Attribute Table
	LuaAttributeTable* pWeaponAttrs = LuaAttributeTable::Create();
	pWeapon->SetAttributeTable(pWeaponAttrs);
	pWeapon->GetAttributeTable()->SetDataObject(pDO);

	pWeaponAttrs->SetString("Name", sWeaponName.c_str());
	pWeaponAttrs->SetString("Clump", m_sClump.c_str());
	pWeaponAttrs->SetString("SharedAttributes", m_sSharedAttributes.c_str());
	pWeaponAttrs->SetString("Position", "0,0,0");
	pWeaponAttrs->SetInteger("SectorBits", pSpawnDef->GetMappedAreaInfo());

	// Post Construct the Weapon
	ObjectDatabase::Get().DoPostLoadDefaults(pDO);

	// Deactivate the weapon until it is needed (Dario) - This solves a Havok complain that prevents the game to run
	pWeapon->GetPhysicsSystem()->Deactivate();

	// The weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! RangedWeaponDef::Create
//! Create a Ranged Type Weapon
//!
//!---------------------------------------------------------------------------------------------
CEntity* RangedWeaponDef::Create(Character* pCharacter, ntstd::String sAppend)
{
	ntError(pCharacter);

	// Wrap the character in a ninja lua object for use with attribute tables
	CLuaGlobal::Get().State().Push((CEntity*)pCharacter);
	NinjaLua::LuaObject obCharacter(-1, CLuaGlobal::Get().State(), false);

	// Generate a unique name for the weapon
	ntstd::String sWeaponName = pCharacter->GetName();
	sWeaponName.append(sAppend);

	// Construct the Weapon
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Object_Ranged_Weapon", sWeaponName.c_str(), GameGUID(), 0, true, false);
	CEntity* pWeapon = (CEntity*)pDO->GetBasePtr();

	// Set up Weapon Attribute Table
	LuaAttributeTable* pWeaponAttrs = LuaAttributeTable::Create();
	pWeapon->SetAttributeTable(pWeaponAttrs);
	pWeapon->GetAttributeTable()->SetDataObject(pDO);

	pWeaponAttrs->SetString("Name", sWeaponName.c_str());
	pWeaponAttrs->SetString("Clump", m_sClump.c_str());
	pWeaponAttrs->SetString("SharedAttributes", m_sSharedAttributes.c_str());
	pWeaponAttrs->SetString("Position", "0,0,0");
	pWeaponAttrs->SetAttribute("ParentEntity", obCharacter);
	pWeaponAttrs->SetString("ParentTransform", m_sParentTransform.c_str());
	pWeaponAttrs->SetInteger("SectorBits", pCharacter->GetMappedAreaInfo());

	// Post Construct the Weapon
	ObjectDatabase::Get().DoPostLoadDefaults(pDO);

	if(pWeapon->GetPhysicsSystem())
	{
		pWeapon->GetPhysicsSystem()->Deactivate();
	}

	// Set the Weapons Position and Orientation
	ntAssert(pWeapon->GetHierarchy());
	CMatrix mat;
	CCamUtil::MatrixFromEuler_XYZ(mat, m_ptYPR.X()*DEG_TO_RAD_VALUE, m_ptYPR.Y()*DEG_TO_RAD_VALUE, m_ptYPR.Z()*DEG_TO_RAD_VALUE);
	mat.SetTranslation(m_ptPosition);
	pWeapon->GetHierarchy()->GetRootTransform()->SetLocalMatrix(mat);
	//Store the held-matrix in the ranged weapon itself so that when switching to held mode it uses this matrix instead of
	//going to identity.
	Object_Ranged_Weapon* pRangedWeapon = static_cast<Object_Ranged_Weapon*>(pWeapon);
	pRangedWeapon->SetHeldMatrix(mat);
	
	// The weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! RangedWeaponDef::Create
//! Create a Ranged Type Weapon - Respawn Version
//!
//!---------------------------------------------------------------------------------------------
CEntity* RangedWeaponDef::Create(Character* pCharacter, CEntity* pWeapon)
{
	// Set the Weapons Position and Orientation
	ntAssert(pWeapon->GetHierarchy());
	CMatrix mat;
	CCamUtil::MatrixFromEuler_XYZ(mat, m_ptYPR.X()*DEG_TO_RAD_VALUE, m_ptYPR.Y()*DEG_TO_RAD_VALUE, m_ptYPR.Z()*DEG_TO_RAD_VALUE);
	mat.SetTranslation(m_ptPosition);

	// Unpause it
	pWeapon->Pause(false);
	pWeapon->GetRenderableComponent()->AddRemoveAll_Game(true);

	// Parent it to the character
	pWeapon->SetParentEntity(pCharacter);
	Transform* pTransformParent = pCharacter->GetHierarchy()->GetTransform(CHashedString(m_sParentTransform));
	Transform* pTransformTarget = pWeapon->GetHierarchy()->GetRootTransform();
	pTransformTarget->RemoveFromParent();
	pTransformParent->AddChild(pTransformTarget);
	pWeapon->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
	pWeapon->GetHierarchy()->GetRootTransform()->SetLocalMatrix(mat);
	//Message msg(msg_goto_attachedstate);
	//pWeapon->GetMessageHandler()->QueueMessage(msg);

	// The weapon is now ready to use
	return pWeapon;
}


//!---------------------------------------------------------------------------------------------
//!
//! HeroWeaponSetDef::Create_SwordChainEffect
//!
//!
//!---------------------------------------------------------------------------------------------
void HeroWeaponSetDef::Create_SwordChainEffect(CEntity* pBlade, Hero* pHero, CHashedString transform)
{
	ntError(pBlade);
	ntError(pBlade->GetHierarchy());
	ntError(pHero);
	ntError(pHero->GetHierarchy());

	// Get the named transforms - holding
	Transform* pobParentTransform = pBlade->GetHierarchy()->GetRootTransform();

	// Where the line links to...
	int iIdx = pHero->GetHierarchy()->GetTransformIndex(transform);
	ntAssert_p(iIdx != -1, ("Can't find transform '%s' on %s", ntStr::GetString(transform), pHero->GetName().c_str()));
	Transform* pobLinkTransform = pHero->GetHierarchy()->GetTransform(iIdx);

	// Create the new chain
	RangeStanceChain* pChain = NT_NEW RangeStanceChain(pobParentTransform, pobLinkTransform);
	ntAssert(pChain);

	// Get the renderable component of the holding entity
	CRenderableComponent* pobRenderable = pBlade->GetRenderableComponent();
	ntAssert(pobRenderable);

	// Give the ownership of this item to the parent renderable
	pobRenderable->AddAddtionalRenderable(pChain);
}
