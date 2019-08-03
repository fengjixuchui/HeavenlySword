/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>
#include <hkdynamics/entity/hkEntityListener.h>
#include <hkdynamics/entity/hkEntityActivationListener.h>
#include <hkdynamics/collide/hkCollisionListener.h>
#include <hkdynamics/action/hkAction.h>

static inline void HK_CALL cleanupNullPointers( hkArray<void*>& cleanupArray )
{
	for (int i = cleanupArray.getSize() - 1; i >= 0; i-- )
	{
		if ( cleanupArray[i] == HK_NULL )
		{
			cleanupArray.removeAtAndCopy(i);
		}
	}
}

void HK_CALL hkEntityCallbackUtil::fireEntityAdded( hkEntity* entity ) 
{
	{
		hkArray<hkEntityListener*> &listen = entity->m_entityListeners;
		for ( int i = listen.getSize()-1; i >= 0; i-- )
		{
			if (listen[i] != HK_NULL)
			{
				listen[i]->entityAddedCallback( entity );
			}
		}
	}
	{
		HK_ASSERT2( 0xf0356434, entity->getNumConstraints()==0, "Constraints are alreade attached to the entity before the entity is added to the world" );
		HK_ASSERT2( 0xad000240, entity->getNumActions()    ==0, "Actions are already attached to the entity before the entity is added to the world" );
	}
}

void HK_CALL hkEntityCallbackUtil::fireEntityRemoved( hkEntity* entity ) 
{
	{
		hkArray<hkEntityListener*> &listen = entity->m_entityListeners;
		for ( int i = listen.getSize()-1; i >= 0; i-- )
		{
			if (listen[i] != HK_NULL)
			{
				listen[i]->entityRemovedCallback( entity );
			}
		}
	}

	// master constraints
	{
		hkArray<hkConstraintInternal>& constraints = entity->m_constraintsMaster;
		while( constraints.getSize() )
		{
			HK_ON_DEBUG( int oldsize = constraints.getSize() );
			constraints[0].m_constraint->entityRemovedCallback( entity );
			HK_ASSERT2( 0xf0403423, constraints.getSize() < oldsize, "You have to remove the constraint in the entityRemovedCallback" );
		}
	}

	// slave constraints
	{
		hkArray<hkConstraintInstance*>& constraints = entity->m_constraintsSlave;
		while( constraints.getSize() )
		{
			HK_ON_DEBUG( int oldsize = constraints.getSize() );
			constraints[0]->entityRemovedCallback( entity );
			HK_ASSERT2( 0xf0403423, constraints.getSize() < oldsize, "You have to remove the constraint in the entityRemovedCallback" );
		}
	}

	// actions
	{
		hkArray<hkAction*>& actions = entity->m_actions;
		while( actions.getSize() )
		{
			HK_ON_DEBUG( int oldsize = actions.getSize() );
			actions[0]->entityRemovedCallback( entity );
			HK_ASSERT2( 0xad78dd56, actions.getSize() < oldsize, "You have to remove the action in the entityRemovedCallback." );
		}
	}
}


void HK_CALL hkEntityCallbackUtil::fireEntityShapeSet( hkEntity* entity )
{
	{
		hkArray<hkEntityListener*> &listen = entity->m_entityListeners;
		for ( int i = listen.getSize()-1; i >= 0; i-- )
		{
			if (listen[i] != HK_NULL)
			{
				listen[i]->entityShapeSetCallback( entity );
			}
		}
	}
}

void HK_CALL hkEntityCallbackUtil::fireEntityDeleted( hkEntity* entity )
{
	{
		hkArray<hkEntityListener*> &listen = entity->m_entityListeners;
		for ( int i = listen.getSize()-1; i >= 0; i-- )
		{
			if (listen[i] != HK_NULL)
			{
				listen[i]->entityDeletedCallback( entity );
			}
		}
	}
}

void HK_CALL hkEntityCallbackUtil::fireContactPointAddedInternal( hkEntity* entity, hkContactPointAddedEvent& event)
{
	hkArray<hkCollisionListener*>& listen = entity->m_collisionListeners;
	event.m_callbackFiredFrom = entity;
	for ( int i = listen.getSize()-1; i >= 0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->contactPointAddedCallback( event );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkEntityCallbackUtil::fireContactPointConfirmedInternal( hkEntity* entity, hkContactPointConfirmedEvent& event)
{
	hkArray<hkCollisionListener*>& listen = entity->m_collisionListeners;
	event.m_callbackFiredFrom = entity;
	for ( int i = listen.getSize()-1; i >= 0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->contactPointConfirmedCallback( event );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkEntityCallbackUtil::fireContactPointRemovedInternal( hkEntity* entity, hkContactPointRemovedEvent& event )
{
	hkArray<hkCollisionListener*>& listen = entity->m_collisionListeners;
    event.m_callbackFiredFrom = entity;
	for ( int i = listen.getSize()-1; i >= 0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->contactPointRemovedCallback( event );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkEntityCallbackUtil::fireContactProcessInternal( hkEntity* entity, hkContactProcessEvent& event )
{
	hkArray<hkCollisionListener*>& listen = entity->m_collisionListeners;
    event.m_callbackFiredFrom = entity;
	for ( int i = listen.getSize()-1; i >= 0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->contactProcessCallback( event );
		}
	}
	
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
