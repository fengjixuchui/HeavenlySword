//-------------------------------------------------------------------------------------------------
// METHOD:		
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------

#include "ninjaLua.h"

using namespace NinjaLua;

#ifdef PLATFORM_PS3
//#define DISABLE_NINJALUA
#endif

bool LuaObject::CheckMetaTable(CHashedString pcType)
{
	ntAssert(IsTable());

	CHashedString pcThisType;
	Get(CHashedString(HASH_STRING__TYPE), pcThisType);
	if(ntStr::IsNull(pcThisType) || pcThisType == pcType)
		return true;

	CHashedString pcParentType = pcThisType;
	while(!ntStr::IsNull(pcParentType))
	{
		pcParentType = NinjaLua::ParentSet::Get(pcParentType);
		//if(!strcmp(pcParentType, pcType))
		if (pcParentType == pcType)
			return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!		Function call handler
//!		Slower, but perhaps cleaner. 
//!
//------------------------------------------------------------------------------------------

static int LuaFunctionHandler( lua_State* pState )
{
#ifdef DISABLE_NINJALUA
	UNUSED(pState);
	return 0;
#else
	typedef int (*RealHandler)(LuaState&);
	RealHandler fnHandler = (RealHandler) lua_touserdata( pState, lua_upvalueindex(1) );
	return fnHandler( NinjaLua::GetState( pState ) );
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject() : 
	m_State(0),
	m_iRef(LUA_REFNIL)
{
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject( LuaState& rState ) : 
	m_State( &rState ),
	m_iRef(LUA_REFNIL)
{
	// Check that the LuaState is a valid reference. 
	//ntAssert( &NinjaLua::GetState(rState) != &rState );
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject( const LuaObject& obOther ) : 
	m_State( obOther.m_State ),
	m_iRef(LUA_REFNIL)
{
	if( m_State )
	{
		obOther.Push();
		m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject( int iIndex, LuaState& rState, bool bCopy ) : 
	m_State( &rState ),
	m_iRef(LUA_REFNIL)
{
	if( bCopy || iIndex != -1 )
		lua_pushvalue(*m_State, iIndex );
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject( LuaState& rState, bool Boolean) : 
	m_State( &rState ),
	m_iRef(LUA_REFNIL)
{
	lua_pushboolean( *m_State, Boolean );
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject( LuaState& rState, int Number ) : 
	m_State( &rState ),
	m_iRef(LUA_REFNIL)
{
	lua_pushnumber( *m_State, Number );
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject( LuaState& rState, const char* pcString ) : 
	m_State( &rState ),
	m_iRef(LUA_REFNIL)
{
	lua_pushstring( *m_State, pcString );
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------
LuaObject::LuaObject(LuaState& rState, const CHashedString& str) :
	m_State( &rState ),
	m_iRef(LUA_REFNIL)
{
	lua_pushhashkey	( *m_State, ntStr::GetHashKey(str) );
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject( LuaState& rState, lua_Number Number ) : 
	m_State( &rState ),
	m_iRef(LUA_REFNIL)
{
	lua_pushnumber( *m_State, Number );
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject( LuaState& rState, float Number ) : 
	m_State( &rState ),
	m_iRef(LUA_REFNIL)
{
	lua_pushnumber( *m_State, Number );
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::LuaObject(LuaState& state, int (*fnPtr)(LuaState&))
 :  m_State(&state),
	m_iRef(LUA_REFNIL)
{
	lua_pushlightuserdata(state, (void*)fnPtr);
	lua_pushcclosure(state, LuaFunctionHandler, 1);
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::~LuaObject
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject::~LuaObject()
{
	SetNil();
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::CreateTable
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject LuaObject::CreateTable( LuaState& State )
{
	lua_newtable(State);
	return LuaObject( -1, State, false );
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::SetMetaTable
//!	! 
//!
//------------------------------------------------------------------------------------------

void LuaObject::SetMetaTable(LuaObject& tbl)
{
	ntAssert(tbl.IsTable());
	ntAssert(IsTable() || IsUserData());
	
	Push();
	tbl.Push();
	lua_setmetatable(*m_State, -2);
	lua_pop(*m_State, 1);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetMetaTable
//!	! 
//!
//------------------------------------------------------------------------------------------

LuaObject LuaObject::GetMetaTable()
{
	Push();
	if(!lua_getmetatable(*m_State, -1))
		return LuaObject();

	return LuaObject(-1, *m_State, false);
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetMetaTable
//!	Const Version
//!
//------------------------------------------------------------------------------------------
const LuaObject LuaObject::GetMetaTable() const
{
	Push();
	if(!lua_getmetatable(*m_State, -1))
		return LuaObject();

	return LuaObject(-1, *m_State, false);
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::operator=
//!	! 
//!
//------------------------------------------------------------------------------------------

void LuaObject::operator=( const LuaObject& obOther )
{
	m_State = obOther.m_State;
	obOther.Push();
	m_iRef = luaL_ref(*m_State, LUA_REGISTRYINDEX);
	
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::operator==
//!	Test whether this and another object are equal
//!
//------------------------------------------------------------------------------------------

bool LuaObject::operator==( const LuaObject& obOther )
{
	// Push the 2 items on the lua stack
	Push();
	obOther.Push();
	
	// Perform the test
	int iResult = lua_equal( *m_State, -1, -2 );

	// remove the items from the lua stack
	lua_pop( *m_State, 2 );

	// return the result
	return iResult ? true : false;
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsNil
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::IsNil() const 
{ 
	return (m_iRef == LUA_REFNIL) || GetType() == LUA_TNIL; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsBoolean
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::IsBoolean() const 
{ 
	return (m_iRef != LUA_REFNIL) && GetType() == LUA_TBOOLEAN; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsLightUserData
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::IsLightUserData() const 
{ 
	return (m_iRef != LUA_REFNIL) && GetType() == LUA_TLIGHTUSERDATA; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsNumber
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::IsNumber()	const 
{ 
	return (m_iRef != LUA_REFNIL) && GetType() == LUA_TNUMBER; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsString
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::IsString()	const 
{ 
	return (m_iRef != LUA_REFNIL) && GetType() == LUA_TSTRING; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsTable
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::IsTable() const 
{ 
	return (m_iRef != LUA_REFNIL) && GetType() == LUA_TTABLE; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsFunction
//!	! 
//!
//------------------------------------------------------------------------------------------
bool LuaObject::IsFunction() const 
{ 
	return (m_iRef != LUA_REFNIL) && GetType() == LUA_TFUNCTION; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsFunction
//!	! 
//!
//------------------------------------------------------------------------------------------
bool LuaObject::IsCFunction() const
{
	Push();
	bool bRet = lua_iscfunction(*m_State, -1) != 0;
	lua_pop(*m_State, 1);
	return bRet;
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsUserdata
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::IsUserData() const 
{ 
	return (m_iRef != LUA_REFNIL) && GetType() == LUA_TUSERDATA; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::IsThread
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::IsThread() const 
{ 
	return (m_iRef != LUA_REFNIL) && GetType() == LUA_TTHREAD; 
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetBoolean
//!	! 
//!
//------------------------------------------------------------------------------------------

bool LuaObject::GetBoolean() const 
{ 
	Push(); 
	bool Value = (lua_toboolean(*m_State, -1)!=0); 
	lua_pop( *m_State, 1 ); 
	return Value; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetLightUserdata
//!	! 
//!
//------------------------------------------------------------------------------------------

void* LuaObject::GetLightUserData()	const 
{ 
	Push(); 
	void* Value = lua_touserdata(*m_State, -1); 
	lua_pop( *m_State, 1 ); 
	return Value; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetNumber
//!	! 
//!
//------------------------------------------------------------------------------------------

lua_Number LuaObject::GetNumber() const 
{ 
	Push(); 
	lua_Number Value = lua_tonumber(*m_State, -1); 
	lua_pop( *m_State, 1 ); 
	return Value; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetString
//!	! 
//!
//------------------------------------------------------------------------------------------
//#include <libsn.h>
const char*	LuaObject::GetString() const 
{ 
	Push(); 
	const char* Value = lua_tostring(*m_State, -1); 
	lua_pop( *m_State, 1 ); 

	//if(Value && strstr(Value, "LUA") != 0)
	//{
	//	snPause();
	//}

	return Value; 
}

CHashedString	LuaObject::GetHashedString() const
{
	Push();
	CHashedString Value(lua_tohashkey(*m_State, -1));
	lua_pop( *m_State, 1);

	return Value;
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetUserdata
//!	! This is an unsupported feature
//!
//------------------------------------------------------------------------------------------

void* LuaObject::GetUserData()	const 
{ 
	Push(); 
	void* Value = lua_touserdata(*m_State, -1); 
	lua_pop( *m_State, 1 ); 
	return Value; 
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::SetNil
//!	Assigns Nil to the lua object - freeing anything it was holding on to before
//!
//------------------------------------------------------------------------------------------

void LuaObject::SetNil()
{
	if( m_State && m_iRef != LUA_REFNIL )
	{
		luaL_unref(*m_State, LUA_REGISTRYINDEX, m_iRef);
	}

	m_iRef = LUA_REFNIL;
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::XMove
//!	Moves the data contained in this lua object to another luastate
//!
//------------------------------------------------------------------------------------------
void LuaObject::XMove( LuaState& obNewState )
{
	// If the object is already in the state, then just return now. 
	if( obNewState.IsMine( *this ) )
		return;

	// Place this on the stack
	Push();

	// Move onto the other stack
	lua_xmove( *m_State, obNewState, 1 );

	// pop the item off the stack into this lua object
	Pop(obNewState);
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::Clone
//!	
//! Deep copy using the POWER of the lua stack ;)
//------------------------------------------------------------------------------------------

LuaObject LuaObject::Clone( void )
{
	if( IsTable() )
	{
		// Cache the 'C' lua_State
		lua_State* pLua = *m_State;

		// Create the new table, placing it on the stack
		lua_newtable( pLua );

		// Get the current stack top 
		int iTopAtStart = lua_gettop( pLua );

		// Make another reference to the new table
		lua_pushvalue( pLua, -1 );

		// Push the table to copy on the stack
		Push();

		// Place the first key on the stack
		lua_pushnil(pLua);  

		// Get iterating until the copy is complete
		while( iTopAtStart != lua_gettop( pLua ) )
		{
			// Iterate over the table
			while( lua_next(pLua, -2) ) 
			{
				if( lua_istable( pLua, -1 ) )
				{
					// Create the new table, placing it on the stack
					lua_newtable( pLua );

					// take a copy of the key
					lua_pushvalue( pLua, -3 );

					// take a copy of the new table
					lua_pushvalue( pLua, -2 );

					// set the new table into the parent table
					lua_settable( pLua, -7 );

					// swap the new table stack position with that of the one to copy
					lua_insert( pLua, -2 );
					
					// Push another first key on the stack to search the nested table
					lua_pushnil(pLua);  
				}
				else
				{
					// Make a copy of the key
					lua_pushvalue( pLua, -2 );

					// Move the value at -2 back to the stack top
					lua_insert( pLua, -2 );

					// `key' is at index -2 and `value' at index -1
					lua_settable( pLua, -5 );
				}
			}

			// Pop off the old and new tables
			lua_pop( pLua, 2 );
		}
		
		// Return the new table
		return LuaObject(-1, *m_State, false );
	}

	// return a copy of the object
	return LuaObject( *this );
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetEntryCount
//!	
//! Return the number of elements in this 
//------------------------------------------------------------------------------------------

int LuaObject::GetEntryCount( void )
{
	// Number of elements in the table.
	int iCount = 0;

	if( IsTable() )
	{
		// Cache the 'C' lua_State
		lua_State* pLua = *m_State;

		// Push the table to copy on the stack
		Push();

		// Place the first key on the stack
		lua_pushnil(pLua);  

		// Iterate over the table
		while( lua_next(pLua, -2) ) 
		{
			++iCount;
			lua_pop(pLua, 1);
		}
	}

	return iCount;
}

//------------------------------------------------------------------------------------------
//!  global  LuaFunctionErrorHandler
//!
//!  @param [in, out]  pLua lua_State *    
//!
//!  @return int	-	number of values placed on the stack
//!
//!  @remarks 
//!
//!  @author GavB @date 31/07/2006
//------------------------------------------------------------------------------------------

int LuaFunctionErrorHandler( lua_State* L )
{
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");

	if (!lua_istable(L, -1)) 
	{
		lua_pop(L, 1);
		return 1;
	}

	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) 
	{
		lua_pop(L, 2);
		return 1;
	}

	/* pass error message */
	lua_pushvalue(L, 1);  

	/* skip this function and traceback */
	lua_pushinteger(L, 2);  

	/* call debug.traceback */
	lua_call(L, 2, 1);  

	return 1;
}


//------------------------------------------------------------------------------------------
//!
//!	LuaFunction::Caller
//!	
//!
//------------------------------------------------------------------------------------------
int LuaFunction::Caller( int iArgs, int iResults )
{
	// push an error handler on to the stack
	//lua_pushcfunction( *m_State, LuaFunctionErrorHandler );
	//int Error = lua_pcall( *m_State, iArgs, iResults, -1 );

	int Error = lua_pcall( *m_State, iArgs, iResults, 0 );

	if( Error == LUA_ERRRUN )
	{
		ntPrintf("/%s\n", lua_tostring( *m_State, -1 ) );
		lua_pop( *m_State, 1 );
	}
	else if( Error == LUA_ERRMEM )
	{
		ntPrintf("/%s\n", lua_tostring( *m_State, -1 ) );
		lua_pop( *m_State, 1 );
	}
	else if( Error == LUA_ERRERR )
	{
		ntPrintf("/%s\n", lua_tostring( *m_State, -1 ) );
		lua_pop( *m_State, 1 );
	}

	return Error;
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::RegisterRaw
//!	
//!
//------------------------------------------------------------------------------------------
void LuaObject::RegisterRaw( const char* pcName, int (*FuncPtr)(LuaState* pLua) )
{
	if( !m_State ) return;
	Push();
	lua_pushstring( *m_State, pcName );
	lua_pushlightuserdata( *m_State, (void*) FuncPtr );
	lua_pushcclosure( *m_State, &CallTypeRaw::Call, 1);
	lua_settable( *m_State, -3 );
	lua_pop( *m_State, 1 );
}
