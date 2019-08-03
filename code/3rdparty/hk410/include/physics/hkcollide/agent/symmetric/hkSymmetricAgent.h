/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SYMMETRIC_AGENT_H
#define HK_COLLIDE2_SYMMETRIC_AGENT_H


#include <hkcollide/agent/symmetric/hkSymmetricAgentLinearCast.h>


	/// A symmetric agent is used to wrap a non symmetric agent with its mirrored type. The collision dispatcher
	/// uses this class when creating agents that deal with two different shape types - for instance, the hkSphereTriangleAgent.
	/// Note: if you don't want to implement the linearCast in your Agent,
    /// then you can subclass from hkIterativeLinearCaster,
	/// however instead of using the hkSymmetricAgent, you have to use hkSymmetricAgentLinearCast
	///
	/// Reason for existance of this class:
	/// hkSymmetricAgent only adds implementation of virtual void linearCast( ..) to its base class -- hkSymmetricAgentLinearCast.
	/// Implementation of this function is missing in hkSymmetricAgentLinearCast. The reason is for this class to be usable
	/// by agent's that do not implement their own virtual void linearCast(..) method and use the implementation provided 
	/// by hkIterativeLinearCastAgent. That implementation already uses the virtual getClosesPoints() function which has input hkCdBodies flipped
	/// (as it is defined in the hkSymmetricAgentLinearCast). Therefore virtual void linearCast(..) it's not overridden in hkSymmetricAgentLinearCast
	/// to avoid double-flipping of input hkCdBodies.
template<typename AGENT>
class hkSymmetricAgent : public hkSymmetricAgentLinearCast<AGENT>
{
	public:

		HK_FORCE_INLINE hkSymmetricAgent( hkContactMgr* mgr ): hkSymmetricAgentLinearCast<AGENT>(mgr){}

			/// Called by the appropriate createAgent() function.
		HK_FORCE_INLINE hkSymmetricAgent( const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkContactMgr* mgr );

			// hkCollisionAgent interface implementation.
		virtual void linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector ); 

};



#include <hkcollide/agent/symmetric/hkSymmetricAgent.inl>

#endif // HK_COLLIDE2_SYMMETRIC_AGENT_H

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
