/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/agent/hkCollidable.h>
#include <hkdynamics/entity/hkEntity.h>
#include <hkutilities/collide/filter/disableentity/hkDisableEntityCollisionFilter.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkDisableEntityCollisionFilter);

hkDisableEntityCollisionFilter::hkDisableEntityCollisionFilter()
{
	HK_WARN(0x1e56ab3c, "hkDisableEntityCollisionFilter is deprecated, please do not use");
}

hkDisableEntityCollisionFilter::~hkDisableEntityCollisionFilter()
{
	// remove self as listener from any entities.

	for ( int i = 0; i < m_disabledEntities.getSize(); i++ )
	{
		if ( m_disabledEntities[i]->getEntityListeners().indexOf(this) >= 0 )
		{
			m_disabledEntities[i]->removeEntityListener( this );
		}
	}
}

hkBool hkDisableEntityCollisionFilter::isCollisionEnabled(const hkCollidable& a,const hkCollidable& b) const
{
	for (int i=0; i < m_disabledEntities.getSize(); i++)
	{
		const hkCollidable* stored_collidable = m_disabledEntities[i]->getCollidable();
		if ((stored_collidable == &a) || (stored_collidable == &b))
		{
			return false;
		}
	}
	return true;
}


hkBool hkDisableEntityCollisionFilter::isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const
{
	return true;
}

hkBool hkDisableEntityCollisionFilter::isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const
{
	return true;
}

hkBool hkDisableEntityCollisionFilter::isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const
{
	return true;
}


hkBool hkDisableEntityCollisionFilter::addEntityToFilter( hkEntity* new_entity)
{
	if (!new_entity)
	{
		return false;
	}
	for (int i=0; i < m_disabledEntities.getSize(); i++)
	{
		if (m_disabledEntities[i] == new_entity)
		{
			return false;
		}
	}
	m_disabledEntities.pushBack(new_entity);

	// add to 'new_entity's listeners if this filter is not there
	if ( new_entity->getEntityListeners().indexOf(this) < 0 )
	{
		new_entity->addEntityListener(this);
	}
	return true;
}

hkBool hkDisableEntityCollisionFilter::removeEntityFromFilter( hkEntity* new_entity)
{
	if (!new_entity)
	{
		return false;
	}
	int i = 0;
	while (i < m_disabledEntities.getSize())
	{
		const hkEntity* stored_entity = m_disabledEntities[i];
		
		if (stored_entity == new_entity)
		{
			m_disabledEntities.removeAt(i);
			return true;
		}
		else
		{
			i++;
		}
	}	
	return false;
}

void hkDisableEntityCollisionFilter::entityRemovedCallback(hkEntity* entity)
{
	if (entity)
	{
		removeEntityFromFilter(entity);
		entity->removeEntityListener( this );
	}
}

void hkDisableEntityCollisionFilter::entityDeletedCallback( hkEntity* entity )
{
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
