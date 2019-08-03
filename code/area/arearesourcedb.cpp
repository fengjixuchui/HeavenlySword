//--------------------------------------------------
//!
//!	\file arearesourcedb.cpp
//!	Database containing all resources managed by the
//! Area system
//!
//--------------------------------------------------

#include "area/arearesourcedb.h"
#include "gfx/texturemanager.h"
#include "gfx/clump.h"
#include "gfx/renderer.h"
#include "anim/animloader.h"
#include "game/luaglobal.h"
#include "game/shellconfig.h"
#include "input/inputhardware.h"
#include "blendshapes/blendshapes_managers.h"

#ifdef PLATFORM_PS3
#	include "core/archive.h"
#	include "core/wad.h"
#endif

// BINDFUNC:		Area_RegisterResource_Clump(  )
// DESCRIPTION:		Registers a texture resource with the current area.
static void Area_RegisterResource_Clump( const char* pName )
{
	ntAssert_p( AreaResourceDB::Exists(), ("Cannot call Area_RegisterResource_Clump with no AreaResourceDB singleton") );
	AreaResourceDB::Get().AutoAddAreaResource( pName, AreaResource::CLUMP );
}

// BINDFUNC:		Area_RegisterResource_Texture(  )
// DESCRIPTION:		Registers a texture resource with the current area.
static void Area_RegisterResource_Texture( const char* pName )
{
	ntAssert_p( AreaResourceDB::Exists(), ("Cannot call Area_RegisterResource_Texture with no AreaResourceDB singleton") );
	AreaResourceDB::Get().AutoAddAreaResource( pName, AreaResource::TEXTURE );
}

// BINDFUNC:		Area_RegisterResource_Anim(  )
// DESCRIPTION:		Registers an animation resource with the current area.
static void Area_RegisterResource_Anim( const char* pName )
{
	ntAssert_p( AreaResourceDB::Exists(), ("Cannot call Area_RegisterResource_Anim with no AreaResourceDB singleton") );
	AreaResourceDB::Get().AutoAddAreaResource( pName, AreaResource::ANIM );
}

// BINDFUNC:		Area_RegisterResource_BSAnim(  )
// DESCRIPTION:		Registers an animation resource with the current area.
static void Area_RegisterResource_BSAnim( const char* pName )
{
	ntAssert_p( AreaResourceDB::Exists(), ("Cannot call Area_RegisterResource_BSAnim with no AreaResourceDB singleton") );
	AreaResourceDB::Get().AutoAddAreaResource( pName, AreaResource::BSANIM );
}

// BINDFUNC:		Area_RegisterResource_AnimContainer(  )
// DESCRIPTION:		Registers an animation container resource with the current area.
static void Area_RegisterResource_AnimContainer( const char* pName )
{
	ntAssert_p( AreaResourceDB::Exists(), ("Cannot call Area_RegisterResource_Anim with no AreaResourceDB singleton") );
	AreaResourceDB::Get().AutoAddAreaResource( pName, AreaResource::ANIM_CONTAINER );
}

// BINDFUNC:		Area_RegisterResource_BSAnimContainer(  )
// DESCRIPTION:		Registers an bs animation container resource with the current area.
static void Area_RegisterResource_BSAnimContainer( const char* pName )
{
	ntAssert_p( AreaResourceDB::Exists(), ("Cannot call Area_RegisterResource_Anim with no AreaResourceDB singleton") );
	AreaResourceDB::Get().AutoAddAreaResource( pName, AreaResource::BSANIM_CONTAINER );
}




#ifdef PLATFORM_PS3

//--------------------------------------------------
//!
//!	NotifyLoadingCallback
//!
//--------------------------------------------------
static void NotifyLoadingCallback( const Loader::Request &req )
{
	// mark when a resource entry is about to be loaded
	ntAssert_p( req.m_Resource == Loader::LRT_AREA_RESOURCE, ("Wrong type of resource here") );

	AreaResource* pResource = (AreaResource*)req.m_UserData;
	if (pResource)
		pResource->MarkLoadProceeding();
}

//--------------------------------------------------
//!
//!	Loading finished call back
//!
//--------------------------------------------------
static void LoadFinishedCallback( Loader::CallbackStage stage, volatile void *arg )
{
	UNUSED( arg );
	UNUSED( stage );

	if ( stage == Loader::LCS_SYNCPOINT )
	{
		AreaResourceDB::Get().DumpAreaSysRam();
	}
}

#endif

//--------------------------------------------------
//!
//!	AreaResourceDB::AsycLoadCallback
//! Callback made to handle result of async loader
//!
//--------------------------------------------------
void	AreaResourceDB::AsycLoadCallback( const char* pFullPath, void* pUserData, void* pFileData, uint32_t fileSize )
{
	UNUSED(pFullPath);
	AreaResource* pResource = (AreaResource*)pUserData;

	if (pResource)
	{
		ntAssert_p(pResource->GetStatus() == AreaResource::LOADING,("Resource must be loading here"));
		ntAssert_p(strcmp( pResource->GetFullPath(),pFullPath ) == 0, ("Resource Pointer doesn not match loaded resource"));

		if (pResource->GetRefCount() == 0)
		{
			// since our request was made, it must have been canceled, mark as unloaded
			pResource->MarkUnloaded();
		}
		else
		{
			// all in order so process the resource, and mark as loaded.
			pResource->Load(pFileData,fileSize);
		}

		// remove us from the database confirmed list
		ResList::iterator it = ntstd::find( AreaResourceDB::Get().m_toBeConfirmed.begin(), AreaResourceDB::Get().m_toBeConfirmed.end(), pResource );
		ntAssert_p( it !=  AreaResourceDB::Get().m_toBeConfirmed.end(), ("This resource (%s) was not in the load list") );
		AreaResourceDB::Get().m_toBeConfirmed.erase( it );
	}
}

#ifdef PLATFORM_PS3
#define ADD_VRAM_CALLBACK
#endif

#ifdef ADD_VRAM_CALLBACK

extern Ice::Render::Region*		s_freeVideoRegionStart;
extern Ice::Render::Region*		s_usedVideoRegionStart;

typedef ntstd::Map< uint32_t,uint32_t >		AllocCounter;
typedef ntstd::pair< uint32_t,uint32_t >	AllocPair;
typedef ntstd::Vector< AllocPair >			AllocList;

class ICEAllocSizeComparator
{
public:
	bool operator()( AllocPair& first, AllocPair& second ) const { return ( first.first > second.first ); }
};

class ICEAllocCountComparator
{
public:
	bool operator()( AllocPair& first, AllocPair& second ) const { return ( first.second > second.second ); }
};

void DumpNamedBlockMap( AllocCounter& count, const char* pTag )
{
	AllocList freeBlocks;
	for (AllocCounter::iterator it = count.begin(); it != count.end(); ++it)
		freeBlocks.push_back( AllocPair( it->first, it->second ) );

	ntPrintf( Debug::DCU_RESOURCES, "\nICE VRAM %s block count by size:\n", pTag );
	ntPrintf( Debug::DCU_RESOURCES, "-------------------------------------\n" );
	
	ntstd::sort( freeBlocks.begin(), freeBlocks.end(), ICEAllocSizeComparator() );
	for (AllocList::iterator it = freeBlocks.begin(); it != freeBlocks.end(); ++it)
	{
		ntPrintf( Debug::DCU_RESOURCES, "%d bytes: %d blocks\n", it->first, it->second );
	}

	ntPrintf( Debug::DCU_RESOURCES, "\nICE VRAM %s block count by freqency:\n", pTag );
	ntPrintf( Debug::DCU_RESOURCES, "-------------------------------------\n" );

	ntstd::sort( freeBlocks.begin(), freeBlocks.end(), ICEAllocCountComparator() );
	for (AllocList::iterator it = freeBlocks.begin(); it != freeBlocks.end(); ++it)
	{
		ntPrintf( Debug::DCU_RESOURCES, "%d blocks: %d bytes\n", it->second, it->first );
	}
}

void DumpICEAllocatorState()
{
	ntPrintf( Debug::DCU_RESOURCES, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
	ntPrintf( Debug::DCU_RESOURCES, "!!!!!!!!!               ICE VRAM ALLOCATION                 !!!!!!!!!!!!\n" );
	ntPrintf( Debug::DCU_RESOURCES, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );

	ntError( s_freeVideoRegionStart );
	ntError( s_usedVideoRegionStart );

	AllocCounter total;

	if (s_freeVideoRegionStart)
	{
		AllocCounter count;
		for (Ice::Render::Region* r = s_freeVideoRegionStart; r; r = r->m_next)
		{
			if ( count.find(r->m_size) == count.end() )
				count[r->m_size] = 0;

			if ( total.find(r->m_size) == total.end() )
				total[r->m_size] = 0;

			count[r->m_size]++;
			total[r->m_size]++;
		}

		DumpNamedBlockMap( count, "free" );
	}

	if (s_usedVideoRegionStart)
	{
		AllocCounter count;
		for (Ice::Render::Region* r = s_usedVideoRegionStart; r; r = r->m_next)
		{
			if ( count.find(r->m_size) == count.end() )
				count[r->m_size] = 0;

			if ( total.find(r->m_size) == total.end() )
				total[r->m_size] = 0;

			count[r->m_size]++;
			total[r->m_size]++;
		}

		DumpNamedBlockMap( count, "used" );
	}

	if (0)
	{
		DumpNamedBlockMap( total, "total" );

		// try clumping them up to see what the wastage verses entry count would be like
		AllocCounter rounded;

		uint32_t wastage = 0;
		uint32_t maxdouble = 8192;	// over this we dont double any more 1k
		uint32_t coarsebuckets = 8;
		uint32_t maxcoarse = maxdouble*coarsebuckets;	// over this we dont add multiples of 8k to bucket them

		for (AllocCounter::iterator it = total.begin(); it != total.end(); ++it)
		{
			if ( it->first < maxdouble )
			{
				// buckets double in size
				uint32_t roundSize = Util::NextPow2( it->first );

				if ( rounded.find(roundSize) == rounded.end() )
					rounded[roundSize] = 0;

				rounded[roundSize] += it->second;
				wastage += (roundSize - it->first) * it->second;
			}
			else if ( it->first <= maxcoarse )
			{
				// 8k wide buckets
				uint32_t bucketSize = maxdouble;

				for (uint i = 1; i <= coarsebuckets; i++)
				{
					if (it->first <= maxdouble*i)
					{
						bucketSize = maxdouble*i;
						break;
					}
				}

				if ( rounded.find(bucketSize) == rounded.end() )
					rounded[bucketSize] = 0;

				rounded[bucketSize] += it->second;
				wastage += (bucketSize - it->first) * it->second;
			}
			else
			{
				// round to nearest 1k 
				uint32_t roundSize = ROUND_POW2( it->first, 1024 );

				if ( rounded.find(roundSize) == rounded.end() )
					rounded[roundSize] = 0;

				rounded[roundSize] += it->second;
				wastage += (roundSize - it->first) * it->second;
			}
		}

		ntPrintf( Debug::DCU_RESOURCES, "\nICE VRAM block spread old: %d\n", total.size() );
		ntPrintf( Debug::DCU_RESOURCES, "ICE VRAM block spread new: %d wasted: %d\n", rounded.size(), wastage );
		ntPrintf( Debug::DCU_RESOURCES, "-------------------------------------\n" );

		DumpNamedBlockMap( rounded, "estimate" );
	}
}

void OutOfVRAMCallback()
{
	ntPrintf( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
	ntPrintf( "!!!!!!!!!    OUT OF VRAM, check resource.log for details.   !!!!!!!!!!!!\n" );
	ntPrintf( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
	
	DumpICEAllocatorState();
	AreaResourceDB::Get().DumpDatabase( 0xffffffff, false );
}

void GcAllocFailureCallbackFunc( uint32_t sizeInBytes, uint32_t alignement, Gc::MemoryContext context )
{
	if (Gc::kVideoMemory == context)
	{
		OutOfVRAMCallback();
	}
	else
	{
		ntError_p( 0, ("ICE failed to allocate HOST MEMORY of size (%x%d)\n", sizeInBytes ) );
	}
}

#endif

//--------------------------------------------------
//!
//!	AreaResourceDB::ctor
//!
//--------------------------------------------------
AreaResourceDB::AreaResourceDB()
{
	// register our manifest bind functions
	NinjaLua::LuaObject globals = CLuaGlobal::Get().State().GetGlobals();
	globals.Register( "Area_RegisterResource_Clump",	Area_RegisterResource_Clump );
	globals.Register( "Area_RegisterResource_Texture",	Area_RegisterResource_Texture );
	globals.Register( "Area_RegisterResource_Anim",		Area_RegisterResource_Anim );
	globals.Register( "Area_RegisterResource_BSAnim",	Area_RegisterResource_BSAnim );
	globals.Register( "Area_RegisterResource_AnimContainer",	Area_RegisterResource_AnimContainer );
	globals.Register( "Area_RegisterResource_BSAnimContainer",	Area_RegisterResource_BSAnimContainer );

	ResetDatabase();

#ifdef PLATFORM_PS3
	// ------------------------------------------------------------------------------
	// Initialise the async' loader.
	// ------------------------------------------------------------------------------
	Loader::Initialise();
	Loader::RegisterResourceHandler( Loader::LRT_AREA_RESOURCE, &AsycLoadCallback );
	Loader::RegisterPreProcessMessage( Loader::LRT_AREA_RESOURCE, &NotifyLoadingCallback );
	Loader::AddUserRequestList( &m_loadRequests );
#endif

#ifdef ADD_VRAM_CALLBACK
	GcKernel::RegisterAllocFailureCallback( &GcAllocFailureCallbackFunc );
#endif
}

//--------------------------------------------------
//!
//!	AreaResourceDB::dtor
//!
//--------------------------------------------------
AreaResourceDB::~AreaResourceDB()
{
#ifdef PLATFORM_PS3
	// ------------------------------------------------------------------------------
	// Delete the async' loader.
	// ------------------------------------------------------------------------------
	Loader::Destroy();
#endif

	if (m_eInitPhase == INITIALISED)
		UnloadDatabase();
}

//--------------------------------------------------
//!
//!	AreaResourceDB::ResetDatabase
//! Empty our database
//!
//--------------------------------------------------
void	AreaResourceDB::ResetDatabase()
{
	ResMap::iterator it = m_resourceDB.begin();
	while ( it != m_resourceDB.end() )
	{
		NT_DELETE( it->second );
		it = m_resourceDB.erase( it );
	}

	m_clumps.clear();
	m_textures.clear();
	m_anims.clear();
	m_misc.clear();
	m_loadRequests.clear();
	m_toBeConfirmed.clear();

	m_eInitPhase = UNINITIALISED;
	m_bAsyncSubmitRequired = false;
	m_totalTexVRAM = 0;
	m_totalClumpVRAM = 0;
	m_totalAnimRAM = 0;
	m_debugArea = -1;
}

//--------------------------------------------------
//!
//!	AreaResourceDB::PreLevelLoadInit
//! Searches for manifests corresponding to lists
//! of resources required for each area.
//! The manifest files are lua files that call
//! static bind functions.
//!
//--------------------------------------------------
void	AreaResourceDB::PreLevelLoadInit( const char* pLevelName )
{
	LOAD_PROFILE( AreaResourceDB_PreLevelLoadInit )

	ntError_p( m_eInitPhase == UNINITIALISED, ("The resource database is already initialised") );

	// install manifests
	//----------------------------------------------
	m_eInitPhase = INSTALLING_MANIFESTS;

	// global resources first
	m_installArea = -1;

	{
		LOAD_PROFILE( ProcessArmFiles )

	char fileName[ MAX_PATH ];
	snprintf( fileName, MAX_PATH, "levels/%s_global.arm", pLevelName );	
	CLuaGlobal::Get().InstallOptionalFile( fileName );

	// area specific resources next
	for ( m_installArea = 1; m_installArea <= AreaSystem::NUM_AREAS; m_installArea++ )
	{
		snprintf( fileName, MAX_PATH, "levels/%s_area%d.arm", pLevelName, m_installArea );	
		CLuaGlobal::Get().InstallOptionalFile( fileName );
	}
	}

	m_eInitPhase = INSTALLING_LEVEL;

#ifdef LOG_RESOURCE_LOADING
	int32_t texStart = m_totalTexVRAM;
	int32_t clumpStart = m_totalClumpVRAM;
	int32_t animStart = m_totalAnimRAM;
#endif

	// load all of our globally required textures sychronously
	//----------------------------------------------
	{
		LOAD_PROFILE( LoadTextures )
	for ( ResList::iterator it = m_textures.begin(); it != m_textures.end(); ++it )
	{
		if ((*it)->GetAreas() == 0xffffffff)
			(*it)->Request( AreaResource::LOAD_SYNC );
	}
	}

	// now we load all of our clump headers, performing a complete load
	// if the clump is a global resource
	//----------------------------------------------
	{
		LOAD_PROFILE( LoadClumps )
	for ( ResList::iterator it = m_clumps.begin(); it != m_clumps.end(); ++it )
		(*it)->Request( AreaResource::CLUMP_LOAD_HEADER );
	}

	// load all of our globally required anims, again, syncronously
	//----------------------------------------------------------------------
	{
		LOAD_PROFILE( LoadAnims )
	for ( ResList::iterator it = m_anims.begin(); it != m_anims.end(); ++it )
	{
		if ((*it)->GetAreas() == 0xffffffff)
			(*it)->Request( AreaResource::LOAD_SYNC );
	}
	}

#ifdef LOG_RESOURCE_LOADING
	int32_t texTotal = m_totalTexVRAM - texStart;
	int32_t clumpTotal = m_totalClumpVRAM - clumpStart;
	int32_t animTotal = m_totalAnimRAM - animStart;
	ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
	ntPrintf( Debug::DCU_RESOURCES, "- Area global: loaded %.2f Mb of textures.\n", _R(texTotal)/(1024.f*1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "- Area global: loaded %.2f Mb of clump data.\n", _R(clumpTotal)/(1024.f*1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "- Area global: loaded %.2f Mb of anim data.\n", _R(animTotal)/(1024.f*1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
#endif
}

//--------------------------------------------------
//!
//!	AreaResourceDB::PostLevelLoadInit
//! Installs resources that may rely on serialised
//! data.
//!
//--------------------------------------------------
void	AreaResourceDB::PostLevelLoadInit()
{
	ntError_p( m_eInitPhase == INSTALLING_LEVEL, ("The resource database is already initialised") );

	// now do everything else. these are typically controll objects that
	// dont do any specific loading themselves, but fix up objects
	//----------------------------------------------------------------------
	for ( ResList::iterator it = m_misc.begin(); it != m_misc.end(); ++it )
	{
		if ((*it)->GetAreas() == 0xffffffff)
			(*it)->Request( AreaResource::LOAD_SYNC );
	}

	m_eInitPhase = INITIALISED;
}

//--------------------------------------------------
//!
//!	AreaResourceDB::UnloadDatabase
//!
//--------------------------------------------------
void	AreaResourceDB::UnloadDatabase()
{
	ntError_p( m_eInitPhase == INITIALISED, ("The resource database must be initialised") );

	// unload misc items
	for ( ResList::iterator it = m_misc.begin(); it != m_misc.end(); ++it )
		(*it)->Request( AreaResource::UNLOAD_SYNC );

	// unload clumps
	for ( ResList::iterator it = m_clumps.begin(); it != m_clumps.end(); ++it )
		(*it)->Request( AreaResource::CLUMP_UNLOAD_HEADER );

	// unload textures
	for ( ResList::iterator it = m_textures.begin(); it != m_textures.end(); ++it )
		(*it)->Request( AreaResource::UNLOAD_SYNC );

	// unload anims
	for ( ResList::iterator it = m_anims.begin(); it != m_anims.end(); ++it )
		(*it)->Request( AreaResource::UNLOAD_SYNC );

	// make sure we have unloaded everything we should have
	ntError_p( m_totalClumpVRAM == 0, ("We have leaked clump VRAM") );
	ntError_p( m_totalTexVRAM == 0, ("We have leaked disk texture VRAM") );
	ntError_p( m_totalAnimRAM == 0, ("We have leaked animation RAM") );
	
	// now clean out our entries.
	ResetDatabase();
}

//--------------------------------------------------
//!
//!	AreaResourceDB::AutoAddAreaResource
//! Add an new entry in our resource database
//!
//--------------------------------------------------
void AreaResourceDB::AutoAddAreaResource( const char* pName, AreaResource::AR_TYPE type )
{
	ntAssert_p( pName, ("Must provide a valid resource name here"));
	ntAssert_p( m_eInitPhase == INSTALLING_MANIFESTS, ("Only valid to call this function within InstallManifests()"));
	
	if (m_installArea == -1)
	{
		AddAreaResource( pName, type, 0xffffffff );
	}
	else
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(m_installArea), ("Must have a valid area number here"));
		AddAreaResource( pName, type, (1 << GetAreaIndex(m_installArea)) );
	}
}

//--------------------------------------------------
//!
//!	AreaResourceDB::AddAreaResource
//! insert a new entry into the database, or adjust
//! an existing one.
//!
//--------------------------------------------------
uint32_t	AreaResourceDB::AddAreaResource( const char* pRawName, AreaResource::AR_TYPE type, uint32_t areas )
{
	ntAssert_p( pRawName, ("Must have a valid resource name here"));
	
	// make sure we have a consistent naming style for resource entries
	char pFullResourcePath[MAX_PATH];
	if (type == AreaResource::CLUMP)
	{
		CClumpLoader::MakePlatformClumpName( pRawName, pFullResourcePath );
	}
	else if (type == AreaResource::TEXTURE)
	{
		TextureManager::MakePlatformTextureName( pRawName, pFullResourcePath );
	}
	else if (type == AreaResource::ANIM)
	{
		CAnimLoader::MakePlatformAnimName( pRawName, pFullResourcePath );
	}
	else if (type == AreaResource::BSANIM)
	{
		BSAnimManager::MakePlatformName( pRawName, pFullResourcePath );
	}
	else if (type == AreaResource::ANIM_CONTAINER)
	{
		strcpy( pFullResourcePath, pRawName );
	}
	else if (type == AreaResource::BSANIM_CONTAINER)
	{
		strcpy( pFullResourcePath, pRawName );
	}
	else if (type == AreaResource::SPEEDGRASS || type == AreaResource::SPEEDTREE)
	{
		strcpy( pFullResourcePath, pRawName );
	}
	else if (type == AreaResource::WATER )
	{
		strcpy( pFullResourcePath, pRawName );
	}


	else
	{
		ntError(0);
		TransformName( pRawName, pFullResourcePath );
	}

	uint32_t pathHash = CHashedString( pFullResourcePath ).GetValue();
	AreaResource* pEntry = GetEntry( pathHash );

	if (pEntry == 0)
	{
		// doesnt exist, add a new entry
		pEntry = NT_NEW AreaResource( pFullResourcePath, pathHash, type, areas );
		m_resourceDB[ pathHash ] = pEntry;

		// sort into various sub lists
		switch (type)
		{
		case AreaResource::CLUMP:			
			m_clumps.push_back(pEntry);		
			break;

		case AreaResource::TEXTURE:	
			m_textures.push_back(pEntry);	
			break;	
		
		case AreaResource::ANIM:
		case AreaResource::BSANIM:			
			m_anims.push_back(pEntry);
			break;	
		
		case AreaResource::ANIM_CONTAINER:	
		case AreaResource::BSANIM_CONTAINER:
		case AreaResource::SPEEDTREE:
		case AreaResource::SPEEDGRASS:
		case AreaResource::WATER:
			m_misc.push_back(pEntry);
			break;

		default:
			ntAssert(0);
			break;
		}
	}
	else
	{
		// make sure this is really the same resource and something hasnt gone wrong
		ntAssert_p( strcmp( pEntry->GetFullPath(), pFullResourcePath ) == 0, ("Resource %s clashes with new resource %s. (pathHash = %d)", pEntry->GetFullPath(), pFullResourcePath, pathHash ) );
		ntAssert_p( pEntry->GetType() == type, ("Resource type differes from original type for resource %s\n", pFullResourcePath ) );

		// mark as required for this sector
		pEntry->SetAreas( pEntry->GetAreas() | areas );
	}

	return pathHash;
}

//--------------------------------------------------
//!
//!	AreaResourceDB::Update
//! Update, handles async load / unload requests
//!
//--------------------------------------------------
void AreaResourceDB::Update()
{
#	ifdef PLATFORM_PS3
	{
		//
		// 	Run through our list of deferred areas and for each one, check if
		// 	the corresponding WAD has been loaded yet, if it has then issue
		// 	an ASync load request of that area - it should never fail due to
		// 	a non-existant WAD as that's what we've just checked.
		//
		// 	We don't check whether we're using WADs here as if we aren't the
		// 	deferred list will always be empty anyway.
		//
		for (	DeferredAreaContainer::iterator it = m_DeferredAreas.begin();
				it != m_DeferredAreas.end(); )
		{
			int32_t area_number = *it;
			ntAssert_p( AreaSystem::IsValidAreaNumber( area_number ), ("Invalid area number found in deferred list.") );

			if ( ArchiveManager::Get().IsAreaWadLoaded( area_number ) )
			{
				it = m_DeferredAreas.erase( it );

				Area_LoadAsync( area_number );
			}
			else
			{
				++it;
			}
		}
	}
#	endif

	if (m_bAsyncSubmitRequired)
	{
		SubmitResourceRequests();
		m_bAsyncSubmitRequired = false;

#ifdef PLATFORM_PS3
		// insert our final load request call back
		m_loadRequests.push_back( Loader::Request(0,0,Loader::LRT_AREA_RESOURCE, &LoadFinishedCallback) );
#endif

#ifndef _RELEASE
		for (int i = 0; i < AreaSystem::NUM_AREAS; i++)
		{
			// this one was requested, so count all in the pending list for this sector
			if ( m_loadCount[i] == 0 )
			{
				for (	ResList::iterator it = m_toBeConfirmed.begin();
						it != m_toBeConfirmed.end(); ++it )
				{
					if ((*it)->GetAreas() & (1<<i))
						m_loadCount[i]++;
				}
			}
		}
#endif
	}

#ifdef PLATFORM_PS3

	// -----------------------------------------------------------------------------
	// Update the async loader.
	// ------------------------------------------------------------------------------
	Loader::Update();

#else

	// on the PC side we fake the load list processing.
	if (m_loadRequests.empty() == false)
	{
		while (m_loadRequests.empty() == false)
		{
			// process each request in turn
			AreaResource* pNext = m_loadRequests.front();
			pNext->MarkLoadProceeding();
			m_loadRequests.pop_front();
			
			Util::SetToPlatformResources();

			// open the data file if it exists.
			uint8_t *pFileData = NULL;
			uint32_t fileSize = 0;
			if (File::Exists( pNext->GetFullPath() ))
			{
				File dataFile;
				LoadFile( pNext->GetFullPath(), File::FT_READ | File::FT_BINARY, dataFile, &pFileData );
				fileSize = dataFile.GetFileSize();
			}

			AsycLoadCallback(pNext->GetFullPath(),pNext,(void *)pFileData,fileSize);

			// delete file data if it exists
			if (pFileData)
			{
				NT_DELETE_ARRAY( pFileData );
			}

			Util::SetToNeutralResources();
		}

		// print out summary
		DumpAreaSysRam();
	}

#endif

	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F1 ) )
	{
		uint32_t iAreaMask = 0;

		iAreaMask |= (AreaManager::Get().GetCurrActiveArea() != AreaSystem::INVALID_AREA) ? 
			(1<<(AreaManager::Get().GetCurrActiveArea() - 1)) : 0;

		iAreaMask |= (AreaManager::Get().GetLastActiveArea() != AreaSystem::INVALID_AREA) ? 
			(1<<(AreaManager::Get().GetLastActiveArea() - 1)) : 0;

		iAreaMask |= (AreaManager::Get().GetNextActiveArea() != AreaSystem::INVALID_AREA) ? 
			(1<<(AreaManager::Get().GetNextActiveArea() - 1)) : 0;

		DumpDatabase( iAreaMask, true );
	}

	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F1, KEYM_CTRL ) )
	{
		DumpDatabase( 0xffffffff, false );
	}

#ifdef ADD_VRAM_CALLBACK
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F1, KEYM_SHIFT ) )
	{
		DumpICEAllocatorState();
	}
#endif
}

//--------------------------------------------------
//!
//!	AreaResourceDB::SendAreaRequest
//! Send resource requests to all within this area
//!
//--------------------------------------------------
void AreaResourceDB::SendAreaRequest( uint32_t areaMask, AreaResource::AR_REQUEST req )
{
	switch (req)
	{
	case AreaResource::LOAD_ASYNC:
		{
#ifdef LOG_RESOURCE_LOADING
			ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
			ntPrintf( Debug::DCU_RESOURCES, "- Area %d: Async load requested\n", DebugGetLastAreaLoaded() );
			ntPrintf( Debug::DCU_RESOURCES, "- Area texture VRAM was %.2f Mb.\n", GetAreaSysTexRamMB() );
			ntPrintf( Debug::DCU_RESOURCES, "- Area clump VRAM was %.2f Mb.\n", GetAreaSysClumpRamMB() );
			ntPrintf( Debug::DCU_RESOURCES, "- Area anim RAM was %.2f Mb.\n", GetAreaSysAnimRamMB() );
			ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
#endif

			for (ResMap::iterator it = m_resourceDB.begin(); it!= m_resourceDB.end(); ++it)
			{
				if (it->second->GetAreas() & areaMask)
					it->second->Request( req );
			}
		}
		break;

	case AreaResource::UNLOAD_ASYNC:
		{
#ifdef LOG_RESOURCE_LOADING
			ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
			ntPrintf( Debug::DCU_RESOURCES, "- Area %d: Async unload requested\n", DebugGetLastAreaLoaded() );
			ntPrintf( Debug::DCU_RESOURCES, "- Area texture VRAM was %.2f Mb.\n", GetAreaSysTexRamMB() );
			ntPrintf( Debug::DCU_RESOURCES, "- Area clump VRAM was %.2f Mb.\n", GetAreaSysClumpRamMB() );
			ntPrintf( Debug::DCU_RESOURCES, "- Area anim RAM was %.2f Mb.\n", GetAreaSysAnimRamMB() );
			ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
#endif

			for (ResMap::iterator it = m_resourceDB.begin(); it!= m_resourceDB.end(); ++it)
			{
				if (it->second->GetAreas() & areaMask)
					it->second->Request( req );
			}
		}
		break;

	case AreaResource::LOAD_SYNC:
		{
#ifdef LOG_RESOURCE_LOADING
			int32_t texStart = m_totalTexVRAM;
			int32_t clumpStart = m_totalClumpVRAM;
			int32_t animStart = m_totalAnimRAM;
#endif

			// textures first, then clumps
			for (ResList::iterator it = m_textures.begin(); it!= m_textures.end(); ++it)
			{
				if (((*it)->GetAreas() & areaMask) && ((*it)->GetStatus() != AreaResource::LOADED))
					(*it)->Request( req );
			}
			for (ResList::iterator it = m_clumps.begin(); it!= m_clumps.end(); ++it)
			{
				if (((*it)->GetAreas() & areaMask) && ((*it)->GetStatus() != AreaResource::LOADED))
					(*it)->Request( req );
			}
			// then anims
			for (ResList::iterator it = m_anims.begin(); it!= m_anims.end(); ++it)
			{
				if (((*it)->GetAreas() & areaMask) && ((*it)->GetStatus() != AreaResource::LOADED))
					(*it)->Request( req );
			}
			// then everything else
			for (ResList::iterator it = m_misc.begin(); it!= m_misc.end(); ++it)
			{
				if (((*it)->GetAreas() & areaMask) && ((*it)->GetStatus() != AreaResource::LOADED))
					(*it)->Request( req );
			}

#ifdef LOG_RESOURCE_LOADING
			int32_t clumpTotal = m_totalClumpVRAM - clumpStart;
			int32_t texTotal = m_totalTexVRAM - texStart;
			int32_t animTotal = m_totalAnimRAM - animStart;
			ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
			ntPrintf( Debug::DCU_RESOURCES, "- Area %d: loaded %.2f Mb of texture data.\n", DebugGetLastAreaLoaded(), _R(texTotal)/(1024.f*1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "- Area %d: loaded %.2f Mb of clump data.\n",  DebugGetLastAreaLoaded(), _R(clumpTotal)/(1024.f*1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "- Area %d: loaded %.2f Mb of anim data.\n",  DebugGetLastAreaLoaded(), _R(animTotal)/(1024.f*1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
#endif
		}
		break;

	case AreaResource::UNLOAD_SYNC:
		{
#ifdef LOG_RESOURCE_LOADING
			int32_t texStart = m_totalTexVRAM;
			int32_t clumpStart = m_totalClumpVRAM;
			int32_t animStart = m_totalAnimRAM;
#endif

			// misc
			for (ResList::iterator it = m_misc.begin(); it!= m_misc.end(); ++it)
			{
				if ((*it)->GetAreas() & areaMask)
					(*it)->Request( req );
			}
			// clumps, then textures
			for (ResList::iterator it = m_clumps.begin(); it!= m_clumps.end(); ++it)
			{
				if ((*it)->GetAreas() & areaMask)
					(*it)->Request( req );
			}
			for (ResList::iterator it = m_textures.begin(); it!= m_textures.end(); ++it)
			{
				if ((*it)->GetAreas() & areaMask)
					(*it)->Request( req );
			}
			// then anims
			for (ResList::iterator it = m_anims.begin(); it!= m_anims.end(); ++it)
			{
				if ((*it)->GetAreas() & areaMask)
					(*it)->Request( req );
			}

#ifdef LOG_RESOURCE_LOADING
			int32_t clumpTotal = clumpStart - m_totalClumpVRAM;
			int32_t texTotal = texStart - m_totalTexVRAM;
			int32_t animTotal = animStart - m_totalAnimRAM;
			ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
			ntPrintf( Debug::DCU_RESOURCES, "- Area %d: unloaded %.2f Mb of texture data.\n", DebugGetLastAreaLoaded(), _R(texTotal)/(1024.f*1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "- Area %d: unloaded %.2f Mb of clump data.\n",  DebugGetLastAreaLoaded(), _R(clumpTotal)/(1024.f*1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "- Area %d: unloaded %.2f Mb of anim data.\n",  DebugGetLastAreaLoaded(), _R(animTotal)/(1024.f*1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
#endif
		}
		break;

	default:
		ntError_p(0,("Invalid request"));
		break;
	}
}

//--------------------------------------------------
//!
//!	AreaResourceDB::SubmitResourceRequests
//! We have recently had a bunch of async requests.
//! concatenate them into one set of commands to process.
//!
//--------------------------------------------------
void AreaResourceDB::SubmitResourceRequests()
{
	// UNLOAD
	// ---------------------------------------------------------------

	// misc allows anims to be unloaded (as containers keep ref counts)
	for (ResList::iterator it = m_misc.begin(); it!= m_misc.end(); ++it)
	{
		if (((*it)->RequiresSubmit() && (*it)->GetRefCount() == 0))
			(*it)->SubmitAsync();
	}

	// vram resources next to allow VRAM allocator to collapse maximum free space	
	for (ResList::iterator it = m_clumps.begin(); it!= m_clumps.end(); ++it)
	{
		if (((*it)->RequiresSubmit() && (*it)->GetRefCount() == 0))
			(*it)->SubmitAsync();
	}

	for (ResList::iterator it = m_textures.begin(); it!= m_textures.end(); ++it)
	{
		if (((*it)->RequiresSubmit() && (*it)->GetRefCount() == 0))
			(*it)->SubmitAsync();
	}
	
	// main memory allocations last
	for (ResList::iterator it = m_anims.begin(); it!= m_anims.end(); ++it)
	{
		if (((*it)->RequiresSubmit() && (*it)->GetRefCount() == 0))
			(*it)->SubmitAsync();
	}

	// LOAD
	// ---------------------------------------------------------------

	// we subimit texture requsests first, then clumps, so theyre 
	// processed in the correct order for load
	for (ResList::iterator it = m_textures.begin(); it!= m_textures.end(); ++it)
	{
		if ((*it)->RequiresSubmit())
			(*it)->SubmitAsync();
	}

	for (ResList::iterator it = m_clumps.begin(); it!= m_clumps.end(); ++it)
	{
		if ((*it)->RequiresSubmit())
			(*it)->SubmitAsync();
	}

	// anims next
	for (ResList::iterator it = m_anims.begin(); it!= m_anims.end(); ++it)
	{
		if ((*it)->RequiresSubmit())
			(*it)->SubmitAsync();
	}

	// last things to be submitted are misc items such as anim containers
	// as they may rely on previously loaded items
	for (ResList::iterator it = m_misc.begin(); it!= m_misc.end(); ++it)
	{
		if ((*it)->RequiresSubmit())
			(*it)->SubmitAsync();
	}
}

//--------------------------------------------------
//!
//!	AreaResourceDB::RequestAsyncLoad
//! Called to request the async load of a resource
//!
//--------------------------------------------------
void AreaResourceDB::RequestAsyncLoad( uint32_t resID )
{
	AreaResource* pResource = GetEntry( resID );
	ntAssert_p( pResource, ("Resource not present in the area resource database") );

#ifdef _DEBUG
	ntAssert_p( DebugIsLoadRequested(resID) == false, ("Already loading resource %s\n.", pResource->GetFullPath() ) );
	for (ResList::iterator it = m_toBeConfirmed.begin(); it!= m_toBeConfirmed.end(); ++it)
	{
		ntAssert_p( *it != pResource, ("Already loading resource %s\n.", pResource->GetFullPath() ) );
	}
#endif

	pResource->MarkLoadRequested();
	m_toBeConfirmed.push_back( pResource );

#ifdef PLATFORM_PS3
	// is our real load request list
	m_loadRequests.push_back( Loader::Request( pResource->GetFullPath(), pResource, Loader::LRT_AREA_RESOURCE ) );
#else
	// simple list
	m_loadRequests.push_back( pResource );
#endif
}

//--------------------------------------------------
//!
//!	AreaResourceDB::CancelAsyncLoad
//! Called to cancel the async load of a resource
//!
//--------------------------------------------------
void AreaResourceDB::CancelAsyncLoad( uint32_t resID )
{
	AreaResource* pResource = GetEntry( resID );
	ntAssert_p( DebugIsLoadRequested(resID) == true, ("Must be loading resource %s\n.", pResource->GetFullPath() ) );

#ifdef PLATFORM_PS3
	bool bFound = false;
	for ( Loader::RequestList::iterator it = m_loadRequests.begin(); it != m_loadRequests.end(); ++it )
	{
		AreaResource* pReqResource = (AreaResource*)it->m_UserData;
		if (pReqResource == pResource)
		{
			m_loadRequests.erase( it );
			bFound = true;
			break;
		}
	}
	ntAssert( bFound );
#else
	ResList::iterator it = ntstd::find( m_loadRequests.begin(), m_loadRequests.end(), pResource );
	ntAssert( it != m_loadRequests.end() );
	m_loadRequests.erase( it );
#endif

	ResList::iterator it2 = ntstd::find( m_toBeConfirmed.begin(), m_toBeConfirmed.end(), pResource );
	ntAssert( it2 != m_toBeConfirmed.end() );
	m_toBeConfirmed.erase( it2 );

	pResource->MarkLoadRequestCanceled();
}

#ifndef _RELEASE
//--------------------------------------------------
//!
//!	Debug functions for testing status of loads
//!
//--------------------------------------------------
bool AreaResourceDB::DebugIsLoadRequested( uint32_t resID )
{
	AreaResource* pResource = GetEntry( resID );
	ntAssert_p( pResource, ("Resource not present in the area resource database") );

	if ( pResource->GetStatus() != AreaResource::LOAD_REQUESTED )
		return false;

#ifdef PLATFORM_PS3
	for ( Loader::RequestList::iterator it = m_loadRequests.begin(); it != m_loadRequests.end(); ++it )
	{
		AreaResource* pReqResource = (AreaResource*)it->m_UserData;
#else
	for (ResList::iterator it = m_loadRequests.begin(); it!= m_loadRequests.end(); ++it)
	{
		AreaResource* pReqResource = *it;

#endif
		if (pReqResource == pResource)
		{
			for (ResList::iterator it2 = m_toBeConfirmed.begin(); it2 != m_toBeConfirmed.end(); ++it2)
			{
				if (*it2 == pResource)
					return true;
			}
			return false;
		}
	}
	return false;
}

bool AreaResourceDB::DebugIsLoadProceeding( uint32_t resID )
{
	AreaResource* pResource = GetEntry( resID );
	ntAssert_p( pResource, ("Resource not present in the area resource database") );

	if ( pResource->GetStatus() != AreaResource::LOADING )
		return false;

	for (ResList::iterator it = m_toBeConfirmed.begin(); it!= m_toBeConfirmed.end(); ++it)
	{
		if (*it == pResource)
		{
			return true;
		}
	}
	return false;
}
#endif

//--------------------------------------------------
//!
//!	AreaResourceDB::LoadFinished
//! Check status of to be confirmed list to make
//! sure all our loads are finished
//!
//--------------------------------------------------
bool AreaResourceDB::LoadFinished( uint32_t areaMask )
{
#ifdef _DEBUG
	// some sanity checking
	for (ResMap::iterator it = m_resourceDB.begin(); it!= m_resourceDB.end(); ++it)
	{
		if (it->second->GetAreas() & areaMask)
		{
			ntAssert_p( it->second->GetStatus() != AreaResource::UNLOADED, ("This resource is not even loading") );
		}
	}
#endif

	// when resources are loaded asynchronously, they are removed from this list
	for (ResList::iterator it = m_toBeConfirmed.begin(); it!= m_toBeConfirmed.end(); ++it)
	{
		if ((*it)->GetAreas() & areaMask)
			return false;
	}

	return true;
}

//--------------------------------------------------
//!
//!	AreaResourceDB::Area_LoadAsync
//! 
//!
//--------------------------------------------------
void AreaResourceDB::Area_LoadAsync( int32_t iAreaNumber )
{
#	ifdef PLATFORM_PS3
	{
		// We required the check for whether we're using WADs or not
		// because if we aren't using WADs then IsAreaWadLoaded will
		// always return false, the area will be deferred but no WADs
		// will actually be loaded so we'll loop forever waiting for
		// them!
		if ( g_ShellOptions->m_bUsingWADs && !ArchiveManager::Get().IsAreaWadLoaded( iAreaNumber ) )
		{
			m_DeferredAreas.push_back( iAreaNumber );
		}
	}
#	endif

#ifndef _RELEASE
	m_loadCount[ GetAreaIndex(iAreaNumber) ] = 0;
#endif

	m_debugArea = iAreaNumber;
	ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
	SendAreaRequest( (1 << GetAreaIndex(iAreaNumber)), AreaResource::LOAD_ASYNC );
	m_bAsyncSubmitRequired = true;
}


//--------------------------------------------------
//!
//!	AreaResourceDB::DumpDatabase
//! dump the area resource database to a log file
//!
//--------------------------------------------------
uint32_t DumpResList( const AreaResourceDB::ResList& list )
{
	uint32_t total = 0;

	for ( AreaResourceDB::ResList::const_iterator it = list.begin(); it != list.end(); ++it )
	{
		ntPrintf( Debug::DCU_RESOURCES, "#  %.4f Mb    %s\n", _R((*it)->m_resourceSize) / (1024.f * 1024.f), (*it)->GetFullPath() );
		total += (*it)->m_resourceSize;
	}

	return total;
}

void DumpResListString( const AreaResourceDB::ResList& list, const char* pString )
{
	for ( AreaResourceDB::ResList::const_iterator it = list.begin(); it != list.end(); ++it )
	{
		ntPrintf( Debug::DCU_RESOURCES, "#  %s %s\n", pString, (*it)->GetFullPath() );
	}
}

void BuildGlobalList( const AreaResourceDB::ResList& source, AreaResourceDB::ResList& result )
{
	result.clear();
	for ( AreaResourceDB::ResList::const_iterator it = source.begin(); it != source.end(); ++it )
	{
		if ((*it)->GetAreas() == 0xffffffff)
		{
			ntError_p( (*it)->GetStatus() == AreaResource::LOADED, ("Global resources must be loaded already") );
			result.push_back( *it );
		}		
	}
}

void BuildSectorList( const AreaResourceDB::ResList& source, AreaResourceDB::ResList& result, uint32_t areaMask, AreaResource::AR_STATUS status )
{
	result.clear();
	for ( AreaResourceDB::ResList::const_iterator it = source.begin(); it != source.end(); ++it )
	{
		// we ignore anything in global here
		if (((*it)->GetAreas() != 0xffffffff) && ((*it)->GetAreas() & areaMask) && ((*it)->GetStatus() == status))
		{
			result.push_back( *it );
		}		
	}
}

class AreaResourceSizeComparator
{
public:
	bool operator()( const AreaResource* pFirst, const AreaResource* pSecond ) const
	{
		return ( pFirst->m_resourceSize > pSecond->m_resourceSize );
	}
};

void SortResourceList( AreaResourceDB::ResList& list )
{
	ntstd::Vector<AreaResource*> toSort( list.begin(), list.end() );
	ntstd::sort( toSort.begin(), toSort.end(), AreaResourceSizeComparator() );
	list.clear();

	for ( ntstd::Vector<AreaResource*>::iterator it = toSort.begin(); it != toSort.end(); ++it )
	{
		list.push_back( *it );
	}
}

void AreaResourceDB::DumpDatabase( uint32_t areaMask, bool bSortBySize )
{
	ResList temp;
	uint32_t total;

	ntPrintf( Debug::DCU_RESOURCES, "\n\n" );
	ntPrintf( Debug::DCU_RESOURCES, "#################################################################\n" );
	ntPrintf( Debug::DCU_RESOURCES, "#                         AREA STATUS DUMP\n");
	ntPrintf( Debug::DCU_RESOURCES, "#\n");
	ntPrintf( Debug::DCU_RESOURCES, "#                          TEXTURES: %.4f Mb\n", _R(m_totalTexVRAM) / (1024.f * 1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "#                          CLUMPS:   %.4f Mb\n", _R(m_totalClumpVRAM) / (1024.f * 1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "#                          ANIMS:    %.4f Mb\n", _R(m_totalAnimRAM) / (1024.f * 1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "#\n");
	ntPrintf( Debug::DCU_RESOURCES, "#################################################################\n" );
	ntPrintf( Debug::DCU_RESOURCES, "#                         AREA GLOBAL CLUMPS\n#\n");

	BuildGlobalList(m_clumps,temp);

	if (bSortBySize)
		SortResourceList(temp);

	total = DumpResList( temp );

	ntPrintf( Debug::DCU_RESOURCES, "#\n");
	ntPrintf( Debug::DCU_RESOURCES, "#                          TOTAL GLOBAL CLUMPS: %.4f Mb\n", _R(total) / (1024.f * 1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "#################################################################\n" );
	ntPrintf( Debug::DCU_RESOURCES, "#                         AREA GLOBAL TEXTURES\n#\n");

	BuildGlobalList(m_textures,temp);

	if (bSortBySize)
		SortResourceList(temp);

	total = DumpResList( temp );

	ntPrintf( Debug::DCU_RESOURCES, "#\n");
	ntPrintf( Debug::DCU_RESOURCES, "#                          TOTAL GLOBAL TEXTURES: %.4f Mb\n", _R(total) / (1024.f * 1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "#################################################################\n" );
	ntPrintf( Debug::DCU_RESOURCES, "#                         AREA GLOBAL ANIMATIONS\n#\n");

	BuildGlobalList(m_anims,temp);

	if (bSortBySize)
		SortResourceList(temp);

	total = DumpResList( temp );

	ntPrintf( Debug::DCU_RESOURCES, "#\n");
	ntPrintf( Debug::DCU_RESOURCES, "#                          TOTAL GLOBAL ANIMS: %.4f Mb\n", _R(total) / (1024.f * 1024.f) );
	ntPrintf( Debug::DCU_RESOURCES, "#################################################################\n" );

	for ( int32_t i = 0; i < 32; i++ )
	{
		if ((1<<i) & areaMask)
		{
			ntPrintf( Debug::DCU_RESOURCES, "#                         AREA %d CLUMPS\n#\n", i+1 );

			BuildSectorList( m_clumps, temp, 1<<i, AreaResource::LOADED );

			if (bSortBySize)
				SortResourceList(temp);

			total = DumpResList( temp );

			BuildSectorList( m_clumps, temp, 1<<i, AreaResource::LOADING );
			DumpResListString( temp, "LOADING" );

			BuildSectorList( m_clumps, temp, 1<<i, AreaResource::LOAD_REQUESTED );
			DumpResListString( temp, "REQUESTED" );

			ntPrintf( Debug::DCU_RESOURCES, "#\n");
			ntPrintf( Debug::DCU_RESOURCES, "#                          TOTAL AREA %d CLUMPS: %.4f Mb\n", i+1, _R(total) / (1024.f * 1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "#################################################################\n" );
			ntPrintf( Debug::DCU_RESOURCES, "#                         AREA %d TEXTURES\n#\n", i+1 );

			BuildSectorList( m_textures, temp, 1<<i, AreaResource::LOADED );

			if (bSortBySize)
				SortResourceList(temp);

			total = DumpResList( temp );

			BuildSectorList( m_textures, temp, 1<<i, AreaResource::LOADING );
			DumpResListString( temp, "LOADING" );

			BuildSectorList( m_textures, temp, 1<<i, AreaResource::LOAD_REQUESTED );
			DumpResListString( temp, "REQUESTED" );

			ntPrintf( Debug::DCU_RESOURCES, "#\n");
			ntPrintf( Debug::DCU_RESOURCES, "#                          TOTAL AREA %d TEXTURES: %.4f Mb\n", i+1, _R(total) / (1024.f * 1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "#################################################################\n" );
			ntPrintf( Debug::DCU_RESOURCES, "#                         AREA %d ANIMS\n#\n", i+1 );

			BuildSectorList( m_anims, temp, 1<<i, AreaResource::LOADED );

			if (bSortBySize)
				SortResourceList(temp);

			total = DumpResList( temp );

			BuildSectorList( m_anims, temp, 1<<i, AreaResource::LOADING );
			DumpResListString( temp, "LOADING" );

			BuildSectorList( m_anims, temp, 1<<i, AreaResource::LOAD_REQUESTED );
			DumpResListString( temp, "REQUESTED" );

			ntPrintf( Debug::DCU_RESOURCES, "#\n");
			ntPrintf( Debug::DCU_RESOURCES, "#                          TOTAL AREA %d ANIMS: %.4f Mb\n", i+1, _R(total) / (1024.f * 1024.f) );
			ntPrintf( Debug::DCU_RESOURCES, "#################################################################\n" );
		}
	}
}

//--------------------------------------------------
//!
//!	AreaResourceDB::Area_LoadFraction
//! Debug stuff for reporting load progress.
//!
//--------------------------------------------------
#ifndef _RELEASE
float AreaResourceDB::Area_LoadFraction( int32_t iAreaNumber )
{
	ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );

	if (m_loadCount[ GetAreaIndex(iAreaNumber) ] == 0)
		return 1.f;

	uint32_t iMask = (1 << GetAreaIndex(iAreaNumber));
	int32_t iCount = 0;
	
	for (	ResList::iterator it = m_toBeConfirmed.begin();
			it != m_toBeConfirmed.end(); ++it )
	{
		if (iMask & (*it)->GetAreas())
			iCount++;
	}

	return 1.f - (_R(iCount)/_R(m_loadCount[ GetAreaIndex(iAreaNumber) ]));
}

//--------------------------------------------------
//!
//!	AreaResourceDB::GetSizeFor
//! Query database for total size of loaded resources
//! mustMatch = submask of areas we want to match
//! areasToTest	= area bits to consider at all
//!
//--------------------------------------------------
int32_t AreaResourceDB::GetSizeFor( uint32_t mustMatch, uint32_t areasToTest, AreaResource::AR_TYPE type )
{
	int32_t iResult = 0;
	switch ( type )
	{
	case AreaResource::CLUMP:
		{
			for (ResList::iterator it = m_clumps.begin(); it!= m_clumps.end(); ++it)
			{
				if	(
					(((*it)->GetAreas() & areasToTest) == mustMatch) &&
					((*it)->GetStatus() == AreaResource::LOADED)
					)
				{
					iResult += (*it)->m_resourceSize;
				}
			}
		}
		break;

	case AreaResource::TEXTURE:
		{
			for (ResList::iterator it = m_textures.begin(); it!= m_textures.end(); ++it)
			{
				if	(
					(((*it)->GetAreas() & areasToTest) == mustMatch) &&
					((*it)->GetStatus() == AreaResource::LOADED)
					)
				{
					iResult += (*it)->m_resourceSize;
				}
			}
		}
		break;

	case AreaResource::ANIM:
		{
			for (ResList::iterator it = m_anims.begin(); it!= m_anims.end(); ++it)
			{
				if	(
					(((*it)->GetAreas() & areasToTest) == mustMatch) &&
					((*it)->GetStatus() == AreaResource::LOADED)
					)
				{
					iResult += (*it)->m_resourceSize;
				}
			}
		}
		break;

	case AreaResource::ANIM_CONTAINER:
	case AreaResource::BSANIM_CONTAINER:
	case AreaResource::SPEEDTREE:
	case AreaResource::WATER:
		break;

	default:
		ntAssert(0);
		break;
	}

	return iResult;
}

#endif




//--------------------------------------------------
//!
//!	ClumpMemTracker
//!
//--------------------------------------------------
#ifdef TRACK_AREA_RAM
ClumpMemTracker::ClumpMemTracker()
{
	m_startValueVB = Renderer::ms_iVBAllocs;
	m_startValueIB = Renderer::ms_iIBAllocs;
}

int ClumpMemTracker::GetDelta() const
{
	return (Renderer::ms_iVBAllocs - m_startValueVB) + (Renderer::ms_iIBAllocs - m_startValueIB);
}
#endif
