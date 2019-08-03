/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkReal hkContactPointMaterial::getFriction() const
{ 
	return m_friction * (1.0f / 256.0f); 
}

inline int hkContactPointMaterial::getFriction8_8() const
{ 
	return m_friction; 
}

inline void hkContactPointMaterial::setFriction( hkReal r )
{ 
	HK_ASSERT2( 0xf0160256, r>=0.0f && r <= 255.0f, "Friction value out of range" );
	m_friction = hkUint16(hkMath::hkToIntFast(r* 256.0f)); 
}

inline hkReal hkContactPointMaterial::getRestitution() const
{ 
	return m_restitution * (1.0f / 128.0f ); 
}

inline void hkContactPointMaterial::setRestitution( hkReal r )
{ 
	HK_ASSERT2( 0xf0160256, r>=0.0f && r < 1.99f, "Restitution value out of range" );
	m_restitution = hkUchar(hkMath::hkToIntFast(r* 128.0f)); 
}

#if !defined (HK_PLATFORM_PS3SPU)
inline void* hkContactPointMaterial::getUserData() const
{ 
	return m_userData; 
}

inline void hkContactPointMaterial::setUserData( void* data )
{ 
	m_userData = data; 
}
#endif

inline hkBool hkContactPointMaterial::isPotential()
{
	return (m_flags & CONTACT_IS_NEW_AND_POTENTIAL) != 0;
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
