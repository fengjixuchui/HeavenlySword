/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES
*
***************************************************************************************************/

#ifndef	_GUILUA_H
#define	_GUILUA_H

#include "lua/ninjalua.h"

class CGuiScreen;

class CGuiLua
{
private:
	CGuiLua();

public:

	static void Initialise();
	static void Uninitialise();

	static NinjaLua::LuaFunction GetLuaFunction(const char* pcFunction);

	static void PushContext(CGuiScreen* pobScreen);
	static void PopContext();
	static CGuiScreen* GetContext();

	static NinjaLua::LuaObject GetGlobalStore();

private:

	typedef ntstd::List<CGuiScreen*> ContextStack;
	static ContextStack* ms_pobContextStack;
	static NinjaLua::LuaObject ms_obGlobalStore;
};
#endif // _GUILUA_H
