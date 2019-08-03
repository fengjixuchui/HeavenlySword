/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
//

#include <hkcollide/hkCollide.h>
#include <hkinternal/collide/mopp/code/hkMoppCode.h>
#include <hkcollide/shape/mopp/modifiers/hkRemoveTerminalsMoppModifier.h>
#include <hkinternal/collide/mopp/machine/hkMoppMachine.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>

hkRemoveTerminalsMoppModifier::hkRemoveTerminalsMoppModifier( const hkMoppCode* moppCode, const hkShapeCollection* shapeCollection, hkArray<hkShapeKey>& shapesToRemove )
{
	m_tempShapesToRemove = &shapesToRemove;
	
	//
	//	Calc the aabb of all nodes
	//
	hkAabb aabb;
	{
		const hkReal tolerance = 0.0f;

		hkShapeCollection::ShapeBuffer shapeBuffer;

		if ( !shapesToRemove.getSize() )
		{
			return;
		}
		{
			hkShapeKey key = shapesToRemove[0];
			const hkShape* childShape = shapeCollection->getChildShape( key, shapeBuffer );
			childShape->getAabb( hkTransform::getIdentity(), tolerance, aabb );
		}
		
		for ( int i = 1; i < shapesToRemove.getSize(); i++)
		{
			hkShapeKey key = shapesToRemove[i];
			const hkShape* childShape = shapeCollection->getChildShape( key, shapeBuffer );

			hkAabb localAabb;
			childShape->getAabb(  hkTransform::getIdentity(), tolerance, localAabb );
			aabb.m_min.setMin4( aabb.m_min, localAabb.m_min );
			aabb.m_max.setMax4( aabb.m_max, localAabb.m_max );
		}
	}

	hkMoppModifyVirtualMachine_queryAabb( moppCode, aabb, this);
}

hkRemoveTerminalsMoppModifier::~hkRemoveTerminalsMoppModifier()
{

}

void hkRemoveTerminalsMoppModifier::applyRemoveTerminals( hkMoppCode* moppCode )
{
	for (int i = 0; i < m_removeInfo.getSize(); i++)
	{
		hkMoppRemoveInfo& rm = m_removeInfo[i];
		hkUchar* program = const_cast<hkUchar*>(&moppCode->m_data[0]) + rm.m_moppOffset;
		rm.m_originalMoppCode = *program;
		*program = 0;
	}
}

void hkRemoveTerminalsMoppModifier::undoRemoveTerminals( hkMoppCode* moppCode )
{
	for (int i = 0; i < m_removeInfo.getSize(); i++)
	{
		hkMoppRemoveInfo& rm = m_removeInfo[i];
		hkUchar* program = const_cast<hkUchar*>(&moppCode->m_data[0]) + rm.m_moppOffset;
		HK_ASSERT2(0x317ca32c,  *program == 0, "Inconsistence use of undoRemoveTerminals, maybe called twice?" );

		*program = rm.m_originalMoppCode;
	}
}

			// hkMoppModifier interface implementation
hkBool hkRemoveTerminalsMoppModifier::shouldTerminalBeRemoved( unsigned int id, const unsigned int *properties )
{
	int find = m_tempShapesToRemove->indexOf( id );
	return find >=0;
}

			// hkMoppModifier interface implementation
void hkRemoveTerminalsMoppModifier::addTerminalRemoveInfo( int relativeMoppAddress )
{
	hkMoppRemoveInfo& rm = m_removeInfo.expandOne();
	rm.m_moppOffset = relativeMoppAddress;
	rm.m_originalMoppCode = 0;
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
