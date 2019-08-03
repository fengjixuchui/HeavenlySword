// Luaexposed.cpp
//

#include "ninjaLua.h"
#include "game/shellconfig.h"

using namespace NinjaLua;

// Global list entry point
ExposedLuaInit*	ExposedLuaInit::gExposedLuaList = 0;
ParentSet::Map* ParentSet::m_pMap = 0;


void ParentSet::Add(CHashedString pcChild, CHashedString pcParent)
{
	if(!m_pMap)
		m_pMap = NT_NEW Map;
	m_pMap->insert(ntstd::pair<CHashedString, CHashedString>(pcChild, pcParent));
}

void ParentSet::Clean()
{
	if (m_pMap)
	{
		NT_DELETE( m_pMap );
		m_pMap = 0;
	}
}


CHashedString ParentSet::Get(CHashedString pcChild)
{
	Map::const_iterator it = m_pMap->find(pcChild);
	return it == m_pMap->end() ? CHashedString() : it->second;
}

//------------------------------------------------------------------------------------------
//!
//!	LuaExposedContainer::Add
//!	Add a binding to this container
//!
//------------------------------------------------------------------------------------------
void LuaExposedContainer::Add(CHashedString pcName, const LuaBinding& obLuaBinding, bool bGet)
{
	(bGet ? m_ExposedGet : m_ExposedSet).insert(Pair(pcName, &obLuaBinding));
}


//------------------------------------------------------------------------------------------
//!
//!	LuaExposedContainer::Inherit
//!	Inherit the bindings from another container
//!
//------------------------------------------------------------------------------------------
void LuaExposedContainer::Inherit(LuaExposedContainer** pCont, CHashedString pcChild, CHashedString pcParent)
{
	m_pInherited = pCont;
	NinjaLua::ParentSet::Add(pcChild, pcParent);
}


//-------------------------------------------------------------------------------------------------
// METHOD:		
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------

const LuaBinding* LuaExposedContainer::Find(CHashedString pcName, bool bGet) const
{
	Map::const_iterator obIt = (bGet ? m_ExposedGet : m_ExposedSet).find(pcName);

	if(obIt == (bGet ? m_ExposedGet : m_ExposedSet).end())
	{
		if(m_pInherited)
		{
			// Try to find it on our parent container
			return (*m_pInherited)->Find(pcName, bGet);
		}
		return 0;
	}

	return obIt->second;
}

//-------------------------------------------------------------------------------------------------
// METHOD:			ExposedLuaInit::Free
// DESCRIPTION:		
//-------------------------------------------------------------------------------------------------
void ExposedLuaInit::Free(void)
{
	// Remove the container.. 
	if( m_prLuaContainer )
	{
		NT_DELETE( m_prLuaContainer );
		m_prLuaContainer = 0;
	}

	// If there is another item in the list, then free that's LuaContainer
	if( m_Next )
		m_Next->Free();

}


//------------------------------------------------------------------------------------------
//!
//!	NinjaLua::LogLuaWarning
//!	Log a Lua Warning
//!
//------------------------------------------------------------------------------------------
void NinjaLua::LogLuaWarning(NinjaLua::LuaState& state, const char* pcFormat, ...)
{
	char acBuffer[1024];
	acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
	int iResult;
	va_list	stArgList;
	va_start( stArgList, pcFormat );
	iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
	va_end( stArgList );

	Debug::LuaWarning((char*)acBuffer);
	ntPrintf("%s: warning - %s\n", state.FileAndLine(), (const char*)acBuffer);

	if(g_ShellOptions->m_bLuaDebugOnWarnings)
		Debug::WaitForLuaDebugger(false);
}


//------------------------------------------------------------------------------------------
//!
//!	NinjaLua::LogLuaWarning
//!	Log a Lua Error
//!
//------------------------------------------------------------------------------------------
void NinjaLua::LogLuaError(NinjaLua::LuaState& state, const char* pcFormat, ...)
{
	char acBuffer[1024];
	acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
	int iResult;
	va_list	stArgList;
	va_start( stArgList, pcFormat );
	iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
	va_end( stArgList );

	char acBuffer2[1024];
	sprintf((char*)acBuffer2, "%s: ERROR - %s\n", state.FileAndLine(), (const char*)acBuffer);
	Debug::LuaWarning((char*)acBuffer2);
	Debug::WaitForLuaDebugger(false);
}
