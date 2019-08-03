/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkMultiThreadLock& hkBroadPhase::getMultiThreadLock()
{
	return m_multiThreadLock;
}

inline const hkMultiThreadLock& hkBroadPhase::getMultiThreadLock() const
{
	// Have to cast away const to add the property if necessary
	hkBroadPhase* nonConstThis = const_cast<hkBroadPhase*>(this);
	return nonConstThis->getMultiThreadLock();
}

inline void hkBroadPhase::markForRead( ) const
{
	m_multiThreadLock.markForRead();
}

inline void hkBroadPhase::markForWrite( )
{
	m_multiThreadLock.markForWrite();
}

inline void hkBroadPhase::unmarkForRead( ) const
{
	m_multiThreadLock.unmarkForRead();
}

inline void hkBroadPhase::unmarkForWrite()
{
	m_multiThreadLock.unmarkForWrite();
}

inline void hkBroadPhase::lock()
{
	if ( m_criticalSection )
	{
		lockImplementation();
	}
	else
	{
		markForWrite();
	}
}

inline void hkBroadPhase::unlock()
{
	if ( m_criticalSection )
	{
		unlockImplementation();
	}
	else
	{
		unmarkForWrite();
	}
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
