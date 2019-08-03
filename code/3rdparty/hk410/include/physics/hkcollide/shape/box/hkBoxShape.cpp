/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/box/hkBoxShape.h>


#include <hkmath/linear/hkVector4Util.h>
#include <hkcollide/util/hkAabbUtil.h>

#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkbase/class/hkTypeInfo.h>

#if defined HK_COMPILER_MSVC
	// C4701: local variable 'lastNormal' and 'hitNormal' may be used without having been initialized
#	pragma warning(disable: 4701)
#endif

#if HK_POINTER_SIZE==4
HK_COMPILE_TIME_ASSERT( sizeof(hkBoxShape) == 32 );
#endif

HK_REFLECTION_DEFINE_VIRTUAL(hkBoxShape);

static inline hkBool HK_CALL isPositive(const hkVector4& v )
{
	return (v(0) > 0.0f) && (v(1) > 0.0f) && (v(2) > 0.0f);
}

void hkBoxShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("BoxShape", collector->MEMORY_SHARED, this);
	collector->endObject();
}

hkBoxShape::hkBoxShape( const hkVector4& halfExtents, hkReal radius )
:	hkConvexShape(radius), m_halfExtents(halfExtents)
{
	m_halfExtents(3) = ( m_halfExtents( 0 ) < m_halfExtents( 1 ) ) ? m_halfExtents ( 0 ) : m_halfExtents( 1 );
	m_halfExtents(3) = ( m_halfExtents( 2 ) < m_halfExtents( 3 ) ) ? m_halfExtents ( 2 ) : m_halfExtents( 3 );

	HK_ASSERT2(0x1cda850c,  isPositive(m_halfExtents), "hkBoxShape passed a NONPOSITIVE-valued extent");
}

hkShapeType hkBoxShape::getType() const
{
	return HK_SHAPE_BOX;
}

void hkBoxShape::setHalfExtents(const hkVector4& halfExtents)
{
	HK_ASSERT2(0x5e756678,  isPositive(halfExtents), "hkBoxShape passed a NONPOSITIVE-valued extent");
	m_halfExtents = halfExtents;
	m_halfExtents(3) = ( m_halfExtents( 0 ) < m_halfExtents( 1 ) ) ? m_halfExtents ( 0 ) : m_halfExtents( 1 );
	m_halfExtents(3) = ( m_halfExtents( 2 ) < m_halfExtents( 3 ) ) ? m_halfExtents ( 2 ) : m_halfExtents( 3 );

}

			// hkConvexShape interface implentation.
void hkBoxShape::getFirstVertex(hkVector4& v) const
{
	v = m_halfExtents;
}


void hkBoxShape::getAabb(const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkAabbUtil::calcAabb( localToWorld, m_halfExtents, tolerance + m_radius, out );
}


void hkBoxShape::getSupportingVertex(const hkVector4& direction, hkCdVertex &supportVertex) const
{
	static_cast<hkVector4&>(supportVertex) = m_halfExtents;
	hkVector4Util::mulSigns4(supportVertex, direction);

	// get a unique number that specifies which corner we've got.
	int vertexID = HK_VECTOR3_COMPARE_MASK_XYZ & supportVertex.compareLessThanZero4();
	supportVertex.setInt24W( vertexID );
}

// featureBitSet  bits 0x  [
// actually this should be used for any architecture with xyz in bitfield reversed

#if defined(HK_COMPILER_HAS_INTRINSICS_IA32) && HK_CONFIG_SIMD_ENABLED == HK_CONFIG_SIMD
HK_COMPILE_TIME_ASSERT( HK_VECTOR3_COMPARE_MASK_XYZ == 0x07 );
static HK_ALIGN16( const float hkBoxShape_vertexSignArray[8][4] ) = {
	{ 1.0f, 1.0f, 1.0f, 0.0f },	// zyx = 000
	{-1.0f, 1.0f, 1.0f, 0.0f },	// zyx = 001
	{ 1.0f,-1.0f, 1.0f, 0.0f },	// zyx = 010
	{-1.0f,-1.0f, 1.0f, 0.0f },	// zyx = 011
	{ 1.0f, 1.0f,-1.0f, 0.0f },	// zyx = 100
	{-1.0f, 1.0f,-1.0f, 0.0f }, // zyx = 101
	{ 1.0f,-1.0f,-1.0f, 0.0f }, // zyx = 110
	{-1.0f,-1.0f,-1.0f, 0.0f }	// zyx = 111
};

#else
HK_COMPILE_TIME_ASSERT( HK_VECTOR3_COMPARE_MASK_XYZ == 0x0e );
static HK_ALIGN16( const float hkBoxShape_vertexSignArray[8][4] ) = {
	{ 1.0f, 1.0f, 1.0f, 0.0f },	// xyz = 000
	{ 1.0f, 1.0f,-1.0f, 0.0f },	// xyz = 001
	{ 1.0f,-1.0f, 1.0f, 0.0f },	// xyz = 010
	{ 1.0f,-1.0f,-1.0f, 0.0f },	// xyz = 011
	{-1.0f, 1.0f, 1.0f, 0.0f },	// xyz = 100
	{-1.0f, 1.0f,-1.0f, 0.0f }, // xyz = 101
	{-1.0f,-1.0f, 1.0f, 0.0f }, // xyz = 110
	{-1.0f,-1.0f,-1.0f, 0.0f }	// xyz = 111
};

#endif

void hkBoxShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	for (int i = numIds-1; i>=0; i--)
	{
		int index;
		if(HK_VECTOR3_COMPARE_MASK_XYZ == 0x00000007)
		{
			index = ids[0] << 4;
		}
		else
		{
			// ack PS2 ( and maybe others ) have xyz field at 8,4,2 instead of 1,2,4
			index = ids[0] << 3;
		}
		verticesOut[0].setMul4( m_halfExtents, *hkAddByteOffsetConst<hkVector4>( (const hkVector4*)hkBoxShape_vertexSignArray, index ));
		verticesOut[0].setInt24W( ids[0] );
		verticesOut++;
		ids++;
	}
}

void hkBoxShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	infoOut.m_numSpheres = 8;
	infoOut.m_useBuffer = true;
}

static HK_ALIGN16( const float vertexDirectionArray[8][4] ) = {
	{ 1.0f, 1.0f, 1.0f, 1.0f },	// zyx = 000
	{-1.0f, 1.0f, 1.0f, 1.0f },	// zyx = 001
	{ 1.0f,-1.0f, 1.0f, 1.0f },	// zyx = 010
	{-1.0f,-1.0f, 1.0f, 1.0f },	// zyx = 011
	{ 1.0f, 1.0f,-1.0f, 1.0f },	// zyx = 100
	{-1.0f, 1.0f,-1.0f, 1.0f }, // zyx = 101
	{ 1.0f,-1.0f,-1.0f, 1.0f }, // zyx = 110
	{-1.0f,-1.0f,-1.0f, 1.0f }	// zyx = 111
};

const hkSphere* hkBoxShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	hkSphere* s = sphereBuffer;

	hkVector4 radius = m_halfExtents;
	radius(3) = m_radius;

	const hkVector4* vertexDirection = reinterpret_cast<const hkVector4*>(vertexDirectionArray);
	for(int i = 3; i>=0; i--)
	{
		hkVector4 v0; v0.setMul4(radius, vertexDirection[0]);
		hkVector4 v1; v1.setMul4(radius, vertexDirection[1]);
		s[0].setPositionAndRadius(v0);
		s[1].setPositionAndRadius(v1);
		s+=2;
		vertexDirection+=2;
	}
	return sphereBuffer;
}





// Boundary coordinate sign bit meanings for the "AND" of the 'outcodes'
// sign (b)	sign (t)
// 0		0		whole segment inside box
// 1		0		b is outside box, t is in
// 0		1		b is inside box, t is out
// 1		1		whole segment outside box

// ray-box intersection with ideas from 'A trip down the graphics pipeline', Jim Blinn
// if ray starts within the box, no intersection is returned
// return 1 for success, 0 for no intersection. when success then hitpoint and hitnormal is filled



hkBool hkBoxShape::castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const
{
	HK_TIMER_BEGIN("rcBox", HK_NULL);
	hkVector4 negativeHalfExtents;
	hkVector4 positiveHalfExtents;
	hkVector4 radiusExtents;
	radiusExtents.setAll(m_radius);
	positiveHalfExtents.setAdd4( m_halfExtents, radiusExtents );
	negativeHalfExtents.setNeg4( positiveHalfExtents );

	int outcode0a = positiveHalfExtents.compareLessThanEqual4(input.m_from) & HK_VECTOR3_COMPARE_MASK_XYZ;
	int outcode0b = input.m_from.compareLessThanEqual4(negativeHalfExtents) & HK_VECTOR3_COMPARE_MASK_XYZ;
	int outcode1a = positiveHalfExtents.compareLessThanEqual4(input.m_to) & HK_VECTOR3_COMPARE_MASK_XYZ;
	int outcode1b = input.m_to.compareLessThanEqual4(negativeHalfExtents) & HK_VECTOR3_COMPARE_MASK_XYZ;

	if ((!((outcode0a & outcode1a)||(outcode0b & outcode1b))) && // ray is not it's completely at the outside of (or more) faces
		((outcode0a|outcode0b)))	//'from' is not completely inside
	{
		const int maskiterator [3] = {
			HK_VECTOR3_COMPARE_MASK_X,
			HK_VECTOR3_COMPARE_MASK_Y,
			HK_VECTOR3_COMPARE_MASK_Z
		};
		int mask;
		int clipcode = (outcode0a| outcode1a);
		int	outcode = outcode0a;
		float enteringLambda = 0.f;
		float exitingLambda  =	results.m_hitFraction;	//don't go to the end of the ray (1.f) but current mindist
		float normaldir=1.0f;
		hkVector4 hitnormal;

		for (int j=1; j>=0; normaldir=-1.f, j--)
		{
			for (int i=2;i>=0;i--)
			{
				mask = maskiterator[i];
				if (clipcode & mask)
				{
					//all the usual dot product calculations disappear
					//because the normal only has 1 non-zero component
					//also the plane-equation H(n,d) with n the normal and d the plane constant
					//is the same for both negative and positive planes
					//plane equation constand d for plane i
					//plane (n,d), x.n + d = 0, so d = -x dot n, for x in plane
					const float plane_d = negativeHalfExtents(i);
					const float spd0 = normaldir * input.m_from(i) + plane_d;  // optimized from normal.dot(from)+plane_d;
					const float spd1 = normaldir * input.m_to(i)   + plane_d;  //optimized from normal.dot(to)+plane_d;
					const float lambda = spd0 / (spd0-spd1);

					//entering or exiting, outcode!
					if (outcode & mask) {
						if ( lambda >= enteringLambda )
						{
							enteringLambda = lambda;
							hitnormal.setZero4();
							hitnormal(i) = normaldir;
						}
					}
					else //exiting
					{
						exitingLambda = hkMath::min2(exitingLambda,lambda);
					}

					if	(exitingLambda < enteringLambda)
					{
						HK_TIMER_END();
						return false;
					}
				} //endif (clipcode & mask)
			}
			clipcode = (outcode0b| outcode1b);
			outcode = outcode0b;
		}

		results.m_normal = hitnormal;
		results.m_hitFraction = enteringLambda;
		results.setKey( HK_INVALID_SHAPE_KEY );
		// results.m_extraInfo = index of face
		HK_TIMER_END();
		return true;
	}
	HK_TIMER_END();
	return false;
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
