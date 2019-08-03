/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CONVEX_LIST_AGENT_H
#define HK_COLLIDE2_CONVEX_LIST_AGENT_H

#include <hkinternal/collide/gjk/gskmanifold/hkGskManifold.h>
#include <hkcollide/agent/gjk/hkPredGskfAgent.h>
#include <hkinternal/collide/agent3/machine/1n/hkAgent1nTrack.h>

class hkCollisionDispatcher;
struct hkExtendedGskOut;
struct hkCollisionInput;

	/// A hkConvexListAgent handles collisions between a hkConvexShape and a list of other convex shapes
class hkConvexListAgent : public hkPredGskfAgent
{
	public:
			/// Registers the agent with the collision dispatcher.
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);

			// hkCollisionAgent interface implementation.
		virtual void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

			// hkCollisionAgent interface implementation.
		virtual void linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector ); 

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector ); 

			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector ) ;

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector ) ;

			// hkCollisionAgent interface implementation.
		virtual void getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		virtual void updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner );

			// hkCollisionAgent interface implementation.
		virtual void invalidateTim(hkCollisionInput& input);

			// hkCollisionAgent interface implementation.
		virtual void warpTime(hkTime oldTime, hkTime newTime, hkCollisionInput& input);

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
		hkConvexListAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const  hkCollisionInput& input, hkContactMgr* mgr );

		/// Destructor, called by cleanup().
		~hkConvexListAgent(){}


		void switchToGskMode(hkCollisionConstraintOwner& constraintOwner);

		void switchToStreamMode(hkCollisionConstraintOwner& constraintOwner);

	public:
		/// Agent creation function used by the hkCollisionDispatcher. 
		static hkCollisionAgent* HK_CALL createConvexConvexListAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB, const  hkCollisionInput& input, hkContactMgr* mgr);
		static hkCollisionAgent* HK_CALL createConvexListConvexAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB, const  hkCollisionInput& input, hkContactMgr* mgr);
		static hkCollisionAgent* HK_CALL createConvexListConvexListAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB, const  hkCollisionInput& input, hkContactMgr* mgr);

	protected:
		hkCollisionDispatcher* m_dispatcher;

		hkBool		m_inGskMode;
		hkBool      m_processFunctionCalled; 
		hkInt16		m_inStreamModeCounter;

		struct StreamData
		{
			hkAgent1nTrack  m_agentTrack;
			hkReal		    m_inStreamModeTimDist;
		};

		StreamData& getStream()
		{ 
			HK_ASSERT( 0xf0457534, m_inGskMode == false );
			return *reinterpret_cast<StreamData*>(&m_manifold);
		}
};


#endif // HK_COLLIDE2_CONVEX_LIST_AGENT_H

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
