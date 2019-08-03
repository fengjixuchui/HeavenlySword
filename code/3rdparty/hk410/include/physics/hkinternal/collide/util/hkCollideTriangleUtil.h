/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLIDE_TRIANGLEUTIL_H
#define HK_COLLIDE2_COLLIDE_TRIANGLEUTIL_H

/// Triangle utilities, including ray intersection, normal calculation, and point-in-triangle tests. 
/// These methods are used internally by the engine. You should use the hkShape, and hkCollisionAgent interfaces instead
class hkCollideTriangleUtil
{
	public:
		/// calculate the barycentric coordinates of a point projected onto a triangle.
		/// Note: result 0 and 2 are always sign correct, result 1 is calculated as 1.0f - p0 - p2, this function is not epsilon safe.
		static void HK_CALL calcBarycentricCoordinates(const hkVector4& pos, const hkVector4& t0, const hkVector4& t1, const hkVector4& t2, hkReal result[3]);


		//
		//	EDGE EDGE
		//

		enum 
		{		
			CLSLS_POINTA_START = 1,		
			CLSLS_POINTA_END   = 2,		
			CLSLS_POINTB_START = 4,		
			CLSLS_POINTB_END   = 8		
		};

		struct ClosestLineSegLineSegResult
		{
			hkVector4 m_closestPointA;
			hkVector4 m_closestAminusClosestB;
			hkReal	  m_distanceSquared;
		};

		/// returns (see enum): if bit is set, point can be deleted
		static int HK_CALL closestLineSegLineSeg( const hkVector4& A, const hkVector4& dA, const hkVector4& B, const hkVector4& dB, ClosestLineSegLineSegResult& result );



		//
		//	POINT EDGE
		//

		struct ClosestPointLineSegResult
		{
			hkVector4 m_pointOnEdge;
		};

		static int HK_CALL closestPointLineSeg( const hkVector4& A, const hkVector4& B0, const hkVector4& B1, ClosestPointLineSegResult& result );




		//
		//	POINT TRIANGLE 
		//

		/// Fast way to repeatedly get the closest point between a triangle and a vertex
		struct ClosestPointTriangleCache
		{
			hkReal m_QQ;
			hkReal m_RR;
			hkReal m_QR;
			hkReal m_invTriNormal;
		};

		struct ClosestPointTriangleResult
		{
			hkVector4 hitDirection;
			hkReal	  distance;
			void calcClosestPoint( const hkVector4& point, hkVector4& closestPoint )
			{
				closestPoint.setAddMul4( point, hitDirection, distance );
			}

		};


		static void HK_CALL setupClosestPointTriangleCache( const hkVector4* triangle, ClosestPointTriangleCache& cache );

		enum ClosestPointTriangleStatus
		{
			HIT_TRIANGLE_FACE = 0,
			HIT_TRIANGLE_EDGE
		};

		static ClosestPointTriangleStatus HK_CALL closestPointTriangle( const hkVector4& position, const hkVector4* tri, const ClosestPointTriangleCache& cache, ClosestPointTriangleResult& result );


		//
		//	for fast triangle distance calculation
		//
		struct PointTriangleDistanceCache
		{
			hkReal m_invEdgeLen[3];
			hkReal m_invNormalLen;
			hkReal m_normalLen;
		};

		static void HK_CALL setupPointTriangleDistanceCache( const hkVector4* triangle, PointTriangleDistanceCache& cache );


			/// Calculate the distances to the triangle planes (edge0 = x ... edge2 = z, dist to triangle = w
		static void HK_CALL calcTrianglePlaneDirections( const hkVector4* tri, const PointTriangleDistanceCache& cache, hkTransform& planeEquationsOut, hkVector4& normalOut );

		//
		//	MASKS
		//

		/// returns the index of the bit set if passed in a compare mask or a index-8 of the only bit not set, works only on XYZ
		/// returns -1 for invalid input
		static const hkInt8 maskToIndex[];

		static const hkInt8 vertexToEdgeLut[];

		static inline int HK_CALL getNextModulo3( int i ) { return vertexToEdgeLut[i+2]; }
		static inline int HK_CALL getPrevModulo3( int i ) { return vertexToEdgeLut[i]; }

	protected:

		hkCollideTriangleUtil();

};

//#include <hkinternal/collide/util/hkCollideTriangleUtil.inl>

#endif // HK_GEOMETRY2_TRIANGLEUTIL_H

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
