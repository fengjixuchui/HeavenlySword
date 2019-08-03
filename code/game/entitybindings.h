/***************************************************************************************************
*
*	FILE			entitybindings.h
*
*	DESCRIPTION		
*
***************************************************************************************************/

#ifndef _ENTITYBINDINGS_H
#define _ENTITYBINDINGS_H

class LuaAttributeTable;
class CEntity;

/***************************************************************************************************
*
*	CLASS			CEntityBindings
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CEntityBindings
{
public:
	static void Register();
};


//! can be called from C++ by faking a table
#ifdef BYPASS_LPCD_PASS_BY_VALUE
int CreateEntityFromLua( LuaState* pobState );
#else
void CreateEntityFromLuaAttributeTable( LuaAttributeTable* pAttrTable );
#endif

#endif // _ENTITYBINDINGS_H
