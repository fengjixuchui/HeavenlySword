/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>
#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkcollide/shape/hkShape.h>
#include <hkdynamics/entity/hkEntityDeactivator.h>
#include <hkdynamics/world/util/hkWorldOperationQueue.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkbase/class/hkTypeInfo.h>
#include <hkdynamics/motion/rigid/hkStabilizedSphereMotion.h>
#include <hkdynamics/motion/rigid/hkStabilizedBoxMotion.h>
#include <hkdynamics/motion/rigid/hkFixedRigidMotion.h>
#include <hkdynamics/motion/rigid/hkKeyframedRigidMotion.h>
#include <hkdynamics/motion/rigid/ThinBoxMotion/hkThinBoxMotion.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkEntity);

#if HK_POINTER_SIZE == 4
	// make sure that the size of the entity is below 512. Else we are wasting lots of hkPoolMemory
	HK_COMPILE_TIME_ASSERT( sizeof (hkEntity) <= 0x200 );
#endif

hkEntity::hkEntity( const hkShape* shape )
:	hkWorldObject( shape, BROAD_PHASE_ENTITY )
{
	// This is used by the simulation islands, and is not initialized correctly until the entity has been added to the world.
	m_storageIndex = hkObjectIndex(-1);	
	m_simulationIsland = HK_NULL;
	m_deactivator = HK_NULL;
	m_uid = hkUint32(-1);
	m_solverData = 0;
}

hkEntity::hkEntity( class hkFinishLoadedObjectFlag flag ) :
	hkWorldObject( flag ),
	m_material( flag ),
	m_motion( flag )
{
	if( flag.m_finishing )
	{
		void* motion = &m_motion;
		switch( m_motion.getType() )
		{
			case hkMotion::MOTION_SPHERE_INERTIA:
				new (motion) hkSphereMotion( flag );
				break;
			case hkMotion::MOTION_STABILIZED_SPHERE_INERTIA:
				new (motion) hkStabilizedSphereMotion( flag );
				break;
			case hkMotion::MOTION_BOX_INERTIA:
				new (motion) hkBoxMotion( flag );
				break;
			case hkMotion::MOTION_STABILIZED_BOX_INERTIA:
				new (motion) hkStabilizedBoxMotion( flag );
				break;
			case hkMotion::MOTION_KEYFRAMED:
				new (motion) hkKeyframedRigidMotion( flag );
				break;
			case hkMotion::MOTION_FIXED:
				new (motion) hkFixedRigidMotion( flag );
				break;
			case hkMotion::MOTION_THIN_BOX_INERTIA:
				new (motion) hkThinBoxMotion( flag );
				break;
			default:
				HK_ASSERT(0x5ba44035,0);
		}
	}
}


void hkEntity::calcStatistics( hkStatisticsCollector* collector ) const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );

	collector->beginObject( "Entity", collector->MEMORY_INSTANCE, this );
	{
		//
		hkWorldObject::calcStatistics(collector);

		collector->addChildObject( "SavedMotion", collector->MEMORY_ENGINE,  m_motion.m_savedMotion );

		collector->addChildObject( "Deactivator", collector->MEMORY_ENGINE,  m_deactivator );

		collector->addArray( "CollisionListnr",         collector->MEMORY_ENGINE,  m_collisionListeners );
		collector->addArray( "ActLstnrPtrs",            collector->MEMORY_ENGINE,  m_activationListeners );
		collector->addArray( "ListenerPtrs.",           collector->MEMORY_ENGINE,  m_entityListeners );
	} 
	collector->endObject();
}


hkEntity::~hkEntity()
{	
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );

	HK_ASSERT(0xad000212, m_actions.isEmpty());
	HK_ASSERT(0xad000213, m_constraintsMaster.isEmpty());
	HK_ASSERT(0xad000214, m_constraintsSlave.isEmpty());

	hkEntityCallbackUtil::fireEntityDeleted( this );

	if ( m_deactivator != HK_NULL )
	{
		m_deactivator->removeReference();
	}
	
	// getCollidable()->getShape()->removeReference(); // CK: now done in the hkWorldObject dtor

	HK_ASSERT2(0x140a19b8,  getWorld() == HK_NULL, "removeReference() or destructor called while hkEntity is still in simulation" );
}

void hkEntity::setDeactivator( hkEntityDeactivator* deactivator )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );

	if ( deactivator != HK_NULL )
	{
		deactivator->addReference();
	}

	if ( m_deactivator != HK_NULL )
	{
		m_deactivator->removeReference();
	}

	m_deactivator = deactivator;
}


void hkEntity::addEntityListener( hkEntityListener* el )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	HK_ASSERT2(0x63a5ad5d, m_entityListeners.indexOf( el ) < 0, "You tried to add an entity listener twice" );
	int emptyIndex = m_entityListeners.indexOf( HK_NULL );
	if ( emptyIndex >= 0)
	{
		m_entityListeners[emptyIndex] = el;
	}
	else
	{
		m_entityListeners.pushBack( el );
	}
}

void hkEntity::removeEntityListener( hkEntityListener* el)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	int i = m_entityListeners.indexOf( el );
	HK_ASSERT2(0x79e1d7d7, i >= 0, "You tried to remove an entity listener, which was never added" );
	m_entityListeners[i] = HK_NULL;
}

void hkEntity::addEntityActivationListener( hkEntityActivationListener* el)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	HK_ASSERT2(0x6515b54d, m_activationListeners.indexOf( el ) < 0, "You tried to add an entity activation listener twice" );

	int emptyIndex = m_activationListeners.indexOf( HK_NULL );
	if ( emptyIndex >= 0)
	{
		m_activationListeners[emptyIndex] = el;
	}
	else
	{
		m_activationListeners.pushBack( el );
	}

}

void hkEntity::removeEntityActivationListener( hkEntityActivationListener* el)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	int i = m_activationListeners.indexOf( el );
	HK_ASSERT2(0x271bc759, i >= 0, "You tried to remove an entity activation listener, which was never added" );

	m_activationListeners[i] = HK_NULL;
}


void hkEntity::addCollisionListener( hkCollisionListener* cl )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	HK_ASSERT2(0x6515b54e, m_collisionListeners.indexOf( cl ) < 0, "You tried to add a collision listener twice" );

	int emptyIndex = m_collisionListeners.indexOf( HK_NULL );
	if ( emptyIndex >= 0)
	{
		m_collisionListeners[emptyIndex] = cl;
	}
	else
	{
		m_collisionListeners.pushBack(cl);
	}
}



void hkEntity::removeCollisionListener( hkCollisionListener* cl)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	int i = m_collisionListeners.indexOf( cl );
	HK_ASSERT2(0x403757e4, i >= 0, "You tried to remove a collision listener, which was never added" );

	m_collisionListeners[i] = HK_NULL;
}


hkDynamicsContactMgr* hkEntity::findContactMgrTo(const hkEntity* entity)
{
	int numCollisionEntries = this->getLinkedCollidable()->m_collisionEntries.getSize();
	{
		for (int i = 0; i < numCollisionEntries; i++)
		{
			const hkCollidable* collisionPartner = this->getLinkedCollidable()->m_collisionEntries[i].m_partner;
			if ( collisionPartner == entity->getCollidable() )
			{
				return (hkDynamicsContactMgr*)this->getLinkedCollidable()->m_collisionEntries[i].m_agentEntry->m_contactMgr;
			}
		}
	}

	return HK_NULL;
}


static HK_FORCE_INLINE hkBool hkEntity_isActive(const hkEntity* entity) 
{
	if ( entity->getSimulationIsland() == HK_NULL )
	{
		return false;
	}
	else
	{
		return entity->getSimulationIsland()->isActive();
	}
}

hkBool hkEntity::isActive() const
{
	return hkEntity_isActive( this );
}

void hkEntity::activate()
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );

		// keep the entity alive for a while
	m_motion.m_deactivationNumInactiveFrames[0] = 0;
	m_motion.m_deactivationNumInactiveFrames[1] = 0;

	if (!hkEntity_isActive(this) && !isFixed() && m_world)
	{
		hkWorldOperationUtil::markIslandActive(m_world, m_simulationIsland);
	}
}

void hkEntity::deactivate()
{
	HK_ASSERT2(0xf0ff0091, m_world, "hkEntity::deactivate() called for hkEntity which has not been added to an hkWorld");
	HK_ASSERT2(0xf0ff0092, !isFixed(), "hkEntity::deactivate() called for a fixed body");

	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_RW, this, HK_ACCESS_RW );

	if (hkEntity_isActive(this))
	{
		if ( m_simulationIsland->m_sparseEnabled || m_simulationIsland->m_splitCheckRequested )
		{
			HK_ASSERT2( 0xf02343de, !m_world->areCriticalOperationsLocked(), "You cannot call this function from a callback. Call deactivateAsCriticalOperation instead");

			int old = m_world->m_minDesiredIslandSize;

			// force splitting this island: see details in hkWorldOperationUtil::splitSimulationIsland
			m_world->m_minDesiredIslandSize = 0;
			m_simulationIsland->m_sparseEnabled = false;
			m_simulationIsland->m_splitCheckRequested = true;
			hkWorldOperationUtil::splitSimulationIsland( m_world, m_simulationIsland );
			m_world->m_minDesiredIslandSize = old;
		}
			// warning: m_simulationIsland might be changed here, we have to use the member variable

			// make sure we will get the deactivation. Check hkRigidMotionUtilCanDeactivateFinal for details
		for (int i = 0; i < m_simulationIsland->m_entities.getSize(); i++)
		{
			hkEntity* other = m_simulationIsland->m_entities[i];
			other->m_motion.m_deactivationRefPosition[0](3) = HK_REAL_MAX;
			other->m_motion.m_deactivationRefPosition[1](3) = HK_REAL_MAX;
		}

		hkWorldOperationUtil::markIslandInactive(m_world, m_simulationIsland);
	}
}

void hkEntity::activateAsCriticalOperation()
{
	if (m_world && m_world->areCriticalOperationsLocked())
	{
		hkWorldOperation::ActivateEntity op;
		op.m_entity = this;
		m_world->queueOperation( op );
		return;
	}
	activate();
}

void hkEntity::deactivateAsCriticalOperation()
{
	if (m_world && m_world->areCriticalOperationsLocked())
	{
		hkWorldOperation::DeactivateEntity op;
		op.m_entity = this;
		m_world->queueOperation( op );
		return;
	}

	deactivate();
}

void hkEntity::deallocateInternalArrays()
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	// Need to deallocate any arrays in the entity that are 0 size
	// else warn user that they should call the in place destructor.

	// If this entity was loaded by a pack file 
	// and the motion pointer has been allocated separately by the user
	// then warn the user
	if (m_motion.m_memSizeAndFlags != 0)
	{
		HK_WARN(0x234f224a, "Entity at address " << this << " was loaded from a packfile but has a user allocated hkMotionState.\nPlease call in-place destructor to deallocate.\n");
	}


	// Linked Collidables collision entries
	hkArray<hkLinkedCollidable::CollisionEntry>& collisionEntries = getLinkedCollidable()->m_collisionEntries;
	if (collisionEntries.getSize() == 0)
	{
		collisionEntries.clearAndDeallocate();
	}
	else
	{
		HK_ASSERT2(0x234f223a, 0, "Entity at address " << this << " has linked collidables with non-zero collision entries array.\nPlease call in-place destructor to deallocate.\n");
	}

	// Constraints Master
	if (m_constraintsMaster.getSize() == 0)
	{
		m_constraintsMaster.clearAndDeallocate();
	}
	else
	{
		HK_WARN(0x234f224a, "Entity at address " << this << " has non-zero m_constraintsMaster array.\nPlease call in-place destructor to deallocate.\n");
	}

	// Constraints Slave
	if (m_constraintsSlave.getSize() == 0)
	{
		m_constraintsSlave.clearAndDeallocate();
	}
	else
	{
		HK_WARN(0x234f224a, "Entity at address " << this << " has non-zero m_constraintsSlave array.\nPlease call in-place destructor to deallocate.\n");
	}

	// Constraints Runtime
	if (m_constraintRuntime.getSize() == 0)
	{
		m_constraintRuntime.clearAndDeallocate();
	}
	else
	{
		HK_WARN(0x234f224a, "Entity at address " << this << " has non-zero m_constraintRuntime array.\nPlease call in-place destructor to deallocate.\n");
	}



	// Collision Listeners
	{
		hkBool hasCollisionListeners = false;
		for (int i = 0; i < m_collisionListeners.getSize(); i++)
		{
			if (m_collisionListeners[i] != HK_NULL)
			{
				hasCollisionListeners = true;
				break;
			}
		}

		if (!hasCollisionListeners)
		{
			m_collisionListeners.clearAndDeallocate();
		}
		else
		{
			HK_WARN(0x234f224a, "Entity at address " << this << " has non-zero m_collisionListeners array.\nPlease call in-place destructor to deallocate.\n");
		}
	}

	// Activation Listeners
	{
		hkBool hasActivationListeners = false;
		for (int i = 0; i < m_activationListeners.getSize(); i++)
		{
			if (m_activationListeners[i] != HK_NULL)
			{
				hasActivationListeners = true;
				break;
			}
		}

		if (!hasActivationListeners)
		{
			m_activationListeners.clearAndDeallocate();
		}
		else
		{
			HK_WARN(0x234f224b, "Entity at address " << this << " has non-zero m_activationListeners array.\nPlease call in-place destructor to deallocate.\n");
		}
	}

	// Entity Listeners
	{
		hkBool hasEntityListeners = false;
		for (int i = 0; i < m_entityListeners.getSize(); i++)
		{
			if (m_entityListeners[i] != HK_NULL)
			{
				hasEntityListeners = true;
				break;
			}
		}

		if (!hasEntityListeners)
		{
			m_entityListeners.clearAndDeallocate();
		}
		else
		{
			HK_WARN(0x234f224c, "Entity at address " << this << " has non-zero m_entityListeners array.\nPlease call in-place destructor to deallocate.\n");
		}
	}

	// Actions
	// Rather than resize this array on removal of an action hkWorld 
	// just replaces the action with a null action. If all the actions
	// are null we can go ahead and safely deallocate
	if (m_actions.isEmpty())
	{
		m_actions.clearAndDeallocate();
	}
	else
	{
		HK_WARN(0x234f224d, "Entity at address " << this << " has non-zero m_actions array.\nPlease call in-place destructor to deallocate.\n");
	}
}

int hkEntity::getNumConstraints() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, m_simulationIsland, HK_ACCESS_RO);
	return m_constraintsMaster.getSize() + m_constraintsSlave.getSize();
}

hkConstraintInstance* hkEntity::getConstraint( int i )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, m_simulationIsland, HK_ACCESS_RW);
	HK_ASSERT2(0, i >=0 && i < getNumConstraints(), "Constraint index out of range.");
	return i < m_constraintsMaster.getSize() ? m_constraintsMaster[i].m_constraint : m_constraintsSlave[i - m_constraintsMaster.getSize()];
}

const hkConstraintInstance* hkEntity::getConstraint( int i ) const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, m_simulationIsland, HK_ACCESS_RO);
	HK_ASSERT2(0, i >=0 && i < getNumConstraints(), "Constraint index out of range.");
	return i < m_constraintsMaster.getSize() ? m_constraintsMaster[i].m_constraint : m_constraintsSlave[i - m_constraintsMaster.getSize()];
}

/// Returns read only access to the internal constraint master list
const hkArray<struct hkConstraintInternal>&  hkEntity::getConstraintMastersImpl() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, m_simulationIsland, HK_ACCESS_RO);
	return m_constraintsMaster;
}

/// Returns read write access to the internal constraint master list
hkArray<struct hkConstraintInternal>&  hkEntity::getConstraintMastersRwImpl()
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, m_simulationIsland, HK_ACCESS_RW);
	return m_constraintsMaster;
}

const hkArray<class hkConstraintInstance*>&  hkEntity::getConstraintSlavesImpl() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, m_simulationIsland, HK_ACCESS_RO);
	return m_constraintsSlave;
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
