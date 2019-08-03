/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_BV_TREE_SHAPE_STREAM_AGENT_H
#define HK_COLLIDE2_BV_TREE_SHAPE_STREAM_AGENT_H

#include <hkcollide/agent/bvtree/hkBvTreeAgent.h>
#include <hkinternal/collide/agent3/machine/1n/hkAgent1nTrack.h>

	/// This agent deals with collisions between hkBvTreeShapes and other shapes.  It traverses the bounding volume tree and dispatches 
	/// collision agents for those child shapes that are found to be collision candidates with the other shape.
	/// The difference to hkBvTreeAgent is that this agent uses a memory stream to store the agents.
	/// As a result, memory consumption and fragmentation is reduced significantly.
	/// However only hkPredGskAgent3 and hkCapsuleTriangleAgent3 are supporting this technology.
	/// This Agent as well handles welding of inner landscape edges.
class hkBvTreeStreamAgent : public hkCollisionAgent
{
	public:

			/// Registers this agent with the collision dispatcher.
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);
		
		static void HK_CALL registerConvexListAgent(hkCollisionDispatcher* dispatcher);

		static void HK_CALL registerMultiRayAgent(hkCollisionDispatcher* dispatcher);

			// hkCollisionAgent interface implementation.
		virtual void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

			// hkCollisionAgent interface implementation.
		virtual void cleanup(hkCollisionConstraintOwner& info);

		
			// hkCollisionAgent interface implementation.
		virtual void getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );


			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector ) ;


			// hkCollisionAgent interface implementation.
		virtual void linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector );

			// hkCollisionAgent interface implementation.
		virtual void updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner );

			// hkCollisionAgent interface implementation.
		virtual void invalidateTim(hkCollisionInput& input);

			// hkCollisionAgent interface implementation.
		virtual void warpTime(hkTime oldTime, hkTime newTime, hkCollisionInput& input);

	protected:
		hkResult prepareCollisionPartnersProcess( hkAgent3ProcessInput& input, hkArray<hkShapeKey>& hitList );

		/// Constructor, called by the agent creation function.
		hkBvTreeStreamAgent( const hkCdBody& bodyA, 	const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

		///Destructor, called by cleanup().
		~hkBvTreeStreamAgent(){}

		void calcStatistics( hkStatisticsCollector* collector) const;
		
	public:
			/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createShapeBvAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

		/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createBvTreeShapeAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

			// Extra convex list dispatch functions
		static hkCollisionAgent* HK_CALL dispatchBvTreeConvexList( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );
		static hkCollisionAgent* HK_CALL dispatchConvexListBvTree( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );


	protected:
		hkCollisionDispatcher* m_dispatcher;
		hkAabb	m_cachedAabb;
		hkAgent1nTrack m_agentTrack;
};

#endif // HK_COLLIDE2_BV_TREE_SHAPE_STREAM_AGENT_H

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
