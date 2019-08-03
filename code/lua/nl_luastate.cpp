//------------------------------------------------------------------------------------------
//!
//!	\file nl_luastate.cpp
//!
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "ninjalua.h"

using namespace NinjaLua;


//------------------------------------------------------------------------------------------
//!  global static  FatalError
//!
//!  @param [in, out]  state lua_State *    
//!
//!  @return int 
//!
//!  @remarks 
//!
//!  @author GavB @date 23/08/2006
//------------------------------------------------------------------------------------------
static int FatalError( lua_State* state )
{
	const char* err = lua_tostring(state, 1);
	ntPrintf(err);
	UNUSED( err );

	return -1;
}


//------------------------------------------------------------------------------------------
//!
//!	GetState
//!	For a given a lua_state, return the associated NinjaLua::LuaState
//!
//------------------------------------------------------------------------------------------
LuaState& NinjaLua::GetState( lua_State* pLua )
{
	lua_pushlightuserdata( pLua, pLua );
	lua_gettable( pLua, LUA_REGISTRYINDEX );
	LuaState* pState = (LuaState*) lua_touserdata( pLua, -1 );
	lua_pop( pLua, 1 );
	ntAssert( pState );
	return *pState;
}


//------------------------------------------------------------------------------------------
//!
//!	SetState
//!	In the 'C' lua_State assign in the registry using the lua_State pointer as an index a
//! pointer to the LuaState
//!
//------------------------------------------------------------------------------------------
static void SetState( lua_State* pLua, LuaState& State )
{
	lua_pushlightuserdata( pLua, pLua );
	lua_pushlightuserdata( pLua, &State );
	lua_settable( pLua, LUA_REGISTRYINDEX );
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::LuaState
//!	
//!
//------------------------------------------------------------------------------------------
LuaState::LuaState(lua_State* pLua) :
	m_pLua(pLua)
{
	// Set the state
	SetState(pLua, *this);

	// Create a globals luaobject
	m_plobGlobals = NT_NEW LuaObject(LUA_GLOBALSINDEX, *this);
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::LuaState
//!	
//!
//------------------------------------------------------------------------------------------
LuaState::LuaState()
{
	m_pLua = lua_open();
	ntAssert(m_pLua);
	SetState(m_pLua, *this);

	// Create a globals luaobject
	m_plobGlobals = NT_NEW LuaObject(LUA_GLOBALSINDEX, *this);

	lua_atpanic(m_pLua, FatalError);
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::~LuaState
//!	Destruction
//!
//------------------------------------------------------------------------------------------
LuaState::~LuaState() 
{
	NT_DELETE(m_plobGlobals);
	m_plobGlobals = 0;

	lua_close( m_pLua );
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::GetGlobals
//!	Return the lua globals table. 
//!
//------------------------------------------------------------------------------------------
LuaObject LuaState::GetGlobals() const
{
	return *m_plobGlobals;
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::GetMetaTable
//!	For a given metatable name, return the metatable object associated with it. If no
//! metatable is found for the given name, one is created. 
//!
//------------------------------------------------------------------------------------------
const LuaObject& LuaState::GetMetaTable( const char* pcName )
{
	int				Index	= -1;
	CHashedString	obName	= CHashedString(pcName);

	// Find the metatable.
	for( int iCount = 0; iCount < CLuaGlobal::s_MtCount; ++iCount )
	{
		if( obName == CLuaGlobal::s_MetaTables[ iCount ].m_DebugId )
		{
			Index = iCount;
			break;
		}
	}

	// If the table wasn't found, then create a new table
	if( Index < 0 )
	{
		// Create a metatable if required
		luaL_newmetatable( m_pLua, pcName );

		// Sanity check.
		ntError( CLuaGlobal::s_MtCount < (int)(sizeof(CLuaGlobal::s_MetaTables) / sizeof(CLuaGlobal::MetaTableUnit)) );

		// Update the metatable counter
		Index = ++CLuaGlobal::s_MtCount;

		// save the metatable in a static table
		CLuaGlobal::s_MetaTables[ Index ].m_obMetaTable		= LuaObject( -1, *this, false );
		CLuaGlobal::s_MetaTables[ Index ].m_DebugId			= obName;
	}

	// Return the metatable
	return CLuaGlobal::s_MetaTables[ Index ].m_obMetaTable;
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::GetTop
//!	Returns the top of the stack
//!
//------------------------------------------------------------------------------------------
int LuaState::GetTop(void) const
{
	return lua_gettop( m_pLua );
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::IsMine
//!	Does the Luaobject belong to the instance of the lua table
//!
//------------------------------------------------------------------------------------------
bool LuaState::IsMine( const LuaObject& obObject ) const
{
	LuaState* pState = obObject.GetState();
	
	// If the state is nil, then return false
	if( !pState ) 
		return false;

	return ( pState->m_pLua == m_pLua );
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::FileAndLine
//!	Whats our file and line?
//!
//------------------------------------------------------------------------------------------
const char* LuaState::FileAndLine(int iLevel) const
{
	static char buf[512];
	lua_Debug dbg;
	lua_getstack (*this, iLevel, &dbg);
	if(lua_getinfo(*this, "Sln", &dbg))
	{
		char acFullPath[256];
		sprintf(acFullPath, "/%s", dbg.source);
		sprintf(buf, "%s(%d)", acFullPath, dbg.currentline);
		return buf;
	}
	
	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::Create                                                            [STATIC]
//!	Create a new luastate.
//!
//------------------------------------------------------------------------------------------
LuaState* LuaState::Create(bool bInitStandardLibrary, bool bMultithreaded)
{
	LuaState* state = NT_NEW LuaState();
	LuaAutoBlock ob( *state );

	if(bMultithreaded)
	{
		ntAssert_p(true, ("Multithreaded Lua Not Supported.\n"));
	}

	if(bInitStandardLibrary)
	{
		luaopen_base(&(**state));
		luaopen_table(&(**state));
		luaopen_string(&(**state));
		luaopen_math(&(**state));
		luaopen_debug(&(**state));
	}

	return state;
}


//------------------------------------------------------------------------------------------
//!
//!	LuaState::Destroy                                                           [STATIC]
//!	Tidy up states when we're done with them.
//!
//------------------------------------------------------------------------------------------
void LuaState::Destroy(LuaState*& state)
{
	NT_DELETE( state );
	state = 0;
}
