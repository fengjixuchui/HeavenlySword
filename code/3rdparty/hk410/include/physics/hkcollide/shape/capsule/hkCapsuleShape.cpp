/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkCapsuleShape);

// assert to verify the correctness of getVertices()
HK_COMPILE_TIME_ASSERT( sizeof(hkVector4) == sizeof(hkSphere) );



hkCapsuleShape::hkCapsuleShape( const hkVector4& vertexA, const hkVector4& vertexB, hkReal radius)
	: hkConvexShape(radius)
{
	m_vertexA = vertexA;
	m_vertexB = vertexB;
	m_vertexA(3) = radius;
	m_vertexB(3) = radius;
#ifdef HK_DEBUG
	hkVector4 diff; diff.setSub4( vertexB, vertexA );
	HK_ASSERT2( 0xf010345, hkReal(diff.length3()) != 0.0f, "You are not allowed to create a capsule with identical vertices");
#endif
}


void hkCapsuleShape::getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const
{
	// direction is already in "this" space, so:

	hkVector4 diff; diff.setSub4( m_vertexB, m_vertexA );
	hkReal dot1 = diff.dot3(direction);

	int vertexId;
	if(dot1 < 0.0f)
	{
		static_cast<hkVector4&>(supportingVertex) = m_vertexA;
		vertexId = 0;
	}
	else
	{
		static_cast<hkVector4&>(supportingVertex) = m_vertexB;
		vertexId = hkSizeOf(hkVector4);
	}
	supportingVertex.setInt24W( vertexId );
}

void hkCapsuleShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	for (int i = numIds-1; i>=0; i--)
	{
		static_cast<hkVector4&>(verticesOut[0]) =  *hkAddByteOffsetConst<hkVector4>( getVertices(), ids[0] );	// do a quick address calculation
		verticesOut[0].setInt24W( ids[0] );
		verticesOut++;
		ids++;
	}
}

void hkCapsuleShape::getFirstVertex(hkVector4& v) const 
{
	v = getVertex(1);
}

int hkCapsuleShape::getNumVertices() const
{
	return 2;
}

void hkCapsuleShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	infoOut.m_numSpheres = 2;
	infoOut.m_useBuffer = true;
}

const hkSphere* hkCapsuleShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	sphereBuffer[0].setPositionAndRadius( m_vertexA );
	sphereBuffer[1].setPositionAndRadius( m_vertexB );
	return &sphereBuffer[0];
}



void hkCapsuleShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkVector4 tol4; tol4.setAll3(tolerance + m_radius);

	hkVector4 obj[2];
	hkVector4Util::transformPoints( localToWorld, this->getVertices(), 2, obj );

	out.m_min.setMin4( obj[0], obj[1] );
	out.m_min.sub4( tol4 ); 
	
	out.m_max.setMax4( obj[0], obj[1] );
	out.m_max.add4( tol4 ); 
}


void hkCapsuleShape::closestInfLineSegInfLineSeg( const hkVector4& A, const hkVector4& dA, const hkVector4& B, const hkVector4& dB, hkReal& distSquared, hkReal& t, hkReal &u, hkVector4& p, hkVector4& q ) 
{
	hkVector4 d12; d12.setSub4(B,A);
	hkReal R=dA.dot3(dB);

	hkReal S1=dA.dot3(d12);
	hkReal S2=dB.dot3(d12);

	hkReal D1=dA.dot3(dA);
	hkReal D2=dB.dot3(dB);
	//HK_ASSERT2(0x13635242,  D1 != 0.0f, "Length of segment A is zero");
	//HK_ASSERT2(0x494dab88,  D2 != 0.0f, "Length of segment B is zero");


	// Step 1, compute D1, D2, R and the denominator.
	// The cases (a), (b) and (c) are covered in Steps 2, 3 and 4 as
	// checks for division by zero.

	hkReal denom = D1*D2-R*R;  
	hkReal eps = (hkMath::fabs(D1*D2) + R*R) * (HK_REAL_EPSILON * 8.0f);

	if( hkMath::fabs(denom) > eps )
	{
		t=(S1*D2-S2*R)/denom;
	}
	else
	{
		t = 0;
	}
	
	// Step 2
	u=(t*R-S2)/D2;

	hkVector4 a; a = A; a.addMul4( t, dA );
	hkVector4 b; b = B; b.addMul4( u, dB );
	hkVector4 diff; diff.setSub4( a,b );

	p = a;
	q = b;

	distSquared = diff.lengthSquared3();
}

void hkCapsuleShape::closestPointLineSeg( const hkVector4& A, const hkVector4& B, const hkVector4& B2, hkVector4& pt )
{
	hkVector4 d12; d12.setSub4( B, A );
	hkVector4 dB;  dB.setSub4( B2, B );

	hkReal S2 = dB.dot3(d12);
	hkReal D2 = dB.dot3(dB);

	HK_ASSERT2(0x58206027,  D2 != 0.0f, "Length of segment B is zero");

	hkReal u = -S2;

	// If u not in range, modify
	if(u<=0)
	{
		pt = B;
		return;
	}
	else
	{
		if(u>=D2)
		{
			pt = B2;
			return;
		}
		else
		{
			u /= D2;
			pt.setAddMul4( B, dB, u );
			return;
		}
	}
}

hkBool hkCapsuleShape::castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& output) const
{
	HK_TIMER_BEGIN("rcCapsule", HK_NULL);
	{
	    hkVector4 res;
	    closestPointLineSeg( input.m_from, getVertex(0), getVertex(1), res );
    
	    // Ray starts inside capsule... reject!
	    hkVector4 join; join.setSub4(input.m_from, res);
	    hkReal sToCylDist = join.length3();
	    if(sToCylDist < m_radius)
	    {
		    goto returnFalse;
	    }

		// Work out closest points to cylinder
		hkReal infInfDistSquared = HK_REAL_MAX;
		hkReal t, u;
		hkVector4 p,q;

		hkVector4 dA;
		dA.setSub4(input.m_to, input.m_from);
		hkVector4 dB;
		dB.setSub4(getVertex(1), getVertex(0));

		// Get distance between inf lines + parametric description (t, u) of closest points, 
		closestInfLineSegInfLineSeg(input.m_from, dA, getVertex(0), dB, infInfDistSquared, t, u, p, q);


		// Is infinite ray within radius of infinite cylinder?
		if(infInfDistSquared > m_radius * m_radius)
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
				axisLength = 0.0f;
				axis.setZero4();
			}

			hkVector4 dir = dA;
			hkReal component = dir.dot3(axis);

			hkVector4 flatDir;
			flatDir.setAddMul4(dir, axis, -component);

			// Flatdir is now a ray firing in the "plane" of the cyliner. 
			hkReal d;
			d = hkMath::sqrt(m_radius * m_radius - infInfDistSquared);

			// Convert d to a parameterisation instead of absolute distance along ray
			d *= flatDir.lengthInverse3();

			// This represents a parameterisation along the ray of the intersection point
			ipT = t - d;
		}

		// Intersection parameterization with infinite cylinder is outside length of ray
		// or is greater than a previous hit fraction
		if( ipT >= output.m_hitFraction )
		{
			goto returnFalse;
		}

		hkReal ptHeight;
		hkReal pointAProj = getVertex(0).dot3(axis);

		// Find intersection point of actual ray with infinite cylinder
		hkVector4 intersectPt;
		intersectPt.setInterpolate4( input.m_from, input.m_to, ipT );

		// Test height of intersection point w.r.t. cylinder axis 
		// to determine hit against actual cylinder
		// Intersection point projected against cylinder
		const hkReal ptProj = intersectPt.dot3(axis);
		ptHeight = ptProj - pointAProj;

		if( ipT >= 0 ) // Actual ray (not infinite ray) must intersect with infinite cylinder
		{
			if(ptHeight >0 && ptHeight < axisLength) // Report hit against cylinder part
			{
				// Calculate normal
				hkVector4 projPtOnAxis;
				projPtOnAxis.setInterpolate4( getVertex(0), getVertex(1), ptHeight / axisLength );
				hkVector4 normal;	normal.setSub4( intersectPt, projPtOnAxis );

				normal.normalize3();
				output.m_normal = normal;
				output.m_hitFraction = ipT; // This is a parameterization along the ray
				output.m_extraInfo = HIT_BODY;
				output.setKey( HK_INVALID_SHAPE_KEY );
				HK_TIMER_END();
				return true;
			}
		}

		// Cap tests

		
		{
			// Check whether start point is inside infinite cylinder or not
			hkReal fromLocalProj = input.m_from.dot3(axis);
			hkReal projParam = fromLocalProj - pointAProj;

			hkVector4 fromPtProjAxis;
			fromPtProjAxis.setInterpolate4( getVertex(0), getVertex(1), projParam / axisLength );

			hkVector4 axisToRayStart;
			axisToRayStart.setSub4(input.m_from, fromPtProjAxis);

			if((axisToRayStart.length3() > m_radius) && (ipT < 0))
			{
 				// Ray starts outside infinite cylinder and points away... must be no hit
				goto returnFalse;
			}

			// Ray can only hit 1 cap... Use intersection point
			// to determine which sphere to test against (NB: this only works because
			// we have discarded cases where ray starts inside)
			hkShapeRayCastInput subInput;
			int extra = (ptHeight <= 0) ? HIT_CAP0 : HIT_CAP1;
			subInput.m_from.setSub4( input.m_from, getVertex(extra) );
			subInput.m_to.setSub4( input.m_to, getVertex(extra) );
			
			hkSphereShape point( m_radius );
			HK_TIMER_END();
			if( point.castRay( subInput, output) )
			{
				output.m_extraInfo = extra;
				return true;
			}
			return false;
		}
	}
returnFalse:
	HK_TIMER_END();
	return false;
}

hkShapeType hkCapsuleShape::getType() const
{ 
	return HK_SHAPE_CAPSULE; 
}

void hkCapsuleShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("CapsuleShape", collector->MEMORY_SHARED, this);
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
