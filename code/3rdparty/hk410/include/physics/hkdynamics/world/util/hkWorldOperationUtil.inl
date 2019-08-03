/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkdynamics/world/hkSimulationIsland.h>

	// don't remove, this is actually used
void hkWorldOperationUtil::mergeIslandsIfNeeded( hkEntity* entityA, hkEntity* entityB )
{
	HK_ACCESS_CHECK_OBJECT( entityA->getWorld(), HK_ACCESS_RO );
	if ( !entityA->isFixed() && !entityB->isFixed() &&
	     entityA->getSimulationIsland() != entityB->getSimulationIsland() )
	{
		hkWorldOperationUtil::mergeIslands(entityA->getWorld(), entityA, entityB);
	}
}

void hkWorldOperationUtil::putIslandOnDirtyList( hkWorld* world, hkSimulationIsland* island )
{
	HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, island, HK_ACCESS_RW );
	HK_ASSERT(0xf0ff0064, world == island->getWorld());
	if (island->m_dirtyListIndex == HK_INVALID_OBJECT_INDEX)
	{
		island->m_dirtyListIndex = hkObjectIndex(world->m_dirtySimulationIslands.getSize());
		world->m_dirtySimulationIslands.pushBack(island);
	}
}

int hkWorldOperationUtil::estimateIslandSize( int numEntities, int numConstraints )
{
		// lets assume that every entity will get at least one constraint
	return hkMath::max2(numEntities, numConstraints);
}

bool hkWorldOperationUtil::canIslandBeSparse( hkWorld* world, int size )
{
	return size < int(world->m_minDesiredIslandSize);
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
