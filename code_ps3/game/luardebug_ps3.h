//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file luardebug.cpp                                                                     
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _LUA_RDEBUGGER_PS3_H
#define _LUA_RDEBUGGER_PS3_H

///////////////////////////////
// Required Includes           
///////////////////////////////
#include "core/net.h"			// for SOCKET
#include "game/luaglobal.h"

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDebugServer                                                               [STATIC]   
//! Provides functionality for remote debugging of lua scripts active in the game.          
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDebugServer
{
public:
	static void StartListening(bool bAutoLauch, bool bProfiling);
	static void EnableDebugging();
	static void DisableDebugging();

	static void CleanUp();

	// Breakpoint Methods.
	//static void ToggleBreakpoint(const char* pcFile, int iLine);
	static void SetBreakpoint(const char* pcFile, int iLine);
	static void RemoveBreakpoint(const char* pcFile, int iLine);
	static bool IsBreakPoint(const char* pcFile, int iLine);
	static void ClearBreakPoints();

	// Global Watch Requests
	static void AddGlobalWatch(const char* varname);
	static void ClearGlobalWatches();

	// Asserts and Traces
	static void Assert();
	static bool AutoLaunchEnabled() {return m_bAutoLaunch;}
	static void EnableAutoLaunch(bool b) {m_bAutoLaunch = b;}
	static void Trace(const char* pcText);

// Structures and Enumerations
private:
	struct BreakPointInfo    // Holds information about active breakpoints.
	{
		BreakPointInfo(const char* pc, int i) {strcpy(acFile, pc); iLine = i;}
		char acFile[256];
		int iLine;
	};

	struct VariableInfo    // Holds information about a watched variable.
	{
		VariableInfo() {pTableItems = 0; iTableSize = 0;}
		~VariableInfo() {if(pTableItems) delete[] pTableItems;}

		char sName[64];     // Our name,
		const char* sType; // Our type,
		char sValue[64];  // Our value or...

		// Table Items if we're a table.
		int			  iTableSize;
		VariableInfo* pTableItems;
	};

	enum EXECUTE_MODE
	{
		EM_NORMAL,		// Running normally,
		EM_STEPOVER,    // Step over next line,
		EM_STEPIN,      // Step into function,
		EM_STEPOUT      // Step out of function.
	};

// Private Methods
private:
	// Debug Hooks.
	static void DebuggingHook(struct lua_State* state, struct lua_Debug* ar);
	static void DebugCallHook();
	static void DebugReturnHook();
	static void DebugLineHook(struct lua_State* state, struct lua_Debug* ar);

	// Stepping Mechanisms.
	friend class LuaDbgMsg_StepOver;
	friend class LuaDbgMsg_StepInto;
	friend class LuaDbgMsg_StepOut;
	friend class LuaDbgMsg_Go;
	static void StepOver() {m_eMode = EM_STEPOVER; m_iCallDepth = 0;}
	static void StepInto() {m_eMode = EM_STEPIN;   }
	static void StepOut()  {m_eMode = EM_STEPOUT;  m_iCallDepth = 0;}
	static void Go()	   {m_eMode = EM_NORMAL;   }

	// Line breaking as a result of a breakpoint or step-over/in/out.
	static void LuaDebugBreak(lua_State* state, lua_Debug* ar);

	// Watch variable information.
	static void GetVariableInfo(VariableInfo& info, lua_State* state, int iDepth);
	static void GetVariableInfo(VariableInfo& info, LuaPlus::LuaObject& object, int iDepth);
	static void SendVariableInfo(const VariableInfo& info, int iParent, bool bGlobal);

private:
	static bool   m_bActive;	// Is the debug server active?
	static bool   m_bConnected; // Do we have a connection?

	// Breakpoints
	static ntstd::List<BreakPointInfo*>* m_BreakPoints;

	// Global Watches
	static ntstd::List<char*>* m_GlobalWatches;
	static int m_iVarID;

	// Sockets
	static SOCKET m_listenSock;
	static SOCKET m_dbgSock;


	// Mode of execution
	static EXECUTE_MODE m_eMode;
	static int          m_iCallDepth;     // Track the relative call depth so we don't step over and end up in a different function...

	// Auto-lauch debugger if we ntAssert?
	static bool m_bAutoLaunch;
};

#endif // _LUA_RDEBUGGER_PS3_H
