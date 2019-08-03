/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/collide/hkContactUpdater.h>
#include <hkdynamics/entity/hkEntity.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/collide/hkDynamicsContactMgr.h>
#include <hkcollide/agent/hkCollisionAgent.h>
#include <hkconstraintsolver/constraint/contact/hkContactPointProperties.h>
#include <hkinternal/collide/agent3/hkAgent3.h>

void HK_CALL hkContactUpdater::defaultFrictionUpdateCallback( hkContactUpdateEvent& event )
{
	for ( int j = 0; j < event.m_contactPointIds.getSize(); ++j )
	{
		hkContactPointProperties* prop = event.m_contactMgr.getContactPointProperties(event.m_contactPointIds[j]);

		const hkMaterial& materialA = static_cast<hkEntity*>(event.m_collidableA.getOwner())->getMaterial();
		const hkMaterial& materialB = static_cast<hkEntity*>(event.m_collidableB.getOwner())->getMaterial();

		prop->setFriction( hkMaterial::getCombinedFriction( materialA.getFriction(), materialB.getFriction() ) );

	}
}

static inline void fireIslandContacts( hkEntity* entity, hkSimulationIsland* island, hkContactUpdater::ContactUpdateCallback cb )
{
	hkLinkedCollidable& collidableEx = *entity->getLinkedCollidable();
	for (int i = 0; i < collidableEx.m_collisionEntries.getSize(); i++)
	{
		hkAgentNnEntry* entry = collidableEx.m_collisionEntries[i].m_agentEntry;

		HK_ASSERT(0xf0ff008e, entry->m_contactMgr != HK_NULL);

		hkCollidable* collA = entry->getCollidableA();
		hkCollidable* collB = entry->getCollidableB();
		hkContactUpdateEvent event( static_cast<hkDynamicsContactMgr&>(*entry->m_contactMgr), *collA, *collB);

		event.m_contactMgr.getAllContactPointIds(event.m_contactPointIds);
		event.m_callbackFiredFrom = entity;

		cb( event );
	}
}

void hkContactUpdater::updateContacts( hkEntity* entity, hkContactUpdater::ContactUpdateCallback cb )
{
	HK_ASSERT2(0x76d83a81, entity->getWorld() != HK_NULL, "You are trying to update contact points for a rigid body which has not been added to the world");
	HK_ASSERT(0x59c69847, entity->getSimulationIsland() != HK_NULL );
	
	if ( !entity->isFixed() )
	{
		fireIslandContacts( entity, entity->getSimulationIsland(), cb );
	}
	else
	{
		// have to go through ALL islands to update contacts to update a fixed body
		hkWorld* world = entity->getWorld();
		{
			for ( int i = 0; i < world->getActiveSimulationIslands().getSize(); ++i )
			{
				fireIslandContacts( entity, world->getActiveSimulationIslands()[i], cb );
			}
		}
		{
			for ( int i = 0; i < world->getInactiveSimulationIslands().getSize(); ++i )
			{
				fireIslandContacts( entity, world->getInactiveSimulationIslands()[i], cb );
			}
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
