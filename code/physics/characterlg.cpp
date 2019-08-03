//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/CharacterController.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.09
//!
//---------------------------------------------------------------------------------------------------------

#include "core/user.h"

#include "config.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "advancedcharactercontroller.h"

#include "system.h"
#include "compoundlg.h"
#include "animatedlg.h"
#include "maths_tools.h"
#include "interactionlistener.h"
#include "physicsTools.h"

#include "game/entitymanager.h"
#include "core/exportstruct_clump.h"
#include "core/exportstruct_anim.h"
#include "anim/hierarchy.h"
#include "Physics/world.h"
#include "game/attacks.h"
#include "game/syncdcombat.h"
#include "game/strike.h"
#include "game/messagehandler.h"
#include "game/interactioncomponent.h"
#include "game/entityai.h"
#include "collisionlistener.h"
#include "physics/havokthreadutils.h"

#include "havokincludes.h"

#include "collisionbitfield.h"
#include "world.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/hkmath.h>
#include <hkanimation/rig/hkPose.h>
#include <hkanimation/rig/hkSkeleton.h>
#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkcollide\collector\raycollector\hkClosestRayHitCollector.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/phantom/hkShapePhantom.h>
#include <hkcollide/castutil/hkLinearCastInput.h>
#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkdynamics/phantom/hkShapePhantom.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.h>
#include <hkcollide/collector/pointcollector/hkAllCdPointCollector.h>
#include <hkcollide/collector/bodypaircollector/hkAllCdBodyPairCollector.h>
#include <hkcollide/collector/pointcollector/hkClosestCdPointCollector.h>
#include <hkcollide/agent/hkCdPointCollector.h>
#include <hkcollide/collector/pointcollector/hkRootCdPoint.h>
#include <hkcollide/agent/hkCdBody.h>
#include <hkcollide/agent/hkCdPoint.h>
#include <hkdynamics/phantom/hkAabbPhantom.h>
#include <hkmath/basetypes/hkAabb.h>
#include <hkutilities/charactercontrol/statemachine/util/hkCharacterMovementUtil.h>
#include <hkutilities/charactercontrol/characterproxy/hkCharacterProxyListener.h>
#include <hkdynamics/phantom/hkPhantomOverlapListener.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkconstraintsolver/simplex/hkSimplexSolver.h>
#include <hkdynamics/action/hkArrayAction.h>
#include <hkcollide/castutil/hkWorldRayCastInput.h>
#include <hkcollide/castutil/hkWorldRayCastOutput.h>
#include <hkcollide/collector/raycollector/hkAllRayHitCollector.h>
#include <hkutilities/inertia/hkInertiaTensorComputer.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkdynamics/constraint/contact/hkSimpleContactConstraintData.h>
#endif

#include "core/visualdebugger.h"
#include "core/gatso.h"

#include "camera/camutils.h"

namespace Physics {

	static bool isFixedOrKeyframed( const hkRigidBody* const bd )
	{
		return( ( bd->getMotionType() == hkMotion::MOTION_FIXED ) || ( bd->getMotionType() == hkMotion::MOTION_KEYFRAMED ) );
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::Construct
	*
	*	DESCRIPTION		Make an instance.
	*
	***************************************************************************************************/
	CharacterController* CharacterController::Construct( Character* p_ent, CColprimDesc* obColprim, CharacterControllerType eType)
	{
		CharacterController* pobCharacter = 0;

		switch ( eType )
		{
		case CharacterController::FULL_CONTROLLER:
			{
				pobCharacter = NT_NEW FullCharacterController( p_ent, obColprim );
				break;
			}
		case CharacterController::RIGID_BODY:
			{
				pobCharacter = NT_NEW RigidCharacterController( p_ent, obColprim );
				break;
			}
		case CharacterController::RAYCAST_PHANTOM:
			{
				pobCharacter = NT_NEW RaycastPhantomCharacterController( p_ent, obColprim );
				break;
			}
		case CharacterController::DUMMY:
			{
				pobCharacter = NT_NEW DummyCharacterController( p_ent, obColprim );
				break;
			}
		case CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT:
			{
				ntError( 0 );
			}
		}

		ntError( pobCharacter );

		return pobCharacter;
	}

	/***************************************************************************************************
	*
	*	CLASS			CharacterController::CharacterController
	*
	*	DESCRIPTION		Construct abstract CharacterController with common values.
	*
	***************************************************************************************************/
	CharacterController::CharacterController( Character* p_entity, CColprimDesc* pobColprim )
	{
		m_entity = p_entity;
		m_isActive = false;
		m_world = NULL;
		m_bDoneGroundCheckThisFrame = false;
		m_bGravity = true;
		m_bApplyMovementAbsolutely = false;
		m_obPositionOnNextActivation.Clear();
		m_bPositionOnNextActivationSet = false;
		m_pobColPrim = 0;		
		m_pobCombatPhysicsPushVolumeDescriptors = 0;
		m_pobCombatPhysicsPushVolumeData = 0; 
		m_bGrounded = false;
		m_bUseHolding = false;
		m_pobColPrim = pobColprim;
		m_standShape = 0;
		m_holdingShape = 0;
		m_obEntityCFlag.base = 0;

		m_pobRoot = m_entity->GetHierarchy()->GetRootTransform();
		m_softParent = 0; 
		m_relToSoftParent = 0;
	}

	/***************************************************************************************************
	*
	*	CLASS			CharacterController::~CharacterController
	*
	*	DESCRIPTION		Destroy the abstract character controller.
	*
	***************************************************************************************************/
	CharacterController::~CharacterController( )
	{
		if (m_standShape)
			m_standShape->removeReference();

		if (m_holdingShape)
			m_holdingShape->removeReference();

		NT_DELETE(m_relToSoftParent);
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::CleanupCombatPhysicsPushVolumeDescriptors
	*
	*	DESCRIPTION		Remove the currently active combat physics volumes.
	*
	***************************************************************************************************/
	void CharacterController::RemoveCombatPhysicsPushVolumesFromWorld()
	{
		GATSO_PHYSICS_START("CharacterController::RemoveCombatPhysicsPushVolumesFromWorld");
		if (!m_world)
			return;

		Physics::WriteAccess mutex;

		if (m_pobCombatPhysicsPushVolumeDescriptors)
		{
			PushVolumeDataList::iterator obIt = m_pobCombatPhysicsPushVolumeData->begin();
			while (obIt != m_pobCombatPhysicsPushVolumeData->end())
			{
				if ((*obIt)->m_pobPhantom && (*obIt)->m_pobPhantom->getWorld())
				{
					//(*obIt)->m_pobPhantom->getCollidableRw()->setCollisionFilterInfo(0);
					//CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom((*obIt)->m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
					m_world->removePhantom((*obIt)->m_pobPhantom);					
				}
				obIt++;
			}
		}

		GATSO_PHYSICS_STOP("CharacterController::RemoveCombatPhysicsPushVolumesFromWorld");
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::CleanupCombatPhysicsPushVolumeDescriptors
	*
	*	DESCRIPTION		Remove (and destruct if neccessary) the given combat physics volumes.
	*
	***************************************************************************************************/
	void CharacterController::CleanupCombatPhysicsPushVolumeDescriptors( PushVolumeDataList* pobCombatPhysicsPushVolumeData)
	{
		Physics::WriteAccess mutex;

		if (pobCombatPhysicsPushVolumeData)
		{
			PushVolumeDataList::iterator obIt = pobCombatPhysicsPushVolumeData->begin();
			while (obIt != pobCombatPhysicsPushVolumeData->end())
			{
				if ((*obIt)->m_pobPhantom && (*obIt)->m_pobPhantom->getWorld())
				{
					m_world->removePhantom((*obIt)->m_pobPhantom);
				}
				else if ((*obIt)->m_pobPhantom)
				{
					(*obIt)->m_pobPhantom->removeReference();
					(*obIt)->m_pobPhantom = 0;
				}

				obIt++;
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::AddCombatPhysicsPushVolumesToWorld
	*
	*	DESCRIPTION		Add (and construct if neccessary) the currently active combat physics volumes.
	*
	***************************************************************************************************/
	void CharacterController::AddCombatPhysicsPushVolumesToWorld()
	{
		GATSO_PHYSICS_START("CharacterController::AddCombatPhysicsPushVolumesToWorld");
		if ( !m_world )
			return;

		Physics::WriteAccess mutex;

		if (m_pobCombatPhysicsPushVolumeDescriptors && m_pobCombatPhysicsPushVolumeData)
		{
			PushVolumeDescriptorList::iterator										obIt = m_pobCombatPhysicsPushVolumeDescriptors->begin();
			ntstd::List<CombatPhysicsPushVolumeData *, Mem::MC_ENTITY>::iterator 	obItData = m_pobCombatPhysicsPushVolumeData->begin();			

			while (obIt != m_pobCombatPhysicsPushVolumeDescriptors->end())
			{
				if (!(*obItData)->m_pobPhantom || !(*obItData)->m_pobPhantom->getWorld())
				{
					int iIndex = m_entity->GetHierarchy()->GetTransformIndex(((*obIt)->m_obBindToTransform));
					if (iIndex > -1)
					{
						CMatrix obMtx = m_entity->GetHierarchy()->GetTransform(iIndex)->GetWorldMatrix();
						hkTransform obTransform(Physics::MathsTools::CQuatTohkQuaternion(CQuat(obMtx)),Physics::MathsTools::CPointTohkVector(obMtx.GetTranslation()));

						Physics::EntityCollisionFlag obCollisionFlag;
						obCollisionFlag.base = m_obEntityCFlag.base;
						obCollisionFlag.flags.i_collide_with = (	Physics::RAGDOLL_BIT						|
																	Physics::SMALL_INTERACTABLE_BIT				|
																	Physics::LARGE_INTERACTABLE_BIT				|
																	// Added for strike2 generation
																	Physics::CHARACTER_CONTROLLER_ENEMY_BIT		|
																	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	);

						if (!(*obItData)->m_pobPhantom)
						{
							hkShape* pobShape = 0;
							if ((*obIt)->m_fSphereRadius > 0.0f)
							{
								pobShape = HK_NEW hkSphereShape((*obIt)->m_fSphereRadius);
							}
							else if ((*obIt)->m_fCapsuleRadius > 0.0f && (*obIt)->m_fCapsuleLength > 0.0f)
							{
								hkVector4 obVertexA = obTransform.getTranslation();
								hkVector4 obVertexB = obVertexA;
								obVertexB(1) += (*obIt)->m_fCapsuleLength;
								obVertexA.sub4(obTransform.getTranslation());
								obVertexB.sub4(obTransform.getTranslation());
								
								pobShape = HK_NEW hkCapsuleShape(obVertexA,obVertexB,(*obIt)->m_fCapsuleRadius);
							}
							else if ((*obIt)->m_fBoxHalfExtentX > 0.0f && (*obIt)->m_fBoxHalfExtentY > 0.0f && (*obIt)->m_fBoxHalfExtentZ > 0.0f)
							{
								pobShape = HK_NEW hkBoxShape(hkVector4((*obIt)->m_fBoxHalfExtentX,(*obIt)->m_fBoxHalfExtentY,(*obIt)->m_fBoxHalfExtentZ));
							}

							ntError(pobShape);

							(*obItData)->m_pobPhantom = HK_NEW hkSimpleShapePhantom(pobShape, obTransform, (int)obCollisionFlag.base);
							
							Physics::FilterExceptionFlag exceptionFlag; 
							exceptionFlag.base = 0;
							exceptionFlag.flags.exception_set |= Physics::IGNORE_FIXED_GEOM;
							//exceptionFlag.flags.exception_set |= Physics::IGNORE_CCs;
							hkPropertyValue val2((int)exceptionFlag.base);	
							(*obItData)->m_pobPhantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);
							hkPropertyValue val((void*)m_entity);
							(*obItData)->m_pobPhantom->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
						}

						if (!(*obItData)->m_pobPhantom->getWorld())
							m_world->addPhantom((*obItData)->m_pobPhantom);

						(*obItData)->m_pobPhantom->getCollidableRw()->setCollisionFilterInfo((int)obCollisionFlag.base);
						CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom((*obItData)->m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);

						ntError((*obItData)->m_pobPhantom && (*obItData)->m_pobPhantom->getWorld());

						// do not care about phantom position at the moment, before it will be used position will be set.
						// if position is set now that's overwrite m_obPreviousPosition 
						/*hkVector4 obXAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetXAxis());
						obXAxis.mul4((*obIt)->m_fXAxisPositionOffset);
						hkVector4 obYAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetYAxis());
						obYAxis.mul4((*obIt)->m_fYAxisPositionOffset);
						hkVector4 obZAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetZAxis());
						obZAxis.mul4((*obIt)->m_fZAxisPositionOffset);
						obTransform.getTranslation().add4(obXAxis);
						obTransform.getTranslation().add4(obYAxis);
						obTransform.getTranslation().add4(obZAxis);
						(*obItData)->m_pobPhantom->setTransform(obTransform);
						(*obItData)->m_obPreviousPosition = obTransform.getTranslation();*/
					}
				}

				obIt++;
				obItData++;
			}
		}

		GATSO_PHYSICS_STOP("CharacterController::AddCombatPhysicsPushVolumesToWorld");
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::UpdateCombatPhysicsPushVolumes
	*
	*	DESCRIPTION		Get all collisions with the feet phantom and apply forces outwards.
	*
	***************************************************************************************************/
	void CharacterController::UpdateCombatPhysicsPushVolumes( float p_timestep )
	{
		GATSO_PHYSICS_START("CharacterController::UpdateCombatPhysicsPushVolumes");
		
		
		Physics::WriteAccess mutex;

		// Add them to the world 
		AddCombatPhysicsPushVolumesToWorld();

		if (m_pobCombatPhysicsPushVolumeDescriptors && m_pobCombatPhysicsPushVolumeDescriptors->size() > 0)
		{
			PushVolumeDescriptorList::iterator obIt = m_pobCombatPhysicsPushVolumeDescriptors->begin();
			PushVolumeDataList::iterator obItData = m_pobCombatPhysicsPushVolumeData->begin();
			while (obIt != m_pobCombatPhysicsPushVolumeDescriptors->end())
			{				
				(*obItData)->ResetPreviousRBsWithAgeGreaterThan(1);

				int iIndex = m_entity->GetHierarchy()->GetTransformIndex(((*obIt)->m_obBindToTransform));
				if (iIndex > -1)
				{
					// Set position and rotation from transform in hierarchy
					CMatrix obMtx = m_entity->GetHierarchy()->GetTransform(iIndex)->GetWorldMatrix();
					CPoint obTranslation( obMtx.GetTranslation() );
					obMtx.SetTranslation( CPoint( CONSTRUCT_CLEAR ) );
					// Take into account rotational offsets
					CMatrix obRotationOffsetMatrix( CONSTRUCT_IDENTITY );
					if ( (*obIt)->m_fXAxisRotationOffset != 0.0f || (*obIt)->m_fYAxisRotationOffset != 0.0f || (*obIt)->m_fZAxisRotationOffset != 0.0f )
						CCamUtil::MatrixFromEuler_XYZ( obRotationOffsetMatrix, (*obIt)->m_fXAxisRotationOffset * DEG_TO_RAD_VALUE, (*obIt)->m_fYAxisRotationOffset * DEG_TO_RAD_VALUE, (*obIt)->m_fZAxisRotationOffset * DEG_TO_RAD_VALUE );
					obMtx = obRotationOffsetMatrix * obMtx;
					obMtx.SetTranslation( obTranslation );
					hkTransform obTransform(Physics::MathsTools::CQuatTohkQuaternion(CQuat(obMtx)),Physics::MathsTools::CPointTohkVector(obMtx.GetTranslation()));
					// Shift it along any positional offsets
					hkVector4 obXAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetXAxis());
					obXAxis.mul4((*obIt)->m_fXAxisPositionOffset);
					hkVector4 obYAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetYAxis());
					obYAxis.mul4((*obIt)->m_fYAxisPositionOffset);
					hkVector4 obZAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetZAxis());
					obZAxis.mul4((*obIt)->m_fZAxisPositionOffset);
					obTransform.getTranslation().add4(obXAxis);
					obTransform.getTranslation().add4(obYAxis);
					obTransform.getTranslation().add4(obZAxis);
					// Record it's last position so we get a psuedo velocity
					(*obItData)->m_obPreviousPosition = (*obItData)->m_pobPhantom->getTransform().getTranslation();
					// Set it
					(*obItData)->m_pobPhantom->setTransform(obTransform);
				
					// Collect all penetrations that collide with me
					GATSO_PHYSICS_START("CharacterController::GetPenetrations");
					hkAllCdBodyPairCollector obCollector; 
					Physics::CPhysicsWorld::Get().GetPenetrations((*obItData)->m_pobPhantom->getCollidable(), (hkCollisionInput&)*Physics::CPhysicsWorld::Get().GetCollisionInput(), obCollector);
					GATSO_PHYSICS_STOP("CharacterController::GetPenetrations");
					//(*obItData)->m_pobPhantom->getPenetrations(obCollector);
					for( int i = 0; i < obCollector.getHits().getSize(); ++i )
					{
						// Get a rigid body or phantom
						const hkCollidable* collidable = obCollector.getHits()[i].m_rootCollidableB;
						Physics::EntityCollisionFlag infoA;
						infoA.base = collidable->getCollisionFilterInfo();

						hkRigidBody* pobRB = hkGetRigidBody(collidable);
						hkPhantom* pobPhantom = hkGetPhantom(collidable);

						if (pobRB)
						{
							if ((infoA.flags.i_am & Physics::CHARACTER_CONTROLLER_PLAYER_BIT) || 
								(infoA.flags.i_am & Physics::CHARACTER_CONTROLLER_ENEMY_BIT))
							{
								// hmmm do nothing for rigid character controllers
								continue;
							}

							Physics::WriteAccess mutex;

							if (((*obIt)->m_bPushOnce && (*obItData)->IsRBInPrevious(pobRB)))
							{
								// Don't want to do anything here because we're set to only push once and the RB is still within the volume
								// This'll keep it hanging around in our list until we no longer collide with it
								(*obItData)->DecrementRBAge(pobRB);
							}
							else
							{								
								// Bring it into simulation
								pobRB->activate();

								// Check per entity specific collision status
								CEntity* pobEntity = 0;
								if( pobRB && pobRB->hasProperty(PROPERTY_ENTITY_PTR) )
									pobEntity = (CEntity*)pobRB->getProperty(PROPERTY_ENTITY_PTR).getPtr();
								if( pobEntity && m_entity->GetInteractionComponent()->CanCollideWith(pobEntity) )
								{
									// Try to calculate some direction to apply the impulse in
									hkVector4 obRBPosition = pobRB->getPosition();
									float fMagnitude = 0.0f; // Need this to see if it's worth pushing
									hkVector4 obMovement(0,0,0,0);		
									if ((*obIt)->m_bUseMovementOfVolume)
									{
										// Movement vector of this volume since the last frame
										hkVector4 obVolumeMovement = (*obItData)->m_pobPhantom->getTransform().getTranslation();
										obVolumeMovement.sub4((*obItData)->m_obPreviousPosition);
										float fMovement = obVolumeMovement.length3();
										if (fMovement > 0.0f)
										{
											fMagnitude += obVolumeMovement.length3(); // Need this to see if it's worth pushing
											obVolumeMovement.normalize3();
											obMovement.add4(obVolumeMovement);
										}
									}
									if ((*obIt)->m_bUseFromCentreOfVolume)
									{
										// Vector from the centre of the volume to the RB
										hkVector4 obFromCentre = obRBPosition;
										obFromCentre.sub4((*obItData)->m_pobPhantom->getTransform().getTranslation());
										float fMovement = obFromCentre.length3();
										if (fMovement > 0.0f)
										{
											fMagnitude += fMovement;
											obFromCentre.normalize3();	
											obMovement.add4(obFromCentre);
										}
									}									
									obMovement.normalize3();

									// Make sure its collision details have been initialised properly, don't have to do this, but it just makes it obvious if an RB hasn't been setup properly
									
									if(	( infoA.base != 0 )		&&
										( infoA.base != 65536 ) )
									{
										// Is it worth doing anything?
										if( fMagnitude > EPSILON )
										{
											// Combine multipliers
											float fFactor = 1.0f;
											if ((*obIt)->m_bUseMassMultiplier)
												fFactor *= 1.0f + pobRB->getMass(); // Adding one so it never slows effect
											if ((*obIt)->m_bUseMagnitudeMultiplier)
												fFactor *= 1.0f + fMagnitude; // Adding one so it never slows effect

											if(infoA.flags.i_am & Physics::LARGE_INTERACTABLE_BIT)
											{
												obMovement.mul4((*obIt)->m_fPushFactorLargeInteractable * fFactor);
											}
											if(infoA.flags.i_am & Physics::SMALL_INTERACTABLE_BIT)
											{
												obMovement.mul4((*obIt)->m_fPushFactorSmallInteractable * fFactor);
											}
											if(infoA.flags.i_am & Physics::RAGDOLL_BIT)
											{
												obMovement.mul4((*obIt)->m_fPushFactorRagdoll * fFactor);
											}											

											// If this is collapsable and we're flagged as being able to collapse it... collapse it.
											Physics::CompoundLG* pobCollapsable = (Physics::CompoundLG*)pobEntity->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::COMPOUND_RIGID_LG);
											if (pobCollapsable && !pobCollapsable->IsCollapsed())
											{
												if (((*obIt)->m_bCanCollapseCollapsablesWhenNotAttacking) || ((*obIt)->m_bCanCollapseCollapsablesWhenAttacking && m_entity->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING))
												{
													// Flag for collapse
													Message msgSwordStrike(msg_sword_strike);
													msgSwordStrike.SetEnt( CHashedString(HASH_STRING_OTHER), m_entity);
													pobEntity->GetMessageHandler()->QueueMessage(msgSwordStrike);
													obMovement.mul4((*obIt)->m_fPushFactorCollapse);
												}
											}

											// If this is an animated collapsable and we're flagged as being able to collapse it... collapse it.
											Physics::AnimatedLG* pobAnimatedCollapsable = (Physics::AnimatedLG*)pobEntity->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
											if (pobAnimatedCollapsable && !pobAnimatedCollapsable->IsDynamic())
											{
												if (((*obIt)->m_bCanCollapseCollapsablesWhenNotAttacking) || ((*obIt)->m_bCanCollapseCollapsablesWhenAttacking && m_entity->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING))
												{
													// Flag for collapse
													Message msgSwordStrike(msg_sword_strike);
													msgSwordStrike.SetEnt( CHashedString(HASH_STRING_OTHER), m_entity);
													pobEntity->GetMessageHandler()->QueueMessage(msgSwordStrike);
													obMovement.mul4((*obIt)->m_fPushFactorCollapse);
												}
											}

											// Only apply the impulse if I'm in the right combat state
											if (!(*obIt)->m_bOnlyActiveWhenAttacking || ((*obIt)->m_bOnlyActiveWhenAttacking && m_entity->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING))
											{
												pobRB->applyPointImpulse( obMovement, obTransform.getTranslation() );
											}

											// We've just pushed this one, so keep a record of it
											(*obItData)->AddPreviousRB(pobRB);

											//ntPrintf("%s %f\n",pobEntity->GetName().c_str(),obRB->getMass());
										}					
									}
								}
							}
						}
						else if (pobPhantom)
						{
							Physics::WriteAccess mutex(pobPhantom);

							Physics::EntityCollisionFlag infoA;
							infoA.base = pobPhantom->getCollidable()->getCollisionFilterInfo();

							// Check per entity specific collision status
							CEntity* pobEntity = 0;
							if( pobPhantom && pobPhantom->hasProperty(PROPERTY_ENTITY_PTR) )
								pobEntity = (CEntity*)pobPhantom->getProperty(PROPERTY_ENTITY_PTR).getPtr();
							if ((*obIt)->m_bShouldStrikeCharactersWhenAttacking && pobEntity && pobEntity != m_entity && (infoA.flags.i_am & Physics::CHARACTER_CONTROLLER_ENEMY_BIT || infoA.flags.i_am & Physics::CHARACTER_CONTROLLER_PLAYER_BIT) )
							{
								// Is my parent entity in a strike 2 window? And is this guy ok to be hit?
								if (pobEntity->GetAttackComponent() && 
									m_entity->GetAttackComponent()->IsInStrike2Window() &&
									(pobEntity->GetAttackComponent()->AI_Access_GetState() == CS_STANDARD || 
									pobEntity->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING || 
									pobEntity->GetAttackComponent()->AI_Access_GetState() == CS_DEFLECTING) &&
									m_entity->GetAttackComponent()->AI_Access_GetCurrentStrike()->GetTargetP() != pobEntity )
								{
									// If so then strike this other entity
									CStrike* pobGeneratedStrike = NT_NEW_CHUNK( Mem::MC_MISC ) CStrike(	m_entity, 
													pobEntity, 
													m_entity->GetAttackComponent()->AI_Access_GetCurrentStrike()->GetAttackDataP(), 
													m_entity->GetAttackComponent()->AI_Access_GetCurrentStrike()->GetAttackTimeScalar(), 
													m_entity->GetAttackComponent()->AI_Access_GetCurrentStrike()->GetAttackRange(), 
													false,
													false,
													true,
													false,
													false,
													false,
													0,
													m_entity->GetPosition() );

									// Send to the opponent
									SyncdCombat::Get().PostStrike( pobGeneratedStrike );
								}
							}
						}
					}
				}

				// Age all the RBs so we know how long they've been hanging around and can clear out the old ones
				(*obItData)->IncrementPreviousRBsAge();

				++obIt;
				++obItData;
			}
		}

		RemoveCombatPhysicsPushVolumesFromWorld();

		GATSO_PHYSICS_STOP("CharacterController::UpdateCombatPhysicsPushVolumes");
	}

	/***************************************************************************************************
	*
	*	CLASS			CharacterController::SetCombatPhysicsPushVolumeDescriptors
	*
	*	DESCRIPTION		Sets the currently active list of combat physics volumes.
	*
	***************************************************************************************************/
	void CharacterController::SetCombatPhysicsPushVolumeDescriptors( PushVolumeDescriptorList* pobCombatPhysicsPushVolumeDescriptors, 
																	 PushVolumeDataList* pobCombatPhysicsPushVolumeData )
	{
		//RemoveCombatPhysicsPushVolumesFromWorld();
		m_pobCombatPhysicsPushVolumeDescriptors = pobCombatPhysicsPushVolumeDescriptors;
		m_pobCombatPhysicsPushVolumeData = pobCombatPhysicsPushVolumeData;
		//AddCombatPhysicsPushVolumesToWorld();
	}

	/***************************************************************************************************
	*
	*	CLASS			CharacterController::GetMaxSlope
	*
	*	DESCRIPTION		Maximum slope this can walk up.
	*
	***************************************************************************************************/
	float CharacterController::GetMaxSlope()
	{
		return 45.0f * DEG_TO_RAD_VALUE;
	}

	/***************************************************************************************************
	*
	*	CLASS			CharacterController::GetMaxSlope
	*
	*	DESCRIPTION		Cosine of maximum slope this can walk up.
	*
	***************************************************************************************************/
	float CharacterController::GetMaxSlopeCosine()
	{
		return hkMath::cos( GetMaxSlope() );
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::ComputeEntityPositionFromCharacter
	*
	*	DESCRIPTION		Using the position of the Havok capsule, this computes where the entities root
	*					transform should be.
	*
	***************************************************************************************************/
	CPoint CharacterController::ComputeEntityPositionFromCharacter() const
	{
		hkVector4 obOldPosition( this->GetPosition() );

		// Translate the old position into havok data
		CPoint obHKPosition(	obOldPosition(0),
								obOldPosition(1) - m_fCapsuleHalfLength,
								obOldPosition(2));
		return obHKPosition;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::ComputeCharacterPositionFromEntity
	*
	*	DESCRIPTION		Using the position of the root transform of the entity it represents, this 
	*					will return where the capsule should be.
	*
	***************************************************************************************************/
	hkVector4 CharacterController::ComputeCharacterPositionFromEntity(CPoint* pobPosition) const
	{
		CPoint obOldPosition;
		if (!pobPosition)
			obOldPosition = m_pobRoot->GetWorldMatrix().GetTranslation();
		else
			obOldPosition = *pobPosition;

		// Translate the old position into havok data
		hkVector4 obHKPosition(	obOldPosition.X(),
								obOldPosition.Y() + m_fCapsuleHalfLength,
								obOldPosition.Z());
		return obHKPosition;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::GetEntityPosition
	*
	*	DESCRIPTION		Returns root translation of entity this represents.
	*
	***************************************************************************************************/
	CPoint CharacterController::GetEntityPosition() const
	{
		return ComputeEntityPositionFromCharacter();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::GetEntityWorldMatrix
	*
	*	DESCRIPTION		Returns root transform of entity this represents.
	*
	***************************************************************************************************/
	void CharacterController::GetEntityWorldMatrix( CMatrix& obMatrix )
	{
		obMatrix = m_pobRoot->GetWorldMatrix();
		obMatrix.SetTranslation(GetEntityPosition());
	}
	CMatrix CharacterController::GetEntityWorldMatrix( )
	{
		CMatrix obMatrix = m_pobRoot->GetWorldMatrix();
		obMatrix.SetTranslation(GetEntityPosition());
		return obMatrix;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::IsActive
	*
	*	DESCRIPTION		Am I active.
	*
	***************************************************************************************************/
	bool CharacterController::IsActive()
	{
		return m_isActive;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::PausePresenceInHavokWorld
	*
	*	DESCRIPTION		Pause/Unpause phantoms presence in world.
	*
	***************************************************************************************************/
	void CharacterController::PausePresenceInHavokWorld(bool pause)
	{
		if (pause)
		{
			if (m_isActive)
				RemoveFromWorld(CPhysicsWorld::Get().GetHavokWorldP());
		}
		else
		{
			if (m_isActive)
				AddToWorld(CPhysicsWorld::Get().GetHavokWorldP());
		}
	};

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::IsActive
	*
	*	DESCRIPTION		This is where I'll be next time I'm activated.
	*
	***************************************************************************************************/
	void CharacterController::SetPositionOnNextActivation(CPoint& obPos)
	{
		m_obPositionOnNextActivation = obPos;
		m_bPositionOnNextActivationSet = true;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterController::SetSoftParent
	*
	*	DESCRIPTION		Attach character controller to entity
	*
	***************************************************************************************************/

	void CharacterController::SetSoftParent(CEntity * entity)
	{
		m_softParent = entity;

		if (!m_softParent)
			return;

		if (!m_relToSoftParent)
			m_relToSoftParent = NT_NEW CMatrix();

		*m_relToSoftParent = m_entity->GetRootTransformP()->GetWorldMatrix() * m_softParent->GetRootTransformP()->GetWorldMatrix().GetAffineInverse();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::RigidCharacterController
	*
	*	DESCRIPTION		Construct.
	*
	***************************************************************************************************/
	RigidCharacterController::RigidCharacterController( Character* p_entity, CColprimDesc* obColprim )
		: CharacterController( p_entity, obColprim )
	{
		m_pobBody = 0;
		m_pobHoldingBody = 0;
		m_pobAntiGravityAction = 0;
		m_pobAntiGravityActionHolding = 0;
		m_pobUberGravityAction = 0;
		m_pobUberGravityActionHolding = 0;
		m_pobCharacterInteractionListener = 0;

		SetupCharacterVolume();
	}

	class CharacterAntiGravityAction : public hkArrayAction 
	{
		public:

			CharacterAntiGravityAction( const hkArray<hkEntity*>& entities ):
				hkArrayAction( entities )
			{
			}

		private:

			virtual void applyAction( const hkStepInfo& stepInfo )
			{
				hkArray< hkEntity * > entitiesOut;
				getEntities(entitiesOut);
				for( int i = 0; i < entitiesOut.getSize(); i++)
				{
					hkRigidBody* m_body = (hkRigidBody*)entitiesOut[i];
					Physics::WriteAccess mutex(m_body);
					float fForce = m_body->getMass() * (-fGRAVITY);
					m_body->applyForce(stepInfo.m_deltaTime, hkVector4(0.0f,fForce,0.0f));
				}
			}
			virtual hkAction *  clone (const hkArray< hkEntity * > &newEntities, const hkArray< hkPhantom * > &newPhantoms) const { return 0; };
	};

	class CharacterUberGravityAction : public hkArrayAction 
	{
		public:

			CharacterUberGravityAction( const hkArray<hkEntity*>& entities ):
				hkArrayAction( entities )
			{
			}

		private:

			virtual void applyAction( const hkStepInfo& stepInfo )
			{
				hkArray< hkEntity * > entitiesOut;
				getEntities(entitiesOut);
				for( int i = 0; i < entitiesOut.getSize(); i++)
				{
					hkRigidBody* m_body = (hkRigidBody*)entitiesOut[i];
					Physics::WriteAccess mutex(m_body);
					float fForce = m_body->getMass() * (fGRAVITY);
					m_body->applyForce(stepInfo.m_deltaTime, hkVector4(0.0f,fForce,0.0f));
				}
			}
			virtual hkAction *  clone (const hkArray< hkEntity * > &newEntities, const hkArray< hkPhantom * > &newPhantoms) const { return 0; };
	};

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::~RigidCharacterController
	*
	*	DESCRIPTION		Destroy.
	*
	***************************************************************************************************/
	RigidCharacterController::~RigidCharacterController( )
	{
		Physics::WriteAccess mutex;

		if(GetBodyP()->isAddedToWorld())
			RemoveFromWorld( m_world );

		if (m_pobCharacterInteractionListener)
		{
			m_pobBody->removeCollisionListener(m_pobCharacterInteractionListener);
			m_pobHoldingBody->removeCollisionListener(m_pobCharacterInteractionListener);
			HK_DELETE( m_pobCharacterInteractionListener );
		}

		m_pobBody->removeReference();
		m_pobHoldingBody->removeReference();
		m_pobBody = 0;
		m_pobHoldingBody = 0;

		if (m_pobAntiGravityAction)
		{
			HK_DELETE( m_pobAntiGravityAction );
		}

		if (m_pobAntiGravityActionHolding)
		{
			HK_DELETE( m_pobAntiGravityActionHolding );
		}

		if (m_pobUberGravityAction)
		{
			HK_DELETE( m_pobUberGravityAction );
		}

		if (m_pobUberGravityActionHolding)
		{
			HK_DELETE( m_pobUberGravityActionHolding );
		}
	};

	CharacterAntiGravityAction* RigidCharacterController::GetAntiGravityAction()
	{
		if (m_bUseHolding)
		{
			return m_pobAntiGravityActionHolding;
		}
		else
		{
			return m_pobAntiGravityAction;
		}
	}

	CharacterUberGravityAction* RigidCharacterController::GetUberGravityAction()
	{
		if (m_bUseHolding)
		{
			return m_pobUberGravityActionHolding;
		}
		else
		{
			return m_pobUberGravityAction;
		}
	}
	
	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::Activate
	*
	*	DESCRIPTION		Active after calling this.
	*
	***************************************************************************************************/
	void RigidCharacterController::Activate( )
	{
		Physics::WriteAccess mutex;
		
		if( !m_isActive )
		{
			AddToWorld(CPhysicsWorld::Get().GetHavokWorldP());
			SetPosition( ComputeCharacterPositionFromEntity() );
		}

		m_isActive = true;
	};

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::UpdateCollisionFilter
	*
	*	DESCRIPTION		3 guesses what this function does. If you still get it wrong after 3 tries, 
	*					I'm going to do a service to society and lock you in a box.
	*
	***************************************************************************************************/
	void RigidCharacterController::UpdateCollisionFilter()
	{
		if (GetBodyP())
			GetBodyP()->getWorld()->updateCollisionFilterOnEntity( GetBodyP(), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK ,HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);   
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::Deactivate
	*
	*	DESCRIPTION		Won't be active after calling this.
	*
	***************************************************************************************************/
	void RigidCharacterController::Deactivate( )
	{
		if( m_isActive )
		{
			// Update hierarchy with my position. 					
			m_pobRoot->SetLocalTranslation( ComputeEntityPositionFromCharacter() );

			RemoveFromWorld(CPhysicsWorld::Get().GetHavokWorldP());
		}

		m_isActive = false;
	};

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::GetShapePhantomP
	*
	*	DESCRIPTION		Returns the shape phantom.
	*
	***************************************************************************************************/
	hkRigidBody*	 RigidCharacterController::GetBodyP () const
	{
		if (m_bUseHolding)
			return m_pobHoldingBody;
		else
			return m_pobBody;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::RegisterSystem
	*
	*	DESCRIPTION		Registers this with collision system.
	*
	***************************************************************************************************/
	void RigidCharacterController::RegisterSystem( System* p_system )
	{
		p_system->GetCollisionListener()->RegisterCharacterController( const_cast<Physics::RigidCharacterController*>(this) );
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetupCharacterVolume
	*
	*	DESCRIPTION		Creates and initialises the shapes and rigid bodies.
	*
	***************************************************************************************************/
	#define fPERCENTAGE_HOLDING_SIZE 1.0f
	#define fSIZE_SPHERE_STEP 0.16f
	void RigidCharacterController::SetupCharacterVolume()
	{
		// Only capsules are supported atm
		ntError(m_pobColPrim->m_eType == CV_TYPE_CAPSULE)

		const float fLength = m_pobColPrim->m_obCapsuleData.fLength;
		const float fRadius = m_pobColPrim->m_obCapsuleData.fRadius;

		m_fCapsuleHalfLength = (fLength * 0.5f) + fRadius;

		// Construct a shape
		hkVector4 vertexA(0, (fLength*0.5f), 0);
		hkVector4 vertexB(0, (fLength*-0.5f), 0);		

		// Create a capsule to represent the character standing
		m_standShape = HK_NEW hkCapsuleShape(vertexA, vertexB, fRadius);
		hkVector4 size; size.setSub4( vertexA, vertexB );
		//m_halfSize = (size(1) * 0.5f) + fRadius + 0.10f;

		float fIncrease = fPERCENTAGE_HOLDING_SIZE * fRadius;
		float fHoldRadius = fRadius + fIncrease;

		hkVector4 axis; axis.setSub4( vertexA, vertexB ); // vertexA - vertexB
		axis.normalize3(); axis.mul4(fIncrease);

		vertexB.add4(axis);
		vertexA.sub4(axis);

		// Allow the entity to modify the holding capsule, this code was added so that
		// the archer could use the holding capsule shape as a covering capsule

		{
			CPoint ptA = Physics::MathsTools::hkVectorToCPoint(vertexA);
			CPoint ptB = Physics::MathsTools::hkVectorToCPoint(vertexB);

			m_entity->ModifyPhysicsHoldingShape( ptA, ptB, fHoldRadius);

			vertexA = Physics::MathsTools::CPointTohkVector(ptA);
			vertexB = Physics::MathsTools::CPointTohkVector(ptB);
		}

		m_holdingShape = HK_NEW hkCapsuleShape(vertexA, vertexB, fHoldRadius);		

		m_obEntityCFlag.base = 0;
		if( m_entity->IsPlayer()|| m_entity->IsFriendly()  )
			m_obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_PLAYER_BIT;
		else 
			m_obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_ENEMY_BIT;

		if (m_entity->IsImportant())
			m_obEntityCFlag.flags.i_am_important = 1;

		if( m_entity->IsPlayer() || m_entity->IsBoss() || m_entity->IsFriendly() || ( m_entity->IsAI() && ((AI*)m_entity)->AttackAI() )  )
		{
			m_obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	| 
													Physics::RAGDOLL_BIT						|
													Physics::LARGE_INTERACTABLE_BIT				|
													Physics::SMALL_INTERACTABLE_BIT				|	
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
		} else {
			m_obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
													//Physics::RAGDOLL_BIT						|
													Physics::LARGE_INTERACTABLE_BIT				|	
													Physics::SMALL_INTERACTABLE_BIT				|	
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
		}

		hkRigidBodyCinfo obBodyInfo;
		obBodyInfo.m_collisionFilterInfo = m_obEntityCFlag.base;
		obBodyInfo.m_shape = m_standShape;
		obBodyInfo.m_position = ComputeCharacterPositionFromEntity();
		obBodyInfo.m_mass = 60.0f;
		obBodyInfo.m_motionType = hkMotion::MOTION_SPHERE_INERTIA;
		obBodyInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obBodyInfo.m_restitution = 0.0f;
		obBodyInfo.m_friction = 0.0f;
		obBodyInfo.m_linearDamping = 0.0f;
		obBodyInfo.m_angularDamping = 0.0f;
		obBodyInfo.m_solverDeactivation = hkRigidBodyCinfo::SOLVER_DEACTIVATION_OFF;
		obBodyInfo.m_rigidBodyDeactivatorType = hkRigidBodyDeactivator::DEACTIVATOR_NEVER;

		hkRigidBodyCinfo obHoldingBodyInfo;
		obHoldingBodyInfo.m_collisionFilterInfo = m_obEntityCFlag.base;
		obHoldingBodyInfo.m_shape = m_holdingShape;
		obHoldingBodyInfo.m_position = ComputeCharacterPositionFromEntity();
		obHoldingBodyInfo.m_mass = 60.0f;
		obHoldingBodyInfo.m_motionType = hkMotion::MOTION_SPHERE_INERTIA;
		obHoldingBodyInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obHoldingBodyInfo.m_restitution = 0.0f;
		obHoldingBodyInfo.m_friction = 0.0f;
		obHoldingBodyInfo.m_linearDamping = 0.0f;
		obHoldingBodyInfo.m_angularDamping = 0.0f;
		obHoldingBodyInfo.m_solverDeactivation = hkRigidBodyCinfo::SOLVER_DEACTIVATION_OFF;
		obHoldingBodyInfo.m_rigidBodyDeactivatorType = hkRigidBodyDeactivator::DEACTIVATOR_NEVER;

		hkMassProperties obBodyMassProperties;
		obBodyMassProperties.m_mass = obBodyInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(m_standShape->getVertex(0), m_standShape->getVertex(1), m_standShape->getRadius(), obBodyMassProperties.m_mass, obBodyMassProperties);
		obBodyInfo.m_inertiaTensor = obBodyMassProperties.m_inertiaTensor;
		// Mul the Y inertia by a massive amount so it never tips over 
		obBodyInfo.m_inertiaTensor.getColumn(1)(1) *= 1000000000.0f; 

		hkMassProperties obHoldingBodyMassProperties;
		obHoldingBodyMassProperties.m_mass = obHoldingBodyInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(m_holdingShape->getVertex(0), m_holdingShape->getVertex(1), m_holdingShape->getRadius(), obHoldingBodyMassProperties.m_mass, obHoldingBodyMassProperties);
		obHoldingBodyInfo.m_inertiaTensor = obHoldingBodyMassProperties.m_inertiaTensor;
		// Mul the Y inertia by a massive amount so it never tips over
		obHoldingBodyInfo.m_inertiaTensor.getColumn(1)(1) *= 1000000000.0f; 

		m_pobBody = HK_NEW hkRigidBody(obBodyInfo);
		m_pobHoldingBody = HK_NEW hkRigidBody(obHoldingBodyInfo);

		Physics::WriteAccess mutex;

		m_pobBody->setCollisionFilterInfo((int)m_obEntityCFlag.base);
		m_pobHoldingBody->setCollisionFilterInfo((int)m_obEntityCFlag.base);
		
		hkPropertyValue val((void*)m_entity);
		m_pobBody->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		m_pobHoldingBody->addProperty(Physics::PROPERTY_ENTITY_PTR, val);

		m_pobBody->setName(m_entity->GetName().c_str());
		m_pobHoldingBody->setName(m_entity->GetName().c_str());

		hkArray<hkEntity*> array, arrayHolding;
		array.pushBack( m_pobBody );
		arrayHolding.pushBack( m_pobHoldingBody );
		m_pobAntiGravityAction = HK_NEW CharacterAntiGravityAction(array);
		m_pobAntiGravityActionHolding = HK_NEW CharacterAntiGravityAction(arrayHolding);
		m_pobUberGravityAction = HK_NEW CharacterUberGravityAction(array);
		m_pobUberGravityActionHolding = HK_NEW CharacterUberGravityAction(arrayHolding);

		m_pobCharacterInteractionListener = HK_NEW RigidCharacterInteractionListener(m_entity,this);
		m_pobBody->addCollisionListener(m_pobCharacterInteractionListener);
		m_pobHoldingBody->addCollisionListener(m_pobCharacterInteractionListener);
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::RemoveFromWorld
	*
	*	DESCRIPTION		Remove everything (character phantom, feet phantom, associated listenes).
	*
	***************************************************************************************************/
	void RigidCharacterController::RemoveFromWorld (hkWorld* pWorld)
	{
		Physics::WriteAccess mutex;

		RemoveCombatPhysicsPushVolumesFromWorld();

		if (m_pobBody->getWorld())
		{
			m_world->removeEntity(m_pobBody);
		}

		if (m_pobHoldingBody->getWorld())
		{
			m_world->removeEntity(m_pobHoldingBody);
		}			
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::GetLinearVelocity
	*
	*	DESCRIPTION		Returns linear velocity of the character controller, or zero if there isn't one.
	*
	***************************************************************************************************/
	hkVector4 RigidCharacterController::GetLinearVelocity( )
	{	
		return GetBodyP()->getLinearVelocity();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::IsCharacterControllerOnGround
	*
	*	DESCRIPTION		Returns true if the character controller is supported by something below them.
	*
	***************************************************************************************************/
	bool RigidCharacterController::IsCharacterControllerOnGround( void ) 	
	{
		bool bSetGrounded = false;

		if (!m_bDoneGroundCheckThisFrame)
		{
			Physics::ReadAccess mutex(GetBodyP());

			for (int i = 0; i < GetBodyP()->getNumConstraints(); ++i)
			{
				hkRigidBody* pobOtherBody = 0;
				hkConstraintInstance* pobConstraint = GetBodyP()->getConstraint(i);
				if (pobConstraint->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_CONTACT)
				{
					hkSimpleContactConstraintData* pobData = (hkSimpleContactConstraintData*)pobConstraint->getData();
					pobOtherBody = hkGetRigidBody(pobConstraint->getOtherEntity(GetBodyP())->getCollidable());
					if ( pobOtherBody && isFixedOrKeyframed(pobOtherBody) )
					{
						hkVector4 obAverageNormal(0.0f,0.0f,0.0f);
						for (int j = 0; j < pobData->getNumContactPoints(); j++)
						{
							obAverageNormal.add4( pobData->getContactPoint(pobData->getContactPointIdAt(j)).getNormal() );
						}
						obAverageNormal.mul4(1.0f / pobData->getNumContactPoints());

						// Hack for collision normals not pointing up the right way - DGF
						if (fabs(obAverageNormal(1)) > 0.75f)
						{
							bSetGrounded = true;
							m_bGrounded = true;
							break;
						}
					}
				}
			}

			m_bDoneGroundCheckThisFrame = true;
		}

		// If we didn't find ground, always assume we're not grounded
		if (!bSetGrounded)
			m_bGrounded = false;
		
		return m_bGrounded;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetCollidable
	*
	*	DESCRIPTION		Set if this is to collide with stuff
	*
	***************************************************************************************************/
	void RigidCharacterController::SetCollidable (bool isCollidable)
	{
		if ( isCollidable )
		{
			Physics::ReadAccess read_mutex( GetBodyP() );

			if ( GetBodyP()->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
			{
				Physics::WriteAccess write_mutex;
				GetBodyP()->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
			
				if( GetBodyP()->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( GetBodyP(), HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}
		}
		else
		{
			Physics::WriteAccess mutex;

			if(GetBodyP()->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
			{
				GetBodyP()->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
			}

			Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
			exceptionFlag.flags.exception_set |= Physics::ALWAYS_RETURN_FALSE_BIT;
			hkPropertyValue val2( (int)exceptionFlag.base );
			GetBodyP()->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

			if ( GetBodyP()->getWorld() )
			{
				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( GetBodyP(), HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			}
		}

	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetDynamicCollidable
	*
	*	DESCRIPTION		Set if this is to collide with dynamic stuff
	*
	***************************************************************************************************/
	void RigidCharacterController::SetDynamicCollidable (bool isCollidable)
	{
		if( isCollidable )
		{
			Physics::ReadAccess read_mutex( GetBodyP() );

			if( GetBodyP()->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
			{
				Physics::WriteAccess write_mutex;

				GetBodyP()->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
			
				if( GetBodyP()->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( GetBodyP(), HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}
		}
		else
		{
			Physics::WriteAccess mutex;

			if( GetBodyP()->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
			{
				GetBodyP()->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
			}

			Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
			exceptionFlag.flags.exception_set |= Physics::ONLY_FIXED_GEOM;
			hkPropertyValue val2((int)exceptionFlag.base);
			GetBodyP()->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

			if ( GetBodyP()->getWorld() )
			{
				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( GetBodyP(), HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetRagdollCollidable
	*
	*	DESCRIPTION		Set if this is to collide with ragdolls.
	*
	***************************************************************************************************/
	void RigidCharacterController::SetRagdollCollidable(bool isCollidable)
	{
		if( isCollidable )
		{
			Physics::ReadAccess read_mutex( GetBodyP() );

			if( GetBodyP()->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
			{
				Physics::WriteAccess write_mutex;

				GetBodyP()->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
			
				if( GetBodyP()->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( GetBodyP(), HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}
		}
		else
		{
			Physics::WriteAccess mutex;

			if( GetBodyP()->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
			{
				GetBodyP()->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
			}

			Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
			exceptionFlag.flags.exception_set |= Physics::RAGDOLL_BIT;
			hkPropertyValue val2((int)exceptionFlag.base);
			GetBodyP()->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

			if ( GetBodyP()->getWorld() )
			{
				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( GetBodyP(), HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetFilterExceptionFlags
	*
	*	DESCRIPTION		Set collision filter flags
	*
	***************************************************************************************************/
	void RigidCharacterController::SetFilterExceptionFlags( unsigned int uiFlags )
	{
		Physics::ReadAccess read_mutex;

		if( GetBodyP()->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT) ) // Remove any existing filter exception properties
		{			
			GetBodyP()->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
		}

		if (uiFlags!=0) // If flags have been set, then we add a filter exception property
		{
			Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
			exceptionFlag.flags.exception_set |= uiFlags;
			hkPropertyValue val2((int)exceptionFlag.base);
			GetBodyP()->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);			
		}
		
		if( GetBodyP()->getWorld() )
		{
			Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( GetBodyP(), HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetHoldingCapsule
	*
	*	DESCRIPTION		Change to bigger capsule for holding stuff
	*
	***************************************************************************************************/
	void RigidCharacterController::SetHoldingCapsule(bool bHoldingSomething)
	{
		if (m_bUseHolding == bHoldingSomething)
			return; // Nothing to do, we're already set, and if we did anything in this case it'd cause badness

		if (bHoldingSomething)
		{
			// Set holding body to position of body
			m_pobHoldingBody->setPosition(m_pobBody->getPosition());
		}
		else
		{
			m_pobBody->setPosition(m_pobHoldingBody->getPosition());
		}

		RemoveFromWorld(m_world);
		m_bUseHolding = bHoldingSomething;
		AddToWorld(m_world);
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::AddToWorld
	*
	*	DESCRIPTION		Add this character phantom, feet phantom and associated listeners to the world
	*
	***************************************************************************************************/
	void RigidCharacterController::AddToWorld (hkWorld* pWorld)
	{
		Physics::WriteAccess mutex;

		m_world = pWorld;

		if (m_bUseHolding)
		{
			if (m_pobHoldingBody->getWorld() != m_world)
			{
				m_world->addEntity(m_pobHoldingBody);
				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( m_pobHoldingBody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			
				m_obDisplacementLastFrame = CDirection(0.0f,0.0f,0.0f);

				AddCombatPhysicsPushVolumesToWorld();
			}
		}
		else
		{
			if (m_pobBody->getWorld() != m_world)
			{
				m_world->addEntity(m_pobBody);
				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( m_pobBody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				
				m_obDisplacementLastFrame = CDirection(0.0f,0.0f,0.0f);

				AddCombatPhysicsPushVolumesToWorld();
			}
		}	
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::Update
	*
	*	DESCRIPTION		Update the rigid body in the world. If syncing with anything, will just set positions.
	*
	***************************************************************************************************/
	void RigidCharacterController::Update( float fTimeDelta, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform )
	{
		if(0.f == fTimeDelta || !m_isActive)
			return;
	
		GATSO_PHYSICS_START("RigidCharacterController::Update");

		if (!m_bGravity)
		{
			if (!GetAntiGravityAction()->getWorld() )
			{
				CPhysicsWorld::Get().AddAction( GetAntiGravityAction() );
			}
		}
		else
		{
			if (GetAntiGravityAction()->getWorld() )
			{
				CPhysicsWorld::Get().RemoveAction( GetAntiGravityAction() );
			}
		}

		// For hard sync we just set phantom position explicitly
		// Also when in External_Control_State we use a bind function to ensure movements are done absolutely
		if ( eSynchronised == WS_SYNC_NONE && !m_bApplyMovementAbsolutely )
		{
			CPoint		obCurrentEntityPosition;
			hkVector4	obCurrentBodyPosition;
			if (m_bPositionOnNextActivationSet)
			{
				obCurrentEntityPosition = m_obPositionOnNextActivation;
				obCurrentBodyPosition = ComputeCharacterPositionFromEntity(&m_obPositionOnNextActivation);
				
				// Set these positions
				SetPosition(obCurrentBodyPosition);
				CMatrix obNewMatrix( obGoalOrientation );
				obNewMatrix.SetTranslation( obCurrentEntityPosition );
				m_pobRoot->SetLocalMatrix( obNewMatrix );
			}

			// Get positions as they stand now
			obCurrentEntityPosition = ComputeEntityPositionFromCharacter();
			obCurrentBodyPosition = GetPosition();				
			
			// How much do we want to move?			
			hkVector4 obDesiredDisplacement(	obGoalTranslation.X() - obCurrentEntityPosition.X(),
												obGoalTranslation.Y() - obCurrentEntityPosition.Y(),
												obGoalTranslation.Z() - obCurrentEntityPosition.Z() );
			
			// If we've just been activated in a new position, ignore what the movement wants us to do cos it's probably useless
			if (m_bPositionOnNextActivationSet)
			{
				obDesiredDisplacement = hkVector4::getZero();
			}

			hkVector4 obDesiredVelocity( obDesiredDisplacement );
			obDesiredVelocity.mul4( ( 1.0f / fTimeDelta ) );

			// Clear this flag so next frame we do one from scratch
			m_bDoneGroundCheckThisFrame = false;

			//ntPrintf("VelNeeded: %f %f %f\n",obDesiredVelocity(0),obDesiredVelocity(1),obDesiredVelocity(2));
			//ntPrintf("GoalTrans: %f %f %f\n",obGoalTranslation.X(),obGoalTranslation.Y(),obGoalTranslation.Z());
			//ntPrintf("Movement: %f %f %f\n",obDesiredDisplacement(0),obDesiredDisplacement(1),obDesiredDisplacement(2));

			hkVector4 obCurrentVelocity = GetBodyP()->getLinearVelocity();
	
			// Only do these if we're not doing anything special in combat
			if (!m_entity->GetAttackComponent() || m_entity->GetAttackComponent()->AI_Access_IsInCSStandard())
			{
				// Check to see if our velocity is ok in terms of slope - quite hard to do without contact normal information
				CDirection obSlopedVelocity = Physics::MathsTools::hkVectorToCDirection(obCurrentVelocity);
				CDirection obFlat(obSlopedVelocity.X(),0.0f,obSlopedVelocity.Z());
				obSlopedVelocity.Normalise();
				obFlat.Normalise();
				float fACos = acos(obFlat.Dot(obSlopedVelocity));
				if ( obSlopedVelocity.Y() > 0.0f && fACos > GetMaxSlope() )
				{
					// Going up too much of a slope, go back!
					obDesiredVelocity.mul4(-2.0f);
					obDesiredDisplacement.mul4(-2.0f);
				}
				else if (m_bGravity)
				{
					// Apply velocity with a bit of consideration for current velocity so we don't lose gravity			
					if (obDesiredVelocity(1) == 0.0f && obCurrentVelocity(1) < 0.0f)
						obDesiredVelocity(1) = obCurrentVelocity(1);
					else
						obDesiredVelocity(1) = 0.0f;
				}
			}

			// If I'm only in contact with one rigid, then take action to prevent sliding
			/*if (m_bGravity && GetBodyP()->getNumConstraints() > 0)
			{
				bool bSlide = false;
				hkRigidBody* pobOtherBody = 0;

				for (int i = 0; i < GetBodyP()->getNumConstraints(); ++i)
				{
					hkConstraintInstance* pobConstraint = GetBodyP()->getConstraint(i);
					if (pobConstraint->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_CONTACT)
					{
						// Get the first other rigid we can
						if (!pobOtherBody)
						{
							pobOtherBody = hkGetRigidBody(pobConstraint->getOtherEntity(GetBodyP())->getCollidable());
						}
						else
						{
							// Then compare it to all the others
							if (pobOtherBody != pobConstraint->getOtherEntity(GetBodyP()))
							{
								// We're in contact with more than one thing and should slide
								bSlide = true;
								break;
							}
						}
					}
				}

				if (!bSlide)
				{
					// Deactivate the body, stops most slides, it'll get reactivated by havok if it's involved in a collision or something
					//m_pobBody->deactivate();
				}
			}*/
			
			SetLinearVelocity(obDesiredVelocity);

			// Set ent matrix
			if (bUpdateEntityTransform)
			{				
				CMatrix obNewEntityMatrix = CMatrix( obGoalOrientation, obCurrentEntityPosition );
				m_pobRoot->SetLocalMatrix( obNewEntityMatrix );
			}

			// And update our push/collapse volumes
			UpdateCombatPhysicsPushVolumes( fTimeDelta );

			// Clear flags
			m_bPositionOnNextActivationSet = false;

			m_obDisplacementLastFrame = Physics::MathsTools::hkVectorToCDirection(obDesiredDisplacement);
		}
		else // Absolute movement, set translation/orientation explicitly
		{
			CMatrix obNewMatrix(obGoalOrientation);
			obNewMatrix.SetTranslation(obGoalTranslation);
			m_pobRoot->SetLocalMatrix( obNewMatrix );
			SetPosition(ComputeCharacterPositionFromEntity());
		}

		GATSO_PHYSICS_STOP("RigidCharacterController::Update");
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetPosition
	*
	*	DESCRIPTION		Set body position.
	*
	***************************************************************************************************/
	void RigidCharacterController::SetPosition (const hkVector4& position)
	{
		Physics::WriteAccess mutex;

		GetBodyP()->setPosition(position);

		if (m_pobCombatPhysicsPushVolumeDescriptors)
		{
			PushVolumeDescriptorList::iterator obIt = m_pobCombatPhysicsPushVolumeDescriptors->begin();
			PushVolumeDataList::iterator obItData = m_pobCombatPhysicsPushVolumeData->begin();
			while (obIt != m_pobCombatPhysicsPushVolumeDescriptors->end())
			{
				int iIndex = m_entity->GetHierarchy()->GetTransformIndex(((*obIt)->m_obBindToTransform));
				if (iIndex > -1)
				{
					CMatrix obMtx = m_entity->GetHierarchy()->GetTransform(iIndex)->GetWorldMatrix();
					hkTransform obTransform(Physics::MathsTools::CQuatTohkQuaternion(CQuat(obMtx)),Physics::MathsTools::CPointTohkVector(obMtx.GetTranslation()));
					hkVector4 obXAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetXAxis());
					obXAxis.mul4((*obIt)->m_fXAxisPositionOffset);
					hkVector4 obYAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetYAxis());
					obYAxis.mul4((*obIt)->m_fYAxisPositionOffset);
					hkVector4 obZAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetZAxis());
					obZAxis.mul4((*obIt)->m_fZAxisPositionOffset);
					obTransform.getTranslation().add4(obXAxis);
					obTransform.getTranslation().add4(obYAxis);
					obTransform.getTranslation().add4(obZAxis);
					(*obItData)->m_pobPhantom->setTransform(obTransform);
				}

				obIt++;
				obItData++;
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetLinearVelocity
	*
	*	DESCRIPTION		Set character controller desired velocity.
	*
	***************************************************************************************************/
	void RigidCharacterController::SetLinearVelocity (const hkVector4& vel)
	{
		Physics::WriteAccess mutex(GetBodyP());

		if (vel.lengthSquared3() < 0.0001) // numerical zero
			GetBodyP()->setLinearDamping(1); // prevent sliding 
		else
			GetBodyP()->setLinearDamping(0);

		GetBodyP()->setLinearVelocity(vel);
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::GetPosition
	*
	*	DESCRIPTION		Get phantom position.
	*
	***************************************************************************************************/
	hkVector4 RigidCharacterController::GetPosition() const
	{
		Physics::ReadAccess mutex(GetBodyP());

		hkTransform obTransform;
		GetBodyP()->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obTransform);
		return obTransform.getTranslation();
	}

	/***************************************************************************************************
	*
	*	FUNCTION			RigidCharacterController::Debug_RenderCollisionInfo
	*
	*	DESCRIPTION		Render the rigid character controller.
	*
	***************************************************************************************************/
	void RigidCharacterController::Debug_RenderCollisionInfo()
	{
#ifndef _GOLD_MASTER

		DebugCollisionTools::RenderCollisionFlags(GetBodyP());

		{
		hkTransform obTransform;
		GetBodyP()->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obTransform);		

		hkQuaternion obRotation(obTransform.getRotation());
		
		CMatrix obWorldMatrix(
			CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
			CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));

		DebugCollisionTools::RenderShape(obWorldMatrix,GetBodyP()->getCollidable()->getShape());
		}

		if (m_pobCombatPhysicsPushVolumeDescriptors)
		{
			PushVolumeDataList::iterator obItData = m_pobCombatPhysicsPushVolumeData->begin();
			PushVolumeDescriptorList::iterator obIt = m_pobCombatPhysicsPushVolumeDescriptors->begin();
			while (obItData != m_pobCombatPhysicsPushVolumeData->end())
			{
				if ((*obItData)->m_pobPhantom)
				{
					int iIndex = m_entity->GetHierarchy()->GetTransformIndex(((*obIt)->m_obBindToTransform));
					if (iIndex > -1)
					{
						CMatrix obMtx = m_entity->GetHierarchy()->GetTransform(iIndex)->GetWorldMatrix();
						g_VisualDebug->RenderAxis(obMtx,0.5f);
					}
					const hkTransform& obTransform = (*obItData)->m_pobPhantom->getTransform();
					hkQuaternion obRotation(obTransform.getRotation());
					CMatrix obWorldMatrix(CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
											CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));
					DebugCollisionTools::RenderShape(obWorldMatrix,(*obItData)->m_pobPhantom->getCollidable()->getShape());
				}

				obItData++;
				obIt++;
			}
		}
#endif
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RigidCharacterController::SetKOState
	*
	*	DESCRIPTION		Set KO status. 
	*
	***************************************************************************************************/
	void  RigidCharacterController::SetKOState(KO_States state)
	{
		Physics::WriteAccess mutex;

		ChatacterControllerCollisionFlag obFlag;
		obFlag.base = m_pobBody->getCollidable()->getCollisionFilterInfo();
		obFlag.flags.i_am_in_KO_state = state;
		m_pobBody->setCollisionFilterInfo(obFlag.base);		
		if (m_pobBody->getWorld())
			m_pobBody->getWorld()->updateCollisionFilterOnEntity( m_pobBody, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK ,HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);   

		obFlag.base = m_pobHoldingBody->getCollidable()->getCollisionFilterInfo();
		obFlag.flags.i_am_in_KO_state = state;

		m_pobHoldingBody->setCollisionFilterInfo(obFlag.base);	
		if (m_pobHoldingBody->getWorld())
			m_pobHoldingBody->getWorld()->updateCollisionFilterOnEntity( m_pobHoldingBody, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);   
	}
	/***************************************************************************************************
	*
	*	CLASS			CharacterCanWalkOver
	*
	*	DESCRIPTION		Checks if we can walk over.
	*
	***************************************************************************************************/
	#define fMIN_FEET_SIZE 0.6f
	static bool CharacterCanWalkOver(hkRigidBody* obRB, const hkVector4& obEntityPos)
	{
		if((obRB) && (!isFixedOrKeyframed(obRB)))
		{		
			Physics::ReadAccess mutex(obRB);

			Physics::EntityCollisionFlag infoA;
			infoA.base = obRB->getCollidable()->getCollisionFilterInfo();

			if(	( infoA.base != 0 )		&&
				( infoA.base != 65536 ) )
			{
				hkTransform t; t.setIdentity();
				hkAabb aabb;
				obRB->getCollidable()->getShape()->getAabb(t,0.01f,aabb);
				hkVector4 obHalfExtents;
				obHalfExtents.setSub4(aabb.m_max, aabb.m_min);
				float min = fabs(obHalfExtents(0));
				if(fabs(obHalfExtents(1)) < min)
					min = fabs(obHalfExtents(1));
				if(fabs(obHalfExtents(2)) < min)
					min = fabs(obHalfExtents(2));

				float diffy = obRB->getPosition()(1) - obEntityPos(1);

				if((min < fMIN_FEET_SIZE) && (diffy < fMIN_FEET_SIZE))
				{
					return true;
				}
			}
		}

		return false;
	}

	/***************************************************************************************************
	*
	*	CLASS			MyFullCharacterControllerPhantomOverlapListener
	*
	*	DESCRIPTION		Rejects overlaps from collision we can walk over.
	*
	***************************************************************************************************/
	class MyFullCharacterControllerPhantomOverlapListener : public hkPhantomOverlapListener 
	{
	public:

		MyFullCharacterControllerPhantomOverlapListener ( CEntity* pobMyEntity )
		{
			m_pobEntity = pobMyEntity;
		};

		virtual void collidableAddedCallback(   const hkCollidableAddedEvent& event )
		{
			if(CharacterCanWalkOver(hkGetRigidBody(event.m_collidable), Physics::MathsTools::CPointTohkVector(m_pobEntity->GetPosition())))
			{
				Physics::WriteAccess mutex(hkGetRigidBody(event.m_collidable));
				hkGetRigidBody(event.m_collidable)->activate();
				event.m_collidableAccept = HK_COLLIDABLE_REJECT;
				return;
			}

			event.m_collidableAccept = HK_COLLIDABLE_ACCEPT;
		};

		virtual void collidableRemovedCallback( const hkCollidableRemovedEvent& event ) {};

	private:
		CEntity* m_pobEntity;
	};

	/***************************************************************************************************
	*
	*	FUNCTION		CharacterContactListener::GetTriangleNormal
	*
	*	DESCRIPTION		Find normal of triangle in shape. 
	*
	***************************************************************************************************/

	hkVector4 CharacterContactListener::GetTriangleNormal(const hkShape * shape, const hkShapeKey& key) const
	{
		switch(shape->getType())
		{
		case HK_SHAPE_TRIANGLE:
			return Tools::CalcTriangleNormal(static_cast<const hkTriangleShape*>(shape)->getVertices());
		case HK_SHAPE_TRIANGLE_COLLECTION:
			{
				const hkMeshShape *meshShape = static_cast<const hkMeshShape *>(shape);
				hkShapeCollection::ShapeBuffer buffer;
				const hkTriangleShape* triangle = static_cast<const hkTriangleShape*>(meshShape->getChildShape(key, buffer));
				return Tools::CalcTriangleNormal(triangle->getVertices());
			}
		case HK_SHAPE_MOPP:
		case HK_SHAPE_BV_TREE:
			{
				return GetTriangleNormal(static_cast<const hkBvTreeShape *>(shape)->getShapeCollection(), key);				
			}
		case HK_SHAPE_TRANSFORM:
			{
				const hkTransformShape * transformShape = static_cast<const hkTransformShape *>(shape);
				hkVector4 result;
				result.setRotatedDir(transformShape->getRotation(), GetTriangleNormal(transformShape->getChildShape(), key));			
				return result;
			}
		default:
			return hkVector4(0,0,0,0);
		}
	}
	
	/***************************************************************************************************
	*
	*	FUNCTION		CharacterContactListener::contactsFoundCallback
	*
	*	DESCRIPTION		Find if contact is with invisible wall and if yes if it should be ignored or character killed.
	*
	***************************************************************************************************/
	bool CharacterContactListener::IgnoreContact(hkRootCdPoint& pt, bool & kill) const
	{
		Physics::ChatacterControllerCollisionFlag characterFlag; characterFlag.base = pt.m_rootCollidableA->getCollisionFilterInfo();
		ntAssert(characterFlag.flags.i_am & (Physics::CHARACTER_CONTROLLER_PLAYER_BIT | Physics::CHARACTER_CONTROLLER_ENEMY_BIT));

		Physics::AIWallCollisionFlag wallFlag; wallFlag.base = pt.m_rootCollidableB->getCollisionFilterInfo();
		if ((wallFlag.flags.i_am & Physics::AI_WALL_BIT) == 0)
			return false;		

		hkPhantom * phantom = hkGetPhantom(pt.m_rootCollidableA);
		ntAssert(phantom);
		CEntity  * pobEntity = 0;
		if ( phantom && phantom->hasProperty(PROPERTY_ENTITY_PTR) )
			pobEntity = (CEntity*) phantom->getProperty(PROPERTY_ENTITY_PTR).getPtr();

		kill = false;

		if (wallFlag.flags.one_sided && GetTriangleNormal(pt.m_rootCollidableB->getShape(), pt.m_shapeKeyB).dot3(pt.m_contact.getNormal()) > 0)
		{		
			// ignore contact it is from wrong side of the one sided wall
			return true;
		}		

		if (wallFlag.flags.kill_passing_KO && pobEntity && pobEntity->GetAttackComponent() && pobEntity->GetAttackComponent()->AI_Access_GetState() == CS_KO )
		{
			// Special cases for ledge recoveries
			if ( pobEntity->IsPlayer() && pobEntity->GetAttackComponent()->CanLedgeRecover() )
			{
				if (pobEntity->GetAttackComponent()->GetLastKOPosition().Y() + pobEntity->GetAttackComponent()->GetLedgeRecoverRequiredHeight() < pobEntity->GetPosition().Y() )
				{
					(const_cast< CAttackComponent* >(pobEntity->GetAttackComponent()))->NotifyNeedsLedgeRecover();
				
					// Ignore this contact
					return true;
				}
				else
				{
					// Hit the wall
					return false;
				}
			}
			else if ( pobEntity->GetAttackComponent()->IsDoingLedgeRecover() )
			{
				// Ignore this contact
				return true;
			}	
			else if (characterFlag.flags.i_am_important)
			{
				if (characterFlag.flags.i_am_in_KO_state & wallFlag.flags.not_collide_with_KO_states_important)
					return (kill = true);
			}
			else
			{
				if (characterFlag.flags.i_am_in_KO_state & wallFlag.flags.not_collide_with_KO_states_unimportant)
					return (kill = true);
			}
		}
		return false;
	}

	void CharacterContactListener::contactsFoundCallback(hkAllCdPointCollector& castCollector, hkAllCdPointCollector& startPointCollector)
	{
		hkArray< hkRootCdPoint > & castHits = castCollector.getHits();
		for(int i = 0; i < castHits.getSize(); i++)
		{
			hkPhantom * phantom = hkGetPhantom(castHits[i].m_rootCollidableA);
			ntAssert(phantom);
			CEntity  * pobEntity = 0;
			if ( phantom && phantom->hasProperty(PROPERTY_ENTITY_PTR) )
				pobEntity = (CEntity*) phantom->getProperty(PROPERTY_ENTITY_PTR).getPtr();

			bool kill = false;
			bool ignore = IgnoreContact(castHits[i], kill);

			if (kill)
			{
				// Tell character that they need to die very soon 	
				if (pobEntity && pobEntity->IsCharacter() && pobEntity->ToCharacter()->GetAttackComponent())
					pobEntity->ToCharacter()->GetAttackComponent()->NotifyDieOutOfCurrentMovement();
			}

			if (ignore)
			{
				//remove point
				castHits.removeAt(i);
				i--;
			}
		}

		hkArray< hkRootCdPoint > & startHits = startPointCollector.getHits();
		for(int i = 0; i < startHits.getSize(); i++)
		{
			bool kill = false;
			bool ignore = IgnoreContact(startHits[i],kill);
			if (kill)
			{
				// kill character 				
				hkPhantom * phantom = hkGetPhantom(startHits[i].m_rootCollidableA);
				ntAssert(phantom);
				CEntity  * pobEntity = 0;
				if ( phantom && phantom->hasProperty(PROPERTY_ENTITY_PTR) )
					pobEntity = (CEntity*) phantom->getProperty(PROPERTY_ENTITY_PTR).getPtr();

				// Tell character that they need to die very soon 	
				if (pobEntity && pobEntity->IsCharacter() && pobEntity->ToCharacter()->GetAttackComponent())
					pobEntity->ToCharacter()->GetAttackComponent()->NotifyDieOutOfCurrentMovement();
			}

			if (ignore)
			{
				//remove point
				startHits.removeAt(i);
				i--;
			}
		}
	}


	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::FullCharacterController
	*
	*	DESCRIPTION		Construct.
	*
	***************************************************************************************************/
	FullCharacterController::FullCharacterController( Character* p_entity, CColprimDesc* pobColPrim )
		: CharacterController(p_entity,pobColPrim)
	{
		m_characterProxy = NULL;
		m_phantom = 0;
		m_wallContactListener = 0;

		SetupCharacterVolume();
	};

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::~FullCharacterController
	*
	*	DESCRIPTION		Destroy.
	*
	***************************************************************************************************/
	FullCharacterController::~FullCharacterController( )
	{
		if(m_phantom->isAddedToWorld())
			RemoveFromWorld( m_world );

		if (m_pol)
		{
			m_phantom->removePhantomOverlapListener( m_pol );
			HK_DELETE( m_pol );
		}

		if (m_listener)
		{
			m_characterProxy->removeCharacterProxyListener( m_listener );
			HK_DELETE( m_listener );
		}

		if (m_wallContactListener)
		{
			m_characterProxy->removeCharacterProxyContactListener( m_wallContactListener );
			HK_DELETE( m_wallContactListener );
		}

		if (m_phantom)
		{
			m_phantom->removeReference();
		}

		if (m_characterProxy)
		{
			m_characterProxy->removeReference();
		}
	};
	
	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::Activate
	*
	*	DESCRIPTION		Active after calling this.
	*
	***************************************************************************************************/
	void FullCharacterController::Activate( )
	{
		Physics::WriteAccess mutex;
		
		if( false == m_isActive )
		{
			AddToWorld(CPhysicsWorld::Get().GetHavokWorldP());
			SetPosition( ComputeCharacterPositionFromEntity() );
			// See if we're in a wall, if so, move us towards the player till we're not
			hkAllCdPointCollector obGroundPenetrationCollector;
			m_characterProxy->getShapePhantom()->getClosestPoints(obGroundPenetrationCollector);
			hkVector4 obDisplacementCorrection(0,0,0,0);
			hkVector4 obCurrentPhantomPosition(m_characterProxy->getPosition());
			// If we hit, and it's a penetration
 			if (obGroundPenetrationCollector.getHits().getSize() > 0)
			{		
				for (int i = 0; i < obGroundPenetrationCollector.getHits().getSize(); i++)
				{
					if ( obGroundPenetrationCollector.getHits()[i].m_contact.getDistance() < 0 )
					{
						// Try a rigid body...
						hkRigidBody* pobRB = hkGetRigidBody( obGroundPenetrationCollector.getHits()[i].m_rootCollidableB );

						if (
							(obGroundPenetrationCollector.getHits()[i].m_rootCollidableB->getShape()->getType() == HK_SHAPE_MOPP || 
							obGroundPenetrationCollector.getHits()[i].m_rootCollidableB->getShape()->getType() == HK_SHAPE_BV_TREE ||
							obGroundPenetrationCollector.getHits()[i].m_rootCollidableB->getShape()->getType() == HK_SHAPE_BV) // We have static level geom shapes
							||
							(pobRB && !(pobRB->getMass() > 0.0f)) // We have a fixed rigid body of some form
						)
						{
							hkVector4 obNormal = obGroundPenetrationCollector.getHits()[i].m_contact.getNormal();
							// If this isn't a standard ground normal
							if (obNormal(1) < 0.95f)
							{
								// Check to see if the normal is facing in the same direction as the player
								CDirection obHSNormal(Physics::MathsTools::hkVectorToCDirection(obNormal));
								CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
								if (pobPlayer)
								{
									CPoint obPlayerPosition = pobPlayer->GetPosition();
									CDirection obHereToPlayer = CDirection( Physics::MathsTools::hkVectorToCPoint(obCurrentPhantomPosition) - obPlayerPosition );
									obHereToPlayer.Normalise();
									// If it's not, flip it so it does - we don't want to correct phantom penetration into a wall
									if (obHereToPlayer.Dot(obHSNormal) > PI*0.5f)
									{
										ntPrintf("If you see this message, and there's a bloke in a wall, tell Duncan!\n");
										obNormal.mul4(-2.5f);
									}
								}
							}
							// I have penetrated, so I should add correction displacement (distance will be -ve if we're penetrating)
							obNormal.mul4(obGroundPenetrationCollector.getHits()[i].m_contact.getDistance()*-2.5f);
							obDisplacementCorrection.add4(obNormal);
						}
					}
				}
			}

			// Add our correction to the position of our phantom
			obCurrentPhantomPosition.add4(obDisplacementCorrection);
			m_characterProxy->setPosition(obCurrentPhantomPosition,true);
			// Update entity position
			CMatrix obMtx = m_pobRoot->GetLocalMatrix();	
			obMtx.SetTranslation( obMtx.GetTranslation() + Physics::MathsTools::hkVectorToCPoint(obDisplacementCorrection) );
			m_pobRoot->SetLocalMatrix(obMtx);
		}

		m_isActive = true;
	};

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::UpdateCollisionFilter
	*
	*	DESCRIPTION		3 guesses what this function does. If you still get it wrong after 3 tries, 
	*					I'm going to do a service to society and lock you in a box.
	*
	***************************************************************************************************/
	void FullCharacterController::UpdateCollisionFilter()
	{
		if (m_phantom->getWorld())
			m_phantom->getWorld()->updateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS ); 
		if (m_pobRigidBody->getWorld())
			m_pobRigidBody->getWorld()->updateCollisionFilterOnEntity( m_pobRigidBody, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK ,HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);   
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::Deactivate
	*
	*	DESCRIPTION		Won't be active after calling this.
	*
	***************************************************************************************************/
	void FullCharacterController::Deactivate( )
	{
		if( m_isActive )
			RemoveFromWorld(CPhysicsWorld::Get().GetHavokWorldP());

		m_isActive = false;
	};

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::RegisterSystem
	*
	*	DESCRIPTION		Registers this with collision system.
	*
	***************************************************************************************************/
	void FullCharacterController::RegisterSystem( System* p_system )
	{
		p_system->GetCollisionListener()->RegisterCharacterController( const_cast<FullCharacterController*>(this) );
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetupCharacterVolume
	*
	*	DESCRIPTION		Creates the shape that the character phantom will take.
	*
	***************************************************************************************************/
	#define fPERCENTAGE_HOLDING_SIZE 1.0f
	#define fSIZE_SPHERE_STEP 0.16f
	void FullCharacterController::SetupCharacterVolume()
	{
		m_characterProxy = NULL;
		
		const float fLength = m_pobColPrim->m_obCapsuleData.fLength;
		const float fRadius = m_pobColPrim->m_obCapsuleData.fRadius;

		m_fCapsuleHalfLength = (fLength * 0.5f) + fRadius;

		// Construct a shape
		hkVector4 vertexA(0, (fLength*0.5f), 0);
		hkVector4 vertexB(0, (fLength*-0.5f), 0);		

		// Create a capsule to represent the character standing
		m_standShape = HK_NEW hkCapsuleShape(vertexA, vertexB, fRadius);
		hkVector4 size; size.setSub4( vertexA, vertexB );

		float fIncrease = fPERCENTAGE_HOLDING_SIZE * fRadius;
		float fHoldRadius = fRadius + fIncrease;

		hkVector4 axis; axis.setSub4( vertexA, vertexB ); // vertexA - vertexB
		axis.normalize3(); axis.mul4(fIncrease);

		//Error if havok is going to bail anyway, but give a reason that people can understand.
		user_error_p((fLength*0.5f) != fIncrease, ("(Length*2) must NOT equal fIncrease (Radius * fPERCENTAGE_HOLDING_SIZE) or you get duplicate points at the centre of the capsule (sphere), which havok complains about"));

		//Assert for good-measure if fLength*0.5 isn't bigger than fIncrease. In this case, the two points that make up the capsule will have
		//passed each other and the capsule will be upside down. While you will still get a capsule, it won't be correct, because you would have
		//moved the point far-enough from it's own vertical-limit that it actually penetrates the opposite vertical-limit, so the height of the
		//capsule won't be what you set it to.

		//E.G. Height = 1.0, Radius = 0.6. Capsule-point at 0.5 is moved to -0.1, Capsule-point at -0.5 is moved to 0.1. Height-boundaries on the
		//capsule now go from -0.6->0.6 ... an increase of 0.2, and the height change will increase further as the radius increases.
		//Of course, this isn't a crash-problem, just a height-change, so if this assert is a bit strict, then make it a ntPrintf warning
		//instead.
		user_warn_p((fLength*0.5f) > fIncrease, ("Length should really be over twice the size of fIncrease (Radius * fPERCENTAGE_HOLDING_SIZE). Entity : %s", m_entity ? m_entity->GetName().c_str() : "UNKNOWN" ));
		vertexB.add4(axis);
		vertexA.sub4(axis);

		// Allow the entity to modify the holding capsule, this code was added so that
		// the archer could use the holding capsule shape as a covering capsule

		{
			CPoint ptA = Physics::MathsTools::hkVectorToCPoint(vertexA);
			CPoint ptB = Physics::MathsTools::hkVectorToCPoint(vertexB);

			m_entity->ModifyPhysicsHoldingShape( ptA, ptB, fHoldRadius);

			vertexA = Physics::MathsTools::CPointTohkVector(ptA);
			vertexB = Physics::MathsTools::CPointTohkVector(ptB);
		}

		m_holdingShape = HK_NEW hkCapsuleShape(vertexA, vertexB, fHoldRadius);	

		m_obEntityCFlag.base = 0;

		if( m_entity->IsPlayer() || m_entity->IsFriendly() )
			m_obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_PLAYER_BIT;
		else 
			m_obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_ENEMY_BIT;

		if (m_entity->IsImportant())
			m_obEntityCFlag.flags.i_am_important = 1;

		if( m_entity->IsPlayer() || m_entity->IsBoss() || m_entity->IsFriendly() || ( m_entity->IsAI() && ((AI*)m_entity)->AttackAI() )  )
		{
			m_obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	| 
													Physics::RAGDOLL_BIT						|
													Physics::LARGE_INTERACTABLE_BIT				|	
													Physics::SMALL_INTERACTABLE_BIT				|	
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
		} else {
			m_obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	| //Turned off for the test
													Physics::RAGDOLL_BIT						|
													Physics::LARGE_INTERACTABLE_BIT				|
													Physics::SMALL_INTERACTABLE_BIT				|	
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
		}

		hkTransform trf; 
		trf.setIdentity();
		m_phantom = HK_NEW hkSimpleShapePhantom( m_standShape, trf, (int)m_obEntityCFlag.base );
		hkPropertyValue val((void*)m_entity);
		m_phantom->addProperty(Physics::PROPERTY_ENTITY_PTR, val);	

		// Define a character proxy
		hkCharacterProxyCinfo cpci;
		cpci.m_position.set(0,0,0);
		cpci.m_staticFriction = 0.0f;
		cpci.m_dynamicFriction = 1.0f;
		cpci.m_extraDownStaticFriction = 0.0f;
		cpci.m_extraUpStaticFriction = 0.0f;
		cpci.m_velocity.set(0.0f,0.0f,0.0f);
		cpci.m_penetrationRecoverySpeed = 20.0f;
		if(m_entity->IsPlayer())
			// cpci.m_keepDistance = 0.10f; the hero is now floating 0.1 m over ground
			cpci.m_keepDistance = 0.0f;
		else
			//cpci.m_keepDistance = 0.05f;
			cpci.m_keepDistance = 0.0f;

		//cpci.m_keepContactTolerance = 0.02f;

		cpci.m_shapePhantom = m_phantom;
		cpci.m_up = hkVector4(0.0f, 1.0f, 0.0f, 0.0f);
		cpci.m_maxSlope = GetMaxSlope();
		cpci.m_up = hkVector4(0,1,0,0);
		cpci.m_userPlanes = 4;
		cpci.m_characterStrength = 400.0f;
		cpci.m_characterMass = 1.0f;

		// This will ignore collisions from objects below a certain height so the player can walk over debris as defined in CanWalkOver
		m_pol = HK_NEW MyFullCharacterControllerPhantomOverlapListener(m_entity);
		m_phantom->addPhantomOverlapListener(m_pol);
		// Construct a character proxy
		float fUpdateEverySeconds = 0.0f; // Update all the time

		// we know that phantom is not yet in world. Disable warning:
		// Warning : 'Shape phantom has not yet been added to the world. Initial position has been ignored'
		hkError::getInstance().setEnabled( 0x6cee9071, false ); 

		m_characterProxy = HK_NEW hsCharacterProxy( cpci, fUpdateEverySeconds );

		hkError::getInstance().setEnabled( 0x6cee9071, true );
		
		m_listener = HK_NEW CharacterInteractionListener(m_entity,this);
		m_characterProxy->addCharacterProxyListener(m_listener);

		m_wallContactListener = NT_NEW CharacterContactListener();
		m_characterProxy->addCharacterProxyContactListener(m_wallContactListener);

		// Rigid core
		hkRigidBodyCinfo obBodyInfo;

		// it is used only for interaction with character controllers
		Physics::ChatacterControllerCollisionFlag sphereCollFlag;
		sphereCollFlag.base = m_obEntityCFlag.base;
		sphereCollFlag.flags.i_collide_with = Physics::CHARACTER_CONTROLLER_PLAYER_BIT	|  
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	;

		obBodyInfo.m_collisionFilterInfo = m_obEntityCFlag.base;

		if (m_entity->IsPlayer()) // Make the rigid slightly bigger to stop other characters pushing - this hacktastic fix is copyright Duncan
			obBodyInfo.m_shape = HK_NEW hkSphereShape(fRadius * 1.05f);
		else
			obBodyInfo.m_shape = HK_NEW hkSphereShape(fRadius * 0.75f);
		obBodyInfo.m_position = ComputeCharacterPositionFromEntity();
		obBodyInfo.m_mass = 0.0f;
		obBodyInfo.m_restitution = 0.0f;
		obBodyInfo.m_friction = 0.0f;
		obBodyInfo.m_motionType = hkMotion::MOTION_FIXED;

		m_pobRigidBody = HK_NEW hkRigidBody(obBodyInfo);
		Physics::WriteAccess mutex(m_pobRigidBody);
		m_pobRigidBody->setCollisionFilterInfo((int)sphereCollFlag.base);		
		m_pobRigidBody->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		m_pobRigidBody->setName(m_entity->GetName().c_str());

		obBodyInfo.m_shape->removeReference();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::RemoveFromWorld
	*
	*	DESCRIPTION		Remove everything (character phantom, feet phantom, associated listenes).
	*
	***************************************************************************************************/
	void FullCharacterController::RemoveFromWorld (hkWorld* pWorld)
	{
		Physics::WriteAccess mutex;

        if (m_phantom->getWorld())
			m_phantom->getWorld()->removePhantom(m_phantom);

		if (m_pobRigidBody->getWorld())
		{
			m_pobRigidBody->getWorld()->removeEntity(m_pobRigidBody);
		}

		RemoveCombatPhysicsPushVolumesFromWorld();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::GetLinearVelocity
	*
	*	DESCRIPTION		Returns linear velocity of the character controller, or zero if there isn't one.
	*
	***************************************************************************************************/
	hkVector4 FullCharacterController::GetLinearVelocity( )
	{
		if (m_characterProxy && m_phantom->getWorld())
		{
			return m_characterProxy->getLinearVelocity();
		}

		return hkVector4::getZero();
	}

	class hkCountGroundCdPointCollector : public hkAllCdPointCollector
	{
		public:
			int good;
			int wrong;
			hkVector4 Position;
			hkVector4 m_obGroundContactPosition;

				/// Constructor calls reset.
			inline hkCountGroundCdPointCollector(float angleCos, const hkVector4& p/*, float lenght*/)
				:m_angleCos(angleCos)
			{
				good = 0;
				wrong = 0;
				Position = p;
				m_obGroundContactPosition = p;
			};

			inline virtual ~hkCountGroundCdPointCollector()
			{};

		protected:

				// implements the hkCdPointCollector interface.
			virtual void addCdPoint( const hkCdPoint& event ) 
			{
				bool isFixed = false;

				hkRigidBody* rb =hkGetRigidBody( event.m_cdBodyA.getRootCollidable() );
				if( rb && isFixedOrKeyframed(rb) )
					isFixed = true;

				rb = hkGetRigidBody( event.m_cdBodyB.getRootCollidable() );
				if( rb && isFixedOrKeyframed(rb) )
					isFixed = true;

				if( isFixed == false )
					return;


				if( isFixed )
				{
					if(  event.m_contact.getNormal()(1) < 0.2f )
					{
						wrong++;
						return;
					}

					const hkReal surfaceVerticalComponent = event.m_contact.getNormal().dot3( hkVector4(0.0f, 1.0f, 0.0f, 0.0f) );
					if ( surfaceVerticalComponent > 0.01f && surfaceVerticalComponent < m_angleCos  )
					{
						wrong++;
						return;
					}
				}

				// Do not consider that CC are ground
				hkPhantom* obPH1 = hkGetPhantom(event.m_cdBodyA.getRootCollidable());
				hkPhantom* obPH2 = hkGetPhantom(event.m_cdBodyB.getRootCollidable());
				if( obPH1 && obPH2 )
					return;

				if( m_obGroundContactPosition(1) > event.m_contact.getPosition()(1) )
				{
					m_obGroundContactPosition = event.m_contact.getPosition();
				}

				good++;

				hkRootCdPoint& hit = m_hits.expandOne();
				hit.m_contact = event.m_contact;
				hit.m_rootCollidableA = event.m_cdBodyA.getRootCollidable();
				hit.m_shapeKeyA = event.m_cdBodyA.getShapeKey();
				hit.m_rootCollidableB = event.m_cdBodyB.getRootCollidable();
				hit.m_shapeKeyB = event.m_cdBodyB.getShapeKey();
			}

			float m_angleCos;
	};

	class hkAllGroundCdPointCollector : public hkAllCdPointCollector
	{
		public:

				/// Constructor calls reset.
			inline hkAllGroundCdPointCollector(float angleCos, float maxAltitude)
				:m_angleCos(angleCos), 
				m_maxAltitude( maxAltitude )
			{};

			inline virtual ~hkAllGroundCdPointCollector()
			{};

		protected:

				// implements the hkCdPointCollector interface.
			virtual void addCdPoint( const hkCdPoint& event ) 
			{
				//bool isFixed = false;
				hkRigidBody* rb =hkGetRigidBody( event.m_cdBodyA.getRootCollidable() );
				if( rb && ( isFixedOrKeyframed(rb) == false ) )
				{
					// Let's try to compute the max y altitude
					hkAabb aabb;
					hkTransform obRBTransform;
					rb->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obRBTransform);
					rb->getCollidable()->getShape()->getAabb(obRBTransform, 0.01f, aabb);
					if( aabb.m_max(1) > m_maxAltitude )
						return;

					EntityCollisionFlag infoA;
					infoA.base = rb->getCollidable()->getCollisionFilterInfo();
					if(infoA.flags.i_am & RAGDOLL_BIT)
						return;
				}

				rb =hkGetRigidBody( event.m_cdBodyB.getRootCollidable() );
				if( rb && ( isFixedOrKeyframed(rb) == false ) )
				{
					// Let's try to compute the max y altitude
					hkAabb aabb;
					hkTransform obRBTransform;
					rb->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obRBTransform);
					rb->getCollidable()->getShape()->getAabb(obRBTransform, 0.01f, aabb);
					if( aabb.m_max(1) > m_maxAltitude )
						return;

					EntityCollisionFlag infoA;
					infoA.base = rb->getCollidable()->getCollisionFilterInfo();
					if(infoA.flags.i_am & RAGDOLL_BIT)
						return;
				}

				// Do not consider that CC are ground
				hkPhantom* obPH1 = hkGetPhantom(event.m_cdBodyA.getRootCollidable());
				hkPhantom* obPH2 = hkGetPhantom(event.m_cdBodyB.getRootCollidable());
				if( obPH1 && obPH2 )
					return;

				hkRootCdPoint& hit = m_hits.expandOne();

				hit.m_contact = event.m_contact;
				hit.m_rootCollidableA = event.m_cdBodyA.getRootCollidable();
				hit.m_shapeKeyA = event.m_cdBodyA.getShapeKey();

				hit.m_rootCollidableB = event.m_cdBodyB.getRootCollidable();
				hit.m_shapeKeyB = event.m_cdBodyB.getShapeKey();

				hkVector4 p( event.m_contact.getPosition() );
				p.add4( event.m_contact.getNormal() );
				//HK_DISPLAY_LINE( p, event.m_contact.getPosition(), hkColor::GREEN );
			}

			float m_angleCos;
			float m_maxAltitude;
	};	

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::IsCharacterControllerOnGround
	*
	*	DESCRIPTION		Returns true if the character controller is supported by something below them.
	*
	***************************************************************************************************/
	bool FullCharacterController::IsCharacterControllerOnGround( void ) 	
	{
		if (m_characterProxy && m_phantom->getWorld() && !m_bDoneGroundCheckThisFrame)
		{
			// Initialise
			m_obGroundInfo.m_supportedState = hkSurfaceInfo::UNSUPPORTED;

			hkVector4 down(0.0f,-1.0f,0.0f,0.0f);

			hkCountGroundCdPointCollector collec(GetMaxSlopeCosine(), m_characterProxy->getPosition());
			m_characterProxy->checkSupportWithCollector(down, m_obGroundInfo, collec);
			m_bDoneGroundCheckThisFrame = true;
		}

		return (m_obGroundInfo.m_supportedState != hkSurfaceInfo::UNSUPPORTED);
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::GetCurrentCharacterControllerGround
	*
	*	DESCRIPTION		Returns surface info from directly below the character.
	*
	***************************************************************************************************/
	hkSurfaceInfo FullCharacterController::GetCurrentCharacterControllerGround( )  
	{
		if (m_characterProxy && m_phantom->getWorld() && !m_bDoneGroundCheckThisFrame)
		{
			// Initialise
			m_obGroundInfo.m_supportedState = hkSurfaceInfo::UNSUPPORTED;

			hkVector4 down(0.0f,-1.0f,0.0f,0.0f);

			hkCountGroundCdPointCollector collec(GetMaxSlopeCosine(), m_characterProxy->getPosition());
			m_characterProxy->checkSupportWithCollector(down, m_obGroundInfo, collec);
			m_bDoneGroundCheckThisFrame = true;
		}

		return m_obGroundInfo;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetCollidable
	*
	*	DESCRIPTION		Set if this is to collide with stuff
	*
	***************************************************************************************************/
	void FullCharacterController::SetCollidable (bool isCollidable)
	{
		if ( isCollidable )
		{
			if ( m_characterProxy && m_phantom)
			{
				Physics::ReadAccess read_mutex( m_phantom );

				if ( m_phantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					Physics::WriteAccess write_mutex( m_phantom );
					m_phantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				
					if( m_phantom->getWorld() )
					{
						Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
					}
				}
			}

		}
		else
		{
			if ( m_characterProxy && m_phantom )
			{
				Physics::WriteAccess mutex( m_phantom );

				if(m_phantom->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
				{
					m_phantom->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
				}

				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::ALWAYS_RETURN_FALSE_BIT;
				hkPropertyValue val2( (int)exceptionFlag.base );
				m_phantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

				if ( m_phantom->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}

		}

	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetDynamicCollidable
	*
	*	DESCRIPTION		Set if this is to collide with dynamic stuff
	*
	***************************************************************************************************/
	void FullCharacterController::SetDynamicCollidable (bool isCollidable)
	{
		if( isCollidable )
		{
			if( m_characterProxy && m_phantom )
			{
				Physics::ReadAccess read_mutex( m_phantom );

				if( m_phantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					Physics::WriteAccess write_mutex( m_phantom );

					m_phantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				
					if( m_phantom->getWorld() )
					{
						Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
					}
				}
			}

			if (m_pobRigidBody)
			{
				Physics::ReadAccess mutex(m_pobRigidBody);

				if( m_pobRigidBody->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					Physics::WriteAccess write_mutex(m_pobRigidBody);

					m_pobRigidBody->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				
					if( m_pobRigidBody->getWorld() )
					{
						Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( m_pobRigidBody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
					}
				}
			}
		}
		else
		{
			if ( m_characterProxy && m_phantom )
			{
				Physics::WriteAccess mutex( m_phantom );

				if( m_phantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					m_phantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				}

				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::ONLY_FIXED_GEOM;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_phantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

				if ( m_phantom->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}

			if ( m_pobRigidBody->getWorld() )
			{
				Physics::WriteAccess mutex( m_pobRigidBody );

				if( m_pobRigidBody->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					m_pobRigidBody->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				}

				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::ONLY_FIXED_GEOM;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_pobRigidBody->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( m_pobRigidBody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetRagdollCollidable
	*
	*	DESCRIPTION		Set if this is to collide with ragdoll.
	*
	***************************************************************************************************/
	void FullCharacterController::SetRagdollCollidable(bool isCollidable)
	{
		if( isCollidable )
		{
			if( m_characterProxy && m_phantom )
			{
				Physics::ReadAccess read_mutex( m_phantom );

				if( m_phantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					Physics::WriteAccess write_mutex( m_phantom );

					m_phantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				
					if( m_phantom->getWorld() )
					{
						Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
					}
				}
			}

			if (m_pobRigidBody)
			{
				Physics::ReadAccess read_mutex( m_pobRigidBody );

				if( m_pobRigidBody->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					Physics::WriteAccess write_mutex;

					m_pobRigidBody->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				
					if( m_pobRigidBody->getWorld() )
					{
						Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( m_pobRigidBody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
					}
				}
			}
		}
		else
		{
			if ( m_characterProxy && m_phantom )
			{
				Physics::WriteAccess mutex( m_phantom );

				if( m_phantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					m_phantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				}

				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::RAGDOLL_BIT;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_phantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

				if ( m_phantom->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}

			if (m_pobRigidBody)
			{
				Physics::WriteAccess mutex(m_pobRigidBody);

				if( m_pobRigidBody->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					m_pobRigidBody->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				}

				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::RAGDOLL_BIT;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_pobRigidBody->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

				if ( m_pobRigidBody->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( m_pobRigidBody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetFilterExceptionFlags
	*
	*	DESCRIPTION		Set collision filter flags
	*
	***************************************************************************************************/
	void FullCharacterController::SetFilterExceptionFlags( unsigned int uiFlags )
	{
		if ( m_characterProxy && m_phantom )
		{
			Physics::ReadAccess read_mutex( m_phantom );

			if( m_phantom->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT) ) // Remove any existing filter exception properties
			{
				Physics::WriteAccess write_mutex( m_phantom );
				m_phantom->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
			}

			if (uiFlags!=0) // If flags have been set, then we add a filter exception property
			{
				Physics::WriteAccess write_mutex( m_phantom );
				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= uiFlags;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_phantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);			
			}
			
			if( m_phantom->getWorld() )
			{
				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			}
		}

		if (m_pobRigidBody)
		{
			Physics::WriteAccess mutex(m_pobRigidBody);

			if( m_pobRigidBody->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT) ) // Remove any existing filter exception properties
			{			
				m_pobRigidBody->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
			}

			if (uiFlags!=0) // If flags have been set, then we add a filter exception property
			{
				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= uiFlags;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_pobRigidBody->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);			
			}
			
			if( m_pobRigidBody->getWorld() )
			{
				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( m_pobRigidBody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetHoldingCapsule
	*
	*	DESCRIPTION		Change to bigger capsule for holding stuff
	*
	***************************************************************************************************/
	void FullCharacterController::SetHoldingCapsule(bool bShape)
	{		
		if (bShape)
		{
			m_phantom->setShape(m_holdingShape);
		} 
		else 
		{
			m_phantom->setShape(m_standShape);
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetKOState
	*
	*	DESCRIPTION		Set KO status. 
	*
	***************************************************************************************************/
	void  FullCharacterController::SetKOState(KO_States state)
	{
		Physics::WriteAccess mutex;


		if (state)
		{			
			ChatacterControllerCollisionFlag obFlag;
			obFlag.base = m_phantom->getCollidable()->getCollisionFilterInfo();
			obFlag.flags.i_am_in_KO_state = state;
			// Do not collide with other characterts and ragdolls
			// Don't know why this is needed, it's bad so I'm taking it out - DGF
			/*obFlag.flags.i_collide_with &= ~(Physics::CHARACTER_CONTROLLER_PLAYER_BIT | 
											Physics::CHARACTER_CONTROLLER_ENEMY_BIT |
											Physics::RAGDOLL_BIT);*/

			m_phantom->getCollidableRw()->setCollisionFilterInfo(obFlag.base);

			if (m_phantom->getWorld())
				m_phantom->getWorld()->updateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS ); 
		}
		else
		{

			ChatacterControllerCollisionFlag obFlag;
			obFlag.base = m_phantom->getCollidable()->getCollisionFilterInfo();
			obFlag.flags.i_am_in_KO_state = state;

			// Collide with other characterts and ragdolls
			obFlag.flags.i_collide_with = m_obEntityCFlag.flags.i_collide_with;

			m_phantom->getCollidableRw()->setCollisionFilterInfo(obFlag.base);
			if (m_phantom->getWorld())
				m_phantom->getWorld()->updateCollisionFilterOnPhantom( m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS ); 
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::AddToWorld
	*
	*	DESCRIPTION		Add this character phantom, feet phantom and associated listeners to the world
	*
	***************************************************************************************************/
	void FullCharacterController::AddToWorld (hkWorld* pWorld)
	{
		if (m_phantom->getWorld() == pWorld)
			return;

		Physics::WriteAccess mutex;

		m_world = pWorld;

		m_world->addPhantom(m_phantom);
		m_world->addEntity(m_pobRigidBody);
		Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnEntity( m_pobRigidBody, HK_UPDATE_FILTER_ON_ENTITY_DISABLE_ENTITY_ENTITY_COLLISIONS_ONLY, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			
		AddCombatPhysicsPushVolumesToWorld();
	}

	class hkWallCdPointCollector : public hkAllCdPointCollector
	{
		public:
			inline hkWallCdPointCollector(float angleCos)
				:m_angleCos(angleCos)
			{};

			float m_angleCos;

			inline virtual ~hkWallCdPointCollector( )
			{};

		protected:
			virtual void addCdPoint( const hkCdPoint& event ) 
			{
				// Reject all dyanamic bodies
				hkRigidBody* rb = hkGetRigidBody( event.m_cdBodyB.getRootCollidable() );
				if ( rb && rb->getMass() > 0.0f)
					return;

				const hkReal surfaceVerticalComponent = event.m_contact.getNormal().dot3( hkVector4(0.0f, 1.0f, 0.0f, 0.0f) );
				if ( surfaceVerticalComponent < m_angleCos  )
				{
					//ntPrintf("Wall angle %f\n",surfaceVerticalComponent);

					hkRootCdPoint& hit = m_hits.expandOne();

					hit.m_contact = event.m_contact;
					hit.m_rootCollidableA = event.m_cdBodyA.getRootCollidable();
					hit.m_shapeKeyA = event.m_cdBodyA.getShapeKey();

					hit.m_rootCollidableB = event.m_cdBodyB.getRootCollidable();
					hit.m_shapeKeyB = event.m_cdBodyB.getShapeKey();

					hkVector4 a = event.m_contact.getPosition();
					a.add4( event.m_contact.getNormal() );
				}
			}
	};

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::GetCurrentCharacterControllerWall
	*
	*	DESCRIPTION		Gets wall surface info.
	*
	***************************************************************************************************/
	hkSurfaceInfo FullCharacterController::GetCurrentCharacterControllerWall( const hkVector4& obDir, hkVector4& obNorm ) 
	{
		hkSurfaceInfo obWall;
		if (m_characterProxy && m_phantom->getWorld())
		{
			hkVector4 obDirection = obDir;
			obDirection.normalize3();
			
			hkWallCdPointCollector obCollector( GetMaxSlopeCosine() );
			m_characterProxy->checkSupportWithCollector(obDirection, obWall, obCollector);

			if( obCollector.getHits().getSize() > 0 )
				obNorm = obCollector.getHits()[0].m_contact.getNormal();

			for( int i = 1; i < obCollector.getHits().getSize(); i++ )
			{
				obNorm.add4( obCollector.getHits()[i].m_contact.getNormal() );
				obNorm.mul4( 0.5f );
				if( obNorm.lengthSquared3() < EPSILON )
					obNorm = hkVector4( 0.0f, 1.0f, 0.0f );
				obNorm.normalize3();
			}	
		}

		return obWall;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::CheckWallAndCorrectVeloctyAsNeccessary
	*
	*	DESCRIPTION		Checks for a wall, and constrains velocity as neccessary to stop us sliding off
	*
	***************************************************************************************************/
	hkVector4 FullCharacterController::CheckWallAndCorrectVelocityAsNeccessary(hkVector4& obInputVelocity)
	{
		if( obInputVelocity.lengthSquared3() != 0.0f )
		{
			hkVector4 norm;
			hkSurfaceInfo info = GetCurrentCharacterControllerWall( obInputVelocity, norm );
			
			// Quick check to see if this is within our slope tolerance
			hkVector4 obUp(0,1,0,0);
			float fDot = obUp.dot3(info.m_surfaceNormal);
			if (fDot > GetMaxSlopeCosine())
				return obInputVelocity;	// It's a slope we're allowed to walk up, so ignore it	

			// If not, we need a new velocity
			info.m_surfaceNormal(1) = 0.0f;
			if (info.m_surfaceNormal.lengthSquared3() != 0.0f)
			{				
				if( info.m_surfaceNormal.isOk3() )
					info.m_surfaceNormal.normalize3();

				float proj = info.m_surfaceNormal.dot3( obInputVelocity );
				hkVector4 obNormalisedVelocity = obInputVelocity;
				obNormalisedVelocity.normalize3();
				float proj2 = info.m_surfaceNormal.dot3( obNormalisedVelocity );

				if( proj2 < -0.99f )
				{
					obInputVelocity.mul4( 0.0f );
				} 
				else
				{
					if( proj < 0.0f )
					{
						hkVector4 obOutputVelocity( obInputVelocity );
						info.m_surfaceNormal.mul4( -proj );
						obOutputVelocity.add4( info.m_surfaceNormal );

						obInputVelocity = obOutputVelocity;
					}
				}
			}
		}

		return obInputVelocity;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::Update
	*
	*	DESCRIPTION		Update this in the world. If we're hard synced or explicity told to apply stuff
	*					absolutely, then we perform no integration. Otherwise, we integrate using desired
	*					velocities to move ourselves in the world.
	*
	***************************************************************************************************/
	void FullCharacterController::Update( float fTimeDelta, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform )
	{
		if(0.f == fTimeDelta || !m_isActive)
			return;
	
		GATSO_PHYSICS_START("FullCharacterController::Update");

		// For hard sync we just set phantom position explicitly
		// Also when in External_Control_State we use a bind function to ensure movements are done absolutely
		if ( !(eSynchronised == WS_SYNC_HARD || m_bApplyMovementAbsolutely) )
		{
			CPoint		obCurrentEntityPosition;
			hkVector4	obCurrentPhantomPosition;
			if (m_bPositionOnNextActivationSet)
			{
				// Get positions as they stand now
				obCurrentEntityPosition = m_obPositionOnNextActivation;
				obCurrentPhantomPosition = ComputeCharacterPositionFromEntity(&m_obPositionOnNextActivation);
				m_bPositionOnNextActivationSet = false;
			}
			else
			{
				// Get positions as they stand now
				obCurrentEntityPosition = ComputeEntityPositionFromCharacter();
				obCurrentPhantomPosition = GetPosition();
			}			
			
			// How much do we want to move?
			hkVector4 obDesiredDisplacement(	obGoalTranslation.X() - obCurrentEntityPosition.X(),
												obGoalTranslation.Y() - obCurrentEntityPosition.Y(),
												obGoalTranslation.Z() - obCurrentEntityPosition.Z() );

			// If we've just been activated in a new position, ignore what the movement wants us to do cos it's probably useless
			if (m_bPositionOnNextActivationSet)
			{
				obDesiredDisplacement = hkVector4::getZero();
			}

			
			// Let simplex solver take care about walls. So following line is obsolete
			//obDesiredDisplacement = CheckWallAndCorrectVelocityAsNeccessary(obDesiredDisplacement);

			// Calc velocity we need in this frame
			hkVector4 obDesiredVelocity( obDesiredDisplacement );
			obDesiredVelocity.mul4( ( 1.0f / fTimeDelta ) );

			// Clear this flag so next frame we do one from scratch
			m_bDoneGroundCheckThisFrame = false;

			//ntPrintf("VelNeeded: %f %f %f\n",obDesiredVelocity(0),obDesiredVelocity(1),obDesiredVelocity(2));
			//ntPrintf("GoalTrans: %f %f %f\n",obGoalTranslation.X(),obGoalTranslation.Y(),obGoalTranslation.Z());
			//ntPrintf("Movement: %f %f %f\n",obDesiredDisplacement(0),obDesiredDisplacement(1),obDesiredDisplacement(2));

			// Check we're in the world before we try to integrate
			if (m_characterProxy && m_phantom->getWorld())
			{

				hkStepInfo si;
				si.m_deltaTime		= fTimeDelta;
				si.m_invDeltaTime	= 1.0f / fTimeDelta;

				hkVector4 obGravityVelocity = hkVector4::getZero();					

				if ( m_bGravity && eSynchronised == WS_SYNC_NONE ) // We don't want to apply gravity when doing fancy relative anims, so make sure no sync
				{
					obGravityVelocity.addMul4(fTimeDelta, CPhysicsWorld::Get().GetGravity());
					hkVector4  obGravityDir = obGravityVelocity;
					obGravityDir.normalize3();

					hkReal gravComponent =  static_cast<hkReal>(m_characterProxy->getLinearVelocity().dot3(obGravityDir));
					if (m_obGroundInfo.m_supportedState != hkSurfaceInfo::UNSUPPORTED)
					{
						// only velocity in gravity dir. If we already going down so keep going down vel
						if (gravComponent > 0)
							obGravityVelocity.addMul4(gravComponent, obGravityDir);
					}
					else
					{
						// unsuported freefall
						obGravityVelocity.addMul4(gravComponent, obGravityDir);
					}
				}

				if (m_obGroundInfo.m_supportedState == hkSurfaceInfo::SLIDING)
					obDesiredVelocity.add4(obGravityVelocity); // simplex solver will take care about sliding

				if (m_softParent)
				{
					// move according to parent
					CMatrix newTrans = (*m_relToSoftParent) * m_softParent->GetRootTransformP()->GetWorldMatrix();
					obDesiredVelocity.add4(MathsTools::CPointTohkVector( (newTrans.GetTranslation() - m_entity->GetPosition()) * si.m_invDeltaTime)); //GetPhysicsSystem()->GetLinearVelocity())); 

					// rotate according to parent
					CQuat diffRotation = (~m_entity->GetRotation()) * CQuat(newTrans);
					obGoalOrientation = obGoalOrientation * diffRotation;
				}

				m_characterProxy->setLinearVelocity( obDesiredVelocity ); // move velocity
				
				m_listener->Update(); // Update our listener each time this character is moved

				m_characterProxy->integrate( si, obGravityVelocity );
			}

			// Get new positions from the freshly integrated phantom
			CPoint obNewEntityPosition = CPoint( ComputeEntityPositionFromCharacter() );
			CMatrix obNewEntityMatrix = CMatrix( obGoalOrientation, obNewEntityPosition );

			// Set ent matrix
			if (bUpdateEntityTransform)
			{
				// HACK incase they're completely upside down... thanks Chris
				if (obNewEntityMatrix.GetYAxis().Y() < 0)
				{
					// Apply an ugly correction - flip the axes
					CDirection obNewX(obNewEntityMatrix.GetXAxis());
					CDirection obNewY(obNewEntityMatrix.GetYAxis());
					CDirection obNewZ(obNewEntityMatrix.GetZAxis());
					obNewX *= -1;
					obNewY *= -1;
					obNewZ *= -1;
					obNewEntityMatrix.SetXAxis(obNewX);
					obNewEntityMatrix.SetYAxis(obNewY);
					obNewEntityMatrix.SetZAxis(obNewZ);
				}
				m_pobRoot->SetLocalMatrix( obNewEntityMatrix );
				if (m_softParent)
				{
					*m_relToSoftParent = m_pobRoot->GetWorldMatrix() * m_softParent->GetRootTransformP()->GetWorldMatrix().GetAffineInverse();
				}

			}

			// And update our push volumes
			UpdateCombatPhysicsPushVolumes( fTimeDelta );
		}
		else // Hard relative movement, set translation/orientation explicitly
		{
			CMatrix obNewMatrix(obGoalOrientation);
			obNewMatrix.SetTranslation(obGoalTranslation);
			m_pobRoot->SetLocalMatrix( obNewMatrix );
			SetPosition(ComputeCharacterPositionFromEntity());
		}

		// Update RB
		m_pobRigidBody->setTransform(m_phantom->getTransform());

		GATSO_PHYSICS_STOP("FullCharacterController::Update");
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetPosition
	*
	*	DESCRIPTION		Set phantom position.
	*
	***************************************************************************************************/
	void FullCharacterController::SetPosition (const hkVector4& position)
	{
		if (m_characterProxy && m_phantom->getWorld())
		{
			m_characterProxy->setPosition(position, true);
			if (m_pobCombatPhysicsPushVolumeDescriptors)
			{
				PushVolumeDescriptorList::iterator obIt = m_pobCombatPhysicsPushVolumeDescriptors->begin();
				PushVolumeDataList::iterator obItData = m_pobCombatPhysicsPushVolumeData->begin();
				while (obItData != m_pobCombatPhysicsPushVolumeData->end())
				{
					int iIndex = m_entity->GetHierarchy()->GetTransformIndex(((*obIt)->m_obBindToTransform));
					if (iIndex > -1 && (*obItData)->m_pobPhantom)
					{
						CMatrix obMtx = m_entity->GetHierarchy()->GetTransform(iIndex)->GetWorldMatrix();
						hkTransform obTransform(Physics::MathsTools::CQuatTohkQuaternion(CQuat(obMtx)),Physics::MathsTools::CPointTohkVector(obMtx.GetTranslation()));
						hkVector4 obXAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetXAxis());
						obXAxis.mul4((*obIt)->m_fXAxisPositionOffset);
						hkVector4 obYAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetYAxis());
						obYAxis.mul4((*obIt)->m_fYAxisPositionOffset);
						hkVector4 obZAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetZAxis());
						obZAxis.mul4((*obIt)->m_fZAxisPositionOffset);
						obTransform.getTranslation().add4(obXAxis);
						obTransform.getTranslation().add4(obYAxis);
						obTransform.getTranslation().add4(obZAxis);
						(*obItData)->m_pobPhantom->setTransform(obTransform);
					}

					obIt++;
					obItData++;
				}
			}

			// Update RB
			m_pobRigidBody->setTransform(m_phantom->getTransform());
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::SetLinearVelocity
	*
	*	DESCRIPTION		Set character controller desired velocity.
	*
	***************************************************************************************************/
	void FullCharacterController::SetLinearVelocity (const hkVector4& vel)
	{
		if (m_characterProxy && m_phantom->getWorld())
			m_characterProxy->setLinearVelocity(vel);
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::GetPosition
	*
	*	DESCRIPTION		Get phantom position.
	*
	***************************************************************************************************/
	hkVector4 FullCharacterController::GetPosition() const
	{
		if (m_characterProxy && m_phantom->getWorld())
			return m_characterProxy->getPosition();
		else return hkVector4::getZero();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::Debug_RenderCollisionInfo
	*
	*	DESCRIPTION		Render phantom position.
	*
	***************************************************************************************************/
	void FullCharacterController::Debug_RenderCollisionInfo()
	{
#ifndef _GOLD_MASTER
		DebugCollisionTools::RenderCollisionFlags(m_phantom);

		{
		const hkTransform& obTransform = m_phantom->getTransform();

		hkQuaternion obRotation(obTransform.getRotation());
		
		CMatrix obWorldMatrix(
			CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
			CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));

		DebugCollisionTools::RenderShape(obWorldMatrix,m_phantom->getCollidable()->getShape());
		}

		if (m_pobCombatPhysicsPushVolumeDescriptors)
		{
			PushVolumeDataList::iterator obItData = m_pobCombatPhysicsPushVolumeData->begin();
			PushVolumeDescriptorList::iterator obIt = m_pobCombatPhysicsPushVolumeDescriptors->begin();
			while (obItData != m_pobCombatPhysicsPushVolumeData->end())
			{
				if ((*obItData)->m_pobPhantom)
				{
					int iIndex = m_entity->GetHierarchy()->GetTransformIndex(((*obIt)->m_obBindToTransform));
					if (iIndex > -1)
					{
						CMatrix obMtx = m_entity->GetHierarchy()->GetTransform(iIndex)->GetWorldMatrix();
						g_VisualDebug->RenderAxis(obMtx,0.5f);
					}
					const hkTransform& obTransform = (*obItData)->m_pobPhantom->getTransform();
					hkQuaternion obRotation(obTransform.getRotation());
					CMatrix obWorldMatrix(CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
											CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));
					DebugCollisionTools::RenderShape(obWorldMatrix,(*obItData)->m_pobPhantom->getCollidable()->getShape());
				}

				obItData++;
				obIt++;
			}
		}

		// Draw RB
		{
			const hkTransform& obTransform = m_phantom->getTransform();
			hkQuaternion obRotation(obTransform.getRotation());			
			CMatrix obWorldMatrix(
				CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
				CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));

			DebugCollisionTools::RenderShape(obWorldMatrix,m_pobRigidBody->getCollidable()->getShape());
		}
#endif
	}

	/***************************************************************************************************
	*
	*	CLASS			MyRaycastPhantomCharacterControllerOverlapListener
	*
	*	DESCRIPTION		Anything non-static that it overlaps gets pushed away.
	*
	***************************************************************************************************/
	class MyRaycastPhantomCharacterControllerOverlapListener : public hkPhantomOverlapListener 
	{
	public:

		MyRaycastPhantomCharacterControllerOverlapListener ( CEntity* pobMyEntity )
		{
			m_pobEntity = pobMyEntity;
		};

		virtual void collidableAddedCallback( const hkCollidableAddedEvent& event )
		{
			// Accept everything
			event.m_collidableAccept = HK_COLLIDABLE_ACCEPT;

			hkRigidBody* pobRB = hkGetRigidBody(event.m_collidable);
			if (pobRB) // We have a fixed rigid body of some form
			{
				m_apobOverlappers.pushBack(pobRB);
			}
		};

		virtual void collidableRemovedCallback( const hkCollidableRemovedEvent& event ) 
		{
			hkRigidBody* pobRB = hkGetRigidBody(event.m_collidable);
			if (pobRB) // We have a fixed rigid body of some form
			{
				int i = m_apobOverlappers.indexOf(pobRB);
				if (i > -1)
					m_apobOverlappers.removeAt(i);
			}
		};

		bool Update(float fTimeStep, hkVector4& obLastGroundPosition)
		{
			bool bIsOverlappingFixed = false;

			for (int i = 0; i < m_apobOverlappers.getSize(); i++)
			{
				if (m_apobOverlappers[i]->getMass() > 0.0f)
				{
					// Apply a push to the dynamic rigid body
					m_apobOverlappers[i]->activate();
					hkVector4 obPush = m_apobOverlappers[i]->getPosition();
					obPush.sub4(Physics::MathsTools::CPointTohkVector(m_pobEntity->GetPosition()));
					obPush(1) = 0.0f;
					obPush.normalize3();
					obPush.mul4(1.0f/fTimeStep);
					obPush.mul4(0.05f);
					m_apobOverlappers[i]->setLinearVelocity(obPush);
				}
				else if (m_apobOverlappers[i]->getMass() == 0.0f)
				{
						bIsOverlappingFixed = true;
					}
				}

			return bIsOverlappingFixed;
		};
	private:
		CEntity* m_pobEntity;
		hkArray<hkRigidBody*> m_apobOverlappers;
	};

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::RaycastPhantomCharacterController
	*
	*	DESCRIPTION		Construct.
	*
	***************************************************************************************************/
	RaycastPhantomCharacterController::RaycastPhantomCharacterController( Character* p_entity, CColprimDesc* pobColPrim )
		: CharacterController(p_entity,pobColPrim)
	{
		m_pobPhantom = 0;

		m_bCollideWithStatic = true;

		SetupCharacterVolume();
	};

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::~RaycastPhantomCharacterController
	*
	*	DESCRIPTION		Destroy.
	*
	***************************************************************************************************/
	RaycastPhantomCharacterController::~RaycastPhantomCharacterController( )
	{
		if (m_world)
			RemoveFromWorld( m_world );

		if (m_pol)
		{
			if (m_world)
				m_pobPhantom->removePhantomOverlapListener( m_pol );
			HK_DELETE( m_pol );
		}

		if (m_pobPhantom)
		{
			m_pobPhantom->removeReference();
		}
	};
	
	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::Activate
	*
	*	DESCRIPTION		Active after calling this.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::Activate( )
	{
		Physics::WriteAccess mutex;
		
		if( !m_isActive )
		{
			AddToWorld(CPhysicsWorld::Get().GetHavokWorldP());
			SetPosition( ComputeCharacterPositionFromEntity() );
		}

		m_isActive = true;
	};

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::Deactivate
	*
	*	DESCRIPTION		Won't be active after calling this.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::Deactivate( )
	{
		if( m_isActive )
			RemoveFromWorld(CPhysicsWorld::Get().GetHavokWorldP());

		m_isActive = false;
	};

	/***************************************************************************************************
	*
	*	FUNCTION		FullCharacterController::RegisterSystem
	*
	*	DESCRIPTION		Registers this with collision system.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::RegisterSystem( System* p_system )
	{
		p_system->GetCollisionListener()->RegisterCharacterController( const_cast<RaycastPhantomCharacterController*>(this) );
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetupCharacterVolume
	*
	*	DESCRIPTION		Creates the shape that the character phantom will take.
	*
	***************************************************************************************************/
	#define fPERCENTAGE_HOLDING_SIZE 1.0f
	#define fSIZE_SPHERE_STEP 0.16f
	void RaycastPhantomCharacterController::SetupCharacterVolume()
	{		
		const float fLength = m_pobColPrim->m_obCapsuleData.fLength;
		const float fRadius = m_pobColPrim->m_obCapsuleData.fRadius;

		m_fCapsuleHalfLength = (fLength * 0.5f) + fRadius;

		// Construct a shape
		hkVector4 vertexA(0, (fLength*0.5f), 0);
		hkVector4 vertexB(0, (fLength*-0.5f), 0);		

		// Create a capsule to represent the character standing
		m_standShape = HK_NEW hkCapsuleShape(vertexA, vertexB, fRadius);
		hkVector4 size; size.setSub4( vertexA, vertexB );

		float fIncrease = fPERCENTAGE_HOLDING_SIZE * fRadius;
		float fHoldRadius = fRadius + fIncrease;

		hkVector4 axis; axis.setSub4( vertexA, vertexB ); // vertexA - vertexB
		axis.normalize3(); axis.mul4(fIncrease);

		vertexB.add4(axis);
		vertexA.sub4(axis);

		// Allow the entity to modify the holding capsule, this code was added so that
		// the archer could use the holding capsule shape as a covering capsule

		{
			CPoint ptA = Physics::MathsTools::hkVectorToCPoint(vertexA);
			CPoint ptB = Physics::MathsTools::hkVectorToCPoint(vertexB);

			m_entity->ModifyPhysicsHoldingShape( ptA, ptB, fHoldRadius);

			vertexA = Physics::MathsTools::CPointTohkVector(ptA);
			vertexB = Physics::MathsTools::CPointTohkVector(ptB);
		}

		m_holdingShape = HK_NEW hkCapsuleShape(vertexA, vertexB, fHoldRadius);	

		m_obEntityCFlag.base = 0;

		if( m_entity->IsPlayer() || m_entity->IsFriendly() )
			m_obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_PLAYER_BIT;
		else 
			m_obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_ENEMY_BIT;

		if (m_entity->IsImportant())
			m_obEntityCFlag.flags.i_am_important = 1;

		if( m_entity->IsPlayer() || m_entity->IsBoss() || m_entity->IsFriendly() || ( m_entity->IsAI() && ((AI*)m_entity)->AttackAI() )  )
		{
			m_obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	| 
													Physics::RAGDOLL_BIT						|
													Physics::LARGE_INTERACTABLE_BIT				|	
													Physics::SMALL_INTERACTABLE_BIT				|	
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
		} else {
			m_obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	| //Turned off for the test
													Physics::RAGDOLL_BIT						|
													Physics::LARGE_INTERACTABLE_BIT				|		
													Physics::SMALL_INTERACTABLE_BIT				|	
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
		}

		hkTransform trf; 
		trf.setIdentity();
		m_pobPhantom = HK_NEW hkSimpleShapePhantom( m_standShape, trf, (int)m_obEntityCFlag.base );
		hkPropertyValue val((void*)m_entity);
		m_pobPhantom->addProperty(Physics::PROPERTY_ENTITY_PTR, val);	

		// Use a listener to push
		m_pol = HK_NEW MyRaycastPhantomCharacterControllerOverlapListener(m_entity);
		m_pobPhantom->addPhantomOverlapListener(m_pol);
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::RemoveFromWorld
	*
	*	DESCRIPTION		Remove everything (character phantom, feet phantom, associated listenes).
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::RemoveFromWorld (hkWorld* pWorld)
	{
		Physics::WriteAccess mutex;

		m_world->removePhantom(m_pobPhantom);

		RemoveCombatPhysicsPushVolumesFromWorld();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::GetLinearVelocity
	*
	*	DESCRIPTION		Returns linear velocity of the character controller, or zero if there isn't one.
	*
	***************************************************************************************************/
	hkVector4 RaycastPhantomCharacterController::GetLinearVelocity( )
	{
		// Meaningless in this implementation - could use last frames displacement/time
		return hkVector4::getZero();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::IsCharacterControllerOnGround
	*
	*	DESCRIPTION		Returns true if the character controller is supported by something below them.
	*
	***************************************************************************************************/
	bool RaycastPhantomCharacterController::IsCharacterControllerOnGround( void ) 	
	{
		// Could raycast and see if we're half length away from the ground
		// Could get closest contact points and see if one of them is an up vector static

		// This is an expensive check
		//m_obGroundPosition = RaycastGetGroundPosition();

		CPoint obGroundPosition = Physics::MathsTools::hkVectorToCPoint(m_obGroundPosition);
		CDirection obVector = CDirection( GetEntityPosition() - obGroundPosition );

		if ( obVector.LengthSquared() < 0.05f*0.05f )
		{
			return true;
		}

		return false;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::RaycastGetGroundPosition
	*
	*	DESCRIPTION		Performs a raycast from the top of the capsule down to find the ground. If that 
	*					fails it'll do one from the bottom of the capsule up.
	*
	***************************************************************************************************/
	hkVector4 RaycastPhantomCharacterController::RaycastGetGroundPosition( hkVector4* pobPosition )
	{
		if (m_bDoneGroundCheckThisFrame)
		{
			return m_obGroundPosition;
		}

		ntError(m_world);

		// Try a down cast
		{
			// Setup from point
			CPoint obCapsuleTop;
			if ( pobPosition )
			{
				obCapsuleTop = Physics::MathsTools::hkVectorToCPoint( *pobPosition );
				obCapsuleTop.Y() += m_fCapsuleHalfLength;
			}
			else
			{
				obCapsuleTop = Physics::MathsTools::hkVectorToCPoint( m_standShape->getVertex(0) );
				obCapsuleTop = obCapsuleTop * GetEntityWorldMatrix();
			}

			//g_VisualDebug->RenderPoint(obCapsuleTop,10.0f,DC_YELLOW);

			// And to point
			hkVector4 obDownPoint(obCapsuleTop.X(),obCapsuleTop.Y()-100.0f,obCapsuleTop.Z(),0.0f);
			// And prepare a vector describing the ray
			CDirection obRayVector = CDirection( Physics::MathsTools::hkVectorToCPoint(obDownPoint) - obCapsuleTop );

			//g_VisualDebug->RenderLine(obCapsuleTop, obCapsuleTop+obRayVector,DC_YELLOW);

			// Setup input
			hkWorldRayCastInput obIn;
			obIn.m_from = Physics::MathsTools::CPointTohkVector(obCapsuleTop);
			obIn.m_to = obDownPoint;

			// Set collision flags
			Physics::RaycastCollisionFlag obFlag; 
			obFlag.base = 0;
			obFlag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
			obFlag.flags.i_collide_with = (Physics::LARGE_INTERACTABLE_BIT);
			obIn.m_filterInfo = obFlag.base;

			// Cast the ray
			hkAllRayHitCollector obOut;		
			m_world->castRay(obIn,obOut);
			obOut.sortHits();
			float fHitFraction = -1.0f;
			// Sort through the hits using only those that are static geometry and within slope allowance
			for (int i = 0; i < obOut.getHits().getSize(); i++)
			{
				hkRigidBody* pobBody = hkGetRigidBody(obOut.getHits()[i].m_rootCollidable);
				if (pobBody && pobBody->getMass() == 0.0f)
				{
					CDirection obUp(0.0f,1.0f,0.0f);
					CDirection obNormal(obOut.getHits()[i].m_normal(0),obOut.getHits()[i].m_normal(1),obOut.getHits()[i].m_normal(2));
					if (acos(obUp.Dot(obNormal)) < GetMaxSlope())
					{
						fHitFraction = obOut.getHits()[i].m_hitFraction;
						break;
					}
				}
			}

			if ( fHitFraction > 0.0f )
			{
				// Got a hit, use it and return
				obRayVector *= fHitFraction;
				m_bDoneGroundCheckThisFrame = true;
				return Physics::MathsTools::CPointTohkVector( obCapsuleTop + obRayVector );
			}
		}

		// Try an up cast
		{
			CPoint obCapsuleBottom;
			if ( pobPosition )
			{
				obCapsuleBottom = Physics::MathsTools::hkVectorToCPoint( *pobPosition );
				obCapsuleBottom.Y() -= m_fCapsuleHalfLength;
			}
			else
			{
				obCapsuleBottom = Physics::MathsTools::hkVectorToCPoint( m_standShape->getVertex(1) );
				obCapsuleBottom = obCapsuleBottom * GetEntityWorldMatrix();
			}

			hkVector4 obUpPoint(obCapsuleBottom.X(),obCapsuleBottom.Y()+100.0f,obCapsuleBottom.Z(),0.0f);

			CDirection obRayVector = CDirection( Physics::MathsTools::hkVectorToCPoint(obUpPoint) - obCapsuleBottom );

			hkWorldRayCastInput obIn;
			obIn.m_from = Physics::MathsTools::CPointTohkVector(obCapsuleBottom);
			obIn.m_to = obUpPoint;
			
			Physics::RaycastCollisionFlag obFlag; 
			obFlag.base = 0;
			obFlag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
			obFlag.flags.i_collide_with = (Physics::LARGE_INTERACTABLE_BIT);
			obIn.m_filterInfo = obFlag.base;
		
			hkAllRayHitCollector obOut;		
			m_world->castRay(obIn,obOut);
			obOut.sortHits();
			float fHitFraction = -1.0f;
			for (int i = 0; i < obOut.getHits().getSize(); i++)
			{
				hkRigidBody* pobBody = hkGetRigidBody(obOut.getHits()[i].m_rootCollidable);
				if (pobBody && pobBody->getMass() == 0.0f)
				{
					CDirection obUp(0.0f,1.0f,0.0f);
					CDirection obNormal(obOut.getHits()[i].m_normal(0),obOut.getHits()[i].m_normal(1),obOut.getHits()[i].m_normal(2));
					if (acos(obUp.Dot(obNormal)) < GetMaxSlope())
					{
						fHitFraction = obOut.getHits()[i].m_hitFraction;
						break;
					}
				}
			}

			if ( fHitFraction > 0.0f )
			{
				// Got a hit, use it and return
				obRayVector *= fHitFraction;
				m_bDoneGroundCheckThisFrame = true;
				return Physics::MathsTools::CPointTohkVector( obCapsuleBottom + obRayVector );
			}
		}

		// If we're here we couldn't find anything to stand on either above or below for over 100 metres!
		//ntError(0);
		return Physics::MathsTools::CPointTohkVector(m_pobRoot->GetWorldMatrix().GetTranslation());
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::UpdateCollisionFilter
	*
	*	DESCRIPTION		3 guesses what this function does. If you still get it wrong after 3 tries, 
	*					I'm going to do a service to society and lock you in a box.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::UpdateCollisionFilter()
	{
		if (m_pobPhantom->getWorld())
			m_pobPhantom->getWorld()->updateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS ); 
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::RaycastGetGroundPosition
	*
	*	DESCRIPTION		Performs a small raycast within character phantom to find ground. BROKEN.
	*
	***************************************************************************************************/
	hkVector4 RaycastPhantomCharacterController::ShortRaycastGetGroundPosition( hkVector4* pobPosition )
	{
		if (m_bDoneGroundCheckThisFrame)
		{
			return m_obGroundPosition;
		}

		ntError(m_world);

		// Try a down cast
		{
			hkVector4 obAabbPosition;
			if (pobPosition)
				obAabbPosition = *pobPosition;
			else
				obAabbPosition = GetPosition();
			obAabbPosition(1) -= m_fCapsuleHalfLength-0.05f;
			hkVector4 obHalfExtentsPv( 0.05f, 1.0f, 0.05f );
			hkVector4 obHalfExtentsNv( -0.05f, -1.0f, -0.05f );
			obHalfExtentsPv.add4(obAabbPosition);
			obHalfExtentsNv.add4(obAabbPosition);
			hkAabb aabb(	obHalfExtentsNv,
							obHalfExtentsPv );
			hkAabbPhantom* pobAabbPhantom = HK_NEW hkAabbPhantom( aabb, m_obEntityCFlag.base );
			pobAabbPhantom->setAabb(aabb);

			m_world->addPhantom( pobAabbPhantom );
			pobAabbPhantom->getCollidableRw()->setCollisionFilterInfo(m_obEntityCFlag.base);
			m_world->updateCollisionFilterOnPhantom( pobAabbPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );

			// Setup input
			hkWorldRayCastInput obIn;
			obIn.m_from = GetPosition();
			obIn.m_to = GetPosition();
			obIn.m_to(1) -= m_fCapsuleHalfLength*1.5f;

			CDirection obRayVector = CDirection( Physics::MathsTools::hkVectorToCPoint(obIn.m_to) - Physics::MathsTools::hkVectorToCPoint(obIn.m_from) );

			// Set collision flags
			Physics::EntityCollisionFlag obEntityCFlag; 
			obEntityCFlag.base = 0;

			if( m_entity->IsPlayer() || m_entity->IsFriendly() )
				obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_PLAYER_BIT;
			else 
				obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_ENEMY_BIT;

			if( m_entity->IsPlayer() || m_entity->IsFriendly() )
			{
				obEntityCFlag.flags.i_collide_with	= (	Physics::LARGE_INTERACTABLE_BIT	);
			}

			obIn.m_filterInfo = obEntityCFlag.base;

			// Cast the ray
			hkAllRayHitCollector obOut;		
			pobAabbPhantom->castRay(obIn,obOut);
			m_world->removePhantom(pobAabbPhantom);
			HK_DELETE( pobAabbPhantom );
			obOut.sortHits();
			float fHitFraction = -1.0f;
			// Sort through the hits using only those that are static geometry and within slope allowance
			for (int i = 0; i < obOut.getHits().getSize(); i++)
			{
				hkRigidBody* pobBody = hkGetRigidBody(obOut.getHits()[i].m_rootCollidable);
				if (pobBody && pobBody->getMass() == 0.0f)
				{
					CDirection obUp(0.0f,1.0f,0.0f);
					CDirection obNormal(obOut.getHits()[i].m_normal(0),obOut.getHits()[i].m_normal(1),obOut.getHits()[i].m_normal(2));
					if (acos(obUp.Dot(obNormal)) < GetMaxSlope())
					{
						fHitFraction = obOut.getHits()[i].m_hitFraction;
						break;
					}
				}
			}

			if ( fHitFraction > 0.0f )
			{
				// Got a hit, use it and return
				obRayVector *= fHitFraction;
				obIn.m_from.add4(Physics::MathsTools::CDirectionTohkVector( obRayVector ));
				m_bDoneGroundCheckThisFrame = true;
				return obIn.m_from;
			}
		}

		// If we're here we couldn't find anything to stand on either above or below for over 100 metres!
		return Physics::MathsTools::CPointTohkVector(m_pobRoot->GetWorldMatrix().GetTranslation());
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetCollidable
	*
	*	DESCRIPTION		Set if this is to collide with stuff
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::SetCollidable (bool isCollidable)
	{
		m_bCollideWithStatic = isCollidable;

		if ( isCollidable )
		{
			if ( m_pobPhantom )
			{
				Physics::ReadAccess read_mutex( m_pobPhantom );

				if ( m_pobPhantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					Physics::WriteAccess write_mutex( m_pobPhantom );
					m_pobPhantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				
					if( m_pobPhantom->getWorld() )
					{
						Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
					}
				}
			}

		}
		else
		{
			if ( m_pobPhantom )
			{
				Physics::WriteAccess mutex( m_pobPhantom );

				if(m_pobPhantom->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
				{
					m_pobPhantom->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
				}

				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::ALWAYS_RETURN_FALSE_BIT;
				hkPropertyValue val2( (int)exceptionFlag.base );
				m_pobPhantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

				if ( m_pobPhantom->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}

		}

	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetDynamicCollidable
	*
	*	DESCRIPTION		Set if this is to collide with dynamic stuff
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::SetDynamicCollidable (bool isCollidable)
	{
		if( isCollidable )
		{
			if( m_pobPhantom )
			{
				Physics::ReadAccess read_mutex( m_pobPhantom );

				if( m_pobPhantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					Physics::WriteAccess write_mutex( m_pobPhantom );

					m_pobPhantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				
					if( m_pobPhantom->getWorld() )
					{
						Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
					}
				}
			}

		}
		else
		{
			if ( m_pobPhantom )
			{
				Physics::WriteAccess mutex( m_pobPhantom );

				if( m_pobPhantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					m_pobPhantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				}

				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::ONLY_FIXED_GEOM;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_pobPhantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

				if ( m_pobPhantom->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetRagdollCollidable
	*
	*	DESCRIPTION		Set if this is to collide with ragdoll.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::SetRagdollCollidable(bool isCollidable)
	{
		if( isCollidable )
		{
			if( m_pobPhantom )
			{
				Physics::ReadAccess read_mutex( m_pobPhantom );

				if( m_pobPhantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					Physics::WriteAccess write_mutex( m_pobPhantom );

					m_pobPhantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				
					if( m_pobPhantom->getWorld() )
					{
						Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
					}
				}
			}

		}
		else
		{
			if ( m_pobPhantom )
			{
				Physics::WriteAccess mutex( m_pobPhantom );

				if( m_pobPhantom->hasProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ) )
				{
					m_pobPhantom->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				}

				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::RAGDOLL_BIT;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_pobPhantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);

				if ( m_pobPhantom->getWorld() )
				{
					Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				}
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetFilterExceptionFlags
	*
	*	DESCRIPTION		Set collision filter flags
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::SetFilterExceptionFlags( unsigned int uiFlags )
	{
		if ( m_pobPhantom )
		{
			Physics::ReadAccess read_mutex( m_pobPhantom );

			if( m_pobPhantom->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT) ) // Remove any existing filter exception properties
			{
				Physics::WriteAccess write_mutex( m_pobPhantom );
				m_pobPhantom->removeProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT);
			}

			if (uiFlags!=0) // If flags have been set, then we add a filter exception property
			{
				Physics::WriteAccess write_mutex( m_pobPhantom );
				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= uiFlags;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_pobPhantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);			
			}
			
			if( m_pobPhantom->getWorld() )
			{
				Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetHoldingCapsule
	*
	*	DESCRIPTION		Change to bigger capsule for holding stuff
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::SetHoldingCapsule(bool bHold)
	{		
		if (bHold)
		{
			m_pobPhantom->setShape(m_holdingShape);
		} 
		else 
		{
			m_pobPhantom->setShape(m_standShape);
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetKOState
	*
	*	DESCRIPTION		Set KO status. 
	*
	***************************************************************************************************/
	void  RaycastPhantomCharacterController::SetKOState(KO_States state)
	{
		Physics::WriteAccess mutex;

		if (state)
		{			
			ChatacterControllerCollisionFlag obFlag;
			obFlag.base = m_pobPhantom->getCollidable()->getCollisionFilterInfo();
			obFlag.flags.i_am_in_KO_state = state;
			// Do not collide with other characterts and ragdolls
			// Don't know why this is needed, it's bad so I'm taking it out - DGF
			/*obFlag.flags.i_collide_with &= ~(Physics::CHARACTER_CONTROLLER_PLAYER_BIT | 
											Physics::CHARACTER_CONTROLLER_ENEMY_BIT |
											Physics::RAGDOLL_BIT);*/

			m_pobPhantom->getCollidableRw()->setCollisionFilterInfo(obFlag.base);
			if (m_pobPhantom->getWorld())
				m_pobPhantom->getWorld()->updateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS ); 			
		}
		else
		{

			ChatacterControllerCollisionFlag obFlag;
			obFlag.base = m_pobPhantom->getCollidable()->getCollisionFilterInfo();
			obFlag.flags.i_am_in_KO_state = state;
			// Collide with other characterts and ragdolls
			obFlag.flags.i_collide_with = m_obEntityCFlag.flags.i_collide_with;

			m_pobPhantom->getCollidableRw()->setCollisionFilterInfo(obFlag.base);	
			if (m_pobPhantom->getWorld())
				m_pobPhantom->getWorld()->updateCollisionFilterOnPhantom( m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
		}		
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::AddToWorld
	*
	*	DESCRIPTION		Add this character phantom, feet phantom and associated listeners to the world
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::AddToWorld (hkWorld* pWorld)
	{
		if (m_pobPhantom->getWorld() == pWorld)
			return;

		Physics::WriteAccess mutex;

		m_world = pWorld;

		m_world->addPhantom(m_pobPhantom);	

		AddCombatPhysicsPushVolumesToWorld();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::Update
	*
	*	DESCRIPTION		Update this in the world. If we're hard synced or explicity told to apply stuff
	*					absolutely, then we perform no integration. Otherwise, we integrate using desired
	*					velocities to move ourselves in the world.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::Update( float fTimeDelta, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform )
	{
		if(0.f == fTimeDelta || !m_isActive)
			return;
	
		CGatso::Start("RaycastPhantomCharacterController::Update");

		// For hard sync we just set phantom position explicitly
		// Also when in External_Control_State we use a bind function to ensure movements are done absolutely
		if ( !(eSynchronised == WS_SYNC_HARD || m_bApplyMovementAbsolutely) )
		{
			CPoint		obCurrentEntityPosition;
			hkVector4	obCurrentPhantomPosition;
			if (m_bPositionOnNextActivationSet)
			{
				// Get positions as they stand now
				obCurrentEntityPosition = m_obPositionOnNextActivation;
				obCurrentPhantomPosition = ComputeCharacterPositionFromEntity(&m_obPositionOnNextActivation);
				m_bPositionOnNextActivationSet = false;
			}
			else
			{
				// Get positions as they stand now
				obCurrentEntityPosition = ComputeEntityPositionFromCharacter();
				obCurrentPhantomPosition = GetPosition();
			}

			hkVector4 obDesiredDisplacement(0,0,0,0);
			// Push stuff we're overlapping and check for notification of fixed geom overlapping, use the last raycasted ground position to filter out floor
			if (m_pol->Update(fTimeDelta, m_obGroundPosition) && m_bCollideWithStatic)
			{
				// We're overlapping a fixed rigid, need to go back however much we moved last frame
				/*obDesiredDisplacement = Physics::MathsTools::CDirectionTohkVector( m_obDisplacementLastFrame );
				obDesiredDisplacement.mul4(-1.0f);*/

				// Raycast down/up to find some ground to sit myself on
				hkVector4 obGoalTranslationToTest = obCurrentPhantomPosition;
				obGoalTranslationToTest.add4(obDesiredDisplacement);
				CPoint obGroundPosition = Physics::MathsTools::hkVectorToCPoint(RaycastGetGroundPosition( &obGoalTranslationToTest ));
				obGroundPosition.Y() += 0.05f;
				m_pobPhantom->setPosition( ComputeCharacterPositionFromEntity( &obGroundPosition ) );
				m_obGroundPosition = Physics::MathsTools::CPointTohkVector(obGroundPosition);

				//g_VisualDebug->RenderPoint(obGroundPosition,10.0f,DC_RED);

				CPoint obNewGoalTranslation( obGoalTranslation );
				obNewGoalTranslation.Y() = obGroundPosition.Y();

				// How much do we want to move?
				obDesiredDisplacement = hkVector4(	obNewGoalTranslation.X() - obCurrentEntityPosition.X(),
													obNewGoalTranslation.Y() - obCurrentEntityPosition.Y(),
													obNewGoalTranslation.Z() - obCurrentEntityPosition.Z(),
													0.0f );	
			}
			else
			{
				// How much do we want to move?
				obDesiredDisplacement = hkVector4(	obGoalTranslation.X() - obCurrentEntityPosition.X(),
													obGoalTranslation.Y() - obCurrentEntityPosition.Y(),
													obGoalTranslation.Z() - obCurrentEntityPosition.Z(),
													0.0f );			
			}

			// Keep our record updated
			m_obDisplacementLastFrame = Physics::MathsTools::hkVectorToCDirection( obDesiredDisplacement );

			// Clear this flag so next frame we do one from scratch
			m_bDoneGroundCheckThisFrame = false;

			// Check we're in the world
			if ( m_pobPhantom->getWorld() )
			{				
				// Setting position properly with a downward raycast is expensive so...
				//SetPosition( Physics::MathsTools::CPointTohkVector( obGoalTranslation ) );

				// ... Just move the phantom to the desired position. End of.
				obCurrentPhantomPosition.add4( obDesiredDisplacement );
				//CPoint obGroundPosition = Physics::MathsTools::hkVectorToCPoint( ShortRaycastGetGroundPosition( &obCurrentPhantomPosition ) );
				m_pobPhantom->setPosition( obCurrentPhantomPosition );
			}

			// Get new position
			CPoint obNewEntityPosition = CPoint( ComputeEntityPositionFromCharacter() );
			CMatrix obNewEntityMatrix = CMatrix( obGoalOrientation, obNewEntityPosition );

			// Set ent matrix
			if (bUpdateEntityTransform)
			{				
				m_pobRoot->SetLocalMatrix( obNewEntityMatrix );
			}

			// And update our push volumes
			UpdateCombatPhysicsPushVolumes( fTimeDelta );
		}
		else // Hard relative movement, set translation/orientation explicitly
		{
			CMatrix obNewMatrix(obGoalOrientation);
			obNewMatrix.SetTranslation(obGoalTranslation);
			m_pobRoot->SetLocalMatrix( obNewMatrix );
			SetPosition(ComputeCharacterPositionFromEntity());
		}

		CGatso::Stop("RaycastPhantomCharacterController::Update");
	} 

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetPosition
	*
	*	DESCRIPTION		Set phantom position.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::SetPosition (const hkVector4& position)
	{
		if (m_pobPhantom->getWorld())
		{	
			// Raycast down/up to find some ground to sit myself on
			hkVector4 obGoalTranslationToTest = position;
			CPoint obGroundPosition = Physics::MathsTools::hkVectorToCPoint(RaycastGetGroundPosition( &obGoalTranslationToTest ));
			m_pobPhantom->setPosition( ComputeCharacterPositionFromEntity( &obGroundPosition ) );
			m_obGroundPosition = Physics::MathsTools::CPointTohkVector(obGroundPosition);

			if (m_pobCombatPhysicsPushVolumeDescriptors)
			{
				PushVolumeDataList::iterator obItData = m_pobCombatPhysicsPushVolumeData->begin();
				PushVolumeDescriptorList::iterator obIt = m_pobCombatPhysicsPushVolumeDescriptors->begin();
				while (obIt != m_pobCombatPhysicsPushVolumeDescriptors->end())
				{
					int iIndex = m_entity->GetHierarchy()->GetTransformIndex(((*obIt)->m_obBindToTransform));
					if (iIndex > -1)
					{
						CMatrix obMtx = m_entity->GetHierarchy()->GetTransform(iIndex)->GetWorldMatrix();
						hkTransform obTransform(Physics::MathsTools::CQuatTohkQuaternion(CQuat(obMtx)),Physics::MathsTools::CPointTohkVector(obMtx.GetTranslation()));
						hkVector4 obXAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetXAxis());
						obXAxis.mul4((*obIt)->m_fXAxisPositionOffset);
						hkVector4 obYAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetYAxis());
						obYAxis.mul4((*obIt)->m_fYAxisPositionOffset);
						hkVector4 obZAxis = Physics::MathsTools::CDirectionTohkVector(obMtx.GetZAxis());
						obZAxis.mul4((*obIt)->m_fZAxisPositionOffset);
						obTransform.getTranslation().add4(obXAxis);
						obTransform.getTranslation().add4(obYAxis);
						obTransform.getTranslation().add4(obZAxis);
						(*obItData)->m_pobPhantom->setTransform(obTransform);
					}

					obIt++;
					obItData++;
				}
			}
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::SetLinearVelocity
	*
	*	DESCRIPTION		Set character controller desired velocity.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::SetLinearVelocity (const hkVector4& vel)
	{
		// Meaningless in this implementation
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::GetPosition
	*
	*	DESCRIPTION		Get phantom position.
	*
	***************************************************************************************************/
	hkVector4 RaycastPhantomCharacterController::GetPosition() const
	{
		if (m_pobPhantom->getWorld())
			return m_pobPhantom->getCollidable()->getTransform().getTranslation();
		else return hkVector4::getZero();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		RaycastPhantomCharacterController::Debug_RenderCollisionInfo
	*
	*	DESCRIPTION		Render phantom position.
	*
	***************************************************************************************************/
	void RaycastPhantomCharacterController::Debug_RenderCollisionInfo()
	{
#ifndef _GOLD_MASTER
		DebugCollisionTools::RenderCollisionFlags(m_pobPhantom);

		{
		const hkTransform& obTransform = m_pobPhantom->getTransform();

		hkQuaternion obRotation(obTransform.getRotation());
		
		CMatrix obWorldMatrix(
			CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
			CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));

		DebugCollisionTools::RenderShape(obWorldMatrix,m_pobPhantom->getCollidable()->getShape());
		}

		if (m_pobCombatPhysicsPushVolumeDescriptors)
		{
			PushVolumeDescriptorList::iterator obIt = m_pobCombatPhysicsPushVolumeDescriptors->begin();
			PushVolumeDataList::iterator obItData = m_pobCombatPhysicsPushVolumeData->begin();
			while (obIt != m_pobCombatPhysicsPushVolumeDescriptors->end())
			{
				if ((*obItData)->m_pobPhantom)
				{
					int iIndex = m_entity->GetHierarchy()->GetTransformIndex(((*obIt)->m_obBindToTransform));
					if (iIndex > -1)
					{
						CMatrix obMtx = m_entity->GetHierarchy()->GetTransform(iIndex)->GetWorldMatrix();
						g_VisualDebug->RenderAxis(obMtx,0.5f);
					}
					const hkTransform& obTransform = (*obItData)->m_pobPhantom->getTransform();
					hkQuaternion obRotation(obTransform.getRotation());
					CMatrix obWorldMatrix(CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
											CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));
					DebugCollisionTools::RenderShape(obWorldMatrix,(*obItData)->m_pobPhantom->getCollidable()->getShape());
				}

				obIt++;
				obItData++;
			}
		}
#endif
	}

	/***************************************************************************************************
	*
	*	FUNCTION		DummyCharacterController::RegisterSystem
	*
	*	DESCRIPTION		Register me.
	*
	***************************************************************************************************/
	void DummyCharacterController::RegisterSystem( System* pobSystem )
	{
		pobSystem->GetCollisionListener()->RegisterCharacterController( const_cast<Physics::DummyCharacterController*>(this) );
	}

	/***************************************************************************************************
	*
	*	FUNCTION		DummyCharacterController::Activate
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	void DummyCharacterController::Activate( ) 
	{ 
		AddToWorld(CPhysicsWorld::Get().GetHavokWorldP());
		m_isActive = true; 
	}
	
	/***************************************************************************************************
	*
	*	FUNCTION		DummyCharacterController::Deactivate
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/	
	void DummyCharacterController::Deactivate( ) 
	{ 
		RemoveFromWorld(m_world);
		m_isActive = false; 
	}

	/***************************************************************************************************
	*
	*	FUNCTION		DummyCharacterController::AddToWorld
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	void DummyCharacterController::AddToWorld (hkWorld* pWorld) 
	{
		m_world = pWorld;

		AddCombatPhysicsPushVolumesToWorld();
	}

	/***************************************************************************************************
	*
	*	FUNCTION		DummyCharacterController::RemoveFromWorld
	*
	*	DESCRIPTION		
	*
	***************************************************************************************************/
	void DummyCharacterController::RemoveFromWorld (hkWorld* pWorld) 
	{
		RemoveCombatPhysicsPushVolumesFromWorld();

		m_world = 0;
	}

		/***************************************************************************************************
	*
	*	FUNCTION		DummyCharacterController::SetupCharacterVolume
	*
	*	DESCRIPTION		Creates and initialises the shapes and rigid bodies.
	*
	***************************************************************************************************/
	#define fPERCENTAGE_HOLDING_SIZE 1.0f
	#define fSIZE_SPHERE_STEP 0.16f
	void DummyCharacterController::SetupCharacterVolume()
	{
		// Only capsules are supported atm
		ntError(m_pobColPrim->m_eType == CV_TYPE_CAPSULE)

		const float fLength = m_pobColPrim->m_obCapsuleData.fLength;
		const float fRadius = m_pobColPrim->m_obCapsuleData.fRadius;

		m_fCapsuleHalfLength = (fLength * 0.5f) + fRadius;

		// Construct a shape
		hkVector4 vertexA(0, (fLength*0.5f), 0);
		hkVector4 vertexB(0, (fLength*-0.5f), 0);		

		// Create a capsule to represent the character standing
		m_standShape = HK_NEW hkCapsuleShape(vertexA, vertexB, fRadius);
		hkVector4 size; size.setSub4( vertexA, vertexB );
		//m_halfSize = (size(1) * 0.5f) + fRadius + 0.10f;

		float fIncrease = fPERCENTAGE_HOLDING_SIZE * fRadius;
		float fHoldRadius = fRadius + fIncrease;

		hkVector4 axis; axis.setSub4( vertexA, vertexB ); // vertexA - vertexB
		axis.normalize3(); axis.mul4(fIncrease);

		vertexB.add4(axis);
		vertexA.sub4(axis);

		// Allow the entity to modify the holding capsule, this code was added so that
		// the archer could use the holding capsule shape as a covering capsule

		{
			CPoint ptA = Physics::MathsTools::hkVectorToCPoint(vertexA);
			CPoint ptB = Physics::MathsTools::hkVectorToCPoint(vertexB);

			m_entity->ModifyPhysicsHoldingShape( ptA, ptB, fHoldRadius);

			vertexA = Physics::MathsTools::CPointTohkVector(ptA);
			vertexB = Physics::MathsTools::CPointTohkVector(ptB);
		}

		m_holdingShape = HK_NEW hkCapsuleShape(vertexA, vertexB, fHoldRadius);		

		m_obEntityCFlag.base = 0;
		if( m_entity->IsPlayer() || m_entity->IsFriendly() )
			m_obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_PLAYER_BIT;
		else 
			m_obEntityCFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_ENEMY_BIT;

		if (m_entity->IsImportant())
			m_obEntityCFlag.flags.i_am_important = 1;

		if( m_entity->IsPlayer() || m_entity->IsFriendly() || ( m_entity->IsAI() && ((AI*)m_entity)->AttackAI() )  )
		{
			m_obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	| 
													Physics::RAGDOLL_BIT						|
													Physics::LARGE_INTERACTABLE_BIT				|		
													Physics::SMALL_INTERACTABLE_BIT				|	
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
		} else {
			m_obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													//Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
													Physics::RAGDOLL_BIT						|
													Physics::LARGE_INTERACTABLE_BIT				|
													Physics::SMALL_INTERACTABLE_BIT				|	
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		DummyCharacterController::Update
	*
	*	DESCRIPTION		Render phantom position.
	*
	***************************************************************************************************/
	void DummyCharacterController::Update(float fTimeDelta, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform)
	{
		m_obPosition = ComputeCharacterPositionFromEntity(&obGoalTranslation);

		if (bUpdateEntityTransform)
		{
			// Set ent matrix
			if (bUpdateEntityTransform)
			{
				m_pobRoot->SetLocalMatrix( CMatrix( obGoalOrientation, obGoalTranslation ) );
			}

			// And update our push volumes
			UpdateCombatPhysicsPushVolumes( fTimeDelta );
		}
	};
}

#endif
