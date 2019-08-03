/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_Capsule_TRIANGLE_AGENT_H
#define HK_COLLIDE2_Capsule_TRIANGLE_AGENT_H

#include <hkcollide/agent/linearcast/hkIterativeLinearCastAgent.h>
#include <hkinternal/collide/util/hkCollideTriangleUtil.h>

class hkCollisionDispatcher;

	/// This agent handles collisions between hkCapsules and hkTriangles.
class hkCapsuleTriangleAgent : public hkIterativeLinearCastAgent
{
    public:
        
			///Registers this agent with the collision dispatcher.
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);

			// hkCollisionAgent interface implementation.
        virtual inline void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& pointDetails); 
			
			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  );

			// hkCollisionAgent interface implementation.
		virtual void getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		virtual void cleanup(hkCollisionConstraintOwner& constraintOwner);

	protected:

		friend class hkCapsuleConvexWelderAgent;		
	
		/// Constructor, called by the agent creation functions.
		HK_FORCE_INLINE hkCapsuleTriangleAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

        /// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createCapsuleTriangleAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);
        
		/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createTriangleCapsuleAgent(	const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);
				
	private:
		
		/// Returns two candidates of the manifold. If searchManifold = true than an additional third point might be returned
		/// Non-inlined  version for hkCapsuleConvexWelderAgent
		static void HK_CALL getClosestPointsPublic( const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkCollideTriangleUtil::PointTriangleDistanceCache& cache, int searchManifold, hkContactPoint* points );		
		
		/// Returns two candidates of the manifold. If searchManifold = true than an additional third point might be returned
		static HK_FORCE_INLINE void HK_CALL getClosestPointsInl( const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkCollideTriangleUtil::PointTriangleDistanceCache& cache, int searchManifold, hkContactPoint* points );		

	public:
		enum ClosestPointResult
		{
			ST_CP_MISS,
			ST_CP_HIT,
		};
	private:
		static HK_FORCE_INLINE ClosestPointResult getClosestPointInternal( const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollideTriangleUtil::PointTriangleDistanceCache& m_cache, hkContactPoint& cpoint);
	public:

		static ClosestPointResult HK_CALL getClosestPoint( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollideTriangleUtil::PointTriangleDistanceCache& m_cache, hkContactPoint& cpoint);


	protected:
		hkContactPointId m_contactPointId[3];
		hkCollideTriangleUtil::PointTriangleDistanceCache m_triangleCache;

};

#endif // HK_COLLIDE2_Capsule_TRIANGLE_AGENT_H

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
