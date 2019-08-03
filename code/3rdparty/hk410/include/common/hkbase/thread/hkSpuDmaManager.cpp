/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSpuDmaManager.h>


#if defined(HK_PLATFORM_POTENTIAL_SPU) 

void hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId)
{
	getFromMainMemory(dstOnSpu, srcInMainMemory, size, mode, dmaGroupId);
	waitForDmaCompletion(dmaGroupId);
}

void hkSpuDmaManager::getFromMainMemorySmallAndWaitForCompletion(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId)
{
	getFromMainMemorySmall(dstOnSpu, srcInMainMemory, size, mode, dmaGroupId);
	waitForDmaCompletion(dmaGroupId);
}

void hkSpuDmaManager::putToMainMemoryAndWaitForCompletion(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId)
{
	putToMainMemory(dstInMainMemory, srcOnSpu, size, mode, dmaGroupId);
	waitForDmaCompletion(dmaGroupId);
}

void hkSpuDmaManager::putToMainMemorySmallAndWaitForCompletion(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId)
{
	putToMainMemorySmall(dstInMainMemory, srcOnSpu, size, mode, dmaGroupId);
	waitForDmaCompletion(dmaGroupId);
}

#endif


hkInt32 hkSpuDmaUtils::incrementInt32InMainMemory( HK_CPU_PTR(hkInt32*) variable, int increment, int dmaGroupId )
{
	if ( HK_PLATFORM_IS_SPU)
	{
		// synchronize the alignment between both memory locations
		HK_ALIGN16( hkInt32 buffer[4]);
		hkInt32* copy = hkAddByteOffset( &buffer[0], hkUlong(variable)&0xf);

		hkSpuDmaManager::getFromMainMemorySmall(copy, variable, sizeof(copy[0]), hkSpuDmaManager::READ_WRITE, dmaGroupId);
		hkSpuDmaManager::waitForDmaCompletion(dmaGroupId);

		copy[0] += increment;

		hkSpuDmaManager::putToMainMemorySmall(variable, copy, sizeof(copy[0]), hkSpuDmaManager::WRITE_BACK, dmaGroupId);
		hkSpuDmaManager::waitForDmaCompletion(dmaGroupId);

		hkSpuDmaManager::performFinalChecks( variable, copy, sizeof(copy[0]));

		return copy[0];
	}
	else
	{
		hkInt32 newValue = *variable + increment;
		*variable = newValue;
		return newValue;
	}
}

void hkSpuDmaUtils::setInt32InMainMemory( HK_CPU_PTR(hkInt32*) variable, hkInt32 value, int dmaGroupId )
{
	if ( HK_PLATFORM_IS_SPU)
	{
		HK_ALIGN16( hkInt32 buffer[4]);
		hkInt32* copy = hkAddByteOffset( &buffer[0], hkUlong(variable)&0xf);
		copy[0] = value;
		hkSpuDmaManager::putToMainMemorySmall(variable, copy, sizeof(copy[0]), hkSpuDmaManager::WRITE_NEW, dmaGroupId);
		hkSpuDmaManager::waitForDmaCompletion(dmaGroupId);
		hkSpuDmaManager::performFinalChecks( variable, copy, sizeof(copy[0]));
	}
	else
	{
		*variable = value;
	}
}

void hkSpuDmaUtils::setFloat32InMainMemory( HK_CPU_PTR(float*) dstInMainMemory, hkReal value, int dmaGroupId  )
{
	if ( HK_PLATFORM_IS_SPU)
	{
		HK_ALIGN16( hkReal buffer[4]);
		hkReal* copy = hkAddByteOffset( &buffer[0], hkUlong(dstInMainMemory)&0xf);
		copy[0] = value;
		hkSpuDmaManager::putToMainMemorySmall(dstInMainMemory, copy, sizeof(copy[0]), hkSpuDmaManager::WRITE_NEW, dmaGroupId);
		hkSpuDmaManager::waitForDmaCompletion(dmaGroupId);
		hkSpuDmaManager::performFinalChecks( dstInMainMemory, copy, sizeof(copy[0]));
	}
	else
	{
		*dstInMainMemory = value;
	}
}

void hkSpuDmaUtils::setPntrInMainMemory( HK_CPU_PTR(void**) dstInMainMemory, void* value, int dmaGroupId  )
{
	if ( HK_PLATFORM_IS_SPU)
	{
		HK_ALIGN16( void* buffer[4]);
		void** copy = hkAddByteOffset( &buffer[0], hkUlong(dstInMainMemory)&0xf);
		copy[0] = value;
		hkSpuDmaManager::putToMainMemorySmall(dstInMainMemory, copy, sizeof(copy[0]), hkSpuDmaManager::WRITE_NEW, dmaGroupId);
		hkSpuDmaManager::waitForDmaCompletion(dmaGroupId);
		hkSpuDmaManager::performFinalChecks( dstInMainMemory, copy, sizeof(copy[0]));
	}
	else
	{
		*dstInMainMemory = value;
	}
}

void hkSpuDmaUtils::setChar8InMainMemory( HK_CPU_PTR(hkChar*) dstInMainMemory, hkChar value, int dmaGroupId  )
{
	if ( HK_PLATFORM_IS_SPU)
	{
		HK_ALIGN16( hkChar buffer[16]);
		hkChar* copy = hkAddByteOffset( &buffer[0], hkUlong(dstInMainMemory)&0xf);
		copy[0] = value;
		hkSpuDmaManager::putToMainMemorySmall(dstInMainMemory, copy, sizeof(copy[0]), hkSpuDmaManager::WRITE_NEW, dmaGroupId);
		hkSpuDmaManager::waitForDmaCompletion(dmaGroupId);
		hkSpuDmaManager::performFinalChecks( dstInMainMemory, copy, sizeof(copy[0]));
	}
	else
	{
		*dstInMainMemory = value;
	}
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
