/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS_JOBS_H
#define HK_DYNAMICS_JOBS_H

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSpuUtils.h>
#include <hkmath/basetypes/hkStepInfo.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkdynamics/constraint/setup/hkConstraintSolverSetup.h>
#include <hkbase/thread/job/hkJobQueue.h>
#include <hkdynamics/world/commandqueue/hkPhysicsCommandQueue.h>
#if defined (HK_PLATFORM_POTENTIAL_SPU)
#	include <hkbase/thread/hkSpuStack.h>
#endif

enum hkDynamicsJobType
{
	DYNAMICS_JOB_INTEGRATE,
	DYNAMICS_JOB_BUILD_ACCUMULATORS,
	DYNAMICS_JOB_BUILD_JACOBIANS_TASK,
	DYNAMICS_JOB_FIRE_JACOBIAN_SETUP_CALLBACK_AND_BUILD_PPU_JACOBIANS,
	DYNAMICS_JOB_BUILD_JACOBIANS,
	DYNAMICS_JOB_SPLIT_ISLAND,
	DYNAMICS_JOB_SOLVE_CONSTRAINTS,
	DYNAMICS_JOB_INTEGRATE_MOTION,
	DYNAMICS_JOB_BROADPHASE,
	DYNAMICS_JOB_AGENT_SECTOR,
	DYNAMICS_JOB_POST_COLLIDE
};


	/// The base class for all dynamics jobs
class hkDynamicsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkDynamicsJob );

		HK_FORCE_INLINE hkDynamicsJob( hkDynamicsJobType type );
		HK_FORCE_INLINE  hkJobQueue::JobPopFuncResult popDynamicsJobTask( const hkArray<hkSimulationIsland*>& islands, hkDynamicsJob& out );

	public:
		hkEnum<hkDynamicsJobType,hkObjectIndex>	m_jobType;

	protected:
		friend class hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob;
		friend class hkBuildJacobiansJob;
		friend class hkSolveConstraintsJob;
		friend class hkPostCollideJob;
		friend class hkBroadPhaseJob;
		friend class hkAgentSectorJob;
		friend class hkBuildJacobiansTaskJob;
		friend class hkSplitSimulationIslandJob;
		friend class hkIntegrateMotionJob;
		/// this island index is only used internally in the job queue, use m_island instead
		hkObjectIndex m_islandIndex;
	public:

		/// the simulation island: this is set by popJobTask, no need to set it by hand
		hkSimulationIsland* m_island;
};


class hkIntegrateJob : public hkDynamicsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkIntegrateJob);

		HK_FORCE_INLINE hkIntegrateJob(int numIslands);

		HK_FORCE_INLINE  hkJobQueue::JobPopFuncResult popJobTask( const hkArray<hkSimulationIsland*>& islands, hkIntegrateJob& out );

	public:

		int m_numIslands;
};


class hkBuildAccumulatorsJob: public hkDynamicsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkBuildAccumulatorsJob );

		enum { ACCUMULATORS_PER_JOB  = 128 };

		HK_FORCE_INLINE hkBuildAccumulatorsJob(hkDynamicsJobType type = DYNAMICS_JOB_BUILD_ACCUMULATORS) : hkDynamicsJob( type) {; }

		HK_FORCE_INLINE  hkJobQueue::JobPopFuncResult popJobTask( const hkArray<hkSimulationIsland*>& islands, hkBuildAccumulatorsJob& out );

	protected:
		HK_FORCE_INLINE hkBuildAccumulatorsJob(const struct hkBuildJacobianTaskHeader* taskHeader, hkDynamicsJobType type) : hkDynamicsJob( type), m_taskHeader(taskHeader) {; }

	public:

		HK_CPU_PTR(const struct hkBuildJacobianTaskHeader*) m_taskHeader;

			// pointer to the island's entity list in main memory
		HK_CPU_PTR(hkEntity*const*) m_islandEntitiesArray;

			// this offset into m_islandEntitiesArray defines the first entity to be processed in this job/batch
		hkObjectIndex m_firstEntityIdx;

			// number of entities to be processed in this job/batch
		hkObjectIndex m_numEntities;
};


	// This job is never actually added to the queue.
	// This is because it is done by the same job that adds the build accumulators
	// job to the queue.  It is only used as an input to the finishJobAndGetNextJob function.
class hkBuildJacobiansTaskJob: public hkBuildAccumulatorsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkBuildJacobiansTaskJob );

		HK_FORCE_INLINE hkBuildJacobiansTaskJob(const hkIntegrateJob& job, const struct hkBuildJacobianTaskHeader* taskHeader);
};

class hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob : public hkDynamicsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob );

		HK_FORCE_INLINE hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob(const hkBuildAccumulatorsJob& baj);

		HK_CPU_PTR(struct hkBuildJacobianTaskHeader*) m_taskHeader;
};

class hkBuildJacobiansJob : public hkDynamicsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkBuildJacobiansJob );

		HK_FORCE_INLINE hkBuildJacobiansJob(const hkBuildAccumulatorsJob& baj, hkBuildJacobianTask*taskInMainMemory, hkConstraintQueryIn* constraintQueryInInMainMemory);
		HK_FORCE_INLINE hkBuildJacobiansJob(const hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob& fjscb );

		HK_FORCE_INLINE  hkJobQueue::JobPopFuncResult popJobTask( const hkArray<hkSimulationIsland*>& islands, hkBuildJacobiansJob& out );

		union
		{
			struct hkBuildJacobianTask*             m_buildJacobianTask;
			HK_CPU_PTR(struct hkBuildJacobianTask*) m_buildJacobianTaskInMainMemory;
		};

		HK_CPU_PTR(hkConstraintQueryIn*) m_constraintQueryIn;
};

// this job runs parallel to the solve constraint job 
class hkSplitSimulationIslandJob : public hkDynamicsJob
{	
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkSplitSimulationIslandJob );

		HK_FORCE_INLINE hkSplitSimulationIslandJob(const class hkSolveConstraintsJob& job);

		HK_FORCE_INLINE  hkJobQueue::JobPopFuncResult popJobTask( const hkArray<hkSimulationIsland*>& islands, hkSplitSimulationIslandJob& out );

	protected:
		HK_FORCE_INLINE hkSplitSimulationIslandJob(hkDynamicsJobType type, int islandIndex, hkBuildJacobianTaskHeader* m_taskHeader);

	public:
		hkBuildJacobianTaskHeader* m_taskHeader;
};

class hkSolveConstraintsJob : public hkDynamicsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkSolveConstraintsJob );

		hkSolveConstraintsJob() : hkDynamicsJob( DYNAMICS_JOB_SOLVE_CONSTRAINTS) { ; }
		HK_FORCE_INLINE hkSolveConstraintsJob(const hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob& fjscb);
		HK_FORCE_INLINE hkSolveConstraintsJob(const hkBuildJacobiansJob& bjj, const hkBuildJacobianTaskHeader& taskHeader, hkBuildJacobianTaskHeader* taskHeaderInMainMemory);

		HK_CPU_PTR(void*) m_buffer;
		hkUint32   m_bufferSize;

		hkUint32 m_accumulatorsOffset;	// <todo> remove those variables as they make the job pretty big
		hkUint32 m_jacobiansOffset;
		hkUint32 m_solverTempOffset;
		hkUint32 m_schemasOffset;

		hkInt32  m_numSolverResults;

		// Not used by this job, but needed to pass it to the next job
		HK_CPU_PTR(hkBuildJacobianTaskHeader*) m_taskHeader;
};

class hkIntegrateMotionJob : public hkSplitSimulationIslandJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkIntegrateMotionJob);
		HK_FORCE_INLINE hkIntegrateMotionJob( const hkSolveConstraintsJob& job, HK_CPU_PTR(hkBuildJacobianTaskHeader*) taskHeader, int firstEntityIdx, int numEntities, HK_CPU_PTR(void*) solverBuffer );

		HK_FORCE_INLINE  hkJobQueue::JobPopFuncResult popJobTask( const hkArray<hkSimulationIsland*>& islands, hkIntegrateMotionJob& out );

	public:

		enum { ACCUMULATORS_PER_JOB  = 128 };

		hkObjectIndex m_firstEntityIdx;
		hkObjectIndex m_numEntities;

			// the number of inactive frames. This variable is set by the job and analyzed
			// by finish job func
		int m_numInactiveFrames;
		HK_CPU_PTR(void*) m_buffer;
};

	// BROAD PHASE
	// Creates hkAgentSectorJobs, adds to global lists of new and old pairs
	// For now does all broad phase
	// if m_newSplitIslands is set, the pop job assumes that these island
	// were created by the splitIslandJob and finalizes the split,
	// this includes adding new broadphase jobs to the jobqueue for each new
	// simulation island
class hkBroadPhaseJob : public hkDynamicsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkBroadPhaseJob);
		HK_FORCE_INLINE hkBroadPhaseJob( const hkIntegrateJob& job, hkBuildJacobianTaskHeader* taskHeader );
		HK_FORCE_INLINE hkBroadPhaseJob( const hkSplitSimulationIslandJob& job );

		hkJobQueue::JobPopFuncResult popJobTask( hkArray<hkSimulationIsland*>& islands, hkBroadPhaseJob& out );

	public:
		hkObjectIndex m_numIslands;
		hkBuildJacobianTaskHeader* m_taskHeader;
};



// Perform narrow phase collision detection on many agent sectors
class hkAgentSectorJob : public hkDynamicsJob
{
	public:
		enum{	SECTORS_PER_JOB  = 16 };

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkAgentSectorJob);
		HK_FORCE_INLINE hkAgentSectorJob(const hkBroadPhaseJob& job, const hkStepInfo& stepInfo, int numSectors);

		HK_FORCE_INLINE  hkJobQueue::JobPopFuncResult popJobTask( const hkArray<hkSimulationIsland*>& islands, hkAgentSectorJob& out );

	public:
		int m_taskIndex;	// you can calculate the start sector index = m_taskIndex * SECTORS_PER_JOB
		int m_numSectors;

		int m_numTotalTasks;
		HK_CPU_PTR(struct hkAgentSectorHeader*) m_header;
		hkStepInfo m_stepInfo; 
};


#define FIXED_QUEUE_LENGTH 16 * 4 * hkAgentSectorJob::SECTORS_PER_JOB		/* bytes per command * agentsPerSector * sectorsPerJob */

	// This structure is created if several threads doing agent sector jobs on one island.
	// The layout is that this header is followed by an array of pointers to JobInfo.
	// There is one jobInfo for each agentSector job.
	// In the end a hkPostCollideJob is fired to complete the missing constraint modifications
struct hkAgentSectorHeader
{
	static hkAgentSectorHeader* allocate(int numTasks);
	void deallocate(int numTasks);
	static HK_FORCE_INLINE int HK_CALL getAllocatedSize( int numQueues ){ return sizeof(void*) * HK_HINT_SIZE16(numQueues) + sizeof(hkAgentSectorHeader); }

	struct JobInfo
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, JobInfo );
		HK_ALIGN16( hkConstraintInfo m_constraintInfo );
		hkFixedSizePhysicsCommandQueue<FIXED_QUEUE_LENGTH> m_commandQueue;
	};

	HK_FORCE_INLINE  JobInfo* getJobInfo(int index)	{	return (reinterpret_cast<JobInfo**>(this+1))[ HK_HINT_SIZE16(index) ];	}

	mutable int			   m_openJobs;

};


// Perform post narrow phase collision detection on one island
class hkPostCollideJob : public hkDynamicsJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkPostCollideJob);
		HK_FORCE_INLINE hkPostCollideJob( const hkAgentSectorJob& asj );

		int m_numTotalTasks;
		HK_CPU_PTR(hkAgentSectorHeader*) m_header;
};

#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobs.inl>

#endif // HK_DYNAMICS_JOBS_H

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
