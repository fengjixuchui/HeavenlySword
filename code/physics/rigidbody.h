//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/rigidbody.h
//!	
//!	DYNAMICS COMPONENT:
//!		A rigid body class.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.07.11
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_RIGID_BODY_INC
#define _DYNAMICS_RIGID_BODY_INC

#include "config.h"
#include "element.h"

#include "physics/havokincludes.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
//#include <hkbase/memory/hkMemory.h>
#include <hkdynamics/entity/hkRigidBodyCinfo.h>
#include <hkdynamics/constraint/breakable/hkBreakableListener.h>
#include <hkdynamics/collide/hkCollisionListener.h>
#endif
#include "collisionbitfield.h"

#include "behavior.h"

#include "logicgroup.h"
// Forward decl.
class hkRigidBody;
class Transform;
class CEntity;

namespace Physics
{
	class System;
	class Body;

	// -----------------------------------------------------------------------------------
	//	BodyCInfo.
	//	
	//		This structure is used to create the Rigid Body.
	//
	// -----------------------------------------------------------------------------------
	class BodyCInfo
	{
	public:
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkRigidBodyCinfo				m_rigidBodyCinfo;	//!< Structure used by Havok to create the hkRigidBody.
#endif
		Transform*						m_transform;		//!< Transform associated to this body.
		const char *							m_name;				//!< Name of the body. Surprisingly, it's not part of the hkRigidBodyCinfo structure !
		Physics::FilterExceptionFlag	m_exceptionFlag;	//!< Collision bit field for the exception flag.	
		float							m_allowedPenetrationDepth;

		// Constructor / Destructor
		BodyCInfo( );						
		~BodyCInfo( );						
	};

	// -----------------------------------------------------------------------------------
	//	HasCollision.
	//	
	//		Simple collision listener. IsColliding() returns true if entity was colliding the last frame.
	//      Call Update() each frame
	//
	// -----------------------------------------------------------------------------------
	class HasCollision : public hkCollisionListener
	{
	public:
		virtual void contactPointAddedCallback(	hkContactPointAddedEvent& event) {};
		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event) {};
		virtual void contactPointRemovedCallback( hkContactPointRemovedEvent& event ) {};
		virtual void contactProcessCallback( hkContactProcessEvent& event ) {m_contact = true;};

		void Update() {m_contact = false;};
		bool IsColliding() {return m_contact;};
		HasCollision() : m_contact(false) {};

	protected:
		bool m_contact; 		
	};

	
	// -----------------------------------------------------------------------------------
	//	KeyframedMotionParams.
	//	
	//		Struct used during keyframed motion to store last transform and velocities. 
	//
	// The synchronization between animation and body is not perfect. That's why 
	// position and velocities of the havoc body is different that rendered one. 
	// We will remember velocities from last two frames from animation. They can be used f.e. 
	// during switching keyframed -> dynamic motion, to be sure that body continues
	// smoothly in movement.
	// -----------------------------------------------------------------------------------
	struct KeyframedMotionParams
	{
		hkVector4         m_lastTranslation;  //!< Position of body in last frame.
		hkQuaternion	  m_lastRotation; //!< Rotation of body in last frame. 

		hkVector4      m_lastAnimLinearVelocity; 
		hkVector4      m_lastAnimAngularVelocity;
		hkVector4      m_forLastAnimLinearVelocity; 
		hkVector4      m_forLastAnimAngularVelocity;

		KeyframedMotionParams() :
			m_lastTranslation(0,0,0),
			m_lastRotation(0,0,0,1),
			m_lastAnimLinearVelocity(0,0,0),
			m_lastAnimAngularVelocity(0,0,0),
			m_forLastAnimLinearVelocity(0,0,0),
			m_forLastAnimAngularVelocity(0,0,0)
		{};

		void SetLastTransform(const CMatrix& trans);
		CMatrix GetLastTransform() const;
	};

	// -----------------------------------------------------------------------------------
	//	Rigid Body Element.
	//	
	//		Rigid bodies are build with a list of shapes ( 1..N ) and a
	//		set of parameters ( mass, restitution, inertia... )
	//
	// -----------------------------------------------------------------------------------
	class RigidBody : public Element
	{
	public:
		bool m_bDestroyHavokObject;
		
		static RigidBody* ConstructRigidBody( CEntity* p_entity, const BodyCInfo* p_info );		//!< Building the rigid body.
		static RigidBody* ConstructRigidBody( hkRigidBody* body, CEntity* p_entity,  Transform * transform);	//!< Building the rigid body.


		// Generic Interface
		virtual const Element::ELEMENT_TYPE	GetType( ) const;											//!< Pseudo RTTI
		virtual const ntstd::String			GetName( ) const;											//!< Instance are named.
		virtual void						Update( float p_timestep );									//!< Update the instance.
		virtual void						Activate( bool activateInHavok  = false);												//!< An element can be activated into the physics world...
		virtual void						ActivateForced( );
		virtual void						Deactivate( );												//!< ... or deactivated alltogether
		virtual void						AddedToSystem( System* );
		virtual void						RemovedFromSystem( System* );

		// Accessors
		// [Mus] - 2005.07.28 - As long as I can find a way around it, I wont allow external access to the rigid body!
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkRigidBody*						GetHkRigidBody( ) const;										//!< Havok Access
#endif
		Transform*							GetTransform( ) const;										//!< Transform Access
		const CEntity*						GetEntity( ) const;											//!< Entity Access

		void								SetMotionType(EMotionType eMotionType );			//!< Set the motion type of this rigid body
		EMotionType							GetMotionType() const {return m_eMotionType;};
		void								SetBreakableKeyframedMotion(bool on);
		bool								IsInBreakableKeyframedMotion() const {return m_pobBreakableKeyframed;};
		void								SetLastAnimVelocities();

		// Specific stuff. Used by the behaviors or by external access.
		CVector								GetLinearVelocity( ) const;									//!< Return the rigid body linear velocity
		void								SetLinearVelocity( const CVector& p_linearVelocity );		//!< Set the rigid body linear velocity
		CVector								GetAngularVelocity( ) const;								//!< Return the rigid body linear velocity
		void								SetAngularVelocity( const CVector& p_linearVelocity );		//!< Set the rigid body linear velocity
		void								SetHavokFlag( PhysicsProperty p_property, int p_index );	//!< Flag the rigid body.
		void								UpdateCollisionFilter( );									//!< Update the body collision filter.

		void                                SetCollisionFilterInfo(uint32_t info);						//!< Change the rigid body collision filter info. It NOT updates collision filter.
		uint32_t							GetCollisionFilterInfo() const;                             //!< Return the rigid body collisoin filter info.
		void								AddCollisionListener(hkCollisionListener * cl);				//!< Added collision listener to bodies in group
		void								RemoveCollisionListener(hkCollisionListener * cl);			//!< Remove collision listener from bodies in group

		void								ApplyLinearImpulse( const CDirection& obForce );
		void								ApplyAngularImpulse( const CDirection& obForce );
		void								ApplyLocalisedLinearImpulse( const CDirection& p_vel, const CVector& p_point );

		//bool								IsInPenetration() const; //<! Check if body in position defined by its hierarchy transformation is in penetration
		bool								MoveToSafe(const CPoint& endPos);  //<! move body to position if it not collide on its path
		bool						        SetRotationSafe(const CQuat& orient); //<! Set rotation if in new position body is not in collision
		void                                SetHardKeyframing(bool on) {m_bHardKeyframing = on;}; //<! If elements of the group will be keyframed they will always much the trasformation

		void								PausePresenceInHavokWorld(bool pause);
	
		// Destructor
		virtual ~RigidBody( );

		void Debug_RenderCollisionInfo();

	private:
		void	Construct( const BodyCInfo* p_info );												//!< Construct the body.		
		void	SyncBodyOnTransform( const float p_timeStep );										//!< Synchronise the rigid body position with the Transform.
		void	SyncTransformOnBody( );																//!< Synchronise the transform position with the body.
		void	LimitVelocities( hkVector4& newRigidPosition, hkQuaternion& newRigidRotation );		//!< Enforce some limits on the body velocity.
		void	ForceBodyOnTransform( );															//!< Force the rigid body position with the Transform.
		void	AddToWorld( );		//!< Add the body to the world.
		void	AddToWorldForced();
		void	RemoveFromWorld( );																	//!< Remove the body to the world.
		void	AttachToEntity( const CEntity* p_entity );											//!< Attach the body to an entity.		
		void    constraintBrokenCallback(hkBreakableConstraintEvent &);
		static void SetMotionTypeAfterConstruct(RigidBody * body);                                  //!< Set m_eMotionType according to motion type in rigid body

		RigidBody();				//!< Do not use the default constructor.
				
	

		KeyframedMotionParams* m_pobKeyframed; // parameters used during keyframed motion 

		bool			m_firstActivation;	//!< Is the body added for the first time ?

		// Breakable keyframed motion parameters.
		struct BreakableKeyframedMotionParams
		{						
			HasCollision          m_isInContact;             			
			float                 m_maxAngularVelocityBackUp; //!< original angular velocity limit of m_rigidBody
			float                 m_maxLinearVelocityBackUp; //!< original linear velocity limit of m_rigidBody

			BreakableKeyframedMotionParams() : 			
			m_isInContact(),
			m_maxAngularVelocityBackUp(200),
			m_maxLinearVelocityBackUp(200)
			{};
		};

		BreakableKeyframedMotionParams* m_pobBreakableKeyframed; //!< if this pointer is nonzero body is in breakable keyframed motion. 
		
		hkRigidBody*	m_rigidBody;		//!< Havok Rigid Body	
		Transform*		m_transform;		//!< Associated transform
		CEntity*		m_entity;	//!< Pointer to the entity.	
		EMotionType     m_eMotionType; //<! Motion type. 

		bool			m_bHardKeyframing; //!< if body is "hard" keyframed, it allways have identical position with animation		 

		// Store the state of body before pausing presence in Havok world
		enum StateBeforePause
		{
			Undefined =0,
			NotInWorld,
			Deactivated,
			Activated
		};

		StateBeforePause				m_stateBeforePause;
	};

} // Physics
 
#endif // _DYNAMICS_RIGID_BODY_INC
