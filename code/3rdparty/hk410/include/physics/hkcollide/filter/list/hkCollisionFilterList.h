/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_COLLIDE2_COLLISION_FILTER_SET_H
#define HK_COLLIDE2_COLLISION_FILTER_SET_H

#include <hkcollide/filter/hkCollisionFilter.h>

extern const hkClass hkCollisionFilterListClass;

	/// This class allows you to create a collision filter that is composed of multiple other collision filters.
	/// For each of the isCollisionEnabled() methods, this filter iterates through its list of filters in reverse
	/// order. If any collision filter in the list returns false, then the list filter returns false. If all
	/// collision filters in the list return true, the list filter returns true.
class hkCollisionFilterList : public hkCollisionFilter
{
	public:

		HK_DECLARE_REFLECTION();
	
		hkCollisionFilterList();

		hkCollisionFilterList( const hkArray<const hkCollisionFilter*>& collisionFilters );

			/// Destructor removes all references to collision filters
		~hkCollisionFilterList();


			/// Get the list of collision filters
		const hkArray<const hkCollisionFilter*>& getCollisionFilters() const;

			/// Adds a new collision filter: Attention: this does not start reevaluating the collidable pairs.
			/// You have to call hkWorld::updateCollisionFilter to do this.
		void addCollisionFilter( const hkCollisionFilter* filter);

			/// Adds a new collision filter: Attention: this does not start reevaluating the collidable pairs
			/// You have to call hkWorld::updateCollisionFilter to do this.
		void removeCollisionFilter( const hkCollisionFilter* filter);

			/// If any collision filter in the filter list returns false, then this function returns returns false. If all
			/// collision filters in the list return true, this function returns true.
		virtual hkBool isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const;

			/// If any collision filter in the filter list returns false, then this function returns returns false. If all
			/// collision filters in the list return true, this function returns true.
		virtual	hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const;

			/// If any collision filter in the filter list returns false, then this function returns returns false. If all
			/// collision filters in the list return true, this function returns true.
		virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& bShape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const;

			/// If any collision filter in the filter list returns false, then this function returns returns false. If all
			/// collision filters in the list return true, this function returns true.
		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& aInput, const hkCollidable& collidableB ) const;


	protected:

		//hkInplaceArray<const hkCollisionFilter*,3> m_collisionFilters;
		hkArray<const hkCollisionFilter*> m_collisionFilters;

	public:

		hkCollisionFilterList( class hkFinishLoadedObjectFlag flag ) : m_collisionFilters(flag) {}
};

#endif // HK_COLLIDE2_COLLISION_FILTER_SET_H

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
