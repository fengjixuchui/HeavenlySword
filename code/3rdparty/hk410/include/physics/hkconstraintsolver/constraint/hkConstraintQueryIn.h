/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONSTRAINTSOLVER2_CONSTRAINT_QUERY_IN_H
#define HK_CONSTRAINTSOLVER2_CONSTRAINT_QUERY_IN_H

#include <hkmath/hkMath.h>
#include <hkmath/basetypes/hkStepInfo.h>
#include <hkconstraintsolver/solve/hkSolverInfo.h>

class hkVelocityAccumulator;
class hkJacobianElement;


	/// The time information, the constraints get access to
class hkConstraintQueryStepInfo
{
	public:
			/// The delta time of each solvers substep
		HK_ALIGN16( hkPadSpuf<float>		m_substepDeltaTime );

		hkPadSpuf<float>		m_microStepDeltaTime;

			/// The 1.0f/m_substepDeltaTime
		hkPadSpuf<float>		m_substepInvDeltaTime;


		hkPadSpuf<float>		m_frameDeltaTime;
		hkPadSpuf<float>		m_frameInvDeltaTime;
		hkPadSpuf<float>		m_invNumSteps;
		hkPadSpuf<float>		m_invNumStepsTimesMicroSteps;	// = 1.0f/(numSubsteps*numMicroSteps)
	

			// a factor all rhs should be multiplied
		hkPadSpuf<float>		m_rhsFactor;

			// a factor all invMasses should be multiplied
		hkPadSpuf<float>		m_virtMassFactor;

			// a factor for all rhs used for friction calculations
		hkPadSpuf<float> m_frictionRhsFactor;
};


	/// The input structure to hkConstraints::buildJacobian.
class hkConstraintQueryIn: public hkConstraintQueryStepInfo
{
	public:
	
		hkConstraintQueryIn() { }

			/// The velocity accumulator of bodyA. Typically this is a hkVelocityAccumulator
		hkPadSpu<hkVelocityAccumulator*>	m_bodyA;

			/// The velocity accumulator of bodyB. Typically this is a hkVelocityAccumulator
		hkPadSpu<hkVelocityAccumulator*>	m_bodyB;

			/// the transform of rigid body A, identity transform if not available
		hkPadSpu<const hkTransform*>	m_transformA;

			/// the transform of rigid body N, identity transform if not available
		hkPadSpu<const hkTransform*>	m_transformB;

			/// the current global solver settings
		hkPadSpuf<hkReal> m_tau;

			/// the current global solver settings
		hkPadSpuf<hkReal> m_damping;

			/// if this class is used with the hkdynamics library, this points to an hkConstraintInstance
		hkPadSpuLong<HK_CPU_PTR(class hkConstraintInstance*)>	m_constraintInstance;

			/// if this class is used with the hkdynamics library, this points to an hkConstraintRuntime
		hkPadSpu<void*>	m_constraintRuntime;


			// Data to be written to the header schema
		hkPadSpu<hkUint32> m_bodyAOffset;
		hkPadSpu<hkUint32> m_bodyBOffset;

		hkPadSpu<HK_CPU_PTR(void*)> m_constraintRuntimeInMainMemory;

#if defined( HK_PLATFORM_HAS_SPU)
		hkPadSpuLong<HK_CPU_PTR(struct hkConstraintAtom*)> m_atomInMainMemory;
#endif
		inline void set( const hkSolverInfo& si, const hkStepInfo& stepInfo )
		{
			m_substepDeltaTime    = si.m_deltaTime;
			m_microStepDeltaTime  = si.m_deltaTime * si.m_numMicroSteps;
			m_substepInvDeltaTime = si.m_invDeltaTime;
			m_invNumSteps		  = si.m_invNumSteps;
			m_invNumStepsTimesMicroSteps	  = si.m_invNumSteps * si.m_invNumMicroSteps;
			m_tau			      = si.m_tau;
			m_damping		      = si.m_damping;

			m_rhsFactor			  = si.m_tauDivDamp * si.m_invDeltaTime;
			m_virtMassFactor      = si.m_damping;
			m_frictionRhsFactor   = si.m_frictionTauDivDamp * si.m_invDeltaTime;

			m_frameDeltaTime      = stepInfo.m_deltaTime;
			m_frameInvDeltaTime   = stepInfo.m_invDeltaTime;
		}
};



#endif // HK_CONSTRAINTSOLVER2_CONSTRAINT_QUERY_IN_H

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
