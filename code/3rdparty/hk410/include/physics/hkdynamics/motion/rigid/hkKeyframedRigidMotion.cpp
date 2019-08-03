/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/motion/rigid/hkKeyframedRigidMotion.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkKeyframedRigidMotion);
HK_REFLECTION_DEFINE_VIRTUAL(hkMaxSizeMotion);

HK_COMPILE_TIME_ASSERT( sizeof( hkKeyframedRigidMotion) <= sizeof( hkMaxSizeMotion));

hkKeyframedRigidMotion::hkKeyframedRigidMotion(const hkVector4& position, const hkQuaternion& rotation) :
	hkMotion( position, rotation),
	m_savedMotion(HK_NULL),
	m_savedQualityTypeIndex(0)
{
		// The "mass" of a keyframed body is infinite. It must be treated as "unmovable" (though not, obviously, "unmoving"!)
		// by the solver. This actually happens explicitly in the hkRigidMotionUtilApplyForcesAndBuildAccumulators() (which doesn't
		// acually read these values, but we set them anyway for consistency.
	m_inertiaAndMassInv.setZero4();
	m_type = MOTION_KEYFRAMED;
}

hkKeyframedRigidMotion::~hkKeyframedRigidMotion()
{
	if (m_savedMotion)
	{
		m_savedMotion->removeReference();
	}
}

void hkKeyframedRigidMotion::setMass(hkReal m)
{
	HK_ASSERT2(0xad67f4d3, 0, "Error: do not call setMass on a fixed or keyframed object(hkKeyframedRigidMotion)");
}

void hkKeyframedRigidMotion::setMassInv(hkReal mInv)
{
	HK_ASSERT2(0xad67f4d4, 0, "Error: do not call setMassInv on a fixed or keyframed object(hkKeyframedRigidMotion)");
}

void hkKeyframedRigidMotion::getInertiaLocal(hkMatrix3& inertia) const
{
	// The "mass" of a keyframed body is infinite. It must be treated as "unmovable" (though not, obviously "unmoving"!)
	// by the solver. 
	// We create an "invalid" inertia, since it should never be used (would have to be infinite!)
	inertia.setZero();
}

void hkKeyframedRigidMotion::getInertiaWorld(hkMatrix3& inertia) const
{
	// The "mass" of a keyframed body is infinite. It must be treated as "unmovable" (though not, obviously "unmoving"!)
	// by the solver. 
	// We create an "invalid" inertia, since it should never be used (would have to be infinite!)
	inertia.setZero();
}

void hkKeyframedRigidMotion::setInertiaLocal(const hkMatrix3& inertia)
{
	HK_ASSERT2(0x28204ab9, 0, "Error: do not call setInertiaLocal on a fixed of keyframed object (hkKeyframedRigidMotion)");
}


void hkKeyframedRigidMotion::setInertiaInvLocal(const hkMatrix3& inertiaInv)
{
	HK_ASSERT2(0x7b611123, 0, "Error: do not call setInertiaInvLocal on a fixed or keyframed object (hkKeyframedRigidMotion)");
}

void hkKeyframedRigidMotion::getInertiaInvLocal(hkMatrix3& inertiaInv) const
{
	// The "mass" of a keyframed body is infinite. It must be treated as "unmovable" (though not, obviously "unmoving"!)
	// by the solver. 
	inertiaInv.setZero();
}

void hkKeyframedRigidMotion::getInertiaInvWorld(hkMatrix3& inertiaInvOut) const
{
	// The "mass" of a keyframed body is infinite. It must be treated as "unmovable" (though not, obviously "unmoving"!)
	// by the solver. 
	inertiaInvOut.setZero();
}

void hkKeyframedRigidMotion::applyLinearImpulse(const hkVector4& imp)
{
}

void hkKeyframedRigidMotion::applyPointImpulse(const hkVector4& imp, const hkVector4& p)
{
}

void hkKeyframedRigidMotion::applyAngularImpulse(const hkVector4& imp)
{
}

void hkKeyframedRigidMotion::applyForce(const hkReal deltaTime, const hkVector4& force)
{
}

void hkKeyframedRigidMotion::applyForce(const hkReal deltaTime, const hkVector4& force, const hkVector4& p)
{
}

void hkKeyframedRigidMotion::applyTorque(const hkReal deltaTime, const hkVector4& torque)
{
}

void hkKeyframedRigidMotion::setStepPosition( hkReal position, hkReal timestep )
{
}

void hkKeyframedRigidMotion::setStoredMotion( hkMaxSizeMotion* savedMotion )
{
	if (savedMotion)
	{
		savedMotion->addReference();
	}
	if (m_savedMotion)
	{
		m_savedMotion->removeReference();
	}
	m_savedMotion = savedMotion;
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
