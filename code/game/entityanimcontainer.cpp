//--------------------------------------------------
//!
//!	\file entityanimcontainer.cpp
//!	classes that assocaiate animation headers with
//! entitys
//!
//--------------------------------------------------

#include "game/entityanimcontainer.h"
#include "game/randmanager.h"
#include "objectdatabase/dataobject.h"
#include "anim/animloader.h"
#include "area/arearesourcedb.h"

// flag to allow the forced loading of animations missing from ARM files
extern bool g_bAllowMissingData;
extern bool g_bAllowMissingContainers;

//!-----------------------------------------------------------------------------------------------
//!
//! Anim
//! XML object that constitutes an entry within an AnimContainer object
//!
//!-----------------------------------------------------------------------------------------------
class Anim
{
public:
	ntstd::String	m_name;
	ntstd::String	m_filename;
	ntstd::String	m_animEventList;
	ntstd::String	m_animEventList2;
};

START_CHUNKED_INTERFACE(Anim, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(m_name,				Name)
	PUBLISH_VAR_AS(m_filename,			Filename)
	PUBLISH_VAR_AS(m_animEventList,		AnimEventList)
	PUBLISH_VAR_AS(m_animEventList2,	AnimEventList2)
END_STD_INTERFACE

//!-----------------------------------------------------------------------------------------------
//!
//! AnimContainer
//! XML object that constitutes a deep list of Anim objects
//!
//!-----------------------------------------------------------------------------------------------
class AnimContainer
{
public:
//friend class EntityAnimContainer;
	AnimContainer();
	~AnimContainer();

	void PostConstruct();

	DeepList	m_Anims;
};

START_CHUNKED_INTERFACE(AnimContainer, Mem::MC_ENTITY)
	PUBLISH_DEEP_LIST_AS(m_Anims,		Anims)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	AnimContainer::ctor
//!	constructor for animation container
//!
//------------------------------------------------------------------------------------------
AnimContainer::AnimContainer()
{
}

//------------------------------------------------------------------------------------------
//!
//!	AnimContainer::PostConstruct
//!	deferred ctor
//!
//------------------------------------------------------------------------------------------
void AnimContainer::PostConstruct()
{
	AnimContainerManager::Get().AddContainer(this);
}

//------------------------------------------------------------------------------------------
//!
//!	AnimContainer::dtor
//!	descructor for animation container
//!
//------------------------------------------------------------------------------------------
AnimContainer::~AnimContainer()
{
	AnimContainerManager::Get().RemoveContainer(this);
}




//------------------------------------------------------------------------------------------
//!
//!	AnimShortCut::ctor
//!
//!
//------------------------------------------------------------------------------------------
AnimShortCut::AnimShortCut( const Anim* pXMLDef )
{
	// we should get rid of this at some point
	m_shortNameHash = pXMLDef->m_name.c_str();

	// would ideally like this to be converted at load time
	char pPlatformName[MAX_PATH];
	CAnimLoader::MakePlatformAnimName( pXMLDef->m_filename.c_str(), pPlatformName );

	// note, this debug load name is the UNTRANSFORMED anim name, as will act as an input to
	// MakePlatformAnimName within the area system
	if (g_bAllowMissingData)
		m_debugLoadName = pXMLDef->m_filename.c_str();

#ifndef _RELEASE
	m_pXMLDef = pXMLDef;
#endif

	m_pAnimationHeader = 0;

	CHashedString animCacheKey(pPlatformName);
	m_animCacheKey = animCacheKey.GetValue();

	// attach anim event lists if they exist
	InsertAnimEventList( pXMLDef->m_animEventList.c_str() );
	InsertAnimEventList( pXMLDef->m_animEventList2.c_str() );
}

//------------------------------------------------------------------------------------------
//!
//!	AnimShortCut::dtor
//!
//!
//------------------------------------------------------------------------------------------
AnimShortCut::~AnimShortCut()
{
	if (m_pAnimationHeader)
		UninstallAnim();
}

#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!
//!	AnimShortCut::RebuildAnimEventLists
//!	Debug functionality to re-set anim event lists	
//!
//------------------------------------------------------------------------------------------
void AnimShortCut::RebuildAnimEventLists()
{
	m_animEventLists.clear();
	InsertAnimEventList( m_pXMLDef->m_animEventList.c_str() );
	InsertAnimEventList( m_pXMLDef->m_animEventList2.c_str() );
}
#endif

//------------------------------------------------------------------------------------------
//!
//!	AnimShortCut::InstallAnim
//!	Load our animation header from the cache
//!
//------------------------------------------------------------------------------------------
void AnimShortCut::InstallAnim( const char* pDebugTag )
{
	if (g_bAllowMissingData)
	{
		if ( CAnimLoader::Get().Loaded_Cache( m_animCacheKey ) == false)
		{
			// this anim must be missing from the arm file
			ntPrintf( Debug::DCU_ANIM, "*****************************************************************\n" );
			ntPrintf( Debug::DCU_ANIM, "* Area %d: WARNING! missing anim %s.\n", AreaResourceDB::Get().DebugGetLastAreaLoaded(), m_debugLoadName.c_str());
			ntPrintf( Debug::DCU_ANIM, "* REGENERATE ARM! (Used by container %s)\n", pDebugTag );
			ntPrintf( Debug::DCU_ANIM, "*****************************************************************\n" );

			ntPrintf( Debug::DCU_RESOURCES, "*****************************************************************\n" );
			ntPrintf( Debug::DCU_RESOURCES, "* Area %d: WARNING! missing anim %s.\n", AreaResourceDB::Get().DebugGetLastAreaLoaded(), m_debugLoadName.c_str());
			ntPrintf( Debug::DCU_RESOURCES, "* REGENERATE ARM! (Used by container %s)\n", pDebugTag );
			ntPrintf( Debug::DCU_RESOURCES, "*****************************************************************\n" );

			uint32_t areaMask = 0xffffffff;
			if (AreaManager::Get().LevelActive())
				areaMask = 1 << ( AreaResourceDB::Get().DebugGetLastAreaLoaded()-1 );

			uint32_t resID = AreaResourceDB::Get().AddAreaResource( m_debugLoadName.c_str(), AreaResource::ANIM, areaMask );
			ntError_p( m_animCacheKey == resID, ("something has gone wrong with anim cache key generation") );

//			if (AreaManager::Get().LevelActive())
			{
				AreaResource* pResource = AreaResourceDB::Get().GetEntry(resID);
				pResource->Request( AreaResource::LOAD_SYNC );
			}
		}
	}
	else
	{
		user_error_p( CAnimLoader::Get().Loaded_Cache( m_animCacheKey ), 
				("Area %d: WARNING! missing anim %s used by %s. REGENERATE ARM!\n", 
				AreaResourceDB::Get().DebugGetLastAreaLoaded(), m_debugLoadName.c_str(), pDebugTag ));
	}

	ntError_p( m_pAnimationHeader == 0, ("Animation Header has already been installed") );
	ntError_p( CAnimLoader::Get().Loaded_Cache( m_animCacheKey ), ("Animation Header MUST have been loaded by now") );
	m_pAnimationHeader = CAnimLoader::Get().LoadAnim_FromData(m_animCacheKey,0,0,"UNKNOWN");
}

//------------------------------------------------------------------------------------------
//!
//!	AnimShortCut::UninstallAnim
//!	Unload our animation header from the cache
//!
//------------------------------------------------------------------------------------------
void AnimShortCut::UninstallAnim()
{
	ntError_p( m_pAnimationHeader != 0, ("Animation Header has not been installed") );
	CAnimLoader::Get().UnloadAnim_Key(m_animCacheKey);
	m_pAnimationHeader = 0;
}

//------------------------------------------------------------------------------------------
//!
//!	AnimShortCut::InsertAnimEventList
//!	Attactch an anim event list to this animation shortcut
//!
//------------------------------------------------------------------------------------------
#define _DEBUG_REPORT_MISSING_ANIMEVENTLISTS

void AnimShortCut::InsertAnimEventList(const CHashedString& name)
{
	static const CHashedString noAnim("NoAnim");
	static const CHashedString emptyName("\0");
	static const CHashedString nullName("NULL");

	if ((name != noAnim) && (name != emptyName) && (name != nullName))
	{
		CAnimEventList* pEventList = ObjectDatabase::Get().GetPointerFromName<CAnimEventList*>( name );
		
		if (pEventList)
		{
			m_animEventLists.push_back(pEventList);
		}
#ifdef _DEBUG_REPORT_MISSING_ANIMEVENTLISTS
		else
		{
			ntPrintf( "Couldn't find AnimEventList '%s'\n", ntStr::GetString(name) );
		}
#endif
	}	
}





//------------------------------------------------------------------------------------------
//!
//!	EntityAnimContainer::EntityAnimContainer
//!	constructor for animation container, setup XML shortcuts
//!
//------------------------------------------------------------------------------------------
//#define DUMP_LOADED_ANIMS

EntityAnimContainer::EntityAnimContainer( AnimContainer* pXMLData, const CHashedString& contName ) :
	m_installCount( 0 ),
	m_pXMLData( pXMLData ),
	m_name( contName )
{
	#ifdef DUMP_LOADED_ANIMS
	ntPrintf( Debug::DCU_ANIM, ("-- Anims for Container: %s\n"), ntStr::GetString( m_name ) );
	#endif

	DeepListIterator animIt = m_pXMLData->m_Anims.DeepIterator();

	while( animIt )
	{
		const Anim* pNewAnimDef = (Anim*)animIt.GetValue();

		if( pNewAnimDef )
		{
			uint32_t hashVal = CHashedString( pNewAnimDef->m_name.c_str() ).GetValue();

			#ifdef DUMP_LOADED_ANIMS
			ntPrintf( Debug::DCU_ANIM, ("Area_RegisterResource_Anim( \"%s\" )\n"), pNewAnimDef->m_filename.c_str() );
			#endif

			AnimShortCut* pNewShortcut = NT_NEW_CHUNK( Mem::MC_ENTITY ) AnimShortCut( pNewAnimDef );

			if ( m_shortcutAnims.find( hashVal ) == m_shortcutAnims.end() )
			{
				// we're a new entry, add to the map
				m_shortcutAnims[ hashVal ] = pNewShortcut;
			}
			else
			{
				// we're already here, add to the pooled anims
				PoolMap::iterator poolEntry = m_pooledAnims.find( hashVal );

				if ( poolEntry == m_pooledAnims.end() )
				{
					// need a new pool entry, insert our original anim into the head of it
					m_pooledAnims[ hashVal ] = NT_NEW_CHUNK( Mem::MC_ENTITY ) AnimList;

					AnimShortCut* pOriginal = m_shortcutAnims[ hashVal ];
					m_pooledAnims[ hashVal ]->push_back( pOriginal );
				}

				// now add our new animation to this entry
				m_pooledAnims[ hashVal ]->push_back( pNewShortcut );
			}

			m_totalAnims.push_back( pNewShortcut );
		}
		++animIt;
	}
}

EntityAnimContainer::~EntityAnimContainer()
{
	if (AnimationsInstalled())
	{
		m_installCount = 1;
		UninstallAnimations();
	}

	// delete all anims shortcuts first
	for ( AnimList::iterator it = m_totalAnims.begin(); it != m_totalAnims.end(); )
	{
		AnimShortCut* pToDelete = *it;
		NT_DELETE_CHUNK( Mem::MC_ENTITY, pToDelete );
		it = m_totalAnims.erase( it );
	}

	// delete pool lists
	for ( PoolMap::iterator poolIt = m_pooledAnims.begin(); poolIt != m_pooledAnims.end(); )
	{
		AnimList* pToDelete = poolIt->second;
		NT_DELETE_CHUNK( Mem::MC_ENTITY, pToDelete );
		poolIt = m_pooledAnims.erase( poolIt );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimContainer::InstallAnimations
//!	Make sure all our animations are loaded into the anim cache
//!
//------------------------------------------------------------------------------------------
void EntityAnimContainer::InstallAnimations()
{
	const char* pName = ntStr::GetString( m_name );

	if (m_installCount == 0)
	{
	for ( AnimList::iterator it = m_totalAnims.begin(); it != m_totalAnims.end(); ++it )
			(*it)->InstallAnim( pName );
	}
	m_installCount++;
}

//------------------------------------------------------------------------------------------
//!
//!	AnimContainer::UninstallAnimations
//!	Make sure all our animations are unloaded
//!
//------------------------------------------------------------------------------------------
void EntityAnimContainer::UninstallAnimations()
{
	ntError_p( m_installCount > 0, ("This animation container is not allready installed") );
	m_installCount--;

	if (m_installCount == 0)
	{
		for ( AnimList::iterator it = m_totalAnims.begin(); it != m_totalAnims.end(); ++it )
			(*it)->UninstallAnim();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimContainer::FindAnimShortCut
//! retrieve this animation from our set
//!
//------------------------------------------------------------------------------------------
const AnimShortCut*	EntityAnimContainer::FindAnimShortCut( uint32_t cacheKey, bool bSearchPooled ) const
{
	if (bSearchPooled)
	{
		PoolMap::const_iterator poolIt = m_pooledAnims.find( cacheKey );
		if ( poolIt != m_pooledAnims.end() )
		{
			// we are a pooled animation, choose one randomly
			uint32_t num = poolIt->second->size();
			uint32_t anim = ntstd::Min( (uint32_t)grandf( _R( num ) ), num-1 );
			return (*poolIt->second)[ anim ];
		}
	}

	ShortcutMap::const_iterator animIt = m_shortcutAnims.find( cacheKey );
	if ( animIt != m_shortcutAnims.end() )
		return animIt->second;

	return NULL;
}

#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!
//!	EntityAnimContainer::FindAnimShortCut
//!
//------------------------------------------------------------------------------------------
const AnimShortCut*	EntityAnimContainer::FindAnimShortCut( const CAnimationHeader* pHeader ) const
{
	for(	AnimList::const_iterator animIt = m_totalAnims.begin();
			animIt != m_totalAnims.end(); ++animIt )
	{
		if((*animIt)->GetHeader() == pHeader)
			return *animIt;
	}

	return NULL;
}

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimContainer::RebuildAnimEventLists
//!	Debug functionality to rebuild anim event lists
//!
//------------------------------------------------------------------------------------------
void EntityAnimContainer::RebuildAnimEventLists()
{
	for ( AnimList::iterator it = m_totalAnims.begin(); it != m_totalAnims.end(); ++it )
		(*it)->RebuildAnimEventLists();

}
#endif





//------------------------------------------------------------------------------------------
//!
//!	AnimContainerManager::AddContainer
//!	Manage creation of entity animation container objects
//!
//------------------------------------------------------------------------------------------
void AnimContainerManager::AddContainer( AnimContainer* pXMLData )
{
	ntError_p( pXMLData, ("Invalid pointer passed to AddContainer()") );

	// retrieve name of this container, so we can add to hash map
	DataObject* pDO =  ObjectDatabase::Get().GetDataObjectFromPointer( pXMLData );
	ntError_p( pDO, ("MUST have a corresponding data object for this anim container") );
	
	DataObject::NameType containerName = pDO->GetName();
	CHashedString nameHash( containerName );

	// make sure we're not already here
	ntError_p( m_XMLContMap.find( nameHash.GetValue() ) == m_XMLContMap.end(), ("Animation container %s has already been added to the AnimContainerManager", ntStr::GetString(nameHash) ) )		
	ntError_p( m_gameContMap.find( nameHash.GetValue() ) == m_gameContMap.end(), ("Animation container %s has already been added to the AnimContainerManager", ntStr::GetString(nameHash) ) )

	m_XMLContMap[ nameHash.GetValue() ] = pXMLData;
}

//------------------------------------------------------------------------------------------
//!
//!	AnimContainerManager::RemoveContainer
//!	Manage destruction of entity animation container objects
//!
//------------------------------------------------------------------------------------------
void AnimContainerManager::RemoveContainer( AnimContainer* pXMLData )
{
	ntError_p( pXMLData, ("Invalid pointer passed to AddContainer()") );

	// retrieve name of this container, so we can add to hash map
	DataObject* pDO =  ObjectDatabase::Get().GetDataObjectFromPointer( pXMLData );
	ntError_p( pDO, ("MUST have a corresponding data object for this anim container") );
	
	DataObject::NameType containerName = pDO->GetName();
	CHashedString nameHash( containerName );

	// make sure we're already here
	ntError_p( m_XMLContMap.find( nameHash.GetValue() ) != m_XMLContMap.end(), ("Animation container %s has not been added to the AnimContainerManager", ntStr::GetString(nameHash) ) )

	// remove from hash map
	m_XMLContMap.erase( m_XMLContMap.find( nameHash.GetValue() ) );

	// remember, we may not have created our EntityAnimContainer 
	GameMap::iterator gameMapIt = m_gameContMap.find( nameHash.GetValue() );
	if ( gameMapIt != m_gameContMap.end() )
	{
		if (g_bAllowMissingContainers)
			gameMapIt->second->UninstallAnimations();

		NT_DELETE_CHUNK( Mem::MC_ENTITY, gameMapIt->second );
		m_gameContMap.erase( gameMapIt );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	AnimContainerManager::GetAnimContainer
//!	retrieve an item from our cache
//! Also contains defferred creation of EntityAnimContainer objects
//!
//------------------------------------------------------------------------------------------
EntityAnimContainer* AnimContainerManager::GetAnimContainer( uint32_t cacheKey )
{
	XMLMap::iterator xmlMapIt = m_XMLContMap.find( cacheKey );
	if ( xmlMapIt == m_XMLContMap.end() )
		return 0;

	// we create EntityAnimContainer objects when theyre first required by entities
	// (i.e retrieved for the first time) as we know all the containers constituting
	// our deep lists have been hooked up by then.

	GameMap::iterator gameMapIt = m_gameContMap.find( cacheKey );
	if ( gameMapIt == m_gameContMap.end() )
	{
		// retrieve the actual container name as a hash
		DataObject* pDO =  ObjectDatabase::Get().GetDataObjectFromPointer( xmlMapIt->second );
		ntError_p( pDO, ("MUST have a corresponding data object for this anim container") );
	
		DataObject::NameType containerName = pDO->GetName();
		CHashedString nameHash( containerName );

		ntError_p( cacheKey == nameHash.GetValue(), ("Something has gone wrong with our cache key generation here") );

		// create the object
		EntityAnimContainer* pNewContainer = NT_NEW_CHUNK( Mem::MC_ENTITY ) EntityAnimContainer( xmlMapIt->second, nameHash );
		m_gameContMap[ cacheKey ] = pNewContainer;

		if (g_bAllowMissingContainers)
			pNewContainer->InstallAnimations();

		return pNewContainer;
	}

	return gameMapIt->second;
}

