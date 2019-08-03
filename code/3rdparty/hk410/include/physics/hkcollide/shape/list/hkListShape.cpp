/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/list/hkListShape.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkListShape);

hkListShape::hkListShape(const hkShape*const* shapeArray, int numShapes) 
: hkShapeCollection()
{
    setShapes( shapeArray, numShapes, HK_NULL );
}

hkListShape::~hkListShape()
{
	for (int i = 0; i < m_childInfo.getSize(); i++)
	{
		m_childInfo[i].m_shape->removeReference();
	}
}

void hkListShape::setShapes( const hkShape*const* shapeArray, int numShapes, const hkUint32* filterInfo )
{
	HK_ASSERT2(0x282822c7,  m_childInfo.getSize()==0, "You can only call setShapes once during construction.");
	HK_ASSERT2(0x221e5b17,  numShapes, "You cannot create a hkListShape with no child shapes" );
	m_childInfo.setSize(numShapes);

	for (int i = 0; i < numShapes; i++)
	{
		if (shapeArray[i] != HK_NULL)
		{
			m_childInfo[i].m_shape = shapeArray[i];
			m_childInfo[i].m_collisionFilterInfo = filterInfo? filterInfo[i] : 0;
			shapeArray[i]->addReference();
		}
	}
}

hkShapeType hkListShape::getType() const
{
	return HK_SHAPE_LIST;
}

void hkListShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	m_childInfo[0].m_shape->getAabb( localToWorld, tolerance, out );

	hkAabb t;
	for (int i = 1; i < m_childInfo.getSize(); i++)
	{
		m_childInfo[i].m_shape->getAabb( localToWorld, tolerance, t );
		out.m_min.setMin4( out.m_min, t.m_min );
		out.m_max.setMax4( out.m_max, t.m_max );
	}
}


int hkListShape::getNumChildShapes() const 
{ 
	return m_childInfo.getSize(); 
}


hkShapeKey hkListShape::getFirstKey() const
{
	return 0;
}

hkShapeKey hkListShape::getNextKey( hkShapeKey oldKey ) const
{
	if ( static_cast<int>(oldKey + 1) < m_childInfo.getSize() )
	{
		return oldKey + 1;
	}
	else
	{
		return HK_INVALID_SHAPE_KEY;
	}
}


const hkShape* hkListShape::getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const
{
	return m_childInfo[ key ].m_shape;
}

hkUint32 hkListShape::getCollisionFilterInfo( hkShapeKey key ) const
{
	return m_childInfo[ key ].m_collisionFilterInfo;
}


void hkListShape::setCollisionFilterInfo( hkShapeKey index, hkUint32 filterInfo )
{
	m_childInfo[ index ].m_collisionFilterInfo = filterInfo;
}


void hkListShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("ListShape", collector->MEMORY_SHARED, this);

	collector->addArray("ChildPtrs", collector->MEMORY_SHARED, this->m_childInfo);

	for ( int i = 0; i < this->m_childInfo.getSize(); i++ )
	{
		collector->addChildObject( "Child", collector->MEMORY_SHARED, m_childInfo[i].m_shape );
	}
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
