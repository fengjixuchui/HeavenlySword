/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_MIN_DISTANCE_COLLISION_INFO_COLLECTOR_H
#define HK_MIN_DISTANCE_COLLISION_INFO_COLLECTOR_H

#include <hkcollide/shape/hkRayHitCollector.h>
#include <hkcollide/castutil/hkWorldRayCastOutput.h>

/// hkClosestRayHitCollector collects the closest contact point and information about a cast along a linear path.
/// Both the closest contact point for non-penetrating (cast) and for penetrating cases are collected.
class hkClosestRayHitCollector : public hkRayHitCollector
{
	public:
			/// Constructor calls reset.
		inline hkClosestRayHitCollector();

			/// Resets this structure if you want to reuse it for another raycast.
		inline void reset();

		inline virtual ~hkClosestRayHitCollector();
		
			/// returns true, if this class has collected a hit
		inline hkBool hasHit( ) const;

			/// returns the full hit information
		inline const hkWorldRayCastOutput& getHit() const;

	protected:

		virtual void addRayHit( const hkCdBody& cdBody, const hkShapeRayCastCollectorOutput& hitInfo );
	
	protected:
		hkWorldRayCastOutput	m_rayHit;
};

#include <hkcollide/collector/raycollector/hkClosestRayHitCollector.inl>

#endif //HK_MIN_DISTANCE_COLLISION_INFO_COLLECTOR_H


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
