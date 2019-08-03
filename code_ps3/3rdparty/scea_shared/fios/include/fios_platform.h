/**
	\file fios_platform.h

	Definitions of platform-specific functions used elsewhere in FIOS.
	Most of the work involved in porting FIOS to another platform
	should just require implementing the counterparts to these
	functions.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_platform
#define _H_fios_platform

#include "fios_types.h"
#include "fios_base.h"
#include "fios_platform_imp.h"
#include <stdlib.h>

namespace fios {

/** \internal
	@{
*/

	/** Platform-specific code for the FIOS implementation. */
	namespace platform {

// ----------------------------------------------------
// Time
// ----------------------------------------------------

/**	\brief Returns the current time.
	\return Time value */
extern FIOS_EXPORT abstime_t currentTime();

/** \brief Converts from time to nanoseconds. 
	\param[in] abstime Time to convert.
	\return Nanoseconds value */
extern FIOS_EXPORT U64 timeToNanoseconds(abstime_t abstime) FIOS_ATTRIBUTE((__pure__));

/** \brief Converts from nanoseconds to time.
    \param[in] nanos  Nanoseconds to convert
	\return Time value */
extern FIOS_EXPORT abstime_t nanosecondsToTime(U64 nanos) FIOS_ATTRIBUTE((__pure__));

// ----------------------------------------------------
// Date
// ----------------------------------------------------

/** \brief Returns the current date and time.
	\return Date value */
extern FIOS_EXPORT datetime_t currentDate();

/** \brief Converts from abstime_t to datetime_t.
	\param[in]  abstime    Time to convert
	\param[out] pDate      Filled in with datetime_t
	\return True if the conversion was successful, false if the time fell outside the representable dates. */
extern FIOS_EXPORT bool timeToDate(abstime_t time, datetime_t *pDate);

/** \brief Converts from datetime_t to abstime_t.
	\param[in]  date       Date to convert
	\param[out] pTime      Filled in with abstime_t
	\return True if the conversion was successful, false if the date fell outside the representable times. */
extern FIOS_EXPORT bool dateToTime(datetime_t date, abstime_t *pTime);

/** \brief Converts from datetime_t to abstime_t.
	\param[in]  date       Date to convert
	\return Time, or if conversion was unsuccessful, #kTIME_EARLIEST or #kTIME_LATEST. */
inline FIOS_EXPORT abstime_t dateToTime(datetime_t date) { abstime_t time; dateToTime(date, &time); return time; }

/** \brief Converts from datetime_t to native date.
	\param[in]  date         Date to convert
	\param[out] pNativeDate  Filled in with native date
	\return True if the conversion was successful, false if the date fell outside the representable dates. */
extern FIOS_EXPORT bool dateToNativeDate(datetime_t date, nativedate_t *pNativeDate);

/** \brief Converts from native date to datetime_t.
	\param[in]  pNativeDate  Native date to convert
	\param[out] pDate        Filled in with datetime_t
	\return True if the conversion was successful, false if the date fell outside the representable dates. */
extern FIOS_EXPORT bool nativeDateToDate(const nativedate_t *pNativeDate, datetime_t *pDate);

/** \brief Converts from native date to datetime_t.
	\param[in]  pNativeDate  Native date to convert
	\return Date, or if conversion was unsuccessful, #kDATE_EARLIEST or #kDATE_LATEST. */
inline FIOS_EXPORT datetime_t nativeDateToDate(const nativedate_t *pNativeDate) { datetime_t date; nativeDateToDate(pNativeDate,&date); return date; }

// ----------------------------------------------------
// Helpers
// ----------------------------------------------------

template <class T> struct remove_pointer                    { typedef T type; };
template <class T> struct remove_pointer<T*>                { typedef T type; };
template <class T> struct remove_pointer<T* const>          { typedef T type; };
template <class T> struct remove_pointer<T* volatile>       { typedef T type; };
template <class T> struct remove_pointer<T* const volatile> { typedef T type; };

// ----------------------------------------------------
// Atomic operations
// ----------------------------------------------------

extern FIOS_EXPORT bool atomicCompareAndSwap(volatile U32 *pPtr, U32 oldValue, U32 newValue);
inline bool atomicCompareAndSwap(volatile I32 *pPtr, I32 oldValue, I32 newValue) { return atomicCompareAndSwap(reinterpret_cast<volatile U32*>(pPtr),(U32)oldValue,(U32)newValue); }
extern FIOS_EXPORT bool atomicCompareAndSwap(volatile U64 *pPtr, U64 oldValue, U64 newValue);
inline bool atomicCompareAndSwap(volatile I64 *pPtr, I64 oldValue, I64 newValue) { return atomicCompareAndSwap(reinterpret_cast<volatile U64*>(pPtr),(U64)oldValue,(U64)newValue); }
extern FIOS_EXPORT bool atomicCompareAndSwap(const void * volatile *pPtr, const void *oldValue, const void *newValue);

extern FIOS_EXPORT void atomicStore(volatile U32 *pPtr, U32 value);
inline void atomicStore(volatile I32 *pPtr, I32 value) { atomicStore(reinterpret_cast<volatile U32*>(pPtr),(U32)value); }
extern FIOS_EXPORT void atomicStore(volatile U64 *pPtr, U64 value);
inline void atomicStore(volatile I64 *pPtr, I64 value) { atomicStore(reinterpret_cast<volatile U64*>(pPtr),(U64)value); }
extern FIOS_EXPORT void atomicStore(const void * volatile *pPtr, const void *value);

extern FIOS_EXPORT U32 atomicLoad(const volatile U32 *pPtr);
inline I32 atomicLoad(const volatile I32 *pPtr) { return (I32)atomicLoad(reinterpret_cast<volatile const U32*>(pPtr)); }
extern FIOS_EXPORT U64 atomicLoad(const volatile U64 *pPtr);
inline I64 atomicLoad(const volatile I64 *pPtr) { return (I64)atomicLoad(reinterpret_cast<volatile const U64*>(pPtr)); }
extern FIOS_EXPORT void * atomicLoad(const void * const volatile *pPtr);

template <class T> class FIOS_EXPORT atomicInt
{
public:
	inline atomicInt<T>() : m_value(0) {}
	inline atomicInt<T>(T value) : m_value(value) {}
	inline bool compareAndSwap(T oldValue, T newValue) { return atomicCompareAndSwap(const_cast<volatile T*>(&m_value),oldValue,newValue); }
	inline void set(T newValue) { atomicStore(const_cast<volatile T*>(&m_value),newValue); }
	inline T setBits(T bits) { T value, newValue; do { value = get(); newValue = value|bits; } while (!compareAndSwap(value,newValue)); return value; }
	inline T clearBits(T bits) { T value, newValue; do { value = get(); newValue = value&~bits; } while (!compareAndSwap(value,newValue)); return value; }
	inline T get() const { return atomicLoad(const_cast<const volatile T*>(&m_value)); }
	inline operator T () const { return get(); }
	inline atomicInt<T> & operator = (T newValue) { set(newValue); return (*this); }
	inline T add(T delta) { T value, newValue; do { value = get(); newValue = value + delta; } while (!compareAndSwap(value,newValue)); return value; }
	inline T increment() { return add(T(1)); }
	inline T decrement() { return add(T(-1)); }
	inline atomicInt<T> & operator++ () { increment(); return *this; }
	inline atomicInt<T> & operator-- () { decrement(); return *this; }
	inline atomicInt<T> & operator+= (T delta) { add(delta); return *this; }
	inline atomicInt<T> & operator-= (T delta) { add(T(-1*delta)); return *this; }
	inline T operator++ (int) { T oldValue = increment(); return oldValue; }
	inline T operator-- (int) { T oldValue = decrement(); return oldValue; }
protected:
	SCEA_ALIGN_BEG(8)
		volatile T m_value
	SCEA_ALIGN_END(8);
};

class FIOS_EXPORT atomicBool
{
public:
	typedef U32 S;

	inline atomicBool() : m_value() {}
	inline atomicBool(bool value) : m_value(S(value)) {}
	inline bool compareAndSwap(bool oldValue, bool newValue) { return m_value.compareAndSwap(S(oldValue),S(newValue)); }
	inline void set(bool newValue) { m_value.set(S(newValue)); }
	inline bool get() const { return m_value.get() ? true:false; }
	inline operator bool () const { return get(); }
	inline atomicBool & operator = (bool newValue) { set(newValue); return (*this); }

protected:
	atomicInt<S> m_value;
};

template <class T> class FIOS_EXPORT atomicPtr
{
	typedef typename remove_pointer<T>::type base;
public:
	inline atomicPtr<T>() : m_value(NULL) {}
	inline atomicPtr<T>(T value) : m_value(value) {}
	inline bool compareAndSwap(T oldValue, T newValue) { return atomicCompareAndSwap(reinterpret_cast<const void * volatile *>(const_cast<const base * volatile *>(&m_value)),oldValue,newValue); }
	inline void set(T newValue) { atomicStore(reinterpret_cast<const void* volatile *>(const_cast<const base * volatile *>(&m_value)),newValue); }
	inline T get() const { return reinterpret_cast<T>(atomicLoad(reinterpret_cast<const void* const volatile *>(const_cast<const base * const volatile *>(&m_value)))); }
	inline operator T () const { return get(); }
	inline atomicPtr<T> & operator = (T newValue) { set(newValue); return (*this); }
protected:
	SCEA_ALIGN_BEG(8)
		volatile T m_value
	SCEA_ALIGN_END(8);
};

typedef atomicInt<U32> atomicU32;
typedef atomicInt<I32> atomicI32;
typedef atomicInt<U64> atomicU64;
typedef atomicInt<I64> atomicI64;

// ----------------------------------------------------
// Threads
// ----------------------------------------------------

/**
	\enum e_THREADTYPE
	Types of threads created by FIOS.
*/
enum e_THREADTYPE {
	/** The primary scheduler thread needs low latency and high priority */
	kSchedulerThread = 0,
	/** Any media thread which is primarily I/O bound (normal media threads) */
	kIOThread = 1,          
	/** Any media thread which is primarily compute bound (caches, compression, etc) */
	kComputeThread = 2,
	/** Any other thread which should have normal priority (test threads in sample code, etc) */
	kNormalThread = 3,
	/** Any other thread which should have low priority (test threads in sample code, etc) */
	kLowPriorityThread = 4
};

/**
	Thread, same ideas as a pthread
*/
class FIOS_EXPORT thread : public object
{
public:
	FIOS_OBJECT_NEW_AND_DELETE(SCEA::Memory::kMemPurposeFIOSThreadObject)

	/**
		\brief Prototype for thread entrypoints
		\param pArg   Argument to pass to the thread
	*/
	typedef void (*entry_proc)(thread *pThread, void *pArg);

	/**
		\brief Creates a thread.
		\param[in] pName         Name for debugging
		\param[in] threadType    Type of thread to create. This may influence the stack size, priority, and other attributes.
		\param[in] pThreadProc   Pointer to thread proc
		\param[in] pArgument     Argument to the thread
		\param[in] startImmediately  Whether to start running the thread immediately.
		\see e_THREADTYPE
	*/
	thread(
		const char *pName,
		int threadType,
		entry_proc pThreadProc,
		void *pArgument,
		bool startImmediately = true);
	/**
		\brief Destroys a thread object.
	*/
	~thread();
	/**
		\brief Starts the thread if it hasn't been started already.
	*/
	void start();
	/**
		\brief Indicates whether the thread has started.
		\return True if the thread has started.
	*/
	inline bool hasStarted() const { return m_started; }
	/**
		\brief Joins with this thread object.
		The calling thread is blocked until the target thread has completed. When this call exits the
		thread will no longer be running.
	*/
	void join();
	/**
		\brief Returns an identifier for the current thread.
		\return A thread id.
	*/
	static void * currentThreadID();
	/**
		\brief Returns this thread's identifier.
		\return A thread id.
	*/
	void * getThreadID() const;
	/**
		\brief Checks to see if two thread identifiers are equal.
		\param[in] first  First thread id to compare.
		\param[in] second Second thread id to compare.
		\return Boolean value indicating whether they are equal.
	*/
	static bool equal(void *first, void *second);
	/**
		\brief Indicates whether this thread is current.
		\return Boolean value indicating whether this thread is the current thread.
	*/
	inline bool isCurrentThread() const { return thread::equal(getThreadID(),thread::currentThreadID()); }
	/**
		\brief Exits the current thread.
	*/
	static void exit();
	/**
		\brief Sleeps the current thread until a deadline.
		\param[in] deadline   Time to sleep until.
	*/
	static void sleep(abstime_t deadline);
	/**
		\brief Gets the current thread's type.
		This function should only be called from within a thread created by FIOS. On other types of
		thread, the result is undefined.
		\return One of the values defined in #e_THREADTYPE.
	*/
	static int getCurrentThreadType();
	/**
		\brief Changes the current thread's type.
		Although stack sizes cannot be adjusted, this call will adjust thread
		scheduling parameters such as priority, fifo vs round-robin, etc. This is
		generally used to depress thread priorities when a thread needs to go compute-bound.
		\param[in] newType    One of the values defined in #e_THREADTYPE. 
		\return The previous type of the thread.
	*/
	static int setCurrentThreadType(int newType);
	/**
		\brief Gets this thread's name.
		\return Name string
	*/
	inline const char * getName() const { return m_pName; }
	/**
		\brief Gets this thread's entrypoint.
		\return Thread entrypoint.
	*/
	inline entry_proc getThreadProc() const { return m_pThreadProc; }
	/**
		\brief Gets this thread's argument.
		\return Thread argument.
	*/
	inline void *getArgument() const { return m_pArgument; }
	/**
		\brief Gets the native thread (const variant).
		\return Native thread
	*/
	inline const nativethread_t & getNativeThread() const { return m_nativeThread; }
	/**
		\brief Gets the native thread (non-const variant).
		\return Native thread
	*/
	inline nativethread_t & getNativeThread() { return m_nativeThread; }
	/**
		\brief Sets the native thread.
		\param[in] nativeThread  New value.
	*/
	inline void setNativeThread(nativethread_t nativeThread) { m_nativeThread = nativeThread; }
	
protected:
	const char *m_pName;              /**< Name of this thread */
	int m_threadType;                 /**< Type of this thread */
	bool m_started;                   /**< Start flag */
	entry_proc m_pThreadProc;         /**< Entrypoint for this thread */
	void *m_pArgument;                /**< Argument for this thread */
	nativethread_t m_nativeThread;    /**< Native thread */
}; /* class thread */

// ----------------------------------------------------
// Mutexes, Conditions, RWLocks
// ----------------------------------------------------

/**
	Mutex, same idea as pthread_mutex. This mutex class should
	always allow recursive locking.
*/
class FIOS_EXPORT mutex : public object
{
public:
	FIOS_OBJECT_NEW_AND_DELETE(SCEA::Memory::kMemPurposeFIOSMutexObject)

	/** Creates a mutex.
		\param[in] pName    Name for debugging
	*/
	mutex(const char *pName);
	/** Destroys a mutex. */
	~mutex();
	/** Locks a mutex. */
	void lock();
	/** Unlocks a mutex. */
	void unlock();
	/**
		\brief Gets this mutex's name.
		\return Name string
	*/
	inline const char * getName() const { return m_pName; }
	/**
		\brief Gets the native mutex (const variant).
		\return Native mutex.
	*/
	inline const nativemutex_t & getNativeMutex() const { return m_nativeMutex; }
	/**
		\brief Gets the native mutex (non-const variant).
		\return Native mutex.
	*/
	inline nativemutex_t & getNativeMutex() { return m_nativeMutex; }
	/**
		\brief Sets the native mutex.
		\param[in] nativeMutex    New value.
	*/
	inline void setNativeMutex(nativemutex_t nativeMutex) { m_nativeMutex = nativeMutex; }
protected:
	const char *m_pName;               /**< Name of this mutex */
	nativemutex_t m_nativeMutex;       /**< Native mutex */
}; /* class mutex */

/**
	Condition, same idea as pthread_cond
*/
class FIOS_EXPORT cond : public object
{
public:
	FIOS_OBJECT_NEW_AND_DELETE(SCEA::Memory::kMemPurposeFIOSCondObject)

	/** Creates a condition.
		\param[in] pName         Name for debugging.
		\param[in] pNameSuffix   Suffix to the debugging name. Used by rwlocks.
		\param[in] pMutex        Mutex used to wait/signal this cond.
	*/
	cond(const char *pName, const char *pNameSuffix, mutex *pMutex);
	/** Destroys a condition. */
	~cond();
	/** Waits for a condition to be signalled.
		\param[in] mutex Locked mutex guarding the condition. This mutex is released while waiting, and re-acquired before control is returned to you.
		\param[in] deadline Absolute deadline for waiting, where 0 means wait forever.
		\return True if the condition was signalled before the deadline was reached.
	*/
	bool wait(mutex &mutex, abstime_t deadline = 0);
	/** Waits for a condition to be signalled.
		\param[in] pMutex Locked mutex guarding the condition. This mutex is released while waiting, and re-acquired before control is returned to you.
		\param[in] deadline Absolute deadline for waiting, where 0 means wait forever.
		\return True if the condition was signalled before the deadline was reached.
	*/
	inline bool wait(mutex *pMutex, abstime_t deadline = 0) { return wait(*pMutex, deadline); }
	/** Signals one waiter that the condition has been met. The mutex
	    should normally be held by the caller during this operation. */
	void signal();
	/** Signals all waiters that the condition has been met. The
	    mutex should normally be held by the caller during this operation. */
	void broadcast();
	/**
		\brief Gets the native condition (const variant).
		\return  Native condition.
	*/
	inline const nativecond_t & getNativeCond() const { return m_nativeCond; }
	/**
		\brief Gets the native condition (non-const variant).
		\return  Native condition.
	*/
	inline nativecond_t & getNativeCond() { return m_nativeCond; }
	/**
		\brief Sets the native condition.
		\param[in] nativeCond  New value.
	*/
	inline void setNativeCond(nativecond_t nativeCond) { m_nativeCond = nativeCond; }
protected:
	const char *m_pName;               /**< Name of this condition, used for debugging */
	const char *m_pNameSuffix;         /**< Suffix to the name of this condition, used for debugging */
	nativecond_t m_nativeCond;         /**< Native cond */
}; /* class cond */


/**
	\class rwlock
    A read-write lock, allowing multiple readers to share the lock
    while providing exclusive access to writers. Same idea as a
	pthread rwlock.
*/
#if FIOS_HAS_NATIVERWLOCK
class FIOS_EXPORT rwlock : public object
{
public:
	FIOS_OBJECT_NEW_AND_DELETE(SCEA::Memory::kMemPurposeFIOSRWLockObject)

	/** Creates an rwlock.
		\param[in] pName   Name for debugging. */
	rwlock(const char *pName);
	/** Destroys an rwlock. */
	~rwlock();
	/** Acquires a read lock. Multiple readers may have a read lock at the same time. */
	void readLock();
	/** Releases a read lock. */
	void readUnlock();
	/** Acquires a write lock. Writers have exclusive access. */
	void writeLock();
	/** Releases a write lock. */
	void writeUnlock();
	/**
		\brief Gets this rwlock's name.
		\return Name string
	*/
	inline const char * getName() const { return m_pName; }
	/**
		\brief Gets the native rwlock (const variant).
		\return  Native rwlock.
	*/
	inline const nativerwlock_t & getNativeRWLock() const { return m_nativeRWLock; }
	/**
		\brief Gets the native rwlock (non-const variant).
		\return  Native rwlock.
	*/
	inline nativerwlock_t & getNativeRWLock() { return m_nativeRWLock; }
	/**
		\brief Sets the native condition.
		\param[in] nativeRWLock  New value.
	*/
	inline void setNativeRWLock(nativerwlock_t nativeRWLock) { m_nativeRWLock = nativeRWLock; }
protected:
	const char *m_pName;               /**< Name of this condition, used for debugging */
	nativerwlock_t m_nativeRWLock;     /**< Native rwlock */
	atomicU32 m_lockCount;             /**< Lock count for implementation use */
	void *m_pOwningThread;             /**< Owning thread for implementation use */
}; /* class rwlock */

#else /* !FIOS_HAS_NATIVERWLOCK */

/* No native rwlocks... we will define a simple rwlock based on
   cond/mutex that should work for platforms without native
   rwlock support. */
class FIOS_EXPORT rwlock : public object
{
public:
	FIOS_OBJECT_NEW_AND_DELETE(SCEA::Memory::kMemPurposeFIOSRWLockObject)

	/** Creates an rwlock.
		\param[in] pName   Name for debugging. */
	rwlock(const char *pName) :
		m_pName(pName),
		m_mutex(pName),
		m_readCond(pName,"_read",&m_mutex),
		m_writeCond(pName,"_write",&m_mutex),
		m_readersWaiting(0), m_readersActive(0),
		m_writersWaiting(0), m_writersActive(0) { m_classID = object::kRWLock; }
	
	/** Destroys an rwlock. */
	~rwlock() {}
	
	/** Acquires a read lock. Multiple readers may have a read lock at the same time. */
	void readLock() {
		m_mutex.lock();
		if (m_writersActive) {
			m_readersWaiting++;
			while (m_writersActive)
				m_readCond.wait(m_mutex);
			m_readersWaiting--;
		}
		m_readersActive++;
		m_mutex.unlock();
	}
	/** Releases a read lock. */
	void readUnlock() {
		m_mutex.lock();
		if ((--m_readersActive == 0) && (m_writersWaiting > 0))
			m_writeCond.signal();
		m_mutex.unlock();
	}

	/** Acquires a write lock. Writers have exclusive access. */
	void writeLock() {
		m_mutex.lock();
		if (m_writersActive || m_readersActive) {
			m_writersWaiting++;
			while (m_writersActive || m_readersActive)
				m_writeCond.wait(m_mutex);
			m_writersWaiting--;
		}
		m_writersActive++;
		m_mutex.unlock();
	}
	/** Releases a write lock. */
	void writeUnlock() {
		m_mutex.lock();
		m_writersActive = 0;
		if (m_readersWaiting)
			m_readCond.broadcast();
		else if (m_writersWaiting)
			m_writeCond.signal();
		m_mutex.unlock();
	}
	/**
		\brief Gets this rwlock's name.
		\return Name string
	*/
	inline const char * getName() const { return m_pName; }
protected:
	const char *m_pName;       /**< Name of this rwlock */
	mutex m_mutex;             /**< Mutex */
	cond m_readCond;           /**< Read condition */
	cond m_writeCond;          /**< Write condition */
	int m_readersWaiting;      /**< Number of readers waiting. */
	int m_readersActive;       /**< Number of active readers */
	int m_writersWaiting;      /**< Number of writers waiting. */
	int m_writersActive;       /**< Number of active writers */
}; /* class rwlock */
#endif


// ----------------------------------------------------
// Thread priority changing
// ----------------------------------------------------

/** \brief Stack-based class for changing a thread's scheduling parameters. */
class FIOS_EXPORT StThreadPriorityChanger
{
public:
	/** Constructor changes thread priority of the current thread.
		\param newType       New thread type.
		\param shouldChange  Whether to actually change the priority. */
	inline StThreadPriorityChanger(int newType, bool shouldChange = true) : m_newType(newType) { if (shouldChange) change(); }
	/** Destructor restores the old priority. */
	inline ~StThreadPriorityChanger() { restore(); }
	
	/** Indicates whether the thread priority has been changed by this object.
		\return True if the thread priority has been changed by us, false otherwise. */
	inline bool isChanged() const { return m_changed; }
	/** Changes the thread priority. */
	inline void change() { if (!m_changed) m_oldType = thread::setCurrentThreadType(m_newType); m_changed = true; }
	/** Restores the thread priority. */
	inline void restore() { if (m_changed) thread::setCurrentThreadType(m_oldType); m_changed = false; }
	
protected:
	int m_oldType;                  /**< The old priority, before changing */
	int m_newType;                  /**< The new priority, after changing */
	bool m_changed;                 /**< Whether we've changed the thread type */
private:
	StThreadPriorityChanger(const StThreadPriorityChanger &) {}  /**< No copy constructor */
	StThreadPriorityChanger & operator = (StThreadPriorityChanger &) { return *this; }     /**< No assignment operator */
}; /* class StThreadPriorityChanger */



// ----------------------------------------------------
// Locking classes
// ----------------------------------------------------

/** \brief Stack-based class for locking a mutex. */
class FIOS_EXPORT StMutexLocker
{
public:
	/** Constructor locks the mutex.
		\param pMutex        Pointer to a mutex.
		\param shouldLock    Whether to lock. Default is true. */
	inline StMutexLocker(mutex *pMutex, bool shouldLock = true) : m_mutex(*pMutex), m_locked(false) { if (shouldLock) lock(); }
	/** Constructor locks the mutex.
		\param mutex         Mutex.
		\param shouldLock    Whether to lock. Default is true. */
	inline StMutexLocker(mutex &mutex, bool shouldLock = true) : m_mutex(mutex), m_locked(false) { if (shouldLock) lock(); }
	/** Destructor unlocks the mutex. */
	inline ~StMutexLocker() { unlock(); }
	
	/** Indicates whether the lock is currently held by this locker.
		\return True if the lock is currently locked by us, false otherwise. */
	inline bool isLocked() const { return m_locked; }
	/** Locks the mutex. */
	inline void lock() { if (!m_locked) m_mutex.lock(); m_locked = true; }
	/** Unlocks the mutex. */
	inline void unlock() { if (m_locked) m_mutex.unlock(); m_locked = false; }
	
protected:
	mutable mutex &m_mutex;        /**< Mutex to lock */
	bool m_locked;                 /**< Whether we've locked the mutex */
private:
	StMutexLocker(const StMutexLocker &that):m_mutex(that.m_mutex) {}  /**< No copy constructor */
	StMutexLocker & operator = (StMutexLocker &) { return *this; }     /**< No assignment operator */
}; /* class StMutexLocker */

/** \brief Stack-based class for unlocking a locked mutex. */
class FIOS_EXPORT StMutexUnlocker
{
public:
	/** Constructor unlocks the mutex.
		\param pMutex        Pointer to a mutex.
		\param shouldUnlock    Whether to unlock. Default is true. */
	inline StMutexUnlocker(mutex *pMutex, bool shouldUnlock = true) : m_mutex(*pMutex), m_locked(true) { if (shouldUnlock) unlock(); }
	/** Constructor unlocks the mutex.
		\param mutex         Mutex.
		\param shouldUnlock    Whether to unlock. Default is true. */
	inline StMutexUnlocker(mutex &mutex, bool shouldUnlock = true) : m_mutex(mutex), m_locked(true) { if (shouldUnlock) unlock(); }
	/** Destructor locks the mutex. */
	inline ~StMutexUnlocker() { lock(); }
	
	/** Locks the mutex. */
	inline void lock() { if (!m_locked) m_mutex.lock(); m_locked = true; }
	/** Unlocks the mutex. */
	inline void unlock() { if (m_locked) m_mutex.unlock(); m_locked = false; }
	
protected:
	mutable mutex &m_mutex;        /**< Mutex to lock */
	bool m_locked;                 /**< Whether we've locked the mutex */
private:
	StMutexUnlocker(const StMutexUnlocker &that):m_mutex(that.m_mutex) {}  /**< No copy constructor */
	StMutexUnlocker & operator = (StMutexUnlocker &) { return *this; }     /**< No assignment operator */
}; /* class StMutexUnlocker */

/** \brief Stack-based class for acquiring a read lock. */
class StReadLocker
{
public:
	/** Constructor acquires a read lock.
		\param pRWLock        Pointer to an rwlock.
		\param shouldLock     Whether to acquire a read lock. Default is true. */
	inline StReadLocker(rwlock *pRWLock, bool shouldLock = true) : m_rwLock(*pRWLock), m_locked(false) { if (shouldLock) lock(); }
	/** Constructor acquires a read lock.
		\param rwLock         An rwlock.
		\param shouldLock     Whether to acquire a read lock. Default is true. */
	inline StReadLocker(rwlock &rwLock, bool shouldLock = true) : m_rwLock(rwLock), m_locked(false) { if (shouldLock) lock(); }
	/** Destructor releases the read lock. */
	inline ~StReadLocker() { unlock(); }
	/** Indicates whether the lock is currently held by this locker.
		\return True if the lock is currently locked by us, false otherwise. */
	inline bool isLocked() const { return m_locked; }
	/** Acquires a read lock. */
	inline void lock() { if (!m_locked) m_rwLock.readLock(); m_locked = true; }
	/** Releases a read lock. */
	inline void unlock() { if (m_locked) m_rwLock.readUnlock(); m_locked = false; }

protected:
	mutable rwlock &m_rwLock;    /**< rwlock to lock */
	bool m_locked;               /**< Whether we're holding a read lock */
private:
	StReadLocker(const StReadLocker &that):m_rwLock(that.m_rwLock) {} /**< No copy constructor */
	StReadLocker & operator = (StReadLocker &) { return *this; }      /**< No assignment operator */
}; /* class StReadLocker */

/** \brief Stack-based class for acquiring a write lock. */
class FIOS_EXPORT StWriteLocker
{
public:
	/** Constructor acquires a write lock.
		\param pRWLock        Pointer to an rwlock.
		\param shouldLock     Whether to acquire a write lock. Default is true. */
	inline StWriteLocker(rwlock *pRWLock, bool shouldLock = true) : m_rwLock(*pRWLock), m_locked(false) { if (shouldLock) lock(); }
	/** Constructor acquires a write lock.
		\param rwLock         An rwlock.
		\param shouldLock     Whether to acquire a write lock. Default is true. */
	inline StWriteLocker(rwlock &rwLock, bool shouldLock = true) : m_rwLock(rwLock), m_locked(false) { if (shouldLock) lock(); }
	/** Destructor releases the lock. */
	inline ~StWriteLocker() { unlock(); }
	/** Indicates whether the lock is currently held by this locker.
		\return True if the lock is currently locked by us, false otherwise. */
	inline bool isLocked() const { return m_locked; }
	/** Acquires a write lock. */
	inline void lock() { if (!m_locked) m_rwLock.writeLock(); m_locked = true; }
	/** Releases a write lock. */
	inline void unlock() { if (m_locked) m_rwLock.writeUnlock(); m_locked = false; }

protected:
	mutable rwlock &m_rwLock;    /**< rwlock to lock */
	bool m_locked;               /**< Whether we're holding a write lock */
private:
	StWriteLocker(const StWriteLocker &that):m_rwLock(that.m_rwLock) {} /**< No copy constructor */
	StWriteLocker & operator = (StWriteLocker &) { return *this; }      /**< No assignment operator */
}; /* class StWriteLocker */


// ----------------------------------------------------
// Console printing
// ----------------------------------------------------

/** \brief Printf function for debugging output
	This is usually equivalent to the platform's standard printf, but may tweak the
	behavior a bit to compensate for shortcomings in the platform's C library. For
	example, on Win32 FIOS will dynamically translate %lld into %I64d, and on PSP
	FIOS explicitly works around the PSP printf's lack of thread-safety.
	\param fmt   Format string
	\param ...   Additional parameters as in printf.
	\see FIOSvprintf
*/
extern FIOS_EXPORT void FIOSprintf(const char *fmt, ...)
	 FIOS_ATTRIBUTE((__format__(__printf__,1,2)))
	 FIOS_ATTRIBUTE_GCC((__nonnull__(1)));

/** \brief Vprintf function for debugging output
	This is usually equivalent to the platform's standard vprintf, but may tweak the
	behavior a bit to compensate for shortcomings in the platform's C library. For
	example, on Win32 FIOS will dynamically translate %lld into %I64d, and on PSP
	FIOS explicitly works around the PSP printf's lack of thread-safety.
	\param fmt   Format string
	\param ap    va_list with additional parameters as in printf.
*/
extern FIOS_EXPORT void FIOSvprintf(const char *fmt, va_list ap)
	 FIOS_ATTRIBUTE((__format__(__printf__,1,0)))
	 FIOS_ATTRIBUTE_GCC((__nonnull__(1)));


	}; /* namespace platform */

/*@}*/

}; /* namespace fios */

#endif /* _H_fios_platform */
