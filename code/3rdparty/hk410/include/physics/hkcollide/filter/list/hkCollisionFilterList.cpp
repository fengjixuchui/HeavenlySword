/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/filter/list/hkCollisionFilterList.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkCollisionFilterList);

hkCollisionFilterList::hkCollisionFilterList( const hkArray<const hkCollisionFilter*>& collisionFilters )
{
	m_collisionFilters = collisionFilters;
	for (int i  = 0; i < m_collisionFilters.getSize(); ++i)
	{
		m_collisionFilters[i]->addReference();
	}
}

hkCollisionFilterList::~hkCollisionFilterList()
{
	for ( int i = 0; i < m_collisionFilters.getSize(); ++i )
	{
		m_collisionFilters[i]->removeReference();
	}
}



hkBool hkCollisionFilterList::isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const
{
	 // check filters
	for ( int i = m_collisionFilters.getSize()-1; i>=0; i-- )
	{
		if(! m_collisionFilters[i]->isCollisionEnabled( a, b ))
		{
			return false;
		}
	}
	return true;
}


hkBool hkCollisionFilterList::isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const
{
	 // check filters
	for ( int i = m_collisionFilters.getSize()-1; i>=0; i-- )
	{
		if(! m_collisionFilters[i]->isCollisionEnabled( input, a, b, bContainer, bKey ))
		{
			return false;
		}
	}
	return true;
}

hkBool hkCollisionFilterList::isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& bShape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const
{
		 // check filters
	for ( int i = m_collisionFilters.getSize()-1; i>=0; i-- )
	{
		if(! m_collisionFilters[i]->isCollisionEnabled( aInput, bShape, bContainer, bKey ))
		{
			return false;
		}
	}
	return true;
}

hkBool hkCollisionFilterList::isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const
{
		 // check filters
	for ( int i = m_collisionFilters.getSize()-1; i>=0; i-- )
	{
		if(! m_collisionFilters[i]->isCollisionEnabled( a, collidableB ))
		{
			return false;
		}
	}
	return true;
}


hkCollisionFilterList::hkCollisionFilterList()
{
}

void hkCollisionFilterList::addCollisionFilter( const hkCollisionFilter* filter)
{
	filter->addReference();
	m_collisionFilters.pushBack( filter );
}

void hkCollisionFilterList::removeCollisionFilter( const hkCollisionFilter* filter)
{
	int index = m_collisionFilters.indexOf(filter);
	HK_ASSERT2(0x509f9d0d,  index >= 0, "Collision filter was not found under existing filters" );
	m_collisionFilters.removeAt(  index );
	filter->removeReference();
}


const hkArray<const hkCollisionFilter*>& hkCollisionFilterList::getCollisionFilters() const
{	
	return m_collisionFilters;	
}

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
