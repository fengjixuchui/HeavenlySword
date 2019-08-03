//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/animatedlg.h
//!	
//!	DYNAMICS COMPONENT:
//!		Logic groups supporting the old CDynamicsStates.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_ANIMATED_LG_INC
#define _DYNAMICS_ANIMATED_LG_INC

#include "config.h"

#include "logicgroup.h"

namespace Physics
{
	// ---------------------------------------------------------------
	//	The animated logic group contains animated rigid bodies.
	// ---------------------------------------------------------------
	class AnimatedLG : public LogicGroup
	{
	public:

									AnimatedLG( const ntstd::String& p_name, CEntity* p_entity );
		virtual						~AnimatedLG( );
		virtual void				Load(); 
		virtual const GROUP_TYPE	GetType( ) const;
		virtual void				Update( const float p_timestep );
		virtual void				Activate( bool activateInHavok  = false);
		virtual void				Deactivate( );
		virtual CDirection			GetLinearVelocity( );
		virtual RigidBody*			AddRigidBody( const BodyCInfo* p_info );
		virtual RigidBody*			AddRigidBody( RigidBody* body );

		// we do not use any behaviour in AnimatedLG at the moment -> do code a bit more optimised. 
#define NO_ANIMATIONLG_BEHAVIOR
#ifdef NO_ANIMATIONLG_BEHAVIOR
		virtual void						AddBehavior( Behavior* p_behavior )				   ;	//!< Behaviors can be added or removed at will. 
		virtual void						RemoveBehavior( const ntstd::String& p_behaviorID )  ;	//!< Remove any behavior with that id.
		virtual Behavior*					GetBehavior( const ntstd::String& p_behaviorID )	   ;	//!< Return a behavior tied to this id.
#endif

		void						MakeDynamic( CDirection* pobUseDirectionForImpulse = 0 );
		bool						IsDynamic(void) { return m_isDynamic; };
		void						MakeDynamicOnUpdate(CDirection * pobMakeDynamicOnUpdateImpulse = 0);


		virtual void				Debug_RenderCollisionInfo ();

	private:
		bool m_isDynamic:1;
		bool m_bMakeDynamicOnUpdate:1;
		CDirection * m_pobMakeDynamicOnUpdateImpulse; // just pointer used only during MakeDynamicOnUpdate. 
		                                            // Caller is responsible for memory referenced by this pointer
	};

}

#endif // _DYNAMICS_ANIMATED_LG_INC
