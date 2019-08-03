/***************************************************************************************************
*
*	FILE			objectbindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/

#include "game/objectbindings.h"
#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"


//-------------------------------------------------------------------------------------------------
// REMOVED BINDFUNC: object FindObj( string ObjectName )
// DESCRIPTION: Finds a base object from the instance manager
//-------------------------------------------------------------------------------------------------
static NinjaLua::LuaObject FindObj( const char* pcObjName )
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( pcObjName );
	if( !pDO )
	{
		ntError_p( 0, ("'%s' object not found", pcObjName ) );
	}
	LuaAttributeTable* pAttr = LuaAttributeTable::Create();
	pAttr->SetDataObject( pDO );

	return pAttr->GetLuaObjectWrapper();
}


/***************************************************************************************************
*
*	FUCNTIION		ObjectBindings::Register
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CObjectBindings::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	// is this nessecary any more?
	obGlobals.Register( "FindObj", FindObj );
}

