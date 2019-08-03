/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkdynamics/hkDynamics.h>

#include <hkdynamics/world/simulation/backstep/hkBackstepSimulation.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/motion/hkMotion.h>
#include <hkmath/basetypes/hkMotionState.h>
#include <hkmath/linear/hkSweptTransform.h>

#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>

#include <hkcollide/agent/hkProcessCollisionInput.h>


hkBackstepSimulation::hkBackstepSimulation( hkWorld* world, BackstepMode backstepMode ) 
:	hkContinuousSimulation( world ), 
	m_backsteppingMode(backstepMode) 
{
}

void hkBackstepSimulation::simulateToi( hkWorld* world, hkToiEvent& event, hkReal physicsDeltaTime )
{
	world->lockCriticalOperations();
	{
		for (int e = 0; e < 2; e++)
		{
			hkRigidBody* body = static_cast<hkRigidBody*>(event.m_entities[e]);
			hkMotionState& motionState = body->getRigidMotion()->m_motionState;

			if (!body->isFixedOrKeyframed() && motionState.getSweptTransform().getInvDeltaTime() != 0.0f)
			{
				// Backstep
				hkSweptTransformUtil::backStepMotionState( event.m_time, motionState );
				
				// Freeze: Set initial and final positions/orientations in hkSweptTransform to present pos (pos1)
				motionState.getSweptTransform().m_centerOfMass0 = motionState.getSweptTransform().m_centerOfMass1;
				motionState.getSweptTransform().m_rotation0 = motionState.getSweptTransform().m_rotation1;

				motionState.getSweptTransform().m_centerOfMass0(3) = event.m_time;
				motionState.getSweptTransform().m_centerOfMass1(3) = 0.0f; //1.0f / (world->m_timeOfNextPsi - event.m_time);

				hkEntity* entity = body;
				resetCollisionInformationForEntities(&entity, 1, world);

				// Skip broadphase collision detection.

				// Collide body inplace
				if (m_backsteppingMode == NON_PENETRATING)
				{
					// Generate new TOI events
					collideEntitiesNarrowPhaseContinuous(&entity, 1, *world->m_collisionInput);
				}
				else if (m_backsteppingMode == SIMPLE)
				{
					// Collide bodies in discrete mode to properly build contact information.
					collideEntitiesNarrowPhaseDiscrete(&entity, 1, *world->m_collisionInput, FIND_CONTACTS_DEFAULT);
				}
			}
		}
 		if ( m_backsteppingMode == SIMPLE  && !(event.m_entities[0]->isFixedOrKeyframed() ^ event.m_entities[1]->isFixedOrKeyframed()) )
		{
			HK_WARN(0xad2345a, "Continuous collision detection performed for dynamic-dynamic (or fixedOrKeyframed-fixedOrKeyframed) pairs of bodies. This simulation (SIMULATION_TYPE_BACKSTEPPING_SIMPLE) does not prevent pairs of dynamic bodies from penetrating.");
		}
	}
	world->unlockAndAttemptToExecutePendingOperations();
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
