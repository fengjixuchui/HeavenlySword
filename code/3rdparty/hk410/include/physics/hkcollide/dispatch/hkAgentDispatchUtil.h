/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_COLLIDE2_AGENT_DISPATCH_UTIL_H
#define HK_COLLIDE2_AGENT_DISPATCH_UTIL_H

#include <hkbase/memory/hkLocalArray.h>
#include <hkcollide/agent/bvtree/hkBvTreeAgent.h>
#include <hkcollide/agent/hkShapeCollectionFilter.h>
#include <hkcollide/agent/null/hkNullAgent.h>

	// note keys needs to be sorted
template<typename KEY, typename ENTRY, class HELPER>
class hkAgentDispatchUtil
{
	public:
			// update the lists 
		static HK_FORCE_INLINE void HK_CALL update( hkArray<ENTRY>& entries, hkArray<KEY>& keys,
													const hkCdBody& cA, const hkCdBody& cB,
													const hkCollisionInput& input, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner, HELPER& base );

			// Same as update, but optimized for small lists. In only requires that the input always has the same sort order, but
			// does not require sorted input
		static HK_FORCE_INLINE void HK_CALL fastUpdate( hkArray<ENTRY>& entries, hkArray<KEY>& keys,
													const hkCdBody& cA, const hkCdBody& cB,
													const hkCollisionInput& input, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner, HELPER& base );
};




	// requirements:
	// entries are sorted
	// keys are sorted
	// ENTRY needs:								\n
	// class ENTRY {								\n
	//		KEY& getKey()							\n
	//		hkCollisionAgent*	m_collisionAgent;	\n
	//	}											\n
	//
	// the < and == operators are defined for KEY		<br>
	// BASE has to implement inline hkCdBody& getBodyA( hkCollidable& cIn, hkCollisionInput& input, const KEY& key );
	// BASE has to implement inline hkCdBody& getBodyB( hkCollidable& cIn, hkCollisionInput& input, const KEY& key );
	// BASE has to implement inline hkShapeCollection* getShapeCollectionB( );
	//

template<typename KEY, typename ENTRY, class HELPER>
void HK_CALL hkAgentDispatchUtil<KEY,ENTRY,HELPER>::update( hkArray<ENTRY>& entries, hkArray<KEY>& keys,
														   const hkCdBody& cA, const hkCdBody& cB, 
														   const hkCollisionInput& input, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner, HELPER& base )
{
	typename hkArray<ENTRY>::iterator oldEntriesItr = entries.begin();
	typename hkArray<ENTRY>::iterator oldEntriesEnd = entries.end();

	typename hkArray<KEY>::iterator newKeysItr = keys.begin();
	typename hkArray<KEY>::iterator newKeysEnd = keys.end();

	hkLocalArray<ENTRY> newEntries( keys.getSize() );
	newEntries.setSize( keys.getSize() );

	const hkShapeContainer* shapeContainer = base.getShapeCollectionB()->getContainer();

	typename hkArray<ENTRY>::iterator newEntriesItr = newEntries.begin();

	while ( (oldEntriesItr != oldEntriesEnd) && (newKeysItr != newKeysEnd) )
	{
		if (  (*newKeysItr) == (*oldEntriesItr).getKey() )
		{
			// keep element
			*newEntriesItr = *oldEntriesItr;

			newEntriesItr++;
			oldEntriesItr++;
			newKeysItr++;
			continue;
		}

		if (  (*newKeysItr) < (*oldEntriesItr).getKey() )
		{
			// new element
			const hkCdBody& bodyA = cA;
			const hkCdBody& bodyB = cB;
			const hkCdBody& modifiedB = *base.getBodyB( cB, input, *newKeysItr );

			if ( input.m_filter->isCollisionEnabled( input, bodyA, bodyB, *shapeContainer , *newKeysItr ) )
			{
				newEntriesItr->m_collisionAgent = input.m_dispatcher->getNewCollisionAgent( bodyA, modifiedB, input, mgr );
			}
			else
			{
				newEntriesItr->m_collisionAgent = hkNullAgent::getNullAgent();
			}
			newEntriesItr->setKey( *newKeysItr );

			newEntriesItr++;
			newKeysItr++;
			continue;
		}

		{
			// delete element
			if (oldEntriesItr->m_collisionAgent != HK_NULL)
			{
				oldEntriesItr->m_collisionAgent->cleanup( constraintOwner );
			}

			oldEntriesItr++;
		}
	}

	// now, one of the lists is empty
	// check for elements to delete
	while ( oldEntriesItr != oldEntriesEnd )
	{
		if (oldEntriesItr->m_collisionAgent != HK_NULL)
		{
			oldEntriesItr->m_collisionAgent->cleanup( constraintOwner );
		}
		oldEntriesItr++;
	}

	// append the rest
	while ( newKeysItr != newKeysEnd ) 
	{
		const hkCdBody& bodyA = cA;
		const hkCdBody& bodyB = cB;
		const hkCdBody& modifiedB = *base.getBodyB( cB, input, *newKeysItr );

		if ( input.m_filter->isCollisionEnabled( input, bodyA, bodyB, *shapeContainer , *newKeysItr ) )
		{
			newEntriesItr->m_collisionAgent = input.m_dispatcher->getNewCollisionAgent( bodyA, modifiedB, input, mgr );
		}
		else
		{
			newEntriesItr->m_collisionAgent = hkNullAgent::getNullAgent();
		}
		newEntriesItr->setKey( *newKeysItr );

		newEntriesItr++;
		newKeysItr++;
	}


	// copy the results
	entries = newEntries;
}

template<typename KEY, typename ENTRY, class HELPER>
void HK_CALL hkAgentDispatchUtil<KEY,ENTRY,HELPER>::fastUpdate( hkArray<ENTRY>& entries, hkArray<KEY>& hitList,
														   const hkCdBody& cA, const hkCdBody& cB, 
														   const hkCollisionInput& input, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner, HELPER& base )
{

		// go through existing list and check whether we can find each item in the hitlist
		{
			ENTRY* itr = entries.begin();
			ENTRY* end = entries.end();
			hkShapeKey* lastHit = hitList.begin();
			hkShapeKey* endHitList = hitList.end();

			for ( ;itr != end; itr++ )
			{
				if ( lastHit != endHitList &&  itr->getKey() == *lastHit )
				{
					lastHit++;
					continue;
				}
				// Search the entire array
				{
					for (lastHit = hitList.begin(); lastHit!= endHitList; lastHit++ )
					{
						if ( itr->getKey() == *lastHit )
						{
							lastHit++;
							goto hitFound;
						}
					}

				}
				// not found: remove
				itr->m_collisionAgent->cleanup( constraintOwner );
				entries.removeAtAndCopy( static_cast<int>( (hkUlong)(itr - entries.begin())) );
				itr--;
				end--;
hitFound:;
			}

		}
		//
		// Go through the hitlist and check whether we already have that agent
		//

		const hkShapeContainer* shapeContainer = base.getShapeCollectionB()->getContainer();
		{
			if ( hitList.getSize() != entries.getSize() )
			{
				ENTRY* lastHit = entries.begin();
				ENTRY* end = entries.end();
				hkShapeKey* hitItr = hitList.begin();
				hkShapeKey* endHitList = hitList.end();

				for ( ;hitItr != endHitList; hitItr++ )
				{
					if ( lastHit != end &&  lastHit->getKey() == *hitItr )
					{
						lastHit++;
						continue;
					}
					// new found: insert
					int index = static_cast<int>( (hkUlong)(hitItr - hitList.begin()) ); // 64 bit ptr64->ulong->int32
					lastHit = entries.expandAt( index,1  );

					const hkCdBody& bodyA = cA;
					const hkCdBody& bodyB = cB;
					const hkCdBody& modifiedB = *base.getBodyB( cB, input, *hitItr );

					if ( input.m_filter->isCollisionEnabled( input, bodyA, bodyB, *shapeContainer , *hitItr ) )
					{
						lastHit->m_collisionAgent = input.m_dispatcher->getNewCollisionAgent( bodyA, modifiedB, input, mgr );
					}
					else
					{
						lastHit->m_collisionAgent = hkNullAgent::getNullAgent(  );
					}
					lastHit->m_key = *hitItr;

					end = entries.end();
					lastHit++;
				}
			}
		}
}
#endif // HK_COLLIDE2_AGENT_DISPATCH_UTIL_H

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
