/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SERIALIZE_OBJECT_DEEP_COPIER_H
#define HK_SERIALIZE_OBJECT_DEEP_COPIER_H

template <typename K, typename V> class hkPointerMap;

/// Utility to copy an object and all pointed objects.
namespace hkDeepCopier
{
		/// Source To 
	typedef hkPointerMap<const void*, void*> CopyFromOriginal;

		/// Copy an object and all pointed objects.
		/// Copied objects are flattened into a single allocation
		/// like a headerless packfile.
		/// If copyFromOriginal is not null, it is checked for previously copied objects
		/// and deep copying uses these objects instead of recursing. Newly
		/// copied objects are also written into it on completion.
	void* deepCopy( const void* dataIn, const hkClass& klassIn, CopyFromOriginal* copyFromOriginal );
}

#endif //HK_SERIALIZE_DEEP_COPIER_H

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
