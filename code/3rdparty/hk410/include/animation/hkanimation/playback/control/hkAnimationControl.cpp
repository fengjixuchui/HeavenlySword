/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkanimation/hkAnimation.h>
#include <hkanimation/animation/hkAnimationBinding.h>
#include <hkanimation/animation/hkSkeletalAnimation.h>
#include <hkanimation/playback/control/hkAnimationControl.h>
#include <hkanimation/playback/control/hkAnimationControlListener.h>

hkAnimationControl::hkAnimationControl( const hkAnimationBinding* binding )
	: m_binding(binding)
{
	m_weight = 1.0f;
	m_localTime = 0.0f;

	if (binding)
	{
		m_trackWeights.setSize( m_binding->m_animation->m_numberOfTracks, 0xff);
	}
}

hkAnimationControl::~hkAnimationControl()
{
		// Fire deletion events
	for (int i=0; i < m_listeners.getSize(); i++)
	{
		m_listeners[i]->controlDeletedCallback(this);
	}
}

void hkAnimationControl::setAnimationBinding( const hkAnimationBinding* binding)
{
	m_binding = binding;

	//HKA-615 resize the weight array if the binding changes.
	m_trackWeights.setSize( binding->m_animation->m_numberOfTracks );
}

void hkAnimationControl::addAnimationControlListener(hkAnimationControlListener* listener)
{
	HK_ASSERT2(0x5efefba3, m_listeners.indexOf( listener ) < 0, "You tried to add a control listener twice" );
	m_listeners.pushBack( listener );
}

void hkAnimationControl::removeAnimationControlListener(hkAnimationControlListener* listener)
{
	int i = m_listeners.indexOf( listener );
	HK_ASSERT2(0x2c7b3925, i >= 0, "You tried to remove a control listener, which was never added" );
	m_listeners.removeAt(i);
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
