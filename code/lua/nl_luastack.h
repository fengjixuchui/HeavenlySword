#ifndef __LUASTACK_H
#define __LUASTACK_H

class CLuaGlobal;

namespace NinjaLua
{
class LuaObject;

//-------------------------------------------------------------------------------------------------
//!
//! LuaStackObject
//!
//-------------------------------------------------------------------------------------------------
class LuaStackObject
{
public:
	LuaStackObject( LuaState& obState, int Index ) :
		m_obState( obState ),
		m_Index( Index )
	{
	}

	// Return a LuaObject of the stack item
	operator LuaObject (void) const {return LuaObject(m_Index, m_obState, false );}

	// Determine the object type
	bool IsNone() const						{return lua_type(m_obState, m_Index) == LUA_TNONE;}
	bool IsNil() const						{return lua_type(m_obState, m_Index) == LUA_TNIL;}
	bool IsBoolean() const					{return lua_type(m_obState, m_Index) == LUA_TBOOLEAN;}
	bool IsLightUserData() const			{return lua_type(m_obState, m_Index) == LUA_TLIGHTUSERDATA;}
	bool IsInteger() const					{return lua_type(m_obState, m_Index) == LUA_TNUMBER;}
	bool IsNumber() const					{return lua_type(m_obState, m_Index) == LUA_TNUMBER;}
	bool IsString()	const					{return lua_type(m_obState, m_Index) == LUA_TSTRING;}
	bool IsTable() const					{return lua_type(m_obState, m_Index) == LUA_TTABLE;}
	bool IsFunction() const					{return lua_type(m_obState, m_Index) == LUA_TFUNCTION;}
	bool IsUserData() const					{return lua_type(m_obState, m_Index) == LUA_TUSERDATA;}
	bool IsThread()	const					{return lua_type(m_obState, m_Index) == LUA_TTHREAD;}
	template<class TYPE> bool Is() const	{return LuaValue::Is<TYPE>( m_obState, m_Index );}

	// Access the type
	bool 			GetBoolean() const			{return lua_toboolean( m_obState, m_Index ) != 0;}
	void*			GetLightUserData() const	{return (void*) lua_touserdata( m_obState, m_Index );}
	lua_Number		GetNumber() const			{return (lua_Number) lua_tonumber( m_obState, m_Index );}
	const char*		GetString()	const			{return (const char*) lua_tostring( m_obState, m_Index );}
	float			GetFloat() const			{return (float) lua_tonumber( m_obState, m_Index );}
	int				GetInteger() const			{return (int) lua_tonumber( m_obState, m_Index );}
	void* 			GetUserData() const			{return (void*) lua_touserdata( m_obState, m_Index );}
	
	// Another method to allow access to the stack item
	template<class TYPE> TYPE	Get() const		{return LuaValue::Get<TYPE>( m_obState, m_Index );}

	// Get the State
	LuaState&	GetState(void) const {return m_obState;}

	// Get the stack index
	int			GetIndex(void) const {return m_Index;}

private:
	LuaState&	m_obState;
	int			m_Index;
};

//-------------------------------------------------------------------------------------------------
//!
//! LuaStack
//!
//-------------------------------------------------------------------------------------------------
class LuaStack
{
public:
	// -----------------------------------------------------------------------------------
	// constructor
	LuaStack(LuaState& obStack) :
		m_obState( obStack )
	{
	}

	LuaStack(LuaState* pobStack) :
		m_obState( *pobStack )
	{
	}

public:

	// Return a light weight stack object. 
	LuaStackObject operator[] (int Index) const {return LuaStackObject( m_obState, Index);}


private:
	LuaState& m_obState;
};

};

#endif // __LUASTATE_H

