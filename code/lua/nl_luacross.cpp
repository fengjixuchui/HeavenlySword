// This include is required whilst LuaPlus and NinjaLua are running side by side. LuaPlus 
// inserts extra data into the lua_State struct, so when the method NinjaLua::CreateThread 
// is called a Luaplus compatible state is created
//#include <luaplus/LuaPlus.h>

#include "ninjalua.h"


//------------------------------------------------------------------------------------------
//!
//!	LuaState::CreateThread
//!	Create a thread and return the new thread object
//!	
//!	The new state of the thread can be obtained from the state of the lua object
//!
//------------------------------------------------------------------------------------------
NinjaLua::LuaObject NinjaLua::LuaState::CreateThread()
{
	lua_State* pL = lua_newthread(m_pLua);
	ntAssert(pL);
	NinjaLua::LuaState*  pNewNinjaState = NT_NEW NinjaLua::LuaState(pL);
	NinjaLua::LuaObject* pThreadObj     = NT_NEW NinjaLua::LuaObject(*pNewNinjaState);
	//lua_setstateuserdata(pL, pNewNinjaState);  // Was this a LuaPlus extension?

	return *pThreadObj;

	/*
	// Let LuaPlus create the thread. 
	LuaPlus::LuaState* pState = LuaPlus::LuaState::CreateThread( LuaPlus::LuaState::CastState( m_pLua ) );

	// Create a new state
	lua_State *pNewState = pState->GetCState() ; //lua_newthread(m_pLua);
	ntAssert( pNewState );

	// Make a Ninja State
	NinjaLua::LuaState* pNewNinjaState = new NinjaLua::LuaState(pNewState);
	ntAssert( pNewNinjaState );

	struct CTempState : LuaPlus::LuaState { void PushThread() { m_threadObj->PushStack(); } };
	((CTempState*)pState)->PushThread();

	// Grab the thread off the lua stack
	NinjaLua::LuaObject obThread = NinjaLua::LuaObject(-1, *this, false);

	// Move the state to the new stack
	obThread.XMove( *pNewNinjaState );

	// Return the 
	return obThread;
	*/
}
