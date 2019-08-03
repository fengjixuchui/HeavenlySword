/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/hkWorld.h>

#include <hkdynamics/world/util/hkWorldOperationQueue.h>

#include <hkbase/debugutil/hkStatisticsCollector.h>

hkWorldObject::hkWorldObject( class hkFinishLoadedObjectFlag flag )
	:	m_collidable(flag),
		m_properties(flag)
{
	if( flag.m_finishing )
	{
		m_collidable.setOwner(this);
	}
}


hkWorldObject::hkWorldObject( const hkShape* shape, BroadPhaseType type )
:	m_world(HK_NULL),
	m_userData(HK_NULL),
	m_name(HK_NULL),
	m_collidable( shape, (hkMotionState*)HK_NULL, type )
{
	m_collidable.setOwner( this );

	if (shape)
	{
		shape->addReference();
	}
}

hkWorldOperation::Result hkWorldObject::setShape(hkShape* shape)
{
	HK_ASSERT2(0xad45fe22, false, "This function must be overridden in derived classes, if it's to be used.");

//	if (m_world && m_world->areCriticalOperationsLocked())
//	{
//		hkWorldOperation::SetWorldObjectShape op;
//		op.m_worldObject = this;
//		op.m_shape = shape;
//
//		m_world->queueOperation(op);
//		return hkWorldOperation::POSTPONED;
//	}
//
//	// Handle reference counting here.
//	if (getCollidable()->getShape())
//	{
//		getCollidable()->getShape()->removeReference();
//	}
//	getCollidable()->setShape(shape);
//	shape->addReference();

	return hkWorldOperation::DONE;
}


void hkWorldObject::addProperty( hkUint32 key, hkPropertyValue value)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );

	for (int i = 0; i < m_properties.getSize(); ++i)
	{
		if (m_properties[i].m_key == key)
		{
			HK_ASSERT2(0x26ca3b52, 0, "You are trying to add a property to a world object, where a property of that type already exists");
			return;
		}
	}
	hkProperty& p = m_properties.expandOne();
	p.m_value = value;
	p.m_key = key;
}

hkPropertyValue hkWorldObject::removeProperty(hkUint32 key)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );

	for (int i = 0; i < m_properties.getSize(); ++i)
	{
		if (m_properties[i].m_key == key)
		{
			hkProperty found = m_properties[i];
			m_properties.removeAtAndCopy(i);
			return found.m_value;
		}
	}

	HK_ASSERT2(0x62ee448b, 0, "You are trying to remove a property from a world object, where a property of that type does not exist");

	hkPropertyValue returnValue;
	returnValue.m_data = 0;

	return returnValue;
}

hkPropertyValue hkWorldObject::editProperty( hkUint32 key, hkPropertyValue value )
{
#ifdef HK_DEBUG_MULTI_THREADING
	if ( m_world && m_world->m_propertyMasterLock )
	{
		hkCriticalSectionLock lock( m_world->m_propertyMasterLock );
		for ( int i = 0; i < m_world->m_propertyLocks.getSize(); i++)
		{
			hkWorld::PropertyLock* plock = &m_world->m_propertyLocks[i]; 
			if ( plock->m_key == key )
			{
				HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
				HK_ACCESS_CHECK_OBJECT( plock, HK_ACCESS_RW );
				goto SKIP_RW_CHECK;
			}
		}
		HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
SKIP_RW_CHECK: ;
	}
#endif

	for (int i = 0; i < m_properties.getSize(); ++i)
	{
		if (m_properties[i].m_key == key)
		{
			const hkPropertyValue oldValue = m_properties[i].m_value;
			m_properties[i].m_value = value;
			return oldValue;
		}
	}

	HK_ASSERT2(0x6c6f226b, 0, "You are trying to update a property of a world object, where a property of that type does not exist");

	hkPropertyValue returnValue;
	returnValue.m_data = 0;

	return returnValue;
}

void hkWorldObject::lockProperty( hkUint32 key )
{
	if ( !m_world || !m_world->m_propertyMasterLock)
	{
		return;
	}
	hkCriticalSectionLock lock( m_world->m_propertyMasterLock );

	hkWorld::PropertyLock* plock = HK_NULL;
	for ( int i = 0; i < m_world->m_propertyLocks.getSize(); i++)
	{
		if ( m_world->m_propertyLocks[i].m_key == key )
		{
			plock = &m_world->m_propertyLocks[i];
		}
	}

	if ( !plock )
	{
		plock = m_world->m_propertyLocks.expandBy(1);
		plock->m_multiThreadLock.init();
		plock->m_key = key;
		plock->m_lock = new hkCriticalSection( 4000 );
	}
	plock->m_lock->enter();
	plock->m_multiThreadLock.markForWrite();
}

/// unlocks a given locked property
void hkWorldObject::unlockProperty( hkUint32 key )
{
	if ( !m_world || !m_world->m_propertyMasterLock)
	{
		return;
	}
	hkCriticalSectionLock lock( m_world->m_propertyMasterLock );

	for ( int i = 0; i < m_world->m_propertyLocks.getSize(); i++)
	{
		hkWorld::PropertyLock* plock = &m_world->m_propertyLocks[i]; 
		if ( plock->m_key == key )
		{
			plock->m_multiThreadLock.unmarkForWrite();
			plock->m_lock->leave();
			return;
		}
	}
	HK_ASSERT2( 0xf0234334, false, "lockProperty missing for this property" );
}


void hkWorldObject::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->addChildObject("Shape", collector->MEMORY_SHARED, m_collidable.getShape());
	collector->addArray("CollAgtPtr", collector->MEMORY_RUNTIME, m_collidable.m_collisionEntries);
	collector->addArray("Properties", collector->MEMORY_ENGINE, m_properties);
}


void hkWorldObject::addReference()
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	hkReferencedObject::addReference();
}

void hkWorldObject::removeReference()
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	hkReferencedObject::removeReference();
}

void hkWorldObject::addReferenceAsCriticalOperation()
{
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::AddReference op;
		op.m_worldObject = this;
		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	hkReferencedObject::addReference();
}

void hkWorldObject::removeReferenceAsCriticalOperation()
{
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::RemoveReference op;
		op.m_worldObject = this;
		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	hkReferencedObject::removeReference();
}

void hkWorldObject::markForWriteImpl( )
{
#ifdef HK_DEBUG_MULTI_THREADING
	if ( m_world )
	{
		HK_ASSERT2( 0xf0213de, !m_world->m_multiThreadLock.isMarkedForReadRecursive(), "You cannot mark an entity read write, if it is already marked as read only by the hkWorld::markForRead(RECURSIVE)" );
	}
	getMultiThreadLock().markForWrite();
#endif
}

void hkWorldObject::checkReadWrite()
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW);
}

void hkWorldObject::checkReadOnly() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO);
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
