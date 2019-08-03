/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSpuUtils.h>

#if defined HK_SIMULATE_SPU_DMA_ON_CPU
#	include <hkbase/thread/util/hkSpuSimulator.h>
#endif



void hkSpuDmaManager::getFromMainMemory(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId)
{
	HK_ASSERT (0xaf344953, (dmaGroupId >= 0) && (dmaGroupId < 31) );
	HK_ASSERT2(0xad97d84f, !(size                     & 0xf), "size must be a multiple of 16.");
	HK_ASSERT2(0xad97d8dd, !(hkUlong(dstOnSpu)        & 0xf), "dstOnSpu must be 16-byte aligned. Try getFromMainMemorySmall instead.");
	HK_ASSERT2(0xad97d8de, !(hkUlong(srcInMainMemory) & 0xf), "srcInMainMemory must be 16-byte aligned. Try getFromMainMemorySmall instead.");

#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374fe, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->getFromMainMemory( dstOnSpu, srcInMainMemory, size, mode, dmaGroupId );
	return;
#endif
}

void hkSpuDmaManager::putToMainMemory(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId)
{
	HK_ASSERT(0xaf344953, (dmaGroupId >= 0) && (dmaGroupId < 31) );
	HK_ASSERT2(0xad6ed84f, !(size                     & 0xf), "size must be a multiple of 16.");
	HK_ASSERT2(0xad6ed8dd, !(hkUlong(srcOnSpu)        & 0xf), "srcOnSpu must be 16-byte aligned. Try getFromMainMemorySmall instead.");
	HK_ASSERT2(0xad6ed8de, !(hkUlong(dstInMainMemory) & 0xf), "dstInMainMemory must be 16-byte aligned. Try getFromMainMemorySmall instead.");

#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374fe, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->putToMainMemory( dstInMainMemory, srcOnSpu, size, mode, dmaGroupId );
	return;
#endif
}

void hkSpuDmaManager::getFromMainMemorySmall(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId)
{
	HK_ASSERT(0xaf344953, (dmaGroupId >= 0) && (dmaGroupId < 31) );
	HK_ASSERT2(0xad63d84f, (size == 1) || (size == 2) || (size == 4) || (size == 8), "size must be 1,2,4 or 8 bytes");
	HK_ASSERT2(0xad63d8dd, (hkUlong(dstOnSpu) & 0xf) == (hkUlong(srcInMainMemory) & 0xf), "dstOnSpu and srcInMainMemory must have same lower 4 bits.");
	HK_ASSERT2(0xad63d844, !(hkUlong(dstOnSpu) & (size -1)), "dstOnSpu and srcInMainMemory must be aligned to the size of the transfer.");

#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374fe, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->getFromMainMemory( dstOnSpu, srcInMainMemory, size, mode, dmaGroupId );
	return;
#endif
}

void hkSpuDmaManager::putToMainMemorySmall(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId)
{
	HK_ASSERT(0xaf344953, (dmaGroupId >= 0) && (dmaGroupId < 31) );
	HK_ASSERT2(0xad62d84f, (size == 1) || (size == 2) || (size == 4) || (size == 8), "size must be 1,2,4 or 8 bytes");
	HK_ASSERT2(0xad62d8dd, (hkUlong(srcOnSpu) & 0xf) == (hkUlong(dstInMainMemory) & 0xf), "srcOnSpu and dstInMainMemory must have same lower 4 bits.");
	HK_ASSERT2(0xad62d844, !(hkUlong(srcOnSpu) & (size -1)), "srcOnSpu and dstInMainMemory must be aligned to the size of the transfer.");

#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374fe, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->putToMainMemory( dstInMainMemory, srcOnSpu, size, mode, dmaGroupId );
	return;
#endif
}

void hkSpuDmaManager::waitForDmaCompletion(int dmaGroupId)
{
	HK_ASSERT(0xaf344953, (dmaGroupId >= 0) && (dmaGroupId < 31) );

#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374fe, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->waitDmaGroup( 1<<dmaGroupId );
#endif
}

void hkSpuDmaManager::waitForAllDmaCompletion()
{
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374ff, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->waitDmaGroup( 0xffffffff );
#endif
}

void hkSpuDmaManager::performFinalChecks(const HK_CPU_PTR(void*) ppu, const void* spu, int size )
{
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374ff, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->performFinalChecks( ppu, spu, size);
#endif
}

void hkSpuDmaManager::tryToPerformFinalChecks(const HK_CPU_PTR(void*) ppu, const void* spu, int size )
{
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374ff, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->tryToPerformFinalChecks( ppu, spu, size);
#endif
}

void hkSpuDmaManager::deferFinalChecksUntilWait(const HK_CPU_PTR(void*) ppu, const void* spu, int size )
{
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8374ff, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->deferFinalChecksUntilWait( ppu, spu, size);
#endif
}

void hkSpuDmaManager::convertReadOnlyToReadWrite(void* ppu, const void* spu, int size)
{
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	HK_ASSERT2(0xaf8324f1, hkSpuSimulator::Client::getInstance(), "You have to create an instance of hkSpuSimulator::Client first!");
	hkSpuSimulator::Client::getInstance()->convertReadOnlyToReadWrite( ppu, spu, size);
#endif
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
