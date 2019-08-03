/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_CONSTRAINT_SOLVER_SETUP_H
#define HK_DYNAMICS2_CONSTRAINT_SOLVER_SETUP_H

//class SolverInfo;
class hkConstraintInstance;
class hkEntity;
class hkStepInfo;
class hkVelocityAccumulator;
class hkJacobianElement;
class hkSimulationIsland;
class hkVelocityAccumulator;

#include <hkconstraintsolver/solve/hkSolve.h>

#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>


struct hkConstraintSchemaInfo
{
	hkConstraintInstance*     m_constraint;
	hkJacobianSchema* m_schema;
	hkReal            m_allowedPenetrationDepth;
};

struct hkBuildJacobianTaskCollection
{
	// this is a list to the tasks (formerly 'batches') which can be executed by the solver
	// during buildJacobiansTasks this variable is owned and created by the thread which initiated the buildJacobiansTasks.
	// This variable is read only once initialized
	HK_CPU_PTR(struct hkBuildJacobianTask*) m_buildJacobianTasks;
	int										m_numBuildJacobianTasks;

	// a list of constraints that need to fire callbacks before the jacobian setup job is executed
	HK_CPU_PTR(const hkConstraintInternal**)  m_callbackConstraints;
	int                                 m_numCallbackConstraints;

		// a list of constraints which can only be setup on PPU
#if defined(HK_PLATFORM_HAS_SPU)

	HK_CPU_PTR(const hkConstraintInternal**)  m_ppuSetupOnlyConstraints;
	int                                 m_numPpuSetupOnlyConstraints;
#endif
};


	// Rules for reading and writing:
	// All mutables can only be accessed (read or write) in single threaded mode
	// All other variables can be accessed read only
	// this structure is allocated in buildJacobianTasksJob
	// and deleted in the broadphase job
struct hkBuildJacobianTaskHeader
{
	public:

	    HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkBuildJacobianTaskHeader );

		HK_FORCE_INLINE hkBuildJacobianTaskHeader() : m_referenceCount(1), m_buffer(0), m_bufferSize(0) {}

	public:

			// can only be accessed if the job queue is locked
		HK_ALIGN16( mutable int	  m_openJobs );

			// the open jobs before triggering the broadphase, jobs which modify this variable:
			//   - DYNAMICS_JOB_SPLIT_ISLAND
			//   - DYNAMICS_JOB_INTEGRATE_MOTION
			// can only be accessed if the job queue is locked
		mutable int m_numUnfinishedJobsForBroadphase;

			// set to true by the finish job func, 
			// maybe set to 0 by the integrate motion jobs
			// this variable will be examined by the broadphase, which will optionally deactivate the island
		mutable int m_islandShouldBeDeactivated;

			// the broadphase job should delete this structure. 
			// As the split job can create several islands, we get several broadphase jobs for this structure.
			// The last broadphase job should delete this.
			// You are only allowed to access this variable using hkCriticalSection::atomicExchangeAdd()
		mutable long m_referenceCount;

			//
			//	constraints info
			//
	    HK_CPU_PTR(void*) m_buffer;

		hkChar m_exportFinished;

			// this is set to true, if the island requests a split and a split island job should be created
		hkChar m_splitCheckRequested;

	    int   m_bufferSize;
	    HK_CPU_PTR(hkVelocityAccumulator*)	m_accumulatorsBase;
	    HK_CPU_PTR(hkJacobianElement*)		m_jacobiansBase;
	    HK_CPU_PTR(hkJacobianSchema*)		m_schemasBase;
	    HK_CPU_PTR(hkSolverElemTemp*)		m_solverTempBase;

	    hkInt32  m_numSolverResults;

	    HK_CPU_PTR(hkConstraintQueryIn*)    m_constraintQueryIn;

			// the entities of the original islands, if m_newSplitIslands is empty,
			// these entities are identical to the island entities, else they have
			// to be freed when the task header is deleted.
		hkEntity*const* m_allEntities;		
		hkObjectIndex   m_numAllEntities;
		hkObjectIndex   m_entitiesCapacity;		

			//
			// variables set if the island is split by the hkSplitIslandJob
			//
#if !defined (HK_PLATFORM_PS3SPU)
		hkArray<hkSimulationIsland*> m_newSplitIslands;
#else
		HK_ALIGN( hkUint8	m_newSplitIslandsDummy[sizeof(hkArray<hkSimulationIsland*>)],4);
#endif


	public:
		friend struct hkJobQueueUtils;
		friend struct hkSingleThreadedJobsOnIsland;

		mutable struct hkBuildJacobianTaskCollection m_tasks;
};

	// setting this value so that hkBuildJacobianTask ~< 2^x
struct hkBuildJacobianTask
{
#if (HK_POINTER_SIZE == 4)
	enum { MAX_NUM_ATOM_INFOS = 112 };
#else
	enum { MAX_NUM_ATOM_INFOS = 60 };
#endif

	struct AtomInfo
	{
		// atom ptr
		HK_CPU_PTR(hkConstraintAtom*)	    m_atoms;
		HK_CPU_PTR(hkConstraintInstance*)	m_instance;
		HK_CPU_PTR(hkConstraintRuntime*)	m_runtime;
		HK_CPU_PTR(const hkTransform*)		m_transformA;
		HK_CPU_PTR(const hkTransform*)		m_transformB;

		// atom size
		int m_atomsSize;

		int m_runtimeSize;

		// accumulator offsets
		int m_accumulatorOffsetA;
		int m_accumulatorOffsetB;
	};

	inline hkBuildJacobianTask();

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkBuildJacobianTask );
	HK_ALIGN16( HK_CPU_PTR(hkBuildJacobianTask*) m_next );

	HK_CPU_PTR(hkBuildJacobianTaskHeader*)	m_taskHeader;
	HK_CPU_PTR(hkJacobianElement*)			m_jacobiansBase;

	HK_CPU_PTR(hkVelocityAccumulator*)		m_accumulators;
	HK_CPU_PTR(hkJacobianElement*)			m_jacobians;
	HK_CPU_PTR(hkJacobianSchema*)			m_schemas;
	HK_CPU_PTR(hkJacobianSchema*)			m_schemasOfNextTask;

	int m_numAtomInfos;
	AtomInfo m_atomInfos[MAX_NUM_ATOM_INFOS];
};

struct hkConstraintSolverResources
{
	template <typename elem>
	struct BufferState
	{
		elem*	   m_begin;
		elem*	   m_end;
		elem*	   m_current;
		elem*	   m_lastProcessed;
	};

		// a struct used to store a backup of the transform
		// in the velocity accumulator, because the solver expects
		// a zero for the sumVelocities
	struct VelocityAccumTransformBackup
	{
		hkVector4 m_linear;
		hkVector4 m_angular;
	};


	// External data
	hkStepInfo*			   m_stepInfo;
	hkSolverInfo*		   m_solverInfo;
	hkConstraintQueryIn*   m_constraintQueryInput;

	// Memory
	hkVelocityAccumulator* m_accumulators;
	hkVelocityAccumulator* m_accumulatorsEnd;
	VelocityAccumTransformBackup* m_accumulatorsBackup;
	VelocityAccumTransformBackup* m_accumulatorsBackupEnd;

	hkJacobianElement*	   m_jacobians;
	hkJacobianElement*	   m_jacobiansEnd;
	hkSolverElemTemp*	   m_elemTemp;
	hkSolverElemTemp*      m_elemTempEnd;

	BufferState<hkJacobianSchema> m_schemas[2];

	hkVelocityAccumulator* m_accumulatorsCurrent;
	hkJacobianElement*	   m_jacobiansCurrent;
	hkSolverElemTemp*      m_elemTempCurrent;
	hkSolverElemTemp*      m_elemTempLastProcessed;
};


	/// some helper functions for using the constraint solver
class hkConstraintSolverSetup
{
	public:

			// solve and integrate a number of rigid bodies. Returns the number of inactive frames
		static int HK_CALL solve(
							const hkStepInfo& stepInfo, const hkSolverInfo& solverInfo,
							hkConstraintQueryIn& constraintQueryIn, hkSimulationIsland& island,
							hkEntity*const* bodies, int numBodies	);

	public:

			//
			//	Functions used for multithreaded solving
			//
		static int HK_CALL calcBufferSize( hkSimulationIsland& island, int minJacobiansPerJob );


		static int HK_CALL calcBufferOffsetsForSolve( hkSimulationIsland& island, char* bufferIn, int bufferSize, hkBuildJacobianTaskHeader& taskHeader );


			///	Build a partial list of jacobians and schemas. If nextSchema is set,
			/// this function will insert a goto schema at the end of the schemas.
		static void HK_CALL buildJacobianElementBatch(	hkConstraintQueryIn& in,	hkConstraintInternal** constraints, int numConstraints,
													    hkVelocityAccumulator* baseAccum,
														hkJacobianSchema*  schemaOut,
														hkJacobianElement* jacobianOut,
														hkJacobianSchema* nextSchema	);

			///	Each constraint gets queried: It produces a few jacobian elements, returns the number of jacobians generated.
		static void HK_CALL buildJacobianElements( hkConstraintQueryIn& in,
										hkEntity*const* bodies, int numBodies,
										hkVelocityAccumulator* accusIn,
										hkJacobianElement* jacobianOut,
										hkJacobianElement* jacobianOutEnd,
										hkJacobianElement* jacobianOutB,
										hkJacobianSchema*  schemaOut);

private:
		static HK_FORCE_INLINE void HK_CALL _buildAccumulators( const hkStepInfo& info, hkEntity*const* bodiesIn, int numEntities, hkVelocityAccumulator* accumOut );

		static HK_FORCE_INLINE void HK_CALL _buildJacobianElement( const hkConstraintInternal* c,	hkConstraintQueryIn& in, hkVelocityAccumulator* baseAccum,  class hkConstraintQueryOut& out );

		static HK_FORCE_INLINE void HK_CALL _buildJacobianElements( hkConstraintQueryIn& in,
										hkEntity*const* bodies, int numBodies,
										hkVelocityAccumulator* accusIn,
										hkJacobianElement* jacobianOut,
										hkJacobianElement* jacobianOutEnd,
										hkJacobianElement* jacobianOutB,
										hkJacobianSchema*  schemaOut);

	public:

		//
		// Functions used for constraint sub-solving and dynamic feeding of data to the solver
		//
		enum SolverMode
		{
			SOLVER_MODE_PROCESS_ALL,
			SOLVER_MODE_INCREMENTAL_CONTINUE
		};

			/// Acquires the scratchpad and initializes a hkConstraintSolverResources struct
		static void HK_CALL initializeSolverState(hkStepInfo& stepInfo, hkSolverInfo& solverInfo, hkConstraintQueryIn& constraintQueryIn, char* buffer, int bufferSize, hkConstraintSolverResources& s);

			/// Releases the scratchpad
		static void HK_CALL shutdownSolver(hkConstraintSolverResources& s);

			/// Builds accumulators
		static void HK_CALL internalAddAccumulators(hkConstraintSolverResources& s, hkEntity*const * entities, int numEntities);

			///	Each constraint gets queried: It produces a few jacobian elements, returns the number of jacobians generated
		static void HK_CALL internalAddJacobianElements(hkConstraintSolverResources& s,
												hkConstraintInstance** constraints, int numConstraints,
												hkArray<hkConstraintSchemaInfo>& constraintStatus);

		static void HK_CALL subSolve(hkConstraintSolverResources& s, SolverMode mode);

			/// This function integrates the rigid bodies by using the data in the linear and angular velocity
			/// field in the accumulators and not the sumLinearVelocity.
			/// The sumLinearVelocities are typically set in the hkSolver::integrateVelocities, however if
			/// you only call hkSolveStepJacobion, this sumLinearVelocities won't be used and you have to use this
			/// function to integrate your rigid bodies.
		static void HK_CALL oneStepIntegrate( const struct hkSolverInfo& si, const hkStepInfo& info, const hkVelocityAccumulator* accumulatorsBase, hkEntity*const* entities, int numEntities ); 


		static hkBool HK_CALL internalIsMemoryOkForNewAccumulators    (hkConstraintSolverResources& s, hkEntity**     entities,    int numEntities);
		static hkBool HK_CALL internalIsMemoryOkForNewJacobianElements(hkConstraintSolverResources& s, hkConstraintInstance** constraints, int numConstraints);

};


hkBuildJacobianTask::hkBuildJacobianTask()
{
	m_numAtomInfos = 0;
	m_schemasOfNextTask = HK_NULL;
	m_next = HK_NULL;
}


#endif // HK_DYNAMICS2_CONSTRAINT_SOLVER_SETUP_H





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
