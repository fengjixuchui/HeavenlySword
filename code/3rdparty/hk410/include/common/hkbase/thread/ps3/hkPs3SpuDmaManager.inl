/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSpuDmaManager.h>

#include <spu_intrinsics.h>
#include <sys/spu_thread.h>
#include <sys/spu_event.h>
#include <stdint.h>

#include <cell/dma.h>



void hkSpuDmaManager::getFromMainMemory(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId)
{
	HK_CHECK_ALIGN16(dstOnSpu);
	HK_CHECK_ALIGN16(srcInMainMemory);
	HK_ASSERT2(0xaf52de41, dstOnSpu && srcInMainMemory, "dstOnSpu and srcInMainMemory may not be HK_NULL.");
	cellDmaLargeGet(dstOnSpu, (hkUlong)srcInMainMemory, size, dmaGroupId, 0, 0);
}

void hkSpuDmaManager::putToMainMemory(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId)
{
	HK_CHECK_ALIGN16(dstInMainMemory);
	HK_CHECK_ALIGN16(srcOnSpu);
	HK_ASSERT2(0xaf52de42, dstInMainMemory && srcOnSpu, "dstInMainMemory and srcOnSpu may not be HK_NULL.");
	cellDmaLargePut(srcOnSpu, (hkUlong)dstInMainMemory, size, dmaGroupId, 0, 0);
}

void hkSpuDmaManager::getFromMainMemorySmall(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId)
{
	HK_ASSERT (0xaf344953, (dmaGroupId >= 0) && (dmaGroupId < 31) );
	HK_ASSERT2(0xad67d84f, (size == 1) || (size == 2) || (size == 4) || (size == 8),      "size must be 1,2,4 or 8 bytes");
	HK_ASSERT2(0xad67d8dd, (hkUlong(dstOnSpu) & 0xf) == (hkUlong(srcInMainMemory) & 0xf), "dstOnSpu and srcInMainMemory must have same lower 4 bits.");
	HK_ASSERT2(0xad67d844, !(hkUlong(dstOnSpu) & (size -1)),                              "dstOnSpu and srcInMainMemory must be aligned to the size of the transfer.");

	cellDmaSmallGet(dstOnSpu, (hkUlong)srcInMainMemory, size, dmaGroupId, 0, 0);
}

void hkSpuDmaManager::putToMainMemorySmall(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId)
{
	HK_ASSERT (0xaf344953, (dmaGroupId >= 0) && (dmaGroupId < 31) );
	HK_ASSERT2(0xad67d84f, (size == 1) || (size == 2) || (size == 4) || (size == 8),      "size must be 1,2,4 or 8 bytes");
	HK_ASSERT2(0xad67d8dd, (hkUlong(srcOnSpu) & 0xf) == (hkUlong(dstInMainMemory) & 0xf), "srcOnSpu and dstInMainMemory must have same lower 4 bits.");
	HK_ASSERT2(0xad67d844, !(hkUlong(srcOnSpu) & (size -1)),                              "srcOnSpu and dstInMainMemory must be aligned to the size of the transfer.");

	cellDmaSmallPut(srcOnSpu, (hkUlong)dstInMainMemory, size, dmaGroupId, 0, 0);
}

void hkSpuDmaManager::waitForDmaCompletion(int dmaGroupId)
{
	cellDmaWaitTagStatusAll( 1 << dmaGroupId);
}

void hkSpuDmaManager::waitForAllDmaCompletion()
{
	cellDmaWaitTagStatusAll(0xffffffff);
}

void hkSpuDmaManager::performFinalChecks( const HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size )
{
}

void hkSpuDmaManager::tryToPerformFinalChecks(const HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size )
{
}

void hkSpuDmaManager::deferFinalChecksUntilWait(const HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size )
{
}

void hkSpuDmaManager::convertReadOnlyToReadWrite(void* ppu, const void* spu, int size)
{
}

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
