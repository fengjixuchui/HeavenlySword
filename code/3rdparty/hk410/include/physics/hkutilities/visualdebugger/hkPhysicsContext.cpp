/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/world/hkPhysicsSystem.h>
#include <hkdynamics/world/listener/hkWorldDeletionListener.h>
#include <hkdynamics/constraint/hkConstraintData.h>

#include <hkutilities/visualdebugger/hkPhysicsContext.h>
#include <hkutilities/visualdebugger/viewer/hkBroadphaseViewer.h>
#include <hkutilities/visualdebugger/viewer/hkConstraintViewer.h>
#include <hkutilities/visualdebugger/viewer/hkContactPointViewer.h>
#include <hkutilities/visualdebugger/viewer/hkForcedContactPointViewer.h>
#include <hkutilities/visualdebugger/viewer/hkPhantomDisplayViewer.h>
#include <hkutilities/visualdebugger/viewer/hkRigidBodyCentreOfMassViewer.h>
#include <hkutilities/visualdebugger/viewer/hkRigidBodyInertiaViewer.h>
#include <hkutilities/visualdebugger/viewer/hkShapeDisplayViewer.h>
#include <hkutilities/visualdebugger/viewer/hkConvexRadiusViewer.h>
#include <hkutilities/visualdebugger/viewer/hkSweptTransformDisplayViewer.h>
#include <hkutilities/visualdebugger/viewer/hkSimulationIslandViewer.h>
#include <hkutilities/visualdebugger/viewer/hkMousePickingViewer.h>
#include <hkutilities/visualdebugger/viewer/hkWorldMemoryViewer.h>
#include <hkutilities/visualdebugger/viewer/hkVehicleViewer.h>

#include <hkvisualize/hkVisualDebugger.h>

void HK_CALL hkPhysicsContext::registerAllPhysicsProcesses()
{
	hkBroadphaseViewer::registerViewer();
	hkConstraintViewer::registerViewer();
	hkContactPointViewer::registerViewer();

//  Forced viewer not registered by default as it alters the simulation
//  by changing the contact point processing. For advanced users only.
    hkForcedContactPointViewer::registerViewer(); 

	hkVehicleViewer::registerViewer();
	hkPhantomDisplayViewer::registerViewer();
	hkRigidBodyCentreOfMassViewer::registerViewer();
	hkShapeDisplayViewer::registerViewer();
	hkConvexRadiusViewer::registerViewer();
	hkSweptTransformDisplayViewer::registerViewer();
	hkRigidBodyInertiaViewer::registerViewer();
	hkSimulationIslandViewer::registerViewer();
	hkMousePickingViewer::registerViewer();
	hkWorldMemoryViewer::registerViewer();
}

hkPhysicsContext::hkPhysicsContext()
{
	
}

hkPhysicsContext::~hkPhysicsContext()
{
	for (int w=(m_worlds.getSize()-1); w >=0 ; --w)
	{
		removeWorld( m_worlds[w] );
	}
}

void hkPhysicsContext::setOwner(hkVisualDebugger* vdb)
{
	if (m_owner)
	{
		for (int wi=0;wi < m_worlds.getSize(); ++wi)
			removeFromInspection(m_worlds[wi]);	
	}

	m_owner = vdb;

	if (vdb)
	{
		for (int i=0;i < m_worlds.getSize(); ++i)
			addForInspection( m_worlds[i] );	
	}
}

void hkPhysicsContext::removeWorld( hkWorld* oldWorld )
{
	int wi = m_worlds.indexOf(oldWorld);
	if (wi >= 0)
	{
		oldWorld->removeWorldDeletionListener(this);
		for (int i=0; i < m_addListeners.getSize(); ++i)
		{
			m_addListeners[i]->worldRemovedCallback(oldWorld);
		}
		m_worlds.removeAt(wi);

		removeFromInspection( oldWorld );
	}
}

void hkPhysicsContext::addWorld( hkWorld* newWorld )
{
	// make sure we don't have it already
	if (m_worlds.indexOf(newWorld) < 0)
	{
		newWorld->addWorldDeletionListener(this);
		m_worlds.pushBack(newWorld);

		for (int i=0; i < m_addListeners.getSize(); ++i)
		{
			m_addListeners[i]->worldAddedCallback( newWorld );
		}

		addForInspection( newWorld );
	}
}

// XXX get this from a reg so that we don't have to always affect the code footprint..
extern const hkClass hkEntityClass;
extern const hkClass hkPhantomClass;
extern const hkClass hkActionClass;
extern const hkClass hkConstraintInstanceClass;

void hkPhysicsContext::addForInspection(hkWorld* w)
{
	if (m_owner)
	{
		w->addEntityListener(this);
		w->addPhantomListener(this);
		w->addActionListener(this);
		w->addConstraintListener(this);

		// easiest to get the world to give us the info
		// (world itself is not a valid ptr for inspection
		//  as it has no class.. literally ;)
		hkPhysicsSystem* sys = w->getWorldAsOneSystem();
		const hkArray<hkRigidBody*>& rbs = sys->getRigidBodies();
		for (int ri=0; ri < rbs.getSize(); ++ri)
			entityAddedCallback( const_cast<hkRigidBody*>(rbs[ri]) );
		
		const hkArray<hkPhantom*>& phantoms = sys->getPhantoms();
		for (int pi=0; pi < phantoms.getSize(); ++pi)
			phantomAddedCallback( const_cast<hkPhantom*>(phantoms[pi]) );

		const hkArray<hkAction*>& actions = sys->getActions();
		for (int ai=0; ai < actions.getSize(); ++ai)
			actionAddedCallback( const_cast<hkAction*>(actions[ai]) );

		const hkArray<hkConstraintInstance*>& constraints = sys->getConstraints();
		for (int ci=0; ci < constraints.getSize(); ++ci)
			constraintAddedCallback( const_cast<hkConstraintInstance*>(constraints[ci]) );

		sys->removeReference();	
	}
}

void hkPhysicsContext::removeFromInspection(hkWorld* w)
{
	if (m_owner)
	{
		w->removeEntityListener(this);
		w->removePhantomListener(this);
		w->removeActionListener(this);
		w->removeConstraintListener(this);

		hkPhysicsSystem* sys = w->getWorldAsOneSystem();
		const hkArray<hkRigidBody*>& rbs = sys->getRigidBodies();
		for (int ri=0; ri < rbs.getSize(); ++ri)
			entityRemovedCallback(const_cast<hkRigidBody*>(rbs[ri]) );

		const hkArray<hkPhantom*>& phantoms = sys->getPhantoms();
		for (int pi=0; pi < phantoms.getSize(); ++pi)
			phantomRemovedCallback(const_cast<hkPhantom*>(phantoms[pi]) );

		const hkArray<hkAction*>& actions = sys->getActions();
		for (int ai=0; ai < actions.getSize(); ++ai)
			actionRemovedCallback(const_cast<hkAction*>(actions[ai]) );

		const hkArray<hkConstraintInstance*>& constraints = sys->getConstraints();
		for (int ci=0; ci < constraints.getSize(); ++ci)
			constraintRemovedCallback(const_cast<hkConstraintInstance*>(constraints[ci]) );

		sys->removeReference();
	}
}

int hkPhysicsContext::findWorld(hkWorld* world)
{
	return m_worlds.indexOf(world);
}

void hkPhysicsContext::worldDeletedCallback( hkWorld* world )
{
	removeWorld(world);
}

void hkPhysicsContext::entityAddedCallback( hkEntity* entity )
{
	if (m_owner)
		m_owner->addTrackedObject(entity, hkEntityClass, "Entities");
}

void hkPhysicsContext::entityRemovedCallback( hkEntity* entity )
{
	if (m_owner)
		m_owner->removeTrackedObject(entity);
}

void hkPhysicsContext::phantomAddedCallback( hkPhantom* phantom )
{
	if (m_owner)
		m_owner->addTrackedObject(phantom, hkPhantomClass, "Phantoms");
}

void hkPhysicsContext::phantomRemovedCallback( hkPhantom* phantom )
{
	if (m_owner)
		m_owner->removeTrackedObject(phantom);
}

void hkPhysicsContext::constraintAddedCallback( hkConstraintInstance* constraint )
{
	if (m_owner && constraint->getData() && constraint->getData()->getType() != hkConstraintData::CONSTRAINT_TYPE_CONTACT)
	{
		m_owner->addTrackedObject(constraint, hkConstraintInstanceClass, "Constraints");
	}
}

void hkPhysicsContext::constraintRemovedCallback( hkConstraintInstance* constraint )
{
	if (m_owner && constraint->getData() && constraint->getData()->getType() != hkConstraintData::CONSTRAINT_TYPE_CONTACT)
		m_owner->removeTrackedObject(constraint);
}

void hkPhysicsContext::actionAddedCallback( hkAction* action )
{
	if (m_owner)
		m_owner->addTrackedObject(action, hkActionClass, "Actions");
}

void hkPhysicsContext::actionRemovedCallback( hkAction* action )
{
	if (m_owner)
		m_owner->removeTrackedObject(action);
}


void hkPhysicsContext::addWorldAddedListener( hkPhysicsContextWorldListener* cb )
{
	if (m_addListeners.indexOf(cb)< 0)
	{
		m_addListeners.pushBack( cb );
	}
}

void hkPhysicsContext::removeWorldAddedListener( hkPhysicsContextWorldListener* cb )
{
	int index = m_addListeners.indexOf(cb);
	if (index >= 0 )
	{
		m_addListeners.removeAt( index );
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
