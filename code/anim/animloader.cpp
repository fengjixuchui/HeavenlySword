/***************************************************************************************************
*
*	Anim loading
*
*	CHANGES
*
*	8/4/2004		Ben		Created
*
***************************************************************************************************/

#include "anim/animloader.h"
#include "anim/animation.h"
#include "core/file.h"

#include "anim/AnimationHeader.h"
#include "anim/PriorityCache.h"

/***************************************************************************************************
*
*	Prototypes.
*
***************************************************************************************************/
struct AnimLoaderHelpers
{
	static void	Unload ( const CAnimationHeader* anim_data )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_ANIMATION, ( int8_t* )anim_data );
	}

	enum LogType
	{
		MISSING = 0,
		OLD_FORMAT,

		NUM_LOGTYPES
	};
	static void	LogAnimation( const char *filename, LogType log_type );
	static void	GenerateError( const char *filename, LogType log_type, char* pResult );
};

namespace AnimHelpers
{
	void LogAnimationString( const char *str );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::CAnimLoader()
*
*	DESCRIPTION		Class Ctor.
*
***************************************************************************************************/
CAnimLoader::CAnimLoader()
{
	// Initialise our AnimData namespace.
	AnimData::Create();

	// Create the priority cache here as well...
	NT_NEW PriorityCache;

	// install our error animation
	m_pErrorAnim = 0;
	m_pErrorAnim = LoadAnim_Neutral( "entities/characters/hero/animations/hero_default_blank_anim.anim" );
	ntError_p( m_pErrorAnim, ("MUST have error animation already") );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::~CAnimLoader()
*
*	DESCRIPTION		Class Dtor. Unloads all animations and removed them from the member map.
*
***************************************************************************************************/
CAnimLoader::~CAnimLoader()
{
	Clear();

	// delete our error animation
	AnimLoaderHelpers::Unload( m_pErrorAnim );

	// Destroy the priority cache.
	PriorityCache::Kill();

	// Destroy the AnimData...
	AnimData::Destroy();
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::Clear()
*
*	DESCRIPTION		Clear the animation loader caches
*
***************************************************************************************************/
void	CAnimLoader::Clear()
{
	for ( AnimCache::iterator it = m_cache.begin(); it != m_cache.end();  )
	{
		if (IsErrorAnim( it->second.m_pAnim ) == false)
			AnimLoaderHelpers::Unload( it->second.m_pAnim );
		it = m_cache.erase( it );
	}

	m_cacheMap.clear();
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::MakePlatformAnimName( const char* pNeutralName, char* pPlatformName )
*
*	DESCRIPTION		Generates plaform specific name for an animation
*
***************************************************************************************************/
void	CAnimLoader::MakePlatformAnimName( const char* pNeutralName, char* pPlatformName )
{
#ifndef _RELEASE
	// check for some very special case errors
	ntError_p( pNeutralName[0] != '\\', ("Animation path %s has a leading backslash, please remove.", pNeutralName) );
	ntError_p( pNeutralName[0] != '/', ("Animation path %s has a leading forwardslash, please remove.", pNeutralName) );
#endif

	// appends file system root and content directory
	Util::GetFiosFilePath_Platform( pNeutralName, pPlatformName );

#ifdef PLATFORM_PS3
	// we change our file extension to the correct one
	strcpy( strstr( pPlatformName, ".anim" ), ".anim_ps3" );
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::LoadAnim_Neutral( const char *pNeutralName )
*
*	DESCRIPTION		Looks up pcFilename in the map, if found, returns the existing animation
*					header, if not found, loads the animation header then returns it.
*
***************************************************************************************************/
const CAnimationHeader *CAnimLoader::LoadAnim_Neutral( const char *pNeutralName )
{
	char pPlatformName[MAX_PATH];
	MakePlatformAnimName( pNeutralName, pPlatformName );
	return LoadAnim_Platform( pPlatformName );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::GetAnim( const char *pcFilename )
*
*	DESCRIPTION		Looks up pcFilename in the map, if found, returns the existing animation
*					header, if not found, loads the animation header then returns it.
*
***************************************************************************************************/
const CAnimationHeader *CAnimLoader::LoadAnim_Platform( const char *pPlatformName )
{
	// Check anim cache first.
	CHashedString name(pPlatformName);
	RefCountedAnimAdaptor& animAdaptor = m_cache[ name.GetValue() ];

	if( animAdaptor.m_refCount != 0 )
	{
		animAdaptor.AddRef();
		return animAdaptor.m_pAnim;
	}

#ifdef PLATFORM_PC
	Util::SetToPlatformResources();
#endif

	// nope proceed with load
	uint8_t *pReadResult = NULL;
	uint32_t fileSize = 0;
	if ( File::Exists( pPlatformName ) )
	{
		File dataFile;
		LoadFile_Chunk( pPlatformName, File::FT_READ | File::FT_BINARY, Mem::MC_ANIMATION, dataFile, &pReadResult );
		fileSize = dataFile.GetFileSize();
	}

	const CAnimationHeader* pResult = LoadAnim_FromData( name.GetValue(), (void *)pReadResult, fileSize, pPlatformName );

	// free temp buffers and return result
	if (pReadResult)
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_ANIMATION, pReadResult );
	}
	
#ifdef PLATFORM_PC
	Util::SetToNeutralResources();
#endif

	// note: LoadAnim_FromData with have put the header into the cache
	return pResult;
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::LoadAnim_FromData( const char *pcFilename )
*
*	DESCRIPTION		converts the block of memory into an animation header and stores in the cache
*					if its not already present
*
***************************************************************************************************/
const CAnimationHeader *CAnimLoader::LoadAnim_FromData( uint32_t cacheKey, void* pFileData, uint32_t fileSize, const char* pDebugTag )
{
	// Check anim cache first.
	RefCountedAnimAdaptor& animAdaptor = m_cache[ cacheKey ];

	if( animAdaptor.m_refCount != 0 )
	{
		animAdaptor.AddRef();
		return animAdaptor.m_pAnim;
	}

	if ( pFileData == 0 )
	{
		AnimLoaderHelpers::LogAnimation( pDebugTag, AnimLoaderHelpers::MISSING );
		one_time_assert_p( 0xdeadbeb0, false, ("Animation %s is missing.\nYou will not be notified of additional missing anims - check missing_anims.txt\n", pDebugTag) );
		animAdaptor.m_pAnim = m_pErrorAnim;
	}
	else
	{
		const FpAnimClipDef *pobAnimationHeader = reinterpret_cast< FpAnimClipDef * >( pFileData );
		if ( pobAnimationHeader->GetTag() != FpAnimClipDef::GetClassTag() )
		{
			AnimLoaderHelpers::LogAnimation( pDebugTag, AnimLoaderHelpers::OLD_FORMAT );
			one_time_assert_p( 0xdeadbeb1, false, ("Animation %s is in an old format and cannot be loaded.\nYou will not be notified of additional invalid anims - check missing_anims.txt\n", pDebugTag) );
			animAdaptor.m_pAnim = m_pErrorAnim;
		}
		else
		{
			

/*
			if ( pobAnimationHeader->GetLocomotionInfo() == NULL )
			{
				ntPrintf( "ANIM NOT LOCOMOTING: %s\n", pDebugTag );
			}
*/


			int8_t *pReadResult = NT_NEW_ARRAY_CHUNK( Mem::MC_ANIMATION ) int8_t[ fileSize ];
			ntError_p( FwIsAligned( pReadResult, 16 ), ("We're expecting a 16 byte aligned allocation here") );
			
			NT_MEMCPY( pReadResult, pFileData, fileSize );
			animAdaptor.m_pAnim = reinterpret_cast< CAnimationHeader * >( pReadResult );
		}
	}

	ntError_p( animAdaptor.m_pAnim, ("Unable to load animation header %s\n", pDebugTag ) );

	animAdaptor.AddRef();
	m_cacheMap[ animAdaptor.m_pAnim ] = cacheKey;
	return animAdaptor.m_pAnim;
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::UnloadAnim( CAnimationHeader* pobAnimData )
*
*	DESCRIPTION		Will release the passed animation header - currently a no-op.
*
***************************************************************************************************/
bool CAnimLoader::UnloadAnim( const CAnimationHeader *pobAnimData )
{
	ntAssert_p( pobAnimData, ("Must supply a valid animation header here\n" ) );

	CacheMap::iterator it = m_cacheMap.find( pobAnimData );
	ntAssert_p( it != m_cacheMap.end(), ("Animation header not found in anim loader cache") );
	
	AnimCache::iterator it2 = m_cache.find( it->second );
	ntAssert_p( it2 != m_cache.end(), ("Animation caches out of synch. Eek!") );
	
	// if the release will cause an unload clear the maps
	if( it2->second.m_refCount == 1 )
	{
		if ( IsErrorAnim(it2->second.m_pAnim) == false )
		{
			it2->second.Release();
			m_cacheMap.erase( it );
		}

		m_cache.erase( it2 );
		return true;
	}
	else
	{
		it2->second.Release();
		return false;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::UnloadAnim_Key( uint32_t cacheKey )
*
*	DESCRIPTION		find this anim in the cache and release it
*
***************************************************************************************************/
bool CAnimLoader::UnloadAnim_Key( uint32_t cacheKey )
{
	AnimCache::iterator it2 = m_cache.find( cacheKey );
	ntAssert_p( it2 != m_cache.end(), ("Animation header not found in anim loader cache") );
	
	CacheMap::iterator it = m_cacheMap.find( it2->second.m_pAnim );
	ntAssert_p( it != m_cacheMap.end(), ("Animation caches out of synch. Eek!") );
	
	// if the release will cause an unload clear the maps
	if( it2->second.m_refCount == 1 )
	{
		if ( IsErrorAnim(it2->second.m_pAnim) == false )
		{
			it2->second.Release();
			m_cacheMap.erase( it );
		}

		m_cache.erase( it2 );
		return true;
	}
	else
	{
		it2->second.Release();
		return false;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::RefCountedAnimAdaptor
*
*	DESCRIPTION		Interal anim cache release of this resource
*
***************************************************************************************************/
void CAnimLoader::RefCountedAnimAdaptor::Release() 
{
	--m_refCount;
	if( m_refCount == 0 )
	{
		AnimLoaderHelpers::Unload( m_pAnim );
		m_pAnim = 0;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::Loaded_Neutral
*
*	DESCRIPTION		Test for presence of this animation
*
***************************************************************************************************/
bool CAnimLoader::Loaded_Neutral( const char* pNeutralName ) const
{
	char pPlatformName[MAX_PATH];
	MakePlatformAnimName( pNeutralName, pPlatformName );
	return Loaded_Platform( pPlatformName );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::Loaded_Platform
*
*	DESCRIPTION		Test for presence of this animation
*
***************************************************************************************************/
bool CAnimLoader::Loaded_Platform( const char *pPlatformName ) const
{
	CHashedString nameHash(pPlatformName);
	return Loaded_Cache( nameHash.GetValue() );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::Loaded_Cache
*
*	DESCRIPTION		Test for presence of this animation
*
***************************************************************************************************/
bool CAnimLoader::Loaded_Cache( uint32_t cacheKey ) const
{
	AnimCache::const_iterator it = m_cache.find( cacheKey );
	if (it != m_cache.end())
		return (it->second.m_refCount != 0);
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		AnimLoaderHelpers::GenerateError
*
*	NOTES			Spit out the specific error for this string
*
***************************************************************************************************/
void AnimLoaderHelpers::GenerateError( const char *filename, LogType log_type, char* pResult )
{
	switch ( log_type )
	{
		case MISSING:
		{
			sprintf( pResult, "MISSING ANIM:    %s",filename );
			return;
		}

		case OLD_FORMAT:
		{
			sprintf( pResult, "OLD FORMAT ANIM: %s",filename );
			return;
		}

		default:
		{
			sprintf( pResult, "UNKNOWN ERROR:   %s",filename );
			return;
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		AnimLoaderHelpers::LogAnimation
*
*	NOTES			record problems with animation loading
*
***************************************************************************************************/
void AnimLoaderHelpers::LogAnimation( const char *filename, AnimLoaderHelpers::LogType log_type )
{
	char aErrorMSG[ MAX_PATH ];
	GenerateError( filename, log_type, aErrorMSG );

	ntPrintf( Debug::DCU_ANIM, "Anim Error:%s.\n", aErrorMSG );

	AnimHelpers::LogAnimationString( aErrorMSG );
}

void AnimHelpers::LogAnimationString( const char *str )
{
	static bool first_time = true;
	static const char *anim_log_filename = "missing_anims.txt";
	char extended_filename[ 512 ];
	Util::GetFiosFilePath( anim_log_filename, extended_filename );
	int32_t append = File::FT_APPEND;
	if ( first_time )
	{
		append = 0;
		first_time = false;
	}
	File file( extended_filename, File::FT_WRITE | File::FT_TEXT | append );
	if ( !file.IsValid() )
	{
		ntError_p( false, ("Cannot write to missing_anims log file with filename %s.", extended_filename) );
		return;
	}

	file.Write( str, strlen( str ) );

	char new_line = '\n';
	file.Write( &new_line, 1 );
}







