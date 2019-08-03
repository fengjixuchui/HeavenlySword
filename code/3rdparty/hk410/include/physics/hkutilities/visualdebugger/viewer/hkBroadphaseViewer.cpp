/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkutilities/visualdebugger/viewer/hkBroadphaseViewer.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkinternal/collide/broadphase/hkBroadPhase.h>

//#define HK_DISABLE_DEBUG_DISPLAY
#include <hkvisualize/hkDebugDisplay.h>
#include <hkvisualize/hkProcessFactory.h>
#include <hkutilities/visualdebugger/viewer/hkCollideDebugUtil.h>

int hkBroadphaseViewer::m_tag = 0;

void HK_CALL hkBroadphaseViewer::registerViewer()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkProcess* HK_CALL hkBroadphaseViewer::create(const hkArray<hkProcessContext*>& contexts)
{
	return new hkBroadphaseViewer(contexts);
}

hkBroadphaseViewer::hkBroadphaseViewer(const hkArray<hkProcessContext*>& contexts)
: hkWorldViewerBase( contexts )
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			hkWorld* w = m_context->getWorld(i);
			w->markForWrite();
			w->addWorldPostSimulationListener( this );
			w->unmarkForWrite();
		}
	}
}

void hkBroadphaseViewer::worldAddedCallback( hkWorld* world)
{
	world->markForWrite();
	world->addWorldPostSimulationListener( this );
	world->unmarkForWrite();

}

void hkBroadphaseViewer::worldRemovedCallback( hkWorld* world)
{
	world->markForWrite();
	world->removeWorldPostSimulationListener( this );
	world->unmarkForWrite();
}

void hkBroadphaseViewer::postSimulationCallback( hkWorld* world )
{
	HK_TIMER_BEGIN("hkBroadphaseViewer", this);

	hkVector4 dummy;	// MSVC6.0 alignment doesn't seem to work for the array below, so this
								// useless variable was necessary...
    //Had to switch to a simple hkArray, the inplace array caused an ICE on ps3-g++
	//hkInplaceArrayAligned16<hkAabb, 1024> allAabbs;
	hkArray<hkAabb> allAabbs(1024);
	allAabbs.setSizeUnchecked(1024);

	hkBroadPhase* broadPhase = world->getBroadPhase();

	broadPhase->getAllAabbs( allAabbs );
	if(allAabbs.getSize() > m_broadPhaseDisplayGeometries.getSize())
	{
		m_broadPhaseDisplayGeometries.setSize(allAabbs.getSize());
	}

	hkArray<hkDisplayGeometry*> displayGeometries;
	displayGeometries.setSize(allAabbs.getSize());

	// create display geometries 
	for(int i = allAabbs.getSize()-1; i >= 0; i--)
	{
		m_broadPhaseDisplayGeometries[i].setExtents(allAabbs[i].m_min, allAabbs[i].m_max);
		displayGeometries[i] = &(m_broadPhaseDisplayGeometries[i]);
	}

	m_displayHandler->displayGeometry(displayGeometries, hkColor::RED, m_tag);

	HK_TIMER_END();
}

hkBroadphaseViewer::~hkBroadphaseViewer()
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			hkWorld* w = m_context->getWorld(i);
			w->markForWrite();
			w->removeWorldPostSimulationListener( this );
			w->unmarkForWrite();
		}
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
