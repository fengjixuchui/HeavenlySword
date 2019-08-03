//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/singlerigid.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "singlerigidlg.h"

#include "system.h" // Needed for debugging tools

#include "element.h"
#include "behavior.h"
#include "rigidbody.h"
#include "rigidbodybehavior.h"
#include "maths_tools.h"

#include "physics/havokincludes.h"
#include "physics/havokthreadutils.h"

#include "physics/physicsLoader.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/entity/hkRigidBody.h>
#endif

#include "anim/transform.h"

namespace Physics {

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	hkRigidBody* SingleRigidLG::GetRigidBody()
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return body->GetHkRigidBody();
			}
		}
		return 0;
	}
#endif

	void SingleRigidLG::Pause( bool bPause ) 
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess mutex;

		if( bPause )
		{
			for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
					it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					if (m_ePausedState != hkMotion::MOTION_INVALID)
					{
						//ntPrintf("Pausing not paused (SingleRigidLG), tell peterf.");
						return;
					};
					
					Physics::RigidBody* body = (Physics::RigidBody*) current;
					m_ePausedState = body->GetHkRigidBody()->getMotionType();
					m_pausedLinVel = body->GetHkRigidBody()->getLinearVelocity();
					m_pausedAngVel = body->GetHkRigidBody()->getAngularVelocity();
					body->GetHkRigidBody()->setMotionType(hkMotion::MOTION_FIXED);
				}
			}		
			
		} 
		else
		{
			for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
					it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					if (m_ePausedState == hkMotion::MOTION_INVALID)
					{
						//ntPrintf("Unpausing not paused (SingleRigidLG), tell peterf.");
						return;
					};

					Physics::RigidBody* body = (Physics::RigidBody*) current;
					body->GetHkRigidBody()->setMotionType( (hkMotion::MotionType) m_ePausedState);
					body->GetHkRigidBody()->setAngularVelocity( m_pausedAngVel );
					body->GetHkRigidBody()->setLinearVelocity( m_pausedLinVel );
					m_ePausedState = hkMotion::MOTION_INVALID;
				}
			}
		}
#else
		UNUSED( bPause );
#endif
	}

	// -  SingleRigidLG ------------------------------------------------------------------------------------------------------------------
	SingleRigidLG::SingleRigidLG( const ntstd::String& p_name, CEntity* p_entity ) :
		LogicGroup( p_name, p_entity ),
		m_full( false ),
		m_ePausedState(hkMotion::MOTION_INVALID),
		m_pausedLinVel(0,0,0),
		m_pausedAngVel(0,0,0)
	{ 		
	};

	SingleRigidLG::~SingleRigidLG( )
	{ };

	void SingleRigidLG::Load() 
	{
		ntstd::String psFile = CPhysicsLoader::AlterFilenameExtension( ntStr::GetString(m_entity->GetClumpString()) );
		PhysicsData * data = CPhysicsLoader::Get().LoadPhysics_Neutral(psFile.c_str());

		if (data)
		{
			m_loaderDataRef = data;
			PhysicsData::CloningParams params;

			params.m_collisionFlags.base = 0;
			params.m_collisionFlags.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
			params.m_collisionFlags.flags.i_collide_with = (	
				Physics::CHARACTER_CONTROLLER_PLAYER_BIT	|
				Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
				Physics::RAGDOLL_BIT						|
				Physics::SMALL_INTERACTABLE_BIT				|
				Physics::LARGE_INTERACTABLE_BIT				|
				Physics::AI_WALL_BIT);

			params.m_static = false;
			params.m_largeSmallInteractable = true;

			data->CloneIntoLogicGroup(*this, params);
		}
	}

	const LogicGroup::GROUP_TYPE SingleRigidLG::GetType( ) const
	{
		return SINGLE_RIGID_BODY_LG;
	};
		
	void SingleRigidLG::Update( const float p_timestep )
	{ 
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Update( p_timestep );
		}

		ntstd::List<Behavior*>::iterator itb = m_behaviorList.begin();
		while (	itb != m_behaviorList.end() )
		{
			Behavior* event = (*itb);
			++itb;
			bool remove = event->Update( this );
			if( remove )
			{
				m_behaviorList.remove( event );
				NT_DELETE( event );
			}
		}
	};
		
	void SingleRigidLG::Activate( bool activateInHavok )
	{
		if( this->IsActive() == false )
		{
			for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
					it != m_elementList.end(); ++it )
			{
				Element* current = (*it);
				current->Activate( activateInHavok );
			}
			LogicGroup::Activate( activateInHavok );
		}
	};

	void SingleRigidLG::Deactivate( )
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Deactivate( );
		}
		LogicGroup::Deactivate();
	};

	RigidBody* SingleRigidLG::AddRigidBody( const BodyCInfo* p_info )	
	{

		user_error_p( m_full == false, ("More than one rigid body in a Single Ridge Body Name : %s", p_info->m_name) );

		// Construct the body.
		RigidBody* body = LogicGroup::AddRigidBody( p_info );

		m_full = true;

		// Return it for further settings.
		return body;
	};

	

	void SingleRigidLG::ActivateParticleOnContact( )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				body->SetHavokFlag( Physics::PROPERTY_SPAWN_PARTICLES_INT, 1 );
			}
		}
	}

	void SingleRigidLG::AddCheckAtRestBehavior( )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				body->AddBehavior( NT_NEW BodyAtRest( ntstd::String("msg_atrest") ) );
			}
		}
	}

	void  SingleRigidLG::SetPiercingBehavior (bool bEnable)
	{
		if (bEnable)
		{
			for( ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					Physics::RigidBody* body = (Physics::RigidBody*) current;
					body->AddBehavior( NT_NEW PiercingBehavior() );
				}
			}
		}
		else
		{
			for( ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					Physics::RigidBody* body = (Physics::RigidBody*) current;
					body->RemoveBehavior( ntstd::String( "PIERCE" ) );
				}
			}
		}
	}

	bool SingleRigidLG::HasPiercingBehavior ()
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return ( body->GetBehavior( ntstd::String( "PIERCE" )) ? true : false );
			}
		}

		return false;
	}

	void SingleRigidLG::SetDeflectionBehavior (bool bEnable)
	{
		if (bEnable)
		{
			for ( ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					Physics::RigidBody* body = (Physics::RigidBody*) current;
					body->AddBehavior( NT_NEW DeflectionBehavior() );
				}
			}
		}
		else
		{
			for ( ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					Physics::RigidBody* body = (Physics::RigidBody*) current;
					body->RemoveBehavior( ntstd::String( "DEFLECT" ) );
				}
			}
		}
	}

	Behavior* SingleRigidLG::GetDeflectionBehavior ()
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return body->GetBehavior( ntstd::String("DEFLECT") );
			}
		}

		return 0;
	}

	void SingleRigidLG::SetDeflectionRenderer (bool bEnable)
	{
		if (bEnable)
		{
			for ( ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					Physics::RigidBody* body = (Physics::RigidBody*) current;
					body->AddBehavior( NT_NEW DeflectionRendererBehavior() );
				}
			}
		}
		else
		{
			for ( ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					Physics::RigidBody* body = (Physics::RigidBody*) current;
					body->RemoveBehavior( ntstd::String( "DEFLECT_RENDER" ) );
				}
			}
		}
	}

	void SingleRigidLG::AddCheckMovingBehavior( )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				body->AddBehavior( NT_NEW CheckMoving( ntstd::String("msg_moving") ) );
			}
		}

	}

	CDirection SingleRigidLG::GetLinearVelocity( )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return CDirection(body->GetLinearVelocity());
			}
		}

		return CDirection();
	}

	CDirection SingleRigidLG::GetAngularVelocity( )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return CDirection(body->GetAngularVelocity());
			}
		}

		return CDirection();
	}

	void SingleRigidLG::SetLinearVelocity( const CDirection& p_linearVelocity )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				body->SetLinearVelocity( CVector(p_linearVelocity) );
				return;
			}
		}

	}

	void SingleRigidLG::SetAngularVelocity( const CDirection& p_angularVelocity )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				body->SetAngularVelocity( CVector(p_angularVelocity) );
				return;
			}
		}
	}

	void SingleRigidLG::AddAntiGravityBehavior( float p_vel, float p_dur)
	{

		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				body->AddBehavior( NT_NEW AntiGravity( body, p_vel, p_dur) );
			}
		}
	}

	void SingleRigidLG::Debug_RenderCollisionInfo ()
	{
#ifndef _RELEASE
        if (!GetRigidBody()) 
			return;

		DebugCollisionTools::RenderCollisionFlags(GetRigidBody());

		CMatrix obWorldMatrix(
			CQuat(GetRigidBody()->getRotation()(0),GetRigidBody()->getRotation()(1),GetRigidBody()->getRotation()(2),GetRigidBody()->getRotation()(3)),
			CPoint(GetRigidBody()->getPosition()(0),GetRigidBody()->getPosition()(1),GetRigidBody()->getPosition()(2)));
	
		DebugCollisionTools::RenderShape(obWorldMatrix,GetRigidBody()->getCollidable()->getShape());

#endif // _RELEASE
	}	

	bool SingleRigidLG::MoveToSafe(const CPoint& endPos)
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return body->MoveToSafe(endPos);
			}
		}

		return true; 
	}

	bool SingleRigidLG::SetRotationSafe(const CQuat& orient)
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return body->SetRotationSafe(orient);
			}
		}

		return true; 
	}
}
