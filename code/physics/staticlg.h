//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/staticlg.h
//!	
//!	DYNAMICS COMPONENT:
//!		Logic groups supporting the old CDynamicsStates.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_STATIC_LG_INC
#define _DYNAMICS_STATIC_LG_INC

#include "config.h"

#include "logicgroup.h"

namespace Physics
{
	// ---------------------------------------------------------------
	//	The static logic group contains static rigid bodies.
	// ---------------------------------------------------------------
	class StaticLG : public LogicGroup
	{
	public:

									StaticLG( const ntstd::String& p_name, CEntity* p_entity );
		virtual						~StaticLG( );
		virtual void                Load(); 
		virtual const GROUP_TYPE	GetType( ) const;
		virtual void				Update( const float p_timestep );
		virtual void				Activate( bool activateInHavok = false );
		virtual void				Deactivate( );
		virtual RigidBody*			AddRigidBody( const BodyCInfo* p_info );
		// StaticLG does not supports behaviors... Update is than faster 
		virtual void				AddBehavior( Behavior* p_behavior )	{ntAssert(false); UNUSED( p_behavior);} ;	//!< Behaviors can be added or removed at will. 
		virtual void				RemoveBehavior( const ntstd::String& p_behaviorID ) 	{ntAssert(false); UNUSED( p_behaviorID);} ;	//!< Remove any behavior with that id.
		virtual Behavior*			GetBehavior( const ntstd::String& p_behaviorID )	{return NULL; UNUSED( p_behaviorID);};

		virtual void				Debug_RenderCollisionInfo ();
	};
}

#endif // _DYNAMICS_STATIC_LG_INC
