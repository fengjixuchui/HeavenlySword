/**
	\file fios_types.h

	Types and enumerations for the File I/O Scheduler.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/

#ifndef _H_fios_types
#define _H_fios_types

#include "sceacommon/include/sceabasetypes.h"
#include "fios_platform_imp.h"

namespace fios {

/* Forward declarations */
class scheduler;
class media;
class catalogcache;
class filehandle;
class stream;
class op;

/**
 * \addtogroup Types_and_Enumerations	Public Types and Enumerations
 * @{
 */

/**
	\brief Error code.
	Many operations return an error code.
	\see	e_ERRORS
*/
typedef I32 err_t;

/**
	\brief	Errors
*/
enum e_ERRORS
{
	/** No error occurred */
	kNOERR         = 0,    /* 0000 */
	
	/** I/O is in progress */
	kERR_INPROGRESS = 1,   /* 0001 */
	
	/** Callback return code: event was not handled */
	kERR_UNKNOWNCALLBACKEVENT = 2, /* 0002 */
	
	/** Callback return code: no buffer ready yet, try again later */
	kERR_NOTREADY = 3, /* 0003 */
	
	/** Deadline reached in op::waitListDeadline() or equivalent, and I/O has not completed yet */
	kERR_TIMEOUT   = -128, /* FF80 */
	/** Bad path argument, or file not found */
	kERR_BADPATH   = -129, /* FF7F */
	/** Bad filehandle argument */
	kERR_BADHANDLE = -130, /* FF7E */
	/** Bad parameter (invalid or illegal) */
	kERR_BADPARAM     = -131, /* FF7D */
	/** Operation was cancelled */
	kERR_CANCELLED = -132, /* FF7C */
	/** End-of-file was reached during a read; some data may have been returned prior to the EOF. Check the actual bytes transferred. */
	kERR_EOF       = -133, /* FF7B */
	/** Out of memory. */
	kERR_NOMEM     = -134, /* FF7A */
	/** Bad offset during seek */
	kERR_BADOFFSET = -135, /* FF79 */
	/** Callback was NULL when it shouldn't have been */
	kERR_BADCALLBACK = -136, /* FF78 */
	/** Some platform-specific error occurred that is not enumerated here (for example, too many open files) */
	kERR_PLATFORM = -137, /* FF77 */
	/** Access was denied, you don't have appropriate security permission */
	kERR_ACCESS = -138, /* FF76 */
	
	/** Write not supported by media or file access permissions */
	kERR_READONLY  = -144, /* FF70 */
	/** Block APIs not supported by media */
	kERR_NOBLOCK   = -145, /* FF6F */
	/** Path APIs not supported by media */
	kERR_NOPATH    = -146, /* FF6E */
	/** Media is gone and callback indicates it won't return. All operations are terminated with this error until the media returns. */
	kERR_MEDIAGONE = -145  /* FF6D */
};

/*!
	\brief File offset or size.
	This type is parallel to the POSIX off_t, and is used for both offsets and
	sizes. It's large (normally 64 bits) so that it can represent offsets in
	files larger than 2GB, and explicitly signed so that it may contain
	negative offsets.
*/
typedef I64 off_t;

/** \brief Maximum value that can be represented by a fios::off_t. */
#define FIOS_OFF_T_MAX       off_t(0x7FFFFFFFFFFFFFFFLL)

/** \brief Minimum (negative) value that can be represented by a fios::off_t. */
#define FIOS_OFF_T_MIN       off_t(-FIOS_OFF_T_MAX - 1)

/**
	\brief Absolute time.
	This type is a cross-platform absolute time incrementer with an arbitrary
	base and constant frequency. It's used for deadlines and timeouts. To make
	certain math calculations easier it's always a signed value with at least
	64 bits.
	
	This time format is not designed for persistence; in many implementations
	it represents the number of "ticks" of arbitrary length (often clock cycles)
	since the system booted.
	\see	FIOSTime
*/
typedef platform::abstime_t abstime_t;

/**
	\brief Calendar time.
	This type is a compact, persistent, cross-platform UTC-based calendar time with a
	nanosecond resolution. Functions are provided to convert it to and from each
	platform's common native time specifications. This format uses the Unix epoch
	of January 1, 1970 CE and reserves the high bit for future expansion. It
	will overflow near the end of the year 2262 CE.
	
	This time format is designed for persistence and cross-platform usage
	when necessary. Each value represents a fixed point in time that is
	consistent across platforms. See the Wikipedia entry on UTC for more details:
	http://en.wikipedia.org/wiki/Coordinated_Universal_Time
	
	\note UTC requires that the platform implement the notion of "leap seconds"
	for complete precision. Some platforms which do not support fully-compliant
	UTC may provide calendar representations of the UTC time which are different
	by a few seconds from the representation provided by another platform.
	For example, a datetime_t value of X might be represented as
	2007-04-13 00:00:00.000 by one platform, while another platform might represent
	the same value as 2007-04-13 00:00:01.000 -- one second off. However, these
	values will be essentially "correct" for most intents and purposes as long as they are
	used on the same platform where they were created, or used in situations where a small
	clock skew is acceptable. 
*/
typedef U64 datetime_t;

/**
	\brief Priority.
	All operations are given a priority value. When multiple operations are
	running at the same time and cannot all be completed by the deadline, the
	priority is used to resolve conflicts: higher numerical priorities are
	given precedence. For example, a priority of 10 is given precedence
	over a priority of 3.
	\see	e_PRIORITIES
*/
typedef I8 prio_t;

/**
	\brief Media event
	Enumerated value passed to the mediacallback_proc to indicate an event
	that must be handled.
	\see	e_MEDIAEVENTS
	\see	mediacallback_proc
*/
typedef U8 mediaevent_t;

/**
	\brief Stream event
	Enumerated value passed to the streamcallback_proc to indicate an event
	that must be handled.
	\see	e_STREAMEVENTS, streamcallback_proc
*/
typedef U8 streamevent_t;

/**
	\brief Operation event
	Enumerated value passed to the opcallback_proc to indicate an event
	that may be handled.
	\see	e_OPEVENTS, opcallback_proc
*/
typedef U8 opevent_t;

/**
	\brief Whence value for seeks
	This type is used for seek operations on fios::file and fios::stream.
	\see	e_WHENCE
*/
typedef U8 whence_t;

/**
	\brief IO flags used by the media object.
	\see e_IOFLAGS
*/
typedef U8 ioflags_t;

/**
	\brief Identifies a statistic that can be requested from a scheduler
	\see	e_STATTYPE, scheduler::getStatistics()
*/
typedef U8 stattype_t;

/**
	\brief Identifies a media action.
	\see e_MEDIAACTION, media::executeIOP(), mediaioparams
*/
typedef int mediaaction_t;

/**
	\brief Coefficients of a media operation's performance characteristic equation.
	\see media::executeIOP(), mediaioparams
*/
typedef int pcecoefficient_t;

/**
	\brief  Priority values

	Priorities range from -128 to 127, with -128 being the lowest priority and 127 the
	highest. The choice of priorities is entirely up to the client.
	
	Remember that priority values are only used when the scheduler is unable to meet the
	deadlines requested; a lower-priority request may be serviced before a higher-priority
	request as long as both are completed by their respective deadlines.
*/
enum e_PRIORITIES
{
	kPRIO_MIN      = -128,  /**< Minimum priority */
	kPRIO_DEFAULT  = 0,     /**< Suggested default priority */
	kPRIO_MAX      = 127    /**< Maximum priority */
};

/**
	\brief	Media events
	\see mediacallback_proc
*/
enum e_MEDIAEVENTS
{
	/**
		\brief Media was unexpectedly ejected or otherwise gone.

		Put up some UI indicating that the media is gone and wait
		for it to return. If a scheduler is associated with the media, it will be
		suspended until the callback has returned: new I/O requests will be accepted
		but none will be serviced until the callback is done.
		
		You can poll the current media by calling media::readIdentifier() to see
		if the currently-inserted media is the same as the old media. You should
		not return until the old media has been reinserted.
		
		If your callback returns #kERR_MEDIAGONE then all operations are cancelled; if
		your callback returns #kNOERR then the scheduler (if any) is resumed. These are
		the only defined behaviors; other return values currently have the same effect as
		#kERR_MEDIAGONE but this may change in the future.
	*/
	kMEDIAEVENT_GONE	= 1,
	/**
		\brief Media I/O has completed.

		Parameter is a pointer to the struct mediaioparams for the request that
		is about to be issued.  Normally for internal use only: Your callback will only
		receive this message if you are calling the media driver directly and
		explicitly put your callback into the struct mediaioparams.
	*/
	kMEDIAEVENT_IOSTARTING = 2,
	/**
		\brief Media I/O has completed.

		Parameter is a pointer to the struct mediaioparams for the request that
		just completed.  Normally for internal use only: Your callback will only
		receive this message if you are calling the media driver directly and
		explicitly put your callback into the struct mediaioparams.
	*/
	kMEDIAEVENT_IOCOMPLETE = 3
};

/**
	\brief IO flags for media
*/
enum e_IOFLAGS
{
	/** Set to indicate that an I/O is speculative and should not be executed. */
	kIOF_SPECULATIVE = (1<<0),
	/** Set to indicate that an I/O should not use any caches but always read or write directly to the media. */
	kIOF_DONTUSECACHE = (1<<1),
	/** Set to indicate that the data returned from an I/O should not be saved in any RAM caches. */
	kIOF_DONTFILLRAMCACHE = (1<<2),
	/** Set to indicate that the data returned from an I/O should not be saved in any disk caches. */
	kIOF_DONTFILLDISKCACHE = (1<<3),
	
	/** Combined flags, used to indicate that the data returned from an I/O should not be saved in any caches. This
		is the same as specifying both #kIOF_DONTFILLRAMCACHE and #kIOF_DONTFILLDISKCACHE. */
	kIOF_DONTFILLCACHE = (kIOF_DONTFILLRAMCACHE | kIOF_DONTFILLDISKCACHE),
	/** Combined flags, used to indicate that no caching should be performed on this I/O at all. This is the
		same as specifying #kIOF_DONTUSECACHE, #kIOF_DONTFILLRAMCACHE, and #kIOF_DONTFILLDISKCACHE. */
	kIOF_NOCACHE = (kIOF_DONTUSECACHE | kIOF_DONTFILLRAMCACHE | kIOF_DONTFILLDISKCACHE),
};

/**
	\brief Callback used by media drivers.
	This callback should block until the event is completely handled.
	\param[in] pContext    User-specified context pointer
	\param[in] media       Media object that generated this event
	\param[in] event       Event type
	\param[in,out] pParam  Event parameter (may be NULL)
	\return            Return #kNOERR if the event was handled.
	\see	e_MEDIAEVENTS
*/
typedef err_t (*mediacallback_proc)(
	void *pContext,
	media *pMedia,
	mediaevent_t event,
	void *pParam);

/**
	\brief	Load statistic types

	These values are used as input to scheduler::getStatistics() to query the status
	of the scheduler for runtime performance monitoring. 

	\see	stattype_t, scheduler::getStatistics()
*/
enum e_FIOS_STATTYPE {
	/** \brief Number of streams 
		
		This statistic is a value which indicates the number of streams open at
		a time.  The current value is the number of streams open right now, and the
		lifetime value is the maximum number of streams ever opened
		simultaneously.
	*/
	kSTAT_NUMSTREAMS = 1,
	
	/** \brief Number of operations
		
		This statistic is a value which indicates the number of outstanding
		non-stream operations at a time.  The current value is the number of outstanding
		operations in the queue right now, and the lifetime value is the maximum number
		of operations that have ever been in flight.
	*/
	kSTAT_NUMOPS = 2,
	
	/**	\brief Number of open files

		This statistic is a value which indicates the number of open files at a time.
		The current value is the number of open files right now, and the lifetime
		value is the maximum number of files that that have ever been open.
	*/
	kSTAT_NUMFILES = 3,
	
	/** \brief Percentage load

		This statistic is a value from 0 to 100 which indicates what percentage of
		the scheduler's time over the stated period has been spent servicing I/O
		requests.  For example, if the scheduler has been running for 1000 seconds
		and has spent 600 seconds servicing I/O requests, the value returned for
		its lifetime utilization will be 60 (representing 60% load).

		The current value represents the value for the past two seconds, and the
		lifetime value represents the value for the scheduler's lifetime.
	*/
	kSTAT_TOTALLOAD = 4,
	
	/** \brief Percentage load during busy times (when I/O requests are outstanding)

		This statistic is a value from 0 to 100 which indicates what percentage of the
		scheduler's time over its lifetime has been spent servicing I/O requests while
		there were any requests outstanding.
		
		This is primarily useful in judging stream capacity. If only non-stream I/O
		is being used, this value will be 100. If only stream I/O or a mixture of
		stream and non-stream I/O is being used, this value will give an approximation
		of the scheduler's capacity to service additional streams.

		The current value represents the value for the past two seconds, and the
		lifetime value represents the value for the scheduler's lifetime.
	*/
	kSTAT_BUSYLOAD = 5,
	
	/** \brief Estimated percentage of the scheduler's busy time spent seeking.
		
		This statistic is a value from 0 to 100 which indicates what percentage of the
		scheduler's busy time (as returned by #kSTAT_BUSYLOAD) has been spent waiting for
		seeks, layer changes, head alignments, and any other operation that is tied
		to seeking rather than data access.

		This can be used to evaluate the performance of a disk layout.  A perfectly
		optimal disk layout will have a seek load of 0 because all files will be laid
		out in the exact order they are used and no seeks will ever be performed.
		Higher values indicate less optimal layouts.  In practical terms, of course,
		seek load is unavoidable, but minimizing this statistic will ensure an
		optimal layout.

		The sum of the statistics #kSTAT_IOLOAD + #kSTAT_SEEKLOAD should be 100.

		The current value represents the value for the past two seconds, and the
		lifetime value represents the value for the scheduler's lifetime.
	*/
	kSTAT_SEEKLOAD = 6,
	
	/** \brief Estimated percentage of the scheduler's busy time spent reading or writing data.

		This statistic is a value from 0 to 100 which indicates what percentage of the
		scheduler's busy time (as returned by #kSTAT_BUSYLOAD) has been spent on data access
		rather than seeking.

		This can be used to evaluate the performance of a disk layout. A perfectly
		optimal disk layout will have an I/O load of 100 because all files will be
		laid out in the exact order they are used and only data access operations
		will be performed -- never any seeks.  Lower values indicate less optimal
		layouts. In practical terms, of course, an I/O load of 100 is unachievable,
		but maximizing this statistic will ensure an optimal layout.

		The sum of the statistics #kSTAT_IOLOAD + #kSTAT_SEEKLOAD should be 100.

		The current value represents the value for the past two seconds, and the
		lifetime value represents the value for the scheduler's lifetime.
	*/
	kSTAT_IOLOAD = 7,
	
	/** \brief Percentage of non-stream requests that missed their deadlines.

		This statistic is a value from 0 to 100 which indicates what percentage of
		non-stream I/O requests were not completed until past their deadline.
		
		Requests issued with #kDEADLINE_NOW and #kDEADLINE_ASAP will
		essentially always "miss" their deadlines, so they are not counted as part
		of this statistic. The inverse is true for requests issued with
		#kDEADLINE_LATER; these will always meet their deadline, so they are
		not counted either.  Only requests where a valid deadline is specified are
		considered.
		
		This can be used to evaluate the effectiveness of a client's preloading
		strategy. Ideally your miss rate should be 0, and this value is normally
		achievable. A non-zero miss rate may be the source of a performance problem.
		To reduce your miss rate, try one or more of the following:
		
		 - schedule your non-stream requests further in advance
		 - decrease the bitrate of open streams
		 - decrease the number of open streams
		 - issue fewer non-stream requests

		The current value represents the value for the past two seconds, and the
		lifetime value represents the value for the scheduler's lifetime.
	*/
	kSTAT_MISSRATE = 8
};

/**
	\brief Flags used in scheduler::openFile()
	\see scheduler::openFile, scheduler::openFileSync
*/
enum e_OPENFLAGS { /* open flags */
	kO_READ = (1<<0),              /**< Open with read privileges */
	kO_RDONLY = kO_READ,           /**< Synonym for #kO_READ, used to signify read-only */
	kO_WRITE = (1<<1),             /**< Open with write privileges */
	kO_WRONLY = kO_WRITE,          /**< Synonym for #kO_WRITE, used to signify write-only */
	kO_RDWR = kO_READ|kO_WRITE,    /**< Synonym for #kO_READ and #kO_WRITE together, used to signify read-write */
	kO_APPEND = (1<<2),            /**< Write-append-only */
	kO_CREAT = (1<<3),             /**< Create file if it doesn't exist */
	kO_TRUNC = (1<<4),             /**< Truncate file if it already exists */
	
	kO_DYNALLOC = (1<<7)           /**< Filehandle is or should be dynamically allocated. */
};

/**
	\brief  Whence values.
	\see    scheduler::seekFileSync, filehandle::seek
*/
enum e_WHENCE { /* whence */
	kSEEK_SET = 0,     /**< Absolute seek location from the beginning of file. */
	kSEEK_CUR = 1,     /**< Relative seek location, based on current position. */
	kSEEK_END = 2      /**< Relative seek location, from the end of file. */
};

/**
	\brief  Stat flags.
	\see    #kMEDIAACTION_STAT
*/
enum e_STATFLAGS { /* stat flags */
	kSTAT_DIRECTORY = (1<<0),       /**< item is a directory */
	kSTAT_READABLE = (1<<1),        /**< item is readable */
	kSTAT_WRITABLE = (1<<2)         /**< item is writable */
};


/** \brief  File or directory status
	\see    scheduler::stat */
typedef struct stat_t {
	off_t         fileLocation;       /**< File location as a byte address from the start of the media. May be 0 if unavailable. */
	off_t         fileSize;           /**< File size in bytes. Zero for directories. */
	datetime_t    accessDate;         /**< Last time the file was read/accessed. May be 0 if not supported by the OS. */
	datetime_t    modificationDate;   /**< Last time the file was modified. May be 0 if not supported by the OS. */
	datetime_t    creationDate;       /**< Time the file was created. May be 0 if not supported by the OS. */
	U32           statFlags;          /**< FIOS stat flags; see #e_STATFLAGS. */
	I64           nativeUID;          /**< Native POSIX UID. May be 0 if not supported by the OS. */
	I64           nativeGID;          /**< Native POSIX GID. May be 0 if not supported by the OS. */
	I64           nativeDevice;       /**< Native POSIX device number. May be 0 if not supported by the OS. */
	I64           nativeInode;        /**< Native POSIX inode number. May be 0 if not supported by the OS. */
	I64           nativeMode;         /**< Native POSIX mode. May be 0 if not supported by the OS. */
} stat_t;

/**
	\brief  Used to return directory entries.
	\see    #kMEDIAACTION_GETDIRENTRIES
*/
typedef struct direntry_t {
	off_t    fileLocation;            /**< File location as a byte address from the start of the media. Set to 0 if unavailable. */
	off_t    fileSize;                /**< File size in bytes. Set to 0 for directories. */
	U32      statFlags;               /**< Stat flags; see #e_STATFLAGS */
	U16      nameLength;              /**< Name length, for convenience. */
	U16      fullPathLength;          /**< Full path length, for convenience. */
	U16      offsetToName;            /**< Offset from start of full path to start of filename. */
	char     fullPath[FIOS_PATH_MAX]; /**< Full pathname. */
} direntry_t;


/**
	\brief Media file descriptor
	Each media object may have its own notion of a file descriptor. For example, the
	dearchiving layer may need a special fake file descriptor for items within an
	archive that references the containing archive. So this class encapsulates
	a media-layer-specific filehandle.
*/
class FIOS_EXPORT mediafd
{
public:
	inline mediafd() { reset(); }
	inline mediafd(const mediafd &other) { m_issuer = other.m_issuer; m_ptr = other.m_ptr; m_ptr2 = other.m_ptr2; m_int = other.m_int; m_nativeFD = other.m_nativeFD; }
	inline ~mediafd() {}
	
	inline mediafd & operator = (const mediafd &other) { m_issuer = other.m_issuer; m_ptr = other.m_ptr; m_ptr2 = other.m_ptr2; m_int = other.m_int; m_nativeFD = other.m_nativeFD; return *this; }
	inline bool operator == (const mediafd &other) const { return (m_issuer == other.m_issuer) && (m_ptr == other.m_ptr) && (m_ptr2 == other.m_ptr2) && (m_int == other.m_int) && (m_nativeFD == other.m_nativeFD); }
	inline bool operator != (const mediafd &other) const { return !operator ==(other); }
	
	inline mediafd & reset(media *pOwner = NULL) { m_issuer = pOwner; m_ptr = NULL; m_ptr2 = 0; m_int = 0; m_nativeFD = platform::kINVALID_FILEHANDLE; return *this; }

	media *              m_issuer;    //!< Media object that issued this filehandle
	void *               m_ptr;       //!< Storage for the media object's use
	void *               m_ptr2;      //!< Storage for the media object's use
	off_t                m_int;       //!< Storage for the media object's use
	platform::nativefd_t m_nativeFD;  //!< Storage for the media object's use
	
	// The invalid filehandle.
	static const mediafd kINVALID_FILEHANDLE;
};



/**
	\brief	Stream events
	\see streamcallback_proc
*/
enum e_STREAMEVENTS {
	/**
		\brief Stream was opened.

		The parameter is unspecified and currently NULL. Your callback can use this
		as an opportunity to allocate any resources it needs. 
		If you return a value other than #kNOERR the stream will not be opened
		and the error will be propagated to the client who opened the stream.
	*/
	kSTREAMEVENT_INIT = 0,
	
	/**
		\brief Stream has finished loading all of its data.
		
		The parameter is unspecified and currently NULL. This event indicates that
		there is no further I/O left on the stream, although the consumer may not
		have caught up yet.
		
		Your callback may choose to add an extent to the stream (which will restart the
		I/O), or close and possibly delete the stream.  It's safe to close unmanaged
		streams from this callback, but managed streams will need to leave the stream
		open until the consumer has consumed all of the remaining data.
	*/
	kSTREAMEVENT_IOCOMPLETE = 1,
	
	/**
		\brief Stream has provided all of its data to the consumer. (managed streams only)

		The parameter is unspecified and currently NULL. This event indicates that
		there is no further data left in the stream's buffers: it has all been consumed
		by the client.  It's safe to close or delete managed streams from this callback.
	*/
	kSTREAMEVENT_DATACOMPLETE = 2,

	/**
		\brief Stream is being closed.
		
		The parameter is unspecified and currently NULL. Your callback can use this
		event as an opportunity to clean up any allocated resources. The return value
		is ignored and returning an error will not prevent the stream from being closed.
	*/
	kSTREAMEVENT_CLOSE = 3,
	
	/**
		\brief Status query. (unmanaged streams only)

		The parameter is a pointer to a struct #stream_status.  
		
		If you use a ring buffer or equivalent data storage, you should fill in the
		requested information and return #kNOERR so that the scheduler knows how
		close your buffers are to running dry. If you do not use a ring buffer then
		return any other error.
		
		If your callback returns any value other than #kNOERR, you may encounter
		a loss of streaming. The scheduler will attempt to maintain the maximum data
		rate by using your last status report and dead reckoning from the stream's
		max data rate.
	*/
	kSTREAMEVENT_STATUS = 4,
	
	/**
		\brief Get buffer. (unmanaged streams only)

		The parameter is a pointer to a struct #stream_buffer. Your callback should
		prepare and return a buffer for the next I/O. The scheduler will pass in the number
		of bytes it would like to read in a single request, but if this value is too large --
		for example, if the end of the ring buffer is reached -- then the callback should
		modify it appropriately. 

		If no buffer space is available, you should return kERR_NOMEM.
	*/
	kSTREAMEVENT_GETBUFFER = 5,

	/**
		\brief Return buffer. (unmanaged streams only)

		The parameter is a pointer to a struct #stream_buffer. Your callback should
		do whatever it needs to do to consume the bytes. The callback should queue the
		bytes or pass them off to another thread; it should not block during the callback.
		If it blocks the callback thread, the entire scheduler will come to a halt until
		it returns!		
	*/
	kSTREAMEVENT_DATAREADY = 6
};

/** Parameter for #kSTREAMEVENT_STATUS */
typedef struct stream_status {
	U32	bufferSizeTotal;            /**< [out] total size of ring buffer */
	U32	bufferSizeFree;             /**< [out] free bytes in ring buffer */
	off_t	nextReadLocation;       /**< [in,out] next read location in stream */
} stream_status;

/** Parameter for #kSTREAMEVENT_GETBUFFER and #kSTREAMEVENT_DATAREADY */
typedef struct stream_buffer {
	U32	totalSize;            /**< [in,out] requested/actual size of buffer */
	void *pBuffer;            /**< [in,out] buffer */
	off_t readLocation;       /**< [in] read location in stream */
} stream_buffer;

/**
	\brief Stream flags
	\see e_STREAMFLAGS
*/
typedef U32 streamflags_t;

/**
	\brief Stream flag definitions
*/
enum e_STREAMFLAGS
{
	/** Set to indicate that this stream is optional, and should be canceled if it cannot fully buffer before the deadline hits. */
	kSTREAMF_OPTIONAL = (1<<0),
	/** Set to indicate that this stream's I/O should not use any caches but always read or write directly to the media. */
	kSTREAMF_DONTUSECACHE = (1<<1),
	/** Set to indicate that the data returned from this stream's I/O should not be saved in any caches. */
	kSTREAMF_DONTFILLCACHE = (1<<2),
	/** Set to indicate that this stream will never loop, so extent descriptors should be recycled after they are no longer needed. This is most useful for a stream that is being heavily dynamically modified. */
	kSTREAMF_RECYCLEEXTENTS = (1<<3),
	/** Set to indicate that this stream uses unmanaged buffers. If you set this flag, your callback is responsible for providing data buffers to FIOS on demand. */
	kSTREAMF_UNMANAGED = (1<<4),
	/** Set to indicate that this stream is or should be dynamically allocated, rather than allocated from the free pool. */
	kSTREAMF_DYNALLOC = (1<<7),
	/** \internal
		\brief Set to indicate that this stream is closing and no more I/O will be scheduled. */
	kSTREAMF_CLOSING = (1<<8)
};

/**
	\brief Extent flags
	\see e_EXTENTFLAGS
*/
typedef U16 extentflags_t;

/**
	\brief Extent flag definitions
*/
enum e_EXTENTFLAGS
{
	/** Set to indicate that this is a block extent, not a file extent */
	kEXTENTF_BLOCK = (1<<0),
};

/**
	\brief Stream extent

	Describes one extent of a stream. Streams may span multiple files, multiple
	ranges within a file, or multiple ranges of raw blocks.
*/
typedef struct streamextent_t
{
	off_t byteOffset;              /**< byte offset from the start of the extent container. A value of zero means to start at the beginning of the file. */
	off_t byteCount;               /**< length of this extent in bytes. Must be greater than zero, and cannot cause the extent to extend beyond the end of the file or media. */
	union {
		off_t blockNumber;         /**< (block extents) block number for this extent */
		const char *pFilename;     /**< (file extents) filename for this extent */
	} container;                   /**< container for this extent */
	I16 loopCount;                 /**< how many times to repeat this extent, or -1 to loop forever. A value of 0 means not to use this extent at all. */
	extentflags_t flags;           /**< flags for this extent */
	mediafd mediaFD;               /**< Not for client use. Used by the scheduler to hold an open filehandle. */
} streamextent_t;

/**
	\brief Callback used by streams.
	This callback should be quick and not run for any significant length of time
	(either processing data or waiting for another operation). Doing so may delay
	other I/O requests.
	\param[in] pContext    User-specified context pointer
	\param[in] pStream     Stream that generated this event
	\param[in] event       Event type
	\param[in,out] pParam  Event parameter (may be NULL)
	\return            Return #kNOERR if the event was handled.
	\see	e_STREAMEVENTS
*/
typedef err_t (*streamcallback_proc)(
	void *pContext,
	stream *pStream,
	streamevent_t event,
	void *pParam);

/**
	\brief Stream attributes

	This structure describes a stream to be opened. All important aspects of the
	stream are described here, from flags describing its behavior to the components
	of the stream. 
*/
typedef struct streamattr_t
{
	const char *        pName;            /**< Name of this stream for debugging. May be NULL. */
	abstime_t           deadline;         /**< Deadline by which the stream should have its initial chunk ready */
	prio_t              priority;         /**< This stream's priority relative to other streams */
	streamflags_t       flags;            /**< Stream flags */
	U64                 maxDataRate;      /**< Maximum data rate, in bytes per second. */
	size_t              pageSize;         /**< For managed streams: page size for retrieving buffers. If you're unsure or it doesn't really matter, set to stream::getDefaultPageSize() which will return an optimal size for the platform. */
	U32                 minPages;         /**< For managed streams: minimum number of pages that you need to buffer at any given time. This is normally the number of pages you need to stop or pause the stream cleanly. Try to keep this at the actual minimum, because FIOS will automatically buffer your stream further if more memory is available. */
	streamcallback_proc pCallback;        /**< Stream callback */
	void *              pCallbackContext; /**< Stream callback context */
	I32                 loopCount;        /**< How many times to loop this stream, or -1 for forever. */
	U16                 extentCount;      /**< Number of extents in this stream */
	streamextent_t      extents[1];       /**< Initial extents for the stream. */
} streamattr_t;

/**
	\brief	Operation events
	\see opcallback_proc
*/
enum e_OPEVENTS { /* operation events */

	/**
		\brief   Operation completed.

		This operation has completed, either successfully or unsuccessfully. Your
		callback should retrieve the error code with op::wait() and react appropriately.
		You should not block for a significant length of time in this callback; doing so
		may delay other I/O requests!

		It's safe to call most other FIOS APIs from this callback, including async I/O APIs
		that create a new operation, or even deleting the operation that you were called about.
		Do not call sync I/O APIs or otherwise wait for any operation other than the one you
		were called about; this will cause a deadlock.
		
		Typical use of this callback is to do something to receive the data (copy it,
		signal a semaphore, wake a thread, etc) and then delete the operation. 
		
		\note
		Deleting the operation from this callback will cause your callback to be re-entered
		with a #kOPEVENT_DELETE message, which you should be prepared to handle.
		
		Your callback should return #kNOERR. Any other return value will be ignored.
	*/
	kOPEVENT_COMPLETED = 1,

	/**
		\brief   Operation is being deleted.

		This operation is being deleted. Your callback can use this as an opportunity to
		clean up any memory or resources that might have been allocated for the request.
		This callback is executed directly from the thread that deleted the operation.

		Your callback should return #kNOERR. Any other return value will be ignored.
	*/
	kOPEVENT_DELETE = 2
};

/**
	\brief Callback used by operations.
	This callback should be quick and not run for any significant length of time
	(either processing data or waiting for another operation). Doing so may delay
	other I/O requests.
	
	You may delete the incoming operation as part of handling a completion event,
	but be aware that this will cause your callback to be re-entered with a
	deletion event.
	\param[in] pContext    User-specified context pointer
	\param[in] pOp         Operation that generated this event
	\param[in] event       Event type
	\param[in,out] pParam  Event parameter (may be NULL)
	\return            Return #kNOERR if the event was handled.
	\see    e_OPEVENTS
*/
typedef err_t (*opcallback_proc)(
	void *pContext,
	op *pOp,
	opevent_t event,
	void *pParam);

/**
	\brief Operation flags
*/
enum e_OPFLAGS
{
	/** Set to indicate that an I/O is optional, and no good if it comes in late. If it cannot
		complete before the deadline it will be cancelled. */
	kOPF_OPTIONAL = (1<<0),
	/** Set to indicate that an I/O should not use any caches but always read or write directly to the media. */
	kOPF_DONTUSECACHE = (1<<1),
	/** Set to indicate that the data returned from an I/O should not be saved in any RAM caches. */
	kOPF_DONTFILLRAMCACHE = (1<<2),
	/** Set to indicate that the data returned from an I/O should not be saved in any disk caches. */
	kOPF_DONTFILLDISKCACHE = (1<<3),
	
	/** Combined flags, used to indicate that the data returned from an I/O should not be saved in any caches. This
		is the same as specifying both #kOPF_DONTFILLRAMCACHE and #kOPF_DONTFILLDISKCACHE. */
	kOPF_DONTFILLCACHE = (kOPF_DONTFILLRAMCACHE | kOPF_DONTFILLDISKCACHE),
	/** Combined flags, used to indicate that no caching should be performed on this I/O at all. This is the
		same as specifying #kOPF_DONTUSECACHE, #kOPF_DONTFILLRAMCACHE, and #kOPF_DONTFILLDISKCACHE. */
	kOPF_NOCACHE = (kOPF_DONTUSECACHE | kOPF_DONTFILLRAMCACHE | kOPF_DONTFILLDISKCACHE),
	
	/** \internal
		\brief Internal flag used to indicate that this operation is being deleted. */
	kOPF_DELETED = (1<<8),
	/** \internal
		\brief Internal flag used to indicate that this operation is being cancelled. */
	kOPF_CANCELLED = (1<<9),
	/** \internal
	    \brief Internal flag used to indicate that this operation is ready to have its
		callback invoked. */
	kOPF_CALLBACK = (1<<10),
	/** \internal
		\brief Set to indicate that this operation was dynamically allocated. */
	kOPF_DYNALLOC = (1<<11)
};

/**
	\brief Context struct used to simplify creating an operation.

	If you are using tags to identify your I/O, remember that not all I/O
	in the queue belongs to you! Do not assume that userPtr is
	a valid pointer unless the userTag is one of your unique tags.
*/
typedef struct opattr_t
{
	abstime_t        deadline;           /**< Deadline for the operation.
	                                          \see FIOSTime */
	prio_t           priority;           /**< Priority for the operation. When I/O cannot be satisfied by the deadline, operations with higher numerical priorities are given precedence.
	                                          \see prio_t, e_PRIORITIES */
	opcallback_proc  pCallback;          /**< Callback for the operation (may be NULL). If you are making a sync call, or if you are going to call some variant of op::wait(), you generally don't need to set a callback. */
	void *           pCallbackContext;   /**< Callback context pointer (may be NULL). */
	U32              opflags;            /**< Operation flags to control caching behavior and other details.
										      \see e_OPFLAGS */
	U32              userTag;            /**< Tag for your use. Can be used to identify your operations with scheduler::iterateOps(). */
	void *           userPtr;            /**< Pointer for your use. Any additional data you want to associate with the operation can be put here. */
} opattr_t;

/*@}*/

}; /* namespace fios */

#endif /* _H_fios_types */
