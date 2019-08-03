/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkdynamics/hkDynamics.h>

#include <hkdynamics/world/broadphaseborder/hkBroadPhaseBorder.h>
#include <hkdynamics/phantom/hkAabbPhantom.h>
#include <hkdynamics/entity/hkRigidBody.h>


//
// Broad phase border implementation
//

hkBroadPhaseBorder::hkBroadPhaseBorder( hkWorld* world, hkWorldCinfo::BroadPhaseBorderBehaviour type )
{
	m_world = world;
	m_type = type;

	addReference();
	m_world->addWorldDeletionListener( this );

	hkAabb infoAABB;

	const hkVector4 min = m_world->m_broadPhaseExtents[0];
	const hkVector4 max = m_world->m_broadPhaseExtents[1];
	{
		infoAABB.m_min.set( max(0), min(1), min(2));
		infoAABB.m_max.set( max(0), max(1), max(2));
		hkAabbPhantom* ph = new hkAabbPhantom( infoAABB );
		ph->getCollidableRw()->getBroadPhaseHandle()->setType(hkWorldObject::BROAD_PHASE_BORDER);
		ph->addPhantomOverlapListener( this );
		m_phantoms[0] = ph;
		world->addPhantom( ph );
	}
	{
		infoAABB.m_min.set( min(0), min(1), min(2));
		infoAABB.m_max.set( min(0), max(1), max(2));
		hkAabbPhantom* ph = new hkAabbPhantom( infoAABB );
		ph->getCollidableRw()->getBroadPhaseHandle()->setType(hkWorldObject::BROAD_PHASE_BORDER);
		ph->addPhantomOverlapListener( this );
		m_phantoms[1] = ph;
		world->addPhantom( ph );
	}

	{
		infoAABB.m_min.set( min(0), max(1), min(2));
		infoAABB.m_max.set( max(0), max(1), max(2));
		hkAabbPhantom* ph = new hkAabbPhantom( infoAABB );
		ph->getCollidableRw()->getBroadPhaseHandle()->setType(hkWorldObject::BROAD_PHASE_BORDER);
		ph->addPhantomOverlapListener( this );
		m_phantoms[2] = ph;
		world->addPhantom( ph );
	}
	{
		infoAABB.m_min.set( min(0), min(1), min(2));
		infoAABB.m_max.set( max(0), min(1), max(2));
		hkAabbPhantom* ph = new hkAabbPhantom( infoAABB );
		ph->getCollidableRw()->getBroadPhaseHandle()->setType(hkWorldObject::BROAD_PHASE_BORDER);
		ph->addPhantomOverlapListener( this );
		m_phantoms[3] = ph;
		world->addPhantom( ph );
	}

	{
		infoAABB.m_min.set( min(0), min(1), max(2));
		infoAABB.m_max.set( max(0), max(1), max(2));
		hkAabbPhantom* ph = new hkAabbPhantom( infoAABB );
		ph->getCollidableRw()->getBroadPhaseHandle()->setType(hkWorldObject::BROAD_PHASE_BORDER);
		ph->addPhantomOverlapListener( this );
		m_phantoms[4] = ph;
		world->addPhantom( ph );
	}

	{
		infoAABB.m_min.set( min(0), min(1), min(2));
		infoAABB.m_max.set( max(0), max(1), min(2));
		hkAabbPhantom* ph = new hkAabbPhantom( infoAABB );
		ph->getCollidableRw()->getBroadPhaseHandle()->setType(hkWorldObject::BROAD_PHASE_BORDER);
		ph->addPhantomOverlapListener( this );
		m_phantoms[5] = ph;
		world->addPhantom( ph );
	}
}

hkBroadPhaseBorder::~hkBroadPhaseBorder()
{
	for (int i = 0; i < 6; i++ )
	{
		if (m_phantoms[i] != HK_NULL)
		{
			m_phantoms[i]->removeReference();
			m_phantoms[i] = HK_NULL;
		}
	}
}


void hkBroadPhaseBorder::collidableAddedCallback( const hkCollidableAddedEvent& event )
{
	hkRigidBody* body = hkGetRigidBody( event.m_collidable );
	if ( body )
	{
		maxPositionExceededCallback( body );
	}
}


// Callback implementation 
void hkBroadPhaseBorder::collidableRemovedCallback( const hkCollidableRemovedEvent& event )
{
}


void hkBroadPhaseBorder::maxPositionExceededCallback( hkEntity* entity )
{
	hkRigidBody* body = static_cast<hkRigidBody*>(entity);
	switch ( m_type )
	{
		case hkWorldCinfo::BROADPHASE_BORDER_ASSERT:
			HK_ASSERT2( 0xf013323d, 0, "Entity left the broadphase. See hkWorldCinfo::BroadPhaseBorderBehaviour for details." );
			body->setMotionType( hkMotion::MOTION_FIXED );
			break;

		case hkWorldCinfo::BROADPHASE_BORDER_REMOVE_ENTITY:
			m_world->removeEntity( entity );
			HK_WARN( 0x65567363, "Entity left the broadphase and has been removed from the hkWorld. See hkWorldCinfo::BroadPhaseBorderBehaviour for details." );
			break;

		case hkWorldCinfo::BROADPHASE_BORDER_FIX_ENTITY:
			body->setMotionType( hkMotion::MOTION_FIXED );
			HK_WARN( 0x267bc474, "Entity left the broadphase and has been changed to fixed motion type. See hkWorldCinfo::BroadPhaseBorderBehaviour for details." );
			break;

		default:
			break;
	}
}

void hkBroadPhaseBorder::worldDeletedCallback( hkWorld* world )
{
	m_world->removeWorldDeletionListener( this );
	removeReference();
}

void hkBroadPhaseBorder::deactivate()
{
	if ( m_world )
	{
		m_world->removePhantomBatch( reinterpret_cast<hkPhantom**>(&m_phantoms[0]), 6 );
		for (int i =0; i < 6; i++ )
		{
			m_phantoms[i]->removePhantomOverlapListener( this );
			m_phantoms[i]->removeReference();
			m_phantoms[i] = HK_NULL;
		}
		worldDeletedCallback( m_world );
		m_world = HK_NULL;
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
