/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_TRI_SAMPLED_HEIGHTFIELD_COLLECTION
#define HK_TRI_SAMPLED_HEIGHTFIELD_COLLECTION

#include <hkmath/hkMath.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>

extern const hkClass hkTriSampledHeightFieldCollectionClass;

class hkSampledHeightFieldShape;
extern hkReal hkConvexShapeDefaultRadius;


/// You can use this shape to wrap a hkSampledHeightFieldBase shape. To use this, you must also combine this shape
/// with the hkTriSampledHeightFieldBvTreeShape, i.e. create a hkTriSampledHeightFieldBvTreeShape, passing
/// this shape into its constructor. This shape provides an alternative implementation to the sampled heightfield,
/// where a different collision algorithm is used, while maintaining the memory usage of the heightfield.
/// Instead of the approximate collision detection algorithm used by the heightfield shape (where colliding objects
/// are represented by sets of spheres), this shape converts the heightfield into triangles on the fly.
/// Collision detection is thus slower, but fully exact.
/// Because the shapes pulled out during collision detection are actual hkTriangleShapes, they may need a radius
/// (which would not be necessary if using the approximate collision detection algorithm).
/// Ray casting is performed using the standard heightfield raycast implementation.
/// N.B. Per-triangle filtering can be done by inheriting and implementing the method hkShapeCollection::getCollisionFilterInfo( hkShapeKey key )
/// as required. The current implementation does not implement this method because the user can best decide where to store the filter info.
/// NOTE: Linear casting is performed using a simple implementation where the aabb of the linear cast is queried against
/// the heightfield, and all overlapping triangles are checked with a linear cast. WARNING: for long linear casts,
/// (particularly those parallel to the heightfield surface) this can be very slow.
class hkTriSampledHeightFieldCollection : public hkShapeCollection
{
	public:
		HK_DECLARE_REFLECTION();

			/// The constructor takes a hkSampledHeightFieldBase, and a radius. Note that the heightfield
			/// does not have a radius by default, but when converting the heightfield into triangles for collision detection
			/// it is advisable to give the triangles a non-zero radius for best performance.
			/// This adds a reference to the hkSampledHeightFieldBase passed in.
		hkTriSampledHeightFieldCollection( const hkSampledHeightFieldShape* shape, hkReal radius = hkConvexShapeDefaultRadius );

			/// Removes a reference from the referenced heightfield.
		~hkTriSampledHeightFieldCollection();


		//
		// hkShapeCollection interface
		//

			/// Gets the first key for the collection.
			/// The key returned by this function can be used to create a triangle shape corresponding to the key.  
			/// The mapping is as follows: 
			/// The lowest bit is used for the triangle "flag" determining which triangle of the quad is being referenced.
			/// The next lowest 15 bits of the key are used for the x coordinate of the quad.
			/// The upper 16 bits of the key are used for the z coordinate of the quad.
			/// So you can extract subpart/triangle indices like this:<br>
			/// int xCoord = (key & 0x0000ffff) >> 1;
			/// int zCoord = key >> 16;
			/// int whichTriangle = (key & 1);	// will be 0 or 1<br>
			/// For this reason only heightfields of resolution less than 65535 are allowed to be used with this shape.
			/// For more detail please refer to the heightfield section of the user guide.
		virtual hkShapeKey getFirstKey() const;

			/// Get the next key. This iterates over the heightfield's x-z grid. See getFirstKey() for a description of the
			/// key returned.
		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const;

			/// This returns a triangle corresponding to the grid cell indexed by the x-z 
			/// coordinates and the determining "flag" stored in the input shape key.
		virtual const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const;

		//
		// hkShape interface
		//

			/// This forwards to hkSampledHeightFieldBase::getAabb()
		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			/// Get the heightfield referenced by the shape
		const hkSampledHeightFieldShape* getHeightFieldShape() const { return m_heightfield; }

			/// Get the radius of this shape. This will be the radius of all triangles created by the shape.
		inline hkReal getRadius() const { return m_radius; }


	protected:

		const hkSampledHeightFieldShape* m_heightfield;

		hkReal m_radius;

	public:

		hkTriSampledHeightFieldCollection( hkFinishLoadedObjectFlag flag ) {}

};

#endif // #ifndef HK_TRI_SAMPLED_HEIGHTFIELD_COLLECTION

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
