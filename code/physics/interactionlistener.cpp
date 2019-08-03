//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/interactionlistener.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.06
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"
#include "havokincludes.h"

#include "interactionlistener.h"
#include "advancedcharactercontroller.h"
#include "system.h"
#include "world.h"
#include "objectdatabase/dataobject.h"

#include "game/entityinfo.h"
#include "game/entitymanager.h"
#include "game/attacks.h"
#include "game/syncdcombat.h"
#include "game/strike.h"
#include "game/messagehandler.h"
#include "game/movement.h"

#include "collisionbitfield.h"
#include "collisionlistener.h"

#include "physics/maths_tools.h"
#include "physics/system.h"
#include "physics/advancedcharactercontroller.h"
#include "Physics/singlerigidlg.h"
#include "Physics/compoundlg.h"
#include "Physics/rigidbodybehavior.h"

#include "game/randmanager.h"

#include "audio/collisioneffecthandler.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkutilities/charactercontrol/characterproxy/hkCharacterProxy.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkconstraintsolver/simplex/hkSimplexSolver.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/hkDebugDisplay.h>
#include <hkcollide/collector/pointcollector/hkRootCdPoint.h>
#endif

namespace Physics {

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	static float MAX_RB_CLIMB_ALTITUDE_DELTA = 0.5f;

	CharacterInteractionListener::CharacterInteractionListener ( CEntity* pobMyEntity,  CharacterController* pobCC ) :
		m_pobEntity(pobMyEntity),
		m_pobLastCollidee(0),
		m_pobCC(pobCC)
	{
		m_pobAttackData=ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_collision");
		m_pobAttackData2=ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_collision2");
		m_pobAttackData3=ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_collision3");
		m_pobDieData = ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_throw_body");

		m_pobHitBy = 0;
	}

	void CharacterInteractionListener::Update()
	{
		m_pobLastCollidee=0; // Clear the pointer to our last collidee each frame
	}

	void  CharacterInteractionListener::contactPointAddedCallback (const hkRootCdPoint &point) // This entity has a collision registered against it
	{
		Physics::AdvancedCharacterController* pobCharacterState=(Physics::AdvancedCharacterController*)m_pobEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	
		if ( pobCharacterState->IsRagdollActive() ) // Make sure the ragdoll is active
		{
			return;
		}

		// For completeness, this is now split into checks for a rigid body character and a phantom character - we need to handle both

		// Some temp test vars for 2 rigid bodies and 2 phantoms
		hkRigidBody* rbA = hkGetRigidBody( point.m_rootCollidableA ); 
		hkRigidBody* rbB = hkGetRigidBody( point.m_rootCollidableB );
		hkPhantom* phA = hkGetPhantom( point.m_rootCollidableA );
		hkPhantom* phB = hkGetPhantom( point.m_rootCollidableB );

		// The pointers we'll fill with the useful rigid body or phantom
		hkRigidBody* rb = 0;
		hkPhantom* ph = 0;

		// Try to get the other entities pointer off the RBs...
		CEntity* ent = 0;
		if (rbA)
		{
			ent = (CEntity*) rbA->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr(); 
			if ( ent != m_pobEntity )
				rb = rbA; // Got it!
		}
		if (!rb && rbB)
		{
			ent = (CEntity*) rbB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr(); 
			if ( ent != m_pobEntity )
				rb = rbB; // Got it!
		}

		// If we couldn't get it off the RBs, try the phantoms
		if (!rb)
		{
			if (phA)
			{
				ent = (CEntity*) phA->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr(); // Get pointer of collidee entity
				if ( ent != m_pobEntity )
					ph = phA; // Got it!
			}
			if (!ph && phB)
			{
				ent = (CEntity*) phB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr(); // Get pointer of collidee entity
				if ( ent != m_pobEntity )
					ph = phB; // Got it!
			}
		}

		// If the other guy is a phantom...
		if (ph)
		{
			EntityCollisionFlag info;
			info.base = ph->getCollidable()->getCollisionFilterInfo();

			// He's probably a character, if so check if we need to trigger a KO
			if( info.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT || info.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT )
			{
				// If the other guy is KO'd, hasn't been hit by a KO himself, and is a certain grace height above us, then we need to KO
				if (ent->GetAttackComponent()->AI_Access_GetState() == CS_KO && !ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GetStruckByKO() && (ent->GetPosition().Y() - m_pobEntity->GetPosition().Y()) > 0.3f )
				{
					if (ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GetStrikeCount()==0)
					{
						if (grandf(1.0f) > 0.5f)
							ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData2);
						else
							ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData3);
					}
					else
					{
						ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData);
					}

					ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->SetStruckByKO();
				}
			}
		}
		// If the other guy is a rigid body...
		else if( rb )
		{
			EntityCollisionFlag info;
			info.base = rb->getCollidable()->getCollisionFilterInfo();
				
			// He might be a character, if so check if we need to trigger a KO
			if( info.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT || info.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT )
			{
				// If the other guy is KO'd, hasn't been hit by a KO himself, and is a certain grace height above us, then we need to KO
				if (ent->GetAttackComponent()->AI_Access_GetState() == CS_KO && (ent->GetPosition().Y() - m_pobEntity->GetPosition().Y()) > 0.3f )
				{
					if (ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GetStrikeCount()==0)
					{
						if (grandf(1.0f) > 0.5f)
							ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData2);
						else
							ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData3);
					}
					else
					{
						ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData);
					}
				}
			}
			else if ( info.flags.i_am & RAGDOLL_BIT ) // The collidee rigid body is part of a ragdoll
			{				
				if ( ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->IsEnabled() == false )
				{
					return;
				}

				// Get character controller of other entity
				Physics::AdvancedCharacterController* acc = (Physics::AdvancedCharacterController*)ent->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				
				hkVector4 v = point.m_contact.getPosition();
				v.sub4( point.m_contact.getNormal() );
				
				float altitude = rb->getPosition()(1) - m_pobEntity->GetPosition().Y();
				CDirection dir = acc->GetRagdollLinearVelocity();

				bool gotITFromBody = false;
				Physics::AdvancedCharacterController* pobCharacterState2 = 0; // Collidee character controller
				if( 0 == m_pobHitBy )
				{
					m_pobHitBy = (CEntity*) rb->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
					pobCharacterState2=(Physics::AdvancedCharacterController*)m_pobHitBy->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
					gotITFromBody = true;
				}

				bool bThrown = false;

				if ( pobCharacterState2 )
				{
					Physics::RAGDOLL_STATE eState = pobCharacterState2->GetRagdollState();

					if ( eState == DEAD || eState == ANIMATED )
					{
						bThrown = true;
					}
				}

				if(	( ( dir.Length() > 9.0f ) && ( altitude > 0.3f) ) 
					|| bThrown )
				{

					if (bThrown && pobCharacterState2->GetRagdollState() != ANIMATED)
					{
						pobCharacterState2->SetRagdollDead();
					}

					if( gotITFromBody )
					{						
					    if (ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GetStrikeCount()==0)
						{
							if (grandf(1.0f) > 0.5f)
								ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData2);
							else
								ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData3);
						}
						else
						{
							ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData);
						}
					}
				}
			}
		}
	}

	#define fHURT_TRESHOLD 8.f

	void CharacterInteractionListener::objectInteractionCallback(hkCharacterProxy* proxy, const hkCharacterObjectInteractionEvent& input, hkCharacterObjectInteractionResult& output )
	{
		// Check that we hit a rigidbody
		if (!input.m_body)
		{
			return;
		}

		// This is to prevent the listener from registering collisions with the same entity more than once per frame
		#ifdef _PREVENT_DUPLICATE_COLLISIONS
		if (input.m_body==m_pobLastCollidee)
		{
			return;
		}
		else
		{
			m_pobLastCollidee=input.m_body;
		}
		#endif // _PREVENT_DUPLICATE_COLLISIONS

		CEntity* pobCollidee=(CEntity*) input.m_body->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();

		if (pobCollidee)
		{
			if(m_pobEntity->IsEnemy())
			{
				Physics::SingleRigidLG* pobRigidBodyState2 = (Physics::SingleRigidLG*) pobCollidee->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::SINGLE_RIGID_BODY_LG );

				if(( (fabs(input.m_projectedVelocity) > fHURT_TRESHOLD) && (input.m_body->getLinearVelocity().lengthSquared3() > fHURT_TRESHOLD*fHURT_TRESHOLD) )
					|| (pobRigidBodyState2 && pobRigidBodyState2->HasPiercingBehavior()) )
				{
					EntityCollisionFlag infoA;
					infoA.base = input.m_body->getCollidable()->getCollisionFilterInfo();
					
					if( infoA.flags.i_am & RAGDOLL_BIT )
					{					
					} 
					else
					{
						// ----- Rigid body piercing behavior -----

						Physics::SingleRigidLG* pobRigidBodyState = (Physics::SingleRigidLG*) pobCollidee->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::SINGLE_RIGID_BODY_LG );

						if (pobRigidBodyState && pobRigidBodyState->HasPiercingBehavior())
						{
							pobRigidBodyState->SetPiercingBehavior(false);


							if (!m_pobEntity->GetAttackComponent()->GetDisabled() && 
								m_pobEntity->ToCharacter()->GetCurrHealth()<10000.0f && 
								!m_pobEntity->ToCharacter()->IsInvulnerable() )
							{
								// Clear movement controllers and disable collision strike handler
								if(pobCollidee->GetMovement())
									pobCollidee->GetMovement()->ClearControllers();
								pobCollidee->GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable();

								Transform* pobThisTransform = pobCollidee->GetHierarchy()->GetRootTransform();
								Transform* pobTargetTransform = m_pobEntity->GetHierarchy()->GetCharacterBoneTransform(CHARACTER_BONE_PELVIS);

								// Find closest transform to the collision point

								CPoint obCollisionPoint(input.m_position(0),input.m_position(1),input.m_position(2));

								float fBestDist=FLT_MAX;

								// JML - Only consider real bones, there are transforms on ragdolls which don't correspond to visible bones.
								//       This can lead to very strange issues of weird swords under some (rare) circumstances.
								for(CHARACTER_BONE_ID eBone=CHARACTER_BONE_PELVIS; eBone<CHARACTER_BONE_R_KNEE; eBone = CHARACTER_BONE_ID(int(eBone)+1))
								{
									Transform* pTrans = m_pobEntity->GetHierarchy()->GetCharacterBoneTransform(eBone);
									if(!pTrans)
										continue;

									CDirection dDiff = pTrans->GetWorldTranslation() ^ obCollisionPoint;
									float fDist = dDiff.LengthSquared();
									if(fDist < fBestDist)
									{
										fBestDist = fDist;
										pobTargetTransform = pTrans;
									}
								}								
								
								if (!pobTargetTransform) // If for some bizarre reason we don't get a valid transform, revert back to using the root transform
									pobTargetTransform=m_pobEntity->GetHierarchy()->GetRootTransform();


								// Calculate the new translation								
								CPoint obNewTranslation(0.0f,0.0f,0.0f);
								CMatrix obLocalMatrix(pobThisTransform->GetLocalMatrix());
								obLocalMatrix*=pobTargetTransform->GetWorldMatrix().GetAffineInverse(); // Combine with the affine inverse of the parent
								pobThisTransform->SetLocalMatrix(obLocalMatrix);

								CDirection obCorrection(0.0f,-0.25f,0.0f); // -0.25f assumes the object is facing forward in the y axis, so it pushes it out slightly

								CDirection obOffsetCorrection=obCorrection * obLocalMatrix;

								obNewTranslation+=obOffsetCorrection;
								pobThisTransform->SetLocalTranslation(obNewTranslation);

								// Reparent this entity and transform to target
								pobCollidee->SetParentEntity( m_pobEntity );
								pobThisTransform->RemoveFromParent();
								pobTargetTransform->AddChild( pobThisTransform );

								// Deactivate the rigid body
								pobRigidBodyState->Deactivate();

								// Send a notification
								CMessageSender::SendEmptyMessage( "msg_hitragdoll", pobCollidee->GetMessageHandler() );
								
								if ( m_pobEntity->GetAttackComponent() )
								{
									// Find the attack data that wwe are going to hit the opponent with
									CAttackData* pobAttackData = ObjectDatabase::Get().GetPointerFromName< CAttackData* >( "atk_sword_impale" );
									if ( pobAttackData )
									{
										// Issue a strike - using the thrown entity as the originator - but the reaction position 
										// relative to the character that issued the attack
										CStrike* pobStrike = NT_NEW CStrike(	0, 
																			m_pobEntity, 
																			pobAttackData, 
																			1.0f, 
																			1.0f, 
																			false, 
																			false, 
																			false,
																			false,
																			false,
																			false,
																			0,
																			pobCollidee->GetPosition() );

										// Post the strike
										SyncdCombat::Get().PostStrike( pobStrike );
									}
								}
							}
						}

						// ----- Velocity reflection -----
						
						if (pobRigidBodyState)
						{
							Physics::DeflectionBehavior* pobDeflectionBehavior=(Physics::DeflectionBehavior*)pobRigidBodyState->GetDeflectionBehavior();

							if (pobDeflectionBehavior)
								pobDeflectionBehavior->ProcessDeflection();
						}

						if( pobCollidee->GetPhysicsSystem()->GetCollisionStrikeHandler()->IsEnabled() == false )
							return;

						if( ( input.m_body->getMotionType() == hkMotion::MOTION_FIXED ) || ( input.m_body->getMotionType() == hkMotion::MOTION_KEYFRAMED ) )
							return;
					
						Physics::AdvancedCharacterController* pobCharacterState=(Physics::AdvancedCharacterController*)m_pobEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
						if( !pobCharacterState->IsRagdollActive() )
						{	
							pobCollidee->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData);
							
							//if (pobCollidee->GetPhysicsSystem()->GetCollisionListener()->GetPhysicsEffectDefinition()) // Trigger a physics effect bounce
							//{
							//	hkVector4 colPoint = input.m_position;
							//	pobCollidee->GetPhysicsSystem()->GetCollisionListener()->GetPhysicsEffectDefinition()->AddBounceEvent(input.m_projectedVelocity,CPoint(colPoint(0),colPoint(1),colPoint(2)));
							//}

							// Trigger a bounce collision effect (direct replacement for above code)
							CollisionEffectHandler::CollisionEffectHandlerEvent obEvent;
							obEvent.m_eType = CollisionEffectManager::BOUNCE;
							obEvent.m_fRelevantVelocity = fabs(input.m_projectedVelocity);
							obEvent.m_obCollisionPosition = CPoint(input.m_position(0), input.m_position(1), input.m_position(2));
							obEvent.m_pobOther = pobCollidee->GetPhysicsSystem()->GetCollisionEffectHandler();
							m_pobEntity->GetPhysicsSystem()->GetCollisionEffectHandler()->ProcessEvent(obEvent);
						}

						// ----- If a compound rigid body is flagged as do hurt, then it should collapse on impact with a character -----
						Physics::CompoundLG* pobCompoundState = (Physics::CompoundLG*) pobCollidee->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::COMPOUND_RIGID_LG );

						if (pobCompoundState && !pobCompoundState->IsCollapsed())
						{
							Message CollapseMsg(msg_hitcharacter);
							pobCollidee->GetMessageHandler()->QueueMessage(CollapseMsg);

							pobCollidee->GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable();

							return;
						}
					}
				}
			}
			///////////////////////////
		}
	}

	void CharacterInteractionListener::characterInteractionCallback(hkCharacterProxy *proxy, hkCharacterProxy *otherProxy, const hkContactPoint &contact) 
	{
		// Might want to do sliding here?
	}

	void CharacterInteractionListener::processConstraintsCallback (const hkArray< hkRootCdPoint > &manifold, hkSimplexSolverInput &input) 
	{
		if( m_pobHitBy )
		{
			Physics::AdvancedCharacterController* pobCharacterState=(Physics::AdvancedCharacterController*)m_pobHitBy->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if(pobCharacterState->GetAdvancedRagdoll() && !pobCharacterState->GetAdvancedRagdoll()->IsMoving() )
				m_pobHitBy = 0;
		}

		const int numConstraints = input.m_numConstraints;
		
		{
			
			for (int i=0; (i < numConstraints); i++)
			{
				hkSurfaceConstraintInfo& surface	= input.m_constraints[i];

				surface.m_extraUpStaticFriction		= 0.0f;
				surface.m_extraDownStaticFriction	= 0.0f;
				surface.m_staticFriction			= 0.0f;
				
				const hkReal surfaceVerticalComponent = surface.m_plane.dot3( hkVector4(0.0f, 1.0f, 0.0f, 0.0f) );

				if ( surfaceVerticalComponent < m_pobCC->GetMaxSlopeCosine() )
				{
					// slide along the plane with reduced velocity
					surface.m_dynamicFriction			= 0.5f;
				}
				else
				{
					// move freely 
					surface.m_dynamicFriction			= 0.0f;
				}
			};

			for (int i=0; i < manifold.getSize() ; i++)
			{
				hkRigidBody* rb =hkGetRigidBody( manifold[i].m_rootCollidableA );
				if( !rb )
					rb =hkGetRigidBody( manifold[i].m_rootCollidableB );

				if( rb && !rb->isFixedOrKeyframed())
				{
					// Let's try to compute the max y altitude
					hkAabb aabb;
					hkTransform obRBTransform;
                    rb->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obRBTransform);
					rb->getCollidable()->getShape()->getAabb(obRBTransform, 0.01f, aabb);
					if( aabb.m_max(1) >  m_pobCC->GetPosition()(1) - MAX_RB_CLIMB_ALTITUDE_DELTA )
					{
						// slide along the plane with reduced velocity
						input.m_constraints[i].m_dynamicFriction			= 1.0f;
						if( input.m_constraints[i].m_plane(1) > 0.0f )
						{
							hkVector4 orig = input.m_constraints[i].m_plane;
							orig.mul4( input.m_constraints[i].m_plane(3) );
							hkVector4 target = input.m_constraints[i].m_plane;
							target(1) = 0.0f;
							if (target.lengthSquared3()>EPSILON)
								target.normalize3();
							target(3) = orig.dot3( target );
							input.m_constraints[i].m_plane = target;
						}
					} 
				}
			}			
		};
	}

	// FOR RIGID BODY CHARACTERS
	RigidCharacterInteractionListener::RigidCharacterInteractionListener ( CEntity* pobMyEntity,  CharacterController* pobCC ) :
		m_pobEntity(pobMyEntity),
		m_pobLastCollidee(0),
		m_pobCC(pobCC)
	{
		m_pobAttackData=ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_collision");
		m_pobAttackData2=ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_collision2");
		m_pobAttackData3=ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_collision3");
		m_pobDieData = ObjectDatabase::Get().GetPointerFromName<CAttackData*>("atk_throw_body");

		m_pobHitBy = 0;
	}

	void RigidCharacterInteractionListener::Update()
	{
		m_pobLastCollidee=0; // Clear the pointer to our last collidee each frame
	}

	void  RigidCharacterInteractionListener::contactPointAddedCallback (hkContactPointAddedEvent &event) // This entity has a collision registered against it
	{		
		// Get pointer to collidee rigid body
		hkRigidBody* rb = 0; // This is the one we'll actually use, the following 2 are just temp values
		hkRigidBody* rbA = hkGetRigidBody( event.m_bodyA.getRootCollidable() ); 
		hkRigidBody* rbB = hkGetRigidBody( event.m_bodyB.getRootCollidable() ); 

		// Find out which one is non char controller, if both of them are char control, return
		EntityCollisionFlag infoA;
		if (rbA)
			infoA.base = rbA->getCollidable()->getCollisionFilterInfo();
		EntityCollisionFlag infoB;
		if (rbB)
			infoB.base = rbB->getCollidable()->getCollisionFilterInfo();

		// If we're both character RBs, might need to do some KO handling
		if ( 
			(infoA.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT || infoA.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT )
			&&
			(infoB.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT || infoB.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT )
			)
		{
			// Find the entity that isn't us
			CEntity* ent = (CEntity*) rbA->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			if (ent && ent == m_pobEntity)
				ent = (CEntity*) rbB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
			
			// If the other guy is KO'd, hasn't been hit by a KO himself, and is a certain grace height above us, then we need to KO
			if (ent && ent->GetAttackComponent()->AI_Access_GetState() == CS_KO && !ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GetStruckByKO() && (ent->GetPosition().Y() - m_pobEntity->GetPosition().Y()) > 0.3f )
			{
				if (ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GetStrikeCount()==0)
				{
					if (grandf(1.0f) > 0.5f)
						ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData2);
					else
						ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData3);
				}
				else
				{
					ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData);
				}

				ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->SetStruckByKO();
			}

			// We're both characters, bail
			return;
		}

		if (infoA.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT || infoA.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT )
		{
			// A is the character controller, use B
			rb = rbB;
		}
		else
		{
			// B is the character controller, use A
			rb = rbA;
		}

		if( rb )
		{
			EntityCollisionFlag info;
			info.base = rb->getCollidable()->getCollisionFilterInfo();
						
			CEntity* ent = (CEntity*) rb->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr(); // Get pointer of collidee entity	
			if (ent)
			{
				if ( ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->IsEnabled() == false )
					return;

				hkVector4 v = event.m_contactPoint->getPosition();
				v.sub4( event.m_contactPoint->getNormal() );
				
				float altitude = rb->getPosition()(1) - m_pobEntity->GetPosition().Y();
				CDirection dir = Physics::MathsTools::hkVectorToCDirection(rb->getLinearVelocity());

				if(	dir.Length() > 9.0f && altitude > 0.3f )
				{
					if ( ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GetStrikeCount() == 0 )
					{
						if (grandf(1.0f) > 0.5f)
							ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData2);
						else
							ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData3);
					}
					else
					{
						ent->GetPhysicsSystem()->GetCollisionStrikeHandler()->GenerateStrike(m_pobEntity,m_pobAttackData);
					}
				}
			}
		}
	}

#endif

}
