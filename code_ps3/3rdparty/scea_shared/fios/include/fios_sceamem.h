/**
	\file fios_sceamem.h

	Definitions of SCEA::Memory purposes used by FIOS.
	
    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_sceamem
#define _H_fios_sceamem

#include "sceacommon/include/sceamem.h"

/**
	\internal
	@{
*/

namespace SCEA {
	namespace Memory {

//! In the SceaMem API, FIOS has a bank of 256 memory purposes starting at kMemPurposeFIOSFirst.
enum e_MEMPURPOSES
{
	kMemPurposeFIOSMediaObject                       = kMemPurposeFIOSFirst+(0<<kMemPurposeShift),   //!< A media object. Failure causes media initialization to fail.
	kMemPurposeFIOSMediaIdentifierList               = kMemPurposeFIOSFirst+(1<<kMemPurposeShift),   //!< Array containing media identifiers. Failure causes media initialization to fail.
	kMemPurposeFIOSMediaIdentifier                   = kMemPurposeFIOSFirst+(2<<kMemPurposeShift),   //!< Individual media identifiers. Failure causes media initialization to fail.
	kMemPurposeFIOSMediaIOThreadStack                = kMemPurposeFIOSFirst+(3<<kMemPurposeShift),   //!< I/O thread stack, tunable via FIOS_IOTHREAD_STACKSIZE. Failure causes media initialization to fail.
	kMemPurposeFIOSMediaComputeThreadStack           = kMemPurposeFIOSFirst+(4<<kMemPurposeShift),   //!< Compute thread stack, tunable via FIOS_COMPUTETHREAD_STACKSIZE. Failure causes media initialization to fail.
	kMemPurposeFIOSCatalogCacheHashtable             = kMemPurposeFIOSFirst+(5<<kMemPurposeShift),   //!< Hashtable for the catalogcache object. Failure disables the catalog cache.
	kMemPurposeFIOSCatalogCachePath                  = kMemPurposeFIOSFirst+(6<<kMemPurposeShift),   //!< Paths stored by the catalogcache object. Failure disables the catalog cache.
	kMemPurposeFIOSCatalogCacheEntry                 = kMemPurposeFIOSFirst+(7<<kMemPurposeShift),   //!< Entries stored by the catalogcache object. Failure disables the catalog cache.
	kMemPurposeFIOSCatalogCacheTreeIteratorBuffer    = kMemPurposeFIOSFirst+(8<<kMemPurposeShift),   //!< Temp buffers used by the built-in tree iterator when rebuilding. Failure disables the catalog cache.
	kMemPurposeFIOSDearchiverOpenArchiveArray        = kMemPurposeFIOSFirst+(9<<kMemPurposeShift),   //!< Array to store open archive file mediarequest pointers
	kMemPurposeFIOSDearchiverPsarcArchiveArray       = kMemPurposeFIOSFirst+(10<<kMemPurposeShift),  //!< Array to store open archive file mediarequest pointers
	kMemPurposeFIOSCacheIndex                        = kMemPurposeFIOSFirst+(11<<kMemPurposeShift),  //!< In-RAM copy of the cache index. Failure disables the RAM cache.
	kMemPurposeFIOSRAMCache                          = kMemPurposeFIOSFirst+(12<<kMemPurposeShift),  //!< Storage for RAM cache blocks. Failure disables the RAM cache.
	kMemPurposeFIOSDiskCacheIOBuffers                = kMemPurposeFIOSFirst+(13<<kMemPurposeShift),  //!< I/O buffers for the disk cache. Failure disables the disk cache.
	
	kMemPurposeFIOSSchedulerObject                   = kMemPurposeFIOSFirst+(16<<kMemPurposeShift),  //!< A scheduler object. Failure causes scheduler initialization to fail and is passed on to the caller of createSchedulerForMedia().
	kMemPurposeFIOSSchedulerStreamPages              = kMemPurposeFIOSFirst+(17<<kMemPurposeShift),  //!< Buffers for managed stream I/O. Size comes from a parameter to the scheduler's constructor. Failure means no space is used for managed streams.
	kMemPurposeFIOSSchedulerStreamPageTrackers       = kMemPurposeFIOSFirst+(18<<kMemPurposeShift),  //!< Array of structs to track the managed stream pages. Failure means no space is used for managed streams.
	kMemPurposeFIOSSchedulerPreallocatedOps          = kMemPurposeFIOSFirst+(19<<kMemPurposeShift),  //!< Array of preallocated op objects. Size comes from a parameter to the scheduler's constructor * sizeof(op). Failure means no ops will be preallocated.
	kMemPurposeFIOSSchedulerPreallocatedFilehandles  = kMemPurposeFIOSFirst+(20<<kMemPurposeShift),  //!< Array of preallocated filehandle objects. Size comes from a parameter to the scheduler's constructor * sizeof(filehandle). Failure means no filehandles will be preallocated.
	kMemPurposeFIOSSchedulerPreallocatedStreams      = kMemPurposeFIOSFirst+(21<<kMemPurposeShift),  //!< Array of preallocated stream objects. Size comes from a parameter to the scheduler's constructor * sizeof(stream). Failure means no streams will be preallocated.
	kMemPurposeFIOSSchedulerMediaIOParams            = kMemPurposeFIOSFirst+(22<<kMemPurposeShift),  //!< Array of I/O control structures and pcecoefficients to allow the scheduler to track N simultaneous I/O requests to the media. Failure causes scheduler initialization to fail and is passed on to the caller of createSchedulerForMedia().
	kMemPurposeFIOSSchedulerUnknownEstimates         = kMemPurposeFIOSFirst+(23<<kMemPurposeShift),  //!< Array of abstime_t values for the media's unknowns. Failure causes scheduler initialization to fail and is passed on to the caller of createSchedulerForMedia().
	kMemPurposeFIOSSchedulerMediaState               = kMemPurposeFIOSFirst+(24<<kMemPurposeShift),  //!< Most recent media state.  Failure causes scheduler initialization to fail and is passed on to the caller of createSchedulerForMedia().
	kMemPurposeFIOSSchedulerThreadStack              = kMemPurposeFIOSFirst+(25<<kMemPurposeShift),  //!< Scheduler thread stack, tunable via FIOS_SCHEDULERTHREAD_STACKSIZE. Failure causes scheduler initialization to fail and is passed on to the caller of createSchedulerForMedia().
	
	kMemPurposeFIOSStreamObject                      = kMemPurposeFIOSFirst+(32<<kMemPurposeShift),  //!< Dynamically allocated stream object. Only used when kSTREAMF_DYNALLOC is set, or when open streams >= preallocated streams. Failure is passed on to caller of openStreamSync().
	kMemPurposeFIOSStreamExtentList                  = kMemPurposeFIOSFirst+(33<<kMemPurposeShift),  //!< Dynamic array of stream extents. Usually only allocated once, but may be reallocated if extents are being added to the stream. Failure is passed on to caller of addExtents() or openStreamSync().
	kMemPurposeFIOSStreamExtentPath                  = kMemPurposeFIOSFirst+(34<<kMemPurposeShift),  //!< Stream extent path, kept for the lifetime of the extent. The lifetime of the extent is the lifetime of the stream unless kSTREAMF_RECYCLEEXTENTS is set. This allocation may go away in the future. Failure is passed on to caller of addExtents() or openStreamSync().
	kMemPurposeFIOSStreamSeekList                    = kMemPurposeFIOSFirst+(35<<kMemPurposeShift),  //!< Stream seeks, kept until the seek has been fulfilled. Failure is passed on to the caller of stream::seek.
	
	kMemPurposeFIOSFilehandleObject                  = kMemPurposeFIOSFirst+(48<<kMemPurposeShift),  //!< Dynamically allocated filehandle object. Only used when kO_DYNALLOC is set, or when open filehandles >= preallocated filehandles. Failure is passed on to caller of openFile().
	kMemPurposeFIOSFilehandlePath                    = kMemPurposeFIOSFirst+(49<<kMemPurposeShift),  //!< Filehandle path, kept for the lifetime of the filehandle. This allocation may go away in the future. Failure is passed on to caller of openFile().
	kMemPurposeFIOSOpPath                            = kMemPurposeFIOSFirst+(50<<kMemPurposeShift),  //!< Op path, kept for the lifetime of the op. This allocation may go away in the future. Failure is passed on to caller (op fails with kERR_NOMEM).
	
	kMemPurposeFIOSOpObject                          = kMemPurposeFIOSFirst+(64<<kMemPurposeShift),  //!< Dynamically allocated op. Only used when allocated ops >= preallocated ops. Failure is passed on to client who made the request.
	kMemPurposeFIOSThreadObject                      = kMemPurposeFIOSFirst+(65<<kMemPurposeShift),  //!< Thread object.
	kMemPurposeFIOSMutexObject                       = kMemPurposeFIOSFirst+(66<<kMemPurposeShift),  //!< Mutex object.
	kMemPurposeFIOSCondObject                        = kMemPurposeFIOSFirst+(67<<kMemPurposeShift),  //!< Cond object.
	kMemPurposeFIOSRWLockObject                      = kMemPurposeFIOSFirst+(68<<kMemPurposeShift),  //!< RWLock object.

	kMemPurposeFIOSPathStorage                       = kMemPurposeFIOSFirst+(80<<kMemPurposeShift),  //!< Path storage object, allocated once and kept globally.
	kMemPurposeFIOSPathStorageHash                   = kMemPurposeFIOSFirst+(81<<kMemPurposeShift),  //!< Path storage hashtable. May be reallocated to grow.
	kMemPurposeFIOSPathStorageArray                  = kMemPurposeFIOSFirst+(82<<kMemPurposeShift),  //!< Path storage array. May be reallocated to grow.
	kMemPurposeFIOSPathStoragePath                   = kMemPurposeFIOSFirst+(83<<kMemPurposeShift),  //!< Path storage path, allocated once and kept forever.
};

	}; /* namespace Memory */
}; /* namespace SCEA */

/*@}*/

#endif	//	_H_fios_sceamem
