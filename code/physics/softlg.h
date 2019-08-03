//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/softlg.h
//!	
//!	DYNAMICS COMPONENT:
//!		Logic groups supporting the old CDynamicsStates.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.10
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_SOFT_LG_INC
#define _DYNAMICS_SOFT_LG_INC

#include "config.h"

#include "logicgroup.h"
#include "physics/verletdef.h"

namespace Physics
{
	class VerletInstance;

	// ---------------------------------------------------------------
	//	Old cloth implementation.
	// ---------------------------------------------------------------
	class SoftLG : public LogicGroup
	{
	public:

									SoftLG( const ntstd::String& p_name, CEntity* p_entity );
		virtual						~SoftLG( );
		virtual const GROUP_TYPE	GetType( ) const;
		virtual void				Update( const float p_timestep );
		virtual void				Activate( bool activateInHavok = false );
		virtual void				Deactivate( );
		virtual RigidBody*			AddRigidBody( const BodyCInfo* p_info );

		static SoftLG*				Construct( CEntity* ent);

	private:

		VerletInstance* m_pobVerlet;
	};
}

#endif // _DYNAMICS_SOFT_LG_INC
