/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


const hkTransform& hkCdBody::getTransform() const
{
	HK_ASSERT2(0x733aae9d, m_motion, "You cannot call getTransform() on a hkCdBody with null m_motion (eg, a hkAabbPhantom)");
	return *static_cast<const hkTransform*>(m_motion);
}

const hkMotionState* hkCdBody::getMotionState() const
{
	return static_cast<const hkMotionState*>(m_motion);
}

void hkCdBody::setMotionState( const hkMotionState* state )
{
	m_motion = state;
}

void hkCdBody::setTransform( const hkTransform* t )
{
	m_motion = t;
}


const hkShape* hkCdBody::getShape() const
{
	return m_shape;
}

const hkCollidable* hkCdBody::getRootCollidable() const
{	
	const hkCdBody* body = this;
	while( body->m_parent )
	{
		body = body->m_parent;
	}
	return reinterpret_cast<const hkCollidable*>(body);
}

const hkCdBody* hkCdBody::getParent() const
{
	return m_parent;
}

hkCdBody::hkCdBody( const hkShape* shape, const hkMotionState* motionState )
{
	m_shape = shape;
	m_motion = motionState;
	m_parent = HK_NULL;
	m_shapeKey = HK_INVALID_SHAPE_KEY;
}

hkCdBody::hkCdBody( const hkShape* shape, const hkTransform* t )
{
	m_shape = shape;
	m_motion = t;
	m_parent = HK_NULL;
	m_shapeKey = HK_INVALID_SHAPE_KEY;
}


hkCdBody::hkCdBody( const hkCdBody* parent )
{
	m_parent = parent;
	m_motion = parent->m_motion;
}

hkCdBody::hkCdBody( const hkCdBody* parent, const hkMotionState* ms )
{
	m_parent = parent;
	m_motion = ms;
}

hkCdBody::hkCdBody( const hkCdBody* parent, const hkTransform* t )
{
	m_parent       = parent;
	m_motion = t;
}


hkShapeKey hkCdBody::getShapeKey() const
{
	return m_shapeKey;
}


void hkCdBody::setShape( const hkShape* shape )
{
	m_shapeKey = m_parent->m_shapeKey;
	m_shape = shape;
}

void hkCdBody::setShape( const hkShape* shape, hkShapeKey key )
{
	m_shape = shape;
	m_shapeKey = key;
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
