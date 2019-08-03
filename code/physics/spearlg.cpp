//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/spearlg.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "physics/havokincludes.h"

#include "system.h" // Needed for debugging tools

#include "spearlg.h"

#include "element.h"
#include "behavior.h"
#include "rigidbody.h"
#include "rigidbodybehavior.h"
#include "maths_tools.h"
#include "physicsloader.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/basetypes/hkaabb.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/constraint/bilateral/limitedhinge/hkLimitedHingeConstraintData.h>
#include <hkutilities\keyframe\hkKeyFrameUtility.h>
#endif

#include "anim/transform.h"
#include "anim/hierarchy.h"
#include "game/entitymanager.h"
#include "game/interactioncomponent.h"
#include "physics/world.h"
#include "core/timer.h"
#include "camera/camutils.h"

#include "anim/animator.h"
#include "game/query.h"
#include "game/messagehandler.h"
#include "game/randmanager.h"
#include "game/movement.h"
#include "game/attacks.h"

#include "advancedcharactercontroller.h"
#include "system.h"

#ifndef _RELEASE

#include "core/visualdebugger.h"

//#define _SPEAR_DEBUG_OUTPUT
//#define _SPEAR_DEBUG_ACTIVE_RAYCAST
//#define _SPEAR_DEBUG_PREDICTED_RAYCAST

#endif // _RELEASE

namespace Physics {

	// - TOOLS -----------------------------------------------------------------------------------------------------------------

	// -----------------------------------------------------------------------------------
	//	SpearThrown Behavior.
	// -----------------------------------------------------------------------------------
	class SpearThrown : public Behavior
	{
	private:

		float m_fFrontOffset;
		float m_fRearOffset;
		ntstd::List<CEntity*>		m_obIgnoreEntityList;
		CEntity*					m_pobExcludedEntity;
		Transform*					m_pobTransform;
		CEntity*					m_pobParentEntity;
		hkRigidBody*				m_pobRigidBody;
		SpearLG*					m_pobSpearLG;
		bool						m_bFinished;

	public:

						SpearThrown( SpearLG* lg );
		virtual			~SpearThrown( );
		virtual bool	Update( LogicGroup* p_group );	
	};

	//---------------------------------------------------------------
	//!
	//! SpearThrown constructor.
	//!		\param ntstd::String& - message to generate.
	//!
	//---------------------------------------------------------------

	SpearThrown::SpearThrown( SpearLG* lg )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		m_pobSpearLG=lg;

		m_pobTransform = lg->GetRigidBody()->GetTransform();
		m_pobParentEntity = lg->GetEntity();
		m_pobRigidBody = lg->GetRigidBody()->GetHkRigidBody();

		hkAabb obAABB;
		m_pobRigidBody->getCollidable()->getShape()->getAabb(hkTransform::getIdentity(),0.0f,obAABB);

		m_fFrontOffset = obAABB.m_max(2);
		m_fRearOffset = obAABB.m_min(2);

		m_obIgnoreEntityList.push_back(m_pobParentEntity);
		m_pobExcludedEntity = 0;

		m_id = ntstd::String( "SPEAR_THROWN" );

		lg->SetMotionType(HS_MOTION_DYNAMIC);
		lg->GetRigidBody()->AddBehavior( NT_NEW AntiGravity(lg->GetRigidBody(), 10.0f, 10.0f) ); //AntiGravity(10.0f,10.0f);
		lg->AddCheckAtRestBehavior();

		/*
		CMatrix testM = m_pobTransform->GetLocalMatrix();
		CPoint testT = testM.GetTranslation();
		testT.Y() += 0.0f;
		testM.SetTranslation(testT);
		m_pobTransform->SetLocalMatrix(testM);
		hkVector4 posT = m_pobRigidBody->getPosition();
		posT(1) += 0.0f;
		m_pobRigidBody->setPosition(posT);
		*/

		// Ensure the rigid body position/rotation is synchronised with the transform

		//CPoint obPosition(m_pobTransform->GetWorldTranslation());

		/*
			Transform*	m_pobTransform = body->GetTransform();
			//CEntity* m_pobParentEntity = lg->GetEntity();

			CMatrix obOrientation;
			CPoint obLinearVelocity = CPoint( GetLinearVelocity() );
			CCamUtil::CreateFromPoints(obOrientation, CVecMath::GetZeroPoint(), *(CPoint*)&obLinearVelocity);
			CQuat obRotation(obOrientation);

			body->GetHkRigidBody()->setRotation(Physics::MathsTools::CQuatTohkQuaternion(obRotation));

			obOrientation.SetTranslation(m_pobTransform->GetLocalTranslation());
			
			m_pobTransform->SetLocalMatrixFromWorldMatrix(obOrientation);

			body->GetHkRigidBody()->setAngularVelocity(hkVector4::getZero()); 

		CPoint obPosition(m_pobTransform->GetWorldTranslation());
		CQuat obRotation(m_pobTransform->GetWorldRotation());

		m_pobRigidBody->setPosition(
			hkVector4(obPosition.X(),obPosition.Y(),obPosition.Z()));

		m_pobRigidBody->setRotation(
			hkQuaternion(obRotation.X(),obRotation.Y(),obRotation.Z(),obRotation.W()));

		m_pobRigidBody->setAngularVelocity(hkVector4::getZero());
		*/

		m_bFinished = false;

		Update( lg );
#else
		UNUSED( lg );
#endif

	}

	//---------------------------------------------------------------
	//!
	//! SpearThrown destructor.
	//!
	//---------------------------------------------------------------

	SpearThrown::~SpearThrown()			
	{
		// Spear thrown behaviour has been terminated

		//SpearLG* lg = (SpearLG*) this->p_group;

		m_obIgnoreEntityList.clear();

		//m_pobSpearLG->GetRigidBody()->RemoveBehavior("ANTI_GRAV");

		//m_pobSpearLG->RemoveRagdolls();



		//m_iRagdollCount=0;

		//CMessageSender::SendEmptyMessage( "msg_removeragdoll", GetEntity()->GetMessageHandler() ); // Send a message to this entity that we are removing ragdolls
	}

	//---------------------------------------------------------------
	//!
	//! SpearThrown event callback.
	//!		\param Element* - element to check.
	//!
	//---------------------------------------------------------------
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	static const float 		fPIERCING_ANGLE_THRESHOLD = 35.0f;
	static const float 		fPIERCING_HIT_FRACTION = 0.8f;
	static const float 		fPIERCING_VELOCITY_SQUARED_THRESHOLD = 1.0f * 1.0f;
	static const float 		fPHYSICS_INVERSE_TIME_DELTA = 30.0f;
	static const float		fRAGDOLL_REPARENT_FRACTION = 0.5f;
	static const float		fRAGDOLL_DISTANCE_FROM_CENTRE_THRESHOLD = 0.2f;
	static const float		fRAGDOLL_TORSO_MIN_Y = 0.8f;
	static const float		fRAGDOLL_TORSO_MAX_Y = 1.8f;
	static const float		fRAGDOLL_SPACING_FRACTION = 0.2f;
	static int flag = 0;
#endif

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

	bool	SpearThrown::Update( LogicGroup* p_group )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( p_group );
		ntAssert( p_group->GetType() == LogicGroup::SPEAR_LG );

		if (m_bFinished)
			return true;

		SpearLG* lg = (SpearLG*) p_group;

		// ----- Align the spear to the direction its travelling in  -----
		CDirection m_obLinearVelocity = lg->GetLinearVelocity();

		hkVector4 obVelocity( m_pobRigidBody->getLinearVelocity() );

		CMatrix obOrientation;
		CCamUtil::CreateFromPoints( obOrientation, CVecMath::GetZeroPoint(), *(CPoint*)&m_obLinearVelocity);
		CQuat obRotation( obOrientation );

		const float fInverseTimeDelta = 1.0f / CTimer::Get().GetGameTimeChange();

		applyHardKeyFrameAsynchronously( m_pobRigidBody->getPosition(), 
										Physics::MathsTools::CQuatTohkQuaternion(obRotation), 
										fInverseTimeDelta,
										m_pobRigidBody );

		m_pobRigidBody->setLinearVelocity( obVelocity );

		obOrientation.SetTranslation( m_pobTransform->GetLocalTranslation( ));
			
		m_pobTransform->SetLocalMatrix( obOrientation ); // Set the transform (used by the renderable)

		// ----- Actual raycast -----

		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
										Physics::RAGDOLL_BIT						|
										Physics::SMALL_INTERACTABLE_BIT				|
										Physics::LARGE_INTERACTABLE_BIT				);


		const CPoint& obTransformPosition( m_pobTransform->GetWorldMatrix().GetTranslation() );

		CDirection obDirection( m_obLinearVelocity );
		obDirection.Normalise();

		CPoint obStart( obTransformPosition );
		obStart+=obDirection * m_fRearOffset;

		CPoint obEnd(obTransformPosition);
		obEnd+=obDirection * m_fFrontOffset;

		TRACE_LINE_QUERY stActualRaycast;
					
		bool bActualRay = CPhysicsWorld::Get().TraceLine(obStart,obEnd,m_obIgnoreEntityList,stActualRaycast,obFlag);

		#ifdef _SPEAR_DEBUG_ACTIVE_RAYCAST
		g_VisualDebug->RenderLine(obStart,obEnd,0xffff0000);
		#endif // _SPEAR_DEBUG_RAYCAST

		// ----- Predicted raycast -----

		Physics::RaycastCollisionFlag obPredictedRaycastFlag; obPredictedRaycastFlag.base = 0;
		obPredictedRaycastFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obPredictedRaycastFlag.flags.i_collide_with = (	Physics::LARGE_INTERACTABLE_BIT	); // This raycast is only concerned with the environment to enable it to stick in things

		TRACE_LINE_QUERY stPredictedRaycast;

		obEnd=obTransformPosition;
		obEnd+=obDirection * (m_fFrontOffset + m_fFrontOffset + -m_fRearOffset + (m_obLinearVelocity.Length() * fInverseTimeDelta));
					
		//bool bPredictedRay=CPhysicsWorld::Get().TraceLine(obStart,obEnd,m_pobParentEntity,stPredictedRaycast,obPredictedRaycastFlag);
		bool bPredictedRay=CPhysicsWorld::Get().TraceLine(obStart,obEnd,m_obIgnoreEntityList,stPredictedRaycast,obPredictedRaycastFlag);

		#ifdef _SPEAR_DEBUG_PREDICTED_RAYCAST
		g_VisualDebug->RenderLine(obStart,obEnd,0xffffff00);
		#endif // _SPEAR_DEBUG_RAYCAST

		
		// ----- Set collision exclusions with stuff we are about to hit -----

		if (bPredictedRay) // We are about to hit something
		{
			if (!(stActualRaycast.pobEntity!=0 && stActualRaycast.pobEntity==m_pobExcludedEntity))
			{
				// Calculate the intersection angle
				float fAngle=stPredictedRaycast.obNormal.X()*obDirection.X() + stPredictedRaycast.obNormal.Y()*obDirection.Y() + stPredictedRaycast.obNormal.Z()*obDirection.Z();

				//ntPrintf("Dot product=%f  acos 1=%f  acos -1=%f\n",fAngle,facosf(1.0f) * RAD_TO_DEG_VALUE,facosf(-1.0f) * RAD_TO_DEG_VALUE);

				if (fAngle>=1.0f)
				{
					fAngle=0.0f;
				}
				else if (fAngle<=-1.0f)
				{
					fAngle=180.0f;
				}
				else
				{
					fAngle=facosf(fAngle) * RAD_TO_DEG_VALUE;

					if (fAngle>90.0f)
						fAngle=180.0f-fAngle;

					if (fAngle<0.0f)
						fAngle=0.0f;
					else if (fAngle>90.0f)
						fAngle=90.0f;

					fAngle=90.0f-fAngle;
				}

				// The collision angle is within our threshold, exclude collision between this entity and the target
				if (fAngle>fPIERCING_ANGLE_THRESHOLD) 
				{
					if (stPredictedRaycast.pobEntity!=m_pobExcludedEntity)
					{
						if (m_pobExcludedEntity) // Re-allow collision with out previously excluded entity
						{
							#ifdef _SPEAR_DEBUG_OUTPUT
							ntPrintf("Spear - Collision entity has changed from %s to %s, re-allowing collision with %s\n",
								m_pobExcludedEntity->GetName().c_str(),
								stPredictedRaycast.pobEntity->GetName().c_str(),
								m_pobExcludedEntity->GetName().c_str());
							#endif // _SPEAR_DEBUG_OUTPUT

							m_pobParentEntity->GetInteractionComponent()->AllowCollisionWith(m_pobExcludedEntity);

							m_pobExcludedEntity=0;
						}	

						if (stPredictedRaycast.pobEntity->GetInteractionComponent())
						{
							#ifdef _SPEAR_DEBUG_OUTPUT
							ntPrintf("Spear - Favorable collision angle=%f, excluding collision with %s\n",fAngle,stPredictedRaycast.pobEntity->GetName().c_str());
							#endif // _SPEAR_DEBUG_OUTPUT
							
							m_pobParentEntity->GetInteractionComponent()->ExcludeCollisionWith(stPredictedRaycast.pobEntity);

							m_pobExcludedEntity=stPredictedRaycast.pobEntity;
						}
					}
				}
				else // The collision angle is no longer favorable
				{
					if (m_pobExcludedEntity) // Re-allow collision with out previously excluded entity
					{
						#ifdef _SPEAR_DEBUG_OUTPUT
						ntPrintf("Spear - collision angle %f no longer favorable, allowing collision with %s\n",fAngle,m_pobExcludedEntity->GetName().c_str());
						#endif // _SPEAR_DEBUG_OUTPUT

						m_pobParentEntity->GetInteractionComponent()->AllowCollisionWith(m_pobExcludedEntity);

						m_pobExcludedEntity=0;
					}

				}
			}
		}
		else // Predicted ray is not colliding with anything
		{
			if (m_pobExcludedEntity) // Re-allow collision with out previously excluded entity
			{
				#ifdef _SPEAR_DEBUG_OUTPUT
				ntPrintf("Spear - no obstacles in path, re-allowing collision with %s\n",m_pobExcludedEntity->GetName().c_str());
				#endif // _SPEAR_DEBUG_OUTPUT

				m_pobParentEntity->GetInteractionComponent()->AllowCollisionWith(m_pobExcludedEntity);

				m_pobExcludedEntity=0;
			}
		}

		// ----- Check to see if the spear has penetrated something up to our hit fraction threshold -----

		if (bActualRay && stActualRaycast.pobEntity &&
			stActualRaycast.fFraction<fPIERCING_HIT_FRACTION &&
			!(stActualRaycast.pobEntity->GetAttackComponent() && stActualRaycast.pobEntity->GetAttackComponent()->GetDisabled()))
		{
			if (stActualRaycast.pobEntity->IsEnemy())
			{
				//ntPrintf("Spear - Intersecting character at %f\n",stActualRaycast.fFraction);

				// Distance from raycast to character's centre

				//		(Cx-Ax)(Bx-Ax) + (Cy-Ay)(By-Ay)
				//	r = -------------------------------
				//				     L^2
				
				// Check to see how close the raycast is to the character's torso
					
				CPoint obSelfPosition(m_pobTransform->GetWorldMatrix().GetTranslation());
				obSelfPosition.Y()=0.0f;

				CPoint obTargetPosition(stActualRaycast.pobEntity->GetPosition());
				float fCollideeY=obTargetPosition.Y();
				obTargetPosition.Y()=0.0f;

				CPoint obA(obSelfPosition);
				obA+=obDirection * m_fRearOffset;
				obA.Y()=0.0f;

				CPoint obB(obSelfPosition);
				obB+=obDirection * m_fFrontOffset;
				obB.Y()=0.0f;

				CDirection obDiff(obB-obA);

				float fLength=obDiff.Length();

				float fR=(obTargetPosition.X()-obA.X())*(obB.X()-obA.X()) + (obTargetPosition.Z()-obA.Z())*(obB.Z()-obA.Z());
				fR/=fLength*fLength;

				CPoint obIntersect(obA);
				obIntersect += obDiff * fR;

				CDirection obIntersectToPosition(obTargetPosition - obIntersect);

				float fDistance=obIntersectToPosition.Length();

				#ifdef _SPEAR_DEBUG_OUTPUT
				ntPrintf("Spear - Distance of raycast from centre=%.3f  Fraction=%.3f\n",	fDistance, fR);
				#endif // _SPEAR_DEBUG_OUTPUT

				#ifdef _SPEAR_DEBUG_RAGDOLL_RAYCAST
				g_VisualDebug->RenderLine(obA, obB, 0xff00ffff);
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),obIntersect,0.1f,0xffff0000);
				#endif // _SPEAR_DEBUG_RAGDOLL_RAYCAST


				const float fDy=stActualRaycast.obIntersect.Y()-fCollideeY;

				if (fR<fRAGDOLL_REPARENT_FRACTION &&
					fDistance<fRAGDOLL_DISTANCE_FROM_CENTRE_THRESHOLD && // Check to see how far the spear is from the centre point of the character
					lg->m_iRagdollCount<iMAX_RAGDOLLS && // Make sure we haven't exceeded our ragdoll limit
					fDy>fRAGDOLL_TORSO_MIN_Y && // Ensure we are in the torso area
					fDy<fRAGDOLL_TORSO_MAX_Y)
				{
					// We probably want to flag the character as dead as well!
					stActualRaycast.pobEntity->GetMovement()->SetEnabled( false );
					stActualRaycast.pobEntity->GetAnimator()->ClearAnimWeights();
					stActualRaycast.pobEntity->GetAnimator()->Disable();

					if( stActualRaycast.pobEntity->GetPhysicsSystem() )
						stActualRaycast.pobEntity->GetPhysicsSystem()->Deactivate();

					Physics::AdvancedRagdoll* pobRagdoll = 0;
					Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*) stActualRaycast.pobEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
					if( pobCharacter )
					{
						pobCharacter->SetRagdollDead();
					}
					
					if (pobRagdoll)	
					{
						lg->m_iRagdollCount++;

						for(int iCount=0; iCount<lg->m_iRagdollCount; ++iCount)
						{
							if (iCount==(lg->m_iRagdollCount-1))
							{
								lg->m_astRagdollLink[iCount].fFractionOffset=fRAGDOLL_REPARENT_FRACTION;
								lg->m_astRagdollLink[iCount].pobEntity=stActualRaycast.pobEntity;

								float fCorrectionOffset=(m_fFrontOffset - m_fRearOffset)*(lg->m_astRagdollLink[iCount].fFractionOffset - fR);

								CDirection obOffset(obDirection);
								obOffset*=fCorrectionOffset;

								CPoint obLocalTranslation(stActualRaycast.pobEntity->GetHierarchy()->GetRootTransform()->GetLocalTranslation());
								obLocalTranslation+=obOffset;
								stActualRaycast.pobEntity->GetHierarchy()->GetRootTransform()->SetLocalTranslation(obLocalTranslation);

								#ifdef _SPEAR_DEBUG_OUTPUT
								ntPrintf("Spear - Intersected %s at %f (Desired Fraction=%f  Fraction correction=%f  Z adjustment=%f)\n",
									lg->m_astRagdollLink[iCount].pobEntity->GetName().c_str(),
									fR,
									lg->m_astRagdollLink[iCount].fFractionOffset,
									lg->m_astRagdollLink[iCount].fFractionOffset - fR,
									obOffset.Length());
								#endif // _SPEAR_DEBUG_OUTPUT
							}
							else
							{
								lg->m_astRagdollLink[iCount].fFractionOffset-=fRAGDOLL_SPACING_FRACTION;

								CPoint obCentre(lg->m_astRagdollLink[iCount].pobEntity->GetPosition());
								obCentre.Y()=0.0f;

								float fR2=(obCentre.X()-obA.X())*(obB.X()-obA.X()) + (obCentre.Z()-obA.Z())*(obB.Z()-obA.Z());
								fR2/=fLength*fLength;

								float fCorrectionOffset=(m_fFrontOffset - m_fRearOffset)*(lg->m_astRagdollLink[iCount].fFractionOffset - fR2);

								CDirection obOffset(obDirection);
								obOffset*=-fCorrectionOffset;

								CPoint obLocalTranslation(lg->m_astRagdollLink[iCount].pobEntity->GetHierarchy()->GetRootTransform()->GetLocalTranslation());
								obLocalTranslation+=obOffset;
								lg->m_astRagdollLink[iCount].pobEntity->GetHierarchy()->GetRootTransform()->SetLocalTranslation(obLocalTranslation);

								#ifdef _SPEAR_DEBUG_OUTPUT
								ntPrintf("Spear - Adjusting %s (Desired Fraction=%f  Actual Fraction=%f  Fraction correction=%f  Z adjustment=%f)\n",
									lg->m_astRagdollLink[iCount].pobEntity->GetName().c_str(),
									lg->m_astRagdollLink[iCount].fFractionOffset,
									fR2,
									lg->m_astRagdollLink[iCount].fFractionOffset - fR2,
									obOffset.Length());
								#endif // _SPEAR_DEBUG_OUTPUT
							}
						}

						pobCharacter->SetRagdollAnimated(Physics::AdvancedRagdoll::PELVIS | Physics::AdvancedRagdoll::SPINE); 		

						// Reparent the ragdoll to the spear
						CMatrix obNewLocalMatrix(stActualRaycast.pobEntity->GetHierarchy()->GetRootTransform()->GetLocalMatrix());
						obNewLocalMatrix *= m_pobTransform->GetWorldMatrix().GetAffineInverse();

						Transform* pobHeldTransform = stActualRaycast.pobEntity->GetHierarchy()->GetRootTransform();
						flag = pobHeldTransform->GetFlags();
						pobHeldTransform->SetLocalMatrix(obNewLocalMatrix);
						pobHeldTransform->RemoveFromParent();
						
						m_pobParentEntity->GetHierarchy()->GetRootTransform()->AddChild( pobHeldTransform );

						#ifdef _SPEAR_DEBUG_OUTPUT
						ntPrintf("Spear - Adding %s to ignore collision list\n",stActualRaycast.pobEntity->GetName().c_str());
						#endif // _SPEAR_DEBUG_OUTPUT	

						m_obIgnoreEntityList.push_back(stActualRaycast.pobEntity);

						// Add any entities that are parented to this character to the ignore list (such as their weapons)
						CEntityQuery obQuery;
						EQCIsChildEntity obClause(stActualRaycast.pobEntity);
						obQuery.AddClause(obClause);
						
						CEntityManager::Get().FindEntitiesByType(obQuery, CEntity::EntType_AI );

						for(QueryResultsContainerType::iterator obIt=obQuery.GetResults().begin(); obIt!=obQuery.GetResults().end(); ++obIt)
						{
							#ifdef _SPEAR_DEBUG_OUTPUT
							ntPrintf("Spear - Adding %s to ignore collision list\n",(*obIt)->GetName().c_str());
							#endif // _SPEAR_DEBUG_OUTPUT	

							m_obIgnoreEntityList.push_back((*obIt));

							m_pobParentEntity->GetInteractionComponent()->ExcludeCollisionWith((*obIt));
						}

						CMessageSender::SendEmptyMessage( "msg_hitragdoll", m_pobParentEntity->GetMessageHandler() ); // Send a message to the spear saying we've hit a ragdoll
						CMessageSender::SendEmptyMessage( "msg_combat_impaled", stActualRaycast.pobEntity->GetMessageHandler() ); // Send a message to the character that they've been impaled
					}
				}
			}
			else // Spear has penetrated a non-character entity, lets make it stick into it
			{

				// Move the spear so that its penetration depth is always the same

				float fCorrectionOffset=(m_fFrontOffset - m_fRearOffset)*(stActualRaycast.fFraction - fPIERCING_HIT_FRACTION);

				CDirection obOffset(obDirection);
				obOffset*=fCorrectionOffset;
				
				CPoint obLocalTranslation(m_pobTransform->GetLocalTranslation());
				obLocalTranslation+=obOffset;
				m_pobTransform->SetLocalTranslation(obLocalTranslation);
				
				// Reparent the spear the entity it hit

				Transform* pobTargetTransform = stActualRaycast.pobEntity->GetHierarchy()->GetRootTransform();

				CMatrix obLocalMatrix(m_pobTransform->GetLocalMatrix());
				obLocalMatrix*=pobTargetTransform->GetWorldMatrix().GetAffineInverse(); // Combine with the affine inverse of the parent
				m_pobTransform->SetLocalMatrix(obLocalMatrix);

				m_pobParentEntity->SetParentEntity( stActualRaycast.pobEntity );
				m_pobTransform->RemoveFromParent();
				pobTargetTransform->AddChild( m_pobTransform );

				if (stActualRaycast.pobEntity->GetPhysicsSystem()) // Apply a force to whatever we've hit
				{
					CDirection obForce(m_obLinearVelocity);
					obForce*=m_pobRigidBody->getMass();
					
					stActualRaycast.pobEntity->GetPhysicsSystem()->ApplyLocalisedLinearImpulse(obForce, CVector(stActualRaycast.obIntersect));
				}

				// Ensure the rigid body position/rotation is synchronised with the transform

				CPoint obPosition(m_pobTransform->GetWorldTranslation());
				CQuat obRotation(m_pobTransform->GetWorldRotation());

				m_pobRigidBody->setPosition(
					hkVector4(obPosition.X(),obPosition.Y(),obPosition.Z()));

				m_pobRigidBody->setRotation(
					hkQuaternion(obRotation.X(),obRotation.Y(),obRotation.Z(),obRotation.W()));

				// ----- Make ragdolls swing from their parented position on the spear -----		

				RigidBody* body = lg->GetRigidBody();
				if( body->GetHkRigidBody()->isAddedToWorld() )
				{
					for(int iCount=0; iCount<lg->m_iRagdollCount; ++iCount)
					{
						Transform* pobHeldTransform = lg->m_astRagdollLink[iCount].pobEntity->GetHierarchy()->GetRootTransform();
						if(pobHeldTransform->GetParent() == lg->GetEntity()->GetHierarchy()->GetRootTransform()) 
						{
							CMatrix obWorldMatrix(pobHeldTransform->GetWorldMatrix());

							pobHeldTransform->RemoveFromParent();

							pobHeldTransform->SetLocalMatrix(obWorldMatrix);

							CHierarchy::GetWorld()->GetRootTransform()->AddChild(pobHeldTransform);

							Physics::AdvancedRagdoll* pobRagdoll = 0;
							Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)  lg->m_astRagdollLink[iCount].pobEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
							if( pobCharacter )
							{
								pobRagdoll = pobCharacter->GetAdvancedRagdoll();
							}

							if(pobRagdoll)
							{
								pobRagdoll->SetState( Physics::DEAD );
								hkRigidBody* bodyA = pobRagdoll->GetRagdollBone( pobRagdoll->SPINE_BONE );
					
								hkRigidBody* bodyB = bodyA->getWorld()->getFixedRigidBody();

								CVector position1 = Physics::MathsTools::hkVectorToCVector(lg->GetRigidBody()->GetHkRigidBody()->getPosition());
								CVector position2 = Physics::MathsTools::hkVectorToCVector(bodyA->getPosition());
								CVector toProject = position2 - position1;
								hkTransform obRBTransform;
								lg->GetRigidBody()->GetHkRigidBody()->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obRBTransform);
								CDirection spear(Physics::MathsTools::hkVectorToCVector(obRBTransform.getRotation().getColumn(2)));
								spear.Normalise();
								float directionDot = toProject.Dot(CVector(spear));

								CVector hingePos = position1 + CVector(directionDot * spear);

								hkLimitedHingeConstraintData* obInfo = HK_NEW hkLimitedHingeConstraintData();
								hkTransform obRBTransformA, obRBTransformB;
								bodyA->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obRBTransformA);
								bodyB->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obRBTransformB);
								obInfo->setInWorldSpace(	obRBTransformA, 
															obRBTransformB, 
															Physics::MathsTools::CVectorTohkVector(hingePos),//m_pobRigidBody->getPosition(), 
															obRBTransform.getRotation().getColumn(2));

								obInfo->setMaxFrictionTorque (0.5f); 
								obInfo->setMaxAngularLimit (0.3f + (0.92f * erandf( 1.0f ))); 
								obInfo->setMinAngularLimit (-0.3f - (0.92f * erandf( 1.0f ))); 

								bodyA->getWorld()->createAndAddConstraintInstance(bodyA, bodyB, obInfo)->removeReference(); 
								obInfo->removeReference();
								obInfo = 0;

							}

							//CMessageSender::SendEmptyMessage( "msg_activateragdoll", m_astRagdollLink[iCount].pobEntity->GetMessageHandler() );
							//CMessageSender::SendEmptyMessage( "msg_combat_impaled", m_astRagdollLink[iCount].pobEntity->GetMessageHandler() ); // Place character in impaleddead state
						}
					}
				}

				body->Deactivate(); // Deactivate the rigid body

				// Generate a message here, so that the spear can drop into an inactive state
				CMessageSender::SendEmptyMessage( "msg_hitsolid", lg->GetEntity()->GetMessageHandler() );

				//lg->Deactivate(); // Deactivate the spear logic group

				m_bFinished = true;

				#ifdef _SPEAR_DEBUG_OUTPUT
				CQuat obOrientation(m_pobTransform->GetWorldRotation());
				ntPrintf("Spear - Parenting spear to %s at fraction %f\n",stActualRaycast.pobEntity->GetName().c_str(),fPIERCING_HIT_FRACTION);
				ntPrintf("Spear - Orientation at time of impact: %f %f %f %f\n",obOrientation.X(),obOrientation.Y(),obOrientation.Z(),obOrientation.W());
				ntPrintf("Spear - Velocity at time of impact: %f %f %f\n",m_pobRigidBody->getLinearVelocity()(0),m_pobRigidBody->getLinearVelocity()(1),m_pobRigidBody->getLinearVelocity()(2));
				#endif // _SPEAR_DEBUG_OUTPUT

				return true;
			}
		}

		return false;
#else
		UNUSED( p_group );
		return true;
#endif
	}	
	
	// -  SpearLG ------------------------------------------------------------------------------------------------------------------
	SpearLG::SpearLG( const ntstd::String& p_name, CEntity* p_entity ) :
		LogicGroup( p_name, p_entity ),
		m_ePausedState(hkMotion::MOTION_INVALID),
		m_pausedLinVel(0,0,0),
		m_pausedAngVel(0,0,0)
	{
		m_iRagdollCount = 0;
		isThrowned = false;
	};

	SpearLG::~SpearLG( )
	{ 
	};

	void SpearLG::Load()
	{
		ntstd::String psFile = CPhysicsLoader::AlterFilenameExtension( ntStr::GetString(m_entity->GetClumpString()) );
		PhysicsData * data = CPhysicsLoader::Get().LoadPhysics_Neutral(psFile.c_str());

		if (data)
		{
			m_loaderDataRef = data;
			PhysicsData::CloningParams params;

			params.m_collisionFlags.base = 0;
			params.m_collisionFlags.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
			params.m_collisionFlags.flags.i_collide_with = ( Physics::SMALL_INTERACTABLE_BIT |
														Physics::LARGE_INTERACTABLE_BIT	);

			params.m_static = false;
			params.m_largeSmallInteractable = false;

			params.m_filterExceptionFlags.flags.exception_set = Physics::IGNORE_ENTITY_PTR_BIT;

			data->CloneIntoLogicGroup(*this, params);
		}
	}

	const LogicGroup::GROUP_TYPE SpearLG::GetType( ) const
	{
		return SPEAR_LG;
	};
		
	void SpearLG::Update( const float p_timestep )
	{ 
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
		
	void SpearLG::Activate( bool activateInHavok )
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

	void SpearLG::Deactivate( )
	{
		#ifdef _SPEAR_DEBUG_OUTPUT
		ntPrintf("SpearLG::Deactivate\n");
		#endif // _SPEAR_DEBUG_OUTPUT

		LogicGroup::Deactivate();
	};

	CDirection SpearLG::GetLinearVelocity( )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return CDirection(body->GetLinearVelocity());
			}
		}

		ntAssert( 0 );
		return CDirection();
	}

	/*SpearLG* lg = (SpearLG*) p_group;

			Transform*	m_pobTransform = lg->GetRigidBody()->GetTransform();
			//CEntity* m_pobParentEntity = lg->GetEntity();

			CMatrix obOrientation;
			CPoint obLinearVelocity = CPoint( lg->GetLinearVelocity() );
			CCamUtil::CreateFromPoints(obOrientation, CVecMath::GetZeroPoint(), *(CPoint*)&obLinearVelocity);
			CQuat obRotation(obOrientation);

			lg->GetRigidBody()->GetHkRigidBody()->setRotation(CQuatTohkQuaternion(obRotation));

			obOrientation.SetTranslation(m_pobTransform->GetLocalTranslation());
			
			m_pobTransform->SetLocalMatrixFromWorldMatrix(obOrientation);

			lg->GetRigidBody()->GetHkRigidBody()->setAngularVelocity(hkVector4::getZero()); 
	*/

	void SpearLG::SetLinearVelocity( const CDirection& p_linearVelocity )
	{
		#ifdef _SPEAR_DEBUG_OUTPUT
		ntPrintf("SpearLG::SetLinearVelocity\n");
		#endif

		Physics::RigidBody* body = GetRigidBody();
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		//if( GetOrientToVelocity()  )
		{
			CMatrix obNewLocalMatrix;
			CCamUtil::CreateFromPoints(obNewLocalMatrix, CVecMath::GetZeroPoint(), *(CPoint*)&p_linearVelocity);
			obNewLocalMatrix.SetTranslation(body->GetTransform()->GetLocalTranslation());

			CQuat obRotation(obNewLocalMatrix);

			

			body->GetTransform()->SetLocalMatrix(obNewLocalMatrix);

			
			body->GetHkRigidBody()->setAngularVelocity(hkVector4::getZero()); 
			body->GetHkRigidBody()->setPosition(hkVector4(obNewLocalMatrix.GetTranslation().X(),obNewLocalMatrix.GetTranslation().Y(),obNewLocalMatrix.GetTranslation().Z()));
			body->GetHkRigidBody()->setRotation(Physics::MathsTools::CQuatTohkQuaternion(obRotation));

			
			/*
			Transform*	m_pobTransform = body->GetTransform();
			//CEntity* m_pobParentEntity = lg->GetEntity();

			CMatrix obOrientation;
			CPoint obLinearVelocity = CPoint( GetLinearVelocity() );
			CCamUtil::CreateFromPoints(obOrientation, CVecMath::GetZeroPoint(), *(CPoint*)&obLinearVelocity);
			CQuat obRotation(obOrientation);

			body->GetHkRigidBody()->setRotation(Physics::MathsTools::CQuatTohkQuaternion(obRotation));

			obOrientation.SetTranslation(m_pobTransform->GetLocalTranslation());
			
			m_pobTransform->SetLocalMatrixFromWorldMatrix(obOrientation);

			body->GetHkRigidBody()->setAngularVelocity(hkVector4::getZero()); 
			//*/
		}
#endif
		body->SetLinearVelocity( CVector(p_linearVelocity) );
				
	}

	float SpearLG::GetMass( )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				return body->GetHkRigidBody()->getMass();
			}
		}

		ntAssert( 0 );
#endif
		return 0.0f;
	}

	void SpearLG::AddThrownBehaviour( )
	{
		#ifdef _SPEAR_DEBUG_OUTPUT
		ntPrintf("SpearLG::AddThrownBehaviour\n");
		#endif

		AddBehavior( NT_NEW SpearThrown( this ) );
		isThrowned = true;
	}

	void SpearLG::RemoveThrownBehaviour( )
	{
		#ifdef _SPEAR_DEBUG_OUTPUT
		ntPrintf("SpearLG::RemoveThrownBehaviour\n");
		#endif

		GetRigidBody()->RemoveBehavior("ANTI_GRAV");
		RemoveRagdolls();

		RemoveBehavior( "SPEAR_THROWN" );
		isThrowned = false;
	}

	RigidBody* SpearLG::GetRigidBody( )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				return body;
			}
		}

		return 0;
	}

	void SpearLG::AddCheckAtRestBehavior( )
	{
		for (	ntstd::List<Physics::Element*>::iterator it = GetElementList().begin(); 
				it != GetElementList().end(); ++it )
		{
			Physics::Element* current = (*it);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				Physics::RigidBody* body = (Physics::RigidBody*) current;
				body->AddBehavior( NT_NEW BodyAtRest( ntstd::String("msg_atrest") ) );
			}
		}
	}

	void SpearLG::RemoveRagdolls ()
	{
		#ifdef _SPEAR_DEBUG_OUTPUT
		ntPrintf("SpearLG::RemoveRagdolls\n");
		#endif // _SPEAR_DEBUG_OUTPUT

		// Set the ragdoll to be fully ragdoll
		for(int iCount=0; iCount<m_iRagdollCount; ++iCount)
		{
			Transform* pobHeldTransform = m_astRagdollLink[iCount].pobEntity->GetHierarchy()->GetRootTransform();
			if(pobHeldTransform->GetParent() == GetEntity()->GetHierarchy()->GetRootTransform()) 
			{
				CMatrix obWorldMatrix(pobHeldTransform->GetWorldMatrix());
				pobHeldTransform->RemoveFromParent(); // Remove the ragdoll from it's parent
				pobHeldTransform->SetLocalMatrix(obWorldMatrix);
				CHierarchy::GetWorld()->GetRootTransform()->AddChild(pobHeldTransform);
				
				Physics::AdvancedRagdoll* pobRagdoll = 0;
				Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*) m_astRagdollLink[iCount].pobEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if( pobCharacter )
				{
					pobRagdoll = pobCharacter->GetAdvancedRagdoll();
				}

				if(pobRagdoll)
				{
					pobRagdoll->Activate( Physics::DEAD );
					CDirection obVelocity(GetLinearVelocity());					
					pobRagdoll->SetLinearVelocity(obVelocity);
				}
			}
		}

		m_iRagdollCount = 0;
	}

	/*
	void SpearLG::EntityRootTransformHasChanged ()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		CQuat obRotation(GetRigidBody()->GetTransform()->GetWorldMatrix());

		obRotation.Normalise();

		hkVector4 obNewPos(GetRigidBody()->GetTransform()->GetWorldMatrix().GetTranslation().X(),GetRigidBody()->GetTransform()->GetWorldMatrix().GetTranslation().Y(),GetRigidBody()->GetTransform()->GetWorldMatrix().GetTranslation().Z());

		hkQuaternion obNewRotation(obRotation.X(),obRotation.Y(),obRotation.Z(),obRotation.W());

		this->GetRigidBody()->GetHkRigidBody()->setPositionAndRotation(obNewPos,obNewRotation);
#endif
	}
	*/

	void SpearLG::Pause( bool bPause ) 
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if( bPause )
		{
			if (m_ePausedState != hkMotion::MOTION_INVALID)
			{
				//ntPrintf("Pausing already paused (SprearLG), tell peterf.");
				return;
			};
			
			m_ePausedState = GetRigidBody()->GetHkRigidBody()->getMotionType();
			GetRigidBody()->GetHkRigidBody()->setMotionType(hkMotion::MOTION_FIXED);
			m_pausedLinVel = GetRigidBody()->GetHkRigidBody()->getLinearVelocity();	
			m_pausedAngVel = GetRigidBody()->GetHkRigidBody()->getAngularVelocity();	
		}
		else
		{
			if (m_ePausedState == hkMotion::MOTION_INVALID)
			{
				//ntPrintf("Unpausing not paused (SpearLG), tell peterf.");
				return;
			};

			GetRigidBody()->GetHkRigidBody()->setMotionType( (hkMotion::MotionType) m_ePausedState);
			GetRigidBody()->GetHkRigidBody()->setAngularVelocity( m_pausedAngVel );
			GetRigidBody()->GetHkRigidBody()->setLinearVelocity( m_pausedLinVel  );
			m_ePausedState = hkMotion::MOTION_INVALID;
		}
#else
		UNUSED( bPause );
#endif
	}

	void SpearLG::Debug_RenderCollisionInfo ()
	{
#ifndef _RELEASE

		DebugCollisionTools::RenderCollisionFlags(GetRigidBody()->GetHkRigidBody());

		CMatrix obWorldMatrix(
			CQuat(GetRigidBody()->GetHkRigidBody()->getRotation()(0),GetRigidBody()->GetHkRigidBody()->getRotation()(1),GetRigidBody()->GetHkRigidBody()->getRotation()(2),GetRigidBody()->GetHkRigidBody()->getRotation()(3)),
			CPoint(GetRigidBody()->GetHkRigidBody()->getPosition()(0),GetRigidBody()->GetHkRigidBody()->getPosition()(1),GetRigidBody()->GetHkRigidBody()->getPosition()(2)));
	
		DebugCollisionTools::RenderShape(obWorldMatrix,GetRigidBody()->GetHkRigidBody()->getCollidable()->getShape());

#endif // _RELEASE
	}
}
