/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


hkWorld* hkSimulationIsland::getWorld()
{
	return m_world;
}



inline hkBool hkSimulationIsland::isActive() const
{
	return m_active;
}

inline hkBool hkSimulationIsland::isFixed() const
{
	return m_storageIndex == HK_INVALID_OBJECT_INDEX;
}



inline const hkArray<hkEntity*>& hkSimulationIsland::getEntities() const 
{ 
	return m_entities; 
}

inline const hkArray<hkAction*>& hkSimulationIsland::getActions() const 
{ 
	return m_actions; 
}


int hkSimulationIsland::getStorageIndex()
{
	return m_storageIndex;
}


inline int hkSimulationIsland::getMemUsageForIntegration()
{
	int sizeForElemTemp = m_constraintInfo.m_numSolverResults * hkSizeOf(hkSolverElemTemp) + 2 * hkSizeOf(hkSolverElemTemp);
	int sizeForAccumulators = getEntities().getSize() * hkSizeOf(hkVelocityAccumulator) + hkSizeOf(hkVelocityAccumulator) + 16; // fixed rigid body + end tag (16 byte aligned)

	return	sizeForAccumulators + 
			m_constraintInfo.m_sizeOfJacobians +
			sizeForElemTemp + 
			m_constraintInfo.m_sizeOfSchemas + 
			HK_SIZE_OF_JACOBIAN_END_SCHEMA;
}


inline void hkSimulationIsland::markAllEntitiesReadOnly() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	for (int i =0; i < m_entities.getSize(); i++)
	{
		m_entities[i]->markForRead();
	}
#endif
}

// helper functions for debugging multithreading
inline void hkSimulationIsland::unmarkAllEntitiesReadOnly() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	for (int i =0; i < m_entities.getSize(); i++)
	{
		m_entities[i]->unmarkForRead();
	}
#endif
}

inline void hkSimulationIsland::unmarkForWrite()
{
	m_multiThreadLock.unmarkForWrite();
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
