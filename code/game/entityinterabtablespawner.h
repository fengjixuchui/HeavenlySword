//------------------------------------------------------------------------------------------
//!
//!	\file game/entityinterabtablespawner.hh
//!	Definition of the Spawn Point object
//!
//------------------------------------------------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_SPAWNER_H
#define	_ENTITY_INTERACTABLE_SPAWNER_H

//------------------------------------------------------------------------------------------
// For Inheritance
//------------------------------------------------------------------------------------------
#include "game/entity.h"
#include "game/entity.inl"

class DataObject;


//------------------------------------------------------------------------------------------
//!
//! GenericSpawnPool
//! 
//!
//------------------------------------------------------------------------------------------
class GenericSpawnPool
{
public:
	void     BuildPool(DataObject* pTemplate, int iSize, const char* pcPrefix);
	CEntity* ActivateNewItem();
	void     Update();

private:
	typedef ntstd::List<CEntity*, Mem::MC_ENTITY> Item_List;
	Item_List m_activeItems;
	Item_List m_inactiveItems;
};



//------------------------------------------------------------------------------------------
//!
//! EntityInteractableSpawner
//! Spawner Entity Definition
//!
//------------------------------------------------------------------------------------------
class EntityInteractableSpawner : public Interactable
{
public:
	EntityInteractableSpawner();
	~EntityInteractableSpawner() {;}

	void OnPostConstruct();
	void OnPostPostConstruct();

	virtual const CUsePointAttr* GetUsePointAttributes()	const {return m_pUsePointAttrs;}

	void Update() {if(!m_bDeferDespawns) m_SpawnPool.Update(); m_bDeferDespawns=false;}

	const CHashedString& GetWalkToAnim()          {return m_sWalkToAnim;}
	const CHashedString& GetRunToAnim()           {return m_sRunToAnim;}
	const CHashedString& GetUseAnim()             {return m_bRunningUse ? m_sRunningUseAnim : m_sNormalUseAnim;}
	float                GetUseAnimPickupTime()   {return m_bRunningUse ? m_fRunningPickupTime : m_fNormalPickupTime;}

	void                 Spawn(CEntity* pParent);

private:
	GenericSpawnPool m_SpawnPool;

	// XML Exposed Data
	CHashedString  m_sWalkToAnim;
	CHashedString  m_sRunToAnim;
	CHashedString  m_sNormalUseAnim;
	CHashedString  m_sRunningUseAnim;
	float          m_fRunningPickupTime;
	float          m_fNormalPickupTime;
	void*          m_pTemplate;
	int            m_iSize;
	CUsePointAttr* m_pUsePointAttrs;

public:
	CEntity* m_pOther;
	bool     m_bRunningUse;
	bool     m_bDeferDespawns;

	HAS_INTERFACE(EntityInteractableSpawner)
};


#endif // _ENTITY_INTERACTABLE_SPAWNER_H
