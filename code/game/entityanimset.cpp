//--------------------------------------------------
//!
//!	\file entityanimset.cpp
//!	Class that glues together global EntityAnimContainer
//! classes with entitys
//!
//--------------------------------------------------

#include "game/entityanimset.h"
#include "game/entityanimcontainer.h"
#include "anim/animloader.h"
#include "anim/animator.h"
#include "tbd/functor.h"

// flag to allow the forced loading of animations missing from ARM files
extern bool g_bAllowMissingData;

//------------------------------------------------------------------------------------------
//!
//! EntityAnimSet::ctor
//!
//!
//------------------------------------------------------------------------------------------
EntityAnimSet::EntityAnimSet() :
//	m_bAnimsAvailable(false),
	g_pAnimationMap(0),
	m_pAnimator(0),
	m_pGetNameFunc(0)
{}

//------------------------------------------------------------------------------------------
//!
//! EntityAnimSet::dtor
//!
//!
//------------------------------------------------------------------------------------------
EntityAnimSet::~EntityAnimSet()
{
	NT_DELETE( m_pGetNameFunc );
	m_pGetNameFunc = NULL;
}

//------------------------------------------------------------------------------------------
//!
//! EntityAnimSet::GetName
//!
//!
//------------------------------------------------------------------------------------------
ntstd::String EntityAnimSet::GetName() const { return ( *m_pGetNameFunc )(); }

//------------------------------------------------------------------------------------------
//!
//! EntityAnimSet::InstallAnimator
//! Hook up our animator object with our shared set of animation and anim event lists
//!
//------------------------------------------------------------------------------------------
void EntityAnimSet::InstallAnimator(CAnimator *pAnimator, const CHashedString& animContainerName)
{
	// Set our animator and animation map
	m_pAnimator = pAnimator;
	
	if (animContainerName.IsNull() == false)
	{
		g_pAnimationMap = AnimContainerManager::Get().GetAnimContainer( animContainerName.GetHash() );
		user_error_p( g_pAnimationMap, ("Could not find a container called %s used by %s", ntStr::GetString(animContainerName), (*m_pGetNameFunc)().c_str() ) );
	}

	ntError_p( m_pAnimator, ("Invalid animator passed to InstallAnimator()") );

	// Register the anim events with this particular entity
	RegisterAnimEventLists( g_pAnimationMap );
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::FindAnimShortCut
//!	find animation shortcut if it exists.
//!
//------------------------------------------------------------------------------------------
const AnimShortCut* EntityAnimSet::FindAnimShortCut( const CHashedString& name, bool bSearchPooled ) const
{
	// search normal anim container
	const AnimShortCut* pResult = NULL;
	
	if (g_pAnimationMap)
		pResult = g_pAnimationMap->FindAnimShortCut( name.GetValue(), bSearchPooled);

	if (pResult == NULL)
	{
		// search NS animation containers
		for (	AnimMapList::const_iterator it = m_gNSAnimationMaps.begin();
				it != m_gNSAnimationMaps.end(); ++it )
		{
			pResult = (*it)->FindAnimShortCut( name.GetValue(), bSearchPooled );
			if (pResult != NULL)
				break;
		} 
	}

	return pResult;
}

#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::FindAnimShortCut
//! debug slow method based on headers, used only for name lookups
//!
//------------------------------------------------------------------------------------------
const AnimShortCut* EntityAnimSet::FindAnimShortCut( const CAnimationHeader* pHeader ) const
{
	// search normal anim container
	const AnimShortCut* pResult = NULL;
	
	if (g_pAnimationMap)
		pResult = g_pAnimationMap->FindAnimShortCut(pHeader);

	if (pResult == NULL)
	{
		// search NS animation containers
		for (	AnimMapList::const_iterator it = m_gNSAnimationMaps.begin();
				it != m_gNSAnimationMaps.end(); ++it )
		{
			pResult = (*it)->FindAnimShortCut(pHeader);
			if (pResult != NULL)
				break;
		}
	}

	return pResult;
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::GetAnimName
//! Get the name of an animation from an animation header. This is essentially the
//!	reverse of the FindAnimHeader function. SHOULD ONLY BE USED FOR DEBUGGING!
//!
//------------------------------------------------------------------------------------------
const char* EntityAnimSet::GetAnimName( const CAnimationHeader* pHeader ) const
{
	const AnimShortCut* pShortcut = FindAnimShortCut( pHeader );
	if (pShortcut)
		return ntStr::GetString( pShortcut->GetShortNameHash() );

	return "Unknown name";
}
#endif

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::FindAnimHeader
//! Retrieve animation header
//!
//------------------------------------------------------------------------------------------
const CAnimationHeader* EntityAnimSet::FindAnimHeader( const CHashedString& shortName, bool bSearchPooled ) const
{
	const AnimShortCut* pShortcut = FindAnimShortCut( shortName, bSearchPooled );
	if (pShortcut)
		return pShortcut->GetHeader();

	user_error_p( m_pGetNameFunc != NULL, ("m_pGetNameFunc should have been set on construction to point at a valid function.") );

	// cant find anything, give them a nice fat error, return the error animation
	user_warn_msg( ( "%s has no animation named %s\n", (*m_pGetNameFunc)().c_str(), ntStr::GetString(shortName) ));
	return CAnimLoader::Get().GetErrorAnimation();
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::InstallNSAnims
//!	Add a set of NS animations for this entity
//!
//------------------------------------------------------------------------------------------
void EntityAnimSet::InstallNSAnims( const CHashedString& animContainerName )
{
	EntityAnimContainer* pNSAnimationMap = AnimContainerManager::Get().GetAnimContainer( animContainerName.GetHash() );
	user_error_p( pNSAnimationMap, ("Failed to retrieve anim container %s used by %s", ntStr::GetString(animContainerName), (*m_pGetNameFunc)().c_str() ));

	// add to our list of ninja sequence containers
	m_gNSAnimationMaps.push_back( pNSAnimationMap );

	// Register the anim events with this particular entity
	RegisterAnimEventLists( pNSAnimationMap );
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::UninstallNSAnims
//!	Remove a set of NS animations for this entity
//!
//------------------------------------------------------------------------------------------
void EntityAnimSet::UninstallNSAnims(const CHashedString& animContainerName)
{
	for (	AnimMapList::iterator it = m_gNSAnimationMaps.begin();
			it != m_gNSAnimationMaps.end(); ++it )
	{
		if ( (*it)->GetName().GetValue() == animContainerName.GetValue() )
		{
			// clean anim event lists
			UnRegisterAnimEventLists( *it );

			// remove from map
			m_gNSAnimationMaps.erase( it );
			return;
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::AreNSAnimsAdded
//!	test to see if a ninja sequence animation container has been added
//!
//------------------------------------------------------------------------------------------
bool EntityAnimSet::AreNSAnimsAdded( const CHashedString& animContainerName ) const
{
	for (	AnimMapList::const_iterator it = m_gNSAnimationMaps.begin();
			it != m_gNSAnimationMaps.end(); ++it )
	{
		if ( (*it)->GetName().GetValue() == animContainerName.GetValue() )
			return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::RegisterAnimEventLists
//!	Register the anim eventlists
//!
//------------------------------------------------------------------------------------------
void EntityAnimSet::RegisterAnimEventLists( EntityAnimContainer* pAnimContainer )
{
	if (pAnimContainer == 0)
		return;

	// FIXME_WIL
	// this is fundamentally broken as animevent lists assume short names are unique, thus
	// anim events are completely broken by anim pools

	// will swap to no pre-processing of anim event lists, and creation of anim event monitors
	// at the point animations are added to animators.

	for(	EntityAnimContainer::ShortcutMap::iterator animIt = pAnimContainer->m_shortcutAnims.begin();
			animIt != pAnimContainer->m_shortcutAnims.end(); ++animIt )
	{
		for (	AnimShortCut::AnimEventLists::iterator eventIt = animIt->second->m_animEventLists.begin();
				eventIt != animIt->second->m_animEventLists.end(); ++eventIt )
		{
			m_pAnimator->GetAnimEventHandler().AddAnimEventMonitor( animIt->second->GetShortNameHash().GetValue(), *eventIt );
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::UnRegisterAnimEventLists
//!	UnRegister the anim eventlists
//!
//------------------------------------------------------------------------------------------
void EntityAnimSet::UnRegisterAnimEventLists( EntityAnimContainer* pAnimContainer )
{
	if (pAnimContainer == 0)
		return;

	// FIXME_WIL
	// this is fundamentally broken as animevent lists assume short names are unique, thus
	// anim events are completely broken by anim pools

	// will swap to no pre-processing of anim event lists, and creation of anim event monitors
	// at the point animations are added to animators.

	for(	EntityAnimContainer::ShortcutMap::iterator animIt = pAnimContainer->m_shortcutAnims.begin();
			animIt != pAnimContainer->m_shortcutAnims.end(); ++animIt )
	{
		for (	AnimShortCut::AnimEventLists::iterator eventIt = animIt->second->m_animEventLists.begin();
				eventIt != animIt->second->m_animEventLists.end(); ++eventIt )
		{
			m_pAnimator->GetAnimEventHandler().RemoveAnimEventMonitor( animIt->second->GetShortNameHash().GetValue() );
		}
	}
}

#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet::RebuildAnimEventLists
//!	Debug functionality
//!
//------------------------------------------------------------------------------------------
void EntityAnimSet::RebuildAnimEventLists( const CHashedString& containerName )
{
	// get rid of all anim event lists registerd so far
	m_pAnimator->GetAnimEventHandler().RemoveAllMonitors();

	bool bFound = false;

	// find the container we're supposed to be rebuilding
	if ( g_pAnimationMap && g_pAnimationMap->GetName().GetValue() == containerName.GetValue() )
	{
		g_pAnimationMap->RebuildAnimEventLists();
		bFound = true;
	}
	else
	{
		for (	AnimMapList::iterator it = m_gNSAnimationMaps.begin();
				it != m_gNSAnimationMaps.end(); ++it )
		{
			if ((*it)->GetName().GetValue() == containerName.GetValue())
			{
				(*it)->RebuildAnimEventLists();
				bFound = true;
				break;
			}
		}
	}

	user_warn_p( bFound, ("Couldnt rebuild container %s, not present on entity %s", ntStr::GetString(containerName), GetName().c_str() ) );

	// now add them all again
	RegisterAnimEventLists( g_pAnimationMap );

	for (	AnimMapList::iterator it = m_gNSAnimationMaps.begin();
			it != m_gNSAnimationMaps.end(); ++it )
	{
		RegisterAnimEventLists( *it );
	}
}
#endif
