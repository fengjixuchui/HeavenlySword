/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SPU_CONSTRAINT_H
#define HK_SPU_CONSTRAINT_H



#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSpuUtils.h>
#include <hkdynamics/world/simulation/multithreaded/spu/hkSpuConfig.h>
#include <hkbase/thread/job/hkJobQueue.h>
#include <hkdynamics/motion/rigid/hkKeyframedRigidMotion.h>


class hkJobQueue;
struct hkBuildJacobianTaskHeader;


struct hkSpuIntegrateDataSet
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ROOT, hkSpuIntegrateDataSet );

	void init(int dmaGroup)
	{
		for (int i = 0; i < HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE; i++)
		{
			m_motionsArray[i] = reinterpret_cast<hkMotion*>(&m_motionsBuffer[i]);
		}

		m_readWriteMotionsDmaGroup = dmaGroup;
	}

	hkPadSpu<int> m_numMotions;

	HK_ALIGN16( hkMaxSizeMotion m_motionsBuffer[HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE] );
	HK_ALIGN16( hkMotion*       m_motionsArray [HK_SPU_BUILD_ACCUMULATORS_BLOCK_SIZE] );

	int m_readWriteMotionsDmaGroup;
};

struct hkSpuIntegrateDataSetTool
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ROOT, hkSpuIntegrateDataSetTool );

	void init(int readWriteMotionsBaseDmaGroup)
	{
		m_dataSet[0].init(readWriteMotionsBaseDmaGroup+0);
		m_dataSet[1].init(readWriteMotionsBaseDmaGroup+1);
		m_dataSet[2].init(readWriteMotionsBaseDmaGroup+2);
	}

	hkSpuIntegrateDataSet m_dataSet[3];
};



hkJobQueue::JobStatus HK_CALL hkSpuProcessNextJob(	hkJobQueue&             queue, 
													const HK_CPU_PTR(void*) worldDynamicsStepInfoAddress, 
													hkJobQueue::WaitStatus  waitStatusIn,
													int                     spuId = -1);



#endif // HK_SPU_CONSTRAINT_H

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
