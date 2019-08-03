#if !defined(CORE_FILE_H)
#define CORE_FILE_H

//-------------------------------------------------------
//!
//!	\file core\file.h
//!	Simple low level file system class.
//!	Designed for low level read write operations, not a 
//! fully produced file system
//!
//-------------------------------------------------------

#ifdef PLATFORM_PS3

#	define USE_FIOS_FOR_SYNCH_IO

#	ifndef USE_FIOS_FOR_SYNCH_IO
		class CellFsFile;
#	endif

	class CompressedFile;
	class CompressedArchive;
	class StandardArchive;

#endif

#include "core/mediatypes.h"
#include "core/mem.h"

//---------------------------------------------------
//!
//!	A opaque type to a file open disk.
//!	Has simple read/write/seek etc functionality
//!
//---------------------------------------------------
class File
{
public:
	enum FILE_TYPE
	{
		FT_TEXT		= (1<<0), //!< Open in text mode if OS supports
		FT_BINARY	= (1<<1), //!< Open in binary mode if OS supports
		FT_READ		= (1<<2), //!< Want to read from this file
		FT_WRITE	= (1<<3), //!< Want to write to this file
		FT_APPEND	= (1<<4), //!< Add to exists else create it
		FT_LOG		= (1<<5), //!< Special mode for debug logging files, ensures caches are turned off for this file (implies write mode)
	};

	//! default do nothing ctor 
	File();

	//------------------------------------------------
	//! ctor for opening read only the specified file
	//! \param pFileName filename to open/create
	//! \param uiType bit or'ed FILE_TYPE enum
	//------------------------------------------------
	File( const char* restrict pFileName, uint32_t uiType, Mem::MEMORY_CHUNK chunk = Mem::MC_MISC );

	//! dtor
	~File();

	//! opens the file of type
	void Open( const char* restrict pFileName, uint32_t uiType, Mem::MEMORY_CHUNK chunk = Mem::MC_MISC, uint32_t alignment = 1 );

	//! closes the file.
	void Close();

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
	enum SeekMode
	{
		SEEK_FROM_START = 0,
		SEEK_FROM_END,
		SEEK_FROM_CURRENT,

		NUM_SEEK_MODES
	};
	void Seek( int32_t offset, SeekMode mode );

	//! Gets the size of a file
	size_t GetFileSize();

	//! Flush any writes to disk.
	void Flush();

	//! Returns true if this file is already in memory because it has come from
	//! a compressed archive.
	bool IsCompressedFile() const
	{
#		ifdef PLATFORM_PC
			return false;
#		else
			return m_CompressedFile != NULL;
#		endif
	}

	//! If this File isn't representative of a compressed file from an archive then we return NULL.
	//! If it is then we return the UNCOMPRESSED data associated with the file. You can still use
	//! all the other functions with compressed archive data (so usage should be transparent) but
	//! this function avoids the implicit memcpy in a "read" call from a file that's essentially
	//! already in memory. The length of this data can be retrieved by calling GetFileSize().
	uint8_t *GetCompressedFileRawData();

	//! You can only call this function on a compressed-file. It tells the compressed-file not
	//! to release its memory in its destructor - i.e. it says that you will take ownership of
	//! its memory. You can get the memory by calling GetCompressedFileRawData(), the length
	//! of the memory is GetFileSize() bytes.
	void TakeOwnershipOfMemory();

private:
	//!	Prevent copying and assignment.
	File( const File & )				/*NOT_IMPLEMENTED*/;
	File &operator = ( const File & )	/*NOT_IMPLEMENTED*/;

private:

#	if defined(PLATFORM_PC)
		void*			m_pFileHandle;	//!< stdio file handle hidden to reduce dependecies
#	elif defined(PLATFORM_PS3)

		friend class CompressedArchive;
		friend class StandardArchive;
	
#		ifdef USE_FIOS_FOR_SYNCH_IO
			int				m_iFileHandle;
#		else
			CellFsFile*		m_pFileImpl;
#		endif

		// On PS3 a File open could open from a compressed archive.
		// In this case we need to read the file in and decompress;
		// all operations are then performed in memory.
		CompressedFile *	m_CompressedFile;

#	endif	// PLATFORM_PS3
};

//!
//!	Load an entire file into memory.
//!
//!		'filename'			:	The extended filename of the file you wish to load.							[INPUT]
//!		'open_flags'		:	The flags to pass to File::Open, i.e. FT_READ etc...						[INPUT]
//!		'file_out'			:	The default constructed file object to Open the file into.					[OUTPUT]
//!		'loaded_file_data'	:	Will be assigned to heap memory and have the file loaded into this memory.	[OUTPUT]
//!
//!	Example usage:
//!
//!		File my_file;
//!		uint8_t *my_data = NULL;
//!		LoadFile( "content_neutral/my_filename.ext", FT_READ | FT_BINARY, my_file, &my_data );
//!
//!	Notes:
//!
//!		- If you need a specific chunk, call LoadFile_Chunk instead.
//!		- If you need aligned memory then call LoadFile_ChunkMemAlign instead.
//!		- It is the callers responsibility to deallocate the heap memory returned in 'loaded_file_data'.
//!		- The deallocation method depends on which of the functions is called:
//!			o LoadFile				-->	Use NT_DELETE_ARRAY.
//!			o LoadFile_Chunk		-->	Use NT_DELETE_ARRAY_CHUNK, passing the same chunk you called with.
//!			o LoadFile_ChunkMemAlign-->	Use NT_FREE_CHUNK, passing the same chunk you called with.
//!
void 		LoadFile_Chunk			( const char *filename, uint32_t open_flags, Mem::MEMORY_CHUNK chunk, File &file_out, uint8_t **loaded_file_data );
void 		LoadFile_ChunkMemAlign	( const char *filename, uint32_t open_flags, Mem::MEMORY_CHUNK chunk, uint32_t alignment, File &file_out, uint8_t **loaded_file_data );
inline void	LoadFile				( const char *filename, uint32_t open_flags, File &file_out, uint8_t **loaded_file_data )
{
	LoadFile_Chunk( filename, open_flags, Mem::MC_MISC, file_out, loaded_file_data );
}


#endif // end CORE_FILE_H
