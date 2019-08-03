//*******************************************************************************
//	
//	CompressedFile.cpp.
//	
//******************************************************************************/

#include "core/fileio_ps3.h"
#include "core/compress.h"
#include "core/waddef.h"

#include "core/compressedfile.h"

//*******************************************************************************
//*******************************************************************************
//
//	CompressedFile implementation.
//
//*******************************************************************************
//*******************************************************************************

//*******************************************************************************
//	
//*******************************************************************************
size_t CompressedFile::Read( void * restrict pOut, size_t sizeToRead )
{
	ntError_p( m_Offset+sizeToRead >= 0 && m_Offset+sizeToRead <= m_Length, ("Attempting to read off the end of the compressed file - read will be truncated to maximum length.") );

	if ( m_Offset + sizeToRead > m_Length )
	{
		sizeToRead = m_Length - m_Offset;
	}

	FwMemcpy( pOut, m_Data+m_Offset, sizeToRead );

	m_Offset += sizeToRead;
	ntError_p( m_Offset >= 0 && m_Offset <= m_Length, ("CompressedFile offset out of bounds after read.") );

	return sizeToRead;
}

//*******************************************************************************
//	
//*******************************************************************************
size_t CompressedFile::Seek( int32_t offset, File::SeekMode mode )
{
	switch ( mode )
	{
		case File::SEEK_FROM_CURRENT:
		{
			ntError( m_Offset+offset >= 0 && m_Offset+offset <= m_Length );
			m_Offset += offset;
			break;
		}

		case File::SEEK_FROM_START:
		{
			ntError( offset >= 0 && offset <= (int32_t)m_Length );
			m_Offset = offset;
			break;
		}

		case File::SEEK_FROM_END:
		{
			ntError( m_Length+offset >= 0 && m_Length+offset <= m_Length );
			m_Offset = m_Length + offset;
		}

		default:
		{
			ntError_p( false, ("Invalid seek mode detected - no seek will be performed.") );
			break;
		}
	}

	return m_Offset;
}

//*******************************************************************************
//	Ctor.
//*******************************************************************************
CompressedFile::CompressedFile( const void *compressed_data, uint32_t comp_length, uint32_t uncomp_length, Mem::MEMORY_CHUNK chunk, uint32_t alignment )
:	m_Data		( NULL )
,	m_Length	( 0 )
,	m_Offset	( 0 )
,	m_Chunk		( chunk )
,	m_Alignment	( alignment )
,	m_OwnsMemory( true )
{
	if ( alignment > 1 )
	{
		m_Data = (uint8_t *)NT_MEMALIGN_CHUNK( chunk, uncomp_length, alignment );
	}
	else
	{
		m_Data = NT_NEW_CHUNK( chunk ) uint8_t[ uncomp_length ];
	}
	ntError( m_Data != NULL );

	Compress::Block data_in;
	data_in.m_Mem = (void *)compressed_data;
	data_in.m_Length = comp_length;

	Compress::Block data_out;
	data_out.m_Mem = m_Data;
	data_out.m_Length = uncomp_length;

	Compress::Unpack( data_in, data_out );

	m_Length = data_out.m_Length;
}

//*******************************************************************************
//	Dtor.
//*******************************************************************************
CompressedFile::~CompressedFile()
{
	if ( m_OwnsMemory )
	{
		if ( m_Alignment > 1 )
		{
			NT_FREE_CHUNK( m_Chunk, (uintptr_t)m_Data );
		}
		else
		{
			NT_DELETE_ARRAY_CHUNK( m_Chunk, (uint8_t *)m_Data );
		}
	}
}

//*******************************************************************************
//*******************************************************************************
//
//	CompressedArchive implementation.
//
//*******************************************************************************
//*******************************************************************************

//*******************************************************************************
//	Ctor.
//*******************************************************************************
CompressedArchive::CompressedArchive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, MediaTypes media_type )
:	Archive( filename, archive_fd, archive_header, media_type )
{}

//*******************************************************************************
//	Ctor.
//*******************************************************************************
CompressedArchive::CompressedArchive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, int32_t area_number, MediaTypes media_type )
:	Archive( filename, archive_fd, archive_header, area_number, media_type )
{}

//*******************************************************************************
//	Dtor.
//*******************************************************************************
CompressedArchive::~CompressedArchive()
{}

//*******************************************************************************
//	Attempt to open the given file from the archive.
//*******************************************************************************
bool CompressedArchive::OpenFile( const char *filename, File *file, Mem::MEMORY_CHUNK chunk, uint32_t alignment ) const
{
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

	int32_t archive_fd = FileManager::open( m_ArchiveName.GetString(), O_RDONLY, m_MediaType );
	if ( archive_fd == -1 )
	{
		ntError_p( false, ("Failed to open archive %s.", m_ArchiveName.GetString()) );
		return false;
	}

	// Get to the start of this file's compressed data.
	FileManager::lseek( archive_fd, m_DataOffset + file_data->m_CompressedDataOffset, SEEK_SET );
	
	// Read in the correct amount of compressed data.
	uint8_t *compressed_data = NT_NEW_CHUNK( chunk ) uint8_t[ file_data->m_CompressedDataLength ];
	FileManager::read( archive_fd, compressed_data, file_data->m_CompressedDataLength );
	FileManager::close( archive_fd );

	// Create a CompressedFile object and set it in the passed File object.
	file->m_CompressedFile = NT_NEW CompressedFile( compressed_data, file_data->m_CompressedDataLength, file_data->m_UncompressedDataLength, chunk, alignment );

	// Cleanup our allocated memory.
	NT_DELETE_ARRAY_CHUNK( chunk, compressed_data );

	ntError_p( file->m_CompressedFile->IsValid(), ("Failed to open a file from an archive that we previous said was present in the archive.") )

	return file->m_CompressedFile->IsValid();
}























