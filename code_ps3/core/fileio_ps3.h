//-------------------------------------------------------
//!
//!	\file core\fileio_ps3.h
//! Entry place for FIOS functionality
//!
//-------------------------------------------------------

#ifndef _CORE_FILEIO_PS3_H
#define _CORE_FILEIO_PS3_H

#include "core/mediatypes.h"

class FileManager_impl;

#ifndef SEEK_SET
# define SEEK_SET   0
# define SEEK_CUR   1
# define SEEK_END   2
#endif

#ifndef O_RDONLY
# define O_RDONLY    0x0000
# define O_WRONLY    0x0001
# define O_RDWR      0x0002
# define O_ACCMODE   0x0003
# define O_APPEND    0x0008
# define O_CREAT     0x0200
# define O_TRUNC     0x0400
#endif

//-------------------------------------------------------
//!
//!	Class responsible for wrapping FIOS functionality.
//!
//-------------------------------------------------------
class FileManager
{
public:
	static void BasicInit		();
	static void GameDataInit	();
	static void SysCacheInit	();
	static void Kill			();

	static int		open			( const char *, int, MediaTypes media_type = DEFAULT_MEDIA );
	static int		openSubFile		( const char *, int, MediaTypes media_type, off_t offset, ssize_t filelength );	// A sub-file is a file within an archive.
	static off_t	lseek			( int d, off_t offset, int whence );
	static ssize_t 	read			( int d, void *buf, size_t nbytes );
	static ssize_t 	pread			( int d, void *buf, size_t nbytes, off_t offset );
	static ssize_t 	write			( int d, const void *buf, size_t nbytes );
	static ssize_t 	pwrite			( int d, const void *buf, size_t nbytes, off_t offset );
	static int		close			( int d );
	static bool		exists			( const char *pPath, MediaTypes media_type = DEFAULT_MEDIA );
	static off_t 	getFileSize		( int d );
	static off_t 	tell			( int d );

	static void	createDirectories	( const char *path, MediaTypes media_type );

private:
	static FileManager_impl* s_pImpl;
};

class FileEnumerator
{
	public:
		//
		//	Public interface.
		//
						// Returns true if there is a next file, false if we've finished.
		bool			AdvanceToNextFile		();

						// Returns the filename of the current file.
		const char *	GetCurrentFilename		()	const;

						// Is the current "file" actually a directory?
		bool			IsDirectory				()	const;

		uint32_t		GetCurrentFilenameLength()	const;

						// Returns true if the FileEnumerator object was created correctly.
		bool			IsValid					()	const	{ return m_File != -1; }

	public:
		//
		//	Ctor, dtor. "dirPath" is an extended filename.
		//
		explicit FileEnumerator( const char *dirPath, MediaTypes media_type = DEFAULT_MEDIA );
		~FileEnumerator();

	private:
		//
		//	Don't copy...
		//
		FileEnumerator( const FileEnumerator & )				NOT_IMPLEMENTED;
		FileEnumerator &operator = ( const FileEnumerator & )	NOT_IMPLEMENTED;

	private:
		//
		//	Private data.
		//
		void *		m_DirInfo;
		int32_t		m_File;
};

#endif // _CORE_FILEIO_PS3_H
