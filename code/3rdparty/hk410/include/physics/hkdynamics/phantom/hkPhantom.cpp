/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>

#include <hkmath/basetypes/hkAabb.h>

#include <hkinternal/collide/broadphase/hkBroadPhase.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseDispatcher.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>

#include <hkdynamics/phantom/hkPhantom.h>
#include <hkdynamics/phantom/hkPhantomListener.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/simulation/hkSimulation.h>

// TODO . this include is temporary: until hkPhantoms;:updateBroadPhase is moved to hkWorldOperationUtil
#include <hkdynamics/world/util/hkWorldOperationQueue.h>

#include <hkcollide/filter/hkCollisionFilter.h>

template<class T>
static inline void HK_CALL cleanupNullPointers( hkArray<T*>& cleanupArray )
{
	for (int i = cleanupArray.getSize() - 1; i >= 0; i-- )
	{
		if ( cleanupArray[i] == HK_NULL )
		{
			cleanupArray.removeAtAndCopy(i);
		}
	}
}

void hkPhantom::firePhantomDeleted( )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );

	for ( int i = m_phantomListeners.getSize()-1; i >= 0; i-- )
	{
		if (m_phantomListeners[i] != HK_NULL)
		{
			m_phantomListeners[i]->phantomDeletedCallback( this );
		}
	}
	//cleanupNullPointers<hkPhantomListener>( m_phantomListeners ); // not necessary, as object is deleted
}


void hkPhantom::firePhantomRemoved( )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );

	for ( int i = m_phantomListeners.getSize()-1; i >= 0; i-- )
	{
		if (m_phantomListeners[i] != HK_NULL)
		{
			m_phantomListeners[i]->phantomRemovedCallback( this );
		}
	}
	cleanupNullPointers<hkPhantomListener>( m_phantomListeners );
}


void hkPhantom::firePhantomAdded( )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );

	for ( int i = m_phantomListeners.getSize()-1; i >= 0; i-- )
	{
		if (m_phantomListeners[i] != HK_NULL)
		{
			m_phantomListeners[i]->phantomAddedCallback( this );
		}
	}
	cleanupNullPointers<hkPhantomListener>( m_phantomListeners );
}

void hkPhantom::firePhantomShapeSet( )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );

	for ( int i = m_phantomListeners.getSize()-1; i >= 0; i-- )
	{
		if (m_phantomListeners[i] != HK_NULL)
		{
			m_phantomListeners[i]->phantomShapeSetCallback( this );
		}
	}
	cleanupNullPointers<hkPhantomListener>( m_phantomListeners );
}

void hkPhantom::updateBroadPhase( const hkAabb& aabb )
{
	if ( m_world != HK_NULL )
	{
		// Check if the world is locked, if so postpone the operation
		if (m_world->areCriticalOperationsLockedForPhantoms())
		{
			hkWorldOperation::UpdatePhantomBP op;
			op.m_phantom = this;
			op.m_aabb = const_cast<hkAabb*>(&aabb);
			m_world->queueOperation(op);
			return;
		}

		// Perform the actual operation
		HK_ACCESS_CHECK_OBJECT( m_world, HK_ACCESS_RW );

		m_world->lockCriticalOperations();

		hkLocalArray<hkBroadPhaseHandlePair> newPairs( m_world->m_broadPhaseUpdateSize );
		hkLocalArray<hkBroadPhaseHandlePair> delPairs( m_world->m_broadPhaseUpdateSize );

		hkBroadPhaseHandle* thisObj = m_collidable.getBroadPhaseHandle();

		m_world->getBroadPhase()->lock();

		m_world->getBroadPhase()->updateAabbs( &thisObj, &aabb, 1, newPairs, delPairs );

		// check for changes
		if ( newPairs.getSize() != 0 || delPairs.getSize() != 0)
		{
			hkTypedBroadPhaseDispatcher::removeDuplicates( newPairs, delPairs );

			m_world->m_broadPhaseDispatcher->removePairs(static_cast<hkTypedBroadPhaseHandlePair*>(delPairs.begin()), delPairs.getSize());
			m_world->m_broadPhaseDispatcher->addPairs( static_cast<hkTypedBroadPhaseHandlePair*>(newPairs.begin()), newPairs.getSize(),  m_world->getCollisionFilter() );

			cleanupNullPointers<hkPhantomOverlapListener>( m_overlapListeners );
		}

		m_world->getBroadPhase()->unlock();

		m_world->unlockAndAttemptToExecutePendingOperations();
	}
	else
	{
		//HK_WARN_ONCE(0x3a15c993,  "Updating the aabb of a phantom that has not been added to a hkWorld");
	}
}
void hkPhantom::addPhantomListener( hkPhantomListener* el)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	HK_ASSERT2(0x782c270a,  m_phantomListeners.indexOf( el ) < 0, "You cannot add a listener twice to a phantom" );
	m_phantomListeners.pushBack( el );
}

void hkPhantom::removePhantomListener( hkPhantomListener* el)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	int i = m_phantomListeners.indexOf( el );
	HK_ASSERT2(0x5b2f9aa5,  i>=0, "Tried to remove a listener which was never added");
	m_phantomListeners.removeAtAndCopy( i );
}


void hkPhantom::addPhantomOverlapListener( hkPhantomOverlapListener* el)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	HK_ASSERT2(0x5d027d23,  m_overlapListeners.indexOf( el ) < 0, "You cannot add a listener twice to a phantom" );
	m_overlapListeners.pushBack( el );
}

void hkPhantom::removePhantomOverlapListener( hkPhantomOverlapListener* el)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	int i = m_overlapListeners.indexOf( el );
	HK_ASSERT2(0x5478016a,  i>=0, "Tried to remove a listener which was never added");
	m_overlapListeners.removeAtAndCopy( i );
}


hkPhantom::~hkPhantom()
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	firePhantomDeleted();
}

void hkPhantom::calcStatistics( hkStatisticsCollector* collector ) const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	hkWorldObject::calcStatistics(collector);
	collector->addArray("OvrlapLisPtr", collector->MEMORY_ENGINE, m_overlapListeners);
	collector->addArray("PhantmLisPtr", collector->MEMORY_ENGINE, m_phantomListeners);
}

void hkPhantom::deallocateInternalArrays()
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );

	// Need to deallocate any arrays in the phantom that are 0 size
	// else warn user that they should call the in place destructor

	// Overlap Listeners
	if (m_overlapListeners.getSize() == 0)
	{
		m_overlapListeners.clearAndDeallocate();
	}
	else
	{
		HK_WARN(0x234f224e, "Phantom at address " << this << " has non-zero m_overlapListeners array.\nPlease call in-place destructor to deallocate.\n");
	}

	// Phantom Listeners
	if (m_phantomListeners.getSize() == 0)
	{
		m_phantomListeners.clearAndDeallocate();
	}
	else
	{
		HK_WARN(0x234f224f, "Phantom at address " << this << " has non-zero m_phantomListeners array.\nPlease call in-place destructor to deallocate.\n");
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
