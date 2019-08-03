/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CHUNK_CACHE
#define HK_CHUNK_CACHE

#include <hkbase/hkBase.h>

/// An interface for cached chunks of data used by the animation playback system.
class hkChunkCache : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ANIM_CACHE);

			/// Retrieves a read only pointer to a preallocated/cached chunk ( returns HK_NULL if key not available )
		virtual const hkUint8* retrieveChunk( hkUint32 key, hkUint32 chunkSize ) = 0;

			/// Allocates a chunk from the cache and binds it to the given key
			/// Returns HK_NULL if it can't allocate
		virtual hkUint8* allocateChunk( hkUint32 key, hkUint32 chunkSize ) = 0;

			/// Marks the chunk associated with the given key as available for reuse / reallocation
		virtual hkBool flushKey( hkUint32 key, hkUint32 chunkSize ) = 0;

			/// Writes cache statistics to an output stream
		virtual hkBool printCacheStats( hkOstream* oStream ) const = 0;
};


#endif // HK_CHUNK_CACHE


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
