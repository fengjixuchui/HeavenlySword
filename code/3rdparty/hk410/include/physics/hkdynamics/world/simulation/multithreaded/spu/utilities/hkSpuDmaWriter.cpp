/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/simulation/multithreaded/spu/utilities/hkSpuDmaWriter.h>
#include <hkbase/thread/hkSpuDmaManager.h>



void hkSpuDmaWriter::writeBackBuffer(int sizebufferUsed)
{
	const int prevMultipleOf16 = sizebufferUsed & (~0x0f);

	// Dma writeback
	{
		HK_ASSERT2(0xad6789dd, !m_dstEnd || hkAddByteOffsetCpuPtr( m_currentDstPosition.val(), prevMultipleOf16 ) <= m_dstEnd, "Ppu destination buffer overflown." );
		hkSpuDmaManager::putToMainMemory( m_currentDstPosition, m_workBuffer, prevMultipleOf16, hkSpuDmaManager::WRITE_NEW, m_workDmaGroup );
		m_currentDstPosition = hkAddByteOffsetCpuPtr( m_currentDstPosition.val(), prevMultipleOf16 );
	}

	// Swap buffers
	{ hkPadSpu<void*> h = m_workBuffer;   m_workBuffer   = m_transferBuffer;   m_transferBuffer = h; }
	{ hkPadSpu<int>  h = m_workDmaGroup; m_workDmaGroup = m_transferDmaGroup; m_transferDmaGroup = h; }


	// Make sure our new work buffer dma has finished.
	{
		hkSpuDmaManager::waitForDmaCompletion(m_workDmaGroup);
		HK_ON_DEBUG( m_wasBufferRequested = false );

		hkSpuDmaManager::tryToPerformFinalChecks( HK_NULL, m_workBuffer.val(), 0 );
	}

	// Copy the remaining data (always < 16 bytes) to our new work buffer (using a 2x64bit copy)

	{
		const int sub16 = sizebufferUsed - prevMultipleOf16;
		hkUint64* dest   = (hkUint64*)(m_workBuffer.val());
		hkUint64* source = (hkUint64*)( hkAddByteOffset(m_transferBuffer.val(), prevMultipleOf16 ) );
		dest[0] = source[0];
		dest[1] = source[1];
		m_currentPositionInWorkBuffer = hkAddByteOffset( dest, sub16 );
	}
}



void hkSpuDmaWriter::flush()
{
	const int bufferUsed = (int)hkGetByteOffset( m_workBuffer, m_currentPositionInWorkBuffer );
	const int bufferUsedAligned = HK_NEXT_MULTIPLE_OF(16, bufferUsed );

	// Dma writeback
	if ( bufferUsed > 0 )
	{
		HK_ASSERT2(0xad6789dd, !m_dstEnd || hkAddByteOffsetCpuPtr( m_currentDstPosition.val(), bufferUsedAligned ) <= m_dstEnd, "Ppu destination buffer overflown." );
		hkSpuDmaManager::putToMainMemory( m_currentDstPosition, m_workBuffer, bufferUsedAligned, hkSpuDmaManager::WRITE_NEW, m_workDmaGroup );
		m_currentDstPosition = hkAddByteOffsetCpuPtr( m_currentDstPosition.val(), bufferUsed );
	}

	// reset the work buffer
	m_currentPositionInWorkBuffer = m_workBuffer;

	// safety: wait for dma's finished for both buffers
	hkSpuDmaManager::waitForDmaCompletion(m_workDmaGroup);
	hkSpuDmaManager::waitForDmaCompletion(m_transferDmaGroup);

	if ( bufferUsed > 0 )
	{
		hkSpuDmaManager::performFinalChecks( HK_NULL, m_workBuffer.val(), 0 );
	}

	hkSpuDmaManager::tryToPerformFinalChecks( HK_NULL, m_transferBuffer.val(), 0 );
}



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
