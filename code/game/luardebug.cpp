//------------------------------------------------------------------------------------------
//!                                                                                                                                              
//!	\file luardebug.cpp                                                     
//!                                                                                         
//------------------------------------------------------------------------------------------

// Notes:
//
// Need multithreading libraries for the PS3 and Networking should be shifted out into some
// more libraries too.
#ifndef _GOLD_MASTER

                                                                                               
///////////////////////////////////////////////////////////////////////////////////////////////
// Includes                                                                                    
///////////////////////////////////////////////////////////////////////////////////////////////
#include "game/luardebug.h"                                                                         
#include "game/luaglobal.h"          
#include "game/luaremotedebugmessages.h"     
#include "core/timer.h"
                                                                                                                        

#if defined(PLATFORM_PC)
	#include <shellapi.h>
#endif
                                                                                               
                                                                                               
//-------------------------------------------------------------------------------------------------
//  Forward Declarations                                                                       
//-------------------------------------------------------------------------------------------------
//static char *GetNetworkErrorString(int iErr);                                                  

bool operator<(LuaDebugServer::ProfilingKey LHS, LuaDebugServer::ProfilingKey RHS)
{
	return strcmp(LHS.GetName(), RHS.GetName())<0;
}

                                                                                              
//-------------------------------------------------------------------------------------------------
// Statics                                                                                     
//-------------------------------------------------------------------------------------------------
bool   LuaDebugServer::m_bActive    = false;
bool   LuaDebugServer::m_bConnected = false;
bool   LuaDebugServer::m_bProfiling = false;
bool   LuaDebugServer::m_bAutoLaunch = false;

// Breakpoints and Watches
ntstd::List<LuaDebugServer::BreakPointInfo*>* LuaDebugServer::m_BreakPoints = 0;
ntstd::List<char*>*                           LuaDebugServer::m_GlobalWatches = 0;
int LuaDebugServer::m_iVarID = 1;

// Sockets
SOCKET LuaDebugServer::m_listenSock;
SOCKET LuaDebugServer::m_dbgSock;

// Threads and Semaphores
#if defined(PLATFORM_PC)
HANDLE LuaDebugServer::m_hThread;
HANDLE LuaDebugServer::m_hStepSemaphore = CreateSemaphore(0, 0, 1, "DebugStepMutex");
#endif

// Execute Mode
LuaDebugServer::EXECUTE_MODE LuaDebugServer::m_eMode = EM_NORMAL;
int LuaDebugServer::m_iCallDepth = 0;

// Profiling
ntstd::Map<LuaDebugServer::ProfilingKey, LuaDebugServer::ProfilingInfo>* LuaDebugServer::m_profilingData = 0;
ntstd::List< ntstd::pair<LuaDebugServer::ProfilingKey, uint64_t> >* LuaDebugServer::m_profilingStack = 0;

//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::StartListening
//! Listen for incoming connections to the debug server.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::StartListening(bool bAutoLaunch, bool bProfiling)
{
#if defined(PLATFORM_PC)
	// Set up the debug channel listener
	WSADATA obWSAData;
	int iErr = WSAStartup(MAKEWORD(2,0), &obWSAData);

	if(iErr)
	{
		ntPrintf("LuaDebug - WSAStartup Error\n");
		return;
	}

	m_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(m_listenSock == INVALID_SOCKET)
	{
		ntPrintf("LuaDebug - Could not create socket\n");
		return;
	}

	sockaddr_in local;
	local.sin_addr.S_un.S_addr = INADDR_ANY;
	local.sin_port = htons(2500);
	local.sin_family = AF_INET;

	if(bind(m_listenSock, (sockaddr*)&local, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		ntPrintf("LuaDebug - bind Failure\n");
		return;
	}

	if(listen(m_listenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		ntPrintf("LuaDebug - listen Error");
		return;
	}

	DWORD threadID;
	m_hThread = CreateThread(0, 0, ThreadFunc, 0, 0, &threadID);
#endif

	m_bActive = true;
	m_bAutoLaunch = bAutoLaunch;
	m_bProfiling  = bProfiling;

	if(m_bProfiling)
		EnableDebugging();
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::EnableDebugging
//! Set our debugging hooks.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::EnableDebugging()
{
	// Create the profiling info map if profiling is enabled.
	if(m_bProfiling)
	{
		m_profilingData = NT_NEW ntstd::Map<ProfilingKey, ProfilingInfo>;
		m_profilingStack = NT_NEW ntstd::List< ntstd::pair<ProfilingKey, uint64_t> >;
	}

	// Get our hooks into lua!
	lua_sethook(&(*CLuaGlobal::Get().State()), DebuggingHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::DisableDebugging
//! Clear our debugging hooks.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::DisableDebugging()
{
	// Remove the hook
	lua_sethook(&(*CLuaGlobal::Get().State()), 0, 0, 0);

	// Log any profiling data
	if(m_bProfiling && m_profilingData)
	{
		LogProfilingInfo();
		NT_DELETE( m_profilingData );
		m_profilingData = 0;
		NT_DELETE( m_profilingStack );
		m_profilingStack = 0;
	}

	// Clear break points and global watches
	ClearBreakPoints();
	ClearGlobalWatches();

	// Start running code again.
	m_eMode = EM_NORMAL;

#if defined(PLATFORM_PC)
	ReleaseSemaphore(m_hStepSemaphore, 1, 0);
#endif
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::CleanUp
//! Tidy up after a debugging sessions closes.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::CleanUp()
{
	// Log the profiling information
	if(m_bProfiling && m_profilingData)
	{
		LogProfilingInfo();
		NT_DELETE( m_profilingData );
		m_profilingData = 0;
		NT_DELETE( m_profilingStack );
		m_profilingStack = 0;
	}

	ClearBreakPoints();
	ClearGlobalWatches();

#if defined(PLATFORM_PC)
	// Kill the reader thread in a violent and nasty way.
	if(m_hThread)
		TerminateThread(m_hThread, 0);
#endif

#if defined(PLATFORM_PC)
	// Close our sockets
	closesocket(m_listenSock);
	closesocket(m_dbgSock);
#else
	socketclose(m_listenSock);
	socketclose(m_dbgSock);
#endif

	// And we're deactive
	m_bConnected = false;
	m_bActive = false;
}


#if defined(PLATFORM_PC)
//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::ThreadFunc
//! The thread for reading data from the debug client.
//!
//-------------------------------------------------------------------------------------------------
DWORD WINAPI LuaDebugServer::ThreadFunc(LPVOID)
{
	if(!m_bActive)
		return 0;

	while(true)
	{
		// Wait for accept data
		sockaddr_in addr;
		int			addrLen = sizeof(sockaddr_in);
		m_dbgSock = accept(m_listenSock, (sockaddr*)&addr, &addrLen);

		if(m_dbgSock == SOCKET_ERROR)
			ntPrintf("Debug Server - Accept Error.\n");
		else
		{
			ntPrintf("Debug Server - Established Connection.\n");
			if(!m_bProfiling)
				EnableDebugging();
			m_bConnected = true;
		}

		// Check for debug data
		for(;;)
		{
			char buf[1024];
			int iLen = recv(m_dbgSock, buf, 1024, 0);

			if(iLen == SOCKET_ERROR)
			{
				closesocket(m_dbgSock);
				m_bConnected = false;
				break;
			}

			//ntPrintf("Lua Debug Server: received a message of length %d\n", iLen);
			LuaDbgMsg* pMsg = LuaDbgMsg::Decode(buf, iLen);

			if(pMsg)
				pMsg->Despatch();

			NT_DELETE( pMsg );
		}

		ntPrintf("Debug Server - Connection Closed.\n");

		// Stop Debugging
		if(!m_bProfiling)
			DisableDebugging();
	}

	// Debugging Session Over
	return 0;
}
#endif


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::SetBreakpoint
//! Set a breakpoint on a line in a script file.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::SetBreakpoint(const char* pcFile, int iLine)
{
	if(m_BreakPoints == 0)
		m_BreakPoints = NT_NEW ntstd::List<LuaDebugServer::BreakPointInfo*>;

	for(ntstd::List<LuaDebugServer::BreakPointInfo*>::iterator it = m_BreakPoints->begin(); it != m_BreakPoints->end(); it++)
	{
		if((*it)->iLine == iLine && !strcmp(pcFile, (*it)->acFile))
			return;
	}

	m_BreakPoints->push_back(NT_NEW BreakPointInfo(pcFile, iLine));
}

//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::ToggleBreakpoint
//! Toggle a breakpoint on a line in a script file.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::RemoveBreakpoint(const char* pcFile, int iLine)
{
	if(m_BreakPoints == 0)
		m_BreakPoints = NT_NEW ntstd::List<LuaDebugServer::BreakPointInfo*>;

	for(ntstd::List<LuaDebugServer::BreakPointInfo*>::iterator it = m_BreakPoints->begin(); it != m_BreakPoints->end(); it++)
	{
		if((*it)->iLine == iLine && !strcmp(pcFile, (*it)->acFile))
		{
			m_BreakPoints->erase(it);
			NT_DELETE( *it );
			return;
		}
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::IsBreakPoint
//! Returns true if there is a breakpoint on this particular line.
//!
//-------------------------------------------------------------------------------------------------
bool LuaDebugServer::IsBreakPoint(const char* pcFile, int iLine)
{
	if(m_BreakPoints == 0)
		return false;

	char sFile[512];

	// Trim the path
	int i;
	for(i = strlen(pcFile); i > 0 && pcFile[i-1] != '\\' && pcFile[i-1] != '/' && pcFile[i-1] != '@'; i--);

	strcpy(sFile, pcFile+i);

	for(ntstd::List<LuaDebugServer::BreakPointInfo*>::iterator it = m_BreakPoints->begin(); it != m_BreakPoints->end(); it++)
	{
		if((*it)->iLine == iLine && !strcmp(sFile, (*it)->acFile))
			return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::ClearBreakPoints
//! Remove all breakpoints from the scripts.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::ClearBreakPoints()
{
	if(!m_BreakPoints)
		return;

	while(!m_BreakPoints->empty())
	{
		BreakPointInfo* pBP = m_BreakPoints->back();
		m_BreakPoints->pop_back();
		NT_DELETE( pBP );
	}

	NT_DELETE( m_BreakPoints );
	m_BreakPoints = 0;
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::AddGlobalWatch
//! Watch this global for the user
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::AddGlobalWatch(const char* varname)
{
	if(m_GlobalWatches == 0)
		m_GlobalWatches = NT_NEW ntstd::List<char*>;

	// strdup won't work with our mem manager
	char *dup = NT_NEW char[strlen(varname)];
	strcpy(dup, varname);

	m_GlobalWatches->push_back(dup);


	// Grab the global now and send it...
	NinjaLua::LuaObject var(CLuaGlobal::Get().State().GetGlobals()[varname]);
	VariableInfo info;
	GetVariableInfo(info, var, 0);
	strncpy(info.sName, varname, 63);
	lua_pop(&(*CLuaGlobal::Get().State()), -11);

	SendVariableInfo(info, 0, true);
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::ClearGlobalWatches
//! Clear out all the global watches
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::ClearGlobalWatches()
{
	if(!m_GlobalWatches)
		return;

	while(!m_GlobalWatches->empty())
	{
		char* pszWatch = m_GlobalWatches->back();
		m_GlobalWatches->pop_back();
		NT_DELETE_ARRAY( pszWatch );
	}

	NT_DELETE( m_GlobalWatches );
	m_GlobalWatches = 0;
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::Assert
//! Lua has asserted, break in the lua debugger if it's attached, or launch it if specified to in
//! the game.config file.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::Assert()
{
	if(m_bConnected) 
	{
		m_eMode = EM_STEPOVER; 
		m_iCallDepth = 0;
	}
	else if(m_bAutoLaunch)
	{
		ntPrintf("Launching Lua Debugger...");
#if defined(PLATFORM_PC)
		ShellExecute(GetForegroundWindow(), "open", "Z:\\Heavenly_Sword\\code\\tools\\bin\\SciTE\\SciTE.exe", 0, 0, SW_SHOWNORMAL);
		WaitForSingleObject(m_hStepSemaphore, INFINITE);
		m_eMode = EM_STEPOVER;
		m_iCallDepth = 0;
#endif
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::Trace
//! Output some debug messages in the remote debugger
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::Trace(const char* pcText)
{
	char buf[1024];
	LuaDbgMsg_InformOutput msg(pcText);
	msg.Serialise(buf, 1024);

	send(m_dbgSock, buf, msg.Length(), 0);
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::LogProfilingInfo
//! Log the profiling information that we've gathered so far.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::LogProfilingInfo()
{
	// Sort by total time...
	ntstd::List<ProfilingInfo> sorted;

	for(ntstd::Map<ProfilingKey, ProfilingInfo>::iterator it = m_profilingData->begin(); it != m_profilingData->end(); it++)
	{
		sorted.push_back(it->second);
	}

	sorted.sort();


	ntPrintf("Lua Profiling Information\n"
		     "-------------------------\n\n");

	for(ntstd::List<ProfilingInfo>::iterator it = sorted.begin(); it != sorted.end(); it++)
	{
		it->Output();
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::DebuggingHook
//! Called everytime a line of lua is executed or a function is stepped into or out of.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::DebuggingHook(lua_State* state, lua_Debug* ar)
{
	if (ar->event == LUA_HOOKCALL)
		DebugCallHook(state, ar);
	else if(ar->event == LUA_HOOKRET)
		DebugReturnHook(state, ar);
	else if(ar->event == LUA_HOOKLINE)
		DebugLineHook(state, ar);


	if(m_eMode != EM_STEPOVER && m_BreakPoints && m_BreakPoints->size() > 0)
	{
		lua_getinfo(state, "S", ar);
		
		if(IsBreakPoint(ar->source, ar->currentline))
		{
			ntPrintf("LUA: Breakpoint!\n");

			LuaDebugBreak(state, ar);
		}
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::DebugCallHook
//! Called when we step into a function.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::DebugCallHook(lua_State* state, lua_Debug* ar)
{
	if(m_eMode == EM_STEPIN)
	{
		m_iCallDepth = 0;
		m_eMode = EM_STEPOVER;
	}
	else
	{
		m_iCallDepth++;
	}

	// Collect profiling information if asked to.
	if(m_bProfiling)
	{
		if(!lua_getinfo(state, "Sln", ar))
		{

			ntAssert(false);
			return;
		}

		if(ar->name)
		{
			m_profilingStack->push_back(ntstd::pair<ProfilingKey, uint64_t>(ProfilingKey(ar->name), CTimer::GetHWTimer()));
		}
		else
		{
			if(ar->short_src)
			{
				char buf[256];
				sprintf(buf, "%s(%d)", ar->short_src, ar->currentline);

				m_profilingStack->push_back(ntstd::pair<ProfilingKey, uint64_t>(ProfilingKey((char*)&buf), CTimer::GetHWTimer()));
			}
			else
			{
				m_profilingStack->push_back(ntstd::pair<ProfilingKey, uint64_t>(ProfilingKey("<null>"), CTimer::GetHWTimer()));
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::DebugReturnHook
//! Called when we step out of a function.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::DebugReturnHook(lua_State* state, lua_Debug* ar)
{
	if(m_eMode == EM_STEPOUT && m_iCallDepth <= 0)
	{
		m_iCallDepth = 0;
		m_eMode = EM_STEPOVER;
	}
	else
	{
		m_iCallDepth--;
	}

	// Collect profiling information if asked to.
	if(m_bProfiling)
	{
		if(!lua_getinfo(state, "Sln", ar))
		{
			ntAssert(false);
			return;
		}

		uint64_t uiEnd = CTimer::GetHWTimer();
		ntstd::pair<ProfilingKey, uint64_t> stk = m_profilingStack->back();
		m_profilingStack->pop_back();

		ntstd::Map<ProfilingKey, ProfilingInfo>::iterator it;
		it = m_profilingData->find(stk.first);

		if(it == m_profilingData->end())
		{
			ProfilingInfo info(stk.first);
			info.LogCall(uiEnd-stk.second);
			m_profilingData->insert(ntstd::pair<ProfilingKey, ProfilingInfo>(info.GetKey(), info));
		}
		else
		{
			ntAssert((it->first==stk.first));
			it->second.LogCall(uiEnd-stk.second);
		}
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::DebugLineHook
//! Called when we step over a line.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::DebugLineHook(struct lua_State* state, struct lua_Debug* ar)
{
	if(m_eMode == EM_STEPOVER && m_iCallDepth <= 0)
	{
		lua_getinfo(state, "S", ar);

		LuaDebugBreak(state, ar);
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::StepOver
//! Step onto the next line of the script
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::StepOver() 
{
	m_eMode = EM_STEPOVER;
	m_iCallDepth = 0;
	
#if defined(PLATFORM_PC)
	ReleaseSemaphore(m_hStepSemaphore, 1, 0); 
#endif
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::StepInto
//! Step into a function
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::StepInto()
{
	m_eMode = EM_STEPIN;   
	
#if defined(PLATFORM_PC)
	ReleaseSemaphore(m_hStepSemaphore, 1, 0); 
#endif
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::StepOut
//! Step out of this funtion
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::StepOut()  
{
	m_eMode = EM_STEPOUT;  
	m_iCallDepth = 0;
	
#if defined(PLATFORM_PC)
	ReleaseSemaphore(m_hStepSemaphore, 1, 0); 
#endif
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::Go
//! Resume execution of the game and continue the script.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::Go()	   
{
	m_eMode = EM_NORMAL;
	
#if defined(PLATFORM_PC)
	ReleaseSemaphore(m_hStepSemaphore, 1, 0); 
#endif
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::LuaDebugBreak
//! Break on a line of lua script.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::LuaDebugBreak(lua_State* state, lua_Debug* ar)
{
	// 1. Compose a message to the client informing them we've stopped here...
	LuaDbgMsg_InformCurrentLine msg(ar->source, ar->currentline);
	char buf[1024];
	msg.Serialise(buf, 256);

	int iRet = send(m_dbgSock, buf, msg.Length(), 0);

	ntPrintf("Lua Debug BP LINE (%s, %d) LEN[%d\\%d]\n", ar->source, ar->currentline, iRet, msg.Length());
	UNUSED( iRet );

	// 2. Send off info on all the local variables...
	for(int i = 1; true; i++)
	{
		// Get the local at index i in this state.
		int iState = lua_gettop(state);
		if(iState == 0)
		{
			bool bBreak;
			bBreak = true;
		}
		const char* varname = lua_getlocal(state, ar, i);
		NinjaLua::LuaObject var(-1, NinjaLua::GetState(state), false);

		if(!varname || !strcmp(varname, "(*temporary)"))
			break;

		// Get the type and value of this variable
		VariableInfo info;
		GetVariableInfo(info, var, 0);
		strncpy(info.sName, varname, 63);

		SendVariableInfo(info, 0, false);
	}

	// 3. Send info on all the globals that have been requested to be watched.
	if(m_GlobalWatches)
	{
		m_iVarID = 1;
		for(ntstd::List<char*>::iterator it = m_GlobalWatches->begin(); it != m_GlobalWatches->end(); it++)
		{
			NinjaLua::LuaObject var(NinjaLua::GetState(state).GetGlobals()[*it]);
			VariableInfo info;
			GetVariableInfo(info, var, 0);
			strncpy(info.sName, *it, 63);
			lua_pop(state, -11);

			SendVariableInfo(info, 0, true);
		}
	}

	// 4. And send info on our call stack too...
	lua_Debug	dbgInfo;
	int			iLevel = 0;

	// This is the clear message.
	{
		LuaDbgMsg_InformCallstack stackmsg("*", "*", -1);
		stackmsg.Serialise(buf, 1024);
		send(m_dbgSock, buf, stackmsg.Length(), 0);
	}

	// How deep are we?
	while(lua_getstack(state, iLevel++, &dbgInfo))
	{	
		if(!lua_getinfo(state, "Sln", &dbgInfo))
			continue;

		LuaDbgMsg_InformCallstack stackmsg(dbgInfo.name ? dbgInfo.name : "<none>", dbgInfo.short_src, dbgInfo.currentline);
		stackmsg.Serialise(buf, 1024);
		send(m_dbgSock, buf, stackmsg.Length(), 0);
	}


	/////////////////////////////////////////////////////////////////
	// Pause the game while we're on a breakpoint.
	/////////////////////////////////////////////////////////////////
#if defined(PLATFORM_PC)
	DWORD dwResult = WaitForSingleObject(m_hStepSemaphore, INFINITE);

	switch(dwResult)
	{
	case WAIT_OBJECT_0:
		ntPrintf("Semaphore OK\n");
		break;
	case WAIT_TIMEOUT:
		ntPrintf("Semaphore timed out\n");
		break;
	}
#endif
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::SendGlobalInfo
//! Send information on a variable to the debug client
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::SendVariableInfo(const VariableInfo& info, int iParent, bool bGlobal)
{
	char buf[1024];

	// Info the debug client about this variable
	LuaDbgMsg_InformVariable varmsg(info.sName, (const char*)info.sType, info.sValue, m_iVarID++, iParent, bGlobal);
	varmsg.Serialise(buf, 1024);

#ifdef _LUA_DEBUG_NETDATA
	ntPrintf("SEND [[%s]] (%d/%d)\n", buf, strlen(buf), varmsg.Length());
#endif

	send(m_dbgSock, buf, varmsg.Length(), 0);

	// Info the debug client about members of this table variable
	iParent = m_iVarID-1;
	for(int iIdx = 0; iIdx < info.iTableSize; iIdx++)
	{
		SendVariableInfo(info.pTableItems[iIdx], iParent, bGlobal);

		//LuaDbgMsg_InformVariable varsubmsg(info.pTableItems[iIdx].sName, (const char*)info.pTableItems[iIdx].sType, info.pTableItems[iIdx].sValue, iVarID++, iParentID, true);
		//varsubmsg.Serialise(buf, 1024);

		//send(m_dbgSock, buf, varsubmsg.Length(), 0);
		//ntPrintf("SSUB [[%s]] (%d/%d)\n", buf, strlen(buf), varsubmsg.Length());
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::GetVariableInfo
//! Gets info for a ninjalua luaobject
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::GetVariableInfo(VariableInfo& info, const NinjaLua::LuaObject& object, int iDepth)
{
	switch(object.GetType())
	{
	case LUA_TNIL:
		info.sType = "NIL";
		info.sValue[0] = 0;
		break;
	case LUA_TBOOLEAN:
		info.sType = "Boolean";
		sprintf(info.sValue, "%s", object.GetBoolean() ? "True" : "False");
		break;
	case LUA_TNUMBER:
		info.sType = "Number";
		sprintf(info.sValue, "%.3f", object.GetFloat());
		break;
	case LUA_TSTRING:
		info.sType = "String";
		sprintf(info.sValue, "%s", object.GetString());
		break;
	case LUA_TTABLE:
		{
			info.sType = "Table";
			info.sValue[0] = 0;
			info.iTableSize = 0;

			if(iDepth < 15)
			{
				for(NinjaLua::LuaIterator it(object); it; ++it)
				{
					info.iTableSize++;
				}

				info.pTableItems = NT_NEW LuaDebugServer::VariableInfo[info.iTableSize];

				int iIdx = 0;
				for(NinjaLua::LuaIterator it(object); it; ++it)
				{
					const NinjaLua::LuaObject& key = it.GetKey();
					if(key.IsString())
						sprintf(info.pTableItems[iIdx].sName, "%s", key.GetString());
					else if(key.IsInteger())
						sprintf(info.pTableItems[iIdx].sName, "[%d]", key.GetInteger());
					else
						sprintf(info.pTableItems[iIdx].sName, "<BAD>");

					GetVariableInfo(info.pTableItems[iIdx], it.GetValue(), iDepth+1);
					iIdx++;
				}
			}
		}
		break;
	case LUA_TFUNCTION:
		info.sType = object.IsCFunction() ? "C-Function" : "Lua-Function";
		info.sValue[0] = 0;
		break;
	case LUA_TUSERDATA:
		{
			const NinjaLua::LuaObject metatbl  = object.GetMetaTable();
			NinjaLua::LuaObject metatype = metatbl["_type"];
			if(metatype.IsString())
				info.sType = metatype.GetString();
			else
				info.sType = "Unknown UserData";
			
			// Handle this generically in the future... - JML TODO:
			info.sValue[0] = 0;
		}
		break;
	case LUA_TLIGHTUSERDATA:
		info.sType = "LightUserData";
		info.sValue[0] = 0;
		break;
	case LUA_TTHREAD:
		info.sType = "Thread";
		info.sValue[0] = 0;
		break;
	default:
		info.sType = "Error";
		info.sValue[0] = 0;
		break;
	}
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::ProfilingInfo::Output
//! Output a line of profiling information.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::ProfilingInfo::Output()
{
	if(m_iCalls > 0)
	{
		int64_t uiAvg = m_uiTime / m_iCalls;
		ntPrintf("%64s, %5d Calls, Total %.2fms, Avg %.2fms, Max %.2fms, Min %.2fms\n", m_key.GetName(), m_iCalls, 
			                                                                            m_uiTime   *CTimer::GetHWTimerPeriod()*1000.f, 
			                                                                            uiAvg      *CTimer::GetHWTimerPeriod()*1000.f, 
                                                                                        m_uiMaxTime*CTimer::GetHWTimerPeriod()*1000.f,
                                                                                        m_uiMinTime*CTimer::GetHWTimerPeriod()*1000.f);
		UNUSED( uiAvg );
	}
}


#endif //_GOLD_MASTER
