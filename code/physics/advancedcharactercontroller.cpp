//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/advancedcharactercontroller.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.25
//!
//---------------------------------------------------------------------------------------------------------
#include "Physics/system.h"
#include "game/movementstate.h"
#include "game/movement.h"

#include "core/gatso.h"

#include "config.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include "advancedcharactercontroller.h"

#include "collisionbitfield.h"
#include "maths_tools.h"

#include "anim/transform.h"
#include "core/timer.h"

#include "hsCharacterProxy.h"
#include "world.h"

#include "IKFootPlacement.h"

#include <hkdynamics/phantom/hkShapePhantom.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/hkDebugDisplay.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entitymanager.h"

#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

#include "core/osddisplay.h"

// For ragdoll to animated blend
#include "game/attacks.h"
#include "hierarchy_tools.h"

#include "game/inputcomponent.h"

#include "camera/camutils.h"

#include "game/query.h"
#include "game/aicomponent.h"

#include "effect/combateffect.h"
#include "effect/combateffects_trigger.h"

#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkShapeRayCastOutput.h>

#include "game/movementcontrollerinterface.h"

#include "army/armymanager.h"

#include "physics/havokthreadutils.h"

#include "physics/physicsLoader.h"

#define COLLIDE_ALL_FLAG (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| \
							Physics::CHARACTER_CONTROLLER_ENEMY_BIT	| \
							Physics::RAGDOLL_BIT						| \
							Physics::LARGE_INTERACTABLE_BIT				| \
							Physics::TRIGGER_VOLUME_BIT					| \
							Physics::AI_WALL_BIT)

#define COLLIDE_NO_CC (		Physics::RAGDOLL_BIT						| \
							Physics::LARGE_INTERACTABLE_BIT				| \
							Physics::AI_WALL_BIT						| \
							Physics::TRIGGER_VOLUME_BIT					)

#define COLLIDE_NO_ENEMY (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| \
							Physics::RAGDOLL_BIT						| \
							Physics::LARGE_INTERACTABLE_BIT				| \
							Physics::AI_WALL_BIT						| \
							Physics::TRIGGER_VOLUME_BIT					)

// [scee_st] not particularly happy at chunking to MC_ENTITY, but overlaps with attack definitions.
START_CHUNKED_INTERFACE	(CombatPhysicsPushVolumeDescriptor, Mem::MC_ENTITY)
	ISTRING	(CombatPhysicsPushVolumeDescriptor, BindToTransform)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, SphereRadius)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, CapsuleRadius)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, CapsuleLength)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, BoxHalfExtentX)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, BoxHalfExtentY)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, BoxHalfExtentZ)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, XAxisPositionOffset)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, YAxisPositionOffset)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, ZAxisPositionOffset)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, XAxisRotationOffset)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, YAxisRotationOffset)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, ZAxisRotationOffset)
	IBOOL	(CombatPhysicsPushVolumeDescriptor, PushOnce)
	IBOOL	(CombatPhysicsPushVolumeDescriptor, UseMovementOfVolume)
	IBOOL	(CombatPhysicsPushVolumeDescriptor, UseFromCentreOfVolume)
	IBOOL	(CombatPhysicsPushVolumeDescriptor, UseMassMultiplier)
	IBOOL	(CombatPhysicsPushVolumeDescriptor, UseMagnitudeMultiplier)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, PushFactorLargeInteractable)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, PushFactorSmallInteractable)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, PushFactorRagdoll)
	IFLOAT	(CombatPhysicsPushVolumeDescriptor, PushFactorCollapse)
	IBOOL	(CombatPhysicsPushVolumeDescriptor, OnlyActiveWhenAttacking)
	IBOOL	(CombatPhysicsPushVolumeDescriptor, CanCollapseCollapsablesWhenNotAttacking)
	IBOOL	(CombatPhysicsPushVolumeDescriptor, CanCollapseCollapsablesWhenAttacking)
	PUBLISH_VAR_AS(m_bShouldStrikeCharactersWhenAttacking, ShouldStrikeCharactersWhenAttacking)
END_STD_INTERFACE

START_STD_INTERFACE	(CombatPhysicsStrikeVolumeDescriptor)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, SphereRadius)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, CapsuleRadius)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, CapsuleLength)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, BoxHalfExtentX)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, BoxHalfExtentY)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, BoxHalfExtentZ)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, ConcentricWaveSweep)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, ConcentricWaveInnerRadius)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, ConcentricWaveOuterRadius)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, ConcentricWaveExpansionSpeed)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, ConcentricWaveAngle)
	IFLOAT	(CombatPhysicsStrikeVolumeDescriptor, ConcentricWaveHeight)
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bCollapseInteractables, true, CollapseInteractables )
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_fCollapseInteractableStrength, 50.0f, CollapseInteractableStrength )
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bStopAtInteractables, false, StopAtInteractables )
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bRattleInteractables, false, RattleInteractables )
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bDoRaycastCollisionCheckForStrike, false, DoRaycastCollisionCheckForStrike )
	IREFERENCE( CombatPhysicsStrikeVolumeDescriptor, AlternateAttackData )
	IREFERENCE( CombatPhysicsStrikeVolumeDescriptor, MovementDescriptor )
	PUBLISH_VAR_AS(m_obEffectsScript, EffectsScript)
	PUBLISH_VAR_AS(	m_obLoopingSoundCue, LoopingSoundCue )
	PUBLISH_VAR_AS(	m_obLoopingSoundBank, LoopingSoundBank )
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_fLoopingSoundRange, 0.0f, LoopingSoundRange )
	PUBLISH_VAR_AS(	m_obPassbySoundCue, PassbySoundCue )
	PUBLISH_VAR_AS(	m_obPassbySoundBank, PassbySoundBank )
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_fPassbySoundRange, 0.0f, PassbySoundRange )
END_STD_INTERFACE

START_STD_INTERFACE	(CombatPhysicsStrikeVolumeMovementDescriptor)
	PUBLISH_VAR_AS	(m_obStartPosition, StartPosition)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fStartTime, 0.0f, StartTime)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fTimeout, 0.0f, Timeout)
END_STD_INTERFACE

START_STD_INTERFACE	(CombatPhysicsStrikeVolumeStraightMovementDescriptor)
	COPY_INTERFACE_FROM(CombatPhysicsStrikeVolumeMovementDescriptor)
	DEFINE_INTERFACE_INHERITANCE(CombatPhysicsStrikeVolumeMovementDescriptor)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTravelVector, CDirection( CONSTRUCT_CLEAR ), TravelVector)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fAngle, 0.0f, Angle)
	IFLOAT	(CombatPhysicsStrikeVolumeStraightMovementDescriptor, Speed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxHomingRotationSpeed, 0.0f, MaxHomingRotationSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fHomeWithinAngle, 90.0f, HomeWithinAngle)
END_STD_INTERFACE

START_STD_INTERFACE	(CombatPhysicsTransformBasedMovementDescriptor)
	COPY_INTERFACE_FROM(CombatPhysicsStrikeVolumeMovementDescriptor)
	DEFINE_INTERFACE_INHERITANCE(CombatPhysicsStrikeVolumeMovementDescriptor)

	PUBLISH_VAR_AS( m_obTransform, Transform )
	PUBLISH_VAR_AS( m_fSpeed, Speed )
	PUBLISH_VAR_AS( m_fAngleDeltaTrigger, AngleDeltaTrigger )
END_STD_INTERFACE


namespace Physics
{

	const LogicGroup::GROUP_TYPE AdvancedCharacterController::GetType( ) const
	{
		return LogicGroup::ADVANCED_CHARACTER_CONTROLLER;
	}

	void AdvancedCharacterController::Activate( bool activateInHavok)
	{
		// Reset the goal
		m_GoalOrientation = CQuat( m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetEntityWorldMatrix() );
		m_GoalTranslation = m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetEntityWorldMatrix().GetTranslation();
		
		// Assume by default that you want the CC
		ActivateCharacterController();

		// Make sure the CC is correctly set
		EntityRootTransformHasChanged();

		//m_headIK = NT_NEW IKLookAt(m_entity->GetHierarchy(), m_entity );
		if (!m_footIK && m_entity->IsPlayer())
			m_footIK = NT_NEW_CHUNK ( MC_PHYSICS ) Physics::IKFootPlacement( m_entity->GetHierarchy() );
	}

	void AdvancedCharacterController::Deactivate( )
	{
		DeactivateCharacterController();
		if (m_advRagdoll)
		DeactivateRagdoll();
	}

	bool AdvancedCharacterController::IsActive()
	{
		return ( IsCharacterControllerActive() || IsRagdollActive() );
	}

	void AdvancedCharacterController::AddedToSystem( System* sys )						   
	{
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->RegisterSystem( sys );
	}

	void AdvancedCharacterController::RemovedFromSystem( System* sys)
	{}

	RigidBody* AdvancedCharacterController::AddRigidBody( const BodyCInfo* p_info )
	{
		ntAssert(0);
		return 0;
	}

	RigidBody* AdvancedCharacterController::AddRigidBody( RigidBody* body )	
	{
		ntAssert(0);
		return 0;
	}

	void AdvancedCharacterController::UpdateCollisionFilter( )
	{
		if (m_advRagdoll && m_advRagdoll->IsActive())
			m_advRagdoll->UpdateCollisionFilter();
		else
			m_apobCharacterControllers[m_eCurrentCharacterControllerType]->UpdateCollisionFilter();
	}

	void AdvancedCharacterController::EntityRootTransformHasChanged()
	{
		// Make sure the physics representation is currently where it says it is
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->SetPosition( m_apobCharacterControllers[m_eCurrentCharacterControllerType]->ComputeCharacterPositionFromEntity() );
		
		// Reset the goals.
		m_GoalOrientation = CQuat( m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetEntityWorldMatrix() );
		m_GoalTranslation = m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetEntityWorldMatrix().GetTranslation();

		if (m_advRagdoll && m_advRagdoll->IsActive())
			m_advRagdoll->ResetEntireRagdoll(true,true, true);
	}

	void AdvancedCharacterController::Pause( bool bPause )
	{ }

	CDirection AdvancedCharacterController::GetLinearVelocity( )
	{
		if( m_advRagdoll && m_advRagdoll->IsActive() )
			return m_advRagdoll->GetLinearVelocity();
		else
			return Physics::MathsTools::hkVectorToCDirection(m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetLinearVelocity());
	}

	CDirection AdvancedCharacterController::GetAngularVelocity( )
	{
		if (m_advRagdoll && m_advRagdoll->IsActive())
			return m_advRagdoll->GetAngularVelocity();
		else
			return CDirection(0.0,0.0,0.0);
	}

	void AdvancedCharacterController::SetLinearVelocity( const CDirection& p_linearVelocity )
	{
		if (m_advRagdoll && m_advRagdoll->IsActive())
			m_advRagdoll->SetLinearVelocity( p_linearVelocity );
		else
			m_apobCharacterControllers[m_eCurrentCharacterControllerType]->SetLinearVelocity(Physics::MathsTools::CDirectionTohkVector(p_linearVelocity));
	}

	void AdvancedCharacterController::SetAngularVelocity( const CDirection& p_angularVelocity )
	{
		if (m_advRagdoll && m_advRagdoll->IsActive())
			m_advRagdoll->SetAngularVelocity( p_angularVelocity );
		else
			return;
	}

	void AdvancedCharacterController::RagdollApplyLinearImpulse( const CDirection& p_linearVelocity )
	{
		if (m_advRagdoll && m_advRagdoll->IsActive())
			m_advRagdoll->ApplyLinearImpulse( p_linearVelocity );
	}

	void AdvancedCharacterController::RagdollApplyAngularImpulse( const CDirection& p_angularVelocity )
	{
		if (m_advRagdoll && m_advRagdoll->IsActive())
			m_advRagdoll->ApplyAngularImpulse( p_angularVelocity );
	}

	AdvancedCharacterController::AdvancedCharacterController( Character* p_entity, CColprimDesc* p_CharacterColPrim, ntstd::String sRagdollClump ):
		LogicGroup( p_entity->GetName(), p_entity ),
		m_entity( p_entity )
	{
		onGroundTimeOut = 0.0f;
		m_GoalOrientation = CQuat( CONSTRUCT_IDENTITY );
		m_GoalTranslation = CPoint( CONSTRUCT_CLEAR );

		m_hierarchy = m_entity->GetHierarchy();

		// 1 - Construct the character controllers
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c] = CharacterController::Construct( m_entity, p_CharacterColPrim, (CharacterController::CharacterControllerType)c );
		}
		// The type we're gonna start off with until something switches us
		if ( m_entity->IsCharacter() && m_entity->ToCharacter()->GetCanUseCheapCC() )
			m_eCurrentCharacterControllerType = CharacterController::RIGID_BODY;
		else
			m_eCurrentCharacterControllerType = CharacterController::FULL_CONTROLLER;

		// 2 - Construct the ragdoll
		
		if (sRagdollClump != "NULL")
		{
		const char* clumpString = ntStr::GetString(sRagdollClump);
		ntError( ntStr::GetLength(clumpString) > 0 && ntStr::GetLength(clumpString) < MAX_PATH );
			
		ntstd::String newPSName = CPhysicsLoader::AlterFilenameExtension( clumpString);

		m_advRagdoll = NT_NEW_CHUNK ( MC_PHYSICS ) AdvancedRagdoll( m_entity, newPSName.c_str() );
		ntError_p( m_advRagdoll != NULL, ("Couldn't load ragdoll %s - but should have at least found default ragdoll!", newPSName.c_str()) );
		}
		else
		{
			// Not every character must to have a ragdoll... For example Demon. 
			// Demon does not have character like hierarchy what leads to crashes... 

			m_advRagdoll = NULL;
		}

		m_PreviousGravityFactor = 0.0f;
		m_PelvisSpring = 1.0f;

		m_Synchronised = WS_SYNC_NONE;

		m_bBlendingFromRagdollToAnimation = false;
		m_fRagdollToHierarchyBlendAmount = -1.0f;

		m_bCharacterControllerWasActiveLastFrame = m_bRagdollWasActiveLastFrame = false;
		m_obCharacterControllerPhantomPositionLastFrame = Physics::MathsTools::hkVectorToCPoint(m_apobCharacterControllers[m_eCurrentCharacterControllerType]->ComputeCharacterPositionFromEntity());

		m_footIK = 0;
		m_bFootIKEnabled = true;
	}

	AdvancedCharacterController::~AdvancedCharacterController()
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			NT_DELETE( m_apobCharacterControllers[c] );
		}

		NT_DELETE_CHUNK( MC_PHYSICS, m_advRagdoll );
		NT_DELETE_CHUNK( MC_PHYSICS, m_footIK);
	}

	void AdvancedCharacterController::SetCharacterControllerCollidable( bool is )
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetCollidable( is );
		}

		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->SetCollidable( is );
	}

	void AdvancedCharacterController::SetCharacterControllerRagdollCollidable( bool is )
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetRagdollCollidable( is );
		}

		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->SetRagdollCollidable( is );
	}

	void AdvancedCharacterController::SetWorldSpaceSynchronised( WS_SYNC_TYPE status )
	{
		if( m_Synchronised == WS_SYNC_NONE )
		{
			if( status != WS_SYNC_NONE )
			{
				m_synchronisedReference = m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetEntityPosition();
				m_computedLocalReference = m_synchronisedReference;
			}
		} 

		m_Synchronised = status;
	}

	void AdvancedCharacterController::SetCharacterControllerDynamicCollidable( bool is )
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetDynamicCollidable( is );
		}

		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->SetDynamicCollidable( is );
	}

	void AdvancedCharacterController::SetCharacterControllerSetFilterExceptionFlags( unsigned int uiFlags )
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetFilterExceptionFlags( uiFlags );
		}
	}

	void AdvancedCharacterController::SetCharacterControllerHoldingCapsule( bool is )
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetHoldingCapsule( is );
		}
	}

	void AdvancedCharacterController::GetRootWorldMatrix(CMatrix& obMatrix)
	{
		CMatrix obRet(CONSTRUCT_IDENTITY);
		if (m_apobCharacterControllers[m_eCurrentCharacterControllerType]->IsActive())
			m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetEntityWorldMatrix( obRet );
		else if (m_advRagdoll && m_advRagdoll->IsActive())
			obRet = m_advRagdoll->GetIdealEntityWorldMatrix();
		obMatrix = obRet;
	}

	void AdvancedCharacterController::SetMoveTo( const CPoint& obPosition )		
	{ 
		m_GoalTranslation = obPosition;
	}

	void AdvancedCharacterController::SetTurnTo( const CQuat& obOrientation )	
	{ 
		m_GoalOrientation = obOrientation; 
	}

	bool AdvancedCharacterController::IsOnGround()
	{
		if( m_advRagdoll && m_advRagdoll->IsActive() )
		{
			if (m_advRagdoll->GetState() == DEAD)
				return !m_advRagdoll->IsMoving();
			else
				return m_advRagdoll->IsInContactWithGround();
		} 
		else 
		{
			return m_apobCharacterControllers[m_eCurrentCharacterControllerType]->IsCharacterControllerOnGround();
		}
	}

	bool AdvancedCharacterController::IsCharacterControllerActive()
	{
		return m_apobCharacterControllers[m_eCurrentCharacterControllerType]->IsActive();
	}

	void AdvancedCharacterController::ActivateCharacterController()
	{
		if (m_advRagdoll && m_advRagdoll->IsActive())
		{
			CPoint obRagdollPosition = m_advRagdoll->GetPosition();
			m_apobCharacterControllers[m_eCurrentCharacterControllerType]->SetPositionOnNextActivation(obRagdollPosition);
			DeactivateRagdoll();
		}
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->Activate();
	}

	void AdvancedCharacterController::DeactivateCharacterController()
	{
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->Deactivate();
	}

	void AdvancedCharacterController::SetRagdollAnimated(int iBoneFlags, bool bAnimateFromRagdollBones)
	{
		ntError_p(m_advRagdoll, ("Character cannot go to ragdoll state. It doesn't have any ragdoll."));
		m_advRagdoll->SetAnimatedBones(iBoneFlags);
		m_advRagdoll->Activate( ANIMATED );
		DeactivateCharacterController();
	}

	void AdvancedCharacterController::SetRagdollAnimatedAbsolute(int iBoneFlags, bool bAnimateFromRagdollBones)
	{
		ntError_p(m_advRagdoll, ("Character cannot go to ragdoll state. It doesn't have any ragdoll."));
		m_advRagdoll->SetAnimatedBones(iBoneFlags);
		m_advRagdoll->Activate( ANIMATED_ABSOLUTE );
		DeactivateCharacterController();
	}

	void AdvancedCharacterController::SetRagdollTransformTracking()
	{
		ntError_p(m_advRagdoll, ("Character cannot go to ragdoll state. It doesn't have any ragdoll."));
		m_advRagdoll->Activate(TRANSFORM_TRACKING);
		DeactivateCharacterController();
	}

	void AdvancedCharacterController::SetRagdollBoneTransformTrackingMapping(int iBoneFlag, Transform* pobTransform)
	{
		ntError_p(m_advRagdoll, ("Character cannot go to ragdoll state. It doesn't have any ragdoll."));
		m_advRagdoll->SetBoneTransformTrackingMapping(iBoneFlag,pobTransform);
	}

	void AdvancedCharacterController::SetRagdollTransformTrackingAnimated()
	{
		ntError_p(m_advRagdoll, ("Character cannot go to ragdoll state. It doesn't have any ragdoll."));
		m_advRagdoll->Activate(TRANSFORM_TRACKING_ANIMATED);
		DeactivateCharacterController();
	}

	void AdvancedCharacterController::SetRagdollDead()
	{
		ntError_p(m_advRagdoll, ("Character cannot go to ragdoll state. It doesn't have any ragdoll."));
		m_advRagdoll->Activate( DEAD );
		DeactivateCharacterController();
	}

	void AdvancedCharacterController::SetRagdollDisabled()
	{
		DeactivateRagdoll();
	}

	void AdvancedCharacterController::SetRagdollTurnDynamicOnContact(bool bDynamic)
	{
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		m_advRagdoll->SetTurnDynamicOnContact(bDynamic);
	}

	bool AdvancedCharacterController::GetRagdollTurnDynamicOnContact()
	{
		return m_advRagdoll && m_advRagdoll->GetTurnDynamicOnContact();
	} 

	void AdvancedCharacterController::SetRagdollTrajectory( float fDistanceMultiplier, float fAngleMultiplier )
	{
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		m_advRagdoll->SetRagdollTrajectory( fDistanceMultiplier, fAngleMultiplier );
	}
	
	void AdvancedCharacterController::Update( const float fTimeDelta )
	{        
		GATSO_PHYSICS_START("AdvancedCharacterController::Update");

		ntAssert(m_entity->ToCharacter());

		if ( IsCharacterControllerActive() )
		{
			/*if (CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_S, KEYM_SHIFT ) )
			{
				SwitchCharacterControllerType((Physics::CharacterController::CharacterControllerType) ((m_eCurrentCharacterControllerType+1)%Physics::CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT) );
			}*/

			m_apobCharacterControllers[m_eCurrentCharacterControllerType]->Update( CTimer::Get().GetGameTimeChange() * m_entity->GetTimeMultiplier(), m_GoalTranslation, m_GoalOrientation, m_Synchronised, true);
			m_obCharacterControllerPhantomPositionLastFrame = Physics::MathsTools::hkVectorToCPoint(m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetPosition());
			m_bCharacterControllerWasActiveLastFrame = true;
			m_bRagdollWasActiveLastFrame = false;

			// footIK only for player
			if (m_footIK && m_entity->IsPlayer())
			{			
				m_footIK->Update( m_bFootIKEnabled, fTimeDelta);
			}
		} 

		if( IsRagdollActive() )
		{
			ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
			m_advRagdoll->Update( CTimer::Get().GetGameTimeChange() * m_entity->GetTimeMultiplier(), m_GoalTranslation, m_GoalOrientation, m_Synchronised, true);
			m_bCharacterControllerWasActiveLastFrame = false;
			m_bRagdollWasActiveLastFrame = true;
		}

		if (m_bBlendingFromRagdollToAnimation)
		{
			// Blend it in gradually
			float fTimeToBlendFromRagdoll = m_entity->GetAttackComponent()->GetAttackMovementBlendInTimes()->m_fFlooredBlendInTime;
			m_fTimeSpentBlendingFromRagdoll += fTimeDelta;			
			m_fRagdollToHierarchyBlendAmount = CMaths::SmoothStep(m_fTimeSpentBlendingFromRagdoll/fTimeToBlendFromRagdoll);
			m_fRagdollToHierarchyBlendAmount = ntstd::Clamp(m_fRagdollToHierarchyBlendAmount,0.0f, 1.0f);

			if (m_fTimeSpentBlendingFromRagdoll >= fTimeToBlendFromRagdoll || m_entity->GetAttackComponent()->AI_Access_GetState() != CS_FLOORED)
			{
				// We need to stop now
				m_bBlendingFromRagdollToAnimation = false;
				return;
			}

			// Get current pose after animation has done it's bit
			hkArray<hkQsTransform> aobCurrentAnimatedPose(m_aobRagdollBlendFromHierarchy.getSize());
			Physics::HierarchyTools::GetLocalPoseRecursive(aobCurrentAnimatedPose, m_entity->GetHierarchy(), m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild());

			// Blend each transform of our current pose with the pose we had at the start
			hkArray<hkQsTransform> aobNewBlendedAnimatedPose(m_aobRagdollBlendFromHierarchy.getSize());
			for (int i = 0; i < aobNewBlendedAnimatedPose.getSize(); i++)
			{
				hkVector4 obBlendedTranslation(0,0,0,0);
				hkQuaternion obBlendedRotation(0,0,0,1);
				hkVector4 obFromTranslation = m_aobRagdollBlendFromHierarchy[i].m_translation;
				hkVector4 obToTranslation = aobCurrentAnimatedPose[i].m_translation;
				hkQuaternion obFromRotation = m_aobRagdollBlendFromHierarchy[i].m_rotation;
				hkQuaternion obToRotation = aobCurrentAnimatedPose[i].m_rotation;
				
				obBlendedTranslation.setInterpolate4(obFromTranslation,obToTranslation,m_fRagdollToHierarchyBlendAmount);
				obBlendedRotation.setSlerp(obFromRotation,obToRotation,m_fRagdollToHierarchyBlendAmount);

				hkQsTransform obBlendedTransform(obBlendedTranslation,obBlendedRotation);
				aobNewBlendedAnimatedPose[i] = obBlendedTransform;
			}

			// Overwrite the hierarchy with our new blended goodness
			HierarchyTools::SetLocalPoseRecursive(aobNewBlendedAnimatedPose, m_entity->GetHierarchy(), m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild(), 0, false);
			// Blend the root between the ragdolls weird twisted one and the animators nice one
			CPoint obNewTranslation = CPoint::Lerp(m_obRagdollBlendFromRoot.GetTranslation(),m_entity->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation(),m_fRagdollToHierarchyBlendAmount);
			CQuat obNewRotation = CQuat::Slerp(CQuat(m_obRagdollBlendFromRoot), CQuat(m_entity->GetHierarchy()->GetRootTransform()->GetWorldMatrix()),m_fRagdollToHierarchyBlendAmount);
			CMatrix obNewRoot( obNewRotation, obNewTranslation );
			m_entity->GetHierarchy()->GetRootTransform()->SetLocalMatrixFromWorldMatrix(obNewRoot);
		}

		GATSO_PHYSICS_STOP("AdvancedCharacterController::Update");
	}

	void AdvancedCharacterController::Debug_RenderCollisionInfo ()
	{
#ifndef _RELEASE
		if (m_advRagdoll && m_advRagdoll->IsActive())
		{
			m_advRagdoll->Debug_RenderCollisionInfo();
		}
		else
		{
			m_advRagdoll->Debug_RenderCollisionInfo_Animated();
			m_apobCharacterControllers[m_eCurrentCharacterControllerType]->Debug_RenderCollisionInfo();
		}
#endif // _RELEASE
	}

	RAGDOLL_STATE AdvancedCharacterController::GetRagdollState() 
	{ 
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		return m_advRagdoll->GetState(); 
	}

	bool AdvancedCharacterController::IsRagdollActive() 
	{ 
		return m_advRagdoll && m_advRagdoll->IsActive(); 
	}

	CDirection AdvancedCharacterController::GetRagdollLinearVelocity() 
	{ 
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		return m_advRagdoll->GetLinearVelocity(); 
	}

	void AdvancedCharacterController::RagdollBodyApplyImpulse(int iHitArea, const CPoint& position, const CDirection& impulse)
	{
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));

		return m_advRagdoll->ApplyImpulseToBody(iHitArea, position, impulse);
	}

	void AdvancedCharacterController::SetApplyCharacterControllerGravity(bool bGravity)
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetApplyGravity(bGravity);
		}
	}

	bool AdvancedCharacterController::GetApplyCharacterControllerGravity()
	{
		return m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetApplyGravity();
	}

	void AdvancedCharacterController::SetDoCharacterControllerMovementAbsolutely(bool bAbsolute)
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetDoMovementAbsolutely(bAbsolute);
		}
	}

	bool AdvancedCharacterController::GetDoCharacterControllerMovementAbsolutely()
	{
		return m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetDoMovementAbsolutely();
	}

	void AdvancedCharacterController::SetRagdollLinearVelocity(CDirection& obVel) 
	{ 
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		m_advRagdoll->SetLinearVelocity(obVel); 
	}

	void AdvancedCharacterController::AddRagdollLinearVelocity(CDirection& obVel) 
	{ 
		m_advRagdoll->AddLinearVelocity(obVel); 
	}

	void AdvancedCharacterController::DeactivateRagdoll() 
	{ 
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		m_advRagdoll->Deactivate(); 
	}

	void AdvancedCharacterController::SetSendRagdollMessageOnAtRest(const char* pcMsg)
	{
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		m_advRagdoll->SetSendMessageOnAtRest(pcMsg);
	}

	void AdvancedCharacterController::SetRagdollAnimatedBones(int iBoneFlags)
	{
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		m_advRagdoll->SetAnimatedBones(iBoneFlags);
	}
	
	void AdvancedCharacterController::AddRagdollAnimatedBone(int iBoneFlag)
	{
		m_advRagdoll->AddAnimatedBone(iBoneFlag);
	}

	void AdvancedCharacterController::SetRagdollExemptFromCleanup(bool bExemption)
	{
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		m_advRagdoll->SetExemptFromCleanup(bExemption);
	}

	void AdvancedCharacterController::StartBlendFromRagdollToAnimation(hkArray<hkQsTransform>* paobTransforms, CMatrix& obRoot)
	{
		m_bBlendingFromRagdollToAnimation = true;
		m_fTimeSpentBlendingFromRagdoll = 0.0f;
		m_aobRagdollBlendFromHierarchy = *paobTransforms; // Make a copy
		m_obRagdollBlendFromRoot = obRoot;
		m_fRagdollToHierarchyBlendAmount = 0.0f; // Start fully ragdoll
	}

	void AdvancedCharacterController::RagdollTwitch()
	{
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		m_advRagdoll->Twitch();
	}

	bool AdvancedCharacterController::GetCharacterControllerWasActiveLastFrame()
	{
		return m_bCharacterControllerWasActiveLastFrame;
	}

	CPoint AdvancedCharacterController::GetCharacterControllerPhantomPositionLastFrame()
	{
		return m_obCharacterControllerPhantomPositionLastFrame;
	}

	void AdvancedCharacterController::SetRagdollAntiGravity( bool bAntiGrav )
	{
		ntError_p(m_advRagdoll, ("Character cannot use function for ragdolls. It doesn't have any ragdoll."));
		if (m_advRagdoll && m_advRagdoll->IsActive())
			m_advRagdoll->SetAntiGravity(bAntiGrav);
	}

	void AdvancedCharacterController::SetCharacterCombatPhysicsPushVolumeDescriptors( PushVolumeDescriptorList* pobCombatPhysicsPushVolumeDescriptors,
																					  PushVolumeDataList* pobCombatPhysicsPushVolumeData)
	{
		ntAssert(!pobCombatPhysicsPushVolumeDescriptors || pobCombatPhysicsPushVolumeDescriptors->size() == pobCombatPhysicsPushVolumeData->size());

		// Update this on all our character controllers, they all need to know about it in case we switch
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetCombatPhysicsPushVolumeDescriptors(pobCombatPhysicsPushVolumeDescriptors, pobCombatPhysicsPushVolumeData);
		}
	}

	void AdvancedCharacterController::CleanupCharacterCombatPhysicsPushVolumeDescriptors( PushVolumeDataList* pobCombatPhysicsPushVolumeDescriptors)
	{
		// The clean up can be done by any one of the character controllers, it's just a havok thing that I wanted to pass down responsibility for
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->CleanupCombatPhysicsPushVolumeDescriptors(pobCombatPhysicsPushVolumeDescriptors);
	}

	void AdvancedCharacterController::SwitchCharacterControllerType(CharacterController::CharacterControllerType eTo)
	{
		if (eTo == m_eCurrentCharacterControllerType)
			return;

		// Save the position and velocity of the current controller
		hkVector4 obPosition = m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetPosition();
		hkVector4 obVelocity = m_apobCharacterControllers[m_eCurrentCharacterControllerType]->GetLinearVelocity();
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->Deactivate();
		m_eCurrentCharacterControllerType = eTo;
		// Switch the new one on and carry over position and velocity
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->Activate();
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->SetPosition(obPosition);
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->SetLinearVelocity(obVelocity);
	}

	CharacterController::CharacterControllerType AdvancedCharacterController::GetCharacterControllerType()
	{
		return m_eCurrentCharacterControllerType;
	}

	void AdvancedCharacterController::SetKOState(KO_States state)
	{
		// set KO state to controllers
		if (m_advRagdoll)
		m_advRagdoll->SetKOState(state);

		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetKOState( state );
		}	
	}

	// SOFT PARENTING
	void AdvancedCharacterController::SetSoftParent(CEntity * parent)
	{
		for (int c = 0; c < CharacterController::CHARACTER_CONTROLLER_TYPE_COUNT; c++)
		{
			m_apobCharacterControllers[c]->SetSoftParent( parent );
		}
	}

	bool AdvancedCharacterController::IsSoftParented() const
	{
		return  m_apobCharacterControllers[m_eCurrentCharacterControllerType]->IsSoftParented();
	}

	void AdvancedCharacterController::PausePresenceInHavokWorld(bool pause)
	{
		m_apobCharacterControllers[m_eCurrentCharacterControllerType]->PausePresenceInHavokWorld(pause);
		if (m_advRagdoll)
		m_advRagdoll->PausePresenceInHavokWorld(pause);
	}

	bool AdvancedCharacterController::CastRayOnRagdoll(const CPoint &obSrc, const CPoint &obTarg, Physics::RaycastCollisionFlag obFlag, TRACE_LINE_QUERY& stQuery)
	{
		ntAssert(m_advRagdoll && !m_advRagdoll->IsActive());
		return m_advRagdoll->CastRayOnAnimated(obSrc, obTarg, obFlag, stQuery);
	}
}

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeMovementDescriptor
*
*	DESCRIPTION	Pseudo-abstract base class to describe combat strike volume movements
*
***************************************************************************************************/
CombatPhysicsStrikeVolumeMovementDescriptor::CombatPhysicsStrikeVolumeMovementDescriptor()
{
	m_obStartPosition = CPoint( 0.0f, 0.0f, 0.0f );
	m_fStartTime = 0.0f;
	m_fTimeout = 5.0f;

	m_fXAxisRotationOffset = m_fYAxisRotationOffset = m_fZAxisRotationOffset = 0.0f;
}

CombatPhysicsStrikeVolumeMovementDescriptor::~CombatPhysicsStrikeVolumeMovementDescriptor()
{
}

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeMovement
*
*	DESCRIPTION	Pseudo-abstract base class to execute combat strike volume movements
*
***************************************************************************************************/
CombatPhysicsStrikeVolumeMovement::CombatPhysicsStrikeVolumeMovement(CombatPhysicsStrikeVolumeMovementDescriptor* pobDesc)
{
	m_bDeleteDescriptor = false;
	m_pobDescriptor = pobDesc;
}

CombatPhysicsStrikeVolumeMovement::~CombatPhysicsStrikeVolumeMovement()
{
	if (m_bDeleteDescriptor)
		NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobDescriptor);
}

CPoint CombatPhysicsStrikeVolumeMovement::GetNextPosition(const CEntity* pobOwner, const CEntity* pobTarget, CPoint& obCurrentPosition, float fTimeDelta) 
{ 
	// Shouldn't ever use this base class
	ntAssert(0);

	UNUSED( fTimeDelta ); 
	return obCurrentPosition; 
}

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeStraightMovementDescriptor
*
*	DESCRIPTION	Concrete class to describe straight combat strike volume movements
*
***************************************************************************************************/
CombatPhysicsStrikeVolumeStraightMovementDescriptor::CombatPhysicsStrikeVolumeStraightMovementDescriptor()
: CombatPhysicsStrikeVolumeMovementDescriptor()
{
	m_fSpeed = 0.0f;
}

CombatPhysicsStrikeVolumeStraightMovementDescriptor::~CombatPhysicsStrikeVolumeStraightMovementDescriptor()
{
}

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeStraightMovement
*
*	DESCRIPTION	Concrete class to execute straight combat strike volume movements
*
***************************************************************************************************/
CombatPhysicsStrikeVolumeStraightMovement::CombatPhysicsStrikeVolumeStraightMovement(CombatPhysicsStrikeVolumeStraightMovementDescriptor* pobDesc)
: CombatPhysicsStrikeVolumeMovement(pobDesc)
{
}

CombatPhysicsStrikeVolumeStraightMovement::~CombatPhysicsStrikeVolumeStraightMovement()
{
}

CPoint CombatPhysicsStrikeVolumeStraightMovement::GetNextPosition(const CEntity* pobOwner, const CEntity* pobTarget, CPoint& obCurrentPosition, float fTimeDelta) 
{ 	
	CombatPhysicsStrikeVolumeStraightMovementDescriptor* pobDesc = (CombatPhysicsStrikeVolumeStraightMovementDescriptor*)m_pobDescriptor;
	
	// Do some homing in on a target?
	if (pobTarget && pobDesc->m_fMaxHomingRotationSpeed > 0.0f)
	{
        CDirection obTargetVector( pobTarget->GetPosition() - obCurrentPosition );
		obTargetVector.Y() = 0.0f;
		obTargetVector.Normalise();

		float fAngle = MovementControllerUtilities::RotationAboutY( m_obTravelVector, obTargetVector );
		if ( fabs(fAngle) < pobDesc->m_fHomeWithinAngle * DEG_TO_RAD_VALUE )
		{
			if (fabs(fAngle) > pobDesc->m_fMaxHomingRotationSpeed * DEG_TO_RAD_VALUE * fTimeDelta)
			{
				fAngle < 0 ? fAngle = -pobDesc->m_fMaxHomingRotationSpeed * fTimeDelta : fAngle = pobDesc->m_fMaxHomingRotationSpeed * fTimeDelta;
			}

			CMatrix obRot( CONSTRUCT_IDENTITY );
			obRot.SetFromAxisAndAngle(CDirection( 0.0f, 1.0f, 0.0f ), fAngle );
			m_obTravelVector = m_obTravelVector * obRot;
		}
	}

	return obCurrentPosition + ( (m_obTravelVector * pobDesc->m_fSpeed) * fTimeDelta );
}

hkTransform CombatPhysicsStrikeVolumeStraightMovement::GetInitialTransform(const CEntity* pobOwner) 
{ 
	CombatPhysicsStrikeVolumeStraightMovementDescriptor* pobDesc = (CombatPhysicsStrikeVolumeStraightMovementDescriptor*)m_pobDescriptor;
	
	// Setup inital transform
	hkTransform obTransform;
	CMatrix obOwnerSpace( pobOwner->GetMatrix() );
	CPoint obStartPositionOwnerSpace = pobDesc->m_obStartPosition * obOwnerSpace;
	hkVector4 obTranslation(obStartPositionOwnerSpace.X(),obStartPositionOwnerSpace.Y(),obStartPositionOwnerSpace.Z());
	obTransform.setTranslation(obTranslation);
	
	if (pobDesc->m_obTravelVector.LengthSquared() < 0.01f)
	{
		CMatrix obRot( CONSTRUCT_IDENTITY );
		CMatrix obComp( CONSTRUCT_IDENTITY );
		obOwnerSpace.SetTranslation( CPoint( CONSTRUCT_CLEAR ) );
		CCamUtil::MatrixFromEuler_XYZ(obRot, (pobDesc->m_fXAxisRotationOffset * DEG_TO_RAD_VALUE), (pobDesc->m_fYAxisRotationOffset * DEG_TO_RAD_VALUE) + (pobDesc->m_fAngle * DEG_TO_RAD_VALUE), (pobDesc->m_fZAxisRotationOffset * DEG_TO_RAD_VALUE) );
		obComp = obRot * obOwnerSpace;	
		obTransform.setRotation( Physics::MathsTools::CQuatTohkQuaternion( CQuat( obComp ) ) );

		// And calculate path of volume
		m_obTravelVector = pobOwner->GetMatrix().GetZAxis() * obRot;
		m_obTravelVector.Normalise();
	}
	else
	{
		obTransform.setRotation( Physics::MathsTools::CQuatTohkQuaternion( CQuat( pobOwner->GetMatrix() ) ) );
		m_obTravelVector = pobDesc->m_obTravelVector;
		m_obTravelVector.Normalise();
	}	

	return obTransform;
}

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumeDescriptor
*
*	DESCRIPTION	Class to describe all aspects of a combat strike volume
*
***************************************************************************************************/
CombatPhysicsStrikeVolumeDescriptor::CombatPhysicsStrikeVolumeDescriptor()
{
	m_pobAlternateAttackData = 0;
	m_pobLastStrike = 0;
	m_pobMovementDescriptor = 0;
	m_fConcentricWaveExpansionSpeed = m_fConcentricWaveAngle = m_fConcentricWaveHeight = m_fConcentricWaveInnerRadius = m_fConcentricWaveOuterRadius = m_fConcentricWaveSweep = m_fSphereRadius = m_fCapsuleRadius = m_fCapsuleLength = m_fBoxHalfExtentX = m_fBoxHalfExtentY = m_fBoxHalfExtentZ = 0.0f;
}

CombatPhysicsStrikeVolumeDescriptor::~CombatPhysicsStrikeVolumeDescriptor()
{
}

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolumePhantomOverlapListener
*
*	DESCRIPTION	Class to listen for target hits in combat strike volumes
*
***************************************************************************************************/
CombatPhysicsStrikeVolumePhantomOverlapListener::CombatPhysicsStrikeVolumePhantomOverlapListener( hkSimpleShapePhantom* pobPhantom )
{
	m_pobPhantom = pobPhantom;
};

void CombatPhysicsStrikeVolumePhantomOverlapListener::collidableAddedCallback( const hkCollidableAddedEvent& event )
{
	hkPhantom* pobPhantom = hkGetPhantom(event.m_collidable);
	hkRigidBody* pobRB = hkGetRigidBody(event.m_collidable);

	CEntity* pobEntity = 0;
	hkTransform obOtherTransform;
	if (pobPhantom && pobPhantom->getType() == HK_PHANTOM_SIMPLE_SHAPE)
	{
		hkSimpleShapePhantom* pobOtherPhantom = (hkSimpleShapePhantom*)pobPhantom;

		pobEntity = (CEntity*)pobOtherPhantom->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
		obOtherTransform = pobOtherPhantom->getTransform();
	}
	else if (pobRB)
	{
		pobEntity = (CEntity*)pobRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr();
		obOtherTransform = pobRB->getTransform();
	}

	if (pobEntity)
	{
		// Need to do collision check on the shape because this is called everytime something enters an island, not collides with the shape
		// Do 2 shape raycasts in local space, that's good enough to decide if the entity's phantom is in ours
		CPoint obPoint = Physics::MathsTools::hkVectorToCPoint(obOtherTransform.getTranslation()); // The point we want to check is the position of the other phantom

        if (pobEntity->GetAttackComponent() && 
			pobEntity->GetAttackComponent()->AI_Access_GetState() != CS_FLOORED && 
			pobEntity->GetAttackComponent()->AI_Access_GetState() != CS_RISE_WAIT && 
			pobEntity->GetAttackComponent()->AI_Access_GetState() != CS_KO &&
			pobEntity->GetAttackComponent()->AI_Access_GetState() != CS_RECOVERING &&
			!EntityIsInStrikeList(pobEntity) )
		{
			
			hkTransform obTransform = m_pobPhantom->getTransform();
			hkAabb obShapeAabb;
			m_pobPhantom->getCollidable()->getShape()->getAabb(obTransform,0.001f,obShapeAabb);
			
			// Find largest extent in Y
			float fExtent = 0.0f;
			if (abs(obShapeAabb.m_min(1)) > abs(obShapeAabb.m_max(1)))
				fExtent = abs(obShapeAabb.m_min(1));
			else
				fExtent = abs(obShapeAabb.m_max(1));
			fExtent *= 1.1f; // Make it a bit longer just incase
				
			// Make an up vector from it
			CDirection obRayVector(0.0,1.0,0.0);
			obRayVector *= fExtent; // Lengthen it so its long enough to be outside the shape
			
			// Project our point along it
			CPoint obFrom(obPoint + obRayVector);

			// Into local space of the shape
			obFrom -= Physics::MathsTools::hkVectorToCPoint(obTransform.getTranslation());
			obPoint -= Physics::MathsTools::hkVectorToCPoint(obTransform.getTranslation());
			
			// Cast a ray from this projected point to the point
			hkShapeRayCastInput obInput;
			hkShapeRayCastOutput obOutput;
			obInput.m_from.set(obFrom.X(),obFrom.Y(),obFrom.Z());
			obInput.m_to.set(obPoint.X(),obPoint.Y(),obPoint.Z());
			m_pobPhantom->getCollidable()->getShape()->castRay(obInput,obOutput);
			
			// If it's hit then we need to do it again from the opposite direction and check for a hit
			if (obOutput.hasHit())
			{
				// Reset ray stuff
				obOutput.reset();
				
				// Reflect vector
				obRayVector *= -1;
				
				// Project point along it
				obFrom = obPoint + obRayVector;
				
				// Cast a ray from this projected point to the point
				obInput.m_from.set(obFrom.X(),obFrom.Y(),obFrom.Z());
				obInput.m_to.set(obPoint.X(),obPoint.Y(),obPoint.Z());
				m_pobPhantom->getCollidable()->getShape()->castRay(obInput,obOutput);
				
				// If it's hit then the point must be within the volume
				if (obOutput.hasHit())
					m_obEntitesToStrike.push_back(pobEntity);
			}
		}
		else if (pobEntity->GetEntType() == CEntity::EntType_Interactable && !EntityIsInInteractableList(pobEntity))
		{
			// We've hit an interactable, safe to assume they have a rigid body representation
			// Don't want to get hit by dynamic debris from collapsed objects so be sure they're fixed/keyed
			if (pobRB && pobRB->isFixedOrKeyframed())
			{
				hkTransform obTransform = m_pobPhantom->getTransform();
				hkAabb obShapeAabb;
				m_pobPhantom->getCollidable()->getShape()->getAabb(obTransform,0.001f,obShapeAabb);
				
				// Find largest extent in Y
				float fExtent = 0.0f;
				if (abs(obShapeAabb.m_min(1)) > abs(obShapeAabb.m_max(1)))
					fExtent = abs(obShapeAabb.m_min(1));
				else
					fExtent = abs(obShapeAabb.m_max(1));
				fExtent *= 1.1f; // Make it a bit longer just incase
					
				// Make an up vector from it
				CDirection obRayVector(0.0,1.0,0.0);
				obRayVector *= fExtent; // Lengthen it so its long enough to be outside the shape
				
				// Project our point along it
				CPoint obFrom(obPoint + obRayVector);

				// Into local space of the shape
				obFrom -= Physics::MathsTools::hkVectorToCPoint(obTransform.getTranslation());
				obPoint -= Physics::MathsTools::hkVectorToCPoint(obTransform.getTranslation());
				
				// Cast a ray from this projected point to the point
				hkShapeRayCastInput obInput;
				hkShapeRayCastOutput obOutput;
				obInput.m_from.set(obFrom.X(),obFrom.Y(),obFrom.Z());
				obInput.m_to.set(obPoint.X(),obPoint.Y(),obPoint.Z());
				m_pobPhantom->getCollidable()->getShape()->castRay(obInput,obOutput);
				
				// If it's hit then we need to do it again from the opposite direction and check for a hit
				if (obOutput.hasHit())
				{
					// Reset ray stuff
					obOutput.reset();
					
					// Reflect vector
					obRayVector *= -1;
					
					// Project point along it
					obFrom = obPoint + obRayVector;
					
					// Cast a ray from this projected point to the point
					obInput.m_from.set(obFrom.X(),obFrom.Y(),obFrom.Z());
					obInput.m_to.set(obPoint.X(),obPoint.Y(),obPoint.Z());
					m_pobPhantom->getCollidable()->getShape()->castRay(obInput,obOutput);
					
					// If it's hit then the point must be within the volume
					if (obOutput.hasHit())
						m_obInteractableEntities.push_back(pobEntity);
				}
			}
		}
	}

	event.m_collidableAccept = HK_COLLIDABLE_ACCEPT;
};

void CombatPhysicsStrikeVolumePhantomOverlapListener::collidableRemovedCallback( const hkCollidableRemovedEvent& event ) {};

bool CombatPhysicsStrikeVolumePhantomOverlapListener::EntityIsInStrikeList( const CEntity* pobEnt )
{
	for ( ntstd::List< const CEntity* >::iterator obIt = m_obEntitesToStrike.begin();
			obIt != m_obEntitesToStrike.end();
			obIt++ )
	{
		if ((*obIt) == pobEnt)
			return true;
	}

	return false;
}

bool CombatPhysicsStrikeVolumePhantomOverlapListener::EntityIsInInteractableList( const CEntity* pobEnt )
{
	for ( ntstd::List< const CEntity* >::iterator obIt = m_obInteractableEntities.begin();
			obIt != m_obInteractableEntities.end();
			obIt++ )
	{
		if ((*obIt) == pobEnt)
			return true;
	}

	return false;
}

const ntstd::List< const CEntity* >& CombatPhysicsStrikeVolumePhantomOverlapListener::Update()
{
	return m_obEntitesToStrike;
};

CombatPhysicsTransformBasedMovementDescriptor::CombatPhysicsTransformBasedMovementDescriptor()
{
	m_obReferenceVector = CDirection( CONSTRUCT_CLEAR );
	m_fAngleDeltaTrigger = 10.0f;
}

void CombatPhysicsTransformBasedMovementDescriptor::Initialise(CEntity* pobParent)
{
	Transform* pobTransform = pobParent->GetHierarchy()->GetTransform(m_obTransform);
	if (pobTransform)
	{
		m_obReferenceVector = CDirection( pobTransform->GetWorldMatrix().GetTranslation() -  pobParent->GetPosition() );
		m_obReferenceVector.Y() = 0.0f;
		m_obReferenceVector.Normalise();
	}
}

float CombatPhysicsTransformBasedMovementDescriptor::GetStartTime(CEntity* pobParent)
{
	Transform* pobTransform = pobParent->GetHierarchy()->GetTransform(m_obTransform);
	if (pobTransform)
	{
		CDirection obRootToTransform( pobTransform->GetWorldMatrix().GetTranslation() - pobParent->GetPosition() );
		obRootToTransform.Y() = 0.0f;
		obRootToTransform.Normalise();
		
		if (MovementControllerUtilities::RotationAboutY( m_obReferenceVector, obRootToTransform ) > m_fAngleDeltaTrigger * DEG_TO_RAD_VALUE)
		{
			// We want to spawn a volume right now, reset reference
			m_obReferenceVector = CDirection( pobTransform->GetWorldMatrix().GetTranslation() -  pobParent->GetPosition() );
			m_obReferenceVector.Y() = 0.0f;
			m_obReferenceVector.Normalise();

			return 0.0f;
		}
	}

	return 100.0f;
}

/***************************************************************************************************
*
*	CLASS		CombatPhysicsStrikeVolume
*
*	DESCRIPTION	Class to execute described combat strike volume behaviour
*
***************************************************************************************************/
CombatPhysicsStrikeVolume::CombatPhysicsStrikeVolume(const CStrike* pobStrike, CombatPhysicsStrikeVolumeDescriptor* pobDescriptor)
{
	// Fill up values we might need later
	m_pobDescriptor = pobDescriptor;
	m_pobOwner = pobStrike->GetOriginatorP();
	if (!pobDescriptor->GetAlternateAttackData())
		m_pobAttackData = pobStrike->GetAttackDataP();
	else
		m_pobAttackData = pobDescriptor->GetAlternateAttackData();
	m_pobStrikePointer = pobStrike;
	m_pobTarget = pobStrike->GetTargetP();

	m_fAge = 0.0f;

	// Get initial position/orientation from movement
	switch (pobDescriptor->GetMovementDescriptor()->GetType())
	{
	case CombatPhysicsStrikeVolumeMovementDescriptor::CPSVM_TRANSFORM:
		{
			CombatPhysicsTransformBasedMovementDescriptor* pobDesc = (CombatPhysicsTransformBasedMovementDescriptor*)pobDescriptor->GetMovementDescriptor();
			Transform* pobTransform = pobStrike->GetOriginatorP()->GetHierarchy()->GetTransform(pobDesc->m_obTransform);
			if (pobTransform)
			{
				CDirection obRootToTransform( pobTransform->GetWorldMatrix().GetTranslation() -  pobStrike->GetOriginatorP()->GetPosition() );
				obRootToTransform.Y() = 0.0f;
				obRootToTransform.Normalise();
				CombatPhysicsStrikeVolumeStraightMovementDescriptor* pobStraight = NT_NEW_CHUNK(Mem::MC_ENTITY) CombatPhysicsStrikeVolumeStraightMovementDescriptor();
				pobStraight->m_fAngle = 0.0f; //MovementControllerUtilities::RotationAboutY( pobStrike->GetOriginatorP()->GetMatrix().GetZAxis(), obRootToTransform );
				pobStraight->m_fSpeed = pobDesc->m_fSpeed;
				pobStraight->m_fTimeout = pobDesc->m_fTimeout;
				pobStraight->m_obStartPosition = pobTransform->GetWorldMatrix().GetTranslation();
				
				m_pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CombatPhysicsStrikeVolumeStraightMovement(pobStraight);
				m_pobMovement->SetDeleteDescriptor(true);
			}
			break;
		}
	case CombatPhysicsStrikeVolumeMovementDescriptor::CPSVM_STRAIGHT:
		{
			m_pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CombatPhysicsStrikeVolumeStraightMovement((CombatPhysicsStrikeVolumeStraightMovementDescriptor*)pobDescriptor->GetMovementDescriptor());
			break;
		}
	case CombatPhysicsStrikeVolumeMovementDescriptor::CPSVM_COUNT:
		{
			// This is ok, concentric waves don't have a specific movement descriptor
			//ntError(0);
			m_pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CombatPhysicsStrikeVolumeMovement(pobDescriptor->GetMovementDescriptor());
			break;
		}
	}

	hkTransform obTransform = m_pobMovement->GetInitialTransform(m_pobOwner);

	m_pobTransform = NT_NEW Transform();
	CMatrix obMtx(Physics::MathsTools::hkQuaternionToCQuat(hkQuaternion(obTransform.getRotation())), Physics::MathsTools::hkVectorToCPoint(obTransform.getTranslation()));
	m_pobTransform->SetLocalMatrix(obMtx);
	CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobTransform );

	hkShape* pobShape = 0;
	if (m_pobDescriptor->m_fSphereRadius > 0.0f)
	{
		pobShape = HK_NEW hkSphereShape(m_pobDescriptor->m_fSphereRadius);
	}
	else if (m_pobDescriptor->m_fCapsuleRadius > 0.0f && m_pobDescriptor->m_fCapsuleLength > 0.0f)
	{
		hkVector4 obVertexA = obTransform.getTranslation();
		hkVector4 obVertexB = obVertexA;
		obVertexB(1) += m_pobDescriptor->m_fCapsuleLength;
		obVertexA.sub4(obTransform.getTranslation());
		obVertexB.sub4(obTransform.getTranslation());
		
		pobShape = HK_NEW hkCapsuleShape(obVertexA,obVertexB,m_pobDescriptor->m_fCapsuleRadius);
	}
	else if (m_pobDescriptor->m_fBoxHalfExtentX > 0.0f && m_pobDescriptor->m_fBoxHalfExtentY > 0.0f && m_pobDescriptor->m_fBoxHalfExtentZ > 0.0f)
	{
		pobShape = HK_NEW hkBoxShape(hkVector4(m_pobDescriptor->m_fBoxHalfExtentX,m_pobDescriptor->m_fBoxHalfExtentY,m_pobDescriptor->m_fBoxHalfExtentZ));
	}

	if (pobShape)
	{
		Physics::EntityCollisionFlag obCollisionFlag;
		obCollisionFlag.base = 0;	
		if (m_pobOwner->IsPlayer())
			obCollisionFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_PLAYER_BIT;
		else
			obCollisionFlag.flags.i_am = Physics::CHARACTER_CONTROLLER_ENEMY_BIT;
		//if (m_pobOwner->IsPlayer())
		//	obCollisionFlag.flags.i_collide_with = Physics::CHARACTER_CONTROLLER_ENEMY_BIT;
		//else
		obCollisionFlag.flags.i_collide_with = Physics::CHARACTER_CONTROLLER_ENEMY_BIT | Physics::CHARACTER_CONTROLLER_PLAYER_BIT | Physics::LARGE_INTERACTABLE_BIT;

		m_pobPhantom = HK_NEW hkSimpleShapePhantom(pobShape, obTransform, (int)obCollisionFlag.base);
		
		Physics::FilterExceptionFlag exceptionFlag; 
		exceptionFlag.base = 0;
		exceptionFlag.flags.exception_set |= Physics::IGNORE_FIXED_GEOM;
		hkPropertyValue val2((int)exceptionFlag.base);	
		m_pobPhantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);
		hkPropertyValue val((void*)m_pobOwner);
		m_pobPhantom->addProperty(Physics::PROPERTY_ENTITY_PTR, val);

		m_pobListener = HK_NEW CombatPhysicsStrikeVolumePhantomOverlapListener(m_pobPhantom);
		m_pobPhantom->addPhantomOverlapListener(m_pobListener);

		pobShape->removeReference();
	}
	else // We've got a non havok type of volume to setup, at the moment just the 'concentric waves'
	{
		m_pobPhantom = 0;
		m_pobListener = 0;

		ntError( m_pobDescriptor->m_fConcentricWaveSweep > 0.0f && m_pobDescriptor->m_fConcentricWaveOuterRadius > 0.0f && m_pobDescriptor->m_fConcentricWaveHeight > 0.0f );
	
		m_fConcentricWaveSweep = m_pobDescriptor->m_fConcentricWaveSweep;
		m_fConcentricWaveInnerRadius = m_pobDescriptor->m_fConcentricWaveInnerRadius;
		m_fConcentricWaveOuterRadius = m_pobDescriptor->m_fConcentricWaveOuterRadius;
		m_fConcentricWaveHeight = m_pobDescriptor->m_fConcentricWaveHeight;
		m_fConcentricWaveExpansionSpeed = m_pobDescriptor->m_fConcentricWaveExpansionSpeed;
		m_fConcentricWaveAngle = m_pobDescriptor->m_fConcentricWaveAngle;

		m_pobTransform->SetLocalMatrixFromWorldMatrix(m_pobOwner->GetMatrix());
	}

	// Default audio
	m_uiLoopingSoundId = 0;	
	m_uiPassbySoundId = 0;	
	m_bAudio = true;

	m_obDisplacementLastFrame = CDirection( CONSTRUCT_CLEAR );
}

CombatPhysicsStrikeVolume::~CombatPhysicsStrikeVolume()
{
	if (m_pobTransform)
	{
		m_pobTransform->RemoveFromParent();
		NT_DELETE( m_pobTransform );
	}

	if (m_pobPhantom && m_pobListener)
	{
		m_pobPhantom->removePhantomOverlapListener(m_pobListener);
		HK_DELETE( m_pobListener );
	}

	if (m_pobPhantom && m_pobPhantom->getWorld())
		Physics::CPhysicsWorld::Get().RemovePhantom(m_pobPhantom);

	if (m_pobPhantom)
		HK_DELETE( m_pobPhantom );

	NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobMovement);
}

float CombatPhysicsStrikeVolume::GetWidth() const
{
	if (m_pobDescriptor->m_fSphereRadius > 0.0f)
	{
		return m_pobDescriptor->m_fSphereRadius * 2.0f;
	}
	else if (m_pobDescriptor->m_fCapsuleRadius > 0.0f && m_pobDescriptor->m_fCapsuleLength > 0.0f)
	{
		return m_pobDescriptor->m_fCapsuleRadius * 2.0f;	
	}
	else if (m_pobDescriptor->m_fBoxHalfExtentX > 0.0f && m_pobDescriptor->m_fBoxHalfExtentY > 0.0f && m_pobDescriptor->m_fBoxHalfExtentZ > 0.0f)
	{
		return m_pobDescriptor->m_fBoxHalfExtentX * 2.0f;
	}
	else
	{
		return ( m_fConcentricWaveInnerRadius + m_fConcentricWaveOuterRadius ) * 0.5f;
	}
}

float CombatPhysicsStrikeVolume::GetHeight() const
{
	if (m_pobDescriptor->m_fSphereRadius > 0.0f)
	{
		return m_pobDescriptor->m_fSphereRadius * 2.0f;
	}
	else if (m_pobDescriptor->m_fCapsuleRadius > 0.0f && m_pobDescriptor->m_fCapsuleLength > 0.0f)
	{
		return m_pobDescriptor->m_fCapsuleLength + (m_pobDescriptor->m_fCapsuleRadius * 2.0f);	
	}
	else if (m_pobDescriptor->m_fBoxHalfExtentX > 0.0f && m_pobDescriptor->m_fBoxHalfExtentY > 0.0f && m_pobDescriptor->m_fBoxHalfExtentZ > 0.0f)
	{
		return m_pobDescriptor->m_fBoxHalfExtentY * 2.0f;
	}
	else
	{
		return m_fConcentricWaveHeight;
	}
}

float CombatPhysicsStrikeVolume::GetDepth() const
{
	if (m_pobDescriptor->m_fSphereRadius > 0.0f)
	{
		return m_pobDescriptor->m_fSphereRadius * 2.0f;
	}
	else if (m_pobDescriptor->m_fCapsuleRadius > 0.0f && m_pobDescriptor->m_fCapsuleLength > 0.0f)
	{
		return m_pobDescriptor->m_fCapsuleRadius * 2.0f;	
	}
	else if (m_pobDescriptor->m_fBoxHalfExtentX > 0.0f && m_pobDescriptor->m_fBoxHalfExtentY > 0.0f && m_pobDescriptor->m_fBoxHalfExtentZ > 0.0f)
	{
		return m_pobDescriptor->m_fBoxHalfExtentZ * 2.0f;
	}
	else
	{
		return m_fConcentricWaveOuterRadius - m_fConcentricWaveInnerRadius;
	}
}

const ntstd::List< const CEntity* >& CombatPhysicsStrikeVolume::Update(float fTimeDelta)
{
	// Age me
	m_fAge += fTimeDelta;

	// Use bazooka shot functionality to cause an army reaction
	#ifdef PLATFORM_PS3
	if (ArmyManager::Get().Exists())
	{
		float fRadius = 0.0f;
		if (m_pobDescriptor->m_fSphereRadius > 0.0f)
		{
			fRadius = m_pobDescriptor->m_fSphereRadius;
		}
		else if (m_pobDescriptor->m_fCapsuleRadius > 0.0f && m_pobDescriptor->m_fCapsuleLength > 0.0f)
		{
			fRadius = m_pobDescriptor->m_fCapsuleRadius;
		}
		else if (m_pobDescriptor->m_fBoxHalfExtentX > 0.0f && m_pobDescriptor->m_fBoxHalfExtentY > 0.0f && m_pobDescriptor->m_fBoxHalfExtentZ > 0.0f)
		{
			fRadius = m_pobDescriptor->m_fBoxHalfExtentX;
			if (m_pobDescriptor->m_fBoxHalfExtentY > fRadius)
                fRadius = m_pobDescriptor->m_fBoxHalfExtentY;
			if (m_pobDescriptor->m_fBoxHalfExtentZ > fRadius)
				fRadius = m_pobDescriptor->m_fBoxHalfExtentZ;
		}
		else
		{
			fRadius = m_fConcentricWaveOuterRadius;
		}

		if ( m_pobAttackData->m_eAttackClass == AC_GRAB_STRIKE )
		{
			ArmyManager::Get().SyncAttackStrikeVolumeAt(m_pobTransform->GetWorldMatrix().GetTranslation(),fRadius);
		}
		else if (m_pobAttackData->m_eAttackClass == AC_SPEED_MEDIUM ||
				m_pobAttackData->m_eAttackClass == AC_SPEED_FAST )
		{
			ArmyManager::Get().SpeedAttackStrikeVolumeAt(m_pobTransform->GetWorldMatrix().GetTranslation(),fRadius);
		}
		else if (m_pobAttackData->m_eAttackClass == AC_POWER_MEDIUM ||
				m_pobAttackData->m_eAttackClass == AC_POWER_FAST )
		{
			ArmyManager::Get().PowerAttackStrikeVolumeAt(m_pobTransform->GetWorldMatrix().GetTranslation(),fRadius);
		}
		else if (m_pobAttackData->m_eAttackClass == AC_RANGE_MEDIUM ||
				m_pobAttackData->m_eAttackClass == AC_RANGE_FAST )
		{
			ArmyManager::Get().RangeAttackStrikeVolumeAt(m_pobTransform->GetWorldMatrix().GetTranslation(),fRadius);
		}
	}
	#endif

	UpdateAudio();

	// Havok implementation of volume
	if (m_pobPhantom)
	{
		Physics::WriteAccess mutex;

		// In the havok implementations this should always be empty
		ntAssert( m_obEntitesToStrike.size() == 0 );

		// Add the phantom to the world
		Physics::CPhysicsWorld::Get().AddPhantom(m_pobPhantom);
		Physics::CPhysicsWorld::Get().UpdateCollisionFilterOnPhantom(m_pobPhantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);

		// Place it
		hkTransform obTransform = m_pobPhantom->getTransform();
		CPoint obCurrentPosition = Physics::MathsTools::hkVectorToCPoint(obTransform.getTranslation());
		CPoint obNextPosition = m_pobMovement->GetNextPosition(m_pobOwner,m_pobTarget,obCurrentPosition,fTimeDelta);
		CDirection obMovementVector( obNextPosition - obCurrentPosition );
		obMovementVector.Normalise();

		// Save this
		m_obDisplacementLastFrame = CDirection( obNextPosition - obCurrentPosition );

		// Temporary render
		#ifndef _RELEASE
		g_VisualDebug->RenderLine( obCurrentPosition, obCurrentPosition+obMovementVector, DC_GREEN );
		#endif

		// Construct new axes to make the volume face where it's going
		CMatrix obMtx( CONSTRUCT_IDENTITY );
		obMtx.SetYAxis( CDirection( 0.0f, 1.0f, 0.0f ) );
		obMtx.SetZAxis( obMovementVector );
		obMtx.SetXAxis( obMtx.GetYAxis().Cross( obMtx.GetZAxis() ) );
		obMtx.SetTranslation( obNextPosition );

		obTransform.setTranslation(Physics::MathsTools::CPointTohkVector(obMtx.GetTranslation()));
		obTransform.setRotation(Physics::MathsTools::CMatrixTohkQuaternion(obMtx));
			
		m_pobPhantom->setTransform(obTransform);
		m_pobTransform->SetLocalMatrixFromWorldMatrix(obMtx);

		// Remove it again so our char controller doesn't try to collide with it like a wall
		Physics::CPhysicsWorld::Get().RemovePhantom(m_pobPhantom);

		// Temporary render
		#ifndef _RELEASE		
		Physics::DebugCollisionTools::RenderShape(obMtx,m_pobPhantom->getCollidable()->getShape());
		#endif

		m_obEntitesToStrike = m_pobListener->Update();
		return m_obEntitesToStrike;
	}
	else // Manual entity query type
	{
		// in the non-havok implementations, this should always be null
		ntAssert( m_pobListener == 0 );

		// Get a matrix for the wave origin
		CMatrix obSurface = m_pobTransform->GetWorldMatrix();
		CMatrix obRot( CONSTRUCT_IDENTITY );
		CCamUtil::MatrixFromEuler_XYZ(obRot, 0.0f, m_fConcentricWaveAngle * DEG_TO_RAD_VALUE, 0.0f);
		obSurface = obSurface * obRot;

		// Do expansion
		m_fConcentricWaveOuterRadius += fTimeDelta * m_fConcentricWaveExpansionSpeed;
		m_fConcentricWaveInnerRadius += fTimeDelta * m_fConcentricWaveExpansionSpeed;

		// Save this - not true displacement, just blatting in expansion speed, code that uses this needs to know that concentric waves are special
		m_obDisplacementLastFrame = CDirection( m_fConcentricWaveExpansionSpeed, m_fConcentricWaveExpansionSpeed, m_fConcentricWaveExpansionSpeed );

		// Create my query object
		CEntityQuery obQuery;

		// Only allow the query to execute on AI's, players and bosses
		int iEntTypeMask = CEntity::EntType_Character;

		CEQCProximitySegment obProximitySub;
		obProximitySub.SetRadius( m_fConcentricWaveOuterRadius );
		obProximitySub.SetAngle( m_fConcentricWaveSweep * DEG_TO_RAD_VALUE );
		obProximitySub.SetMatrix( obSurface );
		obQuery.AddClause( obProximitySub );

		CEQCHeightRange obHeightSub;	
		obHeightSub.SetBottom( -0.5f );
		obHeightSub.SetTop( m_fConcentricWaveHeight );
		obHeightSub.SetRelativeY( obSurface.GetTranslation().Y() );
		obQuery.AddClause( obHeightSub );

		CEQCIsTargetableByPlayer obTargettableByPlayerSub;
		obQuery.AddClause( obTargettableByPlayerSub );

		// If there is an inner distance specified exclude items within it
		CEQCProximityColumn obColumnSub;
		if ( m_fConcentricWaveInnerRadius > 0.0f )
		{
			// Use a column check for removal - quicker
			obColumnSub.SetRadius( m_fConcentricWaveInnerRadius );
			obColumnSub.SetMatrix( obSurface );

			obQuery.AddUnClause( obColumnSub );
		}

		obQuery.AddExcludedEntity( *m_pobOwner );

		// Send my query to the entity manager and copy over any entities we need to strike
		CEntityManager::Get().FindEntitiesByType( obQuery, iEntTypeMask );
		for ( QueryResultsContainerType::const_iterator obIt = obQuery.GetResults().begin(); obIt != obQuery.GetResults().end(); ++obIt )
		{
			m_obEntitesToStrike.push_back((*obIt));
		}

		// Fill in interactable entities
		CEntityQuery obQuery2;

		iEntTypeMask = CEntity::EntType_Interactable;;

		obProximitySub.SetRadius( m_fConcentricWaveOuterRadius );
		obProximitySub.SetAngle( m_fConcentricWaveSweep * DEG_TO_RAD_VALUE );
		obProximitySub.SetMatrix( obSurface );
		obQuery2.AddClause( obProximitySub );

		obHeightSub.SetBottom( -0.5f );
		obHeightSub.SetTop( m_fConcentricWaveHeight );
		obHeightSub.SetRelativeY( obSurface.GetTranslation().Y() );
		obQuery2.AddClause( obHeightSub );

		if ( m_fConcentricWaveInnerRadius > 0.0f )
		{
			obColumnSub.SetRadius( m_fConcentricWaveInnerRadius );
			obColumnSub.SetMatrix( obSurface );
			obQuery2.AddUnClause( obColumnSub );
		}

		// Send my query to the entity manager and copy over any entities we need to strike
		CEntityManager::Get().FindEntitiesByType( obQuery2, iEntTypeMask );
		for ( QueryResultsContainerType::const_iterator obIt = obQuery2.GetResults().begin(); obIt != obQuery2.GetResults().end(); ++obIt )
		{
			m_obInteractableEntities.push_back((*obIt));
		}


		// Debug render
		#ifndef _RELEASE
		obSurface.SetTranslation( CPoint( obSurface.GetTranslation().X(), obSurface.GetTranslation().Y() + 0.01f, obSurface.GetTranslation().Z() ) );
		g_VisualDebug->RenderArc( obSurface, m_fConcentricWaveOuterRadius, m_fConcentricWaveSweep, DC_CYAN );
		g_VisualDebug->RenderArc( obSurface, m_fConcentricWaveInnerRadius, m_fConcentricWaveSweep, DC_CYAN );
		obSurface.SetTranslation( CPoint( obSurface.GetTranslation().X(), obSurface.GetTranslation().Y() + m_fConcentricWaveHeight, obSurface.GetTranslation().Z() ) );
		g_VisualDebug->RenderArc( obSurface, m_fConcentricWaveOuterRadius, m_fConcentricWaveSweep, DC_CYAN );
		g_VisualDebug->RenderArc( obSurface, m_fConcentricWaveInnerRadius, m_fConcentricWaveSweep, DC_CYAN );
		#endif

		return m_obEntitesToStrike;
	}
}

bool CombatPhysicsStrikeVolume::IsEntityInPreviouslyStruckList(const CEntity* pobEntity)
{
	for (ntstd::List<const CEntity*>::const_iterator obIt = m_obEntitiesStruck.begin();
		obIt != m_obEntitiesStruck.end();
		obIt++)
	{
		if ((*obIt) == pobEntity)
			return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//	CombatPhysicsStrikeVolume::UpdateAudio
//------------------------------------------------------------------------------------------
void CombatPhysicsStrikeVolume::UpdateAudio()
{
	// Check if projectile audio enabled
	if (!m_bAudio)
		return;

	// Calculate projectile proximity to listener
	CPoint projPos = m_pobTransform->GetWorldMatrix().GetTranslation();
	CPoint listenerPos = AudioSystem::Get().GetListenerPosition();

	if (!m_pobPhantom)
	{
		CDirection projToListener(listenerPos - projPos);
		CDirection projToListenerN( projToListener );
		projToListenerN.Normalise();

		projToListenerN *= m_fConcentricWaveOuterRadius;
		projPos += projToListenerN;
	}

	CDirection projToListener = projPos^listenerPos;

	float fProximitySqr = projToListener.LengthSquared();

	// Start/stop loop as req'd
	if (!m_pobDescriptor->m_obLoopingSoundBank.IsNull() && !m_pobDescriptor->m_obLoopingSoundCue.IsNull())
	{
		// Test if looping sound currently playing
		if (0 == m_uiLoopingSoundId)
		{
			// Start looping sound when projectile within audible range (zero or negative range indicates ALWAYS audible)
			if (m_pobDescriptor->m_fLoopingSoundRange <= 0.0f
				|| fProximitySqr <= m_pobDescriptor->m_fLoopingSoundRange*m_pobDescriptor->m_fLoopingSoundRange)
			{
				// Start sound
				if (AudioSystem::Get().Sound_Prepare(m_uiLoopingSoundId, ntStr::GetString(m_pobDescriptor->m_obLoopingSoundBank), ntStr::GetString(m_pobDescriptor->m_obLoopingSoundCue)))
				{
					AudioSystem::Get().Sound_Play(m_uiLoopingSoundId);
					AudioSystem::Get().Sound_SetPosition(m_uiLoopingSoundId, projPos);
				}
				else
				{
					// Ensure identifier remains invalid (possible for a valid handle to be supplied even on sound prep failure)
					m_uiLoopingSoundId = 0;
				}
			}
		}
		// Stop looping sound beyond audible range
		else if (m_pobDescriptor->m_fLoopingSoundRange > 0.0f
			&& fProximitySqr > m_pobDescriptor->m_fLoopingSoundRange*m_pobDescriptor->m_fLoopingSoundRange)
		{
			// Stop sound
			AudioSystem::Get().Sound_Stop(m_uiLoopingSoundId);
			m_uiLoopingSoundId = 0;
		}
		// Update looping sound position
		else
		{
			// TODO: doppler effect?
			AudioSystem::Get().Sound_SetPosition(m_uiLoopingSoundId, projPos);
		}
	}

	// Trigger passby sound as req'd (no passby for player's own projectiles)
	if (!m_pobDescriptor->m_obPassbySoundBank.IsNull()
		&& !m_pobDescriptor->m_obPassbySoundCue.IsNull())
	{
		// Test if passby sound not currently playing
		if (0 == m_uiPassbySoundId)
		{
			// Trigger passby sound when projectile within audible range (zero or negative range indicates NEVER audible)
			if (m_pobDescriptor->m_fPassbySoundRange > 0.0f
				&& fProximitySqr <= m_pobDescriptor->m_fPassbySoundRange*m_pobDescriptor->m_fPassbySoundRange)
			{
				// Trigger sound
				if (AudioSystem::Get().Sound_Prepare(m_uiPassbySoundId, ntStr::GetString(m_pobDescriptor->m_obPassbySoundBank), ntStr::GetString(m_pobDescriptor->m_obPassbySoundCue)))
				{
					AudioSystem::Get().Sound_Play(m_uiPassbySoundId);
					AudioSystem::Get().Sound_SetPosition(m_uiPassbySoundId, projPos);
				}
				else
				{
					// Ensure identifier remains invalid (possible for a valid handle to be supplied even on sound prep failure)
					m_uiPassbySoundId = 0;
				}
			}
		}
		// Update passby sound position
		else // if (AudioSystem::Get().Sound_IsPlaying(m_uiPassbySoundId))
		{
			AudioSystem::Get().Sound_SetPosition(m_uiPassbySoundId, projPos);
		}
	}
}

void CombatPhysicsStrikeVolume::ResetEntityList()
{
	// Update our history of people we've struck
	for (ntstd::List<const CEntity*>::const_iterator obIt = m_obEntitesToStrike.begin();
		obIt != m_obEntitesToStrike.end();
		obIt++)
	{
		m_obEntitiesStruck.push_back((*obIt));
	}

	if (m_pobListener)
		m_pobListener->Reset();
	if (m_obEntitesToStrike.size() > 0)
		m_obEntitesToStrike.clear();
	if (m_obInteractableEntities.size() > 0)
		m_obInteractableEntities.clear();
}	

float CombatPhysicsStrikeVolume::GetDistanceFrom(CPoint& obPosition)
{
	CVector obVector(obPosition - m_pobTransform->GetWorldMatrix().GetTranslation());
	float fDistance = obVector.Length();

	if (m_pobPhantom)
	{
		return fDistance;
	}
	else
	{
		// Bit shit at the moment, assumes 360 coverage, and that only the leading edge counts
		return fabsf(fDistance - m_fConcentricWaveOuterRadius);
	}
}

#endif

