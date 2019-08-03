/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSpuUtils.h>
#include <hkbase/thread/hkSpuDmaManager.h>

#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuConstraint.h>
#include <hkbase/thread/hkSpuStack.h>
#include <hkbase/thread/job/hkJobQueue.h>
#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuConfig.h>

#include <hkconstraintsolver/solve/hkSolverInfo.h>

#include <sys/spu_thread.h>
#include <stdint.h>
#include <cell/spurs.h>
#include <spu_printf.h>


#define HK_SPURS_TOTAL_BUFFER_SIZE (HK_SPU_TOTAL_BUFFER_SIZE - 5000)

// The size of the buffer to be allocated for solving constraint systems.  The size available depends
// on the size of the compiled elf.

// Allocate a buffer for solving constraint systems. 
static HK_ALIGN16(char buffer[HK_SPURS_TOTAL_BUFFER_SIZE]);
static HK_ALIGN( char queueBuf[sizeof(hkJobQueue)], 128 );

struct hkSpursArgs
{
	const void* m_solverInfoAddress;
	const void* m_queueAddress;
	const void* m_monitorCacheAddress;
	int m_spuId;
};

// this function is used to reliably catch asserts on the spu when debugging
void hkAssertFailed(int id, const char* text)
{
	//@@@PS3SF: <todo: move id to r0
	if ( text )
	{
		HK_SPU_DEBUG_PRINTF( (text) );
	}
	__asm__("stop");
}

void cellSpursMain(qword argTask, uint64_t argTaskset)
{ 
	
	hkSpuStack::getInstance().initMemory(buffer, HK_SPURS_TOTAL_BUFFER_SIZE);


//	hkSpuInitMonitors( (char*)localMonitorStream, sizeof(localMonitorStream) );
	
//	hkSpuMonitorCache monitorCache;
//	hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &monitorCache, (const void*)hkUlong(monitorCacheEA), sizeof(hkSpuMonitorCache), hkSpuDmaManager::READ_COPY );
//	hkSpuDmaManager::performFinalChecks( (const void*)hkUlong(monitorCacheEA), &monitorCache, sizeof(hkSpuMonitorCache) );


	hkSpursArgs& spursArgs = (hkSpursArgs&)(argTask);

	HK_CHECK_ALIGN16(spursArgs.m_queueAddress);
	hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &queueBuf, (const void*)spursArgs.m_queueAddress, sizeof(hkJobQueue), hkSpuDmaManager::READ_COPY  );
	hkSpuDmaManager::performFinalChecks( (const void*)hkUlong(spursArgs.m_queueAddress), &queueBuf, sizeof(hkJobQueue));

	hkJobQueue* queue = (hkJobQueue*)&queueBuf;

	//
	// This call will exit immediately if prepareStepWorld has not been called from the PPU.
	// If prepareStepWorld has been called it will wait until a constraint job is available, (due to the PPU stepWorld() call)
	// and will end when no more constraint jobs are available.
	//
	hkSpuProcessNextJob( *queue, spursArgs.m_solverInfoAddress, hkJobQueue::WAIT_FOR_NEXT_JOB );

	queue->m_taskCompletionSemaphore.release();

	cellSpursExit ();

	return;

}


/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20061009)
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
