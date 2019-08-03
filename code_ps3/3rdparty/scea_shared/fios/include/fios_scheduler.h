/**
	\file fios_scheduler.h

	Class definitions for the fios::scheduler class.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_scheduler
#define _H_fios_scheduler

#include "fios_types.h"
#include "fios_base.h"
#include "fios_platform.h"
#include "fios_media.h"
#include "fios_op.h"
#include "fios_filehandle.h"
#include "fios_stream.h"

namespace fios {

class streampage;

/**
* \addtogroup FIOSScheduler	Scheduler
* @{
*/
/** \brief This class is the main entry point for clients of FIOS.
	All accesses that generate I/O are available as calls to the scheduler.
	The scheduler is able to queue a large number of requests and optimize
	their order to complete them all in the shortest amount of time possible.
	
	There is exactly one scheduler object for each media object, and there is
	normally one media object for each physical drive connected to the machine.
	Schedulers act independently of each other; if more than one scheduler has
	been instantiated (because more than one media object is available), then
	all the schedulers will run their I/O in parallel.
	
	The main application is responsible for creating and configuring scheduler
	objects. You should create a scheduler for the media where your game files
	reside, and then make it the default scheduler with scheduler::setDefault().
	This is also your chance to set up the media layer and instantiate any cache
	or emulation layers you want.
*/
class FIOS_EXPORT scheduler : public object
{
public:
		/** \brief Default values for scheduler parameters */
		enum e_DEFAULTS {
			kDefaultNumOps = 128,         /**< Number of operations that are statically allocated by default. */
			kDefaultNumFiles = 32,        /**< Number of filehandles that are statically allocated by default. */
			kDefaultNumStreams = 32,      /**< Number of stream handles that are statically allocated by default. */
		};
		
		/** \brief Factory method to create a scheduler object.
			Normally only the main application creates scheduler objects, and it does so exactly once for each type
			of media that it will access. Middleware should use getSchedulerForMedia() or getSchedulerForMediaIdentifier()
			instead.

			This call will fail and return NULL if the media already has a scheduler associated with it.
			\param[in] pMedia      Media object.
			\param[in] pAllocator  Memory allocator that this scheduler should use. May be NULL, in which case the scheduler will use the FIOS global allocator specified in FIOSInit().
			\param[in] sizeOfManagedBufferSpace   Memory to reserve for stream buffer management. May be 0 if you will only use unmanaged streams.
			\param[in] numOps      Number of operations to statically allocate. Additional operations may be allocated, but will fall back to dynamic allocation.
			\param[in] numFiles    Number of filehandles to statically allocate. Additional filehandles may be allocated, but will fall back to dynamic allocation.
			\param[in] numStreams  Number of streams to statically allocate. Additional streams may be allocated, but will fall back to dynamic allocation.
			\return Newly-allocated scheduler. This scheduler may be retrieved later with getSchedulerForMedia() or getSchedulerForMediaIdentifier().
		*/
		static scheduler * createSchedulerForMedia(
			media *pMedia,
			FIOS_ALLOCATOR *pAllocator = 0, // see fios_configuration.h
			size_t sizeOfManagedBufferSpace = 0,
			U32 numOps = kDefaultNumOps,
			U32 numFiles = kDefaultNumFiles,
			U32 numStreams = kDefaultNumStreams);
		
		/** \brief Factory method to find a scheduler object.
			\param[in] pMedia       Media object.
			\return A scheduler which can be used to access the media, or NULL if no scheduler is available for that media.
		*/
		static inline scheduler * getSchedulerForMedia(media *pMedia) { return (pMedia == NULL) ? NULL:pMedia->getScheduler(); }
		
		/** \brief Factory method to find a scheduler object from a media identifier.
			\param[in] pIdentifier   Media identifier.
			\return A scheduler which can be used to access the media, or NULL if no media with that identifier exists or no scheduler is available for it.
		*/
		static inline scheduler * getSchedulerForMediaIdentifier(const char *pIdentifier) { return getSchedulerForMedia(media::getMediaForIdentifier(pIdentifier)); }
		
		/** \brief Destroys and deallocates a scheduler object allocated by a factory method.
			If there are any requests in the queue, the scheduler may need some time to shut down. Any
			currently executing I/O operations are allowed to finish. Any pending but not yet executed
			I/O requests are immediately cancelled as if by op::cancel(). The scheduler will
			refuse any further I/O that might be requested from the callbacks.
			\param[in] pScheduler    Scheduler object to destroy.
		*/
		static void destroyScheduler(
			scheduler *pScheduler);
		
		/** \brief Factory method to retrieve the default scheduler.
			\return The default scheduler, or NULL if no default scheduler has been set.
		*/
		static scheduler * getDefaultScheduler();
		
		/** \brief Sets the default scheduler.
			\param[in] pScheduler    The new scheduler to become the default. (may be NULL)
		*/
		static void setDefaultScheduler(scheduler *pScheduler);
		
		/** \brief Sets this scheduler as the default scheduler.
			\see setDefaultScheduler()
		*/
		inline void setDefault() { scheduler::setDefaultScheduler(this); }

protected:
		FIOS_OBJECT_NEW_AND_DELETE(SCEA::Memory::kMemPurposeFIOSMediaObject|SCEA::Memory::kPoolIDLongTerm)

		/** \internal
			\brief Creates a scheduler object.
			Clients should not use this method directly. Instead they should use either createSchedulerForMedia(),
			getSchedulerForMedia(), or getSchedulerForMediaIdentifier().
			\param[in] pMedia        Media object to control. This media object must not already have an associated scheduler.
			\param[in] pAllocator    Memory allocator.
			\param[in] sizeOfManagedBufferSpace   Amount of memory to reserve for managed buffers.
			\param[in] numOps        Number of operations to statically allocate.
			\param[in] numFiles      Number of filehandles to statically allocate.
			\param[in] numStreams    Number of streams to statically allocate.
			\see scheduler::getDefaultScheduler, scheduler::getSchedulerForMedia, scheduler::getSchedulerForMediaIdentifier
		*/
		        scheduler(
		            media *pMedia,
					FIOS_ALLOCATOR *pAllocator, // see fios_configuration.h
					size_t sizeOfManagedBufferSpace,
					U32 numOps,
					U32 numFiles,
					U32 numStreams);

		/** \brief Destroys a scheduler object.
		*/
		virtual ~scheduler();
public:
		/** \brief Gets the maximum possible number of operations in-flight before dynamic allocation will be used.
			\return Number of operations
		*/
		inline U32 getMaxOpCount() const { return m_numStaticOps; }
		
		/** \brief Gets the media object that was passed in at creation time.
			\return Media object
		*/
		inline media * getMedia() const { return m_pMedia; }
		
		/** \brief Gets the allocator associated with the scheduler.
			\return Memory allocator
		*/
		inline FIOS_ALLOCATOR * getAllocator() const { return m_pAllocator; } // see fios_configuration.h
		
		/** \brief Gets the effective allocator associated with the scheduler.
			\return If non-NULL, the scheduler's allocator. Otherwise the global allocator is returned.
		*/
		FIOS_ALLOCATOR * getEffectiveAllocator() const;
		
		/** \brief Checks to see whether the scheduler is idle.

			A scheduler is considered idle if no I/O is being processed. A scheduler may be
			idle even if file handles and streams are open, as long as they are idle and no
			I/O is scheduled for any of them. A suspended scheduler is not considered idle
			if there is pending I/O that has been suspended.
			
			\return              True if the scheduler is idle, false otherwise.
		*/
		bool isIdle() const;
		
		/** \brief   Temporarily suspends a scheduler.

			A scheduler may be suspended to stop all I/O temporarily. This call increments
			the scheduler's suspend count; while the suspend count is non-zero the scheduler
			is suspended.  New I/O requests will be accepted while the scheduler is suspended,
			but they will not be processed until the scheduler's suspend count reaches zero. 
			
			\param[in] pWhy    Optional string which can be used to indicate why the scheduler is being suspended. (may be NULL)
			\see scheduler::getSuspendCount(), scheduler::resume()
		*/
		void suspend(const char *pWhy = NULL);

		/** \brief   Returns the scheduler's current suspend count.

			The suspend count indicates how many times the scheduler has been
			suspended. Upon creation a scheduler's suspend count will be zero. This
			value is only modified by calls to scheduler::suspend() and
			scheduler::resume(), which should be equally balanced.
			
			\return              The current suspend count.
		*/
		inline U32 getSuspendCount() const { return m_suspendCount; }
		
		/** \brief   Checks to see whether a scheduler is suspended.
			\return  True if the scheduler is suspended, false otherwise.
		*/
		inline bool isSuspended() const { return (getSuspendCount() > 0); }
		
		/** \brief   Resumes a scheduler after suspension.
			This call decrements the scheduler's suspend count.  If the suspend count reaches
			zero as a result, the scheduler is resumed and I/O is started immediately. If the
			suspend count is already zero, this call has no effect.
			\param[in] pWhy     Optional string which can be used to indicate why the scheduler was suspended. (may be NULL)
		*/
		void resume(const char *pWhy = NULL);

		/** \brief  Gets statistics about the scheduler's use.
			This call returns statistics about the scheduler's load in several different ways.
			The statType parameter indicates the type of information you are requesting. See
			#e_FIOS_STATTYPE for details about the different statistics you can request and
			their meaning.
			\param[in]   statType        Type of statistic requested. See #e_FIOS_STATTYPE.
			\param[out]  pLifetimeValue  The cumulative value of the statistic over the lifetime of the scheduler.
			\param[out]  pRecentValue    The value of the statistic either right now, or over the last two seconds.
		*/
		void getStatistics(
			stattype_t statType,
			U32 *pLifetimeValue,
			U32 *pRecentValue);
		
		/**
		 * \addtogroup FIOSPath	Path APIs
		 * @{
		 */
		/** \brief Indicates whether the media supports path IO.
			\result True if the media supports path APIs, false otherwise.
		*/
		inline bool supportsPathIO() const { return m_pMedia->supportsPathIO(); }
		
		/** \brief Returns the size of a file.
			When a catalog cache is being used, this call operates instantaneously and the
			sync version may be safely used. When no catalog cache is present, this call may
			require some I/O.
			\stdparam
			\param[in]   pPath      Path to a file on the media.
			\param[out]  pByteSize  Upon successful completion, filled in with the size of the file.
			\asyncreturn
		*/
		op * getFileSize(
			const opattr_t *pAttr,
			const char *pPath,
			off_t *pByteSize);
		
		/** \brief Returns the size of a file (sync)
			This sync call is equivalent to scheduler::getFileSize() followed by op->syncWait().
			\stdparam
			\param[in]   pPath     Path to a file on the media.
			\param[out]  pByteSize  Upon successful completion, filled in with the size of the file.
			\see scheduler::getFileSize, op::wait
			\syncreturn
		*/
		inline err_t getFileSizeSync(
			const opattr_t *pAttr,
			const char *pPath,
			off_t *pByteSize) { return getFileSize(pAttr,pPath,pByteSize)->syncWait(); }
		
		/** \brief Indicates whether a file exists at a path.
			When a catalog cache is being used, this call operates instantaneously and the sync
			version may be safely used. When no catalog cache is in place, this operation may
			require some I/O.
			
			\note This function will return FALSE if a directory exists at the path instead of a file.
			If you wish to check for any item, use #itemExists().
			\stdparam
			\param[in]  pPath      Path to a file on the media.
			\param[out] pExists    Upon successful completion, a TRUE or FALSE value will be put here.
			\asyncreturn
		*/
		op * fileExists(
			const opattr_t *pAttr,
			const char *pPath,
			bool *pExists);
		
		/** \brief Indicates whether a file exists. (sync)
			This sync call is equivalent to #fileExists() followed by op::syncWait.
			
			\note This function will return FALSE if a directory exists at the path instead of a file.
			If you wish to check for any item, use #itemExistsSync().
			\stdparam
			\param[in]  pPath    Path to a file on the media.
			\param[out] pExists  Upon successful completion, filled in with a value indicating the presence of the file.
			\syncreturn
		*/
		inline err_t fileExistsSync(
			const opattr_t *pAttr,
			const char *pPath,
			bool *pExists) { return fileExists(pAttr,pPath,pExists)->syncWait(); }
		
		/** \brief Indicates whether a directory exists at a path.
			When a catalog cache is being used, this call operates instantaneously and the sync
			version may be safely used. When no catalog cache is in place, this operation may
			require some I/O.
		
			This function will return FALSE if a file exists at the path instead of a directory.
			If you wish to check for any item, use scheduler::itemExists().
			\stdparam
			\param[in]  pPath    Path to a directory on the media.
			\param[out] pExists  Upon successful completion, filled in with a value indicating the presence of the directory.
			\asyncreturn
		*/
		op * directoryExists(
			const opattr_t *pAttr,
			const char *pPath,
			bool *pExists);
		
		/** \brief Indicates whether a directory exists. (sync)
			This sync call is equivalent to scheduler::directoryExists followed by op::syncWait.

			This function will return FALSE if a file exists at the path instead of a directory.
			If you wish to check for any item, use #itemExistsSync().
			\stdparam
			\param[in]  pPath    Path to a directory on the media.
			\param[out] pExists  Upon successful completion, filled in with a value indicating the presence of the directory.
			\syncreturn
		*/
		inline err_t directoryExistsSync(
			const opattr_t *pAttr,
			const char *pPath,
			bool *pExists) { return directoryExists(pAttr,pPath,pExists)->syncWait(); }
		
		/** \brief Indicates whether any item exists at a path.
			When a catalog cache is being used, this call operates instantaneously and the sync
			version may be safely used. When no catalog cache is in place, this operation may
			require some I/O.
			
			\note This function will return TRUE if any item (file, directory, or other) exists at the
			path. If you wish to check for one or the other, use scheduler::fileExists or 
			scheduler::directoryExists.
			\stdparam
			\param[in]  pPath    Path on the media.
			\param[out] pExists  Upon successful completion, filled in with a value indicating the presence of an item.
			\asyncreturn
		*/
		op * itemExists(
			const opattr_t *pAttr,
			const char *pPath,
			bool *pExists);
		
		/** \brief Indicates whether any item exists at a path. (sync)
			This sync call is equivalent to scheduler::itemExists followed by op::syncWait.

			\note This function will return TRUE if any item (file, directory, or other) exists at the
			path. If you wish to check for one or the other, use scheduler::fileExists or 
			scheduler::directoryExists.
			\stdparam
			\param[in]  pPath    Path on the media.
			\param[out] pExists  Upon successful completion, filled in with a value indicating the presence of an item.
			\syncreturn
		*/
		inline err_t itemExistsSync(
			const opattr_t *pAttr,
			const char *pPath,
			bool *pExists) { return itemExists(pAttr,pPath,pExists)->syncWait(); }
		
		/** \brief Returns a full set of status information for a file or directory.
			\note Because it may require more I/O to get all the information needed by #stat, you should
			only use this call if you are looking for information other than size and existence.
			If you just want to check for existence, use #fileExists, #directoryExists, or
			#itemExists instead. If you just want to get a file's size (and indirectly
			test for existence), use #getFileSize instead.
			\stdparam
			\param[in]  pPath     Path on the media.
			\param[out] pStatus   Upon successful completion, filled in with a fios::stat_t.
			\asyncreturn
		*/
		op * stat(
			const opattr_t *pAttr,
			const char *pPath,
			stat_t *pStatus);
		
		/** \brief Returns a full set of status information for a file or directory.
			This sync call is equivalent to scheduler::stat followed by op::syncWait.
			\stdparam
			\param[in]  pPath    Path on the media.
			\param[out] pStatus  Upon successful completion, filled in with a fios::stat_t.
			\syncreturn
		*/
		inline err_t statSync(
			const opattr_t *pAttr,
			const char *pPath,
			stat_t *pStatus) { return stat(pAttr,pPath,pStatus)->syncWait(); }
		
		/** \brief Reads bytes from a file without opening it.
			When a catalog cache is being used, this call reads the appropriate blocks directly. If
			no catalog cache is present, this call is equivalent to open/seek/read/close.
			\stdparam
			\param[in]  pPath      Path to a file on the media.
			\param[out] pBuf       Buffer to receive file data.
			\param[in]  byteOffset Offset within the file to read from.
			\param[in]  byteCount  Requested number of bytes to read.
			\asyncreturn
		*/
		op * readFile(
			const opattr_t *pAttr,
			const char *pPath,
			void *pBuf,
			off_t byteOffset,
			off_t byteCount);
		
		/** \brief  Reads bytes from a file without opening it. (sync)

			This sync call is equivalent to scheduler::readFile() followed by op::syncWait().

			\stdparam
			\param[in]  pPath      Path to a file on the media.
			\param[out] pBuf       Buffer to receive file data.
			\param[in]  byteOffset Offset within the file to read from.
			\param[in]  byteCount  Requested number of bytes to read.
			\syncreturn
		*/
		inline err_t readFileSync(
			const opattr_t *pAttr,
			const char *pPath,
			void *pBuf,
			off_t byteOffset,
			off_t byteCount) { return readFile(pAttr,pPath,pBuf,byteOffset,byteCount)->syncWait(); }
		
		/** \brief  Writes bytes to a file without opening it.

			When a catalog cache is being used, this call writes the appropriate blocks directly. If
			no catalog cache is present, this call is equivalent to open/seek/write/close. As with a
			normal write, this call will overwrite bytes within the file until the end of the file is
			reached. Once the end of the file is reached any remaining data is appended.

			\stdparam
			\param[in]  pPath      Path to a file on the media.
			\param[in]  pBuf       Buffer with file data to write.
			\param[in]  byteOffset Offset within the file to write to.
			\param[in]  byteCount  Requested number of bytes to write.
			\asyncreturn
		*/
		op * writeFile(
			const opattr_t *pAttr,
			const char *pPath,
			const void *pBuf,
			off_t byteOffset,
			off_t byteCount);

		/** \brief  Writes bytes to a file without opening it. (sync)

			This sync call is equivalent to scheduler::writeFile() followed by op::syncWait().
			
			\stdparam
			\param[in]  pPath      Path to a file on the media.
			\param[in]  pBuf       Buffer with file data to write.
			\param[in]  byteOffset Offset within the file to read from.
			\param[in]  byteCount  Requested number of bytes to read.
			\syncreturn
		*/
		inline err_t writeFileSync(
			const opattr_t *pAttr,
			const char *pPath,
			const void *pBuf,
			off_t byteOffset,
			off_t byteCount) { return writeFile(pAttr,pPath,pBuf,byteOffset,byteCount)->syncWait(); }
		
		/** \brief	Creates a directory.
			\stdparam
			\param[in]  pPath              Path on the media to the directory that you want to create.
			\asyncreturn
		*/
		op * createDirectory(
			const opattr_t *pAttr,
			const char *pPath);
		
		/** \brief  Creates a directory (sync).
			\stdparam
			\param[in]  pPath              Path on the media to the directory that you want to create.
			\syncreturn
		*/
		inline err_t createDirectorySync(
			const opattr_t *pAttr,
			const char *pPath) { return createDirectory(pAttr,pPath)->syncWait(); }
		
		/** \brief Iterates through a directory.
			This call retrieves one child from a directory. It's best for light use, where you
			only have one directory being iterated at a time. Entries are not returned in any specific
			order.
			
			Entries will always represent real files or directories. Special entries such as "." and
			".." will not be returned.
			\stdparam
			\param[in]  pPath       Directory to get children from.
			\param[in]  childIndex  Zero-based index representing the child to retrieve. Increment to iterate.
			\param[out] pChildEntry Receives the directory entry.
			\asyncreturn
		*/
		op * readDirectory(
			const opattr_t *pAttr,
			const char *pPath,
			off_t childIndex,
			direntry_t *pChildEntry);
		
		/** \brief Iterates through a directory (sync)
			This call is equivalent to scheduler::readDirectory followed by op::syncWait.
			\stdparam
			\param[in]  pPath       Directory to get children from.
			\param[in]  childIndex  Zero-based index representing the child to retrieve. Increment to iterate.
			\param[out] pChildEntry Receives the directory entry.
			\syncreturn
		*/
		inline err_t readDirectorySync(
			const opattr_t *pAttr,
			const char *pPath,
			off_t childIndex,
			direntry_t *pChildEntry) { return readDirectory(pAttr,pPath,childIndex,pChildEntry)->syncWait(); }
		
		/** \brief Deletes a file or directory.
			This call deletes a file or directory. Files must not be open, or the call
			will fail with kERR_ACCESS. Directories must be empty, or the call will fail
			with kERR_ACCESS. 
			\stdparam
			\param[in]  pPath       File or directory to delete.
			\asyncreturn
		*/
		op * unlink(
			const opattr_t *pAttr, 
			const char *pPath);
		
		/** \brief Deletes a file or directory. (sync)
			This call is equivalent to scheduler::unlinkSync followed by op::syncWait.
			\stdparam
			\param[in]  pPath       File or directory to delete.
			\syncreturn
		*/
		inline err_t unlinkSync(
			const opattr_t *pAttr,
			const char *pPath) { return unlink(pAttr,pPath)->syncWait(); }

		/*@}*/

		/**
		 * \addtogroup FIOSFH		Filehandle APIs
		 * @{
		 */
		
		/** \brief Open a file.

			When a catalog cache is being used, this call operates synchronously and the sync
			version may be safely used. When no catalog cache is being used, this call may
			require some I/O.

			\stdparam
			\param[in]  pPath     Path to a file on the media.
			\param[in]  openFlags Open flags.
			\param[out] pNewFH    Upon successful completion, pointer to the newly-opened filehandle.
			\asyncreturn
			\see e_OPENFLAGS
		*/
		op * openFile(
			const opattr_t *pAttr,
			const char *pPath,
			U32 openFlags,
			filehandle **pNewFH);

		/** \brief Open a file. (sync)
			This sync call is equivalent to scheduler::openFile() followed by op::syncWait().
			\stdparam
			\param[in]  pPath      Path to a file on the media.
			\param[in]  openFlags  Open flags.
			\param[out] pNewFH     Upon successful completion, pointer to file handle.
			\syncreturn
			\see e_FIOS_OPENFLAGS, scheduler::open(), op::syncWait()
		*/
		inline err_t openFileSync(
			const opattr_t *pAttr,
			const char *pPath,
			U32 openFlags,
			filehandle **pNewFH) { return openFile(pAttr,pPath,openFlags,pNewFH)->syncWait(); }
		
		/** \brief      Seek within an open file.
			This call changes the current position within the file, and returns the updated
			position. It can be used to query the current position by using #kSEEK_CUR
			with an offset of 0.
			
			This call is equivalent to filehandle::seek.
			
			In the current implementation, this call operates immediately and the operation
			attributes are ignored.
			\stdparam
			\param[in]  pFH       File handle.
			\param[in]  offset    Offset to seek to.
			\param[in]  whence    How to seek (from start, current location, or end).
			\param[out] pResult   New offset (may be NULL).
			\syncreturn
			\see e_WHENCE, filehandle::seek
		*/
		inline err_t seekFileSync(
			const opattr_t *pAttr,
			filehandle *pFH,
			off_t offset,
			whence_t whence,
			off_t *pResult = NULL) { (void)pAttr; return pFH->seek(offset,whence,pResult); }
		
		/** \brief		Read from an open file.
			\stdparam
			\param[in]  pFH        File handle.
			\param[out] pBuf       Buffer to receive file data.
			\param[in]  byteCount  Number of bytes to read.
			\param[in]  offset     Offset inside the file at which to read. If the offset is -1, the current offset is used and the file pointer is advanced after the read.
			\asyncreturn
		*/
		op * readFile(
			const opattr_t *pAttr,
			filehandle *pFH,
			void *pBuf,
			off_t byteCount,
			off_t offset = -1);

		/** \brief      Read from an open file (sync).
			This sync call is equivalent to scheduler::readFile() followed by op::syncWait().
			\stdparam
			\param[in]  pFH        File handle.
			\param[out] pBuf       Buffer to receive file data.
			\param[in]  byteCount  Number of bytes to read.
			\param[in]  offset     Offset inside the file at which to read. If the offset is -1, the current offset is used and the file pointer is advanced after the read.
			\syncreturn
			\see scheduler::readFile, op::syncWait
		*/
		inline err_t readFileSync(
			const opattr_t *pAttr,
			filehandle *pFH,
			void *pBuf,
			off_t byteCount,
			off_t offset = -1) { return readFile(pAttr,pFH,pBuf,byteCount,offset)->syncWait(); }
		
		/** \brief      Write to an open file.
			\stdparam
			\param[in]  pFH        File handle.
			\param[in]  pBuf       Buffer with file data to write.
			\param[in]  byteCount  Number of bytes to write.
			\param[in]  offset     Offset inside the file at which to write. If the offset is -1, the current offset is used and the file pointer is advanced after the read.
			\asyncreturn
		*/
		op * writeFile(
			const opattr_t *pAttr,
			filehandle *pFH,
			const void *pBuf,
			off_t byteCount,
			off_t offset = -1);

		/** \brief      Write to an open file (sync)
			\stdparam
			\param[in]  pFH       File handle.
			\param[in]  pBuf      Buffer with file data to write.
			\param[in]  byteCount Number of bytes to write.
			\param[in]  offset     Offset inside the file at which to write. If the offset is -1, the current offset is used and the file pointer is advanced after the read.
			\syncreturn
			\see scheduler::writeFile, op::syncWait
		*/
		inline err_t writeFileSync(
			const opattr_t *pAttr,
			filehandle *pFH,
			const void *pBuf,
			off_t byteCount,
			off_t offset = -1) { return writeFile(pAttr,pFH,pBuf,byteCount,offset)->syncWait(); }

		/** \brief     Close an open file.
			The scheduler will wait until all pending I/O on the file handle is complete
			before releasing its memory and deleting the object.
			\stdparam
			\param[in]   pFH      File handle.
			\asyncreturn
		*/
		op * closeFile(
			const opattr_t *pAttr,
			filehandle *pFH);
		
		/** \brief    Close an open file (sync)
			\stdparam
			\param[in]   pFH      File handle. This will be disposed and should not be referenced after this call completes.
			\syncreturn
			\see scheduler::closeFile, op::syncWait
		*/
		inline err_t closeFileSync(
			const opattr_t *pAttr,
			filehandle *pFH) { return closeFile(pAttr,pFH)->syncWait(); }
		
		/*@}*/

		/**
		 *	\addtogroup	FIOSStream	Stream APIs
		 * @{
		 */

		/** \brief    Opens a stream. (path variant)
			
			There are two primary variants of streams: managed and unmanaged.
			
			With managed buffers, a stream's buffers are provided by FIOS and
			allocated from the scheduler's memory allocator. This allows the scheduler to
			perform optimizations and adjust streaming buffer sizes dynamically
			as the I/O load changes. Clients ask FIOS for the next page when they
			are ready for it, and they receive a pointer to internal storage.
			
			New code should usually be written to use managed streams. Benefits of
			managed streams include: memory consumption is easily predictable and
			never fragmented, buffers are automatically shared and balanced between
			streams, and advanced optimization strategies become possible for FIOS.
			
			With unmanaged buffers, a stream's buffers are provided by the
			callback. If you already have a ring-buffer solution and don't want to
			change your code, you can use unmanaged buffers with your stream. To
			specify an unmanaged stream, use a value of zero for the pageSize argument.
			
			\param[in]   pStreamAttr      Description of the stream.
			\param[out]  pStream          The newly-created stream.
		*/
		err_t openStreamSync(
			const streamattr_t *pStreamAttr,
			stream **pStream);
		
		/** \brief  Close a stream.
		    This call is always accepted and operates asynchronously. The scheduler will
			wait until all pending I/O on the stream is complete before releasing its
			memory and deleting the object.
			\param[in]   pStream       Stream to close.
		*/
		void closeStream(
			stream *pStream);
		
		/*@}*/
		/**
		 *  \addtogroup FIOSBlock Block APIs
		 * @{
		 */
		/** \brief Indicates whether the media supports block IO.
			\result True if the media supports block APIs, false otherwise.
		*/
		inline bool supportsBlockIO() const { return m_pMedia->supportsBlockIO(); }
		
		/** \brief Get the media's native block size.
			Returns 0 if the media cannot be accessed with block APIs.
			\result The native block size in bytes.
		*/
		inline size_t getMediaBlockSize() const { return m_pMedia->getBlockSize(); }
		
		/** \brief Get the media's native block count.
			Returns 0 if the media cannot be accessed with block APIs.
			\result The number of blocks on the media.
		*/
		inline off_t getMediaBlockCount() const { return m_pMedia->getBlockCount(); }

		/** \brief Read directly from a set of blocks.
			The client specifies a block size in this call.  The media does have a native block
			size, but the client may choose to operate in a different block size if that is more
			convenient. Conversion and blocking/deblocking will be done automatically.
			\stdparam
			\param[in]  blockSize     Block size.
			\param[in]  blockNumber   Block number from which to read.
			\param[in]  blockCount    Number of blocks to read.
			\param[out] pBuf          Buffer to receive data, at least as big as blockSize * blockCount.
			\asyncreturn
		*/
		op * readBlocks(
			const opattr_t *pAttr,
			size_t blockSize,
			off_t blockNumber,
			off_t blockCount,
			void *pBuf);

		/** \brief Read directly from a set of blocks. (sync)
			This sync call is equivalent to scheduler::readBlocks() followed by op::syncWait().
			\stdparam
			\param[in]  blockSize     Block size.
			\param[in]  blockNumber   Block number from which to read.
			\param[in]  blockCount    Number of blocks to read.
			\param[out] pBuf          Buffer to receive data, at least as big as blockSize * blockCount.
			\syncreturn
			\see scheduler::readBlocks, op::syncWait
		*/
		inline err_t readBlocksSync(
			const opattr_t *pAttr,
			size_t blockSize,
			off_t blockNumber,
			off_t blockCount,
			void *pBuf) { return readBlocks(pAttr,blockSize,blockNumber,blockCount,pBuf)->syncWait(); }
		
		/** \brief  Write directly to a set of blocks.
			The client specifies a block size in this call.  The media does have a native block
			size, but the client may choose to operate in a different block size if that is more
			convenient. Conversion and blocking/deblocking will be done automatically.
			\stdparam
			\param[in]  blockSize     Block size.
			\param[in]  blockNumber   Block number at which to write.
			\param[in]  blockCount    Number of blocks to write.
			\param[in]  pBuf          Data to write, at least as big as blockSize * blockCount.
			\asyncreturn
		*/
		op * writeBlocks(
			const opattr_t *pAttr,
			size_t blockSize,
			off_t blockNumber,
			off_t blockCount,
			void *pBuf);
		
		/** \brief Write directly to a set of blocks. (sync)
			This sync call is equivalent to scheduler::writeBlocks() followed by op::syncWait().
			\stdparam
			\param[in]  blockSize     Block size.
			\param[in]  blockNumber   Block number at which to write.
			\param[in]  blockCount    Number of blocks to write.
			\param[in]  pBuf          Data to write, at least as big as blockSize * blockCount.
			\syncreturn
			\see scheduler::writeBlocks, op::syncWait
		*/
		inline err_t writeBlocksSync(
			const opattr_t *pAttr,
			size_t blockSize,
			off_t blockNumber,
			off_t blockCount,
			void *pBuf) { return writeBlocks(pAttr,blockSize,blockNumber,blockCount,pBuf)->syncWait(); }
		

		/*@}*/
		/**
		 *  \addtogroup FIOSOp Operations
		 * @{
		 */
		/** \brief Cancels an operation.
			If the operation has not yet executed, its error code will be set to #kERR_CANCELLED
			and it will be removed from the queue. If a callback is set, the callback will be invoked
			with a #kOPEVENT_COMPLETED event.
			
			\note
			Because I/O happens asynchronously, it's possible that the operation may be executing or
			already completed when you issue this call, in which case it's too late to cancel! If
			that happens the operation will not be cancelled; instead the error code will be set to
			the result of the operation as usual.
			
			This function does not dispose the operation handle; you should still delete the operation
			to reclaim the memory used by the object. Since deleting an operation does an implicit
			cancel, you may wish to simply delete operations to cancel them.
			\param[in]  pOp     Operation handle.
			\see op::cancel
		*/
		void cancelOp(
			op *pOp);
		
		/** \brief Deletes an operation.
			If the operation has not yet completed, it is canceled first with cancelOp().
			The operation pointer may be reused for another operation later.
			\param[in]  pOp     Operation handle.
			\see scheduler::cancelOp
		*/
		void deleteOp(
			op *pOp);
		
		/** \brief Reschedules an operation.
			If the operation has not yet executed, its priority and deadline are changed and
			it is moved to the incoming queue for rescheduling.

			\note
			Because I/O happens asynchronously, it's possible that the operation may be executing or
			already completed when you issue this call, in which case this call has no effect.
			\param[in]  pOp       Operation handle.
			\param[in]  deadline  New deadline.
			\param[in]  priority  New priority.
			\see op::setDeadline
		*/
		void rescheduleOp(
			op *pOp,
			abstime_t deadline,
			prio_t priority);
		
		/** \brief Returns the estimated time at which an operation will complete
			This call returns the scheduler's current best estimate of when the operation
			will be complete.  This value may change at any time, either because additional
			I/O requests were received or because the scheduling algorithm finds an optimization
			opportunity.
			
			\note
			The scheduler's optimization algorithm does not always push operations forward. Any
			individual operation may be pushed later in the schedule, rather than earlier, as
			long as it does not violate any deadlines and results in a shorter overall time for
			the list of scheduled I/Os as a whole.
			\param[in]  pOp    Operation handle
			\return   The current estimate for the completion time.
		*/
		abstime_t getEstimatedCompletion(
			const op *pOp) const;
		
		/** \brief Performs an action on all operations in the queue.
			This can be used to selectively cancel operations or otherwise
			modify them -- for example, you could use an action to 
			cancel a whole series of level-related operations at once.
			
			The action callback should do its job quickly, because I/O and scheduling
			may be blocked until it completes.
			\param[in]  pAction         Action callback.
			\param[in]  pActionContext  Action context.
		*/
		void iterateOps(
			void (*pAction)(void *context, op *pOp),
			void *pActionContext);

protected:
		/** \internal
			\brief Allocates an operation.
			Used by all functions that return new operations.
			\param[in]  pAttr        Operation attributes (may be NULL).
			\return Newly-allocated operation, or NULL.
		*/
		op * allocateOp(
			const opattr_t *pAttr);
		/** \internal
			\brief Deallocates an operation.
			Called by deleteOp. The operation should be in the completed list.
			\param[in]  pOp         Operation to delete.
		*/
		void deallocateOp(
			op *pOp);

		/** \internal
			\brief Moves a newly-created operation to the incoming queue.
			Used by all functions that create an operation.
			\param[in] pOp   Operation to accept.
			\return Returns the operation.
		*/
		op * acceptOp(op *pOp);

		/** \internal
			\brief Moves an operation to the completed list.
			Used by all functions that complete an operation, including functions
			that create an operation that executes immediately. This does not remove
			the operation from any list it might be in; that must be done prior to this call.
			\param[in] pOp   Operation to complete.
			\return Returns the operation.
		*/
		op * completeOp(op *pOp);

		/*@}*/

private:
	friend class op;                       /**< The op class needs to access some internal variables */
	friend class stream;                   /**< The stream class needs to access some internal methods */

	// Class variables.
	static platform::atomicPtr<scheduler *> s_pDefaultScheduler; /**< Default scheduler. */
	
	// Unchanging member variables. Set up in the constructor and never changed.
	mutable platform::mutex m_objectLock;      /**< Object lock for accessing member variables. */
	mutable platform::mutex m_opLock;          /**< Object lock for accessing operations that belong to this scheduler. */
	mutable platform::rwlock m_opCallbackLock; /**< Rwlock for accessing operation callbacks. */
	mutable platform::mutex m_completedLock;   /**< Object lock guarding the completed list. */
	platform::thread m_schedulerThread;    /**< Scheduler thread */
	mutable platform::mutex m_ioLock;      /**< Mutex used for m_ioCond, m_idleCond, and other I/O related stuff */
	mutable platform::cond m_ioCond;       /**< Condition used when waiting for I/O or new operations */
	mutable platform::cond m_idleCond;     /**< Condition used to signal when the scheduler is idle. */
	media *m_pMedia;                       /**< Media object controlled by this scheduler. */
	U32 m_numStaticOps;                    /**< Number of statically allocated ops. */
	U32 m_numStaticFiles;                  /**< Number of statically allocated filehandles. */
	U32 m_numStaticStreams;                /**< Number of statically allocated streams. */
	size_t m_sizeOfManagedBufferSpace;     /**< Size reserved for managed buffers. */
	void *m_staticOpAllocation;            /**< Static op allocation. */
	void *m_staticFileAllocation;          /**< Static file allocation. */
	void *m_staticStreamAllocation;        /**< Static stream allocation. */
	void *m_staticBufferSpaceAllocation;   /**< Static managed buffer allocation. */
	mutable FIOS_ALLOCATOR *m_pAllocator;  /**< Allocator for this scheduler */
	
	bool m_ioDone;                         /**< Whether any ops in the issue list are done. Protected by m_ioLock. */
	
	// Changeable member variables
	platform::atomicU32 m_suspendCount;    /**< Suspend count */
	platform::atomicBool m_acceptingNewIO; /**< Whether this scheduler is accepting new I/O. Normally true, set to false during deletion. */
	
	// Worker thread stuff
	platform::mutex m_workerLock;
	platform::cond m_workerCond;
	bool m_workerThreadsRunning;
	U32 m_issueNumber;
	platform::atomicU32 m_dispatchNumber;
	platform::atomicInt<abstime_t> m_lastDispatchTime;
	
	platform::atomicU32 m_ioGeneration;    /**< Live generation count, bumped every time an I/O completion callback occurs */
	U32 m_myGeneration;                    /**< The last generation we handled in the scheduler loop. */
	
	// PCE math -- everything here protected by m_ioLock and only accessed by the scheduler thread
	abstime_t *m_pUnknownEstimate;         /**< Estimates of unknown values. Array of size m_pMedia->getNumberOfCoefficients() */
	abstime_t m_Emax;                      /**< Maximum error seen by the scheduler after estimates have stabilized (after m_opTotal > 50) */
	void *m_pCurrentMediaState;            /**< Pointer to current media state (state right now). Access protected by m_ioLock. */
	void *m_pProjectedMediaState;          /**< Pointer to projected media state (state after committed I/Os have completed). Access protected by m_ioLock. */
	abstime_t m_projectedCompletion;       /**< Projected time at which already-committed I/O will complete. */
	
	// Stream buffer
	streampage *m_pStreamPages;            /**< Array of streampages. */
	U32 m_numStreamPages;                  /**< Number of stream pages in the array. */
	collections::list<streampage *> m_pFreeStreamPages; /**< Free pages that aren't being used go here. Access protected by m_objectLock. */
	U64 m_sumOfStreamDataRates;            /**< Sum of the data rates of all open streams. */

	// Op lists
	collections::atomicList<op *> m_pFreeOps;      /**< Free operations, available for allocation */
	collections::atomicList<op *> m_pIncomingOps;  /**< Incoming operations, not yet scheduled */
	collections::list<op *> m_pScheduleQueue;      /**< Queued and scheduled ops. Access protected by m_opLock. */
	collections::list<op *> m_pIssueList;          /**< Issued ops waiting for completion. Access protected by m_opLock. */
	collections::list<op *> m_pCompletedOps;       /**< Completed but not-yet-deleted ops. Access protected by m_completedLock. */
	platform::atomicU32 m_opCount;              /**< Total operation count. Includes incoming, scheduled, and completed ops (but not free ops) */
	platform::atomicU32 m_activeOpCount;        /**< Active operations currently executing. Never exceeds m_pMedia->getMaxConcurrentOps(). */
	platform::atomicU32 m_opTotal;              /**< Number of operations that have completed in total. */
	platform::atomicU32 m_opHighWaterMark;      /**< Highest number of operations allocated at any one time. */
	U32 m_overlapCompletions;                   /**< Number of ops completed via our overlap logic */
	mediaioparams *m_pIOPs;                     /**< I/O request structs. Access from scheduler protected by m_opLock, but media may access directly. */
	
	// Filehandle lists
	collections::list<filehandle *> m_pFreeFilehandles;       /**< Free filehandles, available for allocation. */
	collections::list<filehandle *> m_pAllocatedFilehandles;  /**< Allocated filehandles. Access protected by m_objectLock. */
	U32 m_fhCount;                               /**< Active filehandle count. Access protected by m_objectLock. */
	U32 m_fhTotal;                               /**< Number of filehandles opened in total. */
	U32 m_fhHighWaterMark;                       /**< Highest number of filehandles open at any one time. Access protected by m_objectLock. */
	
	// Stream lists
	collections::list<stream *> m_pFreeStreams;      /**< Free streams, available for allocation. Access protected by m_objectLock. */
	collections::list<stream *> m_pAllocatedStreams; /**< Allocated streams. Access protected by m_objectLock. */
	U32 m_streamCount;                             /**< Active stream count. Access protected by m_objectLock. */
	U32 m_streamTotal;                             /**< Number of streams opened in total. */
	U32 m_streamHighWaterMark;                     /**< Highest number of streams open at any one time. Access protected by m_objectLock. */
	
	/** \brief Allocates a filehandle object.
		\param[in]  flags   Flags for the object.
		\param[in]  pPath   Path for the object to store.
		\return Newly-allocated filehandle, or NULL.
	*/
	filehandle * allocateFH(
		U32 flags,
		const char *pPath);
	
	/** \brief Deallocates a filehandle object.
		\param[in]  pFH    Filehandle to delete.
	*/
	void deallocateFH(
		filehandle *pFH);

	/** \brief Allocates a stream object.
		\return Newly-allocated stream, or NULL.
	*/
	stream * allocateStream(
		const streamattr_t *pStreamAttr);

	/** \brief Deallocates a stream object.
		\param[in]  pStream    Stream to delete.
	*/
	void deallocateStream(
		stream *pStream);
	
	/** \brief Enqueues I/O for a stream object.
		\param[in]  pStream    Stream for which to schedule I/O.
	*/
	void enqueueStreamIO(
		stream *pStream);
	
	/** \brief Gets a stream buffer.
		\param[in]  pStream     Stream to get a buffer for.
		\param[out] pBuffer     Filled in with the stream buffer.
		\param[out] pBufferSize Filled in with the stream buffer size.
		\param[out] pBufferPage Filled in with a pointer to the streampage struct for this buffer.
		\return True if a buffer is available.
	*/
	bool getStreamBuffer(
		stream *pStream,
		void **pBuffer,
		off_t *pBufferSize,
		streampage **pBufferPage);
	
	/** \brief Callback used for stream ops.
		\param[in]      pContext   Context pointer (scheduler object)
		\param[in]      pOp        Op that is the target of the call
		\param[in]      event      What happened
		\param[in,out]  pParam     Parameters for the event.
		\return Error code.
	*/
	static err_t streamOpCallback(
		void *pContext,
		op *pOp,
		opevent_t event,
		void *pParam);

	/** \brief Called by streamOpCallback when an op completes.
		\param[in]      pOp        Op that completed
	*/
	void streamOpDone(
		op *pOp);
	
	/** \brief Frees a stream page for re-use.
		\param[in]      pStreamPages   Linked list of stream pages that have just been released.
	*/
	void deallocateStreamPages(
		streampage *pStreamPages);

	/** \brief Entrypoint for the scheduler thread.
		\param[in]  pThread    Thread pointer.
		\param[in]  pArg       Argument, which is a scheduler instance pointer.
	*/
	static void threadEntry(
		platform::thread *pThread,
		void *pArg);
	
	/** \brief Entrypoint for the media worker threads.
		\param[in]  pThread    Thread pointer.
		\param[in]  pArg       Argument, which is a scheduler instance pointer.
	*/
	static void mediathreadEntry(
		platform::thread *pThread,
		void *pArg);

	/** \brief Whether the scheduler thread has work to do.
		\return True if there is work to do, false otherwise. */
	bool schedulerShouldRun();
	
	/** \brief Issues a new I/O for each free slot in the concurrency list. */
	void dispatchNewRequests();
	
	/** \brief Runs the scheduling algorithm globally on the entire queue. */
	void reorderQueue();
	
	/** \brief Accumulates a batch of PCE coefficients from a completed I/O into our estimate
		\param[in] pCoefficients Coefficients from the completed I/O
		\param[in] elapsedTime   Time elapsed during the I/O
	*/
	void
	accumulatePCECoefficients(
		const pcecoefficient_t *pCoefficients,
		abstime_t elapsedTime);
	
	/** \brief Advances an op forward one stage.
	    \param[in] pOp         Operation pointer.
		\param[in] pRequest    The struct mediarequest from the previous stage of the operation.
	*/
	void
	stageCompleteOp(
		op *pOp,
	    mediarequest *pRequest);
	
	/** \brief Executes a request.
		\param[in]     pOp         Operation pointer.
		\param[in,out] pIOP        A struct mediaioparams to be used for execution. Normally from m_pIOPs.
	*/
	void
	dispatchRequest(
		op *pOp,
		mediaioparams *pIOP);
	
	/** \brief Callback that media driver invokes after operation is complete.
		\param[in]    pContext     Context, in this case a pointer to the scheduler object.
		\param[in]    pMedia       Media object that issued the callback.
		\param[in]    event        Event that triggered the callback, in this case kMEDIAEVENT_IOCOMPLETE.
		\param[in]    pParam       Event parameter, in this case a pointer to a mediaioparams struct.
		\syncreturn
	*/
	static err_t
	requestCallback(
		void *pContext,
		media *pMedia,
		mediaevent_t event,
		void *pParam);
	
	/** \brief Moves an op to the schedule queue.
		\param[in] pOp   Operation to schedule.
		This O(n) routine walks the entire queue of I/O, simulating I/O and looking for
		the best spot for the incoming op.
	*/
	void
	scheduleOp(
		op *pOp);
	
	/** \brief Speculatively executes an op.
		\param[in]     pOp           Operation to speculatively execute.
		\param[in,out] pP            Priority of the highest-priority op whose deadline has been violated so far. May be NULL.
		\param[in,out] pN            Number of ops of priority (*pP) whose deadlines have been violated so far. May be NULL.
		\param[in,out] pT            Time, updated by this call.
		\param[in,out] pMediaState   Media state, updated by this call.
		\return TRUE if the op failed to meet its deadline.
	*/
	bool
	speculateOp(
		op *       pOp,
		prio_t *   pP,
		U32 *      pN,
		abstime_t *pT,
		void *pMediaState);
	
	/** \brief Scheduler thread.
		This function contains the scheduler's main thread loop.
	*/
	void thread();

	/** \brief Media worker threads.
		This function contains the loop for the worker thread(s).
	*/
	void mediathread(
		platform::thread *pThread);
};
/*@}*/


/* Some additional inlines are listed here due to circular dependencies. */
inline void      op::cancel() { return m_pScheduler->cancelOp(this); }
inline void      op::operator delete(void *pMem) { op *pOp = reinterpret_cast<op*>(pMem); pOp->m_pScheduler->deleteOp(pOp); }
inline abstime_t op::getEstimatedCompletion() const { return m_pScheduler->getEstimatedCompletion(this); }
inline void      op::setDeadline(abstime_t deadline) { m_pScheduler->rescheduleOp(this, deadline, getPriority()); }
inline void      op::setPriority(prio_t priority) { m_pScheduler->rescheduleOp(this, getDeadline(), priority); }
inline void      op::setDeadlineAndPriority(abstime_t deadline, prio_t priority) { m_pScheduler->rescheduleOp(this, deadline, priority); }

inline op *  filehandle::read(const opattr_t *pAttr, void *pBuf, off_t byteCount) { return m_pScheduler->readFile(pAttr,this,pBuf,byteCount); }
inline err_t filehandle::readSync(const opattr_t *pAttr, void *pBuf, off_t byteCount) { return m_pScheduler->readFileSync(pAttr,this,pBuf,byteCount); }
inline op *  filehandle::pread(const opattr_t *pAttr, void *pBuf, off_t byteCount, off_t offset) { return m_pScheduler->readFile(pAttr,this,pBuf,byteCount,offset); }
inline err_t filehandle::preadSync(const opattr_t *pAttr, void *pBuf, off_t byteCount, off_t offset) { return m_pScheduler->readFileSync(pAttr,this,pBuf,byteCount,offset); }
inline op *  filehandle::write(const opattr_t *pAttr, const void *pBuf, off_t byteCount) { return m_pScheduler->writeFile(pAttr,this,pBuf,byteCount); }
inline err_t filehandle::writeSync(const opattr_t *pAttr, const void *pBuf, off_t byteCount) { return m_pScheduler->writeFileSync(pAttr,this,pBuf,byteCount); }
inline op *  filehandle::pwrite(const opattr_t *pAttr, const void *pBuf, off_t byteCount, off_t offset) { return m_pScheduler->writeFile(pAttr,this,pBuf,byteCount,offset); }
inline err_t filehandle::pwriteSync(const opattr_t *pAttr, const void *pBuf, off_t byteCount, off_t offset) { return m_pScheduler->writeFileSync(pAttr,this,pBuf,byteCount,offset); }
inline void  filehandle::close() { m_pScheduler->closeFileSync(NULL,this); }
inline void  filehandle::operator delete(void *pMem) { filehandle *pFH = reinterpret_cast<filehandle*>(pMem); pFH->m_pScheduler->closeFileSync(NULL,pFH); }

inline void  stream::close() { m_pScheduler->closeStream(this); }
inline void  stream::operator delete(void *pMem) { stream *pStream = reinterpret_cast<stream*>(pMem); pStream->m_pScheduler->closeStream(pStream); }

}; /* namespace fios */


#endif /* _H_fios_scheduler */

