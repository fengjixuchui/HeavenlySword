//------------------------------------------------------------------------------------------
//!
//!	\file lua_enums.h
//!
//------------------------------------------------------------------------------------------

#include "game/luaglobal.h"
#include "lua/ninjalua.h"

#ifdef USER_JohnL
  //#define LUA_ENUM_DUMP
#endif

#ifdef LUA_ENUM_DUMP
	static const char* l_pcPrefix = 0;
	static void dumpluaenumprefix(const char* pcPrefix)   {l_pcPrefix = pcPrefix;}
	static void dumpluaenum(const char* pcName, int iVal) {ntPrintf("LuaEnum: %s.%s = %d\n", l_pcPrefix, pcName, iVal);}
#else
	static void dumpluaenumprefix(const char*) {;}
	static void dumpluaenum(const char*, int) {;}
#endif

#define ENUM_STARTEX(n,t)																				\
	{																									\
		if(t&ENUM_LUA)																					\
		{																								\
			/*NinjaLua::LuaObject obMt = rState.GetMetaTable(#n);*/										\
			/*obMt.Set("_type", #n);*/																	\
			dumpluaenumprefix(#n);																		\
			NinjaLua::LuaObject obTbl = NinjaLua::LuaObject::CreateTable(rState);						\
			rState.GetGlobals().Set(#n, obTbl);															\
			unsigned int i = 0;

#define ENUM_STARTEX_PUBLISH_AS(n,t,a)																	\
	{																									\
		if(t&ENUM_LUA)																					\
		{																								\
			/*NinjaLua::LuaObject obMt = rState.GetMetaTable(#n);*/										\
			/*obMt.Set("_type", #n);*/																	\
			dumpluaenumprefix(#a);																		\
			NinjaLua::LuaObject obTbl = NinjaLua::LuaObject::CreateTable(rState);						\
			rState.GetGlobals().Set(#a, obTbl);															\
			unsigned int i = 0;

/*#define _ENUM_SET(k, v) obTbl.Push();						\
						lua_pushstring(rState, k);			\
						lua_pushinteger(rState, v);			\
						obMt.Push();						\
						lua_setmetatable(rState,-2);		\
						lua_settable(rState, -3);			\
						lua_pop(rState, 1);*/

#define _ENUM_SET(k,v)  obTbl.Set(k,v)

#define ENUM_SET(x, n)			i = n; dumpluaenum(#x, i);  _ENUM_SET(#x, i++);
#define ENUM_SET_AS(x, n, as)	i = n; dumpluaenum(#as, i); _ENUM_SET(#as, i++);
#define ENUM_AUTO(x)			dumpluaenum(#x, i);  _ENUM_SET(#x, i++);
#define ENUM_AUTO_AS(x, as)		dumpluaenum(#as, i); _ENUM_SET(#as, i++);

#define ENUM_END()																						\
		}																								\
	}


void BuildLuaEnums()
{
	NinjaLua::LuaState& rState = CLuaGlobal::Get().State();
	#include "editable/enumlist.h"
	#include "lua_enum_list.h"
	#include "editable/enums_formations.h"

};
