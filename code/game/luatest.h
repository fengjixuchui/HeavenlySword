/***************************************************************************************************
*
*	DESCRIPTION		Test environment for LuaPlus
*
*	NOTES			Examples of how to use lua
*
***************************************************************************************************/

#ifndef _LUA_TEST
#define _LUA_TEST

#define		TEST_SCRIPT_PATH		LOCAL_ROOT"data\\luatest\\"

#ifdef		_DEBUG
//#define		LUA_DEBUG_MEMORY
//#define		LUA_DEBUG_FUNC_REG
#define		LUA_ASSERT_ON_ERROR
#endif

// Evilness to prevent us from including "LuaPlus/LuaPlus.h"
// or doing "using namespace LuaPlus" in the header
//-----------------------------------------------------------
namespace LuaPlus { class LuaState; class LuaStackObject; }
using LuaPlus::LuaState;
using LuaPlus::LuaStackObject;

struct lua_State;

/***************************************************************************************************
*
*	CLASS			CLuaTestRig
*
*	DESCRIPTION		Object that hides lua testing from the game
*
***************************************************************************************************/
class CLuaTestRig
{
public:
	CLuaTestRig( void );
	~CLuaTestRig( void );

	LuaState*	GetState( void ) { return m_pobState; }

private:
	// test routines
	void	TestThread( void );

	void	TestDump( void );
	void	TestDumpGlobals( void );
	void	TestGarbageCollect( void );
	void	TestNewCall( void );
	void	TestClone( void );
	void	TestPointer( void );
	void	TestTableIterator( void );
	
	void	TestScriptFormat( void );
	void	TestScriptCallback( void );
	void	TestScriptSave( void );
	void	TestScriptArray( void );

	// members
	LuaState*	m_pobState;
};

#endif _LUA_TEST