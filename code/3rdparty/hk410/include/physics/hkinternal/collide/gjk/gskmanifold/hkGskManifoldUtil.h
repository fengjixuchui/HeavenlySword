/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_GSK_MANIFOLD_UTIL_H
#define HK_COLLIDE2_GSK_MANIFOLD_UTIL_H

#include <hkinternal/collide/gjk/gskmanifold/hkGskManifold.h>

struct hkGskManifold;
class hkCdBody;
struct hkProcessCollisionInput;
struct hkExtendedGskOut;
class hkGskCache;
class hkContactMgr;
#include <hkcollide/shape/hkCdVertex.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkinternal/collide/gjk/hkGsk.h>


struct hkGskManifoldWork
{
	hkCdVertex m_vertices[16];
	hkVector4 m_masterNormal;
	hkReal    m_radiusA;
	hkReal    m_radiusB;
	hkReal    m_keepContact;
	hkReal    m_radiusSumSqrd;

	inline const hkVector4& getVertexA( hkUint32 offset ) const
	{
		return *hkAddByteOffsetConst( &m_vertices[0], offset );
	}

	inline const hkVector4& getVertexB( hkUint32 offset ) const
	{
		return *hkAddByteOffsetConst( &m_vertices[0], offset );
	}
};

enum hkGskManifoldAddStatus
{
	HK_GSK_MANIFOLD_POINT_REPLACED0,	// means it replace point 0 in the result array
	HK_GSK_MANIFOLD_POINT_REPLACED1,
	HK_GSK_MANIFOLD_POINT_REPLACED2,
	HK_GSK_MANIFOLD_POINT_REPLACED3,
	HK_GSK_MANIFOLD_POINT_ADDED,
	HK_GSK_MANIFOLD_POINT_REJECTED,
	HK_GSK_MANIFOLD_TWO_POINT2_REJECTED,
};

enum hkGskManifoldUtilMgrHandling
{
	HK_GSK_MANIFOLD_CREATE_ID_ALWAYS,
	HK_GSK_MANIFOLD_NO_ID_FOR_POTENTIALS
};

extern "C"
{
		/// searches the manifold for a given point. returns false(=0) if the point can't be found,
		/// returns true otherwise and moves the found point to the first element in the manifold;
		///    (This is useful, as it allows for skipping the first point in hkGskManifold_verifyAndGetPoints, 
		///    because you have the worldspace information already available)
	bool HK_CALL hkGskManifold_doesPointExistAndResort(  hkGskManifold& manifold, const hkGskCache& newPoint );

		/// initializes the work variables
	HK_FORCE_INLINE void HK_CALL hkGskManifold_init( const hkGskManifold& manifold, const hkVector4& masterNormal, const hkCdBody& cA, const hkCdBody& cB, hkReal keepContactMaxDist, hkGskManifoldWork& work );

		/// verify the points from firstPointIndex to manifol.m_numContactPoints
		/// if (valid) put world space point into the output array.
		/// else free contact point id and remove point
	void HK_CALL hkGskManifold_verifyAndGetPoints( hkGskManifold& manifold, const hkGskManifoldWork& work, int firstPointIndex, hkProcessCollisionOutput& out, hkContactMgr* contactMgr  );


		/// tries to add a point to the manifold. 
		/// Allocates and frees contact point ids as necessary
		/// The new point will always by point 0 in the contactPoint Array in the gskManifold
	hkGskManifoldAddStatus HK_CALL hkGskManifold_addPoint(  hkGskManifold& manifold, const hkCdBody& bodyA,		const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, 
		                                                    const hkGskCache& newPoint, hkProcessCdPoint* newCdPointInResultArray, hkProcessCdPoint* resultPointArray,
															hkContactMgr* mgr, hkGskManifoldUtilMgrHandling mgrHandling );
		
		/// Removes a point from the manifold
	void HK_CALL hkGskManifold_removePoint( hkGskManifold& manifold, int index );
	
		/// Remove all contact points and reset the manifold
	void HK_CALL hkGskManifold_cleanup( hkGskManifold& manifold, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner );

}


HK_FORCE_INLINE void HK_CALL hkGskManifold_init( const hkGskManifold& manifold, const hkVector4& masterNormal, const hkCdBody& cA, const hkCdBody& cB, hkReal keepContactMaxDist, hkGskManifoldWork& work )
{
	work.m_keepContact = keepContactMaxDist;
	const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(cA.getShape());
	const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(cB.getShape());
	work.m_radiusA = shapeA->getRadius();
	work.m_radiusB = shapeB->getRadius();
	hkReal maxDist = work.m_keepContact + work.m_radiusA + work.m_radiusB;
	work.m_radiusSumSqrd = maxDist * maxDist;
	work.m_masterNormal = masterNormal;

	if ( ! manifold.m_numContactPoints )
	{
		return;
	}

	const hkGskManifold::VertexId* vertexIds = manifold.getVertexIds();

	shapeA->convertVertexIdsToVertices( vertexIds, manifold.m_numVertsA, &work.m_vertices[0] );
	hkVector4Util::transformPoints( cA.getTransform(), &work.m_vertices[0], manifold.m_numVertsA, &work.m_vertices[0] );

	int b = manifold.m_numVertsA;
	hkCdVertex* verts = &work.m_vertices[b];
	shapeB->convertVertexIdsToVertices( vertexIds+ b, manifold.m_numVertsB, verts );
	hkVector4Util::transformPoints( cB.getTransform(), verts, manifold.m_numVertsB, verts );
}


#endif //HK_COLLIDE2_GSK_MANIFOLD_UTIL_H

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
