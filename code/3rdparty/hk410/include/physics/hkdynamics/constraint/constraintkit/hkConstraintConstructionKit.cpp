/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/constraintkit/hkConstraintConstructionKit.h>
#include <hkdynamics/constraint/constraintkit/hkGenericConstraintScheme.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/constraint/motor/hkConstraintMotor.h>

// set up construction kit

void hkConstraintConstructionKit::begin( hkGenericConstraintData* constraint )
{
	HK_ASSERT2(0x54566fc5,  hkGenericConstraintDataScheme::e_numCommands <= 0xFF, "constraint construction kit: too many opcodes" );
	m_scheme = constraint->getScheme();
	m_constraint = constraint;
	m_stiffnessReference = 0;
	m_dampingReference = 0;

	m_linearDofSpecifiedA[0] = false;
	m_linearDofSpecifiedA[1] = false;
	m_linearDofSpecifiedA[2] = false;
	m_linearDofSpecifiedB[0] = false;
	m_linearDofSpecifiedB[1] = false;
	m_linearDofSpecifiedB[2] = false;
	m_pivotSpecifiedA = false;
	m_pivotSpecifiedB = false;

	m_angularBasisSpecifiedA = false;
	m_angularBasisSpecifiedB = false;
}

// linear constraint

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setLinearDofA( const hkVector4& dof, int axis )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setLinearDofA );
	m_scheme->m_commands.pushBack( axis );
	const int dataIndex = m_scheme->m_data.getSize(); 
	m_scheme->m_data.pushBack( dof );
	m_linearDofSpecifiedA[axis] = true;
	return dataIndex;
}

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setLinearDofB( const hkVector4& dof, int axis )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setLinearDofB );
	m_scheme->m_commands.pushBack( axis );
	const int dataIndex = m_scheme->m_data.getSize(); 
	m_scheme->m_data.pushBack( dof );
	m_linearDofSpecifiedB[axis] = true;
	return dataIndex;
}

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setLinearDofWorld( const hkVector4& dof, int axis )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setLinearDofW );
	m_scheme->m_commands.pushBack( axis );
	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( dof );
	m_linearDofSpecifiedA[axis] = true;
	m_linearDofSpecifiedB[axis] = true;
	return dataIndex;
}

void hkConstraintConstructionKit::constrainLinearDof( int axis )
{
	HK_ASSERT2(0x1583f683, (m_pivotSpecifiedA == true), "Cannot constrain Linear DOF: Pivot not yet specified in A space");
	HK_ASSERT2(0x2404229b, (m_pivotSpecifiedB == true), "Cannot constrain Linear DOF: Pivot not yet specified in B space");
	HK_ASSERT2(0x6bf36a59, ((m_linearDofSpecifiedB[axis] == true) || (m_linearDofSpecifiedA[axis] == true)), "Cannot constrain Linear DOF: Axis not yet specified");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_constrainLinearW );
	m_scheme->m_commands.pushBack( axis );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
}

void hkConstraintConstructionKit::constrainAllLinearDof()
{
	HK_ASSERT2(0x57a68a69, (m_pivotSpecifiedA == true), "Cannot constrain All Linear DOF: Pivot not yet specified in A space");
	HK_ASSERT2(0x6311d307, (m_pivotSpecifiedB == true), "Cannot constrain All Linear DOF: Pivot not yet specified in B space");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_constrainAllLinearW );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
}

// angular constraint

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setAngularBasisA( const hkMatrix3& dofBasis )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setAngularBasisA );
	const int dataIndex = m_scheme->m_data.getSize(); 
	m_scheme->m_data.pushBack( dofBasis.getColumn(0) );
	m_scheme->m_data.pushBack( dofBasis.getColumn(1) );
	m_scheme->m_data.pushBack( dofBasis.getColumn(2) );
	m_angularBasisSpecifiedA = true;
	return dataIndex;
}

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setAngularBasisB( const hkMatrix3& dofBasis )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setAngularBasisB );
	const int dataIndex = m_scheme->m_data.getSize(); 
	m_scheme->m_data.pushBack( dofBasis.getColumn(0) );
	m_scheme->m_data.pushBack( dofBasis.getColumn(1) );
	m_scheme->m_data.pushBack( dofBasis.getColumn(2) );
	m_angularBasisSpecifiedB = true;
	return dataIndex;
}	

void hkConstraintConstructionKit::setAngularBasisABodyFrame()
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setAngularBasisAidentity );
	m_angularBasisSpecifiedA = true;
}

void hkConstraintConstructionKit::setAngularBasisBBodyFrame()
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setAngularBasisBidentity );
	m_angularBasisSpecifiedB = true;
}


// constrain object to only rotate around degree of freedom specified.
void hkConstraintConstructionKit::constrainToAngularDof( int axis )
{
	HK_ASSERT2(0x530d4b4d, (m_angularBasisSpecifiedA == true), "Cannot constrain Angular DOF: Basis not yet specified in A space");
	HK_ASSERT2(0x21a81142, (m_angularBasisSpecifiedB == true), "Cannot constrain Angular DOF: Basis not yet specified in B space");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_constrainToAngularW );
	m_scheme->m_commands.pushBack( axis );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
}

// constrain object so it can not rotate relative to second object
void hkConstraintConstructionKit::constrainAllAngularDof()
{
	HK_ASSERT2(0x66d851d1, (m_angularBasisSpecifiedA == true), "Cannot constrain Angular DOF: Basis not yet specified in A space");
	HK_ASSERT2(0x34708bb6, (m_angularBasisSpecifiedB == true), "Cannot constrain Angular DOF: Basis not yet specified in B space");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_constrainAllAngularW );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
}

// pivot point

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setPivotA( const hkVector4& pivot )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setPivotA );
	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( pivot );
	m_pivotSpecifiedA = true;
	return dataIndex;
}

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setPivotB( const hkVector4& pivot )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setPivotB );
	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( pivot );
	m_pivotSpecifiedB = true;
	return dataIndex;
}

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setPivotsHelper( hkRigidBody* bodyA, hkRigidBody* bodyB, const hkVector4& pivot )
{

	hkVector4 attachA, attachB;
	attachA.setTransformedInversePos( bodyA->getTransform(), pivot );
	attachB.setTransformedInversePos( bodyB->getTransform(), pivot );
	
	int dataIndex = setPivotA( attachA );
	setPivotB( attachB );

	m_pivotSpecifiedA = true;
 	m_pivotSpecifiedB = true;
 
	return dataIndex;
}

// limits, friction, motors

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setLinearLimit( int axis, hkReal min, hkReal max )
{
	HK_ASSERT2(0x55ac42ef, (m_pivotSpecifiedA == true), "Cannot set limits for Linear DOF: Pivot not yet specified in A space");
	HK_ASSERT2(0x330b7c7e, (m_pivotSpecifiedB == true), "Cannot set limits for Linear DOF: Pivot not yet specified in B space");
	HK_ASSERT2(0x6533be94, ((m_linearDofSpecifiedB[axis] == true) || (m_linearDofSpecifiedA[axis] == true)), "Cannot constrain Linear DOF: Axis not yet specified");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setLinearLimit );
	m_scheme->m_commands.pushBack( hkUchar(axis) );
	hkVector4 limit; limit.set( min, max, 0, 0 );
	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( limit );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_LINEAR_LIMITS_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
	return dataIndex;
}

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setAngularLimit( int axis, hkReal min, hkReal max )
{
	HK_ASSERT2(0x1396cee8, (m_angularBasisSpecifiedA == true), "Cannot set limits for Angular DOF: Basis not yet specified in A space");
	HK_ASSERT2(0x7c1a15e6, (m_angularBasisSpecifiedB == true), "Cannot set limits for Angular DOF: Basis not yet specified in B space");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setAngularLimit );
	m_scheme->m_commands.pushBack( hkUchar(axis) );
	hkVector4 limit; limit.set( min, max, 0, 0 );  //!me change to 
	//!me class hkLimitParam : public class hkVector4{ hkLimitVector( min,max) getMin() setMin()
	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( limit );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_LIMITS_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	return dataIndex;
}


hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setConeLimit( int axis, hkReal angle )
{
	HK_ASSERT2(0x6ff88eba, (m_angularBasisSpecifiedA == true), "Cannot set limits for Angular DOF: Basis not yet specified in A space");
	HK_ASSERT2(0x3f73c278, (m_angularBasisSpecifiedB == true), "Cannot set limits for Angular DOF: Basis not yet specified in B space");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setConeLimit );
	m_scheme->m_commands.pushBack( hkUchar(axis) );
	hkVector4 limit; limit.set( hkMath::cos(angle), 100.0f, 0, 0 );  //!me change to 

	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( limit );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_LIMITS_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	return dataIndex;
}


hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setTwistLimit( int twistAxis, int planeAxis, hkReal min, hkReal max )
{

	HK_ASSERT2(0x7d13f7fc, (m_angularBasisSpecifiedA == true), "Cannot set limits for Angular DOF: Basis not yet specified in A space");
	HK_ASSERT2(0x2adc9420, (m_angularBasisSpecifiedB == true), "Cannot set limits for Angular DOF: Basis not yet specified in B space");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setTwistLimit );
	m_scheme->m_commands.pushBack( hkUchar(twistAxis) );
	m_scheme->m_commands.pushBack( hkUchar(planeAxis) );
	hkVector4 limit; limit.set( hkMath::sin(min), hkMath::sin(max), 0, 0 );  //!me change to 

	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( limit );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_LIMITS_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	return dataIndex;

}


hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setAngularMotor( int axis, hkConstraintMotor* motor )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setAngularMotor );
	m_scheme->m_commands.pushBack( hkUchar(axis) );

	const int dataIndex = m_scheme->m_data.getSize();
	hkVector4 motorData; motorData.set( static_cast< float >(m_scheme->m_motors.getSize()), 0, 0 );
	
	HK_ASSERT2(0x647f8842,  motor, "can't add a null motor with constraint kit" );

	motor->addReference();

	m_scheme->m_data.pushBack( motorData ); 
	m_scheme->m_motors.pushBack( motor );

	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_MOTOR_SCHEMA, HK_SIZE_OF_JACOBIAN_AA  );
	return dataIndex;
}

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setLinearMotor( int axis, hkConstraintMotor* motor )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setLinearMotor );
	m_scheme->m_commands.pushBack( hkUchar(axis) );

	const int dataIndex = m_scheme->m_data.getSize();
	hkVector4 motorData; motorData.set( static_cast< float >(m_scheme->m_motors.getSize()), 0, 0 );
	
	HK_ASSERT2(0x6838c050,  motor, "can't add a null motor with constraint kit" );

	motor->addReference();

	m_scheme->m_data.pushBack( motorData ); 
	m_scheme->m_motors.pushBack( motor );

	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_LINEAR_MOTOR_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA  );
	return dataIndex;
}


hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setAngularFriction( int axis, hkReal maxImpulse )
{
	HK_ASSERT2(0x1fa2673c, (m_angularBasisSpecifiedA == true), "Cannot set friction for Angular DOF: Basis not yet specified in A space");
	HK_ASSERT2(0x526a3b98, (m_angularBasisSpecifiedB == true), "Cannot set friction for Angular DOF: Basis not yet specified in B space");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setAngularFriction );
	m_scheme->m_commands.pushBack( hkUchar(axis) );
	hkVector4 coef; coef.set( maxImpulse, 0, 0, 0 );
	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( coef );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_FRICTION_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	return dataIndex;
}

hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setLinearFriction( int axis, hkReal maxImpulse )
{
	HK_ASSERT2(0x3e9305ff, (m_pivotSpecifiedA == true), "Cannot set friction for Linear DOF: Pivot not yet specified in A space");
	HK_ASSERT2(0x32b39556, (m_pivotSpecifiedB == true), "Cannot set friction for Linear DOF: Pivot not yet specified in B space");
	HK_ASSERT2(0x2e60e5f2, (m_linearDofSpecifiedA[axis] == true || m_linearDofSpecifiedB[axis] == true), "Cannot set limits for Linear DOF: Axis not yet specified in neither A or B space");
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setLinearFriction );
	m_scheme->m_commands.pushBack( hkUchar(axis) );
	hkVector4 coef; coef.set( maxImpulse, 0, 0, 0 );  //!me change to hkFrictionParam
	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( coef );
	m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_1D_FRICTION_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
	return dataIndex;
}

// tau, damping


	/// Sets the stiffness of the subsequent constraints.  remember to restore it later
hkGenericConstraintData::hkParameterIndex hkConstraintConstructionKit::setStrength( hkReal strength )
{
	m_stiffnessReference++;
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_setStrength );
	hkVector4 tauAndDampingV; tauAndDampingV.set( strength, 0, 0, 0 );
	const int dataIndex = m_scheme->m_data.getSize();
	m_scheme->m_data.pushBack( tauAndDampingV );
	return dataIndex;
}


	/// restore the stiffness back to solver defaults
void hkConstraintConstructionKit::restoreStrength()
{
	m_stiffnessReference--;
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_restoreStrengh );
	//m_scheme->m_info.add( HK_SIZE_OF_JACOBIAN_RESTORE_STIFFNESS_DAMPING_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
}

//modifiers

void hkConstraintConstructionKit::addConstraintModifierCallback( hkConstraintModifier *cm, int userData )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_doConstraintModifier );
	m_scheme->m_modifiers.pushBack( cm );
	m_scheme->m_commands.pushBack( userData ); 
}


void hkConstraintConstructionKit::addRhsModifierCallback( hkConstraintModifier *cm, int userData )
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_doRhsModifier );
	m_scheme->m_modifiers.pushBack( cm );
	m_scheme->m_commands.pushBack( userData ); 
}

// commands

void hkConstraintConstructionKit::end()
{
	m_scheme->m_commands.pushBack( hkGenericConstraintDataScheme::e_endScheme );
	HK_ASSERT(0x61bd39b8,  m_stiffnessReference == 0 );
	HK_ASSERT(0x38259980,  m_dampingReference == 0 );
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
