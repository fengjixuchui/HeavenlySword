/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

/***
 *** INLINE FUNCTION DEFINITIONS
 ***/


template<typename AGENT>
void hkSymmetricAgentLinearCast<AGENT>::processCollision(	const hkCdBody& bodyA, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	hkProcessCdPoint* pp = result.m_firstFreeContactPoint;

	hkTime oldToi = result.m_toi.m_time;

	AGENT::processCollision(bodyB, bodyA, input, result);

	//
	//	Flip all new normals
	//
	for ( ; pp < result.m_firstFreeContactPoint; pp++)
	{
		pp->m_contact.setFlipped( pp->m_contact );
	}

	// Flip the toi normal, if a new a toi was reported by this processCollision call
	if( oldToi != result.m_toi.m_time )
	{
		result.m_toi.flip();
	}
}

template<typename AGENT>	
void hkSymmetricAgentLinearCast<AGENT>::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector ) 
{
	hkSymmetricAgentFlipCollector flip( collector );
	AGENT::getClosestPoints(bodyB , bodyA , input, flip );
}

template<typename AGENT>	
void hkSymmetricAgentLinearCast<AGENT>::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector ) 
{
	hkSymmetricAgentFlipCollector flip( collector );
	AGENT::staticGetClosestPoints(bodyB , bodyA , input, flip );
}


template<typename AGENT>
void hkSymmetricAgentLinearCast<AGENT>::getPenetrations(	const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector)
{
	hkSymmetricAgentFlipBodyCollector flip(collector);
	AGENT::getPenetrations(bodyB, bodyA, input, flip);
}


template<typename AGENT>
void hkSymmetricAgentLinearCast<AGENT>::staticGetPenetrations(	const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector)
{
	hkSymmetricAgentFlipBodyCollector flip(collector);
	AGENT::staticGetPenetrations(bodyB, bodyA, input, flip);
}


template<typename AGENT>
inline hkSymmetricAgentLinearCast<AGENT>::hkSymmetricAgentLinearCast( const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkContactMgr* mgr ) 
:	AGENT( B, A, input, mgr )
{
}



template<typename AGENT>
void hkSymmetricAgentLinearCast<AGENT>::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	hkLinearCastCollisionInput	flippedInput = input;
	flippedInput.m_path.setNeg4( input.m_path );

	hkSymmetricAgentFlipCastCollector flip( input.m_path, collector );
	if ( startCollector )
	{
		hkSymmetricAgentFlipCastCollector startFlip( input.m_path, *startCollector );
		AGENT::staticLinearCast(bodyB, bodyA, flippedInput, flip, &startFlip );
	}
	else
	{
		AGENT::staticLinearCast(bodyB , bodyA , flippedInput, flip, HK_NULL );
	}
}

template<typename AGENT>
void hkSymmetricAgentLinearCast<AGENT>::updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	AGENT::updateShapeCollectionFilter( bodyB, bodyA, input, constraintOwner );
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
