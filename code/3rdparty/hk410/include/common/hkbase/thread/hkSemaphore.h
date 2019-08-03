/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HK_SEMAPHORE_H
#define HKBASE_HK_SEMAPHORE_H

#include <hkbase/config/hkConfigThread.h>

#if defined(HK_PLATFORM_PS3) 
#	include <sys/synchronization.h>	
#endif

	/// A wrapper class for a semaphore.
	/// Semaphores are about 10 times slower than critical sections, but
	/// they are more flexible.
	/// You can think of a Semaphore as a set of tokens.
	/// A call to acquire() grabs a token and release puts a token back.
	/// If acquire() can not get a token, it simply waits until another thread calls release.
	/// If the number of tokens is maxCount, release will do nothing.
class hkSemaphore
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE, hkSemaphore);

			/// Create a semaphore with an initial count and a maximum count
		hkSemaphore( int initialCount = 0, int maxCount = 1000 );

			/// Destruct the Semaphore
		~hkSemaphore();

			/// This call will block until the semaphore is released.
		void acquire();
			
			/// Release the semaphore. Releases a thread blocked by acquire().
		void release(int count = 1);

	protected:

#if defined(HK_PLATFORM_PS3) && (HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED)

		struct hkSemaphorePS3
		{
			int         curCount;
			int         maxCount;
			sys_mutex_t mutex;
			sys_cond_t  cond;
		} m_semaphore;

#else
		void* m_semaphore;
#endif

	public:
		hkSemaphore( hkFinishLoadedObjectFlag f) {}
};

#endif // HKBASE_HK_SEMAPHORE_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
