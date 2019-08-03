/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_PULLEY_CONSTRAINT_H
#define HK_DYNAMICS2_PULLEY_CONSTRAINT_H

#include <hkdynamics/constraint/hkConstraintData.h>
#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>
#include <hkconstraintsolver/solve/hkSolverResults.h>

//extern const hkClass hkPulleyConstraintDataClass;

/// A stiff spring constraint. It holds the constrained bodies apart at a specified distance, 
/// as if they were attached at each end of an invisible rod.
class hkPulleyConstraintData : public hkConstraintData
{
	public:

		HK_DECLARE_REFLECTION();
			
		hkPulleyConstraintData();

			/// Sets the pulley up with world space information. 
			/// Will compute a rest length too (so call setlength after this if needed)
		inline void setInWorldSpace(const hkTransform& bodyATransform, const hkTransform& bodyBTransform, 
								const hkVector4& pivotAW, const hkVector4& pivotBW,
								const hkVector4& pulleyPivotAW, const hkVector4& pulleyPivotBW, hkReal leverageOnBodyB );

			/// Sets the pulley up with body space information. 	
			/// Will compute a rest length too (so call setlength after this if needed)
			/// and that is why the transforms are given (so that the length can be computed in world space)
		inline void setInBodySpace(const hkTransform& bodyATransform, const hkTransform& bodyBTransform,
								const hkVector4& pivotA, const hkVector4& pivotB,
								const hkVector4& pulleyPivotAW, const hkVector4& pulleyPivotBW, hkReal leverageOnBodyB );

			/// Gets the length of the rope. Full length == length of rope from bodyA to pulleyPivotA + leverageRation * (length of rope from body B to pulleyPivotB)
		inline hkReal getRopeLength(hkReal length); 
			/// Gets the leverage ratio of the pulley. Pulley exerts 'leverageRatio' times greater forces on bodyB.
		inline hkReal getLeverageOnBodyB();

			/// Sets the length of the rope. Full length == length of rope from bodyA to pulleyPivotA + leverageRation * (length of rope from body B to pulleyPivotB)
		inline void setRopeLength(hkReal length);
			/// Sets the leverage ratio of the pulley. Pulley exerts 'leverageRatio' times greater forces on bodyB.
		inline void setLeverageOnBodyB(hkReal leverageOnBodyB);

			/// Check consistency of constraint
		hkBool isValid() const;

			/// Get type from this constraint	
 		virtual int getType() const;

		
	public:

		enum 
		{
			SOLVER_RESULT_LIN_0 = 0,		// linear constraint 
			SOLVER_RESULT_MAX = 1
		};

		struct Runtime
		{
			class hkSolverResults m_solverResults[1/*VC6 doesn't like the scoping for SOLVER_RESULT_MAX*/];
		};

		inline const Runtime* getRuntime( hkConstraintRuntime* runtime ){ return reinterpret_cast<Runtime*>(runtime); }

		struct Atoms
		{
			HK_DECLARE_REFLECTION();

			struct hkSetLocalTranslationsConstraintAtom    m_translations;
			struct hkPulleyConstraintAtom                  m_pulley;

			Atoms(){}

			// get a pointer to the first atom
			const hkConstraintAtom* getAtoms() const { return &m_translations; }

			// get the size of all atoms (we can't use sizeof(*this) because of align16 padding)
			int getSizeOfAllAtoms() const               { return hkGetByteOffsetInt(this, &m_pulley+1); }

			Atoms(hkFinishLoadedObjectFlag f) : m_translations(f), m_pulley(f) {}
		};

		HK_ALIGN16( struct Atoms m_atoms );

	public:

		// hkConstraintData interface implementations
		virtual void getConstraintInfo( hkConstraintData::ConstraintInfo& infoOut ) const;

		// hkConstraintData interface implementations
		virtual void getRuntimeInfo( hkBool wantRuntime, hkConstraintData::RuntimeInfo& infoOut ) const;
	
	public:

		hkPulleyConstraintData(hkFinishLoadedObjectFlag f) : hkConstraintData(f), m_atoms(f) {}
};


///////////////////////////////////////////////////////////////

#include <hkdynamics/constraint/pulley/hkPulleyConstraintData.inl>

#endif // HK_DYNAMICS2_PULLEY_CONSTRAINT_H

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
