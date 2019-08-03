/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkmath/hkMath.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkutilities/visualdebugger/viewer/hkForcedContactPointViewer.h>

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>

#include <hkcollide/agent/hkCdBody.h>
#include <hkcollide/agent/hkProcessCdPoint.h>
#include <hkcollide/agent/hkProcessCollisionOutput.h>

#include <hkvisualize/hkDebugDisplay.h>
#include <hkutilities/visualdebugger/viewer/hkCollideDebugUtil.h>
#include <hkvisualize/hkProcessFactory.h>

int hkForcedContactPointViewer::m_tag = 0;

void HK_CALL hkForcedContactPointViewer::registerViewer()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkProcess* HK_CALL hkForcedContactPointViewer::create(const hkArray<hkProcessContext*>& contexts)
{
	return new hkForcedContactPointViewer(contexts);
}

hkForcedContactPointViewer::hkForcedContactPointViewer(const hkArray<hkProcessContext*>& contexts)
: hkWorldViewerBase( contexts)
{
	
}

void hkForcedContactPointViewer::init()
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			worldAddedCallback( m_context->getWorld(i));
		}
	}
}

hkForcedContactPointViewer::~hkForcedContactPointViewer()
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			worldRemovedCallback( m_context->getWorld(i));
		}
	}
}

void hkForcedContactPointViewer::worldAddedCallback( hkWorld* w)
{
	w->markForWrite();
	w->addCollisionListener( this );
	w->unmarkForWrite();
}

void hkForcedContactPointViewer::worldRemovedCallback( hkWorld* w)
{
	w->markForWrite();
	w->removeCollisionListener( this );
	w->unmarkForWrite();
}

void hkForcedContactPointViewer::contactPointConfirmedCallback(       hkContactPointConfirmedEvent& event)
{
	// This version displays new contact points as big "stars"
	const hkVector4& p = event.m_contactPoint->getPosition();

	for (int i=0; i < 20; i++ )
	{
		hkVector4 dir; dir.set( hkMath::sin( i * 1.0f ), hkMath::cos( i * 1.0f ), hkMath::sin(i * 5.0f ) );
		hkVector4 pdir; pdir.setAdd4(p, dir);
		m_displayHandler->displayLine(p, pdir, hkColor::GREEN, m_tag);
	}


	hkEntity* entityA = hkGetRigidBody( &event.m_collidableA );
	if ( entityA )
	{
		entityA->setProcessContactCallbackDelay(0);
	}

	hkVector4 pos2; pos2.setAdd4( event.m_contactPoint->getPosition(), event.m_contactPoint->getNormal() );
	m_displayHandler->displayLine(event.m_contactPoint->getPosition(), pos2, hkColor::RED, m_tag);
}

void hkForcedContactPointViewer::contactPointAddedCallback( hkContactPointAddedEvent& event)
{
	// This version displays new contact points as big "stars"
	const hkVector4& p = event.m_contactPoint->getPosition();

	for (int i=0; i < 20; i++ )
	{
		hkVector4 dir; dir.set( hkMath::sin( i * 1.0f ), hkMath::cos( i * 1.0f ), hkMath::sin(i * 5.0f ) );

		hkVector4 pdir; pdir.setAdd4(p, dir);
		m_displayHandler->displayLine(p, pdir, hkColor::RED, m_tag);
	}


	hkEntity* entityA = hkGetRigidBody( event.m_bodyA.getRootCollidable() );
	if ( entityA )
	{
		entityA->setProcessContactCallbackDelay(0);
	}

	hkVector4 pos2; pos2.setAdd4( event.m_contactPoint->getPosition(), event.m_contactPoint->getNormal() );
	m_displayHandler->displayLine(event.m_contactPoint->getPosition(), pos2, hkColor::RED, m_tag);
}

void hkForcedContactPointViewer::contactPointRemovedCallback( hkContactPointRemovedEvent& event)
{

}

static void _displayArrow( hkDebugDisplayHandler* handler, const hkVector4& from, const hkVector4& dir, int color, int tag  )
{
	// Check that we have a valid direction
	if (dir.lengthSquared3() < HK_REAL_EPSILON)
	{
		return;
	}

	hkVector4 to; to.setAdd4( from, dir );
	hkVector4 ort; hkVector4Util::calculatePerpendicularVector( dir, ort );
	ort.normalize3();
	hkVector4 ort2; ort2.setCross( dir, ort );

	ort.mul4( dir.length3() );

	const hkReal c = 0.85f;
	hkVector4 p; p.setInterpolate4( from, to, c );
	hkVector4 p0; p0.setAddMul4( p, ort, 1.0f - c );
	hkVector4 p1; p1.setAddMul4( p, ort, -(1.0f - c) );
	hkVector4 p2; p2.setAddMul4( p, ort2, 1.0f - c );
	hkVector4 p3; p3.setAddMul4( p, ort2, -(1.0f - c) );

	handler->displayLine( from, to, color, tag );
	handler->displayLine( to, p0, color, tag );
	handler->displayLine( to, p1, color, tag );
	handler->displayLine( to, p2, color, tag );
	handler->displayLine( to, p3, color, tag );
}

void hkForcedContactPointViewer::contactProcessCallback( hkContactProcessEvent& event)
{
	hkProcessCollisionData& result = event.m_collisionData;

	int size = result.getNumContactPoints();
	for (int i = 0; i < size; i++ )
	{
		const hkVector4& pos = result.m_contactPoints[i].m_contact.getPosition();
		const hkVector4& normal = result.m_contactPoints[i].m_contact.getNormal();

		_displayArrow(m_displayHandler, pos, normal, hkColor::GREEN, m_tag );
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
