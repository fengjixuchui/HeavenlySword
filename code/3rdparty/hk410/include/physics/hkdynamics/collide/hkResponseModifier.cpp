/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkdynamics/collide/hkResponseModifier.h>
#include <hkdynamics/collide/hkSimpleConstraintContactMgr.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/world/util/hkWorldConstraintUtil.h>
#include <hkdynamics/world/hkSimulationIsland.h>



void HK_CALL hkResponseModifier::setInvMassScalingForContact( hkDynamicsContactMgr* manager, hkRigidBody* bodyA, hkRigidBody* bodyB, hkConstraintOwner& constraintOwner, float factorA, float factorB )
{
	hkConstraintInstance* instance = manager->getConstraintInstance();
	if ( !instance || !instance->m_internal )
	{
		return;
	}

	// If the next line fires an assert, read the hkResponseModifier reference manual
	constraintOwner.checkAccessRw();

	HK_TIMER_BEGIN("SetMassChang", HK_NULL);

	float fA = factorA;
	float fB = factorB;
	if ( bodyA == instance->getEntityB() )
	{
		fA = factorB;
		fB = factorA;
	}


	//
	// Search existing modifier list for matching type (and update, if already present)
	//
	{
		hkModifierConstraintAtom* modifier = hkWorldConstraintUtil::findModifier( instance, hkConstraintAtom::TYPE_MODIFIER_MASS_CHANGER );
		if ( modifier )
		{
			hkMassChangerModifierConstraintAtom* massChangermodifier = reinterpret_cast<hkMassChangerModifierConstraintAtom*>(modifier);
			massChangermodifier->m_factorA = fA;
			massChangermodifier->m_factorB = fB;
			goto END;
		}
	}

	//
	// Build and insert new modifier atom
	//
	{
		hkMassChangerModifierConstraintAtom* massChangermodifier = new hkMassChangerModifierConstraintAtom;
		massChangermodifier->m_factorA = fA;
		massChangermodifier->m_factorB = fB;
		hkWorldConstraintUtil::addModifier( instance, constraintOwner, massChangermodifier );
	}

END:

	HK_TIMER_END();
}



void HK_CALL hkResponseModifier::setImpulseScalingForContact( hkDynamicsContactMgr* manager, hkRigidBody* bodyA, hkRigidBody* bodyB, hkConstraintOwner& constraintOwner, hkReal usedImpulseFraction, hkReal maxAcceleration )
{
	hkConstraintInstance* instance = manager->getConstraintInstance();
	if ( !instance )
	{
		return;
	}

		// If the next line fires an assert, read the hkResponseModifier reference manual
	constraintOwner.checkAccessRw();

	HK_TIMER_BEGIN("SetSoftContact", HK_NULL);

	//
	// Search existing modifier list for matching type (and update, if already present)
	//
	{
		hkModifierConstraintAtom* container = hkWorldConstraintUtil::findModifier( instance, hkConstraintAtom::TYPE_MODIFIER_SOFT_CONTACT );
		if ( container )
		{
			//@@@PS3: <todo: remove modifier if usedImpulseFraction is 1.0
			hkSoftContactModifierConstraintAtom* softContactContainer = reinterpret_cast<hkSoftContactModifierConstraintAtom*>(container);
			softContactContainer->m_tau = usedImpulseFraction;
			softContactContainer->m_maxAcceleration = maxAcceleration;

			goto END;
		}
	}

	//
	// Build and insert new modifier atom
	//
	{
		hkSoftContactModifierConstraintAtom* softContactContainer = new hkSoftContactModifierConstraintAtom;
		softContactContainer->m_tau = usedImpulseFraction;
		softContactContainer->m_maxAcceleration = maxAcceleration;
		hkWorldConstraintUtil::addModifier( instance, constraintOwner, softContactContainer );
	}

END:
	HK_TIMER_END();
}



void HK_CALL hkResponseModifier::setSurfaceVelocity( hkDynamicsContactMgr* manager, hkRigidBody* body, hkConstraintOwner& constraintOwner, const hkVector4& velWorld )
{
	hkConstraintInstance* instance = manager->getConstraintInstance();
	if ( !instance )
	{
		return;
	}

		// If any of these asserts gets fired, read the hkResponseModifier reference manual
	constraintOwner.checkAccessRw();

	HK_TIMER_BEGIN("SetSurfVel", HK_NULL);

	hkVector4 velocity = velWorld;
	if (instance->getEntityA() == body )
	{
		velocity.setNeg4(velocity);
	}


	//
	// Search existing modifier list for matching type (and update, if already present)
	//
	{
		hkModifierConstraintAtom* modifier = hkWorldConstraintUtil::findModifier( instance, hkConstraintAtom::TYPE_MODIFIER_MOVING_SURFACE );
		if ( modifier )
		{
			hkMovingSurfaceModifierConstraintAtom* movingSurfaceContainer = reinterpret_cast<hkMovingSurfaceModifierConstraintAtom*>(modifier);
			movingSurfaceContainer->getVelocity() = velocity;
			goto END;
		}
	}

	//
	// Build and insert new modifier atom
	//
	{
		hkMovingSurfaceModifierConstraintAtom* movingSurfaceContainer = new hkMovingSurfaceModifierConstraintAtom;
		movingSurfaceContainer->getVelocity() = velocity;
		hkWorldConstraintUtil::addModifier( instance, constraintOwner, movingSurfaceContainer );
	}

END:

	HK_TIMER_END();
}



void HK_CALL hkResponseModifier::clearSurfaceVelocity( hkDynamicsContactMgr* manager, hkConstraintOwner& constraintOwner, hkRigidBody* body )
{
	hkConstraintInstance* instance = manager->getConstraintInstance();
	if ( !instance )
	{
		return;
	}

	// If you run onto those asserts, read the hkResponseModifier reference manual
	constraintOwner.checkAccessRw();

	HK_TIMER_BEGIN("ClrSurfVel", HK_NULL);

		// destroy modifier atom
	hkWorldConstraintUtil::removeModifier( instance, constraintOwner, hkConstraintAtom::TYPE_MODIFIER_MOVING_SURFACE );

	HK_TIMER_END();
}

void HK_CALL hkResponseModifier::setLowSurfaceViscosity( hkDynamicsContactMgr* manager, hkConstraintOwner& constraintOwner )
{
	hkConstraintInstance* instance = manager->getConstraintInstance();
	if ( !instance )
	{
		return;
	}

		// If you get those asserts, check the hkResponseModifier reference manual
	constraintOwner.checkAccessRw();

	HK_TIMER_BEGIN("SetSurfVisc", HK_NULL);

	//
	// Search existing modifier list for matching type
	//
	{
		hkModifierConstraintAtom* modifier = hkWorldConstraintUtil::findModifier( instance, hkConstraintAtom::TYPE_MODIFIER_VISCOUS_SURFACE );
		if ( modifier )
		{
			goto END;
		}
	}

	//
	// Build and insert new modifier atom
	//
	{
		hkViscousSurfaceModifierConstraintAtom* viscousSurfaceContainer = new hkViscousSurfaceModifierConstraintAtom;
		hkWorldConstraintUtil::addModifier( instance, constraintOwner, viscousSurfaceContainer );
	}

END:

	HK_TIMER_END();
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
