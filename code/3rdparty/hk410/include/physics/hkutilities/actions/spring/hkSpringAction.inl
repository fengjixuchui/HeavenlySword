/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline const hkVector4& hkSpringAction::getPositionAinA()
{
	return m_positionAinA;
}

inline const hkVector4& hkSpringAction::getPositionBinB()
{
	return m_positionBinB;
}

inline void hkSpringAction::setStrength(hkReal str)
{
  m_strength = str;
}

inline hkReal hkSpringAction::getStrength()
{
	return m_strength;
}

inline void hkSpringAction::setDamping(hkReal damp)
{
  m_damping = damp;
}

inline hkReal hkSpringAction::getDamping()
{
	return m_damping;
}

inline void hkSpringAction::setRestLength(hkReal restLength)
{
  m_restLength = restLength;
}

inline hkReal hkSpringAction::getRestLength()
{
	return m_restLength;
}

inline void hkSpringAction::setOnCompression(hkBool onCompression)
{
  m_onCompression = onCompression;
}

inline hkBool hkSpringAction::getOnCompression()
{
	return m_onCompression;
}

inline void hkSpringAction::setOnExtension(hkBool onExtension)
{
  m_onExtension = onExtension;
}

inline hkBool hkSpringAction::getOnExtension()
{
	return m_onExtension;
}

inline const hkVector4& hkSpringAction::getLastForce() 
{ 
	return m_lastForce; 
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
