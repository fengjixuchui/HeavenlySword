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
                                                                                               
#include "luaplus/LuaPlus.h"
                                                                                               
#include "game/luaremotedebugmessages.h"                                                            

                                                                                              
                                                                                               
///////////////////////////////////////////////////////////////////////////////////////////////
//  Forward Declarations                                                                       
///////////////////////////////////////////////////////////////////////////////////////////////

                                                                                              
///////////////////////////////////////////////////////////////////////////////////////////////
// Statics                                                                                     
///////////////////////////////////////////////////////////////////////////////////////////////
bool   LuaDebugServer::m_bActive = false;
bool   LuaDebugServer::m_bConnected = false;

ntstd::List<LuaDebugServer::BreakPointInfo*>* LuaDebugServer::m_BreakPoints = 0;
ntstd::List<char*>*                           LuaDebugServer::m_GlobalWatches = 0;
int LuaDebugServer::m_iVarID = 1;

SOCKET LuaDebugServer::m_listenSock;
SOCKET LuaDebugServer::m_dbgSock;

LuaDebugServer::EXECUTE_MODE LuaDebugServer::m_eMode = EM_NORMAL;
int LuaDebugServer::m_iCallDepth = 0;

bool LuaDebugServer::m_bAutoLaunch = false;
bool LuaDebugServer::m_bProfiling  = false;


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::StartListening
//! Listen for incoming connections to the debug server.
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::StartListening(bool bAutoLaunch, bool bProfiling)
{
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

	// And we're deactive
	m_bConnected = false;
	m_bActive = false;
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
	UNUSED( pcText );
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
	UNUSED( state );
	UNUSED( ar );
}


//-------------------------------------------------------------------------------------------------
//!
//! LuaDebugServer::SendGlobalInfo
//! Send information on a variable to the debug client
//!
//-------------------------------------------------------------------------------------------------
void LuaDebugServer::SendVariableInfo(const VariableInfo& info, int iParent, bool bGlobal)
{
	UNUSED( info );
	UNUSED( iParent );
	UNUSED( bGlobal );
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
				LuaPlus::LuaObject table(&CLuaGlobal::Get().GetState(), -1);
				info.iTableSize = table.GetTableCount();
				info.pTableItems = NEW LuaDebugServer::VariableInfo[info.iTableSize];

				int iIdx = 0;
				for(LuaPlus::LuaTableIterator it(table); it; ++it)
				{
					LuaPlus::LuaObject &key = it.GetKey();
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
			LuaPlus::LuaObject object(LuaPlus::LuaState::CastState(state), -1);
			LuaPlus::LuaObject metatbl  = object.GetMetaTable();
			LuaPlus::LuaObject metatype = metatbl["_type"];
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
void LuaDebugServer::GetVariableInfo(VariableInfo& info, LuaPlus::LuaObject& object, int iDepth)
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
				for(LuaPlus::LuaTableIterator it(object); it; ++it)
				{
					LuaPlus::LuaObject &key = it.GetKey();
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
			LuaPlus::LuaObject metatbl  = object.GetMetaTable();
			LuaPlus::LuaObject metatype = metatbl["_type"];
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