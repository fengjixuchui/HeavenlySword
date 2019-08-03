/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/chain/powered/hkPoweredChainData.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstance.h>
#include <hkdynamics/constraint/chain/hkConstraintChainInstanceAction.h>

#include <hkdynamics/constraint/bilateral/ballandsocket/hkBallAndSocketConstraintData.h>

#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/constraint/motor/hkMotorConstraintInfo.h>
#include <hkconstraintsolver/constraint/chain/hkPoweredChainSolverUtil.h>
#include <hkconstraintsolver/jacobian/hkJacobianElement.h>

#include <hkconstraintsolver/solve/hkSolve.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkbase/class/hkTypeInfo.h>

#include <hkdynamics/world/hkSimulationIsland.h>

#include <hkdynamics/motion/hkMotion.h>

// solver includes:
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>

#include <hkdynamics/constraint/motor/hkConstraintMotor.h>
#include <hkdynamics/constraint/motor/position/hkPositionConstraintMotor.h>

#include <hkbase/htl/hkAlgorithm.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkPoweredChainData);


hkPoweredChainData::hkPoweredChainData() : m_tau(0.6f), m_damping(1.0f)
{
	m_cfmLinAdd = 0.1f * HK_REAL_EPSILON;
	m_cfmLinMul = 1.0f;
	m_cfmAngAdd = 0.1f * HK_REAL_EPSILON;
	m_cfmAngMul = 1.0f;

	m_maxErrorDistance = 0.1f;

	m_atoms.m_bridgeAtom.init( this );
}

hkPoweredChainData::hkPoweredChainData(hkFinishLoadedObjectFlag f) : hkConstraintChainData(f), m_atoms(f), m_infos(f)
{
	m_atoms.m_bridgeAtom.init( this );
}


hkPoweredChainData::~hkPoweredChainData()
{
	for (int i = 0; i < m_infos.getSize(); i++)
	{
		for (int m = 0; m < 3; m++)
		{
			if (m_infos[i].m_motors[m])
			{
				m_infos[i].m_motors[m]->removeReference();
			}
		}
	}
}


int hkPoweredChainData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_POWERED_CHAIN;
}

void hkPoweredChainData::addConstraintInfoInBodySpace(const hkVector4& pivotInA, const hkVector4& pivotInB, const hkQuaternion& aTc, 
														hkConstraintMotor* xMotor, hkConstraintMotor* yMotor, hkConstraintMotor* zMotor)
{
	ConstraintInfo& info = m_infos.expandOne();
	info.m_pivotInA = pivotInA;
	info.m_pivotInB = pivotInB;
	info.m_aTc = aTc;

	info.m_motors[0] = xMotor; xMotor->addReference();
	info.m_motors[1] = yMotor; yMotor->addReference();
	info.m_motors[2] = zMotor; zMotor->addReference();

	info.m_switchBodies = false;
	info.m_bTc = hkQuaternion::getIdentity();

	//info.m_cfmLinear = m_cfm;
	//info.m_cfmAngular = m_cfm;
}





void hkPoweredChainData::getConstraintInfo( hkConstraintData::ConstraintInfo& info ) const
{
	info.m_atoms = const_cast<hkConstraintAtom*>(m_atoms.getAtoms());
	info.m_sizeOfAllAtoms = m_atoms.getSizeOfAllAtoms();
	info.clear();
	info.addHeader();

	const int numConstraints = this->m_infos.getSize();

	const int schemaAccumulatorsSize = (1 + numConstraints) * sizeof(hkVelocityAccumulator*);
	const int schemaConstraintStatusSize = numConstraints * sizeof(hk3dAngularMotorSolverInfo);

	const int jacobianMatrix6dSize = numConstraints * sizeof(hkConstraintChainMatrix6Triple);
	const int jacobianTempElements6dSize = (1 + numConstraints ) * sizeof(hkVector8);
	const int jacobianVelocity6dSize = numConstraints * sizeof(hkVector8);

	int schemaSize = HK_SIZE_OF_JACOBIAN_POWERED_CHAIN_SCHEMA + schemaAccumulatorsSize + schemaConstraintStatusSize;
	int jacobianSize = 3 * numConstraints * HK_SIZE_OF_JACOBIAN_LAA
		             + 3 * numConstraints * HK_SIZE_OF_JACOBIAN_AA
				     + jacobianMatrix6dSize
					 + jacobianTempElements6dSize
	                 + jacobianVelocity6dSize;
//	int jacobianSize = reinterpret_cast<int>( dummy->getEnd(HK_NULL) );
	int resultsSize = 6 * numConstraints;

	info.add(schemaSize, jacobianSize);
	info.m_numSolverResults += resultsSize - 1; // one added in the info.add(a,b) call
}


void hkPoweredChainData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	const int numConstraints = m_infos.getSize();
	const int childSolverResultMax = 6;

	{
		infoOut.m_numSolverResults = numConstraints * childSolverResultMax;
		infoOut.m_sizeOfExternalRuntime = infoOut.m_numSolverResults * sizeof(hkSolverResults)
										+ HK_NEXT_MULTIPLE_OF(4, numConstraints * sizeof(hk3dAngularMotorSolverInfo::Status)) 
										+ numConstraints * sizeof(hkQuaternion); 
	}
}

void HK_CALL hkPoweredChainData::enableMotor(hkConstraintChainInstance* instance, int constraintIndex, int motorIndex)
{
	HK_ASSERT2(0xad7899dd, motorIndex >=0 && motorIndex < 3, "motorIndex must be in [1,3] range");
	// xxx remove the data dependency
	hkPoweredChainData* data = static_cast<hkPoweredChainData*>(instance->getData());
	hk3dAngularMotorSolverInfo::Status* statuses = data->getConstraintFlags( instance->getRuntime() );
	HK_ASSERT2(0xad8bdd9d, instance->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_POWERED_CHAIN && statuses && constraintIndex < instance->getNumConstraints(), "Wrong constraint type or no runtime or constraintIndex out of range.");

	hk3dAngularMotorSolverInfo::Status zeroMask = static_cast<hk3dAngularMotorSolverInfo::Status>(~(hk3dAngularMotorSolverInfo::ANGULAR_1ST << (2 * motorIndex)));
	hk3dAngularMotorSolverInfo::Status onStatus = static_cast<hk3dAngularMotorSolverInfo::Status>(hk3dAngularMotorSolverInfo::MOTOR_NOT_BROKEN << (2 * motorIndex));

	statuses[constraintIndex] = statuses[constraintIndex] & zeroMask;
	statuses[constraintIndex] = statuses[constraintIndex] | onStatus;
}

void HK_CALL hkPoweredChainData::disableMotor(hkConstraintChainInstance* instance, int constraintIndex, int motorIndex)
{
	HK_ASSERT2(0xad7899dd, motorIndex >=0 && motorIndex < 3, "motorIndex must be in [1,3] range");

	// xxx remove the data dependency
	hkPoweredChainData* data = static_cast<hkPoweredChainData*>(instance->getData());
	hk3dAngularMotorSolverInfo::Status* statuses = data->getConstraintFlags( instance->getRuntime() );
	HK_ASSERT2(0xad8bdd9d, instance->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_POWERED_CHAIN && statuses && constraintIndex < instance->getNumConstraints(), "Wrong constraint type or no runtime or constraintIndex out of range.");

	hk3dAngularMotorSolverInfo::Status zeroMask = static_cast<hk3dAngularMotorSolverInfo::Status>(~(hk3dAngularMotorSolverInfo::ANGULAR_1ST << (2 * motorIndex)));
	hk3dAngularMotorSolverInfo::Status offStatus = static_cast<hk3dAngularMotorSolverInfo::Status>(hk3dAngularMotorSolverInfo::MOTOR_DISABLED << (2 * motorIndex));

	statuses[constraintIndex] = statuses[constraintIndex] & zeroMask;
	statuses[constraintIndex] = statuses[constraintIndex] | offStatus;
}


	// returns ( from-1 * to ) * 2.0f
static inline void HK_CALL estimateAngleToLs(const hkQuaternion& from, const hkQuaternion& to, hkVector4& angleOut)
{
	angleOut.setCross(from.getImag(),         to.getImag());
	angleOut.addMul4(to.m_vec.getSimdAt(3),   from.getImag());
	angleOut.subMul4(from.m_vec.getSimdAt(3), to.getImag());
	angleOut.add4(angleOut);
	if ( hkMath::isNegative(  to.getImag().dot4( from.getImag() ) ) )
	{
		angleOut.setNeg4( angleOut );
	}
}

static HK_FORCE_INLINE void HK_CALL hk1dAngularVelocityMotorCommitJacobianInMotorInfo( hk1dConstraintMotorInfo& info, const hkConstraintQueryIn &in, hkConstraintQueryOut &out, hk1dAngularMotorSolverInfo* motorInfoOut )
{
	hk2AngJacobian* jac = reinterpret_cast<hk2AngJacobian*>(out.m_jacobians.val());

	hk1dAngularMotorSolverInfo* si = motorInfoOut;

	si->m_maxImpulsePerSubstep = info.m_maxForce * in.m_microStepDeltaTime;
	si->m_minImpulsePerSubstep = info.m_minForce * in.m_microStepDeltaTime;
	si->m_velocity = info.m_targetVelocity;
	si->m_tau = info.m_tau;
	si->m_damping = info.m_damping;
	si->m_usedImpulseFactor = 1.0f;

	const hkReal rhs = info.m_targetPosition * in.m_substepInvDeltaTime;
	jac->setAngularRhs( rhs );

	out.m_jacobians = jac->next();
	//out.m_jacobianSchemas = sc+1;
}


void hkPoweredChainData::buildJacobian( const hkConstraintQueryIn &inNotValid, hkConstraintQueryOut &out )
{
	{
		hkSolverResults* solverResults = static_cast<hkSolverResults*>( inNotValid.m_constraintRuntime.val() ) ;
		hkBeginConstraints( inNotValid, out, solverResults, sizeof(hkSolverResults) );
	}

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

		hkInplaceArray<hk3dAngularMotorSolverInfo, 32> motorsState; motorsState.setSize(numConstraints);


		// Initialize first body info
		hkEntity* rB = entities[0]; // yes, body A
		hkMotion* cB = rB->getMotion();
		newIn.m_bodyB = hkAddByteOffset(baseAccum, rB->m_solverData );
		accumulators.pushBackUnchecked( hkVelocityAccumulatorOffset(baseAccum, newIn.m_bodyB) );

		newIn.m_transformB = &(static_cast<const hkMotion*>(cB)->getTransform());
		HK_ASSERT( 0xf0140201, &rB->getCollidable()->getTransform() == newIn.m_transformB );

		hkConstraintQueryOut outAngular = out;
		outAngular.m_jacobians = hkAddByteOffset( out.m_jacobians.val(), numConstraints * 3 * HK_SIZE_OF_JACOBIAN_LAA );
		HK_ON_DEBUG( hkJacobianElement* originalAngularJacobianPointer = outAngular.m_jacobians);

			// set the tau to use the user tau functions
		newIn.m_rhsFactor = newIn.m_substepInvDeltaTime;
		newIn.m_virtMassFactor = 1.0f;


		for (int i = 0; i < numConstraints; i++)
		{
			newIn.m_bodyA = newIn.m_bodyB;
			newIn.m_transformA = newIn.m_transformB;

			rB = entities[i+1];//  m_constraintInstances[i]->getEntityB();
			cB = rB->getMotion();
			newIn.m_bodyB = hkAddByteOffset(baseAccum, rB->m_solverData );

			accumulators.pushBack( hkVelocityAccumulatorOffset(baseAccum, newIn.m_bodyB) );


			newIn.m_transformB = &(static_cast<const hkMotion*>(cB)->getTransform());
			HK_ASSERT( 0xf0140201, &rB->getCollidable()->getTransform() == newIn.m_transformB );

			hkVector4 posA; posA._setTransformedPos( *newIn.m_transformA, m_infos[i].m_pivotInA );
			hkVector4 posB; posB._setTransformedPos( *newIn.m_transformB, m_infos[i].m_pivotInB );

			// we're ignoring the shema generated by the pulley.
			hkLoadVelocityAccumulators( newIn );
			hkStabilizedBallSocketConstraintBuildJacobian( posA, posB, m_maxErrorDistance, newIn, out );
			out.m_jacobianSchemas = initialSchemaPointer;

			//
			// And here angular parts
			//
			{
				hkQuaternion rotA; rotA.set( newIn.m_transformA->getRotation() );
				hkQuaternion rotB; rotB.set( newIn.m_transformB->getRotation() );

				if (m_infos[i].m_switchBodies)
				{
					hkAlgorithm::swap(rotA, rotB);
				}

				// Apply constraint space offset here
				{
					hkQuaternion wTc; wTc.setMul(rotB, m_infos[i].m_bTc);
					rotB = wTc;
				}

				const hkQuaternion& aTc = m_infos[i].m_aTc;

				// copy values, due to SIMD alignment
				hkReal* oldTargetFrameF = &getMotorRuntimeQuaternions(inNotValid.m_constraintRuntime)[i*4];
				hkQuaternion oldTargetFrame; oldTargetFrame.m_vec.set( oldTargetFrameF[0], oldTargetFrameF[1], oldTargetFrameF[2], oldTargetFrameF[3] );

				// <todo: have a boolean flag in runtime marking the runtime as [un]initialized
				if (oldTargetFrame.m_vec.lengthSquared4() == 0.0f)
				{
					oldTargetFrame = aTc;
				}

				// Convention:
				//    a a space
				//    ob current b-space
				//	  nb new/target b-space

				hkQuaternion target_wTnb; target_wTnb.setMul(rotA, aTc);
				hkQuaternion target_wTob; target_wTob.setMul(rotA, oldTargetFrame);
				oldTargetFrameF[0] = aTc.m_vec(0);
				oldTargetFrameF[1] = aTc.m_vec(1);
				oldTargetFrameF[2] = aTc.m_vec(2);
				oldTargetFrameF[3] = aTc.m_vec(3);


				hkVector4 deltaTarget;    estimateAngleToLs( rotB, target_wTnb, deltaTarget); // == target_wTob^1 * target_wTnb 
				hkVector4 positionError;  estimateAngleToLs( rotB, target_wTob, positionError);
				deltaTarget.sub4( positionError );

				if (m_infos[i].m_switchBodies)
				{
					deltaTarget.setNeg3(deltaTarget);
					positionError.setNeg3(positionError);
				}

				hkRotation constraintSpace; constraintSpace.set( rotB );

				for (int j = 0; j < 3; j++)
				{
					const hkVector4 constrainedDofW = constraintSpace.getColumn(j);

					//////////////////////////////////////////////////////////////////////////
					hkSolverResults* solverResults = getSolverResults( chainInstance->getRuntime() );
					
					{
						HK_ASSERT2( 0xf032ef45, m_infos[i].m_motors[j] != HK_NULL, "You must supply motors for this constraint to work" );

						hkConstraintMotorInput motorIn;

						hk1dAngularVelocityMotorBeginJacobian( constrainedDofW, newIn, outAngular.m_jacobians, motorIn ); 

						motorIn.m_stepInfo    = &newIn;
						motorIn.m_lastResults = solverResults[i*6/*6solverresults per constraint*/ + 3 + j];

						motorIn.m_deltaTarget   = deltaTarget(j);
						motorIn.m_positionError = positionError(j);

						hkConstraintMotorOutput motorOut;
						hkCalcMotorData(m_infos[i].m_motors[j], &motorIn, &motorOut );

						hk1dAngularVelocityMotorCommitJacobianInMotorInfo( motorOut, newIn, outAngular, &motorsState[i].m_motorInfos[j] );

						outAngular.m_jacobianSchemas = initialSchemaPointer;
					}
				}
			}
		}

		HK_ASSERT2(0xad6777dd, originalAngularJacobianPointer == out.m_jacobians, "Internal error: angular vs linear jacobians not properly placed.");
		out.m_jacobians = outAngular.m_jacobians;

		//
		// Initialize the schema and build the constraint matrix
		//
		{
			HK_ASSERT2(0xad674d4d, numConstraints == accumulators.getSize() - 1, "Number of chained constraints and number of velocity accumulators don't match.");
			hk3dAngularMotorSolverInfo::Status *const  childConstraintStatusFlags = getConstraintFlags( chainInstance->getRuntime() );

			for (int c = 0; c < numConstraints; c++)
			{
				motorsState[c].m_broken = childConstraintStatusFlags[c];
			}

			hkPoweredChainBulidJacobianParams params;
			params.m_numConstraints = numConstraints;
			params.m_chainTau = m_tau;
			params.m_chainDamping = m_damping;
			params.m_cfm.m_linAdd = m_cfmLinAdd;
			params.m_cfm.m_linMul = m_cfmLinMul;
			params.m_cfm.m_angAdd = m_cfmAngAdd;
			params.m_cfm.m_angMul = m_cfmAngMul;
			params.m_accumulators = accumulators.begin();
			params.m_accumsBase = baseAccum;
			params.m_motorsState = motorsState.begin();
			params.m_maxTorqueHysterisys = 0.0f;
			params.m_childConstraintStatusWriteBackBuffer = childConstraintStatusFlags;
			params.m_jacobiansStart = initialJacobianPointer;

			hkPoweredChainBuildJacobian( params, inNotValid, out );
		}
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
