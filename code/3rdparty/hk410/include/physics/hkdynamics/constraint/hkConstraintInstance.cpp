/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/constraint/hkConstraintOwner.h>

// for cloning:
#include <hkdynamics/constraint/bilateral/limitedhinge/hkLimitedHingeConstraintData.h>
#include <hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkbase/class/hkTypeInfo.h>
#include <hkdynamics/entity/hkRigidBody.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkConstraintInstance);


hkConstraintInstance::hkConstraintInstance(hkEntity* entityA, hkEntity* entityB, hkConstraintData* data, hkConstraintInstance::ConstraintPriority priority)
	:	m_owner(HK_NULL),
		m_data(data),
		// entities, below
		m_priority(priority),
		m_wantRuntime(false),
		m_name(HK_NULL),
		m_userData(0), // can call setUserData to set it.
		m_internal(HK_NULL)
{
	// setup contraintInternal members
	m_entities[0] = entityA;
	m_entities[1] = entityB;
	m_constraintModifiers = HK_NULL;

	HK_ASSERT2(0xf0fe4356, entityA, "EntityA not set.");

	//Chris: Upon addition to the world, a constraint instance has
	// its any null entity (of which only B can be null)
	// set as the world's fixed body. Thus NULL is allowed for B
	// and this happens by default in the serialization snapshot
	// and constraints created without knowledge of the world
	// have no access to a fixed body unless they make a dummy one
	// HK_ASSERT2(0xf0fe4356, entityB, "EntityB not set.");


	{
		m_entities[0]->addReferenceAsCriticalOperation();
		if (m_entities[1] != HK_NULL) 
		{
			m_entities[1]->addReferenceAsCriticalOperation();
		}
		m_data->addReference();
	}
}

hkConstraintInstance::hkConstraintInstance(hkConstraintInstance::ConstraintPriority priority)
:	m_owner(HK_NULL),
	m_constraintModifiers(HK_NULL),
	m_priority(priority),
	m_wantRuntime(false),
	m_name(HK_NULL),
	m_userData(0),
	m_internal(HK_NULL)
{
}

void hkConstraintInstance::setPriority( hkConstraintInstance::ConstraintPriority priority ) 
{
	m_priority = priority;
	if ( m_internal )
	{
		m_internal->m_priority = priority;
	}
}

hkSimulationIsland* hkConstraintInstance::getSimulationIsland()
{
	if (!m_entities[0]->isFixed())
	{
		return m_entities[0]->getSimulationIsland();
	}
	return m_entities[1]->getSimulationIsland();
}


void hkConstraintInstance::entityAddedCallback(hkEntity* entity)
{
	HK_ASSERT2(0x11f2a0b1, 0, "Internal Error");
}

void hkConstraintInstance::entityDeletedCallback( hkEntity* entity )
{
	HK_ASSERT2(0x11f2a0b1, 0, "Internal Error: the constraint should be holding a reference to its bodies.");
}

void hkConstraintInstance::entityRemovedCallback(hkEntity* entity)
{
	// before checkin, make sure that this assert is ok ? and then remove the if-condition
	//HK_ASSERT2(0xad6777dd, m_owner != HK_NULL, "internal error.");
	if ( m_owner != HK_NULL )
	{
		HK_ASSERT2(0xad4bd4d3, entity->getWorld(), "Internal error: entity passed in hkConstraintInstance::entityRemovedCallback is already removed from the world (Constraints must be removed first).");
		hkWorldOperationUtil::removeConstraintImmediately(entity->getWorld(), this);
	}
}

hkConstraintInstance* hkConstraintInstance::clone(hkEntity* newEntityA, hkEntity* newEntityB) const
{
	HK_ASSERT2(0xad67888a, getType() == hkConstraintInstance::TYPE_NORMAL, "Cloning only works for normal constraints, not for constraint chains, for example.");

	// info previously you could set targets independently to the cloned instance
	
	hkConstraintInstance* instance = new hkConstraintInstance( newEntityA, newEntityB, getData(), m_priority );
	return instance;
}

hkConstraintInstance::~hkConstraintInstance()
{
	HK_ASSERT2(0x733aae9d, HK_NULL == m_owner, "hkConstraintInstance has an owner and should not be deleted.");

	if (m_entities[0] != HK_NULL)
	{
		m_entities[0]->removeReference(); 
	}
	if (m_entities[1] != HK_NULL)
	{
		m_entities[1]->removeReference();
	}

	if (m_data)
	{
		m_data->removeReference();
	}
}

// Not to be used.
hkConstraintInstance::hkConstraintInstance()
{
	HK_ASSERT(0,0);
}

void hkConstraintInstance::pointNullsToFixedRigidBody()
{
	HK_ASSERT( 0x225ef5d3, m_owner == HK_NULL );
	
	for( int i = 0; i < 2; ++i )
	{
		if( m_entities[i] == HK_NULL )
		{
			hkEntity* other = getOtherEntity(m_entities[i]);
			if( other && other->getWorld() )
			{
				m_entities[i] = other->getWorld()->getFixedRigidBody();
				m_entities[i]->addReference();
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
