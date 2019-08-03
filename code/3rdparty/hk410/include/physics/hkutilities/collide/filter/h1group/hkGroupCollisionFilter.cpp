/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/agent/hkCollidable.h>
#include <hkdynamics/entity/hkEntity.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/castutil/hkWorldRayCastInput.h>
#include <hkutilities/collide/filter/h1group/hkGroupCollisionFilter.h>
#include <hkbase/class/hkTypeInfo.h>
#include <hkcollide/shape/hkShapeContainer.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkGroupCollisionFilter);

hkGroupCollisionFilter::hkGroupCollisionFilter()
: m_noGroupCollisionEnabled(true)
{
	// Initially disable all collision groups
	for (int i=0; i<32; i++)
	{
		m_collisionGroups[i] = 0;
	}
	HK_WARN_ONCE(0x74653015, "hkGroupCollisionFilter is deprecated, please use hkcollide/util/filter/group/hkGroupFilter instead");
}

hkBool hkGroupCollisionFilter::isCollisionEnabled(hkUint32 groupA, hkUint32 groupB) const
{
	if ( (groupA==0) || (groupB==0) )
	{
		return m_noGroupCollisionEnabled;
	}

	unsigned int supersetA = 0;


	if ( groupA & 0x000000ff )
	{
		supersetA |= (groupA & 0x00000001) ? m_collisionGroups[0] : 0x0;
		supersetA |= (groupA & 0x00000002) ? m_collisionGroups[1] : 0x0;
		supersetA |= (groupA & 0x00000004) ? m_collisionGroups[2] : 0x0;
		supersetA |= (groupA & 0x00000008) ? m_collisionGroups[3] : 0x0;
		supersetA |= (groupA & 0x00000010) ? m_collisionGroups[4] : 0x0;
		supersetA |= (groupA & 0x00000020) ? m_collisionGroups[5] : 0x0;
		supersetA |= (groupA & 0x00000040) ? m_collisionGroups[6] : 0x0;
		supersetA |= (groupA & 0x00000080) ? m_collisionGroups[7] : 0x0;
	}
	if ( groupA & 0x0000ff00 )
	{
		supersetA |= (groupA & 0x00000100) ? m_collisionGroups[8] : 0x0;
		supersetA |= (groupA & 0x00000200) ? m_collisionGroups[9] : 0x0;
		supersetA |= (groupA & 0x00000400) ? m_collisionGroups[10] : 0x0;
		supersetA |= (groupA & 0x00000800) ? m_collisionGroups[11] : 0x0;
		supersetA |= (groupA & 0x00001000) ? m_collisionGroups[12] : 0x0;
		supersetA |= (groupA & 0x00002000) ? m_collisionGroups[13] : 0x0;
		supersetA |= (groupA & 0x00004000) ? m_collisionGroups[14] : 0x0;
		supersetA |= (groupA & 0x00008000) ? m_collisionGroups[15] : 0x0;
	}
	if ( groupA & 0x00ff0000 )
	{
		supersetA |= (groupA & 0x00010000) ? m_collisionGroups[16] : 0x0;
		supersetA |= (groupA & 0x00020000) ? m_collisionGroups[17] : 0x0;
		supersetA |= (groupA & 0x00040000) ? m_collisionGroups[18] : 0x0;
		supersetA |= (groupA & 0x00080000) ? m_collisionGroups[19] : 0x0;
		supersetA |= (groupA & 0x00100000) ? m_collisionGroups[20] : 0x0;
		supersetA |= (groupA & 0x00200000) ? m_collisionGroups[21] : 0x0;
		supersetA |= (groupA & 0x00400000) ? m_collisionGroups[22] : 0x0;
		supersetA |= (groupA & 0x00800000) ? m_collisionGroups[23] : 0x0;
	}
	if ( groupA & 0xff000000 )
	{
		supersetA |= (groupA & 0x01000000) ? m_collisionGroups[24] : 0x0;
		supersetA |= (groupA & 0x02000000) ? m_collisionGroups[25] : 0x0;
		supersetA |= (groupA & 0x04000000) ? m_collisionGroups[26] : 0x0;
		supersetA |= (groupA & 0x08000000) ? m_collisionGroups[27] : 0x0;
		supersetA |= (groupA & 0x10000000) ? m_collisionGroups[28] : 0x0;
		supersetA |= (groupA & 0x20000000) ? m_collisionGroups[29] : 0x0;
		supersetA |= (groupA & 0x40000000) ? m_collisionGroups[30] : 0x0;
		supersetA |= (groupA & 0x80000000) ? m_collisionGroups[31] : 0x0;
	}	
	return (supersetA & groupB)!=0;
}

hkBool hkGroupCollisionFilter::isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const
{
	return isCollisionEnabled( a.getCollisionFilterInfo(), b.getCollisionFilterInfo() );
}

hkBool hkGroupCollisionFilter::isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const
{
	hkUint32 infoB = bContainer.getCollisionFilterInfo( bKey );
	return isCollisionEnabled( a.getRootCollidable()->getCollisionFilterInfo(), infoB );
}

hkBool hkGroupCollisionFilter::isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const 
{
	hkUint32 infoB = bContainer.getCollisionFilterInfo( bKey );
	return isCollisionEnabled( aInput.m_filterInfo, infoB );
}

hkBool hkGroupCollisionFilter::isCollisionEnabled( const hkWorldRayCastInput& aInput, const hkCollidable& collidableB ) const
{
	return isCollisionEnabled( aInput.m_filterInfo, collidableB.getCollisionFilterInfo() );
}



void hkGroupCollisionFilter::enableCollisionGroups(hkUint32 groupBitsA, hkUint32 groupBitsB)
{
	if ( groupBitsA == 0 && groupBitsB == 0)
	{
		m_noGroupCollisionEnabled = true;
		return;
	}
	for (int i=0; i< 32; i++)
	{
		int b = 1<<i;
		if ( b & groupBitsA )
		{
			m_collisionGroups[i] |= groupBitsB;
		}
		if ( b & groupBitsB )
		{
			m_collisionGroups[i] |= groupBitsA;
		}
	}
}

void hkGroupCollisionFilter::disableCollisionGroups(hkUint32 groupBitsA, hkUint32 groupBitsB)
{
	if ( groupBitsA == 0 && groupBitsB == 0)
	{
		m_noGroupCollisionEnabled = false;
		return;
	}

	for (int i=0; i< 32; i++)
	{
		int b = 1<<i;
		if ( b & groupBitsA )
		{
			m_collisionGroups[i] &= ~groupBitsB;
		}
		if ( b & groupBitsB )
		{
			m_collisionGroups[i] &= ~groupBitsA;
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
