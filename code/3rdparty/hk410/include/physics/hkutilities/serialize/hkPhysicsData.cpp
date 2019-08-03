/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkmath/hkMath.h>
#include <hkdynamics/action/hkAction.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/world/hkPhysicsSystem.h>
#include <hkutilities/serialize/hkPhysicsData.h>
#include <hkcollide/filter/hkCollisionFilter.h>
#include <hkcollide/dispatch/hkAgentRegisterUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkPhysicsData);

hkPhysicsData::hkPhysicsData()
{
	m_worldCinfo = HK_NULL;
}


void hkPhysicsData::populateFromWorld( const hkWorld* world )
{
	if ( m_worldCinfo != HK_NULL )
	{
		HK_WARN(0xd698765d, "Overwriting world cinfo" );
	}
	else
	{
		m_worldCinfo = new hkWorldCinfo();
	}
	world->getCinfo(*m_worldCinfo);


	// systems
	world->getWorldAsSystems(m_systems);
}

hkPhysicsData::~hkPhysicsData()
{
	if ( m_worldCinfo != HK_NULL )
	{	
		m_worldCinfo->removeReference();
	}


	for (int p=0; p < m_systems.getSize(); ++p)
	{
		m_systems[p]->removeReference();
	}
}

hkWorld* hkPhysicsData::createWorld(hkBool registerAllAgents)
{
	hkWorldCinfo defaultCinfo;
	hkWorld* w = HK_NULL;
	if (!m_worldCinfo)
	{
		w = new hkWorld(defaultCinfo);
	}
	else
	{
		w = new hkWorld(*m_worldCinfo);
	}

	w->markForWrite();

	if (registerAllAgents)
	{
		hkAgentRegisterUtil::registerAllAgents( w->getCollisionDispatcher() );
	}

	for (int p=0; p < m_systems.getSize(); ++p)
	{
		w->addPhysicsSystem(m_systems[p]);
	}

	if (m_worldCinfo && m_worldCinfo->m_collisionFilter)
	{
		w->updateCollisionFilterOnWorld(HK_UPDATE_FILTER_ON_WORLD_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
	}

	w->unmarkForWrite();
	return w;
}

static void tryMoveEntity( hkPhysicsSystem* fromSystem, hkPhysicsSystem* toSystem, hkEntity* entity )
{
	hkRigidBody* rb = static_cast<hkRigidBody*>(entity);
	const int fromIndex = fromSystem->getRigidBodies().indexOf(rb);
	const int toIndex = toSystem->getRigidBodies().indexOf(rb);
	if (  (toIndex == -1) && (fromIndex != -1) )
	{
		toSystem->setActive(toSystem->isActive() || rb->isActive());
		toSystem->addRigidBody(rb);
		fromSystem->removeRigidBody(fromIndex);
	}
}

// This converts the system into:
// One system for unconstrained fixed rigid bodies
// One system for unconstrained keyframed bodies
// One system for unconstrained moving bodies
// One system per group of constrained bodies (such as a ragdoll)
// One system to contain all the phantoms in the input system
// The user data pointer of the input system is set to the user data for all systems created
void hkPhysicsData::splitPhysicsSystems(const hkPhysicsSystem* inputSystemConst, SplitPhysicsSystemsOutput& output )
{
	hkPhysicsSystem* inputSystem = new hkPhysicsSystem();
	{
		for (int i = 0; i < inputSystemConst->getRigidBodies().getSize(); ++i)
		{
			inputSystem->addRigidBody( inputSystemConst->getRigidBodies()[i] );
		}
	}
	{
		for (int i = 0; i < inputSystemConst->getActions().getSize(); ++i)
		{
			inputSystem->addAction( inputSystemConst->getActions()[i] );
		}
	}
	{
		for (int i = 0; i < inputSystemConst->getConstraints().getSize(); ++i)
		{
			inputSystem->addConstraint( inputSystemConst->getConstraints()[i] );
		}
	}
	{
		for (int i = 0; i < inputSystemConst->getPhantoms().getSize(); ++i)
		{
			inputSystem->addPhantom( inputSystemConst->getPhantoms()[i] );
		}
	}

	const hkArray<hkRigidBody*>& rigidBodies = inputSystem->getRigidBodies();

	//
	// First system is an array of rigid bodies
	//
	hkPhysicsSystem* fixedSystem = new hkPhysicsSystem();
	fixedSystem->setName("Fixed Rigid Bodies");
	fixedSystem->setUserData(inputSystem->getUserData());
	{
		int i = 0;
		fixedSystem->setActive(false);
		while (i < rigidBodies.getSize())
		{
			if (rigidBodies[i]->isFixed())
			{
				fixedSystem->addRigidBody(rigidBodies[i]);
				inputSystem->removeRigidBody(i);
			}
			else
			{
				i++;
			}
		}
	}
	if ( fixedSystem->getRigidBodies().getSize() != 0 )
	{
		output.m_unconstrainedFixedBodies = fixedSystem;
	}
	else
	{
		delete fixedSystem;
		output.m_unconstrainedFixedBodies = HK_NULL;
	}


	//
	// Separate out all the constrained systems into separate physics systems, and unconstrained bodies into another system
	//
	hkPhysicsSystem* ballisticSystem = new hkPhysicsSystem();
	ballisticSystem->setName("Unconstrained Rigid Bodies");
	ballisticSystem->setUserData(inputSystem->getUserData());
	ballisticSystem->setActive(false);

	hkPhysicsSystem* keyframedSystem = new hkPhysicsSystem();
	keyframedSystem->setName("Keyframed Rigid Bodies");
	keyframedSystem->setUserData(inputSystem->getUserData());
	keyframedSystem->setActive(false);

	const hkArray<hkConstraintInstance*>& constraints = inputSystem->getConstraints();
	const hkArray<hkAction*>& actions = inputSystem->getActions();


	while ( rigidBodies.getSize() > 0 )
	{
		//
		// Create a new system with the first rigid body in the rigidBodies list, and move all linked 
		// rigid bodies, actions and constraints into the system
		//

		hkPhysicsSystem* currentSystem = new hkPhysicsSystem();
		currentSystem->setName("Constrained System");
		currentSystem->setUserData(inputSystem->getUserData());

		currentSystem->setActive(rigidBodies[0]->isActive());
		currentSystem->addRigidBody(rigidBodies[0]);
		inputSystem->removeRigidBody(0);

		//
		// Very slow, but simple loop 
		// - keep iterating until we find no more constraints or actions linking to the currentSystem
		//
		bool newBodyAdded = true;
		while (newBodyAdded)
		{
			newBodyAdded = false;
			for (int j = 0; j < currentSystem->getRigidBodies().getSize(); ++j)
			{
				hkRigidBody* currentRb = currentSystem->getRigidBodies()[j];
				{
					int i = 0;
					while (i < constraints.getSize())
					{
						// (casts are for gcc)
						if ( (constraints[i]->getEntityA() == (const hkEntity*)currentRb) || (constraints[i]->getEntityB() == (const hkEntity*)currentRb) )
						{
							newBodyAdded = true;
							currentSystem->addConstraint(constraints[i]);
							hkEntity* otherBody = constraints[i]->getOtherEntity(currentRb);
							if((otherBody!=HK_NULL) && (!otherBody->isFixed()))
							{
								tryMoveEntity( inputSystem, currentSystem, otherBody );
							}
							inputSystem->removeConstraint(i);
						}
						else
						{
							i++;
						}
					}
				}
				{
					int i = 0;
					while (i < actions.getSize())
					{
						hkArray<hkEntity*> entities;
						actions[i]->getEntities( entities );
						bool actionMoved = false;
						for (int k  = 0; k < entities.getSize(); k++)
						{
							if (entities[k] == (const hkEntity*)currentRb)
							{
								newBodyAdded = true;
								currentSystem->addAction(actions[i]);
								inputSystem->removeAction(i);
								actionMoved = true;

								for (int l  = 0; l < entities.getSize(); l++)
								{
									if ( entities[l] != (const hkEntity*)currentRb )
									{
										if (!entities[l]->isFixed())
										{
											tryMoveEntity(inputSystem, currentSystem, entities[l]);
										}
									}
								}
							}
						}
						if (!actionMoved)
						{
							i++;
						}
					}
				}
			}
		}
		//
		// If the entity is not linked to any other, add it to the keyframed or ballistic system
		//
		if ((currentSystem->getConstraints().getSize() == 0) && (currentSystem->getActions().getSize() == 0))
		{
			hkRigidBody* rb = currentSystem->getRigidBodies()[0];
			// EXP-811 : RBs not yet added to the world will return False for isActive() - but we want to treat them as active
			const bool rbActive = (rb->getSimulationIsland()==HK_NULL) || rb->isActive();

			if (rb->getMotionType() == hkMotion::MOTION_KEYFRAMED)
			{
				keyframedSystem->setActive(keyframedSystem->isActive() || rbActive);
				keyframedSystem->addRigidBody(rb);
			}
			else
			{
				ballisticSystem->setActive(ballisticSystem->isActive() || rbActive);
				ballisticSystem->addRigidBody(rb);
			}
			delete currentSystem;
		}
		else
		{
			output.m_constrainedSystems.pushBack(currentSystem);
		}
	}
	HK_ASSERT(0xaa928746, inputSystem->getConstraints().getSize() == 0);
	HK_ASSERT(0xaa928746, inputSystem->getActions().getSize() == 0);
	

	if ( keyframedSystem->getRigidBodies().getSize() != 0 )
	{
		output.m_unconstrainedKeyframedBodies = keyframedSystem;
	}
	else
	{
		delete keyframedSystem;
		output.m_unconstrainedKeyframedBodies = HK_NULL;
	}

	if ( ballisticSystem->getRigidBodies().getSize() != 0 )
	{
		output.m_unconstrainedMovingBodies = ballisticSystem;
	}
	else
	{
		delete ballisticSystem;
		output.m_unconstrainedMovingBodies = HK_NULL;
	}

	const hkArray<hkPhantom*>& phantoms = inputSystem->getPhantoms();
	if (phantoms.getSize() > 0)
	{
		hkPhysicsSystem* phantomSystem = new hkPhysicsSystem();
		phantomSystem->setName("Phantoms");
		phantomSystem->setUserData(inputSystem->getUserData());

		output.m_phantoms = phantomSystem;

		for (int i = 0; i < phantoms.getSize(); ++i)
		{
			phantomSystem->addPhantom(phantoms[i]);
		}

	}
	else
	{
		output.m_phantoms = HK_NULL;
	}

	delete inputSystem;
}


// Look for a physics system by name (case insensitive)
hkPhysicsSystem* hkPhysicsData::findPhysicsSystemByName (const char* name) const
{
	for (int i=0; i < m_systems.getSize(); i++)
	{
		const char* sysName = m_systems[i]->getName();

		if (sysName && (hkString::strCasecmp(sysName, name)==0))
		{
			return m_systems[i];
		}
	}

	return HK_NULL;
}

// Look for a rigid body by name (case insensitive)
hkRigidBody*  hkPhysicsData::findRigidBodyByName (const char* name) const
{
	for (int s=0; s<m_systems.getSize(); s++)
	{
		hkPhysicsSystem* system = m_systems[s];

		for (int r=0; r<system->getRigidBodies().getSize(); r++)
		{
			hkRigidBody* rb = system->getRigidBodies()[r];
			const char* rbName = rb->getName();

			if (rbName && (hkString::strCasecmp(rbName, name)==0))
			{
				return rb;
			}
		}
	}

	return HK_NULL;

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
