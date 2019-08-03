//--------------------------------------------------
//!
//!	\file BSAnimContainer.cpp
//!	Blendshape animation container and XML database obj
//!
//--------------------------------------------------


#include "blendshapes/anim/bsanimcontainer.h"
#include "objectdatabase/DataObject.h"
#include "blendshapes/anim/bsanimator.h"
#include "area/arearesourcedb.h"

extern bool g_bAllowMissingData;
extern bool g_bAllowMissingContainers;

/////////////////////////////////////////////////////////////////////////////
//
//						XML interface
//
/////////////////////////////////////////////////////////////////////////////
struct BSAnim
{
	CKeyString			m_obShortName;
	CKeyString			m_obFileName;
};

START_CHUNKED_INTERFACE(BSAnim, Mem::MC_PROCEDURAL )
	PUBLISH_VAR_AS( m_obShortName, Name )
	PUBLISH_VAR_AS( m_obFileName, Filename )
END_STD_INTERFACE

struct BSAnimContainer
{
	DeepList	m_BSAnims;
};

START_CHUNKED_INTERFACE(BSAnimContainer, Mem::MC_PROCEDURAL)
	PUBLISH_DEEP_LIST_AS( m_BSAnims, BSAnims )
END_STD_INTERFACE

/////////////////////////////////////////////////////////////////////////////
//
//						BSAnimShortcut
//
/////////////////////////////////////////////////////////////////////////////

BSAnimShortcut::BSAnimShortcut( const CHashedString& shortName, const char* pFileName, const char* containerName, bool bWithinNinjaSeq ) :
	m_obShortName(shortName),
	m_obContainerName( containerName )
{
	// would ideally like this to be converted at load time
	char pPlatformName[MAX_PATH];
	BSAnimManager::MakePlatformName( pFileName, pPlatformName );

	m_pobBSAnimHeader = 0;

	CHashedString animCacheKey(pPlatformName);
	m_animCacheKey = animCacheKey.GetValue();

	if (g_bAllowMissingContainers || bWithinNinjaSeq)
	{
		// note, this debug load name is the UNTRANSFORMED anim name, as will act as an input to
		// MakePlatformAnimName within the area system
		m_debugLoadName = pFileName;
		InstallAnim( containerName );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	BSAnimShortcut::dtor
//!
//!
//------------------------------------------------------------------------------------------
BSAnimShortcut::~BSAnimShortcut()
{
	if (m_pobBSAnimHeader)
		UninstallAnim();
}

//------------------------------------------------------------------------------------------
//!
//!	BSAnimShortcut::InstallAnim
//!	Load our animation header from the cache
//!
//------------------------------------------------------------------------------------------
void BSAnimShortcut::InstallAnim( const char* pDebugTag )
{
	// check if we're already installed
	if (m_pobBSAnimHeader)
		return;

	if (g_bAllowMissingData)
	{
		if ( BSAnimManager::Get().Loaded_Cache( m_animCacheKey ) == false)
		{
			// this anim must be missing from the arm file
			ntPrintf( Debug::DCU_ANIM, "*****************************************************************\n" );
			ntPrintf( Debug::DCU_ANIM, "* Area %d: WARNING! missing bs anim %s.\n", AreaResourceDB::Get().DebugGetLastAreaLoaded(), m_debugLoadName.c_str());
			ntPrintf( Debug::DCU_ANIM, "* REGENERATE ARM! (Used by container %s)\n", pDebugTag );
			ntPrintf( Debug::DCU_ANIM, "*****************************************************************\n" );

			ntPrintf( Debug::DCU_RESOURCES, "*****************************************************************\n" );
			ntPrintf( Debug::DCU_RESOURCES, "* Area %d: WARNING! missing bs anim %s.\n", AreaResourceDB::Get().DebugGetLastAreaLoaded(), m_debugLoadName.c_str());
			ntPrintf( Debug::DCU_RESOURCES, "* REGENERATE ARM! (Used by container %s)\n", pDebugTag );
			ntPrintf( Debug::DCU_RESOURCES, "*****************************************************************\n" );

			uint32_t areaMask = 0xffffffff;
			if (AreaManager::Get().LevelActive())
				areaMask = 1 << ( AreaResourceDB::Get().DebugGetLastAreaLoaded()-1 );

			uint32_t resID = AreaResourceDB::Get().AddAreaResource( m_debugLoadName.c_str(), AreaResource::BSANIM, areaMask );
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
		user_error_p( BSAnimManager::Get().Loaded_Cache( m_animCacheKey ), 
				("Area %d: WARNING! missing anim %s used by %s. REGENERATE ARM!\n", 
				AreaResourceDB::Get().DebugGetLastAreaLoaded(), m_debugLoadName.c_str(), pDebugTag ));
	}

	ntError_p( m_pobBSAnimHeader == 0, ("Animation Header has already been installed") );
	ntError_p( BSAnimManager::Get().Loaded_Cache( m_animCacheKey ), ("Animation Header MUST have been loaded by now") );
	m_pobBSAnimHeader = BSAnimManager::Get().Load_FromData(m_animCacheKey,0,0,"UNKNOWN");
}

//------------------------------------------------------------------------------------------
//!
//!	BSAnimShortcut::UninstallAnim
//!	Unload our animation header from the cache
//!
//------------------------------------------------------------------------------------------
void BSAnimShortcut::UninstallAnim()
{
	ntError_p( m_pobBSAnimHeader != 0, ("Animation Header has not been installed") );
	//BSAnimManager::Get().Unload_Key(m_animCacheKey);
	m_pobBSAnimHeader = 0;	// smart pointer will release the data
}

/////////////////////////////////////////////////////////////////////////////
//
//						BSAnimShortcutContainer
//
/////////////////////////////////////////////////////////////////////////////

BSAnimShortcutContainer::BSAnimShortcutContainer() 
: m_obBSAnims()
{
	m_obBSAnims.clear();
	BSAnimContainerManager::Get().RegisterContainer( this );
}

BSAnimShortcutContainer::~BSAnimShortcutContainer()
{
	BSAnimContainerManager::Get().UnRegisterContainer( this );
	RemoveAllBSAnims();
}


void BSAnimShortcutContainer::AddBSAnim( CHashedString shortName, const char* fileName, const char* containerName, bool bWithinNinjaSeq )
{
	ntAssert( fileName );
	ntAssert( containerName );
	ntAssert_p( !HasBSAnim( shortName ), ("bsanimation %s was already added\n",ntStr::GetString(shortName)) );
	
	m_obBSAnims[ shortName ] = NT_NEW_CHUNK(Mem::MC_PROCEDURAL) BSAnimShortcut( shortName, fileName, containerName, bWithinNinjaSeq );
}


bool BSAnimShortcutContainer::HasBSAnim( CHashedString shortName ) const
{
	return ( m_obBSAnims.find( shortName ) != m_obBSAnims.end() );
}


// NOTE: does it make sense to return another header if the one we asked for is not found? 
// I'd rather have it return null if not found... Let me know if this is changed as it may
// affect the bsanimator behaviour /Ozz
BSAnimHeaderPtr_t BSAnimShortcutContainer::GetBSAnimHeader( CHashedString shortName ) const
{
	BSAnimShortcutCollection::const_iterator it = m_obBSAnims.find( shortName );
	return it == m_obBSAnims.end() ? 0 : it->second->GetHeader();
}


void BSAnimShortcutContainer::AddBSAnimsFromContainer( DataObject* pDO, bool bWithinNinjaSeq )
{
	ntAssert_p( pDO, ("BSAnimContainer database object ptr is null") );
	ntAssert_p( !IsBSAnimContainerAdded(ntStr::GetString(pDO->GetName())), ("BSAnimContainer %s was already added",ntStr::GetString(pDO->GetName())) );

	StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
	DataInterfaceField* pField = pInterface->GetFieldByName( CHashedString(HASH_STRING_BSANIMS) );
	ntError(pField);

	const DeepList& pAnims = pField->GetAs<DeepList>( pDO );
	ntAssert( pField->GetType() == typeof_num( pAnims ) );

	DeepListIterator animIt = pAnims.DeepIterator();
	while( animIt )
	{
		const void* pThisAnim = animIt.GetValue();
		DataObject* pThisDO = ObjectDatabase::Get().GetDataObjectFromPointer( pThisAnim );
		if( pThisAnim )
		{
			ntAssert( pThisDO );
			StdDataInterface* pThisInterface = ObjectDatabase::Get().GetInterface( pThisDO );

			const ntstd::String shortName = pThisInterface->GetData( pThisDO, "Name" );
			const ntstd::String fileName = pThisInterface->GetData( pThisDO, "Filename" ).c_str();
			
			AddBSAnim( ntStr::GetString(shortName), ntStr::GetString(fileName), ntStr::GetString(pDO->GetName()), bWithinNinjaSeq );
		}
		++animIt;
	}
}


void BSAnimShortcutContainer::AddBSAnimsFromContainer( const CHashedString& containerName, bool bWithinNinjaSeq )
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( containerName );
	AddBSAnimsFromContainer( pDO, bWithinNinjaSeq );
}

void BSAnimShortcutContainer::RemoveBSAnimsFromContainer( CHashedString containerName  )
{
	ntAssert_p( IsBSAnimContainerAdded(containerName), ("BSAnimContainer %s was not previously added", ntStr::GetString(containerName)) );

	for ( BSAnimShortcutCollection::iterator it = m_obBSAnims.begin(); it != m_obBSAnims.end(); )
	{
		if ( it->second->GetContainerHash() == CHashedString(containerName) )
		{
			it = RemoveBSAnim( it );
		}
		else 
		{
			++it;
		}

	}
}

void BSAnimShortcutContainer::RemoveBSAnim( CHashedString shortName )
{
	BSAnimShortcutCollection::iterator it = m_obBSAnims.find( shortName );
	if ( it != m_obBSAnims.end() )
	{
		RemoveBSAnim( it );
	}
	else
	{
		ntAssert_p( false, ("BSAnimation %s couldn't be found", ntStr::GetString(shortName)) );
	}
}

void BSAnimShortcutContainer::RemoveAllBSAnims( void )
{
	while( !m_obBSAnims.empty() )
	{
		RemoveBSAnim( m_obBSAnims.begin() );
	}
}


bool BSAnimShortcutContainer::IsBSAnimContainerAdded( CHashedString containerName ) const
{
	for ( BSAnimShortcutCollection::const_iterator it = m_obBSAnims.begin(); it != m_obBSAnims.end(); ++it )
	{
		if ( it->second->GetContainerHash() == CHashedString(containerName) )
		{
			return true;
		}

	}
	return false;
}

BSAnimShortcutContainer::BSAnimShortcutCollection::iterator BSAnimShortcutContainer::RemoveBSAnim( BSAnimShortcutCollection::iterator it )
{
	// delete iterator from local container
	NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, it->second );
	return m_obBSAnims.erase( it );
}	

// called by area system to add or remove BS animations
void	BSAnimShortcutContainer::InstallAnimations( const CHashedString& containerName )
{
	for ( BSAnimShortcutCollection::const_iterator it = m_obBSAnims.begin(); it != m_obBSAnims.end(); ++it )
	{
		if ( it->second->GetContainerHash() == containerName )
		{
			it->second->InstallAnim( ntStr::GetString( containerName ) );
		}
	}
}

void	BSAnimShortcutContainer::UninstallAnimations( const CHashedString& containerName )
{
	for ( BSAnimShortcutCollection::const_iterator it = m_obBSAnims.begin(); it != m_obBSAnims.end(); ++it )
	{
		if ( it->second->GetContainerHash() == containerName )
		{
			it->second->UninstallAnim();
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//! BSAnimContainerManager
//! Singleton that maintains animation containers
//! Has global rather than level scope
//!
//------------------------------------------------------------------------------------------
void BSAnimContainerManager::RegisterContainer( BSAnimShortcutContainer* pCont )
{
	m_registerConts.push_back( pCont );
}

void BSAnimContainerManager::UnRegisterContainer( BSAnimShortcutContainer* pCont )
{
	for (	ContList::iterator it =  m_registerConts.begin();
			it != m_registerConts.end(); ++it )
	{
		if (*it == pCont)
		{
			m_registerConts.erase( it );
			return;
		}
	}
	ntError_p( 0, ("Failed to find container in BSAnimContainerManager") )
}

void BSAnimContainerManager::InstallAnimsForContainer( const CHashedString& containerName )
{
	for (	ContList::iterator it =  m_registerConts.begin();
			it != m_registerConts.end(); ++it )
	{
		(*it)->InstallAnimations( containerName );
	}
}	

void BSAnimContainerManager::RemoveAnimsForContainer( const CHashedString& containerName )
{
	for (	ContList::iterator it =  m_registerConts.begin();
			it != m_registerConts.end(); ++it )
	{
		(*it)->UninstallAnimations( containerName );
	}
}

//eof

