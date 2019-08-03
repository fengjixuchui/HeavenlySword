/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>
#include <hkconstraintsolver/simpleConstraints/hkSimpleConstraintUtil.h>

void HK_CALL hkBridgeConstraintAtom_callData( class hkConstraintData* m_constraintData, const hkConstraintQueryIn &in, hkConstraintQueryOut &out )
{
	m_constraintData->buildJacobian( in, out );
}

void hkBridgeConstraintAtom::init( class hkConstraintData* constraintData )
{
	this->m_constraintData = constraintData;
	this->m_buildJacobianFunc = hkBridgeConstraintAtom_callData;
}


void hkMassChangerModifierConstraintAtom::collisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	hkReal massScaleFactorA = m_factorA;
	hkReal massScaleFactorB = m_factorB;

	inA.m_invMass *= massScaleFactorA;
	inA.m_invInertia.mul( massScaleFactorA );

	inB.m_invMass *= massScaleFactorB;
	inB.m_invInertia.mul( massScaleFactorB );
}

void hkMassChangerModifierConstraintAtom::collisionResponseEndCallback( const hkContactPoint& cp, float impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
}

static hkBodyVelocity s_bodyVelocities[2];
static hkBool s_bodyVelocitiesInitialized = false;

void hkSoftContactModifierConstraintAtom::collisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	HK_WARN_ONCE(0x12015a6e, "Using soft contacts (scaling of response force) in TOI events. This may cause a significant performance drop. This pair of bodies should not use continuous collision detection. Reduce the quality type of either of the bodies.");
	HK_ASSERT2(0x172400f2, s_bodyVelocitiesInitialized == false, "hkSoftContactConstraintData uses static variables for processing of TOI collision. It assumed that TOI are always processed in series one after another. If that's changed this class has to be rewritten. ");
	s_bodyVelocities[0] = velA;
	s_bodyVelocities[1] = velB;
}

void hkSoftContactModifierConstraintAtom::collisionResponseEndCallback( const hkContactPoint& cp, float impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	hkReal usedImpulseFraction = this->m_tau;

	hkBodyVelocity& oldA = s_bodyVelocities[0];
	hkBodyVelocity& oldB = s_bodyVelocities[1];

	velA.m_linear.setInterpolate4(oldA.m_linear, velA.m_linear, usedImpulseFraction);
	velA.m_angular.setInterpolate4(oldA.m_angular, velA.m_angular, usedImpulseFraction);
	velB.m_linear.setInterpolate4(oldB.m_linear, velB.m_linear, usedImpulseFraction);
	velB.m_angular.setInterpolate4(oldB.m_angular, velB.m_angular, usedImpulseFraction);
}

void hkMovingSurfaceModifierConstraintAtom::collisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	hkVector4 vel = getVelocity();
	// project velocity into the contact plane (so that objects do not sink in)
	hkSimdReal dot = vel.dot3(cp.getNormal());
	hkVector4 perp; perp.setMul4( dot, vel );
	vel.sub4( perp );
	velB.m_linear.add4( vel );
}

void hkMovingSurfaceModifierConstraintAtom::collisionResponseEndCallback( const hkContactPoint& cp, float impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	hkVector4 vel = getVelocity();

	// project velocity into the contact plane (so that objects do not sink in)
	hkSimdReal dot = vel.dot3(cp.getNormal());
	hkVector4 perp; perp.setMul4( dot, vel );
	vel.sub4( perp );
	velB.m_linear.sub4( vel );
}


int hkModifierConstraintAtom::addModifierDataToConstraintInfo( hkConstraintInfo& cinfo ) const
{
	int callBackRequest = CALLBACK_REQUEST_NONE;
	const hkModifierConstraintAtom* modifier = this;

	switch( modifier->getType() )
	{

	case hkConstraintAtom::TYPE_MODIFIER_VISCOUS_SURFACE:
		{
			const hkViscousSurfaceModifierConstraintAtom* m = static_cast<const hkViscousSurfaceModifierConstraintAtom*>( modifier );
			callBackRequest |= m->getConstraintInfo ( cinfo );
			break;
		}
	case hkConstraintAtom::TYPE_MODIFIER_SOFT_CONTACT:
		{
			const hkSoftContactModifierConstraintAtom* m = static_cast<const hkSoftContactModifierConstraintAtom*>( modifier );
			callBackRequest |= m->getConstraintInfo ( cinfo );
			break;
		}

	case hkConstraintAtom::TYPE_MODIFIER_MASS_CHANGER:
		{
			const hkMassChangerModifierConstraintAtom* m = static_cast<const hkMassChangerModifierConstraintAtom*>( modifier );
			callBackRequest |= m->getConstraintInfo ( cinfo );
			break;
		}

	case hkConstraintAtom::TYPE_MODIFIER_MOVING_SURFACE:
		{
			const hkMovingSurfaceModifierConstraintAtom* m = static_cast<const hkMovingSurfaceModifierConstraintAtom*>( modifier );
			callBackRequest |= m->getConstraintInfo ( cinfo );
			break;
		}

	default:
		{
			HK_ASSERT2(0xaf673682, 0, "Unknown constraint modifier type.");
			break;
		}
	}
	return callBackRequest;
}

int HK_CALL hkModifierConstraintAtom::addAllModifierDataToConstraintInfo( hkModifierConstraintAtom* modifier, hkConstraintInfo& cinfo )
{
	int callBackRequest = CALLBACK_REQUEST_NONE;
	
	hkConstraintAtom* atom = modifier;

	while ( 1 )
	{
		// abort if we reached the constraint's original atom
		if ( !atom->isModifierType() )
		{
			break;
		}
		hkModifierConstraintAtom* mac = reinterpret_cast<hkModifierConstraintAtom*>(atom);

		callBackRequest |= mac->addModifierDataToConstraintInfo( cinfo );

		atom = mac->m_child;
	}
	return callBackRequest;
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
