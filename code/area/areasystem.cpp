//--------------------------------------------------
//!
//!	\file area.cpp
//!	An entity based area system
//!
//--------------------------------------------------

#include "area/areasystem.h"
#include "area/areasystem.inl"
#include "area/areadata.h"
#include "area/arearesourcedb.h"

#include "objectdatabase/dataobject.h"

// for checkpoints
#include "game/shellmain.h"
#include "game/shelllevel.h"
#include "game/entitycheckpoint.h"

// for debug stuff only
#include "core/visualdebugger.h"
#include "input/inputhardware.h"

#include "game/entitymanager.h"
#include "game/staticentity.h"

#include "physics/system.h"

using namespace AreaSystem;

// BINDFUNC:		AreaManager_LoadArea( int32_t iAreaID )
// DESCRIPTION:		Loads an area specified by iAreaID
static void AreaManager_LoadArea( int32_t iAreaID )
{
	ntAssert_p( AreaManager::Exists(), ("Cannot call AreaManager_LoadArea with no AreaManager singleton") );
	ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaID), ("AreaManager_LoadArea called with an invalid area ID") );
	AreaManager::Get().LoadNextArea( iAreaID );
}

// BINDFUNC:		AreaManager_ActivateArea( int32_t iAreaID )
// DESCRIPTION:		Activate an area specified by iAreaID
static void AreaManager_ActivateArea( int32_t iAreaID )
{
	ntAssert_p( AreaManager::Exists(), ("Cannot call AreaManager_ActivateArea with no AreaManager singleton") );
	ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaID), ("AreaManager_ActivateArea called with an invalid area ID") );
	AreaManager::Get().ActivateArea( iAreaID, 0 );
}

// BINDFUNC:		AreaManager_UnloadArea( int32_t iAreaID )
// DESCRIPTION:		Unload an area specified by iAreaID
static void AreaManager_UnloadArea( int32_t iAreaID )
{
	ntAssert_p( AreaManager::Exists(), ("Cannot call AreaManager_UnloadArea with no AreaManager singleton") );
	ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaID), ("AreaManager_UnloadArea called with an invalid area ID") );
	AreaManager::Get().UnloadArea( iAreaID );
}

// BINDFUNC:		AreaManager_IsAreaLoaded( int32_t iAreaID )
// DESCRIPTION:		Test if a given area is loaded already
static bool AreaManager_IsAreaLoaded( int32_t iAreaID )
{
	ntAssert_p( AreaManager::Exists(), ("Cannot call AreaManager_IsAreaLoaded with no AreaManager singleton") );
	ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaID), ("AreaManager_IsAreaLoaded called with an invalid area ID") );
	return AreaManager::Get().IsAreaLoaded( iAreaID );
}

// BINDFUNC:		AreaManager_IsAreaActive( int32_t iAreaID )
// DESCRIPTION:		Test if a given area is active already
static bool AreaManager_IsAreaActive( int32_t iAreaID )
{
	ntAssert_p( AreaManager::Exists(), ("Cannot call AreaManager_IsAreaActive with no AreaManager singleton") );
	ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaID), ("AreaManager_IsAreaActive called with an invalid area ID") );
	return AreaManager::Get().IsAreaActive( iAreaID );
}




//--------------------------------------------------
//!
//!	AreaManager::ctor
//!
//--------------------------------------------------
AreaManager::AreaManager()
{
	NT_NEW AreaResourceDB;
	Reset();

	// register our exposed lua functions
	NinjaLua::LuaObject globals = CLuaGlobal::Get().State().GetGlobals();
	globals.Register( "AreaManager_LoadArea",		AreaManager_LoadArea );
	globals.Register( "AreaManager_ActivateArea",	AreaManager_ActivateArea );
	globals.Register( "AreaManager_UnloadArea",		AreaManager_UnloadArea );
	globals.Register( "AreaManager_IsAreaLoaded",	AreaManager_IsAreaLoaded );
	globals.Register( "AreaManager_IsAreaActive",	AreaManager_IsAreaActive );	
}

//--------------------------------------------------
//!
//!	AreaManager::dtor
//!
//--------------------------------------------------
AreaManager::~AreaManager()
{
	Reset();
	AreaResourceDB::Kill();
}

//--------------------------------------------------
//!
//!	AreaManager::Reset
//! Called on level shutdown, and in our ctor
//!
//--------------------------------------------------
void AreaManager::Reset()
{
	// clear status of our areas
	m_eState = AS_INACTIVE;
	m_iCurrActiveArea = INVALID_AREA;
	m_iLastActiveArea = INVALID_AREA;
	m_iNextActiveArea = INVALID_AREA;
	
	// unload any areas
	// NB check for quick restarting here and dont unload re-used resources
	for( int32_t i = 1; i <= NUM_AREAS; i++ )
	{
		if (m_areas[i].GetStatus() == Area::VISIBLE_AND_ACTIVE)
			Area_MakeInactive(i);

		if (m_areas[i].GetStatus() > Area::UNLOADED)
			Area_AbortLoadOrUnload(i,false);

		m_areas[i].Reset();
	}

	m_visibleAreas = AreaInfo(0);
	m_activeAreas = AreaInfo(0);

	m_allEnts.clear();
	m_pPrincipleEntity = 0;
	m_pLastPortal = 0;
	m_bDebugRender = false;

	ntAssert( m_delayedInsert.empty() );
	m_resolvedContainers.clear();
}

//--------------------------------------------------
//!
//!	AreaManager::LoadNextArea
//! Request the asyc load of an area, mark it as
//! 'next'
//!
//--------------------------------------------------
void AreaManager::LoadNextArea( int32_t iAreaToLoad )
{
	ntError_p( iAreaToLoad != m_iCurrActiveArea, ("Trying to load the currently active area"));

	if (iAreaToLoad == m_iLastActiveArea)
	{
		// no need to do anything, this MUST already be loaded / loading
		ntError_p( m_areas[m_iLastActiveArea].GetStatus() >= Area::LOADING, ("last active area must be loaded already") );
		return;
	}

	// no need to do anything, as we're already processing this load request
	if (iAreaToLoad == m_iNextActiveArea)
		return;
	
	// already loading a next area, need to handle this
	if (m_iNextActiveArea != INVALID_AREA)
	{
		if (m_iLastActiveArea == INVALID_AREA)
		{
			// we have no last area, so we can shunt the current 'next' one to 'last'
			// and just process two loads simultaniously.
			m_iLastActiveArea = m_iNextActiveArea;
		}
		else
		{
			// stop processing the current next area
			Area_AbortLoadOrUnload( m_iNextActiveArea );
		}
	}

	// finally make the load request of the next area
	m_iNextActiveArea = iAreaToLoad;
	Area_LoadAsync( m_iNextActiveArea );
}

//--------------------------------------------------
//!
//!	AreaManager::ActivateArea
//!	Set this area to active (will block untill loaded
//! if its not here yet, so make sure you preload first)
//!	pLastPortal is the last portal (if any) that triggered
//! the load. This prevents activation flipping in the
//! case of overlapping portals.
//!
//--------------------------------------------------
void AreaManager::ActivateArea( int32_t iAreaToActivate, SectorTransitionPortal* pLastPortal )
{
	if (iAreaToActivate == m_iCurrActiveArea)
	{
		user_warn_p(0, ("Trying to activate the currently active area"));
		return;
	}

	// if this sector is not loaded we need to deal with that
	if (m_areas[iAreaToActivate].GetStatus() == Area::UNLOADED)
	{
		if	(
			(iAreaToActivate != m_iNextActiveArea) &&
			(m_iNextActiveArea != INVALID_AREA)
			)
		{
			// only unload next if we dont have a free area
			if ( m_iLastActiveArea != INVALID_AREA )
				Area_AbortLoadOrUnload(m_iNextActiveArea);
		}
		
		Area_LoadAsync( iAreaToActivate );
	}
	
	// activate the sector (spin if not loaded yet)
	Area_LoadThenActivate( iAreaToActivate );

	// no need to unload the last sector untill the next load request happens
	if (iAreaToActivate != m_iLastActiveArea)
		m_iNextActiveArea = m_iLastActiveArea;

	m_iLastActiveArea = m_iCurrActiveArea;
	m_iCurrActiveArea = iAreaToActivate;

	// turn off the processing of the last one
	Area_MakeInactive(m_iLastActiveArea);

	m_pLastPortal = pLastPortal;
}

//--------------------------------------------------
//!
//!	AreaManager::UnloadArea
//!	Unload this area (must be either last or next,
//! and loaded or loading)
//!
//--------------------------------------------------
void AreaManager::UnloadArea( int32_t iAreaToUnload )
{
	ntError_p( iAreaToUnload != m_iCurrActiveArea, ("Trying to unload the currently active area"));

	if (m_areas[iAreaToUnload].GetStatus() == Area::UNLOADED)
		return;

	if ( iAreaToUnload == m_iLastActiveArea )
	{
		Area_AbortLoadOrUnload(m_iLastActiveArea);

		if (m_iNextActiveArea != INVALID_AREA)
		{
			m_iLastActiveArea = m_iNextActiveArea;
			m_iNextActiveArea = INVALID_AREA;
		}
		else
		{
			m_iLastActiveArea = INVALID_AREA;
		}
	}
	else if ( iAreaToUnload == m_iNextActiveArea )
	{
		Area_AbortLoadOrUnload(m_iNextActiveArea);
		m_iNextActiveArea = INVALID_AREA;
	}
	else
	{
		ntError_p(0, ("Area %d is not the last or next area, so this call is invalid", iAreaToUnload) );
	}
}

//--------------------------------------------------
//!
//!	AreaManager::Update
//! Check for sector transitions, load requests and load finalises
//!
//--------------------------------------------------
void AreaManager::Update()
{
	ntError_p( LevelActive(), ("Must have an active level for this to be valid") );

	DebugUpdate();

	// just use the player if this hasnt been set explicitly.
	const CEntity* pPrincipleEnt = m_pPrincipleEntity;
	if ((pPrincipleEnt == 0) && (CEntityManager::Exists()))
		pPrincipleEnt = CEntityManager::Get().GetPlayer();

	if ( pPrincipleEnt )
	{
		CPoint focusPos = pPrincipleEnt->GetPosition();

		// check for area load requests
		//------------------------------------------------------------------------
		for (	TriggerList::const_iterator it = m_areas[m_iCurrActiveArea].GetTriggers().begin();
				it != m_areas[m_iCurrActiveArea].GetTriggers().end(); ++it )
		{
			// note: we could be inside multiple triggers, this is a mapping issue,
			// though we will assert if more than 3 are loaded
			if ((*it)->Inside(focusPos))
			{
				LoadNextArea( (*it)->m_iAreaToLoad );
				continue;
			}
		}

		// check for area transition requests
		//-------------------------------------------------------------------

		// NOTE, if the transition portal for the next area overlaps this one,
		// we have a problem, as we're going to flip back and forth between the areas.
		// A possible solution is to keep the last portal around and only transition
		// again if we're NOT within that one anymore...

		if( (m_pLastPortal) && (!m_pLastPortal->Inside(focusPos)) )
			m_pLastPortal = 0;

		for (	PortalList::const_iterator it = m_areas[m_iCurrActiveArea].GetPortals().begin();
				it != m_areas[m_iCurrActiveArea].GetPortals().end(); ++it )		
		{
			if( (*it)->Inside(focusPos) && (m_pLastPortal == 0) )
			{
				ActivateArea( (*it)->m_iAreaToActivate, *it );
				break;
			}
		}
	}

	// handle async loading of area(s).
	// this bit doesnt care about the 3 area rule and will quite happily try
	// to load any that have been requested, and make them all visible when they are
	//---------------------------------------------------------
	AreaResourceDB::Get().Update();

	for( int32_t i = 1; i <= NUM_AREAS; i++ )
	{
		if (m_areas[i].GetStatus() == Area::LOADING )
		{
			if (Area_LoadFinished(i))
				Area_MakeVisible(i);
		}
	}
}


//--------------------------------------------------
//!
//!	AreaManager::ActivateLevelFromCheckpoint
//! called after ents are serialised, we need to turn
//! on our entities now, but sees if there is a valid
//! checkpoint being used and get the first area to
//! activate from that
//!
//--------------------------------------------------
void AreaManager::ActivateLevelFromCheckpoint( int nCheckpointID )
{
	ntAssert( nCheckpointID >= 0 );

	ShellLevel* pLoadingLevel = ShellMain::Get().GetCurrLoadingLevel();
	ntAssert( pLoadingLevel );

	Object_Checkpoint* pCheckpoint = pLoadingLevel->GetCheckpointByID( nCheckpointID );

	if ( pCheckpoint )
	{
		// Get first sectorbit
		for( int32_t i = 1; i <= NUM_AREAS; i++ )
		{
			if (pCheckpoint->m_areaInfo.Within( i ) == true )
			{
				ActivateLevel( i );
				return;
			}
		}
	}

	// Otherwise, activate level normally
	ActivateLevel();
}


//--------------------------------------------------
//!
//!	AreaManager::ActivateLevel
//! called after ents are serialised, we need to turn
//! on our entities now.
//!
//--------------------------------------------------
void AreaManager::ActivateLevel( int32_t iAreaToActivate )
{
	ntError_p( LoadingLevel(), ("Must be loading a level for this to be valid") );
	m_eState = AS_LEVEL_ACTIVE;

	// finish construction of all entities registered with us
	for ( EntitySet::iterator it = m_allEnts.begin(); it != m_allEnts.end(); ++it )
		(*it)->OnLevelStart();

	// make sure all ents registered with us are switched off and invisible.
	// this will change when we create ents in this state
	for ( EntitySet::iterator it = m_allEnts.begin(); it != m_allEnts.end(); ++it )
	{
		Entity_SetActivity(*it,false);
		Entity_SetVisibility(*it,false);
	}

	// obtain startup area
	if (iAreaToActivate == ACTIVATE_FIRST)
	{
		// NOTE: how we get the first area to activate should be mapped, 
		// but ACTIVATE_FIRST mean use the lowest numbered area available

		for( int32_t i = 1; i <= NUM_AREAS; i++ )
		{
			if (m_areas[i].Empty() == false)
			{
				iAreaToActivate = i;
				break;
			}
		}

		// didnt find a valid one, must be an empty level, set to 1st
		iAreaToActivate = 1;
	}
	ntError_p( IsValidAreaNumber(iAreaToActivate), ("Invalid area number to use as first active area") );

	// load in the first sector, make it visible and active
	Area_LoadSync( iAreaToActivate );
	Area_LoadThenActivate( iAreaToActivate );

	// print out how much we're using
	AreaResourceDB::Get().DumpAreaSysRam();

	m_iCurrActiveArea = iAreaToActivate;

	// now we have an active area, can add all the delayed ones that didnt have a 
	// valid area before. (These must all be unparented dynamic ents).
	while ( !m_delayedInsert.empty() )
	{
		AddEntity( m_delayedInsert.back() );
		m_delayedInsert.pop_back();
	}
}

//--------------------------------------------------
//!
//!	AreaManager::ForceReactivateLevel
//! called when a level is running, forces the
//! currently active sector, unloads everything else
//! and assumes youve dumped the principle entity
//! in a valid place for the system to carry on working.
//!
//--------------------------------------------------
void AreaManager::ForceReactivateLevel( int iAreaToActivate )
{
	ntError_p( LevelActive(), ("Must have an active level for this to be valid") );
	ntError_p( IsValidAreaNumber(iAreaToActivate), ("Invalid area number to use as first active area") );

	// unload old areas if not required
	if (m_iCurrActiveArea != iAreaToActivate)
	{
		Area_MakeInactive(m_iCurrActiveArea);
		Area_AbortLoadOrUnload(m_iCurrActiveArea);
	}

	if (m_iLastActiveArea != INVALID_AREA)
		Area_AbortLoadOrUnload(m_iLastActiveArea);

	if (m_iNextActiveArea != INVALID_AREA)
		Area_AbortLoadOrUnload(m_iNextActiveArea);

	// load the new area if its not loaded aleady
	if (m_areas[iAreaToActivate].GetStatus() == Area::UNLOADED )
		Area_LoadAsync( iAreaToActivate );
	
	if (m_areas[iAreaToActivate].GetStatus() != Area::VISIBLE_AND_ACTIVE )
	{
		Area_LoadThenActivate( iAreaToActivate );
	}
	else
	{
		// flush any async unload we may have
		AreaResourceDB::Get().Update();
	}

	m_iCurrActiveArea = iAreaToActivate;
	m_iLastActiveArea = INVALID_AREA;
	m_iNextActiveArea = INVALID_AREA;

#ifndef _RELEASE
	// finally validate everything is as we think it should be
	for( int32_t i = 1; i <= NUM_AREAS; i++ )
	{
		if (i == m_iCurrActiveArea)
		{
			ntError_p( m_areas[i].GetStatus() == Area::VISIBLE_AND_ACTIVE, ("Level restart failed") );
		}
		else
		{
			ntError_p( m_areas[i].GetStatus() == Area::UNLOADED, ("Level restart failed") );
		}
	}
#endif
}

//--------------------------------------------------
//!
//!	AreaManager::AddEntity
//! Dump this entity into all the areas his AreaInfo 
//! indicates he's within. We can auto assign areas
//! to ents that do not have a valid one mapped, via
//! the container they're within, or what the current
//! active areas are.
//!
//--------------------------------------------------
void AreaManager::AddEntity( CEntity* pEntity )
{
	ntAssert_p( !Inactive(), ("Area system not active, invalid to call AddEntity()") );

	// do we need to auto assign a sector ID ? 
	if (pEntity->m_areaInfo.RequireAreaAutoAssignment())
	{
		if (LoadingLevel())
		{
			// only do this on level load
			int32_t iAreaWithin = ResolveAreaContainer(pEntity);

			// INVALID_AREA indicates an ent that is not within a container labeled
			// Area N, where N is the area number. It could be an old level, or a dynamic ent
			if (iAreaWithin != INVALID_AREA)
			{
				// we have a parent container of the type Area 1 etc, stick it in that.
				pEntity->m_areaInfo.AddArea(iAreaWithin);
			}
			else
			{
				// delay decision untill after area initionalisation, as then we know
				// what initial areas we're going to activate.
				m_delayedInsert.push_front( pEntity );
				return;
			}
		}
		else
		{
			// just use whatever is active at the moment
			pEntity->m_areaInfo = m_activeAreas;
		}
	}

	// now insert into our areas
#ifdef _DEBUG
	EntitySet::iterator it = m_allEnts.find( pEntity );
	ntError_p( it == m_allEnts.end(), ("Adding entity that is already within sector system") );
#endif

	m_allEnts.insert( pEntity );

	// add entity to relevent sectors
	for( int32_t i = 1; i <= NUM_AREAS; i++ )
	{
		if (pEntity->m_areaInfo.Within(i))
			m_areas[i].Add(pEntity);
	}


	// Set our status if this is after level load (if its before,
	// these guys will get turned on in the initialiser)
	if ( LevelActive() )
	{
		// I'm moving this into CEntity::PostPostConstruct as it was causing problems
		// when it was being called in the middle of PostConstruct and prevented all
		// Army Sections from working...
		//-------------------------------------------------------------
		//pEntity->OnLevelStart();

		if ( m_visibleAreas.Intersects(pEntity->m_areaInfo) )
		{
			Entity_SetActivity( pEntity, true );
			Entity_SetVisibility( pEntity, true );
		}
		else
		{
			Entity_SetActivity( pEntity, false );
			Entity_SetVisibility( pEntity, false );
		}		
	}
}

//--------------------------------------------------
//!
//!	AreaManager::RemoveEntity
//! Pull this entity out of all the areas he's within
//! NB this should be called very rarely, only if an
//! ent is dynamically destroyed, as Reset is called
//! before entity destruction on level shutdown
//!
//--------------------------------------------------
void AreaManager::RemoveEntity( CEntity* pEntity )
{
	// ignore if we're switched off
	if (Inactive())
		return;

	// possible we are in the delay list and not really in the
	// sector system at all yet...
	if (LoadingLevel() && pEntity->m_areaInfo.RequireAreaAutoAssignment())
	{
		EntityList::iterator it = ntstd::find(m_delayedInsert.begin(), m_delayedInsert.end(), pEntity);
		ntError_p( it != m_delayedInsert.end(), ("Entity should be in delay list") );
		m_delayedInsert.erase( it );
		return;
	}

	// nope, we really should be removed
	for (int32_t i = 1; i <= NUM_AREAS; i++)
	{
		if (pEntity->m_areaInfo.Within(i))
			m_areas[i].Remove( pEntity );
	}

#ifndef _RELEASE
	EntitySet::iterator it = m_allEnts.find( pEntity );
	ntError_p( it != m_allEnts.end(), ("Removing entity that is not within sector system") );
#endif

	m_allEnts.erase( pEntity );
}

//--------------------------------------------------
//!
//!	AreaManager::AddLoadTrigger
//! Insert a area load trigger into the right area
//!
//--------------------------------------------------
void AreaManager::AddLoadTrigger( SectorLoadTrigger* pLoadTrigger )
{
	ntError_p( LoadingLevel(), ("Only valid to add load triggers within level load") );

	int32_t iAreaWithin = ResolveAreaContainer(pLoadTrigger);

	if (iAreaWithin == INVALID_AREA)
	{
		#ifndef _RELEASE
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pLoadTrigger );
		const char* pName = ntStr::GetString(pDO->GetName());
		UNUSED(pName);

		user_warn_p( 0, ("SectorLoadTrigger (%s) mapped that does not have a valid parent container.", pName) );
		#endif
		return;
	}
	
	if	(
		(!IsValidAreaNumber(pLoadTrigger->m_iAreaToLoad)) ||
		(pLoadTrigger->m_iAreaToLoad == iAreaWithin)
		)
	{
		#ifndef _RELEASE
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pLoadTrigger );
		const char* pName = ntStr::GetString(pDO->GetName());
		UNUSED(pName);

		user_warn_p( 0, ("SectorLoadTrigger (%s) mapped with and invalid SectorToLoad parameter (%d).", pName, pLoadTrigger->m_iAreaToLoad ) );
		#endif
		return;
	}

	m_areas[iAreaWithin].Add( pLoadTrigger );
}

//--------------------------------------------------
//!
//!	AreaManager::AddTransitionPortal
//! Insert a area transition portal into the right area
//!
//--------------------------------------------------
void AreaManager::AddTransitionPortal( SectorTransitionPortal* pPortal )
{
	ntError_p( LoadingLevel(), ("Only valid to add transition portals within level load") );

	int32_t iAreaWithin = ResolveAreaContainer(pPortal);

	if (iAreaWithin == INVALID_AREA)
	{
		#ifndef _RELEASE
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pPortal );
		const char* pName = ntStr::GetString(pDO->GetName());
		UNUSED(pName);

		user_warn_p( 0, ("SectorTransitionPortal (%s) mapped that does not appear to have a valid parent container.", pName) );
		#endif
		return;
	}

	if	(
		(!IsValidAreaNumber(pPortal->m_iAreaToActivate)) ||
		(pPortal->m_iAreaToActivate == iAreaWithin)
		)
	{
		#ifndef _RELEASE
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pPortal );
		const char* pName = ntStr::GetString(pDO->GetName());
		UNUSED(pName);

		user_warn_p( 0, ("SectorTransitionPortal (%s) mapped with and invalid SectorToActivate parameter (%d).", pName, pPortal->m_iAreaToActivate ) );
		#endif
		return;
	}

	m_areas[iAreaWithin].Add( pPortal );
}

//--------------------------------------------------
//!
//!	AreaManager::SetToActiveAreas
//! set entity to the currently active areas
//!
//--------------------------------------------------
void AreaManager::SetToEntityAreas( const CEntity* pSrc, CEntity* pDest )
{
	ntAssert( pSrc && pDest );
	ntAssert_p( !Inactive(), ("Area system not active, invalid to call SetToEntityAreas()") );

	// bail out early if we're in-game and parent is same as child
	if (LevelActive() && (pSrc->m_areaInfo.Equals(pDest->m_areaInfo)))
		return;

	// remove, set then add ensures we have correct entity state set
	RemoveEntity( pDest );
	pDest->m_areaInfo = pSrc->m_areaInfo;
	AddEntity( pDest );
}


//--------------------------------------------------
//!
//!	AreaManager::SetToActiveAreas
//! Sets an entity to the currently active areas
//!
//--------------------------------------------------
void AreaManager::SetToActiveAreas( CEntity* pEntity )
{
	ntError_p( LevelActive(), ("Only valid to call SetToActiveAreas() when level is running") );

	// remove, set then add ensures we're added to the correct sectors
	RemoveEntity( pEntity );
	pEntity->m_areaInfo = m_activeAreas;
	AddEntity( pEntity );
}

//--------------------------------------------------
//!
//!	AreaManager::SetToAlwaysActive
//! Sets an entity be within all areas
//!
//--------------------------------------------------
void AreaManager::SetToAlwaysActive( CEntity* pEntity )
{
	ntError_p( LevelActive(), ("Only valid to call SetToActiveAreas() when level is running") );

	// remove, set then add ensures we're added to the correct sectors
	RemoveEntity( pEntity );
	pEntity->m_areaInfo = AreaInfo( 0xffffffff );
	AddEntity( pEntity );
}

//--------------------------------------------------
//!
//!	AreaManager::Entity_SetVisibility
//! This adds or removes the entity's renderables to the 
//! level's renderable list.
//! NOTE: It is valid to set an ent to visible even
//! if it is already visible, and vice versa
//!
//--------------------------------------------------
void AreaManager::Entity_SetVisibility( CEntity* pEntity, bool bVisible )
{
	if (bVisible)
	{
		pEntity->FixUpAreaResources();
		if (pEntity->GetPhysicsSystem())
			pEntity->GetPhysicsSystem()->PausePresenceInHavokWorld(false);
	}
	else
	{
		ntAssert(pEntity->IsPaused());
		pEntity->ReleaseAreaResources();
		if (pEntity->GetPhysicsSystem())
			pEntity->GetPhysicsSystem()->PausePresenceInHavokWorld(true);
	}

	CRenderableComponent* pRenderable = pEntity->GetRenderableComponent();
	
	if (pRenderable)
		pRenderable->AddRemoveAll_AreaSystem( bVisible );

	if (pEntity->m_replacesEnt)
	{
		CEntity* pReplaces = CEntityManager::Get().FindStatic( pEntity->m_entToReplace );
		
		if (pReplaces == 0)
			pReplaces = CEntityManager::Get().FindEntity( pEntity->m_entToReplace );

		if (pReplaces == 0)
		{
			user_error_p( 0, ("Entity %s is marked as replacing none existant entity %s. Tell a designer!\n.",
							ntStr::GetString(pEntity->m_Name),
							ntStr::GetString(pEntity->m_entToReplace) ) );
		}

		if (pReplaces)
		{
			ntError_p( pReplaces != pEntity, ("Cannot replace ourselves.") );

			CRenderableComponent* pRepRenderable = pReplaces->GetRenderableComponent();
			
			if (pRepRenderable)
				pRepRenderable->AddRemoveAll_Game( !bVisible );
		}
	}
}

//--------------------------------------------------
//!
//!	AreaManager::ResolveAreaContainer
//! called to find the XML container for this object
//! NOTE: this adjusts m_resolvedContainers. 
//!
//--------------------------------------------------
int32_t AreaManager::ResolveAreaContainer( const void* ptr )
{
	// find parent container of this pointer
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( ptr );
	ntError_p( pDO, ("Argument for ResolveAreaContainer() MUST have a corresponding DataObject") );

	ObjectContainer* pContainer = pDO->GetParent();
	ntError_p( pContainer, ("Argument for ResolveAreaContainer() MUST have a corresponding ObjectContainer") );

	// see if exists already in our sector ID container map
	if ( m_resolvedContainers.find( pContainer ) == m_resolvedContainers.end() )
	{
		// havent had a pointer from this container yet, see if this 
		// container is marked as belonging to an area

		// NB this is rather weak, as it relies on a naming convention for
		// layers in MrED.
		DataObject* pContainerDO = ObjectDatabase::Get().GetDataObjectFromPointer( pContainer );
		ntError_p( pContainerDO, ("All containers MUST have a corresponding DataObject") );

		const char* pName = ntStr::GetString(pContainerDO->GetName());
		if (strstr(pName,"Area"))
		{
			int32_t iNumber;
			int iValid = 0;
			iValid = sscanf( pName, "Area %d", &iNumber );

			ntError_p( iValid, ("Area number extraction failed for container: %s.", pName ));
			ntError_p( IsValidAreaNumber(iNumber), ("Area number extracted is out of valid range (1->%d).", NUM_AREAS) ); 

			m_resolvedContainers[pContainer] = iNumber;
		}
		else
		{
			// sector is not named appropriately, mark it as such
			m_resolvedContainers[pContainer] = INVALID_AREA;
		}
	}

	return 	m_resolvedContainers[pContainer];
}

//--------------------------------------------------
//!
//!	AreaManager::DebugUpdate
//! Render some helpful info
//!
//--------------------------------------------------
#ifndef _RELEASE
int32_t	GetGDDR( uint32_t mustMatch, uint32_t areasToTest )
{
	int32_t result = 0;
	result += AreaResourceDB::Get().GetSizeFor(mustMatch,areasToTest,AreaResource::CLUMP);
	result += AreaResourceDB::Get().GetSizeFor(mustMatch,areasToTest,AreaResource::TEXTURE);
	return result;
}

int32_t	GetXDDR( uint32_t mustMatch, uint32_t areasToTest )
{
	int32_t result = 0;
	result += AreaResourceDB::Get().GetSizeFor(mustMatch,areasToTest,AreaResource::ANIM);
	return result;
}

void Box( bool shrink, float fX1, float fX2, float fY1, float fY2, uint32_t iColour )
{
	if (shrink)
	{
		fX1+=1.0f;
		fX2-=1.0f;
		fY1+=1.5f;
		fY2-=1.5f;
	}

	g_VisualDebug->RenderLine( CPoint(fX1,fY1,0), CPoint(fX2,fY1,0), iColour, DPF_DISPLAYSPACE);
	g_VisualDebug->RenderLine( CPoint(fX1,fY2,0), CPoint(fX2,fY2,0), iColour, DPF_DISPLAYSPACE);
	g_VisualDebug->RenderLine( CPoint(fX1,fY1,0), CPoint(fX1,fY2,0), iColour, DPF_DISPLAYSPACE);
	g_VisualDebug->RenderLine( CPoint(fX2,fY1,0), CPoint(fX2,fY2,0), iColour, DPF_DISPLAYSPACE);
}

void BoxWithMessage( bool shrink, float fX1, float fX2, float fY1, float fY2, uint32_t iColour, const char* pcTxt, ... )
{
	char pcBuffer[ MAX_PATH ];

	// Format the text.
	va_list	stArgList;
	va_start(stArgList, pcTxt);
	vsnprintf(pcBuffer, MAX_PATH-1, pcTxt, stArgList);
	va_end(stArgList);

	Box( shrink, fX1, fX2, fY1, fY2, iColour );
	g_VisualDebug->Printf2D( fX1+1.0f, fY1-12.0f, iColour, 0, pcBuffer );
}

#endif


void AreaManager::DebugUpdate()
{
#ifndef _RELEASE

	// Sanity checking: enforce 2 visible_inactive and 1
	// visible_active sector rules

	ntError( LevelActive() );

	int iLoadedCount = 0;
	int iVisibleCount = 0;
	int iActiveCount = 0;

	for (int32_t i = 1; i <= NUM_AREAS; i++)
	{
		if ( m_areas[i].GetStatus() != Area::UNLOADED )
			iLoadedCount++;

		if ( m_visibleAreas.Includes(i) )
			iVisibleCount++;

		if ( m_activeAreas.Includes(i) )
			iActiveCount++;
	}

	ntError_p( iLoadedCount <= 3, ("Number of loaded sectors has exceeded 3") );
	ntError_p( iVisibleCount <= 3, ("Number of visible sectors has exceeded 3") );
	ntError_p( iActiveCount == 1, ("Number of active sectors greater than 1") );

	if(CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_ENTER, KEYM_SHIFT))
		m_bDebugRender = !m_bDebugRender;

	if (m_bDebugRender)
	{
		float fLeftAlign = 30.0f;
		float fFromTop = 290.0f;

		// debug render any load volumes or portals for this area
		for (	TriggerList::const_iterator it = m_areas[m_iCurrActiveArea].GetTriggers().begin();
				it != m_areas[m_iCurrActiveArea].GetTriggers().end(); ++it )
		{
			(*it)->DebugRender();
		}

		for (	PortalList::const_iterator it = m_areas[m_iCurrActiveArea].GetPortals().begin();
				it != m_areas[m_iCurrActiveArea].GetPortals().end(); ++it )
		{
			(*it)->DebugRender();
		}

		// render status of area system
		char acAreaList[512];
		sprintf( acAreaList, ("visible areas: ") );

		for ( int32_t i = 1; i <= NUM_AREAS; i++ )
		{
			if ( m_visibleAreas.Includes(i) )
			{
				char acTemp[32];
				sprintf( acTemp, "%d ", i );
				strcat( acAreaList, acTemp );
			}
		}

		g_VisualDebug->Printf2D( fLeftAlign, fFromTop, DC_WHITE, 0, "AREA SYSTEM: %s", acAreaList );
		fFromTop += 12.0f;

		sprintf( acAreaList, ("active areas: ") );

		for ( int32_t i = 1; i <= NUM_AREAS; i++ )
		{
			if ( m_activeAreas.Includes(i) )
			{
				char acTemp[32];
				sprintf( acTemp, "%d ", i );
				strcat( acAreaList, acTemp );
			}
		}

		g_VisualDebug->Printf2D( fLeftAlign, fFromTop, DC_WHITE, 0, "AREA SYSTEM: %s", acAreaList );
		fFromTop += 12.0f;

		bool bHaveLoading = false;
		sprintf( acAreaList, ("loading areas: ") );

		for ( int32_t i = 1; i <= NUM_AREAS; i++ )
		{
			if ( m_areas[i].GetStatus() == Area::LOADING )
			{
				bHaveLoading = true;

				float fPercent = AreaResourceDB::Get().Area_LoadFraction(i);

				char acTemp[32];
				sprintf( acTemp, "%d:%.0f%%  ", i, fPercent * 100.0f );
				strcat( acAreaList, acTemp );
			}
		}
		
		if (bHaveLoading)
		{
			g_VisualDebug->Printf2D( fLeftAlign, fFromTop, DC_WHITE, 0, "AREA SYSTEM: %s", acAreaList );
		}

		fFromTop += 12.0f;

		// TBD_ANIM
		// generalise this so i can add GDDR and XDDR views.

		// okay, horrible debug display code, but its late and im tired.
		// render curr, next then last resources graphically
		//----------------------------------------------------------------
		static const float gddrMax = ((32.f*3)+(50.f)) * (1024.f*1024.f); // 3 sectors + character
		static const float displayWidth = 1000.0f;
		static const float blockHeight = 50.0f;

		float fX1 = fLeftAlign;
		float fX2 = fX1 + displayWidth;
		float fY1 = fFromTop + 12.0f;

		// border
		Box( false, fX1, fX2, fY1, fY1+blockHeight*4, DC_BLACK );

		// total
		int32_t gddrTotal = AreaResourceDB::Get().m_totalTexVRAM + AreaResourceDB::Get().m_totalClumpVRAM;
		
		fX2 = ((_R(gddrTotal) * displayWidth) / gddrMax) + fX1;
		BoxWithMessage( false, fX1, fX2, fY1, fY1+blockHeight*4, DC_RED, "TOTAL: %.1f Mb", _R(gddrTotal)/(1024.0f*1024.0f) );

		// get globals first
		//----------------------------------------------------------------
		int32_t gddrGlobals = GetGDDR( 0xffffffff, 0xffffffff );
		gddrTotal -= gddrGlobals;

		fX2 = ((_R(gddrGlobals) * displayWidth) / gddrMax) + fX1;
		fY1 += blockHeight;

		BoxWithMessage( true, fX1, fX2, fY1, fY1+blockHeight*3, DC_RED, "GLOBAL: %.1f Mb", _R(gddrGlobals)/(1024.0f*1024.0f) );

		if ((m_iLastActiveArea != INVALID_AREA) && (m_iNextActiveArea != INVALID_AREA))
		{
			// resources that reside in at least curr, last and next, but not global
			//----------------------------------------------------------------
			uint32_t allVisible = (1<<(m_iCurrActiveArea-1)) | (1<<(m_iLastActiveArea-1)) | (1<<(m_iNextActiveArea-1));
			
			int32_t gddrAdditional = GetGDDR( allVisible, allVisible );
			ntAssert( gddrAdditional >= gddrGlobals ); // we must have at least the globals here.

			// get the true additional
			gddrAdditional -= gddrGlobals;
			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1, fY1+blockHeight*3, DC_WHITE, "%d %d %d: %.1f Mb", m_iCurrActiveArea, m_iNextActiveArea, m_iLastActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}

			// resources that reside in curr and next only
			//----------------------------------------------------------------
			int32_t mustMatch = (1<<(m_iCurrActiveArea-1)) | (1<<(m_iNextActiveArea-1));
			gddrAdditional = GetGDDR( mustMatch, allVisible );

			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1, fY1+blockHeight*2, DC_WHITE, "%d %d: %.1f Mb", m_iCurrActiveArea, m_iNextActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}

			// resources that reside in curr and last only
			//----------------------------------------------------------------
			mustMatch = (1<<(m_iCurrActiveArea-1)) | (1<<(m_iLastActiveArea-1));
			gddrAdditional = GetGDDR( mustMatch, allVisible );

			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1, fY1+blockHeight, DC_WHITE, "%d %d: %.1f Mb", m_iCurrActiveArea, m_iLastActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				Box( true, fX1, fX2, fY1+blockHeight*2, fY1+blockHeight*3, DC_WHITE );
				gddrTotal -= gddrAdditional;
			}

			// resources that reside in next and last only
			//----------------------------------------------------------------
			mustMatch = (1<<(m_iNextActiveArea-1)) | (1<<(m_iLastActiveArea-1));
			gddrAdditional = GetGDDR( mustMatch, allVisible );

			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1+blockHeight*1, fY1+blockHeight*3, DC_WHITE, "%d %d: %.1f Mb", m_iNextActiveArea, m_iLastActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}

			// resources that reside in curr
			//----------------------------------------------------------------
			gddrAdditional = GetGDDR( (1<<(m_iCurrActiveArea-1)), allVisible );
			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1, fY1+blockHeight*1, DC_WHITE, "%d: %.1f Mb", m_iCurrActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}

			// resources that reside in next
			//----------------------------------------------------------------
			gddrAdditional = GetGDDR( (1<<(m_iNextActiveArea-1)), allVisible );
			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1+blockHeight*1, fY1+blockHeight*2, DC_WHITE, "%d: %.1f Mb", m_iNextActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}

			// resources that reside in last
			//----------------------------------------------------------------
			gddrAdditional = GetGDDR( (1<<(m_iLastActiveArea-1)), allVisible );
			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1+blockHeight*2, fY1+blockHeight*3, DC_WHITE, "%d: %.1f Mb", m_iLastActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}
		}
		else if ((m_iLastActiveArea != INVALID_AREA) || (m_iNextActiveArea != INVALID_AREA))
		{
			int32_t second = (m_iLastActiveArea != INVALID_AREA) ? m_iLastActiveArea : m_iNextActiveArea;

			// resources that reside in at least curr, and the second, but not global
			//----------------------------------------------------------------
			uint32_t allVisible = (1<<(m_iCurrActiveArea-1)) | (1<<(second-1));
			
			int32_t gddrAdditional = GetGDDR( allVisible, allVisible );
			ntAssert( gddrAdditional >= gddrGlobals ); // we must have at least the globals here.

			// get the true additional
			gddrAdditional -= gddrGlobals;
			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1, fY1+blockHeight*2, DC_WHITE, "%d %d: %.1f Mb", m_iCurrActiveArea, second, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}

			// resources that reside in curr
			//----------------------------------------------------------------
			gddrAdditional = GetGDDR( (1<<(m_iCurrActiveArea-1)), allVisible );
			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1, fY1+blockHeight*1, DC_WHITE, "%d: %.1f Mb", m_iCurrActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}

			// resources that reside in second
			//----------------------------------------------------------------
			gddrAdditional = GetGDDR( (1<<(second-1)), allVisible );
			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1+blockHeight*1, fY1+blockHeight*2, DC_WHITE, "%d: %.1f Mb", second, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}
		}
		else
		{
			// resources that reside in just curr, but not global
			//----------------------------------------------------------------
			uint32_t allVisible = (1<<(m_iCurrActiveArea-1));
			
			int32_t gddrAdditional = GetGDDR( allVisible, allVisible );
			ntAssert( gddrAdditional >= gddrGlobals ); // we must have at least the globals here.

			// get the true additional
			gddrAdditional -= gddrGlobals;
			if (gddrAdditional > 0)
			{
				fX1 = fX2;
				fX2 = ((_R(gddrAdditional) * displayWidth) / gddrMax) + fX1;

				BoxWithMessage( true, fX1, fX2, fY1, fY1+blockHeight*1, DC_WHITE, "%d: %.1f Mb", m_iCurrActiveArea, _R(gddrAdditional)/(1024.0f*1024.0f) );
				gddrTotal -= gddrAdditional;
			}
		}
		ntAssert( gddrTotal == 0 ); // we have not accounted for all resources somehow
	}

#endif
}
