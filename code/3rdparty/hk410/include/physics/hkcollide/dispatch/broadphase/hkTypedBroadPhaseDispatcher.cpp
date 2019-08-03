/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseDispatcher.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandle.h>
#include <hkcollide/dispatch/broadphase/hkNullBroadPhaseListener.h>

#include <hkbase/htl/hkAlgorithm.h>

#include <hkbase/debugutil/hkTraceStream.h>

#include <hkbase/htl/hkPointerMapBase.h>
#include <hkbase/memory/hkLocalBuffer.h>

hkTypedBroadPhaseDispatcher::hkTypedBroadPhaseDispatcher()
{
	m_nullBroadPhaseListener = new hkNullBroadPhaseListener();
	for (int a = 0; a < HK_MAX_BROADPHASE_TYPE; a++ )
	{
		for (int b = 0; b < HK_MAX_BROADPHASE_TYPE; b++ )
		{
			m_broadPhaseListeners[a][b] = m_nullBroadPhaseListener;
		}
	}
}

hkTypedBroadPhaseDispatcher::~hkTypedBroadPhaseDispatcher()
{
	delete m_nullBroadPhaseListener;
}

void hkTypedBroadPhaseDispatcher::addPairs(	hkTypedBroadPhaseHandlePair* newPairs, int numNewPairs, const hkCollidableCollidableFilter* filter ) const
{
	while ( --numNewPairs >=0 )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(newPairs->m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(newPairs->m_b)->getOwner() );

		if( filter->isCollisionEnabled( *collA, *collB ) )
		{
			int typeA = newPairs->getElementA()->getType();
			int typeB = newPairs->getElementB()->getType();
			HK_ASSERT(0xf0ff0010, 0 <= typeA && typeA < HK_MAX_BROADPHASE_TYPE);
			HK_ASSERT(0xf0ff0011, 0 <= typeB && typeB < HK_MAX_BROADPHASE_TYPE);
			m_broadPhaseListeners[typeA][typeB]->addCollisionPair(*newPairs);
		}
		newPairs++;
	}
}

void hkTypedBroadPhaseDispatcher::removePairs( hkTypedBroadPhaseHandlePair* deletedPairs, int numDeletedPairs ) const
{
	while ( --numDeletedPairs >=0 )
	{
		int typeA = deletedPairs->getElementA()->getType();
		int typeB = deletedPairs->getElementB()->getType();
		HK_ASSERT(0xf0ff0012, 0 <= typeA && typeA < HK_MAX_BROADPHASE_TYPE);
		HK_ASSERT(0xf0ff0013, 0 <= typeB && typeB < HK_MAX_BROADPHASE_TYPE);
		m_broadPhaseListeners[typeA][typeB]->removeCollisionPair(*deletedPairs);
		deletedPairs++;
	}
}

// value = position << 32 + count

#define POSITION_FROM_VALUE(value)    (hkUint32(value) >> 8 )
#define COUNT_FROM_VALUE(value)        (0xff & int(value))
#define VALUE_FROM_POSITION_AND_COUNT(position, count) hkUint64( (position << 8) | count )

static inline hkUint64 keyFromPair( const hkBroadPhaseHandlePair& pair )
{
#if ( HK_POINTER_SIZE == 4 )
	return reinterpret_cast<const hkUint64&>(pair);
#elif ( HK_POINTER_SIZE == 8 )
	// merge the lower 32 bits from each 64bit pointer to form a single 64bit value ( pair.m_a | pair.m_b )
	hkUlong a = reinterpret_cast<hkUlong>(pair.m_a);
	hkUlong b = reinterpret_cast<hkUlong>(pair.m_b);
	HK_ASSERT( 0, (a & 0xffffffff00000000) == 0 );
	HK_ASSERT( 0, (b & 0xffffffff00000000) == 0 );
	return (a & 0x00000000ffffffff) | ((b & 0x00000000ffffffff ) << 32 );
#endif
}

void hkTypedBroadPhaseDispatcher::removeDuplicates( hkArray<hkBroadPhaseHandlePair>& newPairs, hkArray<hkBroadPhaseHandlePair>& delPairs )
{
	hkToiPrintf("rem.dupl", "# removing duplicates: %dn %dd\n", newPairs.getSize(), delPairs.getSize());

	int min = hkMath::min2( newPairs.getSize(), delPairs.getSize() );

	if ( min < 32  ) 
	{
		for (int d = 0; d < delPairs.getSize(); d++)
		{
			for (int n = 0; n < newPairs.getSize(); n++)
			{
				if (		(newPairs[n].m_a == delPairs[d].m_a && newPairs[n].m_b == delPairs[d].m_b )
						||	(newPairs[n].m_b == delPairs[d].m_a && newPairs[n].m_a == delPairs[d].m_b ) )
				{
					newPairs.removeAt(n);
					delPairs.removeAt(d--);
					break;
				}
			}
		}
		return;
	}


	HK_COMPILE_TIME_ASSERT( sizeof(hkBroadPhaseHandlePair) == 2*sizeof(void*) );

	{
		int bufferSizeBytes = hkPointerMapBase<hkUint64>::getSizeInBytesFor(newPairs.getSize());
		hkLocalBuffer<char> buffer(bufferSizeBytes);
		hkPointerMapBase<hkUint64> newPairsTable( buffer.begin(), bufferSizeBytes );

		{
			for (int n = 0; n < newPairs.getSize(); n++)
			{
				hkBroadPhaseHandlePair pair = newPairs[n];
				if (pair.m_a > pair.m_b)
				{
					hkAlgorithm::swap(pair.m_a, pair.m_b);
				}
				hkUint64 key = keyFromPair(pair);
				hkPointerMapBase<hkUint64>::Iterator it = newPairsTable.findKey( key );
				if (newPairsTable.isValid(it))
				{
					hkInt64 value = newPairsTable.getValue(it);
					// increase count (lower hkInt16)
					value++;
					HK_ASSERT2(0xad000730, COUNT_FROM_VALUE(value) != 0, "Count overflow");
					newPairsTable.setValue(it, value);

					// note: we'd need to store the position of this doubled entry here.
					// but as we may assume that such a doubled entry will have a corresponding
					// deletedPair we mark it invalid in  the newPairsList straight away
					newPairs[n].m_a = HK_NULL;
				}
				else
				{
					hkInt64 value = VALUE_FROM_POSITION_AND_COUNT(n, 1);
					newPairsTable.insert( key, value );
				}
			}
		}

		{
			for (int d = 0; d < delPairs.getSize(); d++)
			{
				hkBroadPhaseHandlePair pair = delPairs[d];
				if (pair.m_a > pair.m_b)
				{
					hkAlgorithm::swap(pair.m_a, pair.m_b);
				}


				hkPointerMapBase<hkUint64>::Iterator it = newPairsTable.findKey( keyFromPair(pair) );

				if (newPairsTable.isValid(it))
				{
					// remove both entries from the list
					//hkUint64 n = newPairsTable.getValue(it);
					//newPairsTable.remove(it);
					//newPairs[(int)n].m_a = HK_NULL;
					//delPairs.removeAt(d--);

					// 
					{
						hkUint64 value = newPairsTable.getValue(it);
						hkInt32 count = COUNT_FROM_VALUE(value);
						if (count > 1)
						{
							value--;
							newPairsTable.setValue(it, value);
						}
						else // count == 1
						{
							newPairsTable.remove(it);
							hkInt32 n = POSITION_FROM_VALUE(value);
							newPairs[n].m_a = HK_NULL;
						}
					}

					delPairs.removeAt(d--);
				}
			}
		}

		// Shrink back the newPairs list.
		{
			int nextNull = 0;
			for (int i=0 ; i < newPairs.getSize(); i++)
			{
				if (newPairs[i].m_a != HK_NULL)
				{
					newPairs[nextNull++] = newPairs[i];
				}
			}
			newPairs.setSize(nextNull);
		}
	}
}




/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
