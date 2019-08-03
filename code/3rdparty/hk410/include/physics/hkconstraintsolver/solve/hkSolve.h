/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONSTRAINTSOLVER2_SOLVE_H
#define HK_CONSTRAINTSOLVER2_SOLVE_H

#include <hkmath/hkMath.h>
#include <hkconstraintsolver/solve/hkSolverInfo.h>

class hkJacobianSchema;
class hkVelocityAccumulator;
class hkJacobianElement;

struct hkSolverElemTemp
{
	hkReal m_impulseApplied;
};


extern "C"
{
	// can't return a hkBool due to C linkage, int is used instead {0|1}
	// info pretty much read only, but cannot be shared between threads

	int HK_CALL hkSolveConstraints( const hkSolverInfo& info, hkJacobianSchema *schemas,  hkVelocityAccumulator * accumulators, hkJacobianElement* jacobians, hkSolverElemTemp* temp );

	void HK_CALL hkSolveUpload();

	void HK_CALL hkSolveStepJacobians ( const hkSolverInfo& i, hkJacobianSchema *schemas, hkVelocityAccumulator* accumulators, hkJacobianElement* jacobians, hkSolverElemTemp* tmp );

	void HK_CALL hkExportImpulsesAndRhs( const hkSolverInfo& i, const hkSolverElemTemp* temp, hkJacobianSchema *schemas, hkVelocityAccumulator* accums, hkJacobianElement* jacobians);


		/// Get the violating velocities for some constraints.
		/// Contact constraint - returns min( 0.0f, seperatingVelocity ).
		/// Bilateral constraint - returns the violating velocity.
		/// All the rest return 0.0f.
		///
		/// Continues until:
		///  - the constraint is finished by finding the end mark or a new header schema.
		///  - we reach the friction part a contact constraint.
	    ///  - we reach maxNumVelocities.
		///
		/// Returns the number of velocities calculated.
	int HK_CALL hkSolveGetToiViolatingConstraintVelocity( hkSolverInfo& i,	hkJacobianSchema *schemas, hkVelocityAccumulator* accums, hkJacobianElement* jacobians, int maxNumVelocities, hkReal* velocitiesOut );
}

#define HK_SIZE_OF_JACOBIAN_END_SCHEMA 4
#define HK_SIZE_OF_JACOBIAN_SINGLE_CONTACT_SCHEMA 4
#define HK_SIZE_OF_JACOBIAN_PAIR_CONTACT_SCHEMA 8
#define HK_SIZE_OF_JACOBIAN_1D_FRICTION_SCHEMA 12
#define HK_SIZE_OF_JACOBIAN_1D_ANGULAR_FRICTION_SCHEMA 12
#define HK_SIZE_OF_JACOBIAN_1D_ANGULAR_LIMITS_SCHEMA 16
#define HK_SIZE_OF_JACOBIAN_1D_LINEAR_LIMIT_SCHEMA 12
#define HK_SIZE_OF_JACOBIAN_1D_ANGULAR_SCHEMA 4
#define HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA	4
#define HK_SIZE_OF_JACOBIAN_1D_BILATERAL_USER_TAU_SCHEMA 12

#define HK_SIZE_OF_JACOBIAN_1D_LINEAR_LIMITS_SCHEMA	12
#define HK_SIZE_OF_JACOBIAN_1D_LINEAR_MOTOR_SCHEMA 28
#define HK_SIZE_OF_JACOBIAN_1D_ANGULAR_MOTOR_SCHEMA 28
#define HK_SIZE_OF_JACOBIAN_1D_PULLEY_SCHEMA 12
#define HK_SIZE_OF_JACOBIAN_RELATIVE_ORIENTATION_SCHEMA 4
#define HK_SIZE_OF_JACOBIAN_GOTO8_SCHEMA 4
#define HK_SIZE_OF_JACOBIAN_STIFF_SPRING_CHAIN_SCHEMA 16
#define HK_SIZE_OF_JACOBIAN_BALL_SOCKET_CHAIN_SCHEMA 16

#define HK_SIZE_OF_JACOBIAN_SET_MASS_SCHEMA 4
#define HK_SIZE_OF_JACOBIAN_ADD_VELOCITY_SCHEMA 4


#define HK_SIZE_OF_JACOBIAN_AA  32
#define HK_SIZE_OF_JACOBIAN_LAA  48
#define HK_SIZE_OF_JACOBIAN_LLAA 64
#define HK_SIZE_OF_JACOBIAN_GOTO_SCHEMA 8

#if HK_POINTER_SIZE==4
	#define HK_SIZE_OF_JACOBIAN_2D_FRICTION_SCHEMA 20
	#define HK_SIZE_OF_JACOBIAN_3D_FRICTION_SCHEMA 24
	#define HK_SIZE_OF_JACOBIAN_HEADER_SCHEMA 24
	#define HK_SIZE_OF_JACOBIAN_POWERED_CHAIN_SCHEMA 24

#elif HK_POINTER_SIZE==8
	#define HK_SIZE_OF_JACOBIAN_2D_FRICTION_SCHEMA 32
	#define HK_SIZE_OF_JACOBIAN_3D_FRICTION_SCHEMA 40
	#define HK_SIZE_OF_JACOBIAN_HEADER_SCHEMA 32
	#define HK_SIZE_OF_JACOBIAN_POWERED_CHAIN_SCHEMA 32

#else
#	error unknown pointer size
#endif



#endif // HK_CONSTRAINTSOLVER2_SOLVE_H

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
