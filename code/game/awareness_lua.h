//------------------------------------------------------------------------------------------
//!
//!	\file awareness_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_AWARENESS_LUA_INC
#define	_AWARENESS_LUA_INC

#include "lua/ninjalua.h"
#include "game/interactiontarget.h"

class Awareness_Lua
{
public:
	HAS_LUA_INTERFACE()

	CEntity* Lua_FindInteractionTarget ( int iCharacterType = 0 );
	CEntity* Lua_FindNamedInteractionTarget(const char* pcNameSubstring, int iCharacterType = 0);

	CInteractionTarget FindInteractionTarget(	int iCharacterType = 0 );
	CInteractionTarget FindNamedInteractionTarget(const char* pcNameSubstring, int iCharacterType = 0);
};

#endif //_AWARENESS_LUA_INC
