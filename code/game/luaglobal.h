/***************************************************************************************************
*
*	DESCRIPTION		Definition of our Lua global state management class
*
*	NOTES			Using Lua in the game? Include this file.. please don't include the files 
*					referenced in this header separately.
*
***************************************************************************************************/

#ifndef	_LUAGLOBAL_H
#define	_LUAGLOBAL_H

#include "lua/ninjalua.h"
#include "game/luaexptypes.h"

class CMessage;
class CEntity;
class CInterface;
class LuaAttributeTable;
class DataObject;
class StdDataInterface;

namespace NinjaLua
{
	class LuaState;
	class LuaObject;
};


#define METATABLE_COUNT			(50)

/***************************************************************************************************
*
*	CLASS			CLuaGlobal
*
*	DESCRIPTION		Object that contains single lua state for the game to use.
*
***************************************************************************************************/

class CLuaGlobal : public Singleton<CLuaGlobal>
{
public:
	struct MetaTableUnit
	{
		NinjaLua::LuaObject		m_obMetaTable;
		CHashedString			m_DebugId;
	};

	// 
	static MetaTableUnit	s_MetaTables[METATABLE_COUNT];
	static int				s_MtCount;

public:
	CLuaGlobal();
	~CLuaGlobal();

	void Update();

	void InstallFile( const char* pcScriptFile, bool bAddPath = true );
	void InstallOptionalFile(const char* pcScriptFile);

	NinjaLua::LuaState& State()			{ntAssert(m_pNinjaState); return *m_pNinjaState;}

	// Targ is the default entity for scripts (can be 0, but usually scripts will be run
	// in the context of an entity).
	CEntity* GetTarg() const				{return m_pobTarg;}
	void SetTarg( CEntity* pobTarg );

	CMessage* GetMessage() const			{return m_pobMessage;}
	void SetMessage( CMessage* pobMessage )	{m_pobMessage = pobMessage;}

	// Create a lua table from an XML object.
	LuaAttributeTable* TableFromBaseObject( DataObject* pDO ) const;
	LuaAttributeTable* TableFromInterface( StdDataInterface* pobInterface ) const;
	LuaAttributeTable* TableFromNada( void ) const;

	void ReloadNextUpdate(const char* pcFile) {m_bReload = true; strcpy(m_sReload, pcFile);}
	void ExecBufferNextUpdate(const char* pcBuffer) {strcpy(m_sExecBuffer, pcBuffer);}

	// Function helpers - could go on luahelp?
	static bool CLuaGlobal::CallLuaFunc(CHashedString obFunctionName, CEntity* pobTarg = 0);
	static NinjaLua::LuaFunction CLuaGlobal::GetLuaFunc(CHashedString obFunctionName, CEntity* pobTarg = 0);
	static NinjaLua::LuaFunction CLuaGlobal::GetLuaFunc(CHashedString obTableName, CHashedString obFunctionName, CEntity* pobTarg = 0);

private:
	void InstallAttributeTableUserType();

private:
	NinjaLua::LuaState*	m_pNinjaState;
	CEntity*			m_pobTarg;
	CMessage*			m_pobMessage;

	bool m_bReload;
	char m_sReload[256];
	char m_sExecBuffer[2048];
};


#endif	//_LUAGLOBAL_H
