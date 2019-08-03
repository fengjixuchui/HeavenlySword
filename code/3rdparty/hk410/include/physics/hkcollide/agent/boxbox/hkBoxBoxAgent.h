/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_BOX_BOX_AGENT_H
#define HK_COLLIDE2_BOX_BOX_AGENT_H

#include <hkcollide/agent/linearcast/hkIterativeLinearCastAgent.h>
#include <hkcollide/agent/boxbox/hkBoxBoxManifold.h>


class hkCollisionDispatcher;
	/// An hkBoxBoxAgent handles collisions between pairs of hkBoxShapes. This produces high quality manifolds, in many cases with a good CPU
	/// Note: The implementation of this Agent handles vertex-face and edge-edge combinations, it does not handle vertex-vertex situations.
	/// That means, if you are using getClosestPoints() with a tolerance >= 0.0, the closest distance to a box might actually be outside the box.
	/// So if you are heavily relying on getClosestPoints() with long distances, you should not enable the hkBoxBoxAgent in the hkAgentRegisterUtil
class hkBoxBoxAgent : public hkIterativeLinearCastAgent
{
	public:

			/// Registers this agent with the collision dispatcher.
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);

			// hkCollisionAgent interface implementation.
		virtual void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector ) ;

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector ) ;

			// hkCollisionAgent interface implementation.
		virtual void getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		virtual void cleanup(hkCollisionConstraintOwner& info);

			/// Setting this true forces the box-box detection to try all edge combinations for additional contact points immediately after one contact point is found.
			/// This improves the fidelity of the simulation at the cost of CPU cycles.  Defaults to false.
		inline static void HK_CALL setAttemptToFindAllEdges( hkBool findEm )
		{
			m_attemptToFindAllEdges = findEm;
		}

			/// If this is true, it forces the box-box detection to try all edge combinations for additional contact points immediately after one contact point is found.
			/// This improves the fidelity of the simulation at the cost of CPU cycles.  Defaults to false.
		inline static hkBool HK_CALL getAttemptToFindAllEdges()
		{
			return m_attemptToFindAllEdges;
		}

	protected:
			/// Called by createBoxBoxAgent().
		HK_FORCE_INLINE hkBoxBoxAgent( hkContactMgr* contactMgr ):  hkIterativeLinearCastAgent(contactMgr){}

			/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createBoxBoxAgent(	const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* contactMgr);


	protected:
		hkBoxBoxManifold m_manifold;

		static hkBool m_attemptToFindAllEdges;
};

#endif // HK_COLLIDE2_BOX_BOX_AGENT_H

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
