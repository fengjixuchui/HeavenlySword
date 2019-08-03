/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/dynamics/suspendInactiveAgents/hkSuspendInactiveAgentsUtil.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>
#include <hkinternal/collide/agent3/hkAgent3.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkinternal/collide/agent3/machine/nn/hkAgentNnMachine.h>
#include <hkcollide/filter/hkCollisionFilter.h>

#include <hkcollide/agent/hkCdBody.h>

hkSuspendInactiveAgentsUtil::hkSuspendInactiveAgentsUtil(hkWorld* world, OperationMode mode, InitContactsMode initContactsMode )
:	m_world(world), m_mode(mode), m_initContactsMode(initContactsMode)
{
	addReference();
	world->addWorldDeletionListener( this );
	world->addIslandActivationListener( this );
}

hkSuspendInactiveAgentsUtil::~hkSuspendInactiveAgentsUtil()
{
	if ( m_world )
	{
		m_world->removeWorldDeletionListener( this );
		m_world = HK_NULL;
	}
}
		
namespace {

	class NeverCollideFilter : public hkCollisionFilter
	{
		virtual hkBool isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const { return false; }
		virtual	hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const { return false; }
		virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const { return false; }
		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const { return false; }
	};

	class Clear1nTracksFilter : public hkCollisionFilter
	{ 
	public:
		Clear1nTracksFilter( const hkCollisionFilter* filter ) : m_originalFilter(filter) { HK_ASSERT2(0xad7865dd, m_originalFilter, "Original filter must be specified."); m_originalFilter->addReference(); }

		~Clear1nTracksFilter() { m_originalFilter->removeReference(); }

		virtual hkBool isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const 
		{
			HK_ASSERT2(0xad78d6a0, false, "This function should be never called."); return true;
		}

		virtual hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bCollection, hkShapeKey bKey  ) const  
		{	
			if ( b.getShape()->getType() == HK_SHAPE_MOPP 
			  || b.getShape()->getType() == HK_SHAPE_BV_TREE )
			{
				return false;
			}

			return m_originalFilter->isCollisionEnabled (input, a, b, bCollection, bKey);
		}

		virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const  
		{	
			HK_ASSERT2(0xad78d6a0, false, "This function should be never called."); return true;
		}

		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const  
		{	
			HK_ASSERT2(0xad78d6a0, false, "This function should be never called."); return true;
		}

	protected:
		const hkCollisionFilter* m_originalFilter;
	};
}

void hkSuspendInactiveAgentsUtil::islandDeactivatedCallback( hkSimulationIsland* island )
{
	// This is only called from hkWorldOperationUtil::cleanupDirtyIslands.
	HK_ACCESS_CHECK_OBJECT( island->getWorld(), HK_ACCESS_RW );
	HK_ASSERT2( 0xad7899de, island->getWorld()->areCriticalOperationsLocked(), "Critical operations are expected to be locked.");

	NeverCollideFilter neverCollideFilter;
	Clear1nTracksFilter clear1nTracksFilter(m_world->getCollisionFilter());

	hkCollisionInput input = *m_world->getCollisionInput();
	switch(m_mode)
	{
		case SUSPEND_ALL_COLLECTION_AGENTS: input.m_filter = &neverCollideFilter; break;
		case SUSPEND_1N_AGENT_TRACKS:       input.m_filter = &clear1nTracksFilter; break;
	}

	HK_FOR_ALL_AGENT_ENTRIES_BEGIN(island->m_agentTrack, entry)
	{
		hkAgentNnMachine_UpdateShapeCollectionFilter( entry, input, *island );
	}
	HK_FOR_ALL_AGENT_ENTRIES_END;
}

void hkSuspendInactiveAgentsUtil::islandActivatedCallback( hkSimulationIsland* island )
{
	// This is only called from hkWorldOperationUtil::cleanupDirtyIslands and from the engine, e.g. during island merges.
	// This is not safe is the updateShapeCollectioFilter would remove any agents.

	HK_ACCESS_CHECK_OBJECT( island->getWorld(), HK_ACCESS_RW );
	HK_ASSERT2( 0xad7899df, island->getWorld()->areCriticalOperationsLocked(), "Critical operations are expected to be locked.");

	hkCollisionInput input = *m_world->getCollisionInput();

	if (m_mode == SUSPEND_ALL_COLLECTION_AGENTS)
	{
		HK_FOR_ALL_AGENT_ENTRIES_BEGIN(island->m_agentTrack, entry)
		{
			hkAgentNnMachine_UpdateShapeCollectionFilter( entry, input, *island );
		}
		HK_FOR_ALL_AGENT_ENTRIES_END;
	}

	if (m_initContactsMode == INIT_CONTACTS_FIND)
	{
		m_world->findInitialContactPoints( island->m_entities.begin(), island->m_entities.getSize() );
	}
}

void hkSuspendInactiveAgentsUtil::worldDeletedCallback( hkWorld* world )
{
	world->removeWorldDeletionListener( this );
	m_world = HK_NULL;
	removeReference();
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
