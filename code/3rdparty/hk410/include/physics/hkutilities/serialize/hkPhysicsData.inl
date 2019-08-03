/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

hkWorldCinfo* hkPhysicsData::getWorldCinfo()
{
	return m_worldCinfo;
}

void hkPhysicsData::setWorldCinfo( hkWorldCinfo* info )
{	
	if (info != HK_NULL)
	{
		info->addReference();
	}
	if (m_worldCinfo != HK_NULL)
	{
		m_worldCinfo->removeReference();
	}
	m_worldCinfo = info;
}

void hkPhysicsData::addPhysicsSystem( hkPhysicsSystem* system )
{
	HK_ASSERT2( 0x76e8b552, system != HK_NULL, "Null hkPhysicsSystem pointer passed to hkPhysicsData::addPhysicsSystem");
	HK_ASSERT2( 0x98765eee, m_systems.indexOf(system) == -1, "Trying to add a system that is already in the physics data" );

	system->addReference();
	m_systems.pushBack(system);
}

void hkPhysicsData::removePhysicsSystem( int i )
{
	m_systems[i]->removeReference();
	m_systems.removeAt(i);
}

const hkArray<hkPhysicsSystem*>& hkPhysicsData::getPhysicsSystems() const
{
	return m_systems;
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
