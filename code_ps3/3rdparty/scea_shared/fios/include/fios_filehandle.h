/**
	\file fios_filehandle.h

	Class definitions for the fios::filehandle class.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_filehandle
#define _H_fios_filehandle

#include "fios_types.h"
#include "fios_base.h"
#include "fios_platform.h"

namespace fios {

/**
	\addtogroup FIOSFH Filehandle APIs
	@{
*/

/**
	\brief File handle class
*/
class FIOS_EXPORT filehandle : public object
{
protected:
	/** Constructor. Don't instantiate directly.
		\see scheduler::openFile, scheduler::openFileSync */
	filehandle();
public:
	/** Destructor. */
	~filehandle();

	/** Returns the scheduler associated with this filehandle.
		\return Scheduler object */
	inline scheduler * getScheduler() const { return m_pScheduler; }
	
	/** Returns the media filehandle associated with this object.
		\return Media filehandle */
	inline const mediafd & getMediaFilehandle() const { return m_mediaFD; }
	
	/** Returns the file's flags.
		\return Flags
		\see e_OPENFLAGS
	*/
	inline U32 getFlags() const { return m_flags; }

	/** \brief Seeks within a file.
		This call does not actually seek the media filehandle; it simply
		updates the offset within the object.
		\param[in]  offset  New offset
		\param[in]  whence  Where to seek from
		\param[out] pResult Final offset (may be NULL)
		\syncreturn
	*/
	err_t seek(
		off_t offset,
		whence_t whence = kSEEK_SET,
		off_t *pResult = NULL);
	
	/** Gets the file's offset
		\return Offset */
	off_t getOffset() const;

	/** \brief Reads from a file at the current offset, and advances the offset.
		\stdparam
		\param[out] pBuf       Buffer to receive file data.
		\param[in]  byteCount  Number of bytes to read.
		\asyncreturn
	*/
	op * read(
		const opattr_t *pAttr,
		void *pBuf,
		off_t byteCount); /* inlined in fios_scheduler.h */
	
	/** \brief Reads from a file at the current offset, and advances the offset (sync)
		\stdparam
		\param[in]  pBuf      Buffer to receive file data.
		\param[in]  byteCount Number of bytes to read.
		\syncreturn
	*/
	err_t readSync(
		const opattr_t *pAttr,
		void *pBuf,
		off_t byteCount); /* inlined in fios_scheduler.h */
	
	/** \brief Reads from a file at a specified offset. Does not advance the offset.
		\stdparam
		\param[out] pBuf       Buffer to receive file data.
		\param[in]  byteCount  Number of bytes to read.
		\param[in]  offset     Offset in the file from which to read.
		\asyncreturn
	*/
	op * pread(
		const opattr_t *pAttr,
		void *pBuf,
		off_t byteCount,
		off_t offset); /* inlined in fios_scheduler.h */
	
	/** \brief Reads from a file at a specified offset. Does not advance the offset. (sync)
		\stdparam
		\param[out] pBuf       Buffer to receive file data.
		\param[in]  byteCount  Number of bytes to read.
		\param[in]  offset     Offset in the file from which to read.
		\syncreturn
	*/
	err_t preadSync(
		const opattr_t *pAttr,
		void *pBuf,
		off_t byteCount,
		off_t offset); /* inlined in fios_scheduler.h */
	
	/** \brief Writes to a file at the current offset, and advances the offset.
		\stdparam
		\param[in]  pBuf      Buffer with file data to write.
		\param[in]  byteCount Number of bytes to write.
		\asyncreturn
	*/
	op * write(
		const opattr_t *pAttr,
		const void *pBuf,
		off_t byteCount); /* inlined in fios_scheduler.h */
	
	/** \brief Writes to a file at the current offset, and advances the offset. (sync)
		\stdparam
		\param[in]  pBuf      Buffer with file data to write.
		\param[in]  byteCount Number of bytes to write.
		\syncreturn
	*/
	err_t writeSync(
		const opattr_t *pAttr,
		const void *pBuf,
		off_t byteCount); /* inlined in fios_scheduler.h */
	
	/** \brief Writes to a file at a specified offset. Does not advance the offset.
		\stdparam
		\param[in]  pBuf      Buffer with file data to write.
		\param[in]  byteCount Number of bytes to write.
		\param[in]  offset    Offset at which to write.
		\asyncreturn
	*/
	op * pwrite(
		const opattr_t *pAttr,
		const void *pBuf,
		off_t byteCount,
		off_t offset); /* inlined in fios_scheduler.h */
	
	/** \brief Writes to a file at a specified offset. Does not advance the offset. (sync)
		\stdparam
		\param[in]  pBuf      Buffer with file data to write.
		\param[in]  byteCount Number of bytes to write.
		\param[in]  offset    Offset at which to write.
		\syncreturn
	*/
	err_t pwriteSync(
		const opattr_t *pAttr,
		const void *pBuf,
		off_t byteCount,
		off_t offset); /* inlined in fios_scheduler.h */
	
	/** \brief Closes a file synchronously.
		This function destroys the filehandle object.
	*/
	void close(); /* inlined in fios_scheduler.h */

	/** Deletion operator. Has the same effect as closing the file synchronously.
		\param[in] pPtr Filehandle object to delete.
		\see filehandle::close, scheduler::closeFileSync
	*/
	void operator delete(void *pPtr); /* inlined in fios_scheduler.h */
	
	/** Gets the file's size
		\return Size in bytes */
	inline off_t getFileSize() const { return m_fileSize; }
	
	/** Gets the file's path
		\return C-string representing the file path */
	inline const char * getPath() const { return m_path; }
	
private:
	friend class scheduler;            /**< This class is manipulated directly by the scheduler class. */
	friend class collections::list<filehandle*>;       /**< The list class needs our next ptr. */
	friend class collections::atomicList<filehandle*>; /**< The atomic list class needs our next ptr. */
	mutable platform::mutex m_objectLock; /**< Object lock for accessing member variables */
	scheduler * m_pScheduler;          /**< Scheduler associated with this filehandle */
	filehandle * m_pNext;              /**< Next pointer for queuing */
	mediafd     m_mediaFD;             /**< Media filehandle; if equal to mediafd::kINVALID_FILEHANDLE then this is a direct-from-disk filehandle */
	char        m_path[FIOS_PATH_MAX]; /**< Path used to open this file */
	off_t       m_diskOffset;          /**< Byte offset from start of disk */
	off_t       m_fileOffset;          /**< Offset within file */
	off_t       m_fileSize;            /**< Size of the file */
	U32         m_flags;               /**< Filehandle flags; see #e_OPENFLAGS */
	U32         m_pendingRequests;     /**< Number of pending mediarequests. The filehandle will not be closed as long as this is non-zero. */

	/** \brief Gets the file's byte address on disk.
		\return Byte offset from start of disk.
	*/
	off_t getDiskOffset() const;
	
}; /* class filehandle */

/*@}*/

}; /* namespace fios */

#endif /* _H_fios_filehandle */
