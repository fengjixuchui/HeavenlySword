/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/agent/null/hkNullAgent.h>
#include <hkcollide/dispatch/hkAgentRegisterUtil.h>


//#include <hkcollide/dispatch/contactmgr/hkNullContactMgrFactory.h>

//static hkNullContactMgr hkNullContactMgr;
static hkNullAgent hkNullAgentInstance;

hkNullAgent::hkNullAgent()
:	hkCollisionAgent( HK_NULL )
{
}

void HK_CALL hkNullAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  )
{
	HK_ON_DEBUG( const char* typeA = hkGetShapeTypeName( bodyA.getShape()->getType() ) );
	HK_ON_DEBUG( const char* typeB = hkGetShapeTypeName( bodyB.getShape()->getType() ) );
	HK_WARN_ONCE(0x3ad17e8b,  "Have you called hkAgentRegisterUtil::registerAllAgents?\n" \
								"Do not know how to get closest points between " << typeA << " and " << typeB << " types.");
}

void HK_CALL hkNullAgent::staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_ON_DEBUG( const char* typeA = hkGetShapeTypeName( bodyA.getShape()->getType() ) );
	HK_ON_DEBUG( const char* typeB = hkGetShapeTypeName( bodyB.getShape()->getType() ) );
	HK_WARN_ONCE(0x3ad17e8c,  "Have you called hkAgentRegisterUtil::registerAllAgents?\n" \
								"Do not know how to get penetrations for " << typeA << " and " << typeB << " types.");
}

void HK_CALL hkNullAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_ON_DEBUG( const char* typeA = hkGetShapeTypeName( bodyA.getShape()->getType() ) );
	HK_ON_DEBUG( const char* typeB = hkGetShapeTypeName( bodyB.getShape()->getType() ) );
	HK_WARN_ONCE(0x3ad17e8d,  "Have you called hkAgentRegisterUtil::registerAllAgents?\n" \
								"Do not know how to make linear casting between " << typeA << " and " << typeB << " types.");
}

hkCollisionAgent* HK_CALL hkNullAgent::createNullAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
												const hkCollisionInput& input, hkContactMgr* mgr )
{
	HK_ON_DEBUG( const char* typeA = hkGetShapeTypeName( bodyA.getShape()->getType() ) );
	HK_ON_DEBUG( const char* typeB = hkGetShapeTypeName( bodyB.getShape()->getType() ) );
	HK_WARN_ONCE(0x3ad17e8a,  "Have you called hkAgentRegisterUtil::registerAllAgents?\n" \
								"Do not know how to dispatch types " << typeA << " vs " << typeB);
	return &hkNullAgentInstance;
}

hkNullAgent* HK_CALL hkNullAgent::getNullAgent()
{
	return &hkNullAgentInstance;
}


/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
