/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/collide/surfacevelocity/filtered/hkFilteredSurfaceVelocityUtil.h>
#include <hkdynamics/collide/hkResponseModifier.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/world/hkSimulationIsland.h>

hkFilteredSurfaceVelocityUtil::hkFilteredSurfaceVelocityUtil(hkRigidBody* body, const hkVector4& surfaceVelocityWorld, int propertyId)
:	hkSurfaceVelocityUtil(body, surfaceVelocityWorld)
{
	m_propertyId = propertyId;
}


void hkFilteredSurfaceVelocityUtil::enableEntity(hkEntity *entity, hkConstraintOwner* constraintOwner )
{
	HK_ASSERT2(0xafc84671, entity->hasProperty(m_propertyId) == true, "Property is missing. Make sure that you add this property to all potentially filtered objects.");

	//
	// mark entity as 'enabled'
	//
	{
		entity->lockProperty(m_propertyId);
		HK_ASSERT2(0xafc8467c, entity->getProperty(m_propertyId).getInt() == 0, "Failed to enable surface velocity for entity, entity already enabled.");
		entity->editProperty(m_propertyId, 1);
		entity->unlockProperty(m_propertyId);
	}

	if ( !constraintOwner )
	{
		constraintOwner = entity->getSimulationIsland();
	}
	// If you get a crash here and you want to understand constraintOwner, please read the reference manual for hkResponseModifier
	constraintOwner->checkAccessRw();

	// Check 
	//  - whether the contact manager exists,
	//  - whether it is of the simple contact response type
	//  - and whether its contact constraint has an owner already
	hkDynamicsContactMgr* contactMgr = m_rigidBody->findContactMgrTo(entity);
	hkConstraintInstance* constraintInstance;
	{
		if ( !contactMgr  )
		{
			return;
		}

		constraintInstance = contactMgr->getConstraintInstance();
		if ( !constraintInstance || !constraintInstance->getOwner())
		{
			return;
		}
	}

		// set surface velocity for entities which are already colliding
	hkResponseModifier::setSurfaceVelocity( contactMgr, m_rigidBody, *constraintOwner, m_surfaceVelocity );

	return;
}

void hkFilteredSurfaceVelocityUtil::disableEntity(hkEntity *entity, hkConstraintOwner* constraintOwner )
{
	//
	// remove 'enabled' mark from entity
	//
	{
		entity->lockProperty(m_propertyId);
		HK_ASSERT2(0xafc846ff, entity->getProperty(m_propertyId).getInt() == 1, "Failed to disable surface velocity for entity, entity not enabled yet.");
		entity->editProperty(m_propertyId, 0);
		entity->unlockProperty(m_propertyId);
	}
	if ( !constraintOwner )
	{
		constraintOwner = entity->getSimulationIsland();
	}
	// If you get a crash here and you want to understand constraintOwner, please read the reference manual for hkResponseModifier
	constraintOwner->checkAccessRw();

	// Check 
	//  - whether the contact manager exists,
	//  - whether it is of the simple contact response type
	//  - and whether its contact constraint has an owner already
	hkDynamicsContactMgr* contactMgr = m_rigidBody->findContactMgrTo(entity);
	hkConstraintInstance* constraintInstance;
	{
		if ( !contactMgr  )
		{
			return;
		}

		constraintInstance = contactMgr->getConstraintInstance();
		if ( !constraintInstance || !constraintInstance->getOwner())
		{
			return;
		}
	}

	// clear surface velocity for entities that are still colliding
	hkResponseModifier::clearSurfaceVelocity( contactMgr, *constraintOwner, m_rigidBody );

	return;
}


void hkFilteredSurfaceVelocityUtil::contactPointAddedCallback(hkContactPointAddedEvent& event)
{

	// find the 'other' rigid body of this potential contact point
	hkEntity* otherEntity;
	{
		hkRigidBody *bodyA = hkGetRigidBody(event.m_bodyA.getRootCollidable());
		hkRigidBody *bodyB = hkGetRigidBody(event.m_bodyB.getRootCollidable());
		if ( bodyA == m_rigidBody )
		{
			otherEntity = bodyB;
		}
		else
		{
			otherEntity = bodyA;
		}
	}

	// abort if entity's not marked as 'enabled'
	if ( otherEntity->getProperty(m_propertyId).getInt() == 0 )
	{
		return;
	}

	hkSurfaceVelocityUtil::contactPointAddedCallback(event);
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
