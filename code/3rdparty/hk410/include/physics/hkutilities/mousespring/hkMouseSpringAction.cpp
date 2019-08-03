/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/mousespring/hkMouseSpringAction.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkconstraintsolver/simpleConstraints/hkSimpleConstraintUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkMouseSpringAction);

hkMouseSpringAction::hkMouseSpringAction( hkRigidBody* rb )
: hkUnaryAction( rb ),
  m_springDamping(0.5f),
  m_springElasticity(0.3f),
  m_maxRelativeForce(250.0f),
  m_objectDamping(0.95f)
{
	m_positionInRbLocal.setZero4();
	m_mousePositionInWorld.setZero4();
}

hkMouseSpringAction::hkMouseSpringAction( 
	const hkVector4& positionInRbLocal, const hkVector4& mousePositionInWorld,
	const hkReal springDamping, const hkReal springElasticity, 
	const hkReal objectDamping, hkRigidBody* rb )
:	hkUnaryAction( rb ),
	m_positionInRbLocal( positionInRbLocal ),
	m_mousePositionInWorld( mousePositionInWorld ),
	m_springDamping( springDamping ),
	m_springElasticity( springElasticity ),
	m_objectDamping( objectDamping )
{
	m_maxRelativeForce = 250.0f; // that's 25 times gravity
}

void hkMouseSpringAction::setMousePosition( const hkVector4& mousePositionInWorld )
{
	if ( !mousePositionInWorld.equals3( m_mousePositionInWorld ) )
	{
		hkRigidBody* rb = static_cast<hkRigidBody*>( m_entity );
		if ( rb && rb->isAddedToWorld() )
		{
			rb->activate();
		}
	}
	m_mousePositionInWorld = mousePositionInWorld;
}

void hkMouseSpringAction::setMaxRelativeForce(hkReal newMax)
{
	m_maxRelativeForce = newMax;
}



void hkMouseSpringAction::applyAction( const hkStepInfo& stepInfo )
{
	hkRigidBody* rb = static_cast<hkRigidBody*>( m_entity );

	// calculate and apply the rigid spring mouse impluse
	const hkVector4& pMouse = m_mousePositionInWorld;
	//m_positionInRbLocal.setZero(); // for centra picking

	hkVector4 pRb; pRb.setTransformedPos(rb->getTransform(), m_positionInRbLocal);

	hkVector4 ptDiff;
	ptDiff.setSub4(pRb, pMouse);

	// calculate the jacobian
	hkMatrix3 jacobian;
	{
		hkReal massInv = rb->getMassInv();

		hkVector4 r;
		r.setSub4( pRb, rb->getCenterOfMassInWorld() );

		hkMatrix3 rhat;
		rhat.setCrossSkewSymmetric(r);

		hkMatrix3 inertialInvWorld;
		rb->getInertiaInvWorld(inertialInvWorld);
	
		//jacobian.setZero(); this is not necessary!
		jacobian.setDiagonal( massInv, massInv, massInv );

		// calculate: jacobian -= (rhat * inertialInvWorld * rhat)
		hkMatrix3 temp;
		temp.setMul(rhat, inertialInvWorld);
		hkMatrix3 temp2;
		temp2.setMul(temp, rhat);
		jacobian.sub(temp2);
	}
	

	// invert the jacobian as: jacobian * impluse = velocityDelta...
	// we want to calculate the impluse
	const hkResult jacInvertResult = jacobian.invert(0.0000001f);
	if ( jacInvertResult != HK_SUCCESS )
	{
		return;
	}

	// apply damping
	hkVector4 linearVelocity;
	hkVector4 angularVelocity;
	{
		linearVelocity = rb->getLinearVelocity();
		linearVelocity.mul4(m_objectDamping);
		rb->setLinearVelocity(linearVelocity);

		angularVelocity = rb->getAngularVelocity();
		angularVelocity.mul4(m_objectDamping);
		rb->setAngularVelocity(angularVelocity);
	}

	// calculate the velocity delta
	hkVector4 delta;
	{
		hkVector4 relVel;
		rb->getPointVelocity(pRb, relVel);
		delta.setMul4(m_springElasticity * stepInfo.m_invDeltaTime, ptDiff);
		delta.addMul4(m_springDamping, relVel);
	}

	// calculate the impluse
	hkVector4 impulse;
	{
		impulse.setMul3(jacobian, delta);	// jacobian is actually the jacobian inverse here!
		impulse.setNeg4(impulse);
	}

	//
	//	clip the impulse
	//
	hkReal impulseLen2 = impulse.lengthSquared3();
	hkReal maxImpulse  = rb->getMass() * stepInfo.m_deltaTime * m_maxRelativeForce; 
	if ( impulseLen2 > maxImpulse * maxImpulse )
	{
		hkReal factor = maxImpulse * hkMath::sqrtInverse( impulseLen2 );
		impulse.mul4( factor );
	}

	// alternative method
	if (0)
	{
		rb->activate();

		hkSimpleConstraintInfoInitInput inputA;
			inputA.m_massRelPos.setSub4( pRb, rb->getCenterOfMassInWorld() );
			inputA.m_invMass = rb->getMassInv();
			rb->getInertiaInvWorld( inputA.m_invInertia );
			

		hkSimpleConstraintInfoInitInput inputB;
			inputB.m_invMass = 0.0f;
			inputB.m_invInertia.setZero();
			inputB.m_massRelPos.setZero4();

		hkSimpleConstraintInfo info;
		hkBodyVelocity velA;
			velA.m_linear = linearVelocity;
			velA.m_angular = angularVelocity;
		hkBodyVelocity velB;
			velB.m_linear.setZero4();
			velB.m_angular.setZero4();

			hkSimpleConstraintUtil_InitInfo( inputA, inputB, hkTransform::getIdentity().getRotation(), info );

		hkVector4 pointVelocity;
		hkSimpleConstraintUtil_getPointVelocity( info, velA, velB, pointVelocity );


		hkSimpleConstraintUtil_applyImpulse( info, impulse, velA, velB );
	}

	rb->applyPointImpulse(impulse, pRb);
}


void hkMouseSpringAction::entityRemovedCallback(hkEntity* entity) 
{
	hkUnaryAction::entityRemovedCallback(entity);

	// this line is needed for mouse action
	m_entity = HK_NULL;
}

hkAction* hkMouseSpringAction::clone( const hkArray<hkEntity*>& newEntities, const hkArray<hkPhantom*>& newPhantoms ) const
{
	HK_ASSERT2(0xf578efca, newEntities.getSize() == 1, "Wrong clone parameters given to a mousespring action (needs 1 body).");
	// should have two entities as we are a unary action.
	if (newEntities.getSize() != 1) return HK_NULL;

	HK_ASSERT2(0x5b74e112, newPhantoms.getSize() == 0, "Wrong clone parameters given to a mousespring action (needs 0 phantoms).");
	// should have no phantoms.
	if (newPhantoms.getSize() != 0) return HK_NULL;

	hkMouseSpringAction* ms = new hkMouseSpringAction( (hkRigidBody*)newEntities[0] );
	ms->m_positionInRbLocal = m_positionInRbLocal;
	ms->m_mousePositionInWorld = m_mousePositionInWorld;
	ms->m_springDamping = m_springDamping;
	ms->m_springElasticity = m_springElasticity;
	ms->m_maxRelativeForce = m_maxRelativeForce;
	ms->m_objectDamping = m_objectDamping;
	ms->m_userData = m_userData;
	return ms;
}





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
