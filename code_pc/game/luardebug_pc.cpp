//------------------------------------------------------------------------------------------
//!                                                                                                                                              
//!	\file luardebug.cpp                                                     
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                               
                                                                                               
///////////////////////////////////////////////////////////////////////////////////////////////
// Includes                                                                                    
///////////////////////////////////////////////////////////////////////////////////////////////
#include "game/luardebug.h"                                                                         
#include "game/luaglobal.h"                                                                         
                                                                                               
#include "game/luaremotedebugmessages.h"                                                            
                                                                                               
#include "lua/ninjalua.h"

#include <shellapi.h>
                                                                                               
                                                                                               
//-------------------------------------------------------------------------------------------------
//  Forward Declarations                                                                       
//-------------------------------------------------------------------------------------------------
static char *GetNetworkErrorString(int iErr);                                                  

                                                                                              
//-------------------------------------------------------------------------------------------------
// Statics                                                                                     
//-------------------------------------------------------------------------------------------------
bool   LuaDebugServer::m_bActive    = false;
bool   LuaDebugServer::m_bConnected = false;
bool   LuaDebugServer::m_bProfiling = false;
bool   LuaDebugServer::m_bAutoLaunch = false;

ntstd::List<LuaDebugServer::BreakPointInfo*>* LuaDebugServer::m_BreakPoints = 0;
ntstd::List<char*>*                           LuaDebugServer::m_GlobalWatches = 0;
int LuaDebugServer::m_iVarID = 1;

SOCKET LuaDebugServer::m_listenSock;
SOCKET LuaDebugServer::m_dbgSock;

HANDLE LuaDebugServer::m_hThread;

LuaDebugServer::EXECUTE_MODE LuaDebugServer::m_eMode = LuaDebugServer::EXECUTE_MODE::EM_NORMAL;
HANDLE LuaDebugServer::m_hStepSemaphore = CreateSemaphore(0, 0, 1, "DebugStepMutex");
int LuaDebugServer::m_iCallDepth = 0;


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::StartListening
//! Listen for incoming connections to the debug server.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::StartListening(bool bAutoLaunch, bool bProfiling)
{
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

	m_bActive = true;
	m_bAutoLaunch = bAutoLaunch;
	m_bProfiling  = bProfiling;

	DWORD threadID;
	m_hThread = CreateThread(0, 0, ThreadFunc, 0, 0, &threadID);

}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::EnableDebugging
//! Set our debugging hooks.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::EnableDebugging()
{
	// Get our hooks into lua!
	lua_sethook(CLuaGlobal::Get().GetState(), DebuggingHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
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
	lua_sethook(CLuaGlobal::Get().GetState(), 0, 0, 0);

	// Clear break points and global watches
	ClearBreakPoints();
	ClearGlobalWatches();

	// Start running code again.
	m_eMode = EM_NORMAL;
	ReleaseSemaphore(m_hStepSemaphore, 1, 0);
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::CleanUp
//! Tidy up after a debugging sessions closes.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::CleanUp()
{
	ClearBreakPoints();
	ClearGlobalWatches();

	// Kill the reader thread in a violent and nasty way.
	if(m_hThread)
		TerminateThread(m_hThread, 0);

	// Close our sockets
	closesocket(m_listenSock);
	closesocket(m_dbgSock);

	// And we're deactive
	m_bConnected = false;
	m_bActive = false;
}


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

			delete pMsg;
		}

		ntPrintf("Debug Server - Connection Closed.\n");

		// Stop Debugging
		DisableDebugging();
	}

	// Debugging Session Over
	return 0;
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::SetBreakpoint
//! Set a breakpoint on a line in a script file.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::SetBreakpoint(const char* pcFile, int iLine)
{
	if(m_BreakPoints == 0)
		m_BreakPoints = NEW ntstd::List<LuaDebugServer::BreakPointInfo*>;

	for(ntstd::List<LuaDebugServer::BreakPointInfo*>::iterator it = m_BreakPoints->begin(); it != m_BreakPoints->end(); it++)
	{
		if((*it)->iLine == iLine && !strcmp(pcFile, (*it)->acFile))
			return;
	}

	m_BreakPoints->push_back(NEW BreakPointInfo(pcFile, iLine));
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
		m_BreakPoints = NEW ntstd::List<LuaDebugServer::BreakPointInfo*>;

	for(ntstd::List<LuaDebugServer::BreakPointInfo*>::iterator it = m_BreakPoints->begin(); it != m_BreakPoints->end(); it++)
	{
		if((*it)->iLine == iLine && !strcmp(pcFile, (*it)->acFile))
		{
			m_BreakPoints->erase(it);
			delete *it;
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
		delete pBP;
	}

	delete m_BreakPoints;
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
		m_GlobalWatches = NEW ntstd::List<char*>;

	// strdup won't work with our mem manager
	char *dup = NEW char[strlen(varname)];
	strcpy(dup, varname);

	m_GlobalWatches->push_back(dup);


	// Grab the global now and send it...
	lua_getglobal(CLuaGlobal::Get().GetState(), varname);
	VariableInfo info;
	GetVariableInfo(info, CLuaGlobal::Get().GetState(), 0);
	strncpy(info.sName, varname, 63);
	lua_pop(CLuaGlobal::Get().GetState(), -11);

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
		delete[] pszWatch;
	}

	delete m_GlobalWatches;
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
		ShellExecute(GetForegroundWindow(), "open", "Z:\\Heavenly_Sword\\code\\tools\\bin\\SciTE\\SciTE.exe", 0, 0, SW_SHOWNORMAL);
		WaitForSingleObject(m_hStepSemaphore, INFINITE);
		m_eMode = EM_STEPOVER;
		m_iCallDepth = 0;
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
//! LuaDebugServer::DebuggingHook
//! Called everytime a line of lua is executed or a function is stepped into or out of.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::DebuggingHook(lua_State* state, lua_Debug* ar)
{
	if (ar->event == LUA_HOOKCALL)
		DebugCallHook();
	else if(ar->event == LUA_HOOKRET)
		DebugReturnHook();
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
void LuaDebugServer::DebugCallHook()
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
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::DebugReturnHook
//! Called when we step out of a function.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::DebugReturnHook()
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

	// 2. Send off info on all the local variables...
	for(int i = 1; true; i++)
	{
		// Get the local at index i in this state.
		const char* varname = lua_getlocal(state, ar, i);

		if(!varname)
			break;

		// Get the type and value of this variable
		VariableInfo info;
		GetVariableInfo(info, state, 0);
		strncpy(info.sName, varname, 63);

		SendVariableInfo(info, 0, false);
	}

	// 3. Send info on all the globals that have been requested to be watched.
	if(m_GlobalWatches)
	{
		m_iVarID = 1;
		for(ntstd::List<char*>::iterator it = m_GlobalWatches->begin(); it != m_GlobalWatches->end(); it++)
		{
			lua_getglobal(state, *it);
			VariableInfo info;
			GetVariableInfo(info, state, 0);
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
//! Returns debugging info about the variable at -1 on the stack.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::GetVariableInfo(VariableInfo& info, lua_State* state, int iDepth)
{
	switch(lua_type(state, -1))
	{
	case LUA_TNIL:
		info.sType = "NIL";
		info.sValue[0] = 0;
		break;
	case LUA_TBOOLEAN:
		info.sType = "Boolean";
		sprintf(info.sValue, "%s", lua_toboolean(state, -1) ? "True" : "False");
	case LUA_TNUMBER:
		info.sType = "Number";
		sprintf(info.sValue, "%.3f", lua_tonumber(state, -1));
		break;
	case LUA_TSTRING:
		info.sType = "String";
		sprintf(info.sValue, "%s", lua_tostring(state, -1));
		break;
	case LUA_TTABLE:
		{
			info.sType = "Table";
			info.sValue[0] = 0;

			if(iDepth < 15)
			{
				LuaObject table(&CLuaGlobal::Get().GetState(), -1);
				info.iTableSize = table.GetTableCount();
				info.pTableItems = NEW LuaDebugServer::VariableInfo[info.iTableSize];

				int iIdx = 0;
				for(LuaTableIterator it(table); it; ++it)
				{
					LuaObject &key = it.GetKey();
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
			else
				info.iTableSize = 0;
		}
		break;
	case LUA_TFUNCTION:
		info.sType = lua_iscfunction(state, -1) ? "C-Function" : "Lua-Function";
		info.sValue[0] = 0;
		break;
	case LUA_TUSERDATA:
		{
			LuaObject object(LuaState::CastState(state), -1);
			LuaObject metatbl  = object.GetMetaTable();
			LuaObject metatype = metatbl["_type"];
			if(metatype.IsString())
				info.sType = metatype.GetString();
			else
				info.sType = "Unknown UserData";
			
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
//! LuaDebugServer::GetVariableInfo
//! Gets info for a luaplus luaobject
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::GetVariableInfo(VariableInfo& info, LuaObject& object, int iDepth)
{
	switch(object.Type())
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

			if(iDepth < 15)
			{
				info.iTableSize = object.GetTableCount();
				info.pTableItems = NEW LuaDebugServer::VariableInfo[info.iTableSize];

				int iIdx = 0;
				for(LuaTableIterator it(object); it; ++it)
				{
					LuaObject &key = it.GetKey();
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
			else
				info.iTableSize = 0;
		}
		break;
	case LUA_TFUNCTION:
		info.sType = object.IsCFunction() ? "C-Function" : "Lua-Function";
		info.sValue[0] = 0;
		break;
	case LUA_TUSERDATA:
		{
			LuaObject metatbl  = object.GetMetaTable();
			LuaObject metatype = metatbl["_type"];
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
//! GetNetworkErrorString
//! Returna string detailing any network failures.
//!
//-------------------------------------------------------------------------------------------------
static char *GetNetworkErrorString(int iErr)
{
	switch(iErr)
	{
	case WSANOTINITIALISED:
		return "WSANOTINITIALISED";
	case WSAENETDOWN:
		return "WSAENETDOWN";
	case WSAEACCES:
		return "WSAEACCES";
	case WSAEINVAL:
		return "WSAEINVAL";
	case WSAEINTR:
		return "WSAEINTR";
	case WSAEINPROGRESS:
		return "WSAEINPROGRESS";
	case WSAEFAULT:
		return "WSAEFAULT";
	case WSAENETRESET:
		return "WSAENETRESET";
	case WSAENOBUFS:
		return "WSAENOBUFS";
	case WSAENOTCONN:
		return "WSAENOTCONN";
	case WSAENOTSOCK:
		return "WSAENOTSOCK";
	case WSAEOPNOTSUPP:
		return "WSAEOPNOTSUPP";
	case WSAESHUTDOWN:
		return "WSAESHUTDOWN";
	case WSAEWOULDBLOCK:
		return "WSAEWOULDBLOCK";
	case WSAEMSGSIZE:
		return "WSAEMSGSIZE";
	case WSAEHOSTUNREACH:
		return "WSAEHOSTUNREACH";
	case WSAECONNABORTED:
		return "WSAECONNABORTED";
	case WSAECONNRESET:
		return "WSAECONNRESET";
	case WSAEADDRNOTAVAIL:
		return "WSAEADDRNOTAVAIL";
	case WSAEAFNOSUPPORT:
		return "WSAEAFNOSUPPORT";
	case WSAEDESTADDRREQ:
		return "WSAEDESTADDRREQ";
	case WSAENETUNREACH:
		return "WSAENETUNREACH";
	case WSAETIMEDOUT :
		return "WSAETIMEDOUT ";
	}

	return "WSAE? - An unknown ntError has occurred.";
}
