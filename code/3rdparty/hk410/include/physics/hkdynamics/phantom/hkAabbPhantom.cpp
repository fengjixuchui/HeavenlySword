/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>

#include <hkdynamics/phantom/hkPhantomType.h>
#include <hkdynamics/phantom/hkAabbPhantom.h>
#include <hkmath/basetypes/hkAabb.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkcollide/filter/hkCollisionFilter.h>

#include <hkcollide/castutil/hkWorldRayCastInput.h>
#include <hkcollide/castutil/hkWorldRayCastOutput.h>
#include <hkcollide/shape/hkRayHitCollector.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkAabbPhantom);

hkAabbPhantom::hkAabbPhantom( const hkAabb& aabb, hkUint32 collisionFilterInfo )
: hkPhantom( HK_NULL )
{
	m_aabb = aabb;
	HK_ASSERT(0x16b2d783,  m_aabb.isValid());

	m_collidable.setCollisionFilterInfo( collisionFilterInfo );
}

hkAabbPhantom::~hkAabbPhantom()
{
}

hkPhantomType hkAabbPhantom::getType() const
{
	return HK_PHANTOM_AABB;
}

// hkPhantom clone functionality
hkPhantom* hkAabbPhantom::clone() const
{
	const hkCollidable* c = getCollidable();
	hkAabbPhantom* ap = new hkAabbPhantom( m_aabb, c->getCollisionFilterInfo() );

	// stuff that isn 't in the ctor (the array of listeners (not ref counted etc, assume clone wants the same list to start with)
	ap->m_overlapListeners = m_overlapListeners;
	ap->m_phantomListeners = m_phantomListeners;

	ap->copyProperties( this );
	return ap;
}

void hkAabbPhantom::calcAabb( hkAabb& aabb )
{
	aabb = m_aabb;
}

void hkAabbPhantom::setAabb(const hkAabb& newAabb)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RW );
	m_aabb = newAabb;
	HK_ASSERT(0x270ca8e7,  m_aabb.isValid());

	updateBroadPhase( m_aabb );
}


void hkAabbPhantom::castRay( const hkWorldRayCastInput& input, hkRayHitCollector& collector ) const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );
	HK_ASSERT2(0x5fc4e16f, collector.m_earlyOutHitFraction == 1.0f, "Your collector is not reset");

#ifdef HK_DEBUG
	//
	//	Check whether our ray is inside the AABB
	//
	{
		hkAabb rayAabb;
		rayAabb.m_min.setMin4( input.m_from, input.m_to );
		rayAabb.m_max.setMax4( input.m_from, input.m_to );
		HK_ASSERT2(0x7a3770bc,  m_aabb.contains(rayAabb ), "The aabb of the hkAabbPhantom does not include the ray. Did you forget to call setAabb");
	}
#endif

	HK_TIMER_BEGIN("rcPhantom", HK_NULL);

	hkShapeRayCastInput sinput;
	sinput.m_filterInfo = input.m_filterInfo;

	if (input.m_enableShapeCollectionFilter )
	{
		sinput.m_rayShapeCollectionFilter = m_world->getCollisionFilter();
	}
	else
	{
		sinput.m_rayShapeCollectionFilter = HK_NULL;
	}

	const hkCollidable* const* col = m_overlappingCollidables.begin();

	for (int i = m_overlappingCollidables.getSize()-1; i >=0 ; i--)
	{
		
		const hkShape* shape = (*col)->getShape();
		if ( shape != HK_NULL )
		{
			const hkTransform& trans = (*col)->getTransform();
			sinput.m_from._setTransformedInversePos( trans, input.m_from );
			sinput.m_to.  _setTransformedInversePos( trans, input.m_to );

			const hkCdBody& body = **col;
				
			shape->castRayWithCollector( sinput, body, collector );
		}
		col++;
	}
	HK_TIMER_END();
}

void hkAabbPhantom::castRay( const hkWorldRayCastInput& input, hkWorldRayCastOutput& output ) const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );
	HK_ASSERT2(0x1344cc46,  output.m_rootCollidable == HK_NULL, "Your output has not been reset");
#ifdef HK_DEBUG
	//
	//	Check whether our ray is inside the AABB
	//
	{
		hkAabb rayAabb;
		rayAabb.m_min.setMin4( input.m_from, input.m_to );
		rayAabb.m_max.setMax4( input.m_from, input.m_to );
		HK_ASSERT2(0x26c49b67,  m_aabb.contains(rayAabb ), "The aabb of the hkAabbPhantom does not include the ray. Did you forget to call setAabb");
	}
#endif
	HK_TIMER_BEGIN("rcPhantom", HK_NULL);

	hkShapeRayCastOutput rayResults;

	hkShapeRayCastInput sinput;
	sinput.m_filterInfo = input.m_filterInfo;

	if (input.m_enableShapeCollectionFilter )
	{
		sinput.m_rayShapeCollectionFilter = m_world->getCollisionFilter();
	}
	else
	{
		sinput.m_rayShapeCollectionFilter = HK_NULL;
	}
	
	const hkCollidable* const * col = m_overlappingCollidables.begin();

	for (int i = m_overlappingCollidables.getSize()-1; i >=0 ; i--)
	{	
		const hkShape* shape = (*col)->getShape();
		if ( shape != HK_NULL )
		{
			const hkTransform& trans = (*col)->getTransform();
			sinput.m_from._setTransformedInversePos( trans, input.m_from );
			sinput.m_to.  _setTransformedInversePos( trans, input.m_to );
				
			hkBool hit = shape->castRay( sinput, output);
			if (hit)
			{
				output.m_rootCollidable = *col;
			}
		}
		col++;
	}
	
	if ( output.m_rootCollidable != HK_NULL )
	{
		const hkTransform& trans = output.m_rootCollidable->getTransform();
		output.m_normal.setRotatedDir( trans.getRotation(), output.m_normal);
	}
	HK_TIMER_END();
}

hkBool hkAabbPhantom::isOverlappingCollidableAdded( hkCollidable* collidable )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	for( int i = 0; i<m_overlappingCollidables.getSize(); i++ )
	{
		if( m_overlappingCollidables[i] == collidable )
		{
			return true;
		}
	}
	return false;
}

void hkAabbPhantom::addOverlappingCollidable( hkCollidable* collidable )
{
	// note: the filtering happens before this function is called

	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RW );

	hkCollidableAccept accept = fireCollidableAdded( collidable );

	if ( accept == HK_COLLIDABLE_ACCEPT  )
	{
		m_overlappingCollidables.pushBack( collidable );
	}
}

void hkAabbPhantom::removeOverlappingCollidable( hkCollidable* collidable )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RW );

	// Get the index of the collidable and remove it from the list (with index validity check)
	int index = m_overlappingCollidables.indexOf( collidable );

	fireCollidableRemoved( collidable, index >= 0 );

	if( index >= 0 )
	{
		m_overlappingCollidables.removeAt( index );
	}

}

void hkAabbPhantom::calcStatistics( hkStatisticsCollector* collector ) const 
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );

	collector->beginObject("AabbPhantom", collector->MEMORY_INSTANCE, this);
	hkPhantom::calcStatistics(collector);
	collector->addArray("OvrlpCollPtr", collector->MEMORY_RUNTIME, m_overlappingCollidables);
	collector->endObject();
}

void hkAabbPhantom::deallocateInternalArrays()
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RW );

	// Need to deallocate any arrays that are 0 size
	// else warn user that they should call the in place destructor
	
	// Overlap Listeners
	if (m_overlappingCollidables.getSize() == 0)
	{
		m_overlappingCollidables.clearAndDeallocate();
	}
	else
	{
		HK_WARN(0x234f224e, "Phantom at address " << this << " has non-zero m_overlappingCollidables array.\nPlease call in-place destructor to deallocate.\n");
	}

	hkPhantom::deallocateInternalArrays();
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
