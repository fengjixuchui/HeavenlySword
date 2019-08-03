/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkmath/hkMath.h>
#include <hkbase/debugutil/hkStreamStatisticsCollector.h>

#include <hkdynamics/world/hkWorld.h>

//#define HK_DISABLE_DEBUG_DISPLAY
#include <hkvisualize/hkProcessFactory.h>
#include <hkvisualize/hkDebugDisplayHandler.h>
#include <hkutilities/visualdebugger/viewer/hkWorldMemoryViewer.h>

int hkWorldMemoryViewer::m_tag = 0;

void HK_CALL hkWorldMemoryViewer::registerViewer()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkProcess* HK_CALL hkWorldMemoryViewer::create(const hkArray<hkProcessContext*>& contexts)
{
	return new hkWorldMemoryViewer(contexts);
}

hkWorldMemoryViewer::hkWorldMemoryViewer(const hkArray<hkProcessContext*>& contexts)
: hkWorldViewerBase( contexts), m_collector(500000)
{
	
}

void hkWorldMemoryViewer::init()
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
			worldAddedCallback( m_context->getWorld(i));
	}
}

hkWorldMemoryViewer::~hkWorldMemoryViewer()
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			worldRemovedCallback( m_context->getWorld(i));
		}
	}
}

void hkWorldMemoryViewer::step( hkReal frameTimeInMs )
{
	m_collector.reset();
	m_collector.beginSnapshot(m_collector.MEMORY_ALL);
	for (int w=0; w < m_context->getNumWorlds(); ++w)
	{
		hkWorld* world = m_context->getWorld(w);
		world->markForWrite(); // accessing the broadphase data currently requires RW access.
		m_collector.addChildObject("World", m_collector.MEMORY_ENGINE, world );
		world->unmarkForWrite();

	}
	m_collector.endSnapshot();

	if (m_collector.getDataStream())
	{
		m_displayHandler->sendMemStatsDump( m_collector.getDataStream(), m_collector.getDataStreamSize() );
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
