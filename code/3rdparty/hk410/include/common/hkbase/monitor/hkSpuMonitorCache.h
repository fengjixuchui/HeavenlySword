/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SPU_MONITOR_CACHE__H
#define HK_SPU_MONITOR_CACHE__H

#include <hkbase/thread/hkSpuUtils.h>
#include <hkbase/thread/hkCriticalSection.h>

/// The spu monitor cache is setup on the PPU 
/// and given to any desired SPU program that can use it as the 
/// destination for their own local timer stream (as setup with hkSpuInitMonitors)
/// sendToMainMemoryAndReset will send the local data to the current free cache
/// area and current free string cache area as pointed to by the m_current members
/// of the cache.
struct hkSpuMonitorCache
{
	hkSpuMonitorCache() : m_dataLock(10000) {}

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE_CLASS, hkSpuMonitorCache);


	struct StringLookup
	{
		hkUint32 id; // a char* on the SPU == 32bit.
		/// This struct is followed by null terminated string, with extra bytes to pad to 4 byte align
	};

		/// Call this on PPU once all your timed SPU jobs are done:
	void addToMonitorStreamAndResetPpu();

		/// Call this on SPU to take the current local stream and send it and any new strings
		/// to the shared cache of SPU timers in main mem. It will reset the local stream.
	void sendToMainMemoryAndResetSpu( HK_CPU_PTR(hkSpuMonitorCache*) effectiveAddr, hkSpuMonitorCache::StringLookup* localStringTable, int localStringTableSize );

	void clearTimersSpu( HK_CPU_PTR(hkSpuMonitorCache*) effectiveAddr );

	void getStringTableSpu( hkSpuMonitorCache::StringLookup* localStringTable );


	HK_ALIGN( HK_CPU_PTR(char*) m_current, 16); // so it can be dma'd by itself. Assumed to be at start of struct
	HK_CPU_PTR(char*) m_start; 	// points to the start of the cache. Setup only.
	HK_CPU_PTR(char*) m_end; 	// points to the end of the whole cache. Do not exceed.

	HK_ALIGN( HK_CPU_PTR(StringLookup*) m_stringTableCurrent, 16); // so it can be dma'd by itself. 
	HK_CPU_PTR(StringLookup*) m_stringTableStart; 	// points to the start of the shared string table. Setup only.
	HK_CPU_PTR(StringLookup*) m_stringTableEnd; 	// points to the end of the shared string table. Do not exceed.

	hkCriticalSection m_dataLock; // aquire this before inspecting m_current, and update m_current before releasing
};


/// Init monitor streams with given static buffer on an SPU. Call this once at the start of your SPU program, 
/// and/or instead of hkMonitorStream::reset each frame / job
/// buffer should be aligned to at least 16bytes, and buffer size is usually a in the order of a few kilobytes, 
/// depending on how much timers you used (each timer == name of timer + chars to indicate start and stop of timer,
/// so depends on amount of timers (watch for recursion) and the length of timer names used.
void HK_CALL hkSpuInitMonitors( char* buffer, hkUint32 bufferSize );

#endif // HK_SPU_MONITOR_CACHE__H

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
