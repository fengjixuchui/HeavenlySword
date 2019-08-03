/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



namespace {

	// this is nearly a duplication of hkBuildJacobianTask
	struct hkBuildJacobiansFromTaskInput
	{
		hkBuildJacobianTask::AtomInfo* m_atomInfos;
		int m_numAtomInfos;

		//output:
		hkVelocityAccumulator* m_accumulators;
		hkJacobianElement*	   m_jacobians;
		hkJacobianSchema*	   m_schemas;

		hkJacobianSchema*	   m_schemasOfNextTask;
		void*                  m_nextTask;

		hkJacobianElement*	   m_jacobiansBase;
	};

}



static HK_FORCE_INLINE hkJobQueue::JobStatus hkCpuIntegrateMotionJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& jobInOut, hkJobQueue::WaitStatus waitStatus )
{
	HK_TIMER_BEGIN_LIST("Integrate", "IntMotion");

	hkIntegrateMotionJob& job = reinterpret_cast<hkIntegrateMotionJob&>(jobInOut);

	int firstEntityIdx = job.m_firstEntityIdx;
	int numEntities    = job.m_numEntities;

	hkVelocityAccumulator* accumulatorBatch;
	{
		hkVelocityAccumulator* accumulatorsBase = job.m_taskHeader->m_accumulatorsBase;
		accumulatorBatch = &accumulatorsBase[1+firstEntityIdx];
	}
	hkEntity*const* entityBatch = &job.m_taskHeader->m_allEntities[firstEntityIdx];

	HK_ON_DEBUG_MULTI_THREADING( {	 for ( int i=0; i < numEntities; i++ ){ entityBatch[i]->markForWrite(); }	} );

	int numInactiveFrames = hkRigidMotionUtilApplyAccumulators( tl.m_world->m_dynamicsStepInfo.m_solverInfo,  tl.m_world->m_dynamicsStepInfo.m_stepInfo, accumulatorBatch, (hkMotion*const*)entityBatch, numEntities, HK_OFFSET_OF(hkEntity, m_motion));
	job.m_numInactiveFrames = numInactiveFrames;

	HK_ON_DEBUG_MULTI_THREADING( {	 for ( int i=0; i < numEntities; i++ ){ entityBatch[i]->unmarkForWrite(); }	} );

	HK_TIMER_END_LIST();

	return jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_CPU, &jobInOut, jobInOut, 0, waitStatus );
}



static HK_FORCE_INLINE hkJobQueue::JobStatus hkCpuSolveConstraintsJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& jobInOut, hkJobQueue::WaitStatus waitStatus )
{
	HK_TIMER_BEGIN_LIST("Integrate", "Solve");

	const hkSolveConstraintsJob& job = reinterpret_cast<hkSolveConstraintsJob&>(jobInOut);

	// save some of the data in the job (as job is modified below when transforming to broadphase job)
	hkBuildJacobianTaskHeader* taskHeader = job.m_taskHeader;

	hkVelocityAccumulator* accumulators = reinterpret_cast<hkVelocityAccumulator*>( hkAddByteOffset(job.m_buffer, job.m_accumulatorsOffset) );
	hkJacobianElement*	   jacobians    = reinterpret_cast<hkJacobianElement*>    ( hkAddByteOffset(job.m_buffer, job.m_jacobiansOffset)    );
	hkSolverElemTemp*	   solverTemp   = reinterpret_cast<hkSolverElemTemp*>     ( hkAddByteOffset(job.m_buffer, job.m_solverTempOffset)   );
	hkJacobianSchema*	   schemas      = reinterpret_cast<hkJacobianSchema*>     ( hkAddByteOffset(job.m_buffer, job.m_schemasOffset)      );

	//
	//	zero solver results
	//
	{
	
		for (int i = 0; i < job.m_numSolverResults; i++)		
		{			
			solverTemp[i].m_impulseApplied = 0.0f;		
		}
	}
	{
		const hkWorldDynamicsStepInfo& stepInfo = tl.m_world->m_dynamicsStepInfo;	

		hkSolveConstraints( stepInfo.m_solverInfo, schemas, accumulators, jacobians, solverTemp );

		//HK_MONITOR_ADD_VALUE( "NumJacobians", float(island->m_numSolverResults), HK_MONITOR_TYPE_INT );
		//HK_MONITOR_ADD_VALUE( "NumEntities",  float(island->getEntities().getSize()), HK_MONITOR_TYPE_INT );
	}

	int numEntities = taskHeader->m_numAllEntities;

	{
		hkJobQueue::JobQueueEntry jobBuffer;
		new (&jobBuffer) hkIntegrateMotionJob(job, taskHeader, 0, numEntities, taskHeader->m_buffer);
		jobQueue.addJob( jobBuffer, hkJobQueue::JOB_HIGH_PRIORITY, hkJobQueue::JOB_TYPE_HINT_SPU );
	}

	HK_TIMER_SPLIT_LIST("Export");

	// export solver data
	hkExportImpulsesAndRhs(tl.m_world->m_dynamicsStepInfo.m_solverInfo, taskHeader->m_solverTempBase, taskHeader->m_schemasBase, taskHeader->m_accumulatorsBase, taskHeader->m_jacobiansBase );

	HK_TIMER_END_LIST();

	return jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_CPU, &jobInOut, jobInOut, 0, waitStatus );
}



static HK_FORCE_INLINE void HK_CALL hkCpuBuildJacobiansFromTask(const hkBuildJacobiansFromTaskInput& input, hkConstraintQueryIn &queryIn)
{
	hkConstraintQueryOut queryOut;
	{
		queryOut.m_jacobians            = input.m_jacobians;
		queryOut.m_jacobiansVirtualBase = input.m_jacobiansBase;
		queryOut.m_jacobianSchemas      = input.m_schemas;
	}


	const hkBuildJacobianTask::AtomInfo* atomInfos = input.m_atomInfos;
	int numAtomInfos = input.m_numAtomInfos;


	for (int a = 0; a < numAtomInfos; a++)
	{
		const hkBuildJacobianTask::AtomInfo& atomInfo = atomInfos[a];

		// prepare queryIn
		{
			queryIn.m_transformA                    = atomInfo.m_transformA;
			queryIn.m_transformB                    = atomInfo.m_transformB;

			queryIn.m_bodyA                         = hkAddByteOffset(input.m_accumulators, atomInfo.m_accumulatorOffsetA);
			queryIn.m_bodyB                         = hkAddByteOffset(input.m_accumulators, atomInfo.m_accumulatorOffsetB);
			queryIn.m_bodyAOffset                   = atomInfo.m_accumulatorOffsetA;
			queryIn.m_bodyBOffset                   = atomInfo.m_accumulatorOffsetB;

			queryIn.m_constraintInstance            = atomInfo.m_instance;
			queryIn.m_constraintRuntime             = atomInfo.m_runtime;

 			queryIn.m_constraintRuntimeInMainMemory = atomInfo.m_runtime;

#if defined (HK_PLATFORM_HAS_SPU)
			queryIn.m_atomInMainMemory              = atomInfo.m_atoms;
#endif
		}

		{
			hkSolverBuildJacobianFromAtoms( atomInfo.m_atoms, atomInfo.m_atomsSize, queryIn, queryOut );
		}
	}

	//
	// Add "End" or "Goto" schema
	//
	{
		if ( !input.m_nextTask )
		{
			// no 'next task' -> write end schema
			hkJacobianSchema* endSchema = static_cast<hkJacobianSchema*>(queryOut.m_jacobianSchemas);
			*(reinterpret_cast<hkInt32*>(endSchema)) = 0;
			queryOut.m_jacobianSchemas = endSchema+1;
		}
		else
		{
			int branchOffset = hkGetByteOffsetInt( queryOut.m_jacobianSchemas, input.m_schemasOfNextTask);
			if ( branchOffset >= 256 )
			{
				// add Goto schema
				hkJacobianGotoSchema* gotoSchema = static_cast<hkJacobianGotoSchema*>(queryOut.m_jacobianSchemas.val() );
				gotoSchema->initOffset(branchOffset);
				queryOut.m_jacobianSchemas = gotoSchema+1;
			}
			else if ( branchOffset > 0 )
			{
				// add 8bit Goto schema
				hkJacobianGoto8Schema* goto8Schema = static_cast<hkJacobianGoto8Schema*>(queryOut.m_jacobianSchemas.val() );
				goto8Schema->initOffset(branchOffset);
				queryOut.m_jacobianSchemas = goto8Schema+1;
			}
		}
#ifdef HK_DEBUG
		HK_ASSERT(0xaf6451ed, queryOut.m_jacobianSchemas.val() <= input.m_schemasOfNextTask);
#endif
	}
}



static HK_FORCE_INLINE hkJobQueue::JobStatus HK_CALL hkCpuBuildJacobiansJob(	hkMtThreadStructure& tl, hkJobQueue& jobQueue, 
																				hkJobQueue::JobQueueEntry& jobInOut,
																				hkJobQueue::WaitStatus waitStatus)
{
	HK_TIMER_BEGIN_LIST("Integrate", "BuildJac" );

	const hkBuildJacobiansJob& job = reinterpret_cast<hkBuildJacobiansJob&>(jobInOut);

 	HK_ALIGN16( hkConstraintQueryIn queryIn );
	queryIn = *job.m_constraintQueryIn;

	hkBuildJacobiansFromTaskInput input;
	{
		hkBuildJacobianTask* task = job.m_buildJacobianTask;

		input.m_atomInfos			= task->m_atomInfos;
		input.m_numAtomInfos		= task->m_numAtomInfos;
		input.m_accumulators		= task->m_accumulators;
		input.m_jacobians			= task->m_jacobians;
		input.m_schemas				= task->m_schemas;
		input.m_jacobiansBase		= task->m_jacobiansBase;
		input.m_schemasOfNextTask	= task->m_schemasOfNextTask;
		input.m_nextTask            = task->m_next;
	}

	hkSimulationIsland* island = job.m_island;
	island->markAllEntitiesReadOnly();
	hkCpuBuildJacobiansFromTask(input, queryIn);
	island->unmarkAllEntitiesReadOnly();

	HK_TIMER_END_LIST();

	return jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_CPU, &jobInOut, jobInOut, 0, waitStatus );
}



static HK_FORCE_INLINE hkJobQueue::JobStatus HK_CALL hkCpuBuildAccumulatorsJob( hkMtThreadStructure& tl, hkJobQueue& jobQueue, hkJobQueue::JobQueueEntry& nextJobOut )
{
	const hkBuildAccumulatorsJob& job = reinterpret_cast<hkBuildAccumulatorsJob&>(nextJobOut);

	HK_TIMER_BEGIN_LIST("Integrate", "BuildAccum" );

	hkSimulationIsland* island = job.m_island;
	island->markAllEntitiesReadOnly();

	{
		hkMotion*const* motions				= (hkMotion*const*)( job.m_islandEntitiesArray + job.m_firstEntityIdx );
		int numMotions						= job.m_numEntities;
		int motionsOffset					= HK_OFFSET_OF(hkEntity, m_motion);
		hkVelocityAccumulator* accumulators	= job.m_taskHeader->m_accumulatorsBase + 1 + job.m_firstEntityIdx;

		hkRigidMotionUtilApplyForcesAndBuildAccumulators( tl.m_collisionInput.m_stepInfo, motions, numMotions, motionsOffset, accumulators );
	}

	island->unmarkAllEntitiesReadOnly();

	HK_TIMER_END_LIST();

	return jobQueue.finishJobAndGetNextJob( tl.m_threadType, (const hkJobQueue::JobQueueEntry*)&job, nextJobOut, &tl );
}



HK_FORCE_INLINE void hkSingleThreadedJobsOnIsland::setSchemasAndJacPtrsInTasks( hkBuildJacobianTaskHeader* taskHeader, hkJacobianSchema* schemas, hkJacobianElement* jacobians )
{
	hkUlong schemasShift   = hkGetByteOffsetInt(taskHeader->m_schemasBase,   schemas  );
	hkUlong jacobiansShift = hkGetByteOffsetInt(taskHeader->m_jacobiansBase, jacobians);

	hkBuildJacobianTask* task = taskHeader->m_tasks.m_buildJacobianTasks;

	HK_ASSERT(0xaf355667, taskHeader->m_schemasBase   == task->m_schemas  );
	HK_ASSERT(0xaf355667, taskHeader->m_jacobiansBase == task->m_jacobians);

	while ( task )
	{
		task->m_schemas           = hkAddByteOffset(task->m_schemas,           schemasShift  );
		task->m_schemasOfNextTask = hkAddByteOffset(task->m_schemasOfNextTask, schemasShift  );
		task->m_jacobians         = hkAddByteOffset(task->m_jacobians,         jacobiansShift);

		task = task->m_next;
	}
}



HK_FORCE_INLINE hkJobQueue::JobStatus hkSingleThreadedJobsOnIsland::cpuFireJacobianSetupCallbackAndBuildPpuOnlyJacobiansJob(	hkJobQueue& jobQueue, 
																						hkJobQueue::JobQueueEntry& jobInOut,
																						hkJobQueue::WaitStatus waitStatus)
{
	const hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob& job = reinterpret_cast<hkFireJacobianSetupCallbackAndBuildPpuJacobiansJob&>(jobInOut);

	hkBuildJacobianTaskHeader* taskHeader = job.m_taskHeader;

	if ( taskHeader->m_tasks.m_numCallbackConstraints > 0 )
	{

		HK_TIMER_BEGIN_LIST("Integrate", "FireJSCB" );

		for ( int i = 0; i < taskHeader->m_tasks.m_numCallbackConstraints; i++ )
		{
			const hkConstraintInternal* ci = taskHeader->m_tasks.m_callbackConstraints[i];

			if ( ci->m_callbackRequest & hkConstraintAtom::CALLBACK_REQUEST_CONTACT_POINT )
			{
				hkConstraintAtom* terminalAtom = hkWorldConstraintUtil::getTerminalAtom(ci);
				hkSimpleContactConstraintAtom* atom = reinterpret_cast<hkSimpleContactConstraintAtom*>( terminalAtom );

				// fire callbacks for each new point
				hkContactPointProperties* cpp = atom->getContactPointProperties();
				hkContactPoint*           cp  = atom->getContactPoints();
				int nA                        = atom->m_numContactPoints;

				for (int cindex = nA-1; cindex >=0 ; cp++, cpp++, cindex-- )
				{
					if ( cpp->m_flags & hkContactPointProperties::CONTACT_IS_NEW_AND_POTENTIAL )
					{

						hkConstraintInstance* constraint = ci->m_constraint;
						hkRigidBody* bodyA = constraint->getRigidBodyA();
						hkRigidBody* bodyB = constraint->getRigidBodyB();

						hkReal projectedVel;
						{
							hkVector4 velA;
							{
								int accuOffsetA = ci->m_entities[0]->m_solverData;
								hkVelocityAccumulator* acc = hkAddByteOffset(taskHeader->m_accumulatorsBase, accuOffsetA);
								hkVector4 relPos; relPos.setSub4( cp->getPosition(), acc->getCenterOfMassInWorld() );
								velA.setCross( bodyA->getAngularVelocity(), relPos);
								velA.add4( bodyA->getLinearVelocity() );
							}
							hkVector4 velB;
							{
								int accuOffsetB = ci->m_entities[1]->m_solverData;
								hkVelocityAccumulator* acc = hkAddByteOffset(taskHeader->m_accumulatorsBase, accuOffsetB);
								hkVector4 relPos; relPos.setSub4( cp->getPosition(), acc->getCenterOfMassInWorld() );
								velB.setCross( bodyB->getAngularVelocity(), relPos);
								velB.add4( bodyB->getLinearVelocity() );
							}

							hkVector4 deltaVel; deltaVel.setSub4( velA, velB );
							projectedVel = deltaVel.dot3( cp->getNormal() );
						}

						if ( !bodyA->isFixed()) { bodyA->markForWrite(); }
						if ( !bodyB->isFixed()) { bodyB->markForWrite(); }

						hkSimpleContactConstraintData* constraintData = reinterpret_cast<hkSimpleContactConstraintData*>( ci->m_constraint->getData() );
						hkContactPointConfirmedEvent event( hkContactPointAddedEvent::TYPE_MANIFOLD, *bodyA->getCollidable(), *bodyB->getCollidable(), constraintData, cp, cpp, 0.0f, projectedVel );


						hkWorld* world = bodyA->getWorld();
						hkWorldCallbackUtil::fireContactPointConfirmed( world, event );
						hkEntityCallbackUtil::fireContactPointConfirmed( bodyA, event );
						hkEntityCallbackUtil::fireContactPointConfirmed( bodyB, event );

						if ( !bodyB->isFixed()) { bodyB->unmarkForWrite(); }
						if ( !bodyA->isFixed()) { bodyA->unmarkForWrite(); }

						const hkWorldDynamicsStepInfo& env = *reinterpret_cast<const hkWorldDynamicsStepInfo*>(world->getCollisionInput()->m_dynamicsInfo);

						hkReal restitution = cpp->getRestitution();
						const hkReal MIN_RESTITUTION_TO_USE_CORRECT_RESPONSE = 0.3f;
						if ( event.m_projectedVelocity < -env.m_solverInfo.m_contactRestingVelocity && restitution > MIN_RESTITUTION_TO_USE_CORRECT_RESPONSE )
						{
							hkSimpleConstraintUtilCollideParams params;
							{
								params.m_direction = cp->getNormal();
								params.m_friction  = cpp->getFriction();
								params.m_restitution = restitution;
								params.m_extraSeparatingVelocity = 0.0f;
								params.m_extraSlope = 0.0f;
								params.m_externalSeperatingVelocity = event.m_projectedVelocity;
							}

							hkVelocityAccumulator* accuA;
							{
								int accuOffsetA = ci->m_entities[0]->m_solverData;
								accuA = hkAddByteOffset(taskHeader->m_accumulatorsBase, accuOffsetA);
							}

							hkVelocityAccumulator* accuB;
							{
								int accuOffsetB = ci->m_entities[1]->m_solverData;
								accuB = hkAddByteOffset(taskHeader->m_accumulatorsBase, accuOffsetB);
							}

							hkSimpleCollisionResponse::SolveSingleOutput2 out2;
							hkSimpleCollisionResponse::solveSingleContact2( constraintData, *cp, params, bodyA, bodyB, accuA, accuB, out2 );

							// make a simple guess for the friction 
							// Unfortunately we can't take the initial impulse, as this impulse is way higher than the resting impulse
							cpp->m_impulseApplied = 0.0f; // out2.m_impulse * 0.5f; 
							cpp->m_internalDataA = cp->getDistance();
						}
						else
						{
							{
								hkReal sumInvMass = bodyA->getMassInv() + bodyB->getMassInv();
								hkReal mass = 1.0f / (sumInvMass + 1e-10f );
								cpp->m_impulseApplied = -0.2f * mass * projectedVel * ( 1.0f + cpp->getRestitution() );
							}
							{
								hkReal ErrorTerm = cpp->getRestitution() * projectedVel * taskHeader->m_constraintQueryIn->m_substepDeltaTime;
								ErrorTerm *= -1.3f;
								cpp->m_internalSolverData = ErrorTerm;
								hkReal dist = cp->getDistance() + ErrorTerm;
								cpp->m_internalDataA = dist;
							}
						}

						cpp->m_flags &= ~hkContactPointProperties::CONTACT_IS_NEW_AND_POTENTIAL;

					}
				}

				// clear the callback flag once the callback has been fired
				const_cast<hkConstraintInternal*>(ci)->m_callbackRequest &= ~hkConstraintAtom::CALLBACK_REQUEST_CONTACT_POINT;
			}
		}

		taskHeader->m_tasks.m_callbackConstraints[0]->getMasterEntity()->getSimulationIsland()->getMultiThreadLock().unmarkForRead();
		hkDeallocateChunk( taskHeader->m_tasks.m_callbackConstraints, taskHeader->m_tasks.m_numCallbackConstraints, HK_MEMORY_CLASS_CONSTRAINT_SOLVER );
		taskHeader->m_tasks.m_callbackConstraints = HK_NULL;
		taskHeader->m_tasks.m_numCallbackConstraints = 0;

		HK_TIMER_END_LIST();
	}



#if defined (HK_PLATFORM_PS3) || defined (HK_SIMULATE_SPU_DMA_ON_CPU)

	//
	// build the jacobians that are only allowed to be constructed on ppu
	//
	if ( taskHeader->m_tasks.m_numPpuSetupOnlyConstraints > 0 ) {

		HK_TIMER_BEGIN_LIST("Integrate", "BuildPpuJac" );

		hkConstraintQueryOut queryOut;
		{
			queryOut.m_jacobians            = taskHeader->m_jacobiansBase;
			queryOut.m_jacobiansVirtualBase = taskHeader->m_jacobiansBase;
			queryOut.m_jacobianSchemas      = taskHeader->m_schemasBase;
		}

		for ( int i = 0; i < taskHeader->m_tasks.m_numPpuSetupOnlyConstraints; i++ )
		{
			const hkConstraintInternal* constraintInternal = taskHeader->m_tasks.m_ppuSetupOnlyConstraints[i];

			// prepare queryIn
			HK_ALIGN16( hkConstraintQueryIn queryIn );
			{
				queryIn                                 = *taskHeader->m_constraintQueryIn;

				queryIn.m_transformA                    = &constraintInternal->m_entities[0]->getMotion()->getTransform();
				queryIn.m_transformB                    = &constraintInternal->m_entities[1]->getMotion()->getTransform();

				queryIn.m_bodyA                         = hkAddByteOffset(taskHeader->m_accumulatorsBase, constraintInternal->m_entities[0]->m_solverData);
				queryIn.m_bodyB                         = hkAddByteOffset(taskHeader->m_accumulatorsBase, constraintInternal->m_entities[1]->m_solverData);
				queryIn.m_bodyAOffset                   = constraintInternal->m_entities[0]->m_solverData;
				queryIn.m_bodyBOffset                   = constraintInternal->m_entities[1]->m_solverData;

				queryIn.m_constraintInstance            = constraintInternal->m_constraint;
				queryIn.m_constraintRuntime             = constraintInternal->m_runtime;
				queryIn.m_constraintRuntimeInMainMemory = constraintInternal->m_runtime;

				queryIn.m_atomInMainMemory              = constraintInternal->getAtoms();
			}

			{
				hkSolverBuildJacobianFromAtoms( constraintInternal->getAtoms(), constraintInternal->getAtomsSize(), queryIn, queryOut );
			}
		}

		// align schemas on a 16 byte boundary
		{
			hkJacobianSchema* next = reinterpret_cast<hkJacobianSchema*>(HK_NEXT_MULTIPLE_OF( 16, hkUlong(queryOut.m_jacobianSchemas.val()) ));

			hkJacobianGoto8Schema* schema =  static_cast<hkJacobianGoto8Schema*>(queryOut.m_jacobianSchemas.val());
			schema->initGoto( next );
			queryOut.m_jacobianSchemas = next;

			// end schema
			*(reinterpret_cast<hkInt32*>(next)) = 0;
		}

		hkDeallocateChunk( taskHeader->m_tasks.m_ppuSetupOnlyConstraints, taskHeader->m_tasks.m_numPpuSetupOnlyConstraints, HK_MEMORY_CLASS_CONSTRAINT_SOLVER );
		taskHeader->m_tasks.m_ppuSetupOnlyConstraints    = HK_NULL;
		taskHeader->m_tasks.m_numPpuSetupOnlyConstraints = 0;

		hkSingleThreadedJobsOnIsland::setSchemasAndJacPtrsInTasks( taskHeader, queryOut.m_jacobianSchemas, queryOut.m_jacobians );

		HK_TIMER_END_LIST();
	}

#endif

	return jobQueue.finishJobAndGetNextJob( hkJobQueue::THREAD_TYPE_CPU, &jobInOut, jobInOut, 0, waitStatus );
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
