/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkanimation/hkAnimation.h>
#include <hkanimation/playback/control/default/hkDefaultAnimationControl.h>
#include <hkanimation/playback/control/default/hkDefaultAnimationControlListener.h>
#include <hkanimation/motion/hkAnimatedReferenceFrame.h>

hkDefaultAnimationControl::hkDefaultAnimationControl( const hkAnimationBinding* binding )
: hkAnimationControl(binding)
{
	m_binding = binding;
	m_masterWeight = 1;
	m_playbackSpeed = 1;
	m_overflowCount = 0;
	m_underflowCount = 0;

	m_easeInCurve.set(0,0,1,1);
	m_easeOutCurve.set(1,1,0,0);

	// Begin fully eased in
	m_easeInvDuration = 10.0f;	// 0.1 sec ease
	m_easeT = 1.0f;				// Fully eased
	m_easeDir = true;			// In

	m_cropStartAmountLocalTime = 0.0f;
	m_cropEndAmountLocalTime = 0.0f;
}

hkDefaultAnimationControl::hkDefaultAnimationControl(const hkDefaultAnimationControl& other)
: hkAnimationControl(other.getAnimationBinding())
{
	m_masterWeight = other.m_masterWeight;
	m_playbackSpeed = other.m_playbackSpeed;
	m_overflowCount = other.m_overflowCount;
	m_underflowCount = other.m_underflowCount;

	m_easeInCurve = other.m_easeInCurve;
	m_easeOutCurve = other.m_easeOutCurve;

	// Begin fully eased in
	m_easeInvDuration = other.m_easeInvDuration;
	m_easeT = other.m_easeT;
	m_easeDir = other.m_easeDir;

	m_cropStartAmountLocalTime = other.m_cropStartAmountLocalTime;
	m_cropEndAmountLocalTime = other.m_cropEndAmountLocalTime;
}

void hkDefaultAnimationControl::update( hkReal stepDelta )
{
	// Grab animPeriod
	hkReal animationPeriod = HK_REAL_MAX;

	if (m_binding != HK_NULL)
	{
		const hkSkeletalAnimation* animation = m_binding->m_animation;

		if ( animation )
		{
			// apply cropping to the duration, but don't crop more than 100%
			animationPeriod = hkMath::max2( 0.0f, animation->m_duration - m_cropEndAmountLocalTime - m_cropStartAmountLocalTime );
		}
	}

	// extract the cropping offset from m_localTime (we'll put it back at the end of this function)
	m_localTime -= m_cropStartAmountLocalTime;

	// this handles the case where m_localTime was initialized or explicitly set to before the crop point
	if ( m_localTime < 0.0f )
	{
		m_localTime = 0.0f;
	}

	// if the clip is 100% cropped then don't do anything else
	if ( animationPeriod != 0.0f )
	{
		// Advance time
		m_localTime += stepDelta * m_playbackSpeed;

		int callbackLoops = 0;

		// Check for overflow
		if (m_localTime > animationPeriod)
		{
			int loops = (int) (m_localTime / animationPeriod);
			m_localTime -= animationPeriod * loops;
			m_localTime = hkMath::max2( m_localTime, 0.0f );
			m_overflowCount += loops;
			callbackLoops = loops;
		}
		else if (m_localTime < 0.0f)
		{
			int loops = (int) ( -m_localTime / animationPeriod );
			m_localTime += animationPeriod * (loops + 1);
			m_localTime = hkMath::min2( m_localTime, animationPeriod );
			m_underflowCount += (loops+1);
			callbackLoops = -(loops+1);
		}

		// Check the local time is in bounds
		HK_ASSERT2( 0x13452345, (m_localTime >= 0.0f) && (m_localTime <= animationPeriod), "Local time out of bounds");
		

		// Advance ease curves
		bool fireEaseListeners = false;
		if (m_easeT < 1.0f)
		{
			m_easeT += hkMath::fabs( stepDelta ) * m_easeInvDuration;

			// Check for completion
			if (m_easeT > 1.0f)
			{
				m_easeT = 1.0f;
				fireEaseListeners = true;
			}
		}

		// Modulate the weight based on ease curve
		HK_ASSERT2(0x65ab5be5, (m_easeT >= 0.0f) && (m_easeT <= 1.0f), " Ease curves param out of range");

		const hkVector4& pts = m_easeDir ? m_easeInCurve : m_easeOutCurve;
		const hkReal& t = m_easeT;

		const hkReal curveVal = 
			((t*t*t) * (-pts(0)   + 3*pts(1) - 3*pts(2) + pts(3)) + 
			(t*t)    * (3*pts(0)  - 6*pts(1) + 3*pts(2)) + 
			t        * (-3*pts(0) + 3*pts(1))+
			pts(0));

		m_weight = m_masterWeight * curveVal;

		// Fire all events at the end of update

		// Overflow/Underflow events
		if (callbackLoops!=0)
		{
			for (int i=0; i < m_defaultListeners.getSize(); i++)
			{
				if (callbackLoops > 0)
				{
					m_defaultListeners[i]->loopOverflowCallback(this, stepDelta, callbackLoops);
				}
				else
				{
					m_defaultListeners[i]->loopUnderflowCallback(this, stepDelta, -callbackLoops);
				}
			}
		}

		// Ease in / out events
		if (fireEaseListeners)
		{
			for (int i=0; i < m_defaultListeners.getSize(); i++)
			{
				if (m_easeDir)
				{
					m_defaultListeners[i]->easedInCallback(this, stepDelta);
				}
				else
				{
					m_defaultListeners[i]->easedOutCallback(this, stepDelta);
				}
			}
		}
	}

	// put the cropping offset back into m_localTime
	m_localTime += m_cropStartAmountLocalTime;
}

void hkDefaultAnimationControl::getFutureTime( hkReal stepDelta, hkReal& localTimeOut, int& loopsOut) const
{
	hkReal duration = m_binding->m_animation->m_duration - m_cropEndAmountLocalTime - m_cropStartAmountLocalTime;

	// if the clip is 100% cropped just leave the time alone
	if ( duration <= 0.0f )
	{
		localTimeOut = m_localTime;
		loopsOut = 0;
	}
	else
	{
		// Advance time, but factor out the cropping offset
		localTimeOut = m_localTime - m_cropStartAmountLocalTime + stepDelta * m_playbackSpeed;

		// Check for overflow
		loopsOut = 0;

		// Overflow
		if (localTimeOut > duration)
		{
			loopsOut = (int) (localTimeOut / duration);
			localTimeOut -= duration * loopsOut;

		}
		// Underflow
		else if (localTimeOut < 0.0f)
		{
			loopsOut = (int) ( -localTimeOut / duration ) + 1;
			localTimeOut += duration * loopsOut;
		}

		// put back in cropping offset
		localTimeOut += m_cropStartAmountLocalTime;
	}
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
