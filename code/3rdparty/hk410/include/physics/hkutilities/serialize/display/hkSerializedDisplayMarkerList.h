/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_SERIALIZE_DISPLAYMARKER_LIST_H
#define HK_UTILITIES2_SERIALIZE_DISPLAYMARKER_LIST_H

#include <hkutilities/serialize/display/hkSerializedDisplayMarker.h>

class hkSerializedDisplayMarkerList : public hkReferencedObject
{
public:

	HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES);
	HK_DECLARE_REFLECTION();

	inline hkSerializedDisplayMarkerList();

	inline ~hkSerializedDisplayMarkerList();

		/// Get the number of markers mapped by this class.
	inline int getNumMarkers() const;
	
		/// Make sure that pi is with range.
	inline hkSerializedDisplayMarker* getMarker(int pi) const;

		/// Add a marker to the list
	inline void addMarker( hkSerializedDisplayMarker* m );

public:
	
	hkArray<hkSerializedDisplayMarker*> m_markers; // the transform for this marker.

	hkSerializedDisplayMarkerList(hkFinishLoadedObjectFlag flag) : m_markers(flag) { }
};

#include <hkutilities/serialize/display/hkSerializedDisplayMarkerList.inl>

#endif // HK_UTILITIES2_SERIALIZE_DISPLAYMARKER_LIST_H

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
