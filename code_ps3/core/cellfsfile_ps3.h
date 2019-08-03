//-------------------------------------------------------
//!
//!	\file core\cellfsfile_ps3.h
//!	CellFs version of File, now depricated and only
//! used for debug functionality as we have moved all
//! syncronous file IO to FIOS.
//!
//-------------------------------------------------------

#if !defined(CORE_CELLFSFILE_H)
#define CORE_CELLFSFILE_H

#ifndef CORE_FILE_H
#include "core/file.h"
#endif

//---------------------------------------------------
//!
//!	CellFsFile
//! Was the PS3 side implementation of File before
//! move to FIOS synchronous IO.
//!
//---------------------------------------------------
class CellFsFile
{
public:

	//! default do nothing ctor 
	CellFsFile();

	//------------------------------------------------
	//! ctor for opening read only the specified file
	//! \param pFileName filename to open/create
	//! \param uiType bit or'ed FILE_TYPE enum
	//------------------------------------------------
	CellFsFile( const char* restrict pFileName, uint32_t uiType );

	//! dtor
	~CellFsFile();

	//! opens the file of type
	void Open(  const char* restrict pFileName, uint32_t uiType );

	//! is this file valid (open correctly etc.)
	bool IsValid() const;

	//! does this file exist at all?
	static bool Exists( const char* pFileName );

	//! Write to disk.
	void Write( const void* pIn, size_t sizeToWrite );

	//! Read from disk.
	size_t Read( void* restrict pOut, size_t sizeToRead );

	//! Read from disk into an allocated buffer
	size_t AllocateRead( char** ppOut );

	//! Returns our current offset from the start of the file.
	size_t Tell() const;

	//! Sets our current offset from the start of the file for all subsequent operations.
	void Seek( int32_t offset, File::SeekMode mode );

	//! Gets the size of a file
	size_t GetFileSize();

	//! Flush any writes to disk.
	void Flush();

private:
	int		m_iFileHandle;
};

#endif // end CORE_CELLFSFILE_H
