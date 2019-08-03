/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkDynamicsCpIdMgr::hkDynamicsCpIdMgr()
{
}

// Returns an index into m_values[] and sets m_values[id] = value.
inline int hkDynamicsCpIdMgr::newId( int value )
{
	for ( int i = m_values.getSize()-1; i >= 0 ; i-- )
	{
		if ( m_values[i] == FREE_VALUE )
		{
			m_values[i] = hkValueType(value);
			return i;
		}
	}

	m_values.pushBack( hkValueType(value) );
	return m_values.getSize()-1;
}

	// Finds the index of an value.
inline int hkDynamicsCpIdMgr::indexOf( int value ) const
{
	int i;
	for ( i = m_values.getSize()-1; i >= 0 ; i-- )
	{
		if ( m_values[i] == value )
		{
			return i;
		}
	}
	return i;
}

// Adds this->m_ids[ id ] to the freelist.
inline void hkDynamicsCpIdMgr::freeId( int id )
{
	HK_ASSERT2(0x1fd4eea3, m_values[id] != FREE_VALUE, "Too many contact points in a single collision pair. The system only handles 255 contact points or less between two objects.\
This is probably the result of creating bad collision geometries (i.e. meshes with many triangles in the same place) or having a too large collision tolerance. \
It can also result from not creating a hkBvTreeShape about your mesh shape.");

	m_values[id] = FREE_VALUE;
}

inline void hkDynamicsCpIdMgr::decrementValuesGreater( int relIndex )
{
	for ( int i = m_values.getSize()-1; i >= 0 ; i-- )
	{
		if ( m_values[i] == FREE_VALUE )
		{
			continue;
		}
		if ( m_values[i] > relIndex )
		{
			m_values[i] --;
		}
	}
}

inline void hkDynamicsCpIdMgr::getAllUsedIds( hkArray<hkContactPointId>& ids ) const 
{
	for ( int i = 0; i < m_values.getSize(); ++i )
	{
		if ( m_values[i] != FREE_VALUE )
		{
			ids.pushBack(  hkContactPointId(i)  );
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
