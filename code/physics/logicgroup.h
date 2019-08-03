//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/logicgroup.h
//!	
//!	DYNAMICS COMPONENT:
//!		The logic group is a set of element.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_LOGIC_GROUPS_INC
#define _DYNAMICS_LOGIC_GROUPS_INC

#include "config.h"
//#include "xmlinterfaces.h"

#include <hkdynamics/collide/hkCollisionListener.h>
#include "game/entity.h"

/*
	The main issue with LG is that they may implement some very different methods. Some methods may have a meaning to the group, whereas some other may not.
	If behaviors provide an easy way to tweak the main update and achieve some interesting results, I need a way to deal with some generic operations like 
	get/set velicities, oe applying impulses.
	To deal with that, their is the notion of interfaces that an LG may implements or not. 
*/

namespace Physics
{

	enum EMotionType
	{
		HS_MOTION_DYNAMIC		= 0,
		HS_MOTION_KEYFRAMED	= 1,
		HS_MOTION_FIXED		= 2,
		HS_MOTION_BREAKABLE_KEYFRAMED	= 3,
		HS_MOTION_KEYFRAMED_STOPPED = 4, // body is not moving, setting this motion type will help performance
	};

	class Element;
	class Behavior;
	class BodyCInfo;
	class RigidBody;
	class System;
	class PhysicsDataRef;
	class PhysicsData;

		// Simple ref class	
	class PhysicsDataRef
	{
	public:
		PhysicsDataRef() : m_data(0) {};
		PhysicsDataRef(PhysicsData* data);
		PhysicsDataRef(const PhysicsDataRef& ref);
		const PhysicsDataRef& operator=(const PhysicsDataRef& ref);
		const PhysicsDataRef& operator=( PhysicsData* data);		
		~PhysicsDataRef();		

	protected:
		PhysicsData * m_data;
	};


	// ---------------------------------------------------------------
	//	A LogicGroup is mainly a list of Elements. The LogicGroup
	//	provide a way to give some meaning to a set of elements.
	// ---------------------------------------------------------------
	class LogicGroup
	{
	public:

		// Their is various logic groups types.
		// At the moment, they derive from the old dynamics component types.
		enum GROUP_TYPE
		{
			SYSTEM,						// Default group.
			
			// [ Mus ] - 2005.08.01 --> To implement in order to support the old dynamics component.

			STATIC_LG,					// Legacy from the old dynamics component.
			ANIMATED_LG,				// Legacy from the old dynamics component.
			SINGLE_RIGID_BODY_LG,		// Legacy from the old dynamics component.
			COMPOUND_RIGID_LG,			// Legacy from the old dynamics component.
			CHARACTER_CONTROLLER_LG,	// Legacy from the old dynamics component.	
			RAGDOLL_LG,					// Legacy from the old dynamics component.
			SOFT_BODY_LG, 				// Legacy from the old dynamics component.
			SPEAR_LG,					// Legacy from the old dynamics component.
			PROJECTILE_LG,				// Legacy from the old dynamics component.
			ADVANCED_CHARACTER_CONTROLLER,
			ADVANCE_RAGDOLL

		};
		// - Load from file ------------------------------------------------------------
		virtual void						Load()											{};

		// - Metadata	----------------------------------------------------------------
		virtual const GROUP_TYPE			GetType( ) const								= 0;	//!< Pseudo RTTI
		virtual const ntstd::String&		GetName( ) const								   ;	//!< LogicGroups are named.
		virtual CEntity*					GetEntity( ) const								   ;	//!< Entity Access
		
		// - Main update	------------------------------------------------------------
		virtual void						Update( const float p_timestep )				= 0;	//!< Update the Element.
		
		// - World interaction 	--------------------------------------------------------

		/** A logic group can be activated into the physics world. 
		  * But if activateInHavok is false the rigid bodies will be not turn to "active" in Havok...
		  */
		virtual void						Activate( bool activateInHavok = false)					   ;
		virtual void						Deactivate( )									   ;	//!< ... or deactivated alltogether
		virtual bool						IsActive( )										   ;
		virtual void						PausePresenceInHavokWorld(bool add)								   ;		
		virtual void						AddedToSystem( System* )						   ;	//!< Called when the group is added to a system.
		virtual void						RemovedFromSystem( System* )					   ;

		// - Behaviors		------------------------------------------------------------
		virtual void						AddBehavior( Behavior* p_behavior )				   ;	//!< Behaviors can be added or removed at will. 
		virtual void						RemoveBehavior( const ntstd::String& p_behaviorID )  ;	//!< Remove any behavior with that id.
		virtual Behavior*					GetBehavior( const ntstd::String& p_behaviorID )	   ;	//!< Return a behavior tied to this id.

		// - Cosntructors/Dest.		----------------------------------------------------
											LogicGroup( const ntstd::String&, CEntity* )         ;	//!< Constructor.
		virtual 							~LogicGroup( )									   ;	//!< Virtual destructor :-)

		// - Element List management	------------------------------------------------
		virtual RigidBody*					AddRigidBody( const BodyCInfo* p_info )			   ;	//!< Add a rigid body.
		virtual RigidBody*					AddRigidBody( RigidBody* body )					   ;

		// Constraint List management - keep a list of all constraints we add to bodies in this system
		//virtual void AddConstraint(psConstraint* pobConstraint);

		// - Element management	--------------------------------------------------------
		virtual Element*					GetElementByName( const ntstd::String& p_name )	   ;	//!< We should be able to access to instances by name.
		virtual ntstd::List<Physics::Element*>&	GetElementList( )								   ;	//!< Return the elements list.
		//virtual void						ReadFromXML( const char* xmlFile )				= 0;	//!< Read the content of the XML file and add it to the group.	
		virtual void						UpdateCollisionFilter( )						   ;	//!< Update the collision filters.
		virtual void                        SetCollisionFilterInfo(uint32_t info);					//!< Change the collision filter info. It NOT updates collision filter.
		virtual uint32_t					GetCollisionFilterInfo() const;                         //!< Return the collisoin filter info of the first element.
		virtual void						AddCollisionListener(hkCollisionListener * cl);         //!< Added collision listener to bodies in group
		virtual void						RemoveCollisionListener(hkCollisionListener * cl);		//!< Remove collision listener from bodies in group

		virtual void						EntityRootTransformHasChanged();
		virtual void						Pause( bool bPause );
		void                                SetHardKeyframing(bool on);
		virtual void						SetMotionType( EMotionType eMotionType );
		virtual EMotionType					GetMotionType();
		
		// Constraint management
		//virtual psConstraint* GetConstraintByName(const ntstd::String& p_name);

		// - Standard interfaces --------------------------------------------------------
		virtual CDirection					GetLinearVelocity( );
		virtual CDirection					GetAngularVelocity( );
		virtual void						SetLinearVelocity( const CDirection& p_linearVelocity );	
		virtual void						SetAngularVelocity( const CDirection& p_angularVelocity );
		virtual void						ApplyLinearImpulse( const CDirection& obForce );
		virtual void						ApplyLocalisedLinearImpulse( const CDirection& p_vel, const CVector& p_point );
		virtual void						ApplyAngularImpulse( const CDirection& obForce );

		// - Collision debugging --------------------------------------------------------

		virtual void						Debug_RenderCollisionInfo () {}

	protected:

		ntstd::String					m_name;				//!< Logic Group name.
		CEntity*						m_entity;			//!< Associated entity.
		ntstd::List<Behavior*>			m_behaviorList;		//!< List of behaviors.
		ntstd::List<Element*>			m_elementList;		//!< List of elements.
		Physics::PhysicsDataRef			m_loaderDataRef;    //!< Reference to data used to create lg
		//ntstd::List<psConstraint*>		m_obConstraintList;
		bool							m_isActive;
	};

} // Physics

#endif // _DYNAMICS_LOGIC_GROUPS_INC
