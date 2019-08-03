/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SPHERE_TRIANGLE_AGENT_H
#define HK_COLLIDE2_SPHERE_TRIANGLE_AGENT_H

#include <hkcollide/agent/linearcast/hkIterativeLinearCastAgent.h>
#include <hkinternal/collide/util/hkCollideTriangleUtil.h>

class hkCollisionDispatcher;

	/// This agent handles collisions between hkSphereShape and hkTriangleShape
class hkSphereTriangleAgent : public hkIterativeLinearCastAgent
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
		virtual void cleanup(hkCollisionConstraintOwner& info);

	protected:
		
		/// Constructor, called by the agent creation functions.
		HK_FORCE_INLINE hkSphereTriangleAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

        /// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createSphereTriangleAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);
        
		/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createTriangleSphereAgent(	const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr);


	public:

		enum ClosestPointResult
		{
			ST_CP_MISS,
			ST_CP_FACE,
			ST_CP_EDGE
		};

		static ClosestPointResult HK_CALL getClosestPoint( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollideTriangleUtil::ClosestPointTriangleCache& m_cache, hkContactPoint& cpoint);

		static HK_FORCE_INLINE ClosestPointResult getClosestPointInl( const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollideTriangleUtil::ClosestPointTriangleCache& m_cache, hkContactPoint& cpoint);

		hkContactPointId m_contactPointId;

		hkCollideTriangleUtil::ClosestPointTriangleCache m_closestPointTriangleCache;

};

#endif // HK_COLLIDE2_SPHERE_TRIANGLE_AGENT_H

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
