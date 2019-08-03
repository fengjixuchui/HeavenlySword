/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/hkWorld.h>

#include <hkbase/thread/hkSpuDmaManager.h>
#include <hkdynamics/constraint/setup/hkConstraintSolverSetup.h>
#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobs.h>
#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobQueueUtils.h>
#include <hkdynamics/world/hkSimulationIsland.h>


hkJobQueue::JobPopFuncResult hkBroadPhaseJob::popJobTask( hkArray<hkSimulationIsland*>& islands, hkBroadPhaseJob& out )
{
	out = *this;
	out.m_island = islands[m_islandIndex];

	hkBuildJacobianTaskHeader* taskHeader = this->m_taskHeader;

	// check for pending split simulation island jobs
	if ( taskHeader && taskHeader->m_newSplitIslands.getSize() )
	{
		int numNewIslands = taskHeader->m_newSplitIslands.getSize();
		taskHeader->m_referenceCount += numNewIslands;

		// give the new islands to the job on the queue
		this->m_islandIndex = hkObjectIndex(islands.getSize());
		this->m_numIslands  = hkObjectIndex(numNewIslands);

		hkArray<hkSimulationIsland*>& newIslands = taskHeader->m_newSplitIslands;
		hkArray<hkSimulationIsland*>& activeIslands = out.m_island->getWorld()->m_activeSimulationIslands;

		// still need to add the new islands
		for (int i =0 ; i <  numNewIslands; i++)
		{
			hkSimulationIsland* island = newIslands[i];
			island->m_storageIndex = hkObjectIndex(activeIslands.getSize());
			activeIslands.pushBack(island);
		}
		newIslands.clearAndDeallocate();
		hkDeallocateChunk( (void**)out.m_taskHeader->m_allEntities, taskHeader->m_entitiesCapacity, HK_MEMORY_CLASS_BASE );
		taskHeader->m_allEntities = HK_NULL;

		return hkJobQueue::DO_NOT_POP_QUEUE_ENTRY;
	}

	// if possible split the job into two parts
	if ( m_numIslands > 1 )
	{
		m_numIslands    -= 1;
		m_islandIndex   += 1;
		out.m_numIslands = 1;
		return hkJobQueue::DO_NOT_POP_QUEUE_ENTRY;
	}

	return hkJobQueue::POP_QUEUE_ENTRY;
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
