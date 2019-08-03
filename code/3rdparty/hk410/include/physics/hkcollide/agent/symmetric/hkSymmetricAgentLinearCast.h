/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SYMMETRIC_AGENT_LINEAR_CAST_H
#define HK_COLLIDE2_SYMMETRIC_AGENT_LINEAR_CAST_H

#include <hkcollide/agent/hkProcessCollisionOutput.h>
#include <hkcollide/agent/hkCdPointCollector.h>
#include <hkcollide/agent/hkCdBodyPairCollector.h>

class hkCdBody;
struct hkCollisionInput;
class hkContactMgr;


	/// A special symmetric agent, which should be used instead of the hkSymmetricAgent if you derive
	/// the agent from the hkIterativeLinearCastAgent
template<typename AGENT>
class hkSymmetricAgentLinearCast : public AGENT
{
	public:

		HK_FORCE_INLINE hkSymmetricAgentLinearCast( hkContactMgr* mgr ): AGENT(mgr){}

			/// Called by the appropriate createAgent() function.
		HK_FORCE_INLINE hkSymmetricAgentLinearCast( const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkContactMgr* mgr );

			// hkCollisionAgent interface implementation.
		virtual void getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			// hkCollisionAgent interface implementation.
		virtual void getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  ) ;


			// hkCollisionAgent interface implementation.
		static void HK_CALL staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  ) ;

			// hkCollisionAgent interface implementation.
		static void HK_CALL staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector ); 

			// hkCollisionAgent interface forward call.
		virtual void processCollision(	const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result);

			// hkCollisionAgent interface forward call.
		virtual void updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner );

};

/// a private class to hkSymmetricAgent, declared outside, as this class is not templated
class hkSymmetricAgentFlipCollector: public hkCdPointCollector
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkCdPointCollector);
		hkCdPointCollector& m_collector;
		hkSymmetricAgentFlipCollector( hkCdPointCollector& c): m_collector(c){}
		virtual void addCdPoint( const hkCdPoint& point );
};

/// a private class to hkSymmetricAgent, declared outside, as this class is not templated
class hkSymmetricAgentFlipCastCollector: public hkCdPointCollector
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkSymmetricAgentFlipCastCollector);
		hkVector4 m_path;
		hkCdPointCollector& m_collector;

		hkSymmetricAgentFlipCastCollector( const hkVector4& path, hkCdPointCollector& c): m_path(path), m_collector(c){}
		virtual void addCdPoint( const hkCdPoint& point );
};

/// a private class to hkSymmetricAgent, declared outside, as this class is not templated
class hkSymmetricAgentFlipBodyCollector: public hkCdBodyPairCollector
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkSymmetricAgentFlipBodyCollector);
		hkCdBodyPairCollector& m_collector;
		hkSymmetricAgentFlipBodyCollector( hkCdBodyPairCollector& c): m_collector(c){}
		virtual void addCdBodyPair( const hkCdBody& bodyA, const hkCdBody& bodyB);
};


#include <hkcollide/agent/symmetric/hkSymmetricAgentLinearCast.inl>

#endif // HK_COLLIDE2_SYMMETRIC_AGENT_LINEAR_CAST_H

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
