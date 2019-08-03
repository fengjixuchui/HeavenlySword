//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/compoundlg.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "compoundlg.h"

#include "system.h" // Needed for debugging tools

#include "element.h"
#include "behavior.h"
#include "rigidbody.h"
#include "rigidbodybehavior.h"

#include "physics/havokincludes.h"
#include "physics/havokthreadutils.h"
#include "physics/physicsLoader.h"
#include "physics/maths_tools.h"



#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkutilities/inertia/hkInertiaTensorComputer.h>
#endif

#include "core/timer.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/message.h"

// temporary fix... will be part of physics tools soon
//Creates hkConvexListShape if it is possible, hkListShape otherwise. 
hkShape * CreateListShape(hkShape ** list, int n);

//Creates hkConvexTransformShape if shape is convex, hkTransformShape otherwise.
// BEWARE: function also removeReference() from input shape. 
hkShape * CreateTransformShape(hkShape *shape, const hkTransform& trf);

namespace Physics {

	// -----------------------------------------------------------------------------------
	//	GroupAtRest Behavior.
	//		Generate a message whent the rigid body is at rest.
	// -----------------------------------------------------------------------------------
	class GroupAtRest : public Behavior
	{
	private:

		float			m_fTimeAtRest;

	public:

						GroupAtRest( const ntstd::String& p_message );
		virtual			~GroupAtRest( );
		virtual bool	Update( LogicGroup* p_group );	
	};

	//---------------------------------------------------------------
	//!
	//! GroupAtRest constructor.
	//!		\param ntstd::String& - message to generate.
	//!
	//---------------------------------------------------------------

	GroupAtRest::GroupAtRest( const ntstd::String& p_message ):
		m_fTimeAtRest( 0.0f )
	{
		m_id = p_message;
	}

	//---------------------------------------------------------------
	//!
	//! GroupAtRest destructor.
	//!
	//---------------------------------------------------------------

	GroupAtRest::~GroupAtRest()			
	{ }

	static const float fATREST_ANGULAR_SCALE = 1.0f;
	static const float fATREST_SPEED_THRESHOLD = 5.0f * 5.0f;
	static const float fTIME_AT_REST_DURATION = 0.75f;

	//---------------------------------------------------------------
	//!
	//! GroupAtRest event callback.
	//!		\param Element* - element to check.
	//!
	//---------------------------------------------------------------

	bool	GroupAtRest::Update( LogicGroup* p_group )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( p_group );

		bool	hasComeToRest = false;
		float fOverallSpeed = 0.0f;

		for(ntstd::List<Element*>::iterator obIt = p_group->GetElementList().begin(); obIt!=p_group->GetElementList().end(); ++obIt)
		{
			if( (*obIt)->GetType() == Element::RIGID_BODY )
			{
				RigidBody*		elem = (RigidBody*) (*obIt);
				hkRigidBody*	body = elem->GetHkRigidBody();

				fOverallSpeed+=body->getLinearVelocity().lengthSquared3();
				fOverallSpeed+=body->getAngularVelocity().lengthSquared3() * (hkSimdReal) fATREST_ANGULAR_SCALE;
			}
			
			
		}

		if (fOverallSpeed<fATREST_SPEED_THRESHOLD)
		{
			hasComeToRest = true;
		}

		if( hasComeToRest )
		{
			m_fTimeAtRest += CTimer::Get().GetGameTimeChange();

			if( m_fTimeAtRest > fTIME_AT_REST_DURATION )
			{
				CMessageSender::SendEmptyMessage( m_id.c_str(), p_group->GetEntity()->GetMessageHandler() );
				return true;
			}
		}
#else
		UNUSED (p_group) ;
#endif
		return false;
	}	

	// - COMPOUND -------------------------------------------------------------------
	CompoundLG::CompoundLG( const ntstd::String& p_name, CEntity* p_entity ) :
		LogicGroup( p_name, p_entity ),
		isCollapsed( false ),
		m_bDeferCollapse( false ),
		m_bFixFirst( false )
	{
	};

	CompoundLG::~CompoundLG( )
	{
		for (	ntstd::List<RigidBody*>::iterator it = m_bodiesList.begin(); 
				it != m_bodiesList.end(); ++it )
		{
			RigidBody* current = (*it);
			NT_DELETE( current );
		}

		m_bodiesList.clear();
	};

	void CompoundLG::Load()
	{
		ntstd::String psFile = CPhysicsLoader::AlterFilenameExtension( ntStr::GetString(m_entity->GetClumpString()) );
		PhysicsData * data = CPhysicsLoader::Get().LoadPhysics_Neutral(psFile.c_str());

		if (data)
		{
			m_loaderDataRef = data;
			// 1.) Import rigid bodies of parts
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

			// 2.) They are not in the world at the moment

			ntstd::List<Element*>::iterator itElem = m_elementList.begin();
			while( itElem != m_elementList.end() )
			{
				Element* current = (*itElem);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					RigidBody* body = (RigidBody*) current;
					m_bodiesList.push_back(body);
					++ itElem;
				}
			}

			m_elementList.clear();

			// 3.) Create main body
			// We have a set of rigid bodies. Now se need to create the main body, an envelope of those bodies. 			 
			// The main body properties (f.e. friction, restitution) will be average values. The shape of main bodies
			// will be the sum of all shapes. 
			
			CScopedArray<hkShape*> apobShapeArray( NT_NEW hkShape*[m_bodiesList.size()] );
			Physics::BodyCInfo mainInfo;
			mainInfo.m_rigidBodyCinfo.m_mass = 0;
			mainInfo.m_rigidBodyCinfo.m_linearDamping = 0;
			mainInfo.m_rigidBodyCinfo.m_angularDamping = 0;
			mainInfo.m_rigidBodyCinfo.m_friction = 0;
			mainInfo.m_rigidBodyCinfo.m_restitution = 0;

			int iShapeIndex = 0; 			
			for(ntstd::List<RigidBody*>::iterator it = m_bodiesList.begin(); it != m_bodiesList.end(); ++it, ++iShapeIndex)
			{
				hkRigidBody * body = (*it)->GetHkRigidBody();

				hkShape * pobShape = const_cast<hkShape *>(body->getCollidable()->getShape());
				pobShape->addReference();

				/// add trasform shape to shift the shape to correct position
				hkTransform trf;
				if ((*it)->GetTransform() != GetEntity()->GetHierarchy()->GetRootTransform())
				{
					const CMatrix& locTransform = (*it)->GetTransform()->GetLocalMatrix();
					trf = Physics::MathsTools::CMatrixTohkTransform(locTransform );																
					pobShape = CreateTransformShape( pobShape, trf );
				}
				else
				{
					//  hmm this colapsable body consist only from one shape... !!!
					trf.setIdentity();
				}

				apobShapeArray[iShapeIndex] = pobShape;

				// sum masses
				mainInfo.m_rigidBodyCinfo.m_mass += body->getMass();
				hkVector4 addMassCenter;
				addMassCenter.setTransformedPos(trf, body->getCenterOfMassInWorld());
				mainInfo.m_rigidBodyCinfo.m_centerOfMass.addMul4(body->getMass(), addMassCenter);

				mainInfo.m_rigidBodyCinfo.m_linearDamping += body->getLinearDamping();
				mainInfo.m_rigidBodyCinfo.m_angularDamping += body->getAngularDamping();
				mainInfo.m_rigidBodyCinfo.m_friction += body->getMaterial().getFriction();
				mainInfo.m_rigidBodyCinfo.m_restitution += body->getMaterial().getRestitution();
				if (mainInfo.m_rigidBodyCinfo.m_maxLinearVelocity <  body->getMaxLinearVelocity())
					mainInfo.m_rigidBodyCinfo.m_maxLinearVelocity = body->getMaxLinearVelocity();

				if (mainInfo.m_rigidBodyCinfo.m_maxAngularVelocity <  body->getMaxAngularVelocity())
					mainInfo.m_rigidBodyCinfo.m_maxAngularVelocity = body->getMaxAngularVelocity();		
			}

			// mass center is average of mass center weight by mass. 
			mainInfo.m_rigidBodyCinfo.m_centerOfMass.mul4(1.0f / mainInfo.m_rigidBodyCinfo.m_mass);

			int n = m_bodiesList.size();

			mainInfo.m_rigidBodyCinfo.m_linearDamping /= n;
			mainInfo.m_rigidBodyCinfo.m_angularDamping /= n;
			mainInfo.m_rigidBodyCinfo.m_friction /= n;
			mainInfo.m_rigidBodyCinfo.m_restitution /= n; 
			mainInfo.m_rigidBodyCinfo.m_shape = CreateListShape(apobShapeArray.Get(),n);

			Physics::EntityCollisionFlag obFlag; obFlag.base = 0;
			obFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
			obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	|
				Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
				Physics::RAGDOLL_BIT						|
				Physics::SMALL_INTERACTABLE_BIT				|
				Physics::LARGE_INTERACTABLE_BIT				|
				Physics::AI_WALL_BIT);

			mainInfo.m_rigidBodyCinfo.m_collisionFilterInfo = (int)obFlag.base;
			mainInfo.m_rigidBodyCinfo.m_motionType	= hkMotion::MOTION_DYNAMIC;
			mainInfo.m_rigidBodyCinfo.m_qualityType	= HK_COLLIDABLE_QUALITY_MOVING;
			
			// mass properties, for a moment take simply box  inertia
			hkAabb obAABB;
			mainInfo.m_rigidBodyCinfo.m_shape->getAabb(hkTransform::getIdentity(),0.0f,obAABB);		
			hkVector4 obHalfExtents;
			obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
			obHalfExtents.mul4(0.5f);

			hkMassProperties obMassProperties;
			obMassProperties.m_centerOfMass = mainInfo.m_rigidBodyCinfo.m_centerOfMass;
			obMassProperties.m_mass = mainInfo.m_rigidBodyCinfo.m_mass;

			hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,mainInfo.m_rigidBodyCinfo.m_mass,obMassProperties);
			mainInfo.m_rigidBodyCinfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;			

			mainInfo.m_transform = m_entity->GetHierarchy()->GetRootTransform();
			m_uncollapsedRigidName = m_entity->GetName();		
			mainInfo.m_name = m_uncollapsedRigidName.c_str();

			AddRigidBody(&mainInfo);
		}
	}
	const LogicGroup::GROUP_TYPE CompoundLG::GetType( ) const
	{
		return COMPOUND_RIGID_LG;
	};
		
	void CompoundLG::Update( const float p_timestep )
	{ 
		if( m_bDeferCollapse )
		{
			DoCollapse();
			m_bDeferCollapse = false;
		}
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
		
	void CompoundLG::Activate( bool activateInHavok )
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

	void CompoundLG::Deactivate( )
	{
		if( this->IsActive() == true )
		{
			for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
					it != m_elementList.end(); ++it )
			{
				Element* current = (*it);
				current->Deactivate( );
			}
			LogicGroup::Deactivate();
		}
	};

	RigidBody* CompoundLG::AddRigidBody( const BodyCInfo* p_info )	
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// StaticLG accespt only static rigid bodies !
		ntAssert( p_info->m_rigidBodyCinfo.m_motionType != hkMotion::MOTION_FIXED  ) ;
#endif
		return LogicGroup::AddRigidBody( p_info );
	};

	RigidBody* CompoundLG::AddRigidBody( RigidBody* p_body )	
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// StaticLG accespt only static rigid bodies !
		ntAssert( p_body->GetHkRigidBody()->getMotionType() != hkMotion::MOTION_FIXED  ) ;
#endif
		return LogicGroup::AddRigidBody( p_body );
	};

	void CompoundLG::AddCheckAtRestBehavior( )
	{
		AddBehavior( NT_NEW GroupAtRest( ntstd::String("msg_atrest") ) );
	}

	void CompoundLG::Collapse( bool bFixFirst )
	{
		ntAssert( !isCollapsed );
		m_bDeferCollapse = true;
		m_bFixFirst = bFixFirst;
	}
	void CompoundLG::DoCollapse( )
	{
		ntAssert( m_bDeferCollapse == true );
		// collapsing can not occur within a simulation step
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if( m_bodiesList.size() != 0 )
		{
			hkVector4 obLinearVelocity;
			hkVector4 obAngularVelocity;

			for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
					it != GetElementList().end(); ++it )
			{
				Physics::Element* current = (*it);
				if( current->GetType() == Physics::Element::RIGID_BODY )
				{
					Physics::RigidBody* body = (Physics::RigidBody*) current;
					obLinearVelocity = body->GetHkRigidBody()->getLinearVelocity();
					obAngularVelocity = body->GetHkRigidBody()->getAngularVelocity();
					body->Deactivate();
					body->RemovedFromSystem( GetEntity()->GetPhysicsSystem() );
					NT_DELETE( body );
				}
			}

			m_elementList.clear();

			for (	ntstd::List<RigidBody*>::iterator itb = m_bodiesList.begin(); 
					itb != m_bodiesList.end(); ++itb )
			{
				obLinearVelocity.mul4(-1.0f); // This alternates the velocities of the pieces to make it look like the object is splitting in two

				RigidBody* body = (*itb);
				AddRigidBody( body );
				body->ActivateForced();
				body->GetHkRigidBody()->setAngularVelocity(obAngularVelocity);
				body->GetHkRigidBody()->setLinearVelocity(obLinearVelocity);
				body->GetHkRigidBody()->activate();
				body->AddedToSystem( GetEntity()->GetPhysicsSystem() );

				if (m_bFixFirst && itb == m_bodiesList.begin())
				{
					body->SetMotionType(HS_MOTION_FIXED);
				}
				else
				{
					body->SetMotionType(HS_MOTION_DYNAMIC);
				}
			}

			m_bodiesList.clear();
		}
#endif
		isCollapsed = true;
	}
	
	bool CompoundLG::MoveToSafe(const CPoint& endPos) 
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
			it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return body->MoveToSafe(endPos); // hmmm considering that compoundLg has only one body at the moment										
			}
		}

		return true;
	}

	bool CompoundLG::SetRotationSafe(const CQuat& orient)
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return body->SetRotationSafe(orient); // hmmm considering that compoundLg has only one body at the moment					
			}
		}

		return true; 
	}


	void CompoundLG::AddAntiGravityBehavior( float p_vel, float p_dur)
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

	void CompoundLG::Debug_RenderCollisionInfo ()
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
