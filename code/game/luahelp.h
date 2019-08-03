/***************************************************************************************************
*
*	DESCRIPTION		This helper object allows you to configure what libraries are loaded for a given
*					lua state. it also provides overide functions for core functions such as print,
*					ntAssert, errors etc.
*
*	NOTES			This should list all the methods provided by the base JAM lua libs, as well as
*					al the standard library stuff. If you add new methods / overidden librarys (such as
*					maths) please load them here and update the documentation.
*					Standard libraries are well documented in the lua Ref manual, just their names are
*					listed here to you can find what lives where quickly.
*
***************************************************************************************************/
/***************************************************************************************************
*
*	JAMLIB			tostring(var)	- this is defined to allow printing when the base lib is not loaded
*					print(...)		- use comma seperated values to print to our debug output.
*					ntAssert(cond)	- same as our ntAssert
*					ntAssert_p(cond,msg)	- same as our ntAssert_p
*					LOG(...)		- similar to print, we may wish to pipe this to a file
*					_ALERT(...)		- similar to print, used to raise errors by lua.
***************************************************************************************************/

#ifndef _LUA_HELPER
#define _LUA_HELPER

#include "lua/ninjalua.h"

/***************************************************************************************************
*
*	CLASS			CLuaHelper
*
*	DESCRIPTION		static class that provides handy helper functions for lua debugging and such
*
***************************************************************************************************/
class CLuaHelper
{
public:
	static void	Register();
	static const char* RetrieveLuaString(NinjaLua::LuaState* pobState, NinjaLua::LuaStack& args);
	static const char* FormatLuaString(const char* pcString, const char* pcStartStr, const char* pcEndStr);
	static void        DumpCallStack(NinjaLua::LuaState& state);
	static lua_State*  CreateThread(lua_State* pParent);

	static CPoint PointFromTable(const NinjaLua::LuaObject& obj)
	{
		ntAssert(obj.IsTable());
		ntAssert(obj[1].IsNumber());
		ntAssert(obj[2].IsNumber());
		ntAssert(obj[3].IsNumber());

		return CPoint(obj[1].GetFloat(),
                      obj[2].GetFloat(),
		              obj[3].GetFloat());

	}

	static CDirection DirectionFromTable(const NinjaLua::LuaObject& obj)
	{
		ntAssert(obj.IsTable());
		return CDirection(obj["x"].GetFloat(),
		                  obj["y"].GetFloat(),
		                  obj["z"].GetFloat());
	}
};


#endif
