/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkbase/debugutil/hkStatisticsCollector.h>
#include <hkdynamics/phantom/hkPhantomType.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkcollide/agent/hkCollisionInput.h>
#include <hkcollide/agent/hkLinearCastCollisionInput.h>
#include <hkcollide/agent/hkCdPointCollector.h>
#include <hkcollide/agent/hkCdBodyPairCollector.h>
#include <hkcollide/agent/hkCollisionAgent.h>
#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkcollide/castutil/hkLinearCastInput.h>
#include <hkmath/basetypes/hkAabb.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkSimpleShapePhantom);
	
hkSimpleShapePhantom::hkSimpleShapePhantom(	 const hkShape* shape, const hkTransform& transform, hkUint32 collisionFilterInfo )
:	hkShapePhantom( shape, transform )
{
	m_collidable.setCollisionFilterInfo( collisionFilterInfo );
}

// N.B. Setting the shape of a hkShapePhantom with setShapeFastUnsafe() while it is in the world is very dangerous!
// If any hkCachingShapePhantoms are also in the world and overlap with this phantom, they will not be
// notified of the shape change, and their cached agents will become invalid.
// You should remove this phantom from the world, change the shape, then re-add.
//hkWorldOperation::Result hkSimpleShapePhantom::setShapeFastUnsafe( hkShape* shape )
//{
//	hkWorld* world = getWorld();
//	hkWorldOperation::Result baseResult;
//	if (world)
//	{
//		//world->blockExecutingPendingOperations(true);
//		set collidable's shape directly from this f.//baseResult = hkShapePhantom::setShape(shape);
//		//world->blockExecutingPendingOperations(false);
//	}
//	else
//	{
//		baseResult = hkShapePhantom::setShape(shape);
//	}
//
//	if (baseResult == hkWorldOperation::DONE && world)
//	{	
//		// Update the aabb. This is allowable if you know *for certain* that it will not
//		// corrupt any hkCachingShapePhantoms.
//
//		hkAabb aabb;
//		hkReal halfTolerance = 0.5f * m_world->getCollisionInput()->getTolerance();
//		shape->getAabb( m_motionState.getTransform(), halfTolerance, aabb );
//
//		world->lockCriticalOperations();
//		updateBroadPhase( aabb );
//		world->unlockAndAttemptToExecutePendingOperations();
//	}
//
//	return baseResult;
//}

hkSimpleShapePhantom::~hkSimpleShapePhantom()
{

}

hkPhantomType hkSimpleShapePhantom::getType() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	return HK_PHANTOM_SIMPLE_SHAPE;
}

// hkPhantom clone functionality
hkPhantom* hkSimpleShapePhantom::clone() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	const hkCollidable* c = getCollidable();
	hkSimpleShapePhantom* ssp = new hkSimpleShapePhantom(c->getShape(), m_motionState.getTransform(), c->getCollisionFilterInfo() );
	
	// stuff that isn 't in the ctor (the array of listeners (not ref counted etc, assume clone wants the same list to start with)
	ssp->m_overlapListeners = m_overlapListeners;
	ssp->m_phantomListeners = m_phantomListeners;

	ssp->copyProperties( this );
	
	return ssp;
}

void hkSimpleShapePhantom::setPositionAndLinearCast( const hkVector4& position, const hkLinearCastInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startCollector )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RW, this, HK_ACCESS_RW );

	m_motionState.getTransform().setTranslation( position );

	hkAabb aabb;
	hkVector4 path;

	//
	//	calculate the correct aabb and update the broadphase
	//
	{
		hkReal halfTolerance = 0.5f * m_world->getCollisionInput()->getTolerance();
		m_collidable.getShape()->getAabb( m_motionState.getTransform(), halfTolerance + input.m_startPointTolerance, aabb );

		path.setSub4( input.m_to, position );
		hkVector4 zero; zero.setZero4();
		hkVector4 pathMin; pathMin.setMin4( zero, path );
		hkVector4 pathMax; pathMax.setMax4( zero, path );

		aabb.m_min.add4( pathMin );
		aabb.m_max.add4( pathMax );
		updateBroadPhase( aabb );
	}

	//
	//	Setup the linear cast input
	//
	hkLinearCastCollisionInput lcInput;
	{
		lcInput.set( *m_world->getCollisionInput() );
		lcInput.setPathAndTolerance( path, input.m_startPointTolerance );
		lcInput.m_maxExtraPenetration = input.m_maxExtraPenetration;
	}

	
	//
	//	Do the cast
	//
	{
		hkCollisionDispatcher* dispatcher = m_world->getCollisionDispatcher();
		for ( int i = m_collisionDetails.getSize() - 1; i >= 0; i-- )
		{
			hkCollisionDetail& det = m_collisionDetails[i];
			hkShapeType typeB = det.m_collidable->getShape()->getType();

			hkCollisionDispatcher::LinearCastFunc linearCast = dispatcher->getLinearCastFunc( m_collidable.getShape()->getType(), typeB );
			linearCast( m_collidable, *det.m_collidable, lcInput, castCollector, startCollector );
		}
	}
}

void hkSimpleShapePhantom::getClosestPoints( hkCdPointCollector& collector )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );
	HK_ASSERT2(0x2fe9081e, m_world != HK_NULL, "hkSimpleShapePhantom has not been added to an hkWorld."); 
	const hkCollisionInput& input = *m_world->getCollisionInput(); 
	hkCollisionDispatcher* dispatcher = input.m_dispatcher;
	for ( int i = m_collisionDetails.getSize() - 1; i >= 0; i-- )
	{
		hkCollisionDetail& det = m_collisionDetails[i];
		hkShapeType typeB = det.m_collidable->getShape()->getType();

		hkCollisionDispatcher::GetClosestPointsFunc getClosestPointsFunc = dispatcher->getGetClosestPointsFunc( m_collidable.getShape()->getType(), typeB );
		(*getClosestPointsFunc)( m_collidable, *det.m_collidable, input, collector );
	}
}
	
void hkSimpleShapePhantom::getPenetrations( hkCdBodyPairCollector& collector )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RO, this, HK_ACCESS_RO );
	HK_ASSERT2(0x13b152a4, m_world != HK_NULL, "hkSimpleShapePhantom has not been added to an hkWorld."); 
	hkCollisionInput* input = m_world->getCollisionInput();
	hkCollisionDispatcher* dispatcher = input->m_dispatcher;
	for ( int i = m_collisionDetails.getSize() - 1; i >= 0; i-- )
	{
		hkCollisionDetail& det = m_collisionDetails[i];
		hkShapeType typeB = det.m_collidable->getShape()->getType();

		hkCollisionDispatcher::GetPenetrationsFunc getPenetrationsFunc = dispatcher->getGetPenetrationsFunc( m_collidable.getShape()->getType(), typeB );
		(*getPenetrationsFunc)( m_collidable, *det.m_collidable, *input, collector );
		if ( collector.getEarlyOut() )
		{
			break;
		}
	}
}

hkBool hkSimpleShapePhantom::isOverlappingCollidableAdded( hkCollidable* collidable )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	for( int i = 0; i<m_collisionDetails.getSize(); i++ )
	{
		if( m_collisionDetails[i].m_collidable == collidable )
		{
			return true;
		}
	}
	return false;
}

void hkSimpleShapePhantom::addOverlappingCollidable( hkCollidable* collidable )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );

	if ( collidable->getShape() != HK_NULL )
	{
		hkCollidableAccept accept = fireCollidableAdded( collidable );

		if ( accept == HK_COLLIDABLE_ACCEPT  )
		{
			hkCollisionDetail& det = m_collisionDetails.expandOne();
			det.m_collidable = collidable;
		}
	}
}

void hkSimpleShapePhantom::removeOverlappingCollidable( hkCollidable* collidable )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );

	if ( collidable->getShape() != HK_NULL )
	{
		//
		//	We are not checking the filter, so our implementation is a little bit
		//  forgiving, if people change filters on the fly
		//
		for ( int i = m_collisionDetails.getSize() - 1; i >= 0; i-- )
		{
			if ( m_collisionDetails[i].m_collidable == collidable )
			{
				fireCollidableRemoved( collidable, true );
				m_collisionDetails.removeAt( i );
				return;
			}
		}
		fireCollidableRemoved( collidable, false );
	}
}

void hkSimpleShapePhantom::calcStatistics( hkStatisticsCollector* collector ) const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	collector->beginObject("SimplePhantm", collector->MEMORY_INSTANCE, this);
	hkPhantom::calcStatistics(collector);
	collector->addArray("OverlapPtr", collector->MEMORY_RUNTIME, m_collisionDetails);
	collector->endObject();
}

void hkSimpleShapePhantom::deallocateInternalArrays()
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	// Need to deallocate any arrays that are 0 size
	// else warn user that they should call the in place destructor

	// Collision Details
	if (m_collisionDetails.getSize() == 0)
	{
		m_collisionDetails.clearAndDeallocate();
	}
	else
	{
		HK_WARN(0x234f224e, "Phantom at address " << this << " has non-zero m_collisionDetails array.\nPlease call in-place destructor to deallocate.\n");
	}

	hkShapePhantom::deallocateInternalArrays();
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
