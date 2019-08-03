/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SHAPE_TYPES_H
#define HK_COLLIDE2_SHAPE_TYPES_H


/// All shape types. The dispatcher has only to implement at least the types that can be used as secondary types
enum hkShapeType
{
	HK_SHAPE_INVALID = 0,
		
		// 
		//	The abstract base shapes<br>
		//

		/// All shapes which inherit from hkConvexShape have this as an alternate type.
	HK_SHAPE_CONVEX,

		/// The first real shape.
	HK_FIRST_SHAPE_TYPE = HK_SHAPE_CONVEX,


		/// All shapes which inherit from hkShapeCollection have this as an alternate type.
	HK_SHAPE_COLLECTION,

		/// All shapes which inherit from hkBvTreeShape have this as an alternate type.
	HK_SHAPE_BV_TREE,

		//
		//	Special convex shapes, which get their private agents for better performance.
		//
		/// hkSphereShape type.
	HK_SHAPE_SPHERE,

		/// hkCylinderShape type.
	HK_SHAPE_CYLINDER, 
		/// hkTriangleShape type.
	HK_SHAPE_TRIANGLE,
		/// hkBoxShape type.
	HK_SHAPE_BOX,
		/// hkCapsuleShape type.
	HK_SHAPE_CAPSULE,
		/// hkConvexVerticesShape type.
	HK_SHAPE_CONVEX_VERTICES,
		/// hkConvexPieceShape type.
	HK_SHAPE_CONVEX_PIECE,

		//
		//	Special HK_SHAPE_COLLECTION implementations
		//
		/// hkMultiSphereShape type.
	HK_SHAPE_MULTI_SPHERE,
		/// hkListShape type.
	HK_SHAPE_LIST,

		/// hkConvexListShape, a List of convex pieces which are treated as a single convex object if possible.
	HK_SHAPE_CONVEX_LIST,

		/// hkConvexTranslateShape, allows to translate other convex shapes (especially boxes).
	HK_SHAPE_CONVEX_TRANSLATE,

		/// hkConvexTransformShape, allows to transform other convex shapes (especially boxes), usually faster
		/// than hkTransformShape.
	HK_SHAPE_CONVEX_TRANSFORM,

		/// A shape collection which only returns triangles as child shapes, e.g. hkMeshShape.
	HK_SHAPE_TRIANGLE_COLLECTION,


		// 
		// Special shapes
		// 
		/// hkMultiRayShape type.
	HK_SHAPE_MULTI_RAY,
		/// hkHeightFieldShape type.
	HK_SHAPE_HEIGHT_FIELD,
		/// hkSampledHeightFieldShape type.
	HK_SHAPE_SAMPLED_HEIGHT_FIELD,
		// hkTriPatchShape type.
	HK_SHAPE_TRI_PATCH,
		/// hkSphereRepShape type.
	HK_SHAPE_SPHERE_REP, 
		/// hkBvShape type.
	HK_SHAPE_BV,
		/// hkPlaneShape type.
	HK_SHAPE_PLANE,
		/// hkMoppBvTreeShape type.
	HK_SHAPE_MOPP,


		//
		//	Single shapes which are processed by unary agents.
		//
		
		/// hkTransformShape type.
	HK_SHAPE_TRANSFORM,
		/// hkPhantomCallbackShape type.
	HK_SHAPE_PHANTOM_CALLBACK,


		//
		//	user shapes
		//

	HK_SHAPE_USER0,
	HK_SHAPE_USER1,
	HK_SHAPE_USER2,

		///	The end of the shape type list.
	HK_SHAPE_MAX_ID,

		/// All shape flag, used by the hkCollisionDispatcher.
	HK_SHAPE_ALL = -1

};

	/// A utility function to return a useful name for a given shape type
const char* HK_CALL hkGetShapeTypeName( hkShapeType type );


class hkCollisionDispatcher;

	/// Register all havok specific shapeTypes
	/// This needs to be called at setup time to tell the dispatcher which
	/// shapes inherit from other shapes
void HK_CALL hkRegisterAlternateShapeTypes( hkCollisionDispatcher* dis );


#endif // HK_COLLIDE2_SHAPE_TYPES_H

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
