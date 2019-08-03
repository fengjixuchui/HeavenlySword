/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/agent/transform/hkTransformAgent.h>
#include <hkcollide/shape/transform/hkTransformShape.h>

#include <hkmath/linear/hkVector4Util.h>

void HK_CALL hkTransformAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
		// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createTransformBAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkTransformAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkTransformAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkTransformAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_ALL, HK_SHAPE_TRANSFORM );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createTransformAAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_TRANSFORM, HK_SHAPE_ALL );
	}
}

void hkTransformAgent::cleanup(	hkCollisionConstraintOwner& constraintOwner )
{
	m_childAgent->cleanup( constraintOwner );
	delete this;
}

hkTransformAgent::hkTransformAgent(const hkCdBody& bodyAIn, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
: hkCollisionAgent( mgr )
{
	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAIn.getShape());
	const hkShape* childShape = tShapeA->getChildShape();

	hkMotionState ms = *bodyAIn.getMotionState();

	ms.getTransform().setMul( bodyAIn.getTransform(), tShapeA->getTransform());

	hkCdBody bodyA( &bodyAIn, &ms);
	bodyA.setShape( childShape );

	m_childAgent = input.m_dispatcher->getNewCollisionAgent( bodyA, bodyB, input, mgr );
}

hkCollisionAgent* HK_CALL hkTransformAgent::createTransformAAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkTransformAgent* agent = new hkTransformAgent( bodyA, bodyB, input, mgr );
	return agent;
}


hkCollisionAgent* HK_CALL hkTransformAgent::createTransformBAgent(const hkCdBody& bodyA, const hkCdBody& bodyB,
														const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkTransformAgent* agent = new hkSymmetricAgent<hkTransformAgent>(bodyA, bodyB, input,mgr);
	return agent;
}


void hkTransformAgent::processCollision(const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x1792c490,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN( "Transform", HK_NULL );

	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAin.getShape());
	
	hkMotionState ms;

	//
	//	Calc transform
	//
	ms.getTransform().setMul( bodyAin.getTransform(), tShapeA->getTransform());

	//
	//	Calc swept transform
	//
	{
		hkSweptTransform& st = ms.getSweptTransform();
		const hkSweptTransform& ss = bodyAin.getMotionState()->getSweptTransform();

		st.m_centerOfMass0 = ss.m_centerOfMass0;
		st.m_centerOfMass1 = ss.m_centerOfMass1;

		st.m_rotation0.setMul( ss.m_rotation0, tShapeA->getRotation() );
		st.m_rotation1.setMul( ss.m_rotation1, tShapeA->getRotation() );

		st.m_centerOfMassLocal._setTransformedInversePos( tShapeA->getTransform(), ss.m_centerOfMassLocal );
	}
	const hkMotionState& ss = *bodyAin.getMotionState();
	ms.m_deltaAngle = ss.m_deltaAngle;
	ms.m_objectRadius    = ss.m_objectRadius;

	hkCdBody copyBodyA( &bodyAin,  &ms);
	copyBodyA.setShape( tShapeA->getChildShape() );

	m_childAgent->processCollision( copyBodyA, bodyB, input, result );

	HK_TIMER_END();
}

		// hkCollisionAgent interface implementation.
void hkTransformAgent::linearCast( const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_INTERNAL_TIMER_BEGIN( "Transform", HK_NULL);

	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAin.getShape());
	
	hkTransform t;	t.setMul( bodyAin.getTransform(), tShapeA->getTransform());

	hkCdBody copyBodyA( &bodyAin,  &t);
	copyBodyA.setShape( tShapeA->getChildShape() );

	m_childAgent->linearCast( copyBodyA, bodyB, input, collector, startCollector);

	HK_INTERNAL_TIMER_END();
}

void hkTransformAgent::staticLinearCast( const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_INTERNAL_TIMER_BEGIN( "Transform", HK_NULL );

	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAin.getShape());
	
	hkTransform t;	t.setMul( bodyAin.getTransform(), tShapeA->getTransform());

	hkCdBody copyBodyA( &bodyAin,  &t);
	copyBodyA.setShape( tShapeA->getChildShape() );

	hkShapeType typeA = copyBodyA.getShape()->getType();
	hkShapeType typeB = bodyB.getShape()->getType();
	hkCollisionDispatcher::LinearCastFunc linearCastFunc = input.m_dispatcher->getLinearCastFunc( typeA, typeB );
	linearCastFunc( copyBodyA, bodyB, input, collector, startCollector);

	HK_INTERNAL_TIMER_END();
}

void hkTransformAgent::getClosestPoints( const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& pointDetails)
{
	HK_INTERNAL_TIMER_BEGIN( "Transform", HK_NULL );

	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAin.getShape());
	
	hkTransform t;	t.setMul( bodyAin.getTransform(), tShapeA->getTransform());

	hkCdBody copyBodyA( &bodyAin,  &t);
	copyBodyA.setShape( tShapeA->getChildShape() );

	m_childAgent->getClosestPoints( copyBodyA, bodyB, input, pointDetails);

	HK_INTERNAL_TIMER_END();
}

void hkTransformAgent::staticGetClosestPoints( const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& pointDetails)
{
	HK_INTERNAL_TIMER_BEGIN( "Transform", HK_NULL );

	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAin.getShape());
	
	hkTransform t;	t.setMul( bodyAin.getTransform(), tShapeA->getTransform());

	hkCdBody copyBodyA( &bodyAin,  &t);
	copyBodyA.setShape( tShapeA->getChildShape() );

	hkShapeType typeA = copyBodyA.getShape()->getType();
	hkShapeType typeB = bodyB.getShape()->getType();
	hkCollisionDispatcher::GetClosestPointsFunc getClosestPointFunc = input.m_dispatcher->getGetClosestPointsFunc( typeA, typeB );

	getClosestPointFunc( copyBodyA, bodyB, input, pointDetails);

	HK_INTERNAL_TIMER_END();
}

void hkTransformAgent::getPenetrations( const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_INTERNAL_TIMER_BEGIN( "Transform", HK_NULL );

	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAin.getShape());
	
	hkTransform t;	t.setMul( bodyAin.getTransform(), tShapeA->getTransform());

	hkCdBody copyBodyA( &bodyAin,  &t);
	copyBodyA.setShape( tShapeA->getChildShape() );

	HK_INTERNAL_TIMER_END();
	m_childAgent->getPenetrations( copyBodyA, bodyB, input, collector );
}

void hkTransformAgent::staticGetPenetrations( const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_INTERNAL_TIMER_BEGIN( "Transform", HK_NULL );

	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAin.getShape());
	
	hkTransform t;	t.setMul( bodyAin.getTransform(), tShapeA->getTransform());

	hkCdBody copyBodyA( &bodyAin,  &t);
	copyBodyA.setShape( tShapeA->getChildShape() );

	hkShapeType typeA = copyBodyA.getShape()->getType();
	hkShapeType typeB = bodyB.getShape()->getType();
	hkCollisionDispatcher::GetPenetrationsFunc getPenetrationsFunc = input.m_dispatcher->getGetPenetrationsFunc( typeA, typeB );
	getPenetrationsFunc( copyBodyA, bodyB, input, collector );

	HK_INTERNAL_TIMER_END();
}

void hkTransformAgent::updateShapeCollectionFilter( const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	const hkTransformShape* tShapeA = static_cast<const hkTransformShape*>(bodyAin.getShape());
	
	hkTransform t;	t.setMul( bodyAin.getTransform(), tShapeA->getTransform());

	hkCdBody copyBodyA( &bodyAin,  &t);
	copyBodyA.setShape( tShapeA->getChildShape() );

	m_childAgent->updateShapeCollectionFilter( copyBodyA, bodyB, input, constraintOwner );

}

void hkTransformAgent::invalidateTim( hkCollisionInput& input )
{
	m_childAgent->invalidateTim( input );
}

void hkTransformAgent::warpTime( hkTime oldTime, hkTime newTime, hkCollisionInput& input )
{
	m_childAgent->warpTime( oldTime, newTime, input );
}

void hkTransformAgent::removePoint( hkContactPointId idToRemove )
{
	m_childAgent->removePoint( idToRemove );
}

void hkTransformAgent::commitPotential( hkContactPointId newId )
{
	m_childAgent->commitPotential( newId );
}

void hkTransformAgent::createZombie( hkContactPointId idTobecomeZombie )
{
	m_childAgent->createZombie( idTobecomeZombie );
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
