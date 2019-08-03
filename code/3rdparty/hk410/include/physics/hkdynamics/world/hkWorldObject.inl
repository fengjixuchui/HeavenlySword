/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


inline hkWorld* hkWorldObject::getWorld() const
{ 
	return m_world;
}

hkWorldObject::~hkWorldObject()
{
	// Do not delete this if it is still in a world.
	HK_ASSERT2(0x3933b5a9,  getWorld() == HK_NULL, "hkWorldObject is in an hkWorld and cannot be deleted.");

	if (m_collidable.getShape())
	{
		m_collidable.getShape()->removeReference();
	}
}

inline hkCollidable* hkWorldObject::getCollidableRw()
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadWrite();
#endif
	return &m_collidable;
}

inline const hkCollidable* hkWorldObject::getCollidable() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadOnly();
#endif

	return &m_collidable;
}

inline hkLinkedCollidable* hkWorldObject::getLinkedCollidable()
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadWrite();
#endif
	return &m_collidable;
}

inline hkBool hkWorldObject::isAddedToWorld() const
{
	return m_world != HK_NULL;
}

void hkWorldObject::setWorld( hkWorld* world )
{
	m_world = world;
}

inline hkPropertyValue hkWorldObject::getProperty( hkUint32 key ) const
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadOnly();
#endif
	for (int i = 0; i < m_properties.getSize(); ++i)
	{
		if (m_properties[i].m_key == key)
		{
			return m_properties[i].m_value;
		}
	}

	hkPropertyValue returnValue;
	returnValue.m_data = 0;

	return returnValue;
}

inline hkBool hkWorldObject::hasProperty( hkUint32 key ) const
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadOnly();
#endif
	for (int i = 0; i < m_properties.getSize(); ++i)
	{
		if (m_properties[i].m_key == key)
		{
			return true;
		}
	}
	return false;
}

inline void hkWorldObject::unlockPropertiesFromLoadedObject()
{
	m_properties.setLocked(false);
}

inline void hkWorldObject::clearAndDeallocateProperties()
{
	m_properties.clearAndDeallocate();
}

inline void* hkWorldObject::getUserData() const
{
	return m_userData;
}

inline void hkWorldObject::setUserData( void* data )
{
	m_userData = data;
}

inline const char* hkWorldObject::getName() const
{
	return m_name;
}

inline void hkWorldObject::setName( const char* name )
{
	m_name = name;
}


inline hkMultiThreadLock& hkWorldObject::getMultiThreadLock()
{
	return m_multithreadLock;
}

inline void hkWorldObject::markForRead( )
{
	getMultiThreadLock().markForRead();
}

inline void hkWorldObject::markForWrite( )
{
#ifdef HK_DEBUG_MULTI_THREADING
	hkWorldObject::markForWriteImpl();
#endif
}

inline void hkWorldObject::unmarkForRead( )
{
	getMultiThreadLock().unmarkForRead();
}

inline void hkWorldObject::unmarkForWrite()
{
	getMultiThreadLock().unmarkForWrite();
}



inline const hkMultiThreadLock& hkWorldObject::getMultiThreadLock() const
{
	return m_multithreadLock;
}


inline void hkWorldObject::copyProperties( const hkWorldObject* otherObj )
{
	m_properties = otherObj->m_properties;
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
