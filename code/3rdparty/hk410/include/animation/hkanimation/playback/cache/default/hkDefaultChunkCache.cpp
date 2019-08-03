/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/playback/cache/default/hkDefaultChunkCache.h>


hkDefaultChunkCache::hkDefaultChunkCache( struct hkDefaultChunkCacheCinfo& cinfo )
{
	// parse to cinfo to construct a new cache
	if( cinfo.m_numberOfCachePools )
	{
		// set the number of cache pools the will be handling
		m_numberOfCachePools = cinfo.m_numberOfCachePools;

		// check for any array size mismatch issues		
		if( cinfo.m_cachePools.getSize() != (int)m_numberOfCachePools ) 
		{
			HK_ASSERT2( 0x11470384, HK_NULL, "Number of cache pools specified does not match array data size!" );
		}

		// set the array sizes to match this		
		m_cachePools.setSize( m_numberOfCachePools );
		m_cacheData.setSize( m_numberOfCachePools );
		m_cachePoolTables.setSize( m_numberOfCachePools );

#if defined( HK_CACHE_STATS )

		m_cachePoolStatsTables.setSize( m_numberOfCachePools );

#endif

		// copy the cinfo array data to the internal array
		m_cachePools = cinfo.m_cachePools;

		// verify that the pool chunk sizes are in ascending order
		{
			for( hkUint32 i = 0; i < ( m_numberOfCachePools - 1 ); i++ )
			{
				if( m_cachePools[i].m_chunkSize > m_cachePools[i+1].m_chunkSize )
				{
					HK_ASSERT2( 0x11cfac58, HK_NULL, "Cache pool chunk sizes are not in ascending order!" );
				}
			}

		}

		// allocate heap memory for the cache pool data chunks
		{
			for( hkUint32 i = 0; i < m_numberOfCachePools; i++ )
			{
				// every slot in the the various buckets now has 'chunk' number of bytes allocated
				m_cacheData[i] = hkAllocateChunk<hkUint8>( ( m_cachePools[i].m_chunkSize * 
											  m_cachePools[i].m_buckets * 
											  m_cachePools[i].m_slots ), HK_MEMORY_CLASS_ANIM_CACHE );
			}
		}

		// allocate heap memory for the cache pool information tables
		{
			for( hkUint32 i = 0; i < m_numberOfCachePools; i++ )
			{
				// this gives us a grid table structure
				m_cachePoolTables[i] = hkAllocateChunk<struct slotUnit>( m_cachePools[i].m_buckets * m_cachePools[i].m_slots, HK_MEMORY_CLASS_ANIM_CACHE );

				// initialise each slot unit
				for( hkUint32 j = 0; j < ( m_cachePools[i].m_buckets * m_cachePools[i].m_slots ); j++ )
				{
					m_cachePoolTables[i][j].m_key = HK_NULL;
					m_cachePoolTables[i][j].m_lru = ( j % m_cachePools[i].m_slots );	// L.R.U. scheme
				}
			}
		}

#if defined( HK_CACHE_STATS )

		// allocate heap memory for the cache pool statistics tables
		{
			for( hkUint32 i = 0; i < m_numberOfCachePools; i++ )
			{
				// this gives us a grid table structure
				m_cachePoolStatsTables[i] = hkAllocateChunk<struct slotStatisticsInfo>( m_cachePools[i].m_buckets * m_cachePools[i].m_slots, HK_MEMORY_CLASS_ANIM_CACHE );

				// initialise each slot statistics structure
				for( hkUint32 j = 0; j < ( m_cachePools[i].m_buckets * m_cachePools[i].m_slots ); j++ )
				{
					m_cachePoolStatsTables[i][j].m_hits = 0;
					m_cachePoolStatsTables[i][j].m_misses = 0;
				}
			}
		}

#endif

	}

	else
	{
		HK_ASSERT2( 0x221a5af2, HK_NULL, "Zero cache pools specified in cache cinfo!" );
	}
}


hkDefaultChunkCache::~hkDefaultChunkCache()
{
	// free allocated heap memory for cache pools and hash tables
	for( hkUint32 i = 0; i < m_numberOfCachePools; i++ )
	{
		// heap de-allocations
		const hkUint32 numSlots = m_cachePools[i].m_buckets * m_cachePools[i].m_slots;
		hkDeallocateChunk( m_cacheData[i], ( m_cachePools[i].m_chunkSize * numSlots ), HK_MEMORY_CLASS_ANIM_CACHE );
		hkDeallocateChunk( m_cachePoolTables[i], numSlots, HK_MEMORY_CLASS_ANIM_CACHE );

#if defined( HK_CACHE_STATS )
		hkDeallocateChunk( m_cachePoolStatsTables[i], numSlots, HK_MEMORY_CLASS_ANIM_CACHE );
#endif
	}
}


const hkUint8* hkDefaultChunkCache::retrieveChunk( hkUint32 key, hkUint32 chunkSize )
{
	// retrieve the specified key
	hkUint8* chunkPointer = HK_NULL;
	hkBool success = manipulateKey( key, chunkSize, 3, &chunkPointer );

	return( success ? chunkPointer : HK_NULL );
}


hkBool hkDefaultChunkCache::lockKey( hkUint32 key, hkUint32 chunkSize )
{
	// lock the specified key
	return( manipulateKey( key, chunkSize, 1, HK_NULL ) );
}


hkBool hkDefaultChunkCache::unlockKey( hkUint32 key, hkUint32 chunkSize )
{
	// unlock the specified key
	return( manipulateKey( key, chunkSize, 0, HK_NULL ) );
}


hkBool hkDefaultChunkCache::flushKey( hkUint32 key, hkUint32 chunkSize )
{
	// flush the specified key
	return( manipulateKey( key, chunkSize, 2, HK_NULL ) );
}


hkBool hkDefaultChunkCache::manipulateKey( hkUint32 key, hkUint32 chunkSize, hkUint8 operation, hkUint8** chunkPointer )
{
	// fill in hashKeyInfo structure to query cache
	struct hashKeyInfo info;

	info.m_key = key;
	info.m_chunkSize = chunkSize;

	// run the key through the hashing function
	hkBool success = hashKey( info );

	if( !success )
	{
		// hashed key failure
		return false;
	}

	// get pointer to cache pool bucket
	struct slotUnit* slot = m_cachePoolTables[ info.m_cachePool ];
	slot += ( info.m_bucket * m_cachePools[ info.m_cachePool ].m_slots );

#if defined( HK_CACHE_STATS )

	// get pointer to the cache pool stats table
	struct slotStatisticsInfo* slotSI = m_cachePoolStatsTables[ info.m_cachePool ];
	slotSI += ( info.m_bucket * m_cachePools[ info.m_cachePool ].m_slots );

#endif

	// try find the key in this bucket
	for( hkUint32 i = 0; i < m_cachePools[ info.m_cachePool ].m_slots; i++ )
	{
		if( slot[i].m_key == key )
		{

			switch( operation )
			{
			case( 0 ):
				{
					// found key - unlock it ( lru == -1 is the lock signifier )
					if( slot[i].m_lru == -1 )
					{
						hkUint32 unlockedSlots = 0;

						// key returns to the available pool with highest m_lru value	
						for( hkUint32 j = 0; j < m_cachePools[ info.m_cachePool ].m_slots; j++ )
						{
							if( slot[j].m_lru != -1 )
							{
								unlockedSlots++;
							}
						}

						slot[i].m_lru = unlockedSlots;

					}

					break;
				}

			case( 1 ):
				{
					// found key - lock it ( m_lru == -1 is the lock signifier )
					slot[i].m_lru = -1;
					break;
				}

			case( 2 ):
				{
					// flush key
					slot[i].m_key = HK_NULL;

					// re-shuffle the L.R.U. frequency values appropriately
					hkInt32 lru = slot[i].m_lru;

					for( hkUint32 idx = 0; idx < m_cachePools[ info.m_cachePool ].m_slots; idx++ )
					{
						if( slot[ idx ].m_lru > lru )
						{
							slot[ idx ].m_lru--;
						}

					}

					// set the flushed slot to be the first to be re-used
					slot[i].m_lru = ( m_cachePools[ info.m_cachePool ].m_slots - 1 );

					break;
				}

			case( 3 ):
				{
					// retrieve key ( assuming user will read data )
					*chunkPointer = ( ( m_cacheData[ info.m_cachePool ] ) +
									  ( m_cachePools[ info.m_cachePool ].m_slots * info.m_bucket * m_cachePools[ info.m_cachePool ].m_chunkSize ) +
									  ( i * m_cachePools[ info.m_cachePool ].m_chunkSize ) 
									);

#if defined( HK_CACHE_STATS )

					// update the stats table for read access to slot
					slotSI[i].m_hits++;
#endif

					break;
				}

			default:
				{
				// invalid option
				return false;
				}
			}

			// successfully (un)locked / retrieved / flushed key
			return true;

		}
	}

	// could not find the key - unable to lock
	return false;
}


hkUint8* hkDefaultChunkCache::allocateChunk( hkUint32 key, hkUint32 chunkSize )
{
	// fill in hashKeyInfo structure to query cache
	struct hashKeyInfo info;

	info.m_key = key;
	info.m_chunkSize = chunkSize;

	// run the key through the hashing function
	hkBool success = hashKey( info );

	if( !success )
	{
		// hashed key failure
		return HK_NULL;
	}

	// get pointer to cache pool bucket
	struct slotUnit* slot = m_cachePoolTables[ info.m_cachePool ];
	slot += ( info.m_bucket * m_cachePools[ info.m_cachePool ].m_slots );

#if defined( HK_CACHE_STATS )

	// get pointer to the cache pool stats table
	struct slotStatisticsInfo* slotSI = m_cachePoolStatsTables[ info.m_cachePool ];
	slotSI += ( info.m_bucket * m_cachePools[ info.m_cachePool ].m_slots );

#endif

	// convienence structure
	struct highestUnlockedSlot
	{
		// which slot
		hkInt32 m_slot;

		// L.R.U. value
		hkInt32 m_lru;

		highestUnlockedSlot()
		{
			m_slot = -1;
			m_lru = -1;
		}

	};
	highestUnlockedSlot hus;

	// number of unlocked slots
	hkUint32 unlockedSlots = 0;

	// try find the least recently used slot in this bucket ( checking for locked slots )
	for( hkUint32 i = 0; i < m_cachePools[ info.m_cachePool ].m_slots; i++ )
	{
		// check if the slot is locked
		if( slot[i].m_lru != -1 )
		{
			if( slot[i].m_lru > hus.m_lru )
			{
				hus.m_slot = i;
				hus.m_lru = slot[i].m_lru;
			}

			unlockedSlots++;

		}
	}

	if( !unlockedSlots )
	{
		return HK_NULL;
	}


	// increment each slot's lru value modulus the number of unlocked slots
	for( hkUint32 j = 0; j < m_cachePools[ info.m_cachePool ].m_slots; j++ )
	{
		if( slot[j].m_lru != -1 )
		{
			slot[j].m_lru++;
			slot[j].m_lru = ( slot[j].m_lru % unlockedSlots );
		}
	}

	// set slot attributes
	slot[ hus.m_slot ].m_key = key;
	slot[ hus.m_slot ].m_lru = 0;

	hkUint8* chunkPointer = ( ( m_cacheData[ info.m_cachePool ] ) +
								( m_cachePools[ info.m_cachePool ].m_slots * info.m_bucket * m_cachePools[ info.m_cachePool ].m_chunkSize ) +
								( hus.m_slot * m_cachePools[ info.m_cachePool ].m_chunkSize ) 
							);

#if defined( HK_CACHE_STATS )

	// update the stats table for trashing to slot
	slotSI[ hus.m_slot ].m_misses++;

#endif


	return( chunkPointer );

}


hkBool hkDefaultChunkCache::flushCachePool( hkUint32 cachePool )
{
	if( cachePool < m_numberOfCachePools )
	{
		// flushes ALL entries in the specified cache pool ( does not observe lock state )
		struct slotUnit* slot = m_cachePoolTables[ cachePool ];

		for( hkUint32 i = 0; i < ( m_cachePools[ cachePool ].m_buckets * m_cachePools[ cachePool ].m_slots ); i++ )
		{
			// reset the slotUnit
			slot[i].m_key = HK_NULL;
			slot[i].m_lru = ( i % m_cachePools[ cachePool ].m_slots );	// L.R.U. scheme
		}

		// success
		return true;
	}

	else
	{
		// failure
		return false;
	}

}


hkBool hkDefaultChunkCache::printCacheStats( hkOstream* oStream ) const
{

#if !defined( HK_CACHE_STATS )

	*oStream << "\nCache statistics are only available when \"HK_CACHE_STATS\" has been defined!\n";
	return false;

#else

	hkUint32 i = 0;

	*oStream << "\n --- BEGIN CACHE STATISTICS --- \n";

	// display unavailable chunk size information
	*oStream << "\nRequests for unavailable chunk sizes:\n";
	*oStream << "-------------------------------------\n";

	for( i = 0; i < hkUint32(m_unavailableChunks.getSize()); i++ )
	{
		// print chunk details
		*oStream << "Chunk Size: " << m_unavailableChunks[i].m_chunkSize << "\t# Requests: " << m_unavailableChunks[i].m_frequency << "\n";
	}

	if( !m_unavailableChunks.getSize() )
	{
		*oStream << "None.\n";
	}

	// display information on cache pool usage
	*oStream << "\n\nCache Pools slot activity ( hits / misses ) information:\n";
	*oStream << "--------------------------------------------------------\n";

	// display individual slot information
	for( i = 0; i < m_numberOfCachePools; i++ )
	{
		*oStream << "\n.......................................\n";
		*oStream << "  Cache Pool: " << i << "    Chunk Size: " << m_cachePools[i].m_chunkSize << "   \n";
		*oStream << ".......................................\n";

		for( hkUint32 j = 0; j < m_cachePools[i].m_buckets; j++ )
		{
			*oStream << "\nBucket " << j << "\n";

			if( j > 9 )
			{
				*oStream << "^^^^^^^^^\n";
			}

			else
			{
				*oStream << "^^^^^^^^\n";
			}

			for( hkUint32 k = 0; k < m_cachePools[i].m_slots; k++ )
			{
				*oStream << "\t\tSlot " << k << " ( " << m_cachePoolStatsTables[i][( j * m_cachePools[i].m_slots) + k].m_hits << 
							" / " << m_cachePoolStatsTables[i][( j * m_cachePools[i].m_slots) + k].m_misses << " )\n";

			}

		}

	}

	*oStream << "\n --- END CACHE STATISTICS --- \n\n";

	return true;

#endif

}


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
