/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/chain/stiffspring/hkStiffSpringChainData.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstance.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstanceAction.h>

#include <hkdynamics/constraint/bilateral/stiffspring/hkStiffSpringConstraintData.h>
#include <hkdynamics/world/hkSimulationIsland.h>

#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/constraint/chain/hkChainConstraintInfo.h>
#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkbase/class/hkTypeInfo.h>

#include <hkdynamics/motion/hkMotion.h>

// solver includes:
#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>

#include <hkbase/htl/hkAlgorithm.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkStiffSpringChainData);

hkStiffSpringChainData::hkStiffSpringChainData() : m_tau(0.6f), m_damping(1.0f), m_cfm(0.1f * HK_REAL_EPSILON) 
{
	m_atoms.m_bridgeAtom.init( this );
}

hkStiffSpringChainData::hkStiffSpringChainData(hkFinishLoadedObjectFlag f) : hkConstraintChainData(f), m_atoms(f) 
{
	m_atoms.m_bridgeAtom.init( this );
}


hkStiffSpringChainData::~hkStiffSpringChainData()
{
}

int hkStiffSpringChainData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_STIFF_SPRING_CHAIN;
}


void hkStiffSpringChainData::addConstraintInfoInBodySpace(const hkVector4& pivotInA, const hkVector4& pivotInB, hkReal springLength)
{
	ConstraintInfo& info = m_infos.expandOne();
	info.m_pivotInA = pivotInA;
	info.m_pivotInB = pivotInB;
	info.m_springLength = springLength;
}


void hkStiffSpringChainData::getConstraintInfo( hkConstraintData::ConstraintInfo& info ) const
{
	info.m_atoms = const_cast<hkConstraintAtom*>(m_atoms.getAtoms());
	info.m_sizeOfAllAtoms = m_atoms.getSizeOfAllAtoms();
	info.clear();
	info.addHeader();

	const int numConstraints = this->m_infos.getSize();

	const int schemaAccumulatorsSize = (1 + numConstraints) * sizeof(hkVelocityAccumulator*);
	const int jacobianMatrix1dSize = numConstraints * 3 * sizeof(hkReal);
	const int jacobianTempElements1dSize = (1 + numConstraints ) * sizeof(hkReal);

	int schemaSize = HK_SIZE_OF_JACOBIAN_STIFF_SPRING_CHAIN_SCHEMA + schemaAccumulatorsSize;
	int jacobianSize = numConstraints * HK_SIZE_OF_JACOBIAN_LAA
				    + jacobianMatrix1dSize
			        + jacobianTempElements1dSize;
	jacobianSize = HK_NEXT_MULTIPLE_OF(16, jacobianSize);
	int resultsSize = numConstraints;

	info.add(schemaSize, jacobianSize);
	info.m_numSolverResults += resultsSize - 1; // one added in the info.add(a,b) call
}


void hkStiffSpringChainData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	if ( wantRuntime )
	{
		int childSolverResultMax = hkStiffSpringConstraintData::SOLVER_RESULT_MAX;
		infoOut.m_numSolverResults = m_infos.getSize() * childSolverResultMax;
		infoOut.m_sizeOfExternalRuntime = infoOut.m_numSolverResults * sizeof(hkSolverResults); //* sizeof( Runtime );
	}
	else
	{
		infoOut.m_numSolverResults = 0;
		infoOut.m_sizeOfExternalRuntime = 0;
	}
}




void hkStiffSpringChainData::buildJacobian( const hkConstraintQueryIn &inNotValid, hkConstraintQueryOut &out )
{
	hkSolverResults* solverResults = static_cast<hkSolverResults*>( inNotValid.m_constraintRuntime.val() ) ;
//	solverResults = HK_NULL;

	hkBeginConstraints( inNotValid, out, solverResults, sizeof(hkSolverResults) );

	hkJacobianSchema*  initialSchemaPointer = out.m_jacobianSchemas;

	hkConstraintQueryIn newIn = inNotValid;
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
			HK_ASSERT2(0xad677d6d, inNotValid.m_constraintInstance, "internal error");
			chainInstance = reinterpret_cast<hkConstraintChainInstance*>(inNotValid.m_constraintInstance.val());
			const hkEntity* chainCA = chainInstance->getEntityA();
			baseAccum = hkAddByteOffset(inNotValid.m_bodyA.val(), - int(chainCA->m_solverData) );
		}

		{
			// Check whether the accompanying action is properly added to the world
			HK_ASSERT2(0xad5677dd, chainInstance->m_action->getWorld() == static_cast<hkSimulationIsland*>(reinterpret_cast<hkConstraintChainInstance*>(inNotValid.m_constraintInstance.val())->getOwner())->getWorld(), "The action and the chain instance must be both added to the world before running simulation.");
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

		newIn.m_rhsFactor = newIn.m_substepInvDeltaTime;
		newIn.m_virtMassFactor = 1.0f;

		for (int i = 0; i < numConstraints; i++)
		{
			newIn.m_bodyA      = newIn.m_bodyB;
			newIn.m_transformA = newIn.m_transformB;

			rB = entities[i+1];//  m_constraintInstances[i]->getEntityB();
			cB = rB->getMotion();
			newIn.m_bodyB = hkAddByteOffset(baseAccum, rB->m_solverData );

			accumulators.pushBack( hkVelocityAccumulatorOffset(baseAccum, newIn.m_bodyB) );


			newIn.m_transformB = &(static_cast<const hkMotion*>(cB)->getTransform());
			HK_ASSERT( 0xf0140201, &rB->getCollidable()->getTransform() == newIn.m_transformB );

			hkVector4 posA; posA._setTransformedPos( *newIn.m_transformA, m_infos[i].m_pivotInA );
			hkVector4 posB; posB._setTransformedPos( *newIn.m_transformB, m_infos[i].m_pivotInB );

			////// CONSTRATIN BUILDING CODE HERE

			//
			// Code copied from stiff-spring constraint .cpp
			//

			hk1dLinearBilateralConstraintInfo bp;
			hkReal springLength;
			{
				bp.m_pivotA.setTransformedPos( *newIn.m_transformA, m_infos[i].m_pivotInA );
				bp.m_pivotB.setTransformedPos( *newIn.m_transformB, m_infos[i].m_pivotInB );

				hkVector4 sepDist;	sepDist.setSub4( bp.m_pivotA, bp.m_pivotB );
				springLength = sepDist.normalizeWithLength3();

				if ( springLength > 0.0f )
				{
					bp.m_constrainedDofW = sepDist;
				}
				else
				{
					bp.m_constrainedDofW = hkTransform::getIdentity().getColumn(0);
					springLength = 0.0f;
				}
			}
			{
				// we're ignoring the shema generated by the pulley.
				hkLoadVelocityAccumulators( newIn );
				hk1dLinearBilateralConstraintBuildJacobian( bp, newIn, out); 
				// HOW HACKY IS THAT -- do we always have only one shema buffer ?? check !!
				out.m_jacobianSchemas = initialSchemaPointer;

				hkReplacePreviousRhs( m_infos[i].m_springLength - springLength, newIn, out );

			}			
		}

		HK_ASSERT2(0xad674d4d, numConstraints == accumulators.getSize() - 1, "Number of chained constraints and number of velocity accumulators don't match.");
		hkStiffSpringChainBuildJacobian( numConstraints, m_tau, m_damping, m_cfm, accumulators.begin(), baseAccum, initialJacobianPointer, inNotValid, out );
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
