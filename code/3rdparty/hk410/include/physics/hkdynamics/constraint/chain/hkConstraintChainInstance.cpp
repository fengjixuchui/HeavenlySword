/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstance.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstanceAction.h>

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL( hkConstraintChainInstance );

hkConstraintChainInstance::hkConstraintChainInstance(hkConstraintChainData* data)
: hkConstraintInstance(PRIORITY_PSI)
{
	HK_ASSERT2(0xad675544, data->getType() >= hkConstraintData::BEGIN_CONSTRAINT_CHAIN_TYPES, "You're passing a non-chain-constraint data to a hkConstraintChainInstance's ctor.");

	m_data = data;
	data->addReference();

	m_entities[0] = HK_NULL;
	m_entities[1] = HK_NULL;

	m_action = new hkConstraintChainInstanceAction(this);
}


void hkConstraintChainInstance::addEntity(hkEntity* entity) 
{
	HK_ASSERT2(0xad6d5d44, m_owner == HK_NULL, "Cannot add entities when constraint chain is added to the world");
	
	if (m_chainedEntities.getSize() < 2 )
	{
		const int idx = m_chainedEntities.getSize();
		HK_ASSERT2(0xad6888d0, m_entities[idx] ==  HK_NULL || m_entities[idx] == entity, "First or second entity added is different from that passed in the hkConstraintChainInstance's constructor.");
		if (m_entities[idx] == HK_NULL)
		{
			m_entities[idx] = entity;
			entity->addReference();
		}
	}

	m_chainedEntities.pushBack( entity );
	entity->addReference();
}

hkConstraintChainInstance::~hkConstraintChainInstance()
{
	for (int i = 0; i < m_chainedEntities.getSize(); i++)
	{
		m_chainedEntities[i]->removeReference();
	}

	HK_ASSERT2(0xad78dd33, m_action->getWorld() == HK_NULL && m_action->getReferenceCount() == 1, "hkConstraintChainInstanceAction's lifetime cannot exceed that of its hkConstraintChainInstance.");
	m_action->removeReference();
}

void hkConstraintChainInstance::entityRemovedCallback(hkEntity* entity)
{
	// before checkin, make sure that this assert is ok ?
	HK_ASSERT2(0xad6777dd, m_owner != HK_NULL, "internal error.");

	HK_ASSERT2(0xad4bd4d3, entity->getWorld(), "Internal error: entity passed in hkConstraintInstance::entityRemovedCallback is already removed from the world (Constraints must be removed first).");
	hkWorld* world = entity->getWorld();

	world->lockCriticalOperations();
	{
		// Adding constraint chain's action
		world->removeActionImmediately(m_action);

		// info: locking done in the hkWorldOperationUtil function
		hkWorldOperationUtil::removeConstraintFromCriticalLockedIsland(world, this);

	}
	world->unlockAndAttemptToExecutePendingOperations();
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
