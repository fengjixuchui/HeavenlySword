/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/castutil/hkWorldRayCaster.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkRayHitCollector.h>
#include <hkcollide/filter/hkCollisionFilter.h>
#include <hkinternal/collide/broadphase/hkBroadPhase.h>


hkReal 	hkWorldRayCaster::addBroadPhaseHandle( const hkBroadPhaseHandle* broadPhaseHandle, int castIndex )
{
	const hkCollidable* col = static_cast<hkCollidable*>( static_cast<const hkTypedBroadPhaseHandle*>(broadPhaseHandle)->getOwner() );
	const hkShape* shape = col->getShape();

	// phantoms do not have shapes
	hkRayHitCollector* collector = hkAddByteOffset( m_collectorBase, m_collectorStriding * castIndex );
	if (shape)
	{
		const hkWorldRayCastInput& input = m_input[castIndex];
		if ( m_filter->isCollisionEnabled( input, *col ) )
		{
			const hkTransform& trans = col->getTransform();

			// use the from from the first input and the too from the real input
			m_shapeInput.m_from._setTransformedInversePos( trans, m_input->m_from);
			m_shapeInput.m_to.  _setTransformedInversePos( trans, input.m_to);
			m_shapeInput.m_filterInfo = input.m_filterInfo;

			shape->castRayWithCollector( m_shapeInput, *col, *collector);
		}
	}
	return collector->m_earlyOutHitFraction;
}



void hkWorldRayCaster::castRay( const hkBroadPhase& broadphase, const hkWorldRayCastInput& input, const hkCollisionFilter* filter, hkRayHitCollector& collector )
{
	HK_ASSERT2(0x39256aff,  filter, "You need to specify a valid filter");
	HK_ASSERT2(0x20260c7c,  collector.m_earlyOutHitFraction == 1.0f, "Your collector has not been reset");

	HK_TIMER_BEGIN("RayCast", HK_NULL);
	m_input = &input;
	m_collectorBase = &collector;
	m_collectorStriding = 0;
	m_filter = filter;

	if ( input.m_enableShapeCollectionFilter )
	{
		m_shapeInput.m_rayShapeCollectionFilter = filter;
	}
	else
	{
		m_shapeInput.m_rayShapeCollectionFilter = HK_NULL;
	}

	hkBroadPhase::hkCastRayInput rayInput;
	rayInput.m_from = input.m_from;
	rayInput.m_toBase = &input.m_to;

	broadphase.castRay( rayInput, this, 0 );
	HK_TIMER_END();
}

void hkWorldRayCaster::castRay( const hkBroadPhase& broadphase, const hkWorldRayCastInput& input, const hkCollisionFilter* filter, hkBroadPhaseAabbCache* cache, hkRayHitCollector& collector )
{
	HK_ASSERT2(0x4527a42f,  filter, "You need to specify a valid filter");
	HK_ASSERT2(0x381faadf,  collector.m_earlyOutHitFraction == 1.0f, "Your collector has not been reset");

	HK_TIMER_BEGIN("RayCstCached", HK_NULL);
	m_input = &input;
	m_collectorBase = &collector;
	m_collectorStriding = 0;
	m_filter = filter;

	if ( input.m_enableShapeCollectionFilter )
	{
		m_shapeInput.m_rayShapeCollectionFilter = filter;
	}
	else
	{
		m_shapeInput.m_rayShapeCollectionFilter = HK_NULL;
	}

	hkBroadPhase::hkCastRayInput rayInput;
	rayInput.m_from = input.m_from;
	rayInput.m_toBase = &input.m_to;
	rayInput.m_aabbCacheInfo = cache;

	broadphase.castRay( rayInput, this, 0 );
	HK_TIMER_END();
}

void hkWorldRayCaster::castRayGroup( hkBroadPhase& broadphase, const hkWorldRayCastInput* inputArray, int numRays, const hkCollisionFilter* filter, hkRayHitCollector* collectorBase, int collectorStriding ) 
{
	HK_TIMER_BEGIN("RayCastGroup", HK_NULL);
	hkAabb aabb;
	{
		aabb.m_min.setMin4( inputArray->m_from, inputArray->m_to );
		aabb.m_max.setMax4( inputArray->m_from, inputArray->m_to );

		const hkWorldRayCastInput* in = inputArray;
		in++;
		for ( int x = numRays-2; x>=0; x--)
		{
			aabb.m_min.setMin4( aabb.m_min, in->m_to );
			aabb.m_min.setMin4( aabb.m_min, in->m_from );

			aabb.m_max.setMax4( aabb.m_max, in->m_to );
			aabb.m_max.setMax4( aabb.m_max, in->m_from );
			in++;
		}
	}
	int cacheSize = broadphase.getAabbCacheSize();
	hkBroadPhaseAabbCache* cache = static_cast<hkBroadPhaseAabbCache*>(hkThreadMemory::getInstance().allocateStack( cacheSize ));

	broadphase.calcAabbCache( aabb, cache );
	//
	//	Cast rays
	//
	{
		const hkWorldRayCastInput* in = inputArray;
		hkRayHitCollector* collector = collectorBase;
		for ( int x = numRays-1; x>=0; x--)
		{
			castRay( broadphase, *in, filter, cache, *collector);
			in++;
			collector = hkAddByteOffset( collector, collectorStriding );
		}
	}
	hkThreadMemory::getInstance().deallocateStack( cache );
	HK_TIMER_END();
}

void hkWorldRayCaster::castRaysFromSinglePoint( hkBroadPhase& broadphase, const hkWorldRayCastInput* inputArray, int numRays, const hkCollisionFilter* filter, hkBroadPhaseAabbCache* cache, hkRayHitCollector* collectorBase, int collectorStriding )
{
	HK_ASSERT2(0x6fe88ced,  filter || !inputArray->m_enableShapeCollectionFilter, "You need to specify a valid filter");

	HK_TIMER_BEGIN("RayCastFSP", HK_NULL);
	m_input = inputArray;
	m_filter = filter;
	m_collectorBase = collectorBase;
	m_collectorStriding = collectorStriding;

	if ( inputArray->m_enableShapeCollectionFilter )
	{
		m_shapeInput.m_rayShapeCollectionFilter = filter;
	}
	else
	{
		m_shapeInput.m_rayShapeCollectionFilter = HK_NULL;
	}

	hkBroadPhase::hkCastRayInput rayInput;
	rayInput.m_from = inputArray->m_from;
	rayInput.m_toBase = &inputArray->m_to;
	rayInput.m_toStriding = hkSizeOf( hkWorldRayCastInput );
	rayInput.m_aabbCacheInfo = cache;
	rayInput.m_numCasts = numRays;

	broadphase.castRay( rayInput, this, 0 );
	HK_TIMER_END();
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
