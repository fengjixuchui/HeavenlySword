/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkContactPointAddedEvent::hkContactPointAddedEvent(	hkDynamicsContactMgr& contactMgr,
															const hkProcessCollisionInput& collisionInput,
															hkProcessCollisionOutput& collisionOutput,
															const hkCdBody& a,	const hkCdBody& b,
															hkContactPoint* ccp, 
															const hkGskCache* gskCache,
															hkContactPointMaterial* dcp, 
															hkReal projectedVelocity,
															Type pointType )

:	m_bodyA( a ),
	m_bodyB( b ),
	m_type( pointType ),
	m_contactPoint( ccp ),
	m_gskCache(gskCache),
	m_contactPointMaterial( dcp ),
	m_projectedVelocity( projectedVelocity ),
	m_internalContactMgr( contactMgr ),
	m_collisionInput( collisionInput ),
	m_collisionOutput( collisionOutput )
{
	m_status = HK_CONTACT_POINT_ACCEPT;
}

inline hkToiPointAddedEvent::hkToiPointAddedEvent(	hkDynamicsContactMgr& contactMgr,	
													const hkProcessCollisionInput& collisionInput,
													hkProcessCollisionOutput& collisionOutput,
													const hkCdBody& a,	const hkCdBody& b, 
													hkContactPoint* cp,  const hkGskCache* gskCache, hkContactPointMaterial* cpp, 
													hkTime toi, hkReal projectedVelocity)
	:   hkContactPointAddedEvent(  contactMgr, collisionInput, collisionOutput, a, b, cp, gskCache, cpp, projectedVelocity, TYPE_TOI), m_toi(toi)
{
}

inline hkManifoldPointAddedEvent::hkManifoldPointAddedEvent(	hkContactPointId contactPointId, 
																hkDynamicsContactMgr& contactMgr,	
																const hkProcessCollisionInput& collisionInput,
																hkProcessCollisionOutput& collisionOutput,
																const hkCdBody& a,	const hkCdBody& b, 
																hkContactPoint* cp,   const hkGskCache* gskCache, hkContactPointMaterial* cpp, 
																hkReal projectedVelocity)
	:   hkContactPointAddedEvent(  contactMgr, collisionInput, collisionOutput, a, b, cp, gskCache, cpp, projectedVelocity, TYPE_MANIFOLD ), m_contactPointId( contactPointId )
{
	m_nextProcessCallbackDelay = 0;
}

inline hkContactPointConfirmedEvent::hkContactPointConfirmedEvent(	hkContactPointAddedEvent::Type type,
																	const hkCollidable& a, const hkCollidable& b, 
																	const hkSimpleContactConstraintData* data,
																	hkContactPoint* cp,	hkContactPointMaterial* cpp, 
																	hkReal rotateNormal,
																	hkReal projectedVelocity)
	:   m_collidableA(a), m_collidableB(b),
		m_contactPoint(cp),	m_contactPointMaterial( cpp ),
		m_rotateNormal(rotateNormal),
		m_projectedVelocity( projectedVelocity ),
		m_type(type), m_contactData(data)
{
}

hkContactPointRemovedEvent::hkContactPointRemovedEvent(	hkContactPointId contactPointId, 
														hkDynamicsContactMgr& contactMgr,	
														hkConstraintOwner& constraintOwner,
														hkContactPointMaterial* mat, 
														hkEntity* ca,	
														hkEntity* cb)
	:	m_contactPointId(contactPointId),
		m_contactPointMaterial(mat),
		m_entityA(ca),
		m_entityB(cb),
		m_internalContactMgr(contactMgr),
		m_constraintOwner( constraintOwner )
{
	if( contactPointId != HK_INVALID_CONTACT_POINT )
	{
		m_contactPointMaterial = m_internalContactMgr.getContactPointProperties( contactPointId );
	}	
}

hkContactProcessEvent::hkContactProcessEvent( hkDynamicsContactMgr& contactMgr, const hkCollidable& ca, const hkCollidable& cb, hkProcessCollisionData& data)
	:	m_collidableA(ca),
		m_collidableB(cb),
		m_collisionData( data ),
		m_internalContactMgr(contactMgr)
{
}

inline hkBool hkContactPointRemovedEvent::isToi()
{
	return m_contactPointId == HK_INVALID_CONTACT_POINT;
}

inline hkToiPointAddedEvent& hkContactPointAddedEvent::asToiEvent()
{
	HK_ASSERT2( 0xf043dea2, m_type == this->TYPE_TOI, "Invalid upcast from hkContactPointAddedEvent to hkToiPointAddedEvent" );
	return static_cast<hkToiPointAddedEvent&>(*this);
}

inline hkBool hkContactPointAddedEvent::isToi()
{
	return m_type == this->TYPE_TOI;

}


inline hkManifoldPointAddedEvent& hkContactPointAddedEvent::asManifoldEvent()
{
	HK_ASSERT2( 0xf043dea2, m_type == this->TYPE_MANIFOLD, "Invalid upcast from hkContactPointAddedEvent to hkManifoldPointAddedEvent" );
	return static_cast<hkManifoldPointAddedEvent&>(*this);
}

inline hkBool hkContactPointConfirmedEvent::isToi() const 
{
	return m_type == hkContactPointAddedEvent::TYPE_TOI;
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
