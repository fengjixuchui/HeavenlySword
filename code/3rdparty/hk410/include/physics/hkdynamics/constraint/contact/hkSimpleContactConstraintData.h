/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_DYNAMICS2_SIMPLE_CONTACT_CONSTRAINT_H
#define HK_DYNAMICS2_SIMPLE_CONTACT_CONSTRAINT_H

#include <hkmath/basetypes/hkContactPoint.h>
#include <hkconstraintsolver/constraint/contact/hkContactPointProperties.h>
#include <hkconstraintsolver/constraint/contact/hkSimpleContactConstraintInfo.h>
#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>

#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/constraint/contact/hkDynamicsCpIdMgr.h>

#include <hkdynamics/constraint/hkConstraintData.h>
#include <hkdynamics/constraint/atom/hkConstraintAtomUtil.h>

class hkContactPoint;
class hkSolverResults;
class hkRigidBody;



/// A group of simple contacts.
class hkSimpleContactConstraintData: public hkConstraintData
{
	public:

		hkSimpleContactConstraintData();
		hkSimpleContactConstraintData(hkConstraintInstance* constraint);
		inline ~hkSimpleContactConstraintData();

			/// Get number of contact points.
		inline int getNumContactPoints();

			/// Get contactPoint at position i.
		inline const hkContactPointId getContactPointIdAt( int index );

			/// Get the contact point for an id.
		inline hkContactPoint& getContactPoint( hkContactPointId id );


			/// Get the result for the same id.
		inline hkContactPointProperties&	getContactPointProperties( hkContactPointId id );

		hkContactPointId allocateContactPoint( hkConstraintOwner& constraintOwner, hkContactPoint** cpOut, hkContactPointProperties** cpPropsOut);

		void freeContactPoint( hkConstraintOwner& constraintOwner, hkContactPointId id );

		virtual hkBool isValid() const;

		// hkConstraintData interface implementations
		virtual void getConstraintInfo( hkConstraintData::ConstraintInfo& infoOut ) const;

			// hkConstraintData interface implementation
		virtual void getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const;

		virtual hkSolverResults* getSolverResults( hkConstraintRuntime* runtime );

		virtual int getType() const;
	
			/// This function is just called before a toi or a normal collision response.
			/// Those collision response functions are called for every new contact point
			/// where the objects have a colliding velocity > hkWorldCinfo.m_contactRestingVelocity
		virtual void collisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB );

		virtual void collisionResponseEndCallback( const hkContactPoint& cp, hkReal impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);

		inline hkSimpleContactConstraintData( hkFinishLoadedObjectFlag f ) { }

	public:

		hkDynamicsCpIdMgr		m_idMgrA;
		void*					m_clientData;
		void*					m_clientData2;
		hkConstraintInstance*	m_constraint;
		hkSimpleContactConstraintAtom*	m_atom;
};



#include <hkdynamics/constraint/contact/hkSimpleContactConstraintData.inl> 

#endif // HK_DYNAMICS2_SIMPLE_CONTACT_CONSTRAINT_H

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
