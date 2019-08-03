//------------------------------------------------------------------------------------------
//!
//!	\file audio_lua.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/audio_lua.h"
#include "audio/gameaudiocomponents.h"

#include "game/luaglobal.h"

//------------------------------------------------------------------------------------------
//  CEntity - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(EntityAudioChannel)
	LUA_EXPOSED_METHOD(ChatterVO,	Lua_ChatterVO, "Play a chatterbox vo on this entity", "string cue, string bank, msg finishMessage", "The name of the cue to play|The source sfx bank|The message to send when the VO finishes")
LUA_EXPOSED_END(EntityAudioChannel)

void Audio_Lua::Lua_ChatterVO(const char* pcCue, const char* pcBank, NinjaLua::LuaObject msg)
{
	UNUSED(pcCue);
	UNUSED(pcBank);
	UNUSED(msg);

//	((EntityAudioChannel*)this)->SetCallback(msg);
//	((EntityAudioChannel*)this)->Play(CHANNEL_VOICE_HIGHPRI, SPATIALIZATION_HRTF, pcBank, pcCue);
}