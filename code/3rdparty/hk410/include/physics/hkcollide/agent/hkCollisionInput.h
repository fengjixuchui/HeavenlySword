/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLISION_INPUT_H
#define HK_COLLIDE2_COLLISION_INPUT_H

class hkCollisionDispatcher;
class hkShapeCollectionFilter;
class hkConvexListFilter;
struct hkCollisionAgentConfig;


	/// The basic structure needed for all hkCollisionAgent queries.
struct hkCollisionInput
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkCollisionInput);

		/// A pointer to the collision dispatcher, needed if more agents need to be created to complete the query
	hkCollisionDispatcher*   m_dispatcher;

		/// The filter used if any shape collections are queried. This filter will be called before any child shapes
		/// are considered for the query.
	const hkShapeCollectionFilter* m_filter;

		/// A filter used to decide how to treat convex list shapes in collisions.
	const hkConvexListFilter* m_convexListFilter;

		/// The collision tolerance. Only points closer than this tolerance will be reported.
	hkReal m_tolerance;

	mutable hkBool m_createPredictiveAgents;


		/// Get the collision tolerance
	hkReal getTolerance() const { return m_tolerance; }

		/// Set the collision tolerance
	void setTolerance(hkReal t) { m_tolerance = t; }
};

#endif // HK_COLLIDE2_COLLISION_INPUT_H

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
