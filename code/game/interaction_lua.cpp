//------------------------------------------------------------------------------------------
//!
//!	\file interaction_lua.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/interaction_lua.h"
#include "game/interactioncomponent.h"
#include "game/luaglobal.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/renderablecomponent.h"
#include "anim/hierarchy.h"
#include "gfx/renderable.h"


//------------------------------------------------------------------------------------------
//  CInteractionComponent - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CInteractionComponent)
	LUA_EXPOSED_METHOD(SetHeight,						SetEntityHeight, "", "", "")
	LUA_EXPOSED_METHOD(ExcludeCollisionWith,			ExcludeCollisionWith, "Stop collision between entities", "entity entToExclude", "")
	LUA_EXPOSED_METHOD(AllowCollisionWith,				AllowCollisionWith, "Allow collision between entities",  "entity entToAllow", "")

	LUA_EXPOSED_METHOD(SetEntityHeightFromRenderable,	Lua_SetEntityHeightFromRenderable, "Define the height of the entity, to assist targeting.", "", "")
	LUA_EXPOSED_METHOD(SetInteractionPriority,			Lua_SetInteractionPriority, "Set the Interaction Priority", "int Priority", "")
LUA_EXPOSED_END(CInteractionComponent)


//------------------------------------------------------------------------------------------
//!
//!	Interaction_Lua::Lua_SetEntityHeightFromRenderable
//! Define the height of the entity, to assist targeting.
//!
//------------------------------------------------------------------------------------------
void Interaction_Lua::Lua_SetEntityHeightFromRenderable()
{
	CEntity* pSelf = ((CInteractionComponent*)this)->m_pobParentEntity;

	if(pSelf && pSelf->GetRenderableComponent())
	{
		//const CRenderable* pobRenderable = pSelf->GetRenderableComponent()->GetRenderable(pSelf->GetHierarchy()->GetRootTransform());
		//float fHeight = pobRenderable->GetBounds().GetHalfLengths().Y()*2.0f;
		float fHeight=pSelf->GetRenderableComponent()->GetWorldSpaceAABB().GetHalfLengths().Y() * 2.0f;
		((CInteractionComponent*)this)->SetEntityHeight(fHeight);
		return;
	}

	ntAssert(false);
}


//------------------------------------------------------------------------------------------
//!
//!	Interaction_Lua::Lua_SetInteractionPriority
//! 
//!
//------------------------------------------------------------------------------------------
void Interaction_Lua::Lua_SetInteractionPriority(int iPriority)
{
	((CInteractionComponent*)this)->SetInteractionType((INTERACTION_PRIORITY)iPriority);
}

//------------------------------------------------------------------------------------------
//!
//!	Interaction_Lua::Lua_AllowCollisionWith
//! 
//!
//------------------------------------------------------------------------------------------
void Interaction_Lua::Lua_AllowCollisionWith(CEntity* pOtherEnt)
{
	CInteractionComponent* pInteractionComp = (CInteractionComponent*)this;

	if (pOtherEnt && !pInteractionComp->CanCollideWith(pOtherEnt)) // Check to make sure we are associated first
	{
		ntAssert(pOtherEnt->GetInteractionComponent());

		CEntity* pSelf = pInteractionComp->m_pobParentEntity;

		pOtherEnt->GetInteractionComponent()->RemoveFromCollisionFilter(pSelf); // Remove parent from target entities list

		pInteractionComp->RemoveFromCollisionFilter(pOtherEnt); // Remove entity from this entities list
	}
}
