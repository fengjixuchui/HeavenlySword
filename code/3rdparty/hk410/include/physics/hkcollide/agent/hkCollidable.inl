/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkCollidable::hkCollidable(const hkShape* shape, const hkMotionState* ms, int type)
:	hkCdBody(shape, ms), m_ownerOffset(0), 
		m_broadPhaseHandle(type)
{
	m_broadPhaseHandle.setOwner(this); // 'this' valid here.
#ifdef HK_DEBUG
	checkPerformance();
#endif
}

inline hkCollidable::hkCollidable(const hkShape* shape, const hkTransform* t, int type)
:	hkCdBody(shape, t) , m_ownerOffset(0),
	m_broadPhaseHandle(type)	
{
	m_broadPhaseHandle.setOwner(this); // 'this' valid here.
#ifdef HK_DEBUG
	checkPerformance();
#endif
}

inline hkCollidable::hkCollidable( class hkFinishLoadedObjectFlag flag )
	: m_broadPhaseHandle(flag)
{
	if( flag.m_finishing )
	{
		m_broadPhaseHandle.setOwner(this);
	}	
}

HK_FORCE_INLINE hkCollidable::~hkCollidable()
{
}

HK_FORCE_INLINE void hkCollidable::setShape(hkShape* shape)
{ 
	m_shape = shape;

	// Just duplicating the constructor behavior, checking if the new m_shape might cause
	// performance loss.
#if defined HK_DEBUG
	checkPerformance();
#endif
}

HK_FORCE_INLINE const hkShape* hkCollidable::getShape() const
{ 
	return m_shape; 
}

HK_FORCE_INLINE void* hkCollidable::getOwner() const
{ 
	return const_cast<char*>( reinterpret_cast<const char*>(this) + m_ownerOffset ); 
}

		
HK_FORCE_INLINE void hkCollidable::setOwner( void* owner )
{
	m_ownerOffset = hkGetByteOffsetInt( this, owner ); // should be within int32 range even on 64 bit as a member

}

inline const hkTypedBroadPhaseHandle* hkCollidable::getBroadPhaseHandle() const
{
	return &m_broadPhaseHandle;
}

inline hkTypedBroadPhaseHandle* hkCollidable::getBroadPhaseHandle()
{
	return &m_broadPhaseHandle;
}

int hkCollidable::getType() const
{
	return m_broadPhaseHandle.getType();
}

HK_FORCE_INLINE hkCollidableQualityType hkCollidable::getQualityType() const
{
	return hkCollidableQualityType( m_broadPhaseHandle.m_objectQualityType );
}

HK_FORCE_INLINE void hkCollidable::setQualityType(hkCollidableQualityType type)
{
	m_broadPhaseHandle.m_objectQualityType = hkUint16(type);
}

hkUint32 hkCollidable::getCollisionFilterInfo() const
{
	return m_broadPhaseHandle.getCollisionFilterInfo();
}

void hkCollidable::setCollisionFilterInfo( hkUint32 info )
{
	m_broadPhaseHandle.setCollisionFilterInfo(info);
}


HK_FORCE_INLINE hkReal hkCollidable::getAllowedPenetrationDepth() const
{
	return m_allowedPenetrationDepth;
}

HK_FORCE_INLINE void hkCollidable::setAllowedPenetrationDepth( hkReal val )
{
	HK_ASSERT2(0xad45bd3d, val > HK_REAL_EPSILON, "Must use a non-zero ( > epsilon ) value when setting allowed penetration depth of bodies.");
	m_allowedPenetrationDepth = val;
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
