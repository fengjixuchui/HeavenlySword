/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>

#include <hkcollide/util/hkAabbUtil.h>
#include <hkbase/class/hkTypeInfo.h>

#if defined HK_COMPILER_MSVC
	// C4701: local variable 'lastNormal' and 'hitNormal' may be used without having been initialized
#	pragma warning(disable: 4701)
#endif

HK_REFLECTION_DEFINE_VIRTUAL(hkConvexVerticesShape);

hkConvexVerticesShape::hkConvexVerticesShape(hkStridedVertices vertsIn, const hkArray<hkVector4>& planeEquations, hkReal radius)
:	hkConvexShape( radius )
{
	HK_ASSERT2(0x393378da, vertsIn.m_striding % hkSizeOf(float) == 0, "vertsIn.m_striding is not a multiple of hkSizeOf(float).");

#ifdef HK_DEBUG
	{
		// Check validity of planes and vertices
		hkBool shapeOk = true;
		for (int v = 0; v < vertsIn.m_numVertices; v++)
		{
			hkVector4 vertex;
			const float* vertexData = hkAddByteOffsetConst( vertsIn.m_vertices, vertsIn.m_striding * v );
			vertex.set( vertexData[0], vertexData[1], vertexData[2] );

			for (int p = 0; p < planeEquations.getSize(); p++)
			{
				hkReal distFromPlane = hkReal( vertex.dot3(planeEquations[p]) ) + planeEquations[p](3);
				if (distFromPlane > 0.1f)
				{
					shapeOk = false;
					break;
				}
			}
		}
		if (!shapeOk)
		{
			HK_WARN(0xad876bb3, "Vertices or planes are invalid");
		}
	}
#endif

	m_planeEquations = planeEquations;

	// copy all the data
	copyVertexData(vertsIn.m_vertices,vertsIn.m_striding,vertsIn.m_numVertices);
}

hkConvexVerticesShape::hkConvexVerticesShape(
		FourVectors* rotatedVertices, int numVertices,
		hkVector4* planes, int numPlanes,
		const hkAabb& aabb, hkReal radius )
	:	hkConvexShape(radius),
		m_rotatedVertices( rotatedVertices, HK_NEXT_MULTIPLE_OF(4, numVertices)/4, HK_NEXT_MULTIPLE_OF(4, numVertices)/4 ),
		m_numVertices(numVertices),
		m_planeEquations(planes, numPlanes, numPlanes)
{
#ifdef HK_DEBUG
	{
		// Check validity of planes and vertices
		hkBool shapeOk = true;
		int checkedVertices = 0;
		for (int v = 0; v < m_rotatedVertices.getSize(); v++)
		{
			for (int rv = 0; rv < 4 && checkedVertices < m_numVertices; rv++, checkedVertices++)
			{
				hkReal* realPtr = reinterpret_cast<hkReal*>(&m_rotatedVertices[v].m_x) + rv;
				hkVector4 originalVertex;
				originalVertex.set( realPtr[0], realPtr[4], realPtr[8] );

				for (int p = 0; p < m_planeEquations.getSize(); p++)
				{
					hkReal distFromPlane = (hkReal)( originalVertex.dot3(m_planeEquations[p]) + hkSimdReal(m_planeEquations[p](3)));
					if (distFromPlane > 0.1f)
					{
						shapeOk = false;
						break;
					}
				}
			}
		}
		if (!shapeOk)
		{
			HK_WARN(0xad876bb3, "Vertices or planes are invalid");
		}
	}
#endif

	m_aabbHalfExtents.setSub4( aabb.m_max, aabb.m_min );
	m_aabbHalfExtents.mul4( 0.5f ); 
	m_aabbCenter.setAdd4( aabb.m_min, aabb.m_max );
	m_aabbCenter.mul4( 0.5f );
}

void hkConvexVerticesShape::getOriginalVertices( hkArray<hkVector4>& vertices ) const
{
	// set the vertex array to the size of the original num of verts passed in
	vertices.setSize( m_numVertices );

	// start at zero but skip the last few rows (possibly) because 
	// the last few rows are probably padded with the same vertex
	for (int i = 0; i < m_numVertices; i++)
	{

		int iBlock = i / 4;
		int iColumn = i & 3;
		// take the value from each column and add back in as a row
		{
			vertices[i].set( m_rotatedVertices[iBlock].m_x(iColumn), m_rotatedVertices[iBlock].m_y(iColumn), m_rotatedVertices[iBlock].m_z(iColumn) );
		}
	}
}

hkShapeType hkConvexVerticesShape::getType() const
{
	return HK_SHAPE_CONVEX_VERTICES;
}


void hkConvexVerticesShape::copyVertexData(const float* vertexIn, int byteStriding, int numVertices)
{
	HK_ASSERT2(0x601f8f0d, numVertices > 0, "numVertices <=0! must be > 0");

	m_numVertices = numVertices;

	int paddedSize = HK_NEXT_MULTIPLE_OF(4, numVertices);

	m_rotatedVertices.setSize( paddedSize / 4 );
	
	const float* v = vertexIn;
	int i;
	for ( i = 0; i < numVertices; i++ )
	{
		int a = i / 4;
		int b = i & 3;
		m_rotatedVertices[a].m_x(b) = v[0];
		m_rotatedVertices[a].m_y(b) = v[1];
		m_rotatedVertices[a].m_z(b) = v[2];
		v = hkAddByteOffset( const_cast<float*>(v), byteStriding );
	};

	// use the last point to fill the rest vertices
	v = hkAddByteOffset( const_cast<float*>(v), -byteStriding );

	for ( ; i < paddedSize; i++ )
	{
		int a = i >> 2;
		int b = i & 3;
		m_rotatedVertices[a].m_x(b) = v[0];
		m_rotatedVertices[a].m_y(b) = v[1];
		m_rotatedVertices[a].m_z(b) = v[2];
	}

	hkAabb aabb;
	hkAabbUtil::calcAabb( vertexIn, numVertices, byteStriding, aabb );

	m_aabbCenter.setAdd4( aabb.m_min, aabb.m_max );
	m_aabbCenter.mul4( 0.5f );

	m_aabbHalfExtents.setSub4( aabb.m_max, aabb.m_min );
	m_aabbHalfExtents.mul4( 0.5f );
}


void hkConvexVerticesShape::getFirstVertex(hkVector4& v) const
{
	v(0) = m_rotatedVertices[0].m_x(0);
	v(1) = m_rotatedVertices[0].m_y(0);
	v(2) = m_rotatedVertices[0].m_z(0);
	v(3) = 0;
}

void hkConvexVerticesShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	infoOut.m_numSpheres = m_numVertices;
	infoOut.m_useBuffer = true;
}

const hkSphere* hkConvexVerticesShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	hkSphere* s = sphereBuffer;
	const FourVectors* fv = &m_rotatedVertices[0];
	int i = m_numVertices - 1;
	for ( ; i >=3 ; i-=4)
	{	
		hkVector4 x;
		x.set(fv->m_x(0), fv->m_y(0), fv->m_z(0), m_radius);	(*s++).setPositionAndRadius(x);
		x.set(fv->m_x(1), fv->m_y(1), fv->m_z(1), m_radius);	(*s++).setPositionAndRadius(x);
		x.set(fv->m_x(2), fv->m_y(2), fv->m_z(2), m_radius);	(*s++).setPositionAndRadius(x);
		x.set(fv->m_x(3), fv->m_y(3), fv->m_z(3), m_radius);	(*s++).setPositionAndRadius(x);
		fv++;
	}

	if ( i >=0 ){	hkVector4 x; x.set(fv->m_x(0), fv->m_y(0), fv->m_z(0), m_radius);	(*s++).setPositionAndRadius(x); }
	if ( i >=1 ){	hkVector4 x; x.set(fv->m_x(1), fv->m_y(1), fv->m_z(1), m_radius);	(*s++).setPositionAndRadius(x); }
	if ( i >=2 ){	hkVector4 x; x.set(fv->m_x(2), fv->m_y(2), fv->m_z(2), m_radius);	(*s++).setPositionAndRadius(x); }

	return sphereBuffer;
}


void hkConvexVerticesShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkAabbUtil::calcAabb( localToWorld, m_aabbHalfExtents, m_aabbCenter, tolerance + m_radius,  out );
}



void hkConvexVerticesShape::getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const
#if defined( HK_PS2) && (HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED)
{
	//HK_INTERNAL_TIMER_BEGIN( "support" );
	{
		// calculate the dot product for four vertices
		union
		{
			hkUint128 indices128;
			int    indices32[4]; 
		} indexUnion;

		hkVector4 bestDots;

		const FourVectors* begin = &m_rotatedVertices[0];
		const FourVectors* end =   begin + m_rotatedVertices.getSize();

		HK_COMPILE_TIME_ASSERT( sizeof( FourVectors ) == 0x30 );
		asm(
			// register usage:	$8 = const 1,1,1,1
			//					$9 = const -1,-1,-1,-1
			//					$10 = current indices
			//					$11 = used for select statement
			//					$12 pointer to input data
			//					%0 indices out
			//					%1 bestDots out
			//					%2 pointer to input data
			//					%3 direction
			//					%4 end of input data
			//					vf8-vf10 xyz

			"		.set noreorder\n"	\
			"		lqc2		vf8, 0x00(%2)   # load x					\n"	\
			"		lqc2		vf9, 0x10(%2)	# load m_y					\n"	\
			"		lqc2		vf10, 0x20(%2)  # load m_z					\n"	\
			"		addiu		$8, $0, 1		# set $8 to 1				\n"	\
			"		pextlw      $8, $8, $8		# broadcast 1				\n"	\
			"       move        $12, %2			# move to increment register\n"	\
			"		pextlw		$8, $8, $8		# broadcast 1				\n"	\
			"		vmulax		ACC, vf8, %3	# x * dir(0)				\n"	\
			"		vmadday		ACC, vf9, %3	# + m_y * dir(1)				\n"	\
			"		vmaddz		%1, vf10, %3    # %1 = ... + m_z * dir(2)	\n"	\
			"		padduw		$10, $0, $0		# initilize indices counter	\n"	\
			"		padduw		%0,  $0, $0		# initilize indices out		\n"	\
			"		addiu		$12, $12, 0x30	# increment pointer			\n"	\
			"		beq			$12, %4, 1f		# jump to end				\n"	\
			"		psubw		$9, $0, $8		# $9 = 0xfffffffffffff		\n"	\
			"	0:	lqc2		vf8, 0x00($12)   # load x					\n"	\
			"		lqc2		vf9, 0x10($12)	# load m_y					\n"	\
			"		lqc2		vf10, 0x20($12)  # load m_z					\n"	\
			"		vmulax		ACC, vf8, %3	# x * dir(0)				\n"	\
			"		vmadday		ACC, vf9, %3	# + m_y * dir(1)				\n"	\
			"		vmaddz		vf8, vf10, %3   # vf8 = ... + m_z * dir(2)	\n"	\
	 		"		padduw		$10, $10, $8	# increment indices			\n"	\
			"		addiu		$12, $12, 0x30	# increment pointer			\n"	\
			"		vsub		vf9, vf8, %1	# compare against best current solution \n" \
			"		vmax		%1,  %1, vf8	# find the best dots		\n"	\
			"		qmfc2		$11, vf9		# get sign bits into 10		\n"	\
			"		psraw		$11, $11, 31	# extend the sign bits		\n"	\
			"		pand		%0,	 $11, %0	# select old indices		\n"	\
			"		pxor		$11, $11, $9	# not sign bits				\n"	\
			"		pand		$11, $11, $10	# indices					\n"	\
			"		bne			$12, %4, 0b		# jump to end				\n"	\
			"		por			%0,  %0, $11	# or results				\n"	\
			"		.set reorder\n"	\
			"   1:		\n"	
			: "=&r"(indexUnion.indices128) , "=&j"(bestDots.getQuad())	// 0, 1
			: "r" ( begin ), "j"( direction.getQuad()), "r" (end)		// 3 4
			: "vf8", "vf9", "vf10", "$8", "$9", "$10", "$11", "$12"
		);

		int vertexId = indexUnion.indices32[0]*4;
		float best = bestDots(0);

		if ( bestDots(1) > best)
		{
			best = bestDots(1);
			vertexId = indexUnion.indices32[1]*4 + 1;
		}
		if ( bestDots(2) > best)
		{
			best = bestDots(2);
			vertexId = indexUnion.indices32[2]*4 + 2;
		}
		if ( bestDots(3) > best)
		{
			best = bestDots(3);
			vertexId = indexUnion.indices32[3]*4 + 3;
		}
		{
			const FourVectors* 	fv = &m_rotatedVertices[vertexId>>2];
			int a = vertexId & 3;
			supportingVertex(0) = fv->m_x(a);
			supportingVertex(1) = fv->m_y(a);
			supportingVertex(2) = fv->m_z(a);
			supportingVertex.setInt24W( vertexId );
		}
		//HK_INTERNAL_TIMER_END();
	}
}
#elif HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
{
	// this version is slightly slower than the next for fpu pc builds.
	//HK_INTERNAL_TIMER_BEGIN( "support", HK_NULL );
	const hkReal negMax = -HK_REAL_MAX;
	const hkQuadReal initialDot = HK_QUADREAL_CONSTANT( negMax, negMax, negMax, negMax );
	hkVector4 bestDot; bestDot = initialDot; 

	const FourVectors* fv = &m_rotatedVertices[0];
	int maxNum = m_rotatedVertices.getSize() << 2;
	int bestIndices[4];
	HK_ASSERT( 0x4c5c7d57, maxNum > 0 ); // must have some elements or bestIndices is uninitialized

	// get max dots four at a time
	for ( int i = 0; i < maxNum; fv++, i+=4 )
	{
		hkVector4 dot;

		// calculate the dot product for four vertices
		{
			hkVector4 d0; d0.setBroadcast(direction, 0);
			hkVector4 d1; d1.setBroadcast(direction, 1);
			hkVector4 d2; d2.setBroadcast(direction, 2);
			hkVector4 x; x.setMul4( d0, fv->m_x );
			hkVector4 y; y.setMul4( d1, fv->m_y );
			dot.setAdd4( x,y );
			hkVector4 z; z.setMul4( d2, fv->m_z );
			dot.add4( z );
		}

		int betterMask = bestDot.compareLessThan4( dot );
		//all values are less then current best 'signed distance'
		if ( betterMask == 0 ) 
		{
			continue;
		}
		// some better
		bestDot.setMax4( bestDot, dot );
		if( betterMask & HK_VECTOR3_COMPARE_MASK_X ) bestIndices[0] = i;
		if( betterMask & HK_VECTOR3_COMPARE_MASK_Y ) bestIndices[1] = i;
		if( betterMask & HK_VECTOR3_COMPARE_MASK_Z ) bestIndices[2] = i;
		if( betterMask & HK_VECTOR3_COMPARE_MASK_W ) bestIndices[3] = i;
	}

	// find the best of the 4 we have
	int bestIndex4;
	{
		int i01 = bestDot(0) > bestDot(1) ? 0 : 1;
		int i23 = bestDot(2) > bestDot(3) ? 2 : 3;
		bestIndex4 = bestDot(i01) > bestDot(i23) ? i01 : i23;
	}

	// extract vertex
	int vertexId = bestIndices[ bestIndex4 ] + bestIndex4;
	{
		fv = &m_rotatedVertices[unsigned(vertexId)>>2];
		int a = vertexId & 3;
		supportingVertex(0) = fv->m_x(a);
		supportingVertex(1) = fv->m_y(a);
		supportingVertex(2) = fv->m_z(a);
		supportingVertex.setInt24W( vertexId );
	}
	//HK_INTERNAL_TIMER_END();
}
#else
{
	//HK_INTERNAL_TIMER_BEGIN( "support", HK_NULL );
	const FourVectors* fv;

	hkVector4 bestDot; 

	int vertexId = 0;

//		// initialize the best distance based on the old dist
//	if (1){
//		vertexId = vertexIdCache;
//		HK_ASSERT2(0x516a1efb,  vertexId>=0 && vertexId < m_rotatedVertices.getSize() * 4, "vertexIdCache invalid (try 0)");
//		fv = &m_rotatedVertices[vertexId>>2];
//		int a = vertexId & 3;
//
//		hkReal dot = fv->m_x(a) * direction(0) + fv->m_y(a) * direction(1) + fv->m_z(a) * direction(2);
//		bestDot.setAll(dot);
//	}
//	else
	{
		const hkReal m = -HK_REAL_MAX;
		bestDot.setAll(m);
	}


	fv = &m_rotatedVertices[0];
	int maxNum = m_rotatedVertices.getSize() << 2;
	for ( int i = 0; i < maxNum; fv++, i+=4 )
	{
		hkVector4 dot;

		// calculate the dot product for four vertices
		{
			hkVector4 x; x.setMul4( direction(0), fv->m_x );
			hkVector4 y; y.setMul4( direction(1), fv->m_y );
			dot.setAdd4( x,y );
			hkVector4 z; z.setMul4( direction(2), fv->m_z );
			dot.setAdd4( dot,z );
		}

		int flags = bestDot.compareLessThan4( dot );
		//all values are less then current best 'signed distance'
		if ( !flags) 
		{
			continue;
		}
		hkReal d;
		switch (flags)
		{
#define TWO_CASE(a,b) if ( dot(a) > dot(b) ) { d = dot(a); vertexId = i + a; } else { d = dot(b); vertexId = i + b; } break;
#define THREE_CASE(a,b,c) if ( dot(a) > dot(b) ) { TWO_CASE(a, c); }else{ TWO_CASE( b, c ) };

			case HK_VECTOR3_COMPARE_MASK_NONE:
			case HK_VECTOR3_COMPARE_MASK_X:
				d = dot(0);
				vertexId = i;
				break;
			case HK_VECTOR3_COMPARE_MASK_Y:
				d = dot(1);
				vertexId = i + 1;
				break;
			case HK_VECTOR3_COMPARE_MASK_Z:
				d = dot(2);
				vertexId = i + 2;
				break;
			case HK_VECTOR3_COMPARE_MASK_W:
				d = dot(3);
				vertexId = i + 3;
				break;
/*
			// save code space by handling these cases in default:

			case HK_VECTOR3_COMPARE_MASK_XY:
				TWO_CASE(0,1);
			case HK_VECTOR3_COMPARE_MASK_XZ:
				TWO_CASE(0,2);
			case HK_VECTOR3_COMPARE_MASK_XW:
				TWO_CASE(0,3);
			case HK_VECTOR3_COMPARE_MASK_YZ:
				TWO_CASE(1,2);
			case HK_VECTOR3_COMPARE_MASK_YW:
				TWO_CASE(1,3);
			case HK_VECTOR3_COMPARE_MASK_ZW:
				TWO_CASE(2,3);
			case HK_VECTOR3_COMPARE_MASK_XYZ:
				THREE_CASE(0,1,2);
			case HK_VECTOR3_COMPARE_MASK_XYW:
				THREE_CASE(0,1,3);
*/
			case HK_VECTOR3_COMPARE_MASK_XZW:
threeCase023:
				THREE_CASE(0,2,3);
			case HK_VECTOR3_COMPARE_MASK_YZW:
threeCase123:
				THREE_CASE(1,2,3);

			default:
			case HK_VECTOR3_COMPARE_MASK_XYZW:
				if ( dot(0) > dot(1) )
				{
					goto threeCase023;
				}
				else
				{
					goto threeCase123;
				}
		}
		bestDot.setAll(d);
	}
	{
		fv = &m_rotatedVertices[unsigned(vertexId)>>2];
		int a = vertexId & 3;
		supportingVertex(0) = fv->m_x(a);
		supportingVertex(1) = fv->m_y(a);
		supportingVertex(2) = fv->m_z(a);
		supportingVertex.setInt24W( vertexId );
	}
	//HK_INTERNAL_TIMER_END();
}
#endif

void hkConvexVerticesShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	for (int i = numIds-1; i>=0; i--)
	{
		int vertexId = ids[0];
		hkVector4& v= verticesOut[0];

		const FourVectors* fv = &m_rotatedVertices[vertexId>>2];
		int a = vertexId & 3;
		v(0) = fv->m_x(a);
		v(1) = fv->m_y(a);
		v(2) = fv->m_z(a);
		v.setInt24W( vertexId );
		verticesOut++;
		ids++;
	}
}



// Boundary coordinate sign bit meanings for the "AND" of the 'outcodes'
// sign (b)	sign (t)
// 0		0		whole segment inside box
// 1		0		b is outside box, t is in
// 0		1		b is inside box, t is out
// 1		1		whole segment outside box

// ray-convex (and ray-box) intersection with ideas from 'A trip down the graphics pipeline', Jim Blinn
// if ray starts within the box, no intersection is returned
// return 1 for success, 0 for no intersection. when success then hitpoint and hitNormal is filled

hkBool hkConvexVerticesShape::castRay(const hkShapeRayCastInput& input,hkShapeRayCastOutput& results) const
{
	HK_TIMER_BEGIN("rcConvexVert", HK_NULL);
	// compute outcodes for begin and endpoint of the ray against every plane
	//	logical AND of the outcodes must be 0, if not, then there exists a face for which
	//	both the begin and endpoint of the ray is outside
	
	if( m_planeEquations.getSize() == 0 )
	{
		HK_WARN(0x530ccd4f, "You are trying to raycast against a convex vertices shape with no plane equations. Raycasting will always return no hit in this case.");
	}

	hkReal enteringLambda = -1.f;
	hkReal exitingLambda = results.m_hitFraction;
	hkVector4 lastNormal;
	for ( int i = m_planeEquations.getSize() - 1; i >=0 ;i--)
	{
		// todo: optimized for simd after testing if the algorithm actually works
		const hkVector4& normal = m_planeEquations[i];
		const hkReal startDistToPlane = normal.dot4xyz1( input.m_from);
		const hkReal endDistToPlane   = normal.dot4xyz1( input.m_to  );
		
		if ( startDistToPlane >= 0.f )
		{
			if ( endDistToPlane >= 0.f )
			{
				HK_TIMER_END();
				return false;
			}
			else
			{
				// calculate the intersection lambda between ray and the current plane
				hkReal lambda = startDistToPlane / (startDistToPlane - endDistToPlane);

				if (enteringLambda <= lambda)
				{
					enteringLambda = lambda;
					lastNormal = normal;
				}
			}
		}
		else
		{
			if ( endDistToPlane >= 0.f )
			{
				// calculate the intersection lambda between ray and the current plane
				hkReal lambda = startDistToPlane / (startDistToPlane - endDistToPlane);
				exitingLambda = hkMath::min2(exitingLambda,lambda);
			}
			else
			{
				continue;
			}
		}

		// if the ray is intersecting a backface earlier then a frontfacing plane, 
		// see picture Blinn page 128 or 'Solid thesis' van den Bergen page 41
		if  (exitingLambda < enteringLambda)
		{
			HK_TIMER_END();
			return false;
		}
	}
	
	//
	// If start point is inside, return no hit
	//
	if ( enteringLambda < 0.f )
	{
		HK_TIMER_END();
		return false;
	}

	//the fact that we reached this point, means that there is a solution
	results.m_normal = lastNormal;
	results.m_hitFraction = enteringLambda;
	results.setKey( HK_INVALID_SHAPE_KEY );

	HK_TIMER_END();
	return true;
}

const hkArray<hkVector4>& hkConvexVerticesShape::getPlaneEquations() const
{
	return m_planeEquations;
}

void hkConvexVerticesShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("CvxVerts", collector->MEMORY_SHARED, this);
	collector->addArray( "Verts", collector->MEMORY_SHARED, m_rotatedVertices );
	collector->addArray( "Verts", collector->MEMORY_SHARED, m_planeEquations );
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
