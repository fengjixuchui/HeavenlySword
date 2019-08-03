/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/util/hkWorldCallbackUtil.h>
#include <hkdynamics/world/hkWorld.h>

#include <hkdynamics/action/hkActionListener.h>
#include <hkdynamics/entity/hkEntityListener.h>
#include <hkdynamics/entity/hkEntityActivationListener.h>
#include <hkdynamics/phantom/hkPhantomListener.h>
#include <hkdynamics/constraint/hkConstraintListener.h>
#include <hkdynamics/world/listener/hkWorldDeletionListener.h>
#include <hkdynamics/world/listener/hkIslandActivationListener.h>
#include <hkdynamics/world/listener/hkWorldPostCollideListener.h>
#include <hkdynamics/world/listener/hkWorldPostSimulationListener.h>
#include <hkdynamics/world/listener/hkWorldPostIntegrateListener.h>
#include <hkdynamics/world/listener/hkIslandPostCollideListener.h>
#include <hkdynamics/world/listener/hkIslandPostIntegrateListener.h>
#include <hkdynamics/collide/hkCollisionListener.h>

#include <hkdynamics/world/hkSimulationIsland.h>


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

void HK_CALL hkWorldCallbackUtil::fireActionAdded( hkWorld* world, hkAction* action )
{
	hkArray<hkActionListener*>& listen = world->m_actionListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->actionAddedCallback( action );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkWorldCallbackUtil::fireActionRemoved( hkWorld* world, hkAction* action )
{
	hkArray<hkActionListener*>& listen = world->m_actionListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->actionRemovedCallback( action );	
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );

}

void HK_CALL hkWorldCallbackUtil::fireEntityAdded( hkWorld* world, hkEntity* entity ) 
{
	hkArray<hkEntityListener*>& listen = world->m_entityListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->entityAddedCallback( entity );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}


void HK_CALL hkWorldCallbackUtil::fireEntityRemoved( hkWorld* world, hkEntity* entity ) 
{
	hkArray<hkEntityListener*>& listen = world->m_entityListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if ( listen[i] != HK_NULL )
		{
			listen[i]->entityRemovedCallback( entity );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );

}

void HK_CALL hkWorldCallbackUtil::fireEntityShapeSet( hkWorld* world, hkEntity* entity ) 
{
	hkArray<hkEntityListener*>& listen = world->m_entityListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->entityShapeSetCallback( entity );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkWorldCallbackUtil::firePhantomAdded( hkWorld* world, hkPhantom* Phantom ) 
{
	hkArray<hkPhantomListener*>& listen = world->m_phantomListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->phantomAddedCallback( Phantom );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}


void HK_CALL hkWorldCallbackUtil::firePhantomRemoved( hkWorld* world, hkPhantom* Phantom ) 
{
	hkArray<hkPhantomListener*>& listen = world->m_phantomListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if ( listen[i] != HK_NULL )
		{
			listen[i]->phantomRemovedCallback( Phantom );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );

}

void HK_CALL hkWorldCallbackUtil::firePhantomShapeSet( hkWorld* world, hkPhantom* Phantom ) 
{
	hkArray<hkPhantomListener*>& listen = world->m_phantomListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if (listen[i] != HK_NULL)
		{
			listen[i]->phantomShapeSetCallback( Phantom );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkWorldCallbackUtil::fireConstraintAdded( hkWorld* world, hkConstraintInstance* constraint ) 
{
	hkArray<hkConstraintListener*>& listen = world->m_constraintListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if ( listen[i] != HK_NULL )
		{
			listen[i]->constraintAddedCallback( constraint );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkWorldCallbackUtil::fireConstraintRemoved( hkWorld* world, hkConstraintInstance* constraint ) 
{
	hkArray<hkConstraintListener*>& listen = world->m_constraintListeners;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if ( listen[i] != HK_NULL )
		{
			listen[i]->constraintRemovedCallback( constraint );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}


void HK_CALL hkWorldCallbackUtil::fireContactPointAdded( hkWorld* world, hkContactPointAddedEvent& event)
{
	hkArray<hkCollisionListener*>& listen = world->m_collisionListeners;
	event.m_callbackFiredFrom = HK_NULL;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if ( listen[i] != HK_NULL )
		{
			listen[i]->contactPointAddedCallback( event );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}
void HK_CALL hkWorldCallbackUtil::fireContactPointConfirmed( hkWorld* world, hkContactPointConfirmedEvent& event)
{
	hkArray<hkCollisionListener*>& listen = world->m_collisionListeners;
	event.m_callbackFiredFrom = HK_NULL;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if ( listen[i] != HK_NULL )
		{
			listen[i]->contactPointConfirmedCallback( event );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkWorldCallbackUtil::fireContactPointRemoved( hkWorld* world, hkContactPointRemovedEvent& event )
{
	hkArray<hkCollisionListener*>& listen = world->m_collisionListeners;
    event.m_callbackFiredFrom = HK_NULL;
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if ( listen[i] != HK_NULL )
		{
			listen[i]->contactPointRemovedCallback( event );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkWorldCallbackUtil::fireContactProcess( hkWorld* world, hkContactProcessEvent& event )
{
	hkArray<hkCollisionListener*>& listen = world->m_collisionListeners;
    event.m_callbackFiredFrom = HK_NULL;    
	for (int i = listen.getSize()-1; i>=0; i-- )
	{
		if ( listen[i] != HK_NULL )
		{
			listen[i]->contactProcessCallback( event );
		}
	}
	hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
	cleanupNullPointers( cleanupArray );
}

void HK_CALL hkWorldCallbackUtil::fireWorldDeleted( hkWorld* world )
{
	{
		hkArray<hkWorldDeletionListener*>& listen = world->m_worldDeletionListeners;
		for (int i = listen.getSize()-1; i>=0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->worldDeletedCallback( world );
			}
		}
	//	HK_ASSERT2(0x387ea930,  listen.getSize() == 0, "A hkWorldDeletionListener did not call hkWorld::removeSimulationListener during a worldDeletedCallback() callback");
		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );
	}
}

void HK_CALL hkWorldCallbackUtil::fireIslandActivated( hkWorld* world, hkSimulationIsland* island )
{
	// The function needs to be locked, because it references a specific island which might get removed during execution of this function.
	world->lockCriticalOperations();
	{
		hkArray<hkIslandActivationListener*>& listen = world->m_islandActivationListeners;
		for ( int i = listen.getSize() - 1; i >= 0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->islandActivatedCallback( island );
			}
		}

		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );

	}
	{
		for (int i = 0; i < island->getEntities().getSize(); ++i)
		{
			hkArray<hkEntityActivationListener*>& listen = island->getEntities()[i]->m_activationListeners;
			for ( int j = listen.getSize() - 1; j >= 0; j-- )
			{
				if ( listen[j] != HK_NULL )
				{
					listen[j]->entityActivatedCallback( island->getEntities()[i] );
				}
			}
			hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
			cleanupNullPointers( cleanupArray );
		}
	}
	world->unlockAndAttemptToExecutePendingOperations();
}


void HK_CALL hkWorldCallbackUtil::fireIslandDeactivated( hkWorld* world, hkSimulationIsland* island )
{
	// The function needs to be locked, because it references a specific island which might get removed during execution of this function.
	world->lockCriticalOperations();
	{
		hkArray<hkIslandActivationListener*>& listen = world->m_islandActivationListeners;
		for ( int i = listen.getSize() - 1; i >= 0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->islandDeactivatedCallback( island );
			}
		}
		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );
	}
	{
		for (int i = 0; i < island->getEntities().getSize(); ++i)
		{
			hkArray<hkEntityActivationListener*>& listen = island->getEntities()[i]->m_activationListeners;
			for ( int j = listen.getSize() - 1; j >= 0; j-- )
			{
				if ( listen[j] != HK_NULL )
				{
					listen[j]->entityDeactivatedCallback( island->getEntities()[i] );
				}
			}
			hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
			cleanupNullPointers( cleanupArray );
		}
	}
	world->unlockAndAttemptToExecutePendingOperations();
}


void HK_CALL hkWorldCallbackUtil::firePostSimulationCallback( hkWorld* world )
{
	{
		hkArray<hkWorldPostSimulationListener*>& listen = world->m_worldPostSimulationListeners;
		for (int i = listen.getSize()-1; i>=0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->postSimulationCallback( world );
			}
		}
		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );
	}
}


void HK_CALL hkWorldCallbackUtil::firePostIntegrateCallback( hkWorld* world, const hkStepInfo& info )
{
	{
		hkArray<hkWorldPostIntegrateListener*>& listen = world->m_worldPostIntegrateListeners;
		for (int i = listen.getSize()-1; i>=0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->postIntegrateCallback( world, info );
			}
		}
		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );
	}
}

void HK_CALL hkWorldCallbackUtil::firePostCollideCallback( hkWorld* world, const hkStepInfo& info )
{
	{
		hkArray<hkWorldPostCollideListener*>& listen = world->m_worldPostCollideListeners;
		for (int i = listen.getSize()-1; i>=0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->postCollideCallback( world, info );
			}
		}
		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );
	}
}


void HK_CALL hkWorldCallbackUtil::fireIslandPostIntegrateCallback( hkWorld* world, hkSimulationIsland* island, const hkStepInfo& info )
{
	{
		hkArray<hkIslandPostIntegrateListener*>& listen = world->m_islandPostIntegrateListeners;
		for (int i = listen.getSize()-1; i>=0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->postIntegrateCallback( island, info );
			}
		}
		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );
	}

}

void HK_CALL hkWorldCallbackUtil::fireIslandPostCollideCallback( hkWorld* world, hkSimulationIsland* island, const hkStepInfo& info )
{
	{
		hkArray<hkIslandPostCollideListener*>& listen = world->m_islandPostCollideListeners;
		for (int i = listen.getSize()-1; i>=0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->postCollideCallback( island, info );
			}
		}
		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );
	}
}

void HK_CALL hkWorldCallbackUtil::fireInactiveEntityMoved( hkWorld* world, hkEntity* entity)
{
	{
		hkArray<hkWorldPostSimulationListener*>& listen = world->m_worldPostSimulationListeners;
		for (int i = listen.getSize()-1; i>=0; i-- )
		{
			if ( listen[i] != HK_NULL )
			{
				listen[i]->inactiveEntityMovedCallback(entity);
			}
		}
		hkArray<void*>& cleanupArray = reinterpret_cast<hkArray<void*>&>(listen);
		cleanupNullPointers( cleanupArray );
	}
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
