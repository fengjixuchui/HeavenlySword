/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SAMPLED_HEIGHT_FIELD_SHAPE_H
#define HK_COLLIDE2_SAMPLED_HEIGHT_FIELD_SHAPE_H

#include <hkcollide/shape/heightfield/hkHeightFieldShape.h>
#include <hkcollide/shape/sampledheightfield/hkSampledHeightFieldBaseCinfo.h>
#include <hkmath/basetypes/hkSphere.h>
#include <hkmath/linear/hkVector4Util.h>

extern const hkClass hkSampledHeightFieldShapeClass;

/// This class implements some default behaviour for the hkSampledHeightFieldShape.
/// It uses the y coordinate for the up axis
/// This is a template class for 2d sampled heightfields.
/// The x and z coordinates are used to lookup a height information
/// If you want to use this class, you need to subclass it and 
/// implement the following functions:
///  - 	HK_FORCE_INLINE hkReal getHeightAt( int x, int z ) const
///     which returns the height at a given x,z
///  -  HK_FORCE_INLINE hkBool getTriangleFlip()
///		this should return true  if two triangles share the edge p(x,z) - p(x+1,z+1)
///     this should return false if the triangles sphere the edge p(x,z+1) - p(x+1,z);
///  -  collideSpheres
///     This should forward to collideSpheresImplementation. This inlines your
///     getHeightAt and getTriangleFlip.
/// <br>
/// Example:<br>
/// class MySampledHeightFieldShape : public hkSampledHeightFieldShape<br>
/// {
///    HK_FORCE_INLINE hkReal getHeightAt( int x, int z ) const{ return x + y; }<br>
///	   HK_FORCE_INLINE hkBool getTriangleFlip(){ return false; }<br>
///    virtual void collideSpheres( const CollideSpheresInput& input, SphereCollisionOutput* outputArray) const
///    {
///        return collideSpheresImplementation(*this, input, outputArray);
///    }
///	};<br>
/// <br>
class hkSampledHeightFieldShape : public hkHeightFieldShape
{
	public:

		HK_DECLARE_REFLECTION();

		hkSampledHeightFieldShape( const hkSampledHeightFieldBaseCinfo& ci );

			/// Get the construction you used to create the heightfield
		void getCinfo( hkSampledHeightFieldBaseCinfo& cinfo ) const;

			///	hkShape Interface 
		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			/// hkHeightFieldShape interface implementation
		virtual void collideSpheres( const CollideSpheresInput& input, SphereCollisionOutput* outputArray) const = 0;

			/// hkHeightFieldShape interface implementation
		virtual void castSphere( const hkSphereCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			/// get the height at x,z. Note x,z are already clipped
		virtual hkReal getHeightAt( int x, int z ) const = 0;

			/// Return true if two triangles share the edge p(x,z) - p(x+1,z+1)
			/// Return false if the triangles sphere the edge p(x,z+1) - p(x+1,z);
		virtual hkBool getTriangleFlip() const = 0;

		hkShapeType getType() const;

			/// hkShape interface implementation
			///  - 	output.m_extraInfo = hitX << 1 + (hitZ<<16). Lowest bit is used for triangle index.
			/// Notes on the implementation:
			///  -  The ray can never tunnel through two neighbouring triangles.
			///  -  The ray is not epsilon proof when it just touches the landscape.
			///  -  The ray algorithm uses a fast walking algorithm over the landscape, 
			///     that means very long rays over very big height fields cost quite some CPU
			///  -  The code is somehow fast, however quite big. Try to combine many ray queries together to minimize instruction cache hits
			///  -  Rays that are nearly vertical to the heightfield get a different optimized treatment.
			///  -  There are no restrictions on the scale, it can be anything, even negative
			///  -  If the ray starts or ends outside the heightfield, no problem, it gets clipped correctly
			///  -  the size of the heightfield is limited to 16k * 16k
		hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& output ) const;

			/// hkShape interface implementation
		void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

		void getHeightAndNormalAt( int xPos, int zPos, hkReal subX, hkReal subZ, hkVector4& normalOut, hkReal& heightOut, int& triangleIndexOut ) const;

	private:
		
			// implementation in .cpp - inlined several times
		HK_FORCE_INLINE void _getHeightAndNormalAt( int xPos, int zPos, hkReal subX, hkReal subZ, hkVector4& normalOut, hkReal& heightOut, int& triangleIndexOut ) const;
		void castRayInternal( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkBool reportPenetratingStartPosition, hkReal maxExtraPenetration, hkRayHitCollector& collector ) const;

	public:
		
		int m_xRes;
		int m_zRes;

		hkReal m_heightCenter;

		hkVector4 m_intToFloatScale;
		hkVector4 m_floatToIntScale;
		hkVector4 m_floatToIntOffsetFloorCorrected;
		hkVector4 m_extents;

	public:

		hkSampledHeightFieldShape( ) {}
		hkSampledHeightFieldShape( hkFinishLoadedObjectFlag flag ) : hkHeightFieldShape(flag) {}
};

// Work around vc6 member templates bug. Should be hkSampledHeightFieldShape::collideSpheres.
template<typename IMPL>
HK_FORCE_INLINE void HK_CALL hkSampledHeightFieldShape_collideSpheres(
	const IMPL& shape,
	const hkHeightFieldShape::CollideSpheresInput& input,
	hkHeightFieldShape::SphereCollisionOutput* outputArray);


#include <hkcollide/shape/sampledheightfield/hkSampledHeightFieldShape.inl>

#endif // HK_COLLIDE2_SAMPLED_HEIGHT_FIELD_SHAPE_H

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
