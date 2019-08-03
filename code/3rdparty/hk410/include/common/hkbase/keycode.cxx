/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// If you are evaluating Havok please add your evaluation keycode to the appropriate platform 
// section below. Three sections are defined to allow you to use the same file across multiple
// platforms.

//
// NB. If you have a client keycode and keyvalue, these are cross-platform and you only need
// to specify it once for all platforms.
//

// In macro below <code> is your key-code and <value> is your client key-value.
// The client key-value should only be specified with a valid client key-code
// and left at its default value of 0x0 otherwise.

#if defined(HK_PLATFORM_GC)
#	define KEYCODE_ATTRIBUES __attribute__((section(".sdata")))
#else
#	define KEYCODE_ATTRIBUES
#endif

#define HAVOK_KEYCODE(code, value)					\
	extern const char         HK_KEYCODE[] KEYCODE_ATTRIBUES = code;	\
	extern const unsigned int HK_KEYVALUE  KEYCODE_ATTRIBUES = value

// Enter your keycode below in the form: HAVOK_KEYCODE(code,0x0)
HAVOK_KEYCODE("CLIENT.PhAn.NinjaTheory", 0x76373eb2);


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
