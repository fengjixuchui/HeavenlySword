/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>
#include <hkcollide/util/hkAabbUtil.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkConvexTranslateShape);


hkConvexTranslateShape::hkConvexTranslateShape( const hkConvexShape* childShape, const hkVector4& translation )
: hkConvexShape( childShape->getRadius()), m_childShape(childShape) 
{
	HK_ASSERT2(0x6acf0520, childShape != HK_NULL, "Child shape cannot be NULL");  
	HK_ASSERT(0x6acf0521, childShape->getContainer() == HK_NULL);  
	m_translation = translation;
	m_translation(3) = 0;
}

hkShapeType hkConvexTranslateShape::getType() const
{
	return HK_SHAPE_CONVEX_TRANSLATE;
}

void hkConvexTranslateShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	m_childShape->getAabb( localToWorld, tolerance, out );

	hkVector4 trans; trans._setRotatedDir( localToWorld.getRotation(), m_translation );
	out.m_min.add4( trans );
	out.m_max.add4( trans );
}


// support for MOPP
hkReal hkConvexTranslateShape::getMaximumProjection( const hkVector4& direction ) const
{
	hkReal localProjection = m_childShape->getMaximumProjection( direction );
	hkReal offset = direction.dot3( m_translation );
	return localProjection + offset;
}

hkBool hkConvexTranslateShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const
{
	HK_TIMER_BEGIN("rcConvTransl", HK_NULL);
	hkShapeRayCastInput subInput = input;

	subInput.m_from.setSub4( input.m_from, m_translation );
	subInput.m_to.setSub4( input.m_to, m_translation );

	results.changeLevel(1);
	const hkBool hit = m_childShape->castRay( subInput, results );
	results.changeLevel(-1);
	if( hit )
	{
		results.setKey(0);
	}	
	HK_TIMER_END();
	return hit;
}


void hkConvexTranslateShape::castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& parentCdBody, hkRayHitCollector& collector ) const
{
	HK_ASSERT2(0x7f1735a0, parentCdBody.getShape() == this, "inconsistent cdBody, shapePointer is wrong" );
	hkShapeRayCastInput subInput = input;
	subInput.m_from.setSub4( input.m_from, m_translation );
	subInput.m_to.setSub4( input.m_to, m_translation );
	hkCdBody cdBody(&parentCdBody);
	const hkShape* child = m_childShape.getChild();
	cdBody.setShape(child, 0);
	child->castRayWithCollector(subInput, cdBody, collector );
}

void hkConvexTranslateShape::getSupportingVertex( const hkVector4& dir, hkCdVertex& supportingVertexOut ) const
{
	HK_ASSERT2( 0x4835a45e, m_translation(3) == 0.0f, "The w component of hkConvexTranslateShape::m_translation must be zero." ); 
	getChildShape()->getSupportingVertex( dir, supportingVertexOut );
	HK_ON_DEBUG( int id = supportingVertexOut.getId() );
	supportingVertexOut.add3clobberW( m_translation );
	HK_ASSERT2( 0xf019fe43, id == supportingVertexOut.getId(), "The supporting vertex ID was changed while applying the translation in hkConvexTranslateShape::getSupportingVertex()." );
}

void hkConvexTranslateShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	getChildShape()->convertVertexIdsToVertices( ids, numIds, verticesOut );
	{
		for (int i = 0; i < numIds; i++)
		{
			verticesOut[i].add3clobberW( m_translation );
		}
	}
}

void hkConvexTranslateShape::getFirstVertex(hkVector4& v) const
{
	getChildShape()->getFirstVertex( v );
	v.add4( m_translation );
}

void hkConvexTranslateShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	getChildShape()->getCollisionSpheresInfo( infoOut );
	infoOut.m_useBuffer = true;
}

const hkSphere* hkConvexTranslateShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	const hkSphere* spheres = getChildShape()->getCollisionSpheres( sphereBuffer );
	hkSphere* spheresOut = sphereBuffer;

	hkCollisionSpheresInfo info;
	getChildShape()->getCollisionSpheresInfo( info );

	{
		for (int i = 0; i < info.m_numSpheres; i++)
		{
			spheresOut->getPositionAndRadius().setAdd4( spheres->getPositionAndRadius(), m_translation );
			spheres++;
			spheresOut++;
		}
	}
	return sphereBuffer;
}

void hkConvexTranslateShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("CvxTranslate", collector->MEMORY_SHARED, this);
	collector->addChildObject( "Child", collector->MEMORY_SHARED, getChildShape() );
	collector->endObject();
}

const hkShapeContainer* hkConvexTranslateShape::getContainer() const
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
