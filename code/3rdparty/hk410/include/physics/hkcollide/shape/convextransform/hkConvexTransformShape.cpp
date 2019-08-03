/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/util/hkAabbUtil.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkConvexTransformShape);


hkConvexTransformShape::hkConvexTransformShape( const hkConvexShape* childShape, const hkTransform& transform )
: hkConvexShape( childShape->getRadius()), m_childShape(childShape) 
{
	HK_ASSERT2(0x6acf0520, childShape != HK_NULL, "Child shape cannot be NULL");  
	m_transform = transform;
}

void hkConvexTransformShape::setTransform(const hkTransform& transform) 
{ 
	m_transform = transform;
}

hkShapeType hkConvexTransformShape::getType() const
{
	return HK_SHAPE_CONVEX_TRANSFORM;
}


void hkConvexTransformShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkTransform worldTshape; worldTshape.setMul ( localToWorld, m_transform );
	m_childShape->getAabb( worldTshape, tolerance, out );
}


// support for MOPP
hkReal hkConvexTransformShape::getMaximumProjection( const hkVector4& direction ) const
{
	hkVector4 localDir; localDir._setRotatedInverseDir( m_transform.getRotation(), direction );
	hkReal localProjection = m_childShape->getMaximumProjection( localDir );
	hkVector4 localSupport;	localSupport.setMul4( localProjection, localDir );
	hkVector4 worldSupport;	worldSupport._setTransformedPos( m_transform, localSupport );
	return worldSupport.dot3( direction );
}

hkBool hkConvexTransformShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const
{
	HK_TIMER_BEGIN("rcConvTransl", HK_NULL);
	hkShapeRayCastInput subInput = input;

	subInput.m_from._setTransformedInversePos( m_transform, input.m_from );
	subInput.m_to._setTransformedInversePos( m_transform, input.m_to );

	results.changeLevel(1);
	const hkBool hit = m_childShape->castRay( subInput, results );
	results.changeLevel(-1);
	if (hit)
	{
		//transform hitnormal from 'childshape' into 'transformshapes' space
		const hkVector4 oldnormal = results.m_normal;
		results.m_normal.setRotatedDir( m_transform.getRotation(), oldnormal );
		results.setKey(0);
	}
	HK_TIMER_END();
	return hit;
}

void hkConvexTransformShape::castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& parentCdBody, hkRayHitCollector& collector ) const
{
	HK_TIMER_BEGIN("rcCxTransform", HK_NULL);
	hkShapeRayCastInput subInput = input;

	subInput.m_from._setTransformedInversePos( m_transform, input.m_from );
	subInput.m_to._setTransformedInversePos( m_transform, input.m_to );

	hkTransform t; t.setMul( parentCdBody.getTransform(), m_transform);

	hkCdBody body( &parentCdBody, &t);
	const hkShape* child = m_childShape.getChild();
	body.setShape( child, 0 );
	child->castRayWithCollector( subInput, body, collector );
	HK_TIMER_END();
}

void hkConvexTransformShape::getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertexOut ) const
{
	hkVector4 localDir; localDir._setRotatedInverseDir( m_transform.getRotation(), direction );
	hkCdVertex localVertex; getChildShape()->getSupportingVertex( localDir, localVertex );
	supportingVertexOut._setTransformedPos( m_transform, localVertex );
	supportingVertexOut.setW( localVertex ); // copy the id
}

void hkConvexTransformShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	getChildShape()->convertVertexIdsToVertices( ids, numIds, verticesOut );
	hkVector4Util::transformSpheres( m_transform, verticesOut, numIds, verticesOut );
}

void hkConvexTransformShape::getFirstVertex(hkVector4& v) const
{
	hkVector4 localVertex; 
	getChildShape()->getFirstVertex( localVertex );
	v._setTransformedPos(m_transform, localVertex);
}

void hkConvexTransformShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	getChildShape()->getCollisionSpheresInfo( infoOut );
	infoOut.m_useBuffer = true;
}

const hkSphere* hkConvexTransformShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	const hkSphere* spheres = getChildShape()->getCollisionSpheres( sphereBuffer );

	hkCollisionSpheresInfo info;
	getChildShape()->getCollisionSpheresInfo( info );
	const hkVector4 *in = &spheres->getPositionAndRadius();
	hkVector4 *out = &sphereBuffer->getPositionAndRadius();
	hkVector4Util::transformSpheres( m_transform, in, info.m_numSpheres, out );

	return sphereBuffer;
}

void hkConvexTransformShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("CvxTransform", collector->MEMORY_SHARED, this);
	collector->addChildObject( "Child", collector->MEMORY_SHARED, getChildShape() );
	collector->endObject();
}

const hkShapeContainer* hkConvexTransformShape::getContainer() const
{
	return &m_childShape;
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
