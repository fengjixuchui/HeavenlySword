/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SIMPLE_WORLD_RAY_CASTER
#define HK_SIMPLE_WORLD_RAY_CASTER

#include <hkinternal/collide/broadphase/hkBroadPhaseCastCollector.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/castutil/hkWorldRayCastInput.h>

struct hkWorldRayCastInput;
struct hkWorldRayCastOutput;
class hkCollisionFilter;
class hkRayCollidableFilter;
class hkBroadPhase;
typedef char hkBroadPhaseAabbCache;

	/// This is an utility class, which connects the hkBroadPhase::castRay() to the hkShape::castRay
	/// This method uses the output structure interface (instead of the collector interface
	/// This is called by hkWorld::castRay(). Usually you should call hkWorld::castRay instead of 
	/// using this class directly.
class hkSimpleWorldRayCaster : public hkBroadPhaseCastCollector
{
	public:
	
		hkSimpleWorldRayCaster(){}

		~hkSimpleWorldRayCaster(){}

			/// This function cast a ray through the world.
			/// The "output" will be filled with the ray hit upon completion.
		void castRay( const hkBroadPhase& broadphase, const hkWorldRayCastInput& input, const hkCollisionFilter* filter, hkWorldRayCastOutput& output );

			/// Casts a group of rays in close proximity
		void castRayGroup( const hkBroadPhase& broadphase, const hkWorldRayCastInput* inputArray, int numRays, const hkCollisionFilter* filter, hkWorldRayCastOutput* outputs ) ;

			/// Same as castRay, only for experts. uses an hkBroadPhaseAabbCache: See hkBroadPhase for Details
		void castRay( const hkBroadPhase& broadphase, const hkWorldRayCastInput& input, const hkCollisionFilter* filter, hkBroadPhaseAabbCache* cache, hkWorldRayCastOutput& output );


			/// Cast multiple rays having the same startpoint
			/// Notes:
			///   - the startpoint is taken only from the first input
			///   - However filtering can be done separate for each ray.
		void castRaysFromSinglePoint( const hkBroadPhase& broadphase, const hkWorldRayCastInput* inputArray, int numRays, const hkCollisionFilter* filter, hkBroadPhaseAabbCache* cache, hkWorldRayCastOutput* outputs );
	protected:
		virtual	hkReal addBroadPhaseHandle( const hkBroadPhaseHandle* broadPhaseHandle, int castIndex );

	protected:
		const hkWorldRayCastInput*   m_input;
		const hkRayCollidableFilter* m_filter;
		hkWorldRayCastOutput*  m_result;

		// used as a temporary storage
		hkShapeRayCastInput    m_shapeInput;
};


#endif //HK_SIMPLE_WORLD_RAY_CASTER

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
