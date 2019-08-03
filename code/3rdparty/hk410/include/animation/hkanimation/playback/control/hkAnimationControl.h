/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_ANIMATION_CONTROL_H
#define HK_ANIMATION_CONTROL_H

#include <hkbase/hkBase.h>

class hkAnimationBinding;
class hkAnimationControlListener;

/// This abstract class is the base class for all animation controllers.
/// It contains the local time and weight for a playing animation.
class hkAnimationControl : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ANIM_CONTROL);

		hkAnimationControl( const hkAnimationBinding* binding );

			/// Destructor notifies listeners of the controls impending demise.
		virtual ~hkAnimationControl();

			/// Advance the local clock by the specified delta time and return the new state
		virtual void update( hkReal stepDelta )= 0;

			/// Return the local time for the control
		inline hkReal getLocalTime() const;

			/// Set the local time for the animation
		inline void setLocalTime( hkReal lTime );

			/// Get the current weight this control
		inline hkReal getWeight() const;

			/// Returns the local time of the control in the future.
			/// Local time is guaranteed to lie in the range 0..duration.
			/// If the stepDelta overflows / underflows the end of the animation then loopsOut is positive / negative.
		virtual void getFutureTime( hkReal stepDelta, hkReal& localTimeOut, int& loopsOut) const = 0;

			/*
			 * Per track weighting / feathering
			 */ 
			
			/// Return the per track weight for this track
			/// If no track weight has been set this defaults to 1
			/// The weight is renormalized when used (0..255 -> 0..1)
		inline hkUint8 getTrackWeight(hkUint32 track) const;

			/// Sets the per track weight for this track
			/// If no weights have already been set then a
			/// weight array in internally allocated and initialized to 1.0f
			/// The weight is renormalized when used (0..255 -> 0..1)
		inline void setTrackWeight(hkUint32 track, hkUint8 weight);

			/// Get the weight values for all tracks (non-const access)
		inline const hkArray<hkUint8>& getTracksWeights() const;

			/// Const access to the original animation binding
		inline const hkAnimationBinding* getAnimationBinding() const;

			/// Set the animation binding
		void setAnimationBinding( const hkAnimationBinding* binding);

			/// Add a listener 
		void addAnimationControlListener(hkAnimationControlListener* listener);

			/// Remove a listener
		void removeAnimationControlListener(hkAnimationControlListener* listener);

	protected:

			/// The localtime used to sample the playing animation
		hkReal	m_localTime;

			/// The current weight for this control
		hkReal	m_weight;

			/// An array of bone track weights (in range 0 to 255 mapped to 0..1)
		hkArray<hkUint8> m_trackWeights;

			/// The binding to the skeleton
		const hkAnimationBinding* m_binding;

			// Animation Control listeners
		hkArray<hkAnimationControlListener*> m_listeners;
};

#include <hkanimation/playback/control/hkAnimationControl.inl>

#endif // HK_ANIMATION_CONTROL_H


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
