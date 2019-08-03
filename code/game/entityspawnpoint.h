//------------------------------------------------------------------------------------------
//!
//!	\file game/entityspawnpoint.h
//!	Definition of the Spawn Point object
//!
//------------------------------------------------------------------------------------------

#ifndef	_ENTITY_SPAWNPOINT_H
#define	_ENTITY_SPAWNPOINT_H

//------------------------------------------------------------------------------------------
// For Inheritance
//------------------------------------------------------------------------------------------
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityai.h"


//------------------------------------------------------------------------------------------
// Forward Declares
//------------------------------------------------------------------------------------------
class EntitySpawnerGlobalProps;


//------------------------------------------------------------------------------------------
//!
//! SpawnPoint
//! SpawnPoint definition
//!
//------------------------------------------------------------------------------------------
struct SpawnPoint
{
	void OnPostConstruct();

	CPoint pt;
	CQuat rot;
	CHashedString sName;

	bool IsValid(bool bOverrideVisChecks);
};



//------------------------------------------------------------------------------------------
//!
//! Spawnee_Def
//! Spawnee definition - Basically an AI which we then clone to make a whole pool of AIs
//!
//------------------------------------------------------------------------------------------
class Spawnee_Def : public AI
{
public:
	Spawnee_Def() {}
	~Spawnee_Def() {}

	void SetSpawnPool(SpawnPool* pSpawnPool) {m_pSpawnPool = pSpawnPool;}

	CHashedString GetAnimatorContainerName() const { return CHashedString(m_sAnimationContainer); }

	// Prevent post & postpost construction
	void OnPostConstruct()     {;}
	void OnPostPostConstruct() {;}

	AI* CreateClone(const char* sName);
};


//------------------------------------------------------------------------------------------
//!
//! SpawnPool
//! Spawn Pool Definition
//!
//------------------------------------------------------------------------------------------
class SpawnPool
{
public:
	SpawnPool() : m_bBuilt(false) {;}
	void OnPostConstruct();

	void BuildPool();
	void PauseHack(); // Temporary until areamanager change

	const Spawnee_Def* GetTemplate() {return m_pTemplate;}
	int                GetSize()     {return m_iSize;}
	int                GetActive()   {return m_activeSpawnees.size();}
	int                GetInactive() {return m_inactiveSpawnees.size();}

	AI* Spawn(const CPoint& pt, const CQuat& rot);
	void DeSpawn(AI* pAI);

	void Update();

	const CHashedString& GetName() {return m_sName;}

// Serialised Members
private:
	HAS_INTERFACE(SpawnPool);
	Spawnee_Def* m_pTemplate;
	int          m_iSize;

// Other Members
private:
	// The Pool of Spawnee AIs
	typedef ntstd::List<AI*, Mem::MC_ENTITY> Spawnee_List;
	Spawnee_List m_activeSpawnees;
	Spawnee_List m_inactiveSpawnees;

	// The Equipment Pool - Each AI can have multiple child entities that it requires to be spawned with them
	enum WeaponType {WT_LEFT, WT_RIGHT, WT_RANGED, WT_COUNT, WT_FIRST=WT_LEFT, WT_LAST=WT_RANGED};
	typedef ntstd::List<CEntity*,         Mem::MC_ENTITY> Weapon_List;
	Weapon_List m_activeWeapons[WT_COUNT];
	Weapon_List m_inactiveWeapons[WT_COUNT];

	// Other Data
	CHashedString m_sName;
	bool          m_bBuilt;

// Helper Methods
private:
	CEntity* FindInactiveWeapon(WeaponType eType);
};


//------------------------------------------------------------------------------------------
//!
//! EntitySpawner
//! Spawner entity
//!
//------------------------------------------------------------------------------------------
class EntitySpawner : public CEntity
{
public:
	EntitySpawner();
	virtual ~EntitySpawner();
	void OnPostConstruct();
	void OnPostPostConstruct();

	void Update();

	AI* Spawn(const CHashedString& sPool, const CHashedString& sPoint, bool bOverrideVisChecks);
	AI* Spawn(const CHashedString& sPool, const CPoint& pt, const CQuat& rot);

	void DebugRender(float& fX, float& fY);

	static void SetGlobalProps(const EntitySpawnerGlobalProps& props);

private:
	void BuildPools();
	void ClearPools() {;}

	SpawnPool*  GetSpawnPool(const Spawnee_Def* pType);
	SpawnPool*  GetSpawnPool(const CHashedString& sPool);
	SpawnPoint* GetSpawnPoint(const CHashedString& sPoint, bool bOverrideVisChecks);
	
private:
	HAS_INTERFACE(EntitySpawner);

	typedef ntstd::List<SpawnPool*,  Mem::MC_ENTITY> SpawnPool_List;
	SpawnPool_List m_poolList;

	typedef ntstd::List<SpawnPoint*, Mem::MC_ENTITY> SpawnPoint_List;
	SpawnPoint_List m_spawnpointList;

	bool m_bActive;

	// Temp Hack
	bool m_bFirstFrame;

public:
	static int m_iMaxActiveSpawnees;
	static int m_iTotalActiveSpawnees;
};


//------------------------------------------------------------------------------------------
//!
//! EntitySpawnerGlobalProps
//! Global Properties for Spawners
//!
//------------------------------------------------------------------------------------------
class EntitySpawnerGlobalProps
{
public:
	void OnPostConstruct() {EntitySpawner::SetGlobalProps(*this);}

	int m_iMaxActiveSpawnees;
};

#endif // _ENTITY_SPAWNPOINT_H
