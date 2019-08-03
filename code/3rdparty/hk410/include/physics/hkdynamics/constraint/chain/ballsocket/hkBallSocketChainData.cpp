/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/chain/ballsocket/hkBallSocketChainData.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstance.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstanceAction.h>

#include <hkdynamics/constraint/bilateral/ballandsocket/hkBallAndSocketConstraintData.h>

#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/constraint/chain/hkChainConstraintInfo.h>
#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkbase/class/hkTypeInfo.h>

#include <hkdynamics/world/hkSimulationIsland.h>

#include <hkdynamics/motion/hkMotion.h>
#include <hkdynamics/motion/hkMotion.h>

// solver includes:
#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>

#include <hkbase/htl/hkAlgorithm.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkBallSocketChainData);

hkBallSocketChainData::hkBallSocketChainData() : m_tau(0.6f), m_damping(1.0f), m_cfm(0.1f * HK_REAL_EPSILON) 
{
	m_maxErrorDistance = 0.1f;
	m_atoms.m_bridgeAtom.init( this );
}

hkBallSocketChainData::hkBallSocketChainData(hkFinishLoadedObjectFlag f) : hkConstraintChainData(f), m_atoms(f), m_infos(f)
{
	m_atoms.m_bridgeAtom.init( this );
}


hkBallSocketChainData::~hkBallSocketChainData()
{
}


int hkBallSocketChainData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_BALL_SOCKET_CHAIN;
}

void hkBallSocketChainData::addConstraintInfoInBodySpace(const hkVector4& pivotInA, const hkVector4& pivotInB)
{
	ConstraintInfo& info = m_infos.expandOne();
	info.m_pivotInA = pivotInA;
	info.m_pivotInB = pivotInB;
}





void hkBallSocketChainData::getConstraintInfo( hkConstraintData::ConstraintInfo& info ) const
{
	info.m_atoms = const_cast<hkConstraintAtom*>(m_atoms.getAtoms());
	info.m_sizeOfAllAtoms = m_atoms.getSizeOfAllAtoms();
	info.clear();
	info.addHeader();

	const int numConstraints = this->m_infos.getSize();
		// The schema stores: accumulator pointers,
	const int schemaAccumulatorsSize = (1 + numConstraints) * sizeof(hkVelocityAccumulatorOffset);
		// The jacobians are appended with the constraint matrix
	const int jacobianMatrix3dSize = numConstraints * sizeof(hkConstraintChainMatrixTriple);
		//                                 and a vector of temporary solver values
	const int jacobianTempElements3dSize = (1 + numConstraints ) * sizeof(hkVector4);

	int schemaSize = HK_SIZE_OF_JACOBIAN_BALL_SOCKET_CHAIN_SCHEMA + schemaAccumulatorsSize;
	int jacobianSize = 3 * numConstraints * HK_SIZE_OF_JACOBIAN_LAA
				    + jacobianMatrix3dSize
					+ jacobianTempElements3dSize;
	int resultsSize = 3 * numConstraints;

	info.add(schemaSize, jacobianSize);
	info.m_numSolverResults += resultsSize - 1; // one added in the info.add(a,b) call
}


void hkBallSocketChainData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	if ( wantRuntime )
	{
		int childSolverResultMax = hkBallAndSocketConstraintData::SOLVER_RESULT_MAX;
		infoOut.m_numSolverResults = m_infos.getSize() * childSolverResultMax;
		infoOut.m_sizeOfExternalRuntime = infoOut.m_numSolverResults * sizeof(hkSolverResults);
	}
	else
	{
		infoOut.m_numSolverResults = 0;
		infoOut.m_sizeOfExternalRuntime = 0;
	}
}





void hkBallSocketChainData::buildJacobian( const hkConstraintQueryIn &inChain, hkConstraintQueryOut &out )
{
	hkSolverResults* solverResults = static_cast<hkSolverResults*>( inChain.m_constraintRuntime.val() ) ;

	hkBeginConstraints( inChain, out, solverResults, sizeof(hkSolverResults) );

	hkJacobianSchema*  initialSchemaPointer = out.m_jacobianSchemas;

	hkConstraintQueryIn newIn = inChain;
	{
		newIn.m_constraintInstance = HK_NULL;
		newIn.m_constraintRuntime  = HK_NULL;
	}

	hkInplaceArray<hkVelocityAccumulatorOffset,32> accumulators;
	hkJacobianElement* initialJacobianPointer = out.m_jacobians;

	{
		hkVelocityAccumulator * baseAccum;
		hkConstraintChainInstance* chainInstance;
		{
			// Calculate base address of accumulators
			HK_ASSERT2(0xad677d6d, inChain.m_constraintInstance, "internal error");
			chainInstance = reinterpret_cast<hkConstraintChainInstance*>(inChain.m_constraintInstance.val());
			const hkEntity* chainCA = chainInstance->getEntityA();
			baseAccum = hkAddByteOffset(inChain.m_bodyA.val(), -int(chainCA->m_solverData) );
		}

		{
			// Check whether the accompanying action is properly added to the world
			HK_ASSERT2(0xad5677dd, chainInstance->m_action->getWorld() == static_cast<hkSimulationIsland*>(reinterpret_cast<hkConstraintChainInstance*>(inChain.m_constraintInstance.val())->getOwner())->getWorld(), "The action and the chain instance must be both added to the world before running simulation.");
		}

		const hkArray<hkEntity*>& entities = chainInstance->m_chainedEntities;
		int numConstraints = entities.getSize() - 1;
		HK_ASSERT2(0xad567755, numConstraints <= m_infos.getSize(), "Not enough pivot sets are specified in the hkChainConstraintData to handle all entities referenced by the hkConstraintChainInstance.");


		// Initialize first body info
		hkEntity* rB = entities[0]; // yes, body A
		hkMotion* cB = rB->getMotion();
		newIn.m_bodyB = hkAddByteOffset(baseAccum, rB->m_solverData );
		accumulators.pushBackUnchecked( hkVelocityAccumulatorOffset(baseAccum, newIn.m_bodyB) );

		newIn.m_transformB = &(static_cast<const hkMotion*>(cB)->getTransform());
		HK_ASSERT( 0xf0140201, &rB->getCollidable()->getTransform() == newIn.m_transformB );

			// set the tau to use the user tau functions
		newIn.m_rhsFactor = newIn.m_substepInvDeltaTime;
		newIn.m_virtMassFactor = 1.0f;


		for (int i = 0; i < numConstraints; i++)
		{
			newIn.m_bodyA = newIn.m_bodyB;
			newIn.m_transformA = newIn.m_transformB;

			rB = entities[i+1];// i.e. m_constraintInstances[i]->getEntityB();
			cB = rB->getMotion();
			newIn.m_bodyB = hkAddByteOffset(baseAccum, rB->m_solverData );

			accumulators.pushBack( hkVelocityAccumulatorOffset(baseAccum, newIn.m_bodyB) );


			newIn.m_transformB = &(static_cast<const hkMotion*>(cB)->getTransform());
			HK_ASSERT( 0xf0140201, &rB->getCollidable()->getTransform() == newIn.m_transformB );

			hkVector4 posA; posA._setTransformedPos( *newIn.m_transformA, m_infos[i].m_pivotInA );
			hkVector4 posB; posB._setTransformedPos( *newIn.m_transformB, m_infos[i].m_pivotInB );

			// we're ignoring the shema generated by the ball-and-socket.
			// xxx use lower level function and remove newIn
			hkLoadVelocityAccumulators( newIn );
			hkStabilizedBallSocketConstraintBuildJacobian( posA, posB, m_maxErrorDistance, newIn, out );
			out.m_jacobianSchemas = initialSchemaPointer;
		}

		//
		// Initialize the schema and build the constraint matrix
		//
		HK_ASSERT2(0xad674d4d, numConstraints == accumulators.getSize() - 1, "Number of chained constraints and number of velocity accumulators don't match.");
		hkBallSocketChainBuildJacobian( numConstraints, m_tau, m_damping, m_cfm, accumulators.begin(), baseAccum, initialJacobianPointer, inChain, out );
	}


	hkEndConstraints();

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
