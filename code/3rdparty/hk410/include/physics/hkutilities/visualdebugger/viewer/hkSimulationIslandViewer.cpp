/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/visualdebugger/viewer/hkSimulationIslandViewer.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkcollide/shape/hkShape.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>
#include <hkinternal/collide/broadphase/hkBroadPhase.h>

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/entity/hkEntity.h>

//#define HK_DISABLE_DEBUG_DISPLAY
#include <hkutilities/visualdebugger/viewer/hkCollideDebugUtil.h>
#include <hkvisualize/hkProcessFactory.h>

int hkSimulationIslandViewer::m_tag = 0;

hkProcess* HK_CALL hkSimulationIslandViewer::create(const hkArray<hkProcessContext*>& contexts )
{
	return new hkSimulationIslandViewer(contexts);
}

void HK_CALL hkSimulationIslandViewer::registerViewer()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkSimulationIslandViewer::hkSimulationIslandViewer( const hkArray<hkProcessContext*>& contexts )
: hkWorldViewerBase(contexts),
	m_showActiveIslands(true),
	m_showInactiveIslands(true)
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			hkWorld* world = m_context->getWorld(i);
			world->markForWrite();
			world->addWorldPostSimulationListener( this );
			world->unmarkForWrite();
		}
	}
}

hkSimulationIslandViewer::~hkSimulationIslandViewer()
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i )
		{
			hkWorld* world = m_context->getWorld(i);
			world->markForWrite();
			world->removeWorldPostSimulationListener(this);
			world->unmarkForWrite();
		}
	}
}

void hkSimulationIslandViewer::worldAddedCallback( hkWorld* world )
{
	world->markForWrite();
		world->addWorldPostSimulationListener( this );
	world->unmarkForWrite();
}

void hkSimulationIslandViewer::worldRemovedCallback( hkWorld* world )
{
	world->markForWrite();
		world->removeWorldPostSimulationListener( this );
	world->unmarkForWrite();
}

void hkSimulationIslandViewer::postSimulationCallback( hkWorld* world )
{
	HK_TIMER_BEGIN("hkSimulationIslandViewer", this);

	//	hkprintf("island display...\n");
	if (m_showActiveIslands)
	{
		const hkArray<hkSimulationIsland*>& islands = world->getActiveSimulationIslands();
		if(islands.getSize() > m_activeIslandDisplayGeometries.getSize())
		{
			m_activeIslandDisplayGeometries.setSize(islands.getSize());
		}

		hkArray<hkDisplayGeometry*> displayGeometries;
		displayGeometries.setSize(islands.getSize());

		hkArray<hkAabb> islandAabbs;
		islandAabbs.setSize(islands.getSize());

		for ( int i = 0; i < islands.getSize(); ++i )
		{
			const hkArray<hkEntity*>& entities = islands[i]->getEntities();

			islandAabbs[i].m_max.setAll(-HK_REAL_MAX);
			islandAabbs[i].m_min.setAll(HK_REAL_MAX);

			// Create one aabb about all the entities.
			hkAabb aabb;

			hkVector4 minExtent;
			hkVector4 maxExtent;	
			maxExtent.setAll(-HK_REAL_MAX);
		    minExtent.setAll(HK_REAL_MAX);

			for ( int j = 0; j < entities.getSize(); ++j )
			{
				const hkCollidable* c = entities[j]->getCollidable();

				// hkCollidable may not have an hkShape.
				if (c->getShape() != HK_NULL)
				{
					world->getBroadPhase()->getAabb( c->getBroadPhaseHandle(), aabb );
					maxExtent.setMax4( maxExtent, aabb.m_max );
					minExtent.setMin4( minExtent, aabb.m_min );
				}
			}

			m_activeIslandDisplayGeometries[i].setExtents(minExtent, maxExtent);
			displayGeometries[i] = &m_activeIslandDisplayGeometries[i];
		}

		m_displayHandler->displayGeometry(displayGeometries, hkColor::BLUE, m_tag);
	}

	if (m_showInactiveIslands)
	{
		const hkArray<hkSimulationIsland*>& islands = world->getInactiveSimulationIslands();
	
		if(islands.getSize() > m_inactiveIslandDisplayGeometries.getSize())
		{
			m_inactiveIslandDisplayGeometries.setSize(islands.getSize());
		}

		hkArray<hkDisplayGeometry*> displayGeometries;
		displayGeometries.setSize(islands.getSize());

		for ( int i = 0; i < islands.getSize(); ++i )
		{
			const hkArray<hkEntity*>& entities = islands[i]->getEntities();

			// Create one aabb about all the entities.
			hkAabb aabb;

			hkVector4 minExtent;
			hkVector4 maxExtent;	
			maxExtent.setAll(-HK_REAL_MAX);
		    minExtent.setAll(HK_REAL_MAX);

			for ( int j = 0; j < entities.getSize(); ++j )
			{
				const hkCollidable* c = entities[j]->getCollidable();

				// hkCollidable may not have an hkShape.
				if (c->getShape() != HK_NULL)
				{
					world->getBroadPhase()->getAabb( c->getBroadPhaseHandle(), aabb );
					maxExtent.setMax4( maxExtent, aabb.m_max );
					minExtent.setMin4( minExtent, aabb.m_min );
				}
			}

			m_inactiveIslandDisplayGeometries[i].setExtents(minExtent, maxExtent);
			displayGeometries[i] = &m_inactiveIslandDisplayGeometries[i];
		}

		m_displayHandler->displayGeometry(displayGeometries, hkColor::GREEN, m_tag);

	}

	HK_TIMER_END();

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
