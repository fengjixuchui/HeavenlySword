/**
	\file fios_platform_imp_ps3_ppu.h

	PS3 PPU implementation of some platform-specific functions found in FIOS.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_platform_imp_ps3_ppu
#define _H_fios_platform_imp_ps3_ppu
#include "sceacommon/include/sceatargetmacros.h"
#if SCEA_TARGET_OS_PS3_PPU || SCEA_TARGET_OS_PS3_PU

#include <sys/system_types.h>
#include <sys/time_util.h>
#include <sys/sys_time.h>
#include <pthread.h>
#include <time.h>
#include <ppu_intrinsics.h>   // __isync() and __lwsync()
#include "alloca.h"
#include "libsceaatomic/include/sceaatomic.h"


namespace fios {
	namespace platform {

/** \internal
@{*/

		/** \def FIOS_SCHEDULERTHREAD_STACKSIZE
			Size of the scheduler thread's stack. */
		#ifndef FIOS_SCHEDULERTHREAD_STACKSIZE
		#define FIOS_SCHEDULERTHREAD_STACKSIZE	(64*1024)
		#endif
		
		/** \def FIOS_SCHEDULERTHREAD_PRIORITY
		    Priority of the scheduler thread. For PS3 at the time of writing this appears
			basically arbitrary; 0 to 3071 with 0 being the highest. I chose 100. */
		#ifndef FIOS_SCHEDULERTHREAD_PRIORITY
		#define FIOS_SCHEDULERTHREAD_PRIORITY   (100)
		#endif

		/** \def FIOS_IOTHREAD_STACKSIZE
		    Size of any media I/O thread's stack. */
		#ifndef FIOS_IOTHREAD_STACKSIZE
        #define FIOS_IOTHREAD_STACKSIZE         (32*1024)
        #endif
        
		/** \def FIOS_IOTHREAD_PRIORITY
		    Priority of any media I/O thread. For PS3 at the time of writing this appears
			basically arbitrary; 0 to 3071 with 0 being the highest. I chose 99. */
		#ifndef FIOS_IOTHREAD_PRIORITY
		#define FIOS_IOTHREAD_PRIORITY          (99)
		#endif

		/** \def FIOS_COMPUTETHREAD_STACKSIZE
			Size of any media compute thread's stack. */
		#ifndef FIOS_COMPUTETHREAD_STACKSIZE
		#define FIOS_COMPUTETHREAD_STACKSIZE    (64*1024)
		#endif
		
		/** \def FIOS_COMPUTETHREAD_PRIORITY
		    Priority of any media compute thread. For PS3 at the time of writing this appears
			basically arbitrary; 0 to 3071 with 0 being the highest. I chose 257. */
		#ifndef FIOS_COMPUTETHREAD_PRIORITY
		#define FIOS_COMPUTETHREAD_PRIORITY     (257)
		#endif

		/** \def FIOS_NORMALTHREAD_STACKSIZE
			Size of any normal thread's stack. */
		#ifndef FIOS_NORMALTHREAD_STACKSIZE
		#define FIOS_NORMALTHREAD_STACKSIZE    (32*1024)
		#endif
		
		/** \def FIOS_NORMALTHREAD_PRIORITY
		    Priority of any normal thread. For PS3 at the time of writing this appears
			basically arbitrary; 0 to 3071 with 0 being the highest. I chose 1000. */
		#ifndef FIOS_NORMALTHREAD_PRIORITY
		#define FIOS_NORMALTHREAD_PRIORITY          (1000)
		#endif

		/** \def FIOS_LOWPRIORITYTHREAD_STACKSIZE
			Size of any low-priority thread's stack. */
		#ifndef FIOS_LOWPRIORITYTHREAD_STACKSIZE
		#define FIOS_LOWPRIORITYTHREAD_STACKSIZE    (32*1024)
		#endif
		
		/** \def FIOS_LOWPRIORITYTHREAD_PRIORITY
		    Priority of any low-priority thread. For PS3 at the time of writing this appears
			basically arbitrary; 0 to 3071 with 0 being the highest. I chose 1500. */
		#ifndef FIOS_LOWPRIORITYTHREAD_PRIORITY
		#define FIOS_LOWPRIORITYTHREAD_PRIORITY     (1500)
		#endif
		
		/** \def FIOS_PATH_MAX
			Maximum length of a media-relative path. */
		#ifndef FIOS_PATH_MAX
		# define FIOS_PATH_MAX                 (256)
		#endif
		
		/** \brief Abstime is platform-specific.
			For PS3, we use the PowerPC timebase as our abstime. */
		typedef I64 abstime_t;
		
		/** \brief Native date is platform-specific.
			For PS3, we use struct tm. */
		typedef struct tm nativedate_t;
		
		/** \brief Native thread type */
		typedef sys_ppu_thread_t nativethread_t;

		/** \brief Native mutex type */
		typedef sys_mutex_t nativemutex_t;

		/** \brief Native cond type */
		typedef sys_cond_t nativecond_t;

		/** \brief Native rwlock type */
		typedef sys_rwlock_t nativerwlock_t;

		/** \def FIOS_HAS_NATIVERWLOCK
			Defined to 1, because the PS3 has a native rwlock type. */
		#ifndef FIOS_HAS_NATIVERWLOCK
		#define FIOS_HAS_NATIVERWLOCK     1
		#endif

		/** \brief Native filehandle type
			PS3 uses an int, but because they stupidly force us to close and reopen files when the BD is ejected,
			we use a pointer to a tracking struct as our native FD type. */
		typedef struct ps3fd * nativefd_t;
		
		/** \brief Illegal filehandle value, used as a marker */
		const nativefd_t kINVALID_FILEHANDLE = NULL;

		/** \brief Fake filehandle value, used by catalog cache and others. Must be different from #kINVALID_FILEHANDLE and either invalid or unlikely. */
		const nativefd_t kFAKE_FILEHANDLE = reinterpret_cast<nativefd_t>(ptrdiff_t(-1LL));
		
		// --------------------------------------------------------------------
		// Atomic operations
		// --------------------------------------------------------------------
		// CompareAndSwap defined by libsceaatomic in the Threads project.
		//
		inline bool atomicCompareAndSwap(volatile U32 *pPtr, U32 oldValue, U32 newValue) {
			return (SCEA::Atomic::CompareAndSwap(pPtr,oldValue,newValue) == oldValue);
		}
		inline bool atomicCompareAndSwap(volatile U64 *pPtr, U64 oldValue, U64 newValue) {
			return (SCEA::Atomic::CompareAndSwap(pPtr,oldValue,newValue) == oldValue);
		}
		inline bool atomicCompareAndSwap(const void * volatile *pPtr, const void *oldValue, const void *newValue) {
		#if SCEA_TARGET_RT_PTR_SIZE_64
			return (SCEA::Atomic::CompareAndSwap(reinterpret_cast<volatile U64*>(pPtr),
				reinterpret_cast<U64>(oldValue), reinterpret_cast<U64>(newValue)) ==
				reinterpret_cast<U64>(oldValue));
		#elif SCEA_TARGET_RT_PTR_SIZE_32
			return (SCEA::Atomic::CompareAndSwap(reinterpret_cast<volatile U32*>(pPtr),
				reinterpret_cast<U32>(oldValue), reinterpret_cast<U32>(newValue)) ==
				reinterpret_cast<U32>(oldValue));
		#else
		#error "Unknown or unset pointer size!"
		#endif
		}
		inline void atomicStore(volatile U32 *pPtr, U32 value) {
			__lwsync(); // flush pending stores
			*pPtr = value;
		}
		inline void atomicStore(volatile U64 *pPtr, U64 value) {
			__lwsync(); // flush pending stores
			*pPtr = value;
		}
		inline void atomicStore(const void * volatile *pPtr, const void *value) {
			__lwsync(); // flush pending stores
			*pPtr = value;
		}
		inline U32 atomicLoad(const volatile U32 *pPtr) {
			U32 result = *pPtr;
			__isync(); // flush speculative loads
			return result;
		}
		inline U64 atomicLoad(const volatile U64 *pPtr) {
			U64 result = *pPtr;
			__isync(); // flush speculative loads
			return result;
		}
		inline void * atomicLoad(const void * const volatile *pPtr) {
			void * pResult = const_cast<void*>(*pPtr);
			__isync(); // flush speculative loads
			return pResult;
		}

		// --------------------------------------------------------------------
		//	Time
		// --------------------------------------------------------------------
		
		extern const double g_nanosecondsToAbstime;
		
		/* Gets the current native time. */
		inline abstime_t  currentTime()
		{
			register abstime_t now;
			SYS_TIMEBASE_GET(now);
			return now;
		}

		/* Converts from native time to nanoseconds. */
		inline U64 timeToNanoseconds(abstime_t abstime)
		{
			// Converting with FP is way faster than doing the conversion in 128-bit integer
			// math. To get any faster you'd have to hardcode the frequency (either directly,
			// indirectly by making certain assumptions, or via dynamic codegen).
			// We lose precision beyond 64 bits, but normally this function is only used for
			// delta-time values which are far less than 64 bits.
			return (U64)(abstime / g_nanosecondsToAbstime);
		}

		/* Converts from nanoseconds to native time. */
		inline abstime_t  nanosecondsToTime(U64 nanos)
		{
			// Converting with FP is way faster than doing the conversion in 128-bit integer
			// math. To get any faster you'd have to hardcode the frequency (either directly,
			// indirectly by making certain assumptions, or via dynamic codegen).
			// We lose precision beyond 64 bits, but normally this function is only used for
			// delta-time values which are far less than 64 bits.
			return (U64)(nanos * g_nanosecondsToAbstime);
		}


/*@}*/
	}; /* namespace platform */
}; /* namespace fios */

#endif /* SCEA_TARGET_OS_PS3_PPU || SCEA_TARGET_OS_PS3_PU */
#endif /* _H_fios_platform_imp */
