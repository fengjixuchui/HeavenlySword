//------------------------------------------------------------------------------------------
//!
//!	\file ai_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_AI_LUA_INC
#define	_AI_LUA_INC

class CEntity;

class AI_Lua
{
public:
	void Lua_MakeConscious();
	void Lua_SetActionUsing(int);
	void Lua_SetControllerModifier(int);
	int Lua_GetControllerModifier();
	CEntity* Lua_FindNearestWeapon(const char* pcWeaponType, float fMaxDistance);
};

#endif //_AI_LUA_INC
