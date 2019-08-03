/***************************************************************************************************
*
*	DESCRIPTION		Implementation of our Lua global state management class
*
*	NOTES
*
***************************************************************************************************/

#include "game/luaglobal.h"
#include "game/luahelp.h"

#include "game/audiobindings.h"
#include "game/camerabindings.h"
#include "game/dynamicsbindings.h"
#include "game/entitybindings.h"
#include "game/aibindings.h"
#include "game/combatbindings.h"
#include "game/movementbindings.h"
#include "game/awarebindings.h"
#include "game/debugbindings.h"
#include "game/userbindings.h"
#include "game/timerbindings.h"
#include "game/tutorialbindings.h"
#include "game/hudbindings.h"
#include "effect/combateffects_trigger.h"
#include "effect/fxbinds.h"
#include "game/checkpointmanager.h"

#include "game/luamem.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/luaattrtable.h"
#include "objectdatabase/dataobject.h"
#include "game/lua_enums.h"

#include "ai/aiformationmanager.h"

// 
CLuaGlobal::MetaTableUnit	CLuaGlobal::s_MetaTables[METATABLE_COUNT];
int							CLuaGlobal::s_MtCount;


/***************************************************************************************************
*
*	FUNCTION		CLuaGlobal::Constructor
*
*	DESCRIPTION		
*
***************************************************************************************************/

CLuaGlobal::CLuaGlobal()
{
	// Clean up the metatables.
	for( int iCount = 0; iCount < METATABLE_COUNT; ++iCount )
		s_MetaTables[ iCount ].m_obMetaTable.SetNil();

	// Create the NinjaLua State
	m_pNinjaState = NinjaLua::LuaState::Create(true, false);
	//lua_atpanic(&(**m_pNinjaState), PanicFunc);

	NinjaLua::ExposedLuaInit::gExposedLuaList->Init(*m_pNinjaState);

	// Register the bindings.
	CLuaHelper::Register();
	CEntityBindings::Register();
	CDynamicsBindings::Register();
	CAudioBindings::Register();
	CCombatBindings::Register();
	MovementBindings::Register();
	AwareBindings::Register();
	DebugBindings::Register();
	TimerBindings::Register();
	TutorialBindings::Register();
	CombatEffectsTrigger::Register();
	FXBinds::Register();
	HudBindings::Register();
	CheckpointManager::Register();
	
	// temporary stuff, whatever is in that file will be removed one day or the other
	CUserBindings::Register();

	LuaAttributeTable::InstallUserType(&(**m_pNinjaState));

	m_bReload = false;
	m_sReload[0]     = 0;
	m_sExecBuffer[0] = 0;

	// Build the lua enumerations now...
	BuildLuaEnums();

	// We should consider starting the garbage collector here.


#ifdef LUA_DEBUG_MEMORY
	CLuaMemoryMan::Get().StartTagging();
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CLuaGlobal::~Destructor
*
*	DESCRIPTION		cleanup state
*
***************************************************************************************************/
CLuaGlobal::~CLuaGlobal()
{
	// Free up the metatables
	for( int iCount = 0; iCount < METATABLE_COUNT; ++iCount )
		s_MetaTables[ iCount ].m_obMetaTable.SetNil();

	// Take out the Trash
	//lua_setgcthreshold(&(*State()), 0);
	ntPrintf("Lua Shutting down...\nLua Memory Usage: %dk\n", lua_gc(&(*State()), LUA_GCCOUNT, 0));
	lua_gc(&(*State()), LUA_GCCOLLECT, 0);

#ifdef LUA_DEBUG_MEMORY
	MemReport::ShowCurrent( MemReport::kAlphabetical );
	CLuaMemoryMan::Get().StopTagging();
#endif

	NinjaLua::LuaState::Destroy(m_pNinjaState);

	// Free up the lua list
	NinjaLua::ExposedLuaInit::gExposedLuaList->Free();

	// Free up the lua exposed map
	NinjaLua::ParentSet::Clean();
}


void CLuaGlobal::Update()
{
	// Take out the Trash
	//lua_setgcthreshold(&(*State()), 0);

	if(m_bReload)
	{
		NinjaLua::InstallFile(*m_pNinjaState, m_sReload);
		m_bReload = false;
#ifndef _RELEASE
		AIFormationManager::Get().LuaFileReloaded();
#endif

	}

	if(m_sExecBuffer[0])
	{
		NinjaLua::DoBuffer(CLuaGlobal::Get().State(), m_sExecBuffer, strlen(m_sExecBuffer), "Remote Debugger Immediate");
		m_sExecBuffer[0] = 0;
	}
}


void CLuaGlobal::InstallFile(const char* pcScriptFile, bool bAddPath)
{
	if(bAddPath)
	{
		static char acName[ MAX_PATH ];
		Util::GetFiosFilePath(pcScriptFile, acName);

		NinjaLua::InstallFile(State(), acName);
	}
	else
	{
		NinjaLua::InstallFile(State(), pcScriptFile);
	}
}

void CLuaGlobal::InstallOptionalFile(const char* pcScriptFile)
{
	char acName[ MAX_PATH ];
	Util::GetFiosFilePath(pcScriptFile, acName);

	if (File::Exists(acName))
	{
		NinjaLua::InstallFile(State(), acName);
	}
}






void CLuaGlobal::SetTarg(CEntity* pobTarg)	
{ 
	if(pobTarg && pobTarg->HasLuaTable())
	{
		// TODO: TIDY: Make a better way.
		NinjaLua::LuaValue::Push<CEntity*>(State(), pobTarg);
		NinjaLua::LuaObject newself(-1, State(), false);
		State().GetGlobals().Set("this", newself);
	}
	m_pobTarg = pobTarg; 
}


LuaAttributeTable* CLuaGlobal::TableFromBaseObject( DataObject* pDO ) const
{
	ntAssert_p(pDO, ("Invalid data object to create a lua attribute table") );

	LuaAttributeTable* pAttrTab	= LuaAttributeTable::Create();
	pAttrTab->SetDataObject(pDO);

	return pAttrTab;
}


LuaAttributeTable* CLuaGlobal::TableFromInterface(StdDataInterface* pobInterface) const
{
	ntAssert(pobInterface);
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(pobInterface);

	LuaAttributeTable* pAttrTab	= LuaAttributeTable::Create();
	pAttrTab->SetDataObject(pDO);

	return pAttrTab;
}

LuaAttributeTable* CLuaGlobal::TableFromNada() const
{
	LuaAttributeTable* pAttrTab	= LuaAttributeTable::Create();
	return pAttrTab;
}

bool CLuaGlobal::CallLuaFunc(CHashedString obFunctionName, CEntity* pobTarg)
{
	if ( CLuaGlobal::Exists() && !ntStr::IsNull(obFunctionName) )
	{
		NinjaLua::LuaObject obObj = CLuaGlobal::Get().State().GetGlobals()[ obFunctionName ];
		
		CLuaGlobal::Get().SetMessage( 0 );
		if ( pobTarg )
		{
			CLuaGlobal::Get().SetTarg( pobTarg );
		}

		if ( !obObj.IsNil() )
		{
			NinjaLua::LuaFunction obFunc(obObj);
			obFunc();
			return true;
		}
	}

	return false;
}

NinjaLua::LuaFunction CLuaGlobal::GetLuaFunc(CHashedString obFunctionName, CEntity* pobTarg)
{
	ntAssert( !ntStr::IsNull(obFunctionName) );
	ntAssert( CLuaGlobal::Exists() );

	NinjaLua::LuaObject obObj = CLuaGlobal::Get().State().GetGlobals()[ obFunctionName ];
	ntError_p( !obObj.IsNil(), ("Function %s not found\n", obFunctionName.GetDebugString() ) );
	CLuaGlobal::Get().SetMessage( 0 );
	if ( pobTarg )
		CLuaGlobal::Get().SetTarg( pobTarg );

	return NinjaLua::LuaFunction(obObj); 
}

NinjaLua::LuaFunction CLuaGlobal::GetLuaFunc(CHashedString obTableName, CHashedString obFunctionName, CEntity* pobTarg)
{
	ntAssert( !ntStr::IsNull(obFunctionName) );
	ntAssert( !ntStr::IsNull(obTableName) );
	ntAssert( CLuaGlobal::Exists() );

	NinjaLua::LuaObject obObj = CLuaGlobal::Get().State().GetGlobals()[ obTableName ];
	NinjaLua::LuaObject obFunc = obObj[ obFunctionName ];
	ntError_p( !obObj.IsNil(), ("Function %s.%s not found\n", obTableName.GetDebugString(), obFunctionName.GetDebugString() ) );
	CLuaGlobal::Get().SetMessage( 0 );
	if ( pobTarg )
		CLuaGlobal::Get().SetTarg( pobTarg );

	return NinjaLua::LuaFunction(obObj);
}
