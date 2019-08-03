/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_CONSTRAINT_H
#define HK_DYNAMICS2_CONSTRAINT_H

#include <hkdynamics/constraint/hkConstraintData.h>

#define HK_STORE_CONSTRAINT_INFO_IN_INTERNAL
class hkConstraintData;
class hkEntity;
class hkRigidBody;
class hkConstraintOwner;
class hkSimulationIsland;
extern const hkClass hkConstraintInstanceClass;

	/// An opaque piece of memory, where constraints can store instance related data
	/// Note: the constraint runtime is not 16 byte aligned
typedef void hkConstraintRuntime;


/// The base class for constraints.
class hkConstraintInstance : public hkReferencedObject
{

	public:
		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CONSTRAINT );

			/// Specifies the priority for each constraint.
			/// Values must be corresponding to hkCollisionDispatcher::CollisionQualityLevel
		enum ConstraintPriority
		{
				/// Invalid priority.
			PRIORITY_INVALID,
				/// Constraint is only solved at regular physics time steps (==PSIs)
			PRIORITY_PSI, 
				/// Constraint is also touched at time of impact events.(TOI).
			PRIORITY_TOI,

				/// For internal use only! Higher quality. Warning: Use this quality only for contact constraints between moving and fixed objects.
				/// Info: Higher priority constraints have higher priority in the solver in PSI steps. They are actually processed as normal PRIORITY_TOI
				///       constraints in TOI events.
			PRIORITY_TOI_HIGHER,
				/// For internal use only! Enforced quality. Warning: Use this quality only for contact constraints between critical and fixed objects.
				/// Info: Forced priority constraints have higher priority in the solver in PSI steps. They have also higher priority in the solver in TOI steps.
				///       Additionally extra CPU time is used at the end of TOI event in attempt to enforce those constraints (with the cost of ignoring the state
				///       of other -- non-forced -- constraints).
			PRIORITY_TOI_FORCED,
		};

			/// Constraint instance's type is queried by the engine to check whether the constraint is a 
			/// 'normal' constraint connecting two entities, or whether it's of a different kind requiring 
			/// special handling. (This is used e.g. when adding/removing constraints to the world.)
		enum InstanceType
		{
				// Standard constraint linking two bodies
			TYPE_NORMAL,
				// Chain of constraints. See hkConstraintChainInstance
			TYPE_CHAIN
		};

		enum AddReferences
		{
			DO_NOT_ADD_REFERENCES,
			DO_ADD_REFERENCES
		};
			//
			// Construction, destruction and cloning.
			//

			/// Construct a constraint instance between A and B, with the data
			/// for the constraint provided. The meaning of the Priority is given in
			/// the enum definition.EntityA can not be NULL, EntityB is allowed to be
			/// NULL and it will be replaced by the world fixed body upon addition to the 
			/// world. 
		hkConstraintInstance(hkEntity* entityA, hkEntity* entityB, hkConstraintData* data, ConstraintPriority priority = PRIORITY_PSI);

			/// Destructor removes references from entities A and B if set.
		~hkConstraintInstance();

			/// Clone the constraint, sharing as much as possible (ie: the constraint data
			/// if it can).
		hkConstraintInstance* clone(hkEntity* newEntityA, hkEntity* newEntityB) const;


	protected:
			/// Internal constructor used by hkSimpleConstraintContactMgr only 
		hkConstraintInstance(ConstraintPriority priority);
		
	public:

		//
		// Accessors
		//

			/// Gets the first constrained entity.
		inline hkEntity* getEntityA() const;

			/// Gets the second constrained entity.
		inline hkEntity* getEntityB() const;

			/// Gets the first constrained body.
		inline hkRigidBody* getRigidBodyA() const;

			/// Gets the second constrained body.
		inline hkRigidBody* getRigidBodyB() const;


			/// Gets either entity A or B which is not entity
		inline hkEntity* getOtherEntity( const hkEntity* entity );

			/// Gets the non fixed island of either entityA or entityB
		hkSimulationIsland* getSimulationIsland();

			/// Get the hkConstraintData object
		inline hkConstraintData* getData() const;

		//
		// Runtime cache data for the constraint.
		//

			/// Get the runtime data.
			/// This can only be called after the constraint has been added to the world.
			/// Note: The runtime data is not 16 byte aligned.
			///
			/// If setWantRuntime has been set to false, this may return HK_NULL.
			/// You can use this data to:
			/// - access the solver results directly
			/// - or to go the specific constraint runtime implementations and ask for the Runtime struct, e.g. hkBallSocketConstraint::getRuntime()
			///
			/// You can get more information about this runtime if you go to the hkConstraintData
			/// and call hkConstraintData::getRuntimeInfo(...)
		inline hkConstraintRuntime* getRuntime() const;

			/// Request that a hkConstraintRuntime is allocated when you add the constraint to the hkWorld.
			/// This must be called before the constraint is added to the world.
			///
			/// If you call setWantRuntime( false ), a runtime might not be allocated for this constraint.
			/// Note:
			///  - Not all constraints have a (external) runtime information.
			///  - Some constraints always allocate a runtime, even if the wantRuntime flag is set to false.
		inline void setWantRuntime( hkBool b );

			/// Get the wantRuntime Flag, see setWantRuntime for details
		inline hkBool getWantRuntime() const ;

		//
		// User data.
		//

			/// Get the user data for the constraint (initialized to 0).
		inline hkUint32 getUserData() const;

			/// Set the user data of the constraint.
		inline void setUserData( hkUint32 data );

		//
		// Priority
		//

			/// Get the priority that this constraint was created with.
		inline ConstraintPriority getPriority() const;

			/// Set the priority for this constraint.
		void setPriority( ConstraintPriority priority );


			/// Get the name of this constraint.
		inline const char* getName() const;

			/// Set the name of this constraint.
			/// IMPORTANT: This data will not be cleaned up by the hkConstraintInstance destructor. You are required to track it yourself.
		inline void setName( const char* name );



	public:

		//
		// Internal functions
		//

		inline hkConstraintOwner* getOwner() const;

			// Gets the master entity, can only be called if the constraint is added to the world
		inline hkEntity* getMasterEntity() const;

			// Gets the slave entity, can only be called if the constraint is added to the world
		inline hkEntity* getSlaveEntity() const;

			// hkEntityListener interface implementation
		virtual void entityAddedCallback(hkEntity* entity);

			// hkEntityListener interface implementation
		virtual void entityRemovedCallback(hkEntity* entity);

			// The constraint should never receive this callback, as the constraint keeps a reference to any entities it operates on.
			// The implementation asserts if called.
		virtual void entityDeletedCallback( hkEntity* entity );

		inline void setOwner( hkConstraintOwner* island );

		inline struct hkConstraintInternal* getInternal();

		void pointNullsToFixedRigidBody();

		virtual InstanceType getType() const { return TYPE_NORMAL; }

	public:

		//
		//	Members
		//
	protected:
		friend class hkSimpleContactConstraintData;

		class hkConstraintOwner* m_owner; //+nosave

		class hkConstraintData*  m_data;

		struct hkModifierConstraintAtom* m_constraintModifiers;

		class hkEntity* m_entities[2];

	public:
		hkEnum<ConstraintPriority,hkUint8> m_priority;

	protected:
			/// Set this to true, if you want to get access to RuntimeData later
		hkBool				m_wantRuntime;

		const char* m_name;

	public:

		hkUint32 m_userData;

	public:

		struct hkConstraintInternal* m_internal; //+nosave

	private:

		friend class hkWorldConstraintUtil;
		friend class hkSimpleConstraintContactMgr;

	public:

			/// The fastconstruction is used by the serialization to
			/// init the vtables and do any extra init (but ONLY of non-serialized members).
		hkConstraintInstance(hkFinishLoadedObjectFlag f) { }

	private:
		/// Default constructor, you cannot derive from this class, you should derive from hkConstraintData
		hkConstraintInstance();
};


struct hkConstraintInternal
{
	//
	//	Public functions
	//
		// Gets the other entity. Not the one specified as the parameter, but the other of the two.
	inline hkEntity* getOtherEntity( const hkEntity* entity ) const;
	inline hkEntity* getMasterEntity() const;
	inline hkEntity* getSlaveEntity() const;

	inline struct hkConstraintAtom* getAtoms() const { return m_atoms; }
	inline int getAtomsSize() const { return m_atomsSize; }

	//
	//	Members
	//
	public:

		class hkConstraintInstance* m_constraint;

			/// The two entities. Attention: the master entity might be entity 0 or 1
		class hkEntity*	  m_entities[2];

	protected:
		friend class hkWorldConstraintUtil;

			/// pointer to the low level constraint information
		struct hkConstraintAtom* m_atoms;

			/// Total size of all constraint information
		hkUint16 m_atomsSize;
	public:

		hkEnum<hkConstraintInstance::ConstraintPriority,hkUint8> m_priority;

		//
		//	internal data
		//

			//
			// data controlled by the constraint owner
			//
		inline void clearConstraintInfo( );
#ifdef HK_STORE_CONSTRAINT_INFO_IN_INTERNAL
		hkUint16 m_sizeOfJacobians;
		hkUint16 m_sizeOfSchemas;
		hkUint16 m_numSolverResults;

		inline void getConstraintInfo( hkConstraintInfo& info ) const;
		inline void addConstraintInfo( const hkConstraintInfo& delta );
		inline void subConstraintInfo( const hkConstraintInfo& delta );
#endif
			//
			// data controlled by hkWorldConstraintUtil
			//

			// Index of the master entity in m_entities.
		hkUint8	m_whoIsMaster;

		hkUint8				m_callbackRequest;

			// Index of the pointer to this hkConsraintInstance stored on the m_constraintsSlave list of the slave entity.
		hkObjectIndex		m_slaveIndex;

		hkUint16			m_runtimeSize;
		hkConstraintRuntime*	m_runtime;
};

#include <hkdynamics/constraint/hkConstraintInstance.inl>

#endif // HK_DYNAMICS2_CONSTRAINT_H




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
