//--------------------------------------------------
//!
//!	\file arearesource.cpp
//!	Entry within the resource database
//!
//--------------------------------------------------

#include "area/arearesource.h"
#include "area/arearesourcedb.h"
#include "gfx/clump.h"
#include "gfx/texturemanager.h"
#include "anim/animloader.h"
#include "game/entityanimcontainer.h"
#include "blendshapes/blendshapes_managers.h"
#include "blendshapes/anim/BSAnimContainer.h"

#ifdef PLATFORM_PS3
#include "speedtree/speedtreemanager_ps3.h"
#include "speedtree/speedgrass_ps3.h"
#endif

#include "water/watermanager.h"

//--------------------------------------------------
//!
//!	AreaResource::ctor
//!
//--------------------------------------------------
AreaResource::AreaResource(	const char* pFullPath, uint32_t	pathHash,
							AR_TYPE		type, uint32_t	areas ) :
	m_pathHash( pathHash ),
	m_type( type ),
	m_status( UNLOADED ),
	m_areas( areas ),
	m_refcount( 0 ),
	m_requiresAsync( false )
{
	ntAssert_p( pFullPath, ("Must pass in a valid resource name here") );
	ntAssert_p( CHashedString(pFullPath).GetValue() == pathHash, ("Path (%s) and hash (0x%x) do not match\n.", pFullPath, pathHash) );

	m_pFullResourcePath = NT_NEW_ARRAY char[strlen(pFullPath)+1];
	strcpy( m_pFullResourcePath, pFullPath );
}

//--------------------------------------------------
//!
//!	AreaResource::dtor
//!
//--------------------------------------------------
AreaResource::~AreaResource()
{
	NT_DELETE_ARRAY( m_pFullResourcePath );
}

//--------------------------------------------------
//!
//!	AreaResource::Load
//! internal load method
//!
//--------------------------------------------------
void	AreaResource::Load( void* pFileData, uint32_t fileSize )
{
	UNUSED(fileSize); // this is unused at the moment, we may need to pass this down to our ctors at some point

	ntAssert_p( GetStatus() != LOADED, ("Can only request an load of resource %s if its not fully loaded\n", GetFullPath() ) );
	ntAssert_p( m_refcount >= 0, ("Ref counting of resource %s is screwed up\n", GetFullPath() ) );

	#ifdef TRACK_AREA_RAM
	m_resourceSize = 0;
	#endif

	switch (GetType())
	{
	case CLUMP:
		{
			#ifndef PLATFORM_PC // we already have the resource size on PC, we're just simulating the resource load
			CLUMP_TRACKER();
			#endif

			CClumpLoader::Get().FixupClumpMeshData( GetPathHash(), pFileData );
			
			#ifdef TRACK_AREA_RAM
			#ifndef PLATFORM_PC // we already have the resource size on PC, we're just simulating the resource load
			m_resourceSize = mem.GetDelta();
			#endif
			AreaResourceDB::Get().m_totalClumpVRAM += m_resourceSize;
			#endif
		}
		break;

	case TEXTURE:
		{
			Texture::Ptr tex = TextureManager::Get().LoadTexture_FromData( GetPathHash(), pFileData, fileSize, GetFullPath() );

			#ifdef TRACK_AREA_RAM
			m_resourceSize = tex->m_iDiskSize;
			AreaResourceDB::Get().m_totalTexVRAM += m_resourceSize;
			#endif
		}
		break;

	case ANIM:
		{
			CAnimLoader::Get().LoadAnim_FromData( GetPathHash(), pFileData, fileSize, GetFullPath() );

			#ifdef TRACK_AREA_RAM
			m_resourceSize = fileSize;
			AreaResourceDB::Get().m_totalAnimRAM += fileSize;
			#endif
		}
		break;

	case BSANIM:
		{
			BSAnimManager::Get().Load_FromData( GetPathHash(), pFileData, fileSize, GetFullPath() );

			#ifdef TRACK_AREA_RAM
			m_resourceSize = fileSize;
			AreaResourceDB::Get().m_totalAnimRAM += fileSize;
			#endif
		}
		break;

	case ANIM_CONTAINER:
		{
			EntityAnimContainer* pContainer = AnimContainerManager::Get().GetAnimContainer( GetPathHash() );
			ntError_p( pContainer, ("Cannot find animation container %s\n", GetFullPath() ) );
			pContainer->InstallAnimations();
		}	
		break;

	case BSANIM_CONTAINER:
		{
			CHashedString containerName( GetFullPath() );
			BSAnimContainerManager::Get().InstallAnimsForContainer( containerName );
		}	
		break;

	case SPEEDTREE:
		{
#ifdef PLATFORM_PS3
			SpeedTreeManager::Get().OnAreaLoad( CHashedString( GetPathHash() ) );
#endif
		}
		break;

	case SPEEDGRASS:
		{
#ifdef PLATFORM_PS3
			SpeedGrass::OnAreaLoad( CHashedString( GetPathHash() ), pFileData, fileSize );
#endif
		}
		break;

	case WATER:
		{
			WaterManager::Get().CreateInstanceAreaResources( CHashedString(GetPathHash()) );
		}
		break;
		
	default:
		{
			ntError_p(0,("unhandled resource type"));
		}
		break;
	}

	#ifdef LOG_RESOURCE_LOADING
	ntPrintf( Debug::DCU_AREA_LOAD, "Area %d: load of resource %s. (%.2fMb)\n", m_requestArea, GetFullPath(), _R(m_resourceSize)/(1024.f*1024.f) );
	#endif

	m_status = LOADED;
}

//--------------------------------------------------
//!
//!	AreaResource::Unload
//! internal unload method
//!
//--------------------------------------------------
void	AreaResource::Unload()
{
	ntAssert_p( GetStatus() == LOADED, ("Can only request an load of resource %s if its fully loaded\n", GetFullPath() ) );
	ntAssert_p( m_refcount == 0, ("Ref counting of resource %s is screwed up\n", GetFullPath() ) );

	bool bSucceded = false;

	switch (GetType())
	{
	case CLUMP:
		{
			#ifndef PLATFORM_PC
			CLUMP_TRACKER();
			#endif

			if (CClumpLoader::Get().FreeupClumpMeshData( GetPathHash() ))
			{
				#ifdef TRACK_AREA_RAM
				#ifndef PLATFORM_PC
				ntAssert_p( m_resourceSize == abs(mem.GetDelta()), ("Resource size (%d) does not match tracked delta (%d)", m_resourceSize, abs(mem.GetDelta()) ) );
				#endif
				AreaResourceDB::Get().m_totalClumpVRAM -= m_resourceSize;
				#endif

				bSucceded = true;
			}
		}
		break;

	case TEXTURE:
		{
			if (TextureManager::Get().UnloadTexture_Platform( GetFullPath() ))
			{
				#ifdef TRACK_AREA_RAM
				AreaResourceDB::Get().m_totalTexVRAM -= m_resourceSize;
				#endif

				bSucceded = true;
			}
		}
		break;

	case ANIM:
		{
			// unfortunately the object database keeps hold of animations till after our release, so we
			// cannot rely on the release returning true here
			CAnimLoader::Get().UnloadAnim_Key( GetPathHash() );

			#ifdef TRACK_AREA_RAM
			AreaResourceDB::Get().m_totalAnimRAM -= m_resourceSize;
			#endif

			bSucceded = true;
		}
		break;

	case BSANIM:
		{
			// unfortunately the object database keeps hold of animations till after our release, so we
			// cannot rely on the release returning true here

			BSAnimManager::Get().Unload_Key( GetPathHash() );

			#ifdef TRACK_AREA_RAM
			AreaResourceDB::Get().m_totalAnimRAM -= m_resourceSize;
			#endif

			bSucceded = true;
		}
		break;

	case ANIM_CONTAINER:
		{
			EntityAnimContainer* pContainer = AnimContainerManager::Get().GetAnimContainer( GetPathHash() );
			ntError_p( pContainer, ("Cannot find animation container %s\n", GetFullPath() ) );
			pContainer->UninstallAnimations();
			bSucceded = true;
		}	
		break;

	case BSANIM_CONTAINER:
		{
			BSAnimContainerManager::Get().RemoveAnimsForContainer( CHashedString( GetFullPath() ) );
			bSucceded = true;
		}	
		break;

	case SPEEDTREE:
		{
#ifdef PLATFORM_PS3
			SpeedTreeManager::Get().OnAreaUnload( CHashedString(GetPathHash()) );
#endif
			bSucceded = true;
		}
		break;

	case SPEEDGRASS:
		{
			bSucceded = true;
#ifdef PLATFORM_PS3
			bSucceded = SpeedGrass::OnAreaUnload( CHashedString( GetPathHash() ) );
#endif
		}
		break;


	case WATER:
		{
			WaterManager::Get().DestroyInstanceAreaResources( CHashedString(GetPathHash()) );
			bSucceded = false;
		}
		break;

	default:
		{
			ntError_p(0,("unhandled resource type"));
		}
		break;
	}

	m_refcount = 0;
	m_status = UNLOADED;

	#ifdef LOG_RESOURCE_LOADING
	if (bSucceded)
		ntPrintf( Debug::DCU_AREA_LOAD, "Area %d: unload of resource %s.\n", m_requestArea, GetFullPath() );
	else
	{		
		ntPrintf( Debug::DCU_AREA_LOAD, "*****************************************************************\n" );
		ntPrintf( Debug::DCU_AREA_LOAD, "* Area %d: FAILED unload of resource %s.\n", m_requestArea, GetFullPath() );
		ntPrintf( Debug::DCU_AREA_LOAD, "*****************************************************************\n" );
	}
	#endif
}

//--------------------------------------------------
//!
//!	AreaResource::Request
//! Request a change in the resources status.
//!
//--------------------------------------------------
void	AreaResource::Request( AR_REQUEST request )
{
	m_requestArea = AreaResourceDB::Get().DebugGetLastAreaLoaded();

	switch (request)
	{
	//--------------------------------------------------
	// install clump header
	//--------------------------------------------------
	case CLUMP_LOAD_HEADER:
		{
			ntAssert( GetType() == CLUMP );
			ntAssert_p( GetStatus() == UNLOADED, ("Can only request a synchronous load of resource %s if its unloaded\n", GetFullPath() ) );
			ntAssert_p( m_refcount == 0, ("Ref counting of resource %s is screwed up\n", GetFullPath() ) );
			
			if (GetAreas() == 0xffffffff)
			{
				// we're a global clump so load it entirely once and have done with it
				CLUMP_TRACKER();
				CClumpLoader::Get().LoadClump_Platform( GetFullPath(), true, false );
				
				m_status = LOADED;
				m_refcount = 1;

				#ifdef TRACK_AREA_RAM
				m_resourceSize = mem.GetDelta();
				AreaResourceDB::Get().m_totalClumpVRAM += m_resourceSize;
				#endif
				#ifdef LOG_RESOURCE_LOADING
				ntPrintf( Debug::DCU_AREA_LOAD, "Area %d: load of resource %s. (%.2fMb)\n", m_requestArea, GetFullPath(), _R(m_resourceSize)/(1024.f*1024.f) );
				#endif
			}
			else
			{
				#if defined (PLATFORM_PC) && defined (TRACK_AREA_RAM)
				// we always allocate here on pc, so we get our resource size now
				CLUMP_TRACKER();
				CClumpLoader::Get().LoadClump_Platform( GetFullPath(), false );
				m_resourceSize = mem.GetDelta();
				#else
				// just load the header
				CClumpLoader::Get().LoadClump_Platform( GetFullPath(), false );
				#endif
			}
		}
		break;

	//--------------------------------------------------
	// uninstall clump header
	//--------------------------------------------------
	case CLUMP_UNLOAD_HEADER:
		{
			ntAssert( GetType() == CLUMP );

			CClumpHeader* pClumpHeader = CClumpLoader::Get().GetClumpFromCache_Key( GetPathHash() );
			ntAssert_p( pClumpHeader, ("Cannot find clump %s in the cache\n", GetFullPath() ) );

			if ((GetAreas() == 0xffffffff) || (m_status == LOADED))
			{
				// we're a global clump so track the vram changes
				CLUMP_TRACKER();
				CClumpLoader::Get().UnloadClump( pClumpHeader );
				
				#ifdef TRACK_AREA_RAM
				ntAssert_p( m_resourceSize == abs(mem.GetDelta()), ("Resource size (%d) does not match tracked delta (%d)", m_resourceSize, abs(mem.GetDelta()) ) );
				AreaResourceDB::Get().m_totalClumpVRAM -= m_resourceSize;
				#endif
				#ifdef LOG_RESOURCE_LOADING
				ntPrintf( Debug::DCU_AREA_LOAD, "Area %d: unload of resource %s. (%.2fMb)\n", m_requestArea, GetFullPath(), _R(m_resourceSize)/(1024.f*1024.f) );
				#endif
			}
			else
			{
				CClumpLoader::Get().UnloadClump( pClumpHeader );
			}

			m_refcount = 0;
			m_status = UNLOADED;
		}
		break;

	//--------------------------------------------------
	// syncronous load of resource
	//--------------------------------------------------
	case LOAD_SYNC:
		{
			ntAssert( m_refcount == 0 );

			// open the data file if it exists.
			#ifdef PLATFORM_PC
			Util::SetToPlatformResources();
			#endif

			char* pFileData = 0;
			size_t fileSize = 0;
			if (File::Exists( GetFullPath() ))
			{
				File dataFile( GetFullPath(), File::FT_READ | File::FT_BINARY );
				ntAssert_p( dataFile.IsValid(), ("Reading from and invalid file\n") );

				fileSize = dataFile.GetFileSize();
				ntAssert_p( fileSize > 0, ("Reading from a zero sized file\n") );

				pFileData = NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[ fileSize ];
				
				size_t readSize = 0;
				readSize = dataFile.Read( pFileData, fileSize );
				ntAssert_p( readSize == fileSize, ("Error reading file\n") );
			}

			Load( pFileData, fileSize );

			// delete file data if it exists
			if (pFileData)
			{
				NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, pFileData );
			}

			#ifdef PLATFORM_PC
			Util::SetToNeutralResources();
			#endif

			m_refcount = 1;
		}
		break;

	//--------------------------------------------------
	// syncronous unload of resource
	//--------------------------------------------------
	case UNLOAD_SYNC:
		if (m_refcount>0)
		{
			--m_refcount;
			if ((m_refcount == 0) && (m_status == LOADED))
				Unload();
		}
		break;

	//--------------------------------------------------
	// asyncronous load of resource
	//--------------------------------------------------
	case LOAD_ASYNC:
		{
			m_refcount++;
			m_requiresAsync = true;
		}
		break;

	//--------------------------------------------------
	// asyncronous unload of resource
	//--------------------------------------------------
	case UNLOAD_ASYNC:
		{
			m_refcount--;
			m_requiresAsync = true;
		}
		break;
	}
}

//--------------------------------------------------
//!
//!	AreaResource::SubmitAsync
//! Process any async requests we've had
//!
//--------------------------------------------------
void AreaResource::SubmitAsync()
{
	ntAssert( m_requiresAsync );
	ntAssert_p( m_refcount >= 0, ("Should never have a negative ref count for resource %s\n", GetFullPath() ) );

	if (m_refcount > 0)
	{
		if (m_status == UNLOADED)
		{
			AreaResourceDB::Get().RequestAsyncLoad( GetPathHash() );
		}
		else if (m_status == LOAD_REQUESTED)
		{
			ntAssert_p( AreaResourceDB::Get().DebugIsLoadRequested( GetPathHash() ) == true, ("This resource must be in the load que already") );
		}
		else if (m_status == LOADING)
		{
			ntAssert_p( AreaResourceDB::Get().DebugIsLoadProceeding( GetPathHash() ) == true, ("This resource must be loading by now") );
		}
	}
	else if (m_refcount == 0)
	{
		if (m_status == LOADED)
		{
			Unload();
		}
		else if (m_status == LOADING)
		{
			ntAssert_p( AreaResourceDB::Get().DebugIsLoadProceeding( GetPathHash() ) == true, ("This resource must be loading by now") );
			// do nothing, call back when loaded will realise this is not wanted
			// by 0 refcount and unload it immediately
		}
		else if (m_status == LOAD_REQUESTED)
		{
			AreaResourceDB::Get().CancelAsyncLoad( GetPathHash() );			
		}
	}

	// have handled async requests
	m_requiresAsync = false;	
}
