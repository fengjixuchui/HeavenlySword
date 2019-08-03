//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file luaremotedebugmessages.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _GOLD_MASTER

//------------------------------------------------------------------------------------------
// For the game
//------------------------------------------------------------------------------------------
#ifndef _LUA_EDITOR
	#include "game/luardebug.h"
	#include "game/luaglobal.h"
	#include "game/LuaRemoteDebugMessages.h"
	#include "core/ClassFactory.h"
#endif //!_LUA_EDITOR


//------------------------------------------------------------------------------------------
// For SciTE - Not used by the game
//------------------------------------------------------------------------------------------
#ifdef _LUA_EDITOR
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <stdio.h>
	#include <fcntl.h>
	#include <stdarg.h>
	#include <sys/stat.h>
	#include <time.h>
	#include <windows.h>
	#include <commctrl.h>
	#include <direct.h>
	#include "Z:\HS\tools\luatools\NinjaScite\scintilla\include\Platform.h"

	#include "Z:\HS\code\game\LuaRemoteDebugMessages.h"
	#define ntstd std
	#include <map>
	#define Map map
	#define NT_NEW new
	#define ntAssert(x)
	#include "Z:\HS\code\core\ClassFactory.h"

	#include "Z:\HS\tools\luatools\NinjaScite\scite\src\SciTE.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scintilla\include\PropSet.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scintilla\include\Accessor.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scintilla\include\WindowAccessor.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scintilla\include\KeyWords.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scintilla\include\Scintilla.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scintilla\include\ScintillaWidget.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scintilla\include\SciLexer.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scite\src\Extender.h"
	#include "Z:\HS\tools\luatools\NinjaScite\scite\src\SciTEBase.h"

	SciTEBase* LuaDbgMsg::m_pSciTE;
#endif //_LUA_EDITOR


//------------------------------------------------------------------------------------------
// Register Debug Messages
//------------------------------------------------------------------------------------------
DECLARE_CLASS_FACTORY(CFactoryStorageArray, LuaDbgMsg, RDBGMSGTYPE, LuaDbgMsgFactory);

#define REGISTER_DBGMSG(msg) \
	REGISTER_CLASS(LuaDbgMsg, RDBGMSGTYPE, CFactoryStorageArray, LuaDbgMsgFactory, msg, msg::GetType())

REGISTER_DBGMSG(LuaDbgMsg_SetBreakPoint);
REGISTER_DBGMSG(LuaDbgMsg_RemoveBreakPoint);
REGISTER_DBGMSG(LuaDbgMsg_InformCurrentLine);
REGISTER_DBGMSG(LuaDbgMsg_StepOver);
REGISTER_DBGMSG(LuaDbgMsg_StepInto);
REGISTER_DBGMSG(LuaDbgMsg_StepOut);
REGISTER_DBGMSG(LuaDbgMsg_Go);
REGISTER_DBGMSG(LuaDbgMsg_InformVariable);
REGISTER_DBGMSG(LuaDbgMsg_InformCallstack);
REGISTER_DBGMSG(LuaDbgMsg_ReloadScript);
REGISTER_DBGMSG(LuaDbgMsg_InformOutput);
REGISTER_DBGMSG(LuaDbgMsg_RequestGlobal);
REGISTER_DBGMSG(LuaDbgMsg_ExecuteScript);


////////////////////////////////////////////////////////////////////////
//
// Decode
//
////////////////////////////////////////////////////////////////////////
LuaDbgMsg *LuaDbgMsg::Decode(char* buf, int iLen)
{
	LuaDbgMsg* pMsg = LuaDbgMsgFactory->GetInstance((RDBGMSGTYPE)buf[0]);

	if(pMsg)
		pMsg->Deserialise(buf+1, iLen-1);

	return pMsg;
}


////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_SetBreakPoint
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_SetBreakPoint::Serialise(char* buf, int) const
{
	buf[0] = (char)GetType();
	strcpy(&buf[1], m_acFile);
	int ifilelen = strlen(m_acFile);
	*((int*)&buf[2+ifilelen]) = m_iLine;
}

void LuaDbgMsg_SetBreakPoint::Deserialise(char* buf, int)
{
	strcpy((char*)m_acFile, buf);
	m_iLine = *((int*)(buf + strlen(buf)+1));
}

void LuaDbgMsg_SetBreakPoint::Despatch() const
{
#ifndef _LUA_EDITOR
	LuaDebugServer::SetBreakpoint(m_acFile, m_iLine);
#endif
}


////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_RemoveBreakPoint
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_RemoveBreakPoint::Serialise(char* buf, int) const
{
	buf[0] = (char)GetType();
	strcpy(&buf[1], m_acFile);
	int ifilelen = strlen(m_acFile);
	*((int*)&buf[2+ifilelen]) = m_iLine;
}

void LuaDbgMsg_RemoveBreakPoint::Deserialise(char* buf, int)
{
	strcpy((char*)m_acFile, buf);
	m_iLine = *((int*)(buf + strlen(buf)+1));
}

void LuaDbgMsg_RemoveBreakPoint::Despatch() const
{
#ifndef _LUA_EDITOR
	if(m_iLine > 0)
		LuaDebugServer::RemoveBreakpoint(m_acFile, m_iLine);
	else
		LuaDebugServer::ClearBreakPoints();
#endif
}

////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_RemoveBreakPoint
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_InformCurrentLine::Serialise(char* buf, int) const
{
	char sFile[512];

	strcpy(sFile, m_acFile);

	buf[0] = (char)GetType();
	strcpy(&buf[1], sFile);
	int ifilelen = strlen(sFile);
	*((int*)&buf[2+ifilelen]) = m_iLine;
}

void LuaDbgMsg_InformCurrentLine::Deserialise(char* buf, int)
{
	strcpy((char*)m_acFile, buf);
	m_iLine = *((int*)(buf + strlen(buf)+1));
}

void LuaDbgMsg_InformCurrentLine::Despatch() const
{
#ifdef _LUA_EDITOR
	m_pSciTE->SetExecuteLine(m_acFile, m_iLine-1);
#endif
}


////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_StepOver
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_StepOver::Despatch() const
{
#ifndef _LUA_EDITOR
	LuaDebugServer::StepOver();
#endif
}

////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_StepInto
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_StepInto::Despatch() const
{
#ifndef _LUA_EDITOR
	LuaDebugServer::StepInto();
#endif
}

////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_StepOut
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_StepOut::Despatch() const
{
#ifndef _LUA_EDITOR
	LuaDebugServer::StepOut();
#endif
}

////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_Go
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_Go::Despatch() const
{
#ifndef _LUA_EDITOR
	LuaDebugServer::Go();
#endif
}

////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_InformVariable
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_InformVariable::Serialise(char* buf, int) const
{
	buf[0] = (char)GetType();
	strcpy(&buf[1], m_acName);
	int iLen = strlen(m_acName);
	strcpy(&buf[2+iLen], m_acType);
	iLen += strlen(m_acType);
	strcpy(&buf[3+iLen], m_acValue);
	iLen += strlen(m_acValue);
	*((int*)&buf[iLen+4])               = m_iID;
	*((int*)&buf[iLen+4+sizeof(int)])   = m_iParent;
	*((bool*)&buf[iLen+4+sizeof(int)*2]) = m_bGlobal;
}

void LuaDbgMsg_InformVariable::Deserialise(char* buf, int)
{
	strcpy((char*)m_acName, buf);
	int iLen = strlen(buf);
	strcpy((char*)m_acType, buf + iLen + 1);
	iLen += strlen(buf+iLen+1);
	strcpy((char*)m_acValue, buf + iLen + 2);
	iLen += strlen(buf+iLen+2);
	m_iID     = *((int*)&buf[iLen + 3]);
	m_iParent = *((int*)&buf[iLen + 3 + sizeof(int)]);
	m_bGlobal = *((bool*)&buf[iLen + 3 + sizeof(int)*2]);
}

void LuaDbgMsg_InformVariable::Despatch() const
{
#ifdef _LUA_EDITOR
	m_pSciTE->SetVariableInfo(*this);
#endif
}


////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_InformVariable
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_InformCallstack::Serialise(char* buf, int) const
{
	buf[0] = (char)GetType();
	strcpy(&buf[1], m_acFunc);
	int iLen = strlen(m_acFunc);
	strcpy(&buf[2+iLen], m_acFile);
	iLen += strlen(m_acFile);
	*((int*)&buf[iLen+3]) = m_iLine;
}

void LuaDbgMsg_InformCallstack::Deserialise(char* buf, int)
{
	strcpy((char*)m_acFunc, buf);
	int iLen = strlen(m_acFunc);
	strcpy((char*)m_acFile, buf + iLen + 1);
	iLen += strlen(m_acFile);
	m_iLine  = *((int*)&buf[iLen + 2]);
}

void LuaDbgMsg_InformCallstack::Despatch() const
{
#ifdef _LUA_EDITOR
	if(m_iLine > 0)
		m_pSciTE->AddCallStack(*this);
	else
		m_pSciTE->ClearCallStack();
#endif
}

////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_ReloadScript
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_ReloadScript::Serialise(char* buf, int) const
{
	buf[0] = (char)GetType();
	strcpy(&buf[1], m_acFile);
}

void LuaDbgMsg_ReloadScript::Deserialise(char* buf, int)
{
	strcpy((char*)m_acFile, buf);
}

void LuaDbgMsg_ReloadScript::Despatch() const
{
#ifndef _LUA_EDITOR
	ntPrintf("LUA: Reload Script %s\n", m_acFile);

	//CRITICAL_SECTION
	CLuaGlobal::Get().ReloadNextUpdate(m_acFile);
#endif
}


////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_InformOutput
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_InformOutput::Serialise(char* buf, int) const
{
	buf[0] = (char)GetType();
	strcpy(&buf[1], m_acMsg);
}

void LuaDbgMsg_InformOutput::Deserialise(char* buf, int)
{
	strcpy((char*)m_acMsg, buf);
}


void LuaDbgMsg_InformOutput::Despatch() const
{
#ifdef _LUA_EDITOR
	m_pSciTE->Trace(m_acMsg);
#endif
}


////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_RequestGlobal
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_RequestGlobal::Serialise(char* buf, int) const
{
	buf[0] = (char)GetType();
	strcpy(&buf[1], m_acName);
}


void LuaDbgMsg_RequestGlobal::Deserialise(char* buf, int)
{
	strcpy((char*)m_acName, buf);
}


void LuaDbgMsg_RequestGlobal::Despatch() const
{
#ifndef _LUA_EDITOR
	ntPrintf("LUA: Request Global Watch on %s\n", m_acName);

	LuaDebugServer::AddGlobalWatch(m_acName);	
#endif
}


////////////////////////////////////////////////////////////////////////
//
//  LuaDbgMsg_ExecuteScript
//
////////////////////////////////////////////////////////////////////////
void LuaDbgMsg_ExecuteScript::Serialise(char* buf, int) const
{
	buf[0] = (char)GetType();
	strcpy(&buf[1], m_acScript);
}


void LuaDbgMsg_ExecuteScript::Deserialise(char* buf, int)
{
	strcpy((char*)m_acScript, buf);
}


void LuaDbgMsg_ExecuteScript::Despatch() const
{
#ifndef _LUA_EDITOR
	ntPrintf("LUA: Debugger requests Immediate Execution:  '%s'\n", m_acScript);

	CLuaGlobal::Get().ExecBufferNextUpdate(m_acScript);
#endif
}

#endif
