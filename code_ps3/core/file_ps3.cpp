//---------------------------------------------
//!
//!
//!	\file core\file_pc.cpp
//!
//!
//---------------------------------------------

#include "core/file.h"
#include "core/fileio_ps3.h"
#include "core/cellfsfile_ps3.h"
#include "core/compressedfile.h"
#include "core/wad.h"
#include "game/shellconfig.h"

#ifndef _RELEASE

static CriticalSection s_FileCritical;
static char s_lastAccessedFile[MAX_PATH];
static bool s_valid = false;

void SetLastAccessedFile( const char* pFileName )
{
	if (pFileName)
	{
		ScopedCritical crit( s_FileCritical );
		strcpy( s_lastAccessedFile, pFileName );
		s_valid = true;
	}
}

void GetLastAccessedFile( char* pResult )
{
	if (s_valid)
	{
		ScopedCritical crit( s_FileCritical );
		strcpy( pResult, s_lastAccessedFile );
	}
}

#endif

#ifdef USE_FIOS_FOR_SYNCH_IO

//-----------------------------------------------
//!
//! Creates an invalid file object, that you can open
//! at your leasure
//! 
//-----------------------------------------------
File::File()
:	m_iFileHandle	( -1 )
,	m_CompressedFile( NULL )
{}

//-----------------------------------------------
//!
//!	Creates a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
File::File( const char* restrict pFileName, uint32_t uiType, Mem::MEMORY_CHUNK chunk /*= Mem::MC_MISC*/ )
:	m_iFileHandle	( -1 )
,	m_CompressedFile( NULL )
{
	Open( pFileName, uiType, chunk );
}

//-----------------------------------------------
//!
//!	Opens a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
void File::Open( const char* restrict pFileName, uint32_t uiType, Mem::MEMORY_CHUNK chunk /*= Mem::MC_MISC*/, uint32_t alignment /*= 1*/ )
{
#ifndef _RELEASE
	SetLastAccessedFile(pFileName);
#endif

	int iFIOSFlags = 0;

	if ( uiType & FT_READ )
	{
		iFIOSFlags = (( uiType & FT_WRITE ) == 0) ? O_RDONLY : O_RDWR;
	}
	else if ( uiType & FT_WRITE )
	{
		// the old usage of File on PS3 had an implied truncate and create
		// for files opened with write only.
		iFIOSFlags = O_WRONLY | O_CREAT;

		if (( uiType & FT_APPEND ) == 0)
			iFIOSFlags |= O_TRUNC;
	}

	if ( uiType & FT_APPEND )
		iFIOSFlags |= O_APPEND;

	if ( g_ShellOptions->m_bUsingWADs )
	{
		if ( ArchiveManager::Get().OpenFile( pFileName, this, chunk, alignment ) )
		{
			return;
		}
	}

	// First look in the sys-cache-, then the default-, media.
	MediaTypes media_type = DEFAULT_MEDIA;
	if ( FileManager::exists( pFileName, SYSCACHE_MEDIA ) )
	{
		media_type = SYSCACHE_MEDIA;
	}
	m_iFileHandle = FileManager::open( pFileName, iFIOSFlags, media_type );
	if ( m_iFileHandle >= 0 )
	{
		// If we're using WADs and we're not looking for a WAD file itself and we only want to read,
		// then log this as something that should be inside a WAD to start with!
		if ( g_ShellOptions->m_bUsingWADs && strstr( pFileName, ".wad" ) == NULL && ( uiType & FT_READ ) == FT_READ )
		{
			ntPrintf( Debug::DCU_RESOURCES, "WAD: File not in WAD: %s.\n", pFileName );
		}

		return;
	}

	ntPrintf("FIOS: Failed to open file %s with FIOS error code (%d)\n", pFileName, m_iFileHandle);
}

//! dtor, closes file handle if open
File::~File()
{
	// We only delete the compressed-file on deletion of the File object,
	// not on a Close() operation. This is so we can Close() the file, thereby
	// releasing the file-handle, but at the same time still be able to
	// access the data that was already loaded into memory. This enables us
	// to use File as an implementation of MemFile without having to have
	// the file-handle constantly open.
	NT_DELETE( m_CompressedFile );
	m_CompressedFile = NULL;

	Close();
}

void File::Close()
{
	if ( m_iFileHandle >= 0 )
	{
		int err = FileManager::close( m_iFileHandle );
		ntAssert_p(err >= 0, ("Failed to close file with fios error (%d). See fios_types.h\n", err));
		UNUSED(err);

		m_iFileHandle = -1;
	}
}

//--------------------------------------------
//!
//!	IsValid return false if the file handle is NULL.
//!	File loading/creation problems are the usual suspects
//!
//--------------------------------------------
bool File::IsValid() const
{
	return m_CompressedFile != NULL ? m_CompressedFile->IsValid() : ( m_iFileHandle >= 0 );
}

//--------------------------------------------
//!
//!	Exists return true if the file is present
//!
//--------------------------------------------
bool File::Exists( const char* pFileName )
{
	return ArchiveManager::Get().FileExists( pFileName ) ? true : FileManager::exists( pFileName );
}

//--------------------------------------------
//!
//!	Write data from supplied buffer into a file.
//!	\param pIn buffer where data will written from
//!	\param sizeToWrite amount of bytes to write to disk
//!
//--------------------------------------------
void File::Write( const void* pIn, size_t sizeToWrite )
{
	ntError_p( m_CompressedFile == NULL, ("You cannot write to a compressed file.") );
	FileManager::write( m_iFileHandle, pIn, sizeToWrite );
}

//------------------------------------------------------
//!
//!	Read data from file into a supplied buffer.
//!	\param pOut buffer where data will read into MUST be at least sizeToRead big
//!	\param sizeToRead amount of bytes to read of disk into pOut
//! \return amount actaully read
//!
//------------------------------------------------------
size_t File::Read( void* restrict pOut, size_t sizeToRead )
{
	return m_CompressedFile != NULL ? m_CompressedFile->Read( pOut, sizeToRead ) : FileManager::read( m_iFileHandle, pOut, sizeToRead );
}

//------------------------------------------------------
//!
//!	Allocates an appropriately size buffer and reads file
//! into it.
//!
//------------------------------------------------------
size_t File::AllocateRead( char** ppOut )
{
	ntAssert_p( IsValid(), ("Reading from and invalid file\n") );
	
	size_t fileSize = GetFileSize();
	ntAssert_p( fileSize > 0, ("Reading from a zero sized file\n") );

	*ppOut = NT_NEW char[ fileSize ];
	size_t readSize = Read( *ppOut, fileSize );

	return readSize;
}

//------------------------------------------------------
//!
//! Tell function.
//!
//------------------------------------------------------
size_t File::Tell() const
{
	return m_CompressedFile != NULL ? m_CompressedFile->Tell() : FileManager::tell( m_iFileHandle );
}

//------------------------------------------------------
//!
//! Seek function.
//!
//------------------------------------------------------
void File::Seek( int32_t offset, SeekMode mode )
{
	if ( m_CompressedFile != NULL )
	{
		m_CompressedFile->Seek( offset, mode );
		return;
	}

	static const int32_t SeekModesPS3[ File::NUM_SEEK_MODES ] =
	{
		SEEK_SET,
		SEEK_END,
		SEEK_CUR
	};

	FileManager::lseek( m_iFileHandle, offset, SeekModesPS3[mode] );
}

//------------------------------------------------------
//!
//! Flush the file buffers.
//!
//------------------------------------------------------
void File::Flush()
{
	// dont need to do this on PS3
}

//------------------------------------------------------
//!
//! Returns the size of this file.
//!
//------------------------------------------------------
size_t File::GetFileSize()
{
	return m_CompressedFile != NULL ? m_CompressedFile->GetFileSize() : FileManager::getFileSize( m_iFileHandle );
}

//------------------------------------------------------
//!
//! Returns the data from the compressed file.
//!
//------------------------------------------------------
uint8_t *File::GetCompressedFileRawData()
{
	return m_CompressedFile != NULL ? m_CompressedFile->GetData() : NULL;
}

//------------------------------------------------------
//!
//! 
//!
//------------------------------------------------------
void File::TakeOwnershipOfMemory()
{
	if ( m_CompressedFile != NULL )
	{
		m_CompressedFile->TakeOwnershipOfMemory();
	}
}

#else // USE_FIOS_FOR_SYNCH_IO

//-----------------------------------------------
//!
//! Creates an invalid file object, that you can open
//! at your leasure
//! 
//-----------------------------------------------
File::File()
:	m_CompressedFile( NULL )
{
	m_pFileImpl = NT_NEW CellFsFile;
}

//-----------------------------------------------
//!
//!	Creates a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
File::File( const char* restrict pFileName, uint32_t uiType, Mem::MEMORY_CHUNK )
:	m_CompressedFile( NULL )
{
	m_pFileImpl = NT_NEW CellFsFile( pFileName, uiType );
}

//-----------------------------------------------
//!
//!	Opens a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
void File::Open( const char* restrict pFileName, uint32_t uiType, Mem::MEMORY_CHUNK, uint32_t )
{
#ifndef _GOLD_MASTER
	SetLastAccessedFile(pFileName);
#endif

	m_pFileImpl->Open( pFileName, uiType );
}

//! dtor, closes file handle if open
File::~File()
{
	Close();
}

void File::Close()
{
	NT_DELETE( m_pFileImpl );
}

//--------------------------------------------
//!
//!	IsValid return false if the file handle is NULL.
//!	File loading/creation problems are the usual suspects
//!
//--------------------------------------------
bool File::IsValid() const
{
	return m_pFileImpl->IsValid();
}

//--------------------------------------------
//!
//!	Exists return true if the file is present
//!
//--------------------------------------------
bool File::Exists( const char* pFileName )
{
	return CellFsFile::Exists( pFileName );
}

//!
//!
//!	Write data from supplied buffer into a file.
//!	\param pIn buffer where data will written from
//!	\param sizeToWrite amount of bytes to write to disk
//!
void File::Write( const void* pIn, size_t sizeToWrite )
{
	m_pFileImpl->Write( pIn, sizeToWrite );
}

//------------------------------------------------------
//!
//!	Read data from file into a supplied buffer.
//!	\param pOut buffer where data will read into MUST be at least sizeToRead big
//!	\param sizeToRead amount of bytes to read of disk into pOut
//! \return amount actaully read
//!
//------------------------------------------------------
size_t File::Read( void* restrict pOut, size_t sizeToRead )
{
	return m_pFileImpl->Read( pOut, sizeToRead );
}

//------------------------------------------------------
//!
//!	Allocates an appropriately size buffer and reads file
//! into it.
//!
//------------------------------------------------------
size_t File::AllocateRead( char** ppOut )
{
	return m_pFileImpl->AllocateRead( ppOut );
}

//------------------------------------------------------
//!
//! Tell function.
//!
//------------------------------------------------------
size_t File::Tell() const
{
	return m_pFileImpl->Tell();
}

//------------------------------------------------------
//!
//! Seek function.
//!
//------------------------------------------------------
void File::Seek( int32_t offset, SeekMode mode )
{
	m_pFileImpl->Seek( offset, mode );
}

//------------------------------------------------------
//!
//! Flush the file buffers.
//!
//------------------------------------------------------
void File::Flush()
{
	m_pFileImpl->Flush();
}

//------------------------------------------------------
//!
//! Returns the size of this file.
//!
//------------------------------------------------------
size_t File::GetFileSize()
{
	return m_pFileImpl->GetFileSize();
}

//------------------------------------------------------
//!
//! Can't read from compressed files without FIOS...
//!
//------------------------------------------------------
const uint8_t *File::GetCompressedFileRawData() const
{
	return NULL;
}

//------------------------------------------------------
//!
//! 
//!
//------------------------------------------------------
void File::TakeOwnershipOfMemory()
{
}

#endif // USE_FIOS_FOR_SYNCH_IO

void LoadFile_Chunk( const char *filename, uint32_t open_flags, Mem::MEMORY_CHUNK chunk, File &file_out, uint8_t **loaded_file_data )
{
	ntError( loaded_file_data != NULL );
	ntError_p( !file_out.IsValid(), ("You can't pass an already open File object.") );
	file_out.Open( filename, open_flags, chunk );

	if ( file_out.IsValid() )
	{
		if ( file_out.IsCompressedFile() )
		{
			file_out.TakeOwnershipOfMemory();
			*loaded_file_data = (uint8_t *)file_out.GetCompressedFileRawData();
		}
		else
		{
			size_t file_size = file_out.GetFileSize();
			ntAssert_p( file_size > 0, ("Reading from a zero sized file.\n") );

			*loaded_file_data = NT_NEW_ARRAY_CHUNK( chunk ) uint8_t[ file_size ];
			size_t read_size = file_out.Read( *loaded_file_data, file_size );

			UNUSED( read_size );
			UNUSED( file_size );
			ntAssert_p( read_size == file_size, ("Error reading file.\n") );
		}
	}
}

void LoadFile_ChunkMemAlign( const char *filename, uint32_t open_flags, Mem::MEMORY_CHUNK chunk, uint32_t alignment, File &file_out, uint8_t **loaded_file_data )
{
	ntError( loaded_file_data != NULL );
	ntError_p( !file_out.IsValid(), ("You can't pass an already open File object.") );
	file_out.Open( filename, open_flags, chunk, alignment );

	if ( file_out.IsValid() )
	{
		if ( file_out.IsCompressedFile() )
		{
			file_out.TakeOwnershipOfMemory();
			*loaded_file_data = (uint8_t *)file_out.GetCompressedFileRawData();
		}
		else
		{
			size_t file_size = file_out.GetFileSize();
			ntAssert_p( file_size > 0, ("Reading from a zero sized file.\n") );

			*loaded_file_data = (uint8_t *)NT_MEMALIGN_CHUNK( chunk, file_size, alignment );
			size_t read_size = file_out.Read( *loaded_file_data, file_size );

			UNUSED( read_size );
			UNUSED( file_size );
			ntAssert_p( read_size == file_size, ("Error reading file.\n") );
		}
	}
}


