/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_ENTITY_CALLBACK_UTIL_H
#define HK_DYNAMICS2_ENTITY_CALLBACK_UTIL_H

class hkEntity;
struct hkContactPointAddedEvent;
struct hkContactPointConfirmedEvent;
struct hkContactPointRemovedEvent;
struct hkContactProcessEvent;

class hkEntityCallbackUtil
{
	public:

			/// go through both, the entity and the collision listeners
		static void HK_CALL fireEntityAdded( hkEntity* entity );
		static void HK_CALL fireEntityRemoved( hkEntity* entity );
		static void HK_CALL fireEntityShapeSet( hkEntity* entity );
		static void HK_CALL fireEntityDeleted( hkEntity* entity );

			// fast inline functions calling the internal functions
		static HK_FORCE_INLINE void HK_CALL fireContactPointAdded( hkEntity* entity, hkContactPointAddedEvent& event);
		static HK_FORCE_INLINE void HK_CALL fireContactPointConfirmed( hkEntity* entity, hkContactPointConfirmedEvent& event);
		static HK_FORCE_INLINE void HK_CALL fireContactPointRemoved( hkEntity* entity, hkContactPointRemovedEvent& event );
		static HK_FORCE_INLINE void HK_CALL fireContactProcess( hkEntity* entity, hkContactProcessEvent& event );

	protected:
		static void HK_CALL fireContactPointAddedInternal( hkEntity* entity, hkContactPointAddedEvent& event);
		static void HK_CALL fireContactPointConfirmedInternal( hkEntity* entity, hkContactPointConfirmedEvent& event);
		static void HK_CALL fireContactPointRemovedInternal( hkEntity* entity, hkContactPointRemovedEvent& event );
		static void HK_CALL fireContactProcessInternal( hkEntity* entity, hkContactProcessEvent& event );

};

void HK_CALL hkEntityCallbackUtil::fireContactPointAdded( hkEntity* entity, hkContactPointAddedEvent& event)
{
	if ( entity->m_collisionListeners.getSize())
	{
		fireContactPointAddedInternal(entity, event);
	}
}

void HK_CALL hkEntityCallbackUtil::fireContactPointConfirmed( hkEntity* entity, hkContactPointConfirmedEvent& event)
{
	if ( entity->m_collisionListeners.getSize())
	{
		fireContactPointConfirmedInternal( entity, event );
	}
}

void HK_CALL hkEntityCallbackUtil::fireContactPointRemoved( hkEntity* entity, hkContactPointRemovedEvent& event )
{
	if ( entity->m_collisionListeners.getSize())
	{
		fireContactPointRemovedInternal( entity, event );
	}
}

void HK_CALL hkEntityCallbackUtil::fireContactProcess( hkEntity* entity, hkContactProcessEvent& event )
{
	if ( entity->m_collisionListeners.getSize())
	{
		fireContactProcessInternal( entity, event );
	}
}
#endif // HK_DYNAMICS2_ENTITY_CALLBACK_UTIL_H

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
