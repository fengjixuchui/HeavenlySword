/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include "physics/config.h"
#include <hkbase\config\hkConfigVersion.h>

#if HAVOK_SDK_VERSION_MAJOR != 4
#include "physics/hkSpuThreadUtil.h"

#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuConstraint.h>
#include <hkbase/thread/hkSpuUtils.h>
#include <hkbase/hkBase.h>
#include <hkdynamics/world/simulation/multithreaded/job/hkJobQueue.h>
#include <hkconstraintsolver/solve/hkSolverInfo.h>
#include <hkbase/thread/hkThread.h>
#include "physics/hkPhysicsThreads.h"

#include <hkdynamics/world/hkWorld.h>
#include <hkbase/basesystem/hkBaseSystem.h>

/////////////////////////////////////////////////////////////////////
// SPU constraints
/////////////////////////////////////////////////////////////////////

#define BUFFER_SIZE 20000

struct Params
{
	void* arg1;
	void* arg2;
	void* arg3;
	void* arg4;
};



int hkConstraintSolveThreadMainFunc(const void* solverInfoAddress,  void* queueAddress, const void* c, const void* d)
{
	char* buffer = hkAlignedAllocate<char> (16, BUFFER_SIZE, HK_MEMORY_CLASS_DYNAMICS);


	HK_CHECK_ALIGN16(solverInfoAddress);
	HK_CHECK_ALIGN16(queueAddress);
	HK_SPU_DEBUG_PRINTF("SPU task started\n");	
	hkJobQueue& queue = *(hkJobQueue*)queueAddress;

	// This loops indefinitely looking for constraint jobs to solve
	while (1)
	{
		hkSpuConstraintSolve( queue, solverInfoAddress, hkJobQueue::WAIT_FOR_NEXT_JOB, buffer, BUFFER_SIZE );
	}

	HK_SPU_DEBUG_PRINTF("SPU task ending\n");

	hkAlignedDeallocate(buffer);

	return 1;
}

// This function must be called once per step() call.
void* HK_CALL hkConstraintSolveSpursTask(void* params)
{
	Params* p = (Params*)params;
	void* solverInfoAddress = p->arg1;
	void* queueAddress = p->arg2;
	char* buffer = hkAlignedAllocate<char> (16, BUFFER_SIZE, HK_MEMORY_CLASS_DYNAMICS);

	HK_CHECK_ALIGN16(solverInfoAddress);
	HK_CHECK_ALIGN16(queueAddress);
	HK_SPU_DEBUG_PRINTF("SPU task started\n");	
	hkJobQueue& queue = *(hkJobQueue*)queueAddress;

	// This will solve constraint jobs from a step() call until there are no more jobs to solve.
	hkSpuConstraintSolve( queue, solverInfoAddress, hkJobQueue::WAIT_FOR_NEXT_JOB, buffer, BUFFER_SIZE );

	HK_SPU_DEBUG_PRINTF("SPU task ending\n");

	hkAlignedDeallocate(buffer);

	return 0;
}


void* HK_CALL hkConstraintSolveThreadMain(void *v)
{
	Params* p = (Params*)v;
	hkConstraintSolveThreadMainFunc(p->arg1, p->arg2, p->arg3, p->arg4 );

	return 0;
}


/////////////////////////////////////////////////////////////////////
// Standard CPU thread 
/////////////////////////////////////////////////////////////////////


void* HK_CALL hkCpuThreadMainFunc(void *v)
{
	//
	// Initialize
	//

	// Initialize the thread memory for this thread, referencing the global memory instance
	hkThreadMemory threadMemory(&hkMemory::getInstance(), 16);
	{ 
		hkBaseSystem::initThread( &threadMemory );
	}
	char* stackBuffer;
	{
		int stackSize = 2000000; // Stack size of 2 Mb.
		stackBuffer = hkAllocate<char>( stackSize, HK_MEMORY_CLASS_BASE);
		threadMemory.setStackArea( stackBuffer, stackSize);
	}

	// Step
	((hkWorld*)v)->childThreadStep();


	// Quit
	threadMemory.setStackArea(0, 0);
	hkDeallocate(stackBuffer);
	hkBaseSystem::clearThreadResources();

	return 0;
}

void* HK_CALL hkCpuThreadMain(void *v)
{
	Params* p = (Params*)v;
	hkCpuThreadMainFunc( p->arg1 );

	return 0;
}

#endif //#if HAVOK_SDK_VERSION_MAJOR != 4

/*
* Havok SDK - DEMO RELEASE, BUILD(#20060116)
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
