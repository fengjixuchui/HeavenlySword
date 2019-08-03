#ifndef _NINJALUA_H
#define _NINJALUA_H

class CEntity;

extern "C" 
{
/*/
#include "Lua.h"
#include "Lualib.h"
#include "lauxlib.h"
//*//*/
#include "luaplus/include/lua.h"
#include "luaplus/include/lualib.h"
#include "luaplus/include/lauxlib.h"
//*/
#include "lua51/lua.h"
#include "lua51/lualib.h"
#include "lua51/lauxlib.h"
};


/*
#include "LuaState.h"
#include "Luavalue.h"
#include "Luaargs.h"
#include "LuaObject.h"
#include "Luaexposed.h"
#include "Luafunction.h"
/*/
#include "lua/nl_luastate.h"
#include "lua/nl_luavalue.h"
#include "lua/nl_luaexposed.h"
#include "lua/nl_luaargs.h"
#include "lua/nl_luaObject.h"
#include "lua/nl_luafunction.h"
#include "lua/nl_luastack.h"
#include "lua/nl_luadofile.h"
//*/

#include "game/luaglobal.h"

#endif // _NINJALUA_H


