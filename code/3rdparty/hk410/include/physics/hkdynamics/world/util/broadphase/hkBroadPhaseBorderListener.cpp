/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/util/broadphase/hkBroadPhaseBorderListener.h>
#include <hkdynamics/phantom/hkPhantom.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>


void hkBroadPhaseBorderListener::addCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	if (   pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_BORDER
		&& pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		return;
	}
	if ( pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collA->getOwner() );
		p->addOverlappingCollidable( collB );
	}

	if ( pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collB->getOwner() );
		p->addOverlappingCollidable( collA );
	}
}


void hkBroadPhaseBorderListener::removeCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	if (   pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_BORDER
		&& pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		return;
	}

	if ( pair.getElementA()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collA->getOwner() );
		p->removeOverlappingCollidable( collB );
	}

	if ( pair.getElementB()->getType() == hkWorldObject::BROAD_PHASE_BORDER )
	{
		hkCollidable* collA = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
		hkCollidable* collB = static_cast<hkCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );
		hkPhantom* p = static_cast<hkPhantom*>( collB->getOwner() );
		p->removeOverlappingCollidable( collA );
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
