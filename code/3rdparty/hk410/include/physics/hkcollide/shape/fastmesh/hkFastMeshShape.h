/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_FAST_MESH_SHAPE_H
#define HK_COLLIDE2_FAST_MESH_SHAPE_H

#include <hkcollide/shape/mesh/hkMeshShape.h>

extern const hkClass hkFastMeshShapeClass;


/// A specialized mesh, which implements a faster version of getChildShape() by
/// using a number of special requirements:
/// - only one subpart is allowed.
/// - all the vertices must be aligned on a 16 byte boundary
/// - only 16 bit triangle indices are used
class hkFastMeshShape: public hkMeshShape
{
	public:

		HK_DECLARE_REFLECTION();

		hkFastMeshShape( hkReal radius = hkConvexShapeDefaultRadius, int numBitsForSubpartIndex = 12 ) : hkMeshShape(radius, numBitsForSubpartIndex) { }

			//	hkShapeCollection interface implementation.
			/// Because the hkMeshShape references into your data,
			/// it must create a new hkTriangleShape to return to the caller when this function is called.
			/// This triangle is stored in the char* buffer 
			/// Degenerate triangles in the client data are handled gracefully through this method.
		virtual const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const;

	public:

		hkFastMeshShape( hkFinishLoadedObjectFlag flag ) : hkMeshShape(flag) {}

};

#endif // HK_COLLIDE2_FAST_MESH_SHAPE_H

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
