/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/stopwatch/hkSystemClock.h>

#if defined(HK_PLATFORM_WIN32) || defined(HK_PLATFORM_XBOX) || defined(HK_PLATFORM_XBOX360)
#	include <hkbase/stopwatch/impl/hkWindowsSystemClock.h>
	typedef hkWindowsSystemClock hkDefaultSystemClock;

#elif defined(HK_ARCH_IA32) && defined(HK_PLATFORM_UNIX)
#	include <hkbase/stopwatch/impl/hkLinuxSystemClock.h>
	typedef hkLinuxSystemClock hkDefaultSystemClock;

#elif defined(HK_PLATFORM_PS2)
#	include <hkbase/stopwatch/impl/hkPs2SystemClock.h>
	typedef hkPs2SystemClock hkDefaultSystemClock;

#elif defined(HK_PLATFORM_MAC)
#	include <hkbase/stopwatch/impl/hkMacSystemClock.h>
	typedef hkMacSystemClock hkDefaultSystemClock;

#elif defined( HK_PLATFORM_PS3) 
#   include <hkbase/stopwatch/impl/hkPs3SystemClock.h>
    typedef hkPs3SystemClock hkDefaultSystemClock;

#elif defined (HK_PLATFORM_PS3SPU)
#   include <hkbase/stopwatch/impl/hkPs3SystemClock.h>
	typedef hkSystemClock hkDefaultSystemClock;

#elif defined(HK_PLATFORM_GC)
#	include <hkbase/stopwatch/impl/hkNgcSystemClock.h>
	typedef hkNgcSystemClock hkDefaultSystemClock;

#elif defined(HK_PLATFORM_PSP)
#	include <hkbase/stopwatch/impl/hkR3000SystemClock.h>
	typedef hkR3000SystemClock hkDefaultSystemClock;
	
#else
#  error ERROR: No SystemClock implementation available!
#endif

#if !defined(HK_PLATFORM_PS3SPU)

#if defined(HK_COMPILER_MWERKS)
#	pragma force_active on
#endif

HK_SINGLETON_CUSTOM_IMPLEMENTATION(hkSystemClock, hkDefaultSystemClock);

#endif

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
