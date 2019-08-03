/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>

#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/monitor/hkSpuMonitorCache.h>

#if defined(HK_PLATFORM_PS3SPU)

#include <spu_intrinsics.h>
#include <stdint.h>
#include <cell/dma.h>

void hkSpuInitMonitors( char* buffer, hkUint32 bufferSize )
{
	// reset the decrementer, to lessen chance of overflow
	spu_writech(SPU_WrDec, 0);
	hkMonitorStream::getInstance().setStaticBuffer( buffer, bufferSize );
}

void hkSpuMonitorCache::addToMonitorStreamAndResetPpu()
{
	HK_ASSERT2(0x4345467e, false, "You chould not call this on an SPU.");
}

#else // !HK_PLATFORM_PS3SPU, so PPU or debug SPU job on on a PC platform

void hkSpuInitMonitors( char* buffer, hkUint32 bufferSize )
{
	// set the static monitor stream buffer
	hkMonitorStream::getInstance().setStaticBuffer( buffer, bufferSize );
}

static char* _findString( hkUint32 strID, hkSpuMonitorCache::StringLookup* localStringTable, int localStringTableSize )
{
	hkSpuMonitorCache::StringLookup* curEntry = localStringTable;
	hkSpuMonitorCache::StringLookup* endEntry = (hkSpuMonitorCache::StringLookup*)( ((char*)curEntry)+localStringTableSize );
	while ( curEntry && curEntry->id && (curEntry != endEntry) )
	{
		if (curEntry->id == 0xBDBDBDBD)
		{
			++curEntry; //padding entry
			continue;
		}

		if ( strID == curEntry->id)
		{
			return (char*)(curEntry + 1); // have our string
		}
		
		char* se = (char*)(++curEntry);
		
		// find next entry: 
		while (*se)
		{
			++se; // skip str chars
		}

		++se; // skip null

		// skip padding, if any (should check to see if we overflow here too..)
		while ( *(unsigned char*)se == 0xBD )
		{
			++se;
		}

		curEntry = (hkSpuMonitorCache::StringLookup*)se;
	}

	return HK_NULL;
}

struct _Command32
{
	hkUint32 m_commandAndMonitor;

	void reflectCmd(const char* cmdStr)
	{
		hkMonitorStream& stream = hkMonitorStream::getInstance();
		if ( stream.memoryAvailable() )
		{
			hkMonitorStream::Command* h = reinterpret_cast<hkMonitorStream::Command*>(stream.getEnd());	
			h->m_commandAndMonitor = cmdStr;
			stream.setEnd( (char*)(h+1) );
		}
	}
};

struct _AddValueCommand32 : public _Command32
{
	float m_value;
	
	void reflect(const char* cmdStr)
	{
		hkMonitorStream& stream = hkMonitorStream::getInstance();
		if ( stream.memoryAvailable() )
		{
			hkMonitorStream::AddValueCommand* h = reinterpret_cast<hkMonitorStream::AddValueCommand*>(stream.getEnd());	
			h->m_commandAndMonitor = cmdStr;
			h->m_value = m_value;
			stream.setEnd( (char*)(h+1) );
		}
	}
};

struct _TimerCommand32 : public _Command32
{
	hkUint32	m_time0;
	hkUint32	m_time1;
	
	void reflectTC(const char* cmdStr)
	{
		hkMonitorStream& stream = hkMonitorStream::getInstance();
		if ( stream.memoryAvailable() )
		{
			hkMonitorStream::TimerCommand* h = reinterpret_cast<hkMonitorStream::TimerCommand*>(stream.getEnd());	
			h->m_commandAndMonitor = cmdStr;
			h->m_time0 = m_time0;
			h->m_time1 = m_time1;
			stream.setEnd( (char*)(h+1) );
		}
	}
};

struct _TimerBeginListCommand32 : public _TimerCommand32
{
	hkUint32 m_nameOfFirstSplit;
	void reflectTCL(const char* cmdStr, hkSpuMonitorCache::StringLookup* localStringTable, int localStringTableSize)
	{
		hkMonitorStream& stream = hkMonitorStream::getInstance();
		hkMonitorStream::TimerBeginListCommand* h = reinterpret_cast<hkMonitorStream::TimerBeginListCommand*>(stream.getEnd());	
		h->m_commandAndMonitor = cmdStr;
		h->m_time0 = m_time0;
		h->m_time1 = m_time1;
		h->m_nameOfFirstSplit = _findString(m_nameOfFirstSplit, localStringTable, localStringTableSize );
		stream.setEnd( (char*)(h+1) );
	}
};

static void _expandAndAddMonitors( char* frameStart, char* frameEnd, hkSpuMonitorCache::StringLookup* localStringTable, int localStringTableSize )
{
	char* current = frameStart;
	char* end = frameEnd;
	hkMonitorStream& stream = hkMonitorStream::getInstance();

	while ( (current < end) && stream.memoryAvailable() )
	{
		_Command32* command = reinterpret_cast<_Command32*>( current );
		char* realCommandStr = _findString(command->m_commandAndMonitor, localStringTable, localStringTableSize );
		HK_ASSERT( 0x0, realCommandStr );
		//printf(" cmd[%s]\n", realCommandStr);

		switch( realCommandStr[0] )
		{
		case 'S':		// split list
		case 'E':		// timer end
		case 'l':		// list end
		case 'T':		// timer begin
			{
				_TimerCommand32* com = reinterpret_cast<_TimerCommand32*>( current );
				com->reflectTC(realCommandStr);
				current = (char *)(com+1);
				break;
			}

		case 'L':		// timer list begin
			{
				_TimerBeginListCommand32* com = reinterpret_cast<_TimerBeginListCommand32*>( current );
				com->reflectTCL(realCommandStr, localStringTable, localStringTableSize);
				current = (char *)(com+1);
				break;
			}

		case 'M':
			{
				_AddValueCommand32* com = reinterpret_cast<_AddValueCommand32*>( current );
				com->reflect(realCommandStr);
				current = (char *)(com+1);
				break;
			}
		case 'P':
		case 'p':
		case 'N':
			{
				_Command32* com = reinterpret_cast<_Command32*>( current );
				com->reflectCmd(realCommandStr);
				current = (char *)(com+1);
				break;
			}

		default:
			HK_ASSERT2(0x5120d10a, 0, "Inconsistent Monitor capture data" ); 	
			return;
		}
	}
}

void hkSpuMonitorCache::addToMonitorStreamAndResetPpu()
{
	m_dataLock.enter();

	char* cacheStart = m_start;
	char* cacheEnd = m_current;
	int cacheSize = int(cacheEnd - cacheStart);
	
	char* strCacheStart = (char*)m_stringTableStart;
	char* strCacheEnd = (char*)m_stringTableCurrent;
	int strCacheSize = int(strCacheEnd - strCacheStart);

	if (cacheSize > 0)
	{
		// all pointers (to strings) in the stream are 32bit.
		// our PPU, that will analyse the results, is 64bit
		// and the strings need to the fixed up to, so can't just memcpy..
		
		//printf("Have %d bytes of timers and have %d bytes of strings.\n", cacheSize, strCacheSize);

		// run through the timer stream and replace all 32bit ids with 64bit ptrs into the 
		// cached string table
		_expandAndAddMonitors( cacheStart, cacheEnd, (hkSpuMonitorCache::StringLookup*)strCacheStart, strCacheSize );
	}

	// Do not reset. This is a bit of a hack, to continue to show timers even if the SPU has not supplied new ones.
	//m_current = m_start;

	m_dataLock.leave();
}

#endif // specific PPU / SPU code


//
// Common Code
//

static bool _insertString(const char* str, hkSpuMonitorCache::StringLookup* localStringTable, int localStringTableSize, hkSpuMonitorCache::StringLookup*& newEntry, int& newEntryBytes)
{
	hkSpuMonitorCache::StringLookup* curEntry = localStringTable;
	hkSpuMonitorCache::StringLookup* endEntry = (hkSpuMonitorCache::StringLookup*)( ((char*)curEntry)+localStringTableSize );
	while ( curEntry && curEntry->id && (curEntry != endEntry) )
	{
		if (curEntry->id == 0xBDBDBDBD)
		{
			++curEntry; //padding entry
			continue;
		}

		if ( (hkUlong)str == curEntry->id)
		{
			return false; // no new
		}
		
		char* se = (char*)(++curEntry);
		
		// find next entry: 
		while (*se)
		{
			++se; // skip str chars
		}
		++se; // skip null
		
		// skip padding, if any (should check to see if we overflow here too..)
		while ( *(unsigned char*)se == 0xBD ){	++se;	}

		curEntry = (hkSpuMonitorCache::StringLookup*)se;
	}

	// past end?
	if (curEntry == endEntry)
	{
		return false; // no new
	}

	// pad to align
	bool firstNewEntry = (newEntry == HK_NULL); 
	while(firstNewEntry && ((hkUlong)curEntry & 0xf))
	{
		// This is to locally pad up, so that out start addr is 16byte aligned for transfer
		curEntry->id = 0xBDBDBDBD; // 4 char pads onto the end of last str (if not last str then should be aligned already..)
		++curEntry;
	}
	
	// have a null entry (assumes str table is null initialized), and have not found ours yet
	curEntry->id = int((hkUlong)str);
	newEntry = curEntry;
	++curEntry;
	
	int si=0;
	hkUchar* ts = (hkUchar*)curEntry;
	// don't want to link in string funcs etc, so do it by hand
	while (str[si])
	{
		int index = si++;
		ts[index] = str[index];
	}
	ts[si++] = 0x00; // null term

	while ((si & 0x03) != 0) // index (start of next entry at this stage) must always be mutliple of 4
	{
		ts[si++] = 0xBD;
	}

	newEntryBytes = si + 4; // 4 == sizeof(int32 id)
	return true;
}

static int _extractStrings( const char* frameStart, const char* frameEnd, hkSpuMonitorCache::StringLookup* localStringTable, int localStringTableSize, hkSpuMonitorCache::StringLookup*& lowestNewEntry  )
{
	const char* current = frameStart;
	const char* end = frameEnd;

	lowestNewEntry = HK_NULL;
	hkSpuMonitorCache::StringLookup* tempNewEntry = HK_NULL;
	int totalNewBytes = 0;
	int tempNewBytes;

	while ( current < end )
	{
		const hkMonitorStream::Command* command = reinterpret_cast<const hkMonitorStream::Command*>( current );
		char* str = const_cast<char*>(command->m_commandAndMonitor);
		if (_insertString(str, localStringTable, localStringTableSize, tempNewEntry, tempNewBytes))
		{
			if (lowestNewEntry == HK_NULL)
				lowestNewEntry = tempNewEntry;
			totalNewBytes += tempNewBytes;
		}

		switch( command->m_commandAndMonitor[0] )
		{
		case 'S':		// split list
		case 'E':		// timer end
		case 'l':		// list end
		case 'T':		// timer begin
			{
				const hkMonitorStream::TimerCommand* com = reinterpret_cast<const hkMonitorStream::TimerCommand*>( current );
				current = (const char *)(com+1);
				break;
			}

		case 'L':		// timer list begin
			{
				const hkMonitorStream::TimerBeginListCommand* com = reinterpret_cast<const hkMonitorStream::TimerBeginListCommand*>( current );
				char* n = const_cast<char*>(com->m_nameOfFirstSplit);
				if (_insertString(n, localStringTable, localStringTableSize, tempNewEntry, tempNewBytes))
				{
					if (lowestNewEntry == HK_NULL)
						lowestNewEntry = tempNewEntry;
					totalNewBytes += tempNewBytes;
				}

				current = (const char *)(com+1);
				break;
			}

		case 'M':
			{
				const hkMonitorStream::AddValueCommand* com = reinterpret_cast<const hkMonitorStream::AddValueCommand*>( current );
				current = (const char *)(com+1);
				break;
			}
		case 'P':
		case 'p':
		case 'N':
			{
				const hkMonitorStream::Command* com = reinterpret_cast<const hkMonitorStream::Command*>( current );
				current = (const char *)(com+1);
				break;
			}

		default:
			HK_ASSERT2(0x5120d10a, 0, "Inconsistent Monitor capture data" ); 	return 0;
		}
	}

	return totalNewBytes;
}

#if defined(HK_PLATFORM_POTENTIAL_SPU)

#include <hkbase/thread/hkSpuDmaManager.h>

void hkSpuMonitorCache::clearTimersSpu( HK_CPU_PTR(hkSpuMonitorCache*) effectiveAddr )
{
	m_dataLock.enter();
	HK_ALIGN( HK_CPU_PTR(char*) start, 16);
	start = m_start;
	hkSpuDmaManager::putToMainMemorySmallAndWaitForCompletion( effectiveAddr, &start, sizeof( HK_CPU_PTR(char*) ), hkSpuDmaManager::WRITE_NEW );
	hkSpuDmaManager::performFinalChecks( effectiveAddr, &start, sizeof( HK_CPU_PTR(char*) ) );

	m_dataLock.leave();
}

void hkSpuMonitorCache::getStringTableSpu( hkSpuMonitorCache::StringLookup* localStringTable )
{
	m_dataLock.enter();

	int stringTableSize = (int)((char*)m_stringTableCurrent - (char*)m_stringTableStart);
	if (stringTableSize > 0)
	{
		hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( localStringTable, m_stringTableStart, stringTableSize, hkSpuDmaManager::READ_COPY );
		hkSpuDmaManager::performFinalChecks( m_stringTableStart, localStringTable, stringTableSize );
	}

	m_dataLock.leave();
}


// Have this in the common code section as it can be used debugging on PC potentially
void hkSpuMonitorCache::sendToMainMemoryAndResetSpu( HK_CPU_PTR(hkSpuMonitorCache*) effectiveAddr, hkSpuMonitorCache::StringLookup* localStringTable, int localStringTableSize )
{
	hkSpuMonitorCache* cache = this;
	hkMonitorStream& stream = hkMonitorStream::getInstance();
	char* spuStart = stream.getStart();
	char* spuEnd = stream.getEnd();
	
	// check and pad for alignment (16bytes for general DMA)
	// the timer commands are all multiples of 4 bytes, so we need at most 3 more cmds (NOPs, each 4 byte ptrs on SPU)
	// This is not for the transfer now, but for transfers after this one 
	// that will tag on where this one ends.
	while ( (((hkUlong)spuEnd) & 0x0f) != 0)
	{
		HK_MONITOR_NOP();
		spuEnd = stream.getEnd();
	}
	
	int bytes = int(spuEnd - spuStart);
	if (bytes < 1)
	{
		//HK_SPU_DEBUG_PRINTF(("No mon streams.\n"));
		return;
	}

	//HK_SPU_DEBUG_PRINTF(("Extracting monitor strings from the stream..\n"));

	// TODO: Any new strings added cause the whole string table to be sent again
	//       We could just send any new strings
	hkSpuMonitorCache::StringLookup* strStart;
	int strBytes = _extractStrings(spuStart, spuEnd, localStringTable, localStringTableSize, strStart);
	while ( strBytes & 0xf )
	{
		// need to maker sure we keep alignment of sections, so pad up with auto char padding until our section
		// ends on alignment (so as to fit in with next section to download after this one.
		hkUchar* cstr = ((hkUchar*)strStart) + strBytes;
		*cstr = 0xBD; //special pad char
		++strBytes;
	}

	//HK_SPU_DEBUG_PRINTF(("Waiting for mon lock..\n"));

	cache->m_dataLock.enter();

	HK_CPU_PTR(char*) strCurrentEA = ( HK_CPU_PTR(char*) )( ((char*)effectiveAddr) + HK_OFFSET_OF(hkSpuMonitorCache, m_stringTableCurrent) );

	// DMA up the actual current pointer to the real cache in mem (not the local copy on this SPU)
	// TODO: make a small dma etc:
	//hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &(cache->m_current), effectiveAddr, sizeof( HK_CPU_PTR(char*) ) );
	HK_CPU_PTR(char*) mainStart = cache->m_start; // assumed not to change	
	HK_CPU_PTR(char*) mainEnd = cache->m_end; // assumed not to change	
	int freeBytes = int(mainEnd - mainStart);
	if (freeBytes < bytes)
	{
		bytes = freeBytes;
	}
	
	//HK_SPU_DEBUG_PRINTF(("Have mon lock, %d mon bytes free\n", freeBytes));

	cache->m_current = mainStart + bytes;

	// TODO: make a small dma etc:
	hkSpuDmaManager::putToMainMemorySmallAndWaitForCompletion( effectiveAddr, &(cache->m_current), sizeof( HK_CPU_PTR(char*) ), hkSpuDmaManager::WRITE_NEW );
	hkSpuDmaManager::performFinalChecks(  effectiveAddr, &(cache->m_current), sizeof( HK_CPU_PTR(char*) ) );

	HK_CPU_PTR(char*) mainStrStart = HK_NULL;
			
	if (strBytes > 0) // do we have any new string entries?
	{
		hkSpuDmaManager::getFromMainMemorySmallAndWaitForCompletion( &(cache->m_stringTableCurrent), strCurrentEA, sizeof( HK_CPU_PTR(char*) ), hkSpuDmaManager::READ_WRITE );
		mainStrStart = ( HK_CPU_PTR(char*) )( cache->m_stringTableCurrent ); 
		HK_CPU_PTR(char*) mainStrEnd = ( HK_CPU_PTR(char*) )(cache->m_stringTableEnd); // assumed not to change	
		int freeStrBytes = int(mainStrEnd - mainStrStart);
		if (freeStrBytes < strBytes)
		{
			strBytes = freeStrBytes;
		}

		cache->m_stringTableCurrent  = hkAddByteOffsetCpuPtr(cache->m_stringTableCurrent, strBytes);

		hkSpuDmaManager::putToMainMemorySmallAndWaitForCompletion( strCurrentEA, &(cache->m_stringTableCurrent), sizeof( HK_CPU_PTR(char*) ), hkSpuDmaManager::WRITE_BACK );
		hkSpuDmaManager::performFinalChecks(strCurrentEA, &(cache->m_stringTableCurrent), sizeof( HK_CPU_PTR(char*) ));
	}


	// dma it down
	if (bytes > 0)
	{
		//HK_SPU_DEBUG_PRINTF(("DMA %d bytes.\n", bytes));
		hkSpuDmaManager::putToMainMemoryAndWaitForCompletion( mainStart, spuStart, bytes, hkSpuDmaManager::WRITE_NEW );
		hkSpuDmaManager::performFinalChecks(mainStart, spuStart, bytes);
	}
	if (strBytes > 0)
	{
		hkSpuDmaManager::putToMainMemoryAndWaitForCompletion( mainStrStart, strStart, strBytes, hkSpuDmaManager::WRITE_NEW );
		hkSpuDmaManager::performFinalChecks(mainStrStart, strStart, strBytes);
	}

	cache->m_dataLock.leave();

	stream.reset();

	//HK_SPU_DEBUG_PRINTF(("Done with monitors.\n", bytes));
}

#endif

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
