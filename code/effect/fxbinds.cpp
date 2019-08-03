//----------------------------------------------------------------------------------------------------
//!
//!	\file fxbinds.cpp
//!	FX bindings
//!
//----------------------------------------------------------------------------------------------------

#include "fxbinds.h"

#include "game/luaglobal.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/renderablecomponent.h"
#include "game/luaattrtable.h"
#include "anim/hierarchy.h"
#include "objectdatabase/dataobject.h"

#include "effect/effect_manager.h"
#include "effect/effect_trigger.h"
#include "effect/rangestancechain.h"
#include "effect/chainmanchains.h"


//-------------------------------------------------------------------------------------------------
// BINDFUNC: FX.Particles_CreateStatic(string,string,string)
// DESCRIPTION: Spawn a pre-defined particle effect in world space
//-------------------------------------------------------------------------------------------------
static u_int Pfx_CreateStatic(CHashedString pcDefName, CEntity* pEnt, CHashedString pcTransformName)
{
	ntAssert(pEnt);
	ntAssert(pEnt->GetHierarchy());

	CHashedString obTransformHash(pcTransformName);

	const CMatrix& obWorldMatrix=pEnt->GetHierarchy()->GetTransform(obTransformHash)->GetWorldMatrix();
	
	EffectTrigger obEffectTrigger;
	obEffectTrigger.m_pDefinition=ObjectDatabase::Get().GetPointerFromName<void*>(pcDefName);
	
	return obEffectTrigger.ForceTrigger(obWorldMatrix);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: FX.ParticlesCreateAttached(string,string,string)
// DESCRIPTION: Spawn a pre-defined particle effect thats attached to a transform.
//-------------------------------------------------------------------------------------------------
static u_int Pfx_CreateAttached (CHashedString pcDefName, CHashedString pcEntityName, CHashedString pcTransformName)
{
	EffectTrigger obEffectTrigger;
	obEffectTrigger.m_pDefinition=ObjectDatabase::Get().GetPointerFromName<void*>(pcDefName);
	obEffectTrigger.m_obParentEntity=pcEntityName;
	obEffectTrigger.m_obParentTransform=pcTransformName;
	
	return obEffectTrigger.ForceTrigger();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: FX.ParticlesDestroy(number,bool)
// DESCRIPTION: Terminate an existing effect instance.
//-------------------------------------------------------------------------------------------------
static void Pfx_Destroy(u_int uiID, bool bImmediate)
{
	if (bImmediate) // We want to kill the effect immediately
	{
		EffectManager::Get().KillEffectNow(uiID);
	}
	else // We want to expire the effect naturally
	{
		EffectManager::Get().KillEffectWhenReady(uiID);
	}
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	FX.CreateSwordChainEffect()
// DESCRIPTION:	Creates a chain for the range weapons
// A bit specific i know but at this stage it is hard to come up with
// what this could potentially be a example of
//-------------------------------------------------------------------------------------------------
static int CreateSwordChainEffect(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);

	LuaAttributeTable* pHoldingTable = LuaAttributeTable::GetAndCheckFromLuaState(args[1]);
	LuaAttributeTable* pLinkTable = LuaAttributeTable::GetAndCheckFromLuaState(args[2]);

	ntAssert(args[3].IsString());
	const char* pcLinkTransform = args[3].GetString();
								   								   
	// Find the entities from the lua input
	CEntity* pobHoldingEntity = pHoldingTable->GetEntity();
	ntError_p(pobHoldingEntity, ("Holding Entity is nil!"));
	ntError_p(pobHoldingEntity->GetHierarchy(), ("Holding Entity has no hierarchy!"));

	CEntity* pobLinkEntity = pLinkTable->GetEntity();
	ntError_p(pobLinkEntity, ("Link Entity is nil!"));
	ntError_p(pobLinkEntity->GetHierarchy(), ("Link Entity has no hierarchy!"));

	// Get the named transforms - holding
	Transform* pobParentTransform = pobHoldingEntity->GetHierarchy()->GetRootTransform();

	// Where the line links to...
	int iIdx = pobLinkEntity->GetHierarchy()->GetTransformIndex(CHashedString(pcLinkTransform));
	ntAssert_p(iIdx != -1, ("Can't find transform '%s' on %s", pcLinkTransform, pobLinkEntity->GetName().c_str()));
	Transform* pobLinkTransform = pobLinkEntity->GetHierarchy()->GetTransform(iIdx);

	// Create the new chain
	RangeStanceChain* pChain = NT_NEW_CHUNK ( Mem::MC_GFX ) RangeStanceChain(pobParentTransform, pobLinkTransform);
	ntAssert(pChain);

	// Get the renderable component of the holding entity
	CRenderableComponent* pobRenderable = pobHoldingEntity->GetRenderableComponent();
	ntAssert(pobRenderable);

	// Give the ownership of this item to the parent renderable
	pobRenderable->AddAddtionalRenderable(pChain);

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	FX.CreateChainmanChains()
// DESCRIPTION:	Creates a set of chains for the chainman
//-------------------------------------------------------------------------------------------------
static int CreateChainmanChains(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);

	LuaAttributeTable* pTable = LuaAttributeTable::GetAndCheckFromLuaState(args[1]);
	ntError_p(pTable, ("Table is nil!"));
								   								   
	// Find the entities from the lua input
	CEntity* pParent = pTable->GetEntity();
	ntError_p(pParent, ("Parent Entity is nil!"));
	ntError_p(pParent->GetHierarchy(), ("Parent Entity has no hierarchy!"));

	// Parent transform is root, as we're a world space effect
	Transform* pParentTransform = pParent->GetHierarchy()->GetRootTransform();

	// Create the new chain
	ChainmanChains* pChain = NT_NEW_CHUNK ( Mem::MC_GFX ) ChainmanChains(pParentTransform);
	ntAssert(pChain);

	// Get the renderable component of the holding entity
	CRenderableComponent* pRenderable = pParent->GetRenderableComponent();
	ntAssert(pRenderable);

	// Give the ownership of this item to the parent renderable
	pRenderable->AddAddtionalRenderable(pChain);

	// Return a pointer to the renderable
	NinjaLua::LuaValue::Push<ChainmanChains*>(*pobState, pChain);
	//pobState->Push(pChain);

	return 1;
}


void FXBinds::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();
	obGlobals.Set("FX", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));

	// Particle effects
	obGlobals["FX"].Register("Particles_CreateStatic",		Pfx_CreateStatic);
	obGlobals["FX"].Register("Particles_CreateAttached",	Pfx_CreateAttached);
	obGlobals["FX"].Register("Particles_Destroy",			Pfx_Destroy);

	obGlobals["FX"].RegisterRaw("CreateSwordChainEffect", CreateSwordChainEffect);
	obGlobals["FX"].RegisterRaw("CreateChainmanChains", CreateChainmanChains);
}

