/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_PREDICTIVE_WRAPPER_AGENT_H
#define HK_COLLIDE2_PREDICTIVE_WRAPPER_AGENT_H

#include <hkcollide/agent/gjk/hkGskfAgent.h>
#include <hkinternal/collide/gjk/gskmanifold/hkGskManifold.h>

class hkCollisionDispatcher;

	/// This agent is a hkGskfAgent which handles continuous collision detection .
class hkPredGskfAgent : public hkGskfAgent
{
	public:
			/// Registers this agent with the collision dispatcher.
		static void HK_CALL registerAgent( hkCollisionDispatcher* dispatcher );


			// hkCollisionAgent interface implementation.
		virtual void processCollision( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result );

	public:

			/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createPredGskfAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

	public:
		HK_FORCE_INLINE hkPredGskfAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );
};


hkPredGskfAgent::hkPredGskfAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& inputIn, hkContactMgr* mgr )
:	hkGskfAgent( bodyA, bodyB, mgr )
{
}



#endif // HK_COLLIDE2_PREDICTIVE_WRAPPER_AGENT_H

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
