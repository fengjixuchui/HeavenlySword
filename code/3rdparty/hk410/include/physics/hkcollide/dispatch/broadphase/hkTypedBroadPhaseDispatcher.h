/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_TYPED_BROAD_PHASE_HANDLER
#define HK_TYPED_BROAD_PHASE_HANDLER

#include <hkcollide/filter/hkCollidableCollidableFilter.h>

class hkTypedBroadPhaseHandlePair;
class hkBroadPhaseHandlePair;
class hkBroadPhaseListener;

#define HK_MAX_BROADPHASE_TYPE 8

class hkTypedBroadPhaseDispatcher
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO, hkTypedBroadPhaseDispatcher);

		hkTypedBroadPhaseDispatcher();
		
		~hkTypedBroadPhaseDispatcher();

		/// sorts and removes duplicates
		static void HK_CALL removeDuplicates( hkArray<hkBroadPhaseHandlePair>& newPairs, hkArray<hkBroadPhaseHandlePair>& delPairs );

		/// swap each pair's internal data so A > B, then sort
		//static void HK_CALL sortPairs( hkTypedBroadPhaseHandlePair* pairs, int numPairs );

			/// update the dispatcher by merging
		void addPairs(	hkTypedBroadPhaseHandlePair* newPairs, int numNewPairs, const hkCollidableCollidableFilter* filter ) const;
		
		void removePairs( hkTypedBroadPhaseHandlePair* deletedPairs, int numDeletedPairs ) const;


		inline hkBroadPhaseListener* getBroadPhaseListener( int typeA, int typeB );

		inline void setBroadPhaseListener( hkBroadPhaseListener* listener, int typeA, int typeB );

		inline hkBroadPhaseListener* getNullBroadPhaseListener();

	protected:
		hkBroadPhaseListener*    m_broadPhaseListeners[HK_MAX_BROADPHASE_TYPE][HK_MAX_BROADPHASE_TYPE];
		hkBroadPhaseListener* m_nullBroadPhaseListener;
};

#include <hkcollide/dispatch/broadphase/hkTypedBroadPhaseDispatcher.inl>

#endif // HK_TYPED_BROAD_PHASE_HANDLER

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
