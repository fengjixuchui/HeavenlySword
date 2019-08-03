/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

void hkTypedBroadPhaseHandle::setOwner(void* o)
{
	hkInt32 offset = hkGetByteOffsetInt(this, o);
	// +127 is 'offset invalid' flag.
	HK_ASSERT2( 0xfe456f34, offset > -128 && offset < 127, "Can't store offset to typed broadphase handle owner (more than 128 bytes difference).");
	m_ownerOffset = static_cast<hkInt8>( offset );
}

hkTypedBroadPhaseHandle::hkTypedBroadPhaseHandle( int type )
{
	m_type = static_cast<hkInt8>(type);
	m_collisionFilterInfo = 0;
	m_ownerOffset = OFFSET_INVALID;
}

hkTypedBroadPhaseHandle::hkTypedBroadPhaseHandle( void* owner, int type )
{
	m_type = static_cast<hkInt8>(type);
	m_collisionFilterInfo = 0;
	setOwner(owner);
}

int hkTypedBroadPhaseHandle::getType() const
{ 
	return m_type; 
}

void hkTypedBroadPhaseHandle::setType( int type )
{
	m_type = static_cast<hkInt8>(type);
}

void* hkTypedBroadPhaseHandle::getOwner() const
{
	HK_ASSERT2( 0xfe456f35, m_ownerOffset != OFFSET_INVALID, "Owner offset for typed broadphase handle incorrect. Did you call setOwner()?" );
	return const_cast<char*>( reinterpret_cast<const char*>(this) + m_ownerOffset );
}

hkUint32 hkTypedBroadPhaseHandle::getCollisionFilterInfo() const
{
	return m_collisionFilterInfo;
}

void hkTypedBroadPhaseHandle::setCollisionFilterInfo( hkUint32 info )
{
	m_collisionFilterInfo = info;
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
