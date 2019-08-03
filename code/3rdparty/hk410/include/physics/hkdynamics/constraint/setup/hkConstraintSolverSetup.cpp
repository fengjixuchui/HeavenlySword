/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkbase/memory/hkScratchpad.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/debugutil/hkTraceStream.h>

#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryOut.h>
#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>
#include <hkconstraintsolver/constraint/atom/hkBuildJacobianFromAtoms.h>

#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>


#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/constraint/setup/hkConstraintSolverSetup.h>
#include <hkdynamics/constraint/contact/hkSimpleContactConstraintData.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/util/hkWorldConstraintUtil.h>
#include <hkdynamics/motion/hkMotion.h>
#include <hkdynamics/motion/util/hkRigidMotionUtil.h>

// make sure that the sizeof hkBuildJacobianTask is 2^x or just below
#if ( HK_POINTER_SIZE == 4 )
HK_COMPILE_TIME_ASSERT( sizeof(hkBuildJacobianTask) >= 0x0f80);
HK_COMPILE_TIME_ASSERT( sizeof(hkBuildJacobianTask) <= 0x1000);
#endif


	// init the different buffers to taskHeader
int hkConstraintSolverSetup::calcBufferOffsetsForSolve( hkSimulationIsland& island, char* buffer, int bufferSize, hkBuildJacobianTaskHeader& taskHeader )
{
	taskHeader.m_buffer = buffer;
	int numBodies = island.getEntities().getSize();
	{
		taskHeader.m_accumulatorsBase = reinterpret_cast<hkVelocityAccumulator*>(buffer);
			// fixed rigid body + end tag (16 byte aligned)
		int accumSize =  hkSizeOf(hkVelocityAccumulator) + 16 + hkSizeOf(hkVelocityAccumulator) * island.getEntities().getSize();
		buffer += accumSize; 
	}
	{
		HK_ASSERT2(0xad888ddd, !(hkUlong(buffer) & 0x0f), "The jacobian buffer start is not 16-byte aligned!");
		taskHeader.m_jacobiansBase = reinterpret_cast<hkJacobianElement*>(buffer);
		buffer += island.m_constraintInfo.m_sizeOfJacobians;
	}

	// now we continue 4 byte aligned
	{
		int sizeForElemTemp = island.m_constraintInfo.m_numSolverResults * hkSizeOf(hkSolverElemTemp) + 2 * hkSizeOf(hkSolverElemTemp);
		sizeForElemTemp = HK_NEXT_MULTIPLE_OF(16, sizeForElemTemp);
		taskHeader.m_solverTempBase = reinterpret_cast<hkSolverElemTemp*>(buffer);
			// the 2 * sizeof(hkSolverElemTemp) is needed due to micro mode double buffering technique
			// (actually always 3 results are written back)
		buffer += sizeForElemTemp;
	}
	
	{
		taskHeader.m_schemasBase = reinterpret_cast<hkJacobianSchema*>(buffer);
			// schemas + alignment padding for ppu/spu jobs + end schema
		int sizeForSchemas = island.m_constraintInfo.m_sizeOfSchemas + HK_SIZE_OF_JACOBIAN_END_SCHEMA;
#ifdef HK_PLATFORM_HAS_SPU
		// calculate padding for schemas
		{
			int numTasks = (island.m_numConstraints/hkBuildJacobianTask::MAX_NUM_ATOM_INFOS) + (1+1);	// one task is the ppu only constraints
			sizeForSchemas += 12 * ( numTasks );
		}
#endif
		sizeForSchemas = HK_NEXT_MULTIPLE_OF(16, sizeForSchemas);
		buffer += sizeForSchemas;
	}

	int size = int(buffer - (char*)taskHeader.m_buffer);
	taskHeader.m_bufferSize = size;

	if ( taskHeader.m_buffer )
	{
		HK_ASSERT2( 0xf0434434, bufferSize <= size, "The buffer you used is too small" );
		hkVelocityAccumulator* ra = taskHeader.m_accumulatorsBase;
		ra[0].setFixed();
		ra[1+numBodies].m_type = hkVelocityAccumulator::HK_END;
	}
	return size;
}

int hkConstraintSolverSetup::calcBufferSize( hkSimulationIsland& island, int minJacobiansPerJob )
{
	hkBuildJacobianTaskHeader taskHeader;
	return calcBufferOffsetsForSolve( island, HK_NULL, 0, taskHeader );
}


void HK_CALL hkConstraintSolverSetup::_buildAccumulators( const hkStepInfo& info, hkEntity*const* bodiesIn, int numEntities, hkVelocityAccumulator* accumOut )
{
	// set the static rigid body
	accumOut[0].setFixed();

	for (int i = 0; i < numEntities; i++ )
	{
		hkUint32 newVal = (1+i)*sizeof(hkVelocityAccumulator);
			// try to avoid making the cacheline dirty
		if ( newVal != bodiesIn[i]->m_solverData)
		{
			bodiesIn[i]->m_solverData = newVal;
		}
	}

	hkVelocityAccumulator* end = hkRigidMotionUtilApplyForcesAndBuildAccumulators( info, (hkMotion*const*)bodiesIn, numEntities, HK_OFFSET_OF(hkEntity, m_motion), accumOut + 1  );
	end->m_type = hkVelocityAccumulator::HK_END;
}



void HK_CALL hkConstraintSolverSetup::_buildJacobianElement( const hkConstraintInternal* c,
															 hkConstraintQueryIn& in,
															 hkVelocityAccumulator* baseAccum,
															 hkConstraintQueryOut& out
										)
{
	hkMath::prefetch128(c->m_constraint);
	hkMath::prefetch128(c->m_constraint+64);
	hkMath::prefetch128(c->m_constraint+128);

	hkEntity* rA = c->m_entities[0];
	hkEntity* rB = c->m_entities[1];

	hkMotion* cA = rA->getMotion();
	hkMotion* cB = rB->getMotion();
	in.m_bodyA = hkAddByteOffset(baseAccum, rA->m_solverData );
	in.m_bodyB = hkAddByteOffset(baseAccum, rB->m_solverData );

	in.m_transformA = &(static_cast<const hkMotion*>(cA)->getTransform());
	in.m_transformB = &(static_cast<const hkMotion*>(cB)->getTransform());

	in.m_constraintInstance = c->m_constraint;
	in.m_constraintRuntime  = c->m_runtime;

	{
		in.m_bodyAOffset = rA->m_solverData;
		in.m_bodyBOffset = rB->m_solverData;
		in.m_constraintRuntimeInMainMemory = in.m_constraintRuntime;
#	if defined (HK_PLATFORM_HAS_SPU)
		in.m_atomInMainMemory = c->getAtoms();
#	endif
	}


	HK_ASSERT( 0xf0140201, &rA->getCollidable()->getTransform() == in.m_transformA );
	HK_ASSERT( 0xf0140201, &rB->getCollidable()->getTransform() == in.m_transformB );

	HK_ON_DEBUG(hkJacobianSchema* oldSchemas = out.m_jacobianSchemas);

	hkSolverBuildJacobianFromAtoms(	c->getAtoms(), c->getAtomsSize(), in, out );

#ifdef HK_DEBUG
	{
		// check for consistence within the out.m_jacobians variable
		//HK_ASSERT(0x5bd9c048,  info.m_sizeOfJacobians <= info.m_maxSizeOfJacobians );
		// HK_ASSERT(0x194320f2,  out.m_jacobians - el <= info.m_sizeOfJacobians * hkSizeOf(hkJacobianElement));
		// check m_maxSizeOfSchemas
		HK_ON_DEBUG( int currentSizeOfSchemas = hkGetByteOffsetInt(oldSchemas, out.m_jacobianSchemas.val() ));
		HK_ASSERT(0x2e11828a, currentSizeOfSchemas  <= c->m_sizeOfSchemas );
	}
#endif
}

	///	build a partial list of jacobians and schemas. If nextSchema is set, this function will insert a goto schema at the
	/// end of the schemas
void HK_CALL hkConstraintSolverSetup::buildJacobianElementBatch( hkConstraintQueryIn& in,
																hkConstraintInternal** constraints, int numConstraints,
																hkVelocityAccumulator* baseAccum,
																hkJacobianSchema*  schemaOut,
																hkJacobianElement* jacobianOut,
																hkJacobianSchema* nextSchema	)
{
	HK_WARN_ONCE(0xad899dd, "THIS CODE IS OBSOLETE -- IF YOU USE IT NOTE THAT out.m_jacobiansBase ONLY WORKS WHEN THERE IS ONE SINGLE TASK FOR BULIDING JACOBIANS");
	hkConstraintQueryOut out;
	out.m_jacobians            = jacobianOut;
	out.m_jacobiansVirtualBase = jacobianOut; // if there are multiple batches, this will have to get the proper offset to the local batch jacobian buffer.
	out.m_jacobianSchemas      = schemaOut; 

	{
		hkConstraintInternal** pc = constraints;
		hkConstraintInternal** pcEnd = constraints + numConstraints;

		for ( ;pc < pcEnd; pc++ )
		{
			_buildJacobianElement( *pc, in, baseAccum, out );
		}
	}
	if ( !nextSchema )
	{
		// that's the CONSTRAINT_LIST_END flag.
		*(hkUint32*)(out.m_jacobianSchemas.val()) = 0;
	}
	else if ( out.m_jacobianSchemas != nextSchema )
	{
		// insert goto
		HK_ASSERT(0,0);	// goto schema not merged yet
		//HK_ASSERT( 0xf0324d54, hkGetByteOffsetInt( out.m_jacobianSchemas, nextSchema ) >= HK_SIZE_OF_JACOBIAN_GOTO8_SCHEMA );
		//hkGotoSchemaBuildJacobian( nextSchema, in, out );
	}

	// that's the CONSTRAINT_LIST_END flag.
	*(reinterpret_cast<hkInt32*>(out.m_jacobianSchemas.val())) = 0;

}

extern void hkSimpleContactConstraintData_fireCallbacks(class hkSimpleContactConstraintData* constraintData, const hkConstraintQueryIn* in, hkSimpleContactConstraintAtom* atom );

///	Each constraints get queried: It produces a few jacobian elements, returns the number of jacobians generated
void HK_CALL hkConstraintSolverSetup::_buildJacobianElements( hkConstraintQueryIn& in,
										hkEntity*const* bodies, int numBodies,
										hkVelocityAccumulator* baseAccum,
										hkJacobianElement* jacobianOut,
										hkJacobianElement* jacobianOutEnd,
										hkJacobianElement* jacobianOutB,
										hkJacobianSchema*  schemaOut
										)
{
	hkConstraintQueryOut out;

	out.m_jacobians = jacobianOut;
	out.m_jacobiansVirtualBase = jacobianOut;
	out.m_jacobianSchemas = schemaOut; 

	hkEntity*const* e = bodies;
	hkEntity*const* eEnd = bodies + numBodies;
	
	hkInplaceArray<const hkConstraintInternal*, 256> criticalConstraints;

	for ( ; e < eEnd; e++ )
	{
		const hkConstraintInternal* c = (*e)->getConstraintMasters().begin();
		const hkConstraintInternal* cEnd = (*e)->getConstraintMasters().end();

		for ( ;c < cEnd; c++ )
		{
			// collect constraints that need to fire a callback
			if ( c->m_callbackRequest & hkConstraintAtom::CALLBACK_REQUEST_CONTACT_POINT )
			{
				in.m_constraintInstance = c->m_constraint;
				in.m_bodyA = hkAddByteOffset( baseAccum, c->m_constraint->getEntityA()->m_solverData);
				in.m_bodyB = hkAddByteOffset( baseAccum, c->m_constraint->getEntityB()->m_solverData);
				in.m_constraintRuntime = HK_NULL;
				in.m_transformA = &c->m_constraint->getEntityA()->getCollidable()->getTransform();
				in.m_transformB = &c->m_constraint->getEntityB()->getCollidable()->getTransform();

				hkSimpleContactConstraintData* cdata = static_cast<hkSimpleContactConstraintData*>(c->m_constraint->getData());
				hkSimpleContactConstraintAtom* atom = cdata->m_atom;

				hkSimpleContactConstraintData_fireCallbacks( cdata, &in, atom );
			}

			if (c->m_priority >= hkConstraintInstance::PRIORITY_TOI_HIGHER)
			{
				// Put this constraint on a list, which will be processed later
				criticalConstraints.pushBack(c);
				continue;
			}

			if ( out.m_jacobians >= jacobianOutEnd )
			{
					 // a safety check 
				HK_ASSERT(0x77f6b793,  ( (hkUlong)(jacobianOutEnd) & 0x1) == 0 );
				out.m_jacobians = jacobianOutB;
				jacobianOutEnd = reinterpret_cast<hkJacobianElement*>( ~(hkUlong)0 ); 
			}
			_buildJacobianElement( c, in, baseAccum, out );
		}
	}

	{
		for (int i = 0; i < criticalConstraints.getSize(); i++)
		{
			const hkConstraintInternal* c = criticalConstraints[i];

			if ( out.m_jacobians >= jacobianOutEnd )
			{
					 // a safety check 
				HK_ASSERT(0x77f6b793,  ( (hkUlong)(jacobianOutEnd) & 0x1) == 0 );
				out.m_jacobians = jacobianOutB;
				jacobianOutEnd = reinterpret_cast<hkJacobianElement*>( ~(hkUlong)0 ); 
			}
			_buildJacobianElement( c, in, baseAccum, out );
		}
	}
		// that's the CONSTRAINT_LIST_END flag.
	*(hkUint32*)(out.m_jacobianSchemas.val()) = 0;
}


// void HK_CALL hkConstraintSolverSetup::buildJacobianElements( hkConstraintQueryIn& in,
// 										hkEntity*const* bodies, int numBodies,
// 										hkVelocityAccumulator* baseAccum,
// 										hkJacobianElement* jacobianOut,
// 										hkJacobianElement* jacobianOutEnd,
// 										hkJacobianElement* jacobianOutB,
// 										hkJacobianSchema*  schemaOut
// 										)
// 	{
// 	_buildJacobianElements( in, bodies, numBodies, baseAccum, jacobianOut, jacobianOutEnd, jacobianOutB, schemaOut );
// 	}

	// integrates a list of bodies and returns the number of inactive frames
static HK_FORCE_INLINE int HK_CALL hkConstraintSolverSetup_integrate2( const struct hkSolverInfo& si, const hkStepInfo& info, const hkVelocityAccumulator* accumulators, hkEntity*const* bodies, int numBodies )
{
#ifdef HK_DEBUG
	const hkVelocityAccumulator* rm = accumulators + 1;
	for (int i = 0; i<numBodies; i++, rm++)	{	HK_ASSERT( 0xf0232343, hkAddByteOffsetConst( accumulators, bodies[i]->m_solverData) == rm );	}
#endif
	return hkRigidMotionUtilApplyAccumulators(si, info, accumulators+1, (hkMotion*const*)bodies, numBodies, HK_OFFSET_OF(hkEntity, m_motion));
}

/// This function integrates the rigid bodies by using the data in the linear and angular velocity
/// field in the accumulators and not the sumLinearVelocity.
/// The sumLinearVelocities are typically set in the hkSolver::integrateVelocities, however if
/// you only call hkSolveStepJacobion, this sumLinearVelocities won't be used and you have to use this
/// function to integrate your rigid bodies
void HK_CALL hkConstraintSolverSetup::oneStepIntegrate( const struct hkSolverInfo& si, const hkStepInfo& info, const hkVelocityAccumulator* accumulatorsBase, hkEntity*const* entities, int numEntities )
{
	for (int i = 0; i<numEntities; i++)
	{
		hkEntity* entity = entities[i];
		hkMotion* motion = entity->getMotion();

		hkVelocityAccumulator accu;
	{
			const hkVelocityAccumulator* srcAccu = hkAddByteOffsetConst( accumulatorsBase, entity->m_solverData);
			accu = *srcAccu;
		}

		accu.getSumLinearVel()  = accu.m_linearVel;
		accu.getSumAngularVel() = accu.m_angularVel;

		// we have to call that function for one motion at a time because the order of the accumulators might not match the order of the entities (and thus motions)!
		const hkBool processDeactivation = false;
		hkRigidMotionUtilApplyAccumulators(si, info, &accu, &motion, 1, 0, processDeactivation);
	}
}


int HK_CALL hkConstraintSolverSetup::solve(
									const hkStepInfo& stepInfo, const hkSolverInfo& solverInfo,
									hkConstraintQueryIn& constraintQueryIn, hkSimulationIsland& island,
									hkEntity*const * bodies, int numBodies )
{
	HK_INTERNAL_TIMER_BEGIN_LIST("solver","memory");
	int numInactiveFrames = 0;

	char* scratch2 = HK_NULL;

	//
	//	values to be set
	//
	int sizeForElemTemp = island.m_constraintInfo.m_numSolverResults * hkSizeOf(hkSolverElemTemp) + 2 * hkSizeOf(hkSolverElemTemp);
	hkJacobianElement*	jacobiansB= HK_NULL;

#ifdef HK_PLATFORM_PS2

	//
	//	Do an optimistic approach 
	//
	char* scratchpad = static_cast<char*>( hkScratchpad::getInstance().acquire() );
	char* buffer = scratchpad;
	char* bufferEnd = buffer + hkScratchpad::getInstance().getSizeInBytes();

#else

	//
	//	Grep as much buffer memory as possible
	//
	int sizeOfScratchPad = hkThreadMemory::getInstance().getStack().getFreeBytes();
	char* scratchpad = hkAllocateStack<char>( sizeOfScratchPad );
	char* buffer = scratchpad;
	char* bufferEnd = buffer + sizeOfScratchPad;

#endif

	//
	//	Reserve memory for the DOF objects
	//
redoAll:
	hkVelocityAccumulator* velAccums;
	{
		velAccums = reinterpret_cast<hkVelocityAccumulator*>(buffer);
		buffer += hkSizeOf(hkVelocityAccumulator) + 16 + hkSizeOf(hkVelocityAccumulator) * numBodies; // fixed rigid body + end tag (16 byte aligned)
	}

//redoAllbutAccum:
	//
	//	Reserve memory for the jacobians
	//
	hkJacobianElement*	jacobians;
	hkJacobianElement*	jacobiansEnd;
	{
		jacobians = reinterpret_cast<hkJacobianElement*>(buffer);
		buffer += island.m_constraintInfo.m_sizeOfJacobians;
		jacobiansEnd = reinterpret_cast<hkJacobianElement*>(buffer);
	}


redoSchemasAndElemTemp:

	//
	//  Reserve memory for temporary data
	//
	hkSolverElemTemp*		elemTemp;
	{
		elemTemp = reinterpret_cast<hkSolverElemTemp*>(buffer);
			// the 2 * sizeof(hkSolverElemTemp) is needed due to micro mode double buffering technique
			// (actually always 3 results are written back)
		buffer += sizeForElemTemp;
	}
	
	
	//
	// Reserve memory for the schemas
	//
	hkJacobianSchema*	schemas;
	{
		schemas = reinterpret_cast<hkJacobianSchema*>(buffer);
		buffer += island.m_constraintInfo.m_sizeOfSchemas + HK_SIZE_OF_JACOBIAN_END_SCHEMA;
	}



	if ( buffer <= bufferEnd )	// scratchpad ok,  
	{
			//
			//	Convert all rigid bodies to velocity accumulators
			//
		HK_INTERNAL_TIMER_SPLIT_LIST("BuildAccum");
		_buildAccumulators( stepInfo, bodies, numBodies, velAccums );

		{	//	clear a few arrays
			for (int i = 0; i < island.m_constraintInfo.m_numSolverResults; i++)		{			elemTemp[i].m_impulseApplied = 0.0f;		}
		}

			//
			//	Build jacobians
			//
		HK_INTERNAL_TIMER_SPLIT_LIST("BuildJac");
		_buildJacobianElements( constraintQueryIn, bodies, numBodies, velAccums, jacobians, jacobiansEnd, jacobiansB, schemas  );

		HK_INTERNAL_TIMER_SPLIT_LIST("solve");
		// solve constraints can't return a hkBool, so it returns an int instead {0|1}
		int solved = hkSolveConstraints( solverInfo, schemas, velAccums, jacobians, elemTemp );
		HK_MONITOR_ADD_VALUE( "NumJacobians", float(island.m_constraintInfo.m_numSolverResults), HK_MONITOR_TYPE_INT );
		HK_MONITOR_ADD_VALUE( "NumEntities", float(numBodies), HK_MONITOR_TYPE_INT );


		HK_INTERNAL_TIMER_SPLIT_LIST("WriteBack");
		if (solved == 1)
		{
			hkExportImpulsesAndRhs(solverInfo, elemTemp, schemas, velAccums, jacobians );
			HK_INTERNAL_TIMER_SPLIT_LIST("Integrate");
			numInactiveFrames = hkConstraintSolverSetup_integrate2( solverInfo, stepInfo, velAccums, bodies, numBodies );
		}
		HK_INTERNAL_TIMER_END_LIST();

		if ( scratch2 )
		{
			hkDeallocateStack<char>( scratch2 ); 
		}

#ifdef HK_PLATFORM_PS2
		hkScratchpad::getInstance().release(); 
#else
		hkDeallocateStack<char>( scratchpad );
#endif
		return numInactiveFrames;
	}

	//
	//	redo all allocations by using secondary scratchpad
	//
	HK_ASSERT(0x692867c8,  scratch2 == HK_NULL );

	// check for vel accums fitting on scratch
	if ( (char*)jacobians >= bufferEnd )
	{
		// if not, forget scratch1 area entirely
		int bufferSize = static_cast<int>( hkUlong(buffer - scratchpad) );
		scratch2 = hkAllocateStack<char>( bufferSize );

		buffer = scratch2;
		bufferEnd = buffer + bufferSize;

		//!me wouldn't this be nice?
		HK_ASSERT2(0x2bf44da9,  buffer != HK_NULL, "out of memory, trying allocate space for solver on scratch2.  No simulation will happen" );

		goto redoAll;
	}

	// split jac 
	if ( (char*)jacobiansEnd >= bufferEnd )
	{
		int jacSize1    = static_cast<int>( (hkUlong)(bufferEnd - (char*)jacobians) ) - island.m_constraintInfo.m_maxSizeOfJacobians; //64bit change, (was   char* - char* - int)
		int jacSize2    = island.m_constraintInfo.m_sizeOfJacobians + island.m_constraintInfo.m_maxSizeOfJacobians - jacSize1;

		int bufferSize2 = sizeForElemTemp + island.m_constraintInfo.m_sizeOfSchemas + HK_SIZE_OF_JACOBIAN_END_SCHEMA + jacSize2;

		scratch2 = hkAllocateStack<char>( bufferSize2 );

		jacobiansEnd = reinterpret_cast<hkJacobianElement*>(bufferEnd - island.m_constraintInfo.m_maxSizeOfJacobians);

		buffer = scratch2;
		bufferEnd = buffer + bufferSize2;

		jacobiansB = reinterpret_cast<hkJacobianElement*>(buffer);
		buffer += jacSize2;
	}
	else
	{
		// only put schemas and elemTemp on scratch2
		int bufferSize = island.m_constraintInfo.m_sizeOfSchemas + sizeForElemTemp + HK_SIZE_OF_JACOBIAN_END_SCHEMA;
		scratch2 = hkAllocateStack<char>( bufferSize );
		buffer = scratch2;
		bufferEnd = buffer + bufferSize;
	}

	goto redoSchemasAndElemTemp;
}




	// acquires the scratchpad and initializes a hkConstraintSolverResources struct
void HK_CALL hkConstraintSolverSetup::initializeSolverState( hkStepInfo& stepInfo, hkSolverInfo& solverInfo, hkConstraintQueryIn& constraintQueryIn, char* buffer, int bufferSize, hkConstraintSolverResources& s)

{
	// Initialize SolverSate struct
	s.m_stepInfo             = &stepInfo;
	s.m_solverInfo           = &solverInfo;
	s.m_constraintQueryInput = &constraintQueryIn;

	      bufferSize -= bufferSize % 16;
	char* bufferEnd = buffer + bufferSize;


	const int ratioAccumulators = 10;	//hkSizeOf(hkVelocityAccumulator),
	const int ratioAccumulatorsBackup = 2 + ratioAccumulators * sizeof(hkConstraintSolverResources::VelocityAccumTransformBackup)/sizeof(hkVelocityAccumulator);
	const int ratioJacobians = 30;		//hkSizeOf(hkJacobianElement),
	const int ratioElemTemp = 10;		//hkSizeOf(hkSolverElemTemp),
	const int ratioSchema = 10;			//hkSizeOf(hkJacobianSchema),
	const int sumSize = ratioAccumulators + ratioAccumulatorsBackup + ratioJacobians + ratioElemTemp + ratioSchema;

	// normalSchemasSize / forceQualitySchemasSize ratio
	hkReal schemaBuffersRatio = 0.6f;

	{
		hkUlong b = hkUlong(buffer);

			// Memory
		b = HK_NEXT_MULTIPLE_OF(16,b);

		s.m_accumulators     = reinterpret_cast<hkVelocityAccumulator*>(b);
		b = b + bufferSize * ratioAccumulators / sumSize;
		b = HK_NEXT_MULTIPLE_OF(16,b);
		s.m_accumulatorsEnd  = reinterpret_cast<hkVelocityAccumulator*>(b);	
		
		s.m_accumulatorsBackup     = reinterpret_cast<hkConstraintSolverResources::VelocityAccumTransformBackup*>(b);
		b = b + bufferSize * ratioAccumulatorsBackup / sumSize;
		b = HK_NEXT_MULTIPLE_OF(16,b);
		s.m_accumulatorsBackupEnd = reinterpret_cast<hkConstraintSolverResources::VelocityAccumTransformBackup*>(b);

		s.m_jacobians        = reinterpret_cast<hkJacobianElement*>(b);
		b = b + bufferSize * ratioJacobians / sumSize;
		b = HK_NEXT_MULTIPLE_OF(16,b);
		s.m_jacobiansEnd     = reinterpret_cast<hkJacobianElement*>(b);

		s.m_elemTemp         = reinterpret_cast<hkSolverElemTemp*>(b);
		b = b + bufferSize * ratioElemTemp / sumSize;
		b = HK_NEXT_MULTIPLE_OF(16,b);
		s.m_elemTempEnd      = reinterpret_cast<hkSolverElemTemp*>(b);

		s.m_schemas[0].m_begin  = reinterpret_cast<hkJacobianSchema*>(b);
		b = b + (unsigned int)(bufferSize * ratioSchema * schemaBuffersRatio / sumSize);
		b = HK_NEXT_MULTIPLE_OF(16,b);
		s.m_schemas[0].m_end    = reinterpret_cast<hkJacobianSchema*>(b);

		s.m_schemas[1].m_begin  = reinterpret_cast<hkJacobianSchema*>(b);
		HK_ASSERT(0xf0ff0021, b < hkUlong(bufferEnd));
		b = hkUlong(bufferEnd);
		HK_ASSERT2( 0, (b & 0xf) == 0, "Your bufferEnd is not aligned" );
		s.m_schemas[1].m_end    = reinterpret_cast<hkJacobianSchema*>(b);
	}
	s.m_accumulatorsCurrent = s.m_accumulators;
	s.m_jacobiansCurrent    = s.m_jacobians;
	s.m_elemTempCurrent     = s.m_elemTemp;

	s.m_elemTempLastProcessed = s.m_elemTemp;
	s.m_schemas[0].m_lastProcessed = s.m_schemas[0].m_current = s.m_schemas[0].m_begin;
	s.m_schemas[1].m_lastProcessed = s.m_schemas[1].m_current = s.m_schemas[1].m_begin;
}

	// released the scratchpad
void HK_CALL hkConstraintSolverSetup::shutdownSolver(hkConstraintSolverResources& s)
{
}

	// builds accumulators
void HK_CALL hkConstraintSolverSetup::internalAddAccumulators(hkConstraintSolverResources& s, hkEntity*const* entities, int numEntities)
{
	if (!numEntities)
	{
		return;
	}

	hkVelocityAccumulator* mot = s.m_accumulatorsCurrent;

		// set the static rigid body
	if(s.m_accumulatorsCurrent == s.m_accumulators)
	{
		mot->setFixed();
		s.m_accumulatorsCurrent = mot+1;
		hkConstraintSolverResources::VelocityAccumTransformBackup* fixedBackup = s.m_accumulatorsBackup;
		fixedBackup->m_angular.setZero4();
		fixedBackup->m_linear.setZero4();
	}

	for (unsigned int i = 0; i < hkUint32(numEntities); i++ )
	{
		entities[i]->m_solverData = hkUint32(hkGetByteOffset(s.m_accumulators, s.m_accumulatorsCurrent )) + i * sizeof(hkVelocityAccumulator);
	}

	hkVelocityAccumulator* firstAccum = s.m_accumulatorsCurrent;
	hkConstraintSolverResources::VelocityAccumTransformBackup* firstBackup = &s.m_accumulatorsBackup[entities[0]->m_solverData/sizeof(hkVelocityAccumulator)];

	s.m_accumulatorsCurrent = hkRigidMotionUtilApplyForcesAndBuildAccumulators( *s.m_stepInfo, (hkMotion*const*)entities, numEntities,
																				HK_OFFSET_OF(hkEntity, m_motion),
																				firstAccum );
	// initialize the backup and set the velocity to zero
	for (unsigned int i = 0; i < hkUint32(numEntities); i++ )
	{
		firstBackup->m_linear  = firstAccum->getSumLinearVel();
		firstBackup->m_angular = firstAccum->getSumAngularVel();
		firstAccum->getSumLinearVel().setZero4();
		firstAccum->getSumAngularVel().setZero4();
		firstAccum++;
		firstBackup++;
		HK_ASSERT2( 0xf03412de, firstBackup <= s.m_accumulatorsBackupEnd, "Internal velocity accumulator backup buffer error");
	}

	// do not update s.m_accumulatorsCurrent after this.
	s.m_accumulatorsCurrent->m_type = hkVelocityAccumulator::HK_END;

	return;
}

//	Each constraint gets queried: It produces a few jacobian elements, returns the number of jacobians generated
void HK_CALL hkConstraintSolverSetup::internalAddJacobianElements( 
										hkConstraintSolverResources& s,
										hkConstraintInstance** constraints, 	int numConstraints,
										hkArray<hkConstraintSchemaInfo>& constraintStatus
										)
{
	hkConstraintQueryIn& in = *s.m_constraintQueryInput;
	hkConstraintQueryOut out;

	out.m_jacobians       = s.m_jacobiansCurrent;

	hkConstraintInstance** constr = constraints;
	hkConstraintInstance** cEnd   = constraints + numConstraints;


	for ( ;constr < cEnd; constr++ )
	{
		hkConstraintInstance* instance = constr[0];
		int listId = (int)( instance->getPriority() == hkConstraintInstance::PRIORITY_TOI_FORCED );

		hkEntity** entity = &instance->getInternal()->m_entities[0];
		hkMotion* motion[2] = { entity[0]->getMotion(), entity[1]->getMotion() };

		in.m_bodyA = hkAddByteOffset(s.m_accumulators, entity[0]->m_solverData );
		in.m_bodyB = hkAddByteOffset(s.m_accumulators, entity[1]->m_solverData );

		HK_ASSERT( 0xf0ff4565, in.m_bodyA < s.m_accumulatorsCurrent );
		HK_ASSERT( 0xf0ff4566, in.m_bodyB < s.m_accumulatorsCurrent );

		// use our backup transform, as the solver destroyed it (it is used to store the sum velocities)
		{
			hkConstraintSolverResources::VelocityAccumTransformBackup* backupA = &s.m_accumulatorsBackup[entity[0]->m_solverData/sizeof(hkVelocityAccumulator)];
			hkConstraintSolverResources::VelocityAccumTransformBackup* backupB = &s.m_accumulatorsBackup[entity[1]->m_solverData/sizeof(hkVelocityAccumulator)];
			in.m_bodyA.val()->getSumLinearVel()  = backupA->m_linear;	
			in.m_bodyA.val()->getSumAngularVel() = backupA->m_angular;	
			in.m_bodyB.val()->getSumLinearVel()  = backupB->m_linear;	
			in.m_bodyB.val()->getSumAngularVel() = backupB->m_angular;	
		}


		const hkMotionState* msA = static_cast<const hkMotion*>(motion[0])->getMotionState();
		const hkMotionState* msB = static_cast<const hkMotion*>(motion[1])->getMotionState();

		in.m_transformA = &msA->getTransform();
		in.m_transformB = &msB->getTransform();

		in.m_constraintInstance = instance;
		in.m_constraintRuntime = instance->getRuntime();

		HK_ASSERT( 0xf0140201, &entity[0]->getCollidable()->getTransform() == in.m_transformA );
		HK_ASSERT( 0xf0140201, &entity[1]->getCollidable()->getTransform() == in.m_transformB );

		//in.m_constraintInstance = HK_NULL; // this cannot be zeroed for normal contact points
		{
			in.m_bodyAOffset = entity[0]->m_solverData;
			in.m_bodyBOffset = entity[1]->m_solverData;
			in.m_constraintRuntimeInMainMemory = (HK_CPU_PTR(void*))(instance->getRuntime());
#if defined (HK_PLATFORM_HAS_SPU)
			in.m_atomInMainMemory = instance->m_internal->getAtoms();
#endif
			out.m_jacobiansVirtualBase = s.m_jacobians; 
		}


		//add ConstraintSchemaInfo
		{
			hkConstraintSchemaInfo& info = constraintStatus.expandOne();
			info.m_constraint     = instance;
			info.m_schema         = s.m_schemas[listId].m_current;
			hkReal allowedPenetrationDepthA = entity[0]->getCollidable()->m_allowedPenetrationDepth;
			hkReal allowedPenetrationDepthB = entity[1]->getCollidable()->m_allowedPenetrationDepth;
			info.m_allowedPenetrationDepth = hkMath::min2( allowedPenetrationDepthA, allowedPenetrationDepthB );

			out.m_jacobianSchemas = s.m_schemas[listId].m_current;
		}

		// fire contact callbacks
		if ( instance->m_internal->m_callbackRequest & hkConstraintAtom::CALLBACK_REQUEST_CONTACT_POINT )
		{
			hkSimpleContactConstraintData* cdata = static_cast<hkSimpleContactConstraintData*>(instance->getData());
			hkSimpleContactConstraintAtom* atom = cdata->m_atom;

			hkSimpleContactConstraintData_fireCallbacks( cdata, &in, atom );
		}
		hkSolverBuildJacobianFromAtoms(	instance->m_internal->getAtoms(), instance->m_internal->getAtomsSize(), in, out );

		s.m_jacobiansCurrent          = out.m_jacobians;
		s.m_schemas[listId].m_current = out.m_jacobianSchemas;
		{
#ifdef HK_STORE_CONSTRAINT_INFO_IN_INTERNAL
			s.m_elemTempCurrent       += instance->m_internal->m_numSolverResults; 
#	ifdef HK_DEBUG
			hkConstraintData::ConstraintInfo cinfo;		instance->getData()->getConstraintInfo(cinfo);
			HK_ASSERT( 0xf032edfd, cinfo.m_numSolverResults == instance->m_internal->m_numSolverResults);
#	endif
#else
			hkConstraintInfo cinfo;	instance->getData()->getConstraintInfo(cinfo);
			s.m_elemTempCurrent       += cinfo.m_numSolverResults;
#endif
		}

		{
			in.m_bodyA.val()->getSumLinearVel().setZero4();
			in.m_bodyA.val()->getSumAngularVel().setZero4();	
			in.m_bodyB.val()->getSumLinearVel().setZero4();	
			in.m_bodyB.val()->getSumAngularVel().setZero4();	
		}
	}


	// that's the CONSTRAINT_LIST_END flag.
	*(reinterpret_cast<hkInt32*>(s.m_schemas[0].m_current)) = 0;
	*(reinterpret_cast<hkInt32*>(s.m_schemas[1].m_current)) = 0;

	return;
}	

hkBool hkConstraintSolverSetup::internalIsMemoryOkForNewAccumulators    (hkConstraintSolverResources& s, hkEntity**     entities,    int numEntities)
{
	int bytesLeft = int ( hkGetByteOffset(s.m_accumulatorsCurrent, s.m_accumulatorsEnd ) );

		// set the static rigid body
	if(s.m_accumulatorsCurrent == s.m_accumulators)
	{
		numEntities++;
	}
	bytesLeft -= 16; // end flag

	if ( numEntities * hkSizeOf(hkVelocityAccumulator) > bytesLeft )
	{
		return false;
	}
	return true;

}

hkBool hkConstraintSolverSetup::internalIsMemoryOkForNewJacobianElements(hkConstraintSolverResources& s, hkConstraintInstance** constraints, int numConstraints)
{
	hkJacobianElement* jacobiansCurrent = s.m_jacobiansCurrent;
	hkJacobianSchema*  schemasCurrent[2] = { s.m_schemas[0].m_current, s.m_schemas[1].m_current };
	hkSolverElemTemp*  elemTempCurrent  = s.m_elemTempCurrent;

	hkConstraintInstance** constr = constraints;
	hkConstraintInstance** cEnd   = constraints + numConstraints;


	for ( ;constr < cEnd; constr++ )
	{
		int whichType = constr[0]->getPriority() >= hkConstraintInstance::PRIORITY_TOI_FORCED;
#ifdef HK_STORE_CONSTRAINT_INFO_IN_INTERNAL
		hkConstraintInternal* ci = constr[0]->m_internal;

		jacobiansCurrent = hkAddByteOffset( jacobiansCurrent, ci->m_sizeOfJacobians ) ;
		elemTempCurrent  += ci->m_numSolverResults;
		schemasCurrent[ whichType ] = hkAddByteOffset(schemasCurrent[ whichType ], ci->m_sizeOfSchemas );
#else
		hkConstraintInfo cinfo;		constr[0]->getData()->getConstraintInfo(cinfo);

		jacobiansCurrent = hkAddByteOffset( jacobiansCurrent, cinfo.m_sizeOfJacobians) ;
		elemTempCurrent  += cinfo.m_numSolverResults;
		schemasCurrent[ whichType ] = hkAddByteOffset(schemasCurrent[ whichType ], cinfo.m_sizeOfSchemas );
#endif
	}

	if ( jacobiansCurrent  > s.m_jacobiansEnd         ||
		 elemTempCurrent   > s.m_elemTempEnd          ||
		 hkAddByteOffset( schemasCurrent[0], 4) > s.m_schemas[0].m_end || // extra space for the END_SCHEMA
		 hkAddByteOffset( schemasCurrent[1], 4) > s.m_schemas[1].m_end )  // extra space for the END_SCHEMA
	{
		return false;
	}

	return true;	
}



void HK_CALL hkConstraintSolverSetup::subSolve(hkConstraintSolverResources& s, SolverMode mode)
{
	if (mode == SOLVER_MODE_PROCESS_ALL)
	{
		s.m_schemas[0].m_lastProcessed = s.m_schemas[0].m_begin;
		s.m_schemas[1].m_lastProcessed = s.m_schemas[1].m_begin;
	}

	// clear solver results for new elements
	{
		for ( hkSolverElemTemp* sr = s.m_elemTempLastProcessed; sr < s.m_elemTempCurrent; sr++ ){	sr->m_impulseApplied = 0.0f;	}
		s.m_elemTempLastProcessed = s.m_elemTempCurrent;
	}

#ifdef HK_DEBUG
	{
		for ( hkVelocityAccumulator* v = s.m_accumulators; v < s.m_accumulatorsCurrent; v++)
		{
			HK_ASSERT(0xf02de44, v->getSumLinearVel().equals3( hkVector4::getZero(), 0.0f ));
			HK_ASSERT(0xf02de43, v->getSumAngularVel().equals3( hkVector4::getZero(), 0.0f ));
		}
	}
#endif

	if ( s.m_schemas[0].m_lastProcessed != s.m_schemas[0].m_current )
	{
		hkSolveStepJacobians(*s.m_solverInfo, s.m_schemas[0].m_lastProcessed, s.m_accumulators, s.m_jacobians, s.m_elemTemp);
	}
	if ( s.m_schemas[1].m_lastProcessed != s.m_schemas[1].m_current )
	{
		hkSolveStepJacobians(*s.m_solverInfo, s.m_schemas[1].m_lastProcessed, s.m_accumulators, s.m_jacobians, s.m_elemTemp);
	}

	s.m_schemas[0].m_lastProcessed = s.m_schemas[0].m_current;
	s.m_schemas[1].m_lastProcessed = s.m_schemas[1].m_current;
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
