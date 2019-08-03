/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_PAIRWISE_COLLISION_FILTER_H
#define HK_UTILITIES2_PAIRWISE_COLLISION_FILTER_H

#include <hkcollide/filter/hkCollisionFilter.h>
#include <hkdynamics/entity/hkEntityListener.h>
#include <hkdynamics/entity/hkEntity.h>

extern const hkClass hkPairwiseCollisionFilterClass;

/// A simple filter to disable collisions on a pairwise basis. 
/// Note that this is currently VERY INEFFICIENT with large numbers of pairs, as it is implemented using a linear list.
/// If you want to use this style of collision filter in a production game you should definitely use your own optimized filter.
class hkPairwiseCollisionFilter : public hkCollisionFilter, private hkEntityListener
{
	public:
 
		HK_DECLARE_REFLECTION();

			///
		hkPairwiseCollisionFilter();

			///
		~hkPairwiseCollisionFilter();

			/// Disables collisions between this pair of collidables.
		void disableCollisionPair(hkEntity* a, hkEntity* b);

			/// Enables collisions between this pair of collidables.
		void enableCollisionPair(hkEntity* a, hkEntity* b);

			/// hkCollisionFilter interface implementation.
			/// This implementation checks if the pair is in the disabled list.
		virtual hkBool isCollisionEnabled(const hkCollidable& a, const hkCollidable& b ) const;

			// hkShapeCollectionFilter interface forwarding
		virtual	hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const;

			// hkRayShapeCollectionFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const;

			// hkRayCollidableFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const;

			/// hkEntityListener implementation - removes any references to removed entities.
		virtual void entityRemovedCallback(hkEntity* entity);

			/// hkEntityListener implementation.
		virtual void entityDeletedCallback(hkEntity* entity);
		
			/// gives the number of disabled pairs in the filter
		virtual int  getNumDisabledPairs() const;

	public:

		struct CollisionPair
		{
			HK_DECLARE_REFLECTION();

			inline CollisionPair(hkEntity* a, hkEntity* b);
			
			hkEntity* m_a;
			hkEntity* m_b;
		};

	public:

		hkArray<struct CollisionPair> m_disabledPairs; // changed back from hkMap is it is just an array too.

	public:

		hkPairwiseCollisionFilter(hkFinishLoadedObjectFlag f) {}
};

inline hkBool operator == (const hkPairwiseCollisionFilter::CollisionPair& a, const hkPairwiseCollisionFilter::CollisionPair& b);

#include <hkutilities/collide/filter/pairwise/hkPairwiseCollisionFilter.inl>

#endif // HK_PAIRWISE_COLLISION_FILTER_H

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
