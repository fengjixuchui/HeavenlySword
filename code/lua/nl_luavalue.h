#ifndef LUAVALUE_INC
#define LUAVALUE_INC

namespace NinjaLua
{

#define LV_MISMATCH(T)			LogLuaWarning(rState, "Type Mismatch, expecting %s not %s.\n",LuaValue::ArgTypeName<T*>(),LuaValue::TypeName(rState,iIndex));
#define LV_APPLY_METADATA(T)	T->pMetaTable->Push(); lua_setmetatable(rState,-2); //ntPrintf("Applying Metatable %s\n", T->pMetaTable->Get<const char*, const char*>("_type"));

bool CheckMetaData(LuaState& state, LuaObject& meta, const char* pcType);

#define LV_CHECK_METADATA(T)	lua_getmetatable(rState, iIndex);					\
								LuaObject meta = LuaObject(-1, rState,false);		\
								bool bCHECK = meta.CheckMetaTable(#T);				\
								UNUSED(bCHECK);

#define LV_DECLARE_USERDATA2(T, TSHORT)																																																											\
namespace NinjaLua {																																																															\
namespace LuaValue																																																																\
{																																																																				\
	template<>static bool			Is<T*>(LuaState& rState, int iIndex)				{if(lua_isnil(rState, iIndex)) return true; if(!lua_isuserdata(rState, iIndex)) return false; LV_CHECK_METADATA(TSHORT); return bCHECK;}												\
	template<>static bool			Is<const T*>(LuaState& rState, int iIndex)			{if(lua_isnil(rState, iIndex)) return true; if(!lua_isuserdata(rState, iIndex)) return false; LV_CHECK_METADATA(TSHORT); return bCHECK;}												\
	template<>static const char*	ArgTypeName<T*>()									{return "NinjaLua_Type("#TSHORT"*)";}																																					\
	template<>static T*				Get<T*>(LuaState& rState, int iIndex)				{if(lua_isnil(rState, iIndex)) return 0; LV_CHECK_METADATA(TSHORT); if(lua_isuserdata(rState, iIndex)&&bCHECK) return *(T**)lua_touserdata(rState, iIndex); LV_MISMATCH(T); return 0;}	\
	template<>static const T*		Get<const T*>(LuaState& rState, int iIndex)			{if(lua_isnil(rState, iIndex)) return 0; LV_CHECK_METADATA(TSHORT); if(lua_isuserdata(rState, iIndex)&&bCHECK) return *(T**)lua_touserdata(rState, iIndex); LV_MISMATCH(T); return 0;}	\
	template<>static T&				Get<T&>(LuaState& rState, int iIndex)				{LV_CHECK_METADATA(TSHORT); ntAssert(lua_isuserdata(rState, iIndex) && bCHECK);  return *(*(T**) lua_touserdata(rState, iIndex));}														\
	template<>static int			Push<T*>(LuaState& rState, T* Value)				{if(!Value) return 0; *(T**) lua_newuserdata(rState, sizeof(T*)) = Value; LV_APPLY_METADATA(Value); return 1;}																			\
	template<>static int			Push<const T*>(LuaState& rState, const T* Value)	{if(!Value) return 0; *(const T**) lua_newuserdata(rState, sizeof(T*)) = Value; LV_APPLY_METADATA(Value); return 1;}																	\
	/*template<>static int			Push<T&>(LuaState& rState, T& Value)                {*(*(T**) lua_newuserdata(rState, sizeof(T*))) = &Value; LV_APPLY_METADATA(T); return 1;}*/																								\
}}

#define LV_DECLARE_USERDATA(T) LV_DECLARE_USERDATA2(T,T);

/*#define LV_DECLARE_ENUM(E)																																														\
namespace NinjaLua {																																																\
namespace LuaValue																																																	\
{																																																					\
	template<>static bool			Is<E>(LuaState& rState, int iIndex)		{if(lua_isnil(rState, iIndex)) return false; if(!lua_isnumber(rState, iIndex)) return false; LV_CHECK_METADATA(E); return bCHECK;}		\
	template<>static const char*	ArgTypeName<E>()						{return "NinjaLua_Enum("#E")";}																											\
	template<>static E				Get<E>(LuaState& rState, int iIndex)	{LV_CHECK_METADATA(E); ntAssert(lua_isnumber(rState, iIndex) && bCHECK);  return (E)lua_tointeger(rState, iIndex);}						\
	template<>static int			Push<E>(LuaState& rState, E Value)		{lua_pushinteger(rState, (int)Value); LV_APPLY_METADATA(E); return 1;}																	\
}}*/

//------------------------------------------------------------------------------------------
//!
//!	LuaAutoPop
//!	
//!
//------------------------------------------------------------------------------------------
struct LuaAutoPop
{
	LuaState&	m_State;
	int			m_Pop;

	explicit 
	LuaAutoPop(LuaState& rState,  int iPop ) : 
		m_State( rState ),
		m_Pop( iPop )
	{
	}

	~LuaAutoPop( void )
	{
		lua_pop( m_State, m_Pop );
	}
};

//------------------------------------------------------------------------------------------
//!
//!	LuaAutoBlock
//!	
//!
//------------------------------------------------------------------------------------------
struct LuaAutoBlock
{
	LuaState&	m_State;
	int			m_OldTop;

	explicit 
	LuaAutoBlock(LuaState& rState ) : 
		m_State( rState ),
		m_OldTop( rState.GetTop() )
	{
	}

	~LuaAutoBlock( void )
	{
		if( m_OldTop < m_State.GetTop() )
			lua_pop( m_State, m_State.GetTop() - m_OldTop );
	}
};

inline TLuaHashKey LuaGetHashKey(const CHashedString& str)
{
	return str.GetHash();
}

//------------------------------------------------------------------------------------------
//!
//!	LuaValue                                                                                 
//!	
//!
//------------------------------------------------------------------------------------------
namespace LuaValue
{
	// Type Checking
	// -------------
	template<typename TYPE>	static bool Is(LuaState&, int iIndex);

	template<>static bool Is<bool>(LuaState& rState, int iIndex)					{return lua_isboolean(rState, iIndex) ? true : false;}
	template<>static bool Is<int>(LuaState& rState, int iIndex)						{return lua_isnumber(rState, iIndex) ? true : false;}
	template<>static bool Is<uint32_t>(LuaState& rState, int iIndex)				{return lua_isnumber(rState, iIndex) ? true : false;}
	template<>static bool Is<float>(LuaState& rState, int iIndex)					{return lua_isnumber(rState, iIndex) ? true : false;}
	template<>static bool Is<double>(LuaState& rState, int iIndex)					{return lua_isnumber(rState, iIndex) ? true : false;}
	template<>static bool Is<const char*>(LuaState& rState, int iIndex)				{return lua_isstring(rState, iIndex) || lua_isnil(rState, iIndex) ? true : false;}
	template<>static bool Is<ntstd::String>(LuaState& rState, int iIndex)			{return lua_isstring(rState, iIndex) ? true : false;}
	template<>static bool Is<CHashedString>(LuaState& rState, int iIndex)			{return lua_isstring(rState, iIndex) ? true : false;}
	template<>static bool Is<CHashedString const&>(LuaState& rState, int iIndex)	{return lua_isstring(rState, iIndex) ? true : false;}

	static const char* TypeName(LuaState& rState, int iIndex)
	{
		switch(lua_type(rState, iIndex))
		{
		case LUA_TNIL:
			return "nil";
		case LUA_TBOOLEAN:
			return "boolean";
		case LUA_TLIGHTUSERDATA:
			return "lightuserdata";
		case LUA_TNUMBER:             // Remove Enum Section...
			return "number";
		case LUA_TSTRING:
			return "String";
		case LUA_TTABLE:
			return "table";
		case LUA_TFUNCTION:
			return "function";
		case LUA_TUSERDATA:
		{
			static char buf[128];
			lua_getmetatable(rState, iIndex);
			lua_pushstring(rState, "_type");
			lua_gettable(rState,-2); 

			if(Is<const char*>(rState, -1))
				sprintf(buf, "NinjaLua_UserData(%s)", lua_tostring(rState, -1));
			else
				sprintf(buf, "NinjaLua_UserData(unknown)");

			lua_pop(rState,2);
			return buf;
		}
		case LUA_TTHREAD:
			return "thread";
		//case LUA_TWSTRING:
		//	return "wstring";
		default:
			return "NinjaLua_Type(unknown)";

		}
	}

	template<class TYPE> static const char* ArgTypeName()				{return "NinjaLua_Type(unknown)";}
	template<>			 static const char* ArgTypeName<int>()			{return "number";}
	template<>			 static const char* ArgTypeName<uint32_t>()		{return "number";}
	template<>			 static const char* ArgTypeName<float>()		{return "number";}
	template<>			 static const char* ArgTypeName<double>()		{return "number";}
	template<>			 static const char* ArgTypeName<const char*>()	{return "String";}
	template<>			 static const char* ArgTypeName<CKeyString>()	{return "String";}
	template<>			 static const char* ArgTypeName<bool>()			{return "boolean";}

	// Get Types off the Stack
	// -----------------------
	template<typename TYPE>	static TYPE Get(LuaState&, int iIndex)
	{
		static_assert(false, TemplateInstantiationProhibited);
	}

	template<>static bool          Get<bool>(LuaState& rState, int iIndex)			{ntAssert(lua_isboolean(rState, iIndex)); return lua_toboolean(rState, iIndex) ? true : false;}
	template<>static int           Get<int>(LuaState& rState, int iIndex)			{ntAssert(lua_isnumber(rState, iIndex)); return (int) lua_tonumber(rState, iIndex);}
	template<>static uint32_t      Get<uint32_t>(LuaState& rState, int iIndex)		{ntAssert(lua_isnumber(rState, iIndex)); return (uint32_t) lua_tonumber(rState, iIndex);}
	template<>static float         Get<float>(LuaState& rState, int iIndex)			{ntAssert(lua_isnumber(rState, iIndex)); return (float) lua_tonumber(rState, iIndex);}
	template<>static double        Get<double>(LuaState& rState, int iIndex)		{ntAssert(lua_isnumber(rState, iIndex)); return (double) lua_tonumber(rState, iIndex);}
	template<>static const char*   Get<const char*>(LuaState& rState, int iIndex)	{if(lua_isnil(rState, iIndex)) return 0; ntAssert(lua_isstring(rState, iIndex)); return lua_tostring(rState, iIndex);}
	template<>static ntstd::String Get<ntstd::String>(LuaState& rState, int iIndex)	{ntAssert(lua_isstring(rState, iIndex)); return ntstd::String(lua_tostring(rState, iIndex));}
	template<>static CHashedString Get<CHashedString>(LuaState& rState, int iIndex) {ntAssert(lua_isstring(rState, iIndex)); return CHashedString(lua_tohashkey(rState, iIndex)); }
	template<>static CKeyString Get<CKeyString>(LuaState& rState, int iIndex) {ntAssert(lua_isstring(rState, iIndex)); return CKeyString(lua_tostring(rState, iIndex)); }
	//template<>static const CHashedString& Get<CHashedString const&>(LuaState& rState, int iIndex) {		static_assert(false, TemplateInstantiationProhibited); }

	// Pop Types off the Stack
	// -----------------------
	template<typename TYPE>	static TYPE Pop(LuaState&, int iIndex);

	template<>static bool          Pop<bool>(LuaState& rState, int iIndex)			{LuaAutoPop Pop(rState, 1); return Get<bool>(rState, iIndex);}
	template<>static int           Pop<int>(LuaState& rState, int iIndex)			{LuaAutoPop Pop(rState, 1); return Get<int>(rState, iIndex);}
	template<>static uint32_t      Pop<uint32_t>(LuaState& rState, int iIndex)		{LuaAutoPop Pop(rState, 1); return Get<uint32_t>(rState, iIndex);}
	template<>static float         Pop<float>(LuaState& rState, int iIndex)			{LuaAutoPop Pop(rState, 1); return Get<float>(rState, iIndex);}
	template<>static const char*   Pop<const char*>(LuaState& rState, int iIndex)	{LuaAutoPop Pop(rState, 1); return Get<const char*>(rState, iIndex);}
	template<>static ntstd::String Pop<ntstd::String>(LuaState& rState, int iIndex)	{LuaAutoPop Pop(rState, 1); return Get<ntstd::String>(rState, iIndex);}
	template<>static CKeyString    Pop<CKeyString>(LuaState& rState, int iIndex)	{LuaAutoPop Pop(rState, 1); return Get<CKeyString>(rState, iIndex);}


	// Push Types onto the Stack
	// -------------------------
	template<typename TYPE>	static int Push(LuaState&, TYPE);
//	static int Push(const LuaState&) {return 0;}
	
	template<>static int Push<bool>(LuaState& rState, bool Value)									{lua_pushboolean(rState, Value); return 1;}
	template<>static int Push<int>(LuaState& rState, int Value)										{lua_pushnumber(rState, Value); return 1;}
	template<>static int Push<uint32_t>(LuaState& rState, uint32_t Value)								{lua_pushnumber(rState, Value); return 1;}
	template<>static int Push<float>(LuaState& rState, float Value)									{lua_pushnumber(rState, Value); return 1;}
	template<>static int Push<const int&>(LuaState& rState, const int& Value)						{lua_pushnumber(rState, Value); return 1;}
	template<>static int Push<const char*>(LuaState& rState, const char* Value)						{lua_pushstring(rState, Value); return 1;}
	template<>static int Push<ntstd::String>(LuaState& rState, ntstd::String Value)					{lua_pushstring(rState, Value.c_str()); return 1;}
	template<>static int Push<const ntstd::String&>(LuaState& rState, const ntstd::String& Value)	{lua_pushstring(rState, Value.c_str()); return 1;}
	template<>static int Push<CHashedString>(LuaState& rState, CHashedString Value)					{ return lua_pushhashkey(rState, ntStr::GetHashKey(Value)); }
	template<>static int Push<CKeyString>(LuaState& rState, CKeyString Value)	{lua_pushstring(rState, ntStr::GetString(Value)); return 1;}


	// Get Types off the Stack
	// -----------------------
	template<typename TYPE>	static TYPE Default(LuaState&);

	template<>static bool          Default<bool>(LuaState&)				{return false;}
	template<>static int           Default<int>(LuaState&)				{return 0;}
	template<>static uint32_t      Default<uint32_t>(LuaState&)			{return 0;}
	template<>static float         Default<float>(LuaState&)			{return (float) 0; }
	template<>static double        Default<double>(LuaState&)			{return (double) 0;}
	template<>static const char*   Default<const char*>(LuaState&)		{return 0;}
	template<>static ntstd::String Default<ntstd::String>(LuaState&)	{return ntstd::String();}

	// Get Userdata type off the Stack
	// -----------------------
	/*static const ntstd::String GetUserdataType(LuaState& rState, int iIndex)
	{
		lua_getmetatable(rState, iIndex);  
		lua_pushstring(rState, "_type"); 
		lua_gettable(rState,-2); 
		ntstd::String obReturn = lua_tostring(rState, -1);
		lua_pop(rState,2);
		return obReturn;
	}*/
};

template<typename TYPE> int LuaState::Push(TYPE Value) {return LuaValue::Push<TYPE>( *this, Value ); }

}

#endif // LUAVALUE_INC
