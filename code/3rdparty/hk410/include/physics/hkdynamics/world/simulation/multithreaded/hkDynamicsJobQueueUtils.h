/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS_JOB_QUEUE_UTILS
#define HK_DYNAMICS_JOB_QUEUE_UTILS

#include <hkbase/thread/job/hkJobQueue.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>

class hkWorld;
class hkMultiThreadedSimulation;


enum
{
	JOB_MIN_JACOBIANS_PER_JOB = 512,
};



struct hkMtThreadStructure
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_DYNAMICS, hkMtThreadStructure);

	hkMtThreadStructure( hkWorld* world, hkMultiThreadedSimulation* simulation, hkJobQueue::ThreadType threadType );

	class hkWorld* m_world;
	class hkMultiThreadedSimulation* m_simulation;
	hkJobQueue::ThreadType m_threadType;

	hkProcessCollisionInput m_collisionInput;
	hkConstraintQueryIn		m_constraintQueryIn;
};


struct hkJobQueueUtils
{
	static void                          HK_CALL addDynamicsJob   ( hkJobQueue& queue, hkJobQueue::DynamicData* data, const hkJobQueue::JobQueueEntry& jobIn );
	static hkJobQueue::JobPopFuncResult  HK_CALL popDynamicsJob   ( hkJobQueue& queue, hkJobQueue::DynamicData* data,       hkJobQueue::JobQueueEntry& jobIn, hkJobQueue::JobQueueEntry& jobOut );
	static hkJobQueue::JobCreationStatus HK_CALL finishDynamicsJob( hkJobQueue& queue, hkJobQueue::DynamicData* data, const hkJobQueue::JobQueueEntry& jobIn, hkJobQueue::JobQueueEntryInput& newJobCreated );
};



#endif // HK_DYNAMICS_JOB_QUEUE_UTILS

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
