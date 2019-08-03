/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// Return the local time for the control
inline hkReal hkAnimationControl::getLocalTime() const
{
	return m_localTime;
}

// Set the local time for the animation
inline void hkAnimationControl::setLocalTime( hkReal lTime )
{
	m_localTime = lTime;
}

inline hkReal hkAnimationControl::getWeight() const
{
	return m_weight;
}

inline const hkAnimationBinding* hkAnimationControl::getAnimationBinding() const
{
	return m_binding;
}

inline hkUint8 hkAnimationControl::getTrackWeight(hkUint32 track) const
{
	return m_trackWeights[track];
}

inline void hkAnimationControl::setTrackWeight(hkUint32 track, hkUint8 weight)
{	
	m_trackWeights[track] = weight;
}

// Get the weight values for all tracks (non-const access)
inline const hkArray<hkUint8>& hkAnimationControl::getTracksWeights() const
{
	return m_trackWeights;
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
