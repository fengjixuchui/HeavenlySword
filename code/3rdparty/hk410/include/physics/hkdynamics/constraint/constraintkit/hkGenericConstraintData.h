/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_GENERIC_CONSTRAINT_H
#define HK_DYNAMICS2_GENERIC_CONSTRAINT_H

#include <hkdynamics/constraint/hkConstraintData.h>
#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>
#include <hkdynamics/constraint/constraintkit/hkGenericConstraintScheme.h>

class hkGenericConstraintDataParameters;

extern const hkClass hkGenericConstraintDataClass;

/// A generic constraint for use with the hkConstraintConstructionKit. A generic constraint
/// initially doesn't restrict any movement for its bodies - it must be configured using the kit.
class hkGenericConstraintData : public hkConstraintData
{
	public:

		HK_DECLARE_REFLECTION();

			/// A parameter index. The constraint construction kit returns an index each time you specify a pivot
			/// point, basis, or axis for the constraint, allowing you to access these later.
		typedef int hkParameterIndex;

			/// Creates a new generic constraint.
		hkGenericConstraintData();

		virtual ~hkGenericConstraintData();


			/// Gets the parameter at the index returned during kit construction
		hkVector4* getParameters( hkParameterIndex parameterIndex );

			/// Sets the parameters starting at the index returned during kit construction
		void setParameters( hkParameterIndex parameterIndex, int numParameters, const hkVector4* newValues ); 

			/// Checks consistency of constraint members.
		virtual hkBool isValid() const;

		hkGenericConstraintDataScheme* getScheme();

	public:

		struct hkBridgeAtoms m_atoms;

	protected:

		// commands 

		// linear constraints

		void constrainAllLinearW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void constrainLinearW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void constrainToAngularW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void constrainAllAngularW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		// limits, friction, motors

		inline void setLinearLimitW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void setAngularLimitW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void setConeLimitW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void setTwistLimitW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, const hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void setAngularMotorW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void setLinearMotorW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;

		inline void setAngularFrictionW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;
			
		void setLinearFrictionW( hkArray<int>::iterator& currentCommand, hkArray<hkVector4>::iterator& currentData, hkGenericConstraintDataScheme& scheme, hkGenericConstraintDataParameters& vars, const hkConstraintQueryIn &in, hkConstraintQueryOut &out ) const;
		
		/// end commands

		void hatchScheme( hkGenericConstraintDataScheme* scheme, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

		class hkGenericConstraintDataScheme m_scheme;

	public:
		// Internal functions

		virtual void buildJacobian( const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

		virtual void getConstraintInfo( hkConstraintData::ConstraintInfo& info ) const;

		virtual int getType() const;

		virtual void getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const;

	public:

		hkGenericConstraintData(hkFinishLoadedObjectFlag f);

};

#endif

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
