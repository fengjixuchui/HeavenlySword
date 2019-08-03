/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_BV_TREE_SHAPE_AGENT_H
#define HK_COLLIDE2_BV_TREE_SHAPE_AGENT_H

#include <hkcollide/agent/hkCollisionAgent.h>
#include <hkmath/basetypes/hkAabb.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>

class hkCollisionDispatcher;


	/// This agent deals with collisions between hkBvTreeShapes and other shapes.  It traverses the bounding volume tree and dispatches 
	/// collision agents for those child shapes that are found to be collision candidates with the other shape.
	/// It assumes that bodyB is the bvtree shape
class hkBvTreeAgent : public hkCollisionAgent
{
	public:
			
			/// Registers this agent with the collision dispatcher.
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);
		
			// hkCollisionAgent interface implementation.
		virtual void getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector ) ;

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector ) ;

			// hkCollisionAgent interface implementation.
		virtual void linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector );

			// hkCollisionAgent interface implementation.
		virtual void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

			// hkCollisionAgent interface implementation.
		virtual void cleanup(hkCollisionConstraintOwner& info);

			/// Aabb caching will skip queries to the hkShapeCollection if the aabb's between the two colliding bodies has not changed
			/// You need to disable this if your hkShapeCollection has changed, i.e. elements have been added or removed
		static void HK_CALL setUseAabbCaching( hkBool useIt );

			/// Aabb caching will skip queries to the hkShapeCollection if the aabb's between the two colliding bodies has not changed
		static hkBool HK_CALL getUseAabbCaching();


	public:
		static hkBool m_useFastUpdate;
		
		virtual void updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner );

			// hkCollisionAgent interface implementation.
		virtual void invalidateTim(hkCollisionInput& input);

			// hkCollisionAgent interface implementation.
		virtual void warpTime(hkTime oldTime, hkTime newTime, hkCollisionInput& input);

		HK_FORCE_INLINE static hkResult HK_CALL calcAabbAndQueryTree( 
											const hkCdBody& bodyA,	const hkCdBody& bodyB, const hkTransform& bTa,
											const hkVector4& linearTimInfo, const hkProcessCollisionInput& input,
											hkAabb* cachedAabb, hkArray<hkShapeKey>& hitListOut );
	public:

		/// Constructor, called by the agent creation function.
		hkBvTreeAgent( hkContactMgr* mgr );

	protected:

		void calcStatistics( hkStatisticsCollector* collector) const;


		static hkCollisionAgent* HK_CALL defaultAgentCreate( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& env, hkContactMgr* mgr );


			// interal method
		void prepareCollisionPartners          ( const hkCdBody& bodyA, const hkCdBody& bodyB,  const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner);
		HK_FORCE_INLINE void prepareCollisionPartnersProcess   ( const hkCdBody& bodyA,	const hkCdBody& bodyB,	const hkProcessCollisionInput& input, hkCollisionConstraintOwner& constraintOwner);
		//void prepareCollisionPartnersLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB,  const hkLinearCastCollisionInput& input, hkCollisionConstraintOwner& constraintOwner);

		static inline void HK_CALL calcAabbLinearCast(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkAabb& aabbOut);

		static inline void HK_CALL staticCalcAabb(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkAabb& aabbOut);



		///Destructor, called by cleanup().
		~hkBvTreeAgent(){}

		struct hkBvAgentEntryInfo
		{
			hkShapeKey m_key;
			hkInt32 m_userInfo;
			hkCollisionAgent* m_collisionAgent;	

			hkShapeKey& getKey() { return m_key; }
			void setKey( hkShapeKey& key) { m_key = key; }
		};

		hkArray<hkBvAgentEntryInfo> m_collisionPartners;
		hkAabb	m_cachedAabb;

		/// you'll need to set this to false if you want changes to your ShapeCollection, which do not alter the aabb, to be reflected
		/// in the collision detection.
		static hkBool m_useAabbCaching;

		
			/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createShapeBvAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

			/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createBvTreeShapeAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

			/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createBvBvAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr );

};

#endif // HK_COLLIDE2_BV_TREE_SHAPE_AGENT_H

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
