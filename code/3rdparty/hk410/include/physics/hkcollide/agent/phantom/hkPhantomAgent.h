/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_PHANTOM_AGENT_H
#define HK_COLLIDE2_PHANTOM_AGENT_H

#include <hkcollide/agent/hkCollisionAgent.h>
#include <hkcollide/shape/hkShapeType.h>
class hkCollisionDispatcher;
class hkPhantomCallbackShape;
class hkCollidable;

	/// An hkPhantomAgent handles collisions between hkPhantomShapes and any other shapes. 
	/// No collision response takes place, but events will still be fired.
class hkPhantomAgent : public hkCollisionAgent
{
	public:

			/// Registers this agent with the collision dispatcher.
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);

			// hkCollisionAgent interface implementation.
		virtual void getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		virtual void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);
	
			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& pointDetails);

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  );

			// hkCollisionAgent interface implementation.
		virtual void linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector );



			// hkCollisionAgent interface implementation.
		virtual void cleanup( hkCollisionConstraintOwner& constraintOwner );

	protected:
		
		/// Constructor, called by the agent creation function.
		hkPhantomAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, hkContactMgr* contactMgr);

			/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createPhantomAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);

		/// Creates no agent, but forwards the call to the phantom shape.
		static hkCollisionAgent* HK_CALL createNoPhantomAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);

		const hkCollidable* m_collidableA;
		const hkCollidable* m_collidableB;

		hkPhantomCallbackShape* m_shapeA;
		hkPhantomCallbackShape* m_shapeB;

		hkShapeType m_bodyTypeA;
		hkShapeType m_bodyTypeB;
};



#endif // HK_COLLIDE2_PHANTOM_AGENT_H

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
