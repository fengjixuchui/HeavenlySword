/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_WORLD_CALLBACK_UTIL_H
#define HK_DYNAMICS2_WORLD_CALLBACK_UTIL_H

class hkWorld;
struct hkContactPointAddedEvent;
struct hkContactPointRemovedEvent;
struct hkContactProcessEvent;
struct hkContactPointConfirmedEvent;
class hkStepInfo;
class hkPhantom;

class hkWorldCallbackUtil
{
	public:

		static void HK_CALL fireActionAdded( hkWorld* world, hkAction* action ) ;
		static void HK_CALL fireActionRemoved( hkWorld* world, hkAction* action ) ;

		static void HK_CALL fireEntityAdded( hkWorld* world, hkEntity* entity ) ;
		static void HK_CALL fireEntityRemoved( hkWorld* world, hkEntity* entity ) ;
		static void HK_CALL fireEntityShapeSet( hkWorld* world, hkEntity* entity ) ;

		static void HK_CALL firePhantomAdded( hkWorld* world, hkPhantom* phantom );
		static void HK_CALL firePhantomRemoved( hkWorld* world, hkPhantom* phantom );
		static void HK_CALL firePhantomShapeSet( hkWorld* world, hkPhantom* phantom ) ;

		static void HK_CALL fireConstraintAdded( hkWorld* world, hkConstraintInstance* constraint ) ;
		static void HK_CALL fireConstraintRemoved( hkWorld* world, hkConstraintInstance* constraint ) ;

		static void HK_CALL fireContactPointAdded( hkWorld* world, hkContactPointAddedEvent& event);
		static void HK_CALL fireContactPointConfirmed( hkWorld* world, hkContactPointConfirmedEvent& event );
		static void HK_CALL fireContactPointRemoved( hkWorld* world, hkContactPointRemovedEvent& event );
		static void HK_CALL fireContactProcess( hkWorld* world, hkContactProcessEvent& event );

		static void HK_CALL firePostSimulationCallback( hkWorld* world );
		static void HK_CALL firePostIntegrateCallback( hkWorld* world, const hkStepInfo& info );
		static void HK_CALL firePostCollideCallback( hkWorld* world, const hkStepInfo& info );

			// This fires both island and entity activation callbacks.
		static void HK_CALL fireIslandActivated( hkWorld* world, hkSimulationIsland* island );
			// This fires both island and entity deactivation callbacks.
		static void HK_CALL fireIslandDeactivated( hkWorld* world, hkSimulationIsland* island );

		static void HK_CALL fireIslandPostIntegrateCallback( hkWorld* world, hkSimulationIsland* island, const hkStepInfo& info );
		static void HK_CALL fireIslandPostCollideCallback( hkWorld* world, hkSimulationIsland* island, const hkStepInfo& info );

		static void HK_CALL fireWorldDeleted( hkWorld* world ) ;

		static void HK_CALL fireInactiveEntityMoved( hkWorld* world, hkEntity* entity);

		//static void HK_CALL fireMaxVelocityExceeded( hkWorld* world, hkEntity* entity );


};

#endif // HK_DYNAMICS2_WORLD_CALLBACK_UTIL_H

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
