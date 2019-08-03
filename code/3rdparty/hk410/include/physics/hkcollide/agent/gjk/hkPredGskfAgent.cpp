/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/debugutil/hkTraceStream.h>

#include <hkcollide/agent/hkCollisionAgentConfig.h>
#include <hkcollide/agent/hkCollisionInput.h>
#include <hkcollide/agent/gjk/hkPredGskfAgent.h>
#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkinternal/collide/gjk/gskmanifold/hkGskManifoldUtil.h>
#include <hkinternal/collide/gjk/hkGsk.h>
#include <hkinternal/collide/agent3/predgskagent3/hkPredGskAgent3.h>
#include <hkinternal/collide/gjk/continuous/hkContinuousGsk.h>

void HK_CALL hkPredGskfAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createPredGskfAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive        = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_CONVEX, HK_SHAPE_CONVEX );
	}

}


hkCollisionAgent* HK_CALL hkPredGskfAgent::createPredGskfAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
{
	hkGskBaseAgent* agent;
	if ( mgr )
	{
		agent = new hkPredGskfAgent(bodyA, bodyB, input, mgr);
	}
	else
	{
		agent = new hkGskBaseAgent( bodyA, bodyB, mgr );
	}
	return agent;
}


void hkPredGskfAgent::processCollision( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output )
{
	HK_TIME_CODE_BLOCK("Gsk", HK_NULL);
	HK_INTERNAL_TIMER_BEGIN_LIST("PredGskf","init");

	char names[2][128];
#	if defined HK_DEBUG_TOI
	{
		hkWorldObject* sA = static_cast<hkWorldObject*>( bodyA.getRootCollidable()->getOwner() );
		hkWorldObject* sB = static_cast<hkWorldObject*>( bodyB.getRootCollidable()->getOwner() );
		char* nameA; 
		char* nameB;
		if ( sA ){ nameA = sA->getName(); }else{ hkString::sprintf(names[0], "bodyA" ); nameA = names[0]; } 
		if ( sB ){ nameB = sB->getName(); }else{ hkString::sprintf(names[1], "bodyB" ); nameB = names[1]; } 
		hkToiPrintf("Tst", "#    Tst    %-6s %-6s\n", nameA, nameB );
	}
#endif

    {

		//
		//	validate separating plane
		//
		if ( ! (m_timeOfSeparatingNormal == input.m_stepInfo.m_startTime) )
		{
			if ( !input.m_collisionQualityInfo->m_useContinuousPhysics )
			{
				m_timeOfSeparatingNormal = input.m_stepInfo.m_endTime;
				goto PROCESS_AT_T1;
			}

			HK_INTERNAL_TIMER_BEGIN("recalcT0", this);
			hkTransform tA;
			hkTransform tB;
			hkCdBody bA( &bodyA, &tA ); bA.setShape( bodyA.getShape(), bodyA.getShapeKey());
			hkCdBody bB( &bodyB, &tB ); bB.setShape( bodyB.getShape(), bodyB.getShapeKey());
			hkSweptTransformUtil::lerp2( bodyA.getMotionState()->getSweptTransform(), input.m_stepInfo.m_startTime, tA );
			hkSweptTransformUtil::lerp2( bodyB.getMotionState()->getSweptTransform(), input.m_stepInfo.m_startTime, tB );
		
			const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
			const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());

			hkGsk gsk;
			gsk.init( shapeA, shapeB, m_cache );
			calcSeparatingNormal( bA, bB, input.m_collisionQualityInfo->m_keepContact, gsk, m_separatingNormal );
			gsk.checkForChangesAndUpdateCache( m_cache );
			HK_INTERNAL_TIMER_END();
		}

			// optimistically set the separatingNormal time to the end of the step
		m_timeOfSeparatingNormal = input.m_stepInfo.m_endTime;
		

		const hkMotionState* msA = bodyA.getMotionState();
		const hkMotionState* msB = bodyB.getMotionState();

		//
		//	Calc the relative movement for this timestep
		//
		hkVector4 timInfo; 	hkSweptTransformUtil::calcTimInfo( *msA, *msB, input.m_stepInfo.m_deltaTime, timInfo);

		hkReal realProjectedLinearDelta  = timInfo.dot3( m_separatingNormal );
		hkReal distAtT1 = m_separatingNormal(3) - realProjectedLinearDelta - timInfo(3);

		//
		//	Check for traditional tims
		//
		if ( distAtT1 > input.m_collisionQualityInfo->m_keepContact && distAtT1 > 0.5f * m_allowedPenetration )
		{
			hkToiPrintf("Tim", "#    Tim    %-6s %-6s        dist:%2.4f  \n", names[0], names[1], distAtT1 );
			HK_INTERNAL_TIMER_SPLIT_LIST("tim");

			m_separatingNormal(3) = distAtT1;
			if ( m_manifold.m_numContactPoints )
			{
				hkGskManifold_cleanup( m_manifold, m_contactMgr, *output.m_constraintOwner );
			}
			goto END_OF_FUNCTION;
		}

			//
			//  Check for normal operation
			//
		if ( input.m_collisionQualityInfo->m_useContinuousPhysics )
		{
			//
			//	Advance time using safe time steps
			//
			HK_INTERNAL_TIMER_SPLIT_LIST("toi");
	
			hkAgent3ProcessInput in3;
			in3.m_bodyA = & bodyA;
			in3.m_bodyB = & bodyB;
			in3.m_input = &input;
			in3.m_contactMgr = m_contactMgr;
			in3.m_distAtT1 = distAtT1;
			in3.m_linearTimInfo = timInfo;

			hkCollisionQualityInfo& qi = *input.m_collisionQualityInfo;
			const hkReal distance = m_separatingNormal(3);
			const hkReal minSeparation  = hkMath::min2( qi.m_minSeparation * m_allowedPenetration, distance + qi.m_minExtraSeparation * m_allowedPenetration );
			if (distAtT1 >= minSeparation)
			{
				goto QUICK_VERIFY_MANIFOLD;
			}
			const hkReal toiSeparation = hkMath::min2( qi.m_toiSeparation * m_allowedPenetration, distance + qi.m_toiExtraSeparation * m_allowedPenetration );

			hk4dGskCollideCalcToi( in3, m_allowedPenetration, minSeparation, toiSeparation, m_cache, m_separatingNormal, output );
		}
		else
		{
QUICK_VERIFY_MANIFOLD:
			// tim early out for manifolds
			if ( distAtT1 > input.m_collisionQualityInfo->m_manifoldTimDistance )
			{
				hkToiPrintf("Pts", "#    Pts    %-6s %-6s        dist:%2.4f  \n", names[0], names[1], distAtT1 );
				HK_INTERNAL_TIMER_SPLIT_LIST("getPoints");

				m_separatingNormal(3) = distAtT1;

				hkGskManifoldWork work;
				hkGskManifold_init( m_manifold, m_separatingNormal, bodyA, bodyB, input.getTolerance(), work );
				hkGskManifold_verifyAndGetPoints( m_manifold, work, 0, output, m_contactMgr ); 
				goto END_OF_FUNCTION;
			}
		}

	}

PROCESS_AT_T1:
	HK_INTERNAL_TIMER_SPLIT_LIST("process");
	{
		//hkToiPrintf("Gsk", "#    Gsk    %-6s %-6s        dist:%2.4f  \n", names[0], names[1], distAtT1 );
		hkGskfAgent::processCollisionNoTim( bodyA, bodyB, input, output );
	}
END_OF_FUNCTION:;

	HK_INTERNAL_TIMER_END_LIST();
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
