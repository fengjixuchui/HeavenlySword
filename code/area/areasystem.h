//--------------------------------------------------
//!
//!	\file areasystem.h
//!	Implements an entity based area system
//!
//--------------------------------------------------

#ifndef AREA_SYSTEM_H
#define AREA_SYSTEM_H

class CEntity;
class SectorLoadTrigger;
class SectorTransitionPortal;
class ObjectContainer;
class AreaManager;

//--------------------------------------------------
//!
//!	AreaSystem
//! Useful functions, typedefs and constants
//!
//--------------------------------------------------
namespace AreaSystem
{
	static const int32_t	NUM_AREAS = 32;
	static const int32_t	INVALID_AREA = -1;

	// is this Area ID within the valid range?
	static inline bool IsValidAreaNumber( int32_t iAreaNumber ) 
	{
		return ((iAreaNumber > 0) && (iAreaNumber <= NUM_AREAS));
	}

	typedef ntstd::Set<CEntity*>					EntitySet;
	typedef ntstd::List<CEntity*>					EntityList;
	typedef ntstd::List<SectorLoadTrigger*>			TriggerList;
	typedef ntstd::List<SectorTransitionPortal*>	PortalList;
};

//--------------------------------------------------
//!
//!	Area
//! Area class that is only used by AreaManager
//! Note: Entities can exist in multiple areas and 
//! can move between them, triggers and portals do not,
//! and cannot, respectively.
//!
//--------------------------------------------------
class Area
{
public:
	Area() { Reset(); }

	inline void Reset()
	{
		m_entities.clear();
		m_triggers.clear();
		m_portals.clear();
		m_eStatus = UNLOADED;
	}

	enum AREA_STATUS
	{
		UNLOADED,
		LOADING,
		LOADED,
		VISIBLE_NOT_ACTIVE,
		VISIBLE_AND_ACTIVE,
	};

	// query area state
	inline AREA_STATUS	GetStatus() const				{ return m_eStatus; }
	inline void			SetStatus( AREA_STATUS status )	{ m_eStatus = status; }
	bool				Empty() const					{ return m_entities.empty(); }

	// add and remove objects to area
	inline void Add( SectorLoadTrigger* pObj )			{ m_triggers.push_back(pObj); }
	inline void Add( SectorTransitionPortal* pObj )		{ m_portals.push_back(pObj); }	
	inline void Add( CEntity* pObj );
	inline void Remove( CEntity* pObj );

	const AreaSystem::EntitySet&	GetEnts()		const { return m_entities; }
	const AreaSystem::TriggerList&	GetTriggers()	const { return m_triggers; }
	const AreaSystem::PortalList&	GetPortals()	const { return m_portals; }

private:
	AreaSystem::EntitySet	m_entities;
	AreaSystem::TriggerList	m_triggers;
	AreaSystem::PortalList	m_portals;
	AREA_STATUS				m_eStatus;
};

//--------------------------------------------------
//!
//!	Object that summaries the membership status of 
//! mutlitple areas (represented as integers in the
//! the range 1 -> 32). Is essentially an optimised
//! set of ints.
//!
//--------------------------------------------------
class AreaInfo
{
private: 
	static const uint32_t REQURE_AUTO_ASSIGN = 0;

public:
	AreaInfo() : m_iAreaInfo(REQURE_AUTO_ASSIGN) {};

	// pass through inline for clarity
	inline bool Within( int32_t iAreaNumber ) const { return Includes(iAreaNumber); }

private:
	// only our friends can construct us with info, or manipulate us
	friend class CEntity;
	friend class Static;
	friend class AreaManager;

	// construct from mapped information (currently a bitmask, should
	// really be a list of area identifiers)
	explicit AreaInfo( uint32_t iMappedAreaInfo ) : m_iAreaInfo(iMappedAreaInfo) {}

	// Does this area info block overlap with us?
	inline bool Intersects( const AreaInfo& with ) const { return (m_iAreaInfo & with.m_iAreaInfo); }

	// Does this area info match us ?
	inline bool Equals( const AreaInfo& with ) const { return (m_iAreaInfo == with.m_iAreaInfo); }

	// Does our value signify we require area membership assignment?
	inline bool RequireAreaAutoAssignment() const { return (m_iAreaInfo == REQURE_AUTO_ASSIGN); }

	// Add this area identifier to our set.
	inline void AddArea( int32_t iAreaNumber )
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		m_iAreaInfo |= ( 1 << (iAreaNumber-1) );
	}

	// Remove this area identifier from our set.
	inline void RemoveArea( int32_t iAreaNumber )
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		m_iAreaInfo &= ~( 1 << (iAreaNumber-1) );
	}

	// Is this area within this set?
	inline bool Includes( int32_t iAreaNumber ) const
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		return (m_iAreaInfo & ( 1 << (iAreaNumber-1) ));
	}

	// integer that sumarises set contents concisely
	// NB, if we need more than NUM_AREAS (32) areas in a level, we change this
	// to something else, such as an arbitary list of area IDs... 
	uint32_t m_iAreaInfo;
};

//--------------------------------------------------
//!
//!	AreaManager
//! Controls visibility and activity status of all 
//! entities in the game, and the loading and unloading
//! of the resources required by those entities.
//!
//! In MrEd, entities are mapped within areas numbered
//! from 1 to 32. By default an entity is only visible
//! when that area is visible, and active when that area
//! is active. 
//!	However, entities can belong to multiple areas, via
//! a 'SectorBits' exported field. Their in-game area
//! membership is contained in their m_areaInfo object.
//! This object drives what actual area objects the ent
//! is inserted in when added to the area manager.
//! 
//! A principle entity (such as the player) is tracked.
//! When they intersect with a load volume, the 
//! relevent area is loaded and made visible. When the
//! entity hits a transition portal, the relevant area
//! is made active.
//!
//! The area manager limits the game to a maximum of 3 loaded
//! (and thus visible) areas, 1 of which is active. It is
//! more likely you'd have 2 loaded and visible, and another
//! asynchronously loading in. These areas are reflected 
//! in the 'current', 'last' and 'next' area identifiers.
//! when a load volume is hit, its area becomes the
//! 'next' sector. if there is already a 'next' sector, this
//! is unloaded first. when a portal is hit, 'next' becomes
//! 'current' and is activated, 'current' becomes 'last'
//! and is deactivated. ('last' becomes 'next' so it doesnt
//! have to be unloaded untill another load request comes along ).
//!
//! Should the 3 area rule need to be changed, the
//! only thing that needs adjusting is the area manager
//! update function, all else should remain the same.
//!
//!	New Feature 01.09.06.
//!	Load, activate and unload are now exposed as lua commands
//! for the designers to use via script. They still operate
//! within the 3 area framework, and operate the same as
//! load volume or portal activation.
//!
//! Should the number of areas in a level exceed 32,
//! the AreaInfo object needs to be changed to reflect
//! this, as does its initialisation and the value
//! AreaSystem::NUM_AREAS
//!
//--------------------------------------------------
class AreaManager : public Singleton<AreaManager>
{
public:
	static const int32_t ACTIVATE_FIRST = -1;

	AreaManager();
	~AreaManager();

	// state control of area system
	void Reset();
	void Update();
	void ActivateLevelFromCheckpoint( int nCheckpointID );
	void ActivateLevel( int iAreaToActivate = ACTIVATE_FIRST );
	void ForceReactivateLevel( int iAreaToActivate );
	inline void SignalLoadingLevel();

	// insert or remove entities from area system
	void AddEntity( CEntity* pEntity );
	void RemoveEntity( CEntity* pEntity );

	// area system control volumes
	void AddLoadTrigger( SectorLoadTrigger* pLoadTrigger );
	void AddTransitionPortal( SectorTransitionPortal* pPortal );

	// entity area status manipulation
	void SetToEntityAreas( const CEntity* pSrc, CEntity* pDest );	// set entity to same areas as this other entity
	void SetToActiveAreas( CEntity* pEntity );						// set entity to the currently active areas
	void SetToAlwaysActive( CEntity* pEntity );						// set entity to be always active

	// what entity drives the area system
	inline void SetPrinclipleEntity( CEntity* pPrinciple ) { m_pPrincipleEntity = pPrinciple; }

	// test status of entity system
	inline bool Inactive()		const { return (m_eState == AS_INACTIVE); }
	inline bool LoadingLevel()	const { return (m_eState == AS_LOADING_LEVEL); }
	inline bool LevelActive()	const { return (m_eState == AS_LEVEL_ACTIVE); }

	// get hold of what areas are visible and active
	int32_t	GetCurrActiveArea() const { return m_iCurrActiveArea; }
	int32_t	GetLastActiveArea() const { return m_iLastActiveArea; }
	int32_t	GetNextActiveArea() const { return m_iNextActiveArea; }

	// change the status of areas
	// Caution! Know what your doing here if you use these!
	void LoadNextArea( int32_t iAreaToLoad );
	void ActivateArea( int32_t iAreaToActivate, SectorTransitionPortal* pLastPortal );
	void UnloadArea( int32_t iAreaToUnload );

	// test the status of areas
	inline bool IsAreaLoaded( int32_t iArea );
	inline bool IsAreaActive( int32_t iArea );

private:
	// Structs for auto assignment of Area ID's
	typedef ntstd::Map<ObjectContainer*,int32_t> ContainerLookup;
	ContainerLookup	m_resolvedContainers;

	// called to find the XML container ID for this object
	int32_t ResolveAreaContainer( const void* ptr );

	// state of area system
	enum STATE
	{
		AS_INACTIVE,
		AS_LOADING_LEVEL,
		AS_LEVEL_ACTIVE,
	};

	STATE m_eState;

	// our current state that runs on 2 visible-inactive and 1 visible-active areas.
	// this is easily changed to a system with more than these limits: just change Update().
	int32_t	m_iCurrActiveArea;
	int32_t	m_iLastActiveArea;
	int32_t	m_iNextActiveArea;
	
	// set of areas for the currently active level
	// this class just maps area numbers to an internal
	// array, hiding the fact that area numbers start at 1.
	class Areas
	{
	public:
		inline Area& operator [] ( int32_t iAreaNumber )
		{
			ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
			return m_areas[iAreaNumber-1];
		}

	private:
		Area m_areas[ AreaSystem::NUM_AREAS ];
	};

	Areas m_areas;

	// entity status management
	void		Entity_SetVisibility( CEntity* pEntity, bool bVisible );
	inline void Entity_SetActivity( CEntity* pEntity, bool bActive );

	// area status management
	inline void Area_MakeVisible( int32_t iAreaNumber );
	inline void Area_MakeInvisible( int32_t iAreaNumber );
	inline void Area_MakeActive( int32_t iAreaNumber );
	inline void Area_MakeInactive( int32_t iAreaNumber );

	inline void Area_LoadSync( int32_t iAreaNumber );
	inline void Area_LoadAsync( int32_t iAreaNumber );
	inline void Area_UnloadSync( int32_t iAreaNumber );
	inline void Area_UnloadAsync( int32_t iAreaNumber );

	inline void Area_AbortLoadOrUnload( int32_t iAreaNumber, bool bAsync = true );
	inline void Area_LoadThenActivate( int32_t iAreaNumber );
	inline bool Area_LoadFinished( int32_t iAreaNumber );

	// total set of entities registerd with the area manager
	AreaSystem::EntitySet	m_allEnts;
	AreaSystem::EntityList	m_delayedInsert;

	// sumarises status for sectors
	AreaInfo	m_visibleAreas;
	AreaInfo	m_activeAreas;

	// helpful info
	void DebugUpdate();
	bool m_bDebugRender;
	
	const CEntity*			m_pPrincipleEntity;
	SectorTransitionPortal*	m_pLastPortal;
};

#endif // AREA_SYSTEM_H
