/***************************************************************************************************
*
*	FILE			aibindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/

////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "game/aibindings.h"
#include "game/aicomponent.h"
#include "game/attacks.h"

#include "ai/aistates.h"
#include "ai/aiformationmanager.h"
#include "ai/aiformation.h"
#include "ai/aibehaviourmanager.h"
#include "ai/aibehaviour_formation.h"
#include "ai/aibehaviourpool.h"
#include "ai/aiformationattack.h"
#include "ai/aitasks.h"

#include "game/luaglobal.h"
#include "game/luaattrtable.h"
#include "game/luahelp.h"
#include "game/luardebug.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/query.h"
#include "game/randmanager.h"
#include "game/messagehandler.h"
#include "game/luaexptypes.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// General Bindings
////////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
// This function is now dead, it can be removed once we do away with the old chatterbox.lua system
//-------------------------------------------------------------------------------------------------
// BINDFUNC:    AI_FindEntityForIncidental
// DESCRIPTION:	Finds a random enemy near the player which is suitable for doing an incidental.
// NOTES:		Returns entity state table if found, otherwise returns nil.
//-------------------------------------------------------------------------------------------------
static int AI_FindEntityForIncidental (NinjaLua::LuaState* pState)
{
	CEntityQuery obQuery;
	EQCIsSuitableForIncidental obClause1;
	CEQCProximityColumn obClause2;

	obClause2.SetRadius(10.0f);
	obClause2.SetTranslation(CEntityManager::Get().GetPlayer()->GetPosition());

	obQuery.AddClause(obClause1);
	obQuery.AddClause(obClause2);
	
	CEntityManager::Get().FindEntitiesByType(obQuery, CEntity::EntType_AI);

	if (obQuery.GetResults().size()==0)
	{
		pState->PushNil();
		return 1;
	}

	int iIndex=grand() % obQuery.GetResults().size();

	int iCount=0;

	for(QueryResultsContainerType::iterator obIt=obQuery.GetResults().begin(); obIt!=obQuery.GetResults().end(); ++obIt)
	{
		if (iIndex==iCount)
		{
			(*obIt)->GetAttributeTable()->GetLuaObjectWrapper().Push();

			return 1;
		}

		++iCount;
	}

	pState->PushNil();
	return 1;
}


/***************************************************************************************************
*
*	FUNCTION		CAIBindings::Register
*
*	DESCRIPTION		Register the functions with the scripting enviroment
*
***************************************************************************************************/
void CAIBindings::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	// Misc
	obGlobals.RegisterRaw("AI_FindEntityForIncidental", AI_FindEntityForIncidental);
}
