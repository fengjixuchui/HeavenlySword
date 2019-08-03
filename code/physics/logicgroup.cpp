//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/logicgroup.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "logicgroup.h"
#include "element.h"
#include "behavior.h"
#include "rigidbody.h"
#include "system.h"

namespace Physics {

	void LogicGroup::EntityRootTransformHasChanged()
	{}

	//---------------------------------------------------------------
	//!
	//! Constructor.
	//!		\param const ntstd::String& - Group name.
	//!
	//---------------------------------------------------------------
	LogicGroup::LogicGroup( const ntstd::String& p_name, CEntity*	p_entity ) :
		m_name( p_name ),
		m_entity( p_entity )		
	{
		m_isActive = false;
	}
	
	//---------------------------------------------------------------
	//!
	//! Destructor.
	//!
	//---------------------------------------------------------------
	LogicGroup::~LogicGroup( )	
	{
		for (	ntstd::List<Behavior*>::iterator it = m_behaviorList.begin(); 
				it != m_behaviorList.end(); ++it )
		{
			Behavior* current = (*it);
			NT_DELETE( current );
		}

		m_behaviorList.clear();

		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			NT_DELETE( current );
		}

		m_elementList.clear();
	};

	void LogicGroup::Activate( bool activateInHavok)
	{
		UNUSED(activateInHavok);
		m_isActive = true;
	}

	void LogicGroup::Deactivate( )
	{
		m_isActive = false;
	}

	bool LogicGroup::IsActive( )
	{
		return m_isActive;
	}

	void LogicGroup::PausePresenceInHavokWorld(bool add)
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->PausePresenceInHavokWorld(add);
				++ itElem;
			}
		}		
	}

	//---------------------------------------------------------------
	//!
	//! Called when a group is added to a system.
	//!		\param const System* - System ptr.
	//!
	//---------------------------------------------------------------
	void LogicGroup::AddedToSystem( System* p_system )
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->AddedToSystem( p_system );
		}
	}

	void LogicGroup::RemovedFromSystem( System* p_system )
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->RemovedFromSystem( p_system );
		}
	}
	
	//---------------------------------------------------------------
	//!
	//! Return the logic group name.
	//!		\return const ntstd::String& - Group name.
	//!
	//---------------------------------------------------------------

	const ntstd::String& LogicGroup::GetName( ) const
	{
		return m_name;
	}

	//---------------------------------------------------------------
	//!
	//! Add a behavior.
	//!		\param Behavior& - Behavior.
	//!
	//---------------------------------------------------------------

	void LogicGroup::AddBehavior( Behavior* p_behavior )
	{
		if( ! GetBehavior( p_behavior->m_id ) )
		{
			m_behaviorList.push_back( p_behavior );
		}
	}

	//---------------------------------------------------------------
	//!
	//!	Remove a behavior.
	//!		\param const ntstd::String& - Behavior ID.
	//!
	//---------------------------------------------------------------

	void LogicGroup::RemoveBehavior( const ntstd::String& p_message )
	{
		for (	ntstd::List<Behavior*>::iterator it = m_behaviorList.begin(); 
				it != m_behaviorList.end(); ++it )
		{
			Behavior* event = (Behavior*)(*it);
			if( event->m_id == p_message )
			{
				m_behaviorList.remove( event );
				NT_DELETE( event );
				return;
			}
		}
	}

	//---------------------------------------------------------------
	//!
	//!	Search for a behavior.
	//!		\param const ntstd::String& - Behavior ID.
	//!
	//---------------------------------------------------------------

	Behavior* LogicGroup::GetBehavior( const ntstd::String& p_message )
	{
		for (	ntstd::List<Behavior*>::iterator it = m_behaviorList.begin(); 
				it != m_behaviorList.end(); ++it )
		{
			Behavior* event = (Behavior*)(*it);
			if( event->m_id == p_message )
			{
				return event;
			}
		}

		return 0;
	}

	//---------------------------------------------------------------
	//!
	//!	Add a rigid body to the group.
	//!		\param const BodyCInfo*  - Pointer to the main bodyCInfo.
	//!		\return Element*  - Pointer to the constructed element.
	//!
	//---------------------------------------------------------------

	RigidBody* LogicGroup::AddRigidBody( const BodyCInfo* p_info )
	{
		// Construct the body.
		RigidBody* body = (RigidBody*) RigidBody::ConstructRigidBody( m_entity, p_info );
		
		// Add it to the body list.
		m_elementList.push_back( body );

		if( GetType() == LogicGroup::SYSTEM )
			body->AddedToSystem( (System*) this );

		// Return it for further settings.
		return body;
	}

	RigidBody* LogicGroup::AddRigidBody( RigidBody* body )
	{
		// Add it to the body list.
		m_elementList.push_back( body );

		if( GetType() == LogicGroup::SYSTEM )
			body->AddedToSystem( (System*) this );

		// Return it for further settings.
		return body;
	}



	//----------------------------------------------------------
	//!
	//!	Access to the group elements.
	//!		\return Element* - Pointer to the element. 0 if not found.
	//!		\param  const ntstd::String& - Element name 
	//!
	//----------------------------------------------------------

	Element* LogicGroup::GetElementByName( const ntstd::String& p_name )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			if( current->GetName() == p_name)
			{
				return current;
			}
		}
		return 0;
	}

	

	//----------------------------------------------------------
	//!
	//!	Access to the group elements.
	//!		\return ntstd::List<Physics::Element*>& - List of elements.
	//!
	//----------------------------------------------------------

	ntstd::List<Physics::Element*>&	LogicGroup::GetElementList( )
	{
		return m_elementList;
	}

	//----------------------------------------------------------
	//!
	//!	Update the collision filters. 
	//!
	//----------------------------------------------------------

	void LogicGroup::UpdateCollisionFilter ()
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->UpdateCollisionFilter();
				++ itElem;
			}
		}
	}

	//----------------------------------------------------------
	//!
	//!	Change the collision filter info. It NOT updates collision filter.
	//!
	//----------------------------------------------------------
	void LogicGroup::SetCollisionFilterInfo(uint32_t info)
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->SetCollisionFilterInfo(info);
				++ itElem;
			}
		}
	}

	//----------------------------------------------------------
	//!
	//!	Return the collisoin filter info of the first element.
	//!
	//----------------------------------------------------------
	uint32_t LogicGroup::GetCollisionFilterInfo() const
	{
		ntstd::List<Element*>::const_iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				return body->GetCollisionFilterInfo();				
			}
			++itElem;
		}
		return 0;
	}

	//----------------------------------------------------------
	//!
	//!	Add collision listener to bodies in group.
	//!
	//----------------------------------------------------------
	void LogicGroup::AddCollisionListener(hkCollisionListener * cl)
	{
		ntstd::List<Element*>::const_iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;	
				body->AddCollisionListener(cl);
			}
			++itElem;
		}
	}

	//----------------------------------------------------------
	//!
	//!	Remove collision listener from bodies in group.
	//!
	//----------------------------------------------------------
	void LogicGroup::RemoveCollisionListener(hkCollisionListener * cl)
	{
		ntstd::List<Element*>::const_iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;	
				body->RemoveCollisionListener(cl);
			}
			++itElem;
		}
	}


	CDirection LogicGroup::GetLinearVelocity( )
	{
		return CDirection();
	}

	CDirection LogicGroup::GetAngularVelocity( )
	{
		return CDirection();
	}

	void LogicGroup::SetLinearVelocity( const CDirection& p_vel )
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->SetLinearVelocity( CVector(p_vel) );
				++ itElem;
			}
		}
	}

	void LogicGroup::SetAngularVelocity( const CDirection& p_vel )
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->SetAngularVelocity( CVector(p_vel) );
				++ itElem;
			}
		}
	}

	void LogicGroup::ApplyLinearImpulse( const CDirection& p_vel )
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->ApplyLinearImpulse( p_vel );
				++ itElem;
			}
		}
	}

	void LogicGroup::ApplyLocalisedLinearImpulse( const CDirection& p_vel, const CVector& p_point )
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->ApplyLocalisedLinearImpulse( p_vel, p_point );
				++ itElem;
			}
		}
	}

	void LogicGroup::ApplyAngularImpulse( const CDirection& p_vel )
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->ApplyAngularImpulse( p_vel );
				++ itElem;
			}
		}
	}

	CEntity* LogicGroup::GetEntity( ) const
	{
		return m_entity;
	}

	void LogicGroup::Pause( bool )
	{
	}

	//----------------------------------------------------------
	//!
	//!	Set keyframing mode to bodies in group.
	//!
	//----------------------------------------------------------

	void LogicGroup::SetHardKeyframing(bool on)
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->SetHardKeyframing( on );
				++ itElem;
			}
		}		
	}

	//----------------------------------------------------------
	//!
	//!	Set motion type mode to bodies in group.
	//!
	//----------------------------------------------------------
	void LogicGroup::SetMotionType( EMotionType eMotionType )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				body->SetMotionType( eMotionType );
			}
		}
	}

	//----------------------------------------------------------
	//!
	//!	Get motion type mode to bodies in group.
	//!
	//----------------------------------------------------------

	EMotionType LogicGroup::GetMotionType()
	{
		for ( ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{				
				Physics::RigidBody* body = (Physics::RigidBody*) current;				
				return body->GetMotionType() ;
			}
		}
		return HS_MOTION_DYNAMIC; // defualt;
	}
} // Physics

