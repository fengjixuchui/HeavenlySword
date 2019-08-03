/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkvisualize/hkVisualize.h>

#include <hkbase/stopwatch/hkStopwatch.h>
#include <hkbase/monitor/hkMonitorStreamAnalyzer.h>
#include <hkbase/stream/impl/hkArrayStreamWriter.h>

#include <hkvisualize/hkProcessFactory.h>
#include <hkvisualize/hkProcessContext.h>
#include <hkvisualize/process/hkStatisticsProcess.h>
#include <hkvisualize/hkDebugDisplayHandler.h>
#include <hkvisualize/serialize/hkObjectSerialize.h>
#include <hkvisualize/serialize/hkDisplaySerializeOStream.h>

int hkStatisticsProcess::m_tag = 0;

hkProcess* HK_CALL hkStatisticsProcess::create(const hkArray<hkProcessContext*>& contexts)
{
	return new hkStatisticsProcess(contexts); // doesn't require a context (the monitors are global)
}

void HK_CALL hkStatisticsProcess::registerProcess()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkStatisticsProcess::hkStatisticsProcess(const hkArray<hkProcessContext*>& contexts)
: hkProcess( true) /* user selectable */
{
	// grab the data from the monitors

	hkMonitorStreamFrameInfo frameInfo;
	frameInfo.m_heading = HK_NULL;

	// XX:  make VDB client able to actively choose timers.
#ifdef HK_PLATFORM_PS2
	// take the first two.. whatever they are. Change this to report dcache etc. 
	frameInfo.m_absoluteTimeCounter = hkMonitorStreamFrameInfo::ABSOLUTE_TIME_TIMER_1;
	frameInfo.m_indexOfTimer0 = 0;
	frameInfo.m_indexOfTimer1 = 1;
	frameInfo.m_timerFactor0 = 0;
	frameInfo.m_timerFactor1 =  1e3f / 300e6f;
#else
	frameInfo.m_absoluteTimeCounter = hkMonitorStreamFrameInfo::ABSOLUTE_TIME_TIMER_0;
	frameInfo.m_indexOfTimer0 = 0; // just the one timer.
	frameInfo.m_indexOfTimer1 = 1;
	frameInfo.m_timerFactor0 = 1e3f / float(hkStopwatch::getTicksPerSecond());
	frameInfo.m_timerFactor1 = 1.0f;
#endif

	// info on what is in the stream:
	{
		hkArray<hkObjectSerialize::GlobalFixup> globalFixups;
		hkArrayStreamWriter infoWriter(&m_infoBuffer, hkArrayStreamWriter::ARRAY_BORROW);
		hkObjectSerialize::writeObject( &infoWriter, hkStructureLayout::MsvcWin32LayoutRules, 0x1, &frameInfo, hkMonitorStreamFrameInfoClass, globalFixups);
		HK_ASSERT2(0x54e4565e, globalFixups.getSize() == 0, "Monitor Stream Info should not have external ptrs!");
	}

	// keep the contexts
	// Any one of which could provide the per thread timers
	m_contexts = contexts;
}

hkStatisticsProcess::~hkStatisticsProcess()
{

}

void hkStatisticsProcess::step(hkReal frameTimeInMs)
{
	if (!m_outStream)
		return; // nothing to write to

	hkArray<hkObjectSerialize::GlobalFixup> globalFixups;

	// see if we have any per thread timers:
	hkArray<char*> starts;
	hkArray<char*> ends;
	for (int ci=0; ci < m_contexts.getSize(); ++ci)
	{	
		if (m_contexts[ci]->m_monitorStreamBegins.getSize() > 0)
		{
			starts =  m_contexts[ci]->m_monitorStreamBegins;
			ends = m_contexts[ci]->m_monitorStreamEnds;
			break;
		}
	}

	if (starts.getSize() == 0)
	{
		// data to send (raw mon stream)
		hkMonitorStream& stream = hkMonitorStream::getInstance();
		char* monStreamBegin = stream.getStart();
		char* monStreamEnd = stream.getEnd();
		starts.pushBack(monStreamBegin);
		ends.pushBack(monStreamEnd);
	}

	
	// build a string map for the stream (the whole set of monitors as they should all 
	// share a good few strings)
	m_strBuffer.setSize(0);
	int totalMonLen = 0;
	{
		hkPointerMap<void*, char*> strPtrMap;
		hkMonitorStreamStringMap strMap;

		// for all threads
		for (int ms=0; ms < starts.getSize(); ++ms)
		{
			hkMonitorStreamAnalyzer::extractStringMap(starts[ms], ends[ms], strPtrMap);
			totalMonLen += static_cast<int>( ends[ms] - starts[ms] ); 
		}

		// convert hash table into a compact array
		for (hkPointerMap<void*, char*>::Iterator itr = strPtrMap.getIterator(); strPtrMap.isValid(itr); itr = strPtrMap.getNext(itr) )
		{
			hkMonitorStreamStringMap::StringMap& newEntry = strMap.m_map.expandOne();
			newEntry.m_id = reinterpret_cast<hkUint64>( strPtrMap.getKey(itr) ); // id (ptr on Server)
			newEntry.m_string = strPtrMap.getValue(itr); // string
		}

		// save to a buffer
		hkArrayStreamWriter strWriter(&m_strBuffer, hkArrayStreamWriter::ARRAY_BORROW);
		hkObjectSerialize::writeObject( &strWriter, hkStructureLayout::MsvcWin32LayoutRules, 0x1, &strMap, hkMonitorStreamStringMapClass, globalFixups);
		HK_ASSERT2(0x15456e56, globalFixups.getSize() == 0, "String Map should not have external ptrs!");
	}

	if (totalMonLen < 1)
		return;

	// have the info stream already from the ctor
	
	// work out full packet size
	int numStreams = starts.getSize();
	int infoBufSize = m_infoBuffer.getSize();
	int strBufSize = m_strBuffer.getSize();
	const int packetSize = 1 /*command id*/ + (totalMonLen + (4 * numStreams /*stream len*/) + 4/*num streams*/) + (strBufSize + 4/*len int*/) + (infoBufSize + 4/*len int*/);

	m_outStream->write32u(packetSize);
	m_outStream->write8u(hkStatisticsProcess::HK_SEND_STATISTICS_DUMP);

		// The frame info 
	m_outStream->write32(infoBufSize);
	if (infoBufSize > 0)
		m_outStream->writeRaw(m_infoBuffer.begin(), infoBufSize);
	
		// The string map
	m_outStream->write32(strBufSize);
	if (strBufSize > 0)
		m_outStream->writeRaw(m_strBuffer.begin(), strBufSize);

		// The large data stream(s):
		// num streams * [streamlen, stream data]
	m_outStream->write32(numStreams);
	for (int si=0; si < numStreams; ++si)
	{
		int monLen = static_cast<int>( ends[si] - starts[si] ); 
		m_outStream->write32(monLen);
		if (monLen > 0)
			m_outStream->writeRaw(starts[si], monLen);
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
