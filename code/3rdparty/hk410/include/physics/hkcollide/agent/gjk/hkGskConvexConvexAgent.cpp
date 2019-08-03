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

#include <hkcollide/agent/gjk/hkGskConvexConvexAgent.h>

//HK_COMPILE_TIME_ASSERT( sizeof( hkGskConvexConvexAgent ) == 12/*base*/ + 20/*tim*/ + 16/*cache*/ + 4*48 );


hkGskConvexConvexAgent::hkGskConvexConvexAgent(	const hkCdBody& bodyA,	const hkCdBody& bodyB, hkContactMgr* mgr ): hkGskBaseAgent( bodyA, bodyB, mgr )
{
	m_numContactPoints = 0;
	m_numContactPoints = 0;
}

hkCollisionAgent* HK_CALL hkGskConvexConvexAgent::createGskConvexConvexAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB,
																			const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkGskBaseAgent* agent;
	if ( mgr )
	{
		agent = new hkGskConvexConvexAgent(bodyA, bodyB, mgr);
	}
	else
	{
		agent = new hkGskBaseAgent( bodyA, bodyB, mgr );
	}
	return agent;
}

void HK_CALL hkGskConvexConvexAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	hkCollisionDispatcher::AgentFuncs af;
	af.m_createFunc          = createGskConvexConvexAgent;
	af.m_getPenetrationsFunc = staticGetPenetrations;
	af.m_getClosestPointFunc = staticGetClosestPoints;
	af.m_linearCastFunc      = staticLinearCast;
	af.m_isFlipped           = false;
	af.m_isPredictive		 = false;

	dispatcher->registerCollisionAgent( af, HK_SHAPE_CONVEX, HK_SHAPE_CONVEX);
}




void hkGskConvexConvexAgent::cleanup( hkCollisionConstraintOwner& info )
{
	hkClosestPointManifold::cleanup( m_contactPoints, m_numContactPoints, m_contactMgr, info );
	delete this;
}

#if defined HK_COMPILER_MSVC
	// C4701: local variable 'output' may be used without having been initialized
#	pragma warning(disable: 4701)
#endif



void hkGskConvexConvexAgent::processCollision(const hkCdBody& bodyA,		const hkCdBody& bodyB, 
								const hkProcessCollisionInput& input,		hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x57213df1,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN_LIST( "Gsk", "Tim" );


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

	{
		hkExtendedGskOut output;
		HK_TIMER_SPLIT_LIST( "Gsk" );
		bool hasPoint = getClosestPoint(bodyA,bodyB, input, output);
		m_separatingNormal = output.m_normalInWorld;
		m_separatingNormal(3) = output.m_distance;
		m_timeOfSeparatingNormal = input.m_stepInfo.m_endTime;

		if( hasPoint )
		{
			HK_INTERNAL_TIMER_SPLIT_LIST("addPoint");
			hkCollisionQualityInfo& sq = *input.m_collisionQualityInfo;
			int dim = m_cache.m_dimA + m_cache.m_dimB;
			hkReal createContactRangeMax = (dim==4)? sq.m_create4dContact: sq.m_createContact;

			hkClosestPointManifold::addPoint(bodyA, bodyB, input, result, output, createContactRangeMax, m_contactMgr, *result.m_constraintOwner, m_contactPoints, m_numContactPoints);

			const hkReal epsilon = .001f;

			HK_INTERNAL_TIMER_SPLIT_LIST("getPoints");
			hkClosestPointManifold::getPoints( bodyA, bodyB, input, output.m_distance - epsilon, m_contactPoints, m_numContactPoints, result, m_contactMgr, *result.m_constraintOwner); 
		}
		else
		{
			hkClosestPointManifold::cleanup( m_contactPoints, m_numContactPoints, m_contactMgr, *result.m_constraintOwner );
		}
	}
END:

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
