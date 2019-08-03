//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/havokthreadutils.cpp
//!	
//---------------------------------------------------------------------------------------------------------

#include "physics/havokthreadutils.h"

#include "physics/world.h"

#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkWorldObject.h>


Physics::WriteAccess::WriteAccess()
:	m_WorldObj( NULL )
{
	Physics::CPhysicsWorld::Get().m_pobHavokWorld->markForWrite();
}

Physics::WriteAccess::WriteAccess( hkWorldObject *world_obj )
:	m_WorldObj( world_obj )
{
	world_obj->getMultiThreadLock().markForWrite();
}

Physics::WriteAccess::~WriteAccess()
{
	if ( m_WorldObj == NULL )
	{
		Physics::CPhysicsWorld::Get().m_pobHavokWorld->unmarkForWrite();
	}
	else
	{
		m_WorldObj->getMultiThreadLock().unmarkForWrite();
	}	
}

Physics::ReadAccess::ReadAccess()
:	m_WorldObj( NULL )
{
#	ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::CPhysicsWorld::Get().m_pobHavokWorld->markForRead();
#	endif
}

Physics::ReadAccess::ReadAccess( hkWorldObject *world_obj )
:	m_WorldObj( world_obj )
{
#	ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		world_obj->getMultiThreadLock().markForRead();
#	endif
}

Physics::ReadAccess::~ReadAccess()
{
#	ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if ( m_WorldObj == NULL )
		{
			Physics::CPhysicsWorld::Get().m_pobHavokWorld->unmarkForRead();
		}
		else
		{
			m_WorldObj->getMultiThreadLock().unmarkForRead();
		}
#	endif
}

