//------------------------------------------------------------------------------------------
//!
//!	\file audio_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_AUDIO_LUA_INC
#define	_AUDIO_LUA_INC

namespace NinjaLua {class LuaObject;};

class Audio_Lua
{
public:
	void Lua_ChatterVO(const char* pcCue, const char* pcBank, NinjaLua::LuaObject msg); // Messages should not be lua objects! - JML	
};

#endif //_AUDIO_LUA_INC
