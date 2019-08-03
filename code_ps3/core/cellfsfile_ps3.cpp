//---------------------------------------------
//!
//!
//!	\file core\file_pc.cpp
//!
//!
//---------------------------------------------

#include <cell/fs/cell_fs_file_api.h>
#include <cell/fs/cell_fs_errno.h>
#include "core/cellfsfile_ps3.h"

void PrintCellFsError( CellFsErrno err )
{

#if !defined(_RELEASE)
	// do print out a message to the console
	switch( err )
	{
	case CELL_FS_ENOENT:
		ntPrintf( "CELL_FS_ENOENT: The specified file does not exist\n" ); break;
	case CELL_FS_EINVAL:
		ntPrintf( "CELL_FS_EINVAL: The specified path is invalid OR the length of path without mount point exceeds CELL_FS_MAX_FS_PATH_LENGTH\n" ); break;
	case CELL_FS_EMFILE:
		ntPrintf( "CELL_FS_EMFILE: Too many opened files\n" ); break;
	case CELL_FS_EFSSPECIFIC:
		ntPrintf( "CELL_FS_EFSSPECIFIC: System call related to inter process communication with file system server failed\n" ); break;
	case CELL_FS_ENOSYS:
		ntPrintf( "CELL_FS_ENOSYS: Connection to file system server process failed\n" ); break;
	case CELL_FS_EFAULT:
		ntPrintf( "CELL_FS_EFAULT: parameter is incorred (probably some pointer is NULL)\n" ); break;
	case CELL_FS_ENAMETOOLONG:
		ntPrintf( "CELL_FS_ENAMETOOLONG: The elements of path exceed the limiation of length\n" ); break;
	case CELL_FS_EACCES:
		ntPrintf( "CELL_FS_EACCES: Permission denied\n" ); break;
	case CELL_FS_EISDIR:
		ntPrintf( "CELL_FS_EISDIR: Tried to access a directory\n" ); break;
	case CELL_FS_EEXIST:
		ntPrintf( "CELL_FS_EEXIST: Specified CELL_FS_O_EXCL but the file already exists\n" ); break;
	case CELL_FS_EIO:
		ntPrintf( "CELL_FS_EIO: I/O ntError occurs\n" ); break;
	case CELL_FS_EBADMSG:
		ntPrintf( "CELL_FS_EBADMSG: Received an unexpected communications message\n" ); break;
	case CELL_FS_ENOMEM:
		ntPrintf( "CELL_FS_ENOMEM: Memory and resource are insufficient\n" ); break;
	case CELL_FS_ETIMEDOUT:
		ntPrintf( "CELL_FS_ETIMEDOUT: Communication timeout occurs\n" ); break;
	case CELL_FS_EPIPE:
		ntPrintf( "CELL_FS_EPIPE: File server closed the connection\n" ); break;
	case CELL_FS_EPERM:
		ntPrintf( "CELL_FS_EPERM: A flag violates the protected attribute specified when it is mounted\n" ); break;
	case CELL_FS_ENOSPC:
		ntPrintf( "CELL_FS_ENOSPC: There is not enough room to create the file.\n" ); break;
	}
#else
	UNUSED( err );
#endif

}

//-----------------------------------------------
//!
//! Creates an invalid file object, that you can open
//! at your leasure
//! 
//-----------------------------------------------
CellFsFile::CellFsFile()  :
	m_iFileHandle(0)
{
}

//-----------------------------------------------
//!
//!	Creates a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
CellFsFile::CellFsFile( const char* restrict pFileName, uint32_t uiType ) :
	m_iFileHandle(0)
{
	Open( pFileName, uiType );
}

//-----------------------------------------------
//!
//!	Opens a file of the specified name and type
//!	\param pFileName filename to open or create
//!	\param uiType bitwise AND of File FILE_TYPE enum
//!
//-----------------------------------------------
void CellFsFile::Open( const char* restrict pFileName, uint32_t uiType )
{
	int flags = 0;

	if ( uiType & File::FT_READ )
	{
		flags |= ( (uiType & File::FT_WRITE) == 0 ) ? CELL_FS_O_RDONLY : CELL_FS_O_RDWR;
	}
	else if ( uiType & File::FT_WRITE )
	{
		flags |= CELL_FS_O_WRONLY | CELL_FS_O_CREAT;

		if ( ( uiType & File::FT_APPEND ) == 0 )
			flags |= CELL_FS_O_TRUNC;
	}

	if ( uiType & File::FT_APPEND )
		flags |= CELL_FS_O_APPEND;

	// Cell API has no concept of text or binary...
	m_iFileHandle = -1;
	CellFsErrno err = cellFsOpen( pFileName, flags, &m_iFileHandle, 0, 0 );
	if( err != CELL_FS_SUCCEEDED )
	{
		// don't ntAssert cos the code may happily handle the file not being there but
		PrintCellFsError( err );
	}
}

//! dtor, closes file handle if open
CellFsFile::~CellFsFile()
{
	if( m_iFileHandle != 0 )
		cellFsClose( m_iFileHandle );
}

//--------------------------------------------
//!
//!	IsValid return false if the file handle is NULL.
//!	File loading/creation problems are the usual suspects
//!
//--------------------------------------------
bool CellFsFile::IsValid() const
{
	return (m_iFileHandle != -1);
}

//--------------------------------------------
//!
//!	Exists return true if the file is present
//!
//--------------------------------------------
bool CellFsFile::Exists( const char* pFileName )
{
	CellFsStat stat;
	CellFsErrno err = cellFsStat( pFileName, &stat );
	if( err == CELL_FS_SUCCEEDED)
		return true;
	else
		return false;

}

//!
//!
//!	Write data from supplied buffer into a file.
//!	\param pIn buffer where data will written from
//!	\param sizeToWrite amount of bytes to write to disk
//!
void CellFsFile::Write( const void* pIn, size_t sizeToWrite )
{
	uint64_t toWrite = sizeToWrite;
	uint64_t writtenByteSize;

	CellFsErrno err = cellFsWrite( m_iFileHandle, pIn, toWrite, &writtenByteSize );
	PrintCellFsError( err );
}

//------------------------------------------------------
//!
//!	Read data from file into a supplied buffer.
//!	\param pOut buffer where data will read into MUST be at least sizeToRead big
//!	\param sizeToRead amount of bytes to read of disk into pOut
//! \return amount actaully read
//!
//------------------------------------------------------
size_t CellFsFile::Read( void* restrict pOut, size_t sizeToRead )
{
	ntAssert( IsValid() );
	memset( pOut, 0, sizeToRead );
	uint64_t toRead = sizeToRead;
	uint64_t bytesRead;
	CellFsErrno err = cellFsRead( m_iFileHandle, pOut, toRead, &bytesRead );
	PrintCellFsError( err );
	return bytesRead;
}

//------------------------------------------------------
//!
//!	Allocates an appropriately size buffer and reads file
//! into it.
//!
//------------------------------------------------------
size_t CellFsFile::AllocateRead( char** ppOut )
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
size_t CellFsFile::Tell() const
{
	uint64_t curr_pos( 0 );
	CellFsErrno err = cellFsLseek( m_iFileHandle, 0, CELL_FS_SEEK_CUR, &curr_pos );
	PrintCellFsError( err );
	return curr_pos;
}

//------------------------------------------------------
//!
//! Seek function.
//!
//------------------------------------------------------
void CellFsFile::Seek( int32_t offset, File::SeekMode mode )
{
	static const int32_t SeekModesPS3[ File::NUM_SEEK_MODES ] =
	{
		CELL_FS_SEEK_SET,
		CELL_FS_SEEK_END,
		CELL_FS_SEEK_CUR
	};

	uint64_t curr_pos( 0 );
	CellFsErrno err = cellFsLseek( m_iFileHandle, offset, SeekModesPS3[ mode ], &curr_pos );
	PrintCellFsError( err );
}

//------------------------------------------------------
//!
//! Flush the file buffers.
//!
//------------------------------------------------------
void CellFsFile::Flush()
{
	// there doesn't appear to be a per file flush...
}

//------------------------------------------------------
//!
//! Returns the size of this file.
//!
//------------------------------------------------------
size_t CellFsFile::GetFileSize()
{
	CellFsStat stat;
	CellFsErrno err = cellFsFstat( m_iFileHandle, &stat );
	PrintCellFsError( err );

	return stat.st_size;
}
