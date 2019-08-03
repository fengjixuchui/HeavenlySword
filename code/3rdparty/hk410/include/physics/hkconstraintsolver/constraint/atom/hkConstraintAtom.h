/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_CONSTRAINT_ATOM_H
#define HK_DYNAMICS2_CONSTRAINT_ATOM_H


#include <hkmath/hkMath.h>
#include <hkconstraintsolver/constraint/contact/hkSimpleContactConstraintInfo.h>
#include <hkconstraintsolver/constraint/contact/hkContactPointProperties.h>
#include <hkmath/basetypes/hkContactPoint.h>
#include <hkdynamics/constraint/hkConstraintData.h>

#define HK_DECLARE_NO_REFLECTION()

class hkContactPoint;
class hkConstraintMotor;


#define HK_CONSTRAINT_FILL_PADDING_WITH_ZERO(fromAfter, to) { for (hkUint8* ptr = reinterpret_cast<hkUint8*>(fromAfter.next()); ptr < reinterpret_cast<hkUint8*>(&to); *(ptr++) = 0) { } }


  /// Constraint atoms are building blocks that specify hkConstraintDatas. 
  ///
  /// hkConstraintDatas either use hkBridgeAtoms, which allow them to implement their custom logic, or use a set of generic hkConstraintAtoms
  /// to describe the geometry of the constraint.
  ///
  /// Each hkConstraintAtom is used to specify a constraint's orientation in space or to create one or more solver-constraints of a given type. 
  /// During simulation, hkConstraintAtoms are processed in the order in which they're organized in a hkConstraintData. 
  /// 
  /// Generally the first constraint in a list is one that specifies the local bases of the constraint in each of the constrained bodies' spaces.
  /// Those bases are persistent throughout processing of a list of atoms. The following atoms apply a kind of a constraint (linear, angular, 
  /// limit, motor, etc.) in relation to one or more of the axes of the specified local bases. See individual descriptions of atoms for more info.
  ///
  ///
struct hkConstraintAtom
{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkConstraintAtom );
		HK_DECLARE_NO_REFLECTION();

	public:
		enum AtomType
		{
			TYPE_INVALID = 0,

			TYPE_BRIDGE, 

			TYPE_SET_LOCAL_TRANSFORMS,
			TYPE_SET_LOCAL_TRANSLATIONS,
			TYPE_SET_LOCAL_ROTATIONS,

			TYPE_BALL_SOCKET,
			TYPE_STIFF_SPRING,

			TYPE_LIN,
			TYPE_LIN_SOFT,
			TYPE_LIN_LIMIT,
			TYPE_LIN_FRICTION, 
			TYPE_LIN_MOTOR,

			TYPE_2D_ANG,

			TYPE_ANG,
			TYPE_ANG_LIMIT,
			TYPE_TWIST_LIMIT,
			TYPE_CONE_LIMIT,
			TYPE_ANG_FRICTION,
			TYPE_ANG_MOTOR,

			TYPE_RAGDOLL_MOTOR,

			TYPE_PULLEY,
			TYPE_OVERWRITE_PIVOT,

			TYPE_CONTACT,

				//
				// modifiers, must be the end of the list
				//
			TYPE_MODIFIER_SOFT_CONTACT,
			TYPE_MODIFIER_MASS_CHANGER,
			TYPE_MODIFIER_VISCOUS_SURFACE,
			TYPE_MODIFIER_MOVING_SURFACE,

			TYPE_MAX
		};

		// flag indicating whether this constraint needs some special callback treatment
		// those flags can be combined
		enum CallbackRequest
		{
			CALLBACK_REQUEST_NONE = 0,
			CALLBACK_REQUEST_CONTACT_POINT = 1,
			CALLBACK_REQUEST_SETUP_PPU_ONLY = 2,
		};

	public:
		HK_FORCE_INLINE enum AtomType getType() const { return m_type; }

		HK_FORCE_INLINE int isModifierType() const { return m_type >= TYPE_MODIFIER_SOFT_CONTACT; }

	protected:	
		hkConstraintAtom(enum AtomType type) : m_type(type) {}

	private:
			// Illegal constructor
		hkConstraintAtom();

	public:
		hkEnum<AtomType,hkUint16> m_type;

		hkConstraintAtom(hkFinishLoadedObjectFlag f) {}
};


typedef void (HK_CALL *hkConstraintAtomBuildJacobianFunc) ( class hkConstraintData* m_constraintData, const hkConstraintQueryIn &in, hkConstraintQueryOut &out );

	/// This atom is used to allow to call the old hkConstraintData classes
struct hkBridgeConstraintAtom: public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkBridgeConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkBridgeConstraintAtom(  ): hkConstraintAtom( TYPE_BRIDGE ){ }

		// call this to do stuff not done in the constructor yet
	void init (class hkConstraintData* m_constraintData);

	HK_FORCE_INLINE hkConstraintAtom* next()		{ return (this+1); }

		// bridge atoms are always the last atom, so no need to increment solver result, just make sure the
		// program crashes if the result of this function is used
	HK_FORCE_INLINE int numSolverResults() const    { return 100000; }

	// addToConstraintInfo not needed

	hkConstraintAtomBuildJacobianFunc       m_buildJacobianFunc;	//+nosave +overridetype(void*)

	class hkConstraintData* m_constraintData;

	hkBridgeConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) { init(m_constraintData); }
};

struct hkBridgeAtoms
{
	HK_DECLARE_NO_REFLECTION();
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkBridgeAtoms ); 

	struct hkBridgeConstraintAtom m_bridgeAtom;

	hkBridgeAtoms(){}

	// get a pointer to the first atom
	const hkConstraintAtom* getAtoms() const { return &m_bridgeAtom; }

	// get the size of all atoms (we can't use sizeof(*this) because of align16 padding)
	int getSizeOfAllAtoms() const               { return hkGetByteOffsetInt(this, &m_bridgeAtom+1); }

	hkBridgeAtoms(hkFinishLoadedObjectFlag f) : m_bridgeAtom(f) {}
};

	/// hkSimpleContactConstraintAtom holds contact information for a single hkSimpleContactConstraintData. 
	/// 
	/// It is for internal use only and is unique in the following ways:
	///  - it is not a member of the owning hkConstraintData, it is allocated externally
	///  - its size is dynamic and varies depending on the number of contact points in the constraint
	///  - it is a stand-alone constraint, therefore it derives from hkConstraintAtom and cannot be followed by any other atom
struct hkSimpleContactConstraintAtom : public hkConstraintAtom
{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkSimpleContactConstraintAtom );

	public:
		// Size of hkSimpleContactConstraintAtom is dynamically changed by the engine. It holds up to 256 contact points.
		// We initialize the size of the atom to what it is when no contact points are present.
		hkSimpleContactConstraintAtom() : hkConstraintAtom(TYPE_CONTACT) {}

		HK_FORCE_INLINE hkContactPoint* getContactPoints() const { return reinterpret_cast<hkContactPoint*>( HK_NEXT_MULTIPLE_OF(16, hkUlong( this+1 )) ); }
		HK_FORCE_INLINE hkContactPointProperties* getContactPointProperties() const { return const_cast<hkContactPointProperties*>( reinterpret_cast<const hkContactPointProperties*>( hkAddByteOffsetConst( getContactPoints(), sizeof(hkContactPoint) * m_numReservedContactPoints) ) ); }

	public:
		HK_FORCE_INLINE hkConstraintAtom* next() const { HK_ASSERT2(0x5b5a6955, false, "Not implemented. Need to compute the entire size of contact points & properties."); return HK_NULL; }
		HK_FORCE_INLINE int numSolverResults() const    { return m_numContactPoints+3; }
		
		HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const
		{
			int size = m_numContactPoints;

			infoOut.m_maxSizeOfJacobians  += size * HK_SIZE_OF_JACOBIAN_LAA + ( HK_SIZE_OF_JACOBIAN_LAA * 2 ) + HK_SIZE_OF_JACOBIAN_AA;
			infoOut.m_sizeOfJacobians  += size * HK_SIZE_OF_JACOBIAN_LAA + ( HK_SIZE_OF_JACOBIAN_LAA * 2 );
			infoOut.m_sizeOfSchemas    += size * HK_SIZE_OF_JACOBIAN_SINGLE_CONTACT_SCHEMA + HK_SIZE_OF_JACOBIAN_2D_FRICTION_SCHEMA;
			infoOut.m_numSolverResults += size + 2;
			if ( size >= 2 )
			{
				infoOut.m_sizeOfJacobians += HK_SIZE_OF_JACOBIAN_AA;
				infoOut.m_sizeOfSchemas   +=HK_SIZE_OF_JACOBIAN_3D_FRICTION_SCHEMA - HK_SIZE_OF_JACOBIAN_2D_FRICTION_SCHEMA;
				infoOut.m_numSolverResults += 1;
			}
		}

	public:
		hkUint16 m_sizeOfAllAtoms;
		hkUint16 m_numContactPoints;
		hkUint16 m_numReservedContactPoints;

		hkSimpleContactConstraintDataInfo m_info;

	public:
		hkSimpleContactConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};


	/// Fully eliminates relative linear movement of bodies' pivots.
	///
	/// This is the most common atom. It is advised to place it at the end of the list of atoms to minimize results error.
	/// This atom eliminates 3 degrees of freedom and returns 3 solver results. It has no parameters.
struct hkBallSocketConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkBallSocketConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

public:
	hkBallSocketConstraintAtom() : hkConstraintAtom(TYPE_BALL_SOCKET) {}
		/// Return the next atom after this.
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
		/// This tells how many solver-constraints this atom generates and how may solver-results slots it requires.
	HK_FORCE_INLINE int numSolverResults() const    { return 3; }
		/// This tells how much memory the system will need to store solver schemas and jacobians for this atom.
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( 3 * HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, 3 * HK_SIZE_OF_JACOBIAN_LAA, 3 ); }

	hkBallSocketConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Enforces a constant distance between the pivot points of linked bodies.
struct hkStiffSpringConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkStiffSpringConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

public:
	hkStiffSpringConstraintAtom() : hkConstraintAtom(TYPE_STIFF_SPRING) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA, 1 ); }

		/// The rest length / distance between pivot points.
	hkReal m_length;

	hkStiffSpringConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// This specifies constraint spaces and pivot points in the local spaces of each body.
	///
	/// Pivot points are stored in the translation part of the transforms.
struct hkSetLocalTransformsConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkSetLocalTransformsConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkSetLocalTransformsConstraintAtom() : hkConstraintAtom(TYPE_SET_LOCAL_TRANSFORMS) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 0; }
	// addToConstraintInfo not needed

		/// Constraint orientation and origin/pivot point in bodyA's local space.
	hkTransform m_transformA;
		/// Constraint orientation and origin/pivot point in bodyB's local space.
	hkTransform m_transformB;

	hkSetLocalTransformsConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// This specifies pivot points in the local spaces of each body.
	///
	/// Note that this does not overwrite the constraint space's orientation. 
	/// This is used when constraint orientation is irrelevant, e.g. in hkBallAndSocketConstraintData.
struct hkSetLocalTranslationsConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkSetLocalTranslationsConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkSetLocalTranslationsConstraintAtom() : hkConstraintAtom(TYPE_SET_LOCAL_TRANSLATIONS) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 0; }
	// addToConstraintInfo not needed

		/// Pivot point in bodyA's local space.
	hkVector4 m_translationA;
		/// Pivot point in bodyB's local space.
	hkVector4 m_translationB;

	hkSetLocalTranslationsConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// This specifies constraint spaces in the local spaces of each body.
	///
	/// Note that this does not overwrite the pivot points. 
	/// This is used when the constraint space must be reoriented for some atoms in more complex hkConstraintDatas, e.g. in the hkWheelConstraintData.
struct hkSetLocalRotationsConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkSetLocalRotationsConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkSetLocalRotationsConstraintAtom() : hkConstraintAtom(TYPE_SET_LOCAL_ROTATIONS) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 0; }
	// addToConstraintInfo not needed

		/// Constraint orientation in bodyA's local space.
	hkRotation m_rotationA;
		/// Constraint orientation in bodyB's local space.
	hkRotation m_rotationB;

	hkSetLocalRotationsConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

struct hkOverwritePivotConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkOverwritePivotConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkOverwritePivotConstraintAtom() : hkConstraintAtom(TYPE_OVERWRITE_PIVOT), m_copyToPivotBFromPivotA(true) { }
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 0; }

	hkUint8 m_copyToPivotBFromPivotA;

	hkOverwritePivotConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Eliminates relative linear velocity of bodies' pivot points along one specified axis.
	///
	/// This is used when relative linear movement is only partly constrained as it is in e.g. prismatic or point-to-plane constraints.
struct hkLinConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkLinConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkLinConstraintAtom() : hkConstraintAtom(TYPE_LIN) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA, 1 ); }

		/// Specifies the index of the axis of the bodyB's constraint base, that will be constrained.
	hkUint8 m_axisIndex;

	hkLinConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Softens/controls relative linear velocity of bodies' pivot points along one specified axis.
	///
	/// This results in a spring-like reaction, it's used in the hkWheelConstraintData.
struct hkLinSoftConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkLinSoftConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkLinSoftConstraintAtom() : hkConstraintAtom(TYPE_LIN_SOFT) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_BILATERAL_USER_TAU_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA, 1 ); }

		/// Specifies the index of the axis of the bodyB's constraint base, that will be constrained.
	hkUint8 m_axisIndex;
		/// Specifies a custom value for the tau parameter used by the solver.
	hkReal m_tau;
		/// Specifies a custom value for the damping parameter used by the solver.
	hkReal m_damping;

	hkLinSoftConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Limits allowed relative distance between bodies' pivot points along one specified axis.
	///
	/// This allows unconstrained movement within the specified range, and applies hard limits at its ends.
struct hkLinLimitConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkLinLimitConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkLinLimitConstraintAtom() : hkConstraintAtom(TYPE_LIN_LIMIT) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_LINEAR_LIMIT_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA, 1 ); }

		/// The index of the axis of the bodyB's constraint base, that will be limited.
	hkUint8 m_axisIndex;
		/// Minimum distance along the axis (may be negative).
	hkReal m_min;
		/// Maximum distance along the axis (may be negative).
	hkReal m_max;

	hkLinLimitConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Eliminates two degrees of freedom of angular movement and allows relative rotation along a specified axis only.
	///
	/// Angular-constraint atoms are often combined with linear-constraint atoms, e.g. this atoms combined with the ball-and-socket
	/// atom forms a hkHingeConstraintData.
struct hk2dAngConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hk2dAngConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hk2dAngConstraintAtom() : hkConstraintAtom(TYPE_2D_ANG) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 2; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( 2 * HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, 2 * HK_SIZE_OF_JACOBIAN_AA, 2 ); }

		/// Specifies the index of the unconstrained axis of relative rotation in bodyB's constraint base. 
	hkUint8 m_freeRotationAxis;

	hk2dAngConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};


	/// Eliminates one, two, or three degrees of freedom of angular movement. 
	///
	/// Note: this is only tested for eliminating three degrees of freedom.
struct hkAngConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkAngConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkAngConstraintAtom() : hkConstraintAtom(TYPE_ANG) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return m_numConstrainedAxes; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( m_numConstrainedAxes * HK_SIZE_OF_JACOBIAN_1D_BILATERAL_SCHEMA, m_numConstrainedAxes * HK_SIZE_OF_JACOBIAN_AA, m_numConstrainedAxes ); }

		/// Index of the first axis to constrain, in bodyA's constraint base.
	hkUint8 m_firstConstrainedAxis;

		/// Number of subsequent base axes to constrain.
	hkUint8 m_numConstrainedAxes;

	hkAngConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};


	/// Limits allowed relative angle between bodies' rotations along one specified rotation axis.
	///
	/// This allows unconstrained movement within the specified range, and applies hard limits at its ends.
struct hkAngLimitConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkAngLimitConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkAngLimitConstraintAtom() : hkConstraintAtom(TYPE_ANG_LIMIT), m_isEnabled(true) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_LIMITS_SCHEMA, HK_SIZE_OF_JACOBIAN_AA, 1 ); }

		/// Tells whether the atom should be handled by the solver.
		///
		/// Note that if it is not, the atom's corresponding hkSolverResults are not updated.
	hkUint8 m_isEnabled;

		/// The index of the axis in the bodyA's constraint base, that will be limited.
	hkUint8 m_limitAxis;

		/// Mininum angle value in radians (may be negative). 	
	hkReal m_minAngle;

		/// Maximum angle value in radians (may be negative). 	
	hkReal m_maxAngle;

		/// A stiffness factor [0..1] used by the solver; defaults to 1.0.
	hkReal m_angularLimitsTauFactor; //+default(1.0) +absmin(0) +absmax(1)

	hkAngLimitConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};


	/// Limits allowed relative angle between bodies' rotations along one specified rotation axis. 
	///
	/// This constraint allows unconstrained movement within the specified range, and applies hard limits at its ends.
struct hkTwistLimitConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkTwistLimitConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkTwistLimitConstraintAtom() : hkConstraintAtom(TYPE_TWIST_LIMIT), m_isEnabled(true) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_LIMITS_SCHEMA, HK_SIZE_OF_JACOBIAN_AA, 1 ); }

		/// Tells whether the atom should be handled by the solver.
		///
		/// Note that if it is not, the atom's corresponding hkSolverResults are not updated.
	hkUint8 m_isEnabled;

		/// The index of the axis in the bodyA's constraint base, that will be limited.
	hkUint8 m_twistAxis;

		/// The index of a perpendicular axis used as a reference to measure the angle.
	hkUint8 m_refAxis;

		/// Mininum angle value in radians (may be negative). 	
	hkReal m_minAngle;

		/// Maximum angle value in radians (may be negative). 	
	hkReal m_maxAngle;

		/// A stiffness factor [0..1] used by the solver; defaults to 1.0.
	hkReal m_angularLimitsTauFactor; //+default(1.0) +absmin(0) +absmax(1)

	hkTwistLimitConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Limits allowed relative angle between bodies' rotations as measured between two chosen axes.
	///
	/// This allows unconstrained movement within the specified range, and applies hard limits at its ends.
struct hkConeLimitConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkConeLimitConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkConeLimitConstraintAtom() : hkConstraintAtom(TYPE_CONE_LIMIT), m_isEnabled(true) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_LIMITS_SCHEMA, HK_SIZE_OF_JACOBIAN_AA, 1 ); }

		/// Tells whether the atom should be handled by the solver.
		///
		/// Note that if it is not, the atom's corresponding hkSolverResults are not updated.
	hkUint8 m_isEnabled;

		/// The index of the axis in the bodyA's constraint base, that will be used as a reference vector and constrained to lie within the limit cone.
	hkUint8 m_twistAxisInA;

		/// The index of the axis in the bodyB's constraint base, that will be used as a reference and limit-cone axis.
	hkUint8 m_refAxisInB;

		/// Specifies how the angle between the two reference vectors is measured.
	enum MeasurementMode
	{
		// Do not change enumeration values! They're used in calculations.

			/// Zero-angle corresponds to situation where the two vectors are aligned.
		ZERO_WHEN_VECTORS_ALIGNED = 0,
			/// Zero-angle corresponds to situation where the two vectors are perpendicular, and (+)90-degree corresponds to vectors being aligned.
		ZERO_WHEN_VECTORS_PERPENDICULAR = 1
	};

		/// Specifies how the angle between the two reference vectors is measured.
	hkEnum<enum MeasurementMode, hkUint8> m_angleMeasurementMode;


		/// Mininum angle value in radians (may be negative). 		
	hkReal m_minAngle;

		/// Maximum angle value in radians (may be negative).	
	hkReal m_maxAngle;

		/// A stiffness factor [0..1] used by the solver; defaults to 1.0.
	hkReal m_angularLimitsTauFactor; //+default(1.0) +absmin(0) +absmax(1)

	hkConeLimitConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Applies friction torque along one, two, or three specified rotation axes.
struct hkAngFrictionConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkAngFrictionConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkAngFrictionConstraintAtom() : hkConstraintAtom(TYPE_ANG_FRICTION), m_isEnabled(true), m_numFrictionAxes(1) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return m_numFrictionAxes; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( m_numFrictionAxes * HK_SIZE_OF_JACOBIAN_1D_ANGULAR_FRICTION_SCHEMA, m_numFrictionAxes * HK_SIZE_OF_JACOBIAN_AA, m_numFrictionAxes ); }

		/// Tells whether the atom should be handled by the solver.
		///
		/// Note that if it is not, the atom's corresponding hkSolverResults are not updated.
	hkUint8 m_isEnabled;

		/// Index of the first axis to apply friction along, in bodyA's constraint base.
	hkUint8 m_firstFrictionAxis;

		/// Number of subsequent base axes to constrain.
	hkUint8 m_numFrictionAxes;

		/// Maximum allowed torque to be applied due to friction. 	
	hkReal m_maxFrictionTorque;

	hkAngFrictionConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Controls relative rotation angle between bodies around a specified rotation axes.
	///
	/// Note that motor atoms require access to external variables stored in hkConstraintInstance's runtime.
	/// The atom accesses those variables using memory offsets (stored in the atom's members).
	/// Also when the motor is to operate in a range exceeding the [-Pi, Pi] range it must have a reference
	/// onto solver results of a corresponding hkAngLimitConstraintAtom to retrieve the proper angle value.
struct hkAngMotorConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkAngMotorConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkAngMotorConstraintAtom() : hkConstraintAtom(TYPE_ANG_MOTOR) { m_isEnabled = true; m_initializedOffset = 0; m_previousTargetAngleOffset = 0; }
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_ANGULAR_MOTOR_SCHEMA, HK_SIZE_OF_JACOBIAN_AA, 1 ); }

		/// Tells whether the atom should be handled by the solver.
		///
		/// Note that if it is not, the atom's corresponding hkSolverResults are not updated.
	hkBool m_isEnabled;

		/// The index of the axis in the bodyA's constraint base, that will be controlled.
	hkUint8 m_motorAxis;

		/// Memory offset from atom's solver results to runtime's m_initialized member.
	hkInt16 m_initializedOffset;

		/// Memory offset from atom's solver results to runtime's m_previousTargetAngle member
	hkInt16 m_previousTargetAngleOffset;

		/// This is an optional offset to solver results of an angular limit atom.
		/// The results store the actual angle from the last frame, and are needed if the motor
		/// is to allow 'screw' functionality (ie. orientation is not represented by a cyclic 
		/// [-180deg, 180deg] range, but as an unlimited number of degrees/rotations).
	hkInt16 m_correspondingAngLimitSolverResultOffset;

		/// The target angle for the motor.
	hkReal m_targetAngle;

		/// Motor; note that it is reference counted and should be handled by the owning constraint's get/set methods.
	HK_CPU_PTR(class hkConstraintMotor*) m_motor;

	hkAngMotorConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Controls relative rotation angle between bodies in three dimensiond; used by the hkRagdollConstraintData.
	///
	/// Note that motor atoms require access to external variables stored in hkConstraintInstance's runtime.
	/// The atom accesses those variables using memory offsets (stored in the atom's members).
struct hkRagdollMotorConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkRagdollMotorConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkRagdollMotorConstraintAtom() : hkConstraintAtom(TYPE_RAGDOLL_MOTOR) { m_isEnabled = true; m_initializedOffset = 0; m_previousTargetAnglesOffset = 0; }
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 3; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( 3 * HK_SIZE_OF_JACOBIAN_1D_ANGULAR_MOTOR_SCHEMA, 3 * HK_SIZE_OF_JACOBIAN_AA, 3 ); }

		/// Tells whether the atom should be handled by the solver.
		///
		/// Note that if it is not, the atom's corresponding hkSolverResults are not updated.
	hkBool m_isEnabled;

		/// Memory offset from atom's solver results to runtime's m_initialized member.
	hkInt16 m_initializedOffset;

		/// Memory offset from atom's solver results to runtime's m_previousTargetAngle member.
	hkInt16 m_previousTargetAnglesOffset;

		/// The target frame the motors will try to match.
	hkMatrix3 m_targetFrameAinB;

		/// Three motors; note that they are reference counted and should be handled by the owning constraint's get/set methods.
	HK_CPU_PTR(class hkConstraintMotor*) m_motors[3];

	hkRagdollMotorConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Applies friction force along a specified axes.
struct hkLinFrictionConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkLinFrictionConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkLinFrictionConstraintAtom() : hkConstraintAtom(TYPE_LIN_FRICTION), m_isEnabled(true) {}
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_FRICTION_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA, 1  ); }

		/// Tells whether the atom should be handled by the solver.
		///
		/// Note that if it is not, the atom's corresponding hkSolverResults are not updated.
	hkUint8 m_isEnabled;

		/// Index of the axis to apply friction along, in bodyB's constraint base.
	hkUint8 m_frictionAxis;
		
		/// Maximum allowed force to be applied due to friction. 	
	hkReal m_maxFrictionForce;

	hkLinFrictionConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// Controls relative velocity of bodies along a specified axis.
	///
	/// Note that motor atoms require access to external variables stored in hkConstraintInstance's runtime.
	/// The atom accesses those variables using memory offsets (stored in the atom's members).
struct hkLinMotorConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkLinMotorConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkLinMotorConstraintAtom() : hkConstraintAtom(TYPE_LIN_MOTOR) { m_isEnabled = true; m_initializedOffset = 0; m_previousTargetPositionOffset = 0; }
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_LINEAR_MOTOR_SCHEMA, HK_SIZE_OF_JACOBIAN_LAA, 1  ); }

		/// A flag saying whether the motor is active
	hkBool  m_isEnabled;

		/// The index of the axis in the bodyB's constraint base, that will be controlled.
	hkUint8 m_motorAxis;

		/// Memory offset from atom's solver results to runtime's m_initialized member.
	hkInt16 m_initializedOffset;

		/// Memory offset from atom's solver results to runtime's m_previousTargetPosition member.
	hkInt16 m_previousTargetPositionOffset;

		/// The target position for the motor.
	hkReal m_targetPosition;

		/// Motor; note that it is reference counted and should be handled by the owning constraint's get/set methods.
	HK_CPU_PTR(class hkConstraintMotor*) m_motor;

	hkLinMotorConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	/// This implements a functionality of a pulley, where bodies are attached to a rope, and the rope is lead through two pulley wheels at fixed world positions.
struct hkPulleyConstraintAtom : public hkConstraintAtom
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT, hkPulleyConstraintAtom );
	HK_DECLARE_NO_REFLECTION();

	hkPulleyConstraintAtom() : hkConstraintAtom(TYPE_PULLEY) { }
	HK_FORCE_INLINE hkConstraintAtom* next() const { return const_cast<hkConstraintAtom*>( static_cast<const hkConstraintAtom*>(this+1) ); }
	HK_FORCE_INLINE int numSolverResults() const    { return 1; }
	HK_FORCE_INLINE void addToConstraintInfo(hkConstraintInfo& infoOut) const { infoOut.add( HK_SIZE_OF_JACOBIAN_1D_PULLEY_SCHEMA, HK_SIZE_OF_JACOBIAN_LLAA, 1 ); }


		/// Pulley's first fixed pivot point.
	hkVector4 m_fixedPivotAinWorld;
		/// Pulley's second fixed pivot point.
	hkVector4 m_fixedPivotBinWorld;

		/// The rest length (equal to ((BodyA's rope) + leverageOnBodyB * (BodyB's rope length)) )
	hkReal m_ropeLength;
		/// Leverage ratio: e.g. value of 2 means that bodyA's rope length changes by twice as much as bodyB's, 
		/// and the constraint exerts twice as big forces upon bodyB.
	hkReal m_leverageOnBodyB;

	hkPulleyConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};




	//
	//	Atom Modifiers
	//
struct hkModifierConstraintAtom : public hkConstraintAtom
{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CONSTRAINT, hkModifierConstraintAtom );
		HK_DECLARE_NO_REFLECTION();

	public:
			// adds the constraintInfo of one modifier to cinfo and returns the hkConstraintAtom::CallbackRequest
		int addModifierDataToConstraintInfo( hkConstraintInfo& cinfo ) const;

			// adds its constraintInfo of all linked modifiers to cinfo and returns the hkConstraintAtom::CallbackRequest
		static int HK_CALL addAllModifierDataToConstraintInfo( hkModifierConstraintAtom* firstModifier, hkConstraintInfo& cinfo );

	protected:

		hkModifierConstraintAtom(enum AtomType type, int size) : hkConstraintAtom(type), m_modifierAtomSize( hkUint16(size)) {}

	public:

		hkUint16					  m_modifierAtomSize;
		hkUint16                      m_childSize;
		HK_CPU_PTR(hkConstraintAtom*) m_child;

		hkModifierConstraintAtom(hkFinishLoadedObjectFlag f) : hkConstraintAtom(f) {}
};

	//	************************ Soft Contact **************************
	//	************************ Soft Contact **************************
	//	************************ Soft Contact **************************

struct hkSoftContactModifierConstraintAtom : public hkModifierConstraintAtom
{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CONSTRAINT, hkSoftContactModifierConstraintAtom );
		HK_DECLARE_NO_REFLECTION();

	public:

		hkSoftContactModifierConstraintAtom() : hkModifierConstraintAtom(TYPE_MODIFIER_SOFT_CONTACT, sizeof(hkSoftContactModifierConstraintAtom)),  m_tau(0.1f), m_maxAcceleration( 20.0f) { }

		void collisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);
		void collisionResponseEndCallback(   const hkContactPoint& cp, hkReal impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);
		HK_FORCE_INLINE int numSolverResults() const { return 0; }

		int getConstraintInfo( hkConstraintInfo& info ) const	{		return hkConstraintAtom::CALLBACK_REQUEST_SETUP_PPU_ONLY;		}	

	public:

		hkReal m_tau;

			/// The maximum acceleration, the solver will apply
		hkReal m_maxAcceleration;
		hkUchar m_padding[ 32 - sizeof(hkModifierConstraintAtom) - 2*sizeof(hkReal) ];

		hkSoftContactModifierConstraintAtom(hkFinishLoadedObjectFlag f) : hkModifierConstraintAtom(f) {}
};




	//	************************ Mass Changer **************************
	//	************************ Mass Changer **************************
	//	************************ Mass Changer **************************

struct hkMassChangerModifierConstraintAtom : public hkModifierConstraintAtom
{
		HK_DECLARE_NO_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CONSTRAINT, hkMassChangerModifierConstraintAtom );

	public:

		hkMassChangerModifierConstraintAtom() : hkModifierConstraintAtom(TYPE_MODIFIER_MASS_CHANGER, sizeof(hkMassChangerModifierConstraintAtom)) { }

		HK_FORCE_INLINE int numSolverResults() const { return 0; }

		void collisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);
		void collisionResponseEndCallback(   const hkContactPoint& cp, hkReal impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);

		int getConstraintInfo( hkConstraintInfo& info ) const
		{
			info.m_sizeOfSchemas   += 2 * HK_SIZE_OF_JACOBIAN_SET_MASS_SCHEMA  + HK_SIZE_OF_JACOBIAN_HEADER_SCHEMA;
			info.m_sizeOfJacobians += 4 * hkSizeOf(hkVector4);
			return hkConstraintAtom::CALLBACK_REQUEST_NONE;
		}	

	public:

		hkReal m_factorA;
		hkReal m_factorB;
		hkUchar m_padding[ 32 - sizeof(hkModifierConstraintAtom) - 2*sizeof(hkReal) ];

		hkMassChangerModifierConstraintAtom(hkFinishLoadedObjectFlag f) : hkModifierConstraintAtom(f) {}
};




	//	************************ Viscous Surface **************************
	//	************************ Viscous Surface **************************
	//	************************ Viscous Surface **************************

struct hkViscousSurfaceModifierConstraintAtom : public hkModifierConstraintAtom
{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CONSTRAINT, hkViscousSurfaceModifierConstraintAtom );
		HK_DECLARE_NO_REFLECTION();

	public:

		hkViscousSurfaceModifierConstraintAtom() : hkModifierConstraintAtom(TYPE_MODIFIER_VISCOUS_SURFACE, sizeof(hkViscousSurfaceModifierConstraintAtom)) { }

		int getConstraintInfo( hkConstraintInfo& info ) const	{		return hkConstraintAtom::CALLBACK_REQUEST_SETUP_PPU_ONLY;		}	

		HK_FORCE_INLINE int numSolverResults() const { return 0; }

	public:

		hkUchar m_padding[ 16 - sizeof(hkModifierConstraintAtom) ];

		hkViscousSurfaceModifierConstraintAtom(hkFinishLoadedObjectFlag f) : hkModifierConstraintAtom(f) {}
};




	//	************************ Moving Surface **************************
	//	************************ Moving Surface **************************
	//	************************ Moving Surface **************************

struct hkMovingSurfaceModifierConstraintAtom : public hkModifierConstraintAtom
{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CONSTRAINT, hkMovingSurfaceModifierConstraintAtom );
		HK_DECLARE_NO_REFLECTION();

	public:

		hkMovingSurfaceModifierConstraintAtom() : hkModifierConstraintAtom(TYPE_MODIFIER_MOVING_SURFACE, sizeof(hkMovingSurfaceModifierConstraintAtom)) { }

		HK_FORCE_INLINE int numSolverResults() const { return 0; }

		int getConstraintInfo( hkConstraintInfo& info ) const
		{
			info.m_sizeOfSchemas   += 2 * HK_SIZE_OF_JACOBIAN_ADD_VELOCITY_SCHEMA + HK_SIZE_OF_JACOBIAN_HEADER_SCHEMA;
			info.m_sizeOfJacobians += 2 * sizeof(hkVector4);
			return hkConstraintAtom::CALLBACK_REQUEST_NONE;
		}

		void collisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);
		void collisionResponseEndCallback(   const hkContactPoint& cp, hkReal impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);

		hkVector4& getVelocity() { return m_velocity; }
		const hkVector4& getVelocity() const { return m_velocity; }

	public:

 		hkVector4 m_velocity;

		hkMovingSurfaceModifierConstraintAtom(hkFinishLoadedObjectFlag f) : hkModifierConstraintAtom(f) {}
};





#endif // HK_DYNAMICS2_CONSTRAINT_ATOM_H

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
