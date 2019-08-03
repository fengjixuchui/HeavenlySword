//*******************************************************************************
//	
//	Archive.cpp.
//	
//******************************************************************************/

#include "core/compressedfile.h"
#include "core/standardarchive.h"
#include "core/fileio_ps3.h"
#include "core/wad.h"
#include "core/archive.h"
#include "game/shellconfig.h"

//*******************************************************************************
//	Private stuff - helper functions etc...
//*******************************************************************************
namespace
{
	MediaTypes FindMediaType( const char *filename )
	{
		// harddrive, wads have been deployed to gamedata or syscache
		if ( g_ShellOptions->m_bUsingHDD == true )
		{
			if ( FileManager::exists( filename, SYSCACHE_MEDIA ) )
			{
				// We have depolyed WADs, but this WAD is on the sys-cache,
				// not the game-data partition.
				return SYSCACHE_MEDIA;
			}
			return DEFAULT_MEDIA;
		}

		// no harddrive, running from blu ray
		if ( g_ShellOptions->m_bUsingBluRay == true )
		{
			return BLU_RAY_MEDIA;
		}

		// debug app_home case.
		return DEFAULT_MEDIA;
	}
}

//*******************************************************************************
//*******************************************************************************
//
//	Archive implementation.
//
//*******************************************************************************
//*******************************************************************************

//*******************************************************************************
//	Create an archive.
//*******************************************************************************
Archive *Archive::Create( const char *filename, MediaTypes media_type )
{
	int32_t archive_fd = -1;
	archive_fd = FileManager::open( filename, O_RDONLY, media_type );
	if ( archive_fd == -1 )
	{
		ntError_p( false, ("Unable to open compressed archive %s", filename) );
		return NULL;
	}

	Wad::ArchiveHeader archive_header;
	ReadHeader( archive_fd, archive_header );

	if ( ( archive_header.m_Format & Wad::COMPRESSED_ARCHIVE ) == Wad::COMPRESSED_ARCHIVE )
	{
		return NT_NEW CompressedArchive( filename, archive_fd, archive_header, media_type );
	}
	else
	{
		return NT_NEW StandardArchive( filename, archive_fd, archive_header, media_type );
	}

	FileManager::close( archive_fd );

	return NULL;
}

//*******************************************************************************
//	Create an archive.
//*******************************************************************************
Archive *Archive::Create( const char *filename, int32_t area_number, MediaTypes media_type )
{
	int32_t archive_fd = -1;
	archive_fd = FileManager::open( filename, O_RDONLY, media_type );
	if ( archive_fd == -1 )
	{
		ntError_p( false, ("Unable to open compressed archive %s", filename) );
		return NULL;
	}

	Wad::ArchiveHeader archive_header;
	ReadHeader( archive_fd, archive_header );

	if ( ( archive_header.m_Format & Wad::COMPRESSED_ARCHIVE ) == Wad::COMPRESSED_ARCHIVE )
	{
		return NT_NEW CompressedArchive( filename, archive_fd, archive_header, area_number, media_type );
	}
	else
	{
		return NT_NEW StandardArchive( filename, archive_fd, archive_header, area_number, media_type );
	}

	FileManager::close( archive_fd );

	return NULL;
}

//*******************************************************************************
//
//*******************************************************************************
bool Archive::FileExists( const char *filename ) const
{
	uint32_t hash = GetHash( filename );
	const Wad::ArchiveFileData *file_data = GetFileData( hash );
	return file_data != NULL;
}

//*******************************************************************************
//
//*******************************************************************************
Archive::Archive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, MediaTypes media_type )
:	m_NumFiles		( 0 )
,	m_HashTable		( TableSize )
,	m_DataOffset	( 0 )
,	m_IsAreaArchive	( false )
,	m_AreaNumber	( 0xabcdabcd )
,	m_MediaType		( media_type )
{
	ntError( m_HashTable.size() == TableSize );

	CreateArchiveFromFile( archive_fd, archive_header );

	m_DataOffset = archive_header.m_CompressedDataOffset;

	m_ArchiveName = CKeyString( filename );
}

//*******************************************************************************
//
//*******************************************************************************
Archive::Archive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, int32_t area_number, MediaTypes media_type )
:	m_NumFiles		( 0 )
,	m_HashTable		( TableSize )
,	m_DataOffset	( 0 )
,	m_IsAreaArchive	( true )
,	m_AreaNumber	( area_number )
,	m_MediaType		( media_type )
{
	ntError( m_HashTable.size() == TableSize );

	CreateArchiveFromFile( archive_fd, archive_header );

	m_DataOffset = archive_header.m_CompressedDataOffset;

	m_ArchiveName = CKeyString( filename );
}

//*******************************************************************************
//	Dtors alway need a body, even if pure-virtual.
//*******************************************************************************
Archive::~Archive()
{}

//*******************************************************************************
//	Returns the hash corresponding to this filename.
//*******************************************************************************
uint32_t Archive::GetHash( const char *filename )
{
	return CHashedString( filename ).GetHash();
}

//*******************************************************************************
//	Returns the index for the given hash.
//*******************************************************************************
const Wad::ArchiveFileData *Archive::GetFileData( uint32_t hash ) const
{
	uint32_t mod_hash = hash % TableSize;

	ntError( mod_hash >= 0 && mod_hash < TableSize );	// Paranoia...
	const DataVector &data_vector = m_HashTable[ mod_hash ];

	// This loop will (hopefully) always be cheap as each
	// index_vector should only contain a couple of entries at most.
	// This should give us O(1) look-up.
	for (	DataVector::const_iterator it = data_vector.begin();
			it != data_vector.end();
			++it )
	{
		if ( it->m_Hash == hash )
		{
			return &( *it );
		}
	}

	return NULL;
}

//*******************************************************************************
//	Insert an index and filename into the hash table.
//*******************************************************************************
void Archive::Insert( Wad::ArchiveFileData file_data )
{
	uint32_t hash = file_data.m_Hash;
	uint32_t mod_hash = hash % TableSize;

	DataVector &data_vector = m_HashTable[ mod_hash ];

	bool duplicate_found = false;
	for (	DataVector::iterator it = data_vector.begin();
			it != data_vector.end();
			++it )
	{
		if ( it->m_Hash == hash )
		{
			duplicate_found = true;
			break;
		}
	}

	ntError_p( !duplicate_found, ("Duplicate hash-value found! Bad. Bad.") );
	if ( !duplicate_found )
	{
		data_vector.push_back( file_data );
	}
}

//*******************************************************************************
//	Confirm archive type and read in archive header
//*******************************************************************************
void Archive::ReadHeader( int32_t archive_fd, Wad::ArchiveHeader &archive_header )
{
	FileManager::read( archive_fd, &archive_header, sizeof( Wad::ArchiveHeader ) );
}

//*******************************************************************************
//	Construct the class from what appears to be a valid file.
//*******************************************************************************
void Archive::CreateArchiveFromFile( int32_t archive_fd, const Wad::ArchiveHeader &header )
{
	// Load up the array of FileData structures.
	m_NumFiles = header.m_NumFiles;
	Wad::ArchiveFileData *file_data = NT_NEW Wad::ArchiveFileData[ m_NumFiles ];
	FileManager::read( archive_fd, file_data, sizeof( Wad::ArchiveFileData ) * m_NumFiles );

	// Insert each filename and index into the hash-table.
	for ( uint32_t i=0;i<m_NumFiles;i++ )
	{
		Insert( file_data[ i ] );
	}

	NT_DELETE_ARRAY( file_data );
}

//*******************************************************************************
//*******************************************************************************
//
//	ArchiveManager implementation.
//
//*******************************************************************************
//*******************************************************************************

// Define this to have all files not found within the currently loaded WAD-set
// logged to z:\hs\non_wad_files.txt.
//#define LOG_NON_WAD_FILES

//*******************************************************************************
//	Attempt to open a file from the archive.
//*******************************************************************************
bool ArchiveManager::OpenFile( const char *filename, File *file, Mem::MEMORY_CHUNK chunk, uint32_t alignment ) const
{
	ntError_p( filename != NULL, ("You must pass a valid filename pointer.") );
	ntError_p( file != NULL, ("You must pass a valid File pointer,") );

	for (	ArchiveContainer::iterator it = m_Archives.begin();
			it != m_Archives.end();
			++it )
	{
		Archive *archive = *it;

		if ( archive->OpenFile( filename, file, chunk, alignment ) )
		{
			// Move this archive to the front of the archive list.
			// This means we'll always try the least-recently-used
			// archive first each time...
			m_Archives.erase( it );
			m_Archives.push_front( archive );

			return true;
		}
	}

#	ifdef LOG_NON_WAD_FILES
	{
		char line[ 512 ];
		sprintf( line, "Open   - %s\n", filename );
		CellFsFile log_file( "/app_home/non_wad_files.txt", File::FT_TEXT | File::FT_WRITE | File::FT_APPEND );
		log_file.Write( line, strlen( line ) );
	}
#	endif // LOG_NON_WAD_FILES

	return false;
}

//*******************************************************************************
//	Attempt to open a file from the archive.
//*******************************************************************************
bool ArchiveManager::FileExists( const char *filename ) const
{
	ntError_p( filename != NULL, ("You must pass a valid filename pointer.") );

	for (	ArchiveContainer::iterator it = m_Archives.begin();
			it != m_Archives.end();
			++it )
	{
		Archive *archive = *it;

		if ( archive->FileExists( filename ) )
		{
			// Move this archive to the front of the archive list.
			// This means we'll always try the least-recently-used
			// archive first each time...
			m_Archives.erase( it );
			m_Archives.push_front( archive );

			return true;
		}
	}

#	ifdef LOG_NON_WAD_FILES
	{
		char line[ 512 ];
		sprintf( line, "Exists - %s\n", filename );
		CellFsFile log_file( "/app_home/non_wad_files.txt", File::FT_TEXT | File::FT_WRITE | File::FT_APPEND );
		log_file.Write( line, strlen( line ) );
	}
#	endif // LOG_NON_WAD_FILES

	return false;
}

//*******************************************************************************
//	Add an archive to the manager.
//*******************************************************************************
void ArchiveManager::AddArchive( const char *filename, MediaTypes media_type )
{
	ArchiveContainer::iterator existing_archive = ArchiveExists_Internal( filename );
	if ( existing_archive != m_Archives.end() )
	{
		// We want to overwrite the existing archive with this one.
		NT_DELETE( *existing_archive );
		m_Archives.erase( existing_archive );
	}

	Archive *archive = Archive::Create( filename, media_type );
	m_Archives.push_front( archive );
}

//*******************************************************************************
//	Add an archive to the manager.
//*******************************************************************************
void ArchiveManager::AddAreaArchive( const char *filename, int32_t area_number, MediaTypes media_type )
{
	ArchiveContainer::iterator existing_archive = ArchiveExists_Internal( filename );
	if ( existing_archive != m_Archives.end() )
	{
		// We want to overwrite the existing archive with this one.
		NT_DELETE( *existing_archive );
		m_Archives.erase( existing_archive );
	}

	Archive *archive = Archive::Create( filename, area_number, media_type );
	m_Archives.push_front( archive );
}

//*******************************************************************************
//	Remove an archive from the manager.
//*******************************************************************************
void ArchiveManager::RemoveArchive( const char *filename )
{
	CHashedString hashed_filename( filename );

	for (	ArchiveContainer::iterator it = m_Archives.begin();
			it != m_Archives.end();
			++it )
	{
		Archive *archive = *it;
		if ( archive->GetNameHash() == hashed_filename )
		{
			m_Archives.erase( it );
			NT_DELETE( archive );
			return;
		}
	}
}

//*******************************************************************************
//	Work out whether an Archive has already been installed or not.
//*******************************************************************************
bool ArchiveManager::ArchiveExists( const char *filename ) const
{
	return ArchiveExists_Internal( filename ) != m_Archives.end();
}

ArchiveManager::ArchiveContainer::iterator ArchiveManager::ArchiveExists_Internal( const char *filename ) const
{
	CHashedString hashed_filename( filename );

	ArchiveContainer::iterator it = m_Archives.begin();
	for (	;
			it != m_Archives.end();
			++it )
	{
		Archive *archive = *it;
		if ( archive->GetNameHash() == hashed_filename )
		{
			break;
		}
	}

	return it;
}

//*******************************************************************************
//	Remove an archive from the manager.
//*******************************************************************************
bool ArchiveManager::IsAreaWadLoaded( int32_t area_number ) const
{
	for (	ArchiveContainer::const_iterator it = m_Archives.begin();
			it != m_Archives.end();
			++it )
	{
		Archive *archive = *it;
		if ( archive->IsAreaArchive() && archive->GetAreaNumber() == area_number )
		{
			return true;
		}
	}

	return false;
}

//*******************************************************************************
//	Ctor.
//*******************************************************************************
ArchiveManager::ArchiveManager()
{}

//*******************************************************************************
//	Dtor.
//*******************************************************************************
ArchiveManager::~ArchiveManager()
{
	for (	ArchiveContainer::iterator it = m_Archives.begin();
			it != m_Archives.end();
			++it )
	{
		Archive *archive = *it;
		NT_DELETE( archive );
	}
}




















