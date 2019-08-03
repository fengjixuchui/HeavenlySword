/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
 
#ifndef HK_CONSTRAINTSOLVER2_POWERED_CHAIN_BUILD_AND_UPDATE_JACOBIAN_H
#define HK_CONSTRAINTSOLVER2_POWERED_CHAIN_BUILD_AND_UPDATE_JACOBIAN_H

#include <hkconstraintsolver/solve/hkSolverInfo.h>
#include <hkconstraintsolver/constraint/chain/hkChainConstraintInfo.h>
#include <hkmath/linear/hkMatrix6.h>

class hk1Lin2AngJacobian;
class hk2AngJacobian;
class hkVelocityAccumulator;


struct hkConstraintChainMatrix6Triple
{
	hkMatrix6 m_lower;
	hkMatrix6 m_diagonalInv;
	hkMatrix6 m_upper;

	// Diagonal block of the original constraint matrix.
	hkMatrix6 m_mtxDiag;
	// Upper diagonal block of the original constraint matrix. (This block is located to the right of m_mtxDiag, and it is zero for the last row of the matrix.)
	hkMatrix6 m_mtxNextOffdiag;
};


struct hkChainSolverInfo
{
	hkChainSolverInfo( const hkSolverInfo& solverInfo ) : m_solverInfo(solverInfo) {}

	const hkSolverInfo& m_solverInfo;

	hkPadSpu<int> m_numConstraints;
	hkPadSpuf<hkReal> m_schemaTau;
	hkPadSpuf<hkReal> m_schemaDamping;

	hkPadSpu<hk1Lin2AngJacobian*> m_j; 
	hkPadSpu<hk2AngJacobian*> m_jAng;
	hkPadSpu<hkVelocityAccumulatorOffset*> m_va;
	hkPadSpu<hkVelocityAccumulator*> m_accumsBase;
	hkPadSpu<hkConstraintChainMatrix6Triple*> m_triples;
	hkPadSpu<hk3dAngularMotorSolverInfo*> m_motorsState;


	hkPadSpu<hkVector8*> m_gTempBuffer;
	hkPadSpu<hkVector8*> m_velocitiesBuffer;

};


	/// This structure holds a list of cfm parameters for a hkPoweredChain link
struct hkCfmParam
{
		// Additive cfm parameter for the linear velocity equations
	hkReal m_linAdd;

		// Multiplicative cfm parameter for the linear velocity equations
	hkReal m_linMul;
	
		// Additive cfm parameter for the angular velocity equations
	hkReal m_angAdd;

		// Multiplicative cfm parameter for the angular velocity equations
	hkReal m_angMul;
};

	// This structure holds a list of parameters required to build chain's jacobians.
struct hkPoweredChainBulidJacobianParams
{
	int m_numConstraints;
	hkReal m_chainTau;
	hkReal m_chainDamping;
	hkCfmParam m_cfm;
	hkVelocityAccumulatorOffset* m_accumulators;
	hkVelocityAccumulator* m_accumsBase;
	hk3dAngularMotorSolverInfo* m_motorsState;
	hkReal m_maxTorqueHysterisys;
	hk3dAngularMotorSolverInfo::Status* m_childConstraintStatusWriteBackBuffer;
	hkJacobianElement* m_jacobiansStart;
};


extern "C"
{
	void HK_CALL hkPoweredChainBuildJacobian( const hkPoweredChainBulidJacobianParams& params, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	//
	//  Building constraint matrix and computing LU decomposition.
	//
	//  also updating LU decomposition from pre-calculated constraint matrix.
	//
	void hkPoweredChain_BuildConstraintMatrixAndLuDecomposition(int numConstraints, const hkCfmParam& cfm, hk3dAngularMotorSolverInfo* motorsState, hk1Lin2AngJacobian* j, hk2AngJacobian* jAng, hkVelocityAccumulatorOffset* va, hkVelocityAccumulator* accumsBase, hkConstraintChainMatrix6Triple* triples, int bufferSize);

	void hkPoweredChain_UpdateLuDecomposition(int firstConsraintToProcess, int numConstraints, hk3dAngularMotorSolverInfo* motorsState, hkConstraintChainMatrix6Triple* triple);

	/*HK_FORCE_INLINE*/ 
	void hkPoweredChain_ComputeConstraintMatrixLuDecomposition_ForOneRow( hkBool isNotLastRow, hkConstraintChainMatrix6Triple& triple, hk3dAngularMotorSolverInfo* motorsState, hkMatrix6& mtxPrevOffdiag, hkMatrix6*& triplePrevUpper );

	//
	//  Computing constraint matrix blocks
	//
	HK_FORCE_INLINE void hkPoweredChain_ComputeConstraintMatrix_DiagonalAtRow(int row, const hkCfmParam& cfm, hk1Lin2AngJacobian* j, hk2AngJacobian* jAng, hkVelocityAccumulatorOffset* va, hkVelocityAccumulator* accumsBase, hkMatrix6& mtxDiag );

	HK_FORCE_INLINE void hkPoweredChain_ComputeConstraintMatrix_NextOffDiagonalAtRow(int row, hk1Lin2AngJacobian* j, hk2AngJacobian* jAng, hkVelocityAccumulatorOffset* va, hkVelocityAccumulator* accumsBase, hkMatrix6& mtxNextOffdiag );
	
	//
	//  Disabling of angular parts of constraints in the constraint matrix blocks
	//
	void HK_CALL hkPoweredChain_DisableMotorInMatrixRow_ThisConstraint(const hk3dAngularMotorSolverInfo& motorsState, hkBool isNotLastRow, hkMatrix6& mtxDiag, hkMatrix6& mtxNextOffdiag);

	void HK_CALL hkPoweredChain_DisableMotorInMatrixRow_NextConstraint(const hk3dAngularMotorSolverInfo& motorsState, hkMatrix6& mtxNextOffdiag);

	//
	// Constraint solving
	//
	void HK_CALL hkPoweredChain_CalculateVelocities(const hkChainSolverInfo& info, hkVector8* velocities);

	void HK_CALL hkPoweredChain_OverwriteVelocityWithExplicitImpulse(int modifiedConstraintIndex, int modifiedCoordinateIndex, const hk3dAngularMotorSolverInfo* motorsState, hkVector8* velocities);

	void HK_CALL hkPoweredChain_RestoreVelocityValue(int modifiedConstraintIndex, int modifiedCoordinateIndex, const hkChainSolverInfo& info, hkVector8* velocities);

	void HK_CALL hkPoweredChain_SolveConstraintMatrix_ComputeVectorG(const hkChainSolverInfo& info, hkVector8* tempBufferG );

	void HK_CALL hkPoweredChain_ScanAndDisableMotors(const hkChainSolverInfo& info, int& modifiedChildConstraintIndex, int& modifiedCoordianteIndex, hkReal& impulse, hk3dAngularMotorSolverInfo* /*output*/ motorsState );

	void HK_CALL hkPoweredChain_ScanAndEnableMotors(const hkChainSolverInfo& info, int& modifiedChildConstraintIndex, int& modifiedCoordianteIndex, hkReal& impulse, hk3dAngularMotorSolverInfo* /*output*/ motorsState );

	HK_ON_DEBUG(void HK_CALL hkPoweredChain_VerifyVelocities(const hkChainSolverInfo& info, hkVector8* velocities2 ));


}


#endif // HK_CONSTRAINTSOLVER2_POWERED_CHAIN_BUILD_AND_UPDATE_JACOBIAN_H

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
