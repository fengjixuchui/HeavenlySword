//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/compoundlg.h
//!	
//!	DYNAMICS COMPONENT:
//!		Logic groups supporting the old CDynamicsStates.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_COMPOUND_LG_INC
#define _DYNAMICS_COMPOUND_LG_INC

#include "config.h"

#include "logicgroup.h"

#include "physics/havokincludes.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/Motion/hkMotion.h>
#endif

namespace Physics
{

	// ---------------------------------------------------------------
	//	The CompoundLG logic group contains a usually destroyable RB.
	// ---------------------------------------------------------------
	class CompoundLG : public LogicGroup
	{
	public:

		ntstd::List<RigidBody*>			m_bodiesList;		//!< List of elements.

									CompoundLG( const ntstd::String& p_name, CEntity* p_entity );
		virtual						~CompoundLG( );
		virtual void				Load(); 
		virtual const GROUP_TYPE	GetType( ) const;
		virtual void				Update( const float p_timestep );
		virtual void				Activate( bool activateInHavok = false);
		virtual void				Deactivate( );
		virtual RigidBody*			AddRigidBody( const BodyCInfo* p_info );
		virtual RigidBody*			AddRigidBody( RigidBody* p_info );

		void						AddCheckAtRestBehavior( );
		void						Collapse( bool bFixFirstBody = false );
		bool						IsCollapsed() const {return isCollapsed;} 
		bool						MoveToSafe(const CPoint& endPos);
		bool						SetRotationSafe(const CQuat& orient);
		void						AddAntiGravityBehavior( float p_vel, float p_dur);


	private:
		void						DoCollapse( );

		bool isCollapsed;
		bool m_bDeferCollapse;			//!< The collapse is trigger in a callback but we need to defer it to outside the simulation step
		bool m_bFixFirst;

		virtual void				Debug_RenderCollisionInfo ();

		ntstd::String               m_uncollapsedRigidName; 

	};
}

#endif // _DYNAMICS_COMPOUND_LG_INC
