//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/element.h
//!	
//!	DYNAMICS COMPONENT:
//!		An element is the basic component of a system.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.07.11
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_ELEMENT_INC
#define _DYNAMICS_ELEMENT_INC

namespace Physics
{
	class Behavior;
	class System;

	// ---------------------------------------------------------------
	//	A physics Element represent any physics entity. This include:
	//		--> Rigid Bodies
	//		--> Character Controllers
	//		--> Joints / Constraints
	//		--> Actions
	//		--> Phantoms
	//		--> SoftBodies
	// ---------------------------------------------------------------
	class Element
	{
	public:

		// Their is various instance types
		enum ELEMENT_TYPE
		{
			RIGID_BODY,
		};

		virtual const ELEMENT_TYPE	GetType( ) const								= 0;	//!< Pseudo RTTI
		virtual const ntstd::String	GetName( ) const								= 0;	//!< Element are named.
		
		virtual void				Update( float p_timestep )						= 0;	//!< Update the Element.
		
		virtual void				Activate( bool activateInHavok = false)				= 0;	//!< An element can be activated into the physics world...
		virtual void				Deactivate( )									= 0;	//!< ... or deactivated alltogether
		virtual void				AddedToSystem( System* )						= 0;
		virtual void				RemovedFromSystem( System* )					= 0;

		virtual void				AddBehavior( Behavior* p_behavior )				   ;	//!< Behaviors can be added or removed at will. 
		virtual void				RemoveBehavior( const ntstd::String& p_behaviorID )  ;	//!< Remove any behavior with that id.
		virtual Behavior*			GetBehavior( const ntstd::String& p_behaviorID )	   ;	//!< Return a behavior tied to this id.

		virtual 					~Element( )										   ;	//!< We always need a virtual destructor :-)

		virtual void Debug_RenderCollisionInfo() = 0;
	protected:

		ntstd::List<Behavior*>			m_behaviorList;		//!< List of behaviors.

	};

} // Physics
 
#endif // _DYNAMICS_ELEMENT_INC
