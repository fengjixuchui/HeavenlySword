/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkmath/hkMath.h>
#include <hkutilities/visualdebugger/viewer/hkRigidBodyCentreOfMassViewer.h>
#include <hkvisualize/hkDebugDisplayHandler.h>
#include <hkvisualize/hkProcessFactory.h>

#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>

#include <hkvisualize/hkDebugDisplay.h>
#include <hkvisualize/type/hkColor.h>

int hkRigidBodyCentreOfMassViewer::m_tag = 0;

hkProcess* HK_CALL hkRigidBodyCentreOfMassViewer::create(const hkArray<hkProcessContext*>& contexts)
{
	return new hkRigidBodyCentreOfMassViewer(contexts);
}

void HK_CALL hkRigidBodyCentreOfMassViewer::registerViewer()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkRigidBodyCentreOfMassViewer::hkRigidBodyCentreOfMassViewer(const hkArray<hkProcessContext*>& contexts)
: hkWorldViewerBase( contexts )
{
}

void hkRigidBodyCentreOfMassViewer::init()
{
	if (m_context)
	{
		for( int i=0; i < m_context->getNumWorlds(); ++i)
		{
			addWorld( m_context->getWorld(i) );
		}
	}
}

hkRigidBodyCentreOfMassViewer::~hkRigidBodyCentreOfMassViewer()
{
	if (m_context)
	{
		for( int i=0; i < m_context->getNumWorlds(); ++i)
		{
			removeWorld( m_context->getWorld(i) );
		}
	}
}

void hkRigidBodyCentreOfMassViewer::worldRemovedCallback( hkWorld* world )
{
	removeWorld(world);
}

void hkRigidBodyCentreOfMassViewer::worldAddedCallback( hkWorld* world )
{
	addWorld(world);
}

void hkRigidBodyCentreOfMassViewer::removeWorld(hkWorld* world)
{
	world->markForWrite();
	
	world->removeEntityListener( this );
	world->removeWorldPostSimulationListener( this );
	
	// get all the active entities from the active simulation islands
	{
		const hkArray<hkSimulationIsland*>& activeIslands = world->getActiveSimulationIslands();

		for(int i = 0; i < activeIslands.getSize(); i++)
		{
			const hkArray<hkEntity*>& activeEntities = activeIslands[i]->getEntities();
			for(int j = 0; j < activeEntities.getSize(); j++)
			{
				entityRemovedCallback( activeEntities[j] );
			}
		}
	}

	// get all the inactive entities from the inactive simulation islands
	{
		const hkArray<hkSimulationIsland*>& inactiveIslands = world->getInactiveSimulationIslands();

		for(int i = 0; i < inactiveIslands.getSize(); i++)
		{
			const hkArray<hkEntity*>& activeEntities = inactiveIslands[i]->getEntities();
			for(int j = 0; j < activeEntities.getSize(); j++)
			{
				entityRemovedCallback( activeEntities[j] );
			}
		}
	}

	// get all the fixed bodies in the world
	if (world->getFixedIsland())
	{
		const hkArray<hkEntity*>& fixedEntities = world->getFixedIsland()->getEntities();
		for(int j = 0; j < fixedEntities.getSize(); j++)
		{
			entityRemovedCallback( fixedEntities[j] );
		}
	}

	world->unmarkForWrite();
}

void hkRigidBodyCentreOfMassViewer::addWorld(hkWorld* world)
{
	world->markForWrite();
	
	world->addEntityListener( this );
	world->addWorldPostSimulationListener( this );

	// get all the active entities from the active simulation islands
	{
		const hkArray<hkSimulationIsland*>& activeIslands = world->getActiveSimulationIslands();

		for(int i = 0; i < activeIslands.getSize(); i++)
		{
			const hkArray<hkEntity*>& activeEntities = activeIslands[i]->getEntities();
			for(int j = 0; j < activeEntities.getSize(); j++)
			{
				entityAddedCallback( activeEntities[j] );
			}
		}
	}

	// get all the inactive entities from the inactive simulation islands
	{
		const hkArray<hkSimulationIsland*>& inactiveIslands = world->getInactiveSimulationIslands();

		for(int i = 0; i < inactiveIslands.getSize(); i++)
		{
			const hkArray<hkEntity*>& activeEntities = inactiveIslands[i]->getEntities();
			for(int j = 0; j < activeEntities.getSize(); j++)
			{
				entityAddedCallback( activeEntities[j] );
			}
		}
	}


	// get all the fixed bodies in the world
	if (world->getFixedIsland())
	{
		const hkArray<hkEntity*>& fixedEntities = world->getFixedIsland()->getEntities();
		for(int j = 0; j < fixedEntities.getSize(); j++)
		{
			entityAddedCallback( fixedEntities[j] );
		}
	}

	world->unmarkForWrite();
}

void hkRigidBodyCentreOfMassViewer::entityAddedCallback( hkEntity* entity )
{
//	if(entity->getType() == HK_RIGID_BODY)
//	{
		hkRigidBody* rigidBody = static_cast<hkRigidBody*>(entity);
		m_entitiesCreated.pushBack(rigidBody);
//	}
}

void hkRigidBodyCentreOfMassViewer::entityRemovedCallback( hkEntity* entity )
{
//	if( entity->getType() == HK_RIGID_BODY )
//	{
		hkRigidBody* rigidBody = static_cast<hkRigidBody*>(entity);

		// remove the id from the list of 'owned' created entities
		const int index = m_entitiesCreated.indexOf(rigidBody);
	//	HK_ASSERT2(0x4bba802b, index != -1, "Trying to remove body which hkRigidBodyCentreOfMassViewer does not think has been added!");
		if(index >= 0)
		{
			m_entitiesCreated.removeAt(index);
		}
//	}

}

void hkRigidBodyCentreOfMassViewer::postSimulationCallback( hkWorld* world )
{
	HK_TIMER_BEGIN("hkRigidBodyCentreOfMassViewer", this);

	for(int i = 0; i < m_entitiesCreated.getSize(); i++)
	{
		const hkReal mass = m_entitiesCreated[i]->getMass();
		if(mass != 0.0f)
		{
			HK_TIMER_BEGIN("getMassAndLines", this);
			hkVector4 centreOfMass = m_entitiesCreated[i]->getCenterOfMassInWorld();

			hkVector4 xAxis, yAxis, zAxis;

			xAxis.setAdd4(centreOfMass, m_entitiesCreated[i]->getTransform().getRotation().getColumn(0));
			yAxis.setAdd4(centreOfMass, m_entitiesCreated[i]->getTransform().getRotation().getColumn(1));
			zAxis.setAdd4(centreOfMass, m_entitiesCreated[i]->getTransform().getRotation().getColumn(2));
			HK_TIMER_END();
			HK_TIMER_BEGIN("display3lines", this);
			m_displayHandler->displayLine(centreOfMass, xAxis, hkColor::RED, m_tag);
			m_displayHandler->displayLine(centreOfMass, yAxis, hkColor::GREEN, m_tag);
			m_displayHandler->displayLine(centreOfMass, zAxis, hkColor::BLUE, m_tag);
			HK_TIMER_END();
		}
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
