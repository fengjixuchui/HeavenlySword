/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkMotion* hkEntity::getMotion()
{
	return &m_motion;
}

inline hkSimulationIsland* hkEntity::getSimulationIsland() const
{	
	return m_simulationIsland;
}

hkAction* hkEntity::getAction(int i)
{
	return m_actions[i];
}

int hkEntity::getNumActions() const
{
	return m_actions.getSize();
}


inline const hkArray<hkEntityListener*>& hkEntity::getEntityListeners() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadOnly();
#endif
	return m_entityListeners;
}

inline const hkArray<hkEntityActivationListener*>& hkEntity::getEntityActivationListeners() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadOnly();
#endif
	return m_activationListeners;
}

inline const hkArray<hkCollisionListener*>& hkEntity::getCollisionListeners() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadOnly();
#endif
	return m_collisionListeners;
}

inline hkUint16 hkEntity::getProcessContactCallbackDelay() const
{
	return m_processContactCallbackDelay;
}

inline void hkEntity::setProcessContactCallbackDelay( hkUint16 delay )
{
	m_processContactCallbackDelay = delay;
}

inline hkMaterial& hkEntity::getMaterial()
{
	return m_material;
}

inline hkLinkedCollidable* hkEntity::getLinkedCollidable()
{
#ifdef HK_DEBUG_MULTI_THREADING
	checkReadOnly();
#endif
	return &m_collidable;
}

inline const hkMaterial& hkEntity::getMaterial() const
{
	return m_material;
}

inline hkEntityDeactivator* hkEntity::getDeactivator()
{
	return m_deactivator;
}

inline const hkEntityDeactivator* hkEntity::getDeactivator() const
{
	return m_deactivator;
}

inline hkBool hkEntity::isFixed() const
{
	return m_motion.m_type == hkMotion::MOTION_FIXED;
}

hkBool hkEntity::isFixedOrKeyframed() const
{
	return (m_motion.m_type == hkMotion::MOTION_FIXED) || (m_motion.m_type == hkMotion::MOTION_KEYFRAMED);
}

inline hkUint32 hkEntity::getUid() const
{
	return m_uid;
}

const hkArray<struct hkConstraintInternal>&  hkEntity::getConstraintMasters() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	return getConstraintMastersImpl();
#else
	return m_constraintsMaster;
#endif
}

hkArray<struct hkConstraintInternal>&  hkEntity::getConstraintMastersRw()
{
#ifdef HK_DEBUG_MULTI_THREADING
	return getConstraintMastersRwImpl();
#else
	return m_constraintsMaster;
#endif
}

const hkArray<class hkConstraintInstance*>&  hkEntity::getConstraintSlaves() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	return getConstraintSlavesImpl();
#else
	return m_constraintsSlave;
#endif
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
