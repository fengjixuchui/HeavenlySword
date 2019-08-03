/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONSTRAINTSOLVER2_CHAIN_CONSTRAINT_INFO_H
#define HK_CONSTRAINTSOLVER2_CHAIN_CONSTRAINT_INFO_H


/// Stores values of matrices of LU decomposition of a tri-diagonal constraint matrix.
struct hkConstraintChainTriple
{
	hkReal m_lower;
	hkReal m_diagonal; // XXX todo store inverse inv.
	hkReal m_upper;

	//          [ 0.diagonal                          ]   [ 1       1.upper           ]
	// A = LU = [ 1.lower     1.diagonal              ] * [         1         1.upper ]
	//          [             2.lower      2.diagonal ]   [                   1       ]
	//
	// number in 0.diagonal is the triple index.
	// blanks are zero
};

/// Stores values of matrices of LU decomposition of a block-tri-diagonal constraint matrix.
struct hkConstraintChainMatrixTriple
{
	hkMatrix3 m_lower;
	hkMatrix3 m_diagonalInv;
	hkMatrix3 m_upper;

	//          [ 0.diagonalInv^-1                              ]   [ 1       1.upper           ]
	// A = LU = [ 1.lower          1.diagonal^-1                ] * [         1         1.upper ]
	//          [                  2.lower        2.diagonal^-1 ]   [                   1       ]
	//
	// number in 0.diagonal is the triple index.
	// blanks are zero
};


class hkVelocityAccumulator;

class hkVelocityAccumulatorOffset
{
public:
	hkVelocityAccumulatorOffset() {}

	inline hkVelocityAccumulatorOffset(hkVelocityAccumulator* accumulatorBase, hkVelocityAccumulator* theOneAccumulator)  
	{
		m_offset = static_cast<hkUint32>( hkGetByteOffset(accumulatorBase, theOneAccumulator) );
	}

	inline hkVelocityAccumulator& getAccumulator(hkVelocityAccumulator* accumulatorsBase) { return *hkAddByteOffset(accumulatorsBase, m_offset); }

	hkUint32 m_offset;
};




class hkConstraintQueryIn;
class hkConstraintQueryOut;

extern "C"
{

	// Constraint chain
	void HK_CALL hkBallSocketChainBuildJacobian( int numConstraints, hkReal tau, hkReal damping, hkReal cfm, hkVelocityAccumulatorOffset* accumulators, hkVelocityAccumulator* accumBase, hkJacobianElement* jacobiansStart, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	// Constraint chain
	void HK_CALL hkStiffSpringChainBuildJacobian( int numConstraints, hkReal tau, hkReal damping, hkReal cfm, hkVelocityAccumulatorOffset* accumulators, hkVelocityAccumulator* accumBase, hkJacobianElement* jacobiansStart, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

}

#endif // HK_CONSTRAINTSOLVER2_CHAIN_CONSTRAINT_INFO_H

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
