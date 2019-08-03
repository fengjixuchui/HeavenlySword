//--------------------------------------------------
//!
//!	\file entitymanager.h
//!	Singleton that looks after all the entities in the game
//!
//--------------------------------------------------

#if !defined( GAME_ENTITYMANAGER_H)
#define GAME_ENTITYMANAGER_H

#include "game/commandresult.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entitymanager.h"
#include "game/entityplayer.h"
#include "game/entityhero.h"

class Static;
class Player;
class CEntityQuery;
class EntityManagerUtility;
struct lua_State;
namespace NinjaLua { class LuaState; class LuaObject; }


/***************************************************************************************************
*
*	CLASS			CEntityManager
*
*	DESCRIPTION		Something needs to manage all the entities, right? Well.. this seems as good a 
*					place as any.  This will also provide the functionality for queries of the 
*					entities in existance, the framework for querying can be found in query.h.
*
***************************************************************************************************/

class	CEntityManager : public Singleton<CEntityManager>
{
friend	class CEntityBrowser;
friend	class EntityManagerUtility;

	struct EntityBucketInfo
	{
	public:
		const char* m_pcName; 
	};


public:
	bool m_bToRagdoll;
	
	// Construction Destruction
	CEntityManager();
	~CEntityManager();

	// Update hook
	void				Update();

	// Report on the currently active entities
	COMMAND_RESULT		GenerateReport();

	// reset the force field value of all the entity
	void				ResetForceField( void );

	// Find entity by name
	CEntity*			FindEntity( const char* pcEntityName );
	CEntity*			FindEntity( const CHashedString& name );

	// TEMP. just here for area system till this is reorganised.
	Static*				FindStatic( const char* pcEntityName );
	Static*				FindStatic( const CHashedString& name );

	// Perform a more complex entity query
	void				FindEntitiesByType( CEntityQuery& obQuery, uint32_t uiMask  );

	//! gets the player entity, returns null if there is no player
	Player*				GetPlayer( void ) {return m_pPrimaryPlayer;}

	//! set the primary player
	void				SetPrimaryPlayer(Player* pPlayer);

	void				UnparentAllHierarchies ();

	// To be called by entities themselves on construction
	void				Add( CEntity* pobEntity );
	void				Remove( CEntity* pobEntity );
	void				Add( Static* pobEntity );
	void				Remove( Static* pobEntity );
	void				Bucket( CEntity* pobEntity );


	const ntstd::List<Static*>& GetStaticEntityList() const
	{
		return *(ntstd::List<Static*>*)&m_aobEntityBuckets[CEntity::EntIndex_Static];
	}

	ntstd::List<Static*>& GetStaticEntityList()
	{
		return *(ntstd::List<Static*>*)&m_aobEntityBuckets[CEntity::EntIndex_Static];
	}

	HAS_LUA_INTERFACE()

private:

	static volatile uint32_t s_bPassedTest;
	static void FindEntityFunctionTaskAdaptor( void*, void* );
	void FindEntitiesHelper( CEntityQuery& obQuery, CEntity::EntIndex eIndex );


	static int GetEntityForLua( NinjaLua::LuaState& );
	static int SetEntityForLua( NinjaLua::LuaState& );

	QuickPtrList<CEntity>	m_entities;
	QuickPtrList<Static>	m_statics;
	ntstd::List<CEntity*>	m_obEntitiesToDelete;

	// Typedef for entity bucket container
	typedef ntstd::List<CEntity*> EntityBucket;
	typedef ntstd::List<CEntity*>::iterator EntityIterator;
	typedef EntityBucket* BucketIterator;

	// Each bucket in this array is to hold an entity of differing type
	EntityBucket m_aobEntityBuckets[CEntity::m_iNumEntTypes];
	static const EntityBucketInfo m_aobEntityBucketInfos[CEntity::m_iNumEntTypes];

	// Begin and End for bucket array, follows same usage pattern as STL... end is off the end
	BucketIterator BucketBegin() {return &m_aobEntityBuckets[0];}
	BucketIterator BucketEnd() {return &m_aobEntityBuckets[CEntity::m_iNumEntTypes];}

	// Access bucket by type mask (slighty slower than below)
	EntityBucket& GetBucket(CEntity::EntType eType);

	// Access bucket by type index (faster than above)
	EntityBucket& GetBucket(CEntity::EntIndex eIndex)
	{
		ntAssert(eIndex < CEntity::m_iNumEntTypes);
		return m_aobEntityBuckets[eIndex];
	}

	// The primary player
	Player* m_pPrimaryPlayer;

	// Some stats
	static int32_t m_iCurrentQueries;
	static int32_t m_iMaxQueries;

};

LV_DECLARE_USERDATA(CEntityManager);

#endif // GAME_ENTITYMANAGER_H
