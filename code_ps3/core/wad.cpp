//*******************************************************************************
//	
//	Wad.cpp.
//	
//******************************************************************************/

#include <sys/sys_time.h>
#include <cell/cell_fs.h>
#include <sys/paths.h>

#include "core/wad.h"

#include "core/memman.h"
#include "core/timer_ps3.h"
#include "core/compress.h"
#include "core/blurayfile_ps3.h"
#include "core/cellfsfile_ps3.h"
#include "core/fileio_ps3.h"
#include "core/waddef.h"
#include "core/archive.h"

#include "game/shellconfig.h"

#ifndef _MASTER
//#	define WAD_DEBUG_PRINT
#	define TIME_WAD_EXTRACT
//#	define DETAIL_WAD_EXTRACT_TIMING

#	if defined( DETAIL_WAD_EXTRACT_TIMING ) && !defined( TIME_WAD_EXTRACT )
#		define TIME_WAD_EXTRACT
#	endif
#endif

// Private types for install wads.
namespace
{
	struct FilenameAreaPair
	{
		char *	m_Filename;
		int32_t	m_AreaNumber;
	};
	typedef ntstd::List< FilenameAreaPair > WadInstallContainer;
}

namespace Util
{
	void StringToLowerInplace( char * );
}

// Required for direct file-system access through CellFS as FIOS may
// not be setup when we call some of these functions.
extern const char* g_GameDataPath;

//*******************************************************************************
//	File-scope readable only stuff.
//*******************************************************************************
namespace
{
	static bool s_HaveCompleteGUIGameData = false;
	static bool s_HaveCompleteGlobalGameData = false;

	// List of all the level names, NULL terminated.
	static const char *LevelNames[] =
	{
		"purgatory/purgatory",
		"ch0/initialbattle",
		"ch1/fort",
		"ch2/walkways",
		"ch3/temple",
		"ch4/escape",
		"ch5/battlefield",
		"ch6/kingbattle",
		NULL
	};
}

//*******************************************************************************
//	Load a specific WADs contents into the ArchiveManager.
//*******************************************************************************
void Wad::LoadWad( const char *wad_filename, MediaTypes read_media, int32_t area_number/* = -1*/ )
{
	ntError( FileManager::exists( wad_filename, read_media ) );

	ntPrintf( "WAD: Loading WAD %s for area %i, from %s.\n", wad_filename, area_number, read_media == BLU_RAY_MEDIA ? "Blu-Ray" : read_media == DEFAULT_MEDIA ? "Default" : "Sys-Cache" );

	// Create an archive object from the copy...
	if ( area_number == -1 )
	{
		ArchiveManager::Get().AddArchive( wad_filename, read_media );
	}
	else
	{
		ArchiveManager::Get().AddAreaArchive( wad_filename, area_number, read_media );
	}
}

//*******************************************************************************
//	
//*******************************************************************************
static void InstallFile( const char *filename, MediaTypes write_media_type )
{
	ntError_p( write_media_type != BLU_RAY_MEDIA, ("Cannot install a file to blu-ray, it's write only!") );

	// If the file already exists then just skip the copy...
	if ( FileManager::exists( filename, write_media_type ) )
	{
		return;
	}

	int32_t read_fd = -1;
	int32_t write_fd = -1;

	// Open up the file to read from. The blu-ray media device is always
	// where we want to look because if we aren't running from blu-ray it
	// defaults to app-home.
	read_fd = FileManager::open( filename, O_RDONLY, BLU_RAY_MEDIA );
	if ( read_fd == -1 )
	{
		ntError_p( false, ("Unable to install file %s.\nCannot open the source file for reading.", filename) );
		return;
	}

	// Create the directory structure...
	FileManager::createDirectories( filename, write_media_type );

	write_fd = FileManager::open( filename, O_WRONLY | O_CREAT, write_media_type );
	if ( write_fd == -1 )
	{
		FileManager::close( read_fd );
		ntError_p( false, ("Unable to install file %s.\nCannot open the destination file for writing.", filename) );
		return;
	}

	static const uint32_t FileBufferSize = 1024 * 1024;
	uint8_t *file_buffer = NT_NEW uint8_t[ FileBufferSize ];

	// Find the size of the file.
	off_t file_size = FileManager::getFileSize( read_fd );
	ntError_p( file_size > 0, ("Cannot copy a zero sized file.") );
	if ( file_size <= 0 )
	{
		return;
	}

	// Work out how many blocks of read/write pairs we have to do.
	uint32_t num_chunks = (uint32_t)file_size / FileBufferSize;
	uint32_t remaining_size = (uint32_t)file_size - num_chunks*FileBufferSize;

	for ( uint32_t i=0;i<num_chunks;i++ )
	{
		FileManager::read( read_fd, file_buffer, FileBufferSize );
		FileManager::write( write_fd, file_buffer, FileBufferSize );
	}
	FileManager::read( read_fd, file_buffer, remaining_size );
	FileManager::write( write_fd, file_buffer, remaining_size );

	NT_DELETE_ARRAY( file_buffer );

	FileManager::close( write_fd );
	FileManager::close( read_fd );
}

//*******************************************************************************
//	Verify the WAD and start the copy or extraction processes appropriately.
//*******************************************************************************
static void InstallWADSync( const char *wad_filename, MediaTypes write_media_type )
{
	int32_t fd = -1;

	// Work out where to read the WADs from...
	MediaTypes read_media_type = BLU_RAY_MEDIA;

	fd = FileManager::open( wad_filename, O_RDONLY, read_media_type );
	ntError_p( fd != -1, ("Could not open WAD %s for reading.", wad_filename) );
	if ( fd == -1 )
	{
		return;
	}

	Wad::ArchiveHeader header;

	// Use pread as it doesn't advance the file-offset.
	uint32_t read_size = FileManager::pread( fd, &header, sizeof( Wad::ArchiveHeader ), 0 );
	ntError( read_size == sizeof( Wad::ArchiveHeader ) );
	UNUSED( read_size );

#	ifndef _GOLD_MASTER
		if (	header.m_Tag[ 0 ] != Wad::ArchiveHeader::ClassTag()[ 3 ] ||
				header.m_Tag[ 1 ] != Wad::ArchiveHeader::ClassTag()[ 2 ] ||
				header.m_Tag[ 2 ] != Wad::ArchiveHeader::ClassTag()[ 1 ] ||
				header.m_Tag[ 3 ] != Wad::ArchiveHeader::ClassTag()[ 0 ] )
		{
			ntError_p( false, ("Invalid WAD file %s", wad_filename) );
			return;
		}

#		ifdef WAD_DEBUG_PRINT
			ntPrintf( "HEADER: Tag %s, Version=%i.%02i\n\n", header.m_Tag, header.m_MajorVersion, header.m_MinorVersion );
#		endif

		if ( header.m_MajorVersion != Wad::Version::Major || header.m_MinorVersion != Wad::Version::Minor )
		{
			ntError_p( false, ("Attempting to load a different WAD version than we were compiled with.\nHS SELF Version = %d.%02d.\nWAD Version = %d.%02d.", Wad::Version::Major, Wad::Version::Minor, header.m_MajorVersion, header.m_MinorVersion) );
		}
#	endif

#	ifdef TIME_WAD_EXTRACT
		uint64_t timerFrequency = sys_time_get_timebase_frequency();
		double timer_period = 1.0 / ( double )( timerFrequency );

		int64_t start_time = CTimer::GetHWTimer();
#	endif

	// We always want to copy if the destination doesn't already have this file.
	// BUT... we never copy unless g_ShellOptions->m_bUsingHDD is true in the config.
	bool should_copy_wad = false;

	// If we want to "write" to blu-ray then this effectivley means
	// we want to just load from blu-ray and not install.
	if ( write_media_type != BLU_RAY_MEDIA )
	{
		if ( g_ShellOptions->m_bUsingHDD )
		{
			if ( !FileManager::exists( wad_filename, write_media_type ) )
			{
				should_copy_wad = true;
			}
			else
			{
				// The file already exists - open it up and compare the MD5 digests
				// to determine whether we have a different version that needs copying.
				int32_t existing_file = FileManager::open( wad_filename, O_RDONLY, write_media_type );
				ntError_p( existing_file >= 0, ("Why can't we open %s for read? We know it exists...", wad_filename) );

				Wad::ArchiveHeader existing_header;
				ssize_t bytes_read = FileManager::pread( existing_file, &existing_header, sizeof( Wad::ArchiveHeader ), 0 );
				if ( bytes_read < (ssize_t)sizeof( Wad::ArchiveHeader ) )
				{
					should_copy_wad = true;
				}
				else
				{
					if (	existing_header.m_MD5Digest[ 0 ] == header.m_MD5Digest[ 0 ] &&
							existing_header.m_MD5Digest[ 1 ] == header.m_MD5Digest[ 1 ] )
					{
						// Digest is the same for both files - don't bother to copy, it's already there.
						// UNLESS... it's the wrong size, in which case we may have been interrupted
						// when installing this WAD last time and only got part-way through the copy.
						if ( header.m_WadFileLength == (uint32_t)FileManager::getFileSize( existing_file ) )
						{
							should_copy_wad = false;
						}
						else
						{
							should_copy_wad = true;
						}
					}
					else
					{
						// We seem to have a different version so overwrite the existing one.
						should_copy_wad = true;
					}
				}

				FileManager::close( existing_file );
			}
		}
		else
		{
			// This looks wrong but it's actually correct...
			// We know we're not running from HDD and that we aren't installing the
			// wads, therefore the only thing 'write_media_type' is used for is to
			// determine where the final location of the wads will be - i.e. where
			// they'll always be read from throughout the game - which in this case
			// is the blu-ray media device. Note that this works if we haven't got
			// a blu-ray disc in the dev-kit as well because then the blu-ray device
			// is automatically mapped to app-home.
			write_media_type = BLU_RAY_MEDIA;
		}
	}

	if ( should_copy_wad )
	{
		// Copy over the file in chunks of a fixed size.
		static const uint32_t BufferSize = 1024*1024;
		uint8_t *buffer = NT_NEW uint8_t[ BufferSize ];

		uint32_t file_size = 0;
		{
			// Setup the directory structure, if required.
			FileManager::createDirectories( wad_filename, write_media_type );

			int32_t write_file = FileManager::open( wad_filename, O_WRONLY | O_CREAT, write_media_type );
			if ( write_file < 0 )
			{
				ntError_p( false, ("Unable to open destination to write WAD to. WAD is %s.", wad_filename) );
				return;
			}

			file_size = FileManager::getFileSize( fd );
			uint32_t num_buffer_chunks = file_size / BufferSize;
			uint32_t remaining_size = file_size - num_buffer_chunks*BufferSize;

			for ( uint32_t i=0;i<num_buffer_chunks;i++ )
			{
				if ( FileManager::read( fd, buffer, BufferSize ) != (ssize_t)BufferSize )
				{
					ntError_p( false, ("Unable to read from source WAD %s.", wad_filename) );
					return;
				}

				if ( FileManager::write( write_file, buffer, BufferSize ) != (ssize_t)BufferSize )
				{
					ntError_p( false, ("Unable to write to destination WAD %s.", wad_filename) );
					return;
				}
			}

			if ( FileManager::read( fd, buffer, remaining_size ) != (ssize_t)remaining_size )
			{
				ntError_p( false, ("Unable to read from source WAD %s.", wad_filename) );
				return;
			}

			if ( FileManager::write( write_file, buffer, remaining_size ) != (ssize_t)remaining_size )
			{
				ntError_p( false, ("Unable to write to destination WAD %s.", wad_filename) );
				return;
			}

			FileManager::close( write_file );
		}

		NT_DELETE_ARRAY( buffer );
	}

	FileManager::close( fd );

#	ifdef TIME_WAD_EXTRACT
		int64_t end_time = CTimer::GetHWTimer();
		int64_t time_delta = end_time - start_time;
		float time_taken = (float)time_delta * float( timer_period );

		char temp_str[ 512 ];
		snprintf( temp_str, 512, "WAD: Installed %s in: %fs\n", wad_filename, time_taken );
		Debug::AlwaysOutputString( temp_str );
#	endif
}

//*******************************************************************************
//	
//*******************************************************************************
static bool ParseAreaOrderFile( char *full_level_path, File &area_order, WadInstallContainer &wads_to_install )
{
	static const uint32_t MaxAreaOrderFileSize = 1024;
	char area_order_file[ MaxAreaOrderFileSize ];

	ntError_p( area_order.GetFileSize() <= MaxAreaOrderFileSize, ("Area order file is massive! Can't load - waaaay too big at %i bytes. Needs to be <%i bytes.", area_order.GetFileSize(), MaxAreaOrderFileSize) );
	if ( area_order.GetFileSize() > MaxAreaOrderFileSize )
	{
		return false;
	}

	area_order.Read( area_order_file, area_order.GetFileSize() );

	int32_t end_of_full_level_path = strlen( full_level_path );

	char *curr = &( area_order_file[ 0 ] );
	while ( strncmp( curr, "[END]", 5 ) != 0 )
	{
		// Start of each line will be a number - which we want.
		char *str_number = curr;

		// Skip over the number.
		while ( isdigit( *curr ) )
		{
			curr++;
		}

		// Skip the carridge-return/new-line gubbins, replacing with 0s to terminate the number string before.
		while ( *curr == '\n' || *curr == '\r' )
		{
			*curr++ = '\0';
		}

		uint32_t area_number = atoi( str_number );

		// Form the correct filename from this area-number and check whether it exists.
		char filename_end[ 64 ];
		sprintf( filename_end, "_area%i.wad", area_number );
		strcat( full_level_path, filename_end );

		if ( File::Exists( full_level_path ) )
		{
			char *valid_wad_filename = NT_NEW char[ strlen( full_level_path ) ];
			strcpy( valid_wad_filename, full_level_path );

			FilenameAreaPair fap;
			fap.m_Filename = valid_wad_filename;
			fap.m_AreaNumber = area_number;
			wads_to_install.push_back( fap );
		}
		else
		{
			ntPrintf( "***** The area file %s specified in this level's .areaorder file was not found and will be skipped.\n" );
		}

		full_level_path[ end_of_full_level_path ] = '\0';
	}

	return true;
}

//*******************************************************************************
//	
//*******************************************************************************
static void SearchDirectoryForAreaWads( char *full_level_path, WadInstallContainer &wads_to_install )
{
	int32_t end_of_full_level_path = strlen( full_level_path );
	int32_t area_number = 1;

	static const int32_t MaxAreaNumber = 64;	// MrEd only goes up to 32, so hopefully this should be enough allowing for future expansion.
	while ( area_number < MaxAreaNumber )
	{
		// Append ending to level path...
		char filename_ending[ 64 ];
		sprintf( filename_ending, "_area%i.wad", area_number );
		strcat( full_level_path, filename_ending );

		if ( FileManager::exists( full_level_path, BLU_RAY_MEDIA ) )
		{
			char *valid_wad_filename = NT_NEW char[ strlen( full_level_path ) ];
			strcpy( valid_wad_filename, full_level_path );

			FilenameAreaPair fap;
			fap.m_Filename = valid_wad_filename;
			fap.m_AreaNumber = area_number;
			wads_to_install.push_back( fap );
		}

		// Reset level path and name.
		full_level_path[ end_of_full_level_path ] = '\0';

		area_number++;
	}
}

//*******************************************************************************
//	
//*******************************************************************************
void Wad::InstallGlobalWADs( MediaTypes destination_media )
{
	if ( !g_ShellOptions->m_bUsingWADs )
	{
		return;
	}

	MediaTypes read_media_type = BLU_RAY_MEDIA;

	static const char *GlobalWadDir = "global_wads/";	// z:/hs/global_wads/
	FileEnumerator enumerator( GlobalWadDir, read_media_type );

	while ( enumerator.AdvanceToNextFile() )
	{
		if ( enumerator.IsDirectory() )
		{
			continue;
		}

		const char *filename = enumerator.GetCurrentFilename();
		uint32_t filename_len = enumerator.GetCurrentFilenameLength();

		if (	filename_len > 4 && 
				filename[ filename_len - 3 ] == 'w' &&
				filename[ filename_len - 2 ] == 'a' &&
				filename[ filename_len - 1 ] == 'd' )
		{
			char wad_path[ 1024 ];
			sprintf( wad_path, "%s%s", GlobalWadDir, filename );

			InstallWADSync( wad_path, destination_media );
		}
	}

	if ( destination_media == DEFAULT_MEDIA )
	{
		s_HaveCompleteGUIGameData = true;
	}
}

//*******************************************************************************
//	Loads the global WADs' contents into the ArchiveManager.
//*******************************************************************************
void Wad::LoadGlobalWADs( MediaTypes read_media_type )
{
	if ( !g_ShellOptions->m_bUsingWADs )
	{
		return;
	}

	static const char *GlobalWadDir = "global_wads/";	// z:/hs/global_wads/
	FileEnumerator enumerator( GlobalWadDir, read_media_type );

	while ( enumerator.AdvanceToNextFile() )
	{
		if ( enumerator.IsDirectory() )
		{
			continue;
		}

		const char *filename = enumerator.GetCurrentFilename();
		uint32_t filename_len = enumerator.GetCurrentFilenameLength();

		if (	filename_len > 4 && 
				filename[ filename_len - 3 ] == 'w' &&
				filename[ filename_len - 2 ] == 'a' &&
				filename[ filename_len - 1 ] == 'd' )
		{
			char wad_path[ 1024 ];
			sprintf( wad_path, "%s%s", GlobalWadDir, filename );

			LoadWad( wad_path, read_media_type );
		}
	}
}

//*******************************************************************************
//	
//*******************************************************************************
static void InstallGlobalWADForLevel( const char *level_name )
{
	if ( g_ShellOptions->m_bUsingWADs == false )
	{
		return;
	}

	// Also install the global wad for this level, if it exists.
	char full_level_path[ 1024 ];
	sprintf( full_level_path, "content_neutral/levels/%s_global.wad", level_name );
	Util::StringToLowerInplace( full_level_path );

	MediaTypes read_media_type = BLU_RAY_MEDIA;

	if ( FileManager::exists( full_level_path, read_media_type ) )
	{
		InstallWADSync( full_level_path, DEFAULT_MEDIA );
	}
}

//*******************************************************************************
//	
//*******************************************************************************
void Wad::InstallGlobalWADForLevels()
{
	uint32_t i = 0;
	while ( LevelNames[ i ] != NULL )
	{
		InstallGlobalWADForLevel( LevelNames[ i++ ] );
	}

	s_HaveCompleteGlobalGameData = true;
}

//*******************************************************************************
//	
//*******************************************************************************
void Wad::InstallAndLoadLevelWADs( const char *level_name )
{
	if ( g_ShellOptions->m_bUsingWADs == false )
	{
		return;
	}

	char full_level_path[ 1024 ];
	sprintf( full_level_path, "content_neutral/levels/%s", level_name );
	Util::StringToLowerInplace( full_level_path );

	int32_t end_of_full_level_path = strlen( full_level_path );

	//
	// 	Look for the file <full_level_path>.areaorder - this file has one number
	// 	on each line; each one corresponds to the area with that number. The file
	// 	gives the order in which the areas are loaded.
	//
	// 	If we don't find the .areaorder file then we'll search the level directory
	// 	for filenames of the form <full_level_path>_area<X>.wad where <X> is an
	// 	integer.
	//
	WadInstallContainer wads_to_install;
	{
		strcat( full_level_path, ".areaorder" );
	
		File area_order;
		
		if ( File::Exists( full_level_path ) )
		{
			area_order.Open( full_level_path, File::FT_TEXT | File::FT_READ );
		}

		// Reset the full-level path so we can use it again.
		full_level_path[ end_of_full_level_path ] = '\0';

		if ( area_order.IsValid() )
		{
			if ( !ParseAreaOrderFile( full_level_path, area_order, wads_to_install ) )
			{
				// If parsing failed, fall back to searching the directory.
				SearchDirectoryForAreaWads( full_level_path, wads_to_install );
			}
		}
		else
		{
			SearchDirectoryForAreaWads( full_level_path, wads_to_install );
		}
	}

	//
	//	TODO:	Rearrange list of WADs to reflect the checkpoint/area we are
	//			loading first - it might not be the first one if we've loaded
	//			from a save-game!
	//

	// Load the appropriate <level_name>_global.wad file.
	char level_global_wad[ MAX_PATH ];
	sprintf( level_global_wad, "%s_global.wad", full_level_path );
	LoadWad( level_global_wad, DEFAULT_MEDIA );

	// Install and load the first N area WADs synchronously and leave the rest
	// to install on a background thread.
	static const uint32_t MaxNumWadsToInstall = 2;
	uint32_t num_wads_to_install = MaxNumWadsToInstall > wads_to_install.size() ? wads_to_install.size() : MaxNumWadsToInstall;
	for ( uint32_t i=0;i<num_wads_to_install;i++ )
	{
		char *wad_filename = wads_to_install.front().m_Filename;
		int32_t area_number = wads_to_install.front().m_AreaNumber;
		wads_to_install.pop_front();

		InstallWADSync( wad_filename, SYSCACHE_MEDIA );
		LoadWad( wad_filename, SYSCACHE_MEDIA, area_number );

		NT_DELETE_ARRAY( wad_filename );
	}

	// TODO: Setup background thread to trickle-install the remaining wads.
}

//*******************************************************************************
//	
//*******************************************************************************
void Wad::PostGameDataInstall()
{
	// We also required certain files to be present in specific places on the
	// hard-drive (game-data partition). These are installed here.
	// This list of filenames should be NULL terminated so we know when to stop.
	static const char *RequiredFiles[] =
	{
		"content_ps3/entities/characters/nobody/nobody.clump_ps3",
		"content_ps3/entities/characters/hero/animations/hero_default_blank_anim.anim_ps3",
		"content_ps3/data/lensghost_mono.gtf",
		"content_ps3/data/radialblurmask_colour_alpha.gtf",
		"content_ps3/data/solid_red_colour.gtf",
		NULL,
	};

	uint32_t i = 0;
	while ( RequiredFiles[ i ] != NULL )
	{
		InstallFile( RequiredFiles[ i ], DEFAULT_MEDIA );

		i++;
	}
}

//*******************************************************************************
//	
//*******************************************************************************
bool Wad::HaveCompleteGUIGameData()
{
	if ( !g_ShellOptions->m_bUsingWADs )
	{
		return true;
	}

	return s_HaveCompleteGUIGameData;
}

//*******************************************************************************
//	
//*******************************************************************************
bool Wad::HaveCompleteGlobalGameData()
{
	if ( !g_ShellOptions->m_bUsingWADs )
	{
		return true;
	}

	return s_HaveCompleteGlobalGameData;
}

//*******************************************************************************
//	Util function to enumerate all WADs in a given directory.
//*******************************************************************************
namespace
{
	struct WadData
	{
		CHashedString		m_Filename;
		uint64_t			m_MD5Digest[ 2 ];

		WadData()
		{
			m_MD5Digest[ 0 ] = 0;
			m_MD5Digest[ 1 ] = 0;
		}

		bool operator == ( const WadData &rhs ) const
		{
			return	( m_Filename == rhs.m_Filename ) &&
					( m_MD5Digest[ 0 ] == rhs.m_MD5Digest[ 0 ] ) &&
					( m_MD5Digest[ 1 ] == rhs.m_MD5Digest[ 1 ] );
		}

		bool IsValid() const { return m_MD5Digest[ 0 ] != 0 && m_MD5Digest[ 1 ] != 0; }

		static bool LessThan( const WadData &lhs, const WadData &rhs )
		{
			return lhs.m_Filename < rhs.m_Filename;
		}
	};

	typedef ntstd::Vector< WadData > WadDataList;

	static bool GetMD5FromWAD( const char *wad_filename, MediaTypes media_type, uint64_t &md5_0, uint64_t &md5_1 )
	{
		char full_path[ 1024 ];
		switch ( media_type )
		{
			case BLU_RAY_MEDIA:
				Util::GetFullBluRayFilePath( wad_filename, full_path, false );
				break;

			case DEFAULT_MEDIA:
				Util::GetFullGameDataFilePath( wad_filename, full_path, false );
				break;

			case SYSCACHE_MEDIA:
				Util::GetFullSysCacheFilePath( wad_filename, full_path, false );
				break;

			default:
				ntError_p( false, ("Invalid media-type.") );
				return false;
		}

		CellFsFile wad_file( full_path, File::FT_READ );
		ntError_p( wad_file.IsValid(), ("Could not open WAD %s for reading.", full_path) );
		if ( !wad_file.IsValid() )
		{
			return false;
		}

		// Read in the header.
		Wad::ArchiveHeader header;
		wad_file.Read( &header, sizeof( Wad::ArchiveHeader ) );

		if (	header.m_Tag[ 0 ] != Wad::ArchiveHeader::ClassTag()[ 3 ] ||
				header.m_Tag[ 1 ] != Wad::ArchiveHeader::ClassTag()[ 2 ] ||
				header.m_Tag[ 2 ] != Wad::ArchiveHeader::ClassTag()[ 1 ] ||
				header.m_Tag[ 3 ] != Wad::ArchiveHeader::ClassTag()[ 0 ] )
		{
			ntError_p( false, ("Invalid WAD file %s", wad_filename) );
			return false;
		}

		if ( header.m_MajorVersion != Wad::Version::Major || header.m_MinorVersion != Wad::Version::Minor )
		{
			ntError_p( false, ("Attempting to read a different WAD version than we were compiled with.\nHS SELF Version = %d.%02d.\nWAD Version = %d.%02d.", Wad::Version::Major, Wad::Version::Minor, header.m_MajorVersion, header.m_MinorVersion) );
			return false;
		}

		md5_0 = header.m_MD5Digest[ 0 ];
		md5_1 = header.m_MD5Digest[ 1 ];

		return true;
	}

	// Returns true if successful, false otherwise.
	static bool EnumerateWads( const char *path, MediaTypes media_type, WadDataList &wad_data_list )
	{
		ntError_p( path[ strlen( path ) - 1 ] == '/', ("path given (%s) must end in a /", path) );

		// Enumerate all the files in the given directory.
		FileEnumerator enumerator( path, media_type );

		while ( enumerator.AdvanceToNextFile() )
		{
			// Skip directories and files that don't end in "wad".
			if ( enumerator.IsDirectory() )
				continue;

			const char *filename = enumerator.GetCurrentFilename();
			uint32_t filename_len = enumerator.GetCurrentFilenameLength();
			if (	filename_len < 3 ||
					filename[ filename_len - 3 ] != 'w' ||
					filename[ filename_len - 2 ] != 'a' ||
					filename[ filename_len - 1 ] != 'd' )
			{
				continue;
			}

			// Construct a full path for this wad file.
			char wad_path[ 1024 ];
			sprintf( wad_path, "%s%s", path, filename );

			// Fill in the WadData structure and push it onto the list.
			WadData wad_data;
			wad_data.m_Filename = CHashedString( filename );
			if ( !GetMD5FromWAD( wad_path, media_type, wad_data.m_MD5Digest[ 0 ], wad_data.m_MD5Digest[ 1 ] ) )
			{
				// This isn't a valid WAD file, so we fail.
				user_error_p( false, ("%s is not a valid WAD file!", wad_path) );
				return false;
			}

			wad_data_list.push_back( wad_data );
		}

		// Sort the list based on the hash of the filename.
		ntstd::sort( wad_data_list.begin(), wad_data_list.end(), &WadData::LessThan );

		return true;
	}
}

namespace
{
	static bool CheckWads( const WadDataList &hdd_wads, const WadDataList &bluray_wads )
	{
		// If we have more WADs we want to install than WADs in game-data then we need to install.
		if ( hdd_wads.size() < bluray_wads.size() )
		{
			return false;
		}

		// Check if all our blu-ray wads exist in the game-data.
		WadDataList::const_iterator hdd_wad_it = hdd_wads.begin();

		for (	WadDataList::const_iterator blu_ray_wad_it = bluray_wads.begin();
				blu_ray_wad_it != bluray_wads.end();
				++blu_ray_wad_it )
		{
			// The containers are sorted so we can just skip up the the correct
			// entry using LessThan comparisons.
			while (	hdd_wad_it != hdd_wads.end() &&
					WadData::LessThan( *hdd_wad_it, *blu_ray_wad_it ) )
			{
				++hdd_wad_it;
			}

			// If the entries are still on the same then we have a genuine difference.
			if ( !( *hdd_wad_it == *blu_ray_wad_it ) )
			{
				return false;
			}

			// If the entries' MD5 digests differ then we also have a difference.
			if (	hdd_wad_it->m_MD5Digest[ 0 ] != blu_ray_wad_it->m_MD5Digest[ 0 ] ||
					hdd_wad_it->m_MD5Digest[ 1 ] != blu_ray_wad_it->m_MD5Digest[ 1 ] )
			{
				return false;
			}
		}

		// No differences found so everything must be installed correctly.
		return true;
	}
}

//*******************************************************************************
//	Check GameData directories for status of any previous installs
//*******************************************************************************
void Wad::RecordGameDataUsage()
{
	// For game-data, we first check the global wads.
	static const char *GlobalWadDir = "global_wads/";	// z:/hs/global_wads/

	// Work out which wads are installed and which we need to install.
	WadDataList game_data_wad_list;
	bool game_data_wads_read_ok = EnumerateWads( GlobalWadDir, DEFAULT_MEDIA, game_data_wad_list );

	WadDataList blu_ray_wad_list;
	bool blu_ray_wads_read_ok = EnumerateWads( GlobalWadDir, BLU_RAY_MEDIA, blu_ray_wad_list );

	if ( game_data_wads_read_ok && blu_ray_wads_read_ok )
	{
		s_HaveCompleteGUIGameData = CheckWads( game_data_wad_list, blu_ray_wad_list ); 
	}
	else
	{
		// If we failed to read anything then do the install again.
		s_HaveCompleteGUIGameData = false;
	}

	game_data_wad_list.clear();
	blu_ray_wad_list.clear();

	// Next we check the level global wads (i.e. <level-name>_global.wad files).
	bool wads_read_ok = true;
	uint32_t i = 0;
	while ( LevelNames[ i ] != NULL )
	{
		char full_level_path[ 1024 ];
		sprintf( full_level_path, "content_neutral/levels/%s_global.wad", LevelNames[ i ] );
		Util::StringToLowerInplace( full_level_path );

		WadData blu_ray_wad_data;
		blu_ray_wad_data.m_Filename = CHashedString( LevelNames[ i ] );
		if ( !GetMD5FromWAD( full_level_path, BLU_RAY_MEDIA, blu_ray_wad_data.m_MD5Digest[ 0 ], blu_ray_wad_data.m_MD5Digest[ 1 ] ) )
		{
			ntError_p( false, ("Failed to get MD5 from WAD %s", full_level_path) );
			wads_read_ok = false;
			break;
		}

		blu_ray_wad_list.push_back( blu_ray_wad_data );

		WadData hdd_wad_data;
		hdd_wad_data.m_Filename = CHashedString( LevelNames[ i ] );
		if ( !GetMD5FromWAD( full_level_path, DEFAULT_MEDIA, hdd_wad_data.m_MD5Digest[ 0 ], hdd_wad_data.m_MD5Digest[ 1 ] ) )
		{
			ntError_p( false, ("Failed to get MD5 from WAD %s", full_level_path) );
			wads_read_ok = false;
			break;
		}

		game_data_wad_list.push_back( hdd_wad_data );

		i++;
	}

	if ( wads_read_ok )
	{
		ntstd::sort( blu_ray_wad_list.begin(), blu_ray_wad_list.end(), &WadData::LessThan );
		ntstd::sort( game_data_wad_list.begin(), game_data_wad_list.end(), &WadData::LessThan );

		s_HaveCompleteGlobalGameData = CheckWads( game_data_wad_list, blu_ray_wad_list );
	}
	else
	{
		// If we failed to read anything then do the install again.
		s_HaveCompleteGlobalGameData = false;
	}
}

//*******************************************************************************
//	Check SysCache directories for status of any previous installs
//*******************************************************************************
void Wad::RecordSysCacheUsage()
{
}

//*******************************************************************************
//	Check that any existing WADs aren't corrupt.
//	NOTE:	FIOS won't be setup to use game-data when this is called so we
//			just use CellFS calls directly from within this function.
//*******************************************************************************
bool Wad::AreAnyGameDataWadsCorrupted( const char *game_data_path )
{
	// ARV: TODO.
	UNUSED( game_data_path );
	return false;
}












