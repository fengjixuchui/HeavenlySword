/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/action/hkUnaryAction.h>
#include <hkdynamics/entity/hkEntity.h>
#include <hkdynamics/world/hkWorld.h>

hkUnaryAction::hkUnaryAction(hkEntity* body, hkUint32 userData )
: hkAction( userData ), m_entity(body)
{
	if (m_entity != HK_NULL)
	{
		m_entity->addReference();
	}
	else
	{
		//	HK_WARN(0x4621e7da, "hkUnaryAction: body is a NULL pointer");
	}
}

void hkUnaryAction::entityRemovedCallback(hkEntity* entity) 
{
	// Remove self from physics.
	if ( getWorld() != HK_NULL )
	{
		getWorld()->removeActionImmediately(this);
	}
}

hkUnaryAction::~hkUnaryAction()
{
	if (m_entity != HK_NULL)
	{
		m_entity->removeReference();
		m_entity = HK_NULL;
	}
}

// NB: Only intended to be called pre-simualtion i.e. before the hkUnaryAction is 
// added to an hkWorld.
void hkUnaryAction::setEntity(hkEntity* entity)
{
	//HK_ASSERT2(0x76017ab2, getWorld() == HK_NULL, "This hkUnaryAction is already added to an hkWorld.");	
	HK_ASSERT2(0x5163bcc3, entity != HK_NULL, "entity is a NULL pointer. You can use hkWorld::getFixedRigidBody().");	
	if(m_entity != HK_NULL)
	{
		HK_WARN(0x17aaa816, "m_entity is not NULL. This hkUnaryAction already had an hkEntity.");

		if (getWorld())
		{
			getWorld()->detachActionFromEntity(this, m_entity);
		}
		m_entity->removeReference();
		m_entity = HK_NULL;
	}

	m_entity = entity;
	m_entity->addReference();
	if (getWorld())
	{
		getWorld()->attachActionToEntity(this, m_entity);
	}
}

void hkUnaryAction::getEntities( hkArray<hkEntity*>& entitiesOut )
{
	entitiesOut.pushBack( m_entity );
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
