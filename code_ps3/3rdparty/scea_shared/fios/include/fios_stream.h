/**
	\file fios_stream.h

	Class definitions for the fios::stream class.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_stream
#define _H_fios_stream

#include "fios_types.h"
#include "fios_base.h"
#include "fios_platform.h"

namespace fios {

class streampage;
struct streamseek;

/**
 *	\addtogroup	FIOSStream	Stream APIs
 */
/*@{*/

/** \internal
	\brief Struct that tracks pending stream seeks */
typedef struct streamseek {
	off_t from;   //!< Absolute offset at which the seek starts.
	off_t to;     //!< Absolute offset that we're seeking to.
} streamseek;

/**
	The stream class represents an open stream. A stream is a large chunk of data
	where only part of the data is needed at any time, as opposed to having all
	the data resident in memory at once.
	
	A CD digital audio (CD-DA) track is a simple example of a stream: samples are
	played in linear order within the stream, and players only need data from
	the current location to play. Of course, normally streams are buffered some
	distance ahead of the current position.
	
	A FIOS stream is defined by one or more stream extents. Stream extents are
	described by a struct #streamextent_t, and can be whole files, parts of files,
	or block extents on the disc. Each extent may repeat any number of times,
	and the stream as a whole may also be repeated any number of times or forever.
	A stream can also be modified during playback, with extents added or removed
	on the fly.
*/
class FIOS_EXPORT stream : public object
{
public:
	/**
		\brief Returns the stream's callback.
		\param[out] pCallbackContext     Filled in with the callback's context pointer. (may be NULL)
		\return The stream's streamcallback_proc.
	*/
	streamcallback_proc getCallback(void **pCallbackContext = NULL) const;

	/**
		\brief Sets a new stream callback.
		\param[in]  pNewCallback		 The new callback.
		\param[in]  pNewCallbackContext  The new callback's context pointer.
		\param[out] pOldCallback         Filled in with the old callback. (may be NULL)
		\param[out] pOldCallbackContext  Filled in with the old callback's context pointer. (may be NULL)
	*/
	void setCallback(
		streamcallback_proc pNewCallback,
		void * pNewCallbackContext,
		streamcallback_proc *pOldCallback = NULL,
		void ** pOldCallbackContext = NULL);
	
	/**
		\brief Invokes the stream callback.
		\param[in]     event               Event type.
		\param[in,out] pParam              Pointer to event-specific parameters.
		\return Error returned by the callback.
	*/
	err_t callback(
		streamevent_t event,
		void *pParam = NULL);
	
	/**
		\brief Returns the stream's maximum data rate.
		\return Data rate in bytes per second.
	*/
	U64 getMaxDataRate() const;
	
	/**
		\brief Returns the stream's flags.
		\return Flags.
		\see e_STREAMFLAGS
	*/
	streamflags_t getFlags() const { return m_flags; }
	
	/**
		\brief Returns the stream's priority.
		\return Priority relative to other streams.
	*/
	prio_t getPriority() const;

	/**
		\brief Changes the stream's priority.
		\param[in] priority   New priority value.
	*/
	void setPriority(
		prio_t priority);
	
	/**
		\brief Returns the stream's name.
		\return The stream's name, which was set at creation time.
	*/
	const char * getName() const;

	/**
		\brief Adds a list of extents to the stream.
		The extents are appended to the stream's existing extents. 
		\param[in] extentCount     Number of extents.
		\param[in] pExtentArray    Array of extents to add.
		\param[in] restartStream   If the stream has stopped because it's reached the end of its data, should we restart it? Defaults to yes.
		\syncreturn
	*/
	err_t addExtents(
		U16 extentCount,
		const streamextent_t *pExtentArray,
		bool restartStream = true);
	
	/**
		\brief Adds a single extent to the stream.
		\param[in] pExtent         Extent to add.
		\param[in] restartStream   If the stream has stopped because it's reached the end of its data, should we restart it? Defaults to yes.
		\syncreturn
	*/
	inline err_t addExtent(
		const streamextent_t *pExtent,
		bool restartStream = true) { return addExtents(1,pExtent,restartStream); }
	
	/**
		\brief Seeks within the stream.
		Because streams read ahead and are buffered, this function needs an
		extra parameter that isn't in filehandle::seek -- you must tell it
		after what offset in the stream you wish the seek to activate.
		
		Example: Suppose you have a stream which has buffered blocks 500 through
		600 of its extents. You are currently processing block 500, and you discover a
		directive that says that you will need to seek from block 550 to 700. In
		that case you could call
		stream::seek(550*blockSize,kSEEK_SET,700*blockSize,kSEEK_SET).
		
		Both the "from" and "to" offsets are interpreted according to their own whence
		parameter. #kSEEK_SET means the offset is an absolute offset from the start of
		the stream, and #kSEEK_END means the offset is relative to the end of the
		stream. The whence value #kSEEK_CUR is interpreted differently for the "from"
		and "to" offsets: for "from" it means that the offset is relative to the current
		stream offset (the value from stream::getOffset()), and for "to" it means
		that the offset is relative to the "from" offset.
		
		If the stream has not yet buffered the "from" offset, the seek will be
		queued and processed at an appropriate time.  Multiple seeks may be queued,
		but once they are queued they are committed and cannot be revoked.
		
		If your stream has the #kSTREAMF_RECYCLEEXTENTS flag set, you may not be
		able to seek backwards very far because the stream may have recycled the
		old extents already. If you attempt to seek backwards into a recycled extent,
		the stream will return #kERR_BADOFFSET and will not change the current location.
		
		The buffer at pResult will be filled in with the final offset. If toWhence is
		#kSEEK_SET, this will be the same as the value passed in for the "to" parameter.
		If toWhence is #kSEEK_END or #kSEEK_CUR, this will be the "to" value resolved
		appropriately into an absolute offset.
		
		\param[in]  from        Seek after reaching this point in the stream. If this offset is smaller than the current offset, all buffered data is abandoned.
		\param[in]  fromWhence  How to interpret the from offset:  #kSEEK_SET for absolute offset from the start of the stream, #kSEEK_CUR for a relative offset from stream::getOffset(), and #kSEEK_END for a relative offset from the end of the stream.
		\param[in]  to          New offset
		\param[in]  toWhence    How to interpret the new offset: #kSEEK_SET for absolute offset from the start of the stream, #kSEEK_CUR for a relative offset from the "from" offset, and #kSEEK_END for a relative offset from the end of the stream.
		\param[out] pResult     Final offset (may be NULL)
		\syncreturn
	*/
	err_t seek(
		off_t from,
		whence_t fromWhence,
		off_t to,
		whence_t toWhence,
		off_t *pResult = NULL);
	
	/** \brief Gets the stream's offset.
		For unmanaged streams, this returns the offset of the next byte that
		will be sent to the client with #kSTREAMEVENT_DATAREADY. For managed
		streams, this returns the offset of the next byte that will be sent
		to the client by stream::getNextPage().

		The offset returned here is the offset from the start of the stream. If
		you are dynamically modifying the stream by adding extents to it,
		the offset will continue to rise. Recycling extents has no effect on
		the offset.
		
		When a stream loops, its offset returns to zero.
		
		\return Current offset in bytes from the start of the stream. */
	off_t getOffset() const;
	
	/** \brief Returns the sum of the lengths of all extents in the stream.
		
		Loops in individual extents are counted, but loops in the stream as a
		whole are not. If any single stream extent has an infinite loop, the
		stream length is reported as -1.
		
		When there are no infinite loops in the stream extents, the length will
		be the sum of (byteCount * loopCount) across all extents.
	*/
	off_t getLength() const;
	
	/** \brief Returns the total length of the stream, including all loops.
		When there are no infinite loops, the length will be the sum of
		(byteCount * loopCount * loopCount) across all extents.
	*/
	off_t getLoopedLength() const;
	
	/** \brief Returns the requested loop count of the stream.
		This value will be the same one passed in by the client in the loopCount
		of the streamattr_t that was used to initialize the stream. An infinite
		loop count is indicated by a negative value.
	*/
	I32 getLoopCount() const { return m_streamLoopCount; }
	
	/**
		\brief   Temporarily suspends a stream.

		A stream may be suspended to stop all I/O temporarily. This call increments
		the stream's suspend count; while the suspend count is non-zero the stream
		is suspended.  Managed streams can return buffered data while suspended, but
		new I/O will not be issued until the stream's suspend count reaches zero. 
		
		\param[in] pWhy    Optional string which can be used to indicate why the stream is being suspended. (may be NULL)
		\see stream::getSuspendCount(), stream::resume()
	*/
	void suspend(const char *pWhy = NULL);

	/**
		\brief   Returns the stream's current suspend count.

		The suspend count indicates how many times the stream has been
		suspended. Upon creation a stream's suspend count will be zero. This
		value is only modified by calls to stream::suspend() and
		stream::resume(), which should be equally balanced.
		
		\return              The current suspend count.
	*/
	U32 getSuspendCount() const;
	
	/**
		\brief   Checks to see whether a stream is suspended.
		\return  True if the stream is suspended, false otherwise.
	*/
	inline bool isSuspended() const { return (getSuspendCount() > 0); }
	
	/**
		\brief   Resumes a stream after suspension.

		This call decrements the stream's suspend count.  If the suspend count reaches
		zero as a result, the stream is resumed and I/O is started immediately. If the
		suspend count is already zero, this call has no effect.

		\param[in] pWhy     Optional string which can be used to indicate why the stream was suspended. (may be NULL)
	*/
	void resume(const char *pWhy = NULL);
	
	/** \brief Indicates whether the stream is idle.
		Returns true if the stream has no pending I/O and there are no locked pages.
		\return Boolean indicating idle.
	*/
	bool isIdle() const;
	
	/**
		\brief  Closes a stream.
		This call closes a stream and deletes the object. It's equivalent to
		simply deleting the stream.
		\see stream::operator delete, scheduler::closeStream
	*/
	void close(); // inlined in fios_scheduler.h

	/** Deletion operator. Has the same effect as closing the stream.
		\param[in] pPtr Stream object to delete.
		\see stream::close, scheduler::closeStream
	*/
	void operator delete(void *pPtr); // inlined in fios_scheduler.h
	
	/** Indicates whether this stream uses managed buffers.
		\return True if the stream uses managed buffers, false otherwise.
	*/
	inline bool isManaged() const { return ((getFlags() & kSTREAMF_UNMANAGED) == 0); }
	
	/** \brief Gets the next page from a stream that uses managed buffers.
		
		If the page is not available, you may choose to block the calling thread and
		wait for it to become available by passing a true value in the block parameter.
		If you choose non-blocking and the next page is not available, this function will
		return false and give you a NULL buffer.
		
		The page buffer is owned by FIOS. Do not call free() or delete on it.
		When you're done with it, you must call stream::releasePage() to send the
		memory back to the buffer pool.
		
		You can get more than one page at a time, but remember that every page you hold
		is occupying buffer memory and must be eventually released.
		
		\param[out]  pPage       Filled in with the next page of the stream.
		\param[out]  pPageSize   Filled in with the size of this page. This value is normally the page size of the stream, but may be smaller at the end of the stream. May be NULL.
		\param[out]  pPageOffset Filled in with the offset of this page from the start of the stream. May be NULL.
		\param[in]   block       Whether to block the calling thread until the page is available.
		\return  True if the next page was available, false otherwise.
	*/
	bool getNextPage(
		void **pPage,
		size_t *pPageSize = NULL,
		off_t *pPageOffset = NULL,
		bool block = true);
	
	/** \brief Releases a previously acquired page on a stream that uses managed buffers.
		\param[in]   pPage     Stream page.
		\see stream::getNextPage()
	*/
	void releasePage(
		void *pPage);
	
	/** \brief Returns this stream's page size.
		Currently every stream has the same page size.
		\return The page size of this stream.
		\see getDefaultPageSize
	*/
	inline U32 getPageSize() const { return stream::getDefaultPageSize(); }

	/** \brief Returns this stream's page alignment as a log value.
		This value is normally the log of the alignment. An 8-byte alignment
		(two to the third power) causes this function to return 3.
		Currently every stream has the same page alignment.
		\return The page alignment of this stream.
		\see getDefaultPageAlignment()
	*/
	inline U32 getPageAlignment() const { return stream::getDefaultPageAlignment(); }
	
	/** \brief Returns the default streaming page size.
		\return Default page size.
	*/
	static inline U32 getDefaultPageSize() { return 65536; }

	/** \brief Returns the default streaming page alignment as a log value.
		This value is normally the log of the alignment.  An 8-byte alignment
		(two to the third power) causes this function to return 3.
		\return Default page alignment.
	*/
	static inline U32 getDefaultPageAlignment() { return 15; }
	
	/** \brief Indicates whether the stream's I/O is complete.
		This value is set to true just before the kSTREAMEVENT_IOCOMPLETE callback is made.
		\return True if the I/O has been completed.
		\see kSTREAMEVENT_IOCOMPLETE
	*/
	bool ioComplete() const;
	
	/** \brief Indicates whether all the stream's I/O has been delivered to the client.
		This value is set to true just before the kSTREAMEVENT_DATACOMPLETE callback is made.
		\return True if all data has been delivered.
		\see kSTREAMEVENT_DATACOMPLETE
	*/
	bool dataComplete() const;
	
private:
	friend class scheduler;                 /**< The scheduler manipulates this class directly. */
	friend class collections::list<stream*>;       /**< The list class needs our next ptr. */
	friend class collections::atomicList<stream*>; /**< The atomic list class needs our next ptr. */
	scheduler *         m_pScheduler;       /**< The scheduler associated with this object. */
	mutable platform::mutex m_objectLock;   /**< Lock for accessing member variables. */
	mutable platform::cond m_pageCond;      /**< Cond used to signal new pages, protected by m_objectLock */
	mutable stream *    m_pNext;            /**< Next ptr for queuing */
	abstime_t           m_deadline;         /**< deadline by which the stream should have its initial chunk ready */
	prio_t              m_priority;         /**< this stream's priority relative to other streams */
	streamflags_t       m_flags;            /**< stream flags */
	U64                 m_maxDataRate;      /**< maximum data rate, in bytes per second. */
	streamcallback_proc m_pCallback;        /**< stream callback */
	void *              m_pCallbackContext; /**< stream callback context */
	U16                 m_extentArrayAllocation;  /**< The number of slots allocated in the extent array. */
	U16                 m_extentArrayCount; /**< Number of slots that have valid extent data. */
	streamextent_t *    m_pExtentArray;     /**< Extent array holds all the extents in the stream. */
	U16                 m_ioExtent;         /**< Which extent we're currently on. */
	off_t               m_ioOffset;         /**< Offset from the start of the stream to the end of the last op we've issued. */
	off_t               m_ioExtentOffset;   /**< Offset within the current extent. */
	off_t               m_clientOffset;     /**< Offset of next I/O that will be sent to the client. */
	I16                 m_ioExtentLoopCount; /**< Number of times we've looped in the extent. */
	I32                 m_ioStreamLoopCount; /**< Number of times we've looped the stream. */
	I32                 m_streamLoopCount;  /**< Number of times we should loop the stream, or <0 for infinite. */
	bool                m_waitingForOpen;   /**< Set when we're waiting for a file to open. */
	off_t               m_totalLength;      /**< Total length of the stream (no loops), or <0 for infinite. */
	off_t               m_loopedLength;     /**< Total length of the stream (w/ loops), or <0 for infinite. */
	U16                 m_suspendCount;     /**< The suspend count for the stream. */
	U32                 m_streamPageCount;  /**< The number of stream pages that belong to this stream. */
	char                m_name[32];         /**< Name of this stream for debugging. */
	op *				m_pOps[4];          /**< Four read ops queued at once. */
	filehandle *        m_pFH;              /**< One filehandle open at any given time. */
	bool                m_justSeeked;       /**< Set to true just after an immediate seek. Notifies scheduler::enqueueStreamIO that the previous file needs to be closed. */
	bool                m_ioComplete;       /**< Whether this stream's I/O is complete. */
	bool                m_dataComplete;     /**< Whether this stream's data has all been delivered. */
	collections::list<streampage *> m_pStreamPages; /**< Ordered list of stream pages. */
	collections::array<streamseek> m_streamSeeks; /**< Array of pending seeks */
	
	/** Creates a stream. Streams should only be created by the scheduler that owns them. */
	stream();
	/** Destroys a stream. Streams should only be destroyed by the scheduler that owns them. */
	~stream();
	/** Resets a stream and clears out any memory allocated by it in preparation for deletion. */
	void reset();
	/** \brief Gets the current extent in the stream.
		This function returns the current extent in the stream, or NULL if we're at the
		end of the stream.
		\return The current extent, or NULL if at the end of the stream.
	*/
	streamextent_t * getCurrentExtent();
	/** \brief Gets the next extent in the stream.
		This function handles extent looping and recycling. This call may update
		the stream's member variables, including m_ioExtent, m_ioExtentOffset,
		and m_ioExtentLoopCount.
		\param[in] pStream  Stream from which to get the next extent.
		\return The new current extent, or NULL if at the end of the stream.
	*/
	streamextent_t * getNextExtent();
	/** \brief Recalculates the next extent state for a seek.
		\param[in] newOffset   Offset within the stream that will be the next byte returned.
	*/
	void recalculateNextExtent(off_t newOffset);
	
}; /* class stream */



/** \internal
	Page tracking struct for stream data.
*/
class streampage
{
public:
	streampage *m_pNext;     //!< Next page in the stream
	stream *pOwningStream;   //!< Owning stream, or NULL if free
	void *  pPageBuffer;     //!< Page buffer
	off_t   streamOffset;    //!< Absolute offset of this buffer within the stream
	size_t  pageSize;        //!< Capacity of the page
	size_t  dataSize;        //!< How much data is in the page
	size_t  dataOffset;      //!< Offset to the start of the data. Often but not always zero.
	U32     ioDone : 1;      //!< Whether the I/O has completed
	U32     locked : 1;      //!< Whether client is actively holding this page
	U32     recycled : 1;    //!< Whether this page was recycled with valid data still in it
	U32     cancelled : 1;   //!< Set when the stream no longer wants this page
	U32     reserved : 28;   //!< Other flags
}; /* class streampage */



/*@}*/

}; /* namespace fios */

#endif /* _H_fios_stream */
