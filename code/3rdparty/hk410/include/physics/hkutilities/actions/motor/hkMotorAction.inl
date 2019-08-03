/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkMotorAction::hkMotorAction()
: hkUnaryAction(HK_NULL), 
  m_spinRate(0.0f),
  m_gain(2.0f),
  m_active(true)
{
	m_axis.setZero4();
}

inline hkReal hkMotorAction::getSpinRate() const
{
	return m_spinRate;
}

inline void hkMotorAction::setSpinRate(hkReal new_rate)
{
	m_spinRate = new_rate;
}

inline hkReal hkMotorAction::getGain() const
{
	return m_gain;
}

inline void hkMotorAction::setGain(hkReal new_gain)
{
	m_gain = new_gain;
}

inline const hkVector4& hkMotorAction::getAxis() const
{
	return m_axis;
}

inline void hkMotorAction::setAxis(const hkVector4& axis)
{
	m_axis = axis;
}

inline hkBool hkMotorAction::isActive() const
{
	return m_active;
}

inline void hkMotorAction::setActivation(hkBool b)
{
	m_active = b;
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
