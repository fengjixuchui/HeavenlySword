//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/animatedlg.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "animatedlg.h"

#include "system.h" // Needed for debugging tools

#include "element.h"
#include "behavior.h"
#include "rigidbody.h"
#include "physicsLoader.h"
#include "game/entity.h"
#include "core/gatso.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/entity/hkRigidBody.h>
#endif

namespace Physics {
	
	// -  AnimatedLG ------------------------------------------------------------------------------------------------------------------
	AnimatedLG::AnimatedLG( const ntstd::String& p_name, CEntity* p_entity ) :
		LogicGroup( p_name, p_entity ),
		m_isDynamic (false),
		m_bMakeDynamicOnUpdate (false),
		m_pobMakeDynamicOnUpdateImpulse(0)
	{ };

	AnimatedLG::~AnimatedLG( )
	{ };

	void AnimatedLG::Load()
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
			params.m_largeSmallInteractable = false;

			params.m_filterExceptionFlags.flags.exception_set = Physics::IGNORE_ENTITY_PTR_BIT; 

			data->CloneIntoLogicGroup(*this, params);
		}
	}

	const LogicGroup::GROUP_TYPE AnimatedLG::GetType( ) const
	{
		return ANIMATED_LG;
	};
		
	void AnimatedLG::Update( const float p_timestep )
	{ 
		GATSO_PHYSICS_START("AnimatedLG::Update");

		if (m_bMakeDynamicOnUpdate)
		{
			MakeDynamic(m_pobMakeDynamicOnUpdateImpulse);
			m_bMakeDynamicOnUpdate = false;
			m_pobMakeDynamicOnUpdateImpulse = NULL;
		}

		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Update( p_timestep );
		}
#ifndef NO_ANIMATIONLG_BEHAVIOR
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
#endif //NO_ANIMATIONLG_BEHAVIOR

		GATSO_PHYSICS_STOP("AnimatedLG::Update");
	};
		
	void AnimatedLG::Activate( bool activateInHavok )
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

	void AnimatedLG::Deactivate( )
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Deactivate( );
		}
		LogicGroup::Deactivate();
	};

	CDirection AnimatedLG::GetLinearVelocity( )
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

		ntAssert( 0 );
		return CDirection();
	}

	RigidBody* AnimatedLG::AddRigidBody( const BodyCInfo* p_info )	
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// StaticLG accespt only static rigid bodies !
		ntAssert( p_info->m_rigidBodyCinfo.m_motionType == hkMotion::MOTION_DYNAMIC  ); 
#endif
		// Construct the body.
		RigidBody* body = LogicGroup::AddRigidBody( p_info );
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		body->SetMotionType( HS_MOTION_KEYFRAMED );
#endif
	
		return body;
	};

	RigidBody* AnimatedLG::AddRigidBody( RigidBody* body )	
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD		
		//ntAssert( body->GetHkRigidBody()->getMotionType() == hkMotion::MOTION_DYNAMIC  ); 
#endif
		// Construct the body.
		body = LogicGroup::AddRigidBody( body );
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		body->SetMotionType( HS_MOTION_KEYFRAMED );		
#endif
	
		return body;
	};

	void AnimatedLG::MakeDynamicOnUpdate(CDirection * pobMakeDynamicOnUpdateImpulse)
	{
		m_bMakeDynamicOnUpdate = true;
		m_pobMakeDynamicOnUpdateImpulse = pobMakeDynamicOnUpdateImpulse;
	};

	void AnimatedLG::MakeDynamic( CDirection* pobUseDirectionForImpulse )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
			    // BE SURE if body can be dynamic
				hkMotion::MotionType storedMotionType =  body->GetHkRigidBody()->getStoredDynamicMotion() ?  body->GetHkRigidBody()->getStoredDynamicMotion()->getType() :
					hkMotion::MOTION_INVALID;   

				if ( storedMotionType != hkMotion::MOTION_INVALID && storedMotionType != hkMotion::MOTION_FIXED && storedMotionType != hkMotion::MOTION_KEYFRAMED )
				{
					body->SetMotionType( HS_MOTION_DYNAMIC );
					if (pobUseDirectionForImpulse)
					{
						body->ApplyLinearImpulse(*pobUseDirectionForImpulse);
					}
					else
					{
						body->SetLastAnimVelocities(); // because we use asynchronous sim the current velocities on body
												// are not velocities from animation. So set the stored ones... 
					}
				}

				//body->GetHkRigidBody()->setMotionType( hkMotion::MOTION_DYNAMIC );
			}
		}
		m_isDynamic = true;
#endif
	}


	void AnimatedLG::Debug_RenderCollisionInfo ()
	{
#ifndef _RELEASE

		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);

			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;

				hkRigidBody* pobRB = body->GetHkRigidBody();

				DebugCollisionTools::RenderCollisionFlags(pobRB);

				CMatrix obWorldMatrix(
					CQuat(pobRB->getRotation()(0),pobRB->getRotation()(1),pobRB->getRotation()(2),pobRB->getRotation()(3)),
					CPoint(pobRB->getPosition()(0),pobRB->getPosition()(1),pobRB->getPosition()(2)));
			
				DebugCollisionTools::RenderShape(obWorldMatrix,pobRB->getCollidable()->getShape());
			}
		}

#endif // _RELEASE
	}

#ifdef NO_ANIMATIONLG_BEHAVIOR
	void AnimatedLG::AddBehavior( Behavior* p_behavior )
	{
		UNUSED(p_behavior);
		ntAssert_p(false, ("Behaviour are disabled for AnimatedLG to increase the performance, because they were never used. If you want to start using them uncomment NO_ANIMATIONLG_BEHAVIOR macro."));
	}

	void AnimatedLG::RemoveBehavior( const ntstd::String& p_behaviorID )
	{
		UNUSED(p_behaviorID);
		ntAssert_p(false, ("Behaviour are disabled for AnimatedLG to increase the performance, because they were never used. If you want to start using them uncomment NO_ANIMATIONLG_BEHAVIOR macro."));
	}

	Behavior*	AnimatedLG::GetBehavior( const ntstd::String& p_behaviorID )
	{
		UNUSED( p_behaviorID);
		return NULL;
	}
#endif
}
