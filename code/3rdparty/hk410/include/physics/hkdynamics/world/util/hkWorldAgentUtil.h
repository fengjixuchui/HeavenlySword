/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_WORLD_AGENT_UTIL_H
#define HK_DYNAMICS2_WORLD_AGENT_UTIL_H

#include <hkdynamics/world/hkWorldCinfo.h>

class hkCollidablePair;
class hkCollidable;
struct hkCollisionInput;
struct hkAgentNnTrack;
struct hkAgent1nTrack;

class hkWorldAgentUtil
{
	public:
		static hkAgentNnEntry* HK_CALL addAgent( hkLinkedCollidable* collA, hkLinkedCollidable* collB, const hkProcessCollisionInput& input );

		static void HK_CALL removeAgent ( hkAgentNnEntry* agent );

		static void HK_CALL removeAgentAndItsToiEvents ( hkAgentNnEntry* agent );

		static hkSimulationIsland* HK_CALL getIslandFromAgentEntry( hkAgentNnEntry* entry, hkSimulationIsland* candidateA, hkSimulationIsland* candidateB);

		static void HK_CALL updateEntityShapeCollectionFilter( hkEntity* entity, hkCollisionInput& collisionInput );

		static void HK_CALL invalidateTim( hkEntity* entity, hkCollisionInput& collisionInput );

		static void HK_CALL warpTime( hkSimulationIsland* island, hkTime oldTime, hkTime newTime, hkCollisionInput& collisionInput );
};


#endif // HK_DYNAMICS2_WORLD_AGENT_UTIL_H

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
