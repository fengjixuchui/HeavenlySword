/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/filter/group/hkGroupFilter.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkcollide/castutil/hkWorldRayCastInput.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkGroupFilter);

hkGroupFilter::hkGroupFilter()
{
	// Initially enable all collision groups
	for (int i=0; i<32; i++)
	{
		m_collisionLookupTable[i] = 0xffffffff;
	}
	m_nextFreeSystemGroup = 0;
}

hkGroupFilter::~hkGroupFilter()
{
}


hkBool hkGroupFilter::isCollisionEnabled(hkUint32 infoA, hkUint32 infoB) const
{
	// If the objects are in the same system group, but not system group 0,
	// then the decision of whether to collide is based exclusively on the 
	// objects' SubSystemId and SubSystemDontCollideWith.
	// Otherwise, the decision is based exclusively on the objects' layers.

	hkUint32 zeroIfSameSystemGroup = (infoA^infoB) & 0xffff0000;

	// check for identical system groups
	if ( zeroIfSameSystemGroup == 0)
	{
		// check whether system group was set (nonzero)
		if ( (infoA & 0xffff0000) != 0 )
		{
			// check whether we allow collisions
			int idA = (infoA >> 5) & 0x1f;
			int dontCollideB = (infoB >> 10) & 0x1f;
			if ( idA == dontCollideB )
			{
				return false;
			}

			int idB = (infoB >> 5) & 0x1f;
			int dontCollideA = (infoA >> 10) & 0x1f;
			if ( idB == dontCollideA )
			{
				return false;
			}
			return true;
		}
	}

	// use the layers to decide
	hkUint32 f = 0x1f;
	hkUint32 layerBitsA = m_collisionLookupTable[ infoA & f ];
	hkUint32 layerBitsB = hkUint32(1 << (infoB & f));

	return 0 != (layerBitsA & layerBitsB);
}


hkBool hkGroupFilter::isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const
{
	return isCollisionEnabled( a.getCollisionFilterInfo(), b.getCollisionFilterInfo() );
}


hkBool hkGroupFilter::isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const
{
	hkUint32 infoB = bContainer.getCollisionFilterInfo( bKey );
	hkUint32 infoA;
	if ( a.getShapeKey() == HK_INVALID_SHAPE_KEY )
	{
		infoA = a.getRootCollidable()->getCollisionFilterInfo();
	}
	else
	{ 
		// If a has a shape key, then two shape collection are colliding with each other,
		// in this case we have to try to reverse engineer the parent of a to get
		// the proper collision filter
		const hkCdBody* p = a.getParent();
		while(1)
		{
			hkShapeType t = p->getShape()->getType();
			if ( input.m_dispatcher->hasAlternateType( t, HK_SHAPE_COLLECTION ) )
			{
				const hkShapeCollection* aCollection = static_cast<const hkShapeCollection*>( p->getShape() );
				infoA = aCollection->getCollisionFilterInfo( a.getShapeKey() );
				break;
			}

			else if ( input.m_dispatcher->hasAlternateType( t, HK_SHAPE_BV_TREE ) )
			{
				const hkBvTreeShape* tree = static_cast<const hkBvTreeShape*>( p->getShape() );
				const hkShapeCollection* aCollection = tree->getShapeCollection();
				infoA = aCollection->getCollisionFilterInfo( a.getShapeKey() );
				break;
			}
			else if ( input.m_dispatcher->hasAlternateType( t, HK_SHAPE_MULTI_SPHERE ) )
			{
				infoA = a.getRootCollidable()->getCollisionFilterInfo();
				break;
			}
			else
			{
				// We disable filtering for convex list shapes, because we do not filter
				// the collisions in the get supporting vertex call, so the filtering will be inconsistent
				if (input.m_dispatcher->hasAlternateType( t, HK_SHAPE_CONVEX_LIST ) )
				{
					return true;
				}
			}

			p = p->getParent();
			HK_ASSERT2(0x23cd067c,  p, "m_shapeKey is set in hkCdBody, but no hkShapeCollection can be found as a parent");
			if ( p )
			{
				continue;
			}
			infoA = 0;
			break;
		}
	}
	return isCollisionEnabled( infoA, infoB );
}

hkBool hkGroupFilter::isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& bShape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const 
{
	hkUint32 infoB = bContainer.getCollisionFilterInfo( bKey );
	return isCollisionEnabled( aInput.m_filterInfo, infoB );
}

hkBool hkGroupFilter::isCollisionEnabled( const hkWorldRayCastInput& aInput, const hkCollidable& collidableB ) const
{
	return isCollisionEnabled( aInput.m_filterInfo, collidableB.getCollisionFilterInfo() );
}


void hkGroupFilter::enableCollisionsUsingBitfield(hkUint32 layerBitsA, hkUint32 layerBitsB)
{
	HK_ASSERT2(0x3c3a0084,  (layerBitsA|layerBitsB) != 0, "layer bits not set");
	for (int i=0; i< 32; i++)
	{
		int b = 1<<i;
		if ( b & layerBitsA )
		{
			m_collisionLookupTable[i] |= layerBitsB;
		}
		if ( b & layerBitsB )
		{
			m_collisionLookupTable[i] |= layerBitsA;
		}
	}
}

void hkGroupFilter::enableCollisionsBetween(int layerA, int layerB )
{
	HK_ASSERT(0x66c2b6fd,  0 <= layerA && layerA < 32 );
	HK_ASSERT(0x5a285631,  0 <= layerB && layerB < 32 );

	m_collisionLookupTable[layerA] |= hkUint32(1 << layerB);
	m_collisionLookupTable[layerB] |= hkUint32(1 << layerA);
}

void hkGroupFilter::disableCollisionsBetween(int layerA, int layerB )
{
	HK_ASSERT(0x2a168aec,  0 <= layerA && layerA < 32 );
	HK_ASSERT(0x234fb60b,  0 <= layerB && layerB < 32 );
	HK_ASSERT2(0x4ab45935,  layerA > 0, "You are not allowed to disable collision of layer 0");
	HK_ASSERT2(0x358c7ccd,  layerB > 0, "You are not allowed to disable collision of layer 0");

	m_collisionLookupTable[layerA] &= hkUint32(~(1 << layerB));
	m_collisionLookupTable[layerB] &= hkUint32(~(1 << layerA));
}

void hkGroupFilter::disableCollisionsUsingBitfield(hkUint32 layerBitsA, hkUint32 layerBitsB)
{
	HK_ASSERT2(0x41c4fad2,  (layerBitsA|layerBitsB) != 0, "layer bits not set");
	HK_ASSERT2(0x49059b77,  (layerBitsA&1) == 0, "You are not allowed to disable collision of layer 0");
	HK_ASSERT2(0x371ca278,  (layerBitsB&1) == 0, "You are not allowed to disable collision of layer 0");
	for (int i=0; i< 32; i++)
	{
		int b = 1<<i;
		if ( b & layerBitsA )
		{
			m_collisionLookupTable[i] &= ~layerBitsB;
		}
		if ( b & layerBitsB )
		{
			m_collisionLookupTable[i] &= ~layerBitsA;
		}
	}
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
