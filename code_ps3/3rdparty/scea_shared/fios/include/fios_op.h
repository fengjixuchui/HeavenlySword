/**
	\file fios_op.h

	Class definitions for the fios::op class.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_op
#define _H_fios_op

#include "fios_types.h"
#include "fios_base.h"
#include "fios_platform.h"
#include "fios_time.h"
#include <stdarg.h>

namespace fios {

// Forward declarations
class opWaitData;
class streampage;

/**
 *  \addtogroup FIOSOp Operations
 */
/*@{*/

/**
	The operation object, which is returned by every async operation.
*/
class FIOS_EXPORT op
{
public:
	/** \brief Waits for an operation to complete.
	    This is a special sync variant of wait which deletes the operation afterward.
		\param[out]  pFulfilledSize    Returns the fulfilled size, as in op::getFulfilledSize(). May be NULL.
	    \return The result of the operation. */
	err_t syncWait(
		off_t *pFulfilledSize = NULL);
	
	/**
		\brief Waits for a va_list of arguments to complete with a deadline.
		If all operations complete successfully, the result will be #kNOERR.
		If at least one operation did not complete by the deadline, the
		result will be #kERR_TIMEOUT. Otherwise, the result code will be
		the error from one of the operations in the list that completed
		with an error.
		\param[in] deadline  Deadline by which the operations must complete, or 0 to wait forever.
		\param[in] pOp       First argument in the list.
		\param[in] ap        Argument list.
		\return #kNOERR if all operations completed successfully, #kERR_TIMEOUT if at least one operation did not complete by the deadline, or otherwise an error from some operation that finished with an error.
		\see op::waitListDeadline, op::waitListTimeout, op::waitList, op::waitMultipleDeadline, op::waitMultipleTimeout, op::waitMultiple, op::wait, op::waitDeadline, op::waitTimeout
	*/
	static err_t vwaitMultipleDeadline(
		abstime_t deadline,
		op *pOp,
		va_list ap);

	/**
		\brief Waits for all operation arguments to complete with a deadline.
		If all operations complete successfully, the result will be #kNOERR.
		If at least one operation did not complete by the deadline, the
		result will be #kERR_TIMEOUT. Otherwise, the result code will be
		the error from one of the operations in the list that completed
		with an error.
		\param[in] deadline  Deadline by which the operations must complete, or 0 to wait forever.
		\param[in] pOp       Operation handle.
		\param[in] ...       List of additional operations, terminated by NULL.
		\return #kNOERR if all operations completed successfully, #kERR_TIMEOUT if at least one operation did not complete by the deadline, or otherwise an error from some operation that finished with an error.
		\see op::waitListDeadline, op::waitListTimeout, op::waitList, op::waitMultipleTimeout, op::waitMultiple, op::wait, op::waitDeadline, op::waitTimeout
	*/
	static err_t waitMultipleDeadline(
		abstime_t deadline,
		op *pOp,
		...) FIOS_ATTRIBUTE_GCC((__sentinel__));
	
	/**
		\brief Waits for all operation arguments to complete with a relative timeout.
		If all operations complete successfully, the result will be #kNOERR.
		If at least one operation did not complete by the deadline, the
		result will be #kERR_TIMEOUT. Otherwise, the result code will be
		the error from one of the operations in the list that completed
		with an error.
		\param[in] timeout   Timeout (relative to the current time), or 0 to wait forever.
		\param[in] pOp       Operation handle.
		\param[in] ...       List of additional operations, terminated by NULL.
		\return #kNOERR if all operations completed successfully, #kERR_TIMEOUT if at least one operation did not complete by the deadline, or otherwise an error from some operation that finished with an error.
		\see op::waitListDeadline, op::waitListTimeout, op::waitList, op::waitMultipleDeadline, op::waitMultiple, op::wait, op::waitDeadline, op::waitTimeout
	*/
	static err_t waitMultipleTimeout(
		abstime_t timeout,
		op *pOp,
		...) FIOS_ATTRIBUTE_GCC((__sentinel__));
	
	/**
		\brief Waits for all operation arguments to complete with a relative timeout.
		If all operations complete successfully, the result will be #kNOERR.
		If at least one operation did not complete by the deadline, the
		result will be #kERR_TIMEOUT. Otherwise, the result code will be
		the error from one of the operations in the list that completed
		with an error.
		\param[in] pOp       Operation handle.
		\param[in] ...       List of additional operations, terminated by NULL.
		\return #kNOERR if all operations completed successfully, #kERR_TIMEOUT if at least one operation did not complete by the deadline, or otherwise an error from some operation that finished with an error.
		\see op::waitListDeadline, op::waitListTimeout, op::waitList, op::waitMultipleDeadline, op::waitMultipleTimeout, op::wait, op::waitDeadline, op::waitTimeout
	*/
	static err_t waitMultiple(
		op *pOp,
		...) FIOS_ATTRIBUTE_GCC((__sentinel__));
	
	/** \brief Waits for a list of operations to complete with a deadline.
		If all operations complete successfully, the result will be #kNOERR.
		If at least one operation did not complete by the deadline, the
		result will be #kERR_TIMEOUT. Otherwise, the result code will be
		the error from one of the operations in the list that completed
		with an error.
		\param[in] numOps    Number of operations to wait for.
		\param[in] pOpList   List of operations.
		\param[in] deadline  Deadline by which the operations must complete, or 0 to wait forever.
		\return #kNOERR if all operations completed successfully, #kERR_TIMEOUT if at least one operation did not complete by the deadline, or otherwise an error from some operation that finished with an error.
		\see op::waitListTimeout, op::waitList, op::waitMultipleDeadline, op::waitMultipleTimeout, op::waitMultiple, op::wait, op::waitDeadline, op::waitTimeout
	*/
	static err_t waitListDeadline(
		unsigned int numOps,
		op * const *pOpList,
		abstime_t deadline);
	
	/** \brief Waits for a list of operations to complete with a relative timeout.
		If all operations complete successfully, the result will be #kNOERR.
		If at least one operation did not complete by the deadline, the
		result will be #kERR_TIMEOUT. Otherwise, the result code will be
		the error from one of the operations in the list that completed
		with an error.
		\param[in] numOps    Number of operations to wait for.
		\param[in] pOpList   List of operations.
		\param[in] timeout   Timeout (relative to the current time), or 0 to wait forever.
		\return #kNOERR if all operations completed successfully, #kERR_TIMEOUT if at least one operation did not complete by the deadline, or otherwise an error from some operation that finished with an error.
		\see op::waitListDeadline, op::waitList, op::waitMultipleDeadline, op::waitMultipleTimeout, op::waitMultiple, op::wait, op::waitDeadline, op::waitTimeout
	*/
	static inline err_t waitListTimeout(
		unsigned int numOps,
		op * const *pOpList,
		abstime_t timeout) { return waitListDeadline(numOps,pOpList,timeout ? (FIOSGetCurrentTime() + timeout):timeout); }
	
	/** \brief Waits for a list of operations to complete.
		If all operations complete successfully, the result will be #kNOERR.
		Otherwise, the result code will be the error from one of the operations in
		the list that completed with an error.
		\param[in] numOps    Number of operations to wait for.
		\param[in] pOpList   List of operations.
		\return #kNOERR if all operations completed successfully, #kERR_TIMEOUT if at least one operation did not complete by the deadline, or otherwise an error from some operation that finished with an error.
		\see op::waitListDeadline, op::waitListTimeout, op::waitMultipleDeadline, op::waitMultipleTimeout, op::waitMultiple, op::wait, op::waitDeadline, op::waitTimeout
	*/
	static inline err_t waitList(
		unsigned int numOps,
		op * const *pOpList) { return waitListDeadline(numOps,pOpList,0); }

	/** \brief Waits for an operation to complete with a deadline.
		If the operation has not completed by the deadline, 
		\param[in] deadline  Deadline, or 0 to wait forever.
		\return Result from the operation.
		\see op::waitListDeadline, op::waitListTimeout, op::waitList, op::waitMultipleDeadline, op::waitMultipleTimeout, op::waitMultiple, op::wait, op::waitTimeout
	*/
	inline err_t waitDeadline(abstime_t deadline) { op *pOp = this; return op::waitListDeadline(1,&pOp,deadline); }
	
	/** \brief Waits for an operation to complete with a relative timeout.
		\param[in] timeout   Timeout (relative to the current time), or 0 to wait forever.
		\return Result from the operation.
		\see op::waitListDeadline, op::waitListTimeout, op::waitList, op::waitMultipleDeadline, op::waitMultipleTimeout, op::waitMultiple, op::wait, op::waitDeadline
	*/
	inline err_t waitTimeout(abstime_t timeout) { op *pOp = this; return op::waitListDeadline(1,&pOp,FIOSGetCurrentTime() + timeout); }
	
	/** \brief Waits for an operation to complete.
		\return Result from the operation.
		\see op::waitListDeadline, op::waitListTimeout, op::waitList, op::waitMultipleDeadline, op::waitMultipleTimeout, op::waitMultiple, op::waitDeadline, op::waitTimeout
	*/
	inline err_t wait() { op *pOp = this; return op::waitListDeadline(1,&pOp,0); }
	
	/** \brief Cancels an operation.
		\see scheduler::cancelOp
	*/
	void cancel(); // inlined in fios_scheduler.h
	
	/** \brief Placement new.
		\param[in] size         Size to allocate.
		\param[in] pPlacement   Placement.
		\return Returns pPlacement.
	*/
	static void * operator new(size_t size, void *pPlacement) { FIOS_UNUSED(size); return pPlacement; }
	
	/** \brief Frees an operation pointer.
		\param[in] pMem   Operation pointer to delete.
	*/
	static void operator delete(void *pMem); // inlined in fios_scheduler.h
	
	/** \brief Indicates whether an operation is complete.
		\return True if the operation has completed, false otherwise. */
	inline bool isDone() const { return (m_done != 0); }
	
	/** \brief Checks the cancellation status of this operation.
	    \return True if the operation has been cancelled, false otherwise. */
	inline bool isCancelled() const { return FIOS_UNLIKELY((m_opflags & kOPF_CANCELLED) == kOPF_CANCELLED); }
	
	/**
		\brief  Returns the requested data size in bytes.
		This call returns the requested data size from the original call. For calls such
		as scheduler::readFile() and scheduler::readBlocks(), this will be the number of bytes
		requested. For calls which return a file handle or other object, this will be the size
		of the output parameter.
		\result              Request size in bytes.
	*/
	inline off_t getRequestedSize() const { return m_opReqCount; }

	/**
		\brief  Returns the actual fulfilled size in bytes.
		This call returns the number of bytes from the request that have been retrieved
		so far. If the operation has completed successfully, this will be equal to the
		request size. If the operation completed with error code #kERR_EOF, this
		may be any value from 0 to the request size according to how much data was read
		before the end of the file. If an operation is not complete, this may be any
		value from 0 to the request size.
		\result              Actual transfer size in bytes.
	*/
	off_t getFulfilledSize() const;
	
	/**
		\brief  Returns the original request offset in bytes.
		This call returns the requested offset from the original call.
		\result              Offset from the original call, or 0 if not applicable. */
	inline off_t getOffset() const { return m_opOffset; }
	
	/**
		\brief  Gets the user pointer.
		\return The user pointer set at creation.
	*/
	void * getUserPtr() const { return m_attr.userPtr; }
	
	/**
		\brief  Gets the user tag.
		\return The user tag set at creation.
	*/
	U32 getUserTag() const { return m_attr.userTag; }

	/**
		\brief  Returns the data buffer.
		This call returns the data buffer for the operation. For operations such as
		scheduler::readFile() and scheduler::writeFile(), this is the buffer supplied
		by the caller. For operations which return a file handle or other object, this
		is a pointer to the output parameter.
		\return  Data buffer.
	*/
	inline void * getBuffer() const { return m_opBuffer; }

	/**
		\brief  Returns the deadline.
		\return  This operation's deadline.
	*/
	inline abstime_t getDeadline() const { return m_attr.deadline; }

	/**
		\brief  Returns the priority of an operation.
		\return Priority value.
	*/
	inline prio_t getPriority() const { return m_attr.priority; }
	
	/**
		\brief	Returns a unique serial number for this operation.
		Each client request, such as readFile(), receives a unique serial
		number that remains active until the op is complete and deleted.
		This is primarily useful for debug logging.
		
		Serial numbers start at 1 and are incremented with each new request.
		It's theoretically possible for them to wrap if you issue more than
		2^32 ops. This falls into the realm of "highly unlikely" since
		it'd require allocating a new op every 10ms for over a year.
		\return Unique serial number.
	*/
	inline U32 getSerialNumber() const { return m_serialNumber; }
	
	/**
		\brief  Returns the estimated time at which an operation will complete.

		This call returns the scheduler's current best estimate of when the operation
		will be complete.  This value may change at any time, either because additional
		I/O requests were received or because the scheduling algorithm finds an optimization
		opportunity.
		
		\note
		The scheduler's optimization algorithm does not always push operations forward. Any
		individual operation may be pushed later in the schedule, rather than earlier, as
		long as it does not violate any deadlines and results in a shorter overall time for
		the list of scheduled I/Os as a whole.
		
		\return   The current estimated time that the operation will complete.
	*/
	abstime_t getEstimatedCompletion() const; // inlined in fios_scheduler.h

	/**
		\brief  Sets a new deadline.
		This causes the operation to be rescheduled with the new deadline.  If the
		operation is currently executing then this call has no effect.
		\param[in]  deadline   The new deadline.
	*/
	void setDeadline(
		abstime_t deadline); // inlined in fios_scheduler.h
	
	/**
		\brief  Sets a new priority.
		\param[in]  priority   The new priority.
	*/
	void setPriority(
		prio_t priority); // inlined in fios_scheduler.h

	/**
		\brief  Sets a new priority and deadline.
		\param[in]  deadline   The new deadline.
		\param[in]  priority   The new priority.
	*/
	void setDeadlineAndPriority(
		abstime_t deadline,
		prio_t priority); // inlined in fios_scheduler.h
	
	/**
		\brief Returns the operation's callback.
		\param[out] pCallbackContext     Filled in with the callback's context pointer. (may be NULL)
		\return The operation's opcallback_proc.
	*/
	opcallback_proc getCallback(void **pCallbackContext = NULL) const;

	/**
		\brief Sets a new operation callback.
		\param[in]  pNewCallback		 The new callback.
		\param[in]  pNewCallbackContext  The new callback's context pointer.
		\param[out] pOldCallback         Filled in with the old callback. (may be NULL)
		\param[out] pOldCallbackContext  Filled in with the old callback's context pointer. (may be NULL)
	*/
	void setCallback(
		opcallback_proc pNewCallback,
		void * pNewCallbackContext,
		opcallback_proc *pOldCallback = NULL,
		void ** pOldCallbackContext = NULL);
	
	/**
		\brief Invokes the operation callback.
		\param[in]     event               Event type.
		\param[in,out] pParam              Pointer to event-specific parameters.
		\return Error returned by the callback.
	*/
	err_t callback(
		opevent_t event,
		void *pParam = NULL);

private:
	/** \brief Stage definitions, for operations that require more than one operation. */
	typedef enum e_OPSTAGES {
		kOPSTAGE_INVALID = 0,             /**< Invalid stage. */
		kOPSTAGE_CALLBACK = 1,            /**< The last stage for many operations. After the mediarequest completes, the callback is called. */
		kOPSTAGE_EOFCALLBACK,             /**< Same as kOPSTAGE_CALLBACK, but changes the return value to #kERR_EOF. */

		kOPSTAGE_GETFILESIZE_STAT,        /**< MEDIAACTION_STAT for getFileSize, output translation must be done. */

		kOPSTAGE_FILEEXISTS_STAT,         /**< MEDIAACTION_STAT for fileExists, output translation must be done. */
		kOPSTAGE_DIREXISTS_STAT,          /**< MEDIAACTION_STAT for directoryExists, output translation must be done. */
		kOPSTAGE_ITEMEXISTS_STAT,         /**< MEDIAACTION_STAT for itemExists, output translation must be done. */
		
		kOPSTAGE_OPENFILE_OPEN,           /**< open stage of openFile, output translation must be done. */

		kOPSTAGE_CLOSEFILE_DEALLOCATE,    /**< deallocate stage of closeFile, deallocate the FH */

		kOPSTAGE_STREAMEXTENT_OPEN,       /**< opening a streamextent, copy the native FH into the extent */
		kOPSTAGE_STREAMEXTENT_READ        /**< just read a streamextent, handle actCount and data delivery */
	} opstage_t;
	
	friend class scheduler;               /**< This class is manipulated directly by the scheduler class. */
	friend class collections::list<op*>;       /**< The list class needs our next ptr. */
	friend class collections::atomicList<op*>; /**< The atomic list class needs our next ptr. */
	mutable op *   m_pNext;               /**< Next pointer for linked lists */
	opattr_t       m_attr;                /**< This operation's attributes */
	platform::atomicU32 m_opflags;        /**< Operation flags, including those set after the client's opflags have been set. */
	char           m_path[FIOS_PATH_MAX]; /**< Normalized path for this operation */
	U32            m_serialNumber;        /**< Unique serial number for this op */
	mediarequest   m_request;             /**< Current media request represented by this operation. One operation may generate multiple media requests! */
	platform::atomicBool   m_done;        /**< Whether this operation has completed. */
	scheduler      *m_pScheduler;         /**< Scheduler associated with this operation. */
	filehandle     *m_pFH;                /**< Filehandle, if this op is associated with a filehandle (open, read, write, close). */
	stream         *m_pStream;            /**< Owning stream, if this op is associated with a stream. */
	streampage     *m_pStreamPage;        /**< Associated stream page, if this op is associated with a stream data transfer. */
	off_t          m_opReqCount;          /**< Original request size (may not be equal to request size) */
	off_t          m_opActCount;          /**< Actual bytes transferred */
	off_t          m_opOffset;            /**< Original offset (may not be equal to request offset) */
	void *         m_opBuffer;            /**< Original buffer (may not be equal to request buffer) */
	abstime_t      m_estimatedCompletion; /**< Estimated completion time */
	opstage_t      m_opStage;             /**< For multi-stage ops, the current stage. */
	err_t          m_stickyErr;           /**< For multi-stage ops, an error preserved from an earlier stage. */
	opWaitData *   m_pWaitData;           /**< Info on thread waiting for this op in a wait() call. Protected by the scheduler's m_opCallbackLock. */

	/** \brief Resets an operation and makes it ready for reuse */
	void reset();
	
	/** \brief Copies data from a completed op
		\param[in] pCompletedOp    Recently completed op that may overlap this one.
		\return TRUE if this operation was completed by the given data */
	bool copyDataFrom(op *pCompletedOp);
	
	/**	\brief Used by the implementation of op::waitListDeadline().
		\param[in] pWaitData Wait data.
		\param[in] pOp Operation.
	*/
	static void waitDone(
		opWaitData *pWaitData,
		op *pOp);

public:
	/** \internal
		\brief Constructor. Sets a check value in debug builds. */
	op();
	/** \internal
		\brief Check value */
	U32 m_checkValue;
	/** \internal
		\brief Class ID */
	U32 m_classID;

}; /* class op */

/*@}*/

}; /* namespace fios */

#endif /* _H_fios_op */
