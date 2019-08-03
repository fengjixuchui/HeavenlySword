/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CAPSULE_TRIANGLE_AGENT3_H
#define HK_COLLIDE2_CAPSULE_TRIANGLE_AGENT3_H

#include <hkinternal/collide/agent3/hkAgent3.h>

class hkCollisionDispatcher;

	/// a streamed agent implementing capsule triangle collisions.
	/// This agent is a discrete agent only
namespace hkCapsuleTriangleAgent3
{
	hkAgentData* create  ( const hkAgent3Input& input, hkAgentEntry* entry, hkAgentData* freeMemory );

	hkAgentData* process ( const hkAgent3ProcessInput& input, hkAgentEntry* entry, hkAgentData* agentData, hkVector4* separatingNormal, hkProcessCollisionOutput& result);

	void sepNormal( const hkAgent3Input& input, hkAgentData* agentData, hkVector4& separatingNormalOut );

	hkAgentData* cleanup ( hkAgentEntry* entry, hkAgentData* agentData, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner );

	void removePoint ( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId idToRemove );

	void commitPotential ( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId idToRemove );

	void createZombie( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId idToConvert );

	void destroy ( hkAgentEntry* entry, hkAgentData* agentData, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner );

	void registerAgent3( hkCollisionDispatcher* dispatcher );

}

#endif // HK_COLLIDE2_CAPSULE_TRIANGLE_AGENT3_H

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
