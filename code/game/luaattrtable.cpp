/***************************************************************************************************
*
*	DESCRIPTION		Implementation of our Lua global state management class
*
*	NOTES
*
***************************************************************************************************/

#include "objectdatabase/dataobject.h"
#include "game/luahelp.h"
#include "game/luamem.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/luaattrtable.h"
#include "game/luaexptypes.h"

LUA_EXPOSED_START(LuaAttributeTable)

LUA_EXPOSED_END(LuaAttributeTable)

using namespace NinjaLua;

namespace{
void stackDump( lua_State *L )
{
	int i;
	int top = lua_gettop(L);
	for(i=1; i <= top;i++)
	{
		int t = lua_type(L,i);
		switch(t)
		{
		case LUA_TSTRING:
			ntPrintf( "string %s", lua_tostring(L,i));
			break;
		case LUA_TBOOLEAN:
			ntPrintf( lua_toboolean(L,i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			ntPrintf("number", lua_tonumber(L,i));
			break;
		case LUA_TUSERDATA:
			ntPrintf("userdata");
			break;
		default:
			ntPrintf("%s", lua_typename(L,t) );
			break;
		}
		ntPrintf("   ");
	}
	ntPrintf("\n");
}
};

//#define BREAK_ON_LUA_ACCESS "CombatDefinition"

//! Creates a lua attribute table with an empty table by default
LuaAttributeTable::LuaAttributeTable( LAT_CTOR_ENUM eLua ) :
	m_pDataObject( 0 ),
	m_pInterface( 0 )
{
	if( eLua == DEFAULT_LUA_TABLE )
	{
		CreateLuaTable();
	}
	m_SharedAttributes.AssignNil( CLuaGlobal::Get().State() );
}

//! Creates a lua attribute table from an existing lua table
LuaAttributeTable::LuaAttributeTable( NinjaLua::LuaObject& obj ) :
	m_UserData( CLuaGlobal::Get().State() ),
	m_pDataObject( 0 ),
	m_pInterface( 0 )
{
	ntAssert_p( obj.IsTable() || obj.IsUserData(), ("Type is not a not a table, is type %s:%s", obj.GetTypeName(), obj.GetState()->FileAndLine() ) );
	
	m_Table = obj;
}

LuaAttributeTable* LuaAttributeTable::Create()
{
	int top = lua_gettop( CLuaGlobal::Get().State() );
	LuaAttributeTable* pAttrTab	= (LuaAttributeTable*) lua_newuserdata( CLuaGlobal::Get().State(), sizeof( LuaAttributeTable ) );
	NT_PLACEMENT_NEW (pAttrTab) LuaAttributeTable(); // inplace construct
	pAttrTab->m_UserData = NinjaLua::LuaObject( -1, CLuaGlobal::Get().State(), true );
	luaL_getmetatable( CLuaGlobal::Get().State(), "attributetable.metatable" );
	lua_setmetatable( CLuaGlobal::Get().State(), -2 );

	lua_settop( CLuaGlobal::Get().State(), top);

	return pAttrTab;
}

//! Set a dataobject as the XML provider for this table
void LuaAttributeTable::SetDataObject( DataObject* pDO)
{
	m_pDataObject = pDO;
	m_pInterface = ObjectDatabase::Get().GetInterface( pDO );
}

//! Remove Lua table. If you don't need it you can kill it here
void LuaAttributeTable::RemoveLuaTable()
{
	m_Table.SetNil();
}

//! do we have a lua table
bool LuaAttributeTable::HasLuaTable()
{
	return !m_Table.IsNil();
}

//! do we have C++/XML backing
bool LuaAttributeTable::HasDataObject()
{
	return (m_pDataObject != 0);
}

void LuaAttributeTable::CreateLuaTable()
{
	m_Table.AssignNewTable( CLuaGlobal::Get().State() );
}


//! this returns an lua object given the name of the field, XML is searched first in name clashes.
//! Name is a special case and is allows the name of the XML object if we have one
NinjaLua::LuaObject LuaAttributeTable::GetAttribute( CHashedString name )
{
#if defined( BREAK_ON_LUA_ACCESS )
	if( strcmp( name, BREAK_ON_LUA_ACCESS ) == 0 )
	{
		int a = 0;a;
	}
#endif
	//if (name == CHashedString("BSClump"))
	//{
	//	ntBreakpoint();
	//}

	DataInterfaceField* pDIF = 0;

	if(m_pInterface)
	{
		pDIF = m_pInterface->GetFieldByName( name );
	}

	if( pDIF == 0 )
	{
		NinjaLua::LuaObject retVal;
		if (m_pDataObject)
		{
			
			// special case name and entity... this should be looked at 2 string compares its to much
			if( name == CHashedString(HASH_STRING_NAME) )
			{
				retVal.AssignString( CLuaGlobal::Get().State(), ntStr::GetString(m_pDataObject->GetName()) ); 
				return retVal;
			} else
			if( name == CHashedString(HASH_STRING_TYPE) )
			{
				retVal.AssignString( CLuaGlobal::Get().State(), m_pDataObject->GetClassName() ); 
				return retVal;
			} else
			if( name == CHashedString(HASH_STRING_ENTITY) || name == CHashedString(HASH_STRING_ENT) )
			{
				NinjaLua::LuaValue::Push( CLuaGlobal::Get().State(), GetEntity() );
				retVal = NinjaLua::LuaObject( -1, CLuaGlobal::Get().State() );
				return retVal;

			}else 
			if( name == CHashedString(HASH_STRING_BREAKKY) )
			{
				DebugBreakNow();
				retVal.AssignString( CLuaGlobal::Get().State(), "BreakkyXXXX" ); 
				return retVal;

			} else
			if( name == CHashedString(HASH_STRING_SHAREDATTRIBUTES) )
			{
				ntAssert_p( m_Table.GetState() != 0, ("This LuaAttributeTable has no Lua table, so %s is not availible", ntStr::GetString(name)) );
				if( m_SharedAttributes.IsNil() )
				{
					NinjaLua::LuaObject obj = m_Table.Get<NinjaLua::LuaObject>( name );
					if( obj.IsTable() )
					{
						retVal = obj;
						return retVal;
					}
					ntError_p( obj.IsString(), ("Shared Attributes in a lua table must be a string (%s)", obj.GetTypeName() ) );
					DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( obj.GetString() );
					ntError_p( pDO, ( "Cannot find shared attribute %s\n", obj.GetString() ) );
					LuaAttributeTable* pSharedAttributes = LuaAttributeTable::Create();
					pSharedAttributes->SetDataObject( pDO );
					m_SharedAttributes = pSharedAttributes->GetUserData();
				}
				retVal = m_SharedAttributes;
				return retVal;
			}
		}

		if(m_Table.GetState() != 0)
			retVal = m_Table.Get<NinjaLua::LuaObject>( name );
		else
			retVal.AssignNil(CLuaGlobal::Get().State());

		return retVal;
		
	} else
	{

		//stackDump(CLuaGlobal::Get().State() );
		if( m_pDataObject && ( name == CHashedString(HASH_STRING_SHAREDATTRIBUTES)))
		{
			NinjaLua::LuaObject retVal;
			if( m_SharedAttributes.IsNil() )
			{
				DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pDIF->GetAs<void*>(m_pDataObject) );
				LuaAttributeTable* pSharedAttributes = LuaAttributeTable::Create();
				pSharedAttributes->SetDataObject( pDO );
				m_SharedAttributes = pSharedAttributes->GetUserData();
			}
			retVal = m_SharedAttributes;
			return retVal;
		}
	}

	NinjaLua::LuaObject retVal;
	// need actual varibles for typeof magic to work
	unsigned int var_uint;
	int var_int;
	float var_float;
	bool var_bool;
	ntstd::String var_stdstring;
	CHashedString var_hashstring;
	CKeyString	var_keystring;
	CPoint var_point;
	CVector var_vector;
	CQuat var_quat;
	void* var_pointer;
	
	// [scee_st] chunked to LUA
	// NOTE: This force allocation of list *nodes* to the chunk, not the DataObjects themselves
	typedef ntstd::List<class DataObject*, Mem::MC_LUA > DataObjectStdList;
	
	DataObjectStdList var_dataobjectlist;
	
	
	typedef ntstd::List<void*> VoidStdList;
	VoidStdList var_voidlist;


	switch( pDIF->GetType() )
	{
	case typeof_num(var_uint): UNUSED(var_uint);
	case typeof_num(var_int): UNUSED(var_int);
		retVal.AssignInteger( CLuaGlobal::Get().State(), pDIF->GetAs<int>(m_pDataObject) );
		break;
	case typeof_num(var_float): UNUSED(var_float);
		retVal.AssignNumber( CLuaGlobal::Get().State(), pDIF->GetAs<float>(m_pDataObject) );
		break;
	case typeof_num(var_bool): UNUSED(var_bool);		
		retVal.AssignBoolean( CLuaGlobal::Get().State(), pDIF->GetAs<bool>(m_pDataObject) );
		break;
	case typeof_num(var_stdstring): UNUSED(var_stdstring);		
		retVal.AssignString( CLuaGlobal::Get().State(), pDIF->GetAs<ntstd::String>(m_pDataObject).c_str() );
		break;
	case typeof_num(var_hashstring): UNUSED(var_keystring);		
		retVal.AssignHashedString( CLuaGlobal::Get().State(), pDIF->GetAs<CHashedString>(m_pDataObject) );
		break;
	case typeof_num(var_keystring): UNUSED(var_keystring);		
		retVal.AssignString( CLuaGlobal::Get().State(), ntStr::GetString(pDIF->GetAs<CKeyString>(m_pDataObject)) );
	break;


	case typeof_num(var_pointer): UNUSED(var_pointer);		
		{
			//const ntstd::string& sData = pDIF->GetData(m_pDataObject);
			//GameGUID guid;
			//guid.SetFromString(sData);
			//DataObject* pdobj = ObjectDatabase::Get().GetDataObjectFromGUID(guid);
			
			//const char* pcTypeName = pdobj->GetClassName();
			//if(!strcmp(pcTypeName, "EnemyAI") || !strcmp(pcTypeName, "CEntity")) // Dammit how do we do this?  There must be a way!
			//{
			//	CEntity *pEnt = pDIF->GetAs<CEntity*>(m_pDataObject);
			//	retVal.Assign(CLuaGlobal::Get().State(), pEnt);
			//	break;
			//} // elseif() ...
			//else
			{
				void* pv = pDIF->GetAs<void*>(m_pDataObject);

				if(pv)
				{
					DataObject* pdobj = ObjectDatabase::Get().GetDataObjectFromPointer(pv);
					LuaAttributeTable* pAttr = LuaAttributeTable::Create();
					pAttr->SetDataObject(pdobj);
					retVal = pAttr->GetLuaObjectWrapper();
				}
				break;
			}
		}
		break;

	case typeof_num(var_point): UNUSED(var_point);
	{
		CPoint obPoint(pDIF->GetAs<CPoint>(m_pDataObject));

		NinjaLua::LuaObject obTable;
		obTable.AssignNewTable(CLuaGlobal::Get().State());
		obTable.Set("x",obPoint.X());
		obTable.Set("y",obPoint.Y());
		obTable.Set("z",obPoint.Z());

		retVal = obTable;

		break;
	}

	case typeof_num(var_vector): UNUSED(var_vector);
	{
		CPoint obPoint(pDIF->GetAs<CPoint>(m_pDataObject));

		NinjaLua::LuaObject obTable;
		obTable.AssignNewTable(CLuaGlobal::Get().State());
		obTable.Set("x",obPoint.X());
		obTable.Set("y",obPoint.Y());
		obTable.Set("z",obPoint.Z());

		retVal = obTable;

		break;
	}
	case typeof_num(var_quat): UNUSED(var_quat);
	{
		CQuat obQuat(pDIF->GetAs<CQuat>(m_pDataObject));

		NinjaLua::LuaObject obTable;
		obTable.AssignNewTable(CLuaGlobal::Get().State());
		obTable.Set("x",obQuat.X());
		obTable.Set("y",obQuat.Y());
		obTable.Set("z",obQuat.Z());
		obTable.Set("w",obQuat.W());

		retVal = obTable;

		break;
	}

	case typeof_num(var_dataobjectlist): UNUSED(var_dataobjectlist);
	{
		// While not the best method for obtaining a list of data objects, it works.
		NinjaLua::LuaObject obTable;
		obTable.AssignNewTable(CLuaGlobal::Get().State());

		const DataObjectStdList& obList = pDIF->GetAs<DataObjectStdList>(m_pDataObject);
		int iIndex = 1;

		for( DataObjectStdList::const_iterator obIt = obList.begin(); obIt != obList.end(); ++obIt, ++iIndex )
		{
			LuaAttributeTable* pAttr = LuaAttributeTable::Create();
			pAttr->SetDataObject( *obIt );

			NinjaLua::LuaObject object = pAttr->GetLuaObjectWrapper();
			obTable.Set( iIndex, object );
		}

		retVal = obTable;
		break;
	}

	case typeof_num(var_voidlist): UNUSED(var_voidlist);
	{
		// While not the best method for obtaining a list of data objects, it works.
		NinjaLua::LuaObject obTable;
		obTable.AssignNewTable(CLuaGlobal::Get().State());

		const VoidStdList& obList = pDIF->GetAs<VoidStdList>(m_pDataObject);
		int iIndex = 1;

		for( VoidStdList::const_iterator obIt = obList.begin(); obIt != obList.end(); ++obIt)
		{
			LuaAttributeTable* pAttr = LuaAttributeTable::Create();
			DataObject* pDObj = ObjectDatabase::Get().GetDataObjectFromPointer(*obIt);

			if(pDObj)
			{
				pAttr->SetDataObject(pDObj);

				NinjaLua::LuaObject object = pAttr->GetLuaObjectWrapper();
				obTable.Set(iIndex++, object);
			}
			else
			{
				ntPrintf("Warning object not found in attribute table voidlist...\n");
			}
		}

		retVal = obTable;
		break;
	}

	default:
		retVal.AssignNil( CLuaGlobal::Get().State() );
		break;
	};

	return retVal;
}

//! This sets an attribute, XML namespace searched first. Data is poked directly into the XML system
//! currently which may cause problems.
void LuaAttributeTable::SetAttribute( CHashedString name, NinjaLua::LuaObject& value )
{
	DataInterfaceField* pDIF = 0;
	if(m_pInterface)
	{
		pDIF = m_pInterface->GetFieldByName( name );
	}

	if( pDIF == 0 )
	{
		m_Table.Set( name, value );
		return;
	} else
	{
		if (value.IsLightUserData() || value.IsUserData() )
		{
			// The new ninja lua currently stores its pointers as userdata, this will change when 5.1 rolls out. But until then 
			// it should be kept compatible. Userdata stores pointers differently to light userdata, as can be seen in the following
			// pointer de-reference. 
			void* pData = value.IsUserData() ? *(void**)value.GetUserData() : value.GetLightUserData();

			DataObject* pobObject=ObjectDatabase::Get().GetDataObjectFromPointer(pData);
			ntAssert( pobObject != NULL );

			pDIF->SetData( m_pDataObject, pobObject->GetGUID().GetAsString());
			//void* pData2 = pDIF->GetAs<void*>( m_pDataObject );
			//ntAssert( pData2 == pData );
		}
		else
		{
			const char* pcString = value.GetString();
			pDIF->SetData( m_pDataObject, pcString );
		}
	}
}
//! currently which may cause problems.
void LuaAttributeTable::SetAttribute( CHashedString name, const char* value )
{
	DataInterfaceField* pDIF = 0;
	if(m_pInterface)
	{
		pDIF = m_pInterface->GetFieldByName( name );
	}

	if( pDIF == 0 )
	{
		m_Table[name].AssignString( CLuaGlobal::Get().State(), value );
		return;
	} else
	{
		pDIF->SetData( m_pDataObject, value );
	}

}

//! Helper for setting a boolean attribute
void LuaAttributeTable::SetBool( const char* name, bool on )
{
	NinjaLua::LuaObject n;
	n.AssignBoolean( CLuaGlobal::Get().State(), on );
	SetAttribute( name, n );
}

//! Helper for setting a numeric attribute
void LuaAttributeTable::SetNumber( const char* name, float number )
{
	NinjaLua::LuaObject num;
	num.AssignNumber( CLuaGlobal::Get().State(), number );
	SetAttribute( name, num);
}

//! Helper for setting an integer attribute
void LuaAttributeTable::SetInteger( const char* name, int integer )
{
	NinjaLua::LuaObject num;
	num.AssignInteger( CLuaGlobal::Get().State(), integer );
	SetAttribute( name, num);
}

//! Helper for setting a string attribute
void LuaAttributeTable::SetString( const char* name, const char* str )
{
	NinjaLua::LuaObject strng;
	strng.AssignString( CLuaGlobal::Get().State(), str );
	SetAttribute( name, strng );
}

//! Helper for getting a boolean attribute - if there is no
//! item by this name in the table i return a 'nil' value
bool LuaAttributeTable::GetBool( CHashedString name )
{
	NinjaLua::LuaObject on = GetAttribute( name );
	if ( on.IsNil() )
	{
		return false;
	}
	else if ( on.IsBoolean() )
	{
		return on.GetBoolean();
	} 
	else
	{
		ntAssert_p ( false, ( "Not a Bool: %s", on.GetTypeName() ) );
		return false;
	}
}


//! Helper for getting a pointer attribute - if there is no
//! item by this name in the table i return a 'nil' value
void* LuaAttributeTable::GetPointer( CHashedString name )
{
	NinjaLua::LuaObject on = GetAttribute( name );
	if ( on.IsNil() )
	{
		return false;
	}
	else if ( on.IsLightUserData() )
	{
		return on.GetLightUserData();
	} 
	else
	{
		ntAssert_p ( false, ( "Not a pointer" ) );
		return 0;
	}
}

//! Helper for getting a numeric attribute - if there is no
//! item by this name in the table i return a 'nil' value
float LuaAttributeTable::GetNumber( CHashedString name )
{
	NinjaLua::LuaObject num = GetAttribute( name );
	if ( num.IsNil() )
	{
		return 0.f;
	}
	else if ( num.IsNumber() )
	{
		return ( float )num.GetNumber();
	} 
	else
	{
		ntAssert_p( false, ( "Not a Number" ) );
		return 0.f;
	}
}

//! Helper for getting an integer attribute - if there is no
//! item by this name in the table i return a 'nil' value
int LuaAttributeTable::GetInteger( CHashedString name )
{
	NinjaLua::LuaObject num = GetAttribute( name );
	if ( num.IsNil() )
	{
		return 0;
	}
	else if ( num.IsInteger() )
	{
		return num.GetInteger();
	} 
	else
	{
		ntAssert_p( false, ( "Not an Integer" ) );
		return 0;
	}
}

//! Helper for getting a string attribute - if there is no
//! item by this name in the table i return a 'nil' value
ntstd::String LuaAttributeTable::GetString( const CHashedString& name )
{
	NinjaLua::LuaObject str = GetAttribute( name );
	if ( str.IsNil() )
	{
		return "";
	}
	else if ( str.IsString() )
	{
		ntAssert(strstr(str.GetString(), "LUA") == 0);
		return str.GetString();
	} 
	else
	{
		ntAssert_p( false, ( "Not a String" ) );
		return "";
	}
}

CHashedString LuaAttributeTable::GetHashedString( const CHashedString& name )
{
	NinjaLua::LuaObject str = GetAttribute( name );
	if ( str.IsNil() )
	{
		return CHashedString();
	}
	else 
	{
		return str.GetHashedString();
	} 
}



CVector LuaAttributeTable::GetVector( CHashedString name )
{
	NinjaLua::LuaObject str = GetAttribute( name );
	
	if( str.IsTable() )
	{
		CVector obVector;
		obVector.X()=str["x"].GetFloat();
		obVector.Y()=str["y"].GetFloat();
		obVector.Z()=str["z"].GetFloat();
				
		return obVector;
	} else
	{
		ntAssert_p( false, ("Not a vector") );
		return CVecMath::GetZeroVector();
	}

}

CEntity* LuaAttributeTable::GetEntity()
{
	ntAssert_p( m_pDataObject != 0, ("No DataObject, so no entity") );
	ntAssert_p( m_pInterface->CastTo( "CEntity", m_pDataObject) != 0, ("This Dataobject cannot be interfaced casted to a CEntity") );
	return (CEntity*) m_pDataObject->GetBasePtr();
}

NinjaLua::LuaObject	LuaAttributeTable::GetLuaObjectWrapper()
{
	if (m_UserData.GetState() && !m_UserData.IsNil())
		return m_UserData;

	if (m_Table.GetState() && !m_Table.IsNil())
		return m_Table;

	NinjaLua::LuaObject obNil;
	obNil.AssignNil(CLuaGlobal::Get().State());

	return obNil;
}

//! checked version of GetFromLuaState
LuaAttributeTable* LuaAttributeTable::GetAndCheckFromLuaState( lua_State *L )
{
	void* ud = luaL_checkudata(L, 1, "attributetable.metatable" );
	luaL_argcheck( L, ud != 0, 1, "LuaAttributeTable expected" );
	return (LuaAttributeTable*) ud;
}

//! returns 0 if there isn't a LuaAttributeTable on the Lua Stck
LuaAttributeTable* LuaAttributeTable::GetFromLuaState( lua_State *L )
{
	void *p = lua_touserdata(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, "attributetable.metatable");  /* get correct metatable */
	if (p == NULL || !lua_getmetatable(L, 1) || !lua_rawequal(L, -1, -2))
		return 0;
	lua_pop(L, 2);  /* remove both metatables */
	return (LuaAttributeTable*)p;

	//void* ud = luaL_checkudata(L, 1, "attributetable.metatable" );
	//return (LuaAttributeTable*) ud;
}

//! checked version of GetFromLuaState
LuaAttributeTable* LuaAttributeTable::GetAndCheckFromLuaState( NinjaLua::LuaStackObject stk )
{
	void* ud = luaL_checkudata(stk.GetState(), stk.GetIndex(), "attributetable.metatable" );
	luaL_argcheck( stk.GetState(), ud != 0, 1, "LuaAttributeTable expected" );
	return (LuaAttributeTable*) ud;
}

//! returns 0 if there isn't a LuaAttributeTable on the Lua Stck
LuaAttributeTable* LuaAttributeTable::GetFromLuaState( NinjaLua::LuaStackObject stk )
{
	void* ud = luaL_checkudata(stk.GetState(), stk.GetIndex(), "attributetable.metatable" );
	return (LuaAttributeTable*) ud;
}

void LuaAttributeTable::DeepCopyTo( LuaAttributeTable* dest )
{
	bool bHasLuaTable = HasLuaTable() && dest->HasLuaTable();

	// note any source data that has no place in the destination will be added to the lua table if we have one
	if( dest->HasDataObject() )
	{
		StdDataInterface::const_iterator destIt = dest->m_pInterface->begin();
		while( destIt != dest->m_pInterface->end() )
		{
			// see if lua items copied into xml
			NinjaLua::LuaObject luaItem = GetAttribute( (*destIt)->GetName() );
			if( !luaItem.IsNil() )
			{
				dest->SetAttribute( (*destIt)->GetName(), luaItem );
			}
			++destIt;
		}
	}
	if( bHasLuaTable )
	{
		dest->m_Table = m_Table.Clone();
	}


}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//! helper garbage collection for user data types
template<typename T>
int GCMethod(lua_State* L)
{
    reinterpret_cast<T*>(lua_touserdata(L, 1))->~T();
    return 0;
}


//! thunk from lua to GetAttribute
static int getattribute( lua_State *L )
{
	LuaAttributeTable* a = LuaAttributeTable::GetAndCheckFromLuaState( L );
	//const char* str = luaL_check_string(L, 2);
	const char* str = lua_tostring(L,2);
	ntAssert_p(str, ("Bad String for getattribute\n"));

	NinjaLua::LuaObject lo = a->GetAttribute( str );
	lo.Push();

	return 1;
}

//! thunk from lua to SetAttribute
static int setattribute( lua_State *L )
{
	LuaAttributeTable* a = LuaAttributeTable::GetAndCheckFromLuaState( L );
	//const char* key = luaL_check_string(L, 2); // key
	const char* key = lua_tostring(L,2);
	ntAssert_p(key, ("Bad String for setattribute\n"));
	NinjaLua::LuaObject value( 3, CLuaGlobal::Get().State() ); // value
	a->SetAttribute( key, value );

	return 0;
}

//! static setup of this User type into the provided lua state
void LuaAttributeTable::InstallUserType( lua_State *L )
{
	//static const struct luaL_reg attributetablelib[] =
	//{
	//	{ "get", getattribute },
	//	{ "set", setattribute },
	//	{ 0, 0 }
	//};

	luaL_newmetatable( L, "attributetable.metatable" );
	//luaL_openlib( L, "attributetable", attributetablelib, 0 ); // attribute lib
	lua_pushstring( L, "__index" );
	lua_pushcclosure( L, getattribute, 0 );
	//lua_pushstring( L, "get" );
	//lua_gettable( L, 2 ); // get attributetable.get
	lua_settable( L, -3 ); // set the previous get
	lua_pushstring( L, "__newindex" );
	lua_pushcclosure( L, setattribute, 0 );
	//lua_pushstring( L, "set" );
	//lua_gettable( L, 2 ); // get attributetable.set
	lua_settable( L, -3 ); // set the previous set
	lua_pushstring( L, "__gc" );
	lua_pushcfunction( L, GCMethod<LuaAttributeTable> );
	lua_settable( L, -3 );	// set the templated GC to inplace destroy
}
