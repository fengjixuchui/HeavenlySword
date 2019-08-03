/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MOPP_BV_TREE_SHAPE_H
#define HK_COLLIDE2_MOPP_BV_TREE_SHAPE_H

#include <hkcollide/shape/bvtree/hkBvTreeShape.h>

extern const hkClass hkMoppBvTreeShapeClass;

class hkMoppCode;

	/// This class implements a hkBvTreeShape using MOPP technology.
class hkMoppBvTreeShape: public hkBvTreeShape
{
	public:

		HK_DECLARE_REFLECTION();

			/// Constructs a new hkMoppBvTreeShape. You can use the <hkMoppUtility.h> to build a MOPP code.
		hkMoppBvTreeShape( const hkShapeCollection* collection, const hkMoppCode* code);

			// destructor
		virtual ~hkMoppBvTreeShape();

		
			// hkBvTreeShape interface implementation.
		virtual void querySphere( const class hkSphere &sphere, hkArray<hkShapeKey>& hits ) const;

			// hkBvTreeShape interface implementation.
		virtual void queryObb( const hkTransform& obbToMopp, const hkVector4& extent, hkReal tolerance, hkArray<hkShapeKey>& hits ) const;

			// hkBvTreeShape interface implementation.
		virtual void queryAabb( const hkAabb& aabb, hkArray<hkShapeKey>& hits ) const;

			/// Gets the aabb, the current implementation is rather slow, because it simply iterates over all children.
			/// Usually this is not a problem, as the for static objects getAabb will be called a single time.
 		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			// hkShape Interface implementation
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const;

			// hkShape Interface implementation
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;
		
			/// Gets this hkShape's type. For hkMoppBvTreeShapes, this is HK_SHAPE_MOPP.
		virtual hkShapeType getType() const;

			/// Get the internal data used by the MOPP algorithms
		inline const hkMoppCode* getMoppCode() const { return m_code; }

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

	protected:

		const class hkMoppCode* m_code;

	public:

		hkMoppBvTreeShape( hkFinishLoadedObjectFlag flag ) : hkBvTreeShape(flag) {}

};


#endif // HK_COLLIDE2_MOPP_BV_TREE_SHAPE_H

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
