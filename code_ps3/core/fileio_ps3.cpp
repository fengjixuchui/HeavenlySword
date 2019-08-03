//-------------------------------------------------------
//!
//!	\file core\fileio_ps3.cpp
//! Entry place for FIOS functionality
//!
//-------------------------------------------------------

#include "core/fileio_ps3.h"
#include "core/cellfsfile_ps3.h"
#include "core/mem.h"
#include <fios/include/fios.h>
#include <sceacommon/include/sceamem.h>

#include "core/archive.h"
#include "game/shellconfig.h"

// globals
FileManager_impl* FileManager::s_pImpl = NULL;

// we need thi
namespace Util
{
	const char* GetAppHomePath();
}

//-------------------------------------------------------
//!
//!	AllocatorShim
//! Class that funcs through to our allocation system, 
//! used by FIOS.
//!
//-------------------------------------------------------
class AllocatorShim : public SCEA::Memory::Allocator
{
private:
	static const Mem::MEMORY_CHUNK s_FIOSMemChunk = Mem::MC_MISC;

public:
	AllocatorShim()
	{};

	virtual ~AllocatorShim()
	{};

#ifdef _HAVE_MEMORY_TRACKING

	virtual void* Allocate( SCEA::Memory::MemSize size, SCEA::Memory::MemFlags flags, const char* file = 0, int line = 0 )
    {
		uint32_t alignment = MEM_ALIGN_FROM_FLAGS( flags );
		uintptr_t result = Mem::MemAlign( s_FIOSMemChunk, size, alignment, file, NT_FUNC_NAME, line, true );
		return (void*) result;
    }

	virtual void Deallocate( void* pMemory, SCEA::Memory::MemFlags, const char* file = 0, int line = 0)
	{
		uintptr_t toDelete = (uintptr_t)pMemory;
		Mem::Free( s_FIOSMemChunk, toDelete, file, NT_FUNC_NAME, line );
	}

	virtual void* Reallocate( void* pMemory, SCEA::Memory::MemSize newSize, SCEA::Memory::MemFlags flags, const char* = 0, int = 0 )
	{
		ntError_p( 0, ("Reallocate not supported") );
		UNUSED(pMemory);
		UNUSED(newSize);
		UNUSED(flags);
		return NULL;
	}

#else

	virtual void* Allocate( SCEA::Memory::MemSize size, SCEA::Memory::MemFlags flags, const char* = 0, int = 0 )
    {
		uint32_t alignment = MEM_ALIGN_FROM_FLAGS( flags );
		uintptr_t result = Mem::MemAlign( s_FIOSMemChunk, size, alignment, true );
		return (void*) result;
    }

	virtual void Deallocate( void* pMemory, SCEA::Memory::MemFlags, const char* = 0, int = 0)
	{
		uintptr_t toDelete = (uintptr_t)pMemory;
		Mem::Free( s_FIOSMemChunk, toDelete );
	}

	virtual void* Reallocate( void* pMemory, SCEA::Memory::MemSize newSize, SCEA::Memory::MemFlags flags, const char* = 0, int = 0 )
	{
		ntError_p( 0, ("Reallocate not supported") );
		UNUSED(pMemory);
		UNUSED(newSize);
		UNUSED(flags);
		return NULL;
	}

#endif
};

//-------------------------------------------------------
//!
//!	FileManager_impl
//! Implementation of FIOS functionality
//!
//-------------------------------------------------------
class FileManager_impl
{
public:
//	static const int32_t NUMTHREADS = 30;

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::ctor
	//!
	//-------------------------------------------------------
	FileManager_impl()
	{
		m_pAllocator = NT_NEW AllocatorShim;

		// Initialize FIOS. We pass in an allocator object.
		fios::FIOSInit(m_pAllocator);

		Startup();

		NT_NEW ArchiveManager();
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::dtor
	//!
	//-------------------------------------------------------
	~FileManager_impl()
	{
		ArchiveManager::Kill();

		Shutdown();
		NT_DELETE( m_pAllocator );
	}

	//-------------------------------------------------------
	//!
	//!	POSIX file IO functions.
	//! These are ripped wholesale from the POSIX fios example
	//!
	//-------------------------------------------------------

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::open
	//!
	//-------------------------------------------------------
	static int open( const char *pPath, int flags, MediaTypes media_type, off_t offset, ssize_t filelength = -1 )
	{
		ntError_p( media_type >= 0 && media_type < NUM_MEDIA_TYPES, ("Invalid media type.") );

		U32 fiosOpenFlags =
			(((flags & O_ACCMODE) == O_RDONLY) ? fios::kO_RDONLY:0) |
			(((flags & O_ACCMODE) == O_WRONLY) ? fios::kO_WRONLY:0) |
			(((flags & O_ACCMODE) == O_RDWR) ? fios::kO_RDWR:0) |
			((flags & O_APPEND) ? fios::kO_APPEND:0) |
			((flags & O_CREAT) ? fios::kO_CREAT:0) |
			((flags & O_TRUNC) ? fios::kO_TRUNC:0);
		
		// Open the FIOS fh.
		fios::filehandle *pFH = NULL;
		fios::scheduler *pScheduler = g_Schedulers[ media_type ];
		if ( pScheduler == NULL )
			return -1;

#		ifndef _RELEASE
		{
			// Check max path length.
			if ( strlen( pPath ) > 900 )
			{
				user_warn_msg( ("The following path exceeds the maximum path length (900 characters): %s\nThe file will NOT be opened.", pPath) );
				return -1;
			}

			// Extract actual filename.
			const char *filename = NULL;

			const char *temp = pPath;
			while ( (temp = strstr( temp, "/" )) )
			{
				temp++;
				filename = temp;
			}

			if ( strlen( filename ) > 58 )
			{
				user_warn_msg( ("The following filename exceeds the maximum filename length (57 characters): %s\nThe file will NOT be opened.", pPath) );
				return -1;
			}
		}
#		endif

		fios::err_t err = pScheduler->openFileSync(NULL,pPath,fiosOpenFlags,&pFH);
		if (err != fios::kNOERR)
			return -1;
		
		// Stash it in our file table, growing it if necessary.
		if (g_FileTableLock == NULL) initfdlock();
		fios::platform::StWriteLocker lock(g_FileTableLock);
		int fd = 0;

		if ( filelength == -1 )
		{
			ntError( pFH != NULL );
			filelength = pFH->getFileSize();
		}

		// Search
		for ( fd=0; fd<g_FileTable.numEntries; ++fd )
		{
			if ( g_FileTable.pEntries[ fd ].pFH == NULL )
			{
				g_FileTable.pEntries[ fd ].Initialise( pFH, offset, filelength );

				return ( fd + g_FileSkip );
			}
		}

		// Grow
		int oldSize = g_FileTable.numEntries;
		if ((oldSize == 0) || (g_FileTable.pEntries == NULL))
		{
			g_FileTable.numEntries = 4;
			g_FileTable.pEntries = reinterpret_cast<FileTableEntry*>(::calloc(sizeof(FileTableEntry),(size_t)g_FileTable.numEntries));
		}
		else
		{
			g_FileTable.numEntries += 4;
			FileTableEntry *pNewAllocation = reinterpret_cast<FileTableEntry*>(::calloc(sizeof(FileTableEntry),(size_t)g_FileTable.numEntries));
			FileTableEntry *pOldAllocation = g_FileTable.pEntries;
			NT_MEMCPY(pNewAllocation, pOldAllocation, oldSize * sizeof(FileTableEntry));
			g_FileTable.pEntries = pNewAllocation;
			::free(pOldAllocation);
		}
		
		// Setup the FileTableEntry structure.
		g_FileTable.pEntries[ oldSize ].Initialise( pFH, offset, filelength );

		// Return new entry index.
		return (oldSize + g_FileSkip);
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::lseek
	//!
	//-------------------------------------------------------
	static off_t lseek( int fd, off_t offset, int whence )
	{
		off_t subfile_offset = 0;
		ssize_t subfile_length = 0;
		fios::filehandle *pFH = fdlookup( fd, &subfile_offset, &subfile_length );
		if ( pFH == NULL )
		{
			return off_t( -1 );
		}
		
		// We must modify the offset based on our subfile data.
		// Once we've done this we have an offset that should always be used
		// with the SEEK_SET mode.
		switch ( whence )
		{
			case SEEK_CUR:
			{
				// Adding on the current file-handle offset makes it like SEEK_SET...
				offset += pFH->getOffset();

				// Drop through to SEEK_SET case...
			}

			case SEEK_SET:
			{
				offset += subfile_offset;
				break;
			}

			case SEEK_END:
			{
				offset += subfile_offset + subfile_length;
				break;
			}

			default:
			{
				return off_t( -1 );
			}
		}

		// Check bounds on the sub-file range.
		if ( offset < subfile_offset || offset > subfile_length + subfile_offset )
		{
			return off_t( -1 );
		}

		// The "result" variable returned is the absolute offset so we need to subtract
		// our subfile offset from it before returning.
		fios::off_t result;
		return ( pFH->seek( offset, fios::kSEEK_SET, &result ) == fios::kNOERR ) ? off_t( result - subfile_offset ) : off_t( -1 );
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::read
	//!
	//-------------------------------------------------------
	static ssize_t read(int fd, void *buf, size_t nbytes)
	{
		fios::filehandle *pFH = fdlookup(fd);
		if (pFH == NULL) return (ssize_t)-1;
		return (pFH->readSync(NULL,buf,nbytes) == fios::kNOERR) ? (ssize_t)nbytes:(ssize_t)-1;
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::pread
	//!
	//-------------------------------------------------------
	static ssize_t pread(int fd, void *buf, size_t nbytes, off_t offset)
	{
		off_t subfile_offset = 0;
		fios::filehandle *pFH = fdlookup(fd, &subfile_offset);
		if (pFH == NULL) return (ssize_t)-1;
		return (pFH->preadSync(NULL,buf,nbytes,offset+subfile_offset) == fios::kNOERR) ? (ssize_t)nbytes:(ssize_t)-1;
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::write
	//!
	//-------------------------------------------------------
	static ssize_t write(int fd, const void *buf, size_t nbytes)
	{
		fios::filehandle *pFH = fdlookup(fd);
		if (pFH == NULL) return (ssize_t)-1;
		return (pFH->writeSync(NULL,buf,nbytes) == fios::kNOERR) ? (ssize_t)nbytes:(ssize_t)-1;
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::pwrite
	//!
	//-------------------------------------------------------
	static ssize_t pwrite(int fd, const void *buf, size_t nbytes, off_t offset)
	{
		off_t subfile_offset = 0;
		fios::filehandle *pFH = fdlookup(fd, &subfile_offset);
		if (pFH == NULL) return (ssize_t)-1;
		return (pFH->pwriteSync(NULL,buf,nbytes,offset+subfile_offset) == fios::kNOERR) ? (ssize_t)nbytes:(ssize_t)-1;
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::close
	//!
	//-------------------------------------------------------
	static int close(int fd)
	{
		fios::filehandle *pFH = fdlookup(fd);
		if (pFH == NULL) return (ssize_t)-1;
		fios::err_t err = pFH->getScheduler()->closeFileSync(NULL,pFH);
		if (err == fios::kNOERR)
		{
			// Clear out the entry in the file table.
			if (g_FileTableLock == NULL) initfdlock();
			fios::platform::StWriteLocker lock(g_FileTableLock);
			g_FileTable.pEntries[fd - g_FileSkip].pFH = NULL;
			return 0;
		}
		else
			return -1;
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::exists
	//!
	//-------------------------------------------------------
	static bool exists( const char *pPath, MediaTypes media_type )
	{
		ntError_p( media_type >= 0 && media_type < NUM_MEDIA_TYPES, ("Invalid media type.") );

		fios::scheduler* pSched = g_Schedulers[ media_type ];
		if ( pSched == NULL )
		{
			return false;
		}

		bool exists = false;
		if ( pSched->fileExistsSync(NULL, pPath, &exists) != fios::kNOERR )
			return false;
		return exists;
	}

	//-------------------------------------------------------
	//!
	//!	FileManager_impl::getFileSize
	//!
	//-------------------------------------------------------
	static off_t getFileSize (int fd)
	{
		ssize_t subfile_length = 0;
		fios::filehandle *pFH = fdlookup(fd, NULL, &subfile_length);
		if (pFH == NULL) return 0;

		return subfile_length;
	}

	static off_t tell( int fd )
	{
		off_t subfile_offset = 0;
		fios::filehandle *pFH = fdlookup( fd, &subfile_offset );
		if ( pFH == NULL )
		{
			return 0;
		}

		// We must subtract the subfile_offset to give us a file-offset
		// relative to the subfile.
		return pFH->getOffset() - subfile_offset;
	}

	//-------------------------------------------------------
	//!
	//!	Startup function - inits the filesystem.
	//!
	//-------------------------------------------------------

	void GameDataInit()
	{
		// these are howmany to statically allocate, and can be exceeded using dynamic allocation.
		// When we load a single packed WAD rather than textures and clumps individually, these will decrease
		uint32_t iNumOperations = 100;
		uint32_t iNumFiles = 100;

		// transparently use app home if we're not using the harddrive
		const char* pGameDataPath = g_ShellOptions->m_bUsingHDD ? Util::GetGameDataPath() : Util::GetAppHomePath();
		ntError_p( pGameDataPath != NULL, ("Must have a valid game data path by now") );

		// Create a media object. This will be allocated from the global FIOS memory pool.
		// This will be in the game-data area of the hard-drive.
		ntPrintf( "ntFIOS: GameDataInit() Mounting %s\n", pGameDataPath );
		fios::ps3media* pGameDataMedia = new fios::ps3media( pGameDataPath );	

		// Create a new scheduler and replace the old default scheduler with this one
		fios::scheduler* pGameDataSched = fios::scheduler::createSchedulerForMedia( pGameDataMedia, NULL, 0, iNumOperations, iNumFiles, 0 );

		g_OldDefault = g_Schedulers[ DEFAULT_MEDIA ];
		ntError_p( g_OldDefault != NULL, ("Must have had and old media layer") );
		
		g_Schedulers[ DEFAULT_MEDIA ] = pGameDataSched;
		pGameDataSched->setDefault();

		// Log files attempt to create themselves on the hard-drive but assume
		// that the directory structure is already in place...
#		ifndef _GOLD_MASTER
		{
			if ( g_ShellOptions->m_bUsingHDD )
			{
				// ...Log files don't exist in gold-master builds, so we create
				// the directory structure explicitly here.
				char directory_path[ MAX_PATH ];
				sprintf( directory_path, "%scontent_neutral", pGameDataPath );

				int32_t ret = cellFsMkdir( directory_path, CELL_FS_DEFAULT_CREATE_MODE_1 );
				ntError( ret == CELL_OK || ret == CELL_FS_ERROR_EEXIST );

				sprintf( directory_path, "%scontent_ps3", pGameDataPath );

				ret = cellFsMkdir( directory_path, CELL_FS_DEFAULT_CREATE_MODE_1 );
				ntError( ret == CELL_OK || ret == CELL_FS_ERROR_EEXIST );
				UNUSED( ret );
			}
		}
#		endif
	}

	void SysCacheInit()
	{
		// these are howmany to statically allocate, and can be exceeded using dynamic allocation.
		// When we load a single packed WAD rather than textures and clumps individually, these will decrease
		uint32_t iNumOperations = 100;
		uint32_t iNumFiles = 100;

		const char* pSysCachePath = Util::GetSysCachePath();
		ntError_p( pSysCachePath != NULL, ("Must have a valid system cache path by now") );

		ntError( g_Schedulers[ SYSCACHE_MEDIA ] == NULL );

		ntPrintf( "ntFIOS: SysCacheInit() Mounting %s\n", pSysCachePath );
		fios::ps3media* sysCacheMedia = new fios::ps3media( pSysCachePath );
		fios::scheduler* sysCacheScheduler = fios::scheduler::createSchedulerForMedia( sysCacheMedia, NULL, 0, iNumOperations, iNumFiles, 0 );

		g_Schedulers[ SYSCACHE_MEDIA ] = sysCacheScheduler;
	}

	static const char* GetMediaPath( MediaTypes media_type )
	{
		if ( g_Schedulers[ media_type ] )
			return g_Schedulers[ media_type ]->getMedia()->getPrimaryIdentifier();
		return NULL;
	}

private:
	void Startup()
	{
		// our inital default scheduler will use the bluray or app home.
		// GetBluRayPath will be transparently changed before here
		ntPrintf( "ntFIOS: Startup() Mounting %s\n", Util::GetBluRayPath() );
		fios::ps3media* pDefaultMedia = new fios::ps3media( Util::GetBluRayPath() );
	
		// Create a scheduler. This will be allocated from the global FIOS memory pool.

		// these are howmany to statically allocate, and can be exceeded using dynamic allocation.
		// When we load a single packed WAD rather than textures and clumps individually, these will decrease
		uint32_t iNumOperations = 100;
		uint32_t iNumFiles = 100;

		fios::scheduler* pDefualtSched = fios::scheduler::createSchedulerForMedia( pDefaultMedia, NULL, 0, iNumOperations, iNumFiles, 0 );

		ntError( g_Schedulers[ DEFAULT_MEDIA ] == NULL );
		g_Schedulers[ DEFAULT_MEDIA ] = pDefualtSched;

		// Set it as the default. All of our pseudo-POSIX calls go through the default
		// scheduler if they do not have a device path.
		pDefualtSched->setDefault();

		//
		//	Create a media layer and scheduler for the BluRay drive.
		//
		ntError( g_Schedulers[ BLU_RAY_MEDIA ] == NULL );

		// this is transparently app home when working without a bluray
		fios::ps3media *bluRayMedia = new fios::ps3media( Util::GetBluRayPath() );
		fios::scheduler *bluRayScheduler = fios::scheduler::createSchedulerForMedia( bluRayMedia, NULL, 0, iNumOperations, iNumFiles, 0 );
		g_Schedulers[ BLU_RAY_MEDIA ] = bluRayScheduler;
	}

	//-------------------------------------------------------
	//!
	//!	Shuts down the filesystem.
	//!
	//-------------------------------------------------------
	void Shutdown()
	{
		// Destroy the default scheduler and media object.
		// For some obscure reason we can't destroy the default
		// scheduler in the for loop with the other schedulers.
		{
			fios::scheduler* pSched = fios::scheduler::getDefaultScheduler();
			ntError( pSched == g_Schedulers[ DEFAULT_MEDIA ] );
			g_Schedulers[ DEFAULT_MEDIA ] = NULL;

			fios::media* pMedia = pSched->getMedia();

			fios::scheduler::destroyScheduler( pSched );
			delete pMedia;
		}

		// Destroy all the media and scheduler objects.
		static_assert( DEFAULT_MEDIA == 0, Must_be_the_first_media_type );
		for ( uint32_t i=DEFAULT_MEDIA+1;i<NUM_MEDIA_TYPES;i++ )
		{
			if ( g_Schedulers[ i ] == NULL )
			{
				continue;
			}

			fios::media *pMedia = g_Schedulers[ i ]->getMedia();

			fios::scheduler::destroyScheduler( g_Schedulers[ i ] );
			g_Schedulers[ i ] = NULL;

			delete pMedia;
		}

		if (g_OldDefault)
		{
			fios::media *pMedia = g_OldDefault->getMedia();

			fios::scheduler::destroyScheduler( g_OldDefault );
			g_OldDefault = NULL;

			delete pMedia;
		}

		delete g_FileTableLock;
	}

private:
	AllocatorShim*	m_pAllocator;

	//-------------------------------------------------------
	//!
	//!	Structs used in POSIX like functions
	//!
	//-------------------------------------------------------
	struct FileTableEntry
	{
		// The file-handle for this entry's corresponding file.
		fios::filehandle *pFH;

		// These are used to describe files that are a sub-section of an archive.
		off_t offset;
		off_t filelength;

		// Setup the FileTableEntry structure.
		void Initialise( fios::filehandle *fh, off_t offs, off_t length )
		{
			ntError_p( fh != NULL, ("You cannot initialise a FileTableEntry with a NULL filehandle.") );
			pFH = fh;
			offset = offs;

			// If our filelength is -1 then take the length directly from the file-handle,
			// otherwise use the passed in value.
			filelength = length == -1 ? fh->getFileSize() : length;

			// If our offset is non-zero then we need to seek to this offset initially.
			if ( offs != 0 )
			{
				fh->seek( offs );
			}
		}
	};

	struct FileTable
	{
		int numEntries;
		FileTableEntry *pEntries;
	};

	//-------------------------------------------------------
	//!
	//!	Helper functions for POSIX file IO implementations
	//!
	//-------------------------------------------------------
	static void initfdlock()
	{
		fios::platform::rwlock* rw = new fios::platform::rwlock("g_FileTableLock");
		if (!atomicCompareAndSwap((const void * volatile *)&g_FileTableLock,NULL,rw))
			delete rw ;
	}

	static fios::filehandle* fdlookup(int fd, off_t *offset = NULL, ssize_t *filelength = NULL )
	{
		if (g_FileTableLock == NULL)
			initfdlock();

		fios::platform::StReadLocker lock(g_FileTableLock);
		fd -= g_FileSkip;

		if ( fd < 0 || fd >= g_FileTable.numEntries )
		{
			return NULL;
		}

		if ( offset != NULL )
		{
			*offset = g_FileTable.pEntries[ fd ].offset;
		}
		if ( filelength != NULL )
		{
			*filelength = g_FileTable.pEntries[ fd ].filelength;
		}

		return g_FileTable.pEntries[ fd ].pFH;
	}

	//-------------------------------------------------------
	//!
	//!	Globals used in POSIX like functions
	//!
	//-------------------------------------------------------
	static fios::platform::rwlock*	g_FileTableLock;
	static FileTable				g_FileTable;
	static int						g_FileSkip;

	static fios::scheduler *		g_Schedulers[ NUM_MEDIA_TYPES ];
	static fios::scheduler *		g_OldDefault;
};

fios::platform::rwlock*			FileManager_impl::g_FileTableLock = NULL;
FileManager_impl::FileTable		FileManager_impl::g_FileTable = {0,NULL};
int								FileManager_impl::g_FileSkip = 3;
fios::scheduler *				FileManager_impl::g_Schedulers[ NUM_MEDIA_TYPES ] = { NULL, NULL, NULL };
fios::scheduler *				FileManager_impl::g_OldDefault = NULL;




//-------------------------------------------------------
//!
//!	FileManager
//! Class that exposes FIOS functionality
//!
//-------------------------------------------------------
void FileManager::BasicInit()
{
	s_pImpl = NT_NEW FileManager_impl();
}

void FileManager::GameDataInit()
{
	s_pImpl->GameDataInit();
}

void FileManager::SysCacheInit()
{
	s_pImpl->SysCacheInit();
}

void FileManager::Kill()
{
	NT_DELETE( s_pImpl );
}

int FileManager::open( const char *pPath, int flags, MediaTypes media_type/* = DEFAULT_MEDIA*/ )
{
	ntAssert_p( s_pImpl, ("Must have called FileManager::Init() before now") );
	return s_pImpl->open( pPath, flags, media_type, 0 );
}

int FileManager::openSubFile( const char *pPath, int flags, MediaTypes media_type, off_t offset, off_t filelength )
{
	ntAssert_p( s_pImpl, ("Must have called FileManager::Init() before now") );
	return s_pImpl->open( pPath, flags, media_type, offset, filelength );
}

off_t FileManager::lseek(int d, off_t offset, int whence)
{
	ntAssert_p( s_pImpl, ("Must have called FileManager::Init() before now") );
	return s_pImpl->lseek(d, offset, whence);
}

ssize_t FileManager::read(int d, void *buf, size_t nbytes)
{
	ntAssert_p( s_pImpl, ("Must have called FileManager::Init() before now") );
	return s_pImpl->read(d, buf, nbytes);
}

ssize_t FileManager::pread(int d, void *buf, size_t nbytes, off_t offset)
{
	ntAssert_p( s_pImpl, ("Must have called FileManager::Init() before now") );
	return s_pImpl->pread(d, buf, nbytes, offset);
}

ssize_t FileManager::write(int d, const void *buf, size_t nbytes)
{
	ntAssert_p( s_pImpl, ("Must have called FileManager::Init() before now") );
	return s_pImpl->write(d, buf, nbytes);
}

ssize_t FileManager::pwrite(int d, const void *buf, size_t nbytes, off_t offset)
{
	ntAssert_p( s_pImpl, ("Must have called FileManager::Init() before now") );
	return s_pImpl->pwrite(d, buf, nbytes, offset);
}

int FileManager::close(int d)
{
	ntAssert_p( s_pImpl, ("Must have called FileManager::Init() before now") );
	return s_pImpl->close(d);
}

bool FileManager::exists(const char *pPath, MediaTypes media_type/* = DEFAULT_MEDIA*/)
{
	return FileManager_impl::exists(pPath,media_type);
}

off_t FileManager::getFileSize (int d)
{
	return s_pImpl->getFileSize(d);
}

off_t FileManager::tell( int d )
{
	return s_pImpl->tell( d );
}

namespace Util
{
	extern void StringToLowerInplace( char* pString );
}

void FileManager::createDirectories( const char *filename_arg, MediaTypes media_type )
{
	ntError( media_type == DEFAULT_MEDIA || media_type == SYSCACHE_MEDIA );

	extern const char *g_GameDataPath;
	const char *root = g_GameDataPath;
	if ( media_type == SYSCACHE_MEDIA )
	{
		extern const char *g_SysCachePath;
		root = g_SysCachePath;
	}

	// We don't want the root to be present.
	ntError( strstr( filename_arg, root ) == NULL );

	char filename[ 2048 ];
	strcpy( filename, filename_arg );

	Util::StringToLowerInplace( filename );
	
	char *pcNext = strstr( filename, "\\" );
	
	while ( pcNext )
	{
		*pcNext = '/';
		pcNext = strstr( pcNext, "\\" );
	}

	// Strip off the filename and leave the path.
	int32_t idx = (int32_t)strlen( filename ) - 1;
	while ( filename[ idx ] != '/' && filename[ idx ] != '\\' && idx > 0 )
	{
		idx--;
	}
	filename[ idx ] = '\0';

	char *next_dir = filename;
	char *dirname = filename;
	do
	{
		next_dir = strstr( next_dir, "/" );
		if ( next_dir != NULL )
		{
			*next_dir = '\0';
		}

		// Create directory "dirname".
		char fulldir[ 2048 ];
		sprintf( fulldir, "%s%s", root, dirname );

		// Make the directory...
		int32_t ret = cellFsMkdir( fulldir, CELL_FS_DEFAULT_CREATE_MODE_1 );
		ntError( ret == CELL_OK || ret == CELL_FS_ERROR_EEXIST );
		UNUSED( ret );


		if ( next_dir != NULL )
		{
			*next_dir++ = '/';
		}
	}
	while ( next_dir != NULL );
}

//-------------------------------------------------------
//-------------------------------------------------------
//!
//!	FileEnumerator implementation.
//!
//-------------------------------------------------------
//-------------------------------------------------------

//-------------------------------------------------------
//!
//!	AdvanceToNextFile.
//!
//-------------------------------------------------------
bool FileEnumerator::AdvanceToNextFile()
{
	if ( m_File == -1 )
	{
		return false;
	}

	uint64_t rd = 0;
	CellFsDirent *dirInfo = (CellFsDirent *)m_DirInfo;
	cellFsReaddir( m_File, dirInfo, &rd ); // Get the next file in the current directory

	return rd != 0;
}

//-------------------------------------------------------
//!
//!	GetCurrentFilename.
//!
//-------------------------------------------------------
const char *FileEnumerator::GetCurrentFilename() const
{
	if ( m_DirInfo == NULL || m_File == -1 )
	{
		return NULL;
	}

	ntError( m_DirInfo != NULL );
	return ( (CellFsDirent *)m_DirInfo )->d_name;
}

//-------------------------------------------------------
//!
//!	GetCurrentFilenameLength.
//!
//-------------------------------------------------------
uint32_t FileEnumerator::GetCurrentFilenameLength() const
{
	if ( m_DirInfo == NULL || m_File == -1 )
	{
		return NULL;
	}

	ntError( m_DirInfo != NULL );
	return ( (CellFsDirent *)m_DirInfo )->d_namlen;
}

//-------------------------------------------------------
//!
//!	IsDirectory.
//!
//-------------------------------------------------------
bool FileEnumerator::IsDirectory() const
{
	if ( m_DirInfo == NULL || m_File == -1 )
	{
		return false;
	}

	ntError( m_DirInfo != NULL );
	return ( (CellFsDirent *)m_DirInfo )->d_type == CELL_FS_TYPE_DIRECTORY;
}

//-------------------------------------------------------
//!
//!	Ctor.
//!
//-------------------------------------------------------
FileEnumerator::FileEnumerator( const char *dirPath, MediaTypes media_type /* = DEFAULT_MEDIA  */)
:	m_DirInfo	( NT_NEW CellFsDirent )
,	m_File		( -1 )
{
	char full_path[ 1024 ];
	switch ( media_type )
	{
		case BLU_RAY_MEDIA:
			Util::GetFullBluRayFilePath( dirPath, full_path, false );
			break;

		case DEFAULT_MEDIA:
			Util::GetFullGameDataFilePath( dirPath, full_path, false );
			break;

		case SYSCACHE_MEDIA:
			Util::GetFullSysCacheFilePath( dirPath, full_path, false );
			break;

		default:
			ntError_p( false, ("Invalid media-type.") );
			m_File = -1;
			return;
	}

	//
	//	IMPORTANT!
	//		FILE ENUMERATOR IS USED BY FUNCTIONS THAT NEED TO OPERATE *BEFORE*
	//		FIOS IS SETUP APPROPRIATE SO DON'T EVEN THINK ABOUT CHANGING IT TO
	//		USE FIOS - IT *MUST* USE CELL FS CALLS.
	//
	if ( cellFsOpendir( full_path, &m_File ) != CELL_FS_SUCCEEDED )
	{
		m_File = -1;
	}
}

//-------------------------------------------------------
//!
//!	Dtor.
//!
//-------------------------------------------------------
FileEnumerator::~FileEnumerator()
{
	if ( m_File != -1 )
	{
		cellFsClosedir( m_File );
	}

	NT_DELETE( (CellFsDirent *)m_DirInfo );
}
