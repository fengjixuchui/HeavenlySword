/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/lazyadd/hkLazyAddToWorld.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkEntity.h>
#include <hkdynamics/action/hkAction.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>

hkLazyAddToWorld::hkLazyAddToWorld(hkWorld* world)
:	m_world(world)
{
	m_world->addReference();
}

hkLazyAddToWorld::~hkLazyAddToWorld()
{
	m_world->removeReference();
	
	HK_ASSERT2(0x608d0452, m_entities.getSize() == 0, "Cannot delete hkLazyAddToWorld before committing all lazy hkEntities.");
	HK_ASSERT2(0x7b815750, m_actions.getSize() == 0, "Cannot delete hkLazyAddToWorld before committing all lazy hkActions.");
	HK_ASSERT2(0x3d662b9f, m_constraints.getSize() == 0, "Cannot delete hkLazyAddToWorld before committing all lazy hkConstraints.");
}
		
int hkLazyAddToWorld::commitAll()
{
	int totalCommitted = 0;

	// Commit lazy hkEntities.
	int numCommitted = commitAllEntity();
	if(numCommitted == -1)
	{
		return -1;
	}
	totalCommitted += numCommitted;

	// Commit lazy hkActions.
	numCommitted = commitAllAction();
	if(numCommitted == -1)
	{
		return -1;
	}
	totalCommitted += numCommitted;

	// Commit lazy hkConstraints.
	numCommitted = commitAllConstraint();
	if(numCommitted == -1)
	{
		return -1;
	}
	totalCommitted += numCommitted;

	return totalCommitted;
}

int hkLazyAddToWorld::commitAllEntity()
{
	// Initialise to zero.
	int numCommitted = 0;

	//
	// Iterate through the m_entities hkArray, committing each hkEntity.
	//
	while (m_entities.getSize() > 0)
	{
		// Check if it's OK to add this hkAction to m_world.
		hkBool committOk = isValid(m_entities[0]);

		HK_ASSERT2(0x1d8c63fa, committOk, "Cannot commit invalid hkEntity.");

		if (committOk)
		{
			// Add entity to hkWorld.
			m_world->addEntity(m_entities[0]);

			//
			// Remove entity from wait arrays. 
			//
			m_entities[0]->removeReference();
			m_entities.removeAt(0);
			numCommitted++;
		}
		else
		{
			// If any hkEntitiess fail to commit, return failure value (-1) immediately.
			return -1;
		}
	}

	// Return the number of hkEntities committed.
	return numCommitted;
}

int hkLazyAddToWorld::commitAllAction()
{
	// Initialise to zero.
	int numCommitted = 0;

	//
	// Iterate through the m_actions hkArray, committing each hkAction.
	//
	while (m_actions.getSize() > 0)
	{
		// Check if it's OK to add this hkAction to m_world.
		hkBool committOk = isValid(m_actions[0]);

		HK_ASSERT2(0x3f9db3c8, committOk, "Cannot commit invalid hkAction.");

		if (committOk)
		{
			// Add action to hkWorld.
			m_world->addAction(m_actions[0]);

			m_actions[0]->removeReference();
			m_actions.removeAt(0);
			numCommitted++;
		}
		else
		{
			// If any hkActions fail to commit, return failure value (-1) immediately.
			return -1;
		}
	}

	// Return the number of hkActions committed.
	return numCommitted;
}

int hkLazyAddToWorld::commitAllConstraint()
{
	// Initialise to zero.
	int numCommitted = 0;

	//
	// Iterate through the m_constraints hkArray, committing each hkConstraintInstance.
	//
	while (m_constraints.getSize() > 0)
	{
		// Check if it's OK to add this hkConstraintInstance to m_world.
		hkBool committOk = isValid(m_constraints[0]);

		HK_ASSERT2(0x41f286d4, committOk, "Cannot commit invalid hkConstraintInstance.");

		if (committOk)
		{
			// Add constraint to hkWorld.
			m_world->addConstraint(m_constraints[0]);

			m_constraints[0]->removeReference();
			m_constraints.removeAt(0);
			numCommitted++;
		}
		else
		{
			// If any hkConstraints fail to commit, return failure value (-1) immediately.
			return -1;
		}
	}

	// Return the number of hkActions committed.
	return numCommitted;
}
		
int hkLazyAddToWorld::addEntity(hkEntity* entity)
{
	hkBool addOk = entity->getWorld() == HK_NULL;

	HK_ASSERT2(0x4ac56803, addOk, "Cannot add an hkEntity that is already added to an hkWorld.");

	if (addOk)
	{
		entity->addReference();
		m_entities.pushBack(entity);
		return m_entities.getSize();
	}
	else
	{
		return -1;	
	}
}

int hkLazyAddToWorld::addAction(hkAction* action)
{
	hkBool addOk = action->getWorld() == HK_NULL;

	HK_ASSERT2(0x25a9b40b, addOk, "Cannot add an hkAction that is already added to an hkWorld.");

	if (addOk)
	{
		action->addReference();
		m_actions.pushBack(action);
		return m_actions.getSize();
	}
	else
	{
		return -1;	
	}
}

int hkLazyAddToWorld::addConstraint( hkConstraintInstance* constraint )
{
	hkBool addOk = constraint->getOwner() == HK_NULL;

	HK_ASSERT2(0x785721e2, addOk, "Cannot add an hkConstraintInstance that is already added to an hkWorld.");

	if (addOk)
	{
		constraint->addReference();
		m_constraints.pushBack(constraint);
		return m_constraints.getSize();
	}
	else
	{
		return -1;	
	}
}

hkBool hkLazyAddToWorld::isValid(hkEntity* entity)
{
	// If entity's hkCollidable has an hkShape it's valid.
	return ( entity->getCollidable()->getShape() != HK_NULL );
}

hkBool hkLazyAddToWorld::isValid(hkAction* action)
{
	hkArray<hkEntity*> entities;

	action->getEntities(entities);

	//
	// If the action's entities are not HK_NULL it's valid.
	//
	for (int i = 0; i < entities.getSize(); ++i)
	{
		if (entities[i] == HK_NULL)
		{
			return false;	
		}
	}
	return true;
}

hkBool hkLazyAddToWorld::isValid(hkConstraintInstance* constraint)
{
	// If constraint's entities are not HK_NULL, it is valid.
	return (constraint->getEntityA() != HK_NULL 
			&& constraint->getEntityB() != HK_NULL);
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
