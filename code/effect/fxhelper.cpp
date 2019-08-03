//----------------------------------------------------------------------------------------------------
//!
//!	\file fxhelper.cpp
//!	FX Helper functions
//!
//----------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "fxhelper.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "objectdatabase/dataobject.h"

#include "effect/effect_manager.h"
#include "effect/effect_trigger.h"
#include "effect/rangestancechain.h"
#include "effect/chainmanchains.h"


//-------------------------------------------------------------------------------------------------
// FUNC: FXHelper::Pfx_CreateStatic
// DESCRIPTION: Spawn a pre-defined particle effect in world space
//-------------------------------------------------------------------------------------------------
u_int FXHelper::Pfx_CreateStaticMatrix(CHashedString pcDefName, const CMatrix& obPosMatrix)
{
	EffectTrigger obEffectTrigger;
	obEffectTrigger.m_pDefinition = ObjectDatabase::Get().GetPointerFromName<void*>(pcDefName);
	
	return obEffectTrigger.ForceTrigger(obPosMatrix);
}


//-------------------------------------------------------------------------------------------------
// FUNC: FXHelper::Pfx_CreateStatic
// DESCRIPTION: Spawn a pre-defined particle effect in world space
//-------------------------------------------------------------------------------------------------
u_int FXHelper::Pfx_CreateStatic(CHashedString pcDefName, CEntity* pEnt, CHashedString pcTransformName)
{
	ntAssert(pEnt);
	ntAssert(pEnt->GetHierarchy());

	CHashedString obTransformHash(pcTransformName);

	const CMatrix& obWorldMatrix=pEnt->GetHierarchy()->GetTransform(obTransformHash)->GetWorldMatrix();
	
	EffectTrigger obEffectTrigger;
	obEffectTrigger.m_pDefinition = ObjectDatabase::Get().GetPointerFromName<void*>(pcDefName);
	
	return obEffectTrigger.ForceTrigger(obWorldMatrix);
}


//-------------------------------------------------------------------------------------------------
// FUNC: FXHelper::Pfx_CreateAttached(string,string,string)
// DESCRIPTION: Spawn a pre-defined particle effect thats attached to a transform.
//-------------------------------------------------------------------------------------------------
u_int FXHelper::Pfx_CreateAttached (CHashedString pcDefName, CHashedString pcEntityName, CHashedString pcTransformName)
{
	EffectTrigger obEffectTrigger;
	obEffectTrigger.m_pDefinition=ObjectDatabase::Get().GetPointerFromName<void*>(pcDefName);
	obEffectTrigger.m_obParentEntity=pcEntityName;
	obEffectTrigger.m_obParentTransform=pcTransformName;
	
	return obEffectTrigger.ForceTrigger();
}


//-------------------------------------------------------------------------------------------------
// FUNC: FXHelper::Pfx_Destroy(number,bool)
// DESCRIPTION: Terminate an existing effect instance.
//-------------------------------------------------------------------------------------------------
void FXHelper::Pfx_Destroy(u_int uiID, bool bImmediate)
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
