//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file luardebug.cpp                                                                     
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _GOLD_MASTER
#ifndef _LUA_RDEBUGGER_H
#define _LUA_RDEBUGGER_H

//------------------------------------------------------------------------------------------
// Required Includes                                         
//------------------------------------------------------------------------------------------
#if defined(PLATFORM_PC)
	#include <winsock2.h>
#elif defined(PLATFORM_PS3)
	#include <sys/socket.h>
	typedef int SOCKET;
#endif

#include "lua/ninjalua.h"
#include "game/luaglobal.h"


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDebugServer                                                                 [STATIC] 
//! Provides functionality for remote debugging of lua scripts active in the game.          
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDebugServer
{
public:
	// Server Management
	//-----------------------
	static void StartListening(bool bAutoLauch, bool bProfiling);
	static void EnableDebugging();
	static void DisableDebugging();
	static void CleanUp();

	// Breakpoint Methods.
	//-----------------------
	static void SetBreakpoint(const char* pcFile, int iLine);
	static void RemoveBreakpoint(const char* pcFile, int iLine);
	static bool IsBreakPoint(const char* pcFile, int iLine);
	static void ClearBreakPoints();

	// Global Watch Requests
	//-----------------------
	static void AddGlobalWatch(const char* varname);
	static void ClearGlobalWatches();

	// Asserts and Traces
	//-----------------------
	static void Assert();
	static bool AutoLaunchEnabled() {return m_bAutoLaunch;}
	static void EnableAutoLaunch(bool b) {m_bAutoLaunch = b;}
	static void Trace(const char* pcText);

	// Profiling
	//-----------------------
	static void LogProfilingInfo();

// Structures and Enumerations
//-----------------------------
private:
	// --------------------------------------------------------------------------
	// BreakPointInfo
	// Holds information about active breakpoints.
	// --------------------------------------------------------------------------
	struct BreakPointInfo    
	{
		BreakPointInfo(const char* pc, int i) {strcpy(acFile, pc); iLine = i;}
		char acFile[256];
		int iLine;
	};


	// --------------------------------------------------------------------------
	// Holds information about a watched variable.
	// --------------------------------------------------------------------------
	struct VariableInfo
	{
		VariableInfo() {pTableItems = 0; iTableSize = 0;}
		~VariableInfo() {if(pTableItems) NT_DELETE_ARRAY( pTableItems );}

		char sName[64];     // Our name,
		const char* sType; // Our type,
		char sValue[64];  // Our value or...

		// Table Items if we're a table.
		int			  iTableSize;
		VariableInfo* pTableItems;
	};


	// --------------------------------------------------------------------------
	// ProfilingKey
	// Key for using ProfilingInfo in maps
	// --------------------------------------------------------------------------
	class ProfilingKey
	{
	public:
		ProfilingKey(const char* pcName)               {strncpy(m_acName, pcName, MAXPROFILINGNAME);}
		const char* GetName() const                    {return (const char*)&m_acName;}
		bool operator==(const ProfilingKey& rhs) const {return strcmp(m_acName,rhs.m_acName)==0;}

	private:
		static const int MAXPROFILINGNAME = 128;
		char m_acName[MAXPROFILINGNAME];
	};


	// --------------------------------------------------------------------------
	//  ProfilingInfo
	//  Contains profiling Statitistics fora lua script function.
	// --------------------------------------------------------------------------
	class ProfilingInfo
	{
	public:
		ProfilingInfo(ProfilingKey key) : m_key(key) {m_iCalls = 0; m_uiTime=m_uiMaxTime=0;m_uiMinTime=_LLONG_MAX;}
		
		void LogCall(int64_t uiDuration) 
		{
			// Log a call to this script function
			m_iCalls++;
			m_uiTime += uiDuration;

			if(uiDuration > m_uiMaxTime)
				m_uiMaxTime = uiDuration;
			if(uiDuration < m_uiMinTime)
				m_uiMinTime = uiDuration;
		}
		void Output();

		bool operator<(ProfilingInfo& rhs) {return m_uiTime>rhs.m_uiTime;}
		const ProfilingKey& GetKey()       {return m_key;}

	private:
		ProfilingKey  m_key;
		int           m_iCalls;
		int64_t       m_uiTime;
		int64_t       m_uiMaxTime;
		int64_t       m_uiMinTime;
	};


	// --------------------------------------------------------------------------
	// EXECUTE_MODE
	// Enumeration of stepping modes.
	// --------------------------------------------------------------------------
	enum EXECUTE_MODE
	{
		EM_NORMAL,		// Running normally,
		EM_STEPOVER,    // Step over next line,
		EM_STEPIN,      // Step into function,
		EM_STEPOUT      // Step out of function.
	};


// Private Methods
//-----------------
private:
	// Debug Hooks.
	//-----------------------
	static void DebuggingHook(struct lua_State* state, struct lua_Debug* ar);
	static void DebugLineHook(struct lua_State* state, struct lua_Debug* ar);
	static void DebugCallHook(struct lua_State* state, struct lua_Debug* ar);
	static void DebugReturnHook(struct lua_State* state, struct lua_Debug* ar);

	// Stepping Mechanisms.
	//-----------------------
	friend class LuaDbgMsg_StepOver;
	friend class LuaDbgMsg_StepInto;
	friend class LuaDbgMsg_StepOut;
	friend class LuaDbgMsg_Go;
	static void StepOver();
	static void StepInto();
	static void StepOut();
	static void Go();

	// Line breaking as a result of a breakpoint or step-over/in/out.
	//----------------------------------------------------------------
	static void LuaDebugBreak(lua_State* state, lua_Debug* ar);

	// Watch variable information.
	//-----------------------------
	//static void GetVariableInfo(VariableInfo& info, lua_State* state, int iDepth);
	static void GetVariableInfo(VariableInfo& info, const NinjaLua::LuaObject& object, int iDepth);
	static void SendVariableInfo(const VariableInfo& info, int iParent, bool bGlobal);

	// Our reader-socket thread function.
	//------------------------------------
#if defined(PLATFORM_PC)
	static DWORD WINAPI ThreadFunc(LPVOID);
#endif


// Members
//---------
private:
	static bool m_bActive;	      // Is the debug server up and ready to use?
	static bool m_bConnected;    // Do we have a live client connection?
	static bool m_bAutoLaunch;  // Auto-launch debugger on asserts?
	static bool m_bProfiling;  // Is script profiling enabled?

	// Breakpoints
	static ntstd::List<BreakPointInfo*>* m_BreakPoints;

	// Global Watches
	static ntstd::List<char*>* m_GlobalWatches;
	static int m_iVarID;

	// Sockets
	static SOCKET m_listenSock;
	static SOCKET m_dbgSock;

	// Our reader thread handle
#if defined(PLATFORM_PC)
	static HANDLE m_hThread;          // We run the debugger on another thread as we pause the game thread...
	static HANDLE m_hStepSemaphore;   // ... with this semaphore.
#endif

	// Mode of execution
	static EXECUTE_MODE m_eMode;      // Step-In / Step-Out / Step-Over?
	static int          m_iCallDepth; // Track the relative call depth so we don't step over and end up in a different function...

	// Profiling Info
	static ntstd::Map<ProfilingKey, ProfilingInfo>*            m_profilingData;  // Map of functions and timings.
	static ntstd::List< ntstd::pair<ProfilingKey, uint64_t> >* m_profilingStack; // Current function stack for profiling.

	friend bool operator<(LuaDebugServer::ProfilingKey LHS, LuaDebugServer::ProfilingKey RHS); // Comparison operator for profiling map.
};

#endif // _LUA_RDEBUGGER_H
#endif // _GOLD_MASTER
