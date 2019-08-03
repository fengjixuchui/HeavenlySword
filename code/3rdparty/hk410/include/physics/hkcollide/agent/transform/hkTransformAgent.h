/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_TRANSFORM_AGENT_H
#define HK_COLLIDE2_TRANSFORM_AGENT_H

#include <hkcollide/agent/hkCollisionAgent.h>

class hkCollisionDispatcher;

	/// This agent handles collisions between hkTransformShape and other shapes.
class hkTransformAgent : public hkCollisionAgent
{
	public:

			/// Registers this agent with the collision dispatcher.
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);

			// hkCollisionAgent interface implementation.
		virtual void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);
		
			// hkCollisionAgent interface implementation.
		virtual void linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector ); 

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector ); 
		
			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& pointDetails); 
	
			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  );

			// hkCollisionAgent interface implementation.
		virtual void getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		virtual void cleanup( hkCollisionConstraintOwner& constraintOwner );

			// hkCollisionAgent interface implementation.
		virtual void invalidateTim(hkCollisionInput& input);

			// hkCollisionAgent interface implementation.
		virtual void warpTime(hkTime oldTime, hkTime newTime, hkCollisionInput& input);

		// hkCollisionAgent interface implementation.
		virtual void removePoint( hkContactPointId idToRemove );

		// hkCollisionAgent interface implementation.
		virtual void commitPotential( hkContactPointId newId );

		// hkCollisionAgent interface implementation.
		virtual void createZombie( hkContactPointId idTobecomeZombie );

	protected:
		HK_FORCE_INLINE hkTransformAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);

		virtual void updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner );

			/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createTransformAAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);
		static hkCollisionAgent* HK_CALL createTransformBAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);

	protected:
		hkCollisionAgent*	m_childAgent;
};



#endif // HK_COLLIDE2_TRANSFORM_AGENT_H

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
