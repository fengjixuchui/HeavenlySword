//------------------------------------------------------------------------------------------
//!
//!	\file hud/subtitleman.cpp
//!	
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "hud/subtitleman.h"

#include "gui/tutorial.h"
#include "gui/guimanager.h"
#include "game/luaglobal.h"


//------------------------------------------------------------------------------------------
//!
//!	SubtitleMan::SubtitleMan
//!	
//!
//------------------------------------------------------------------------------------------
/*SubtitleMan::SubtitleMan()
{
	ntAssert_p(0, ("SubtitleMan is now defunct"));

	////////////////////////////////////////////////////////////////////////////////////////
	// Create the meta-table for lua.  
	////////////////////////////////////////////////////////////////////////////////////////
	NinjaLua::LuaState& luaState(CLuaGlobal::Get().State());
	NinjaLua::LuaObject luaGlobs = luaState.GetGlobals();

	NinjaLua::LuaObject luaTbl = NinjaLua::LuaObject::CreateTable(luaState);
	luaGlobs.Set("SubtitleMan", luaTbl);

	// Add Function for creating new views
	luaTbl.Register("Show", SubtitleMan::Lua_Show);
	luaTbl.Register("Hide", SubtitleMan::Lua_Hide);
}*/



//------------------------------------------------------------------------------------------
//!
//!	SubtitleMan::Update
//!	
//!
//------------------------------------------------------------------------------------------
/*void SubtitleMan::Update(float fTimeDelta)
{
	ntAssert_p(0, ("SubtitleMan is now defunct"));

	if(!m_itemQueue.empty())
	{
		SubtitleItem& itemNext = m_itemQueue.front();

		itemNext.fDelay -= fTimeDelta;
		if(itemNext.fDelay < 0.f)
		{
			Show(itemNext.sSubtitle);
			m_itemQueue.pop_front();
		}
	}
}*/


//------------------------------------------------------------------------------------------
//!
//!	SubtitleMan::Show
//!	Display a subtitle
//!
//------------------------------------------------------------------------------------------
/*void SubtitleMan::Show(const char* sSubtitle)
{
	ntAssert_p(0, ("SubtitleMan is now defunct"));
	if(strlen(sSubtitle) <= 0)
	{
		CTutorialManager::Get().RemoveScreenMessage();
	}
	else
	{
		CTutorialManager::Get().AddScreenMessage(sSubtitle, "Body", 0.f);
	}
}*/


//------------------------------------------------------------------------------------------
//!
//!	SubtitleMan::Queue_Show
//!	Add a show item to the subtitle queue
//!
//------------------------------------------------------------------------------------------
/*void SubtitleMan::Queue_Show(const char* sSubtitle, float fDelay)
{
	ntAssert_p(0, ("SubtitleMan is now defunct"));
	SubtitleItem item(sSubtitle, fDelay);
	m_itemQueue.push_back(item);
}*/


//------------------------------------------------------------------------------------------
//!
//!	SubtitleMan::Hide
//!	Add a hide item to the subtitle queue
//!
//------------------------------------------------------------------------------------------
/*void SubtitleMan::Queue_Hide(float fDelay)
{
	ntAssert_p(0, ("SubtitleMan is now defunct"));
	SubtitleItem item("", fDelay);
	m_itemQueue.push_back(item);
}*/


//------------------------------------------------------------------------------------------
//!
//!	SubtitleMan::Lua_Show
//!	Display a subtitle - LUA INTERFACE
//!
//------------------------------------------------------------------------------------------
/*void SubtitleMan::Lua_Show(const char* sSubtitle, float fDelay)
{
	ntAssert_p(0, ("SubtitleMan is now defunct"));
	if(SubtitleMan::Exists())
	{
		SubtitleMan::Get().Queue_Show(sSubtitle, fDelay);
	}

	//CTutorialManager::Get().AddScreenMessage(message, font, duration);
}*/


//------------------------------------------------------------------------------------------
//!
//!	SubtitleMan::Lua_Hide
//!	Hide any subtitles - LUA INTERFACE
//!
//------------------------------------------------------------------------------------------
/*void SubtitleMan::Lua_Hide(float fDelay)
{
	ntAssert_p(0, ("SubtitleMan is now defunct"));

	if(SubtitleMan::Exists())
	{
		SubtitleMan::Get().Queue_Hide(fDelay);
	}

	//CTutorialManager::Get().RemoveScreenMessage();
}*/

