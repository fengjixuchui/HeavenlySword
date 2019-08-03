//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aibehaviourpool.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

#include "input/inputhardware.h"
#include "lua/ninjalua.h"
#include "game/aicomponent.h"
#include "game/luaglobal.h"
#include "game/luaexptypes.h"
#include "ai/aistatemachine.h"
// -- #include "ai/aiidle.h"
// -- #include "ai/aipatrol.h"
// -- #include "ai/aiinvestigate.h"
// -- #include "ai/aicover.h"
// -- #include "ai/aichase.h"
// -- #include "ai/aibehaviour_attack.h"
#include "ai/aibehaviour_combat.h"
#include "ai/aibehaviour_lockon.h"
#include "ai/aibehaviour_lua.h"
// -- #include "ai/airandomwalk.h"
// -- #include "ai/aiwalktopoint.h"
// -- #include "ai/aifollow.h"
#include "ai/aibehaviourcontroller.h"
#include "ai/aibehaviourpool.h"

#define PRINT_ACTIVE_BEHAVIOUR_LIST 0

/***************************************************************************************************
* Start exposing the element to Lua
***************************************************************************************************/
LUA_EXPOSED_START(AIBehaviourPool)
	LUA_EXPOSED_METHOD(AddBehaviour,	CreateBehaviour,     "", "", "") 
	LUA_EXPOSED_METHOD(RemoveBehaviour,	DeleteBehaviour_LUA, "", "", "") 
	LUA_EXPOSED_METHOD(SetActive,		SetActive,           "", "", "") 
LUA_EXPOSED_END(AIBehaviourPool)

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::AIBehaviourPool
//! Construction
//!                                                                                         
//------------------------------------------------------------------------------------------
AIBehaviourPool::AIBehaviourPool()
{
	ATTACH_LUA_INTERFACE(AIBehaviourPool);

	m_obStateMachines.clear();
	m_obActiveStateMachines.clear();
	m_obDeleteList.clear();

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Expose the manager to Lua
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	NinjaLua::LuaState& luaState = CLuaGlobal::Get().State();
	NinjaLua::LuaValue::Push( luaState, this );
	luaState.GetGlobals().Set( "BehaviourMan", NinjaLua::LuaObject( -1, luaState, false) );
	m_bLevelHasBeenUnloaded = false;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::~AIBehaviourPool
//! Destruction
//!                                                                                         
//------------------------------------------------------------------------------------------
AIBehaviourPool::~AIBehaviourPool()
{
	LevelUnload();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::Update
//! Update the Behaviour Manager
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIBehaviourPool::Update( const float fTimeChange )
{
	m_bLevelHasBeenUnloaded = false;

	// Debug mech to test that a single entity doesn't get 2 behaviour updates in a frame
	static uint64_t s_uiGobalDebugUpdate = 1;

#ifndef _RELEASE
	static float fTimePassed = 0.0f;
	int index = 0;
#endif

	// update all active machines
	if( fTimeChange > 0.0f )
	{
		for (AIStateMachineList::iterator itr = m_obActiveStateMachines.begin(); itr != m_obActiveStateMachines.end(); ++itr)
		{
			CAIStateMachine* pAIState = (*itr);

			if ( pAIState->ShouldUpdate( pAIState->GetUpdateFlags() ) )
			{
				//ntAssert( pAIState->GetOwnersAIComponent()->m_uiBehaviourTimeStamp != s_uiGobalDebugUpdate && "Time stamps should be different" );

				pAIState->Update( fTimeChange );

				// Update the time stamp
				pAIState->GetOwnersAIComponent()->m_uiBehaviourTimeStamp = s_uiGobalDebugUpdate;
			}
#ifndef _RELEASE
			if (fTimePassed>10.0f && PRINT_ACTIVE_BEHAVIOUR_LIST)
			{
				ntPrintf("AIBehaviourPool: %d - AI: [%s] - Beh: [%s]\n",	index++,
																			ntStr::GetString(pAIState->GetOwnersAIComponent()->GetParent()->GetName()),
																			ntStr::GetString(pAIState->GetBehaviourName()));
			}
#endif
		}
	}

#ifndef _RELEASE
	if (fTimePassed>10.0f)
		fTimePassed = 0.0f;
	else
 		fTimePassed +=fTimeChange;
#endif

	++s_uiGobalDebugUpdate;

	// delete all machines on the delete list
	//if (!m_obDeleteList.empty())
	//{
		//ntPrintf( "%d state machines on the delete list\n", m_obDeleteList.size() );
	//}

	DeleteListEntryList::iterator delItr = m_obDeleteList.begin();
	DeleteListEntry* toRemove = NULL;
	for (; delItr != m_obDeleteList.end();)
	{
		CAIStateMachine* pState = (*delItr)->pState;
		m_obStateMachines.remove( pState );
		m_obActiveStateMachines.remove( pState );
		--(*delItr)->delay;
		if ((*delItr)->delay < 0)
		{
			//ntPrintf( " - deleting 0x%X\n", pState );
			NT_DELETE_CHUNK( Mem::MC_AI, pState );
			toRemove = (*delItr);
		}

		// increment the delete list iterator BEFORE (potentially) removing it from the list
		++delItr;
		if (toRemove != NULL)
		{
			m_obDeleteList.remove( toRemove );
			NT_DELETE_CHUNK( Mem::MC_AI, toRemove );
			toRemove = NULL;
		}
	}

	// cleaning up state machines of dead AIs is still causing crashes, as they are still being destroyed
	// from script a long time after the entity is dead.
/*
	itr = m_obActiveStateMachines.begin();
	for (; itr != m_obActiveStateMachines.end(); ++itr)
	{
		if ((*itr)->EntityDead())
		{
			DeleteBehaviour( (*itr), 600 );
		}
	}
*/

}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::Update
//! Update the Behaviour Manager
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIBehaviourPool::LevelUnload()
{
	m_bLevelHasBeenUnloaded = true;
	AIStateMachineList::iterator itr = m_obStateMachines.begin();
	for (; itr != m_obStateMachines.end(); ++itr)
	{
		CAIStateMachine* pSM = (*itr);
		NT_DELETE_CHUNK( Mem::MC_AI, pSM );
		//pSM->m_pobEnt = NULL;
	}

	m_obStateMachines.clear();

	//AIStateMachineList::iterator itrActive = m_obActiveStateMachines.begin();
	//for (; itrActive != m_obStateMachines.end(); ++itr)
	//{
	//	CAIStateMachine* pSM = (*itrActive);
	//	NT_DELETE_CHUNK( Mem::MC_AI, pSM );
	//}
	m_obActiveStateMachines.clear();

	DeleteListEntryList::iterator delItr = m_obDeleteList.begin();
	for (; delItr != m_obDeleteList.end(); ++delItr)
	{
		DeleteListEntry* pDLE = (*delItr);
		NT_DELETE_CHUNK( Mem::MC_AI, pDLE );
	}
	m_obDeleteList.clear();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::SetActive
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIBehaviourPool::SetActive( CAIStateMachine* pobState, bool bActive )
{
	//ntPrintf( "SetActive( 0x%X, %d )\n", pobState, bActive );

	bool inPool = false;

	// Check that the behaviour exists
	AIStateMachineList::iterator itr = m_obStateMachines.begin();
	for (; itr != m_obStateMachines.end(); ++itr)
	{
		if (pobState == (*itr))
		{
			inPool = true;
		}
	}

	 
	if (inPool)
	{
		if (bActive)
		{

			// Check that it is not already in the list
			AIStateMachineList::iterator itr2		= m_obActiveStateMachines.begin();
			AIStateMachineList::iterator itrEnd2	= m_obActiveStateMachines.end();
			for (; itr2 != itrEnd2; ++itr2)
			{
				CAIStateMachine* pSM = (*itr2);
				if (pobState == pSM)
				{
#ifndef _RELEASE
					// Baaaad. This state is already active!!!
					ntPrintf("(ERROR!) (pobState == pSM) [%s] - Behaviour: %s - AIBehaviourPool::SetActive is trying to add to the list of active behaviour an existing one\n", ntStr::GetString(pSM->GetOwnerEntity()->GetName()), ntStr::GetString(pSM->GetBehaviourName()));
#endif
					return;
				}
#ifndef _RELEASE
				if ( pSM->GetOwnerEntity()->GetName() == pobState->GetOwnerEntity()->GetName() &&
					pSM->GetBehaviourName() == pobState->GetBehaviourName() )
				{
					// Baaaad. This state is already active!!!
					ntPrintf("(ERROR!) (pobState_name == pSM_name )[%s] - Behaviour: %s - AIBehaviourPool::SetActive is trying to add to the list of active behaviour an existing one\n", ntStr::GetString(pSM->GetOwnerEntity()->GetName()), ntStr::GetString(pSM->GetBehaviourName()));
					return;
				}
#endif // _RELEASE

			}


			m_obActiveStateMachines.push_back( pobState );
		}
		else
		{
			m_obActiveStateMachines.remove( pobState );
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::CreateBehaviour
//! Class Factory Lite.
//!                                                                                         
//------------------------------------------------------------------------------------------
CAIStateMachine* AIBehaviourPool::CreateBehaviour( AI* pEntity, const char* strBehaviour )
{
	if (!pEntity || !strBehaviour)
		return NULL;

	CAIStateMachine* pobStateMachine = CAIBehaviourManager::CreateBehaviour( strBehaviour, pEntity->GetAIComponent(), pEntity );

	if( pobStateMachine )
		m_obStateMachines.push_back( pobStateMachine );

	return pobStateMachine;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::DeleteBehaviour_LUA
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIBehaviourPool::DeleteBehaviour_LUA( CAIStateMachine* pobStateMachine )
{
	DeleteBehaviour( pobStateMachine, 0 );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::DeleteBehaviour
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIBehaviourPool::DeleteBehaviour( CAIStateMachine* pobStateMachine, int delay )
{
	if (!pobStateMachine || m_bLevelHasBeenUnloaded) //|| !pobStateMachine->m_pobEnt)
		return;
	
	DeleteListEntryList::iterator delItr = m_obDeleteList.begin();
	for (; delItr != m_obDeleteList.end(); ++delItr)
	{
		DeleteListEntry* pDelListItem = (*delItr);
		if (pobStateMachine == pDelListItem->pState)
			return;
	}

	// It was not found in the delete list
	DeleteListEntry* newEntry = NT_NEW_CHUNK( Mem::MC_AI ) DeleteListEntry;
	newEntry->pState = pobStateMachine;
	newEntry->delay = delay;
	m_obDeleteList.push_back( newEntry );
}

//------------------------------------------------------------------------------------------
//!  public constant  GetCurrentBehaviour
//!
//!  @param [in, out]   CEntity *    
//!
//!  @return CAIStateMachine * 
//!
//!  @remarks  Return the current behaviour for the entity
//!
//!  @author GavB @date 01/08/2006
//------------------------------------------------------------------------------------------
CAIStateMachine* AIBehaviourPool::GetCurrentBehaviour( CEntity* pEntity ) const
{
	// Search through the list of active entities
	for( AIStateMachineList::const_iterator obIt = m_obActiveStateMachines.begin();
			obIt != m_obActiveStateMachines.end();
				++obIt )
	{
		CAIStateMachine* pStateMachine = *obIt;

		if( pStateMachine->GetOwnerEntity() == pEntity )
		{
			return pStateMachine;
		}
	}

	return 0;
}

