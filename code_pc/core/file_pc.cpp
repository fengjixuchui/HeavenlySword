//---------------------------------------------
//!
//!
//!	\file core\file_pc.cpp
//!
//!
//---------------------------------------------

#include "core/file.h"

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

//-----------------------------------------------
//!
//! Creates an invalid file object, that you can open
//! at your leasure
//! 
//-----------------------------------------------
File::File()  :
	m_pFileHandle(0)
{
}

//-----------------------------------------------
//!
//!	Creates a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
File::File( const char* restrict pFileName, uint32_t uiType, Mem::MEMORY_CHUNK chunk ) :
	m_pFileHandle(0)
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
void File::Open( const char* restrict pFileName, uint32_t uiType, Mem::MEMORY_CHUNK, uint32_t )
{
#ifndef _RELEASE
	SetLastAccessedFile(pFileName);
#endif

	char mode[4] = {0};
	int curChar = 0;

	// convery FILE_TYPEs into c mode's;
	if( uiType & File::FT_READ )
	{
		mode[curChar++] = 'r';
		if( uiType & File::FT_WRITE )
		{
			if( uiType & File::FT_APPEND )
			{
				// change out mind about 'r'
				mode[0] = 'w';
				mode[curChar++] = '+';
			} else
			{
				mode[curChar++] = '+';
			}
		}
	} else if( uiType & File::FT_WRITE )
	{
		if( uiType & File::FT_APPEND )
		{
			mode[curChar++] = 'a';
		} else
		{
			mode[curChar++] = 'w';
		}
	} else
	{
		ntError_p( false, ("File created without READ or WRITE") );
	}

	if( uiType & File::FT_TEXT )
	{
		mode[curChar++] = 't';
	} else if( uiType & File::FT_BINARY )
	{
		mode[curChar++] = 'b';
	}


	// lets be a bit naughty... I want write though access for the log file writing text file is flushed straight through (no caching)
	if( uiType & FT_LOG )
	{
		mode[curChar++] = 'c';
	} 
	m_pFileHandle = (void*) fopen( pFileName, mode );
}

//! dtor, closes file handle if open
File::~File()
{
	Close();
}

//! Close the file.
void File::Close()
{
	if( m_pFileHandle != 0 )
	{
		fclose( (FILE*) m_pFileHandle );
		m_pFileHandle = 0;
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
	return (m_pFileHandle != 0);
}

//--------------------------------------------
//!
//!	Exists return true if the file is present
//!
//--------------------------------------------
bool File::Exists( const char* pFileName )
{
	File tmp( pFileName, File::FT_READ );
	return tmp.IsValid();
}

//!
//!
//!	Write data from supplied buffer into a file.
//!	\param pIn buffer where data will written from
//!	\param sizeToWrite amount of bytes to write to disk
//!
void File::Write( const void* restrict pIn, size_t sizeToWrite )
{
	fwrite( pIn, sizeToWrite, 1, (FILE*) m_pFileHandle );
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
	ntAssert( IsValid() );
	memset( pOut, 0, sizeToRead );
	return fread( pOut, sizeof(char), sizeToRead, (FILE*) m_pFileHandle );
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

	if (readSize != fileSize)
	{
		if (ferror((FILE*) m_pFileHandle))
		{
			ntAssert_p( 0, ("Error reading file\n") );
		}

		if (feof((FILE*) m_pFileHandle))
		{
			ntAssert_p( 0, ("Error reading file, End of File reached\n") );
		}
	}

	return readSize;
}

//------------------------------------------------------
//!
//! Tell function.
//!
//------------------------------------------------------
size_t File::Tell() const
{
	return ftell( static_cast< FILE * >( m_pFileHandle ) );
}

//------------------------------------------------------
//!
//! Seek function.
//!
//------------------------------------------------------
void File::Seek( int32_t offset, SeekMode mode )
{
	static const int32_t SeekModesPC[ NUM_SEEK_MODES ] =
	{
		SEEK_SET,
		SEEK_END,
		SEEK_CUR
	};
	fseek( static_cast< FILE * >( m_pFileHandle ), offset, SeekModesPC[ mode ] );
}

//------------------------------------------------------
//!
//! Flush the file buffers.
//!
//------------------------------------------------------
void File::Flush()
{
	fflush( (FILE*) m_pFileHandle );
}

//------------------------------------------------------
//!
//! Returns the size of this file.
//!
//------------------------------------------------------
size_t File::GetFileSize()
{
	long cur = ftell( (FILE*) m_pFileHandle );
	fseek( (FILE*) m_pFileHandle, 0, SEEK_END );
	long ret = ftell( (FILE*) m_pFileHandle );
	fseek( (FILE*) m_pFileHandle, cur, SEEK_SET );

	return (size_t) ret;
}

//------------------------------------------------------
//!
//! We can't have compressed-files on PC, so return NULL.
//!
//------------------------------------------------------
uint8_t *File::GetCompressedFileRawData()
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


