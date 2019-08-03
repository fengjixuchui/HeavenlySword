/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkcollide/hkCollide.h>

#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkinternal/collide/gjk/hkGsk.h>

#include <hkcollide/shape/convex/hkConvexShape.h>

#include <hkcollide/agent/hkCollisionAgentConfig.h>
#include <hkcollide/agent/hkCollisionQualityInfo.h>

#include <hkcollide/agent/gjk/hkGskfAgent.h>
#include <hkinternal/collide/gjk/gskmanifold/hkGskManifoldUtil.h>

#include <hkinternal/collide/agent3/predgskagent3/hkPredGskAgent3.h>
#include <hkinternal/collide/gjk/agent/hkGskAgentUtil.h>

//HK_COMPILE_TIME_ASSERT( sizeof( hkGskfAgent ) == 12/*base*/ + 20/*tim*/ + 16/*cache*/ + 64/*manifold*/ );


hkGskfAgent::hkGskfAgent(	const hkCdBody& bodyA,	const hkCdBody& bodyB, hkContactMgr* mgr ): hkGskBaseAgent( bodyA, bodyB, mgr )
{
}

hkCollisionAgent* HK_CALL hkGskfAgent::createGskfAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB, 
																			const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkGskBaseAgent* agent;
	if ( mgr )
	{
		agent = new hkGskfAgent(bodyA, bodyB, mgr);
	}
	else
	{
		agent = new hkGskBaseAgent( bodyA, bodyB, mgr );
	}
	return agent;
}

void HK_CALL hkGskfAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	hkCollisionDispatcher::AgentFuncs af;
	af.m_createFunc          = createGskfAgent;
	af.m_getPenetrationsFunc = staticGetPenetrations;
	af.m_getClosestPointFunc = staticGetClosestPoints;
	af.m_linearCastFunc      = staticLinearCast;
	af.m_isFlipped           = false;
	af.m_isPredictive		 = false;

	dispatcher->registerCollisionAgent( af, HK_SHAPE_CONVEX, HK_SHAPE_CONVEX );
}




void hkGskfAgent::cleanup( hkCollisionConstraintOwner& constraintOwner )
{
	hkGskManifold_cleanup( m_manifold, m_contactMgr, constraintOwner );
	delete this;
}

void hkGskfAgent::removePoint( hkContactPointId idToRemove )
{
	for ( int i = 0; i < m_manifold.m_numContactPoints; i++)
	{
		if ( m_manifold.m_contactPoints[i].m_id == idToRemove)
		{
			hkGskManifold_removePoint( m_manifold, i );
			break;
		}
	}
}

void hkGskfAgent::commitPotential( hkContactPointId idToCommit )
{
	for ( int i = 0; i < m_manifold.m_numContactPoints; i++)
	{
		if ( m_manifold.m_contactPoints[i].m_id == HK_INVALID_CONTACT_POINT)
		{
			m_manifold.m_contactPoints[i].m_id = idToCommit;
			break;
		}
	}
}

void hkGskfAgent::createZombie( hkContactPointId idTobecomeZombie )
{
	for ( int i = 0; i < m_manifold.m_numContactPoints; i++)
	{
		hkGskManifold::ContactPoint& cp = m_manifold.m_contactPoints[i];
		if ( cp.m_id == idTobecomeZombie)
		{
			cp.m_dimA = 0;
			cp.m_dimB = 0;
			break;
		}
	}
}

#if defined HK_COMPILER_MSVC
	// C4701: local variable 'output' may be used without having been initialized
#	pragma warning(disable: 4701)
#endif





void hkGskfAgent::processCollisionNoTim(const hkCdBody& bodyA,	const hkCdBody& bodyB, 
										const hkProcessCollisionInput& input, 	hkProcessCollisionOutput& output)
{
	hkAgent3ProcessInput in3;
	{
		in3.m_bodyA = &bodyA;
		in3.m_bodyB = &bodyB;
		in3.m_contactMgr = m_contactMgr;
		in3.m_input = &input;

		const hkMotionState* msA = bodyA.getMotionState();
		const hkMotionState* msB = bodyB.getMotionState();
		//hkSweptTransformUtil::calcTimInfo( *msA, *msB, in3.m_linearTimInfo);
		in3.m_aTb.setMulInverseMul(msA->getTransform(), msB->getTransform());
	}
	hkGskAgentUtil_processCollisionNoTim( in3, HK_NULL, HK_NULL, m_cache, m_manifold, m_separatingNormal, output );
}

void hkGskfAgent::processCollision(const hkCdBody& bodyA,		const hkCdBody& bodyB, 
								const hkProcessCollisionInput& input,		hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x57213df1,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN_LIST( "GskAgent", "Tim" );

	//
	//	Get the relative linear movement (xyz) and the worst case angular movment (w)
	//
	if ( ! (m_timeOfSeparatingNormal == input.m_stepInfo.m_startTime) )
	{
		hkVector4 timInfo;
		hkSweptTransformUtil::calcTimInfo( *bodyA.getMotionState(), *bodyB.getMotionState(), input.m_stepInfo.m_deltaTime, timInfo);

		if ( m_separatingNormal(3) > input.getTolerance() )
		{
			m_separatingNormal(3) -= timInfo.dot4xyz1( m_separatingNormal );
			if ( m_separatingNormal(3) > input.getTolerance() )
			{
				goto END;
			}
		}
	}
	HK_TIMER_SPLIT_LIST( "Gsk" );
	{
		m_timeOfSeparatingNormal = input.m_stepInfo.m_endTime;
		processCollisionNoTim( bodyA, bodyB, input, result);
	}
END:;
	HK_TIMER_END_LIST();
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
