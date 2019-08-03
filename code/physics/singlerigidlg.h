//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/singlerigidlg.h
//!	
//!	DYNAMICS COMPONENT:
//!		Logic groups supporting the old CDynamicsStates.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_SINGLE_RIGID_LG_INC
#define _DYNAMICS_SINGLE_RIGID_LG_INC

#include "config.h"

#include "logicgroup.h"

#include "physics/havokincludes.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/hkmath.h>
#endif

class hkRigidBody;

namespace Physics
{
	// ---------------------------------------------------------------
	//	The rigid logic group contain a single rigid body only.
	// ---------------------------------------------------------------
	class SingleRigidLG : public LogicGroup
	{
	private:
		bool						m_full;

	public:

									SingleRigidLG( const ntstd::String& p_name, CEntity* p_entity );
		virtual						~SingleRigidLG( );
		virtual void				Load();

		virtual const GROUP_TYPE	GetType( ) const;
		virtual void				Update( const float p_timestep );
		virtual void				Activate( bool activateInHavok = false );
		virtual void				Deactivate( );
		virtual RigidBody*			AddRigidBody( const BodyCInfo* p_info );
		virtual void				Pause( bool bPause ); 

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkRigidBody*				GetRigidBody();
#endif

		// Behaviours

		void						ActivateParticleOnContact( );

		void						SetPiercingBehavior (bool bEnable);
		bool						HasPiercingBehavior ();

		void						SetDeflectionBehavior (bool bEnable);
		Behavior*					GetDeflectionBehavior ();
		void						SetDeflectionRenderer (bool bEnable);
		
		void						AddCheckAtRestBehavior( );
		void						AddCheckMovingBehavior( );		
		void						AddAntiGravityBehavior( float, float );


		// Velocity

		virtual void				SetLinearVelocity( const CDirection& p_linearVelocity );	
		virtual void				SetAngularVelocity( const CDirection& p_angularVelocity );
		virtual CDirection			GetLinearVelocity( );
		virtual CDirection			GetAngularVelocity( );

		/*
		void						VelocityTurn( float fTurn );
		void						SetRotation (const CQuat& obRotation);
		*/
		bool						MoveToSafe(const CPoint& endPos);
		bool						SetRotationSafe(const CQuat& orient);

		unsigned int				m_ePausedState;
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkVector4					m_pausedLinVel;
		hkVector4					m_pausedAngVel;
#endif
	
		virtual void				Debug_RenderCollisionInfo ();
	};
}

#endif // _DYNAMICS_SINGLE_RIGID_LG_INC
