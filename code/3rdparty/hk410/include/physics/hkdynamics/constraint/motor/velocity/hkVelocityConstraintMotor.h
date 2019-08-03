/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_VELOCITY_CONSTRAINT_MOTOR_H
#define HK_DYNAMICS2_VELOCITY_CONSTRAINT_MOTOR_H

#include <hkdynamics/constraint/motor/hkLimitedForceConstraintMotor.h>

extern const hkClass hkVelocityConstraintMotorClass;

	/// this motor simply tries to reach and keep a desired target velocity
class hkVelocityConstraintMotor : public hkLimitedForceConstraintMotor
{
	public:

		HK_DECLARE_REFLECTION();

			/// Empty constructor. Will zero all values.
		inline hkVelocityConstraintMotor();

			/// Construct a motor with the given properties.
		inline hkVelocityConstraintMotor( hkReal velocityTarget, hkReal maxForce );

			/// Motor clone method
		virtual hkConstraintMotor* clone() const;


	public:
			/// The stiffness of the motor (0..1)
		hkReal m_tau;

			/// velocity target. 
		hkReal m_velocityTarget;
			
			/// Tells the motor to ignore the stored velocity target. 
			/// When set to true, the motor calculates the requested velocity from the target orientations
			/// supplied by the constraint owning the motor.
		hkBool m_useVelocityTargetFromConstraintTargets;

	public:

		hkVelocityConstraintMotor(hkFinishLoadedObjectFlag f) : hkLimitedForceConstraintMotor(f) {}
};

#include <hkdynamics/constraint/motor/velocity/hkVelocityConstraintMotor.inl>

#endif // HK_DYNAMICS2_VELOCITY_CONSTRAINT_MOTOR_H

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
