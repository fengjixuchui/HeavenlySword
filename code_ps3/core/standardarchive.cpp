//*******************************************************************************
//	
//	StandardFile.cpp.
//	
//******************************************************************************/

#include "core/standardarchive.h"

#include "core/waddef.h"
#include "core/fileio_ps3.h"

//*******************************************************************************
//	Ctor.
//*******************************************************************************
StandardArchive::StandardArchive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, MediaTypes media_type )
:	Archive( filename, archive_fd, archive_header, media_type )
{}

//*******************************************************************************
//	Ctor.
//*******************************************************************************
StandardArchive::StandardArchive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, int32_t area_number, MediaTypes media_type )
:	Archive( filename, archive_fd, archive_header, area_number, media_type )
{}

//*******************************************************************************
//	Dtor.
//*******************************************************************************
StandardArchive::~StandardArchive()
{}

//*******************************************************************************
//	Attempt to open the given file from the archive.
//*******************************************************************************
bool StandardArchive::OpenFile( const char *filename, File *file, Mem::MEMORY_CHUNK chunk, uint32_t alignment ) const
{
	UNUSED( chunk );
	UNUSED( alignment );

	// Check the passed File object.
	ntError_p( file != NULL, ("You must pass a valid File object constructed using the default ctor.") );
	ntError_p( file->m_CompressedFile == NULL, ("The passed File object must not previously have been setup.") );
#	ifdef USE_FIOS_FOR_SYNCH_IO
		ntError_p( file->m_iFileHandle == -1, ("The passed File object must not previously have been setup.") );
#	else
		ntError_p( false, ("You must use FIOS to enable support for compressed archives.") );
#	endif

	uint32_t hash = GetHash( filename );
	const Wad::ArchiveFileData *file_data = GetFileData( hash );
	if ( file_data == NULL )
	{
		return false;
	}

	// Because this is an UNCOMPRESSED (standard) archive, the m_CompressedDataOffset
	// and m_CompressedDataLength members of Wad::ArchiveFileData are actually the uncompressed versions.
	off_t subfile_offset = file_data->m_CompressedDataOffset + m_DataOffset;
	ssize_t subfile_length = file_data->m_UncompressedDataLength;
	file->m_iFileHandle = FileManager::openSubFile( m_ArchiveName.GetString(), O_RDONLY, m_MediaType, subfile_offset, subfile_length );

	ntError_p( file->m_iFileHandle >= 0, ("Failed to open archive %s from media-type %i.", m_ArchiveName.GetString(), (uint32_t)m_MediaType) )
	return file->m_iFileHandle >= 0;
}























