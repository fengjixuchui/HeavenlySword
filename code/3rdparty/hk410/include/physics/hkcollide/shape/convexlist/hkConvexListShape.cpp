/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/convexlist/hkConvexListShape.h>
#include <hkbase/class/hkTypeInfo.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkRayShapeCollectionFilter.h>
#include <hkcollide/util/hkAabbUtil.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkConvexListShape);

hkConvexListShape::hkConvexListShape(const hkConvexShape*const* shapeArray, int numShapes) 
:	hkConvexShape( 0 ) // We set radius from the first shape of the shape Array - see below
// [HVK-2338] const ptr to const ptrs, so that I/F match. Should not need the reinterpret_cast but VC doesn't like it otherwise.
{
	m_minDistanceToUseConvexHullForGetClosestPoints = 1.0f;

	#ifdef HK_DEBUG
		const hkReal radius = shapeArray[0]->getRadius();
		for (int i =1; i < numShapes; i++ )
		{
			HK_ASSERT2( 0xf032da3a, hkMath::equal( radius, shapeArray[i]->getRadius(), .1f ), "All child shapes of a hkConvexListShape must have identical radius" );
			if ( !hkMath::equal( radius, shapeArray[i]->getRadius(), .01f ) )
			{
				HK_WARN( 0xf032da3a, "All child shapes of a hkConvexListShape must have identical radius" );
			}
		}
	#endif

	setShapesAndRadius( shapeArray, numShapes );
	setUseCachedAabb( true );
}


hkConvexListShape::~hkConvexListShape()
{
	for (int i = 0; i < m_childShapes.getSize(); i++)
	{
		m_childShapes[i]->removeReference();
	}
}




void hkConvexListShape::getSupportingVertex( const hkVector4& dir, hkCdVertex& supportingVertexOut ) const
{
	hkReal maxDot = - HK_REAL_MAX;
	int subShape = 0;

	for ( int i = 0; i < m_childShapes.getSize(); i++)
	{
		hkCdVertex support;
		const hkConvexShape* shape = static_cast<const hkConvexShape*>( m_childShapes[i] );
		shape->getSupportingVertex( dir, support );
		hkReal dot = support.dot3( dir );
		if ( dot > maxDot )
		{
			maxDot = dot;
			supportingVertexOut = support;
			subShape = i;
		}
	}
	int id = supportingVertexOut.getId();
	HK_ASSERT2( 0xf0ad12f4, id < 256, "The convex list shape can only use child shapes with vertex ids < 256" );
	id +=  (subShape<<8);
	supportingVertexOut.setInt24W( id );
}

void hkConvexListShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	for (int i = 0; i < numIds; i++)
	{
		hkVertexId id = ids[0];
		int subShape = id>>8;
		id &= 0xff;
		const hkConvexShape* shape = static_cast<const hkConvexShape*>( m_childShapes[subShape] );

		shape->convertVertexIdsToVertices( &id, 1, verticesOut );

		// patch the id
		{
			int vid = verticesOut->getId();
			vid +=  (subShape<<8);
			verticesOut->setInt24W( vid );
		}

		ids++;
		verticesOut++;
	}
}

void hkConvexListShape::getFirstVertex(hkVector4& v) const
{
	const hkConvexShape* shape = static_cast<const hkConvexShape*>( m_childShapes[0] );
	shape->getFirstVertex( v );
}


void hkConvexListShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{	
	infoOut.m_numSpheres = 0;
	infoOut.m_useBuffer = true;
	for (int i = 0; i < m_childShapes.getSize(); i++)
	{
		const hkConvexShape* shape = static_cast<const hkConvexShape*>( m_childShapes[i] );

		hkCollisionSpheresInfo childinfo;
		shape->getCollisionSpheresInfo( childinfo );
		HK_ASSERT2( 0xf0e32132, childinfo.m_useBuffer == true, "You can only use shapes for the hkConvexListShape if you subshape copy the spheres in the getCollisionSpheres function" );
		infoOut.m_numSpheres += childinfo.m_numSpheres;
	}
}


const hkSphere* hkConvexListShape::getCollisionSpheres( hkSphere* sphereBuffer ) const 
{
	hkSphere* p = sphereBuffer;
	for (int i = 0; i < m_childShapes.getSize(); i++)
	{
		const hkConvexShape* shape = static_cast<const hkConvexShape*>( m_childShapes[i] );

		shape->getCollisionSpheres( p );

		hkCollisionSpheresInfo childinfo;
		shape->getCollisionSpheresInfo( childinfo );
		p += childinfo.m_numSpheres;
	}
	return sphereBuffer;
}




// Gets this hkShape's primary type. For hkConvexListShapes, this is HK_SHAPE_LIST.
hkShapeType hkConvexListShape::getType() const
{
	return HK_SHAPE_CONVEX_LIST;
}


void hkConvexListShape::setShapesAndRadius( const hkConvexShape*const* shapeArray, int numShapes )
{
	HK_ASSERT2(0x282822c7,  m_childShapes.getSize()==0, "You can only call setShapes once during construction.");
	HK_ASSERT2(0x221e5b17,  numShapes, "You cannot create a hkConvexListShape with no child shapes" );
	
	m_childShapes.setSize(numShapes);
	m_radius = shapeArray[0]->getRadius();

	for (int i = 0; i < numShapes; i++)
	{
		HK_ASSERT2( 0xfeaf9625, shapeArray[i] != HK_NULL, "You cannot create a hkConvexListShape with a shapeArray containing NULL pointers" );
		HK_ASSERT2( 0xa456bdbd, hkMath::equal(m_radius, shapeArray[i]->getRadius()), "You cannot create a hkConvexListShape with child shapes of different radii");

		m_childShapes[i] = shapeArray[i];
		shapeArray[i]->addReference();
	}
}

void hkConvexListShape::setUseCachedAabb( bool useCachedAabb )
{
	m_useCachedAabb = useCachedAabb;
	if (useCachedAabb)
	{
		hkAabb aabb;
		aabb.m_min.setAll(HK_REAL_MAX);
		aabb.m_max.setAll(-HK_REAL_MAX);

		for (int i = 0; i < m_childShapes.getSize(); i++)
		{
			hkAabb localAabb;
			m_childShapes[i]->getAabb( hkTransform::getIdentity(),0, localAabb );

			aabb.m_min.setMin4( aabb.m_min, localAabb.m_min );
			aabb.m_max.setMax4( aabb.m_max, localAabb.m_max );
		}

		m_aabbCenter.setAdd4( aabb.m_min, aabb.m_max );
		m_aabbCenter.mul4( 0.5f );

		m_aabbHalfExtents.setSub4( aabb.m_max, aabb.m_min );
		m_aabbHalfExtents.mul4( 0.5f );
	}
}

bool hkConvexListShape::getUseCachedAabb()
{
	return m_useCachedAabb;
}

void hkConvexListShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	if (m_useCachedAabb)
	{
		hkAabbUtil::calcAabb( localToWorld, m_aabbHalfExtents, m_aabbCenter, tolerance + m_radius,  out );
	}
	else
	{
		m_childShapes[0]->getAabb( localToWorld, tolerance, out );

		hkAabb t;
		for (int i = 1; i < m_childShapes.getSize(); i++)
		{
			m_childShapes[i]->getAabb( localToWorld, tolerance, t );
			out.m_min.setMin4( out.m_min, t.m_min );
			out.m_max.setMax4( out.m_max, t.m_max );
		}
	}
}


int hkConvexListShape::getNumChildShapes() const 
{ 
	return m_childShapes.getSize(); 
}


hkShapeKey hkConvexListShape::getFirstKey() const
{
	return 0;
}

hkShapeKey hkConvexListShape::getNextKey( hkShapeKey oldKey ) const
{
	if ( static_cast<int>(oldKey + 1) < m_childShapes.getSize() )
	{
		return oldKey + 1;
	}
	else
	{
		return HK_INVALID_SHAPE_KEY;
	}
}


const hkShape* hkConvexListShape::getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const
{
	return m_childShapes[ key ];
}

hkUint32 hkConvexListShape::getCollisionFilterInfo( hkShapeKey key ) const
{
	HK_WARN_ONCE(0xeaab8764, "Collision filtering does not work for hkConvexListShapes" );
	return 0;
}



void hkConvexListShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("ConvexListShape", collector->MEMORY_SHARED, this);

	collector->addArray("ChildPtrs", collector->MEMORY_SHARED, this->m_childShapes);

	for ( int i = 0; i < this->m_childShapes.getSize(); i++ )
	{
		collector->addChildObject( "Child", collector->MEMORY_SHARED, m_childShapes[i] );
	}
	collector->endObject();
}

//
// TODO - implement these efficiently, using mopp
//

hkBool hkConvexListShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const
{
	// Note there is no collision filtering with convex list shapes

	HK_TIME_CODE_BLOCK("rcCxList",HK_NULL);

	ShapeBuffer shapeBuffer;
	results.changeLevel(1);
	hkShapeKey bestKey = HK_INVALID_SHAPE_KEY;	

	for (hkShapeKey key = getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = getNextKey( key ) )
	{
		const hkShape* childShape = getChildShape( key, shapeBuffer );
		if ( childShape->castRay( input, results ) )
		{
			bestKey = key;
		}
	}

	results.changeLevel(-1);
	if( bestKey != HK_INVALID_SHAPE_KEY )
	{
		results.setKey(bestKey);
	}

	return bestKey != HK_INVALID_SHAPE_KEY;
}


void hkConvexListShape::castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	HK_TIME_CODE_BLOCK("rcShpCollect",HK_NULL);
	HK_ASSERT2(0x5c50f827,  cdBody.getShape() == this, "inconsistent cdBody, shapePointer is wrong" );

	hkShapeCollection::ShapeBuffer shapeBuffer;

	for (hkShapeKey key = getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = getNextKey( key ) )
	{
		const hkShape* childShape = getChildShape( key, shapeBuffer );
		hkCdBody childBody( &cdBody );
		childBody.setShape( childShape, key );
		childShape->castRayWithCollector( input, childBody, collector );
	}
}


const hkShapeContainer* hkConvexListShape::getContainer() const
{
	return this;
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
