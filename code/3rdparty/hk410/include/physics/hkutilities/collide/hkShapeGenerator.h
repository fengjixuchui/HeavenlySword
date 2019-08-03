/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SHAPE_GENERATOR
#define HK_SHAPE_GENERATOR

#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>

class hkPseudoRandomGenerator;

///This class can be used to create random hkConvexVerticesShapes.
class hkShapeGenerator
{
	public:
		enum Flags { NONE, NO_PLANE_EQUATIONS };

		static hkConvexVerticesShape* HK_CALL createRandomConvexVerticesShape(	const hkVector4& minbox, 
																				const hkVector4& maxbox, 
																				int numvert, 
																				hkPseudoRandomGenerator *generator,
																				Flags flags = NONE );

		static hkConvexVerticesShape* HK_CALL createRandomConvexVerticesShapeWithThinTriangles(	const hkVector4& minbox, 
																								const hkVector4& maxbox, 
																								int numvert, 
																								float minEdgeLen, 
																								hkPseudoRandomGenerator *generator,
																								Flags flags = NONE );

		enum ShapeType
		{
				RANDOM,
				SPHERE,
				CAPSULE,
				BOX,
				TRIANGLE,
				//THIN_TRIANGLE,
				CONVEX_VERTICES,
				CONVEX_VERTICES_BOX,
				SHAPE_MAX
		};

		static hkConvexShape* HK_CALL createConvexShape( const hkVector4& entents, ShapeType type, hkPseudoRandomGenerator *generator );

		static const char* HK_CALL getShapeTypeName( ShapeType type );
};

#endif //HK_SHAPE_GENERATOR


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
