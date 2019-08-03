/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_GSKF_AGENT_H
#define HK_COLLIDE2_GSKF_AGENT_H

#include <hkinternal/collide/gjk/gskmanifold/hkGskManifold.h>
#include <hkcollide/agent/gjk/hkGskBaseAgent.h>

class hkCollisionDispatcher;
struct hkExtendedGskOut;

	/// The hkGskfAgent is an hkGskBaseAgent agent which uses a highly optimized
	/// feature based manifold. This manifold works really well for big triangles
	/// and has no parameters to tune
class hkGskfAgent : public hkGskBaseAgent
{
	public:
			

			/// Registers the agent with the collision dispatcher.
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);

			// hkCollisionAgent interface implementation.
		virtual void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

			// hkCollisionAgent interface implementation.
		virtual void cleanup( hkCollisionConstraintOwner& constraintOwner );

			// hkCollisionAgent interface implementation.
		virtual void removePoint( hkContactPointId idToRemove );

			// hkCollisionAgent interface implementation.
		virtual void commitPotential( hkContactPointId newId );

			// hkCollisionAgent interface implementation.
		virtual void createZombie( hkContactPointId idTobecomeZombie );

	protected:

		friend class hkConvexConvexWelderAgent;

			//
			// Note: to use the next two inline functions include the .inl file
			//
			/// Constructor, called by the createGskConvexConvexAgent() function.
		hkGskfAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, hkContactMgr* mgr);

		/// Destructor, called by cleanup().
		~hkGskfAgent(){}

			// hkCollisionAgent interface implementation.
		void processCollisionNoTim(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

	public:
		/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createGskfAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);

	protected:
	public:
		hkGskManifold m_manifold;

};


#endif // HK_COLLIDE2_GSKF_AGENT_H

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
