/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


	//
	//	********** Dynamics Job ***************
	//
hkDynamicsJob::hkDynamicsJob( hkDynamicsJobType type ): m_jobType(type), m_island(HK_NULL)
{

}

hkJobQueue::JobPopFuncResult hkDynamicsJob::popDynamicsJobTask( const hkArray<hkSimulationIsland*>& islands, hkDynamicsJob& out )
{
	reinterpret_cast<hkJobQueue::JobQueueEntry&>(out) = reinterpret_cast<hkJobQueue::JobQueueEntry&>(*this);

	if ( !HK_PLATFORM_IS_SPU )
	{
		out.m_island = islands[m_islandIndex];
	}
	return hkJobQueue::POP_QUEUE_ENTRY;
}


//
//	********** Dynamics Job ***************
//
hkIntegrateJob::hkIntegrateJob(int numIslands) : hkDynamicsJob(DYNAMICS_JOB_INTEGRATE)
{
	m_islandIndex = 0;
	m_numIslands  = numIslands;
}

hkJobQueue::JobPopFuncResult hkIntegrateJob::popJobTask( const hkArray<hkSimulationIsland*>& islands, hkIntegrateJob& out )
{
	out = *this;
	if ( !HK_PLATFORM_IS_SPU )
	{
		out.m_island = islands[m_islandIndex];
	}

	// if possible split the job into two parts
	if ( m_numIslands > 1 )
	{
		m_islandIndex++;
		m_numIslands--;
		out.m_numIslands = 1;
		return hkJobQueue::DO_NOT_POP_QUEUE_ENTRY;
	}
	return hkJobQueue::POP_QUEUE_ENTRY;
}


hkJobQueue::JobPopFuncResult hkBuildAccumulatorsJob::popJobTask( const hkArray<hkSimulationIsland*>& islands, hkBuildAccumulatorsJob& out )
{
	out = *this;
	if ( !HK_PLATFORM_IS_SPU )
	{
		out.m_island = islands[m_islandIndex];
	}

	// if possible split the job into two parts
	if ( m_numEntities > hkBuildAccumulatorsJob::ACCUMULATORS_PER_JOB )
	{
		m_numEntities    -= hkBuildAccumulatorsJob::ACCUMULATORS_PER_JOB;
		m_firstEntityIdx += hkBuildAccumulatorsJob::ACCUMULATORS_PER_JOB;
		out.m_numEntities = hkBuildAccumulatorsJob::ACCUMULATORS_PER_JOB;
		return hkJobQueue::DO_NOT_POP_QUEUE_ENTRY;
	}
	return hkJobQueue::POP_QUEUE_ENTRY;
}

hkBuildJacobiansTaskJob::hkBuildJacobiansTaskJob(const hkIntegrateJob& job, const struct hkBuildJacobianTaskHeader* taskHeader)
	: hkBuildAccumulatorsJob( taskHeader, DYNAMICS_JOB_BUILD_JACOBIANS_TASK)
{
	m_islandIndex = job.m_islandIndex;
}


hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob::hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob(const hkBuildAccumulatorsJob& baj)
		: hkDynamicsJob( DYNAMICS_JOB_FIRE_JACOBIAN_SETUP_CALLBACK_AND_BUILD_PPU_JACOBIANS)
{
	m_islandIndex = baj.m_islandIndex;
	m_taskHeader  = const_cast<HK_CPU_PTR(hkBuildJacobianTaskHeader*)>(baj.m_taskHeader);
}


hkSplitSimulationIslandJob::hkSplitSimulationIslandJob(const hkSolveConstraintsJob& job): hkDynamicsJob( DYNAMICS_JOB_SPLIT_ISLAND)
{
	m_islandIndex = job.m_islandIndex;
	m_taskHeader  = job.m_taskHeader;
}

hkSplitSimulationIslandJob::hkSplitSimulationIslandJob(hkDynamicsJobType type, int islandIndex, hkBuildJacobianTaskHeader* taskHeader): hkDynamicsJob( type )
{
	m_islandIndex = hkObjectIndex(islandIndex);
	m_taskHeader  = taskHeader;
}

hkJobQueue::JobPopFuncResult hkSplitSimulationIslandJob::popJobTask( const hkArray<hkSimulationIsland*>& islands, hkSplitSimulationIslandJob& out )
{
	out = *this;
	out.m_island = islands[m_islandIndex];
	return hkJobQueue::POP_QUEUE_ENTRY;
}

hkIntegrateMotionJob::hkIntegrateMotionJob( const hkSolveConstraintsJob& job, HK_CPU_PTR(hkBuildJacobianTaskHeader*) taskHeader, int firstEntityIdx, int numEntities, HK_CPU_PTR(void*) solverBuffer )
: hkSplitSimulationIslandJob( DYNAMICS_JOB_INTEGRATE_MOTION, job.m_islandIndex, taskHeader )
{
	m_firstEntityIdx	= hkObjectIndex(firstEntityIdx);
	m_numEntities		= hkObjectIndex(numEntities);
	m_buffer			= solverBuffer;
}

hkJobQueue::JobPopFuncResult hkIntegrateMotionJob::popJobTask( const hkArray<hkSimulationIsland*>& islands, hkIntegrateMotionJob& out )
{
	out = *this;
	if ( !HK_PLATFORM_IS_SPU )
	{
		out.m_island = islands[m_islandIndex];
	}

	// if possible split the job into two parts
	if ( m_numEntities > hkIntegrateMotionJob::ACCUMULATORS_PER_JOB )
	{
		m_numEntities    -= hkIntegrateMotionJob::ACCUMULATORS_PER_JOB;
		m_firstEntityIdx += hkIntegrateMotionJob::ACCUMULATORS_PER_JOB;
		out.m_numEntities = hkIntegrateMotionJob::ACCUMULATORS_PER_JOB;
		return hkJobQueue::DO_NOT_POP_QUEUE_ENTRY;
	}

	return hkJobQueue::POP_QUEUE_ENTRY;
}


hkBroadPhaseJob::hkBroadPhaseJob( const hkIntegrateJob& job, hkBuildJacobianTaskHeader* taskHeader ) : hkDynamicsJob( DYNAMICS_JOB_BROADPHASE)
{
	m_islandIndex = job.m_islandIndex;
	m_numIslands  = 1;
	m_taskHeader  = taskHeader;
}

hkBroadPhaseJob::hkBroadPhaseJob( const hkSplitSimulationIslandJob& job ): hkDynamicsJob( DYNAMICS_JOB_BROADPHASE)
{
	m_islandIndex	= job.m_islandIndex;
	m_numIslands    = 1;
	m_taskHeader    = job.m_taskHeader;
}

hkAgentSectorJob::hkAgentSectorJob(const hkBroadPhaseJob& job, const hkStepInfo& stepInfo, int numSectors) : hkDynamicsJob( DYNAMICS_JOB_AGENT_SECTOR)
{
	m_islandIndex	= job.m_islandIndex;
	m_taskIndex		= 0;
	m_numSectors	= numSectors;
	m_stepInfo		= stepInfo;
	m_header		= 0;
}


inline hkJobQueue::JobPopFuncResult hkAgentSectorJob::popJobTask( const hkArray<hkSimulationIsland*>& islands, hkAgentSectorJob& out )
{
	out = *this;
	if ( !HK_PLATFORM_IS_SPU )
	{
		out.m_island = islands[m_islandIndex];
	}

	// if possible split the job into two parts
	if ( m_numSectors > hkAgentSectorJob::SECTORS_PER_JOB )
	{
		m_numSectors    -= hkAgentSectorJob::SECTORS_PER_JOB;
		m_taskIndex   += 1;
		out.m_numSectors = hkAgentSectorJob::SECTORS_PER_JOB;
		return hkJobQueue::DO_NOT_POP_QUEUE_ENTRY;
	}
	return hkJobQueue::POP_QUEUE_ENTRY;
}

hkBuildJacobiansJob::hkBuildJacobiansJob(const hkBuildAccumulatorsJob& baj, hkBuildJacobianTask*taskInMainMemory, hkConstraintQueryIn* constraintQueryInInMainMemory)
			: hkDynamicsJob( DYNAMICS_JOB_BUILD_JACOBIANS) 
{
	m_islandIndex			        = baj.m_islandIndex;
	m_buildJacobianTaskInMainMemory = taskInMainMemory;
	m_constraintQueryIn				= constraintQueryInInMainMemory;
}

hkBuildJacobiansJob::hkBuildJacobiansJob(const hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob& fjscb ): hkDynamicsJob( DYNAMICS_JOB_BUILD_JACOBIANS) 
{
	m_islandIndex                   = fjscb.m_islandIndex;
	m_buildJacobianTaskInMainMemory = fjscb.m_taskHeader->m_tasks.m_buildJacobianTasks;
	m_constraintQueryIn             = fjscb.m_taskHeader->m_constraintQueryIn;
}

hkSolveConstraintsJob::hkSolveConstraintsJob(const hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob& fjscb ): hkDynamicsJob( DYNAMICS_JOB_SOLVE_CONSTRAINTS ) 
{
	m_islandIndex         = fjscb.m_islandIndex;
	m_buffer              = fjscb.m_taskHeader->m_buffer;

	// offsets should be calculated earlier.  before buildJacobians
	m_accumulatorsOffset  = hkGetByteOffsetCpuPtr(m_buffer, fjscb.m_taskHeader->m_accumulatorsBase);
	m_jacobiansOffset     = hkGetByteOffsetCpuPtr(m_buffer, fjscb.m_taskHeader->m_jacobiansBase);
	m_schemasOffset       = hkGetByteOffsetCpuPtr(m_buffer, fjscb.m_taskHeader->m_schemasBase);
	m_solverTempOffset    = hkGetByteOffsetCpuPtr(m_buffer, fjscb.m_taskHeader->m_solverTempBase);
	m_bufferSize          = fjscb.m_taskHeader->m_bufferSize;

	m_taskHeader          = fjscb.m_taskHeader;
	m_numSolverResults = fjscb.m_taskHeader->m_numSolverResults;
}

hkSolveConstraintsJob::hkSolveConstraintsJob(const hkBuildJacobiansJob& bjj, const hkBuildJacobianTaskHeader& taskHeader, hkBuildJacobianTaskHeader* taskHeaderInMainMemory )
		: hkDynamicsJob( DYNAMICS_JOB_SOLVE_CONSTRAINTS ) 
{
	m_islandIndex         = bjj.m_islandIndex;
	m_buffer              = taskHeader.m_buffer;

	// offsets should be calculated earlier.  before buildJacobians
	m_accumulatorsOffset  = hkGetByteOffsetCpuPtr(m_buffer, taskHeader.m_accumulatorsBase);
	m_jacobiansOffset     = hkGetByteOffsetCpuPtr(m_buffer, taskHeader.m_jacobiansBase);
	m_schemasOffset       = hkGetByteOffsetCpuPtr(m_buffer, taskHeader.m_schemasBase);
	m_solverTempOffset    = hkGetByteOffsetCpuPtr(m_buffer, taskHeader.m_solverTempBase);
	m_bufferSize          = taskHeader.m_bufferSize;

	m_taskHeader          = taskHeaderInMainMemory;
	m_numSolverResults    = taskHeader.m_numSolverResults;
}

hkJobQueue::JobPopFuncResult hkBuildJacobiansJob::popJobTask( const hkArray<hkSimulationIsland*>& islands, hkBuildJacobiansJob& out )
{
	out = *this;
	if ( !HK_PLATFORM_IS_SPU )
	{
		out.m_island = islands[m_islandIndex];
	}
#if defined (HK_PLATFORM_POTENTIAL_SPU)
	// dma in the _OLD_ hkBuildJacobianTask into local memory & assign its reference to job
	// Note: this task memory is deallocated in the finishDynamicsJob()
	if (HK_PLATFORM_IS_SPU)
	{
		struct hkBuildJacobianTask* task = reinterpret_cast<struct hkBuildJacobianTask*>(hkSpuStack::getInstance().allocateStack(sizeof(hkBuildJacobianTask),"hkBuildJacobianTask"));
		hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( task, m_buildJacobianTaskInMainMemory, sizeof(hkBuildJacobianTask), hkSpuDmaManager::READ_ONLY);
		out.m_buildJacobianTask = task;

		HK_ASSERT(0xaf34ef22, task->m_numAtomInfos > 0);

		// if possible split the job into two parts
		if ( task->m_next )
		{
			// the job left on the queue now points to the next task on the list
			m_buildJacobianTaskInMainMemory = task->m_next;
			return hkJobQueue::DO_NOT_POP_QUEUE_ENTRY;
		}
	}
	else
#endif
	{
		// this code must exist on all platforms which can execute this task without dma
		// if possible split the job into two parts
		if ( m_buildJacobianTask->m_next  )
		{
			m_buildJacobianTask = m_buildJacobianTask->m_next;
			return hkJobQueue::DO_NOT_POP_QUEUE_ENTRY;
		}
	}
	return hkJobQueue::POP_QUEUE_ENTRY;
}

hkPostCollideJob::hkPostCollideJob( const hkAgentSectorJob& asj ): hkDynamicsJob( DYNAMICS_JOB_POST_COLLIDE )
{
	const hkDynamicsJob& dj = asj;

	m_header       = asj.m_header;
	m_islandIndex  = dj.m_islandIndex;
	m_numTotalTasks = asj.m_numTotalTasks;
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
