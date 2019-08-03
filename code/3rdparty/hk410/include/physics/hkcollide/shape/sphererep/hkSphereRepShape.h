/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLISION_SPHERES_SHAPE_H
#define HK_COLLIDE2_COLLISION_SPHERES_SHAPE_H

#include <hkcollide/shape/hkShape.h>

extern const hkClass hkSphereShapeClass;
class hkSphere;

#define HK_GET_COLLISION_SPHERE_BUFFER_SIZE 64

/// This interface produces a set of spheres that represent a very simplified version of the objects surface.
/// Note: This interface's function is used by hkHeightFieldShape implementations
class hkSphereRepShape : public hkShape
{
	public:

		HK_DECLARE_REFLECTION();

		struct hkCollisionSpheresInfo
		{
			int m_numSpheres;
			hkBool m_useBuffer;
		};

			/// Get information about the call getCollisionSpheres
		virtual void getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const = 0;

			/// Gets a set of spheres representing a simplified shape. For instance, a box could return its eight corners.
		virtual const hkSphere* getCollisionSpheres( hkSphere* sphereBuffer ) const = 0;
	

	public:

		hkSphereRepShape() {}

		hkSphereRepShape( hkFinishLoadedObjectFlag flag ) : hkShape(flag) {}

};


#endif // HK_COLLIDE2_COLLISION_SPHERES_SHAPE_H

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
