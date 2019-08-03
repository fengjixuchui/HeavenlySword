/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SHAPE_DISPLAY_BUILDER_H
#define HK_COLLIDE2_SHAPE_DISPLAY_BUILDER_H

#include <hkmath/hkMath.h>

#include <hkcollide/shape/hkShapeType.h>

class hkCollidable;
class hkDisplayGeometry;
class hkShape;

	/// A utility class that creates hkDisplayGeometrys from hkCollidables and hkShapes.
	/// This class is used by the hkShapeDisplayViewer.
class hkShapeDisplayBuilder: public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB);

		struct hkShapeDisplayBuilderEnvironment
		{
			hkShapeDisplayBuilderEnvironment();
			int m_spherePhiRes;
			int m_sphereThetaRes;
		};

		hkShapeDisplayBuilder(const hkShapeDisplayBuilderEnvironment& env);

			/// Creates an array of display geometries from a given collidable.
		void buildDisplayGeometries(		const hkShape* shape, 
											hkArray<hkDisplayGeometry*>& displayGeometries);

			/// Creates an array of display geometries from a given shape and a transform.
		void buildShapeDisplay(				const hkShape* shape, 
											const hkTransform& transform, 
											hkArray<hkDisplayGeometry*>& displayGeometries);

			/// Clears the current raw geometry.  A temporary geometry is used to
			/// create display geometries triangle by triangle from a MOPP for example.
		void resetCurrentRawGeometry();

			/// Convert a geometry and send it to the hkDebugDisplay
		static void HK_CALL addObjectToDebugDisplay( const hkShape* shape, hkTransform& t, hkUlong id );
	protected:

		hkDisplayGeometry* getCurrentRawGeometry(hkArray<hkDisplayGeometry*>& displayGeometries);

	protected:

		hkShapeDisplayBuilderEnvironment m_environment;
		hkDisplayGeometry* m_currentGeometry;


};

class hkUserShapeDisplayBuilder : public hkSingleton< hkUserShapeDisplayBuilder >
{
	public:

		hkUserShapeDisplayBuilder() {}

			/// A function to build display geometries for user shapes.  The function is expected to add hkDisplayGeometry objects
			/// to the displayGeometries list.  It may call back the hkShapeDisplayBuilder::buildDisplayGeometries() method on the
			/// builder object passed in to achieve this.
		typedef void (HK_CALL *ShapeBuilderFunction)(	const hkShape* shape,
														const hkTransform& transform,
														hkArray<hkDisplayGeometry*>& displayGeometries,
														hkShapeDisplayBuilder* builder );

			/// You can register functions to build display for your own user types with the shape display builder using this method
		void registerUserShapeDisplayBuilder( ShapeBuilderFunction f, hkShapeType type );

	public:

		struct UserShapeBuilder
		{
			ShapeBuilderFunction f;
			hkShapeType type;
		};
		hkArray< UserShapeBuilder > m_userShapeBuilders;


};


#endif // HK_COLLIDE2_SHAPE_DISPLAY_BUILDER_H

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
