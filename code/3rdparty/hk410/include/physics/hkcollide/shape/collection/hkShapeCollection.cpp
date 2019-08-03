/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkRayShapeCollectionFilter.h>

hkShapeCollection::hkShapeCollection()
{
	m_disableWelding = false;
}

hkBool hkShapeCollection::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const
{
	HK_TIMER_BEGIN("rcShpCollect",HK_NULL);

	ShapeBuffer shapeBuffer;
	results.changeLevel(1);
	hkShapeKey bestKey = HK_INVALID_SHAPE_KEY;	

	if ( !input.m_rayShapeCollectionFilter )
	{
		for (hkShapeKey key = getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = getNextKey( key ) )
		{
			const hkShape* childShape = getChildShape( key, shapeBuffer );
			if ( childShape->castRay( input, results ) )
			{
				bestKey = key;
			}
		}
	}
	else
	{
		for (hkShapeKey key = getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = getNextKey( key ) )
		{
			if ( input.m_rayShapeCollectionFilter->isCollisionEnabled( input, *this, *this, key ) )
			{
				const hkShape* childShape = getChildShape( key, shapeBuffer );
				if ( childShape->castRay( input, results ) )
				{
					bestKey = key;
				}
			}
		}
	}
	results.changeLevel(-1);
	if( bestKey != HK_INVALID_SHAPE_KEY )
	{
		results.setKey(bestKey);
	}
	HK_TIMER_END();
	return bestKey != HK_INVALID_SHAPE_KEY;
}


void hkShapeCollection::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	HK_TIMER_BEGIN("hkShapeCollection::getAabb",HK_NULL);

	out.m_min.setZero4();
	out.m_max.setZero4();

	hkShapeCollection::ShapeBuffer shapeBuffer;

	hkShapeKey key = getFirstKey();
	if ( key != HK_INVALID_SHAPE_KEY )
	{
		const hkShape* childShape = getChildShape( key, shapeBuffer );
		childShape->getAabb( localToWorld, tolerance, out );
	}
	
	for (; key != HK_INVALID_SHAPE_KEY; key = getNextKey( key ) )
	{
		const hkShape* childShape = getChildShape( key, shapeBuffer );

		hkAabb aabb;
		childShape->getAabb( localToWorld, tolerance, aabb );
		out.m_min.setMin4( out.m_min, aabb.m_min );
		out.m_max.setMax4( out.m_max, aabb.m_max );
	}
	HK_TIMER_END();
}

hkReal hkShapeCollection::getMaximumProjection( const hkVector4& direction ) const
{
	HK_TIMER_BEGIN("hkShapeCollection::getMaximumProjection",HK_NULL);
	hkReal result = -HK_REAL_MAX;

	hkShapeCollection::ShapeBuffer shapeBuffer;

	for (hkShapeKey key = getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = getNextKey( key ) )
	{
		const hkShape* childShape = getChildShape( key, shapeBuffer );
		const hkReal p = childShape->getMaximumProjection(direction );
		result = hkMath::max2( result, p );
	}
	HK_TIMER_END();
	return result;
}


void hkShapeCollection::castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	HK_TIMER_BEGIN("rcShpCollect",HK_NULL);
	HK_ASSERT2(0x5c50f827,  cdBody.getShape() == this, "inconsistent cdBody, shapePointer is wrong" );

	hkShapeCollection::ShapeBuffer shapeBuffer;

	if ( !input.m_rayShapeCollectionFilter )
	{
		for (hkShapeKey key = getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = getNextKey( key ) )
		{
			const hkShape* childShape = getChildShape( key, shapeBuffer );
			hkCdBody childBody( &cdBody );
			childBody.setShape( childShape, key );
			childShape->castRayWithCollector( input, childBody, collector );
		}
	}
	else
	{
		for (hkShapeKey key = getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = getNextKey( key ) )
		{
			if ( input.m_rayShapeCollectionFilter->isCollisionEnabled( input, *this, *this, key ) )
			{
				const hkShape* childShape = getChildShape( key, shapeBuffer );
				hkCdBody childBody( &cdBody );
				childBody.setShape( childShape, key );
				childShape->castRayWithCollector( input, childBody, collector );
			}
		}
	}
	HK_TIMER_END();
}



hkShapeType hkShapeCollection::getType() const 
{ 
	return HK_SHAPE_COLLECTION; 
}


void hkShapeCollection::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("Collection", collector->MEMORY_SHARED, this);
	//collector->addChildObject( "Child", collector->MEMORY_SHARED, m_childShape );
	collector->endObject();
}

const hkShapeContainer* hkShapeCollection::getContainer() const
{
	return this;
}

bool hkShapeCollection::disableWelding() const 
{
	return m_disableWelding;
}

/*! \fn const hkShape* hkShapeCollection::getChildShape(const hkShapeKey& key, char*  buffer ) const;
* Note that if you create an object in the buffer passed in, its destructor will not be called. The buffer is simply
* deallocated when the shape is no longer needed. In general this does not matter. However if you are creating a
* shape that references another shape (for example a hkTransformShape) in your implementation of getChildShape
* you should decrement the reference count of the referenced shape, to make up for the fact that the destructor
* of the transform shape will not be called (which would normally do this).
*/

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
