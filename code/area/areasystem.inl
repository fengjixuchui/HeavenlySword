//--------------------------------------------------
//!
//!	\file areasystem.inl
//! inline methods for area system
//!
//--------------------------------------------------

#ifndef AREA_SYSTEM_INL
#define AREA_SYSTEM_INL

// nessecary for inlines
#ifndef	_ENTITY_H
#include "game/entity.h"
#include "game/entity.inl"
#endif

#ifndef _RENDERABLECOMPONENT_H
#include "game/renderablecomponent.h"
#endif

#ifndef AREA_RESOURCE_H
#include "area/arearesourcedb.h"
#endif

#ifndef GFX_RENDERER_H
#include "gfx/renderer.h"
#endif

//--------------------------------------------------
//!
//!	Area::Add
//! Add an entity to this area
//!
//--------------------------------------------------
inline void Area::Add( CEntity* pObj )
{
	ntError_p( m_entities.find( pObj ) == m_entities.end(), ("Adding entity that is already within this area") );
	m_entities.insert(pObj);
}

//--------------------------------------------------
//!
//!	Area::Remove
//! Add an entity to this area
//!
//--------------------------------------------------
inline void Area::Remove( CEntity* pObj )
{
	ntError_p( m_entities.find( pObj ) != m_entities.end(), ("Removing entity that is not within this area") );
	m_entities.erase(pObj);
}





//--------------------------------------------------
//!
//!	AreaManager::IsAreaLoaded
//! test the status of area
//!
//--------------------------------------------------
inline bool AreaManager::IsAreaLoaded( int32_t iArea )
{
	ntAssert_p( AreaSystem::IsValidAreaNumber(iArea), ("AreaManager::IsAreaLoaded called with an invalid area ID (%d)", iArea) );
	return (m_areas[iArea].GetStatus() >= Area::LOADED) ? true : false;
}

//--------------------------------------------------
//!
//!	AreaManager::IsAreaActive
//! test the status of area
//!
//--------------------------------------------------
inline bool AreaManager::IsAreaActive( int32_t iArea )
{
	ntAssert_p( AreaSystem::IsValidAreaNumber(iArea), ("AreaManager::IsAreaActive called with an invalid area ID (%d)", iArea) );
	return (m_areas[iArea].GetStatus() == Area::VISIBLE_AND_ACTIVE) ? true : false;
}

//--------------------------------------------------
//!
//!	AreaManager::SignalLoadingLevel
//! Signal we have started to load the level
//!
//--------------------------------------------------
inline void AreaManager::SignalLoadingLevel()
{
	ntAssert_p( Inactive(), ("Must be inactive for this to be valid") );
	m_eState = AS_LOADING_LEVEL;		
}

//--------------------------------------------------
//!
//!	AreaManager::Entity_SetActivity
//! This adds or removes the entity's components to the 
//! level's update list.
//! NOTE: It is valid to pause an ent even if it is
//! already paused, and vice versa
//!
//--------------------------------------------------
inline void AreaManager::Entity_SetActivity( CEntity* pEntity, bool bActive )
{
	pEntity->Pause( !bActive );
}

//--------------------------------------------------
//!
//!	AreaManager::Area_MakeVisible
//! Make any ents in this area visible that arnt already
//!
//--------------------------------------------------
inline void AreaManager::Area_MakeVisible( int32_t iAreaNumber )
{
	ntError_p( m_areas[iAreaNumber].GetStatus() == Area::LOADED, ("Area must be loaded before it can be activated") );
	ntError_p( m_visibleAreas.Includes(iAreaNumber) == false, ("Area is already visible, something has gone horribly wrong") );

	for (	AreaSystem::EntitySet::const_iterator it = m_areas[iAreaNumber].GetEnts().begin();
			it != m_areas[iAreaNumber].GetEnts().end(); ++it )
	{
		if ((*it)->m_areaInfo.Intersects( m_visibleAreas ) == false)
			Entity_SetVisibility( *it, true );
	}

	m_visibleAreas.AddArea(iAreaNumber);
	m_areas[iAreaNumber].SetStatus( Area::VISIBLE_NOT_ACTIVE );
}

//--------------------------------------------------
//!
//!	AreaManager::Area_MakeInvisible
//! Make any ents in this area invisible that arnt in
//! any other visible areas
//!
//--------------------------------------------------
inline void AreaManager::Area_MakeInvisible( int32_t iAreaNumber )
{
	ntError_p( m_areas[iAreaNumber].GetStatus() == Area::VISIBLE_NOT_ACTIVE, ("Area must be visble but inactive here") );
	ntError_p( m_visibleAreas.Includes(iAreaNumber) == true, ("Area is not visible, something has gone horribly wrong") );

	m_visibleAreas.RemoveArea(iAreaNumber);

	for (	AreaSystem::EntitySet::const_iterator it = m_areas[iAreaNumber].GetEnts().begin();
			it != m_areas[iAreaNumber].GetEnts().end(); ++it )
	{
		if ((*it)->m_areaInfo.Intersects( m_visibleAreas ) == false)
			Entity_SetVisibility( *it, false );
	}

	m_areas[iAreaNumber].SetStatus( Area::LOADED );
}

//--------------------------------------------------
//!
//!	AreaManager::Area_MakeActive
//! Make any ents in this area active that arnt already
//!
//--------------------------------------------------
inline void AreaManager::Area_MakeActive( int32_t iAreaNumber )
{
	ntError_p( m_areas[iAreaNumber].GetStatus() == Area::VISIBLE_NOT_ACTIVE, ("Area must be visble but inactive here") );
	ntError_p( m_activeAreas.Includes(iAreaNumber) == false, ("Area is already active, something has gone horribly wrong") );

	for (	AreaSystem::EntitySet::const_iterator it = m_areas[iAreaNumber].GetEnts().begin();
			it != m_areas[iAreaNumber].GetEnts().end(); ++it )
	{
		if ((*it)->m_areaInfo.Intersects( m_activeAreas ) == false)
			Entity_SetActivity( *it, true );
	}

	m_activeAreas.AddArea(iAreaNumber);
	m_areas[iAreaNumber].SetStatus( Area::VISIBLE_AND_ACTIVE );
}

//--------------------------------------------------
//!
//!	AreaManager::Area_MakeInactive
//! Make any ents in this area inactive that arnt in
//! any other active areas
//!
//--------------------------------------------------
inline void AreaManager::Area_MakeInactive( int32_t iAreaNumber )
{
	ntError_p( m_areas[iAreaNumber].GetStatus() == Area::VISIBLE_AND_ACTIVE, ("Area must be visble and active here") );
	ntError_p( m_activeAreas.Includes(iAreaNumber) == true, ("Area is already active, something has gone horribly wrong") );

	m_activeAreas.RemoveArea(iAreaNumber);

	for (	AreaSystem::EntitySet::const_iterator it = m_areas[iAreaNumber].GetEnts().begin();
			it != m_areas[iAreaNumber].GetEnts().end(); ++it )
	{
		if ((*it)->m_areaInfo.Intersects( m_activeAreas ) == false)
			Entity_SetActivity( *it, false );
	}

	m_areas[iAreaNumber].SetStatus( Area::VISIBLE_NOT_ACTIVE );
}

//--------------------------------------------------
//!
//!	AreaManager::Area_LoadSync
//! Request an area load
//!
//--------------------------------------------------
inline void AreaManager::Area_LoadSync( int32_t iAreaNumber )
{
	ntError_p( m_areas[iAreaNumber].GetStatus() == Area::UNLOADED, ("Area must be unloaded here") );

	m_areas[iAreaNumber].SetStatus( Area::LOADED );
	AreaResourceDB::Get().Area_LoadSync( iAreaNumber );
}

//--------------------------------------------------
//!
//!	AreaManager::Area_LoadAsync
//! Request an area load
//!
//--------------------------------------------------
inline void AreaManager::Area_LoadAsync( int32_t iAreaNumber )
{
	ntError_p( m_areas[iAreaNumber].GetStatus() == Area::UNLOADED, ("Area must be unloaded here") );

	m_areas[iAreaNumber].SetStatus( Area::LOADING );
	AreaResourceDB::Get().Area_LoadAsync( iAreaNumber );
}

//--------------------------------------------------
//!
//!	AreaManager::Area_Unload
//! Unload an area. Does not require update of
//! resource system.
//!
//--------------------------------------------------
inline void AreaManager::Area_UnloadSync( int32_t iAreaNumber )
{
	ntError_p( m_areas[iAreaNumber].GetStatus() == Area::LOADED, ("Area must be loaded here") );

	m_areas[iAreaNumber].SetStatus( Area::UNLOADED );
	AreaResourceDB::Get().Area_UnloadSync( iAreaNumber );	
}

//--------------------------------------------------
//!
//!	AreaManager::Area_UnloadAsync
//! Unload an area. Does requires update of
//! resource system to collapse requests
//!
//--------------------------------------------------
inline void AreaManager::Area_UnloadAsync( int32_t iAreaNumber )
{
	ntError_p(	(m_areas[iAreaNumber].GetStatus() == Area::LOADING) ||
				(m_areas[iAreaNumber].GetStatus() == Area::LOADED), ("Area must be loaded or loading here") );

	m_areas[iAreaNumber].SetStatus( Area::UNLOADED );
	AreaResourceDB::Get().Area_UnloadAsync( iAreaNumber );	
}

//--------------------------------------------------
//!
//!	AreaManager::Area_AbortLoadOrUnload
//! Cancel load if in progress, unload if loaded
//! Invalid for active or unloaded areas
//!
//--------------------------------------------------
inline void AreaManager::Area_AbortLoadOrUnload( int32_t iAreaNumber, bool bAsync )
{
	switch ( m_areas[iAreaNumber].GetStatus() )
	{
	case Area::UNLOADED:			// shouldnt happen
		ntError_p( 0, ("Area_AbortLoadOrUnload should not be called on an unloaded area") );
		break;

	case Area::LOADING:				// simple, cancel the load
		Area_UnloadAsync( iAreaNumber );
		break;

	case Area::LOADED:				// unload the resources
		if (bAsync)
			Area_UnloadAsync( iAreaNumber );
		else
			Area_UnloadSync( iAreaNumber );
		break;

	case Area::VISIBLE_NOT_ACTIVE:	// make invisible then unload
		Area_MakeInvisible( iAreaNumber );
		if (bAsync)
			Area_UnloadAsync( iAreaNumber );
		else
			Area_UnloadSync( iAreaNumber );
		break;

	case Area::VISIBLE_AND_ACTIVE:	// shouldnt happen with current rules
		ntError_p( 0, ("Area_AbortLoadOrUnload should not be called on an active area") );
		break;
	}
}

//--------------------------------------------------
//!
//!	AreaManager::Area_LoadThenActivate
//! Handle loading, spin till loaded then activate
//! Invalid for active or unloaded areas
//!
//--------------------------------------------------
inline void AreaManager::Area_LoadThenActivate( int32_t iAreaNumber )
{
	switch ( m_areas[iAreaNumber].GetStatus() )
	{
	case Area::UNLOADED:	// shouldnt happen
		ntError_p( 0, ("Area_LoadThenActivate should not be called on an unloaded area") );
		break;

	case Area::LOADING:		// spin till loaded, then fall through
		do
		{
			AreaResourceDB::Get().Update();
		}
		while (!Area_LoadFinished(iAreaNumber));

	case Area::LOADED:		// now make visible, then fall through
		Area_MakeVisible(iAreaNumber);

	case Area::VISIBLE_NOT_ACTIVE: // make active then break
		Area_MakeActive(iAreaNumber);
		break;

	case Area::VISIBLE_AND_ACTIVE: // shouldnt happen with current rules
		ntError_p( 0, ("Area_LoadThenActivate should not be called on an active area") );
		break;
	}
}

//--------------------------------------------------
//!
//!	AreaManager::Area_LoadFinished
//! See if a load has finished, mark it so when it is
//!
//--------------------------------------------------
inline bool AreaManager::Area_LoadFinished( int32_t iAreaNumber )
{
	if ( m_areas[iAreaNumber].GetStatus() == Area::LOADED )
		return true;

	if ( AreaResourceDB::Get().Area_LoadFinished(iAreaNumber) )
	{
		m_areas[iAreaNumber].SetStatus( Area::LOADED );
		return true;
	}

	return false;
}

#endif // AREA_SYSTEM_INL

