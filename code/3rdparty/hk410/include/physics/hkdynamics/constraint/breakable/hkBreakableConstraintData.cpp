/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/constraint/breakable/hkBreakableConstraintData.h>
#include <hkconstraintsolver/constraint/bilateral/hk1dBilateralConstraintInfo.h>
#include <hkconstraintsolver/constraint/atom/hkBuildJacobianFromAtoms.h>
#include <hkconstraintsolver/constraint/hkConstraintQueryIn.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkdynamics/constraint/breakable/hkBreakableListener.h>
#include <hkdynamics/constraint/hkConstraintOwner.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkBreakableConstraintData);

hkBreakableConstraintData::hkBreakableConstraintData( hkConstraintData* constraintData, hkWorld* world )  
: m_constraintData(constraintData), 
  m_world(world), 
  m_solverResultLimit(10),
  m_removeWhenBroken(false),
  m_listener(HK_NULL)
{
	m_revertBackVelocityOnBreak = false;
	m_constraintData->addReference();

	hkConstraintData::RuntimeInfo info;
	m_constraintData->getRuntimeInfo( true, info );
	m_childRuntimeSize      = hkUint16(info.m_sizeOfExternalRuntime);
	m_childNumSolverResults = hkUint16(info.m_numSolverResults);

	m_atoms.m_bridgeAtom.init( this );
}

hkBreakableConstraintData::hkBreakableConstraintData(hkFinishLoadedObjectFlag f) : hkConstraintData(f), m_atoms(f) 
{
	m_atoms.m_bridgeAtom.init( this );
}


hkBreakableConstraintData::~hkBreakableConstraintData()
{
	m_constraintData->removeReference();
}

void hkBreakableConstraintData::buildJacobian( const hkConstraintQueryIn &in, hkConstraintQueryOut &out )
{
   // Determine if reaction forces from previous solution should cause a break
	hkSolverResults* results = static_cast<hkSolverResults*>( in.m_constraintRuntime.val() );
	int numResults = m_childNumSolverResults;
	HK_ASSERT2( 0xf06521fe, in.m_constraintRuntime , "The constraint which is wrapped by the hkBreakableConstraintData does not support breaking" );

	Runtime* runtime = getRuntime( in.m_constraintRuntime );
	if ( !runtime->m_isBroken )
	{
        hkReal sumReactionForce = 0;
		for(int j = 0; j < numResults; j++)
		{ 
			hkReal impulse = results[j].m_impulseApplied;
			sumReactionForce += impulse * impulse;
		} 

	    if(sumReactionForce >  m_solverResultLimit * m_solverResultLimit)
		{
   			runtime->m_isBroken = true;
  
			if(m_listener != HK_NULL)
			{
				hkBreakableConstraintEvent data;
				hkConstraintInstance* constraintInstance = static_cast<hkConstraintInstance*>(in.m_constraintInstance.val());
				data.m_constraintInstance = constraintInstance; 
				data.m_forceMagnitude = hkMath::sqrt(sumReactionForce);
				data.m_removed = m_removeWhenBroken;
				data.m_breakableConstraintData = this;
				m_listener->constraintBrokenCallback(data);
			}

			if ( this->m_revertBackVelocityOnBreak )
			{
				// revert back the velocities
				hkSimdReal f = m_solverResultLimit * hkMath::sqrtInverse( sumReactionForce );
				
				hkVector4 linA; { hkReal* s = runtime->m_linearVelcityA; linA.set( s[0], s[1], s[2] ); }
				hkVector4 linB; { hkReal* s = runtime->m_linearVelcityB; linB.set( s[0], s[1], s[2] ); }
				hkVector4 angA; { hkReal* s = runtime->m_angularVelcityA; angA.set( s[0], s[1], s[2] ); }
				hkVector4 angB; { hkReal* s = runtime->m_angularVelcityB; angB.set( s[0], s[1], s[2] ); }

				hkVelocityAccumulator* bodyA = in.m_bodyA;
				hkVelocityAccumulator* bodyB = in.m_bodyB;
				{ hkVector4& d = bodyA->m_linearVel; d.setInterpolate4( linA, d, f ); }
				{ hkVector4& d = bodyB->m_linearVel; d.setInterpolate4( linB, d, f ); }
				{ hkVector4& d = bodyA->m_angularVel; d.setInterpolate4( angA, d, f ); }
				{ hkVector4& d = bodyB->m_angularVel; d.setInterpolate4( angB, d, f ); }
			}
		} 
	}


	if(!runtime->m_isBroken)
    {
		hkVelocityAccumulator* bodyA = in.m_bodyA;
		hkVelocityAccumulator* bodyB = in.m_bodyB;
		// Save the velocity accumulators
		{ hkReal* d = runtime->m_linearVelcityA;  hkVector4& s = bodyA->m_linearVel;  d[0] = s(0); d[1] = s(1); d[2] = s(2); }
		{ hkReal* d = runtime->m_linearVelcityB;  hkVector4& s = bodyB->m_linearVel;  d[0] = s(0); d[1] = s(1); d[2] = s(2); }
		{ hkReal* d = runtime->m_angularVelcityA; hkVector4& s = bodyA->m_angularVel; d[0] = s(0); d[1] = s(1); d[2] = s(2); }
		{ hkReal* d = runtime->m_angularVelcityB; hkVector4& s = bodyB->m_angularVel; d[0] = s(0); d[1] = s(1); d[2] = s(2); }
		
		hkConstraintData::ConstraintInfo info;	m_constraintData->getConstraintInfo(info);
		hkSolverBuildJacobianFromAtoms(	info.m_atoms, info.m_sizeOfAllAtoms, in, out );
	}
	else
	{
        // insert a nop statement into the solver
 		buildNopJacobian(in,out);
		if ( m_removeWhenBroken )
		{
			hkConstraintInstance* constraint = in.m_constraintInstance.val();
			hkWorld* world = constraint->getEntityA()->getWorld();
			world->removeConstraint(constraint);
		}
	}

	// zero results at the end so we can use recursive breakable constraints
	for(int j = 0; j < numResults; j++)
	{
		results[j].m_impulseApplied = 0;   
	}
}


void hkBreakableConstraintData::buildNopJacobian( const hkConstraintQueryIn& in, hkConstraintQueryOut& out )
{
	hkBeginConstraints( in, out, HK_NULL, sizeof(hkSolverResults) );
	hkEndConstraints();
}


void hkBreakableConstraintData::getConstraintInfo( hkConstraintData::ConstraintInfo& info ) const
{
	m_constraintData->getConstraintInfo( info);
	info.m_atoms = const_cast<hkConstraintAtom*>(m_atoms.getAtoms());
	info.m_sizeOfAllAtoms = m_atoms.getSizeOfAllAtoms();
}


void hkBreakableConstraintData::getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const 
{
	infoOut.m_numSolverResults = m_childNumSolverResults;
	infoOut.m_sizeOfExternalRuntime = m_childRuntimeSize + sizeof(Runtime);
}

hkBool hkBreakableConstraintData::isValid() const
{
	return m_constraintData->isValid();
}


int hkBreakableConstraintData::getType() const
{
	return hkConstraintData::CONSTRAINT_TYPE_BREAKABLE;
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
