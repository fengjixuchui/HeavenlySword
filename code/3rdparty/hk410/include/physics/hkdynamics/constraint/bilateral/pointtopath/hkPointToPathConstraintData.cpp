/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkdynamics/constraint/bilateral/pointtopath/hkPointToPathConstraintData.h>
#include <hkdynamics/constraint/bilateral/pointtopath/hkParametricCurve.h>

#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkPointToPathConstraintData);


hkPointToPathConstraintData::hkPointToPathConstraintData() 
: m_path(HK_NULL),
  m_maxFrictionForce(0),
  m_angularConstrainedDOF( OrientationConstraintType(CONSTRAIN_ORIENTATION_NONE) )				 
{
	m_transform_OS_KS[0].setIdentity();
	m_transform_OS_KS[1].setIdentity();

	m_atoms.m_bridgeAtom.init( this );
}

hkPointToPathConstraintData::~hkPointToPathConstraintData()					 
{
	if (m_path)
	{
		m_path->removeReference();
	}
}

void hkPointToPathConstraintData::setPath(hkParametricCurve* path)
{
	if (path)
	{
		path->addReference();
	}
	if (m_path)
	{
		m_path->removeReference();
	}
	m_path = path;
}

void hkPointToPathConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& info ) const
{
	info.m_atoms = const_cast<hkConstraintAtom*>(m_atoms.getAtoms());
	info.m_sizeOfAllAtoms = m_atoms.getSizeOfAllAtoms();
	info.clear();
	info.addHeader();

	// 2 linear
	info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
	info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );

	// 3 angular
	info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );
	info.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_AA );

	// 1 for limit. 1 for friction
	info.add( HK_SIZE_OF_JACOBIAN_1D_LINEAR_LIMIT_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
	info.add( HK_SIZE_OF_JACOBIAN_1D_FRICTION_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA );
}


void hkPointToPathConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	// we need runtime data
	infoOut.m_numSolverResults = SOLVER_RESULT_MAX;
	infoOut.m_sizeOfExternalRuntime = sizeof( Runtime);
}


void hkPointToPathConstraintData::calcPivot( const hkTransform& transformBodyA, hkVector4& pivotOut ) const
{
	pivotOut.setTransformedPos( transformBodyA, m_transform_OS_KS[0].getTranslation() );
}

void hkPointToPathConstraintData::buildJacobian( const hkConstraintQueryIn &in, hkConstraintQueryOut &out )
{
	hkSolverResults* solverResults = &getRuntime( in.m_constraintRuntime )->m_solverResults[0];
	hkBeginConstraints( in, out, solverResults, sizeof(hkSolverResults) );

	// transform from object space to world space
	// Abrivations used:
	// 	A attached object
	// 	R reference object 
	// 	ws world space
	// 	os object space
	// 	ks constraint space
	// For Transforms:
	// ws_T_Rks means
	//   transforms from reference constraint space to world space



	hkTransform ws_T_Rks;	ws_T_Rks.setMul( *in.m_transformB, m_transform_OS_KS[1]);
	hkTransform ws_T_Aks;	ws_T_Aks.setMul( *in.m_transformA, m_transform_OS_KS[0]);

	const hkVector4& headingAws  = ws_T_Aks.getColumn(0);
	const hkVector4& rightAws	 = ws_T_Aks.getColumn(1);
	const hkVector4& upAws		 = ws_T_Aks.getColumn(2);
	const hkVector4& pivotAws    = ws_T_Aks.getTranslation();	

	Runtime* runtime = getRuntime( in.m_constraintRuntime );

	hkVector4 pathPointWs;
	{
		hkVector4 pathPoint; pathPoint.setTransformedInversePos( ws_T_Rks, pivotAws );
		runtime->m_parametricPosition = m_path->getNearestPoint( runtime->m_parametricPosition, pathPoint, pathPoint ); 
		pathPointWs.setTransformedPos( ws_T_Rks, pathPoint );
	}

	hkVector4 tangentRws;
	hkVector4 perpTangentRws;
	hkVector4 perpTangent2Rws;
	{
		hkVector4 tangent;
		m_path->getTangent( runtime->m_parametricPosition, tangent );

		tangentRws.setRotatedDir( ws_T_Rks.getRotation(), tangent );
		hkVector4Util::calculatePerpendicularVector( tangentRws, perpTangentRws );
		perpTangentRws.normalize3();
		perpTangent2Rws.setCross( tangentRws, perpTangentRws );
	}

	hkReal actualDist = m_path->getLengthFromStart( runtime->m_parametricPosition );

	// this is fairly hacky.  This needs to be set up so that tangent.dot( localizedParametricWS )
	// is equal to the distance along the path
	hkVector4 localizedParametricWS;	localizedParametricWS.setAddMul4( pivotAws, tangentRws, -actualDist );

	// handle friction (Note: must be the first constraint)
	if( m_maxFrictionForce > 0 )
	{
		hk1dLinearFrictionInfo lfi;

		// set up the direction the friction works in
		lfi.m_constrainedDofW = tangentRws;
		
		// set distance from the "zero" reference point for the positional friction 
		lfi.m_pivot = localizedParametricWS;
		lfi.m_maxFrictionForce = m_maxFrictionForce;
		lfi.m_lastSolverResults = &runtime->m_solverResults[SOLVER_RESULT_FRICTION];
	    hk1dLinearFrictionBuildJacobian( lfi, in, out ); 
	}

    // constraint off the linear bits
	hk1dLinearBilateralConstraintInfo bp;
    bp.m_pivotA = pivotAws;
    bp.m_pivotB = pathPointWs;
	bp.m_constrainedDofW = perpTangentRws; 
	hk1dLinearBilateralConstraintBuildJacobian( bp, in, out );
	bp.m_constrainedDofW = perpTangent2Rws;
	hk1dLinearBilateralConstraintBuildJacobian( bp, in, out );

	// constraint off angular DOFs

	// at least one constrained dof
	if( m_angularConstrainedDOF > hkPointToPathConstraintData::CONSTRAIN_ORIENTATION_NONE )
	{ 
		hk1dAngularBilateralConstraintInfo bp2;
		bp2.m_zeroErrorAxisAinW = tangentRws;
		bp2.m_perpZeroErrorAxisBinW = upAws;
		bp2.m_constrainedDofW = rightAws;
		hk1dAngularBilateralConstraintBuildJacobian( bp2, in, out );
	
		bp2.m_perpZeroErrorAxisBinW = rightAws;
		bp2.m_constrainedDofW.setNeg4( upAws );
		hk1dAngularBilateralConstraintBuildJacobian( bp2, in, out );

		if( m_angularConstrainedDOF == hkPointToPathConstraintData::CONSTRAIN_ORIENTATION_TO_PATH)
		{
			hkVector4 rightR;	m_path->getBinormal( runtime->m_parametricPosition, rightR );
			hkVector4 rightRws;	rightRws.setRotatedDir( ws_T_Rks.getRotation(), rightR );

			bp2.m_zeroErrorAxisAinW = upAws;
			bp2.m_perpZeroErrorAxisBinW.setNeg4( rightRws );
			bp2.m_constrainedDofW = headingAws;

			hk1dAngularBilateralConstraintBuildJacobian( bp2, in, out );
		}
	}

	// handle limits
	if( !m_path->isClosedLoop() )
	{ 
		hk1dLinearLimitInfo bpLim;
		// get our curve-space position in real length
		bpLim.m_pivotA = pivotAws;
		bpLim.m_pivotB = localizedParametricWS;
		bpLim.m_min = m_path->getLengthFromStart( m_path->getStart() );
		bpLim.m_max = m_path->getLengthFromStart( m_path->getEnd() );
		bpLim.m_constrainedDofW = tangentRws;
		hk1dLinearLimitBuildJacobian( bpLim, in, out );
	}

	hkEndConstraints();
	
}


hkBool hkPointToPathConstraintData::isValid() const
{
	// needs more checks.
	return m_path && (m_angularConstrainedDOF != CONSTRAIN_ORIENTATION_INVALID);
}


int hkPointToPathConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_POINTTOPATH;
}


void hkPointToPathConstraintData::setInWorldSpace(const hkTransform& ws_T_Aos, const hkTransform& ws_T_Ros, const hkVector4& pivotWs, hkParametricCurve *path, const hkTransform& os_T_Rks )
{
	if (path)
	{
		path->addReference();
	}

	if (m_path)
	{
		m_path->removeReference();
	}
	m_path = path;

	m_transform_OS_KS[1] = os_T_Rks;

	// Get the matrix from the path
	hkTransform os_T_RAks;
	{
		hkTransform ws_T_Rks; ws_T_Rks.setMul( ws_T_Ros, os_T_Rks );
		hkReal parametricPosition = 0.0f;
		hkVector4 pathPoint; pathPoint.setTransformedInversePos( ws_T_Rks, pivotWs );
		parametricPosition = m_path->getNearestPoint( parametricPosition, pathPoint, pathPoint ); 

		hkVector4 tangent;		m_path->getTangent( parametricPosition, tangent );
		hkVector4 rightR;		m_path->getBinormal( parametricPosition, rightR );
		hkVector4 upR; upR.setCross( tangent, rightR );

		hkTransform ks_T_Rks;
		ks_T_Rks.getRotation().setCols( tangent, rightR, upR );
		ks_T_Rks.setTranslation( pathPoint );
		HK_ASSERT( 0xf0ef5421, os_T_Rks.getRotation().isOrthonormal() );

		os_T_RAks.setMul( os_T_Rks, ks_T_Rks );
	}

	// transform our matrix into attached space, as we cannot change the m_transform_OS_KS[0]
	hkTransform os_T_Aks;
	{
		hkTransform ws_T_Rks; ws_T_Rks.setMul( ws_T_Ros, os_T_RAks);
		os_T_Aks.setMulInverseMul( ws_T_Aos, ws_T_Rks );
	}
	m_transform_OS_KS[0] = os_T_Aks;
}


void hkPointToPathConstraintData::setInBodySpace(const hkVector4& pivotA, const hkVector4& pivotB, hkParametricCurve *path)
{
	HK_ASSERT2( 0xf03421de, 0, "Warning: Contact Havok support if you want to use this function and if you are not a constraint expert" );
	if (path)
	{
		path->addReference();
	}
	if (m_path)
	{
		m_path->removeReference();
	}
	m_path = path;

	m_transform_OS_KS[0].setIdentity();
	m_transform_OS_KS[1].setIdentity();

	m_transform_OS_KS[0].setTranslation(pivotA);
	m_transform_OS_KS[1].setTranslation(pivotB);
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
