//-------------------------------------------------------
//!
//!	\file core\blurayfile_ps3.h
//!	
//!	File class to transparently access blu-ray and if
//! not present to search the default media object.
//!
//-------------------------------------------------------

#include "core/blurayfile_ps3.h"
#include "core/fileio_ps3.h"


//-----------------------------------------------
//!
//! Creates an invalid file object, that you can open
//! at your leasure
//! 
//-----------------------------------------------
BluRayFile::BluRayFile()
:	m_iFileHandle( -1 )
{}

//-----------------------------------------------
//!
//!	Creates a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
BluRayFile::BluRayFile( const char *pFileName )
{
	Open( pFileName );
}

//-----------------------------------------------
//!
//!	Opens a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
void BluRayFile::Open( const char *pFileName )
{
	m_iFileHandle = FileManager::open( pFileName, O_RDONLY, BLU_RAY_MEDIA );
	if ( m_iFileHandle < 0 )
	{
		// Couldn't find the file on the blu-ray drive...
		m_iFileHandle = FileManager::open( pFileName, O_RDONLY, DEFAULT_MEDIA );
	}

	if ( m_iFileHandle < 0 )
	{
		ntPrintf("FIOS: Failed to open file %s with FIOS error code (%d)\n", pFileName, m_iFileHandle);
	}
}

//! dtor, closes file handle if open
BluRayFile::~BluRayFile()
{
	Close();
}

void BluRayFile::Close()
{
	if ( m_iFileHandle >= 0 )
	{
		int32_t err = FileManager::close( m_iFileHandle );
		ntAssert_p( err >= 0, ("Failed to close file with fios error (%d). See fios_types.h\n", err) );
		UNUSED( err );
	}
}

//--------------------------------------------
//!
//!	IsValid return false if the file handle is NULL.
//!	File loading/creation problems are the usual suspects
//!
//--------------------------------------------
bool BluRayFile::IsValid() const
{
	if ( m_iFileHandle < 0 )
	{
		return false;
	}

	return true;
}

//--------------------------------------------
//!
//!	Exists return true if the file is present
//!
//--------------------------------------------
bool BluRayFile::Exists( const char *pFileName )
{
	if (  FileManager::exists( pFileName, BLU_RAY_MEDIA ) )
	{
		return true;
	}
	else
	{
		return FileManager::exists( pFileName, DEFAULT_MEDIA );
	}
}

//------------------------------------------------------
//!
//!	Read data from file into a supplied buffer.
//!	\param pOut buffer where data will read into MUST be at least sizeToRead big
//!	\param sizeToRead amount of bytes to read of disk into pOut
//! \return amount actaully read
//!
//------------------------------------------------------
size_t BluRayFile::Read( void* restrict pOut, size_t sizeToRead )
{
	return FileManager::read( m_iFileHandle, pOut, sizeToRead );
}

//------------------------------------------------------
//!
//! Tell function.
//!
//------------------------------------------------------
size_t BluRayFile::Tell() const
{
	return FileManager::lseek( m_iFileHandle, 0, SEEK_CUR );
}

//------------------------------------------------------
//!
//! Seek function.
//!
//------------------------------------------------------
void BluRayFile::Seek( int32_t offset, SeekMode mode )
{
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
//! Returns the size of this file.
//!
//------------------------------------------------------
size_t BluRayFile::GetFileSize()
{
	long cur = Tell();
	Seek( 0, SEEK_FROM_END );

	long ret = Tell();
	Seek( cur, SEEK_FROM_START );

	return (size_t) ret;
}





