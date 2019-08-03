//------------------------------------------------------------------------------------------
//!
//!	\file awarebindings.h
//! Contains the interface to the awareness component that is available to the scripting
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/awarebindings.h"
#include "game/awareness.h"
#include "game/luaglobal.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/entitymanager.h"
#include "game/inputcomponent.h" // Running target check
#include "game/query.h"
#include "game/movementcontrollerinterface.h"
#include "game/renderablecomponent.h"
#include "game/aicomponent.h"

#include "physics/system.h"

#include "gfx/renderable.h"

#include "core/boundingvolumes.h"

#include "anim/hierarchy.h"

#include "interactioncomponent.h"

#include "luaattrtable.h"
#include "lua\ninjalua.h"
#include "luaexptypes.h"

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	bool HasAutoLockon()
// DESCRIPTION:	Finds out whether the character is 'locked' on to a character
//-------------------------------------------------------------------------------------------------
static bool HasAutoLockon()
{
	// Get a pointer to the entity we are acting on and check its validity
	CEntity* pobEnt=CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	ntAssert( pobEnt->GetAwarenessComponent() );

	// Get and return the target - may return a null pointer
	return ( pobEnt->GetAwarenessComponent()->GetCurrentAutoLockonP() != 0 );
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	bool IsAwareOfEnemies()
// DESCRIPTION:	Find out whether the character should behave as though there are other characters
//				in the vicinity.
//-------------------------------------------------------------------------------------------------
static bool IsAwareOfEnemies()
{
	// Get a pointer to the entity we are acting on and check its validity
	CEntity* pobEnt=CLuaGlobal::Get().GetTarg();
	ntAssert( pobEnt );
	ntAssert( pobEnt->GetAwarenessComponent() );

	// Get and return the target - may return a null pointer
	return ( pobEnt->GetAwarenessComponent()->IsAwareOfEnemies() != 0 );
}

//------------------------------------------------------------------------------------------
//!
//!	AwareBindings::Register
//!	Register the available awareness bindings with the Lua environment
//!
//------------------------------------------------------------------------------------------
void AwareBindings::Register()
{
	// Get the location of the Lua global objects
	NinjaLua::LuaState& obState = CLuaGlobal::Get().State();
	NinjaLua::LuaObject obGlobals = obState.GetGlobals();

	// Register our functionality	
	obGlobals.Register( "HasAutoLockon", HasAutoLockon );
	obGlobals.Register( "IsAwareOfEnemies", IsAwareOfEnemies );
}
