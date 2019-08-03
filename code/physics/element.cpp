//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/element.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "element.h"
#include "behavior.h"

namespace Physics {
	
	//---------------------------------------------------------------
	//!
	//! Destructor.
	//!
	//---------------------------------------------------------------
	Element::~Element( )	
	{

		for (	ntstd::List<Behavior*>::iterator it = m_behaviorList.begin(); 
				it != m_behaviorList.end(); ++it )
		{
			Behavior* current = (*it);
			NT_DELETE( current );
		}

		m_behaviorList.clear();
	};

	//---------------------------------------------------------------
	//!
	//! Add a behavior.
	//!		\param Behavior& - Behavior.
	//!
	//---------------------------------------------------------------

	void Element::AddBehavior( Behavior* p_behavior )
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

	void Element::RemoveBehavior( const ntstd::String& p_message )
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

	Behavior* Element::GetBehavior( const ntstd::String& p_message )
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

} // Physics
