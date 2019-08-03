/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_WORLD_LINEAR_CASTER
#define HK_WORLD_LINEAR_CASTER

#include <hkinternal/collide/broadphase/hkBroadPhaseCastCollector.h>
#include <hkcollide/agent/hkLinearCastCollisionInput.h>

struct hkCollisionInput;
struct hkLinearCastInput;
struct hkCollisionAgentConfig;
class hkCdPointCollector;
class hkCollisionFilter;
class hkCollidableCollidableFilter;
class hkBroadPhase;

	/// This is a utility class you can use to perform a linear cast with a collidable against all other collidables in the broad
	/// phase. It has one function, linear cast.
	/// It effectively connects hkBroadPhase::castAabb with hkCollisionAgent::linearCast
	/// This is called by hkWorld::linearCast(). Usually you should call hkWorld::linearCast instead of 
	/// using this class directly.
class hkWorldLinearCaster : public hkBroadPhaseCastCollector
{
	public:
	
		hkWorldLinearCaster(){}

		~hkWorldLinearCaster(){}

			/// Inputs are:
			/// - a reference to the broad phase
			/// - the collidable to linear cast
			/// - the linear cast input
			/// - the collidable-collidable filter
			/// - the hkCollisionInput (for the narrow phase linear casts)
			/// - collectors for start points and cast points
			/// For each narrow phase linear cast hit, the collectors will receive a callback.
		void linearCast( const hkBroadPhase& broadphase, const hkCollidable* collA, 
					     const hkLinearCastInput& input, const hkCollidableCollidableFilter* filter,
						 const hkCollisionInput& collInput, hkCollisionAgentConfig* config, 
						 hkCdPointCollector& castCollector, hkCdPointCollector* startPointCollector );

	protected:
		virtual	hkReal addBroadPhaseHandle( const hkBroadPhaseHandle* broadPhaseHandle, int castIndex );

	protected:

		const hkLinearCastInput* m_input;
		const hkCollidableCollidableFilter* m_filter;
		hkCdPointCollector*			 m_castCollector;
		hkCdPointCollector*			 m_startPointCollector;
		const hkCollidable*			 m_collidableA;
		hkShapeType					 m_typeA;

		// used as a temporary storage
		hkLinearCastCollisionInput	m_shapeInput;
};


#endif //HK_BROAD_PHASE_LINEAR_CASTER

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
