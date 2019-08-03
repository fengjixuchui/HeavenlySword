/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONSTRAINTSOLVER2_MOTOR_CONSTRAINT_PUBLIC_H
#define HK_CONSTRAINTSOLVER2_MOTOR_CONSTRAINT_PUBLIC_H



class hkConstraintQueryIn;
class hkConstraintQueryOut;
class hkSolverResults;
class hkConstraintMotor;

/// A structure which is used for building jacobian elements
class hk1dBilateralConstraintStatus
{
public:
	/// The relative mass of the two objects in constraint space
	hkReal			m_virtualMass;
};

/// The low level input structure to set up linear and angular motors for the constraint solver
class hk1dConstraintMotorInfo
{
public:

	// <todo> not used in position motor
	/// position target for the current frame.
	hkReal				m_targetPosition;  

	/// The target velocity
	hkReal				m_targetVelocity;

	/// max force that can be applied
	hkReal				m_maxForce;

	/// max force that can be applied in the reverse direction
	hkReal				m_minForce;

	/// The relative stiffness of the motor between 0..1
	hkReal				m_tau;

	/// The relative damping of the motor between 0..1
	hkReal				m_damping;

};

/// The inputs for the hkConstraintMotor::hkCalcMotorData method
class hkConstraintMotorInput: public hk1dBilateralConstraintStatus
{
public:
	/// delta time information about the solver (
	const class hkConstraintQueryStepInfo* m_stepInfo;

	/// information from the last step
	class hkSolverResults m_lastResults;

	/// new target minus old target
	hkReal m_deltaTarget;

	/// old target minus current position
	hkReal m_positionError;

};

/// The outputs for the hkConstraintMotor::hkCalcMotorData method
class hkConstraintMotorOutput: public hk1dConstraintMotorInfo
{
};



extern "C"
{
		/// Calculate the desired position and the forces we can apply to get there according
		/// to the implemented force law.
	void HK_CALL hkCalcMotorData(const hkConstraintMotor* someMotor, const hkConstraintMotorInput* input, hkConstraintMotorOutput* output);

	// build jacobian element
	void HK_CALL hk1dAngularVelocityMotorBeginJacobian( hkVector4Parameter directionOfConstraint, const hkConstraintQueryIn &in, hkJacobianElement* constraintMatrixBlockOut, hk1dBilateralConstraintStatus& statusOut );

	// must call hk1dAngularVelocityMotorBeginJacobian first to build jacobian in hkConstraintQueryOut
	void HK_CALL hk1dAngularVelocityMotorCommitJacobian( hk1dConstraintMotorInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	// build jacobian element
	void HK_CALL hk1dLinearVelocityMotorBeginJacobian( hkVector4Parameter directionOfConstraint, hkVector4Parameter pivot, const hkConstraintQueryIn &in, hkJacobianElement* constraintMatrixBlockOut, hk1dBilateralConstraintStatus& statusOut );

	// must call hk1dLinearVelocityMotorBeginJacobian first to build jacobian in hkConstraintQueryOut
	void HK_CALL hk1dLinearVelocityMotorCommitJacobian( hk1dConstraintMotorInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

}


#endif // HK_CONSTRAINTSOLVER_MOTOR_CONSTRAINT_PUBLIC_H

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
