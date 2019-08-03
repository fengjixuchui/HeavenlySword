/***************************************************************************************************
*
*	DESCRIPTION		Definition of our Lua attribute table, unified get/set from/to lua and XML
*
*
***************************************************************************************************/

#ifndef	LUAATTRTABLE_H
#define	LUAATTRTABLE_H

// forward decl
#include "game/luaglobal.h"
#include "lua/ninjalua.h"

class DataObject;
class StdDataInterface;
class CEntity;

struct lua_State;

//!--------------------------------------------------------------------
//!
//! Lua->XML binding class. This allow Lua and XML to coexist in harmany
//!	is legal to have one without out the other 
//! i.e. not have an XML backing (a pure Lua table)
//! this then allows both types accesse data in the same way.
//! In general any C++ object which might get a normal table or an XML table
//! should take one of these
//!
//!--------------------------------------------------------------------
class LuaAttributeTable
{
public:
	enum LAT_CTOR_ENUM
	{
		DEFAULT_LUA_TABLE = 0,
		NO_LUA_TABLE,
	};

	// use to subsume an existing lua table
	LuaAttributeTable( NinjaLua::LuaObject& );

	DataObject*	GetDataObject() { return m_pDataObject; }
	void SetDataObject( DataObject* pDO);

	NinjaLua::LuaObject GetAttribute( CHashedString name );

	void SetAttribute( CHashedString name, NinjaLua::LuaObject& value );
	void SetAttribute( CHashedString name, const char* value );

	// helper version of Get and Set Attributes
	void SetNumber( const char* name, float num );
	void SetInteger( const char* name, int integer );
	void SetString( const char* name, const char* str );
	void SetBool( const char* name, bool on );

	// helper version of Get Attributes
	float		GetNumber( CHashedString name );
	int			GetInteger( CHashedString name );
	//ntstd::String	GetString( const char* name );
	ntstd::String GetString( const CHashedString& name );
	CHashedString GetHashedString( const CHashedString& name );
	CVector GetVector( CHashedString name );
	bool		GetBool( CHashedString name );
	void*		GetPointer( CHashedString name );

	//! installs the LuaAttributeTable user type in to the Lua state
	static void InstallUserType( lua_State *L );

	//! Remove Lua table. If you don't need it you can kill it here
	void RemoveLuaTable();

	//! do we have a lua table
	bool HasLuaTable();

	//! do we have C++/XML backing
	bool HasDataObject();

	//! Helper function to extract the entity pointer
	CEntity* GetEntity();

	//! GetLuaObjectWrapper, provides a user data LuaObject representing this attribute table NOT the embedded table
	NinjaLua::LuaObject	GetLuaObjectWrapper();

	//! deep copy this attribute tables data into another attribute table (probably quite slow...)
	void DeepCopyTo( LuaAttributeTable* dest );

	//! Get the internal user data object (voodoo lives here)
	NinjaLua::LuaObject	GetUserData()
	{
		return m_UserData;
	}

	//! Create a new lua table
	void CreateLuaTable();

	//! use only when you really need to access the interal table directly
	NinjaLua::LuaObject GetTable()
	{
		return m_Table;
	}

	//! function takes a lua state with an attribute table on top of stack, returns C++ object return 0 if not on stack
	static LuaAttributeTable* GetFromLuaState( lua_State *L );
	//! function takes a lua state with an attribute table on top of stack, validates and returns C++ object asserts if not on stack
	static LuaAttributeTable* GetAndCheckFromLuaState( lua_State *L );

	//! function takes a luaStackObject of a attribute table , returns C++ object return 0 if not on stack
	static LuaAttributeTable* GetFromLuaState( NinjaLua::LuaStackObject stk );
	//! function takes a luaStackObject of a attribute table , validates and returns C++ object asserts if not on stack
	static LuaAttributeTable* GetAndCheckFromLuaState( NinjaLua::LuaStackObject stk );

	//! does the user data creation so this works with lua properly
	static LuaAttributeTable* Create();

	LuaAttributeTable( LAT_CTOR_ENUM eLua = DEFAULT_LUA_TABLE );

private:

	NinjaLua::LuaObject		m_UserData;		//!< the userdata with the meta table stuff set up. Required
	NinjaLua::LuaObject		m_Table;		//!< the optional lua table that can contain lua data of any type
	DataObject*				m_pDataObject;
	StdDataInterface*		m_pInterface;
	NinjaLua::LuaObject		m_SharedAttributes; //!< if the lua has asked for shared attributes this is valid
												// owned by Lua don't free
};

#endif
