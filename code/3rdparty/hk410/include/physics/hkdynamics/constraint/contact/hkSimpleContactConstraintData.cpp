/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkdynamics/hkDynamics.h>
#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkconstraintsolver/simpleConstraints/hkSimpleConstraintUtil.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>

#include <hkcollide/agent/hkContactMgr.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>

#include <hkdynamics/constraint/response/hkSimpleCollisionResponse.h>
#include <hkdynamics/constraint/contact/hkSimpleContactConstraintData.h>
#include <hkdynamics/constraint/hkConstraintOwner.h>
#include <hkdynamics/collide/hkCollisionListener.h>
#include <hkdynamics/world/util/hkWorldCallbackUtil.h>
#include <hkdynamics/world/util/hkWorldConstraintUtil.h>
#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/world/hkSimulationIsland.h>

HK_COMPILE_TIME_ASSERT( sizeof( hkSimpleContactConstraintData ) < 256 );

#include <hkbase/thread/hkCriticalSection.h>



hkContactPointId hkSimpleContactConstraintData::allocateContactPoint( hkConstraintOwner& constraintOwner, hkContactPoint** cpOut, hkContactPointProperties** cpPropsOut)
{
	int sizeProps = m_atom->m_numContactPoints;

	m_atom->m_info.m_flags |= hkSimpleContactConstraintDataInfo::HK_FLAG_AREA_CHANGED;

	const int newContactIndex = m_atom->m_numContactPoints;

	const hkSimpleContactConstraintAtom* originalAtom = m_atom;
	m_atom = hkSimpleContactConstraintAtomUtil::expandOne(m_atom);

	hkContactPoint* dcp           = &m_atom->getContactPoints()[newContactIndex];
	hkContactPointProperties* cpi = &m_atom->getContactPointProperties()[newContactIndex];

	cpi->init();

	hkUchar& flags = const_cast<hkUchar&>(cpi->m_flags);
	flags = hkContactPointProperties::CONTACT_IS_NEW_AND_POTENTIAL;
	if ( newContactIndex > 0 &&  !(cpi[-1].m_flags & hkContactPointProperties::CONTACT_USES_SOLVER_PATH2) )
	{
		flags |= hkContactPointProperties::CONTACT_USES_SOLVER_PATH2;
	}

	*cpOut = dcp;
	*cpPropsOut = cpi;

	hkConstraintInfo i;
	{
		i.clear();
		int rsize = sizeProps + 1;
		i.m_maxSizeOfJacobians += rsize * HK_SIZE_OF_JACOBIAN_LAA + ( HK_SIZE_OF_JACOBIAN_LAA * 2 + HK_SIZE_OF_JACOBIAN_AA);
		if ( sizeProps == 1 )		// angular friction added 
		{
			i.add( HK_SIZE_OF_JACOBIAN_3D_FRICTION_SCHEMA - HK_SIZE_OF_JACOBIAN_2D_FRICTION_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
		}
		i.add( HK_SIZE_OF_JACOBIAN_SINGLE_CONTACT_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA);
	}

	constraintOwner.addConstraintInfo( m_constraint, i );

	if (originalAtom != m_atom)
	{
		hkWorldConstraintUtil::updateFatherOfMovedAtom( m_constraint, originalAtom, m_atom, m_atom->m_sizeOfAllAtoms );
	}

	return hkContactPointId(m_idMgrA.newId( newContactIndex ));
}


void hkSimpleContactConstraintData::freeContactPoint( hkConstraintOwner& constraintOwner, hkContactPointId id )
{
	int mid = int(id);
	HK_ASSERT2(0x7d4661da,  mid>=0, "hkContactConstraint::freeContactPoint(): Contact point was not found in current list");

	int indexOfRemovePoint = m_idMgrA.m_values[mid];
	m_idMgrA.freeId( mid );

	hkConstraintInfo conInfo;
	conInfo.clear();

	const hkSimpleContactConstraintAtom* originalAtom = m_atom;
	{
		int size = m_atom->m_numContactPoints;

		if ( size == 2 )
		{
			conInfo.add( HK_SIZE_OF_JACOBIAN_3D_FRICTION_SCHEMA - HK_SIZE_OF_JACOBIAN_2D_FRICTION_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
		}

		hkSimpleContactConstraintAtomUtil::removeAtAndCopy( m_atom, indexOfRemovePoint );
		{
			hkUchar& flags = const_cast<hkUchar&>((&m_atom->getContactPointProperties()[indexOfRemovePoint])->m_flags);
			flags &= ~hkContactPointProperties::CONTACT_USES_SOLVER_PATH2;	// unchecked overwrite
		}

		m_atom = hkSimpleContactConstraintAtomUtil::optimizeCapacity(m_atom, 1);

		m_idMgrA.decrementValuesGreater( indexOfRemovePoint );
	}


	conInfo.add( HK_SIZE_OF_JACOBIAN_SINGLE_CONTACT_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA);

	constraintOwner.subConstraintInfo( m_constraint, conInfo );

	if (originalAtom != m_atom)
	{
		// we know that during this call, no constraints are removed or added by other threads, therefore
		// we can simply modify data which is local to this constraint (hkConstraintInternal)
		hkWorldConstraintUtil::updateFatherOfMovedAtom( m_constraint, originalAtom, m_atom, m_atom->m_sizeOfAllAtoms );
	}

	m_atom->m_info.m_flags |= hkSimpleContactConstraintDataInfo::HK_FLAG_POINT_REMOVED | hkSimpleContactConstraintDataInfo::HK_FLAG_AREA_CHANGED;
}



HK_COMPILE_TIME_ASSERT( HK_SIZE_OF_JACOBIAN_3D_FRICTION_SCHEMA >= HK_SIZE_OF_JACOBIAN_2D_FRICTION_SCHEMA);


hkSimpleContactConstraintData::hkSimpleContactConstraintData(hkConstraintInstance* constraint)
{
	m_constraint = constraint;
	m_atom = hkSimpleContactConstraintAtomUtil::allocateAtom(0);
	m_atom->m_info.init();
}

hkSimpleContactConstraintData::hkSimpleContactConstraintData()
{
	m_constraint = HK_NULL;
	m_atom = hkSimpleContactConstraintAtomUtil::allocateAtom(0);
	m_atom->m_info.init();
}


void hkSimpleContactConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& infoOut ) const 
{
	getConstraintInfoUtil( m_atom, m_atom->m_sizeOfAllAtoms, infoOut );
}

void hkSimpleContactConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const
{
	// all data handled internally
	infoOut.m_sizeOfExternalRuntime = 0;
	infoOut.m_numSolverResults = 0;
}

hkSolverResults* hkSimpleContactConstraintData::getSolverResults( hkConstraintRuntime* runtime )
{
	return HK_NULL;
}

void hkSimpleContactConstraintData_fireCallbacks(hkSimpleContactConstraintData* constraintData, const hkConstraintQueryIn* in, hkSimpleContactConstraintAtom* atom )
{
	// fire callbacks for each new point
	hkConstraintInstance* constraintInstance = in->m_constraintInstance;
	hkContactPointProperties* cpp = atom->getContactPointProperties();
	hkContactPoint* cp = atom->getContactPoints();
	int nA = atom->m_numContactPoints;
	for (int cindex = nA-1; cindex >=0 ; cp++, cpp++, cindex-- )
	{
		if ( cpp->m_flags & hkContactPointProperties::CONTACT_IS_NEW_AND_POTENTIAL )
		{
			hkRigidBody* bodyA = constraintInstance->getRigidBodyA();
			hkRigidBody* bodyB = constraintInstance->getRigidBodyB();

			hkReal projectedVel;
			{
				hkVector4 velA;
				{
					hkVelocityAccumulator* acc = in->m_bodyA;
					hkVector4 relPos; relPos.setSub4( cp->getPosition(), acc->getCenterOfMassInWorld() );
					velA.setCross( bodyA->getAngularVelocity(), relPos);
					velA.add4( bodyA->getLinearVelocity() );
				}
				hkVector4 velB;
				{
					hkVelocityAccumulator* acc = in->m_bodyB;
					hkVector4 relPos; relPos.setSub4( cp->getPosition(), acc->getCenterOfMassInWorld() );
					velB.setCross( bodyB->getAngularVelocity(), relPos);
					velB.add4( bodyB->getLinearVelocity() );
				}

				hkVector4 deltaVel; deltaVel.setSub4( velA, velB );
				projectedVel = deltaVel.dot3( cp->getNormal() );
			}

			hkContactPointConfirmedEvent event( hkContactPointAddedEvent::TYPE_MANIFOLD, *bodyA->getCollidable(), *bodyB->getCollidable(), constraintData, cp, cpp, 0.0f, projectedVel );

			hkWorld* world = bodyA->getWorld();
			hkWorldCallbackUtil::fireContactPointConfirmed( world, event );
			hkEntityCallbackUtil::fireContactPointConfirmed( bodyA, event );
			hkEntityCallbackUtil::fireContactPointConfirmed( bodyB, event );

			const hkWorldDynamicsStepInfo& env = *reinterpret_cast<const hkWorldDynamicsStepInfo*>(world->getCollisionInput()->m_dynamicsInfo);

			hkReal restitution = cpp->getRestitution();
			const hkReal MIN_RESTITUTION_TO_USE_CORRECT_RESPONSE = 0.3f;
			if ( event.m_projectedVelocity < -env.m_solverInfo.m_contactRestingVelocity && restitution > MIN_RESTITUTION_TO_USE_CORRECT_RESPONSE )
			{
				hkSimpleConstraintUtilCollideParams params;
				{
					params.m_direction = cp->getNormal();
					params.m_friction  = cpp->getFriction();
					params.m_restitution = restitution;
					params.m_extraSeparatingVelocity = 0.0f;
					params.m_extraSlope = 0.0f;
					params.m_externalSeperatingVelocity = event.m_projectedVelocity;
				}

				hkSimpleCollisionResponse::SolveSingleOutput2 out2;
				hkVelocityAccumulator* accuA = in->m_bodyA;
				hkVelocityAccumulator* accuB = in->m_bodyB;
				hkSimpleCollisionResponse::solveSingleContact2( constraintData, *cp, params, bodyA, bodyB, accuA, accuB, out2 );

				// make a simple guess for the friction 
				// Unfortunately we can't take the initial impulse, as this impulse is way higher than the resting impulse
				cpp->m_impulseApplied = 0.0f; // out2.m_impulse * 0.5f; 
				cpp->m_internalDataA = cp->getDistance();
			}
			else
			{
				{
					hkReal sumInvMass = bodyA->getMassInv() + bodyB->getMassInv();
					hkReal mass = 1.0f / (sumInvMass + 1e-10f );
					cpp->m_impulseApplied = -0.2f * mass * projectedVel * ( 1.0f + cpp->getRestitution() );
				}
				{
					hkReal ErrorTerm = cpp->getRestitution() * projectedVel * in->m_substepDeltaTime;
					ErrorTerm *= -1.3f;
					cpp->m_internalSolverData = ErrorTerm;
					hkReal dist = cp->getDistance() + ErrorTerm;
					cpp->m_internalDataA = dist;
				}
			}

			cpp->m_flags &= ~hkContactPointProperties::CONTACT_IS_NEW_AND_POTENTIAL;
		}
	}

	// clear the callback flag once the callback has been fired
	constraintInstance->m_internal->m_callbackRequest &= ~hkConstraintAtom::CALLBACK_REQUEST_CONTACT_POINT;
}

void hkSimpleContactConstraintData::collisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	hkConstraintInstance* instance = m_constraint;
	if ( !instance->m_constraintModifiers )
	{
		return;
	}

	// iterate over all atom modifiers
	hkConstraintAtom* atom = instance->m_constraintModifiers;

	while ( atom->isModifierType() )
	{
		switch ( atom->getType())
		{
		case hkConstraintAtom::TYPE_MODIFIER_MASS_CHANGER:
			{
				hkMassChangerModifierConstraintAtom* c = reinterpret_cast<hkMassChangerModifierConstraintAtom*>(atom);
				c->collisionResponseBeginCallback( cp, inA, velA, inB, velB );
				break;
			}

		case hkConstraintAtom::TYPE_MODIFIER_SOFT_CONTACT:
			{
				hkSoftContactModifierConstraintAtom* c = reinterpret_cast<hkSoftContactModifierConstraintAtom*>(atom);
				c->collisionResponseBeginCallback( cp, inA, velA, inB, velB );
				break;
			}

		case hkConstraintAtom::TYPE_MODIFIER_MOVING_SURFACE:
			{
				hkMovingSurfaceModifierConstraintAtom* c = reinterpret_cast<hkMovingSurfaceModifierConstraintAtom*>(atom);
				c->collisionResponseBeginCallback( cp, inA, velA, inB, velB );
				break;
			}

		default:
			{
				break;
			}
		}
		atom = static_cast<hkModifierConstraintAtom*>(atom)->m_child;
	}
}

void hkSimpleContactConstraintData::collisionResponseEndCallback( const hkContactPoint& cp, hkReal impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	hkConstraintInstance* instance = m_constraint;

	if ( !instance->m_constraintModifiers )
	{
		return;
	}


	// iterate over all atom modifiers
	hkConstraintAtom* atom = instance->m_constraintModifiers;

	while ( atom->isModifierType() )
	{
		switch ( atom->getType())
		{
		case hkConstraintAtom::TYPE_MODIFIER_MASS_CHANGER:
			{
				hkMassChangerModifierConstraintAtom* c = static_cast<hkMassChangerModifierConstraintAtom*>(atom);
				c->collisionResponseEndCallback( cp, impulseApplied, inA, velA, inB, velB );
				break;
			}

		case hkConstraintAtom::TYPE_MODIFIER_SOFT_CONTACT:
			{
				hkSoftContactModifierConstraintAtom* c = static_cast<hkSoftContactModifierConstraintAtom*>(atom);
				c->collisionResponseEndCallback( cp, impulseApplied, inA, velA, inB, velB );
				break;
			}

		case hkConstraintAtom::TYPE_MODIFIER_MOVING_SURFACE:
			{
				hkMovingSurfaceModifierConstraintAtom* c = static_cast<hkMovingSurfaceModifierConstraintAtom*>(atom);
				c->collisionResponseEndCallback( cp, impulseApplied, inA, velA, inB, velB );
				break;
			}

		default:
			{
				break;
			}
		}
		atom = static_cast<hkModifierConstraintAtom*>(atom)->m_child;
	}
}



hkBool hkSimpleContactConstraintData::isValid() const
{
	return true;
}

int hkSimpleContactConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_CONTACT;
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
