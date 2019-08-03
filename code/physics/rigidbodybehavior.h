//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/rigidbodybehavior.h
//!	
//!	DYNAMICS COMPONENT:
//!		A set of rigid body behavior.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_RIGID_BODY_BEHAVIOR_INC
#define _DYNAMICS_RIGID_BODY_BEHAVIOR_INC

#include "config.h"

#include "behavior.h"

class CEntity;
class hkRigidBody;

namespace Physics
{
	class CAntiGravityAction;

	// -----------------------------------------------------------------------------------
	//	AntiGravity Behavior.
	//		Apply an anti gravity action.
	// -----------------------------------------------------------------------------------
	class AntiGravity : public Behavior
	{
	private:

		CAntiGravityAction*		m_pobAntiGravityAction;

		bool					m_bMessageSent;

	public:

						AntiGravity( Element* p_element, float p_vel, float p_dur );
		virtual			~AntiGravity( );
		virtual bool	Update( Element* p_element );	
	};

	// -----------------------------------------------------------------------------------
	//	BodyAtRest Behavior.
	//		Generate a message whent the rigid body is at rest.
	// -----------------------------------------------------------------------------------
	class BodyAtRest : public Behavior
	{
	private:

		float			m_fTimeAtRest;

	public:

						BodyAtRest( const ntstd::String& p_message );
		virtual			~BodyAtRest( );
		virtual bool	Update( Element* p_element );	
	};

	// -----------------------------------------------------------------------------------
	//	CheckMoving Behavior.
	//		Generate a message whent the rigid body is moving.
	// -----------------------------------------------------------------------------------
	class CheckMoving : public Behavior
	{
	private:

		float			m_fMovingTimer;

	public:

						CheckMoving( const ntstd::String& p_message );
		virtual			~CheckMoving( );
		virtual bool	Update( Element* p_element );	
	};

	// -----------------------------------------------------------------------------------
	//	DeflectionBehavior Behavior.
	//		Set the body to get its velocity deflected in certain conditions.
	// -----------------------------------------------------------------------------------
	class DeflectionBehavior : public Behavior
	{
	public:

		hkRigidBody*	m_pobBody;

		CEntity*		m_pobParentEntity;
		CEntity*		m_pobLastTarget;

		bool			m_bFirstFrame;
		bool			m_bProcessDeflection;

		CPoint			m_obCurrentPosition;
		CQuat			m_obCurrentRotation;
		CDirection		m_obCurrentLinearVelocity;
		CDirection		m_obCurrentAngularVelocity;

		CPoint			m_obDeflectionIntersect;
		CDirection		m_obDeflectionVelocity;

		int				m_iDeflectionCount;

		float			m_fOldMaxLinearVelocity;

						DeflectionBehavior( );
		virtual			~DeflectionBehavior( );
		virtual bool	Update( Element* p_element );	

		void			ProcessDeflection ();
	};

	// -----------------------------------------------------------------------------------
	//	DeflectionBehavior Behavior.
	//		Behavior for rendering the velocity and deflection lines, needed for gameplay.
	// -----------------------------------------------------------------------------------
	class DeflectionRendererBehavior : public Behavior
	{
	public:
						DeflectionRendererBehavior ();

		virtual bool	Update (Element* p_element);
	};


	// -----------------------------------------------------------------------------------
	//	PiercingBehavior Behavior.
	//		Set the body to orient itself with its own velocity and impale rigid bodies
	//		or characters.
	// -----------------------------------------------------------------------------------
	class PiercingBehavior : public Behavior
	{
	public:

						PiercingBehavior( );
		virtual			~PiercingBehavior( );
		virtual bool	Update( Element* p_element );	
	};

} // Physics
 
#endif // _DYNAMICS_RIGID_BODY_BEHAVIOR_INC
