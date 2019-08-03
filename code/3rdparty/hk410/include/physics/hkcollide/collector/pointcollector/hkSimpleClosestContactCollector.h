/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SIMPLE_MIN_DISTANCE_COLLISION_INFO_COLLECTOR_H
#define HK_SIMPLE_MIN_DISTANCE_COLLISION_INFO_COLLECTOR_H

#include <hkcollide/agent/hkCdPointCollector.h>
#include <hkcollide/collector/pointcollector/hkRootCdPoint.h>


/// This class collects the closest contact point, but does not
/// gather any other information. This is useful if you are interested only in the
/// distance, normal and position and not in hkShapeKeys and rootCollidables.
/// Use this class for the maximum performance<br>
/// Note: read the notes for hkCdPointCollector
class hkSimpleClosestContactCollector : public hkCdPointCollector
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkSimpleClosestContactCollector);

			/// Constructor cals reset
		inline hkSimpleClosestContactCollector  ();

			/// Resets the early out condition.
			/// You must call this function if you want to reuse an object of this class.
		inline void reset();

		inline virtual ~hkSimpleClosestContactCollector();
		
			/// Returns true, if this class has collected a hit.
		inline hkBool hasHit( ) const;

			/// Returns only the physical hit information: position, normal, distance
		inline const hkContactPoint& getHitContact() const;

	protected:

			// this implementation keeps track of the hkCdPoint with the smallest distance
		virtual void addCdPoint( const hkCdPoint& pointInfo ) ;

	protected:
		hkBool m_hasHit;
		hkContactPoint	m_hitPoint;
};

#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.inl>

#endif // HK_SIMPLE_MIN_DISTANCE_COLLISION_INFO_COLLECTOR_H


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
