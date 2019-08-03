/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CPU_CONSTRAINT_H
#define HK_CPU_CONSTRAINT_H
 
#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>
#include <hkconstraintsolver/constraint/atom/hkBuildJacobianFromAtoms.h>
#include <hkconstraintsolver/simpleConstraints/hkSimpleConstraintUtil.h>
#include <hkconstraintsolver/jacobian/hkJacobianHeaderSchema.h>

#include <hkdynamics/constraint/response/hkSimpleCollisionResponse.h>
#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>
#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobs.h>
#include <hkdynamics/world/simulation/multithreaded/hkDynamicsJobQueueUtils.h>
 
 

struct hkSingleThreadedJobsOnIsland
{
	static HK_FORCE_INLINE hkJobQueue::JobStatus HK_CALL cpuBroadPhaseJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& nextJobOut );
	static HK_FORCE_INLINE hkJobQueue::JobStatus HK_CALL cpuFireJacobianSetupCallbackAndBuildPpuOnlyJacobiansJob(	hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& jobInOut,	hkJobQueue::WaitStatus waitStatus);
	static HK_FORCE_INLINE void setSchemasAndJacPtrsInTasks( hkBuildJacobianTaskHeader* taskHeader, hkJacobianSchema* schemas, hkJacobianElement* jacobians );
};

#include <hkdynamics/world/simulation/multithreaded/cpu/hkCpuConstraint.inl>



#endif // HK_CPU_CONSTRAINT_H

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
