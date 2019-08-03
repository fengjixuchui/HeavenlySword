//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/interactionlistener.h
//!	
//!	DYNAMICS COMPONENT:
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.19
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_INTERACTION_LISTENER_INC
#define _DYNAMICS_INTERACTION_LISTENER_INC

#include "config.h"
#include "havokincludes.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include <hkmath/hkmath.h>
#include <hkutilities/charactercontrol/characterproxy/hkCharacterProxyListener.h>
#include <hkdynamics/collide/hkCollisionListener.h>

class CEntity;
class hkCharacterProxy;
class hkRigidBody;
class CAttackData;

namespace Physics
{
	class CharacterController;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	class CharacterInteractionListener : public hkCharacterProxyListener
	{
	public:

		CharacterInteractionListener ( CEntity* pobMyEntity,  CharacterController* pobCC );

		void			Update( );
	
		virtual void	objectInteractionCallback(hkCharacterProxy* proxy, const hkCharacterObjectInteractionEvent& input, hkCharacterObjectInteractionResult& output );
		virtual void	characterInteractionCallback(hkCharacterProxy *proxy, hkCharacterProxy *otherProxy, const hkContactPoint &contact); 

		virtual void	processConstraintsCallback (const hkArray< hkRootCdPoint > &manifold, hkSimplexSolverInput &input); 

		virtual void  contactPointAddedCallback (const hkRootCdPoint &point);

	private:

		CEntity*		m_pobEntity;

		hkRigidBody*	m_pobLastCollidee;
		CharacterController*	m_pobCC;
		//AdvancedCharacterController* m_pobCC;

		CAttackData*	m_pobAttackData; // For debris collisions
		CAttackData*	m_pobAttackData2;
		CAttackData*	m_pobAttackData3;
		CAttackData*	m_pobDieData;

		CEntity*		m_pobHitBy;
	};

	class RigidCharacterInteractionListener : public hkCollisionListener
	{
	public:
		RigidCharacterInteractionListener ( CEntity* pobMyEntity,  CharacterController* pobCC );
		void			Update( );	
		virtual void contactPointAddedCallback (hkContactPointAddedEvent& event);
		virtual void contactPointRemovedCallback (hkContactPointRemovedEvent& event)	{};
		virtual void contactProcessCallback (hkContactProcessEvent& event)				{};
		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event){};
	private:

		CEntity*		m_pobEntity;

		hkRigidBody*	m_pobLastCollidee;
		CharacterController*	m_pobCC;

		CAttackData*	m_pobAttackData; // For debris collisions
		CAttackData*	m_pobAttackData2;
		CAttackData*	m_pobAttackData3;
		CAttackData*	m_pobDieData;

		CEntity*		m_pobHitBy;
	};
#endif
}

#endif	// _PS3_RUN_WITHOUT_HAVOK_BUILD
#endif	// _DYNAMICS_INTERACTION_LISTENER_INC
