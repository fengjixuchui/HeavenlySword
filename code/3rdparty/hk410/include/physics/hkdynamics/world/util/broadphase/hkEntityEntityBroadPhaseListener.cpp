/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkcollide/agent/hkProcessCollisionInput.h>

#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.h>
#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkinternal/collide/agent3/machine/nn/hkAgentNnMachine.h>

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>

#include <hkdynamics/world/util/broadphase/hkEntityEntityBroadPhaseListener.h>

hkEntityEntityBroadPhaseListener::hkEntityEntityBroadPhaseListener( hkWorld* world)
{
	m_world = world;
}

void hkEntityEntityBroadPhaseListener::addCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	hkLinkedCollidable* collA = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
	hkLinkedCollidable* collB = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );

	// check for disabled collisions, especially landscape = landscape ones
	hkProcessCollisionInput* input = m_world->getCollisionInput();
	{
		hkCollidableQualityType qt0 = collA->getQualityType();
		hkCollidableQualityType qt1 = collB->getQualityType();
		hkChar collisionQuality = input->m_dispatcher->getCollisionQualityIndex( qt0, qt1 );
		if ( collisionQuality == hkCollisionDispatcher::COLLISION_QUALITY_INVALID )
		{
			return;
		}
		hkCollisionQualityInfo* origInfo = input->m_dispatcher->getCollisionQualityInfo( collisionQuality );
		input->m_createPredictiveAgents = origInfo->m_useContinuousPhysics;
	}
	hkWorldAgentUtil::addAgent(collA, collB, *input);
}


void hkEntityEntityBroadPhaseListener::removeCollisionPair( hkTypedBroadPhaseHandlePair& pair )
{
	hkLinkedCollidable* collA = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_a)->getOwner() );
	hkLinkedCollidable* collB = static_cast<hkLinkedCollidable*>( static_cast<hkTypedBroadPhaseHandle*>(pair.m_b)->getOwner() );

	hkAgentNnEntry* entry = hkAgentNnMachine_FindAgent(collA, collB);

	if (entry)
	{
		hkWorldAgentUtil::removeAgent(entry);
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
