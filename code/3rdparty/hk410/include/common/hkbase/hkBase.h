/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_HKBASE_H
#define HKBASE_HKBASE_H

#include <hkbase/types/hkBaseTypes.h>
#if defined HK_COMPILER_MSVC
	// C4100 'identifier' : unreferenced formal parameter
	//		a function need not use all its arguments
	// C4127 conditional expression is constant
	//		constant conditionals are often used inside asserts
	// C4505 'function' : unreferenced local function has been removed
	//		lots of inline functions are not used in a compilation unit
	// C4510 'class' : default constructor could not be generated
	// C4511 'class' : copy constructor could not be generated
	// C4512 'class' : assignment operator could not be generated
	//		many classes are not designed with value semantics
	// C4514 unreferenced inline/local function has been removed
	//		lots of inline functions are not used in a compilation unit
	// C4714 force inlined function not inlined. This warning is only disabled in debug modes.
#	pragma warning(push)
#	pragma warning(disable: 4100 4127 4324 4505 4510 4511 4512 4514)
#	if defined(HK_DEBUG)
#		pragma warning(disable: 4714) 
#	endif
#	ifndef _CRT_SECURE_NO_DEPRECATE
#		define _CRT_SECURE_NO_DEPRECATE 1
#	endif
#	ifndef _CRT_NONSTDC_NO_DEPRECATE
#		define _CRT_NONSTDC_NO_DEPRECATE 1
#	endif
#endif

/// enable the next define if you want to do SPU debugging on a WIN32 platform
// temporarily moved here
#if defined HK_PLATFORM_WIN32
//#	define HK_SIMULATE_SPU_DMA_ON_CPU
#endif

#if defined(HK_SIMULATE_SPU_DMA_ON_CPU) || defined(HK_PLATFORM_PS3SPU)
#	define HK_PLATFORM_POTENTIAL_SPU
#	if defined( HK_SIMULATE_SPU_DMA_ON_CPU )
#		define HK_PLATFORM_IS_SPU (hkSpuSimulator::Client::getInstance())
#	else
#		define HK_PLATFORM_IS_SPU true
#	endif
#else
#	define HK_PLATFORM_IS_SPU false
#endif
#if defined(HK_PLATFORM_PS3) || defined(HK_PLATFORM_PS3SPU) || defined(HK_SIMULATE_SPU_DMA_ON_CPU)
#	define HK_PLATFORM_HAS_SPU
#endif

#include <hkbase/memory/hkThreadMemory.h>	


#include <hkbase/baseobject/hkBaseObject.h>
#include <hkbase/baseobject/hkReferencedObject.h>
#include <hkbase/singleton/hkSingleton.h>
#include <hkbase/stream/hkOstream.h>	/* <todo> remove */

#include <hkbase/error/hkError.h>

#include <hkbase/htl/hkArray.h>
#include <hkbase/string/hkString.h>		/* <todo> remove, move static functions elsewhere */

#include <hkbase/memory/hkThreadMemory.inl>

#endif // HKBASE_HKBASE_H


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
