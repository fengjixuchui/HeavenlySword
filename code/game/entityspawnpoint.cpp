//------------------------------------------------------------------------------------------
//!
//!	\file game/entityspawnpoint.cpp
//!	Definition of the Spawn Point object
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "game/entityspawnpoint.h"


#include "input/inputhardware.h"
#include "game/renderablecomponent.h"
#include "game/message.h"
#include "game/weapons.h"
#include "fsm.h"
#include "objectdatabase/dataobject.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "core/visualdebugger.h"
#include "physics/system.h"


const float EXTRA_WEAPONS    = 1.f;

int EntitySpawner::m_iTotalActiveSpawnees = 0;
int EntitySpawner::m_iMaxActiveSpawnees = 32;


//------------------------------------------------------------------------------------------
// SpawnPoint XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(SpawnPoint, Mem::MC_ENTITY)
	PUBLISH_VAR_WITH_DEFAULT_AS(pt,  CPoint(0.0f, 0.0f, 0.0f), Position)
	PUBLISH_VAR_WITH_DEFAULT_AS(rot, CQuat(0.0f, 0.0f, 0.0f, 1.0f), Orientation)

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Spawnee_Def XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(Spawnee_Def, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(AI)
	COPY_INTERFACE_FROM(AI)

	OVERRIDE_DEFAULT(IsEnemy, "true")

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// SpawnPool XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(SpawnPool, Mem::MC_ENTITY)
	PUBLISH_PTR_AS(m_pTemplate, Template)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iSize, 0, Count)

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// EntitySpawnerGlobalProps XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(EntitySpawnerGlobalProps, Mem::MC_ENTITY)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMaxActiveSpawnees, 12, MaxActiveSpawnees)
	
	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// EntitySpawner XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(EntitySpawner, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_PTR_CONTAINER_AS(m_poolList,       Pools)
	PUBLISH_PTR_CONTAINER_AS(m_spawnpointList, Spawners)

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)	
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	SpawnPoint::OnPostConstruct
//!	Post Construction
//!
//------------------------------------------------------------------------------------------
void SpawnPoint::OnPostConstruct() 
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	ntAssert(pDO);
	sName = CHashedString(pDO->GetName());
}


//------------------------------------------------------------------------------------------
//!
//!	SpawnPoint::IsValid()
//!	Check visibility and proximity...
//!
//------------------------------------------------------------------------------------------
bool SpawnPoint::IsValid(bool bOverrideVisChecks)
{
	// Don't spawn in this point if it's on camera
	if(bOverrideVisChecks && CamMan::Get().GetPrimaryView()->IsInView(pt))
		return false;
	
	// Don't spawn in this point if it's too close to the player


	// Don't spawn inside anything

	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	SpawnPool::OnPostConstruct
//!	Post Construction
//!
//------------------------------------------------------------------------------------------
void SpawnPool::OnPostConstruct() 
{
	// Set our name string
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	ntAssert(pDO);
	m_sName = CHashedString(pDO->GetName());
}


//------------------------------------------------------------------------------------------
//!
//!	Spawnee_Def::CreateClone()
//!	Bring in the clones!
//!
//------------------------------------------------------------------------------------------
AI* Spawnee_Def::CreateClone(const char* sName) 
{
	// An evil twin is born...
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Spawnee_Def", sName, GameGUID(), 0, true, false);
	Spawnee_Def* pEvilTwin = (Spawnee_Def*)pDO->GetBasePtr();
	pEvilTwin->SetSpawnPool(GetSpawnPool());

	// Copy over all the exposed parameters
	//NT_MEMCPY(pEvilTwin, this, sizeof(Spawnee_Def)); // Can't do this, it's just too evil...
	DataObject*       pDefOb         = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	StdDataInterface* pDefInterface  = ObjectDatabase::Get().GetInterface(pDefOb);
	for(ntstd::Vector<DataInterfaceField*, Mem::MC_ODB>::const_iterator fcIt = pDefInterface->begin(); fcIt != pDefInterface->end(); ++fcIt)
	{
		(*fcIt)->SetData(pDO, (*fcIt)->GetData(pDefOb));
	}

	// Then Post and PostPost Construction
	((AI*)pEvilTwin)->OnPostConstruct();
	((AI*)pEvilTwin)->OnPostPostConstruct();

	// Should be paused and invisible to begin with...
	pEvilTwin->Pause(true, true);
	pEvilTwin->Hide();

	// All Done
	return (AI*)pEvilTwin;
}


//------------------------------------------------------------------------------------------
//!
//!	SpawnPool::BuildPool()
//!	Fill this pool
//!
//------------------------------------------------------------------------------------------
void SpawnPool::BuildPool()
{
	// Sanity Checks
	ntAssert(m_pTemplate);
	if(m_bBuilt)
	{
		ntPrintf("Spawnpool already built\n");
		return;
	}

	// Set the spawnpool on the spawnee template
	m_pTemplate->SetSpawnPool(this);

	// Commence Cloning
	static int s_iUnique = 0;
	int i;
	for(i = 0; i < m_iSize; i++)
	{
		char sName[128];
		sprintf(sName, "%s_%d", "SPAWNEE", i + s_iUnique);

		// Clone the Spawnee from the template
		AI* pAI = m_pTemplate->CreateClone(sName);
		DeSpawn(pAI);
	}

	// Now Make all the weapons, allocate extra weapons
	if(m_pTemplate->GetWeaponsDef())
	{
		for(i = 0; i < int(ceilf(m_iSize * (1.f + EXTRA_WEAPONS))); i++)
		{
			char sName[128];
			sprintf(sName, "%s_%d", "SPAWNEE", i + s_iUnique);
			ntstd::Vector<CEntity*> vecWeapons = m_pTemplate->GetWeaponsDef()->PreCreateWeapons(sName, GetTemplate());

			// Do this inside the precreate function instead...
			for(ntstd::Vector<CEntity*>::iterator it = vecWeapons.begin(); it != vecWeapons.end(); it++)
			{
				if(*it)
				{
					(*it)->Pause(true, true);
					(*it)->Hide();
				}
			}

			m_inactiveWeapons[WT_LEFT].push_back(vecWeapons[WT_LEFT]);
			m_inactiveWeapons[WT_RIGHT].push_back(vecWeapons[WT_RIGHT]);
			m_inactiveWeapons[WT_RANGED].push_back(vecWeapons[WT_RANGED]);
		}
	}

	// Done
	s_iUnique += i;
	m_bBuilt = true;
}


//------------------------------------------------------------------------------------------
//!
//!	SpawnPool::Spawn()
//!	Spawn an AI from this pool
//!
//------------------------------------------------------------------------------------------
AI* SpawnPool::Spawn(const CPoint& pt, const CQuat& rot)
{
	// Find an AI to Spawn
	//--------------------------------
	if(m_inactiveSpawnees.empty())
		return 0;

	AI* pSpawnee = m_inactiveSpawnees.back();

	if(!pSpawnee)
		return 0;

	// And any equipment it might need
	//--------------------------------
	ntstd::Vector<CEntity*> vecWeapons;
	vecWeapons.push_back(FindInactiveWeapon(WT_LEFT));
	vecWeapons.push_back(FindInactiveWeapon(WT_RIGHT));
	vecWeapons.push_back(FindInactiveWeapon(WT_RANGED));

	// Spawn the entity
	//--------------------------------
	if(!pSpawnee->Respawn(vecWeapons))
	{
		return 0;
	}

	// All good, put it in the right set
	//---------------------------------
	m_inactiveSpawnees.pop_back();
	m_activeSpawnees.push_back(pSpawnee);

	// Set our initial position
	//---------------------------------
	pSpawnee->SetPosition(pt);
	pSpawnee->SetRotation(rot);

	// Success
	//---------------------------------
	return pSpawnee;
}


//------------------------------------------------------------------------------------------
//!
//!	SpawnPool::FindInactiveWeapon()
//!	Find a free weapon for a spawnee
//!
//------------------------------------------------------------------------------------------
CEntity* SpawnPool::FindInactiveWeapon(WeaponType eType)
{
	if(m_inactiveWeapons[eType].empty())
		return 0;

	CEntity* pWeapon = m_inactiveWeapons[eType].back();
	if(pWeapon)
	{
		m_inactiveWeapons[eType].pop_back();
		m_activeWeapons[eType].push_back(pWeapon);
		ntError(!pWeapon->GetParentEntity());
	}
	else
	{
		// Grab an active weapon that's been dropped
		for(Weapon_List::iterator it = m_activeWeapons[eType].begin(); it != m_activeWeapons[eType].end(); it++)
		{
			// Don't Despawn if someone's holding us...
			if((*it)->GetParentEntity())
				continue;

			pWeapon = (*it);
			pWeapon->Pause(true);
			pWeapon->Hide();
			return pWeapon;
		}
	}

	return pWeapon;
}


//------------------------------------------------------------------------------------------
//!
//!	SpawnPool::PauseHack()
//!	Temprorary hack until Wil changes the area manager
//!
//------------------------------------------------------------------------------------------
void SpawnPool::PauseHack()
{
	for(Spawnee_List::iterator it = m_inactiveSpawnees.begin(); it != m_inactiveSpawnees.end(); it++)
	{
		(*it)->Pause(true, true);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	SpawnPool::DeSpawn()
//!	Make this entity ready to spawn again
//!
//------------------------------------------------------------------------------------------
void SpawnPool::DeSpawn(AI* pAI)
{
	ntAssert(pAI && pAI->GetSpawnPool() == this);

	//ntPrintf("Despawn AI (%s)\n", pAI->GetName().c_str());

	pAI->Pause(true, true);
	pAI->Hide();

	// Probably the entity has had it's weapons deattached, but if this is it
	// does then pause and hide them.  Note that entities are built with weapons
	// attached on their first incarnation.
	if(pAI->GetLeftWeapon())
	{
		pAI->GetLeftWeapon()->Pause(true, true);
		m_inactiveWeapons[WT_LEFT].push_back(pAI->GetLeftWeapon());

		pAI->GetLeftWeapon()->SetParentEntity(0);
		pAI->GetLeftWeapon()->GetHierarchy()->GetRootTransform()->RemoveFromParent();
		pAI->SetLeftWeapon(0);
	}

	if(pAI->GetRightWeapon())
	{
		pAI->GetRightWeapon()->Pause(true, true);
		m_inactiveWeapons[WT_RIGHT].push_back(pAI->GetRightWeapon());

		pAI->GetRightWeapon()->SetParentEntity(0);
		pAI->GetRightWeapon()->GetHierarchy()->GetRootTransform()->RemoveFromParent();
		pAI->SetRightWeapon(0);
	}

	if(pAI->GetRangedWeapon())
	{
		pAI->GetRangedWeapon()->Pause(true, true);
		m_inactiveWeapons[WT_RANGED].push_back(pAI->GetRangedWeapon());

		pAI->GetRangedWeapon()->SetParentEntity(0);
		pAI->GetRangedWeapon()->GetHierarchy()->GetRootTransform()->RemoveFromParent();
		pAI->SetRangedWeapon(0);
	}

	// Remove it's entry in the active set...
	m_activeSpawnees.remove(pAI);

	// ...and move it to the inactive set.
	m_inactiveSpawnees.push_back(pAI);
	EntitySpawner::m_iTotalActiveSpawnees--;
}


//------------------------------------------------------------------------------------------
//!
//!	SpawnPool::Update()
//!	Update this Spawn Pool
//!
//------------------------------------------------------------------------------------------
void SpawnPool::Update()
{
	// Despawn weapons associated with this pool if we can...
	for(int i = WT_FIRST; i != WT_LAST; i++)
	{
		for(Weapon_List::iterator it = m_activeWeapons[WeaponType(i)].begin(); it != m_activeWeapons[WeaponType(i)].end(); it++)
		{
			// Don't Despawn if someone's holding us...  Or if we're already despawned!
			if((*it)->GetParentEntity() || (*it)->IsPaused())
				continue;

			// If it's not visible then despawn it...
			if(!CamMan::Get().GetPrimaryView()->IsInView((*it)->GetPosition()) && 
				(!(*it)->IsInteractable() || !(*it)->ToInteractable()->InteractableInUse()))
			{
				(*it)->Pause(true, true);
				(*it)->Hide();
				m_inactiveWeapons[WeaponType(i)].push_back(*it);
				m_activeWeapons[WeaponType(i)].erase(it);
				break;
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//! Spawner State Machine
//!
//------------------------------------------------------------------------------------------
STATEMACHINE(SPAWNER_FSM, EntitySpawner)
	SPAWNER_FSM(bool bActive)
	{
		if(bActive)
		{
			SET_INITIAL_STATE(State_Active);
		}
		else
		{
			SET_INITIAL_STATE(State_Inactive);
		}
	}

	void DebugRender(float& fX, float& fY)
	{
		((_baseclass*)GetBase())->DebugRender(fX, fY);
	}

	STATE(State_Active)
		BEGIN_EVENTS
			ON_ENTER
			END_EVENT(true)

			ON_UPDATE
			{
				ME->Update();
			}
			END_EVENT(true)

			EVENT(Deactivate)
				SET_STATE(State_Inactive);
			END_EVENT(true)

			EVENT(msg_spawn)
			{
				AI* pSpawnee = ME->Spawn(msg.GetHashedString(2), msg.GetHashedString(3), msg.GetBool(4));
				CEntity* pSender = msg.GetEnt("Sender");

				if(!pSpawnee)
				{
					if(pSender && pSender->GetMessageHandler())
					{
						Message msgFail(msg_spawn_fail);
						pSender->GetMessageHandler()->Receive(msgFail);
					}
				}
				else
				{
					if(pSender && pSender->GetMessageHandler())
					{
						Message msgFail(msg_spawn_succeed);
						msgFail.SetEnt("Spawnee", pSpawnee);	
						pSender->GetMessageHandler()->Receive(msgFail);
					}
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE // State_Active

	STATE(State_Inactive)
		BEGIN_EVENTS
			ON_ENTER
			END_EVENT(true)

			ON_UPDATE
			END_EVENT(true)

			EVENT(Activate)
				SET_STATE(State_Active);
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Inactive
END_STATEMACHINE


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::EntitySpawner()
//!	Default constructor
//!
//------------------------------------------------------------------------------------------
EntitySpawner::EntitySpawner()
{
	m_bFirstFrame = true;
	m_bActive     = true;
	m_eType       = EntType_Object;
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::OnPostConstruct()
//!	Post Construction
//!
//------------------------------------------------------------------------------------------
void EntitySpawner::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();

	// Create and attach the statemachine
	SPAWNER_FSM* pFSM = NT_NEW SPAWNER_FSM(m_bActive);
	ATTACH_FSM(pFSM);
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::OnPostPostConstruct()
//!	PostPost Construction
//!
//------------------------------------------------------------------------------------------
void EntitySpawner::OnPostPostConstruct()
{
	CEntity::OnPostPostConstruct();

	BuildPools();
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::~EntitySpawner()
//!	Default Destructor
//!
//------------------------------------------------------------------------------------------
EntitySpawner::~EntitySpawner()
{

}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::BuildPools()
//!	Create the AI pools
//!
//------------------------------------------------------------------------------------------
void EntitySpawner::BuildPools()
{
	for(SpawnPool_List::iterator it = m_poolList.begin(); it != m_poolList.end(); it++)
	{
		(*it)->BuildPool();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::Update()
//!	Spawner Update
//!
//------------------------------------------------------------------------------------------
void EntitySpawner::Update()
{
	// First Frame Update Hack - Required until Wil changes the area manager.
	if(m_bFirstFrame)
	{
		m_bFirstFrame = false;

		for(SpawnPool_List::iterator it = m_poolList.begin(); it != m_poolList.end(); it++)
		{
			SpawnPool* pPool = *it;
			pPool->PauseHack();
		}
	}

#ifndef _RELEASE
	// Button press hacks
	if(CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_S))
	{
		CHashedString sPool("SpawnPool1");
		Spawn(sPool, CHashedString::nullString, false);
	}
#endif //_RELEASE

	if(CInputHardware::Get().GetKeyboard().IsKeyHeld(KEYC_PRINTSCREEN))
	{
		float f = 110.f;
		DebugRender(f, f);
	}
	
	// Update the Pools
	for(SpawnPool_List::iterator it = m_poolList.begin(); it != m_poolList.end(); it++)
	{
		(*it)->Update();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::Spawn()
//!	Spawn an entity in this pool
//!
//------------------------------------------------------------------------------------------
AI* EntitySpawner::Spawn(const CHashedString& sPool, const CHashedString& sPoint, bool bOverrideVisChecks)
{
	SpawnPoint* pSpawnPoint = GetSpawnPoint(sPoint, bOverrideVisChecks);

	if(pSpawnPoint) // && m_iTotalActiveSpawnees < m_iMaxActiveSpawnees)
	{
		AI* pSpawnee = Spawn(sPool, pSpawnPoint->pt, pSpawnPoint->rot);
		if(pSpawnee)
		{
			m_iTotalActiveSpawnees++;
		}

		return pSpawnee;
	}
	else
	{
		return 0;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::Spawn()
//!	Spawn an entity in this pool
//!
//------------------------------------------------------------------------------------------
AI* EntitySpawner::Spawn(const CHashedString& sPool, const CPoint& pt, const CQuat& rot)
{
	SpawnPool* pPool = GetSpawnPool(sPool);

	if(!pPool)
		return false;

	return pPool->Spawn(pt, rot);
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::GetSpawnPool()
//!	Who to spawn
//!
//------------------------------------------------------------------------------------------
SpawnPool* EntitySpawner::GetSpawnPool(const Spawnee_Def* pType)
{
	// Find a free entity in the pool
	for(SpawnPool_List::iterator it = m_poolList.begin(); it != m_poolList.end(); it++)
	{
		if((*it)->GetTemplate() == pType)
		{
			return *it;
		}
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::GetSpawnPool()
//!	Who to spawn
//!
//------------------------------------------------------------------------------------------
SpawnPool*  EntitySpawner::GetSpawnPool(const CHashedString& sPool)
{
	for(SpawnPool_List::iterator it = m_poolList.begin(); it != m_poolList.end(); it++)
	{
		if((*it)->GetName() == sPool)
			return *it;
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::GetSpawnPoint()
//!	Where to spawn
//!
//------------------------------------------------------------------------------------------
SpawnPoint* EntitySpawner::GetSpawnPoint(const CHashedString& sPoint, bool bOverrideVisChecks)
{
	SpawnPoint* pRet = 0;

	// Find a valid point to spawn at
	for(SpawnPoint_List::iterator it = m_spawnpointList.begin(); it != m_spawnpointList.end(); it++)
	{
		// Todo: We really need to randomly select one
		if((*it)->IsValid(bOverrideVisChecks))
		{
			pRet = *it;

			// Override if we've specifically asked for this one...
			if((*it)->sName == sPoint)
			return *it;
	}
	}

	return pRet;
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::DebugRender()
//!	
//!
//------------------------------------------------------------------------------------------
void EntitySpawner::DebugRender(float& fX, float& fY)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf2D(fX, fY, 0xffffffff, 0, "Total Active Spawnees = %d", m_iTotalActiveSpawnees);
	fY += 11.f;

	int i = 0;
	for(SpawnPool_List::iterator it = m_poolList.begin(); it != m_poolList.end(); it++, i++)
	{

		g_VisualDebug->Printf2D(fX, fY, 0xffffffff, 0, "Pool %d - %d Active %d Inactive",  i, (*it)->GetActive(), (*it)->GetInactive());
		fY += 11.f;
	}


	i = 0;
	for(SpawnPoint_List::iterator it = m_spawnpointList.begin(); it != m_spawnpointList.end(); it++, i++)
	{
		static unsigned int iaColours[7] = {0xff00ff00, 0xff0000ff, 0xff0000ff, 0xff00ffff, 0xffff00ff, 0xffffff00, 0xffffffff};
		if(i > 7)
			i = 0;

		g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(), (*it)->pt, 1.f, iaColours[i], DPF_WIREFRAME);

		CMatrix mat((*it)->rot, (*it)->pt);
		g_VisualDebug->RenderAxis(mat, 1.f); 
	}
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	EntitySpawner::SetGlobalProps()
//!	
//!
//------------------------------------------------------------------------------------------
void EntitySpawner::SetGlobalProps(const EntitySpawnerGlobalProps& props)
{
	m_iMaxActiveSpawnees = props.m_iMaxActiveSpawnees;
}
