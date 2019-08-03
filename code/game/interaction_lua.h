//------------------------------------------------------------------------------------------
//!
//!	\file interaction_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_INTERACTION_LUA_INC
#define	_INTERACTION_LUA_INC

class CEntity;

class Interaction_Lua
{
public:
	void Lua_SetEntityHeightFromRenderable();
	void Lua_SetInteractionPriority(int iPriority);
	void Lua_AllowCollisionWith(CEntity* pOtherEnt);
};

#endif //_INTERACTION_LUA_INC
