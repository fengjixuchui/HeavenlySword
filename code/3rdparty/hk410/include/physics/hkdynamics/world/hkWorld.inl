/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


inline const hkProcessCollisionInput* hkWorld::getCollisionInput() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_collisionInput;
}

inline hkSolverInfo* hkWorld::getSolverInfo()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	return &m_dynamicsStepInfo.m_solverInfo;
}



inline hkProcessCollisionInput* hkWorld::getCollisionInput()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_collisionInput;
}


inline hkBroadPhase* hkWorld::getBroadPhase() 
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	return m_broadPhase;
}

inline const hkBroadPhase* hkWorld::getBroadPhase() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_broadPhase;
}


inline hkCollisionDispatcher* hkWorld::getCollisionDispatcher() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_collisionDispatcher;
}

inline const hkVector4& hkWorld::getGravity() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_gravity;
}

inline hkTime hkWorld::getCurrentTime() const 
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_simulation->getCurrentTime();
}

inline hkTime hkWorld::getCurrentPsiTime() const 
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_simulation->getCurrentPsiTime();
}

hkRigidBody* hkWorld::getFixedRigidBody()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	return m_fixedRigidBody;
}

const hkRigidBody* hkWorld::getFixedRigidBody() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_fixedRigidBody;
}

inline const hkSimulationIsland* hkWorld::getFixedIsland() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_fixedIsland;
}


inline const hkArray<hkSimulationIsland*>& hkWorld::getInactiveSimulationIslands() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_inactiveSimulationIslands;
}

const hkArray<hkSimulationIsland*>& hkWorld::getActiveSimulationIslands() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkAccessGetActiveSimulationIslands();
#endif
	return m_activeSimulationIslands;
}

inline const hkArray<hkPhantom*>& hkWorld::getPhantoms() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_phantoms;
}

inline const hkCollisionFilter* hkWorld::getCollisionFilter() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_collisionFilter;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Deactivation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline hkReal hkWorld::getHighFrequencyDeactivationPeriod() const
{
	return m_highFrequencyDeactivationPeriod;
}


inline hkReal hkWorld::getLowFrequencyDeactivationPeriod() const
{
	return m_lowFrequencyDeactivationPeriod;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Locking the world, and delaying worldOperations
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void hkWorld::lockCriticalOperations()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	m_criticalOperationsLockCount++;
	HK_ASSERT2(0xad000203, m_criticalOperationsLockCount <= 100, "Internal Error: m_criticalOperationsLockCount corrupted (count went above 100)");
}

void hkWorld::unlockCriticalOperations()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	m_criticalOperationsLockCount--;
	HK_ASSERT2(0xad000202, m_criticalOperationsLockCount >= 0, "Internal Error: m_criticalOperationsLockCount corrupted (count went below 0)");
}

int hkWorld::areCriticalOperationsLocked( ) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	HK_ASSERT2(0xad000200, m_criticalOperationsAllowed, "Warning: areCriticalOperationsLocked() queried while critical operations are disallowed (... and this query is (very probably) only performed when executing critical operations)");
	return m_criticalOperationsLockCount;
}

int hkWorld::areCriticalOperationsLockedUnchecked( ) const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	return m_criticalOperationsLockCount;
}


void hkWorld::lockCriticalOperationsForPhantoms()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	m_criticalOperationsLockCountForPhantoms++;
	HK_ASSERT2(0, m_criticalOperationsLockCountForPhantoms == -1 || m_criticalOperationsLockCountForPhantoms == 0, "Internal Error: m_criticalOperationsLockCountForPhantoms corrupted.");
}

void hkWorld::unlockCriticalOperationsForPhantoms()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	m_criticalOperationsLockCountForPhantoms--;
	HK_ASSERT2(0, m_criticalOperationsLockCountForPhantoms == -1 || m_criticalOperationsLockCountForPhantoms == 0, "Internal Error: m_criticalOperationsLockCountForPhantoms corrupted.");
}

int hkWorld::areCriticalOperationsLockedForPhantoms() const
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RO );
	HK_ASSERT2(0xad000200, m_criticalOperationsAllowed, "Warning: areCriticalOperationsLocked() queried while critical operations are disalloed (... and this query is (very probably) only performed when executing critical operations)");
	HK_ASSERT(0, m_criticalOperationsLockCount + m_criticalOperationsLockCountForPhantoms >= 0);
	return m_criticalOperationsLockCount + m_criticalOperationsLockCountForPhantoms;
}


void hkWorld::blockExecutingPendingOperations(hkBool block)
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0xad000400, m_blockExecutingPendingOperations ^ block, "Internal Error: blocking/unblocking executing pending operations performed multiple times...");
	m_blockExecutingPendingOperations = block;
}

void hkWorld::attemptToExecutePendingOperations()
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	if (/*!areCriticalOperationsLocked()*/ !m_criticalOperationsLockCount && m_pendingOperationsCount && !m_blockExecutingPendingOperations)
	{
		internal_executePendingOperations();
	}
}

void hkWorld::unlockAndAttemptToExecutePendingOperations()
{
	unlockCriticalOperations();
	attemptToExecutePendingOperations();
}

//
// Debugging utility: monitoring of critical operations executions
//
void hkWorld::allowCriticalOperations(hkBool allow)
{
	HK_ACCESS_CHECK_OBJECT( this, HK_ACCESS_RW );
	HK_ASSERT2(0xad000201, m_criticalOperationsAllowed ^ allow, "Internal Error: disabled/enabled critical operations multiple times...");
	m_criticalOperationsAllowed = allow;
}

void hkWorld::setMultithreadedAccessChecking( MtAccessChecking accessCheckState )
{
	if ( MT_ACCESS_CHECKING_ENABLED == accessCheckState )
	{
		m_multiThreadLock.enableChecks();
	}
	else
	{
		m_multiThreadLock.disableChecks();
	}
}

hkWorld::MtAccessChecking hkWorld::getMultithreadedAccessChecking() const
{
	if ( m_multiThreadLock.isCheckingEnabled() )
	{
		return MT_ACCESS_CHECKING_ENABLED;
	}
	else
	{
		return MT_ACCESS_CHECKING_DISABLED;
	}
}


inline void hkWorld::markForRead( ) const
{
	m_multiThreadLock.markForRead();
}

inline void hkWorld::markForWrite( )
{
	m_multiThreadLock.markForWrite();
	m_broadPhase->markForWrite();
}

inline void hkWorld::unmarkForRead( ) const
{
	m_multiThreadLock.unmarkForRead();
}

inline void hkWorld::unmarkForWrite()
{
	m_broadPhase->unmarkForWrite();
	m_multiThreadLock.unmarkForWrite();
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
