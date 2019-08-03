/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// Return the master weight for the control
inline hkReal hkDefaultAnimationControl::getMasterWeight() const
{
	return m_masterWeight;
}

// Set the master weight for this playing animation
inline void hkDefaultAnimationControl::setMasterWeight( hkReal weight )
{
	m_masterWeight = weight;
}

// Return the master weight for the control
inline hkReal hkDefaultAnimationControl::getPlaybackSpeed() const
{
	return m_playbackSpeed;
}

// Set the playback speed.
// If the is set to a negative value the animation will run backwards
inline void hkDefaultAnimationControl::setPlaybackSpeed( hkReal speed )
{
	m_playbackSpeed = speed;
}

inline hkUint32 hkDefaultAnimationControl::getOverflowCount() const
{
	return m_overflowCount;
}

inline void hkDefaultAnimationControl::setOverflowCount( hkUint32 count )
{
	m_overflowCount = count;
}

inline hkUint32 hkDefaultAnimationControl::getUnderflowCount() const
{
	return m_underflowCount;
}

inline void hkDefaultAnimationControl::setUnderflowCount( hkUint32 count )
{
	m_underflowCount = count;
}


inline enum hkDefaultAnimationControl::EaseStatus hkDefaultAnimationControl::getEaseStatus() const
{
	if ( m_easeDir )
	{
		return (m_easeT < 1.0f) ? hkDefaultAnimationControl::EASING_IN : hkDefaultAnimationControl::EASED_IN;
	}
	else
	{
		return (m_easeT < 1.0f) ? hkDefaultAnimationControl::EASING_OUT : hkDefaultAnimationControl::EASED_OUT;
	}
}

inline void hkDefaultAnimationControl::setEaseInCurve(hkReal y0, hkReal y1, hkReal y2, hkReal y3)
{
	m_easeInCurve.set(y0, y1, y2, y3);
}

inline void hkDefaultAnimationControl::setEaseOutCurve(hkReal y0, hkReal y1, hkReal y2, hkReal y3)
{
	m_easeOutCurve.set(y0, y1, y2, y3);
}

// We assume that ease out and in are symmetric
inline hkReal hkDefaultAnimationControl::easeIn(hkReal duration)
{
	const hkReal denominator = duration; // avoid sn compiler warning

	m_easeT = (!m_easeDir) ? (1.0f - m_easeT) : hkMath::max2(m_easeT, 0.0f);
	m_easeInvDuration = (duration > HK_REAL_EPSILON) ? (1.0f / denominator) : HK_REAL_MAX;
	m_easeDir = true;

	return duration * (1.0f - m_easeT);
}

// We assume that ease out and in are symmetric
inline hkReal hkDefaultAnimationControl::easeOut(hkReal duration)
{
	const hkReal denominator = duration; // avoids sn compiler warning

	m_easeT = (m_easeDir) ? (1.0f - m_easeT) : hkMath::max2(m_easeT, 0.0f);
	m_easeInvDuration = (duration > HK_REAL_EPSILON) ? (1.0f / denominator) : HK_REAL_MAX;
	m_easeDir = false;

	return duration * (1.0f - m_easeT);
}
// Add a listener 
inline void hkDefaultAnimationControl::addDefaultControlListener(hkDefaultAnimationControlListener* listener)
{
	HK_ASSERT2(0x5efefba3, m_defaultListeners.indexOf( listener ) < 0, "You tried to add  a control listener twice" );
	m_defaultListeners.pushBack( listener );
}

// Remove a listener
inline void hkDefaultAnimationControl::removeDefaultControlListener(hkDefaultAnimationControlListener* listener)
{
	int i = m_defaultListeners.indexOf( listener );
	HK_ASSERT2(0x2c7b3925, i >= 0, "You tried to remove a control listener, which was never added" );
	m_defaultListeners.removeAt(i);
}

// Set the amount (in local seconds) to crop the start of the animation.
inline void hkDefaultAnimationControl::setCropStartAmountLocalTime( hkReal cropStartAmountLocalTime )
{
	m_cropStartAmountLocalTime = cropStartAmountLocalTime;

	// make sure the local time is kept in range
	if ( m_localTime < m_cropStartAmountLocalTime )
	{
		m_localTime = m_cropStartAmountLocalTime;
	}
}

// Set the amount (in local seconds) to crop the end of the animation.
inline void hkDefaultAnimationControl::setCropEndAmountLocalTime( hkReal cropEndAmountLocalTime )
{
	m_cropEndAmountLocalTime = cropEndAmountLocalTime;

	hkReal endTime = m_binding->m_animation->m_duration - m_cropEndAmountLocalTime;

	// make sure the local time is kept in range
	if ( m_localTime >= endTime )
	{
		m_localTime = endTime;
	}
}

// Get the amount (in local seconds) to crop the start of the animation.
inline hkReal hkDefaultAnimationControl::getCropStartAmountLocalTime()
{
	return m_cropStartAmountLocalTime;
}

// Get the amount (in local seconds) to crop the end of the animation.
inline hkReal hkDefaultAnimationControl::getCropEndAmountLocalTime()
{
	return m_cropEndAmountLocalTime;
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
