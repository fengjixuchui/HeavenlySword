/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkCylinderShape);

// assert to verify the correctness of getVertices()
HK_COMPILE_TIME_ASSERT( sizeof(hkVector4) == sizeof(hkSphere) );


#define HK_USING_APPROXIMATE_FLOAT_TO_INT_CONVERSION
#define HK_CONVERSION_TOLERANCE 0.05f

static hkReal minValueRoundedUpTo1()
{
	for (hkReal value = 0.0f; value < 1.1f; value += 0.01f)
	{
		int intValue = hkMath::hkToIntFast(value);
		HK_ASSERT2(0xad564bb4, (intValue & ~1) == 0, "Fast float-to-int conversion doesn't work as needed.");
		if (intValue != 0)
		{
			return value;
		}
	}
	HK_ASSERT2(0xad564bb4, false, "Fast float-to-int conversion doesn't work as needed.");
	return 1.0f;
}

static void assertRoundUpThreshold(hkReal intRoundUpThreshold)
{
	HK_ON_DEBUG( const char* msg = "hkCylinderShape::s_intRoundUpThreshold is not set properly, most likely the FPU mode has been changed. Reset the mode or reinitialize the threshold again." );
	HK_ASSERT2(0xad5674dd, hkMath::hkToIntFast(1 - intRoundUpThreshold) == 1, msg);
	HK_ASSERT2(0xad5674de, hkMath::hkToIntFast(1 - intRoundUpThreshold - 0.01f) == 0, msg);
}

hkReal hkCylinderShape::s_virtualTesselationParameter    = (2.0f - 2.0f * HK_CONVERSION_TOLERANCE) / 0.707f;  
hkReal hkCylinderShape::s_virtualTesselationParameterInv = 1.0f / ((2.0f - 2.0f * HK_CONVERSION_TOLERANCE) / 0.707f);
hkReal hkCylinderShape::s_intRoundUpThreshold = -1.0f;

void HK_CALL hkCylinderShape::setNumberOfVirtualSideSegements(int numSegments)
{
	HK_ASSERT2(0xad1bb4e3, numSegments >= 8 && numSegments <= 8*16, "hkCylinderShape::setNumberOfVirtualSideSegements only accepts values between 8 and 128.");
	if (numSegments%8 != 0 )
	{
		HK_WARN(0xad1bb4e3, "Number of cylinder side semgents rounded down to a multiple of 8.");
	}
	const hkReal value = (hkReal(numSegments / 8) - 2.0f * HK_CONVERSION_TOLERANCE) / 0.707f;  // 0.707f == sin(pi/4)
	s_virtualTesselationParameter = value;
	s_virtualTesselationParameterInv = 1.0f / value;
}

hkCylinderShape::hkCylinderShape( const hkVector4& vertexA, const hkVector4& vertexB, hkReal cylinderRadius, hkReal paddingRadius )
: hkConvexShape(paddingRadius)
{
	if (s_intRoundUpThreshold < 0.0f)
	{
		s_intRoundUpThreshold = 1.0f - minValueRoundedUpTo1();
	}

	m_vertexA = vertexA;
	m_vertexB = vertexB;

	// Set the actual radius of the cylinder
	setCylinderRadius( cylinderRadius );
	this->presetPerpendicularVector();

	m_cylBaseRadiusFactorForHeightFieldCollisions = 0.8f;
}

hkCylinderShape::hkCylinderShape( hkFinishLoadedObjectFlag flag ) : hkConvexShape(flag) 
{
	if (s_intRoundUpThreshold < 0.0f)
	{
		s_intRoundUpThreshold = 1.0f - minValueRoundedUpTo1();
	}
}

void hkCylinderShape::presetPerpendicularVector()
{
	hkVector4 unitAxis;
	unitAxis.setSub4(m_vertexB, m_vertexA);
	unitAxis.normalize3();
	hkVector4Util::calculatePerpendicularVector( unitAxis, m_perpendicular1 );

	m_perpendicular1.normalize3();
	m_perpendicular2.setCross(unitAxis, m_perpendicular1);
	//m_perpendicular2 must be a unit vector
}

hkReal hkCylinderShape::getCylinderRadius() const
{
	return m_cylRadius;
}

void hkCylinderShape::setCylinderRadius(const hkReal radius)
{
	m_cylRadius = radius;

	// updates the radius of the spheres representing the cylinder
	m_vertexA(3) = ( radius + m_radius );
	m_vertexB(3) = ( radius + m_radius );
}

void hkCylinderShape::decodeVertexId(hkVertexId code, hkVector4& result) const
{
	hkBool baseA     = (code >> VERTEX_ID_ENCODING_IS_BASE_A_SHIFT) & 1;
	hkBool sinSign   = (code >> VERTEX_ID_ENCODING_SIN_SIGN_SHIFT) & 1;
	hkBool cosSign   = (code >> VERTEX_ID_ENCODING_COS_SIGN_SHIFT) & 1;
	hkBool sinLesser = (code >> VERTEX_ID_ENCODING_IS_SIN_LESSER_SHIFT) & 1;
	// get the last 12 bytes for the value
	hkReal value     = hkReal(code & VERTEX_ID_ENCODING_VALUE_MASK);
	value += 0.5f;
	value *= s_virtualTesselationParameterInv;

	hkReal sin, cos;
	if (sinLesser)
	{
		sin = value;
		cos = hkMath::sqrt(1 - sin*sin);
	}
	else
	{
		cos = value;
		sin = hkMath::sqrt(1 - cos*cos);
	}

	// calc both cos & sin then
	// sin = sinLesser * value + (1 - sinLesser) hkMath::sqrt(1-value*value)

	if (!sinSign)
	{
		sin = -sin;
	}
	if (!cosSign)
	{	
		cos = -cos;
	}
	//sin = (-1 + 2 * sinSign) * sin;
	//cos = (-1 + 2 * cosSign) * sin;

	hkVector4 radius;
	{
		hkVector4 tmp1, tmp2;
		tmp1.setMul4(cos, m_perpendicular1);
		tmp2.setMul4(sin, m_perpendicular2);
		radius.setAdd4(tmp1, tmp2);
	}
	radius.mul4(m_cylRadius);
	result.setAdd4( getVertex(1-baseA) , radius );
}

void hkCylinderShape::getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const
{

	// direction is already in "this" space, so:

	// this function returns a point on the cylinder and ignores hkConvesShape::m_radius

	//
	// Generate vertexId
	//

	hkVector4 axis; axis.setSub4(m_vertexB, m_vertexA);

	hkReal cos = m_perpendicular1.dot3(direction);
	hkReal sin = m_perpendicular2.dot3(direction);

	const hkReal len2 = sin * sin + cos * cos;
	if (len2 >= HK_REAL_EPSILON * HK_REAL_EPSILON )
	{
		const hkReal invLen = hkMath::sqrtInverse(len2);
		sin *= invLen;
		cos *= invLen;
	}
	else
	{
		sin = 0.f;
		cos = 1.f;
	}

	const int sinSign = sin >= 0.0f;
	const int cosSign = cos >= 0.0f;
	int sinLesser;
	hkReal value;
	{
		hkReal usin = hkMath::fabs(sin);
		hkReal ucos = hkMath::fabs(cos);

		sinLesser = usin <= ucos;
		value = sinLesser ? usin : ucos;
	}
	// remember on which base the point lies
	const int baseA = axis.dot3(direction) <= 0.0f;

	// encode that info
	int code = 0;
	//
	// Cylinder agent info -- now it is important to synch the  hkPredGskCylinderAgent3 if you change the encoding of the virtual vertices.
	//
	code += baseA     << VERTEX_ID_ENCODING_IS_BASE_A_SHIFT;
	code += sinSign   << VERTEX_ID_ENCODING_SIN_SIGN_SHIFT;
	code += cosSign   << VERTEX_ID_ENCODING_COS_SIGN_SHIFT;
	code += sinLesser << VERTEX_ID_ENCODING_IS_SIN_LESSER_SHIFT;
	HK_ASSERT( 0xf02dfad4, code < 0x10000);
	// got 12 bits to store the (non-negative) value (4096 values)
	// we actually wont use the upper ~1/3 of the range now...
	HK_ASSERT2(0x3bd0155e, value >= 0 && value < 0.708f, "Value used to encode support vertex for the cylinder is negative (and it cannot be).");
#ifndef HK_USING_APPROXIMATE_FLOAT_TO_INT_CONVERSION
	const int intValue = int( value * s_virtualTesselationParameter);
#else
	assertRoundUpThreshold(s_intRoundUpThreshold);
	int intValue = hkMath::hkToIntFast( value * s_virtualTesselationParameter - s_intRoundUpThreshold + HK_CONVERSION_TOLERANCE);
#endif
	HK_ASSERT2(0x3bd0155f, intValue >= 0 && hkReal(intValue) < s_virtualTesselationParameter, "Fast float-to-int conversion returned an invalid value (in cylinder code).");
	HK_ASSERT2(0x3bd0155e, (intValue & VERTEX_ID_ENCODING_VALUE_MASK) == intValue, "The vertexId's value being encoded doesn't fit. Possible cause -- hkCylinderShape::s_virtualTesselationParameter too large (> 16).");
	code += intValue;

	// calculate the position of the encoded vertex -- that way you know exactly what value is in cache
	decodeVertexId( hkVertexId(code), supportingVertex);

	supportingVertex.setInt24W(code);

}

void hkCylinderShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	for (int i = numIds-1; i>=0; --i, ++verticesOut, ++ids)
	{
		decodeVertexId(ids[0], *verticesOut);
		verticesOut->setInt24W( ids[0] );
	}
}

void hkCylinderShape::getFirstVertex(hkVector4& v) const 
{
	v = getVertex(1);
}

int hkCylinderShape::getNumVertices() const
{
	return -1;
}

void hkCylinderShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	infoOut.m_numSpheres = 16 + 2;
	infoOut.m_useBuffer = true;
}

const hkSphere* hkCylinderShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	hkReal cylRadiusForBaseSpheres = m_cylRadius;
	hkReal largeSphereDisplacement = 0.0f;
	{
		hkVector4 symmetryAxis; symmetryAxis.setSub4( getVertex(1), getVertex(0) );
		hkReal heightSqr = symmetryAxis.lengthSquared3();
		if (heightSqr >= 2.0f * 2.0f * m_cylRadius * m_cylRadius)
		{
			// Adding large spheres for smooth rolling
		    cylRadiusForBaseSpheres = m_cylRadius * m_cylBaseRadiusFactorForHeightFieldCollisions;
			largeSphereDisplacement = m_cylRadius;
		}
	}
	

	hkVector4 diag1; diag1.setAdd4(m_perpendicular1, m_perpendicular2);
	diag1.mul4( 1.0f / 1.4142135623730950488016887242097f * cylRadiusForBaseSpheres );
	hkVector4 diag2; diag2.setSub4(m_perpendicular1, m_perpendicular2);
	diag2.mul4( 1.0f / 1.4142135623730950488016887242097f * cylRadiusForBaseSpheres);
	hkVector4 perp1; perp1.setMul4(cylRadiusForBaseSpheres, m_perpendicular1);
	hkVector4 perp2; perp2.setMul4(cylRadiusForBaseSpheres, m_perpendicular2);

	perp1.zeroElement(3);
	perp2.zeroElement(3);
	diag1.zeroElement(3);
	diag2.zeroElement(3);

	for (int cap = 0; cap < 2; ++cap)
	{
		hkVector4* s = reinterpret_cast<hkVector4*>(sphereBuffer + 8 * cap);
		hkVector4 baseCenter = getVertex(cap);
		baseCenter(3) = m_radius;

		s[0].setAdd4(baseCenter, perp1);
		s[1].setAdd4(baseCenter, diag1);
		s[2].setAdd4(baseCenter, perp2);
		s[3].setSub4(baseCenter, diag2);
		s[4].setSub4(baseCenter, perp1);
		s[5].setSub4(baseCenter, diag1);
		s[6].setSub4(baseCenter, perp2);
		s[7].setAdd4(baseCenter, diag2);
	}

	// When the cylinder is long enough, and we can squeeze a big sphere (with its radius equal to the cylinder radius) into it -- we do.
	{
		hkVector4& s0 = *reinterpret_cast<hkVector4*>(sphereBuffer + 16);
		hkVector4& s1 = *reinterpret_cast<hkVector4*>(sphereBuffer + 17);

		// put extra large/center spheres in
		hkVector4 symmetryAxisVersor; symmetryAxisVersor.setCross(m_perpendicular1, m_perpendicular2);
		HK_ON_DEBUG(hkVector4 symmetryAxis; symmetryAxis.setSub4( getVertex(1), getVertex(0) ) );
		HK_ASSERT2(0x708e02c3, symmetryAxisVersor.dot3(symmetryAxis) > 0, "Internal error: wrong axis direction.");

		s0.setAddMul4( getVertex(0), symmetryAxisVersor,  largeSphereDisplacement );
		s1.setAddMul4( getVertex(1), symmetryAxisVersor, -largeSphereDisplacement );
		s0(3) = largeSphereDisplacement + m_radius;
		s1(3) = largeSphereDisplacement + m_radius;
	}

	return sphereBuffer;
}

void hkCylinderShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	// determine how much more dist from each of the cylinder endpoints is needed along each of the axes
	hkVector4 obj[2];
	hkVector4Util::transformPoints( localToWorld, this->getVertices(), 2, obj );

	hkVector4 segment;
	segment.setSub4(obj[1], obj[0]);
	segment.fastNormalize3();
	
	segment(0) = hkMath::sqrt( 1.01f - segment(0) * segment(0) );
	segment(1) = hkMath::sqrt( 1.01f - segment(1) * segment(1) );
	segment(2) = hkMath::sqrt( 1.01f - segment(2) * segment(2) );
	segment.mul4(this->m_cylRadius);

	hkVector4 tol4; tol4.setAll3(tolerance + this->m_radius);
	tol4.add4(segment);

	out.m_min.setMin4( obj[0], obj[1] );
	out.m_min.sub4( tol4 ); 
	
	out.m_max.setMax4( obj[0], obj[1] );
	out.m_max.add4( tol4 ); 
}


hkBool hkCylinderShape::castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& output) const
{
	HK_TIMER_BEGIN("rcCylinder", HK_NULL);
	
	// We now cast rays against the expanded cyl (including the convex radius)
	// Need unit axis, quicker to cross the precomputed perps back then to normalize the vertex dif.
	hkVector4 radiusAxis; radiusAxis.setCross(m_perpendicular2, m_perpendicular1);
	radiusAxis.mul4( m_radius );
	hkVector4 vertex0; vertex0.setAdd4(getVertex(0), radiusAxis);
	hkVector4 vertex1; vertex1.setSub4(getVertex(1), radiusAxis);
	hkReal cylTotalRadius = m_radius + m_cylRadius;

	{
		// modified code of hkCapsuleShape::castRay
		hkVector4 res;
		hkCapsuleShape::closestPointLineSeg( input.m_from, vertex0, vertex1, res );

		// Ray starts inside Cylinder... reject!
		hkVector4 join; join.setSub4(input.m_from, res);
		hkReal sToCylDist = join.length3();
		if(sToCylDist < cylTotalRadius)
		{
			// extra checks for cylinder 
			hkVector4 axis; axis.setSub4(vertex1, vertex0);
			{
				hkVector4 fromBottom; fromBottom.setSub4(input.m_from, vertex0);
				hkReal    dot1 = axis.dot3(fromBottom);
				if (dot1 > 0)
				{
					hkVector4 fromTop; fromTop.setSub4(input.m_from, vertex1);
					hkReal    dot2 = axis.dot3(fromTop);
					if (dot2 < 0)
					{
						// the m_from is between both cylinder bases
						goto returnFalse;
					}
				}
			}
		}

		// Work out closest points to cylinder
		hkReal infInfDistSquared = HK_REAL_MAX;
		hkReal t, u;
		hkVector4 p,q;

		hkVector4 dA;
		dA.setSub4(input.m_to, input.m_from);
		hkVector4 dB;
		dB.setSub4(vertex1, vertex0);

		// Get distance between inf lines + parametric description (t, u) of closest points, 
		hkCapsuleShape::closestInfLineSegInfLineSeg(input.m_from, dA, vertex0, dB, infInfDistSquared, t, u, p, q);

		// Is infinite ray within radius of infinite cylinder?
		if(infInfDistSquared > cylTotalRadius * cylTotalRadius)
		{
			goto returnFalse;
		}

		hkReal axisLength;
		hkVector4 axis;
		hkReal ipT;
		{
			axis = dB;

			// Check for zero axis
			const hkReal axisLengthSqrd = axis.lengthSquared3();
			if (axisLengthSqrd > HK_REAL_EPSILON)
			{
				axisLength = axis.normalizeWithLength3();
			}
			else
			{
				// cylinder cannot have axis of length zero; unlike capsule
				goto returnFalse;
			}

			hkVector4 dir = dA;
			hkReal component = dir.dot3(axis);

			hkVector4 flatDir;
			flatDir.setAddMul4(dir, axis, -component);

			// Flatdir is now a ray firing in the "plane" of the cyliner. 
			hkReal d;
			d = hkMath::sqrt(cylTotalRadius * cylTotalRadius - infInfDistSquared);

			// Convert d to a parameterisation instead of absolute distance along ray
			d *= flatDir.lengthInverse3();

			// This represents a parameterisation along the ray of the intersection point
			ipT = t - d;
		}

		// Intersection parameterization with infinite cylinder is outside length of ray
		if( ipT >= output.m_hitFraction )
		{
			goto returnFalse;
		}

		hkReal pointAProj = vertex0.dot3(axis);

		// Find intersection point of actual ray with infinite cylinder
		hkVector4 intersectPt;
		intersectPt.setInterpolate4( input.m_from, input.m_to, ipT );

		// Test height of intersection point w.r.t. cylinder axis 
		// to determine hit against actual cylinder
		// Intersection point projected against cylinder
		const hkReal ptProj = intersectPt.dot3(axis);
		hkReal ptHeight = ptProj - pointAProj;

		if( ipT >= 0 ) // Actual ray (not infinite ray) must intersect with infinite cylinder
		{
			if(ptHeight > 0 && ptHeight < axisLength) // Report hit against cylinder part
			{
				// Calculate normal
				hkVector4 projPtOnAxis;
				projPtOnAxis.setInterpolate4( vertex0, vertex1, ptHeight / axisLength );
				hkVector4 normal;	normal.setSub4( intersectPt, projPtOnAxis );

				normal.normalize3();
				output.m_normal = normal;
				output.m_hitFraction = ipT; // This is a parameterization along the ray
				output.setKey( HK_INVALID_SHAPE_KEY );
				HK_TIMER_END();
				return true;
			}
		}

		// Cylinder base/cap tests

		{
			// the ray may only hit one cap so..

			// use: 
			//  - pointAProj
			//  - axisLength
			const hkReal fromPointProj = input.m_from.dot3(axis);
			hkReal axisSign;
			hkVector4 capCenter;
			if (fromPointProj < pointAProj)
			{
				// test for A cap only
				axisSign = -1.0f;
				capCenter = vertex0;
			}
			else if (fromPointProj > pointAProj + axisLength)
			{
				// test for A cap only
				axisSign = 1.0f;
				capCenter = vertex1;
			}
			else
			{
				// it is a miss
				goto returnFalse;
			}

			hkReal hitFract;
			{
				hkVector4 toCap; toCap.setSub4(capCenter, input.m_from);
				hitFract = static_cast<hkReal>(toCap.dot3(axis)) * axisSign * (-1.0f);
				if ( hitFract < 0)
				{
					goto returnFalse;
				}
				hkReal div = axisSign * (-1.0f) * static_cast<hkReal>(dA.dot3(axis));
				if ( hitFract >= div * output.m_hitFraction)
				{
					goto returnFalse;
				}
				hitFract /= div;
			}

			hkVector4 hitOnPlane;	hitOnPlane.setInterpolate4(input.m_from, input.m_to, hitFract);

			hkReal dist2;
			{
				hkVector4 diff; diff.setSub4(hitOnPlane, capCenter);
				dist2 = diff.lengthSquared3();
			}

			if (dist2 > cylTotalRadius * cylTotalRadius)
			{
				goto returnFalse;
			}

			output.m_normal.setMul4( axisSign, axis );
			output.m_hitFraction = hitFract;
			output.setKey( HK_INVALID_SHAPE_KEY );
			HK_TIMER_END();
			return true;
		}
	}
returnFalse:
	HK_TIMER_END();
	return false;

}

hkShapeType hkCylinderShape::getType() const
{ 
	return HK_SHAPE_CYLINDER; 
}

void hkCylinderShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("Cylinder", collector->MEMORY_SHARED, this);
	collector->endObject();
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
