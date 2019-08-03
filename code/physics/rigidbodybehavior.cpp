//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/rigidbodybehavior.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "rigidbodybehavior.h"
#include "element.h"
#include "rigidbody.h"
#include "maths_tools.h"
#include "antigravityaction.h"

#include "physics/havokincludes.h"
#include "physics/world.h"
#include "physics/havokthreadutils.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/entity/hkRigidBody.h>
#endif

//#include "ai/aiattackselection.h"
#include "game/attacks.h"

#include "core/timer.h"
#include "core/visualdebugger.h"

#include "game/messagehandler.h"
#include "game/entity.h"
#include "game/query.h" // For deflection stuff
#include "game/entitymanager.h" // For deflection stuff
#include "game/entityinfo.h"

#include "camera/camman.h" // Deflection stuff
#include "camera/camview.h"

#include "anim/transform.h"
#include "anim/hierarchy.h"

#include "camera/camutils.h"

#ifndef _RELEASE

//#define _DEBUG_DEFLECTION

#endif // _RELEASE

const int iDEFLECTION_LIMIT = 2;


namespace Physics {

	static void applyHardKeyFrameAsynchronously( const hkVector4& nextPosition, const hkQuaternion& nextOrientation, hkReal invDeltaTime, hkRigidBody* body)
	{
		hkReal now = body->getWorld()->getCurrentTime();
		hkTransform transformNow; 
		body->getRigidMotion()->approxTransformAt(now, transformNow);
		hkVector4 centerShift;
		centerShift._setRotatedDir( transformNow.getRotation(), body->getRigidMotion()->getMotionState()->getSweptTransform().m_centerOfMassLocal);
		//transformOut.getTranslation().sub4( centerShift );
		hkVector4 centerOfMassNow; centerOfMassNow.setAdd4( transformNow.getTranslation(), centerShift );

		// Get lin vel required
		/*
		{
			hkVector4 linearVelocity;

			hkVector4 newCenterOfMassPosition;
			newCenterOfMassPosition.setRotatedDir( nextOrientation, body->getCenterOfMassLocal() );
			newCenterOfMassPosition.add4( nextPosition );
			linearVelocity.setSub4( newCenterOfMassPosition, centerOfMassNow );

			linearVelocity.setMul4(invDeltaTime, linearVelocity);

			body->setLinearVelocity(linearVelocity);
		}
		*/

		// Get ang vel required
		{
			hkVector4 angularVelocity;
			hkQuaternion quatDif;
			hkQuaternion rotationNow; rotationNow.set(transformNow.getRotation());
			quatDif.setMulInverse(nextOrientation, rotationNow);
			quatDif.normalize();

			hkReal angle = quatDif.getAngle();
			if(angle < 1e-3f)
			{
				angularVelocity.setZero4();
			}
			else
			{
				quatDif.getAxis(angularVelocity);
				angularVelocity.setMul4(angle * invDeltaTime, angularVelocity);		
			}	
			body->setAngularVelocity(angularVelocity);
		}
	}

	AntiGravity::AntiGravity( Element* p_element, float p_vel, float p_dur )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( p_element->GetType() == Element::RIGID_BODY );

		m_id = ntstd::String( "ANTI_GRAV" );
		
		RigidBody*		elem = (RigidBody*) p_element;
		hkRigidBody*	body = elem->GetHkRigidBody();

		Physics::WriteAccess mutex;

		m_pobAntiGravityAction = HK_NEW CAntiGravityAction( body, p_vel, p_dur);
		CPhysicsWorld::Get().AddAction(m_pobAntiGravityAction);

		m_bMessageSent=false;
#else
		UNUSED( p_element ) ;
		UNUSED( p_vel ) ;
		UNUSED( p_dur ) ;
#endif
	}

	AntiGravity::~AntiGravity( )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if (m_pobAntiGravityAction)
		{
			if (m_pobAntiGravityAction->getWorld())
			{
				CPhysicsWorld::Get().RemoveAction( m_pobAntiGravityAction );
				//m_pobAntiGravityAction->removeReference();
				m_pobAntiGravityAction=0;
			}
			else
			{
				m_pobAntiGravityAction->removeReference();
			}
		}
#endif
	}

	bool	AntiGravity::Update( Element* p_element )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if ( m_pobAntiGravityAction->m_fNoGravityDuration <= 0.0f )
		{
			if (!m_bMessageSent)
			{
				RigidBody* elem = (RigidBody*) p_element;
				CMessageSender::SendEmptyMessage("msg_antigrav_off",elem->GetEntity()->GetMessageHandler_const()); // Send a message back to the entity to let it know the anti-grav has finished
				m_bMessageSent=true;
			}

			if ( m_pobAntiGravityAction->m_fRestoreDuration <=0.0f )
			{
				return true;
			}

		}
#endif

		return false;
	}

	//---------------------------------------------------------------
	//!
	//! BodyAtRest constructor.
	//!		\param ntstd::String& - message to generate.
	//!
	//---------------------------------------------------------------

	BodyAtRest::BodyAtRest( const ntstd::String& p_message ):
		m_fTimeAtRest( 0.0f )
	{
		m_id = p_message;
	}

	//---------------------------------------------------------------
	//!
	//! BodyAtRestBehavior destructor.
	//!
	//---------------------------------------------------------------

	BodyAtRest::~BodyAtRest()			
	{ }

	static const float fATREST_ANGULAR_SCALE = 1.0f;
	static const float fATREST_SPEED_THRESHOLD = 5.0f * 5.0f;
	static const float fTIME_AT_REST_DURATION = 0.75f;

	//---------------------------------------------------------------
	//!
	//! BodyAtRest event callback.
	//!		\param Element* - element to check.
	//!
	//---------------------------------------------------------------

	bool	BodyAtRest::Update( Element* p_element )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( p_element );
		ntAssert( p_element->GetType() == Element::RIGID_BODY );
		
		RigidBody*		elem = (RigidBody*) p_element;
		hkRigidBody*	body = elem->GetHkRigidBody();

		bool	hasComeToRest = false;
		float	fOverallSpeed = body->getLinearVelocity().lengthSquared3();
		fOverallSpeed += body->getAngularVelocity().lengthSquared3() * (hkSimdReal) fATREST_ANGULAR_SCALE;
		if( fOverallSpeed < fATREST_SPEED_THRESHOLD )
		{
			hasComeToRest = true;
		}
#else
		RigidBody*		elem = (RigidBody*) p_element;
		bool	hasComeToRest = true;
#endif
		if( hasComeToRest )
		{
			m_fTimeAtRest += CTimer::Get().GetGameTimeChange();

			if( m_fTimeAtRest > fTIME_AT_REST_DURATION )
			{
				CMessageSender::SendEmptyMessage( m_id.c_str(), elem->GetEntity()->GetMessageHandler() );
				return true;
			}
		}

		return false;
	}		

	//---------------------------------------------------------------
	//!
	//! CheckMoving constructor.
	//!		\param ntstd::String& - message to generate.
	//!
	//---------------------------------------------------------------

	CheckMoving::CheckMoving( const ntstd::String& p_message ):
		m_fMovingTimer( 0.0f )
	{
		m_id = p_message;
	}

	//---------------------------------------------------------------
	//!
	//! CheckMovingBehavior destructor.
	//!
	//---------------------------------------------------------------

	CheckMoving::~CheckMoving()			
	{ }

	static const float fMOVING_DURATION = 1.0f;
	static const float fMOVING_SPEED_THRESHOLD = 2.0f;

	//---------------------------------------------------------------
	//!
	//! BodyAtRest event callback.
	//!		\param Element* - element to check.
	//!
	//---------------------------------------------------------------

	bool	CheckMoving::Update( Element* p_element )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( p_element );
		ntAssert( p_element->GetType() == Element::RIGID_BODY );
		
		RigidBody*		elem = (RigidBody*) p_element;
		hkRigidBody*	body = elem->GetHkRigidBody();

		const CVector obVelocity = Physics::MathsTools::hkVectorToCVector( body->getLinearVelocity() );

		if ( obVelocity.LengthSquared() < fMOVING_SPEED_THRESHOLD )
		{
			m_fMovingTimer += CTimer::Get().GetGameTimeChange();
			
			if( m_fMovingTimer > fMOVING_DURATION )
			{
				CMessageSender::SendEmptyMessage( m_id.c_str(), elem->GetEntity()->GetMessageHandler() );

				return true;
			}
		}
#else
		UNUSED ( p_element ) ;
#endif
		return false;
	}		
	
	//---------------------------------------------------------------
	//!
	//! DeflectionBehavior constructor.
	//!
	//---------------------------------------------------------------

	DeflectionBehavior::DeflectionBehavior( ) :
		m_pobBody(0),
		m_pobParentEntity(0),
		m_pobLastTarget(0),
		m_bFirstFrame(true),
		m_bProcessDeflection(false),
		m_obCurrentPosition(CONSTRUCT_CLEAR),
		m_obCurrentRotation(CONSTRUCT_CLEAR),
		m_obCurrentLinearVelocity(CONSTRUCT_CLEAR),
		m_obCurrentAngularVelocity(CONSTRUCT_CLEAR),
		m_obDeflectionIntersect(CONSTRUCT_CLEAR),
		m_obDeflectionVelocity(CONSTRUCT_CLEAR),
		m_iDeflectionCount(0),
		m_fOldMaxLinearVelocity(0.0f)
	{
		m_id = ntstd::String( "DEFLECT" );

#ifdef _DEBUG_DEFLECTION
		ntPrintf("### Deflection -> Construction\n");
#endif // _DEBUG_DEFLECTION
	}

	//---------------------------------------------------------------
	//!
	//! DeflectionBehavior destructor.
	//!
	//---------------------------------------------------------------

	DeflectionBehavior::~DeflectionBehavior()
	{
#ifdef _DEBUG_DEFLECTION
		ntPrintf("### Deflection -> Destruction\n");
#endif // _DEBUG_DEFLECTION

		if (m_fOldMaxLinearVelocity!=0.0f)
		{
			m_pobBody->setMaxLinearVelocity(m_fOldMaxLinearVelocity); // Restore max linear velocity for this body to its default value
		}
	}

	//---------------------------------------------------------------
	//!
	//! DeflectionBehavior event callback.
	//!		\param Element* - element to check.
	//!
	//---------------------------------------------------------------

	bool	DeflectionBehavior::Update( Element* p_element )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		//if (m_iDeflectionCount>=iDEFLECTION_LIMIT || !p_element || p_element->GetType()!=Element::RIGID_BODY) // Check to see if the element is valid...
		if (!p_element || p_element->GetType()!=Element::RIGID_BODY) // Check to see if the element is valid...
			return false;

		if (m_bFirstFrame)
		{
			#ifdef _DEBUG_DEFLECTION
			ntPrintf("### Deflection (%f) -> First frame\n",Physics::CPhysicsWorld::Get().GetFrameTime());
			#endif // _DEBUG_DEFLECTION
		
		RigidBody*		elem = (RigidBody*) p_element;
		m_pobBody = elem->GetHkRigidBody();
			m_pobParentEntity=(CEntity*)elem->GetEntity();

			Physics::WriteAccess mutex( m_pobBody );

			// Place a temporary limit on the velocity of this rigid body, so that it doesn't gain velocity on collisions
			m_fOldMaxLinearVelocity=m_pobBody->getMaxLinearVelocity();
			m_pobBody->setMaxLinearVelocity(m_pobBody->getLinearVelocity().length3()); 

			//ntPrintf("Initial speed=%f  Initial angular velocity=%f,%f,%f\n",m_fSpeed,m_obInitialAngularVelocity.X(),m_obInitialAngularVelocity.Y(),m_obInitialAngularVelocity.Z());

			m_bFirstFrame=false;
		}
		else
		{
			Physics::WriteAccess mutex( m_pobBody );
		}

		if (m_bProcessDeflection)
		{
			CDirection obTemp(m_obDeflectionIntersect - m_obCurrentPosition);

			CDirection obOffset(m_obDeflectionVelocity);
			obOffset.Normalise();
			obOffset*=obTemp.Length();

			CPoint obNewPosition(m_obDeflectionIntersect + obOffset);

			//m_pobBody->setPosition(hkVector4(obNewPosition.X(),obNewPosition.Y(),obNewPosition.Z()));
			m_pobBody->setRotation(hkQuaternion(m_obCurrentRotation.X(),m_obCurrentRotation.Y(),m_obCurrentRotation.Z(),m_obCurrentRotation.W()));

			m_pobBody->setLinearVelocity(hkVector4(m_obDeflectionVelocity.X(),m_obDeflectionVelocity.Y(),m_obDeflectionVelocity.Z()));
			m_pobBody->setAngularVelocity(hkVector4(m_obCurrentAngularVelocity.X(),m_obCurrentAngularVelocity.Y(),m_obCurrentAngularVelocity.Z()));

			m_bProcessDeflection=false;
			++m_iDeflectionCount;

			#ifdef _DEBUG_DEFLECTION
			ntPrintf("### Deflection (%f) -> Rigidbody update: Deflection vel=%f %f %f\n",
				Physics::CPhysicsWorld::Get().GetFrameTime(),
				m_obDeflectionVelocity.X(),m_obDeflectionVelocity.Y(),m_obDeflectionVelocity.Z());
			#endif // _DEBUG_DEFLECTION
		}

		// Store the current state of the rigid body
		m_obCurrentPosition.X()=m_pobBody->getPosition()(0);
		m_obCurrentPosition.Y()=m_pobBody->getPosition()(1);
		m_obCurrentPosition.Z()=m_pobBody->getPosition()(2);

		m_obCurrentRotation.X()=m_pobBody->getRotation()(0);
		m_obCurrentRotation.Y()=m_pobBody->getRotation()(1);
		m_obCurrentRotation.Z()=m_pobBody->getRotation()(2);
		m_obCurrentRotation.W()=m_pobBody->getRotation()(3);

		m_obCurrentLinearVelocity.X()=m_pobBody->getLinearVelocity()(0);
		m_obCurrentLinearVelocity.Y()=m_pobBody->getLinearVelocity()(1);
		m_obCurrentLinearVelocity.Z()=m_pobBody->getLinearVelocity()(2);

		m_obCurrentAngularVelocity.X()=m_pobBody->getAngularVelocity()(0);
		m_obCurrentAngularVelocity.Y()=m_pobBody->getAngularVelocity()(1);
		m_obCurrentAngularVelocity.Z()=m_pobBody->getAngularVelocity()(2);

		// Calculate the deflection vector

		TRACE_LINE_QUERY stQuery;

		Physics::RaycastCollisionFlag obFlag;
		obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
										Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
										Physics::RAGDOLL_BIT						|
										Physics::SMALL_INTERACTABLE_BIT				|
										Physics::LARGE_INTERACTABLE_BIT				);

		if (CPhysicsWorld::Get().TraceLine( m_obCurrentPosition, m_obCurrentPosition + m_obCurrentLinearVelocity, m_pobParentEntity, stQuery, obFlag ))
		{
			m_obDeflectionIntersect=stQuery.obIntersect;

			const CDirection& obNormal = stQuery.obNormal;

			float fF = 2.0f * ( obNormal.X() * m_obCurrentLinearVelocity.X() + obNormal.Y() * m_obCurrentLinearVelocity.Y() + obNormal.Z() * m_obCurrentLinearVelocity.Z() );
				
			m_obDeflectionVelocity.X() = m_obCurrentLinearVelocity.X() - fF * obNormal.X();
			m_obDeflectionVelocity.Y() = m_obCurrentLinearVelocity.Y() - fF * obNormal.Y();
			m_obDeflectionVelocity.Z() = m_obCurrentLinearVelocity.Z() - fF * obNormal.Z();
		}
		else
		{
			m_obDeflectionIntersect=m_obCurrentPosition;
			m_obDeflectionVelocity=m_obCurrentLinearVelocity;// * -1.0f;
		}

		#ifdef _DEBUG_DEFLECTION
		ntPrintf("### Deflection (%f) -> Update: Velocity=%f %f %f  Deflection vel=%f %f %f\n",
			Physics::CPhysicsWorld::Get().GetFrameTime(),
			m_obCurrentLinearVelocity.X(),m_obCurrentLinearVelocity.Y(),m_obCurrentLinearVelocity.Z(),
			m_obDeflectionVelocity.X(),m_obDeflectionVelocity.Y(),m_obDeflectionVelocity.Z());
#endif // _DEBUG_DEFLECTION
		
#else
		UNUSED( p_element );
#endif
		return false;
	}	

	void DeflectionBehavior::ProcessDeflection ()
	{
		if (m_iDeflectionCount>=iDEFLECTION_LIMIT || m_bProcessDeflection || m_bFirstFrame) // We have reached our deflection limit
			return;

		#ifdef _DEBUG_DEFLECTION
		ntPrintf("### Deflection (%f) -> ProcessDeflection\n",Physics::CPhysicsWorld::Get().GetFrameTime());
		#endif // _DEBUG_DEFLECTION

		m_bProcessDeflection=true;

		// ----- Check to see if an enemy entity is within an angle to the deflection so that we can adjust the velocity -----

		const float fMaxDistanceSquared = 20.0f * 20.0f; // Maximum search range
		float fLowestAngle = 90.0f; // Find entities within 45 degrees of the deflection vector
		
		if (m_pobParentEntity->GetMessageHandler())
		{
			CMessageSender::SendEmptyMessage("msg_deflection",m_pobParentEntity->GetMessageHandler());
		}

		/*
			if (stQuery.pobEntity->IsEnemy())
		{
				fLowestAngle=90.0f; // Broaden the deflection angle when hitting character control volumes
		}
			else
		{
				fLowestAngle=45.0f;
		}
		*/

		CEntity* pobNewTarget = 0;

		CDirection obDeflectionVector(m_obDeflectionVelocity);
		obDeflectionVector.Normalise();

		CEQCIsInFront obClause1( m_obDeflectionIntersect, m_obDeflectionVelocity );
		CEQCIsEnemy obClause2;

		CEntityQuery obQuery;
		obQuery.AddClause( obClause1 );
		obQuery.AddClause( obClause2 );

		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AI);

		QueryResultsContainerType::const_iterator obEndIt = obQuery.GetResults().end();

		for( QueryResultsContainerType::const_iterator obIt = obQuery.GetResults().begin(); obIt != obEndIt; ++obIt )
		{
			CEntity* pobEntity=*obIt;

			if( pobEntity != m_pobParentEntity && // Ignore self entity
				pobEntity != m_pobLastTarget && // Ignore the last entity we hit
				!pobEntity->ToCharacter()->IsDead() && // Make sure target is alive
				pobEntity->GetAttackComponent()->AI_Access_GetState() != CS_IMPACT_STAGGERING) // Make sure target isn't recoiling from an impact already
			{
				CPoint obTargetPos(pobEntity->GetPosition());

				CDirection obDelta(	obTargetPos.X()-m_obDeflectionIntersect.X(),
								obTargetPos.Y()-m_obDeflectionIntersect.Y() ,
								obTargetPos.Z()-m_obDeflectionIntersect.Z() );

				if (obDelta.LengthSquared() < fMaxDistanceSquared) // Ensure target entity is within range
				{
					obDelta.Normalise();

					float fDotProduct	= obDeflectionVector.X() * obDelta.X() + obDeflectionVector.Y() * obDelta.Y() + obDeflectionVector.Z() * obDelta.Z();
					float fAngle		= facosf( fDotProduct ) * RAD_TO_DEG_VALUE;

					if( fAngle > 90.0f )
						fAngle = 180.0f - fAngle;

					if( fAngle < 0.0f )
						fAngle = 0.0f;
					else if( fAngle > 90.0f )
						fAngle = 90.0f;

					//ntPrintf("Evaluating target=%s  Angle=%f\n",pobEntity->GetName().c_str(),fAngle);
				
					if( fAngle < fLowestAngle )
					{
						fLowestAngle = fAngle;
						pobNewTarget = (*obIt);

						//ntPrintf("Found suitable target=%s\n",pobNewTarget->GetName().c_str());
					}
				}
			}
		}

		// We have a new target!
		if( pobNewTarget )
		{
			// Make sure we have a clear line of sight to the target

			CPoint obTargetPosition( pobNewTarget->GetHierarchy()->GetRootTransform()->GetWorldTranslation() );
			obTargetPosition.Y() = m_obCurrentPosition.Y();
			//obTargetPosition.Y() += 1.0f;

			Physics::RaycastCollisionFlag obRaycastFlag; obRaycastFlag.base = 0;
			obRaycastFlag.flags.i_am = Physics::LINE_SIGHT_BIT;
			obRaycastFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
													Physics::RAGDOLL_BIT						|
													Physics::SMALL_INTERACTABLE_BIT				|
													Physics::LARGE_INTERACTABLE_BIT				);

			Physics::TRACE_LINE_QUERY stQuery;

			Physics::CPhysicsWorld::Get().TraceLine( m_obCurrentPosition, obTargetPosition,m_pobParentEntity,stQuery, obRaycastFlag );

			if( stQuery.pobEntity==pobNewTarget ) // There is nothing obstructing line of sight with target
			{
				float fSpeed=m_obDeflectionVelocity.Length();

				m_obDeflectionVelocity.X()=obTargetPosition.X() - m_obDeflectionIntersect.X();
				m_obDeflectionVelocity.Y()=obTargetPosition.Y() - m_obDeflectionIntersect.Y();
				m_obDeflectionVelocity.Z()=obTargetPosition.Z() - m_obDeflectionIntersect.Z();
				m_obDeflectionVelocity.Normalise();
				m_obDeflectionVelocity *= fSpeed;

				m_pobLastTarget=pobNewTarget;

				#ifdef _DEBUG_DEFLECTION
				//ntPrintf("Deflection: Found target %s - Speed=%f\n", stQuery.pobEntity->GetName().c_str(), fSpeed);
				#endif // _DEBUG_DEFLECTION
			}
			else
			{
				//ntPrintf("Rejecting target %s, %s is blocking line of sight\n",pobNewTarget->GetName().c_str(),stQuery.pobEntity->GetName().c_str());
			}
		}
	}

	//---------------------------------------------------------------
	//!
	//! DeflectionRendererBehavior constructor.
	//!
	//---------------------------------------------------------------
	DeflectionRendererBehavior::DeflectionRendererBehavior ()
	{
		m_id = ntstd::String("DEFLECT_RENDER");
	}
					
	//---------------------------------------------------------------
	//!
	//! DeflectionRendererBehavior update.
	//!
	//---------------------------------------------------------------

	bool DeflectionRendererBehavior::Update (Element* p_element)
	{
		if (p_element->GetType() != Element::RIGID_BODY) // Ensure this is the correct element type before proceeding
			return true;
	
		const float fRAYCAST_DISTANCE = 100.0f;
		const float fMINIMUM_VELOCITY = 10.0f * 10.0f;

		#ifndef _GOLD_MASTER
		const uint32_t uiRAYCAST_COLOUR = 0xffffff00;
		#endif

		Physics::RaycastCollisionFlag obCollisionFlag;
		obCollisionFlag.base = 0;
		obCollisionFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obCollisionFlag.flags.i_collide_with = (	//Physics::CHARACTER_CONTROLLER_PLAYER_BIT	|  // Ignore player
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
													Physics::RAGDOLL_BIT						|
													Physics::SMALL_INTERACTABLE_BIT				|
													Physics::LARGE_INTERACTABLE_BIT				);

		RigidBody* elem=(RigidBody*)p_element;
		hkRigidBody* pobRB=elem->GetHkRigidBody();

		CPoint obPosition(elem->GetTransform()->GetWorldTranslation()); // Position of this rigid body

		CPoint obRaycastStart;
		CPoint obRaycastEnd;

		Physics::TRACE_LINE_QUERY stQuery;

		if (pobRB->getMotionType()==hkMotion::MOTION_KEYFRAMED) // Rigid body is keyframed, so use a line of sight intersect
		{
			const CMatrix& obCameraMatrix=CamMan::Get().GetPrimaryView()->GetCurrMatrix();

			CPoint obCameraPosition(obCameraMatrix.GetTranslation());

			CDirection obDelta(obPosition - obCameraMatrix.GetTranslation());
			float fDistanceBetweenObjectAndCamera=obDelta.Length();

			CDirection obOffset1=CDirection(0.0f,0.0f,fDistanceBetweenObjectAndCamera) * obCameraMatrix; // Prevent the raycast from hitting stuff thats behind the player, such as a wall
			CDirection obOffset2=CDirection(0.0f,0.0f,fRAYCAST_DISTANCE) * obCameraMatrix;

			obRaycastStart=obCameraPosition + obOffset1; // Raycast begins near the camera
			obRaycastEnd=obCameraPosition + obOffset2; // Raycast ends at a arbitary distance from the camera in the Z-axis
			}
		else // Rigid body is dynamic, so use its velocity
		{
			CDirection obDirection(pobRB->getLinearVelocity()(0),pobRB->getLinearVelocity()(1),pobRB->getLinearVelocity()(2));

			if (obDirection.LengthSquared() < fMINIMUM_VELOCITY) // The velocity the object is travelling at is below the threshold, so skip render
				return true;

			obDirection.Normalise();
			obDirection *= fRAYCAST_DISTANCE;

			obRaycastStart=obPosition;
			obRaycastEnd=obPosition+obDirection;
		}


		// Perform raycast to find what either the line of sight or the direction of travel intersects with
		if (Physics::CPhysicsWorld::Get().TraceLine(obRaycastStart, obRaycastEnd, elem->GetEntity(), stQuery, obCollisionFlag))
		{
			CDirection obProjection(stQuery.obIntersect - obPosition); // Projection is from the RB position to the point where it intersected
			CDirection obDeflection;

			// Calculate the deflection vector
			const CDirection& obNormal = stQuery.obNormal;

			float fF = 2.0f * ( obNormal.X() * obProjection.X() + obNormal.Y() * obProjection.Y() + obNormal.Z() * obProjection.Z() );

			obDeflection.X() = obProjection.X() - fF * obNormal.X();
			obDeflection.Y() = obProjection.Y() - fF * obNormal.Y();
			obDeflection.Z() = obProjection.Z() - fF * obNormal.Z();

#ifndef _GOLD_MASTER
			g_VisualDebug->RenderLine(obPosition,stQuery.obIntersect,uiRAYCAST_COLOUR); // A line from object to intersect point
			g_VisualDebug->RenderLine(stQuery.obIntersect,CPoint(stQuery.obIntersect+obDeflection),uiRAYCAST_COLOUR); // Deflection line
#endif
		}
		else // The raycast didn't hit anything, so draw a single line showing the predicted direction of travel
		{
#ifndef _GOLD_MASTER
			g_VisualDebug->RenderLine(obPosition,obRaycastEnd,uiRAYCAST_COLOUR);
#endif
		}

		return false;
	}


	//---------------------------------------------------------------
	//!
	//! PiercingBehavior constructor.
	//!
	//---------------------------------------------------------------

	PiercingBehavior::PiercingBehavior( )
	{
		m_id = ntstd::String( "PIERCE" );
	}

	//---------------------------------------------------------------
	//!
	//! PiercingBehavior destructor.
	//!
	//---------------------------------------------------------------

	PiercingBehavior::~PiercingBehavior()			
	{
	}

	//---------------------------------------------------------------
	//!
	//! PiercingBehavior event callback.
	//!		\param Element* - element to check.
	//!
	//---------------------------------------------------------------

	bool	PiercingBehavior::Update( Element* p_element )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		ntAssert( p_element );
		ntAssert( p_element->GetType() == Element::RIGID_BODY );
		
		RigidBody*		elem = (RigidBody*) p_element;
		hkRigidBody*	body = elem->GetHkRigidBody();

		hkVector4 obVelocity( body->getLinearVelocity() );

		CMatrix obNewLocalMatrix;
		
		CCamUtil::MatrixFromEuler_XYZ(obNewLocalMatrix,90.0f * DEG_TO_RAD_VALUE,0.0f,180.0f * DEG_TO_RAD_VALUE); // Re-orientate the rigid body (we assume Up is pointing forward)

		CMatrix obTemp;
		CCamUtil::CreateFromPoints( obTemp, CVecMath::GetZeroPoint(), CPoint(obVelocity(0),obVelocity(1),obVelocity(2))); // Calculate the local matrix from the velocity vector

		obNewLocalMatrix *= obTemp;
		obNewLocalMatrix.SetTranslation( elem->GetTransform()->GetLocalTranslation( ));

		CQuat obNewLocalQuat( obNewLocalMatrix );

		const float fInverseTimeDelta = 1.0f / CTimer::Get().GetGameTimeChange();

		applyHardKeyFrameAsynchronously( body->getPosition(), 
										Physics::MathsTools::CQuatTohkQuaternion(obNewLocalQuat), 
										fInverseTimeDelta,
										body );

		body->setLinearVelocity( obVelocity );

		elem->GetTransform()->SetLocalMatrix( obNewLocalMatrix ); // Set the transform (used by the renderable)
#else
		UNUSED( p_element );
#endif

		return false;
	}

} // Physics

