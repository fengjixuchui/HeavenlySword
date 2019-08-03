/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_COLLIDE2_TYPED_BROAD_PHASE_HANDLE_PAIR_H
#define HK_COLLIDE2_TYPED_BROAD_PHASE_HANDLE_PAIR_H

#include <hkinternal/collide/broadphase/hkBroadPhaseHandlePair.h>
#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandle.h>


///	A structure holding the result of a broadphase query.
class hkTypedBroadPhaseHandlePair : public hkBroadPhaseHandlePair
{
	public:

			/// Get the first element.
		HK_FORCE_INLINE hkTypedBroadPhaseHandle* getElementA() const;

			/// Get the second element.
		HK_FORCE_INLINE hkTypedBroadPhaseHandle* getElementB() const;

			/// Given either elementA or elementB, get the other element.
		HK_FORCE_INLINE hkTypedBroadPhaseHandle* getOther(hkTypedBroadPhaseHandle* elem) const;

};

#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseHandlePair.inl>

#endif // HK_COLLIDE2_BROAD_PHASE_HANDLE_PAIR_H

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
