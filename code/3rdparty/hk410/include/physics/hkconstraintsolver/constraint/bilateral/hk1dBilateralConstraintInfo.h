/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
 
#ifndef HK_CONSTRAINTSOLVER2_1D_BILATERAL_CONSTRAINT_INFO_H
#define HK_CONSTRAINTSOLVER2_1D_BILATERAL_CONSTRAINT_INFO_H


class hkJacobianElement;
class hkSolverResults;



///
class hk1dLinearBilateralConstraintInfo
{ 
	public:

		HK_FORCE_INLINE hk1dLinearBilateralConstraintInfo() {}

		/// the pivot point A in world space
		hkVector4 m_pivotA;

		/// the pivot point B in world space
		hkVector4 m_pivotB;

		/// defines the normal of a plane that movement is restricted to lay on, in world space
		hkVector4 m_constrainedDofW;
};

class hk1dLinearBilateralUserTauConstraintInfo: public hk1dLinearBilateralConstraintInfo
{
	public:
			/// The stiffness of the constraint
		hkReal m_tau;

			/// The damping of the constraint
		hkReal m_damping;
};


///
class hk1dAngularBilateralConstraintInfo
{ 
	public:

		/// m_zeroErrorAxisAinW X m_zeroErrorAxisBinW = m_constrainedDofW
		HK_FORCE_INLINE hk1dAngularBilateralConstraintInfo() {}

			/// an axis perpendicular to the constraint axis, in world space
		hkVector4 m_zeroErrorAxisAinW;

			/// defines the axis that rotational movement is not allowed along, in world space
		hkVector4 m_constrainedDofW;

			/// perpendicular to m_zeroErrorAxisAinW axis and the constraint axis, transformed from B's local space into world space
		hkVector4 m_perpZeroErrorAxisBinW;
};


/// 
class hk1dLinearLimitInfo : public hk1dLinearBilateralConstraintInfo
{
	public:

		hkReal m_min;
		hkReal m_max;
		 
};


class hk1dAngularFrictionInfo
{
	public:
		const hkVector4 *m_constrainedDofW;		// points to an array of axis
		hkSolverResults* m_lastSolverResults;
		hkReal m_maxFrictionTorque;
		int		m_numFriction;				// number of frictions added
};


class hk1dLinearFrictionInfo
{
	public:
		
		/// the pivot point in world space
		hkVector4 m_pivot;

		/// defines the normal of a plane that movement is restricted to lay on, in world space
		hkVector4 m_constrainedDofW;

		hkReal m_maxFrictionForce;
		
		hkSolverResults *m_lastSolverResults;
};

	/// 
class hk1dAngularLimitInfo 
{
	public:
			/// defines the axis that rotational movement is not allowed along, in world space
		hkVector4 m_constrainedDofW;

			/// the lower limit of angular freedom
		hkReal m_min;

			/// the upper limit of angular freedom
		hkReal m_max;

			/// The current angle
		hkReal m_computedAngle;	

			/// The tau used by the solver
		hkReal m_tau;
};


	/// Holds parameters needed to build jacobians for a pulley constraint.
struct hkPulleyConstraintInfo
{
		/// Rope attachment point (pivot point) of body A in the global space.
	hkVector4 m_positionA;

		/// Rope attachment point (pivot point) of body B in the global space.
	hkVector4 m_positionB;

		/// Pulley pivot point on the bodyA's side; in the global space.
	hkVector4 m_pulleyPivotA;

		/// Pulley pivot point on the bodyB's side; in the global space
	hkVector4 m_pulleyPivotB;

		/// Combined length of rope used ( equal to (rope on bodyA's side + rope on bodyB's side * leverageOnBodyB) )
	hkReal m_ropeLength;
		
		/// Leverage on body B.
	hkReal m_leverageOnBodyB;
};



class hkJacobianElement;
class hkConstraintQueryIn;
class hkConstraintQueryOut;
class hkSolverResults;

extern "C"
{
		// _Overrides_ right hand side (positional/angular error/drift of the relative tranform of two bodies involved)
		// Called by e.g. hkStiffSpringConstraintData
	void HK_CALL hkReplacePreviousRhs( hkReal customRhs, const hkConstraintQueryIn &in, hkConstraintQueryOut &out, int numBack = 1 );

	void HK_CALL hkBeginConstraints( const hkConstraintQueryIn &in, hkConstraintQueryOut &out, HK_CPU_PTR(hkSolverResults*) sr, int solverResultStriding );


	void HK_CALL hkLoadVelocityAccumulators( const hkConstraintQueryIn &in );

#ifdef HK_CHECK_REG_FOR_PS2
	void HK_CALL hkEndConstraints( );
#endif

//	void HK_CALL hkGotoSchemaBuildJacobian( class hkJacobianSchema* destination, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hk1dLinearBilateralConstraintBuildJacobian( const hk1dLinearBilateralConstraintInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hk1dLinearBilateralConstraintUserTauBuildJacobian( const hk1dLinearBilateralUserTauConstraintInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hk1dAngularBilateralConstraintBuildJacobian( const hk1dAngularBilateralConstraintInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hk1dLinearLimitBuildJacobian( const hk1dLinearLimitInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hk1dAngularLimitBuildJacobian( const hk1dAngularLimitInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hk1dAngularFrictionBuildJacobian( const hk1dAngularFrictionInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hk1dLinearFrictionBuildJacobian( const hk1dLinearFrictionInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hkBallSocketConstraintBuildJacobian( hkVector4Parameter pivotAWs, hkVector4Parameter pivotBWs, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hkStabilizedBallSocketConstraintBuildJacobian( hkVector4Parameter posA, hkVector4Parameter posB, hkReal maxAllowedErrorDistance, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );
		
	void HK_CALL hkSetInvMassBuildJacobian( hkVector4Parameter invMassA, hkVector4Parameter invMassB,  const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	void HK_CALL hkAddVelocityBuildJacobian( hkVector4Parameter deltaVel, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

} 

#ifndef HK_CHECK_REG_FOR_PS2
	HK_FORCE_INLINE void HK_CALL hkEndConstraints( ){}
#endif


#endif // HK_CONSTRAINTSOLVER2_1D_BILATERAL_CONSTRAINT_INFO_H

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
