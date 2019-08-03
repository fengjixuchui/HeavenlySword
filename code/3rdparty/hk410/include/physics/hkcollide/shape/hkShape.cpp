/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/hkRayHitCollector.h>
#include <hkmath/basetypes/hkSphere.h>
#include <hkmath/linear/hkVector4Util.h>

#if HK_POINTER_SIZE==4
HK_COMPILE_TIME_ASSERT( sizeof(hkShape) == 12 );
#endif

hkReal hkShape::getMaximumProjection( const hkVector4& direction ) const
{
	hkTransform localToWorld;
	localToWorld.setIdentity();
	const hkReal tolerance = 0.f;
	hkAabb aabb;
	getAabb( localToWorld, tolerance, aabb);
	
	hkVector4 halfExtents;
	halfExtents.setSub4(aabb.m_max, aabb.m_min);
	halfExtents.mul4( 0.5f );
	hkVector4 center; 
	center.setAdd4(aabb.m_max, aabb.m_min);
	center.mul4( 0.5f );
	
	hkVector4Util::mulSigns4(halfExtents , direction);

	halfExtents.add4(center);

	const hkReal result = halfExtents.dot3( direction );
	return result;
}



/*! \fn hkBool hkShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const
* Generally we recommend that you use the hkWorld or hkPhantom castRay() functions for raycasting.
* Always finds the closest hit and only reports this single hit.
* Returns 0 if there is no hit and 1 if there is a hit.
* The following rules apply:
* 1) A startpoint does not return a hit if the startpoint is in contact with the surface of an object and 
* the ray does not intersect the object. 
* - The exception to this rule is hkTriangleShape which DOES return a hit in rule 1) above.
* 2) If a ray is parallel and exactly tangental to the objects geometric surface, it won't hit. One exception to this rule is hkCapsuleShape which does return a hit.
* 3) If the start point of a ray is inside the object, it won't hit
* 4) It only returns a hit, if the new m_hitFraction is less than the current results.m_hitFraction 
*   which should be initialized with 1.0f)
* 5) It returns true if it hits
* 6) If it hits, than it sets the result.m_hitFraction to less than 1.0f, it does not set it if it does not hit.	
*/

/*! \fn hkShapeType hkShape::getType() const
* The hkCollisionDispatcher uses hkShape types to select an appropriate hkCollisionAgent
* for each pair of potentially colliding objects.
* If the collision dispatcher does not have a suitable registered collision agent for the objects based on their primary types, it 
* goes on to check their secondary types. The list of possible shape types is defined in hkShapeTypes. 
*/

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
