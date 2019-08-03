/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE_STORAGESAMPLEDHEIGHTFIELDSHAPE_H
#define HK_COLLIDE_STORAGESAMPLEDHEIGHTFIELDSHAPE_H

#include <hkcollide/shape/sampledheightfield/hkSampledHeightFieldShape.h>
#include <hkcollide/shape/sampledheightfield/hkSampledHeightFieldBaseCinfo.h>

extern const hkClass hkStorageSampledHeightFieldShapeClass;

/// A heightfield shape which stores the heights as an array of hkReals.
/// This class is most useful for debug snapshots in the same way as hkStorageMeshShape.
/// Normally the heightfield data would be stored in a more memory efficient manner,
/// possibly shared with other components.
class hkStorageSampledHeightFieldShape : public hkSampledHeightFieldShape
{
	public:

		HK_DECLARE_REFLECTION();

		hkStorageSampledHeightFieldShape( const hkSampledHeightFieldShape* hf );

		hkStorageSampledHeightFieldShape( const hkSampledHeightFieldBaseCinfo& info, hkArray<hkReal>& samples );

		HK_FORCE_INLINE hkReal getHeightAt( int x, int z ) const
		{
			return m_storage[z*m_xRes + x];
		}

		HK_FORCE_INLINE hkBool getTriangleFlip() const
		{	
			return m_triangleFlip;
		}

		virtual void collideSpheres( const CollideSpheresInput& input, SphereCollisionOutput* outputArray) const
		{
			hkSampledHeightFieldShape_collideSpheres(*this, input, outputArray);
		}

	public:

		hkArray<hkReal> m_storage;
		hkBool m_triangleFlip;

	public:

		hkStorageSampledHeightFieldShape( const hkFinishLoadedObjectFlag f )
			: hkSampledHeightFieldShape(f), m_storage(f)
		{
		}
};

#endif // HK_COLLIDE_STORAGESAMPLEDHEIGHTFIELDSHAPE_H

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
