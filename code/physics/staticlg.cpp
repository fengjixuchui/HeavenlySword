//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/staticlg.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "staticlg.h"

#include "system.h" // Needed for debugging tools

#include "element.h"
#include "behavior.h"
#include "rigidbody.h"
#include "physicsLoader.h"
#include "core/gatso.h"

namespace Physics {

	// -  StaticLG ------------------------------------------------------------------------------------------------------------------
	StaticLG::StaticLG( const ntstd::String& p_name, CEntity* p_entity ) :
		LogicGroup( p_name, p_entity )
	{ };

	StaticLG::~StaticLG( )
	{ };

	void StaticLG::Load()
	{
		ntstd::String psFile = CPhysicsLoader::AlterFilenameExtension( ntStr::GetString(m_entity->GetClumpString()) );
		PhysicsData * data = CPhysicsLoader::Get().LoadPhysics_Neutral(psFile.c_str());

		if (data)
		{
			m_loaderDataRef = data;
			PhysicsData::CloningParams params;

			params.m_collisionFlags.base = 0;
			params.m_collisionFlags.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
			params.m_collisionFlags.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	|
				Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
				Physics::RAGDOLL_BIT						|
				Physics::SMALL_INTERACTABLE_BIT				|
				Physics::LARGE_INTERACTABLE_BIT				|
				Physics::AI_WALL_BIT);
			params.m_static = true;

			data->CloneIntoLogicGroup(*this, params);
		}
	};

	const LogicGroup::GROUP_TYPE StaticLG::GetType( ) const
	{
		return STATIC_LG;
	};
		
	void StaticLG::Update( const float p_timestep )
	{ 
		UNUSED(p_timestep);
		/* no update on static LG...
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
		}*/		
	};
		
	void StaticLG::Activate( bool activateInHavok )
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Activate( activateInHavok );
		}
		LogicGroup::Activate( activateInHavok );
	};

	void StaticLG::Deactivate( )
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Deactivate( );
		}
		LogicGroup::Deactivate();
	};

	RigidBody* StaticLG::AddRigidBody( const BodyCInfo* p_info )	
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// StaticLG accespt only static rigid bodies !
		ntAssert( p_info->m_rigidBodyCinfo.m_motionType == hkMotion::MOTION_FIXED  ) ;
#endif
		return LogicGroup::AddRigidBody( p_info );
	};

	void StaticLG::Debug_RenderCollisionInfo ()
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
}
