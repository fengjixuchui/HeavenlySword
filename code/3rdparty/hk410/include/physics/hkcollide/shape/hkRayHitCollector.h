/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_RAY_HIT_COLLECTOR
#define HK_RAY_HIT_COLLECTOR

struct hkShapeRayCastInput;
struct hkShapeRayCastCollectorOutput;
class hkCdBody;
typedef hkUint32 hkShapeKey;

#include <hkmath/hkMath.h>

/// This class is used to get ray cast results via a callback. To use it, inherit
/// from this class and implement the addRayHit() function.
/// The class contains a value called "m_earlyOutHitFraction". It is the fraction
/// along the ray beyond which no more hits will be considered. It is initialised
/// to 1 in the default constructor, which means that all hits will be reported.
/// See hkAllRayHitCollector and hkClosestRayHitCollector for concrete implementations
/// of this class.
class hkRayHitCollector
{
	public:

			/// The constructor initialises the early out hit fraction to 1.
		inline hkRayHitCollector();

			/// This is the function called for every hit of the raycaster.
			/// Note: for optimization purposes this should set the m_earlyOutHitFraction to:
			/// - 0.0 if you want to get no more hits
			/// - 1.0 if you want to get all hits (constructor initializes this value to 1.0 by default)
			/// - output.m_hitFraction if you only want to get closer hits than one just found
			/// All hkShape::castRay() methods except for hkShapeCollection (and hkListShape) use
			/// this value to exit early if they can and thus avoid unnecessary calculations.
			/// Note: output is defined in world space.
		virtual void addRayHit( const hkCdBody& cdBody, const hkShapeRayCastCollectorOutput& hitInfo ) = 0;

			/// This resets the m_earlyOutHitFraction to 1. You should call this if you want to
			/// reuse the class for a new ray cast.
		inline void reset();

			/// Virtual destructor for derived objects
		virtual ~hkRayHitCollector() { }

			/// Extract the cdbody hierarchy shapekeys into buf.
		static int HK_CALL shapeKeysFromCdBody( hkShapeKey* buf, int maxKeys, const hkCdBody& body );

	public:

		hkReal m_earlyOutHitFraction;
};

#include <hkcollide/shape/hkRayHitCollector.inl>

#endif // HK_RAY_HIT_COLLECTOR

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
