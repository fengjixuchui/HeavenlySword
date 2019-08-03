/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkMaterial::hkMaterial() 
:	m_friction( 0.5f ),	
	m_restitution( 0.4f )	
{
}

inline hkReal hkMaterial::getFriction() const 
{ 
	return m_friction; 
}

inline hkReal hkMaterial::getRestitution() const 
{
	return m_restitution; 
}
	
inline void hkMaterial::setFriction( hkReal newFriction )
{ 
	HK_ASSERT2( 0xf0160299, newFriction>=0.0f && newFriction <= 255.0f, "Friction value out of range" );
	m_friction = newFriction; 
}

inline void hkMaterial::setRestitution( hkReal newRestitution ) 
{ 
	HK_ASSERT2( 0xf0160256, newRestitution >= 0.0f && newRestitution < 1.99f, "Restitution value out of range" );
	m_restitution = newRestitution; 
}


inline hkReal HK_CALL hkMaterial::getCombinedFriction( hkReal frictionA, hkReal frictionB )
{
	return hkMath::sqrt( frictionA * frictionB );
}

inline hkReal HK_CALL hkMaterial::getCombinedRestitution( hkReal restitutionA, hkReal restitutionB )
{
	return hkMath::sqrt( restitutionA * restitutionB );
}

inline void hkMaterial::setResponseType( enum hkMaterial::ResponseType t )
{
	m_responseType = t;
}

inline enum hkMaterial::ResponseType hkMaterial::getResponseType() const
{
	return m_responseType;
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
