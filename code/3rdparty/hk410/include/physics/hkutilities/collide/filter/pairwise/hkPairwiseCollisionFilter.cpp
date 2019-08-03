/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/agent/hkCollidable.h>
#include <hkdynamics/entity/hkEntity.h>
#include <hkutilities/collide/filter/pairwise/hkPairwiseCollisionFilter.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkPairwiseCollisionFilter);

hkPairwiseCollisionFilter::hkPairwiseCollisionFilter()
{
}

hkPairwiseCollisionFilter::~hkPairwiseCollisionFilter()
{
	// remove self as listener from any entities.
	for ( int i = 0; i < m_disabledPairs.getSize(); ++i )
	{
		if ( m_disabledPairs[i].m_a->getEntityListeners().indexOf(this) >= 0 )
		{
			m_disabledPairs[i].m_a->removeEntityListener( this );
		}
		if ( m_disabledPairs[i].m_b->getEntityListeners().indexOf(this) >= 0 )
		{
			m_disabledPairs[i].m_b->removeEntityListener( this );
		}
	}
}

void hkPairwiseCollisionFilter::disableCollisionPair(hkEntity* a, hkEntity* b)
{
	CollisionPair pair(a, b);
	if (m_disabledPairs.indexOf(pair) > 0)
	{
		HK_ASSERT2(0x3839c853, 0, "Collision pair already disabled");
	}
	else
	{
		m_disabledPairs.pushBack(pair);

		// add to 'entityA's listeners if this filter is not there
		if ( a->getEntityListeners().indexOf(this) < 0 )
		{
			a->addEntityListener(this);
		}

		// add to 'entityA's listeners if this filter is not there
		if ( b->getEntityListeners().indexOf(this) < 0 )
		{
			b->addEntityListener(this);
		}
	}
}

void hkPairwiseCollisionFilter::enableCollisionPair(hkEntity* a, hkEntity* b)
{
	CollisionPair pair(a, b);

	int idx = m_disabledPairs.indexOf(pair);
	if (idx < 0)
	{
		HK_ASSERT2(0x33e5b4fc, 0, "Collision pair already enabled");
	}
	else
	{
		m_disabledPairs.removeAt(idx);
		{
			// remove from 'entityA's listeners if this filter is there
			const hkArray<hkEntityListener*>& listeners = a->getEntityListeners();
			int i = listeners.indexOf(this);
			if (i >= 0)
			{
				a->removeEntityListener(this);
			}
		}

		{
			// remove from 'entityB's listeners if this filter is there
			const hkArray<hkEntityListener*>& listeners = b->getEntityListeners();
			int i = listeners.indexOf(this);
			if (i >= 0)
			{
				 b->removeEntityListener(this);
			}
		}
	}
}

hkBool hkPairwiseCollisionFilter::isCollisionEnabled(const hkCollidable& a, const hkCollidable& b) const
{
	hkEntity* entityA = static_cast<hkEntity*>(a.getOwner());
	hkEntity* entityB = static_cast<hkEntity*>(b.getOwner());
	CollisionPair pair(entityA, entityB);
	return m_disabledPairs.indexOf(pair) < 0;
}


hkBool hkPairwiseCollisionFilter::isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const
{
	return true;
}

hkBool hkPairwiseCollisionFilter::isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const 
{
	return true;
}

hkBool hkPairwiseCollisionFilter::isCollisionEnabled( const hkWorldRayCastInput& aInput, const hkCollidable& collidableB ) const
{
	return true;
}



void hkPairwiseCollisionFilter::entityRemovedCallback(hkEntity* entity)
{
	// temporary implementation - iterate through the internal array

	int i = 0;
	while (i < m_disabledPairs.getSize())
	{
		const hkEntity* entityA = m_disabledPairs[i].m_a;
		const hkEntity* entityB = m_disabledPairs[i].m_b;

		if ( (entityA == entity) || (entityB == entity) )
		{
			// entity has been deleted - remove pair that references it
			m_disabledPairs.removeAt(i);
		}
		else
		{
			i++;
		}
	}	
	entity->removeEntityListener( this );
}

void hkPairwiseCollisionFilter::entityDeletedCallback( hkEntity* entity )
{
}

int hkPairwiseCollisionFilter::getNumDisabledPairs() const
{
	const int numPairs = m_disabledPairs.getSize();
	return numPairs;
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
