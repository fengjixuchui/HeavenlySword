//--------------------------------------------------
//!
//!	\file collisionlistener.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _COLLISIONLISTENER_H
#define _COLLISIONLISTENER_H

#include "Physics/config.h"
#include "physics/havokincludes.h"
#include "physics/physicsmaterial.h"

class CEntity;
class hkRigidBody;
struct hkCharacterObjectInteractionEvent;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/collide/hkCollisionListener.h>
#endif
namespace Physics
{
	class CVolumeData;
	class CharacterController;
	class System;


	//--------------------------------------------------
	//!
	//!	ContactPoint.
	//!	Long Class Description
	//!	Exciting class with lots of stuff to describe
	//!
	//--------------------------------------------------

	class ContactPoint
	{
	public:

		#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ContactPoint(void)
		:
			m_pobRB(0),
			m_iManifoldContacts(0),
			m_uiCollisionEffectMaterial1(0),
			m_uiCollisionEffectMaterial2(0)
		{ }
		hkRigidBody* m_pobRB;
		hkVector4 m_obNormal;
		hkVector4 m_obPosition;
		int m_iManifoldContacts;
		uint64_t m_uiCollisionEffectMaterial1;
		uint64_t m_uiCollisionEffectMaterial2;
		#endif
	};


#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	//--------------------------------------------------
	//!
	//!	CCollisionListener.
	//!	Long Class Description
	//!	Exciting class with lots of stuff to describe
	//!
	//--------------------------------------------------

	enum CONTACT_STATE
	{
		CONTACT_NONE,
		CONTACT_NORMAL,
		CONTACT_SLIDING,
		CONTACT_ROLLING,
	};

	class CCollisionListener : public hkCollisionListener
	{
	public:

		CCollisionListener (System *pobParentSystem, CEntity* pobParentEntity);
		~CCollisionListener ();

		// Registration of havok bodies with listener
		void RegisterCharacterController (CharacterController* pobCharacterController);
		void RegisterRigidBody (hkRigidBody* pobRigidBody);

		void SetContactMonitor (bool bEnable) { m_bContactMonitor=bEnable; } // This activates/deactivates the contact state monitoring

		// Runtime stuff
		void Update ();

		// Callbacks for the listeners (derived from hkCollisionListener)
		virtual void contactPointAddedCallback (hkContactPointAddedEvent& event);
		virtual void contactPointRemovedCallback (hkContactPointRemovedEvent& event);
		virtual void contactProcessCallback (hkContactProcessEvent& event);
		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event);

		CONTACT_STATE GetContactState () { return m_eContactState; } // Fetch the contact state of this rigid body

	protected:

		void ClearContactPoints ();
		void AddContactPoint (hkContactPointAddedEvent& event);
		void RemoveContactPoint (hkContactPointRemovedEvent& event);

		static const int _CONTACT_POINT_POOL_SIZE = 8;

		ContactPoint					m_obContactPointPool[ _CONTACT_POINT_POOL_SIZE ];

		CONTACT_STATE					m_eContactState;

		bool							m_bContactMonitor; // Determines whether manifold contact points are tracked (needed for slide/roll to work)

		System*							m_pobParentSystem;
		CEntity* 						m_pobParentEntity;
		CEntity* 						m_pobLastCollidee;

		Physics::CharacterController*	m_pobCharacterController;

		hkRigidBody*					m_pobRigidBody;

		//CAudioObjectID				m_obSlideSoundID;
		float							m_fBounceInterval;
		float							m_fBounceProjVel;
		bool							m_bTriggerBounce;
		float							m_fSlideTime;
		bool							m_bSlideActivated;

	};

	//--------------------------------------------------
	//!
	//!	CWorldCollisionListener.	 
	//! 
	//! Collision listener for all collisions. Will handle physical materials etc
	//!
	//--------------------------------------------------

	class CWorldCollisionListener : public hkCollisionListener
	{
	public:
		// Callbacks for the listeners (derived from hkCollisionListener)
		virtual void contactPointAddedCallback (hkContactPointAddedEvent& event);
		virtual void contactPointRemovedCallback (hkContactPointRemovedEvent& event) {};
		virtual void contactProcessCallback (hkContactProcessEvent& event) {};
		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event) {};
	};


#endif
}

#endif // _COLLISIONLISTENER_H
