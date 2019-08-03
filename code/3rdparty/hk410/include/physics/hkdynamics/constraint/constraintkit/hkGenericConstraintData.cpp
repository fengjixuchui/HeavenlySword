/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/constraintkit/hkGenericConstraintData.h>
#include <hkdynamics/constraint/constraintkit/hkConstraintModifier.h>
#include <hkdynamics/constraint/constraintkit/hkGenericConstraintParameters.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkdynamics/constraint/motor/hkConstraintMotor.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>
#include <hkconstraintsolver/constraint/bilateral/hkInternalConstraintUtils.h>
#include <hkconstraintsolver/constraint/motor/hkMotorConstraintInfo.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkGenericConstraintData);

hkGenericConstraintData::hkGenericConstraintData() 
{
	m_scheme.m_info.clear();
	m_scheme.m_info.addHeader();
	m_atoms.m_bridgeAtom.init( this );
}

hkGenericConstraintData::hkGenericConstraintData(hkFinishLoadedObjectFlag f) : hkConstraintData(f), m_atoms(f), m_scheme(f)
{
	m_atoms.m_bridgeAtom.init( this );
}


hkGenericConstraintData::~hkGenericConstraintData()
{
	int i;
	for( i = 0; i < m_scheme.m_motors.getSize(); i++ )
	{
		m_scheme.m_motors[i]->removeReference();
	}

	//!me should constraint modifiers be reference counted as well?
}

void hkGenericConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& info ) const
{
	info.m_atoms = const_cast<hkConstraintAtom*>(m_atoms.getAtoms());
	info.m_sizeOfAllAtoms = m_atoms.getSizeOfAllAtoms();
	info.clear();
	(hkConstraintInfo&)info = m_scheme.m_info;
}

void hkGenericConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	// we need runtime data to be able to support lastAngle and friction
	infoOut.m_numSolverResults = m_scheme.m_info.m_numSolverResults;
	infoOut.m_sizeOfExternalRuntime = sizeof( hkSolverResults) * infoOut.m_numSolverResults;
}

void hkGenericConstraintData::buildJacobian( const hkConstraintQueryIn &in, hkConstraintQueryOut &out )
{
	hkSolverResults* solverResults = static_cast<hkSolverResults*>( in.m_constraintRuntime.val() );
	hkBeginConstraints( in, out, solverResults, sizeof(hkSolverResults) );
	hatchScheme( &m_scheme, in, out );
	hkEndConstraints();
}


hkVector4* hkGenericConstraintData::getParameters( hkParameterIndex parameterIndex )
{
	return &m_scheme.m_data[ parameterIndex ];
}


void hkGenericConstraintData::setParameters( hkParameterIndex parameterIndex, int numParameters, const hkVector4* newValues )
{
	int i;
	for( i = parameterIndex; i < parameterIndex + numParameters; i++, newValues++ )
	{
		m_scheme.m_data[ i ] = *newValues;
	}
}

//
// commands 
//

// linear constraints

void hkGenericConstraintData::constrainAllLinearW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{
	hk1dLinearBilateralConstraintInfo bp;
	bp.m_pivotA = vars.m_pivotAw; 
	bp.m_pivotB = vars.m_pivotBw;
	bp.m_constrainedDofW.setZero4();
	bp.m_constrainedDofW(0) = 1.0f;
	hk1dLinearBilateralConstraintBuildJacobian( bp, in, out );
	bp.m_constrainedDofW(0) = 0.0f;
	bp.m_constrainedDofW(1) = 1.0f;
	hk1dLinearBilateralConstraintBuildJacobian( bp, in, out );
	bp.m_constrainedDofW(1) = 0.0f;
	bp.m_constrainedDofW(2) = 1.0f;
	hk1dLinearBilateralConstraintBuildJacobian( bp, in, out );
	vars.m_currentResult += 3;
}

inline void hkGenericConstraintData::constrainLinearW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{
	hk1dLinearBilateralConstraintInfo bp;
	currentCommand++;
	bp.m_constrainedDofW = vars.m_linearBasisW.getColumn( *currentCommand );
	bp.m_pivotA = vars.m_pivotAw; 
	bp.m_pivotB = vars.m_pivotBw;
	hk1dLinearBilateralConstraintBuildJacobian( bp, in, out );
	vars.m_currentResult++;
}

static int hkGenericConstraintDataAxisOrder[5] = { 0, 1, 2, 0, 1 };  // use to avoid any conditionals or modulus

static hkReal HK_CALL calcDeltaAngleAroundAxis( int axis, const hkGenericConstraintDataParameters& vars )
{
	const hkVector4& zeroErrorAxisAinW = vars.m_angularBasisAw.getColumn( hkGenericConstraintDataAxisOrder[axis+1] );
	const hkVector4& negZeroErrorAxisBinW = vars.m_angularBasisBw.getColumn( hkGenericConstraintDataAxisOrder[axis+2] );

	hkReal sinTheta = negZeroErrorAxisBinW.dot3( zeroErrorAxisAinW );
	const hkVector4& cosCheck = vars.m_angularBasisBw.getColumn( hkGenericConstraintDataAxisOrder[axis+1] );
	hkReal cosTheta = cosCheck.dot3( zeroErrorAxisAinW );
	
	//!me do I need atan2???  is it just to determine quadrant?  does small angle formula work here?
	return hkMath::atan2fApproximation( sinTheta, cosTheta );
}



inline void hkGenericConstraintData::constrainToAngularW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{
	currentCommand++;
	int axis = *currentCommand;

	hk1dAngularBilateralConstraintInfo bp;
	bp.m_constrainedDofW = vars.m_angularBasisAw.getColumn( hkGenericConstraintDataAxisOrder[axis+1] );
	bp.m_zeroErrorAxisAinW = vars.m_angularBasisAw.getColumn( hkGenericConstraintDataAxisOrder[axis+2] );
	bp.m_perpZeroErrorAxisBinW = vars.m_angularBasisBw.getColumn( axis );
	hk1dAngularBilateralConstraintBuildJacobian( bp, in, out );	
	
	// keep non-constrained axis as the axis from B from which error is measured
	// so we need to negate one of the other vectors to keep AxB = C
	hkVector4 negated;
	negated.setNeg4( bp.m_constrainedDofW );
	bp.m_constrainedDofW = bp.m_zeroErrorAxisAinW;
	bp.m_zeroErrorAxisAinW = negated;
	hk1dAngularBilateralConstraintBuildJacobian( bp, in, out );	
	vars.m_currentResult += 2;

}



inline void hkGenericConstraintData::constrainAllAngularW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{
	hk1dAngularBilateralConstraintInfo bp;

	bp.m_zeroErrorAxisAinW = vars.m_angularBasisAw.getColumn( 0 );
	bp.m_perpZeroErrorAxisBinW = vars.m_angularBasisBw.getColumn( 1 );
	bp.m_constrainedDofW = vars.m_angularBasisAw.getColumn( 2 );
	hk1dAngularBilateralConstraintBuildJacobian( bp, in, out );	
	
	hk1dAngularBilateralConstraintInfo bp2;
	bp2.m_zeroErrorAxisAinW = vars.m_angularBasisAw.getColumn( 1 );
	bp2.m_perpZeroErrorAxisBinW = vars.m_angularBasisBw.getColumn( 2 );
	bp2.m_constrainedDofW = bp.m_zeroErrorAxisAinW;
	hk1dAngularBilateralConstraintBuildJacobian( bp2, in, out );

	bp2.m_perpZeroErrorAxisBinW = vars.m_angularBasisBw.getColumn( 0 );
	bp2.m_constrainedDofW = bp2.m_zeroErrorAxisAinW;
	bp2.m_zeroErrorAxisAinW = bp.m_constrainedDofW;
	hk1dAngularBilateralConstraintBuildJacobian( bp2, in, out );
	vars.m_currentResult += 3;

}


// limits, friction, motors


inline void hkGenericConstraintData::setLinearLimitW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{
	HK_ALIGN16( hk1dLinearLimitInfo bp );
	currentCommand++;
	bp.m_constrainedDofW = vars.m_linearBasisW.getColumn( *currentCommand );
	bp.m_pivotA = vars.m_pivotAw; 
	bp.m_pivotB = vars.m_pivotBw;
	const hkVector4& limit = *currentData++;
	bp.m_min = limit(0);
	bp.m_max = limit(1);
	hk1dLinearLimitBuildJacobian( bp, in, out );
	vars.m_currentResult++;
}



inline void hkGenericConstraintData::setAngularLimitW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{
	currentCommand++;
	int axis = *currentCommand;

	hk1dAngularLimitInfo ali;
	ali.m_tau = 0.5f;
	ali.m_constrainedDofW = vars.m_angularBasisAw.getColumn( axis );

	ali.m_computedAngle = calcDeltaAngleAroundAxis( axis, vars );

	hkVector4& limit = *currentData;
	currentData++;
	ali.m_min = limit(0);
	ali.m_max = limit(1);

	if(ali.m_computedAngle < 0.0f && limit(2) > 0.0f)
	{
		if((limit(2) - ali.m_computedAngle) > HK_REAL_PI)
		{
			ali.m_computedAngle	+= 2.0f * HK_REAL_PI;
		}
	}

	if(ali.m_computedAngle > 0.0f && limit(2) < 0.0f)
	{
		if((ali.m_computedAngle - limit(2)) > HK_REAL_PI)
		{
			ali.m_computedAngle -= 2.0f * HK_REAL_PI;
		}
	}

	limit(2) = ali.m_computedAngle;
	
	
	hk1dAngularLimitBuildJacobian( ali, in, out );
	vars.m_currentResult++;
}


inline void hkGenericConstraintData::setConeLimitW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{

	currentCommand++;
	int axis = *currentCommand;

	hk1dAngularLimitInfo ali;
	ali.m_tau = 0.5f;
	hkVector4 twist = vars.m_angularBasisAw.getColumn( axis );
	hkVector4 twistRef = vars.m_angularBasisBw.getColumn( axis );

	ali.m_tau = 0.5f;
	ali.m_constrainedDofW.setCross( twist, twistRef );

	hkReal lenSqrd = ali.m_constrainedDofW.lengthSquared3();
	
	// we have a dead spot in the middle because we don't know which direction we are going there
	if( lenSqrd < HK_REAL_EPSILON )
	{
		return;
	}
	ali.m_constrainedDofW.normalize3();

	// cos angle 
	ali.m_computedAngle = twist.dot3fpu( twistRef );

	hkVector4& limit = *currentData;
	currentData++;
	ali.m_min = limit(0);
	ali.m_max = limit(1);

	hk1dAngularLimitBuildJacobian( ali, in, out );
	vars.m_currentResult++;

}


inline void hkGenericConstraintData::setTwistLimitW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{

	currentCommand++;
	int axis = *currentCommand;

	currentCommand++;
	int planeAxis = *currentCommand;

	hk1dAngularLimitInfo ali;
	ali.m_tau = 0.5f;
	const hkVector4& twistAxisAinWorld = vars.m_angularBasisAw.getColumn( axis );
	const hkVector4& twistAxisBinWorld = vars.m_angularBasisBw.getColumn( axis );

	// twist

	// Calculate "twist" angle explicitly
	{
		const hkVector4& planeAxisAinWorld = vars.m_angularBasisAw.getColumn( planeAxis );
		const hkVector4& planeAxisBinWorld = vars.m_angularBasisBw.getColumn( planeAxis );

		hkInternalConstraintUtils_calcRelativeAngle( twistAxisAinWorld, twistAxisBinWorld,
			planeAxisAinWorld, planeAxisBinWorld, 
			ali.m_constrainedDofW, ali.m_computedAngle );

	}

	hkVector4& limit = *currentData;
	currentData++;
	ali.m_min = limit(0);
	ali.m_max = limit(1);

	hk1dAngularLimitBuildJacobian( ali, in, out );
	vars.m_currentResult++;

}


inline void hkGenericConstraintData::setAngularMotorW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{

	// extract info from scheme, do some setup. 
	currentCommand++;
	int axis = *currentCommand;

	hkVector4* motorData = currentData;
	hkSolverResults* solverResults = static_cast<hkSolverResults*>( in.m_constraintRuntime.val() );
	currentData++;

	int motorIndex = static_cast< int >((*motorData)(0));
	hkReal lastAngle = (*motorData)(1);
	hkReal rotations = (*motorData)(2);

	hkConstraintMotor* motor = scheme.m_motors[ motorIndex ];

	const hkVector4& constrainedDofW = vars.m_angularBasisAw.getColumn( axis );

	hkReal currentPosition;

	// do the angle extraction and linearization 
	{
		const hkVector4& zeroErrorAxisAinW    = vars.m_angularBasisAw.getColumn( hkGenericConstraintDataAxisOrder[axis+1] );
		const hkVector4& negZeroErrorAxisBinW = vars.m_angularBasisBw.getColumn( hkGenericConstraintDataAxisOrder[axis+2] );

		hkReal sinTheta = negZeroErrorAxisBinW.dot3( zeroErrorAxisAinW );
		const hkVector4& cosCheck = vars.m_angularBasisBw.getColumn( hkGenericConstraintDataAxisOrder[axis+1] );
		hkReal cosTheta = cosCheck.dot3( zeroErrorAxisAinW );
		
		// shift continuous domain from [ -PI, PI ] to [ 0, 2PI ] 
		// sin(x+PI) = sin(x)cos(PI) + cos(x)sin(PI) = 0-sin(x)
		// sin(x+PI) = cos(x)cos(PI) - sin(x)sin(PI) = -cos(x)-0
		// discontinuities now at x = n*2PI
		currentPosition = hkMath::atan2fApproximation( -sinTheta, -cosTheta ) + HK_REAL_PI;

		// check to see if we pass zero
		if( currentPosition - lastAngle < -HK_REAL_PI )
		{
			rotations++;
		}
		else if( currentPosition - lastAngle > HK_REAL_PI )
		{
			rotations--;
		}

		lastAngle = currentPosition;

		// extra full rotations ( may be -ve count )
		currentPosition	+= 2.0f*HK_REAL_PI*rotations;

		motorData->set( (*motorData)(0), lastAngle, rotations );

	}

	// motor control and solver setup
	if( motor )
	{
		hkConstraintMotorInput motorIn;

		hk1dAngularVelocityMotorBeginJacobian( constrainedDofW, in,out.m_jacobians, motorIn ); 

		motorIn.m_stepInfo = &in;
		motorIn.m_lastResults = solverResults[vars.m_currentResult];
//		motorIn.m_currentPosition = currentPosition;
//		motorIn.m_targetPosition = 0.0f;
// XXX this is broken
		motorIn.m_deltaTarget = 0.0f;
		motorIn.m_positionError = - currentPosition;

		
		hkConstraintMotorOutput motorOut;
		hkCalcMotorData(motor, &motorIn, &motorOut );
		
		hk1dAngularVelocityMotorCommitJacobian( motorOut, in, out );
	}

	vars.m_currentResult++;
}


inline void hkGenericConstraintData::setLinearMotorW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{

	// extract info from scheme, do some setup. 
	currentCommand++;
	int axis = *currentCommand;

	hkVector4* motorData = currentData;
	hkSolverResults* solverResults = static_cast<hkSolverResults*>( in.m_constraintRuntime.val() );
	currentData++;

	int motorIndex = static_cast< int >((*motorData)(0));

	hkConstraintMotor* motor = scheme.m_motors[ motorIndex ];


	const hkVector4 constrainedDofW = vars.m_linearBasisW.getColumn( axis );

	// do the position calculation 
	hkReal currentPosition;
	{
		hkVector4 diff; diff.setSub4( vars.m_pivotAw, vars.m_pivotBw );
		currentPosition = diff.dot3( constrainedDofW );
	}

	// motor control and solver setup
	if( motor )
	{
		hkConstraintMotorInput motorIn;

		// I'll use the pivot on body B.  So it is as if the reference body ( B ) is where the motor is anchored
		hk1dLinearVelocityMotorBeginJacobian( constrainedDofW, vars.m_pivotBw, in, out.m_jacobians, motorIn ); 

		motorIn.m_lastResults = solverResults[vars.m_currentResult];
		//motorIn.m_currentPosition = currentPosition;
		//motorIn.m_targetPosition = 0.0f;
// XXX this is broken
		motorIn.m_deltaTarget = 0.0f;
		motorIn.m_positionError = - currentPosition;

		motorIn.m_stepInfo = &in;
		
		hkConstraintMotorOutput motorOut;
		hkCalcMotorData( motor, &motorIn, &motorOut );
		
		hk1dLinearVelocityMotorCommitJacobian( motorOut, in, out );
	}

	vars.m_currentResult++;
}


inline void hkGenericConstraintData::setAngularFrictionW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{
	hkSolverResults* solverResults = static_cast<hkSolverResults*>( in.m_constraintRuntime.val() );

	currentCommand++;
	int axis = *currentCommand;
	hk1dAngularFrictionInfo afi;
	afi.m_constrainedDofW = &vars.m_angularBasisAw.getColumn( axis );
	const hkVector4& coef = *currentData;
	currentData++;
	afi.m_maxFrictionTorque = coef(0);

	afi.m_numFriction = 1;
	afi.m_lastSolverResults = &(solverResults[ vars.m_currentResult ]);

	hk1dAngularFrictionBuildJacobian( afi, in, out ); 
	vars.m_currentResult++;
}


void hkGenericConstraintData::setLinearFrictionW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const
{
	hkSolverResults* solverResults = static_cast<hkSolverResults*>( in.m_constraintRuntime.val() );

	hk1dLinearFrictionInfo bp;
	currentCommand++;
	bp.m_constrainedDofW = vars.m_linearBasisW.getColumn( *currentCommand );

	bp.m_pivot.setSub4( vars.m_pivotAw, vars.m_pivotBw ); 
	const hkVector4& coef = *currentData;
	currentData++;
	bp.m_maxFrictionForce = coef(0);
	bp.m_lastSolverResults = &(solverResults[ vars.m_currentResult ]);
	hk1dLinearFrictionBuildJacobian( bp, in, out );
	vars.m_currentResult++;
}

//
// end commands
//

void hkGenericConstraintData::hatchScheme( hkGenericConstraintDataScheme* scheme, const hkConstraintQueryIn &inOrig, hkConstraintQueryOut &out )
{
	hkConstraintQueryIn in = inOrig;

	hkGenericConstraintDataParameters vars;
	vars.m_currentResult = 0;

	vars.m_rbA = in.m_transformA;
	vars.m_rbB = in.m_transformB;

	hkArray<int>::iterator currentCommand = scheme->m_commands.begin(); 
	hkArray<hkVector4>::iterator currentData = scheme->m_data.begin();
	hkArray<hkConstraintModifier *>::iterator currentModifier = scheme->m_modifiers.begin();

	while( 1 )
	{
		switch( *currentCommand )
		{
			// linear constraints

			case hkGenericConstraintDataScheme::e_setLinearDofA :
			{
				currentCommand++;
				hkVector4& col = vars.m_linearBasisW.getColumn( *currentCommand );
				col._setRotatedDir( vars.m_rbA->getRotation(), *currentData );
				currentData++;
				break;
			}
			case hkGenericConstraintDataScheme::e_setLinearDofB :
			{
				currentCommand++;
				hkVector4& col = vars.m_linearBasisW.getColumn( *currentCommand );
				col._setRotatedDir( vars.m_rbB->getRotation(), *currentData );
				currentData++;
				break;
			}

			case hkGenericConstraintDataScheme::e_setLinearDofW :
			{
				currentCommand++;
				hkVector4& col = vars.m_linearBasisW.getColumn( *currentCommand );
				col = *currentData;
				currentData++;
				break;
			}

			case hkGenericConstraintDataScheme::e_constrainLinearW :
			{
				constrainLinearW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			case hkGenericConstraintDataScheme::e_constrainAllLinearW :
			{
				constrainAllLinearW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			// angular constraints

			case hkGenericConstraintDataScheme::e_setAngularBasisA :
			{
				hkRotation& rot = reinterpret_cast<hkRotation&>( *currentData );
				currentData += 3;
				vars.m_angularBasisAw.setMul( vars.m_rbA->getRotation(), rot );  
				break;
			}

			case hkGenericConstraintDataScheme::e_setAngularBasisB :
			{
				hkRotation& rot = reinterpret_cast<hkRotation&>( *currentData );
				currentData += 3;
				vars.m_angularBasisBw.setMul( vars.m_rbB->getRotation(), rot );  
				break;
			}

			case hkGenericConstraintDataScheme::e_setAngularBasisAidentity :
			{
				vars.m_angularBasisAw = vars.m_rbA->getRotation();
				break;
			}

			case hkGenericConstraintDataScheme::e_setAngularBasisBidentity :
			{
				vars.m_angularBasisBw = vars.m_rbB->getRotation();
				break;
			}

			case hkGenericConstraintDataScheme::e_constrainToAngularW :
			{
				constrainToAngularW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			case hkGenericConstraintDataScheme::e_constrainAllAngularW :
			{
				constrainAllAngularW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			// limits, motors, friction
			case hkGenericConstraintDataScheme::e_setLinearLimit :
			{
				setLinearLimitW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			case hkGenericConstraintDataScheme::e_setAngularLimit :
			{
				setAngularLimitW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			case hkGenericConstraintDataScheme::e_setConeLimit :
			{
				setConeLimitW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			case hkGenericConstraintDataScheme::e_setTwistLimit :
			{
				setTwistLimitW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			case hkGenericConstraintDataScheme::e_setAngularMotor :
			{
				setAngularMotorW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}
			
			case hkGenericConstraintDataScheme::e_setLinearMotor :
			{
				setLinearMotorW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			case hkGenericConstraintDataScheme::e_setAngularFriction :
			{
				setAngularFrictionW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			case hkGenericConstraintDataScheme::e_setLinearFriction :
			{
				setLinearFrictionW( currentCommand, currentData, *scheme, vars, in, out );
				break;
			}

			// pivot point

			case hkGenericConstraintDataScheme::e_setPivotA :
			{
				vars.m_pivotAw.setTransformedPos( *vars.m_rbA, *currentData );
				currentData++;
				break;
			}

			case hkGenericConstraintDataScheme::e_setPivotB :
			{
				vars.m_pivotBw.setTransformedPos( *vars.m_rbB, *currentData );
				currentData++;
				break;
			}

			case hkGenericConstraintDataScheme::e_setStrength :
			{
				hkReal strength = (*currentData)(0);
				//hkSetTauAndDamping( tau, damping, out );
				currentData++;
				in.m_virtMassFactor = in.m_virtMassFactor * strength;
				break;
			}

			case hkGenericConstraintDataScheme::e_restoreStrengh:
			{
				//hkRestoreTauAndDamping( out );			
				in.m_virtMassFactor		= inOrig.m_virtMassFactor;
				break;
			}

			case hkGenericConstraintDataScheme::e_doConstraintModifier :
			{
				currentCommand++;
				(*currentModifier)->modify( vars, *currentCommand );
				currentModifier++;
				break;
			}

			case hkGenericConstraintDataScheme::e_doRhsModifier :
			{
				currentCommand++;
				hkReal newRhs = (*currentModifier)->modifyRhs( vars, *currentCommand );
				hkReplacePreviousRhs( newRhs, in, out );
				currentModifier++;
				break;
			}

			case hkGenericConstraintDataScheme::e_endScheme :
			{
				return;
				break;
			}

			default: 
			{
				HK_ASSERT2(0x1b6cd4a1,  0, "generic constraint: unknown opcode" );
				break;
			}

		}

		// next command
		currentCommand++;

	}

}

hkGenericConstraintDataScheme* hkGenericConstraintData::getScheme()
{
	return &m_scheme;
}

hkBool hkGenericConstraintData::isValid() const
{
	// Not implemented
	return true;
}

int hkGenericConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_GENERIC;
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
