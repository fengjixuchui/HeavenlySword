/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>

#include <hkdynamics/world/util/hkWorldAgentUtil.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkSimulationIsland.h>

#include <hkdynamics/world/commandqueue/hkPhysicsCommandQueue.h>

#define SIMPLE( ENUM_TYPE, STRUCT_TYPE, EXEC )								\
		case hkPhysicsCommand::ENUM_TYPE:									\
		{																	\
			STRUCT_TYPE* com = static_cast<STRUCT_TYPE*>(begin);			\
			EXEC;															\
			const int size = HK_NEXT_MULTIPLE_OF( 16, sizeof( STRUCT_TYPE ) );\
			begin = hkAddByteOffset( begin, size );							\
			break;															\
		}

static void addConstraintToCriticalLockedIsland( hkWorld* world, hkConstraintInstance* constraint, int callbackRequest)
{
	hkWorldOperationUtil::addConstraintToCriticalLockedIsland( world, constraint );
	constraint->m_internal->m_callbackRequest |= callbackRequest;
}

void HK_CALL hkPhysicsCommandMachineProcess( hkWorld* world, hkPhysicsCommand* begin, hkPhysicsCommand* end )
{
	while ( begin < end )
	{
		switch( begin->m_type )
		{
			SIMPLE( TYPE_ADD_CONSTRAINT_TO_LOCKED_ISLAND,      hkAddConstraintToCriticalLockedIslandPhysicsCommand,      addConstraintToCriticalLockedIsland( world, com->m_object0, com->m_object1) );
			SIMPLE( TYPE_REMOVE_CONSTRAINT_FROM_LOCKED_ISLAND, hkRemoveConstraintFromCriticalLockedIslandPhysicsCommand, hkWorldOperationUtil::removeConstraintFromCriticalLockedIsland( world, com->m_object) );

			default:
				HK_ASSERT2( 0xf02ddd12, false, "Unknown hkPhysicsCommand type");
				return;
		}
	}
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
