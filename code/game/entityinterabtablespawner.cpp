//------------------------------------------------------------------------------------------
//!
//!	\file entityinteractablespawner.h
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "game/entityinterabtablespawner.h"

#include "objectdatabase/dataobject.h"
#include "game/fsm.h"
#include "game/movement.h"
#include "game/messagehandler.h"
#include "game/interactioncomponent.h"
#include "physics/system.h"
#include "camera/camman.h"
#include "camera/camview.h"


//------------------------------------------------------------------------------------------
// EntityInteractableSpawner XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(EntityInteractableSpawner, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_PTR_AS(m_pTemplate,                              Template)
	PUBLISH_VAR_AS(m_iSize,                                  PoolSize)

	PUBLISH_VAR_AS(m_sWalkToAnim,                            WalkToAnim)
	PUBLISH_VAR_AS(m_sRunToAnim,                             RunToAnim)
	PUBLISH_VAR_AS(m_sNormalUseAnim,                         NormalUseAnim)
	PUBLISH_VAR_AS(m_sRunningUseAnim,                        RunningUseAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fNormalPickupTime,  0.05f, NormalPickupTime);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRunningPickupTime, 0.05f, RunningPickupTime);

	PUBLISH_PTR_AS(m_pUsePointAttrs,                         UsePointAttrs);

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//! EntityInteractableSpawner State Machine
//!
//------------------------------------------------------------------------------------------
STATEMACHINE(INTERACTABLESPAWNER_FSM, EntityInteractableSpawner)

	INTERACTABLESPAWNER_FSM()
	{
		SET_INITIAL_STATE(State_Active);
	}

	//--------------------------------------------------------------------------------------
	// State_Inactive
	//--------------------------------------------------------------------------------------
	STATE(State_Inactive)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			END_EVENT(true)

			ON_UPDATE
				ME->Update();
			END_EVENT(true)

			EVENT(Activate)
				SET_STATE(State_Active);
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Inactive

	//--------------------------------------------------------------------------------------
	// State_Active
	//--------------------------------------------------------------------------------------
	STATE(State_Active)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
			END_EVENT(true)

			ON_UPDATE
				ME->Update();
			END_EVENT(true)

			EVENT(Deactivate)
				SET_STATE(State_Inactive);
			END_EVENT(true)

			// Object Usage
			EVENT(msg_action_specific_run)
				ME->m_bRunningUse = true;
			EVENT(msg_action_specific)
				ME->m_pOther = (Character*)msg.GetEnt(CHashedString(HASH_STRING_OTHER));
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				
				if(!ntStr::IsNull(ME->GetUseAnim()))
				{
					// Start use animation on the using character
					ME->m_pOther->GetMovement()->Lua_AltStartFacingMovement(ME->GetUseAnim(), 360.0f, 1.0f, 0.0f, 0.0f, 0.01f);
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME->m_pOther);
				}
				else
				{
					Message msgMovementDone(msg_movementdone);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgMovementDone);
				}

				ME->m_bRunningUse = false;

				// Check for delay before spawning object?
				if(ME->GetUseAnimPickupTime() > 0.f)
				{
					ME->GetInteractionComponent()->SetInteractionType(NONE);
					Message msgOnAttach(msg_think_onreparent);
					ME->GetMessageHandler()->QueueMessageDelayed(msgOnAttach, ME->GetUseAnimPickupTime());
				}
				else
				{
					ME->Spawn(ME->m_pOther);
				}
			END_EVENT(true)

			// Delayed spawning
			EVENT(msg_think_onreparent)
			{
				ME->Spawn(ME->m_pOther);
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE // State_Active

END_STATEMACHINE


//------------------------------------------------------------------------------------------
//!
//!	GenericSpawnPool::BuildPool
//!	PostPost Construction
//!
//------------------------------------------------------------------------------------------
void GenericSpawnPool::BuildPool(DataObject* pDefOb, int iSize, const char* pcPrefix)
{
	StdDataInterface* pDefInterface = ObjectDatabase::Get().GetInterface(pDefOb);
	char*             pcType        = strdup(pDefOb->GetClassName());

	for(int i = strlen(pcType)-1; i>0; i--)
	{
		if(pcType[i] == '_' && 
		   pcType[i+1] == 'T' &&  
		   pcType[i+2] == 'e' &&  
		   pcType[i+3] == 'm' &&  
		   pcType[i+4] == 'p' &&  
		   pcType[i+5] == 'l' &&  
		   pcType[i+6] == 'a' &&  
		   pcType[i+7] == 't' &&  
		   pcType[i+8] == 'e')
		{
			pcType[i] = '\0';
			break;
		}
	}
	

	// Bring on the clones
	for(int i = 0; i < iSize;	i++)
	{
		char acName[128];
		sprintf(acName, "%s_Item%d", pcPrefix, i);

		DataObject* pDO   = ObjectDatabase::Get().ConstructObject(pcType, acName, GameGUID(), 0, true, false);
		CEntity*    pItem = (CEntity*)pDO->GetBasePtr();
		
		// Copy over all the exposed parameters
		for(ntstd::Vector<DataInterfaceField*, Mem::MC_ODB>::const_iterator fcIt = pDefInterface->begin(); fcIt != pDefInterface->end(); ++fcIt)
		{
			(*fcIt)->SetData(pDO, (*fcIt)->GetData(pDefOb));
		}

		ObjectDatabase::Get().DoPostLoadDefaults(pDO);

		// Hide and Pause
		pItem->Pause(true, true);
		pItem->Hide();

		m_inactiveItems.push_back(pItem);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	GenericSpawnPool::ActivateNewItem
//!	PostPost Construction
//!
//------------------------------------------------------------------------------------------
CEntity* GenericSpawnPool::ActivateNewItem() 
{
	if(m_inactiveItems.empty()) 
	{
		return 0;
	}

	CEntity* pEnt = m_inactiveItems.front();
	m_inactiveItems.pop_front();
	m_activeItems.push_back(pEnt);
	pEnt->Show();
	pEnt->Pause(false, true);
	pEnt->Reset();

	return pEnt;
}


//------------------------------------------------------------------------------------------
//!
//!	GenericSpawnPool::ActivateNewItem
//!	PostPost Construction
//!
//------------------------------------------------------------------------------------------
void GenericSpawnPool::Update()
{
	if(m_activeItems.size() < m_inactiveItems.size())
		return;

	for(Item_List::iterator it = m_activeItems.begin(); it != m_activeItems.end(); it++)
	{
		if(!CamMan::Get().GetPrimaryView()->IsInView((*it)->GetPosition()) &&
			(!(*it)->IsInteractable() || !(*it)->ToInteractable()->InteractableInUse()))
		{
			(*it)->Pause(true, true);
			(*it)->Hide();
			m_inactiveItems.push_back(*it);
			m_activeItems.erase(it);
			break;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	EntityInteractableSpawner::EntityInteractableSpawner
//!	Construction
//!
//------------------------------------------------------------------------------------------
EntityInteractableSpawner::EntityInteractableSpawner()
:	m_pOther(0), m_bRunningUse(false), m_bDeferDespawns(false)
{
	m_eInteractableType = EntTypeInteractable_Spawner;
}


//------------------------------------------------------------------------------------------
//!
//!	EntityInteractableSpawner::OnPostConstruct
//!	Post Construction
//!
//------------------------------------------------------------------------------------------
void EntityInteractableSpawner::OnPostConstruct()
{
	Interactable::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();

	// Create and attach the statemachine
	INTERACTABLESPAWNER_FSM* pFSM = NT_NEW INTERACTABLESPAWNER_FSM();
	ATTACH_FSM(pFSM);
}


//------------------------------------------------------------------------------------------
//!
//!	EntityInteractableSpawner::OnPostPostConstruct
//!	PostPost Construction
//!
//------------------------------------------------------------------------------------------
void EntityInteractableSpawner::OnPostPostConstruct()
{
	Interactable::OnPostPostConstruct();

	// Build our pools
	DataObject* pDefOb = ObjectDatabase::Get().GetDataObjectFromPointer(m_pTemplate);
	ntError_p(pDefOb, ("Cannot find template for %s\n", ntStr::GetString(GetName())));
	m_SpawnPool.BuildPool(pDefOb, m_iSize, ntStr::GetString(GetName()));
}


//------------------------------------------------------------------------------------------
//!
//!	EntityInteractableSpawner::Spawn
//!	Spawn an object
//!
//------------------------------------------------------------------------------------------
void EntityInteractableSpawner::Spawn(CEntity* pParent)
{
	ntError(pParent);

	// Get an item out of the pool
	CEntity* pItem = m_SpawnPool.ActivateNewItem();

	if(!pItem)
	{
		user_warn_msg(("Could not activate a new item for %s\n", ntStr::GetString(GetName())));

		Message msg(msg_exitstate);
		pParent->GetMessageHandler()->Receive(msg);
		return;
	}

	// Equip it...
	Message msg(msg_equip);
	msg.SetEnt(CHashedString(HASH_STRING_OTHER), m_pOther);
	pItem->GetMessageHandler()->Receive(msg);

	// We don't want to immediately despawn it...
	m_bDeferDespawns = true;
}

