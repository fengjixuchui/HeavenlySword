/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DEFAULT_CONTROL_LISTENER_H
#define HK_DEFAULT_CONTROL_LISTENER_H

#include <hkbase/hkBase.h>

class hkDefaultAnimationControl;

/// Override this class to get callbacks for default control events
/// These events are : overflow, underflow and end of ease in/out
/// You need to call hkDefaultAnimationControl::addDefaultControlListener() to add a new listener to a default control
class hkDefaultAnimationControlListener
{
	public:

		/// Called whenever the hkDefaultAnimationControl overflows
		virtual void loopOverflowCallback(hkDefaultAnimationControl* control, hkReal deltaTime, hkUint32 overflows) {}

		/// Called whenever the hkDefaultAnimationControl underflows
		virtual void loopUnderflowCallback(hkDefaultAnimationControl* control, hkReal deltaTime, hkUint32 underflows) {}

		/// Called whenever the hkDefaultAnimationControl finishes easing in
		virtual void easedInCallback(hkDefaultAnimationControl* control, hkReal deltaTime) {}

		/// Called whenever the hkDefaultAnimationControl finishes easing out
		virtual void easedOutCallback(hkDefaultAnimationControl* control, hkReal deltaTime) {}

			/// Virtual destructor for derived objects
		virtual ~hkDefaultAnimationControlListener() {}
};


#endif // HK_DEFAULT_CONTROL_LISTENER_H


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
