/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES_POWERED_CHAIN_MAPPER_UTIL_H
#define HK_UTILITIES_POWERED_CHAIN_MAPPER_UTIL_H

#include <hkbase/hkBase.h>


class hkPoweredChainMapperUtil
{
public:

		/// Adds all chains and limit constraints to the world, asserts if the entities are not there yet.
	static void HK_CALL addToWorld(class hkWorld* world, class hkPoweredChainMapper* mapper);

		/// removes all constraints from the world, if they're still in it.
	static void HK_CALL removeFromWorld(class hkWorld* world, class hkPoweredChainMapper* mapper);

};


#endif // HK_UTILITIES_POWERED_CHAIN_MAPPER_UTIL_H

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
