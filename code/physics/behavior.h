//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/behavior.h
//!	
//!	DYNAMICS COMPONENT:
//!		A behavior is simply a callback acting on an element or a logic group.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_BEHAVIOR_INC
#define _DYNAMICS_BEHAVIOR_INC

#include "config.h"

namespace Physics
{
	class Element;
	class LogicGroup;

	// ---------------------------------------------------------------
	//	A behavior is a simple callback system.
	//	The user is free to inherit from this interface in order to 
	//	write any custom behaviors.
	// ---------------------------------------------------------------
	class Behavior
	{
	public:

		//! Name or message tied to this Behavior.
		ntstd::String		m_id;	

		//! Callbacks. Return true if it's self destroying.
		virtual bool	Update( Element* ) 
		{ 
			return true; 
		}	
		virtual bool	Update( LogicGroup* ) 
		{ 
			return true; 
		}

		//! Virtual destructor.
		virtual			~Behavior( )			
		{}			

		//!< Comparaison operator.
		virtual bool	operator == ( const Behavior& p_event ) const
		{
			return ( m_id == p_event.m_id );
		}
	};

} // Physics
 
#endif // _DYNAMICS_BEHAVIOR_INC
