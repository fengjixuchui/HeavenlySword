/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_HEIGHT_FIELD_COLLISION_SPHERES_AGENT_H
#define HK_COLLIDE2_HEIGHT_FIELD_COLLISION_SPHERES_AGENT_H

#include <hkcollide/agent/linearcast/hkIterativeLinearCastAgent.h>

	/// This agent performs the collision between a set of spheres and a 3 dimensional function
class hkHeightFieldAgent : public hkCollisionAgent
{
	public:
			/// register the this agent with everything
		static void HK_CALL registerAgent(hkCollisionDispatcher* dispatcher);

			// hkCollisionAgent interface implementation.
        virtual inline void processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

			// hkCollisionAgent interface implementation.
		virtual void getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetPenetrations(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector); 
			
			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  );


		virtual void linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector );

		static void HK_CALL staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector );


			/// hkAgent interface implementation
		virtual void cleanup( hkCollisionConstraintOwner& constraintOwner );

	protected:
			/// Constructor, called by the createXXX functions
		hkHeightFieldAgent(const hkCdBody& A,const  hkCdBody& B,const  hkCollisionInput& input, hkContactMgr* mgr);

			/// Destructor
		~hkHeightFieldAgent(){}

	protected:
			/// create functions, known by the hkCollisionDispatcher
		static hkCollisionAgent* HK_CALL createHeightFieldAAgent( const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkContactMgr* mgr );
		static hkCollisionAgent* HK_CALL createHeightFieldBAgent( const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkContactMgr* mgr );

		hkArray<hkContactPointId> m_contactPointId;
};

#endif // HK_COLLIDE2_HEIGHT_FIELD_COLLISION_SPHERES_AGENT_H

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
