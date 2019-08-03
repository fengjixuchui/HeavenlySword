/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_LIST_SHAPE_H
#define HK_COLLIDE2_LIST_SHAPE_H

#include <hkcollide/shape/collection/hkShapeCollection.h>

extern const hkClass hkListShapeClass;

/// A simple static list of hkShapes. You can use this shape class to create compound bodies.
/// A list shape can hold a mix of different shape types e.g. an ice cream cone could be made 
/// from a list shape containing a sphere for the ice cream and a convex vertices shape 
/// for the wafer cone.
/// If your list shapes contains many subshapes, consider using a hkBvTreeShape for faster access
class hkListShape : public hkShapeCollection
{
	public:

		HK_DECLARE_REFLECTION();

		struct ChildInfo
		{
			HK_DECLARE_REFLECTION();

			const hkShape* m_shape;
			hkUint32 m_collisionFilterInfo;
		};

	public:

			/// Constructs a list shape with an array of pointers to shapes.
		hkListShape( const hkShape*const* shapeArray, int numShapes );

			/// The destructor removes references to child shapes.
		~hkListShape();


			/// Returns the ith child shape.
		inline const hkShape* getChildShape(int i) const { return m_childInfo[i].m_shape; }

		
		//
		// hkShapeCollection interface
		//


			// hkShapeCollection interface implementation.
		virtual int getNumChildShapes() const;

			/// Get the first child shape key.
		virtual hkShapeKey getFirstKey() const;

			/// Get the next child shape key.
		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const;

			// hkShapeCollection interface implementation.
		virtual hkUint32 getCollisionFilterInfo( hkShapeKey key ) const;

			/// Sets the collisionFilterInfo for a given index
		void setCollisionFilterInfo( hkShapeKey index, hkUint32 filterInfo );

			/// Note that a hkListShape does not use the char* buffer for its returned shape.
		virtual const hkShape* getChildShape(hkShapeKey key, ShapeBuffer& buffer ) const;
			
		
		//
		// hkShape interface
		//


			/// Gets the AABB for a given transform and an extra tolerance.
		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			/// Gets this hkShape's type. For hkListShapes, this is HK_SHAPE_LIST.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;
	public:

		friend class hkListAgent;
		// hkInplaceArray<struct ChildInfo,4> m_childInfo;
		hkArray<struct ChildInfo> m_childInfo;

	protected:

		void setShapes( const hkShape*const* shapeArray, int numShapes, const hkUint32* filterInfoArray = HK_NULL);

	public:

		hkListShape( class hkFinishLoadedObjectFlag flag ) : hkShapeCollection(flag), m_childInfo(flag) {}
};



#endif // HK_COLLIDE2_LIST_SHAPE_H

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
