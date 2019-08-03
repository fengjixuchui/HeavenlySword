/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/mopp/hkMoppBvTreeShape.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>


#include <hkcollide/agent/bvtree/hkMoppAgent.h>
#include <hkcollide/agent/bvtreestream/hkMoppBvTreeStreamAgent.h>

#include <hkinternal/collide/mopp/machine/hkMoppAabbCastVirtualMachine.h>

#include <hkcollide/dispatch/hkAgentDispatchUtil.h>

#ifdef HK_MOPP_DEBUGGER_ENABLED
#	include <hkcollide/collector/pointcollector/hkClosestCdPointCollector.h>
#endif

#include <hkbase/htl/hkAlgorithm.h>


void HK_CALL hkMoppBvTreeStreamAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc           = hkBvTreeStreamAgent::createBvTreeShapeAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkMoppAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc  = hkSymmetricAgent<hkMoppAgent>::staticGetClosestPoints;
		af.m_linearCastFunc       = hkSymmetricAgent<hkMoppAgent>::staticLinearCast;
		af.m_isFlipped            = true;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_MOPP, HK_SHAPE_CONVEX );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          =  hkBvTreeStreamAgent::createShapeBvAgent;
		af.m_getPenetrationsFunc =  hkMoppAgent::staticGetPenetrations;
		af.m_getClosestPointFunc =  hkMoppAgent::staticGetClosestPoints;
		af.m_linearCastFunc      =  hkMoppAgent::staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_CONVEX, HK_SHAPE_MOPP );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = hkMoppAgent::createBvBvAgent;
		af.m_getPenetrationsFunc = hkMoppAgent::staticGetPenetrations;
		af.m_getClosestPointFunc = hkMoppAgent::staticGetClosestPoints;
		af.m_linearCastFunc      = hkMoppAgent::staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_MOPP, HK_SHAPE_MOPP );
	}
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
