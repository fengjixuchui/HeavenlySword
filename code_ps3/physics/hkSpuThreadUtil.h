/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_SPU_UTIL_H
#define HK_UTILITIES2_SPU_UTIL_H

#ifndef HK_PLATFORM_PS3
#	error This file should only be included on ps3 builds.
#endif

#include <hkbase/thread/hkSemaphore.h>
#include <hkbase/thread/hkThread.h>

#include <sys/spu_thread.h>

#define MAX_NUM_SPU_THREADS 4

//
// Wrapper class for creating SPU threads. It has an analogous running mode on the PC.
//
class hkSpuThreadUtil
{
	public:
		//
		//	Start threads from loaded program
		//
		void startSpuThreads( int32_t numSpus, void *arg1, void *arg2, void *arg3, void *arg4 );

	public:
		// Load SPU program
		hkSpuThreadUtil( char *spuProg );

		// Destroy threads
		~hkSpuThreadUtil();

	private:
		int m_numSpus;

		sys_spu_thread_t			threads[ MAX_NUM_SPU_THREADS ];	// SPU thread IDs 
		sys_spu_thread_group_t		m_groupId;						// SPU thread group ID 
		sys_spu_segment_t*			m_segments;						// The SPU segments 
		sys_spu_thread_attribute_t	m_thread_attr;					// SPU thread attribute 

		hkThread					m_thread[ 5 ];
};

int hkGetThreadId();

#endif // !HK_UTILITIES2_SPU_UTIL_H

/*
* Havok SDK - DEMO RELEASE, BUILD(#20060116)
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
