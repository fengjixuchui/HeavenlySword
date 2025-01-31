/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_ROOT_CD_POINT_H
#define HK_ROOT_CD_POINT_H

#include <hkmath/basetypes/hkContactPoint.h>
#include <hkcollide/shape/hkShape.h>

class hkCollidable;
	/// This class is used by the default collectors, hkAllCdPointCollector, and hkClosestCdPointCollector.
	/// As most of data in a hkCdPoint is just temporary, 
	/// we can only extract the root information from a hkCdPoint<br>
	/// If you need more detailed information, you have to implement a collector yourself.
struct hkRootCdPoint
{
		/// The physical information of the contact point.
		/// Note: if this information contains cast results, then m_contact.getDistance
		/// is a fraction between 0 and 1 (and not the Euclidean distance).
	hkContactPoint m_contact;

		/// The root collidable of object A
	const hkCollidable*	m_rootCollidableA;

		/// The shape key of the colliding shape of object A
		/// Note: if your root collidable is a hkMeshShape, this will be the subpart and triangle index
	hkShapeKey m_shapeKeyA;

		/// The root collidable of object B
	const hkCollidable*	m_rootCollidableB;

		/// The shape key of the colliding shape of object B
		/// Note: if your root collidable is a hkMeshShape, this will be the subpart and triangle index
	hkShapeKey m_shapeKeyB;

		/// Comparison operator for sorting RootCdPoints
	inline hkBool operator<( const hkRootCdPoint& b ) const;
};

#include <hkcollide/collector/pointcollector/hkRootCdPoint.inl>

#endif //HK_ROOT_CD_POINT_H

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
