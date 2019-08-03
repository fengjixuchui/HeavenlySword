/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DEFAULT_CONTROL_H
#define HK_DEFAULT_CONTROL_H

#include <hkanimation/playback/control/hkAnimationControl.h>
#include <hkanimation/animation/hkAnimationBinding.h>
#include <hkanimation/animation/hkSkeletalAnimation.h>

class hkDefaultAnimationControlListener;

/// This class represents a simple animation control.
/// It provides basic support for varying playback speed, fading and looping
class hkDefaultAnimationControl : public hkAnimationControl
{
	public:


		hkDefaultAnimationControl( const hkAnimationBinding* binding );
			
		hkDefaultAnimationControl(const hkDefaultAnimationControl& other);

			/*
			 * Control interface
			 */

			/// Advance the local clock by the specified delta time and return the new state
		void update( hkReal stepDelta );


			/// Returns the local time of the control in the future.
			/// Local time is guaranteed to lie in the range 0..duration.
			/// If the stepDelta overflows / underflows the end of the animation then loopsOut is positive / negative.
		virtual void getFutureTime( hkReal stepDelta, hkReal& localTimeOut, int& loopsOut) const;


			/*
			 * Basic Control
			 */

			/// Return the master weight for the control
		inline hkReal getMasterWeight() const;

			/// Set the master weight for this playing animation
		inline void setMasterWeight( hkReal weight );

			/// Return the current playback speed
		inline hkReal getPlaybackSpeed() const;

			/// Set the playback speed.
			/// If speed is set to a negative value the animation will run backwards.
		inline void setPlaybackSpeed( hkReal speed );

			/*
			 * Loop counting
			 */

			/// Return how many times this control has looped passed the end of the animation
		inline hkUint32 getOverflowCount() const;

			/// Reset how many times this control has looped passed the end of the animation
		inline void setOverflowCount( hkUint32 count );

			/// Return how many times this control has looped passed the start of the animation
		inline hkUint32 getUnderflowCount() const;

			/// Reset how many times this control has looped passed the start of the animation
		inline void setUnderflowCount( hkUint32 count );

			/*
			 * Ease Curve Control
			 */

			/// Set the ease in curve. The curve is a Bezier defined by
			/// B(t) = t^3(-p0 + 3p1 - 3p2 + p3) + t^2(3p0 - 6p1 + 3p2) + t^1(-3p0 + 3p1) + t^0(p0)
		inline void setEaseInCurve(hkReal y0, hkReal y1, hkReal y2, hkReal y3);

			/// Set the ease out curve. The curve is a Bezier defined by
			/// B(t) = t^3(-p0 + 3p1 - 3p2 + p3) + t^2(3p0 - 6p1 + 3p2) + t^1(-3p0 + 3p1) + t^0(p0)
		inline void setEaseOutCurve(hkReal y0, hkReal y1, hkReal y2, hkReal y3);

			/// Ease in the control according to the curve.
			/// Returns the time at which the control will be fully eased In
		inline hkReal easeIn(hkReal duration);

			/// Ease out the control according to the curve.
			/// Returns the time at which it will be fully eased out.
		inline hkReal easeOut(hkReal duration);

		enum EaseStatus
		{
			EASING_IN,
			EASED_IN,
			EASING_OUT,
			EASED_OUT
		};

			/// Returns the status of the easing for this control
		inline enum EaseStatus getEaseStatus() const;

			/*
			 * Listener interface
			 */

			/// Add a listener 
		void addDefaultControlListener(hkDefaultAnimationControlListener* listener);

			/// Remove a listener
		void removeDefaultControlListener(hkDefaultAnimationControlListener* listener);

			/*
			 * Cropping
			 */

			/// Set the amount (in local seconds) to crop the start of the animation.
			/// This number should always be between 0 and the duration of the animation
		inline void setCropStartAmountLocalTime( hkReal cropStartAmountLocalTime );

			/// Set the amount (in local seconds) to crop the end of the animation.
			/// This number should always be between 0 and the duration of the animation
			/// For example, to crop the last half second from an animation of any length, 
			/// pass in 0.5f.
		inline void setCropEndAmountLocalTime( hkReal cropEndAmountLocalTime );

			/// Get the amount (in local seconds) to crop the start of the animation.
		inline hkReal getCropStartAmountLocalTime();

			/// Get the amount (in local seconds) to crop the end of the animation.
		inline hkReal getCropEndAmountLocalTime();

	protected:

		// Master weight for this control
		hkReal	m_masterWeight;

		// Speed Control
		hkReal m_playbackSpeed;

		// loop counters
		hkUint32 m_overflowCount;
		hkUint32 m_underflowCount;

		// Ease controls
		hkVector4 m_easeInCurve;	// 4 control pts 
		hkVector4 m_easeOutCurve;	// 4 control pts
		hkReal	m_easeInvDuration;	// 1/duration of the ease
		hkReal	m_easeT;			// Current parameterisation for the ease curve
		hkBool  m_easeDir;			// Direction in/out

		// cropping amounts (in positive local seconds from the respective endpoint of the clip)
		hkReal m_cropStartAmountLocalTime;
		hkReal m_cropEndAmountLocalTime;

			// Control listeners
		hkArray<hkDefaultAnimationControlListener*> m_defaultListeners;
};

#include <hkanimation/playback/control/default/hkDefaultAnimationControl.inl>

#endif // HK_DEFAULT_CONTROL_H


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
