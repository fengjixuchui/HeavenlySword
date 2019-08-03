/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


inline hkEntity* hkConstraintInstance::getEntityA() const
{
	return m_entities[0];
}

inline hkEntity* hkConstraintInstance::getEntityB() const
{
	return m_entities[1];
}


inline hkRigidBody* hkConstraintInstance::getRigidBodyA() const
{
	return reinterpret_cast<hkRigidBody*>(m_entities[0]);
}

inline hkRigidBody* hkConstraintInstance::getRigidBodyB() const
{
	return reinterpret_cast<hkRigidBody*>(m_entities[1]);
}



inline hkEntity* hkConstraintInternal::getMasterEntity() const
{
	return m_entities[m_whoIsMaster];
}

inline hkEntity* hkConstraintInternal::getSlaveEntity() const
{
	return m_entities[1-m_whoIsMaster];
}

inline hkEntity* hkConstraintInstance::getMasterEntity() const
{
	HK_ASSERT2( 0xf056d145, m_internal, "You cannot access the master entity, this constraint is not added to the world yet" );
	return m_internal->getMasterEntity();
}

inline hkEntity* hkConstraintInstance::getSlaveEntity() const
{
	HK_ASSERT2( 0xf056d145, m_internal, "You cannot access the slave entity, this constraint is not added to the world yet" );
	return m_internal->getSlaveEntity();
}

inline void hkConstraintInstance::setWantRuntime( hkBool b )
{
	HK_ASSERT2( 0xf03de567, HK_NULL == m_owner, "You cannot call setWantRuntime after you have added the constraint to the world" );
	m_wantRuntime = b;
}

inline hkBool hkConstraintInstance::getWantRuntime() const
{
	return m_wantRuntime;
}

inline hkEntity* hkConstraintInternal::getOtherEntity( const hkEntity* entity ) const
{
	hkUlong a = hkUlong( m_entities[0] );
	hkUlong b = hkUlong( m_entities[1] );
	hkUlong c = hkUlong( entity );
	return reinterpret_cast<hkEntity*>( a^b^c );
}

inline hkConstraintOwner* hkConstraintInstance::getOwner() const
{
	return m_owner;
}

void hkConstraintInstance::setOwner( hkConstraintOwner* owner )
{
	m_owner = owner;
}

inline hkConstraintData* hkConstraintInstance::getData() const
{
	return m_data;
}

inline hkConstraintInternal* hkConstraintInstance::getInternal()
{
	HK_ASSERT2( 0xf056d145, m_internal, "You cannot access internal, this constraint is not added to the world yet" );
	return m_internal;
}

inline hkConstraintRuntime* hkConstraintInstance::getRuntime() const
{
	HK_ASSERT2( 0xf056d145, m_internal, "You cannot access internal, this constraint is not added to the world yet" );
	return m_internal->m_runtime;
}


inline hkUint32 hkConstraintInstance::getUserData() const
{
	return m_userData;
}

inline void hkConstraintInstance::setUserData( hkUint32 data )
{
	//HK_ACCESS_CHECK_WITH_PARENT( m_entities[0]->getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	m_userData = data;
}

inline const char* hkConstraintInstance::getName() const
{
	return m_name;
}

inline void hkConstraintInstance::setName( const char* name )
{
	m_name = name;
}

inline hkConstraintInstance::ConstraintPriority hkConstraintInstance::getPriority() const
{
	return m_priority;
}

inline hkEntity* hkConstraintInstance::getOtherEntity( const hkEntity* entity )
{
	return hkSelectOther( const_cast<hkEntity*>(entity), m_entities[0], m_entities[1]);
}

void hkConstraintInternal::getConstraintInfo( hkConstraintInfo& info ) const
{
	info.m_maxSizeOfJacobians  = m_sizeOfJacobians;
	info.m_sizeOfJacobians     = m_sizeOfJacobians;
	info.m_sizeOfSchemas       = m_sizeOfSchemas;
	info.m_numSolverResults    = m_numSolverResults;
}

void hkConstraintInternal::clearConstraintInfo( )
{
#ifdef HK_STORE_CONSTRAINT_INFO_IN_INTERNAL
	this->m_numSolverResults = 0;
	this->m_sizeOfJacobians  = 0;
	this->m_sizeOfSchemas    = 0;
#endif
}

void hkConstraintInternal::addConstraintInfo( const hkConstraintInfo& delta)
{
	const hkUint32 numSolverResults = this->m_numSolverResults + delta.m_numSolverResults;
	const hkUint32 sizeOfSchemas    = this->m_sizeOfSchemas    + delta.m_sizeOfSchemas;
	const hkUint32 sizeOfJacobians  = this->m_sizeOfJacobians  + delta.m_sizeOfJacobians;

	HK_ASSERT2( 0xf0ff3244, numSolverResults < 0x10000, "Constraint too big for system" );
	HK_ASSERT2( 0xf0ff3245, sizeOfJacobians  < 0x10000, "Constraint too big for system" );
	HK_ASSERT2( 0xf0ff3246, sizeOfSchemas    < 0x10000, "Constraint too big for system" );

	this->m_numSolverResults = hkUint16(numSolverResults);
	this->m_sizeOfJacobians  = hkUint16(sizeOfJacobians);
	this->m_sizeOfSchemas    = hkUint16(sizeOfSchemas);
}

void hkConstraintInternal::subConstraintInfo( const hkConstraintInfo& delta)
{
	const hkUint32 numSolverResults = this->m_numSolverResults - delta.m_numSolverResults;
	const hkUint32 sizeOfSchemas    = this->m_sizeOfSchemas    - delta.m_sizeOfSchemas;
	const hkUint32 sizeOfJacobians  = this->m_sizeOfJacobians  - delta.m_sizeOfJacobians;

	HK_ASSERT2( 0xf0fe3244, numSolverResults < 0x10000, "Constraint internal inconsistency" );
	HK_ASSERT2( 0xf0fe3245, sizeOfJacobians  < 0x10000, "Constraint internal inconsistency" );
	HK_ASSERT2( 0xf0fe3246, sizeOfSchemas    < 0x10000, "Constraint internal inconsistency" );

	this->m_numSolverResults = hkUint16(numSolverResults);
	this->m_sizeOfJacobians  = hkUint16(sizeOfJacobians);
	this->m_sizeOfSchemas    = hkUint16(sizeOfSchemas);
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
