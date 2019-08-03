//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/spearlg.h
//!	
//!	DYNAMICS COMPONENT:
//!		Logic groups supporting the old CDynamicsStates.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_SPEAR_LG_INC
#define _DYNAMICS_SPEAR_LG_INC

#include "config.h"

#include "logicgroup.h"

class CEntity;
/*
#include "collisionbitfield.h"

#include "physics/havokincludes.h"

class hkRigidBodyCinfo;*/
#include "physics/havokincludes.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/hkmath.h>
#include <hkdynamics/Motion/hkMotion.h>
#endif

namespace Physics
{
	/*class RigidBody;
	class BodyCInfo;
	class Behavior;*/

	// ---------------------------------------------------------------
	//	The spear encapsulate the complex spear behavior.
	// ---------------------------------------------------------------
	static const int			iMAX_RAGDOLLS = 3;
	class SpearLG : public LogicGroup
	{
	public:

		struct RAGDOLL_LINK
		{
			float		fFractionOffset;
			CEntity*	pobEntity;
		};

		int							m_iRagdollCount;
		RAGDOLL_LINK				m_astRagdollLink [iMAX_RAGDOLLS];

									SpearLG( const ntstd::String& p_name, CEntity* p_entity );
		virtual						~SpearLG( );
		virtual void				Load(); 
		virtual const GROUP_TYPE	GetType( ) const;
		virtual void				Update( const float p_timestep );
		virtual void				Activate( bool activateInHavok = false );
		virtual void				Deactivate( );
		//virtual void				EntityRootTransformHasChanged();
		virtual void				Pause( bool bPause );

		virtual CDirection			GetLinearVelocity( );
		virtual void				SetLinearVelocity( const CDirection& p_linearVelocity );	
		float						GetMass( );

		void						AddThrownBehaviour( );
		void						RemoveThrownBehaviour( );
		bool						isThrowned;

		void						AddCheckAtRestBehavior( );

		void						RemoveAntiGravBehaviour ();

		void						RemoveRagdolls ();

		RigidBody*					GetRigidBody( );

		virtual void				Debug_RenderCollisionInfo ();
	private:

		unsigned int				m_ePausedState;
		hkVector4					m_pausedLinVel;
		hkVector4					m_pausedAngVel;
	};

}

#endif // _DYNAMICS_SPEAR_LG_INC
