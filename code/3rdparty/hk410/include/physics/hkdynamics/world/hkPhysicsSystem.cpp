/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/hkPhysicsSystem.h>
#include <hkdynamics/phantom/hkPhantom.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/action/hkAction.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/htl/hkAlgorithm.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkPhysicsSystem); 

hkPhysicsSystem::hkPhysicsSystem()
: m_name(HK_NULL), m_userData(HK_NULL), m_active(true)
{
}

hkPhysicsSystem::~hkPhysicsSystem()
{
	int nrb = m_rigidBodies.getSize();
	for (int r=0; r < nrb; ++r)
	{
		m_rigidBodies[r]->removeReference();
	}

	int np = m_phantoms.getSize();
	for (int p=0; p < np; ++p)
	{
		m_phantoms[p]->removeReference();
	}

	int nc = m_constraints.getSize();
	for (int c=0; c < nc; ++c)
	{
		m_constraints[c]->removeReference();
	}

	int na = m_actions.getSize();
	for (int a=0; a < na; ++a)
	{
		if (m_actions[a]) m_actions[a]->removeReference();
	}
}

void hkPhysicsSystem::copy(const hkPhysicsSystem& toCopy)
{
	m_rigidBodies = toCopy.m_rigidBodies;
	m_phantoms = toCopy.m_phantoms;
	m_constraints = toCopy.m_constraints;
	m_actions = toCopy.m_actions;
	m_name = toCopy.m_name;
	m_userData = toCopy.m_userData;
	m_active = toCopy.m_active;
}


hkPhysicsSystem* hkPhysicsSystem::clone() const
{
	hkPhysicsSystem* newSystem = new hkPhysicsSystem();
	newSystem->m_name = m_name;
	newSystem->m_userData = m_userData;
	newSystem->m_active = m_active;

	newSystem->m_rigidBodies.setSize( m_rigidBodies.getSize() );
	newSystem->m_phantoms.setSize( m_phantoms.getSize() );
	newSystem->m_constraints.setSize( m_constraints.getSize() );
	newSystem->m_actions.setSize( m_actions.getSize() );

	// RigidBodies and Phantoms:
	int i;
	for (i = 0; i < m_rigidBodies.getSize(); ++i )
	{
		HK_ASSERT2(0x374076ab, m_rigidBodies[i] != HK_NULL, "You cannot have a NULL rigidbody pointer in a physics system - remove the element from the array instead." );
		newSystem->m_rigidBodies[i] = m_rigidBodies[i]->clone();
	}	
	for (i = 0; i < m_phantoms.getSize(); ++i )
	{
		HK_ASSERT2(0x374076ac, m_phantoms[i] != HK_NULL, "You cannot have a NULL phantom pointer in a physics system - remove the element from the array instead." );
		newSystem->m_phantoms[i] = m_phantoms[i]->clone();
	}

	// Constraints and Actions.
	for (i = 0; i < m_constraints.getSize(); ++i )
	{
		hkConstraintInstance* c = m_constraints[i];

		HK_ASSERT2(0x374076ad, c != HK_NULL, "You cannot have a NULL constraint pointer in a physics system - remove the element from the array instead." );
		int newBodyAIndex = m_rigidBodies.indexOf( (hkRigidBody*)( c->getEntityA() ) );
		int newBodyBIndex = m_rigidBodies.indexOf( (hkRigidBody*)( c->getEntityB() ) );
		HK_ASSERT2(0x374076ae, (newBodyAIndex >= 0) && (newBodyBIndex >= 0), "Rigidbodies are referenced by a constraint in the physics system that are not in the physics system.");

		hkRigidBody* rbA = newSystem->m_rigidBodies[newBodyAIndex];
		hkRigidBody* rbB = newSystem->m_rigidBodies[newBodyBIndex];
		
		newSystem->m_constraints[i] = c->clone( rbA, rbB );
	}
	
	for (i = 0; i < m_actions.getSize(); ++i )
	{
		hkAction* a = m_actions[i];
		if (!a) 
		{
			newSystem->m_actions[i] = HK_NULL; 
			continue;
		}

		hkArray<hkEntity*> actionEntities;
		a->getEntities(actionEntities);

		hkArray<hkEntity*> newRBs;
		int numE = actionEntities.getSize();
		newRBs.setSize(numE);
		int j = 0;
		for (j = 0; j < numE; ++j)
		{
			hkRigidBody* rb = (hkRigidBody*)actionEntities[j];
			int newBodyIndex = m_rigidBodies.indexOf( rb );
			HK_ASSERT2(0x374076ae, newBodyIndex >= 0, "Rigid bodies not in the physics system are referenced by an action in the physics system.");
			newRBs[j] = newSystem->m_rigidBodies[ newBodyIndex ];
		}

		hkArray<hkPhantom*> actionPhantoms;
		a->getPhantoms(actionPhantoms);

		hkArray<hkPhantom*> newPhantoms;
		int numP = actionPhantoms.getSize();
		newPhantoms.setSize(numP);
		for (j = 0; j < numP; ++j)
		{
			hkPhantom* ph = (hkPhantom*)actionPhantoms[j];
			int newPhantomIndex = m_phantoms.indexOf( ph );
			HK_ASSERT2(0x3cebe3c4, newPhantomIndex >= 0, "Phantoms not in the physics system are referenced by an action in the physics system.");
			newPhantoms[j] = newSystem->m_phantoms[ newPhantomIndex ];
		}

		newSystem->m_actions[i] = a->clone( newRBs, newPhantoms );
	}

	return newSystem;
}

void hkPhysicsSystem::addRigidBody( hkRigidBody* rb )
{
	if (rb)
	{
		rb->addReference();
		m_rigidBodies.pushBack(rb);
	}
}

void hkPhysicsSystem::addPhantom(  hkPhantom* p )
{
	if (p)
	{
		p->addReference();
		m_phantoms.pushBack(p);
	}
}

void hkPhysicsSystem::addConstraint( hkConstraintInstance* c )
{
	if (c)
	{
		c->addReference();
		m_constraints.pushBack(c);
	}
}

void hkPhysicsSystem::addAction( hkAction* a )
{
	if (a)
	{
		a->addReference();
		m_actions.pushBack(a);
	}
}

void hkPhysicsSystem::removeRigidBody( int i )
{
	m_rigidBodies[i]->removeReference();
	m_rigidBodies.removeAt(i);
}

void hkPhysicsSystem::removePhantom( int i )
{
	m_phantoms[i]->removeReference();
	m_phantoms.removeAt(i);
}

void hkPhysicsSystem::removeConstraint( int i )
{
	m_constraints[i]->removeReference();
	m_constraints.removeAt(i);
}

void hkPhysicsSystem::removeAction( int i )
{
	m_actions[i]->removeReference();
	m_actions.removeAt(i);
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
