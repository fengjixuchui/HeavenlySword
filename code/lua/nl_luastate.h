//------------------------------------------------------------------------------------------
//!
//!	\file luastate.h
//!
//------------------------------------------------------------------------------------------

#ifndef __LUASTATE_H
#define __LUASTATE_H

class CLuaGlobal;

namespace NinjaLua
{
	class LuaObject;

	//-------------------------------------------------------------------------------------------------
	//!
	//! LuaState
	//!
	//-------------------------------------------------------------------------------------------------
	class LuaState
	{
	private:
		// Hidden ctor/dtor
		LuaState(lua_State* pLua);
		LuaState();
		~LuaState();

	public:

		// -----------------------------------------------------------------------------------
		// cast operator to access the C lua_State
		operator lua_State* () const {return m_pLua;}

		// -----------------------------------------------------------------------------------
		//	Return the lua globals table. 
		LuaObject GetGlobals() const;

		// -----------------------------------------------------------------------------------
		// Metatable stuff
		const LuaObject& GetMetaTable(const char*);    /// Return the meta table for a given name tag
		const LuaObject* GetMetaTableP(const char* pcName) { return &GetMetaTable(pcName); }

		// -----------------------------------------------------------------------------------
		// Returns the top of the stack
		int GetTop(void) const;

		// -----------------------------------------------------------------------------------
		// LuaObject Helper stuff
		bool IsMine( const LuaObject& ) const;
		const char* FileAndLine(int iLevel = 1) const;

		// Create a thread and return the new thread object
		LuaObject CreateThread();

		// -----------------------------------------------------------------------------------
		// Making the state compatible with LuaPlus
		// Push a bool on to the stack
		void PushNil()                                       {lua_pushnil(*this);}
		template<typename TYPE> inline int Push(TYPE Value);
		void XMove(LuaState& stateTo, int n)				 {lua_xmove(m_pLua, stateTo.m_pLua, n);}

	public:
		static LuaState* Create(bool bInitStandardLibrary = true, bool bMultithreaded = false);
		static void      Destroy(LuaState*& pState);

	private:
		lua_State*	m_pLua;
		LuaObject*  m_plobGlobals;

		// Allow LuaGlobal to create instances of LuaState
		friend class ::CLuaGlobal;
	};


	// Get a NinjaLua State from the C type
	LuaState& GetState( lua_State* pLua );
};

#endif // __LUASTATE_H

