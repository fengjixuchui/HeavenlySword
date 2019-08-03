/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// Helper function that returns a hkPhantom if the collidable's broadphase handle is of type hkWorldObject::BROAD_PHASE_PHANTOM
inline hkPhantom* HK_CALL hkGetPhantom(const hkCollidable* collidable)
{
	if ( collidable->getType() == hkWorldObject::BROAD_PHASE_PHANTOM )
	{
		return static_cast<hkPhantom*>( hkGetWorldObject(collidable) );
	}
	return HK_NULL;
}

hkCollidableAccept hkPhantom::fireCollidableAdded( const hkCollidable* collidable )
{
	hkCollidableAddedEvent event;
	event.m_collidable = collidable;
	event.m_phantom = this;
	event.m_collidableAccept = HK_COLLIDABLE_ACCEPT;

	for ( int i = m_overlapListeners.getSize()-1; i >= 0; i-- )
	{
		if (m_overlapListeners[i] != HK_NULL)
		{
			m_overlapListeners[i]->collidableAddedCallback( event );
		}
	}
	// cleanupNullPointers done at the end of updateBroadPhase 
	return event.m_collidableAccept;
}

void hkPhantom::fireCollidableRemoved( const hkCollidable* collidable, hkBool collidableWasAdded )
{
	hkCollidableRemovedEvent event;
	event.m_collidable = collidable;
	event.m_phantom = this;
	event.m_collidableWasAdded = collidableWasAdded;

	for ( int i = m_overlapListeners.getSize()-1; i >= 0; i-- )
	{
		if (m_overlapListeners[i] != HK_NULL)
		{
			m_overlapListeners[i]->collidableRemovedCallback( event );
		}
	}
	// cleanupNullPointers done at the end of updateBroadPhase 
}


inline hkPhantom::hkPhantom( const hkShape* shape )
	: hkWorldObject( shape, BROAD_PHASE_PHANTOM )
{
	m_collidable.setOwner( this );
}

inline const hkArray<hkPhantomListener*>& hkPhantom::getPhantomListeners() const
{
	return m_phantomListeners;
}

inline const hkArray<hkPhantomOverlapListener*>& hkPhantom::getPhantomOverlapListeners() const
{
	return m_overlapListeners;
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
