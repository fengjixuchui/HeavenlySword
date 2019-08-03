#include "Physics/config.h"
#include "Physics/advancedcharactercontroller.h"
#include "Physics/compoundlg.h"
#include "Physics/system.h"
#include "Physics/spearlg.h"
#include "Physics/singlerigidlg.h"
#include "Physics/rigidbodybehavior.h"
#include "Physics/collisionbitfield.h"
#include "physics/collisionlistener.h"
#include "physics/physicstools.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/randmanager.h"
#include "game/movement.h"
#include "game/messagehandler.h" // For LUA collision message generation
#include "game/shellconfig.h"


#include "effect/effect_manager.h"
#include "effect/effect_trigger.h"

#include "audio/collisioneffecthandler.h"

#include "anim/hierarchy.h"

#include "core/visualdebugger.h" // Debugging
//#include "camera/camman_public.h" // Debugging

#include "core/timer.h"

#include "objectdatabase/dataobject.h"


#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkcollide/collector/bodypaircollector/hkAllCdBodyPairCollector.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkutilities/charactercontrol/characterproxy/hkCharacterProxyListener.h>
#include <hkmath/basetypes/hkcontactpoint.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#endif

#include "physics/havokthreadutils.h"
#include "physics/physicsloader.h"

#ifndef _RELEASE

//#define _SHOW_DEBUG_INFO
//#define _DEBUG_CONTACT_INFO // Uncomment this to enable visual debug text for contact state

#endif // _RELEASE

#define 		_PREVENT_DUPLICATE_COLLISIONS

const float		fPROJECTED_VELOCITY_THRESHOLD_FOR_COLLISION_MESSAGE = 0.5f;
const float		fPROJECTED_VELOCITY_THRESHOLD_FOR_EXTERNAL_BOUNCE_EVENT = 5.0f; // Threshold for bounce sound event if object collides with a character phantom

const u_long 	ulBOUNCE_MESSAGE_COLOUR = 0xffccffff;
const u_long 	ulSLIDE_MESSAGE_COLOUR = 0xffffffcc;
const u_long 	ulROLL_MESSAGE_COLOUR = 0xffccffcc;




void ForceLinkFunction11()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction11() !ATTN!\n");
}




namespace Physics
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD


//!-------------------------------------------------------------------
//! 
//! CCollisionListener::CCollisionListener
//! Called after a contact point is created.
//!
//!-------------------------------------------------------------------
CCollisionListener::CCollisionListener( System* pobParentSystem, CEntity* pobParentEntity ) :
	m_eContactState				( CONTACT_NONE ),
	m_bContactMonitor			( false ),
	m_pobParentSystem			( pobParentSystem ),
	m_pobParentEntity			( pobParentEntity ),
	m_pobLastCollidee			( NULL ),
	m_pobCharacterController	( NULL ),
	m_pobRigidBody				( NULL ),
	m_fBounceInterval			( 0.0f ),
	m_bTriggerBounce			( false ),
	m_fSlideTime				( 0.0f ),
	m_bSlideActivated			( false )
{
	ntAssert( pobParentEntity );
	ntAssert_p(m_pobParentSystem, ("Collision listener constructed with an invalid parent physics system!"));

	ClearContactPoints();
}

//!-------------------------------------------------------------------
//! 
//! CCollisionListener::~CCollisionListener
//! Called after a contact point is created.
//!
//!-------------------------------------------------------------------
CCollisionListener::~CCollisionListener ()
{
}

//!-------------------------------------------------------------------
//! 
//! CCollisionListener::RegisterCharacterController
//! Associates a character controller (a sub-member of a character volume dynamics
//!	state), this is needed as the character controller does not use the same method
//!	for detecting collisions as rigid bodies, only handles one at the moment
//!
//!-------------------------------------------------------------------
void CCollisionListener::RegisterCharacterController (Physics::CharacterController* pobCharacterController)
{
	#ifdef _SHOW_DEBUG_INFO
	ntPrintf("CCollisionListener: Registering shape phantom for %s\n",m_pobParentEntity->GetName().c_str());
	#endif

	ntAssert(m_pobParentEntity);

	m_pobCharacterController=pobCharacterController;
}

//!-------------------------------------------------------------------
//! 
//! CCollisionListener::RegisterRigidBody
//! This listner can handle an arbitrary number of rigid bodies that all use this
//! class as a callback when the rigid bodies collide
//!
//!-------------------------------------------------------------------
void CCollisionListener::RegisterRigidBody (hkRigidBody* pobRigidBody)
{
	#ifdef _SHOW_DEBUG_INFO
	ntPrintf("CCollisionListener: Registering rigid body for %s\n",m_pobParentEntity->GetName().c_str());
	#endif

	ntAssert(m_pobParentEntity);
	ntAssert(pobRigidBody);
	
	Physics::WriteAccess mutex( pobRigidBody );
	pobRigidBody->addCollisionListener(this);

	m_pobRigidBody=pobRigidBody;
}

//!-------------------------------------------------------------------
//! 
//! CCollisionListener::Update
//! Called every frame to determine whether characters have collided with other 
//! rigid bodies or other characters
//!
//!-------------------------------------------------------------------
void CCollisionListener::Update ()
{
	m_pobLastCollidee=NULL;

	// ----- Contact Monitor Update -----

	if (m_bContactMonitor && m_pobRigidBody) // Note: The contact monitor needs a valid rigid body
	{
		const float fVELOCITY_THRESHOLD = 0.2f;
		const float fSLIDE_DOTP_THRESHOLD = 0.15f;
		
		m_eContactState=CONTACT_NONE;

		hkVector4 obVelocity(m_pobRigidBody->getLinearVelocity());
		float fVelocitySqrd=obVelocity.lengthSquared3();
		
		if (fVelocitySqrd<fVELOCITY_THRESHOLD) // The rigid body velocity is below our threshold (i.e. not moving fast enough)
		{
			// Since the object is moving so slowly, we will then check to see if the object is in contact with any other rigid body
			for(int i=0; i<_CONTACT_POINT_POOL_SIZE; ++i)
			{
				if (m_obContactPointPool[i].m_pobRB)
				{
					m_eContactState=CONTACT_NORMAL;
					break;
				}
			}
		}
		else // This rigid body is moving at a sufficient speed to check to see if its sliding or rolling
		{
			// Check to see if this rigid body is moving perpendicular to its surface contact
			obVelocity.normalize3();
			for(int i=0; i<_CONTACT_POINT_POOL_SIZE; ++i)
			{
				if (m_obContactPointPool[i].m_pobRB)
				{
					float fDotProduct=obVelocity(0) * m_obContactPointPool[i].m_obNormal(0) + obVelocity(1) * m_obContactPointPool[i].m_obNormal(1) + obVelocity(2) * m_obContactPointPool[i].m_obNormal(2);

					// Req'd for collision effect handler data
					CEntity* pobEntity = (CEntity*)(m_obContactPointPool[i].m_pobRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr());
					System* pobSystem = pobEntity ? pobEntity->GetPhysicsSystem():0;
					if (!pobSystem)
						continue;

					if (fabsf(fDotProduct)<fSLIDE_DOTP_THRESHOLD) // The rigid body is moving parallel (within the threshold) to its contact surface
					{
						// If the rigid body is already considered to be sliding and its angular velocity is higer than its velocity, then we will consider it to be rolling instead of sliding
						hkVector4 obAngularVelocity(m_pobRigidBody->getAngularVelocity());
						float fAngularVelocitySqrd=obAngularVelocity.lengthSquared3();
						if (fAngularVelocitySqrd>fVelocitySqrd)
						{
							m_eContactState=CONTACT_ROLLING;

							// Notify collision effect handler
							CollisionEffectHandler::CollisionEffectHandlerEvent obEvent;
							obEvent.m_eType = CollisionEffectManager::ROLL;
							obEvent.m_fRelevantVelocity = m_pobRigidBody->getAngularVelocity().length3();
							obEvent.m_obCollisionPosition.X() = m_obContactPointPool[i].m_obPosition(0);
							obEvent.m_obCollisionPosition.Y() = m_obContactPointPool[i].m_obPosition(1);
							obEvent.m_obCollisionPosition.Z() = m_obContactPointPool[i].m_obPosition(2);
							obEvent.m_pobOther = pobSystem->GetCollisionEffectHandler();

							// NOTE (chipb) The default collision effect material type is 0 (i.e. any material).
							// While it is possible to override the base material type (from the filter defintion)
							// in the collision effect handler with "any material" this is probably undesirable.
							// So, here if the material set is any, pass the invalid type to the collision effect
							// handler to revert to the type specified on the entity collision effect filter def.
							if (0 != m_obContactPointPool[i].m_uiCollisionEffectMaterial1)
								obEvent.m_uiCustomMaterial1 = m_obContactPointPool[i].m_uiCollisionEffectMaterial1;
							if (0 != m_obContactPointPool[i].m_uiCollisionEffectMaterial2)
								obEvent.m_uiCustomMaterial1 = m_obContactPointPool[i].m_uiCollisionEffectMaterial2;

							m_pobParentSystem->GetCollisionEffectHandler()->ProcessEvent(obEvent);
						}
						// Body is just sliding
						else
						{
							m_eContactState=CONTACT_SLIDING; // We have something that appears to be sliding

							// Notify collision effect handler
							CollisionEffectHandler::CollisionEffectHandlerEvent obEvent;
							obEvent.m_eType = CollisionEffectManager::SLIDE;
							obEvent.m_fRelevantVelocity = m_pobRigidBody->getLinearVelocity().length3();
							obEvent.m_obCollisionPosition.X() = m_obContactPointPool[i].m_obPosition(0);
							obEvent.m_obCollisionPosition.Y() = m_obContactPointPool[i].m_obPosition(1);
							obEvent.m_obCollisionPosition.Z() = m_obContactPointPool[i].m_obPosition(2);
							obEvent.m_pobOther = pobSystem->GetCollisionEffectHandler();

							// See note above
							if (0 != m_obContactPointPool[i].m_uiCollisionEffectMaterial1)
								obEvent.m_uiCustomMaterial1 = m_obContactPointPool[i].m_uiCollisionEffectMaterial1;
							if (0 != m_obContactPointPool[i].m_uiCollisionEffectMaterial2)
								obEvent.m_uiCustomMaterial1 = m_obContactPointPool[i].m_uiCollisionEffectMaterial2;

							m_pobParentSystem->GetCollisionEffectHandler()->ProcessEvent(obEvent);
						}

						// TODO at present bodies are limitied to collision effects for a single contact point
						break;
					}
				}
			}
		}

		// ----- Contact monitor debug -----
		#ifdef _DEBUG_CONTACT_INFO
		g_VisualDebug->Printf3D(m_pobParentEntity->GetPosition(), 0.0f,0.0f, ulROLL_MESSAGE_COLOUR, 0, "Vel:%.2f  AngVel:%.2f",
			(float)m_pobRigidBody->getLinearVelocity().length3(),(float)m_pobRigidBody->getAngularVelocity().length3());

		if (m_bContactMonitor)
		{
			switch(GetContactState())
			{
				case CONTACT_SLIDING:
				{
					g_VisualDebug->Printf3D(m_pobParentEntity->GetPosition(), 0.0f,10.0f, ulROLL_MESSAGE_COLOUR, 0, "%s: Slide",m_pobParentEntity->GetName().c_str());

					break;
				}
			
				case CONTACT_ROLLING:
				{
					g_VisualDebug->Printf3D(m_pobParentEntity->GetPosition(), 0.0f,10.0f, ulROLL_MESSAGE_COLOUR, 0, "%s: Roll",m_pobParentEntity->GetName().c_str());
					break;
				}

				case CONTACT_NORMAL:
				{
					g_VisualDebug->Printf3D(m_pobParentEntity->GetPosition(), 0.0f,10.0f, ulROLL_MESSAGE_COLOUR, 0, "%s: At Rest",m_pobParentEntity->GetName().c_str());
					break;
				}

				default:
				{
					g_VisualDebug->Printf3D(m_pobParentEntity->GetPosition(), 0.0f,10.0f, ulROLL_MESSAGE_COLOUR, 0, "%s: No contact",m_pobParentEntity->GetName().c_str());
					break;
				}
			}
		}
		#endif // _DEBUG_CONTACT_INFO
	}
}



//!-------------------------------------------------------------------
//! 
//! CCollisionListener::contactPointConfirmedCallback
//! Called after a contact point is created.
//!
//!-------------------------------------------------------------------
/*
void CCollisionListener::contactPointConfirmedCallback( hkContactPointConfirmedEvent& event)
{
	static hkUint16 contactId = 0;
	if (!m_bListen) // We are not listening for collisions
		return;

	// Ignore collisions below a certain threshold
	if (event.m_projectedVelocity >= -fPROJECTED_VELOCITY_THRESHOLD_FOR_COLLISION_MESSAGE &&
		event.m_projectedVelocity <= fPROJECTED_VELOCITY_THRESHOLD_FOR_COLLISION_MESSAGE)
	{
		return;
	}

	// Get pointers to entities involved in this event

	CEntity* pobEntityA = 0;
	CEntity* pobEntityB = 0;

	hkRigidBody* obRB	= hkGetRigidBody(&event.m_collidableA);

	if(obRB)
	{
		pobEntityA = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
	}
	else
	{
		hkPhantom* obPH = hkGetPhantom(&event.m_collidableA);
	
		if(obPH)
		{
			pobEntityA = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
		}
	}

	hkRigidBody* obRB2	= hkGetRigidBody(&event.m_collidableB);

	if(obRB2)
	{
		pobEntityB = (CEntity*) obRB2->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
	}
	else
	{
		hkPhantom* obPH2 = hkGetPhantom(&event.m_collidableB);

		if(obPH2)
		{
			pobEntityB = (CEntity*) obPH2->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			
		}
	}


	// Ignore if its a self collision
	if(pobEntityA==m_pobParentEntity && pobEntityB==m_pobParentEntity)
	{
		return;
	}

	// Pointer to the collidee
	CEntity* pobCollidee;

	if(pobEntityA==m_pobParentEntity)
	{
		pobCollidee = pobEntityB;
	}
	else
	{
		pobCollidee = pobEntityA;
	}

	// ----- Physics audio -----

#ifdef _ENABLE_PHYSICS_SOUND

	if (m_pobPhysicsSoundDefinition)
	{
		if (m_fBounceInterval==0.0f && event.m_projectedVelocity>m_pobPhysicsSoundDefinition->m_fBounceMinProjVel && event.m_projectedVelocity>m_fBounceProjVel) // A bounce event has occurred
		{
			m_fBounceInterval=m_pobPhysicsSoundDefinition->GetBounceInterval();
			m_fBounceProjVel=event.m_projectedVelocity;
			m_bTriggerBounce=true;
		}

	}

#endif // _ENABLE_PHYSICS_SOUND

	// Prevent duplicate collisions being registered in the same frame

#ifdef _PREVENT_DUPLICATE_COLLISIONS

	
	if (pobCollidee == m_pobLastCollidee)
	{
		return;
	}
	else
	{
		m_pobLastCollidee=pobCollidee;
	}

#endif // _PREVENT_DUPLICATE_COLLISIONS


#ifdef _ENABLE_COLLISION_MESSAGES

	// ----- Collision event -----

	CCollisionEvent* pobCollisionEvent=m_obCollisionEventPool.Construct();

	// Our collidee
	pobCollisionEvent->m_pobEntity=pobCollidee;

	// Get the point of the collision
	pobCollisionEvent->m_obPosition.X()=event.m_contactPoint->getPosition()(0);
	pobCollisionEvent->m_obPosition.Y()=event.m_contactPoint->getPosition()(1);
	pobCollisionEvent->m_obPosition.Z()=event.m_contactPoint->getPosition()(2);
	pobCollisionEvent->m_obPosition.W()=0.0f;

	// Get the projected velocity of the collision
	pobCollisionEvent->m_fProjectedVelocity=event.m_projectedVelocity;

	// Calculate the collision angle
	if (m_pobRigidBody)
	{
		hkVector4 obVelocity(m_pobRigidBody->getLinearVelocity());

		float fDotProd;

		if (obVelocity.lengthSquared3()>EPSILON)
		{
			obVelocity.normalize3();

			const hkVector4& obNormal=event.m_contactPoint->getNormal();	

			fDotProd=obNormal(0)*obVelocity(0) + obNormal(1)*obVelocity(1) + obNormal(2)*obVelocity(2);

			if (fDotProd<-1.0f)
				fDotProd=-1.0f;
			else if (fDotProd>1.0f)
				fDotProd=1.0f;
		}
		else
		{
			fDotProd=0.0f;
		}

		pobCollisionEvent->m_fDotProd=fDotProd;
	}

	m_obCollisionEventList.push_back(pobCollisionEvent); // Add this collision event to our list

#endif // _ENABLE_COLLISION_MESSAGES
}

*/

//!-------------------------------------------------------------------
//! 
//! CCollisionListener::contactPointAddedCallback
//! Called after a contact point is created.
//!
//!-------------------------------------------------------------------
void CCollisionListener::contactPointAddedCallback (hkContactPointAddedEvent& event)
{
	if (m_bContactMonitor)
	{
		AddContactPoint(event);
	}


	// Ignore collision too deep.
	//if ( fabs( event.m_contactPoint->getDistance() ) > 0.05f )
	//	return;

	//static hkUint16 contactId = 0;


	// Ignore collisions below a certain threshold
	if (event.m_projectedVelocity >= -fPROJECTED_VELOCITY_THRESHOLD_FOR_COLLISION_MESSAGE &&
		event.m_projectedVelocity <= fPROJECTED_VELOCITY_THRESHOLD_FOR_COLLISION_MESSAGE)
	{
		return;
	}

	// Get pointers to entities involved in this event

	CEntity* pobEntityA = 0;
	CEntity* pobEntityB = 0;

	hkRigidBody* obRB	= hkGetRigidBody(event.m_bodyA.getRootCollidable());

	if(obRB)
	{
		pobEntityA = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
	}
	else
	{
		hkPhantom* obPH = hkGetPhantom(event.m_bodyA.getRootCollidable());
	
		if(obPH)
		{
			pobEntityA = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
		}
	}

	hkRigidBody* obRB2	= hkGetRigidBody(event.m_bodyB.getRootCollidable());

	if(obRB2)
	{
		pobEntityB = (CEntity*) obRB2->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
	}
	else
	{
		hkPhantom* obPH2 = hkGetPhantom(event.m_bodyB.getRootCollidable());

		if(obPH2)
		{
			pobEntityB = (CEntity*) obPH2->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			
		}
	}


	// Ignore if its a self collision
	if(pobEntityA==m_pobParentEntity && pobEntityB==m_pobParentEntity)
	{
		return;
	}

	// Pointer to the collidee
	hkRigidBody* pobThisRigidBody;
	hkRigidBody* pobOtherRigidBody;
	CEntity* pobCollidee;
	uint64_t uiMat1 = 0;
	uint64_t uiMat2 = 0;

	if(pobEntityA==m_pobParentEntity)
	{
		pobCollidee = pobEntityB;
		pobThisRigidBody = obRB;
		pobOtherRigidBody = obRB2;

		psPhysicsMaterial* pobMat = Physics::Tools::GetMaterial(event.m_bodyA);
		if (pobMat)
			uiMat1 = pobMat->GetEffectMaterialBit();
		pobMat = Physics::Tools::GetMaterial(event.m_bodyB);
		if (pobMat)
			uiMat2 = pobMat->GetEffectMaterialBit();
	}
	else
	{
		pobCollidee = pobEntityA;
		pobThisRigidBody = obRB2;
		pobOtherRigidBody = obRB;

		psPhysicsMaterial* pobMat = Physics::Tools::GetMaterial(event.m_bodyB);
		if (pobMat)
			uiMat1 = pobMat->GetEffectMaterialBit();
		pobMat = Physics::Tools::GetMaterial(event.m_bodyA);
		if (pobMat)
			uiMat2 = pobMat->GetEffectMaterialBit();
	}

	if (pobCollidee)
	{
		m_pobParentEntity->GetPhysicsSystem()->GetCollisionCallbackHandler()->Process(event,pobCollidee);

		// Collision effect handling
		// NOTE (chipb) Currently subject to the projected velocity threshold early out above - is this okay?
		CollisionEffectHandler::CollisionEffectHandlerEvent obEvent;
		System* pobSystem = pobCollidee->GetPhysicsSystem();
		obEvent.m_eType = event.m_projectedVelocity < 0.0f ? CollisionEffectManager::BOUNCE:CollisionEffectManager::CRASH;
		obEvent.m_fRelevantVelocity = fabs(event.m_projectedVelocity);
		obEvent.m_obCollisionPosition.X() = event.m_contactPoint->getPosition()(0);
		obEvent.m_obCollisionPosition.Y() = event.m_contactPoint->getPosition()(1);
		obEvent.m_obCollisionPosition.Z() = event.m_contactPoint->getPosition()(2);
		obEvent.m_pobOther = pobSystem ? pobSystem->GetCollisionEffectHandler() : 0;

		// NOTE (chipb) The default collision effect material type is 0 (i.e. any material).
		// While it is possible to override the base material type (from the filter defintion)
		// in the collision effect handler with "any material" this is probably undesirable.
		// So, here if the material set is any, pass the invalid type to the collision effect
		// handler to revert to the type specified on the entity collision effect filter def.
		if (0 != uiMat1)
			obEvent.m_uiCustomMaterial1 = uiMat1;
		if (0 != uiMat2)
			obEvent.m_uiCustomMaterial2 = uiMat2;

		m_pobParentSystem->GetCollisionEffectHandler()->ProcessEvent(obEvent);
	}


	
	// Prevent duplicate collisions being registered in the same frame
	/*
	#ifdef _PREVENT_DUPLICATE_COLLISIONS

	if (pobCollidee == m_pobLastCollidee)
	{
		return;
	}
	else
	{
		m_pobLastCollidee=pobCollidee;
	}

	#endif // _PREVENT_DUPLICATE_COLLISIONS
	*/
}

//!-------------------------------------------------------------------
//!
//! CCollisionListener::contactPointConfirmedCallback
//! Called when a new contact point is used for the very first time. 
//!
//!-------------------------------------------------------------------
void CCollisionListener::contactPointConfirmedCallback( hkContactPointConfirmedEvent& event)
{
	//event.m_collidableA
	/*if (event.m_projectedVelocity >= -fPROJECTED_VELOCITY_THRESHOLD_FOR_COLLISION_MESSAGE &&
		event.m_projectedVelocity <= fPROJECTED_VELOCITY_THRESHOLD_FOR_COLLISION_MESSAGE)
	{
		return;
	}*/

	// Get pointers to entities involved in this event

	CEntity* pobEntityA = 0;
	CEntity* pobEntityB = 0;

	hkRigidBody* obRB	= hkGetRigidBody(&event.m_collidableA);

	if(obRB)
	{
		pobEntityA = (CEntity*) obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
	}
	else
	{
		hkPhantom* obPH = hkGetPhantom(&event.m_collidableA);
	
		if(obPH)
		{
			pobEntityA = (CEntity*) obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
		}
	}

	hkRigidBody* obRB2	= hkGetRigidBody(&event.m_collidableB);

	if(obRB2)
	{
		pobEntityB = (CEntity*) obRB2->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
	}
	else
	{
		hkPhantom* obPH2 = hkGetPhantom(&event.m_collidableB);

		if(obPH2)
		{
			pobEntityB = (CEntity*) obPH2->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			
		}
	}

	// Ignore if its a self collision
	if(pobEntityA==m_pobParentEntity && pobEntityB==m_pobParentEntity)
	{
		return;
	}

	// Pointer to the collidee
	hkRigidBody* pobThisRigidBody = 0;
	hkRigidBody* pobOtherRigidBody = 0;
	CEntity* pobCollidee = 0;

	if(pobEntityA==m_pobParentEntity)
	{
		pobCollidee = pobEntityB;
		pobThisRigidBody = obRB;
		pobOtherRigidBody = obRB2;
	}
	else
	{
		pobCollidee = pobEntityA;
		pobThisRigidBody = obRB2;
		pobOtherRigidBody = obRB;
	}

	if( 0 == pobThisRigidBody )
		return;

	if( pobThisRigidBody->getMotionType() == hkMotion::MOTION_KEYFRAMED )
		return;



	// If required, send a message. 

	if( m_pobParentEntity->GetPhysicsSystem()->GetMsgOnCollision() )
	{
		m_pobParentEntity->GetPhysicsSystem()->SetSendCollisionMessageNextUpdate();
	}

	// ----- Rigid body piercing behavior -----

	Physics::SingleRigidLG* pobRigidBodyState = (Physics::SingleRigidLG*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::SINGLE_RIGID_BODY_LG );

	if (pobRigidBodyState && pobRigidBodyState->HasPiercingBehavior() && pobCollidee)
	{
		pobRigidBodyState->SetPiercingBehavior(false);
	}

	/*
	// HC: Disabled piercing behavior against non-character entities

	if (pobRigidBodyState && pobRigidBodyState->HasPiercingBehavior() && pobCollidee && ( !pobCollidee->GetEntityInfo() || !pobCollidee->GetEntityInfo()->IsInvulnerable()) )
	{
		pobRigidBodyState->SetPiercingBehavior(false);

		// Ensure the angle in which it collided with the target is within 45 degrees
		float fDotProd;

		CDirection obTemp(pobRigidBodyState->GetLinearVelocity());

		if (obTemp.LengthSquared()>EPSILON)
		{
			obTemp.Normalise();

			const hkVector4& obNormal=event.m_contactPoint->getNormal();	

			fDotProd=obNormal(0)*obTemp.X() + obNormal(1)*obTemp.Y() + obNormal(2)*obTemp.Z();

			if (fDotProd<-1.0f)
				fDotProd=-1.0f;
			else if (fDotProd>1.0f)
				fDotProd=1.0f;
		}
		else
		{
			fDotProd=0.0f;
		}

		if (fabsf(fDotProd)>0.707f)
		{
			//pobRigidBodyState->MakeKeyframed();

			// Clear movement controllers and disable collision strike handler
			if(pobCollidee->GetMovement())
				pobCollidee->GetMovement()->ClearControllers();
			pobCollidee->GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable();

			Transform* pobThisTransform = m_pobParentEntity->GetHierarchy()->GetRootTransform();
			Transform* pobTargetTransform = pobCollidee->GetHierarchy()->GetRootTransform();

			CMatrix obLocalMatrix(pobThisTransform->GetLocalMatrix());

			// Ensure the position of this object is updated as though it did pierce whatever it hit, based on its current linear velocity
			CPoint obPosition(obLocalMatrix.GetTranslation());
			
			if (pobRigidBodyState->GetLinearVelocity().LengthSquared()<1.0f)
			{
				obPosition+=pobRigidBodyState->GetLinearVelocity() * ((1.0f/30.0f) * 0.4f);
			}
			else
			{
				// This sets the position of the object to be position 30cm relative from the root to the intersection point.
				obPosition.X()=event.m_contactPoint->getPosition()(0);
				obPosition.Y()=event.m_contactPoint->getPosition()(1);
				obPosition.Z()=event.m_contactPoint->getPosition()(2);

				CDirection obDirection(pobRigidBodyState->GetLinearVelocity());
				obDirection.Normalise();
				obDirection*=-0.3f;

				obPosition+=obDirection;
			}

			obLocalMatrix.SetTranslation(obPosition);


			// Recalculate the local matrix for this rigid body so that its in the correct position when parented to the target rigid body				
			obLocalMatrix*=pobTargetTransform->GetWorldMatrix().GetAffineInverse(); // Combine with the affine inverse of the parent
			pobThisTransform->SetLocalMatrix(obLocalMatrix);

			// Reparent this entity and transform to target
			m_pobParentEntity->SetParentEntity( pobCollidee );
			pobThisTransform->RemoveFromParent();
			pobTargetTransform->AddChild( pobThisTransform );

			// Deactivate the rigid body
			pobRigidBodyState->Deactivate();

			// Send a notification
			CMessageSender::SendEmptyMessage( "msg_hitsolid", m_pobParentEntity->GetMessageHandler() );
		}
	}
	*/

	const float fLINEAR_VELOCITY_THRESHOLD = 15.0f * 15.0f;

	if(( obRB->getLinearVelocity().lengthSquared3() < fLINEAR_VELOCITY_THRESHOLD ) && ( obRB2->getLinearVelocity().lengthSquared3() < fLINEAR_VELOCITY_THRESHOLD ) )
		return;

	// ----- Velocity reflection -----
	
	if (pobRigidBodyState)
	{
		Physics::DeflectionBehavior* pobDeflectionBehavior=(Physics::DeflectionBehavior*)pobRigidBodyState->GetDeflectionBehavior();

		if (pobDeflectionBehavior)
			pobDeflectionBehavior->ProcessDeflection();
	}

	// ----- Compound rigid bodies -----

	Physics::CompoundLG* pobState = (Physics::CompoundLG*) this->m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::COMPOUND_RIGID_LG );
	if ( pobState && !pobState->IsCollapsed() ) 
	{
		const float fREQUIRED_PROJ_VEL = 7.0f;
		const float fREQUIRED_LINEAR_VEL = 200.0f;

		bool bCheckMass=false;

        if (pobOtherRigidBody && // We have a valid rigid body collidee
			(pobOtherRigidBody->getMass()>=pobThisRigidBody->getMass() || // Collidee is same mass or heavier
			pobOtherRigidBody->getMotionType()==hkMotion::MOTION_FIXED || // Collidee is fixed motion type
			pobOtherRigidBody->getMotionType()==hkMotion::MOTION_KEYFRAMED)) // Collidee is fixed motion type
			bCheckMass=true;

		if( bCheckMass &&
			(fabs(event.m_projectedVelocity) > fREQUIRED_PROJ_VEL) &&
			(pobThisRigidBody->getLinearVelocity().lengthSquared3() > fREQUIRED_LINEAR_VEL))
		{
			//ntPrintf("projected velocity=%f\n",event.m_projectedVelocity);
			//ntPrintf("%s collapsing on collision with %s\n",m_pobParentEntity->GetName().c_str(),pobCollidee->GetName().c_str());
			//ntPrintf("%f \n", pobThisRigidBody->getLinearVelocity().lengthSquared3());

			Message CollapseMsg(msg_hitsolid);
			m_pobParentEntity->GetMessageHandler()->QueueMessage(CollapseMsg);

			SetContactMonitor(false); // Turn off the contact monitor
		}
	}

	// ----- Spear behaviour -----

	/*
	Physics::SpearLG* pobState2 = (Physics::SpearLG*) this->m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::SPEAR_LG );
	if( pobState2 )
	{
		//if( pobState2->IsCollapsed() == false )
		if( pobState2->isThrowned )
		{
			//pobState->Collapse();

			CCollisionEvent* pobCollisionEvent=GetNewCollisionEvent();
			
			if( pobCollisionEvent )
			{
				// Our collidee
				pobCollisionEvent->m_pobEntity=pobCollidee;

				// Get the point of the collision
				pobCollisionEvent->m_obPosition.X()=event.m_contactPoint->getPosition()(0);
				pobCollisionEvent->m_obPosition.Y()=event.m_contactPoint->getPosition()(1);
				pobCollisionEvent->m_obPosition.Z()=event.m_contactPoint->getPosition()(2);
				pobCollisionEvent->m_obPosition.W()=0.0f;

				// Get the projected velocity of the collision
				pobCollisionEvent->m_fProjectedVelocity=10000.f;

				// Calculate the collision angle
				#if 0
				if (m_pobRigidBody)
				{
					hkVector4 obVelocity(pobThisRigidBody->getLinearVelocity());

					float fDotProd;

					if (obVelocity.lengthSquared3()>EPSILON)
					{
						obVelocity.normalize3();

						const hkVector4& obNormal=event.m_contactPoint->getNormal();	

						fDotProd=obNormal(0)*obVelocity(0) + obNormal(1)*obVelocity(1) + obNormal(2)*obVelocity(2);

						if (fDotProd<-1.0f)
							fDotProd=-1.0f;
						else if (fDotProd>1.0f)
							fDotProd=1.0f;
					}
					else
					{
						fDotProd=0.0f;
					}

					pobCollisionEvent->m_fDotProd=fDotProd;
				}
				#endif

				m_obCollisionEventList.push_back(pobCollisionEvent); 
			}
			else
			{
				#ifndef _RELEASE
				ntPrintf("Error: allocating a new collision event\n" );
				#endif 
			}
		}
	}
	*/

}

//!-------------------------------------------------------------------
//!
//! CCollisionListener::contactPointRemovedCallback
//! Called before a contact point gets removed. 
//!
//!-------------------------------------------------------------------
void CCollisionListener::contactPointRemovedCallback (hkContactPointRemovedEvent& event)
{
	if (m_bContactMonitor)
	{
		RemoveContactPoint(event);
	}
}

//!-------------------------------------------------------------------
//!
//! CCollisionListener::contactPointRemovedCallback
//! Called just after all collision detection between 2 rigid bodies has been executed.
//!
//!-------------------------------------------------------------------
void CCollisionListener::contactProcessCallback (hkContactProcessEvent& /*event*/)
{
}

//!-------------------------------------------------------------------
//!
//! CCollisionListener::ClearContactPoints
//! Clear our contact tracking list.
//!
//!-------------------------------------------------------------------
void CCollisionListener::ClearContactPoints ()
{
	//ntPrintf("ClearContactPoints\n");

	for(int i=0; i<_CONTACT_POINT_POOL_SIZE; ++i)
	{
		m_obContactPointPool[i].m_pobRB=0;
		m_obContactPointPool[i].m_iManifoldContacts=0;
		m_obContactPointPool[i].m_uiCollisionEffectMaterial1 = 0;
		m_obContactPointPool[i].m_uiCollisionEffectMaterial2 = 0;
	}	
}

//!-------------------------------------------------------------------
//!
//! CCollisionListener::AddContactPoint
//! Adds/updates manifold type contact points to our tracking list.
//!
//!-------------------------------------------------------------------
void CCollisionListener::AddContactPoint (hkContactPointAddedEvent& event)
{
	if (event.isToi()) // Reject this event if it's not a manifold event
		return;

	// Get a pointer to the collidee rigid body
	hkRigidBody* pobRB=0;
	hkRigidBody* pobRB1=hkGetRigidBody(event.m_bodyA.getRootCollidable());
	hkRigidBody* pobRB2=hkGetRigidBody(event.m_bodyB.getRootCollidable());
	uint64_t uiMat1 = 0;
	uint64_t uiMat2 = 0;

	if (pobRB1!=m_pobRigidBody)
	{
		pobRB=pobRB1;
		psPhysicsMaterial* pobMat = Physics::Tools::GetMaterial(event.m_bodyA);
		if (pobMat)
			uiMat1 = pobMat->GetEffectMaterialBit();
		pobMat = Physics::Tools::GetMaterial(event.m_bodyB);
		if (pobMat)
			uiMat2 = pobMat->GetEffectMaterialBit();
	}
	else if (pobRB2!=m_pobRigidBody)
	{
		pobRB=pobRB2;
		psPhysicsMaterial* pobMat = Physics::Tools::GetMaterial(event.m_bodyB);
		if (pobMat)
			uiMat1 = pobMat->GetEffectMaterialBit();
		pobMat = Physics::Tools::GetMaterial(event.m_bodyA);
		if (pobMat)
			uiMat2 = pobMat->GetEffectMaterialBit();
	}

	if (pobRB) // Make sure we have a valid rigid body
	{
		// Normal and position
		hkVector4 obNormal(event.m_contactPoint->getNormal());
		hkVector4 obPos(event.m_contactPoint->getPosition());

		// Check to see if this rigid body is already touching, if it is update the contact normal and manifold contact counter
		for(int i=0; i<_CONTACT_POINT_POOL_SIZE; ++i)
		{
			if (pobRB==m_obContactPointPool[i].m_pobRB)
			{
				m_obContactPointPool[i].m_obNormal=obNormal;
				m_obContactPointPool[i].m_obPosition=obPos;
				++m_obContactPointPool[i].m_iManifoldContacts;
				m_obContactPointPool[i].m_uiCollisionEffectMaterial1 = uiMat1;
				m_obContactPointPool[i].m_uiCollisionEffectMaterial2 = uiMat2;

				//ntPrintf("AddContactPoint -> %x : %d\n",pobRB,m_obContactPointPool[i].m_iManifoldContacts);
				return;
			}
		}

		// This rigid body wasn't touching before so add it
		for(int i=0; i<_CONTACT_POINT_POOL_SIZE; ++i)
		{
			if (m_obContactPointPool[i].m_pobRB==0) // Find a free slot
			{
				m_obContactPointPool[i].m_pobRB=pobRB;
				m_obContactPointPool[i].m_obNormal=obNormal;
				m_obContactPointPool[i].m_obPosition=obPos;
				m_obContactPointPool[i].m_iManifoldContacts=1;
				m_obContactPointPool[i].m_uiCollisionEffectMaterial1 = uiMat1;
				m_obContactPointPool[i].m_uiCollisionEffectMaterial2 = uiMat2;

				//ntPrintf("AddContactPoint -> %x : %d\n",pobRB,m_obContactPointPool[i].m_iManifoldContacts);
				return;
			}
		}
	}
}

//!-------------------------------------------------------------------
//!
//! CCollisionListener::AddContactPoint
//! Removes instances of manifold contact points from our tracking list.
//!
//!-------------------------------------------------------------------
void CCollisionListener::RemoveContactPoint (hkContactPointRemovedEvent& event)
{
	if (event.isToi()) // Reject this event if it's not a manifold event
		return;

	// Get the pointer of the rigid body
	hkRigidBody* pobRB=0;

	if (event.m_entityA!=m_pobRigidBody)
	{
		pobRB=(hkRigidBody*)event.m_entityA;
	}
	else if (event.m_entityB!=m_pobRigidBody)
	{
		pobRB=(hkRigidBody*)event.m_entityB;
	}

	// We have a valid rigid body
	if (pobRB)
	{
		for(int i=0; i<_CONTACT_POINT_POOL_SIZE; ++i)
		{
			if (pobRB==m_obContactPointPool[i].m_pobRB) // Find the matching contact point entry
			{
				--m_obContactPointPool[i].m_iManifoldContacts; // Decrement the number of manifold contacts

				//ntPrintf("RemoveContactPoint -> %x : %d\n",pobRB,m_obContactPointPool[i].m_iManifoldContacts);

				if (m_obContactPointPool[i].m_iManifoldContacts<1) // This rigid body is no longer touching
				{
					m_obContactPointPool[i].m_pobRB=0; // Remove this contact point from the list
				}

				return;
			}
		}
	}
}


//----- World Collision Listener -------------------------------------------------------------------------------------------------------------------------------------------------------


//!-------------------------------------------------------------------
//!
//! CWorldCollisionListener::contactPointAddedCallback
//! Called after a contact point is created.
//! Changes restitution and friction according to materials (if they are assigned to shapes)
//!
//!-------------------------------------------------------------------
void CWorldCollisionListener::contactPointAddedCallback (hkContactPointAddedEvent& event)
{
	// All meshes are singlesided
	if (g_ShellOptions->m_bDoOneSideCollision)
	{	
		if (event.m_bodyA.getShape()->getType() == HK_SHAPE_TRIANGLE)
		{
			EntityCollisionFlag info;
			info.base = event.m_bodyA.getRootCollidable()->getCollisionFilterInfo();

			if ((info.flags.i_am & AI_WALL_BIT) == 0) // Invisible Walls can be one or double sided, let them to decide
			{
				const hkTriangleShape * triangle = static_cast<const hkTriangleShape*>(event.m_bodyA.getShape());

				if (Tools::CalcTriangleNormal(triangle->getVertices()).dot3(event.m_contactPoint->getNormal()) < 0)
				{
					event.m_status = HK_CONTACT_POINT_REJECT;
					return;
				}
			}
		}

		if (event.m_bodyB.getShape()->getType() == HK_SHAPE_TRIANGLE)
		{
			EntityCollisionFlag info;
			info.base = event.m_bodyB.getRootCollidable()->getCollisionFilterInfo();

			if ((info.flags.i_am & AI_WALL_BIT) == 0) // Invisible Walls can be one or double sided, let them to decide
			{
				const hkTriangleShape * triangle = static_cast<const hkTriangleShape*>(event.m_bodyB.getShape());

				if (Tools::CalcTriangleNormal(triangle->getVertices()).dot3(event.m_contactPoint->getNormal()) > 0)
				{
					event.m_status = HK_CONTACT_POINT_REJECT;
					return;
				}
			}
		}
	}

	// find materials
	psPhysicsMaterial * materialA = Physics::Tools::GetMaterial(event.m_bodyA);
	psPhysicsMaterial * materialB = Physics::Tools::GetMaterial(event.m_bodyB);
	
	float fFrictionA;
	float fRestitutionA; 		
	if (materialA == INVALID_MATERIAL || !materialA->QHasValidFrictionAndRestitution())
	{
		if (materialB == INVALID_MATERIAL || !materialB->QHasValidFrictionAndRestitution()) // no material defined on shapes, do not change anything
			return;

		// take material from hkEntity
		const hkMaterial& material = static_cast<hkEntity *>(event.m_bodyA.getRootCollidable()->getOwner())->getMaterial();
		fFrictionA = material.getFriction();
		fRestitutionA = material.getRestitution();
	}
	else
	{
		fFrictionA = materialA->GetFriction();
		fRestitutionA = materialA->GetRestitution();
	}

	float fFrictionB;
	float fRestitutionB;

	if (materialB == INVALID_MATERIAL || !materialB->QHasValidFrictionAndRestitution())
	{
		// take material from hkEntity 
		const hkMaterial& material = static_cast<hkEntity *>(event.m_bodyB.getRootCollidable()->getOwner())->getMaterial();
		fFrictionB = material.getFriction();
		fRestitutionB = material.getRestitution();
	}
	else
	{
		fFrictionB = materialB->GetFriction();
		fRestitutionB = materialB->GetRestitution();
	}

	/// create average value, use geometric average
	float fFriction = fsqrtf(fFrictionA * fFrictionB );
	float fRestitution = fsqrtf(fRestitutionA * fRestitutionB );

	event.m_contactPointMaterial->setFriction(fFriction);
	event.m_contactPointMaterial->setRestitution(fRestitution);
}



#endif // _PS3_RUN_WITHOUT_HAVOK_BUILD
} // namespace Physics


