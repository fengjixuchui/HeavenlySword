/***************************************************************************************************
*
*	Attack system functionality
*
*	CHANGES
*
*	20/03/06	Duncan	Created from mammoth attacks.cpp
*
***************************************************************************************************/

// Necessary includes
#include "game/attacks.h" 
#include "core/visualdebugger.h" 

#include "core/osddisplay.h"

#include "game/hitcounter.h"
#include "game/awareness.h"
#include "game/movement.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/query.h"
#include "game/special.h"
#include "game/entityprojectile.h"
#include "game/renderablecomponent.h"
#include "audio/gameaudiocomponents.h"
#include "game/messagehandler.h"
#include "game/attackdebugger.h"
#include "game/interactioncomponent.h"
#include "camera/coolcam_maya.h"
#include "camera/coolcam_aerialcombo.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camtrans_poirot.h"
#include "camera/camutils.h"

#include "game/syncdcombat.h"
#include "anim/hierarchy.h"
#include "core/exportstruct_anim.h"

#include "physics/advancedcharactercontroller.h"
#include "physics/system.h"
#include "physics/world.h"

// For the syncronised movement
#include "syncdmovement.h"
#include "relativetransitions.h"
#include "continuationtransition.h"
#include "anim/animator.h"

//
//// this should go as soon as we have a good way of requesting announcements
#include "game/aicomponent.h"
#include "ai/aiformationcomponent.h"
#include "ai/aiformationattack.h"
//#include "aistates.h"
//
//// Networking
#include "jamnet/netman.h"
//
//// Combat Cameras
#include "game/attackcameras.h"
#include "objectdatabase/dataobject.h"
//
//// effects
#include "effect/combateffects_trigger.h"
#include "effect/combateffect.h"
#include "effect/effect_shims.h"
#include "effect/effect_manager.h"

#include "lua_enum_list.h"
// To get some data properties off the entity
#include "game/luaattrtable.h"

#include "game/randmanager.h"

#include "camera/timescalarcurve.h"

#include "physics/shapephantom.h"

#include "ai/aiformationmanager.h"

// Bossiness
#include "game/entityboss.h"

// For the nasty E3 NS trigger hack
#include "hud/hudmanager.h"

#include "chatterboxman.h"

#include "superstylesafety.h"

#include "core/gatso.h"
#include "game/entityprojectile.h"


// Style stuff
#include "game/combatstyle.h"	// FIX ME Might not be necissary in the long run

// Horrid king hack.
#include "game/entitykingbohan.h"

// For some printf debug spew
extern char* g_apcCombatStateTable[]; // Get from attack debugger
extern char* g_apcClassNameTable[];


/***************************************************************************************************
*
*	FUNCTION		CombatPhysicsPushVolumesData::CombatPhysicsPushVolumesData
*
*	DESCRIPTION		
*
***************************************************************************************************/

CombatPhysicsPushVolumesData::CombatPhysicsPushVolumesData(const CombatPhysicsPushVolumes& obCombatPhysicsPushVolumes)	
{
	for(unsigned int i = 0; i < obCombatPhysicsPushVolumes.m_obSpeedPhysicsVolumes.size(); i++)
	{
		m_obSpeedPhysicsVolumes.push_back(NT_NEW_CHUNK( Mem::MC_MISC ) CombatPhysicsPushVolumeData);
	}

	for(unsigned int i = 0; i < obCombatPhysicsPushVolumes.m_obPowerPhysicsVolumes.size(); i++)
	{
		m_obPowerPhysicsVolumes.push_back(NT_NEW_CHUNK( Mem::MC_MISC ) CombatPhysicsPushVolumeData);
	}

	for(unsigned int i = 0; i < obCombatPhysicsPushVolumes.m_obRangePhysicsVolumes.size(); i++)
	{
		m_obRangePhysicsVolumes.push_back(NT_NEW_CHUNK( Mem::MC_MISC ) CombatPhysicsPushVolumeData);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CombatPhysicsPushVolumesData::~CombatPhysicsPushVolumesData
*
*	DESCRIPTION		
*
***************************************************************************************************/

CombatPhysicsPushVolumesData::~CombatPhysicsPushVolumesData()
{
	ntstd::List<CombatPhysicsPushVolumeData *, Mem::MC_ENTITY>::iterator obIt = m_obSpeedPhysicsVolumes.begin();
	for(; obIt != m_obSpeedPhysicsVolumes.end(); ++obIt)
	{
		NT_DELETE((*obIt));
	}

	obIt = m_obPowerPhysicsVolumes.begin();
	for(; obIt != m_obPowerPhysicsVolumes.end(); ++obIt)
	{
		NT_DELETE((*obIt));
	}

	obIt = m_obRangePhysicsVolumes.begin();
	for(; obIt != m_obRangePhysicsVolumes.end(); ++obIt)
	{
		NT_DELETE((*obIt));
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CAttackComponent
*
*	DESCRIPTION		
*
***************************************************************************************************/
CAttackComponent::CAttackComponent( Character& obParentEntity, CAttackDefinition* pobAttackDefinition )
:	m_pobParentEntity(&obParentEntity),
	m_pobAttackDefinition( pobAttackDefinition ),
	m_obAttackTracker(),
	m_obAttackMovement( &obParentEntity, pobAttackDefinition->m_pobAttackMovementBlendInTimes ),
	m_iCombatCam( -1 ),
	m_bExternallyDisabled( false ),
	m_bTargetingDisabled( false ),
	m_iCurrEffectsTrigger( 0xffffffff ),
	m_pobMyCurrentStrike( 0 ),
	m_fAttackScalar( 1.0f ),
	m_obEvadeDirection( CONSTRUCT_CLEAR ),
	m_pobStruckStrike( 0 ),
	m_eStruckReactionZone( RZ_FRONT ),
	m_fIncapacityTime( 0.0f ),
	m_fCounteringTime( 0.0f ),
	m_fQuickCounteringTime( 0.0f ),
	m_iStringDepth( 0 ),
	m_bQuickRecover( false ),
	m_bKOAftertouch( false ),
	m_eBlockType( BT_COUNT ),
	m_eRecoveryType( RC_STANDARD ),
	m_eCombatState( CS_STANDARD ),
	m_fStateTime( 0.0f ),
	m_obStrikeList(),
	m_obPreStrikeList(),
	m_obStrikeStack(),
	m_bPreStrikeRequested( false ),
	m_iStrikeWindow( 0 ),
	m_bStrikeLanded( false ),
	m_bKOSuccessful( false ),
	m_fTimeSinceLastDeflect( 0.0f ),
	m_eCurrentStance( ST_SPEED ),
	m_bAffirmStanceNextUpdate(false),
	m_bCanExecuteStanceChangeMovement( false ),
	m_eHitLevel( HL_ZERO ),
	m_fWiggleGetBack( 0.0f ),
	m_bAerialCombo( false ),
	m_iAerialCoolCamID(-1),
	m_bGoingToBeAerialed( false ),
	m_iSyncdDepth( 0 ),
	m_pobSyncdTransform( 0 ),
	m_bIsScalingTime( false ),
	m_pobAttackSpecial( 0 ),
	m_obAttackeeList(),
	m_bInBadCounterDetectWindow( false ),
	m_bBadCounterBeingPunished( false ),
	//m_pcStruckStrikeString( 0 ),
	//m_pcOldStruckStrikeString( 0 ),
	m_fCurrentAttackStrikeProximityCheckAngleDelta( 0.0f ),
	m_obKOTargetVector(),
	m_fKOSpecificAngleToTarget( 0.0f ),
	m_bAutoCounter( false ),
	m_bCombatCamStartedThisAttack( false ),
	m_bSyncAttackInterrupted( false ),
	m_bEvadeCombo( false ),
	m_pobCombatEventLogManager( 0 ),
	m_bGroundAttackRecover( false ),
	m_fAttackButtonTime( -1.0f ),
	m_bNewButtonHeldAttack( false ),
	m_iHighComboReported( 0 ),
	m_pobInstantKORecoverTargetEntity( 0 ),
	m_pobSkillEvadeTargetEntity( 0 ),
	m_bWasFullyRagdolledInKO( false ),
	m_bIsDoingSuperStyleSafetyTransition( false ),
	m_pobSuperStyleSafetyAttacker( 0 ),
	m_pobSuperStyleSafetyReceiver( 0 ),
	m_bSuperStyleSafetyAttackerDone( false ),
	m_bSuperStyleSafetyReceiverDone( false ),
	m_obHitPoint(),
	m_obCounterCameraCheckVolumeHalfExtents(1.0f,.45f,1.0f),
	m_obCounterAttackCheckVolumeHalfExtents(1.0f,.45f,1.0f),
	m_obNonSafetySuperCheckVolumeHalfExtents(1.0f,.45f,1.0f),
	m_bEntityExemptFromRagdollKOs( false ),
	m_fTimeTillNotifyNotUnderAttack( -1.0f ),
	m_abHintForButton(),
	m_obActiveStrikeVolumes(),
	m_bForcedModeEnabled( false ),
	m_pobLastJuggledEntity( 0 ),
	m_fJuggleTargetMemoryTimeSoFar( -1.0f ),
	m_bAttackCollisionBreak(false),
	m_pobDefaultLeadCluster( 0 ),
	m_bEnableStrikeVolumeCreation( false ),
	m_bSpecialStarted( false ),
	m_iCurrentStrikeVolumeEffectsTrigger( 0xffffffff ),
	m_pobSuperStyleStartVolume( 0 ),
	m_bPreviousAttackWasSynchronised( false ),
	m_iDepthInDeflecting( 0 ),
	m_iDepthInRecoiling( 0 ),
	m_obTargetPointOffset( CONSTRUCT_CLEAR )
{
	// Just in case...
	ntAssert(obParentEntity.IsCharacter());

	ATTACH_LUA_INTERFACE(CAttackComponent);

	// Initialise the attack tracker with a pointer to the cluster structure
	m_obAttackTracker.InitialiseAttackStructure( m_pobAttackDefinition->m_pobClusterStructure );
	m_bLeadClusterChanged = false;
	SetDefaultLeadClusterTo(m_pobAttackDefinition->m_pobClusterStructure);

	// If we have a special attack definition then we need to create one
	if ( m_pobAttackDefinition->m_pobSpecialDefinition )
		m_pobAttackSpecial= NT_NEW_CHUNK( Mem::MC_MISC ) AttackSpecial( m_pobParentEntity, m_pobAttackDefinition->m_pobSpecialDefinition );

	// If a debugging component has been created - plug on in
	if ( AttackDebugger::Exists() )
		AttackDebugger::Get().RegisterComponent( this );

	// Make a combat event log manager for people to plug into
	m_pobCombatEventLogManager = NT_NEW_CHUNK( Mem::MC_MISC ) CombatEventLogManager(this->m_pobParentEntity);

	// Reset our data associated with attack details
	//if (m_pobMyCurrentStrike)
	//	ntPrintf("%s: resetting attack %s from CTOR.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString());
	ResetAttack(true);

	// Check if this entity has a specific reaction matrix to load
	const char* pcSpecificReactionMatrix = this->m_pobParentEntity->GetAttributeTable()->GetAttribute("ReactionMatrix").GetString();
	if (pcSpecificReactionMatrix && (*pcSpecificReactionMatrix) != 0)
	{
		// Check that it's the type we need
		DataObject* pobData = ObjectDatabase::Get().GetDataObjectFromName(pcSpecificReactionMatrix);
		if (pobData && strcmp(pobData->GetClassName(),"ReactionMatrix") == 0)
		{
			// We've got our specific ReactionMatrix, cast it and set it
			CAttackDefinition* pobAttackDef = const_cast< CAttackDefinition* >(this->m_pobAttackDefinition);
			pobAttackDef->m_pobReactionMatrix = (ReactionMatrix*)pobData->GetBasePtr();
		}
	}

	// Trigger volume setup - just do the non attacking ones at cotr
	if (m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesNonAttacking)
	{
		m_bSetupCombatPhysicsPushVolumesOnFirstUpdate = true;
		m_pobCombatPhysicsPushVolumesDataNonAttacking = NT_NEW_CHUNK( Mem::MC_MISC ) CombatPhysicsPushVolumesData(*m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesNonAttacking);
	}
	else
	{
		m_bSetupCombatPhysicsPushVolumesOnFirstUpdate = false;
		m_pobCombatPhysicsPushVolumesDataNonAttacking = 0; 
	}

	if (m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesAttacking)
	{		
		m_pobCombatPhysicsPushVolumesDataAttacking = NT_NEW_CHUNK( Mem::MC_MISC ) CombatPhysicsPushVolumesData(*m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesAttacking);
	}
	else
	{		
		m_pobCombatPhysicsPushVolumesDataAttacking = 0; 
	}

	for (int i = 0; i < (int)AB_NUM; i++)
	{
		m_abHintForButton[i] = false;
	}

	m_bMovementFromCombatPoseFlag = false;

	m_bCannotUseRagdoll = false;
        m_bUseProceduralKODeath = false;

	m_bNeedLedgeRecover = false;

	m_bNotifiedToDieOutOfCurrentMovement = false;

	m_bInvulnerableToNormalStrike = m_bInvulnerableToSyncStrike = m_bInvulnerableToProjectileStrike = m_bInvulnerableToCounterStrike = false;

	m_bCanBeHeadshot = false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::~CAttackComponent
*
*	DESCRIPTION		
*
***************************************************************************************************/
CAttackComponent::~CAttackComponent()
{
	// Delete all strikes currently in the stack
	while ( !m_obStrikeStack.empty() )
	{
		NT_DELETE_CHUNK( Mem::MC_MISC, m_obStrikeStack.front() );
		m_obStrikeStack.pop_front();
	}

	// If we have a special unit free it
	if ( m_pobAttackSpecial )
	{
		NT_DELETE_CHUNK( Mem::MC_MISC, m_pobAttackSpecial );
	}

	// Clear the strike list
	m_obStrikeList.clear();
	m_obPreStrikeList.clear();

	// Delete the current strike
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobMyCurrentStrike );
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	
	// Delete the physics push volumes data
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobCombatPhysicsPushVolumesDataNonAttacking );
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobCombatPhysicsPushVolumesDataAttacking );

	// Reset the members to zero so that others know that the data is freed.
	m_pobCombatPhysicsPushVolumesDataNonAttacking = 0;
	m_pobCombatPhysicsPushVolumesDataAttacking = 0;

	// If we have a transform hanging about we should kill it
	if ( m_pobSyncdTransform )
	{
		m_pobSyncdTransform->RemoveFromParent();
		NT_DELETE_CHUNK( Mem::MC_MISC, m_pobSyncdTransform );
		m_pobSyncdTransform = 0;
	}

	// If a debugging component has been created - say goodbye
	if ( AttackDebugger::Exists() )
		AttackDebugger::Get().UnregisterComponent( this );

	// Lose our log manager
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobCombatEventLogManager );

	CleanupCombatPhysicsPushVolumes();
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GetHitCounter
*
*	DESCRIPTION		Get hit counter safly from style manager
*
***************************************************************************************************/
HitCounter* CAttackComponent::GetHitCounter() const
{
	// Only the hero has access to a hit counter
	// If this changes all attack component accesses to the hit counter are via this function
	// so putting a hit counter back on more characters should not be too large a headache - T McK
	if ( m_pobParentEntity->IsPlayer() && m_pobParentEntity->ToPlayer()->IsHero() && StyleManager::Exists() )
	{			
		ntAssert( StyleManager::Get().GetHitCounter() );
		return StyleManager::Get().GetHitCounter();
	}
	else
	{
		return 0;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GetDamageFromStrike
*
*	DESCRIPTION		Get the amount of damage we should take from the strike.
*
***************************************************************************************************/
float CAttackComponent::GetDamageFromStrike(const CStrike* pobStrike) const
{
	ntAssert(pobStrike);

	// Check invulnerability flags
	if ( ( m_bInvulnerableToNormalStrike && !pobStrike->IsCounter() && !pobStrike->ShouldSync() && !pobStrike->GetProjectile() ) 
		||
		( m_bInvulnerableToCounterStrike && pobStrike->IsCounter() ) 
		|| 
		( m_bInvulnerableToProjectileStrike && pobStrike->GetProjectile() )
		|| 
		( m_bInvulnerableToSyncStrike && pobStrike->ShouldSync() && !pobStrike->IsCounter() ) )
	{
		return 0.0f;
	}

	// If we have an incidental strike, look and see what amount of the damage we should take
	if (pobStrike->IsIncidental())
	{
		float fFactor = 1.0f; // Assume normal damage
		
		// If the entity is a projectile
		if( pobStrike->GetProjectile() && pobStrike->GetProjectile()->IsProjectile() )
		{
			const Object_Projectile* pProjectile = pobStrike->GetProjectile()->ToProjectile();
			fFactor += pProjectile->GetAttackScalar();
		}
		
		if ( pobStrike->GetOriginatorP()->GetAttackComponent()->IsDoingSpecial() &&
			(pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST ||
			 pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM) )
			fFactor *= pobStrike->GetOriginatorP()->GetAttackComponent()->m_pobAttackSpecial->GetAttackDamageMultiplier();

		return ( (float)pobStrike->GetAttackDataP()->m_iDamage2 ) * fFactor;
	}
	else
	{
		float fFactor = 1.0f; // Assume normal damage

		// If the entity is a projectile
		if( pobStrike->GetProjectile() && pobStrike->GetProjectile()->IsProjectile() )
		{
			const Object_Projectile* pProjectile = pobStrike->GetProjectile()->ToProjectile();
			fFactor += pProjectile->GetAttackScalar();
		}

		if ( pobStrike->GetOriginatorP() &&
			 pobStrike->GetOriginatorP()->GetAttackComponent()->IsDoingSpecial() &&
			(pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST ||
			 pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM) )
			fFactor *= pobStrike->GetOriginatorP()->GetAttackComponent()->m_pobAttackSpecial->GetAttackDamageMultiplier();

		return (float)pobStrike->GetAttackDataP()->m_iDamage * fFactor;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::RobustIsInWindow
*
*	DESCRIPTION		Finds whether we are currently in a window (FlipFlop) on an attack.  Will also
*					check if the current framerate will miss the whole thing and return true if so.
*
***************************************************************************************************/
int CAttackComponent::RobustIsInWindow( const CFlipFlop& obWindow ) const
{
	// Make sure this is OK
	ntError( m_pobMyCurrentStrike );

	// Have we succeeded in finding a suitable window?
	bool bIsInWindow = false;

	// What window was the one we found
	int iWindow = 0;

	// Find our attack time
	float fAttackTime = m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime( m_fAttackScalar );

	// Loop though each of the 'flops' in the flip flop 
	CFlipFlop::FlipperContainerType::const_iterator obEndIt = obWindow.End();
	for ( CFlipFlop::FlipperContainerType::const_iterator obIt = obWindow.Begin(); obIt != obEndIt; ++obIt )
	{
		// Move through the windows we are testing against
		iWindow++;

		// Find the start and end points of the window
		float fWindowStart = obIt->X1( fAttackTime );
		float fWindowEnd = obIt->X2( fAttackTime );

		// If the current state time is between the window limits then we return true
		if ( ( m_fStateTime >= fWindowStart ) && ( m_fStateTime <= fWindowEnd ) )
		{
			bIsInWindow = true;
			break;
		}

		// If the next update will take us straight over the window - we will return true
		if ( ( m_fStateTime < fWindowStart )
			&&
			( ( m_fStateTime + CTimer::Get().GetGameTimeChange() ) > fWindowEnd ) )
		{
			bIsInWindow = true;
			break;
		}
	}

	// If we found a window return the number
	if ( bIsInWindow )
		return iWindow;
	
	// Otherwise we return zero
	return 0;
}
	

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::SwitchStance
*
*	DESCRIPTION		Returns true is a stance transition is required
*
***************************************************************************************************/
bool CAttackComponent::SwitchStance( STANCE_TYPE eNewStance )
{
	// Do we want to complete any movement?
	bool bMove = true;

	// Don't do the movement if we haven't got the data
	if ( !m_pobAttackDefinition->m_pobStanceSwitchingDef )
		bMove = false;	

	// Don't do movement if external systems say so
	if ( !m_bCanExecuteStanceChangeMovement )
		bMove = false;

	// Don't do this movement if we're running cos it'll stop us in our tracks, which is annoying
	if (m_pobParentEntity->GetInputComponent()->IsDirectionHeld() &&
		m_pobParentEntity->GetPhysicsSystem()->GetLinearVelocity().Length() > 2.0f)
		bMove = false;

	// Otherwise if we are in standard or recovery mode we need to play another transition
	if ( ( m_eCombatState == CS_STANDARD )
		 ||
		 ( ( m_eCombatState == CS_RECOVERING ) && ( m_eRecoveryType != RC_RISING ) && ( !m_bAerialCombo ) ) )
	{
		// Make sure that we are not being asked to switch in vain
		if( eNewStance != m_eCurrentStance )
		{
			// If we want to do some movement
			if ( bMove )
			{
				// Start a transition
				switch( m_eCurrentStance )
				{
				case ST_SPEED:
					// Speed to power
					if ( eNewStance == ST_POWER )
						m_obAttackMovement.StartSwitchMovement( m_pobAttackDefinition->m_pobStanceSwitchingDef->m_obSpeedToPowerAnim );

					// Speed to range
					else
						m_obAttackMovement.StartSwitchMovement( m_pobAttackDefinition->m_pobStanceSwitchingDef->m_obSpeedToRangeAnim );
					break;

				case ST_POWER:
					// Power to speed
					if ( eNewStance == ST_SPEED )
						m_obAttackMovement.StartSwitchMovement( m_pobAttackDefinition->m_pobStanceSwitchingDef->m_obPowerToSpeedAnim );

					// Power to range
					else
						m_obAttackMovement.StartSwitchMovement( m_pobAttackDefinition->m_pobStanceSwitchingDef->m_obPowerToRangeAnim );
					break;
				
				case ST_RANGE:
					// Range to Speed
					if ( eNewStance == ST_SPEED )
						m_obAttackMovement.StartSwitchMovement( m_pobAttackDefinition->m_pobStanceSwitchingDef->m_obRangeToSpeedAnim );

					// Range to Power
					else
						m_obAttackMovement.StartSwitchMovement( m_pobAttackDefinition->m_pobStanceSwitchingDef->m_obRangeToPowerAnim );
					break;

				default:
					break;
				}
							
				// If we are in recovery mode we need to indicate that we have escaped
				if ( m_eCombatState == CS_RECOVERING )
				{
					// Tell the lua state that we want to run away
					CMessageSender::SendEmptyMessage( "msg_combat_breakout", m_pobParentEntity->GetMessageHandler() );

					// End the recovery
					EndRecovery();
				}

				// Pass a message on from the movement system when it is complete
				m_obAttackMovement.SetMovementMessage( "msg_combat_stanceswitch" );
			}

			// Otherwise - if we are not to do the movement transition
			else
			{
				// Just send the message
				CMessageSender::SendEmptyMessage( "msg_combat_stanceswitch", m_pobParentEntity->GetMessageHandler() );
			}

			// Set the current stance
			m_eCurrentStance = eNewStance;

			if (IsDoingSpecial())
				m_pobAttackSpecial->SetSpecialStance(eNewStance);

			// Log me switching stance
			m_pobCombatEventLogManager->AddEvent( CE_CHANGE_STANCE, m_pobParentEntity, (void*)m_eCurrentStance );

			SetCombatPhysicsPushVolumes(false);

			// We have had to make a transition
			return true;
		}

		// Otherwise we have made no transition
		return false;
	}
	
	// Otherwise
	else
	{		
		// Set the current stance
		m_eCurrentStance = eNewStance;

		SetCombatPhysicsPushVolumes(false);
	}

	// Otherwise we just leave the requested stance 
	// to be picked up by the next internal state change
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CleanupCombatPhysicsPushVolumes
*
*	DESCRIPTION		Cleans up the physical representations of the volumes (the list is left for the object manager)
*
***************************************************************************************************/
void CAttackComponent::CleanupCombatPhysicsPushVolumes()
{
	Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if (pobAdvCC)
	{
		if (m_pobCombatPhysicsPushVolumesDataAttacking)
		{
			pobAdvCC->CleanupCharacterCombatPhysicsPushVolumeDescriptors(&m_pobCombatPhysicsPushVolumesDataAttacking->m_obSpeedPhysicsVolumes);
			pobAdvCC->CleanupCharacterCombatPhysicsPushVolumeDescriptors(&m_pobCombatPhysicsPushVolumesDataAttacking->m_obPowerPhysicsVolumes);
			pobAdvCC->CleanupCharacterCombatPhysicsPushVolumeDescriptors(&m_pobCombatPhysicsPushVolumesDataAttacking->m_obRangePhysicsVolumes);
		}
		
		if (m_pobCombatPhysicsPushVolumesDataNonAttacking)
		{
			pobAdvCC->CleanupCharacterCombatPhysicsPushVolumeDescriptors(&m_pobCombatPhysicsPushVolumesDataNonAttacking->m_obSpeedPhysicsVolumes);
			pobAdvCC->CleanupCharacterCombatPhysicsPushVolumeDescriptors(&m_pobCombatPhysicsPushVolumesDataNonAttacking->m_obPowerPhysicsVolumes);
			pobAdvCC->CleanupCharacterCombatPhysicsPushVolumeDescriptors(&m_pobCombatPhysicsPushVolumesDataNonAttacking->m_obRangePhysicsVolumes);
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::SetCombatPhysicsPushVolumes
*
*	DESCRIPTION		Sets the list of combat physics volumes to use on the character controller
*
***************************************************************************************************/
void CAttackComponent::SetCombatPhysicsPushVolumes(bool bAttacking)
{
	// Setup volumes
	Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if (pobAdvCC)
	{
		if (m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesAttacking && bAttacking)
		{
			switch( m_eCurrentStance )
			{
			case ST_SPEED:
				pobAdvCC->SetCharacterCombatPhysicsPushVolumeDescriptors(&m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesAttacking->m_obSpeedPhysicsVolumes,
					&m_pobCombatPhysicsPushVolumesDataAttacking->m_obSpeedPhysicsVolumes);
				break;
			case ST_POWER:
				pobAdvCC->SetCharacterCombatPhysicsPushVolumeDescriptors(&m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesAttacking->m_obPowerPhysicsVolumes,
					&m_pobCombatPhysicsPushVolumesDataAttacking->m_obPowerPhysicsVolumes);
				break;			
			case ST_RANGE:
				pobAdvCC->SetCharacterCombatPhysicsPushVolumeDescriptors(&m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesAttacking->m_obRangePhysicsVolumes,
					&m_pobCombatPhysicsPushVolumesDataAttacking->m_obRangePhysicsVolumes);
				break;
			case ST_COUNT:
				ntAssert(0);
				break;
			}
		}
		else if (m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesNonAttacking && !bAttacking)
		{
			switch( m_eCurrentStance )
			{
			case ST_SPEED:
				pobAdvCC->SetCharacterCombatPhysicsPushVolumeDescriptors(&m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesNonAttacking->m_obSpeedPhysicsVolumes,
					&m_pobCombatPhysicsPushVolumesDataNonAttacking->m_obSpeedPhysicsVolumes);
				break;
			case ST_POWER:
				pobAdvCC->SetCharacterCombatPhysicsPushVolumeDescriptors(&m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesNonAttacking->m_obPowerPhysicsVolumes,
					&m_pobCombatPhysicsPushVolumesDataNonAttacking->m_obPowerPhysicsVolumes);
				break;			
			case ST_RANGE:
				pobAdvCC->SetCharacterCombatPhysicsPushVolumeDescriptors(&m_pobAttackDefinition->m_pobCombatPhysicsPushVolumesNonAttacking->m_obRangePhysicsVolumes,
					&m_pobCombatPhysicsPushVolumesDataNonAttacking->m_obRangePhysicsVolumes);
				break;
			case ST_COUNT:
				ntAssert(0);
				break;
			}
		}
		else
		{
			pobAdvCC->SetCharacterCombatPhysicsPushVolumeDescriptors(0,0);
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateCurrentStance
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::UpdateCurrentStance( void )
{
	// We only do stuff here if we have an input controller
	if ( ( m_pobParentEntity ) && ( m_pobParentEntity->GetInputComponent() ) )
	{
		// Switch on the current-requested state
		switch ( m_eCurrentStance )
		{
		case ST_SPEED:
			// If someone presses power - go to power
			if ( m_pobParentEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_PSTANCE ) )
				SwitchStance( ST_POWER );

			// If someone presses range - go to range
			else if ( m_pobParentEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_RSTANCE ) )
				SwitchStance( ST_RANGE );

			break;

		case ST_POWER:
			// If someone presses range - go to range
			if ( m_pobParentEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_RSTANCE ) )
				SwitchStance( ST_RANGE );

			// Otherwise if power is released...
			else if ( m_pobParentEntity->GetInputComponent()->GetVReleased() & ( 1 << AB_PSTANCE ) || m_bAffirmStanceNextUpdate )
			{
				// If range is held go there
				if ( m_pobParentEntity->GetInputComponent()->GetVHeld() & ( 1 << AB_RSTANCE ) )
					SwitchStance( ST_RANGE );
				else
					SwitchStance( ST_SPEED );
			}

			break;

		case ST_RANGE:
			// If someone presses power - go to power
			if ( m_pobParentEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_PSTANCE ) )
				SwitchStance( ST_POWER );

			// Otherwise if range is released...
			else if ( m_pobParentEntity->GetInputComponent()->GetVReleased() & ( 1 << AB_RSTANCE ) || m_bAffirmStanceNextUpdate )
			{
				// If power is held go there
				if ( m_pobParentEntity->GetInputComponent()->GetVHeld() & ( 1 << AB_PSTANCE ) )
					SwitchStance( ST_POWER );
				else
					SwitchStance( ST_SPEED );
			}

			break;

		default:
			ntAssert( 0 );
			break;
		}

		// Assume the stance switch is complete. 
		m_bAffirmStanceNextUpdate = false;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateCombatPhysicsStrikeVolumes
*
*	DESCRIPTION		Helper to go through and update/act on strike volumes
*
***************************************************************************************************/
void CAttackComponent::UpdateCombatPhysicsStrikeVolumes(float fTimeDelta)
{
	//CGatso::Start("CAttackComponent::UpdateCombatPhysicsStrikeVolumes");

	if (m_bEnableStrikeVolumeCreation)
	{
		// Check through any combat strike volume descriptors and create them at the right time if needs be
		if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetAttackDataP()->m_obStrikeVolumeDescriptors.size() > 0)
		{
			float fCurrentTimeNormal = m_fStateTime / m_pobMyCurrentStrike->GetAttackTime();
		for (ntstd::List<CombatPhysicsStrikeVolumeDescriptor*, Mem::MC_ENTITY>::const_iterator obItDesc = m_pobMyCurrentStrike->GetAttackDataP()->m_obStrikeVolumeDescriptors.begin();
				obItDesc != m_pobMyCurrentStrike->GetAttackDataP()->m_obStrikeVolumeDescriptors.end();
				obItDesc++ )
			{
				if ((*obItDesc)->GetMovementDescriptor()->GetStartTime(m_pobParentEntity) < fCurrentTimeNormal)
				{
					// Check that we've not already created one 
					bool bStrikeMatch = (*obItDesc)->GetLastStrikePointer() == m_pobMyCurrentStrike;

					if (!bStrikeMatch)
					{
						CombatPhysicsStrikeVolume* pobVol = NT_NEW_CHUNK( Mem::MC_MISC ) CombatPhysicsStrikeVolume( m_pobMyCurrentStrike, (*obItDesc) );
						m_obActiveStrikeVolumes.push_back( pobVol );
						(*obItDesc)->SetLastStrikePointer( m_pobMyCurrentStrike ); // Used so we don't recreate it for the same strike when it's dead
						
						// Setup effect
						if (!ntStr::IsNull((*obItDesc)->GetEffectsScript()))
							pobVol->SetCombatEffectsTrigger( m_pobAttackDefinition->m_pCombatEffectsDef->SetupEffectsTrigger(m_pobParentEntity,ntStr::GetString((*obItDesc)->GetEffectsScript()), (*obItDesc)->GetMovementDescriptor()->GetTimeout(), pobVol->GetTransform() ) );
						else
							pobVol->SetCombatEffectsTrigger( 0xffffffff );

						if (m_pobParentEntity->IsBoss())
						{
							((Boss*)m_pobParentEntity)->NotifyCreatedStrikeVolume(pobVol);
						}	
					}
				}
			}	
		}
	}

	// Go through active combat strike volumes and update them
	if (m_obActiveStrikeVolumes.size() > 0)
	{
		for (ntstd::List<CombatPhysicsStrikeVolume*>::iterator obItVol = m_obActiveStrikeVolumes.begin();
			obItVol != m_obActiveStrikeVolumes.end();
			obItVol++ )
		{
			const ntstd::List< const CEntity* >& m_obEntitiesToStrike = (*obItVol)->Update(fTimeDelta);

			if (m_obEntitiesToStrike.size() > 0)
			{
				for ( ntstd::List< const CEntity* >::const_iterator obItEnt = m_obEntitiesToStrike.begin();
						obItEnt != m_obEntitiesToStrike.end();
						obItEnt++ )
				{
					// Check we haven't already got a strike on this entity
					if ( !(*obItVol)->IsEntityInPreviouslyStruckList((*obItEnt)) && 
						// And that they're not in some combat states where it'd be mean to hit them again
						(*obItEnt)->GetAttackComponent()->m_eCombatState != CS_FLOORED && 
						(*obItEnt)->GetAttackComponent()->m_eCombatState != CS_RISE_WAIT && 
						(*obItEnt)->GetAttackComponent()->m_eCombatState != CS_KO )
					{
						// Don't hit the primary target of this strike, unless you're a boss, in which case it's ok
						if ( m_pobParentEntity->IsBoss() || ( m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetTargetP() != (*obItEnt) ) ||
							!m_pobMyCurrentStrike )
						{
							if (!(*obItEnt)->GetAttackComponent()->IsInInvulnerabilityWindow())
							{
								if ((*obItVol)->GetDescriptor()->GetDoRaycastCollisionCheckForStrike())
								{
									Physics::RaycastCollisionFlag obFlag; 
									obFlag.base = 0;
									obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
									obFlag.flags.i_collide_with = (	Physics::LARGE_INTERACTABLE_BIT	| 
																	Physics::SMALL_INTERACTABLE_BIT );

									CPoint obFrom( (*obItVol)->GetOwner()->GetPosition() );
									obFrom.Y() += 0.1f;
									CPoint obTo( (*obItEnt)->GetPosition());
									obTo.Y() += 0.1f;

									const CEntity* pobEnt = Physics::CPhysicsWorld::Get().CastRay( obFrom, obTo, obFlag, true );

									if (!pobEnt)
									{
										CStrike* pobGeneratedStrike = NT_NEW_CHUNK( Mem::MC_MISC ) CStrike(	m_pobParentEntity, 
																					(*obItEnt), 
																					(*obItVol)->GetAttackData(), 
																					m_fAttackScalar, 
																					(*obItVol)->GetAttackData()->m_fMaxDistance, 
																					false,
																					false,
																					false,
																					false,
																					false,
																					false,
																					0,
																					m_pobParentEntity->GetPosition() );

										SyncdCombat::Get().PostStrike( pobGeneratedStrike );
									}
								}
								else
								{
								CStrike* pobGeneratedStrike = NT_NEW_CHUNK( Mem::MC_MISC ) CStrike(	m_pobParentEntity, 
																				(*obItEnt), 
																				(*obItVol)->GetAttackData(), 
																				m_fAttackScalar, 
																				(*obItVol)->GetAttackData()->m_fMaxDistance, 
																				false,
																				false,
																				false,
																				false,
																				false,
																				false,
																				0,
																				m_pobParentEntity->GetPosition() );

								SyncdCombat::Get().PostStrike( pobGeneratedStrike );
							}
						}
					}
				}
			}
			}

			
			const ntstd::List< const CEntity* >& m_obInteractableEntities = (*obItVol)->GetInteractableEntityList();
			
			if (m_obInteractableEntities.size() > 0)
			{
				for ( ntstd::List< const CEntity* >::const_iterator obItEnt = m_obInteractableEntities.begin();
						obItEnt != m_obInteractableEntities.end();
						obItEnt++ )
				{
					if ((*obItVol)->GetDescriptor()->GetCollapseInteractables())
					{
						Message obSmashMessage(msg_smash);
						CDirection obSmashDirection = CDirection( (*obItEnt)->GetPosition() - (*obItVol)->GetTransform()->GetWorldMatrix().GetTranslation() );
						obSmashDirection.Normalise();
						obSmashDirection *= (*obItVol)->GetDescriptor()->GetCollapseCollapsableStrength();
						obSmashMessage.SetFloat("smashDirX", obSmashDirection.X());
						obSmashMessage.SetFloat("smashDirY", obSmashDirection.Y());
						obSmashMessage.SetFloat("smashDirZ", obSmashDirection.Z());
						(*obItEnt)->GetMessageHandler()->Receive(obSmashMessage);
					}

					if ((*obItVol)->GetDescriptor()->GetRattleInteractables())
					{
						Message obSmashMessage(msg_action1);
						(*obItEnt)->GetMessageHandler()->Receive(obSmashMessage);
					}

					if ((*obItVol)->GetDescriptor()->GetStopAtInteractables())
					{
						(*obItVol)->DestroyNow();
					}
				}
			}

			// Been through ents and decided this frame if they need a hit, clear it out now
			(*obItVol)->ResetEntityList();

			if ((*obItVol)->GetAge() >= (*obItVol)->GetDescriptor()->GetMovementDescriptor()->GetTimeout())
			{
				// And now we're finished with the volume so delete it
				if ((*obItVol)->GetCombatEffectsTrigger() != 0xffffffff)
				{
					m_pobAttackDefinition->m_pCombatEffectsDef->AbortEffectsTrigger((*obItVol)->GetCombatEffectsTrigger());
				}

				if (m_pobParentEntity->IsBoss())
				{
					((Boss*)m_pobParentEntity)->NotifyRemovedStrikeVolume((*obItVol));
				}

				NT_DELETE_CHUNK( Mem::MC_MISC,(*obItVol));
				obItVol = m_obActiveStrikeVolumes.erase(obItVol);
				obItVol--;
			}
		}
	}

	//CGatso::Stop("CAttackComponent::UpdateCombatPhysicsStrikeVolumes");
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateJugglingMemory
*
*	DESCRIPTION		Helper to update juggling stuff
*
***************************************************************************************************/
void CAttackComponent::UpdateJugglingMemory( float fTimeDelta )
{
	// Update juggle memory timing if it's active
	if (m_fJuggleTargetMemoryTimeSoFar >= 0.0f)
	{
		m_fJuggleTargetMemoryTimeSoFar += fTimeDelta;
	}

	// Check if it's about time we forgot who we were juggling
	if ( (m_fJuggleTargetMemoryTimeSoFar >= m_pobAttackDefinition->m_fJuggleTargetMemoryTime) )
	{
		//ntPrintf("*** Forgot about juggling, ran out of time.\n");
		m_pobLastJuggledEntity = 0;
		m_fJuggleTargetMemoryTimeSoFar = -1.0f;
	}

	// If they've dropped out of KO, then we need to forget them
	if ( m_pobLastJuggledEntity && m_pobLastJuggledEntity->GetAttackComponent()->m_eCombatState != CS_KO )
	{
		//ntPrintf("*** Forgot about juggling, KO ended.\n");
		m_pobLastJuggledEntity = 0;
		m_fJuggleTargetMemoryTimeSoFar = -1.0f;
	}

	// If we're somehow timing but without an entity to target, stop
	if ( !m_pobLastJuggledEntity && m_fJuggleTargetMemoryTimeSoFar >= 0.0f )
	{
		//ntPrintf("*** Forgot about juggling, no last juggled entity.\n");
		m_pobLastJuggledEntity = 0;
		m_fJuggleTargetMemoryTimeSoFar = -1.0f;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::Update
*
*	DESCRIPTION		Update everything.
*
***************************************************************************************************/
void CAttackComponent::Update( float fTimeDelta )
{
	//CGatso::Start("CAttackComponent::Update");

	// We may have been disabled - if so drop out
	if ( m_bExternallyDisabled )
		return;

	// Update the state time
	m_fStateTime += fTimeDelta;

	// Update juggling logic
	UpdateJugglingMemory(fTimeDelta);

	// Update any strike volumes we may have launched
	UpdateCombatPhysicsStrikeVolumes(fTimeDelta);

	// Assume we want no button hints
	for (int i = 0; i < (int)AB_NUM; i++)
	{
		m_abHintForButton[i] = false;
	}

	// Check if we need to update under attack notifications
	if (m_fTimeTillNotifyNotUnderAttack != -1.0f)
	{
		m_fTimeTillNotifyNotUnderAttack -= fTimeDelta;
		if (m_fTimeTillNotifyNotUnderAttack < 0.0f)
		{
			if (m_pobParentEntity->IsBoss())
			{
				Boss* pobBoss = (Boss*)m_pobParentEntity;
				pobBoss->NotifyUnderAttack(false);
			}

			// Reset to special value so we know
			m_fTimeTillNotifyNotUnderAttack = -1.0f;
		}
	}

	// Setup volumes
	if (m_bSetupCombatPhysicsPushVolumesOnFirstUpdate)
	{
		SetCombatPhysicsPushVolumes(false);
		m_bSetupCombatPhysicsPushVolumesOnFirstUpdate = false;
	}

	// Might have had our health set externally, so quick check for deadness - also prevent AI waking up from dead badness
	if (m_pobParentEntity->IsDead() && m_eCombatState != CS_DYING && m_eCombatState != CS_DEAD)
	{
		ntPrintf("%s: Combat killing because dead in normal state for %f.\n",m_pobParentEntity->GetName().c_str(), m_fStateTime);
		// ntError(0); // [ATTILA] removed for Duncan, was for debug only
		m_pobParentEntity->Kill();
	}

	// Update the world according to our attack TSCurve (if we have one), only if we're not already doing it in a camera though
	// Doing it in this main update so it transcends states and we never get stuck
	if ( m_pobParentEntity->IsPlayer() && !IsInSuperStyleSafetyTransition() && !m_bCombatCamStartedThisAttack && CombatCamProperties::Get().IsTimeScalingEnabled() && m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetAttackDataP()->m_pobTSCurve)
	{	
		CTimer::Get().SetSpecialTimeScalar(m_pobMyCurrentStrike->GetAttackDataP()->m_pobTSCurve->GetScalar(m_fStateTime/m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime(1.0f)));
	}	
	else if( m_pobParentEntity->IsPlayer() )
	{
		CTimer::Get().SetSpecialTimeScalar(1.0f);
	}

	// Update timings for held button attacks
	UpdateButtonHeldAttack(fTimeDelta);

	// Update style level so we select the right grabs and supers
	if ( GetHitCounter() )
		m_eHitLevel = GetHitCounter()->GetCurrentHitLevel();

	// This allows us to react to characters doing a special correctly and the smoothly return to speed
	float fMul = m_pobParentEntity->GetTimeMultiplier() - ( m_pobParentEntity->GetTimeMultiplier() - 1.0f ) * 0.08f;
	m_pobParentEntity->SetTimeMultiplier( fMul );

	// Update the strike stack
	UpdateStrikeStack();

	// Update the special stuff if we have it
	if ( IsDoingSpecial() )
	{
		m_pobAttackSpecial->Update( fTimeDelta );
	}

	// Check if the special we previously started in HL_FOUR is over, and reset style points
	if (GetHitCounter() && !IsDoingSpecial() && m_eHitLevel == HL_FOUR && m_bSpecialStarted)
	{
		SetEnableStrikeVolumeCreation(false);
		m_bSpecialStarted = false;
		GetHitCounter()->Used();
		m_eHitLevel = GetHitCounter()->GetCurrentHitLevel();
	}

	// Update the hit counter if we have it
	// FIX ME combo logic could go into hit counter/style system
	if ( GetHitCounter() )
	{
		HitCounter* pobHitCounter = GetHitCounter();

		// If we have a significant combo - log it
		if ( pobHitCounter->GetHitCount() > 5 && m_iHighComboReported != pobHitCounter->GetHitCount())
		{
			m_iHighComboReported = pobHitCounter->GetHitCount();
#ifdef PLATFORM_PS3
			m_pobCombatEventLogManager->AddEvent(CE_COMBO, 0, (void*)((long int)(m_iHighComboReported)));
#else
			m_pobCombatEventLogManager->AddEvent(CE_COMBO, 0, (void*)m_iHighComboReported);
#endif
		}
		else if ( pobHitCounter->GetHitCount() < 5 )
		{
			m_iHighComboReported = 0;
		}
	}

	// Update our stance - necessary for human controlled characters
	UpdateCurrentStance();

	UpdateBadCounter( fTimeDelta );

	// Update the combat system
	switch( m_eCombatState )
	{
	case CS_STANDARD:			UpdateStandard( fTimeDelta );		break;
	case CS_ATTACKING:			UpdateAttack( fTimeDelta );			break;
	case CS_RECOILING:			UpdateRecoil( fTimeDelta );			break;
	case CS_BLOCK_STAGGERING:	UpdateBlockStagger( fTimeDelta );	break;
	case CS_IMPACT_STAGGERING:	UpdateImpactStagger( fTimeDelta );	break;
	case CS_BLOCKING:			UpdateBlock( fTimeDelta );			break;
	case CS_DEFLECTING:			UpdateDeflecting( fTimeDelta );		break;
	case CS_KO:					UpdateKO( fTimeDelta );				break;
	case CS_FLOORED:			UpdateFloored( fTimeDelta );		break;
	case CS_RISE_WAIT:			UpdateRiseWait( fTimeDelta );		break;
	case CS_HELD:				UpdateHeld( fTimeDelta );			break;
	case CS_RECOVERING:			UpdateRecovery( fTimeDelta );		break;
	case CS_DYING:				UpdateDying( fTimeDelta );			break;
	case CS_DEAD:				break;
	default:					ntAssert(0);							break;
	};

	// Check that we are clearing things up like we should be
	ntAssert( !( m_pobStruckStrike && m_pobMyCurrentStrike ) );

	// Send target point info to the movement system
	if ( IsDoingLedgeRecover() )
	{
		m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint = m_obLastKOPosition;
		m_pobParentEntity->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;

		#ifndef _GOLD_MASTER
			g_VisualDebug->RenderPoint(m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint, 10.0f, DC_RED);
		#endif
	}
	else if ( CanTargetAttack() )
	{
		CDirection obTo(m_pobMyCurrentStrike->GetTargetP()->GetPosition() - m_pobParentEntity->GetPosition() );
		float fLength = obTo.Length();
		obTo.Normalise();
		fLength -= m_obTargetPointOffset.Length();
		if ( fLength >= 0.0f )
			m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint = m_pobParentEntity->GetPosition() + (obTo * fLength);
		else
		{
			m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint = m_pobParentEntity->GetPosition();
			m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint.Y() = m_pobMyCurrentStrike->GetTargetP()->GetPosition().Y();
		}

		m_pobParentEntity->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;
	}
	else if ( !m_pobMyCurrentStrike )
	{
		if ( m_pobStruckStrike && !m_pobStruckStrike->GetOriginatorP() )
		{
			m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint = m_pobStruckStrike->GetInitialOriginatorPosition();
			m_pobParentEntity->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;
		}
		else if ( m_pobStruckStrike && m_pobStruckStrike->GetOriginatorP() ) // This only applies if we have a valid originator that is a character
		{
			m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint = m_pobStruckStrike->GetOriginatorP()->GetPosition();
			m_pobParentEntity->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;
		}
	}
	else if ( m_pobMyCurrentStrike || m_pobStruckStrike )
	{
		m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint.Clear();
		m_pobParentEntity->GetMovement()->m_obMovementInput.m_bTargetPointSet = false;
	}

	// if we have a current attack effect trigger, scale their time deltas accordingly
	CombatEffectsTrigger* pTriggerObj = CombatEffectsDefinition::GetEffectsTrigger( m_iCurrEffectsTrigger );
	if ( pTriggerObj )	
		pTriggerObj->ForceNextTimeDelta( fTimeDelta );


	// Update the list of attackers
	ntstd::List< ntstd::pair<const CEntity*, float> >::iterator obIt = m_obAIAttackList.begin();

	while( obIt != m_obAIAttackList.end() )
	{	// Remove the attack from the list
		if( (obIt->second -= fTimeDelta) <= 0.0f )
			obIt = m_obAIAttackList.erase( obIt );
		else
			++obIt;
	}

	// Keep a rough record of how long the entity hasn't been under attack
	if( !m_obAIAttackList.size() )
		m_fTimeSinceLastDeflect += fTimeDelta;

	//ntAssert(!(m_obAttackTracker.RequestIsReady() && !m_obAttackTracker.GetRequestedAttackLinkP()));

	//CGatso::Stop("CAttackComponent::Update");
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateBadCounter
*
*	DESCRIPTION		Update bad counter detection, and bad counter punishment windows. 
*					Bad counters rely upon PreStrikes, as these are the first indications of incoming
*					attack we get - therefore the highest effective value of the BadCounterDetectTime
*					is autoblockleadtime, as this is how long before a strike a prestrike is sent.
*
***************************************************************************************************/
void CAttackComponent::UpdateBadCounter(float fTimeDelta)
{
	if (!m_pobStruckStrike) // If we stop getting struck, reset the string tracker
		m_pcOldStruckStrikeString = CHashedString();

	if ((m_eCombatState == CS_BLOCKING || m_eCombatState == CS_DEFLECTING || m_eCombatState == CS_RECOVERING) &&
		!m_bInBadCounterDetectWindow && !m_bBadCounterBeingPunished && // If we're not already doing some bad counter stuff
		m_pobAttackDefinition->m_fBadCounterDetectTime != 0.0 && // If we have a bad counter detect time val
		!m_pobParentEntity->IsAI()) // Exclude AIs
	{ // Start the punishable window?
		if ( m_pobStruckStrike )
		{
			// Get the name of the attack, so we can compare it with attack from the past
			m_pcStruckStrikeString = CHashedString(ObjectDatabase::Get().GetNameFromPointer( m_pobStruckStrike->GetAttackDataP() ));
			
			// If we've not got an older attack name, or if this is a different attack in the string of attacks
			if (ntStr::IsNull(m_pcOldStruckStrikeString) || m_pcStruckStrikeString != m_pcOldStruckStrikeString)
			{ 
				// Get the time we need to start detecting bad counters before the strike window
				float fTimeToStartBadCounterWindow = m_pobStruckStrike->GetAttackDataP()->m_obStrike.GetFirstValue(m_pobStruckStrike->GetAttackTime()) - this->m_pobAttackDefinition->m_fBadCounterDetectTime;
				// Use the state time of the attacking player, as this tells us how long they've been in their attack
				if ( m_pobStruckStrike->GetOriginatorP() ) // Make sure we have an originator
				{
					float fTime = m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->GetStateTime();
					if (fTime > fTimeToStartBadCounterWindow)
					{
						//ntPrintf("Starting bad counter window at %f - %f = %f, %s\n", m_pobStruckStrike->GetAttackDataP()->m_obStrike.GetFirstValue(m_pobStruckStrike->GetAttackTime()), this->m_pobAttackDefinition->m_fBadCounterDetectTime, fTimeToStartBadCounterWindow, m_pcStruckStrikeString);
						//ntPrintf("Because %f > %f in overall attack time %f\n", fTime, fTimeToStartBadCounterWindow, m_pobStruckStrike->GetAttackTime());

						m_bInBadCounterDetectWindow = true;
						m_bBadCounterBeingPunished = false;
						m_fTimeInBadCounter = fTime - fTimeToStartBadCounterWindow; // Start tracking how long we've been in the window
						m_pcOldStruckStrikeString = m_pcStruckStrikeString;//ObjectDatabase::Get().GetNameFromPointer( m_pobStruckStrike->GetAttackDataP() ); // Make a note of us starting detection for this strike
					
						// Log that I completely failed to counter
						m_pobCombatEventLogManager->AddEvent(CE_BAD_COUNTER_ATTACK, NULL);
					}
				}
			}
		}
	}

	// Update bad counter detect window
	if (m_bInBadCounterDetectWindow && !m_bBadCounterBeingPunished)
	{
		// Accumulate time
		m_fTimeInBadCounter += fTimeDelta;
				
		if ( !m_pobParentEntity->IsAI() && m_obAttackTracker.GetRequestedAttackType(m_pobParentEntity->GetInputComponent(), m_eCurrentStance) != AM_NONE )
		{
		// Check for an attempted counter
		if (
			m_pobStruckStrike && 
				(m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificKillCounterIndex, m_bAutoCounter) ||
				m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificCounterIndex, m_bAutoCounter))
			)
		{
			if (AttackIsValidCounter(	m_pobStruckStrike->GetAttackDataP()->m_eAttackClass, m_obAttackTracker.GetRequestedAttackClass() ))
			{
				// Begin punishing
				//OSD::Add(OSD::CAMERA, DC_RED, "Bad counter!");
				m_bBadCounterBeingPunished = true;
				m_bInBadCounterDetectWindow = false;
				m_obAttackTracker.Reset();
				m_fTimeInBadCounter = 0.0f; // Start tracking how long we've been punishing
			}
		}
		}

		// Check if time to detect bad counters is over, only if we've not already detected one
		if (!m_bBadCounterBeingPunished && m_fTimeInBadCounter > m_pobAttackDefinition->m_fBadCounterDetectTime)
		{
			// Reset everything
			m_bInBadCounterDetectWindow = false;
			m_bBadCounterBeingPunished = false;
			m_fTimeInBadCounter = 0.0f;
		}
	}

	if (m_bBadCounterBeingPunished && !m_bInBadCounterDetectWindow)
	{
		// Accumulate time
		m_fTimeInBadCounter += fTimeDelta;

		// Check if time to detect bad counters is over
		if (m_fTimeInBadCounter > m_pobAttackDefinition->m_fBadCounterPunishTime)
		{
			// If we're in a counter window, don't come out yet, otherwise we're giving the button masher a counter on a plate
			if (!(m_eCombatState == CS_DEFLECTING && m_fStateTime <= m_fCounteringTime))
			{
				// Reset everything
				m_bInBadCounterDetectWindow = false;
				m_bBadCounterBeingPunished = false;
				m_fTimeInBadCounter = 0.0f;
			}
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CanTargetAttack
*
*	DESCRIPTION		Are we attacking and should we direct our character to the person we are trying
*					to hit.
*
***************************************************************************************************/
bool CAttackComponent::CanTargetAttack( void ) const
{
	// We must have a current attack
	if ( !m_pobMyCurrentStrike )
		return false;

	// And that attack must have a target
	if ( !m_pobMyCurrentStrike->GetTargetP() )
		return false;

	if ( !m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()  )
		return false;

	// And that target must not be evading during our no lockon window
	if ( ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_pobMyCurrentStrike )
		 &&
		 ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_EVADE )
		 &&
		 ( IsInNoLockWindow() > 0 ) )
		return false;

	// If we manage to get through all that then we are good to go
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartNewAttack
*
*	DESCRIPTION		Used by the state system to start a new attack.  Returns true if successful
*
***************************************************************************************************/
//#define SKILL_EVADE_ONLYENEMYAI	//Comment this to allow Kai to SkillEvade strikes done on her from all characters instead of just Enemy AI.
bool CAttackComponent::StartNewAttack( void )
{
	// If a human player is being punished, drop out
	if (m_bBadCounterBeingPunished && !m_pobParentEntity->IsAI())
		return false;

	// If I'm doing a safety transition, then don't start new attacks
	if (m_bIsDoingSuperStyleSafetyTransition)
		return false;

	//CGatso::Start("CAttackComponent::StartNewAttack");

	// Ask the tracker for the appropriate attack
	bool bAttackIssued = false;

	// Force a stance check at this point
	UpdateCurrentStance();
	
	// If we have a input pad save the secondar pad direction at this point - only for evades
	if ( ( m_pobParentEntity->GetInputComponent() ) && ( m_obAttackTracker.RequestIsReady() ) && ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
		m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();
	
	// Debug - shouldn't ever be uncommented in AB, if it is, throw something blunt and heavy at Duncan's face.
	//if (m_pobParentEntity->IsPlayer() && SuperStyleSafetyManager::Get().PointInSuperStyleStartVolume(m_pobParentEntity->GetPosition()) && !SuperStyleSafetyManager::Get().PointInSuperStyleContinueVolume(m_pobParentEntity->GetPosition()))
	//	SuperStyleSafetyManager::Get().GetSuperStyleSafeContinuePoint(m_pobParentEntity->GetPosition());

	m_pobSuperStyleStartVolume = 0;
	if (SuperStyleSafetyManager::Exists())
		m_pobSuperStyleStartVolume = SuperStyleSafetyManager::Get().PointInSuperStyleStartVolume(m_pobParentEntity->GetPosition());

	// Select the attack that we are going to carry out
	bAttackIssued = m_obAttackTracker.SelectStartAttack( m_pobParentEntity, m_eCurrentStance, m_eHitLevel, m_pobSuperStyleStartVolume == 0 ) && m_obAttackTracker.RequestIsReady();

	//If we're Kai and we're evading, then we may want to start a skill-evade.
	//TODO: Move a large chunk of this (aside from the initial "am I Kai?" checks and maybe the "am I evading" ones too) into a seperate
	//function to help keep this one clean.
	//Something like a boolean-returning "StartKaiSkillEvade()" function, store the result, and use it to skip certain remaining steps of
	//the function (if necessary).
	if(m_pobParentEntity && m_pobParentEntity->IsPlayer() && m_pobParentEntity->ToPlayer() && m_pobParentEntity->ToPlayer()->IsArcher())
	{
		ATTACK_EVADE_TYPE eEvadeType = AET_STANDARD;
		ATTACK_EVADE_SECTOR eEvadeSector = AES_NUM_EVADESECTORS;
		float fActualMarginValue = 10.0f;
		if(bAttackIssued && (m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE))
		{
			ntPrintf("We're the archer and we're starting an evade.\n");
			CEntity* pSkillEvadeTarget = 0;
			UNUSED(pSkillEvadeTarget);

			//Get a list of all of the live enemy entities within a 5 metre radius.
			CEntityQuery obQuery;
			//Alive
			CEQCHealthLTE obHealth(0.0f);
			obQuery.AddUnClause( obHealth );
			//Is enemy
			//NOTE: Taken out (at least for now) so that test-levels (where the user controls the enemy of type 'Player') work with this.
#ifdef SKILL_EVADE_ONLYENEMYAI
			CEQCIsEnemy obIsEnemy;
			obQuery.AddClause( obIsEnemy );
#endif
			//Within radius
			CEQCProximitySphere obSphere;
			obSphere.Set(m_pobParentEntity->GetPosition(), m_pobAttackDefinition->m_fSkillEvadeRadius);
			obQuery.AddClause(obSphere);
			//Not us!
			obQuery.AddExcludedEntity(*m_pobParentEntity);

			//Retrieve the list.
#ifdef SKILL_EVADE_ONLYENEMYAI
			CEntityManager::Get().FindEntitiesByType(obQuery, CEntity::EntType_AI);
#else
			CEntityManager::Get().FindEntitiesByType(obQuery, CEntity::EntType_Character);
#endif

			//Now we need to do some processing on this list of entities to see if any of them are suitable to perform
			//a synchronised skill-evade on.
			QueryResultsContainerType& rEntList = obQuery.GetResults();
			QueryResultsContainerType::const_iterator obIt	= rEntList.begin();
			QueryResultsContainerType::const_iterator obItEnd = rEntList.end();
//DEBUG
			int iNumEntitiesInRange = (int)rEntList.size();
			if(iNumEntitiesInRange > 0)
			{
				ntPrintf("A total of %d entities qualify for the initial checks\n", iNumEntitiesInRange);
			}
//DEBUG - End
			for( ; obIt != obItEnd;	++obIt )
			{
				//Get a pointer to this AI so that we can query it further.
				CEntity* pEnemyAI = *obIt;

				//Check if this entity is currently attacking us.
				if(pEnemyAI->GetAttackComponent())
				{
					const CStrike* pStrike = pEnemyAI->GetAttackComponent()->AI_Access_GetCurrentStrike();
					//If this enemy isn't currently attacking at all, then it can't be attacking us!
					if(!pStrike)
					{
						continue;
					}

					//If the enemy DOES have a strike, then check if it's directed at us... if not, we can ignore them.
					bool bAttackingUs = (pStrike->GetTargetP() == m_pobParentEntity);
					if(!bAttackingUs)
					{
						continue;
					}

					//Get a pointer to the attack data so that we can query the specific flip-flops for strikes.
					const CAttackData* pAttackData = pStrike->GetAttackDataP();
					if(!pAttackData)
					{
						ntAssert_p(false, ("No attack data on an in-progress strike... this shouldn't happen..."));
						continue;
					}

					//Now check that the current time on the strike is within a specific margin BEFORE the first strike window.
					float fSkillEvadeMargin = m_pobAttackDefinition->m_fSkillEvadeMargin;
					float fCurrAttackTime = pStrike->GetAttackTime();

					//Loop through each of the flops in the strike flip-flop
					const CFlipFlop* pStrikeFlipFlop = &pAttackData->m_obStrike;
					if(!pStrikeFlipFlop)
					{
						ntAssert_p(false, ("No strike flip-flop on attack data... this shouldn't happen..."));
						continue;
					}
					int iWindow = 0;

					bool bCanSkillEvade = false;
					CFlipFlop::FlipperContainerType::const_iterator obEndIt = pStrikeFlipFlop->End();
					for(CFlipFlop::FlipperContainerType::const_iterator obIt = pStrikeFlipFlop->Begin() ; obIt != obEndIt ; ++obIt)
					{
						//Move through the windows we're testing against.
						iWindow++;

						//Find the start point of the window.
						float fWindowStart = obIt->X1(fCurrAttackTime);

						//If the current state time is in the window margin before this, we can skill-evade it.
						float fLowerMargin = fWindowStart - fSkillEvadeMargin;
						if(fLowerMargin < 0.0f) { fLowerMargin = 0.0f; }
						float fStateTime = pEnemyAI->GetAttackComponent()->GetStateTime();
						if((fStateTime >= fLowerMargin) && (fStateTime <= fWindowStart))
						{
							ntPrintf("***WE CAN SKILL EVADE THIS STRIKE!***\n");
							bCanSkillEvade = true;
							fActualMarginValue = fWindowStart - fStateTime;
							break;	//No need to check any more.
						}

						//Actually, for now at least, just check the first one.
						break;
					}

					//By this point we know that there's an entity that's attacking us within our radius, so now
					//we perform "best" tests to see which one of the entities attacking us is the best one to perform
					//our synchronised skill-evade on (there will usually only be one that matches all requirements
					//anyway, but just for safety sake we'll keep looking after finding a suitable one in-case there's
					//a better one).
					if(bCanSkillEvade)
					{
						//If we don't have a current skill-evade target then we just take this enemy and make it our target.
						if(!pSkillEvadeTarget)
						{
							pSkillEvadeTarget = pEnemyAI;
						}
						//Otherwise, we need to see if this enemy is more suitable as a skill-evade taregt than our current one!
						else
						{
							//Do an angle-based check. The one that's more in-front of the archer is a better target and it means
							//less rotation-popping.
							CDirection obArcherForwards = m_pobParentEntity->GetMatrix().GetZAxis();
							CDirection obArcherToCurrent = CDirection(pSkillEvadeTarget->GetPosition() - m_pobParentEntity->GetPosition());
							CDirection obArcherToNew = CDirection(pEnemyAI->GetPosition() - m_pobParentEntity->GetPosition());

							//For the score we just take the dot-product of the archer's forward vector with the direction to each target.
							//This will range from 1 (directly in-front) to -1 (directly behind), so we can just compare them against each
							//other and the higher one is closest to being infront of the archer... nice and clean.
							float fDirectionScoreCurrent = obArcherForwards.Dot(obArcherToCurrent);
							float fDirectionScoreNew = obArcherForwards.Dot(obArcherToNew);

							//If the new one is better, switch target... otherwise we'll just keep our current one.
							if(fDirectionScoreNew > fDirectionScoreCurrent)
							{
								pSkillEvadeTarget = pEnemyAI;
							}
						}
					}
				}
			}

			//Okay, so now if we have a skill-evade target we're going to want to perform some kind of cool synchronised
			//skill-evade-attack on that target.
			if(pSkillEvadeTarget)
			{
				ntPrintf("***WE HAVE A SKILL EVADE TARGET...***\n");
				//Store it within the attack component for use during BuildStrikeFromData.
				m_pobSkillEvadeTargetEntity = pSkillEvadeTarget;
				//Rather than just saying "use a skill evade" we specify between close-up skill evades (where we're too close to some
				//static geometry), or ones where we can be a bit more free.
				//We retrieve the half-extents to check from our attack definition.
				CPoint obEnvironmentCheckHalfExtents = m_pobAttackDefinition->m_obSkillEvadeAttackEnvironmentCheckHalfExtents;

				//We want this box to be aligned around the player.
				float fPlayerXRot, fPlayerYRot, fPlayerZRot;
				CCamUtil::EulerFromMat_XYZ(m_pobParentEntity->GetMatrix(), fPlayerXRot, fPlayerYRot, fPlayerZRot);

				CPoint obPositionToCheck = m_pobParentEntity->GetPosition();
				obPositionToCheck.Y() += 1.0f;	//Lift it off of the ground a little bit.

				//Check which sector this evade-attack is going to operate in (i.e. is the enemy attacking from in-front, behind, left, right,
				//or the dreaded diagonal? O_o;)
				CDirection obPlayerZ = m_pobParentEntity->GetMatrix().GetZAxis();
				CDirection obPlayerToTarget = CDirection(pSkillEvadeTarget->GetPosition() - m_pobParentEntity->GetPosition());
				obPlayerToTarget.Normalise();
				//Angle in degrees.
				float fAngle = (float)((CMaths::SafeAcosf(obPlayerZ.Dot(obPlayerToTarget))) * (180.0f / 3.14159265358979323846f));
				ntPrintf("***SKILL EVADE: Angle to target is %f", fAngle);
				//In-front?
				if(fAngle < 45.0f)
				{
					ntPrintf("*** FRONT SKILL EVADE ***\n");
					eEvadeSector = AES_FRONT;
				}
				//Behind?
				else if(fAngle > 135.0f)
				{
					ntPrintf("*** BACK SKILL EVADE ***\n");
					eEvadeSector = AES_BACK;
				}
				//Otherwise it's to one of the side segments... we need to check which.
				else
				{
					ntPrintf("*** SIDE SKILL EVADE - ");
					CDirection obPlayerLeft = m_pobParentEntity->GetMatrix().GetXAxis();
					CDirection obPlayerRight = -m_pobParentEntity->GetMatrix().GetXAxis();
					bool bAttackedFromRight = (obPlayerRight.Dot(obPlayerToTarget) > obPlayerLeft.Dot(obPlayerToTarget)) ? true : false;

					eEvadeSector = (bAttackedFromRight == true) ? AES_RIGHT : AES_LEFT;

					ntPrintf((bAttackedFromRight == true) ? "RIGHT ***\n" : "LEFT ***\n");
				}

				ntError_p( eEvadeSector != AES_NUM_EVADESECTORS, ("Failed to select a skill-evade sector.. what happened?") );

				//If our box intersects any static geometry, then we want to use close-skillevade attacks. Otherwise we can safely use the
				//normal skill-evade attacks.
				bool bOpen = false;
				if(!Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obPositionToCheck, obEnvironmentCheckHalfExtents, fPlayerYRot))
				{
					eEvadeType = (fActualMarginValue <= m_pobAttackDefinition->m_fSuperSkillEvadeMargin) ? AET_SUPERSKILLEVADE : AET_SKILLEVADE;
					bOpen = true;
					if(eEvadeType == AET_SKILLEVADE)
					{
						ntPrintf("***PERFORMING OPEN SKILL EVADE***\n");
					}
					else
					{
						ntPrintf("***PERFORMING OPEN SUPER SKILL EVADE***\n");
					}
				}
				else
				{
					eEvadeType = (fActualMarginValue <= m_pobAttackDefinition->m_fSuperSkillEvadeMargin) ? AET_SUPERSKILLEVADE_CLOSE : AET_SKILLEVADE_CLOSE;
					bOpen = false;
					if(eEvadeType == AET_SKILLEVADE_CLOSE)
					{
						ntPrintf("***PERFORMING CLOSE SKILL EVADE***\n");
					}
					else
					{
						ntPrintf("***PERFORMING CLOSE SUPER SKILL EVADE***\n");
					}
				}

//DEBUG-RENDER
#ifndef _GOLD_MASTER
				CQuat obDebugRenderOBBOrientation(CDirection(0.0f, 1.0f, 0.0f), fPlayerYRot);
				g_VisualDebug->RenderOBB(obDebugRenderOBBOrientation, obPositionToCheck, CDirection(obEnvironmentCheckHalfExtents),
					(bOpen == true) ? 0xffffffff : 0xffff0000, DPF_WIREFRAME);
#endif
//DEBUG-RENDER-FINISH

				//Before we select an attack (though we know we're going to), we snap the rotation of the archer so that
				//the enemy's location is exactly in-front, behind, left or right to the character. This will stop the
				//enemy from being snapped into a location we don't like (so is a bit safer).
				ntError_p(m_pobParentEntity && m_pobParentEntity->GetHierarchy() && m_pobParentEntity->GetHierarchy()->GetRootTransform(),
					("Why on earth does the archer not have a hierarchy or a root transform? O_o;"));

				//Calculate how much rotation we're going to need.
				//NOTE: In all of the below I assume positive Y-rotation is clockwise from the character's forward vector.
				//If this in incorrect then all the returned desired rotations need to be negated/switched.
				float fDesiredYRotation = 0.0f;
				switch(eEvadeSector)
				{
				case AES_FRONT:
					{
						//If we're attacked from the front, we need to turn left or right by fAngle.
						CDirection obPlayerLeft = m_pobParentEntity->GetMatrix().GetXAxis();
						CDirection obPlayerRight = -m_pobParentEntity->GetMatrix().GetXAxis();
						bool bFrontRight = (obPlayerRight.Dot(obPlayerToTarget) > obPlayerLeft.Dot(obPlayerToTarget)) ? true : false;

						fDesiredYRotation = (bFrontRight) ? -fAngle : fAngle;
					}
					break;
				case AES_BACK:
					{
						//If we're being attacked from behind, need to turn left or right by fAngle.
						CDirection obPlayerLeft = m_pobParentEntity->GetMatrix().GetXAxis();
						CDirection obPlayerRight = -m_pobParentEntity->GetMatrix().GetXAxis();
						bool bBackRight = (obPlayerRight.Dot(obPlayerToTarget) > obPlayerLeft.Dot(obPlayerToTarget)) ? true : false;
						float f180MinusAngle = 180.0f - fAngle;
						UNUSED(f180MinusAngle);

						fDesiredYRotation = (bBackRight) ? -fAngle : fAngle;
//						fDesiredYRotation = (bBackRight) ? f180MinusAngle : -f180MinusAngle;
					}
					break;
				case AES_LEFT:
					{
						//If we're attacked from the left, we need to turn right by 90-fAngle.
						fDesiredYRotation = -(90 - fAngle);
					}
					break;
				case AES_RIGHT:
					{
						//If we're attacked from the right, we need to turn left by 90-fAngle.
						fDesiredYRotation = 90 - fAngle;
					}
					break;
				default:
					break;
				}

				float fDesiredYRotationRadians = (float)((fDesiredYRotation / 180.0f) * 3.14159265358979323846f);

				//Do the rotation before starting our new attack.
				float fCurrXRot, fCurrYRot, fCurrZRot;
				CCamUtil::EulerFromMat_XYZ(m_pobParentEntity->GetMatrix(), fCurrXRot, fCurrYRot, fCurrZRot);

				float fNewYRot = fCurrYRot + fDesiredYRotationRadians;
				CMatrix obNewRotMatrix(CONSTRUCT_IDENTITY);
				obNewRotMatrix.SetFromAxisAndAngle(CDirection(0.0f, 1.0f, 0.0f), fNewYRot);
				CPoint obCurrentRootTranslation = m_pobParentEntity->GetHierarchy()->GetRootTransform()->GetLocalTranslation();
				CMatrix obNewTranslationMatrix(CONSTRUCT_IDENTITY);
				obNewTranslationMatrix.SetTranslation(obCurrentRootTranslation);

				CMatrix obNewMatrix = obNewRotMatrix * obNewTranslationMatrix;
				m_pobParentEntity->GetHierarchy()->GetRootTransform()->SetLocalMatrix(obNewMatrix);

				//RE-SELECT the attack that we are going to carry out, specifying our new evade-type.
				bAttackIssued = m_obAttackTracker.SelectStartAttack(m_pobParentEntity, m_eCurrentStance, m_eHitLevel,
					m_pobSuperStyleStartVolume == 0, eEvadeType, eEvadeSector) && m_obAttackTracker.RequestIsReady();
			}
		}
	}

	// HACK for style level 4 special starting
	if	( m_pobParentEntity->GetInputComponent() && m_pobParentEntity->GetInputComponent()->GetVPressed() & ( 1 << AB_GRAB ) && m_eHitLevel == HL_FOUR && !IsDoingSpecial() )
	{
		SetEnableStrikeVolumeCreation(true);
		m_bSpecialStarted = true;
		StartSpecial();
	}

	// Do not do a targeting update if we are just to start an attack
	if ( bAttackIssued && !m_obAttackTracker.GetRequestedAttackLinkP()->m_pobButtonHeldAttack )
	{
		// Kick off our first move
		//CGatso::Stop("CAttackComponent::StartNewAttack");
		return StartString();
	}
	else if ( bAttackIssued && m_obAttackTracker.GetRequestedAttackLinkP()->m_pobButtonHeldAttack )
	{
		// Start a timer, wait for release
		m_fAttackButtonTime = 0.0f;
		m_bNewButtonHeldAttack = true;
		//CGatso::Stop("CAttackComponent::StartNewAttack");
		return true; // Will be starting some attack soon so return true here
	}
	
	// If we are here we have failed
	//CGatso::Stop("CAttackComponent::StartNewAttack");
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateButtonHeldAttack
*
*	DESCRIPTION		Updates the logic and timers for detecting held button attacks
*
***************************************************************************************************/
void CAttackComponent::UpdateButtonHeldAttack( float fTimeDelta )
{
	// If we're still counting time for the button hold
	if (m_fAttackButtonTime >= 0.0f && m_obAttackTracker.ButtonHeldAttackStillRequested(*m_pobParentEntity->GetInputComponent()))
	{
		m_fAttackButtonTime += fTimeDelta;

		if ( (m_eCurrentStance == ST_SPEED && m_fAttackButtonTime > m_pobAttackDefinition->m_fHeldAttackThresholdSpeed) ||
			(m_eCurrentStance == ST_RANGE && m_fAttackButtonTime > m_pobAttackDefinition->m_fHeldAttackThresholdRange) ||
			(m_eCurrentStance == ST_POWER && m_fAttackButtonTime > m_pobAttackDefinition->m_fHeldAttackThresholdPower) )
		{
			m_fAttackButtonTime = -1.0f;

			// Try to select the held attack and start it, if it fails then we needs to tell lua to breakout of CombatState
			if (m_obAttackTracker.SwitchToRequestedButtonHeldAttack())
			{
				if (m_bNewButtonHeldAttack && !StartString())
					CMessageSender::SendEmptyMessage( "msg_combat_breakout", m_pobParentEntity->GetMessageHandler() );
				m_bNewButtonHeldAttack = false;
				m_obAttackTracker.SetIsWaitingForHeld(false);
				//ntPrintf("%s: Button held attack %s.\n",ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity), ObjectDatabase::Get().GetNameFromPointer(m_obAttackTracker.GetRequestedAttackDataP()));
			}
		}
	} 
	// Else if player has stopped holding the button before time
	else if (m_fAttackButtonTime >= 0.0f && !m_obAttackTracker.ButtonHeldAttackStillRequested(*m_pobParentEntity->GetInputComponent()))
	{
		m_fAttackButtonTime = -1.0f;
		m_obAttackTracker.SetIsWaitingForHeld(false);

		// If we've got an attack selected, try to start it
		if (m_obAttackTracker.RequestIsReady())
		{
			if (m_bNewButtonHeldAttack && !StartString())
				CMessageSender::SendEmptyMessage( "msg_combat_breakout", m_pobParentEntity->GetMessageHandler() );
			m_bNewButtonHeldAttack = false;
		}
	} 
	else if (m_fAttackButtonTime < 0.0f)
	{
		// Make sure if we're not timing anything, tracker isn't waiting
		m_bNewButtonHeldAttack = false;
		m_obAttackTracker.SetIsWaitingForHeld(false);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::SelectNextAttack
*
*	DESCRIPTION		Used by the state system to select a subsequent attack in a string.
*
***************************************************************************************************/
void CAttackComponent::SelectNextAttack( void )
{
	// See if it is time to select and start a new attack
	if ( ( IsInNextMoveWindow() > 0 ) && ( m_obAttackTracker.WillTakeAttackRequests() ) )
	{
		// Force a stance check at this point
		UpdateCurrentStance();

		m_pobSuperStyleStartVolume = 0;
		if (SuperStyleSafetyManager::Exists())
			m_pobSuperStyleStartVolume = SuperStyleSafetyManager::Get().PointInSuperStyleStartVolume(m_pobParentEntity->GetPosition());

		// Look for the new attacks we can do
		bool bSelected = m_obAttackTracker.SelectNextAttack( m_pobParentEntity, m_eCurrentStance, false, m_eHitLevel, m_pobSuperStyleStartVolume == 0 );

		if (bSelected )
		{
			// Log my selection
			m_pobCombatEventLogManager->AddEvent(CE_SELECTED_ATTACK, m_pobParentEntity, (void*)m_obAttackTracker.GetRequestedAttackDataP() );	
		}

		// Does this have a button held attack on it? If so, we need to wait to evaluate which attack to select
		if (m_obAttackTracker.RequestIsReady() && m_obAttackTracker.GetRequestedAttackLinkP()->m_pobButtonHeldAttack)
		{
			// Start a timer, wait for release
			m_fAttackButtonTime = 0.0f;
			m_bNewButtonHeldAttack = false; // Make sure flag is set to next attack
			m_obAttackTracker.SetIsWaitingForHeld(true);
		}

		// If we have a input pad save the secondar pad direction at this point - only for evades
		if ( ( m_pobParentEntity->GetInputComponent() ) && ( m_obAttackTracker.RequestIsReady() ) && ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
			m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AerialTargetingNotify
*
*	DESCRIPTION		Tell this entity they're gonna be aerialed, used in KO state to stop falling
*
***************************************************************************************************/
void CAttackComponent::AerialTargetingNotify() 
{
	ntError( m_eCombatState == CS_KO );

	// Stop any instant recovering
	//m_bQuickRecover = false;
	// Set a bool to be handled in KO state
	m_bGoingToBeAerialed = true;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AerialTargetingCancel
*
*	DESCRIPTION		Tell this entity they're gonna be aerialed, used in KO state to stop falling
*
***************************************************************************************************/
void CAttackComponent::AerialTargetingCancel() 
{
	ntError( m_eCombatState == CS_KO );

	// Use this as an indicator to tell StartKO what not to do
	m_bGoingToBeAerialed = true;

	// Start a falling movement all over again
	StartKO(m_pobStruckStrike, NULL);

	// m_bGoingToBeAerialed flag cleared in StartKO
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateStandard
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::UpdateStandard( float fTimeDelta )
{
	//CGatso::Start("CAttackComponent::UpdateStandard");

	// Constantly checking to see if we've got something to do in STANDARD
	// Was needed because we now want to be able to continue attack strings while in a 2fs from a movement popout
	if (m_obAttackTracker.RequestIsReady())
	{
		if ( GenerateMyStrike( false, false ) ) 
		{
			StartAttack();
		}
	}

	// Lifeclock recharge from style points - v hacky
	if( m_pobParentEntity && m_pobParentEntity->IsPlayer() && 
			m_pobParentEntity->ToPlayer()->IsHero() && m_pobParentEntity->GetInputComponent() ) // Note: AI characters don't have an input component
	{
		float fHeldTime = m_pobParentEntity->GetInputComponent()->GetVHeldTime(AB_GRAB);

		Hero* pobHero = m_pobParentEntity->ToPlayer()->ToHero();

		pobHero->RechargeLC( fHeldTime, fTimeDelta );
	}

	//CGatso::Stop("CAttackComponent::UpdateStandard");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CanBlock
*
*	DESCRIPTION		Checks for component states in which a block request is allowed
*
***************************************************************************************************/
bool CAttackComponent::CanBlock( BLOCK_TYPE eBlockType ) const
{
	// If we have turned off auto blocking globally then we can't auto block
	if ( !m_bGlobablAutoBlockEnable )
		return false;

	// These are the states which we can block from
	if ( ( m_eCombatState == CS_STANDARD ) 
		 || 
		 ( m_eCombatState == CS_RECOVERING ) 
		 ||
		 ( m_eCombatState == CS_BLOCKING )
		 || 
		 ( m_eCombatState == CS_DEFLECTING )
		 ||
		 ( IsInBlockWindow() > 0 ) )
	{
		if (eBlockType == BT_SPEED && m_pobAttackDefinition->m_bCanAutoBlockSpeed)
			return true;
		if (eBlockType == BT_RANGE && m_pobAttackDefinition->m_bCanAutoBlockRange)
			return true;
		if (eBlockType == BT_POWER && m_pobAttackDefinition->m_bCanAutoBlockPower)
			return true;
		if (eBlockType == BT_GRAB && m_pobAttackDefinition->m_bCanAutoBlockGrabs)
			return true;
	}

	// If we are here then we can't block
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartBlock
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::StartBlock( const CStrike* pobStrike, BLOCK_TYPE eBlockType, float fDefaultBlockTime )
{
	//CGatso::Start("CAttackComponent::StartBlock");

	// Save a pointer to the strike
	m_pobStruckStrike = pobStrike;

	// Move into the defending state
	SetState( CS_BLOCKING );

	// Save the blocking type that we are doing
	m_eBlockType = eBlockType;

	//Horrible boss-specific hack so that the king can have in-air blocks.
	if(m_pobParentEntity && m_pobParentEntity->IsBoss() && (((Boss*)m_pobParentEntity)->GetBossType() == Boss::BT_KING_BOHAN))
	{
		KingBohan* pobKing = (KingBohan*)m_pobParentEntity;

		//If we're attached to the demon, air-block
		if(pobKing->HasAttachedDemon())
		{
			ntPrintf("--> King in-air block\n");
			m_obAttackMovement.StartBlockMovement( m_pobAttackDefinition->m_pobPowerBlock, m_fAutoBlockLeadTime );
		}
		//Otherwise we must be on the ground (no wings), ground-block.
		else
		{
			ntPrintf("--> King on-ground block\n");
			m_obAttackMovement.StartBlockMovement( m_pobAttackDefinition->m_pobSpeedBlock, m_fAutoBlockLeadTime );
		}
	}
	else
	{
		// If we are in speed stance start a speed block
		if ( eBlockType == BT_SPEED )
			m_obAttackMovement.StartBlockMovement( m_pobAttackDefinition->m_pobSpeedBlock, m_fAutoBlockLeadTime );

		// If we are in power stance start a power block
		else if ( eBlockType == BT_POWER )
			m_obAttackMovement.StartBlockMovement( m_pobAttackDefinition->m_pobPowerBlock, m_fAutoBlockLeadTime );

		// If we are in range stance start a range block
		else if ( eBlockType == BT_RANGE )
			m_obAttackMovement.StartBlockMovement( m_pobAttackDefinition->m_pobRangeBlock, m_fAutoBlockLeadTime );
	}

	// If we are in another stance then we shouldn't be here - because there aren't any
	//else
	//	ntAssert( 0 );

	// Calculate an incapacity time - we need to hold an auto blocking character in this state
	// for as short a time as possible - that means the full length of the strike window of the
	// incoming attack plus the time before the strike that a block will start
	if ( m_pobStruckStrike )
	{
		m_fIncapacityTime =	m_fAutoBlockLeadTime + 
							m_pobStruckStrike->GetAttackDataP()->m_obStrike.GetFirstValueLength( m_pobStruckStrike->GetAttackDataP()->GetAttackTime( m_pobStruckStrike->GetAttackTimeScalar() ) );
	}
	else
	{
		m_fIncapacityTime = fDefaultBlockTime;
	}

	// Let the Lua state know that we have effectively gone into a struck state
	m_pobParentEntity->GetMessageHandler()->ReceiveMsg<msg_combat_struck>();

	m_bBadCounterBeingPunished = false;

	//CGatso::Stop("CAttackComponent::StartBlock");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateBlock
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::UpdateBlock( float fTimeDelta )
{
	// update if we're in a start volume
	m_pobSuperStyleStartVolume = 0;
	if (SuperStyleSafetyManager::Exists())
		m_pobSuperStyleStartVolume = SuperStyleSafetyManager::Get().PointInSuperStyleStartVolume(m_pobParentEntity->GetPosition());

	// We can do an attack whilst we are in the block state
	m_obAttackTracker.SelectStartAttack( m_pobParentEntity, m_eCurrentStance, m_eHitLevel, m_pobSuperStyleStartVolume == 0 );

	// If we have a input pad save the secondar pad direction at this point - only for evades
	if ( ( m_pobParentEntity->GetInputComponent() ) && ( m_obAttackTracker.RequestIsReady() ) && ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
		m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();

	// We let the player request attacks here
	if ( m_obAttackTracker.RequestIsReady() && !m_bBadCounterBeingPunished)
	{
		// If we successfully generated a strike...
		if ( GenerateMyStrike( false, false ) && StartAttack() )
		{
			// Let the Lua state know that we have countered - or attacked out of a reaction
			CMessageSender::SendEmptyMessage( "msg_combat_countered", m_pobParentEntity->GetMessageHandler() );

			// Get out of here
			return;
		}
	}

	if ( m_pobParentEntity->IsPlayer() || m_pobParentEntity->IsBoss() )
	{
		// Hold the character in this state for the shortest time possible
		if ( m_fStateTime > m_fIncapacityTime )
			EndBlock();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndBlock
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::EndBlock()
{
	// If we are in speed stance finish the speed block
	if ( m_eCurrentStance == ST_SPEED )
		m_obAttackMovement.StartBlockEndMovement( m_pobAttackDefinition->m_pobSpeedBlock );

	// If we are in power stance finish the power block
	else if ( m_eCurrentStance == ST_POWER )
		m_obAttackMovement.StartBlockEndMovement( m_pobAttackDefinition->m_pobPowerBlock );
	
	// If we're in Range stance, finish the range block
	else if ( m_eCurrentStance == ST_RANGE )
		m_obAttackMovement.StartBlockEndMovement( m_pobAttackDefinition->m_pobRangeBlock );
	
	// If we are in another stance then we shouldn't be here
	else
		ntAssert( 0 );

	// Give the movement system a message to pass to the state if this movement is completed fully
	m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );

	// Clear up the strike
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;

	// Go to the recovery state
	SetState( CS_RECOVERING );
	m_fIncapacityTime = 0.0f;
	m_eRecoveryType = RC_STANDARD;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::StartDeflecting
//! Begin the deflecting state
//!
//------------------------------------------------------------------------------------------
void CAttackComponent::StartDeflecting( const CStrike* pobStrike, CHashedString obChosenReceiverAnim )
{
	//CGatso::Start("CAttackComponent::StartDeflecting");

	// If we're already in a deflection/block, note how deep we get in repeatedly deflecting/blocking
	if (m_eCombatState == CS_DEFLECTING || m_eCombatState == CS_BLOCKING)
	{
		m_iDepthInDeflecting++;
	}

	CChatterBoxMan::Get().Trigger("DeflectedAttack", m_pobParentEntity);

	// Save a pointer to the strike
	m_pobStruckStrike = pobStrike;

	// Do we have a specific response to this attack?
//	CHashedString obSpecificDeflect;// = GetSpecificResponse( pobStrike->GetAttackDataP()->m_obReceiverAnim );
	CHashedString obSpecificDeflect = obChosenReceiverAnim;

	// Log my deflection
	m_pobCombatEventLogManager->AddEvent(CE_GOT_DEFLECT, m_pobStruckStrike->GetOriginatorP(), (void*)m_pobStruckStrike->GetAttackDataP());
	if (m_pobStruckStrike->GetOriginatorP())
		m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_DEFLECT, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP());
	else if ( m_pobStruckStrike->GetProjectile() && m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter() )
		m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_DEFLECT, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );
	
	if (pobStrike->IsWithinExclusionDistance())
	{
		obSpecificDeflect = m_pobAttackDefinition->m_obStrikeProximityCheckExclusionDistanceReactionAnim;
	}

	// If the incoming attack is not syncronised - in which case this movement will be sorted
	if ( pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() || !pobStrike->ShouldSync() )
	{
		// If we have a specific deflection then we can do that
		if ( !obSpecificDeflect.IsNull() )
		{
			m_obAttackMovement.StartDeflectingMovement(	obSpecificDeflect, 
														m_pobStruckStrike->GetAttackTimeScalar() );
		}

		// If we are currently INVINCIBLE we kick off the rise deflection
		else if ( ( m_eCombatState == CS_INSTANTRECOVER ) 
			|| 
			( ( m_eCombatState == CS_RECOVERING ) && ( m_eRecoveryType == RC_RISING ) ) )
		{
			m_obAttackMovement.StartDeflectingMovement(	m_pobAttackDefinition->m_obRiseDeflectionAnim, 
														m_pobStruckStrike->GetAttackTimeScalar() );
		}

		else
		{
			//Another horrible king-specific hack for in-air blocks.
			if(m_pobParentEntity && m_pobParentEntity->IsBoss() && (((Boss*)m_pobParentEntity)->GetBossType() == Boss::BT_KING_BOHAN))
			{
				KingBohan* pobKing = (KingBohan*)m_pobParentEntity;
				if(pobKing->HasAttachedDemon())
				{
					ntPrintf("--> King in-air deflect\n");
					m_obAttackMovement.StartDeflectingMovement(	m_pobAttackDefinition->m_pobPowerDeflections->m_obAnimation[m_pobStruckStrike->GetAttackDataP()->m_eReactionAppearance], 
																m_pobStruckStrike->GetAttackTimeScalar() );
				}
				else
				{
					ntPrintf("--> King on-ground deflect\n");
					m_obAttackMovement.StartDeflectingMovement(	m_pobAttackDefinition->m_pobSpeedDeflections->m_obAnimation[m_pobStruckStrike->GetAttackDataP()->m_eReactionAppearance], 
																m_pobStruckStrike->GetAttackTimeScalar() );
				}
			}
			else
			{
				// ...otherwise start the relevent movement and set the time for recovery
				if ( m_eCurrentStance == ST_SPEED )
				{
					m_obAttackMovement.StartDeflectingMovement(	m_pobAttackDefinition->m_pobSpeedDeflections->m_obAnimation[m_pobStruckStrike->GetAttackDataP()->m_eReactionAppearance], 
																m_pobStruckStrike->GetAttackTimeScalar() );
				}

				else if ( m_eCurrentStance == ST_POWER )
				{
					m_obAttackMovement.StartDeflectingMovement(	m_pobAttackDefinition->m_pobPowerDeflections->m_obAnimation[m_pobStruckStrike->GetAttackDataP()->m_eReactionAppearance], 
																m_pobStruckStrike->GetAttackTimeScalar() );
				}
				else if ( m_eCurrentStance == ST_RANGE )
				{
					m_obAttackMovement.StartDeflectingMovement(	m_pobAttackDefinition->m_pobRangeDeflections->m_obAnimation[m_pobStruckStrike->GetAttackDataP()->m_eReactionAppearance], 
																m_pobStruckStrike->GetAttackTimeScalar() );
				}
				else
					ntAssert( 0 );
			}
		}
	}

	// Calculate the time for which our character will be incapacitated by this deflection
	// If we have a valid override time in our attack definition, use that
	if ( m_pobStruckStrike &&
		(m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_FAST ||
		m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_MEDIUM) &&
		m_pobAttackDefinition->m_fDeflectionTimeOverrideSpeed > 0.0)
	{
		m_fIncapacityTime = m_pobAttackDefinition->m_fDeflectionTimeOverrideSpeed;
	}
	else if ( m_pobStruckStrike &&
		(m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST ||
		m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM) &&
		m_pobAttackDefinition->m_fDeflectionTimeOverrideRange > 0.0)
	{
		m_fIncapacityTime = m_pobAttackDefinition->m_fDeflectionTimeOverrideRange;
	}
	else if ( m_pobStruckStrike &&
		(m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST ||
		m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM) &&
		m_pobAttackDefinition->m_fDeflectionTimeOverridePower > 0.0)
	{
		m_fIncapacityTime = m_pobAttackDefinition->m_fDeflectionTimeOverridePower;
	}
	else
	{
		m_fIncapacityTime = m_pobStruckStrike->GetAttackDataP()->m_fDeflectionTime;
	}
	
	m_fCounteringTime = m_pobAttackDefinition->m_fCounterTime;
	m_fQuickCounteringTime = m_pobAttackDefinition->m_fQuickCounterTime;
	// Make sure the counter attack data is sensible
	//user_warn_p( !( m_fCounteringTime > pobStrike->GetAttackDataP()->m_fDeflectionTime && m_fCounteringTime != pobStrike->GetAttackDataP()->m_fDeflectionTime ), ( "Counter time greater than deflection time of attack %s.\n", pobStrike->GetAttackDataP()->GetNameC(), m_fCounteringTime, pobStrike->GetAttackDataP()->m_fDeflectionTime ) );	
	//user_warn_p( !( m_fQuickCounteringTime > m_fCounteringTime && m_fQuickCounteringTime != m_fQuickCounteringTime ), ( "Quick counter time is greater than counter time.\n" ) );

	// Give the movement system a message to pass to the state if this movement is completed fully
	m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );

	// Set our state to deflecting
	SetState( CS_DEFLECTING );

	// Do a hit effect in this case to show that a counter is available
	if ( pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() || !pobStrike->ShouldSync() )
	{
		DoHitEffect( pobStrike->GetOriginatorP() );
		DoHitSound( pobStrike );
	}

	// 
	m_fTimeSinceLastDeflect = 0.0f;

	//CGatso::Stop("CAttackComponent::StartDeflecting");
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::UpdateDeflecting
//! Update the deflecting state
//!
//------------------------------------------------------------------------------------------
void CAttackComponent::UpdateDeflecting( float fTimeDelta )
{
	// If we are still within the countering time let the character select a counter attack
	// If they're not already being punished for a previous bad counter
	if ( m_fStateTime <= m_fCounteringTime && !m_bBadCounterBeingPunished)
	{
		if ( (m_fStateTime + fTimeDelta) > m_fCounteringTime )
		{
			// Log missed counter opportunity
			m_pobCombatEventLogManager->AddEvent(CE_MISSED_COUNTER, m_pobParentEntity, 0 );	
		}

		if ( ( m_fStateTime <= m_fQuickCounteringTime ) && (m_fStateTime + fTimeDelta) > m_fQuickCounteringTime )
		{
			// Log missed counter opportunity
			m_pobCombatEventLogManager->AddEvent(CE_MISSED_KILL_COUNTER, m_pobParentEntity, 0 );	
		}

		// See if we can counter here
		if ( !m_pobParentEntity->IsAI() && m_obAttackTracker.GetRequestedAttackType(m_pobParentEntity->GetInputComponent(), m_eCurrentStance) != AM_NONE )
		{
			CPoint obCheckPosition = m_pobParentEntity->GetPosition();
			obCheckPosition.Y() += 0.5f;
			if ( m_fStateTime <= m_fQuickCounteringTime ) // Check for a kill counter if we're quick enough
			{
				// Check if we're too close to environment
				if (Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition,m_pobAttackDefinition->m_obCounterAttackEnvironmentCheckHalfExtents,0.0f) || Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition,m_pobAttackDefinition->m_obCounterAttackEnvironmentCheckHalfExtents,45.0f))
					m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificSmallKillCounterIndex, m_bAutoCounter);
				else
					m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificKillCounterIndex, m_bAutoCounter);
			}
			else // Otherwise select a normal one
			{
				// Check if we're too close to environment
				if (Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition,m_pobAttackDefinition->m_obCounterAttackEnvironmentCheckHalfExtents,0.0f) || Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition,m_pobAttackDefinition->m_obCounterAttackEnvironmentCheckHalfExtents,45.0f))
					m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificSmallCounterIndex, m_bAutoCounter);
				else
					m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificCounterIndex, m_bAutoCounter);
			}

			// If we have a input pad save the secondar pad direction at this point - only for evades
			if ( ( m_pobParentEntity->GetInputComponent() ) && ( m_obAttackTracker.RequestIsReady() ) && ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
				m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();
		}

		// We let the player request attacks here
		if ( m_obAttackTracker.RequestIsReady() )
		{
			CPoint obPosition = m_pobParentEntity->GetPosition();
			obPosition.Y() += 0.6f; // Shift this up a bit so we don't collide with the floor				
			// Check that we're not too close to a wall and that the requested attack is a valid counter for the attack we have just received
			if ( AttackIsValidCounter(	m_pobStruckStrike->GetAttackDataP()->m_eAttackClass, m_obAttackTracker.GetRequestedAttackClass() ) )
			{
                //StartAttack is about to be called and remove our struck-strike, so before it does we retrieve any projectile-related
				//information that we can from it (if it exists).
				CEntity* pobProjectile = 0;
				if(m_pobStruckStrike && m_pobStruckStrike->GetProjectile() && m_pobStruckStrike->GetProjectile()->IsProjectile())
				{
					//Non-const so we can cast to Object_Projectile below without a double-cast.
					pobProjectile = const_cast<CEntity*>(m_pobStruckStrike->GetProjectile());
				}

				// If we successfully generated a strike...
				if ( GenerateMyStrike( true, false ) && StartAttack()) 
				{
					m_iDepthInDeflecting = 0;

					// If this was a strike from a projectile, we need to let it know that it got countered
					if (pobProjectile)
					{
						Message obMessage(msg_combat_countered);
						obMessage.SetEnt("Counterer",m_pobParentEntity);
						pobProjectile->GetMessageHandler()->Receive(obMessage);
						Object_Projectile* pobProjCast = static_cast<Object_Projectile*>(pobProjectile);
						pobProjCast->m_bHasBeenCountered = true;

						if (pobProjCast->GetShooter()->IsBoss())
						{
							((Boss*)pobProjCast->GetShooter())->NotifyProjectileCountered(pobProjCast);
						}
					}

					// Let the Lua state know that we have countered
					CMessageSender::SendEmptyMessage( "msg_combat_countered", m_pobParentEntity->GetMessageHandler() );

					// Drop out here
					return;
				}
			}
			
			// if we are here then we should clear the requested attack
			m_obAttackTracker.ClearRequested();
		}
	}
	else
	{
		//If we're not in the counter window anymore, then we've missed our opportunity to counter. Let any projectiles know.
		if (m_pobStruckStrike && m_pobStruckStrike->GetProjectile() && m_pobStruckStrike->GetProjectile()->IsProjectile())
		{
			CEntity* pobProjectile = const_cast<CEntity*>(m_pobStruckStrike->GetProjectile());
			Object_Projectile* pobProjCast = static_cast<Object_Projectile*>(pobProjectile);
			//If we haven't countered it (above), and we haven't already sent this message to it, then send the message to
			//let it know that the character failed to counter the projectile (so it can do it's failed-to-counter stuff, e.g. explode).
			if((pobProjCast->m_bHasBeenCountered == false) && (pobProjCast->m_bFailedToCounterFlagged == false))
			{
				Message obMessage(msg_combat_missedprojectilecounter);
				obMessage.SetEnt("Counterer",m_pobParentEntity);
				pobProjectile->GetMessageHandler()->Receive(obMessage);
				pobProjCast->m_bFailedToCounterFlagged = true;
			}
		}
	}

	// Drop out as soon as we have passed the tiem through which we are incapacitated
	if ( m_fStateTime > m_fIncapacityTime )	
		EndDeflecting();
}

//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::EndDeflecting
//! Tidy up the deflecting state
//!
//------------------------------------------------------------------------------------------
void CAttackComponent::EndDeflecting( void )
{
	// Set the state back to standard
	SetState( CS_RECOVERING );
	m_fIncapacityTime = 0.0f;
	m_eRecoveryType = RC_DEFLECT;

	// Clear up the strike we have been struck by
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartString
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackComponent::StartString()
{ // Return type changed to bool to propogate the success of starting an attack string down to StartNewAttack (and consequently, Lua)
	//CGatso::Start("CAttackComponent::StartString");

	// Make sure that the attackee list is cleared out
	ClearAttackeeList();

		// If we successfully generated a strike...
		if ( GenerateMyStrike( false, false ) && !m_bBadCounterBeingPunished) 
		{
			return StartAttack();
		}
		else 
		{
			return false;
		}

	//CGatso::Stop("CAttackComponent::StartString");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndString
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::EndString()
{
	// Clear the depth
	m_iStringDepth = 0;
	m_iSyncdDepth = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::ResetAttack
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::ResetAttack(bool bResetCamera)
{
	// Indicate no strike selected
	m_bPreStrikeRequested = false;

	// What strike window have we responded to
	m_iStrikeWindow = 0;

	// Clear the strikelist
	m_obStrikeList.clear();
	m_obPreStrikeList.clear();

	// Clear the strike landed flag
	m_bStrikeLanded = false;

	// Clear the flag to say whether we successfully KOd
	m_bKOSuccessful = false;

	// Delete the strike that we produced
	if (m_pobMyCurrentStrike)
	{
		//ntPrintf("%s: resetting attack pointers %s.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString());
		NT_DELETE_CHUNK( Mem::MC_MISC, m_pobMyCurrentStrike );
		m_pobMyCurrentStrike = 0;
	}

	// If we have a hit counter we need to tell it we are finishing an attack
	if ( GetHitCounter() )
		GetHitCounter()->StrikeFinished();

	// Cancel any combat cam
	if(m_iCombatCam >= 0 && bResetCamera)
		m_iCombatCam = m_pobAttackDefinition->m_pobCameras->DeactivateCombatCam(m_iCombatCam, true);

	// free any effects we have
	if (m_pobAttackDefinition->m_pCombatEffectsDef)
		m_pobAttackDefinition->m_pCombatEffectsDef->ReleaseEffectsTrigger( m_iCurrEffectsTrigger );	

	// We may have been fiddling with time stuff
	if ( m_pobAttackSpecial )
		m_pobAttackSpecial->ClearEntityTimeMultipliers();
}

void CAttackComponent::ForceDoneSuperStyleSafetyTransition()
{
	// Force a floored on both entities
	if (m_pobSuperStyleSafetyAttacker)
		m_pobSuperStyleSafetyAttacker->GetAttackComponent()->StartFlooredState();
	if (m_pobSuperStyleSafetyReceiver)
		m_pobSuperStyleSafetyReceiver->GetAttackComponent()->StartFlooredState();

	// And we're done - clean up admin vars
	if (m_pobSuperStyleSafetyAttacker)
		m_pobSuperStyleSafetyAttacker->GetAttackComponent()->m_bIsDoingSuperStyleSafetyTransition = false;
	if (m_pobSuperStyleSafetyReceiver)
		m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_bIsDoingSuperStyleSafetyTransition = false;
	if (m_pobSuperStyleSafetyReceiver)
		m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_pobSuperStyleSafetyAttacker = m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_pobSuperStyleSafetyReceiver = 0;
	if (m_pobSuperStyleSafetyReceiver)
		m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_bSuperStyleSafetyAttackerDone = m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_bSuperStyleSafetyReceiverDone = false;

	m_pobSuperStyleSafetyAttacker = m_pobSuperStyleSafetyReceiver = 0;
	m_bSuperStyleSafetyAttackerDone = m_bSuperStyleSafetyReceiverDone = false;
}

void CAttackComponent::NotifyDoneSuperStyleSafetyTransition(const CEntity* pobEnt)
{
	// Who is this?
	if (pobEnt == m_pobSuperStyleSafetyAttacker)
		m_bSuperStyleSafetyAttackerDone = true;
	if (pobEnt == m_pobSuperStyleSafetyReceiver)
		m_bSuperStyleSafetyReceiverDone = true;
		
	// As soon as one is finished, do the attack
	if (m_bSuperStyleSafetyAttackerDone || m_bSuperStyleSafetyReceiverDone)
	{
		m_bSuperStyleSafetyReceiverDone = true;

		// If this wasn't animated
		if (m_pobMyCurrentStrike->GetAttackDataP()->m_obSuperSafeRotationAnimAttacker.IsNull() && m_pobMyCurrentStrike->GetAttackDataP()->m_obSuperSafeRotationAnimReceiver.IsNull())
		{
			// Force the relative transform to be recalculated
			m_iSyncdDepth = 0;
		}
		
		m_pobSuperStyleSafetyAttacker->GetAttackComponent()->SetAttackCollisionBreak( false );
		m_pobSuperStyleSafetyReceiver->GetAttackComponent()->SetAttackCollisionBreak( false );
		
		// Call StartAttack again so we do the proper superstyle strike now
		StartAttack();

		Message obAttackerMessage(msg_combat_safetytransitiondone_attacker);
		m_pobSuperStyleSafetyAttacker->GetMessageHandler()->Receive(obAttackerMessage);
		Message obReceiverMessage(msg_combat_safetytransitiondone_receiver);
		m_pobSuperStyleSafetyReceiver->GetMessageHandler()->Receive(obReceiverMessage);

		// And we're done - clean up admin vars
		m_pobSuperStyleSafetyAttacker->GetAttackComponent()->m_bIsDoingSuperStyleSafetyTransition = false;
		m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_bIsDoingSuperStyleSafetyTransition = false;
		m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_pobSuperStyleSafetyAttacker = m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_pobSuperStyleSafetyReceiver = 0;
		m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_bSuperStyleSafetyAttackerDone = m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_bSuperStyleSafetyReceiverDone = false;
		m_pobSuperStyleSafetyAttacker = m_pobSuperStyleSafetyReceiver = 0;
		m_bSuperStyleSafetyAttackerDone = m_bSuperStyleSafetyReceiverDone = false;

		
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GetCurrentAttackTurnOffGravity
*
*	DESCRIPTION	
*
***************************************************************************************************/
bool CAttackComponent::GetCurrentAttackTurnOffGravity() const
{
	ntAssert( m_pobMyCurrentStrike );

	return m_pobMyCurrentStrike->GetAttackDataP()->m_bTurnOffGravity;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartAttack
*
*	DESCRIPTION	
*
***************************************************************************************************/
bool CAttackComponent::StartAttack( void )
{
	//CGatso::Start("CAttackComponent::StartAttack");

	m_obTargetPointOffset.Clear();

	// Make sure we don'd have any 'struck-strikes' hanging about
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;

	// Make damn sure we have one of these
	ntError_p( m_pobMyCurrentStrike, ( "This is bad - how did this happen?" ) );

	// Reset these depth counts
	m_iDepthInRecoiling = 0;
	m_iDepthInDeflecting = 0;

	// Make sure we initialise start volume descriptor strike pointers
	for (ntstd::List<CombatPhysicsStrikeVolumeDescriptor*, Mem::MC_ENTITY>::const_iterator obItDesc = m_pobMyCurrentStrike->GetAttackDataP()->m_obStrikeVolumeDescriptors.begin();
		obItDesc != m_pobMyCurrentStrike->GetAttackDataP()->m_obStrikeVolumeDescriptors.end();
		obItDesc++ )
	{
		// Need to clear it when we start a new attack because sometimes although a new CStrike object will be created, it can be at the same addr as the one before
		(*obItDesc)->GetMovementDescriptor()->Initialise(m_pobParentEntity);
		(*obItDesc)->SetLastStrikePointer( 0 );
	}

	// Log me starting an attack
	m_pobCombatEventLogManager->AddEvent(CE_STARTED_ATTACK, m_pobMyCurrentStrike->GetTargetP(), (void*)m_pobMyCurrentStrike->GetAttackDataP());
	
	//ntPrintf("StartAttack: %s\n", ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()));

	// If this attack is not syncd we need to reset the sync' depth
	if ( m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() )
		m_iSyncdDepth = 0;

	// If we have a hit counter we need to tell it we are starting an attack
	if ( GetHitCounter() )
		GetHitCounter()->StrikeStarted();

	// Increase the string depth
	m_iStringDepth++;

	// Give the audio system a heads-up on the workings of the combat system
	//CAudioManager::GlobalTriggerF( "On%sAttack%d",  m_pobParentEntity->GetType().c_str(), m_iStringDepth );

	// Reset this so we start the next attacks combat cam cleanly
	m_bCombatCamStartedThisAttack = false;

	if ( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_EVADE )
	{
		m_bEvadeCombo = true;
	}

	// We may need to set the fact that we are in aerial combat
	if ( ( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackMovementType == AMT_GROUND_TO_AIR ) )
	{
		m_bAerialCombo = true;

		// Log that I managed to counter
		m_pobCombatEventLogManager->AddEvent(CE_STARTED_AERIAL, m_pobMyCurrentStrike->GetTargetP() );

		// Aerials no long hold previous, so gotta be careful before we send a message
		if (m_pobMyCurrentStrike->GetTargetP())
		{
			// Send message to target that they are in aerial
			CMessageSender::SendEmptyMessage("msg_combat_aerialstarted_victim",m_pobMyCurrentStrike->GetTargetP()->GetMessageHandler());
			// Casting away constness, sorry!
			CEntity* pobEnt = const_cast< CEntity* >(m_pobMyCurrentStrike->GetTargetP());
			pobEnt->GetAttackComponent()->AerialTargetingNotify();

			CMessageSender::SendEmptyMessage("msg_combat_aerialstarted_attacker",m_pobParentEntity->GetMessageHandler());

			// Deactivate any active combat cameras.
			m_iCombatCam = m_pobAttackDefinition->m_pobCameras->DeactivateCombatCam(m_iCombatCam, true);
			// And stop any transitions
			CoolCamera* pCoolCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_AerialCombo(*CamMan::GetPrimaryView(), *m_pobParentEntity, *m_pobMyCurrentStrike->GetTargetP());
			//CamTrans_POIRotDef* pDef = NT_NEW CamTrans_POIRotDef(2.0f);
			//pDef->SetPriority(100);
			//pCoolCam->SetEndingTransition(pDef);
			CamMan::GetPrimaryView()->AddCoolCamera(pCoolCam);

			// Remember the aerial coolcam ID
			m_iAerialCoolCamID = pCoolCam->GetID();

			// Must have aerial details defined
			ntError( m_pobAttackDefinition->m_pobAerialDetails );

			m_obTargetPointOffset = m_pobAttackDefinition->m_pobAerialDetails->m_obAerialGrappleTargetOffset * m_pobParentEntity->GetMatrix();
		}
	}

	// This may be a syncronised attack in which case we need to do some cunningness - make sure we have a specific reaction animation though
	if ( !m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() )
	{
		// Make sure that we have a target for this attack
		if ( !m_pobMyCurrentStrike->GetTargetP() )
		{
			ntPrintf( "AHHH %s tried to sync attack %s with NO TARGET! Bailing on this attack messily.\n", m_pobParentEntity->GetName().c_str(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString() );
			//CGatso::Stop("CAttackComponent::StartAttack");
			ResetAttack(true);
			return false;
		}

		// If we're starting a grab strike and we're not safe to continue a superstyle, take the opportunity to do a safety transition before we do anything else, then return to this method later when we're safe
		// We rely upon GenerateMyStrike only allowing AC_GRAB_GOTOs to start if the player is within a start volume
		// We only check if we're already in the continue volume, in which case we can continue without a transition and we just play a linkup anim
		if (m_pobMyCurrentStrike->GetAttackDataP()->m_bNeedsSuperStyleSafety && SuperStyleSafetyManager::Exists() &&
			!SuperStyleSafetyManager::Get().PointInSuperStyleContinueVolume(m_pobParentEntity->GetPosition()) && !IsInSuperStyleSafetyTransition())
		{
			if (!m_pobSuperStyleStartVolume)
			{
				ntPrintf("SuperStyleSafety: Somehow got to the point of start a super without a start volume. Bailing potentially messily.\n");
				ResetAttack(true);
				//CGatso::Stop("CAttackComponent::StartAttack");
				return false;
			}

			// Prepare a safety transition
			SuperStyleSafetyTransitionDef obSafetyMovementDefAttackerRotate, obSafetyMovementDefReceiverRotate;
			SuperStyleSafetyTransitionDef obSafetyMovementDefAttackerTranslate, obSafetyMovementDefReceiverTranslate;
			obSafetyMovementDefAttackerRotate.SetDebugNames( "Attacker Safety Move Rotate", "SuperStyleSafetyTransitionDef" );
			obSafetyMovementDefReceiverRotate.SetDebugNames( "Victim Safety Move Rotate", "SuperStyleSafetyTransitionDef" );
			obSafetyMovementDefAttackerTranslate.SetDebugNames( "Attacker Safety Move Translate", "SuperStyleSafetyTransitionDef" );
			obSafetyMovementDefReceiverTranslate.SetDebugNames( "Victim Safety Move Translate", "SuperStyleSafetyTransitionDef" );
			
			// Set the anim
			obSafetyMovementDefAttackerRotate.m_pobTransitionAnimation = m_pobParentEntity->GetAnimator()->CreateAnimation( m_pobMyCurrentStrike->GetAttackDataP()->m_obSuperSafeRotationAnimAttacker );
			obSafetyMovementDefReceiverRotate.m_pobTransitionAnimation = (const_cast<CEntity*>( m_pobMyCurrentStrike->GetTargetP() ))->GetAnimator()->CreateAnimation( m_pobMyCurrentStrike->GetAttackDataP()->m_obSuperSafeRotationAnimReceiver );
			obSafetyMovementDefAttackerTranslate.m_pobTransitionAnimation = m_pobParentEntity->GetAnimator()->CreateAnimation( m_pobMyCurrentStrike->GetAttackDataP()->m_obSuperSafeTranslationAnimAttacker );
			obSafetyMovementDefReceiverTranslate.m_pobTransitionAnimation = (const_cast<CEntity*>( m_pobMyCurrentStrike->GetTargetP() ))->GetAnimator()->CreateAnimation( m_pobMyCurrentStrike->GetAttackDataP()->m_obSuperSafeTranslationAnimReceiver );

			// Where is it safe for us to continue?
			CPoint obSafePoint;
			switch ( m_pobMyCurrentStrike->GetAttackDataP()->m_eSyncTransform )
			{
			case CRM_ATTACKER_ROOT_INIT:
				obSafePoint = SuperStyleSafetyManager::Get().GetSuperStyleSafeContinuePoint(m_pobParentEntity->GetPosition(),m_pobSuperStyleStartVolume);
				break;
			case CRM_RECEIVER_ROOT_INIT:
				obSafePoint = SuperStyleSafetyManager::Get().GetSuperStyleSafeContinuePoint(m_pobMyCurrentStrike->GetTargetP()->GetPosition(),m_pobSuperStyleStartVolume);
				break;
			case CRM_ORIGIN:
				obSafePoint = SuperStyleSafetyManager::Get().GetSuperStyleSafeContinuePoint(CPoint(0.0,0.0,0.0),m_pobSuperStyleStartVolume);
				break;
			default:
				ntAssert( 0 );
				break;
			}
			
			// If we have an anim to play, the safe points given to the movement should be the same, as they'll be syncing to a relative transform
			if (obSafetyMovementDefAttackerRotate.m_pobTransitionAnimation && obSafetyMovementDefReceiverRotate.m_pobTransitionAnimation &&
				obSafetyMovementDefAttackerTranslate.m_pobTransitionAnimation && obSafetyMovementDefReceiverTranslate.m_pobTransitionAnimation)
			{
				obSafetyMovementDefAttackerRotate.m_obSafePoint = obSafetyMovementDefReceiverRotate.m_obSafePoint = obSafePoint;
				obSafetyMovementDefAttackerTranslate.m_obSafePoint = obSafetyMovementDefReceiverTranslate.m_obSafePoint = obSafePoint;
			}
			else // They need their own points to move to absolutely
			{
				obSafetyMovementDefAttackerRotate.m_obSafePoint = CPoint(obSafePoint.X(),m_pobParentEntity->GetPosition().Y(),obSafePoint.Z());
				obSafetyMovementDefReceiverRotate.m_obSafePoint = CPoint(obSafePoint.X(),m_pobMyCurrentStrike->GetTargetP()->GetPosition().Y(),obSafePoint.Z()) + (m_pobMyCurrentStrike->GetTargetP()->GetPosition() - m_pobParentEntity->GetPosition());
			}
			
			obSafetyMovementDefReceiverRotate.m_pobRelativeTransform = obSafetyMovementDefAttackerRotate.m_pobRelativeTransform = m_pobSyncdTransform;
			obSafetyMovementDefReceiverTranslate.m_pobRelativeTransform = obSafetyMovementDefAttackerTranslate.m_pobRelativeTransform = m_pobSyncdTransform;

			// This attack component needs to be notified when both controllers are finished and ready for the attack proper
			obSafetyMovementDefAttackerRotate.m_pobNotifyThis = obSafetyMovementDefReceiverRotate.m_pobNotifyThis = this;
			obSafetyMovementDefAttackerTranslate.m_pobNotifyThis = obSafetyMovementDefReceiverTranslate.m_pobNotifyThis = this;
			
			// Pass a strike over to the receiver
			if (!GenerateDirectStrike( m_pobMyCurrentStrike->GetTargetP(), false, true, true /*Use super safety transition flag*/ ))
			{
				ntPrintf( "AHHH %s tried to super safety %s but couldn't generate a strike! Check your strike distances in CAttackData. This attack will end in a silly way.\n", m_pobParentEntity->GetName().c_str(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString() );
				//CGatso::Stop("CAttackComponent::StartAttack");
				ResetAttack(true);
				return false;
			}

			// Who's who? Make sure both this (attacker) and my receiver are equally well informed
			this->m_pobSuperStyleSafetyAttacker = m_pobParentEntity;
			this->m_pobSuperStyleSafetyReceiver = (const_cast<CEntity*>( m_pobMyCurrentStrike->GetTargetP() ));
			m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_pobSuperStyleSafetyAttacker = m_pobSuperStyleSafetyAttacker;
			m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_pobSuperStyleSafetyReceiver = m_pobSuperStyleSafetyReceiver;						
			
			// Set control flags so we don't do anything while it's going on
			m_pobSuperStyleSafetyAttacker->GetAttackComponent()->m_bIsDoingSuperStyleSafetyTransition = true;
			m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_bIsDoingSuperStyleSafetyTransition = true;
			
			// Kick off the transition
			if (obSafetyMovementDefAttackerRotate.m_pobTransitionAnimation && obSafetyMovementDefReceiverRotate.m_pobTransitionAnimation)
			{
				obSafetyMovementDefAttackerRotate.m_bTranslate = obSafetyMovementDefReceiverRotate.m_bTranslate = false;
				obSafetyMovementDefAttackerRotate.m_bNotifyAtEnd = obSafetyMovementDefReceiverRotate.m_bNotifyAtEnd = false;
				obSafetyMovementDefAttackerTranslate.m_bRotate = obSafetyMovementDefReceiverTranslate.m_bRotate = false;

				m_pobSuperStyleSafetyAttacker->GetMovement()->BringInNewController(obSafetyMovementDefAttackerRotate, CMovement::DMM_SOFT_RELATIVE, 0.05f);			
				m_pobSuperStyleSafetyReceiver->GetMovement()->BringInNewController(obSafetyMovementDefReceiverRotate, CMovement::DMM_SOFT_RELATIVE, 0.05f);
				m_pobSuperStyleSafetyAttacker->GetMovement()->AddChainedController(obSafetyMovementDefAttackerTranslate, CMovement::DMM_SOFT_RELATIVE, 0.05f);			
				m_pobSuperStyleSafetyReceiver->GetMovement()->AddChainedController(obSafetyMovementDefReceiverTranslate, CMovement::DMM_SOFT_RELATIVE, 0.05f);
			}
			else
			{
				m_pobSuperStyleSafetyAttacker->GetMovement()->BringInNewController(obSafetyMovementDefAttackerRotate, CMovement::DMM_STANDARD, 0.05f);			
				m_pobSuperStyleSafetyReceiver->GetMovement()->BringInNewController(obSafetyMovementDefReceiverRotate, CMovement::DMM_STANDARD, 0.05f);
			}
		}
		else // Either we're doing a normal sync attack, or we need to do a super style safe linkup
		{
			// We may not be doing super safety, but if we may have needed to for this move, we need to make sure the hold and the strike anims are linked up nicely when we're not doing the proper safety transition
			// In place of a proper safety transition, we play a link anim that doesn't do any rotation or translation, all it does is make the hold and the strike anims line up
			if (m_pobMyCurrentStrike->GetAttackDataP()->m_bNeedsSuperStyleSafety && 
				!m_bSuperStyleSafetyAttackerDone && !m_bSuperStyleSafetyReceiverDone) // These stop us from doing this at the end of a proper safety transition
			{
				// Even though we're just playing an anim, we'll use the safety transition functionaliy cos it's there and it stops cameras and attacks from starting and finishing too early
				SuperStyleSafetyTransitionDef obLinkupMovementDefAttacker, obLinkupMovementDefReceiver;
				obLinkupMovementDefAttacker.SetDebugNames( "Attacker Linkup Move", "SuperStyleSafetyTransitionDef" );
				obLinkupMovementDefReceiver.SetDebugNames( "Victim Linkup Move", "SuperStyleSafetyTransitionDef" );
								
				// Set the anim
				obLinkupMovementDefAttacker.m_pobTransitionAnimation = m_pobParentEntity->GetAnimator()->CreateAnimation(m_pobMyCurrentStrike->GetAttackDataP()->m_obSuperSafeLinkupAnimAttacker);
				obLinkupMovementDefReceiver.m_pobTransitionAnimation = (const_cast<CEntity*>( m_pobMyCurrentStrike->GetTargetP() ))->GetAnimator()->CreateAnimation(m_pobMyCurrentStrike->GetAttackDataP()->m_obSuperSafeLinkupAnimReceiver);
								
				obLinkupMovementDefReceiver.m_pobRelativeTransform = obLinkupMovementDefAttacker.m_pobRelativeTransform = m_pobSyncdTransform;
				obLinkupMovementDefReceiver.m_pobRelativeTransform = obLinkupMovementDefAttacker.m_pobRelativeTransform = m_pobSyncdTransform;
				// Set a safe point as where I am right now
				switch ( m_pobMyCurrentStrike->GetAttackDataP()->m_eSyncTransform )
				{
				case CRM_ATTACKER_ROOT_INIT:
					obLinkupMovementDefReceiver.m_obSafePoint = obLinkupMovementDefAttacker.m_obSafePoint = m_pobParentEntity->GetPosition();
					break;
				case CRM_RECEIVER_ROOT_INIT:
					obLinkupMovementDefReceiver.m_obSafePoint = obLinkupMovementDefAttacker.m_obSafePoint = m_pobMyCurrentStrike->GetTargetP()->GetPosition();
					break;
				case CRM_ORIGIN:
					obLinkupMovementDefReceiver.m_obSafePoint = obLinkupMovementDefAttacker.m_obSafePoint = CPoint( CONSTRUCT_CLEAR );
					break;
				default:
					ntAssert( 0 );
					break;
				}

				// This attack component needs to be notified when both controllers are finished and ready for the attack proper
				obLinkupMovementDefAttacker.m_pobNotifyThis = obLinkupMovementDefReceiver.m_pobNotifyThis = this;
					
				// Who's who? Make sure both this (attacker) and my receiver are equally well informed
				this->m_pobSuperStyleSafetyAttacker = m_pobParentEntity;
				this->m_pobSuperStyleSafetyReceiver = (const_cast<CEntity*>( m_pobMyCurrentStrike->GetTargetP() ));
				m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_pobSuperStyleSafetyAttacker = m_pobSuperStyleSafetyAttacker;
				m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_pobSuperStyleSafetyReceiver = m_pobSuperStyleSafetyReceiver;						
				
				// Set control flags so we don't do anything else while it's going on
				m_pobSuperStyleSafetyAttacker->GetAttackComponent()->m_bIsDoingSuperStyleSafetyTransition = true;
				m_pobSuperStyleSafetyReceiver->GetAttackComponent()->m_bIsDoingSuperStyleSafetyTransition = true;
				
				// Pass a strike over to the receiver
				if (!GenerateDirectStrike( m_pobMyCurrentStrike->GetTargetP(), false, true, true /*Use super safety transition flag*/  ))
				{
					ntPrintf( "AHHH %s tried to super safety %s but couldn't generate a strike!\nCheck your strike distances in CAttackData.\nThis attack will end in a silly way.", m_pobParentEntity->GetName().c_str(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString() );
					//CGatso::Stop("CAttackComponent::StartAttack");
					ResetAttack(true);
					return false;
				}

				// Kick off the transition - no rotation or translation because this is just the linkup
				obLinkupMovementDefAttacker.m_bTranslate = obLinkupMovementDefReceiver.m_bTranslate = false;
				obLinkupMovementDefAttacker.m_bRotate = obLinkupMovementDefReceiver.m_bRotate = false;
				m_pobSuperStyleSafetyAttacker->GetMovement()->BringInNewController(obLinkupMovementDefAttacker, CMovement::DMM_SOFT_RELATIVE, 0.05f);			
				m_pobSuperStyleSafetyReceiver->GetMovement()->BringInNewController(obLinkupMovementDefReceiver, CMovement::DMM_SOFT_RELATIVE, 0.05f);
			}
			else
			{
				// Make sure that if this is not our first attack we have a pointer to an initial transform if necessary
				ntAssert( ( m_iSyncdDepth == 0 ) || ( m_pobSyncdTransform ) );

				//ntPrintf("%s: SyncAttack - on %s in %s\n", ObjectDatabase::Get().GetNameFromPointer(this->m_pobParentEntity), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetTargetP()), g_apcCombatStateTable[m_pobMyCurrentStrike->GetOriginatorP()->GetAttackComponent()->m_eCombatState]);

				// Create a movement controller for each of the characters - the attacker
				SimpleRelativeTransitionDef obAttackMoveDef;
				obAttackMoveDef.SetDebugNames( "Attacker sync", "SimpleRelativeTransitionDef" );
				obAttackMoveDef.m_pobAnimation = m_pobParentEntity->GetAnimator()->CreateAnimation(m_pobMyCurrentStrike->GetAttackDataP()->m_obAttackAnimName);
				obAttackMoveDef.m_fMovementDuration =  m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime( m_fAttackScalar );
				obAttackMoveDef.m_bOwnsTransform = false;
				obAttackMoveDef.m_bInteractWithSyncdTransform = m_pobMyCurrentStrike->GetAttackDataP()->m_bInteractWithSyncTransform;
				obAttackMoveDef.m_fCollisionCheckDistance = m_pobMyCurrentStrike->GetAttackDataP()->m_fInteractiveCollisionCheckDistance;
				obAttackMoveDef.m_iCollisionCheckHeightCount = m_pobMyCurrentStrike->GetAttackDataP()->m_iInteractiveCollisionCheckHeightCount;
				obAttackMoveDef.m_fCollisionCheckHeightInterval = m_pobMyCurrentStrike->GetAttackDataP()->m_fInteractiveCollisionCheckHeightInterval;
				obAttackMoveDef.m_fInteractiveCollisionCheckStartHeight = m_pobMyCurrentStrike->GetAttackDataP()->m_fInteractiveCollisionCheckStartHeight;
				obAttackMoveDef.m_fMaxRotationPerSecond = m_pobMyCurrentStrike->GetAttackDataP()->m_obMaxInteractiveSyncRotationSpeed * DEG_TO_RAD_VALUE;
				obAttackMoveDef.m_obMovementSpeed = m_pobMyCurrentStrike->GetAttackDataP()->m_obInteractiveSyncTranslateAbsoluteSpeed;
				obAttackMoveDef.m_bReverseInteractiveStickInput = m_pobMyCurrentStrike->GetAttackDataP()->m_bReverseInteractiveStickInput;

				// ...and the attackee
				SimpleRelativeTransitionDef obReceiverMoveDef;
				obReceiverMoveDef.SetDebugNames( "Victim sync", "SimpleRelativeTransitionDef" );
				obReceiverMoveDef.m_bOwnsTransform = false;
				obReceiverMoveDef.m_pobAnimation = (const_cast<CEntity*>( m_pobMyCurrentStrike->GetTargetP() ))->GetAnimator()->CreateAnimation(m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim);

				// Kick off the syncronised movement - create a transform if necessary
				CEntity* pobLocator = 0;
				if ( m_iSyncdDepth == 0 )
				{
					// If we have a transform hanging about we should kill it
					if ( m_pobSyncdTransform )
					{
						m_pobSyncdTransform->RemoveFromParent();
						NT_DELETE_CHUNK( Mem::MC_MISC, m_pobSyncdTransform );
						m_pobSyncdTransform = 0;
					}

					// Switch on the relative transform type
					CMatrix obSyncMtx;
					CPoint obTranslationCompensation( CONSTRUCT_CLEAR );
					switch ( m_pobMyCurrentStrike->GetAttackDataP()->m_eSyncTransform )
					{
					case CRM_ATTACKER_ROOT_INIT:
						m_pobSyncdTransform = NT_NEW_CHUNK( Mem::MC_MISC ) Transform();

						obSyncMtx = m_pobParentEntity->GetMatrix();

						m_pobSyncdTransform->SetLocalMatrix( obSyncMtx );
						CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobSyncdTransform );
						break;

					case CRM_RECEIVER_ROOT_INIT:
						m_pobSyncdTransform = NT_NEW_CHUNK( Mem::MC_MISC ) Transform();

						obSyncMtx = m_pobMyCurrentStrike->GetTargetP()->GetMatrix();

						m_pobSyncdTransform->SetLocalMatrix( obSyncMtx );
						CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobSyncdTransform );
						break;

					case CRM_ORIGIN:
						// This allows attacks to be linked straight to the origin.
						m_pobSyncdTransform = NT_NEW_CHUNK( Mem::MC_MISC ) Transform();

						obSyncMtx = CMatrix( CONSTRUCT_IDENTITY );

						m_pobSyncdTransform->SetLocalMatrix( obSyncMtx );
						CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobSyncdTransform );
						break;

					case CRM_TO_LOCATOR:
						// This allows attacks to be linked straight to a point in the world
						m_pobSyncdTransform = NT_NEW_CHUNK( Mem::MC_MISC ) Transform();

						pobLocator = CEntityManager::Get().FindEntity( m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncToLocatorName );
						if (pobLocator)
						{
							// Use matrix of locator
							obSyncMtx = CMatrix( m_pobParentEntity->GetRotation(), pobLocator->GetPosition() );
						}
						else
						{
							// Otherwise warn and default to attacker relative
							user_warn_p( 0, ("%s: Couldn't find locator %s to sync to.\n", m_pobParentEntity->GetName().c_str(), m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncToLocatorName.GetDebugString() ));
                            obSyncMtx = m_pobParentEntity->GetMatrix();
						}						

						m_pobSyncdTransform->SetLocalMatrix( obSyncMtx );
						CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobSyncdTransform );

						break;
					
					default:
						ntAssert( 0 );
						break;
					}
				}

				CEntity* pobTarget = const_cast<CEntity*>( m_pobMyCurrentStrike->GetTargetP() );

				// Apply correction to the anim so the relative transform doesn't jump betwen moves
				// Offset the relative transform so when we start the next anim, we appear to be in the same position as we are now
				CMatrix obSyncMtx = m_pobSyncdTransform->GetLocalMatrix();
				CPoint obPositionNow, obRelTranPositionNow, obPositionAnimStart;
				CDirection obMoveTo;
				CAnimationPtr pobLinkup, pobSafetyRotate, pobSafetyTranslate;
				CDirection obLinkupEndTranslation, obSafetyEndTranslation;
				CDirection obDiff( CONSTRUCT_CLEAR );
				switch ( m_pobMyCurrentStrike->GetAttackDataP()->m_eSyncTransform )
				{
				case CRM_ATTACKER_ROOT_INIT:
					// Get where I am now, and where I will be at the start of the next anim
					obPositionNow = m_pobParentEntity->GetPosition();
					obPositionAnimStart = obAttackMoveDef.m_pobAnimation->GetRootTranslationAtTime(0.0f,m_pobParentEntity->GetHierarchy(),true) * obSyncMtx;
					obDiff = CDirection(obPositionAnimStart - obPositionNow); // Get the diff
					
					// Sub this from the translation of the relative transition
					obRelTranPositionNow = obSyncMtx.GetTranslation();
					obRelTranPositionNow -= obDiff; 
					
					// Set it back
					obSyncMtx.SetTranslation(obRelTranPositionNow); 
                    m_pobSyncdTransform->SetLocalMatrix( obSyncMtx );
                       
					break;

				case CRM_RECEIVER_ROOT_INIT:
					// Get where I am now, and where I will be at the start of the next anim
					obPositionNow = m_pobMyCurrentStrike->GetTargetP()->GetPosition();
					obPositionAnimStart = obReceiverMoveDef.m_pobAnimation->GetRootTranslationAtTime(0.0f,m_pobParentEntity->GetHierarchy(),true) * obSyncMtx;
					obDiff = CDirection(obPositionAnimStart - obPositionNow); // Get the diff
					
					// Sub this from the translation of the relative transition
					obRelTranPositionNow = obSyncMtx.GetTranslation();
					obRelTranPositionNow -= obDiff; 
					
					// Set it back
					obSyncMtx.SetTranslation(obRelTranPositionNow); 
                    m_pobSyncdTransform->SetLocalMatrix( obSyncMtx );

					break;

				case CRM_ORIGIN:
					obSyncMtx = CMatrix( CONSTRUCT_IDENTITY );
					m_pobSyncdTransform->SetLocalMatrix( obSyncMtx );
					break;
				case CRM_TO_LOCATOR:
					// We expect a jump to the locator here, so let it happen
					// Dont want to reset to the locator here as if we're in a chain of sync attacks they'll keep resetting back to the same point rather than nicely starting at the locator and continuing on naturally from there
					break;
				default:
					ntAssert( 0 );
					break;
				}

				obAttackMoveDef.m_pobRelativeTransform = m_pobSyncdTransform;
				obReceiverMoveDef.m_pobRelativeTransform = m_pobSyncdTransform;
				
				// Set up the scaled time of the reaction - how much is the other animation scaled by?
				const CAnimationHeader* pobAnim = obAttackMoveDef.m_pobAnimation->GetAnimationHeader();
				float fAnimScale = 1.0f;
				if (pobAnim)
				{
					fAnimScale = obAttackMoveDef.m_fMovementDuration / pobAnim->GetDuration();
				}
				else
				{
					ntError_p(false,("Couldn't find attaker's anim when starting sync attack."));
				}
				pobAnim = obReceiverMoveDef.m_pobAnimation->GetAnimationHeader();
				if (pobAnim)
				{
					obReceiverMoveDef.m_fMovementDuration = fAnimScale * pobAnim->GetDuration();
				}
				else
				{
					ntError_p(false,("Couldn't find attakee's anim when starting sync attack."));
					obReceiverMoveDef.m_fMovementDuration = 1.0f;
				}

				// Pass a strike over to the receiver
				if (!GenerateDirectStrike( m_pobMyCurrentStrike->GetTargetP(), false, true ))
				{
					ntPrintf( "AHHH %s tried to sync attack %s but couldn't generate a strike!\nCheck your strike distances in CAttackData.\nThis attack will end in a silly way.", m_pobParentEntity->GetName().c_str(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString() );
					//CGatso::Stop("CAttackComponent::StartAttack");
					ResetAttack(true);
					return false;
				}

				// Set up all the data we need for our movement
				SyncdMovementDefinition* pobSyncDef = NT_NEW_CHUNK( Mem::MC_MISC ) SyncdMovementDefinition;

				m_pobParentEntity->GetAnimator()->RemoveAllAnimations();
				m_pobParentEntity->GetAnimator()->ClearAnimWeights();
				m_pobParentEntity->GetMovement()->ClearControllers();
				pobTarget->GetAnimator()->RemoveAllAnimations();
				pobTarget->GetAnimator()->ClearAnimWeights();
				pobTarget->GetMovement()->ClearControllers();

				pobSyncDef->SetMovement(	m_pobParentEntity, 
											obAttackMoveDef,
										0.0f,
										false );
				pobSyncDef->SetMovement(	pobTarget, 
											obReceiverMoveDef,
										0.0f,
										false );

				// Go, go, go
				SyncdMovement::Get().InitiateSyncronisedMovement( pobSyncDef );

				// Tell them they are going to get struck as early as possible
				m_pobMyCurrentStrike->GetTargetP()->GetMessageHandler()->ReceiveMsg<msg_combat_struck>();
						
				// Clock the depth of sync'd movement
				m_iSyncdDepth++;
			}
		}
	}

	// ...else this is a standard attack
	else
	{	
		m_obAttackMovement.StartAttackMovement(	*m_pobMyCurrentStrike,
												m_obEvadeDirection,
												m_bPreviousAttackWasSynchronised || m_pobMyCurrentStrike->GetAttackDataP()->m_bNoAnimationBlend );
	}

	// JML ADDED - 06-01-05
	// Inform the target of this attack that they're being attacked!
	const CEntity* pTarget = m_pobMyCurrentStrike->GetTargetP();

	if(pTarget)
	{
		// Inform the entity of the grab
		if( pTarget->IsAI() 
			&& m_pobMyCurrentStrike 
			&& (AC_GRAB_GOTO == m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass ||
				AC_GRAB_HOLD == m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass ||
				AC_GRAB_STRIKE == m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass) ) 
		{
			// 
			const AICombatComponent& aiCombat = ((AI*)pTarget)->GetAIComponent()->GetCombatComponent();

			//
			aiCombat.Event_OnAttacked( AC_2_AAT[m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass], 
										m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime( m_fAttackScalar ),
										m_pobParentEntity,
										true, false);
		}

	}


	// trigger some combat effects - don't want to do it during super safety
	if (m_pobAttackDefinition->m_pCombatEffectsDef)
	{
		bool bDoEffects = true;
		if (IsInSuperStyleSafetyTransition() && !m_bSuperStyleSafetyAttackerDone && !m_bSuperStyleSafetyReceiverDone)
		{
			bDoEffects = false;
		}

		if (bDoEffects)
		{
		m_iCurrEffectsTrigger = m_pobAttackDefinition->m_pCombatEffectsDef->SetupEffectsTrigger(	m_pobParentEntity,
																									pTarget,
																									m_pobMyCurrentStrike->GetAttackDataP(),
																									m_pobMyCurrentStrike->GetAttackTime() );
		}
	}

	// If we have a rotating attack strike proximity angle
	if (m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike2.GetFirstValueLength(m_pobMyCurrentStrike->GetAttackTimeScalar()) != -1.0f &&
		m_pobMyCurrentStrike->GetAttackDataP()->m_fStrike2ProximityCheckAngleCoverage != 0.0f)
	{ 
		// Get the length of the strike2 window
		float fLength = 0.0f;
		if (m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike2.GetNumFlippers() == 1)
		{ // Just use the length of the only flipper
			fLength = m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike2.GetFirstValueLength(m_pobMyCurrentStrike->GetAttackTime());
		}
		else
		{ // Length is total time of all flippers
			CFlipFlop::FlipperContainerType::const_iterator itStart = m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike2.Begin();
			while (itStart != m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike2.End()) 
			{
				fLength += itStart->X2(m_pobMyCurrentStrike->GetAttackTime()) - itStart->X1(m_pobMyCurrentStrike->GetAttackTime());

				itStart++;
			}
		}
		
		// Calculate a delta value for the angle, angleToCover/timeAvailable
		m_fCurrentAttackStrikeProximityCheckAngleDelta = m_pobMyCurrentStrike->GetAttackDataP()->m_fStrike2ProximityCheckAngleCoverage / fLength;
	}
	else
	{ // Zero out angle delta value
		m_fCurrentAttackStrikeProximityCheckAngleDelta = 0.0f;
	}

	// Cool camera stuff for supers
	bool bStopCameraBecauseOfSuperstyleSafety = false; // Don't stop by default
	if ( IsInSuperStyleSafetyTransition() ) // Make sure both attacker and receiver are done before we activate any cameras
		bStopCameraBecauseOfSuperstyleSafety = !(m_bSuperStyleSafetyAttackerDone && m_bSuperStyleSafetyReceiverDone);
	if ( !bStopCameraBecauseOfSuperstyleSafety && (!m_bCombatCamStartedThisAttack || !CamMan::GetPrimaryView()->GetCoolCam(m_iCombatCam)))
	{
		if(m_pobAttackDefinition->m_pobCameras)
		{
			float fButtonHeldTime = m_obAttackTracker.GetAttackRequestTime(*m_pobParentEntity->GetInputComponent());
			// Deactivate the current cam
			m_iCombatCam = m_pobAttackDefinition->m_pobCameras->DeactivateCombatCam(m_iCombatCam);
			// Try to start the next one
			CPoint obPosition = m_pobParentEntity->GetPosition();
			obPosition.Y() += 0.6f; // Shift this up a bit so we don't collide with the floor			
			// Make sure we're in no danger of intersecting level geometry
			if (!Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obPosition, m_obCounterCameraCheckVolumeHalfExtents,0.0f) && !Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obPosition, m_obCounterCameraCheckVolumeHalfExtents,PI*0.25f))
			{
				m_iCombatCam = m_pobAttackDefinition->m_pobCameras->ActivateCombatCam(m_pobMyCurrentStrike, fButtonHeldTime, !m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() ? m_pobSyncdTransform : 0);
				// Mark success so we can cleanly transfer cams later
				m_bCombatCamStartedThisAttack = m_iCombatCam > 0;
				// The camera must respect the attackers time multiplier
				CoolCam_Maya* pCam = (CoolCam_Maya*)CamMan::GetPrimaryView()->GetCoolCam(m_iCombatCam);
				if(pCam && pCam->GetType() == CT_MAYA)
					pCam->SetAttacker(m_pobParentEntity);
			}
			else
			{
				ntPrintf("%s: Cam volume collided with world, no camera for this attack.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString() );
			}
		}
	}

	Message obAttackMessage(msg_combat_attackstarted);
	m_pobParentEntity->GetMessageHandler()->QueueMessage(obAttackMessage);

	// Set the state to attacking
	SetState( CS_ATTACKING );

	// Indicate success
	//CGatso::Stop("CAttackComponent::StartAttack");
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GetAttackStrikeProximityCheckDistance
*
*	DESCRIPTION		Getter for attack debugger.
*
***************************************************************************************************/
float CAttackComponent::GetAttackStrikeProximityCheckDistance() const
{
	if (m_pobMyCurrentStrike)
		return m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckDistance;
	else
		return 0.0f;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GetAttackStrikeProximityCheckExclusionDistance
*
*	DESCRIPTION		Getter for attack debugger.
*
***************************************************************************************************/
float CAttackComponent::GetAttackStrikeProximityCheckExclusionDistance() const
{
	if (m_pobMyCurrentStrike)
		return m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckExclusionDistance;
	else
		return 0.0f;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GetAttackStrikeProximityCheckSweep
*
*	DESCRIPTION		Getter for attack debugger.
*
***************************************************************************************************/
float CAttackComponent::GetAttackStrikeProximityCheckSweep(int iStrike) const
{
	if (m_pobMyCurrentStrike)
	{
		if (iStrike == 1)
		{
			return m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckSweep;
		}
		else if (iStrike == 2)
		{
			return m_pobMyCurrentStrike->GetAttackDataP()->m_fStrike2ProximityCheckSweep;
		}
		else
		{
			ntAssert_p(false,("Unknown strike number"));
			return 0.0f;
		}

	}
	else
	{
		return 0.0f;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GetAttackStrikeProximityCheckAngle
*
*	DESCRIPTION		Returns the angle at which a strike proximity check should be done. Takes into 
*					account any rotation. Needs to be public for attack debugger.
*
***************************************************************************************************/
float CAttackComponent::GetAttackStrikeProximityCheckAngle(int iStrike) const
{
	if (m_pobMyCurrentStrike)
	{
		// If we're doing a rotation (i.e. the delta has some non zero value)
		if (iStrike == 1)
		{
			return m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckAngle;
		}
		else if (iStrike == 2)
		{
			if (m_fCurrentAttackStrikeProximityCheckAngleDelta != 0.0f)
				// Then return the current rotated value
				return m_pobMyCurrentStrike->GetAttackDataP()->m_fStrike2ProximityCheckAngle + (m_fCurrentAttackStrikeProximityCheckAngleDelta * (m_fStateTime - m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike2.GetFirstValue(m_pobMyCurrentStrike->GetAttackTime())));
			else
				// Just return the value in the attack data
				return m_pobMyCurrentStrike->GetAttackDataP()->m_fStrike2ProximityCheckAngle;
		}
		else
		{
			ntAssert_p(false,("Unknown strike number"));
			return 0.0f;
		}
	} 
	else
	{
		return 0.0f;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateAttack
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::UpdateAttack( float fTimeDelta )
{
	//CGatso::Start("CAttackComponent::UpdateAttack");
	UNUSED( fTimeDelta );

	// Make damn sure we have one of these
//	ntError( m_pobMyCurrentStrike );
	user_warn_p(m_pobMyCurrentStrike, ("UpdateAttack recieved with no current strike... we have a horrible hack for alpha to get through this only (forcing back to standard)"));
	if (m_pobMyCurrentStrike == 0)
	{
		//Downgraded from "nasty-alpha-hack" to "fairly reasonable and safe/clean fallback but should ideally never get here".
		ntPrintf("%s: Heinousness in UpdateAttack, no m_pobMyCurrentStrike. We now have fallback for this, but it shouldn't be happening! Tell GavC how you did this.\n",
			ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString());

		// Reset any current attack we may think we're doing (guaranteed to be in CS_ATTACKING at this point).
		m_pobParentEntity->GetMessageHandler()->ReceiveMsg<msg_combat_recovered>();
		m_obAttackTracker.Reset();
		ResetAttack(true);
		EndString();
		//Because msg_combat_recovered won't be received until the next frame we need to manually take them out of CS_ATTACKING here.
		SetState(CS_STANDARD);
		return;
	}

	// Check if we need a button hint
	if (m_pobMyCurrentStrike->GetAttackDataP()->m_eHintInNextMove != AB_NUM)
	{
		m_abHintForButton[m_pobMyCurrentStrike->GetAttackDataP()->m_eHintInNextMove] = true;
	}

	// If we're performing a ranged attack, then we need to query for nearby projectile entities and send a message to any
	// that are within a certain radius. This only applies to the heroine who should be able to deflect/destroy projectiles
	// with a ranged attack.
	if (m_pobParentEntity->IsCharacter() && m_pobParentEntity->ToCharacter()->IsPlayer() && m_pobMyCurrentStrike->GetAttackDataP())
	{
		ATTACK_CLASS eAttackClass = m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass;
		if((eAttackClass == AC_RANGE_FAST) || (eAttackClass == AC_RANGE_MEDIUM))
		{
			//Query for a list of projectiles that are within the strike proximity.
			CEntityQuery obQuery;

			//Only entities within a 2 metre radius.
			CEQCProximitySphere obSphereQuery;
			obSphereQuery.Set(m_pobParentEntity->GetPosition(), m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckDistance);
			obQuery.AddClause(obSphereQuery);

			//Only projectiles (should speed things up a bit).
			CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_Projectile );

			//Get the list of all the found entities and send them messages.
			QueryResultsContainerType& rEntityList = obQuery.GetResults();

			QueryResultsContainerType::const_iterator obIt	= rEntityList.begin();
			QueryResultsContainerType::const_iterator obItEnd = rEntityList.end();
			for( ; obIt != obItEnd ; ++obIt )
			{
				CEntity* pProjectile = *obIt;

				if(!pProjectile)
				{
					ntError_p(false, ("Entity-query returned at least one NULL entity"));
					continue;
				}

				if(!pProjectile->IsProjectile())
				{
					ntError_p(false, ("Our Entity-Query in CEntity::EntType_Projectile returned an object where IsProjectile() is false!"));
					continue;
				}

				if(pProjectile->GetMessageHandler())
				{
					Message obMessage(msg_projectile_hitranged);
					obMessage.SetEnt("Sender", m_pobParentEntity);
					pProjectile->GetMessageHandler()->Receive(obMessage);
				}
			}
		}
	}

	// Update the flag to signal no collision between me and the target
	if ( IsInNoCollideWindow() )
		m_pobParentEntity->GetAttackComponent()->SetAttackCollisionBreak( true );
	else
		m_pobParentEntity->GetAttackComponent()->SetAttackCollisionBreak( false );

	// Check if we need a prestrike - to be sent .15 of a second before the opening of the strike window if we have a strike window
	// Exclude when we're super safe transitioning
	if ( !IsInSuperStyleSafetyTransition()
		&&
		( !m_bPreStrikeRequested )
		 &&
		 ( m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike.GetNumFlippers() > 0 ) )
	{

		if ( m_fStateTime > ( m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike.GetFirstValue( m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime( m_fAttackScalar ) ) - m_fAutoBlockLeadTime ) )
		{
			// Generate a pre strike - this allows an opponent to know what attack is coming
			GenerateDirectStrike( m_pobMyCurrentStrike->GetTargetP(), true, false );
			m_bPreStrikeRequested = true;

			// We also need to send out pre strikes to potential incidental targets too
			if ( m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike2.GetNumFlippers() > 0 )
			{
				GenerateIncidentalStrike( m_pobMyCurrentStrike->GetTargetP(), true );
			}
		}
		// Give AI ents loads of warning about incoming attacks.
		else if( m_pobMyCurrentStrike->GetTargetP() 
				 && 
				 m_pobMyCurrentStrike->GetTargetP()->IsAI() )
		{
			((AI*)m_pobMyCurrentStrike->GetTargetP())->GetAIComponent()->GetCombatComponent().Event_OnAttackWarning( m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime( m_fAttackScalar ), m_pobParentEntity );
		}
	}

	// Currently the strike is not normally used as a window but as a definate point - we may have 
	// multiple strike windows in a single attack so we have to cope with moving between windows
	int iCurrentStrikeWindow = IsInStrikeWindow();
	// Exclude when we're super safe transitioning
	if ( !IsInSuperStyleSafetyTransition() && iCurrentStrikeWindow > 0 )
	{
		// If we have moved to another strie window, clear the strike list and update the window we are in
		// We make the assumption that the divisions in the strike one window will be mirrored in strike 2
		if ( iCurrentStrikeWindow != m_iStrikeWindow )
		{
			if ( m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() )
				m_obStrikeList.clear();

			m_iStrikeWindow = iCurrentStrikeWindow;
		}

		// Request strikes...
		GenerateDirectStrike( m_pobMyCurrentStrike->GetTargetP(), false, false );
	}

	// If we are in the strike 2 window we need to check whether or not we are hitting characters other than 
	// our target - for some attack types we don't want incidental strikes - we can't hit the target here
	if ( !IsInSuperStyleSafetyTransition() && ( IsInStrike2Window() ) )
	{
		GenerateIncidentalStrike( m_pobMyCurrentStrike->GetTargetP(), false );
		SetCombatPhysicsPushVolumes(true);
	}
	else
	{
		SetCombatPhysicsPushVolumes(false);
	}

	// If we have a next attack that we are allowed to execute at this time - do it
	if ( CanExecuteNextAttack() && !m_bBadCounterBeingPunished)
	{
		bool bEvadeAttack = false;

		if ( (m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_EVADE) && ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass != AC_EVADE) )
			bEvadeAttack = true;

		// If we successfully generated a strike...
		if ( GenerateMyStrike( false, false ) && StartAttack() ) 
		{
			if (bEvadeAttack)
				// Log me starting an evade attack - NB after StartAttack() so can use CE_STARTED_ATTACK to leave evade attack state
				m_pobCombatEventLogManager->AddEvent(CE_STARTED_EVADE_ATTACK, m_pobMyCurrentStrike->GetTargetP(), (void*)m_pobMyCurrentStrike->GetAttackDataP());

			return;
		}
	}

	// As soon as the movement popout is reached a character may run away if they wish
	if ( ( IsInMovementWindow() ) 
		 &&
		 ( m_pobParentEntity->GetInputComponent() && ( m_pobParentEntity->GetInputComponent()->IsDirectionHeld() ) )
		 &&
		 ( m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike.GetNumFlippers() > 0 ) )
	{
		ProcessAttackeeList();

		// Start the recovery movement for this attack 
		m_obAttackMovement.StartAttackRecoverMovement( *m_pobMyCurrentStrike, m_pobAttackDefinition->m_pobAerialDetails, m_bStrikeLanded );

		// Tell the lua state that we want to run away
		m_obAttackMovement.SetMovementMessage( "msg_combat_breakout" );

		// We pass in true to indicate we want the strike pointer to hang around for 
		EndRecovery(true);
		return;
	}

	// If we are currently in a combat cam and the request is no longer being made drop out
	if ( ( ( m_iCombatCam >= 0 ) && ( !m_obAttackTracker.CurrentAttackStillRequested( *m_pobParentEntity->GetInputComponent(), m_fStateTime ) ) )
		 &&
		 ( m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() ) )
	{
		m_iCombatCam = m_pobAttackDefinition->m_pobCameras->DeactivateCombatCam(m_iCombatCam);
	}

	// Check to see whether we have done a successful KO move
	if ( ( !m_bKOSuccessful )
		 &&
		 ( m_bStrikeLanded )
		 &&
		 ( ( m_pobMyCurrentStrike->GetTargetP() ) 
		   &&
		   ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent() )
		   && 
		   ( ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState == CS_KO ) 
			 || 
			 ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState == CS_DYING )
			 ||
			 ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState == CS_DEAD )
			 ||
			 ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState == CS_FLOORED ) ) ) )
	{
		// Set a flag to mark the success
		m_bKOSuccessful = true;

		// If it's a juggleKO and it got a target, start timing how long to remember that target for
		if (m_pobMyCurrentStrike->GetAttackDataP()->m_bJuggleKO && m_pobMyCurrentStrike->GetTargetP())
		{
			//ntPrintf("*** Started remembering juggle KO person.\n");
			m_fJuggleTargetMemoryTimeSoFar = 0.0f;
			m_pobLastJuggledEntity = m_pobMyCurrentStrike->GetTargetP();
		}
	}

	// If we have reached the end of the attack state
	if ( m_fStateTime >= m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime( m_fAttackScalar ) )
	{
		// Tell our hit counter if we have not hit anyone
		if ( !m_bStrikeLanded && GetHitCounter() )
			GetHitCounter()->UnSuccessfulHit();

		// Is this an NPC performing a formation attack?
		if( m_pobParentEntity->IsAI() 
			&& ((AI*)m_pobParentEntity)->GetAIComponent()->GetAIFormationComponent()
			&& ((AI*)m_pobParentEntity)->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack()
			&& !m_obAttackTracker.RequestIsReady() )
		{
			((AI*)m_pobParentEntity)->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack()->AttackStateEnded( m_pobParentEntity ); 
		}

		// We need to make sure that everything isn't going pear shaped here - if we have missed
		// the attack popout at this stage we should force it...
		if ( m_obAttackTracker.RequestIsReady() && (m_obAttackTracker.GetRequestedAttackClass() != AC_EVADE) && !m_bBadCounterBeingPunished)
		{
			// If we successfully generated a strike...
			if ( GenerateMyStrike( false, false ) && StartAttack() ) 
			{
				return;
			}
		}

		// If this attack auto selects the next then select it
		if (m_pobMyCurrentStrike->GetAttackDataP()->m_bAutoLink && !m_obAttackTracker.GetIsWaitingForHeld() && !m_bIsDoingSuperStyleSafetyTransition)
		{
			// Auto-select the next attack...
			m_obAttackTracker.SelectNextAttack( m_pobParentEntity, m_eCurrentStance, true, m_eHitLevel, false );			
			//Downgraded from user_error_p and added hacky back-to-standard.
			user_warn_p( m_obAttackTracker.RequestIsReady(), ("Error: Auto-link at the end of an attack chain, attack %s",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetAttackDataP() ))) );
			if(!m_obAttackTracker.RequestIsReady())
			{
				//This was added as going to FMV and then coming back during a superstyle (e.g. surfing) was causing it to crash
				//as we'd lost some of our attack data. This hack forces the player back into standard.
				if(m_pobParentEntity->GetMovement())	//Should always be true, but just to be safe.
				{
					m_pobParentEntity->GetMovement()->ClearControllers();
				}
				//NOTE: If the current strike on the player (if they have one) is a syncronised attack then CompleteRecovery clears
				//the movement controllers anyway, but we do it above for the non-sync'd attacks.
				CompleteRecovery();
				m_obAttackTracker.ClearRequested();
				if(m_pobParentEntity->GetMessageHandler())
				{
					m_pobParentEntity->GetMessageHandler()->ReceiveMsg<msg_combat_recovered>();
				}
			}		
			// If the next attack needs to use the current target - check we have one - otherwise clear the attack
			else if ( ( !m_obAttackTracker.GetRequestedAttackDataP()->m_bHoldPreviousTarget )
				 ||
				 ( ( m_pobMyCurrentStrike->GetTargetP() )
				   &&
				   ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent() )
				   &&
				   ( !m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->GetDisabled() )
				   &&
				   ( ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState != CS_DEAD ) 
				     && 
				     ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState != CS_FLOORED ) 
					 && 
				     ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState != CS_RISE_WAIT )
				     &&
				     !( ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState == CS_RECOVERING ) 
				        && 
					    ( m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()-> m_eRecoveryType == RC_RISING ) ) ) 
						// Check their invulnerability window because we shouldn't autolink with a held target if they're invulnerable
						// ...we'll look very sad and silly doing our sync anims all on our own. 
					&& ( !m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->IsInInvulnerabilityWindow() ) ) )
			{
				CDirection obMeToTarget(0.0,0.0,0.0);
				if (m_pobMyCurrentStrike->GetTargetP())
					obMeToTarget = CDirection(m_pobMyCurrentStrike->GetTargetP()->GetPosition() - m_pobParentEntity->GetPosition());
				float fDistance = obMeToTarget.Length();

				// If we're trying to auto link onto a grab strike type, must check some things
				if ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE
					&&
					m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState == CS_ATTACKING // If they're attacking
					&&
					(m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE || m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD) // ...with a grab strike or hold
					&&
					m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_pobMyCurrentStrike->GetTargetP() == m_pobParentEntity ) // ...and I'm the target of it, then I can't continue because I'm gonna get grabbed
				{
					// Fail here because I'm going to get grabbed by my target
					//ntPrintf("%s: Auto linking target FAILED from %s to %s. Target already grabbing me.\n",ObjectDatabase::Get().GetNameFromPointer( m_pobParentEntity ), ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetAttackDataP() ), ObjectDatabase::Get().GetNameFromPointer( m_obAttackTracker.GetRequestedAttackDataP() ) );					

					m_obAttackTracker.ClearRequested();
				}
				
				// Check if we're going through an invisible wall via a grab...
				if ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE )
				{
					// Raycast for an invisible wall
					CPoint obRayStart( m_pobParentEntity->GetPosition() );
					CPoint obRayEnd( m_pobMyCurrentStrike->GetTargetP()->GetPosition() );
					float fHitFraction = -1.0f;
					CDirection obHitNormal( CONSTRUCT_CLEAR );

					Physics::RaycastCollisionFlag obCollision;
					obCollision.base = 0;
					obCollision.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
					obCollision.flags.i_collide_with = Physics::AI_WALL_BIT;
					if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayEnd, fHitFraction, obHitNormal, obCollision ) )
					{
						// ...and bail completely if we are
						m_obAttackTracker.ClearRequested();
					}			
				}
				
				// If we are linking from our goto, need to check that we can't be blocked before we proceed
				if ( m_obAttackTracker.RequestIsReady() // Need to check this as the previous line of code above can clear it
					&&
					m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_GOTO && (m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE)
					&&
					( (!m_pobMyCurrentStrike->GetTargetP())
					||
					( m_pobMyCurrentStrike->GetTargetP() && m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->GetTargetingDisabled() )
					||
					( fDistance > m_obAttackTracker.GetRequestedAttackDataP()->m_fStrikeProximityCheckDistance ) // If they're out of range
					||
					(	!(m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->CharacterUninterruptibleVulnerableTo(m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass) // If they're not vulnerable
						&& 
						!m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->MayAutoBlockAttack(m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass) ) ) ) ) // And if they can auto block it
				{
					// We have to fail here
					//ntPrintf("%s: Auto linking target FAILED from %s to %s. Grab auto blocked.\n",ObjectDatabase::Get().GetNameFromPointer( m_pobParentEntity ), ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetAttackDataP() ), ObjectDatabase::Get().GetNameFromPointer( m_obAttackTracker.GetRequestedAttackDataP() ) );					

					// If this was a grab, and it can be auto blocked, select failed grab as next sync move if in range
					if (	( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_GOTO || m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD ) &&
							m_pobMyCurrentStrike->GetTargetP() &&
							m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState != CS_DYING &&
							m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->MayAutoBlockAttack(m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass) &&
							m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_pobAttackDefinition->m_bCanAutoBlockGrabs &&
							fDistance < m_obAttackTracker.GetRequestedAttackDataP()->m_fStrikeProximityCheckDistance)
					{
						m_obAttackTracker.ClearRequested();

						if (!m_obAttackTracker.SelectBlockedGrab())
						{
							m_obAttackTracker.ClearRequested();
							ntAssert_p(0, ("%s: Tried a blocked grab, but no blocked grab could be selected!\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(this->m_pobParentEntity))));
						}
					}
					// Else if the target has now got out of range, just fail
					else 
					{	
							m_obAttackTracker.ClearRequested();
					}
				}
				// Else, autolink is fine
				else if ( m_obAttackTracker.RequestIsReady() ) // Need to check this as if we encountered an invisible wall it would have been cleared
				{
					//ntPrintf("%s: Auto linking target from %s to %s\n",ObjectDatabase::Get().GetNameFromPointer( m_pobParentEntity ), ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetAttackDataP() ), ObjectDatabase::Get().GetNameFromPointer( m_obAttackTracker.GetRequestedAttackDataP() ) );
					Message obAttackMessage(msg_combat_autolinked);
					m_pobParentEntity->GetMessageHandler()->QueueMessage(obAttackMessage);

					if ( ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_HOLD ) 
						&& ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackMovementType != AMT_GROUND_TO_AIR )
						&& ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackMovementType != AMT_AIR_TO_AIR ))
					{
						// Log superstyle start
						m_pobCombatEventLogManager->AddEvent(CE_STARTED_SUPERSTYLE, m_pobMyCurrentStrike->GetTargetP(), (void*)m_eHitLevel);
					}
		
					// If I'm successfully starting a grab, log it
					if (m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE)
					{
						m_pobCombatEventLogManager->AddEvent(CE_SUCCESSFUL_GRAB,m_pobMyCurrentStrike->GetTargetP(), (void*)m_eHitLevel );
						if (m_pobMyCurrentStrike->GetTargetP())
							m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_GOT_GRABBED, m_pobParentEntity);
					}

					// Make sure ragdoll states set - can get reset by movement when autolinking without changing combat state
					if (!m_obAttackTracker.GetRequestedAttackDataP()->m_obSyncReceiverAnim.IsNull())
						m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
							Physics::System::CHARACTER_CONTROLLER,
							Physics::System::CHARACTER_CONTROLLER_DO_NOT_COLLIDE );

					// If we have a input pad save the secondary pad direction at this point - only for evades
					if ( ( m_pobParentEntity->GetInputComponent() ) && ( m_obAttackTracker.RequestIsReady() ) && ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
						m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();
				}
			}
			else
			{
				/*if (m_pobMyCurrentStrike->GetTargetP())
					ntPrintf("%s: Auto linking target FAILED from %s to %s. Target %s in %s.\n",ObjectDatabase::Get().GetNameFromPointer( m_pobParentEntity ), ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetAttackDataP() ), ObjectDatabase::Get().GetNameFromPointer( m_obAttackTracker.GetRequestedAttackDataP() ), ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetTargetP() ), g_apcCombatStateTable[m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState] );
				else
					ntPrintf("%s: Auto linking target FAILED from %s to %s\n",ObjectDatabase::Get().GetNameFromPointer( m_pobParentEntity ), ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetAttackDataP() ), ObjectDatabase::Get().GetNameFromPointer( m_obAttackTracker.GetRequestedAttackDataP() ) );*/

				// The autolinked attack is not valid (blocked grabs done after goto, so no need to cater for it here)
				m_obAttackTracker.ClearRequested();
			}
		}

		// If this is a syncronised or auto link attack then we don't need a popout
		if ( ( m_obAttackTracker.RequestIsReady() )
			 && 
			 ( ( !m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() )
			   ||
			   ( m_pobMyCurrentStrike->GetAttackDataP()->m_bAutoLink ) ) )
		{
			// If we successfully generated a strike...
			if ( GenerateMyStrike( false, true ) && StartAttack() ) 
			{
				return;
			}
		}

		// Otherwise start this attack's recovery
		else
		{
			// Hang on until safety done before we decide to end
			if (!m_bIsDoingSuperStyleSafetyTransition)
				EndAttack();
		}
	}

	//CGatso::Stop("CAttackComponent::UpdateAttack");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CanExecuteNextAttack
*
*	DESCRIPTION		Works out whether an attack can be executed at this time
*
***************************************************************************************************/
bool CAttackComponent::CanExecuteNextAttack( )
{
	// Make sure that we are in the state that we think we are
	ntAssert( m_eCombatState == CS_ATTACKING );

	// We have to have an attack lined up
	if ( !m_obAttackTracker.RequestIsReady() )
	{
		return false;
	}

	// If this is a grab attack make sure it hits before we can move on to the next attack
	if ( ( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_GOTO || m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE ) && ( !m_bStrikeLanded ) )
	{
		return false;
	}

	// If this is an evade and we are already evading, or in a sync, or invulnerable
	if ( ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ||
		m_obAttackTracker.GetRequestedAttackClass() == AC_GRAB_GOTO ) && 
		( IsInEvadeAndGrabDenyWindow() ) )
	{
		m_obAttackTracker.ClearRequested();
		return false;
	}

	// If this is from our standard attack set and we are not in the attack popout then we can't proceed
	if ( ( m_obAttackTracker.GetRequestedAttackClass() != AC_EVADE ) && ( IsInAttackWindow() == 0 ) )
	{
		return false;
	}

	/*// If this is an evade and we are already evading, or in a sync, or invulnerable
	if ( ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) && ( m_pobMyCurrentStrike && ( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_EVADE || m_pobMyCurrentStrike->GetAttackDataP()->m_bSyncronisedAttack || IsInInvulnerabilityWindow()) ) )
	{
		m_obAttackTracker.ClearRequested();
		return false;
	}*/

	// If I am in an aerial combo and the current attack hasn't knocked out the target we can't do another attack
	if ( ( m_bAerialCombo ) && ( !m_bKOSuccessful ) )
	{
		m_obAttackTracker.ClearRequested();
		return false;
	}

	// Reject non-aerial grabs and evades when in aerial
	if ( m_bAerialCombo && (( m_obAttackTracker.GetRequestedAttackClass() == AC_GRAB_GOTO && m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackMovementType != AMT_AIR_TO_AIR) || m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
	{
		m_obAttackTracker.ClearRequested();
		return false;
	}

	// If we are here then i can't see any good reason why we can't bash the s*%t out of the target in question
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::ProcessAttackeeList
*
*	DESCRIPTION		Deal with the list of characters we have struck in the previous string
*
***************************************************************************************************/
void CAttackComponent::ProcessAttackeeList( void )
{
	// At the moment what we are doing with this list is really simple but in the future we may
	// want to do lots of clever things here - have characters hang in the air and then fall 
	// suddenly, all kinds of wacky shit
	
	// Make sure we have a current strike
	ntAssert( m_pobMyCurrentStrike );

	// Clear the list away
	ClearAttackeeList();
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::ClearAttackeeList
*
*	DESCRIPTION		Deal with the list of characters we have struck in the previous string
*
***************************************************************************************************/
void CAttackComponent::ClearAttackeeList( void )
{
	m_obAttackeeList.clear();
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndAttack
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::EndAttack()
{
	// We've done this now
	m_bNeedLedgeRecover = false;

	m_obTargetPointOffset.Clear();

	// If Strike2 lasts to the end of an attack, we need to set the non attacking volumes here
	SetCombatPhysicsPushVolumes(false);

	// If we started an evade combo, and the attack we're ending is not the evade that started it, end the combo
	if ( m_bEvadeCombo && m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass != AC_EVADE )
	{
		m_bEvadeCombo = false;
	}

	// If we're grabbing/supering into the last strike, we need to clear our hit count
	if ( m_pobMyCurrentStrike && ( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE )
		&& !m_pobMyCurrentStrike->GetAttackDataP()->m_bAutoLink )
	{
		CChatterBoxMan::Get().Trigger("SuperCompleted", m_pobParentEntity);

		Message obAttackerMessage(msg_combat_superstyledone_attacker);
		m_pobParentEntity->GetMessageHandler()->Receive(obAttackerMessage);
		if (m_pobMyCurrentStrike->GetTargetP())
		{
			Message obReceiverMessage(msg_combat_superstyledone_receiver);
			m_pobMyCurrentStrike->GetTargetP()->GetMessageHandler()->Receive(obReceiverMessage);
		}

		// Log superstyle end
		m_pobCombatEventLogManager->AddEvent(CE_FINISHED_SUPERSTYLE, m_pobMyCurrentStrike->GetOriginatorP(), (void*)m_pobMyCurrentStrike->GetAttackDataP());
	}

	// Reset proximity check angle delta value
	m_fCurrentAttackStrikeProximityCheckAngleDelta = 0.0f;

	// Deal with the list of characters we have struck in the previous string
	ProcessAttackeeList();

	// Make sure that our nasty flags in entity info are reset
	m_pobParentEntity->GetAttackComponent()->SetAttackCollisionBreak( false );

	// End the attack string
	EndString();

	// Make sure we don't have any attacks stored up
	m_obAttackTracker.Reset();

	// Make sure forced mode is not on anymore (for attacks forced over the network)
	m_bForcedModeEnabled = false;

	if ( m_pobMyCurrentStrike && !m_pobMyCurrentStrike->GetAttackDataP()->m_obRecoveryAnimName.IsNull() )
	{
		// Start the recovery movement for this attack 
		m_obAttackMovement.StartAttackRecoverMovement( *m_pobMyCurrentStrike, m_pobAttackDefinition->m_pobAerialDetails, m_bStrikeLanded );

		if (m_pobMyCurrentStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_AERIAL_COMBO)
		{
			CamMan::GetPrimaryView()->RemoveCoolCamera(m_iAerialCoolCamID);
			m_iAerialCoolCamID = -1;

			CChatterBoxMan::Get().Trigger("AerialCompleted", m_pobParentEntity);
		}

		// Give the movement system a message to pass to the state if this movement is completed fully
		m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );
	}
	else 
	{
		// If we are here then we should have had the animation to recover - warn the user
		if(m_pobMyCurrentStrike)
		{
		user_warn_p( 0 , ( "No recovery animation found for %s.\n",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetAttackDataP() ) )) );
	}
		else
		{
			user_warn_p( 0 , ( "No recovery animation found, and no current strike\n" ));
		}
	}

	// Move to the recovery state
	SetState( CS_RECOVERING );
	m_fIncapacityTime = 0.0f;

	// Set up the recovery type - different for grabs
	if ( m_pobMyCurrentStrike && (m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_GOTO || m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE ))
		m_eRecoveryType = RC_GRAB;
	else
		m_eRecoveryType = RC_STANDARD;

	// Reset the attack
	if (m_pobMyCurrentStrike)
		ntPrintf("%s: resetting attack %s from EndAttack.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString());
	ResetAttack(true);
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CanTakeStandardHit
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackComponent::CanTakeStandardHit( void ) const
{
	return (	( m_eCombatState != CS_FLOORED ) 
				&&
				( m_eCombatState != CS_RISE_WAIT )
				&&
				( m_eCombatState != CS_DEAD ) );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CanTakeSpeedExtraHit
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackComponent::CanTakeSpeedExtraHit( void ) const
{
	return (	( m_eCombatState != CS_FLOORED ) 
				&&
				( m_eCombatState != CS_RISE_WAIT )
				&&
				( m_eCombatState != CS_DEAD )
				&&
				( m_eCombatState != CS_KO ) );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateRecovery
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::UpdateRecovery( float fTimeDelta )
{
	// Currently unused 
	UNUSED ( fTimeDelta );

	// Check if the character is trying to run away
	if ( ( m_pobParentEntity->GetInputComponent() && m_pobParentEntity->GetInputComponent()->IsDirectionHeld() )
		 &&
		 ( m_eRecoveryType != RC_RISING )
		 &&
		 ( m_eRecoveryType != RC_GRAB )
		 &&
		 ( !m_bAerialCombo ) )
	{
		// Tell the lua state that we want to run away
		CMessageSender::SendEmptyMessage( "msg_combat_breakout", m_pobParentEntity->GetMessageHandler() );

		// End the recovery
		EndRecovery();

		// Drop out
		return;
	}

	// We can't select another attack in aerial combo or grab recoveries - we just fall
	if ( ( m_bAerialCombo ) || ( m_eRecoveryType == RC_GRAB ) )
	{
		m_obAttackTracker.Reset();
	}

	// Otherwise if we are in a normal state...
	else
	{
		// Update if we're in a start volume
		m_pobSuperStyleStartVolume = 0;
		if (SuperStyleSafetyManager::Exists())
			m_pobSuperStyleStartVolume = SuperStyleSafetyManager::Get().PointInSuperStyleStartVolume(m_pobParentEntity->GetPosition());

        // Select the attack we are going to carry out
		if (!m_bBadCounterBeingPunished)
			m_obAttackTracker.SelectStartAttack( m_pobParentEntity, m_eCurrentStance, m_eHitLevel, m_pobSuperStyleStartVolume == 0 );

		// If we have a input pad save the secondar pad direction at this point - only for evades
		if ( ( m_pobParentEntity->GetInputComponent() ) && ( m_obAttackTracker.RequestIsReady() ) && ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
			m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();
	}

	// If we have found a requested attack - do it
	if ( ( m_obAttackTracker.RequestIsReady() && !m_bBadCounterBeingPunished) )
	{
		// If we successfully generated a strike...
		if ( GenerateMyStrike( false, false ) && StartAttack() ) 
		{
			// Tell the state system we have started an attack
			CMessageSender::SendEmptyMessage( "msg_combat_countered", m_pobParentEntity->GetMessageHandler() );

			// Get out of here
			return;
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndRecovery
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::EndRecovery( bool bLeaveStrike )
{
	// If we are in aerial combo then reset the flag and end the camera
	if ( m_bAerialCombo )
	{
		m_bAerialCombo = false;
		CMessageSender::SendEmptyMessage("msg_combat_aerialended",m_pobParentEntity->GetMessageHandler());

		// I have replaced the RemoveAllCoolCams call here with a remove specific camera.
		// Please please please do not use RemoveAllCoolCams unless it's absolutely necessary! Thanks.
		if(m_iAerialCoolCamID > 0)
		{
			CamMan::GetPrimaryView()->RemoveCoolCamera(m_iAerialCoolCamID);
			m_iAerialCoolCamID = -1;
		}
	}

	// End the evade attack combo here if we haven't already
	if ( m_bEvadeCombo )
	{
		m_bEvadeCombo = false;
	}

	// Give the movement system a message to pass to the state if this movement is completed fully
	m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );

	// Reset the attack
	if (m_pobMyCurrentStrike)
		ntPrintf("%s: resetting attack %s from EndRecovery.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString());
	if (!bLeaveStrike)
	{
		ResetAttack(true);
		EndString();
	}

	// Set the state to default
	SetState( CS_STANDARD );

	// Doing this here because deflect states are very short so we reset this too often usually
	m_iDepthInDeflecting = 0;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CompleteRecovery 
*
*	DESCRIPTION		This will be called from the state system when a recovery movement has been
*					completed.
*
***************************************************************************************************/
void CAttackComponent::CompleteRecovery( )
{	
	// Make sure we are in the state we expect to be in
	if ( m_eCombatState != CS_RECOVERING )
	{
		// If we are not in the recovery state it means that the animation choosen
		// for the reaction state we are in is too short to cover the choosen reaction time.
		// Give people some warning but don't assert on them
		if (m_pobMyCurrentStrike)
		{
			//ntPrintf( "An animation has completed before the reaction time of the state %s in attack %s. \n", g_apcCombatStateTable[ ((int)m_eCombatState) ], ObjectDatabase::Get().GetNameFromPointer( m_pobMyCurrentStrike->GetAttackDataP() ) );
		}
		else if (m_pobStruckStrike)
		{
			//ntPrintf( "An animation has completed before the reaction time of the state %s in attack %s. \n", g_apcCombatStateTable[ ((int)m_eCombatState) ], ObjectDatabase::Get().GetNameFromPointer( m_pobStruckStrike->GetAttackDataP() ) );
		}
		else
		{
			//ntPrintf( "An animation has completed before the reaction time of the state %s. \n", g_apcCombatStateTable[ ((int)m_eCombatState) ] );
		}
	}

	//Synchronised attack hax0r. If we're recovering from a synchronised attack, but not in CS_RECOVERING (skipping fall-and-rise)
	//then we clear the movement controllers to avoid attempted-blending badness.
	if ( (m_eCombatState != CS_RECOVERING) && (m_pobStruckStrike) && (m_pobStruckStrike->ShouldSync()) )
	{
		if(m_pobParentEntity && m_pobParentEntity->GetMovement())
		{
			m_pobParentEntity->GetMovement()->ClearControllers();
		}
	}

	if ( !(m_eCombatState == CS_DYING || m_eCombatState == CS_DEAD) )
	{
		// Move to the idle state
		EndRecovery();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GenerateIncidentalStrike 
*
*	DESCRIPTION		Generate a strike against players that happen to be in the way
*
***************************************************************************************************/
void CAttackComponent::GenerateIncidentalStrike( const CEntity* pobOtherEntity, bool bPreStrike )
{
	// Only allow the query to execute on AI's and players
	int iEntTypeMask = CEntity::EntType_Character;

	// Set up data references based on whether or not this is a pre strike
	ntstd::List< const CEntity* >&	obStrikeListRef		= ( bPreStrike ) ? m_obPreStrikeList : m_obStrikeList;
	float						fAreaFactorRef		= ( bPreStrike ) ? m_pobAttackDefinition->m_fStrikeToBlockFactor : 1.0f;

	// Here we are going to ignore the given target and just grab all the opponents in our attack 
	// strike area - then we'll send all of them strikes if we haven't done so already
	CEQCProximitySegment obProximitySub;
	obProximitySub.SetRadius( m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckDistance * fAreaFactorRef );
	// As this is an incidental strike, use the Strike2 Sweep from CAttackData
	obProximitySub.SetAngle( m_pobMyCurrentStrike->GetAttackDataP()->m_fStrike2ProximityCheckSweep * DEG_TO_RAD_VALUE );

	// Set the matrix for the proximity check
	CMatrix obStrikeTestMatrix( m_pobParentEntity->GetMatrix() );
	obStrikeTestMatrix = CMatrix( CVecMath::GetYAxis(), GetAttackStrikeProximityCheckAngle(2) * DEG_TO_RAD_VALUE ) * obStrikeTestMatrix;
	obProximitySub.SetMatrix( obStrikeTestMatrix );

	// Create my query object
	CEntityQuery obQuery;
	obQuery.AddClause( obProximitySub );

	// Check that the others are all players
	CEQCIsLockonable obLockonSub;
	obQuery.AddClause( obLockonSub );

	// If we are player, make sure we only hit enemys and other players (i.e. we don't hit non-enemy ais)
	CEQCIsTargetableByPlayer obTargettableByPlayerSub;
	if ( m_pobParentEntity->IsPlayer() )
	{
		obQuery.AddClause( obTargettableByPlayerSub );
	}

	// If we are in aerial combo mode we need to check height differently to normal combat
	CEQCHeightRange obHeightSub;
	if ( m_bAerialCombo )
	{
		obHeightSub.SetRelativeY( obStrikeTestMatrix.GetTranslation().Y() );
		obHeightSub.SetTop( m_pobAttackDefinition->m_fAerialStrikeUpperHeight );
		obHeightSub.SetBottom( m_pobAttackDefinition->m_fAerialStrikeLowerHeight );
	}
	else
	{
		obHeightSub.SetRelativeY( obStrikeTestMatrix.GetTranslation().Y() );
		obHeightSub.SetTop( m_pobAttackDefinition->m_fStrikeUpperHeight );
		obHeightSub.SetBottom( m_pobAttackDefinition->m_fStrikeLowerHeight );
	}
	obQuery.AddClause( obHeightSub );

	// If I'm enemy, exclude all other enemies
	CEQCIsEnemy obIsEnemy;
	if (m_pobParentEntity->IsEnemy())
	{
		obQuery.AddUnClause(obIsEnemy);
	}

	// Exclude ourselves
	obQuery.AddExcludedEntity( *m_pobParentEntity );
	// .. and exclude the main target if it's not our juggled entity (we want juggled entity to take Strike2 hits)
	if (pobOtherEntity != m_pobLastJuggledEntity)
		obQuery.AddExcludedEntity( *pobOtherEntity );

	CEQCIsGroundAttackable obGround;	
	if (m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST)
		obQuery.AddClause( obGround );


	// Make sure the entity is allowing itself to recieve attacks
	CEQCIsCombatTargetingDisabled obCombatTargetingDisabled;
	obQuery.AddUnClause( obCombatTargetingDisabled );

	// Send my query to the entity manager
	CEntityManager::Get().FindEntitiesByType( obQuery, iEntTypeMask );
	
	// Loop through all we got and send them a strike
	QueryResultsContainerType::iterator obEnd = obQuery.GetResults().end();
	for ( QueryResultsContainerType::iterator obIt = obQuery.GetResults().begin(); obIt != obEnd; ++obIt )
	{
		CEntity* pEnt	= *obIt;

		// We never hit a target that isn't in a CS_STANDARD state
		bool bInList	= CEntityQueryTools::EntityInList( pEnt, obStrikeListRef );

		// Only strike those not in the list. 
		if( bInList )
		{
			continue;
		}
		
		// Add this entity to the strike list
		obStrikeListRef.push_back( pEnt );

		// Check distance between me and my target
		CDirection obMeToTarget( pEnt->GetPosition() - m_pobParentEntity->GetPosition() );
		float fDistance = obMeToTarget.Length();

		// Generate the actual strike object
		MakeStrike( pEnt, bPreStrike, false, true, m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckExclusionDistance > 0.0f && fDistance < m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckExclusionDistance && fDistance < m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckDistance );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GenerateDirectStrike 
*
*	DESCRIPTION		Generate a strike against the given target players
*
***************************************************************************************************/
bool CAttackComponent::GenerateDirectStrike( const CEntity* pobOtherEntity, bool bPreStrike, bool bSyncronise, bool bSuperSafetyTransition )
{
	// Set up data references based on whether or not this is a pre strike
	ntstd::List< const CEntity* >&	obStrikeListRef		= ( bPreStrike ) ? m_obPreStrikeList : m_obStrikeList;
	float						fAreaFactorRef		= ( bPreStrike ) ? m_pobAttackDefinition->m_fStrikeToBlockFactor : 1.0f;

	if (bSuperSafetyTransition) // Specialness for super safety transitions
	{
		// DON'T add this entity to the strike list - just passing a strike as an administrative thing, the actual attack AFTER this transition will strike properly
		//obStrikeListRef.push_back( pobOtherEntity );

		// Generate the actual strike object
		MakeStrike( pobOtherEntity, bPreStrike, bSyncronise, false, false );

		// We have psuedo-hit someone
		return true;
	}

	// If we have held the previous target and we are syncronised - we have to hit
	if ( ( m_pobMyCurrentStrike->GetAttackDataP()->m_bHoldPreviousTarget ) 
		 &&
		 ( m_pobMyCurrentStrike->ShouldSync() )
		 &&
		 ( obStrikeListRef.size() == 0 ) )
	{
		// If this is a proper strike and we haven't hit this person yet
		if ( !bPreStrike && pobOtherEntity && !CEntityQueryTools::EntityInList( pobOtherEntity, obStrikeListRef ) )
		{
			// Add this entity to the strike list
			obStrikeListRef.push_back( pobOtherEntity );

			// Generate the actual strike object
			MakeStrike( pobOtherEntity, bPreStrike, bSyncronise, false, false );

			// We have hit someone
			return true;
		}

		// Get out of here
		return false;
	}

	// If we have a specific target to strike make sure we really should. We don't need to check counters, 
	// hold previous targets, IKORs, or skill-evades-attacks, or ledge recovers, they'll always strike.
	if ( pobOtherEntity && !m_pobMyCurrentStrike->IsCounter() && !m_pobInstantKORecoverTargetEntity && !m_pobSkillEvadeTargetEntity && !m_bNeedLedgeRecover &&
		!(m_pobMyCurrentStrike->GetAttackDataP()->m_bHoldPreviousTarget && pobOtherEntity == m_pobMyCurrentStrike->GetTargetP() ) )
	{
		// Check whether we should issue a strike based on a simple proximity check
		pobOtherEntity = ProximityTestStrikeTarget( pobOtherEntity, fAreaFactorRef );
	}

	// If the proximity check was passed 
	if ( pobOtherEntity && !CEntityQueryTools::EntityInList( pobOtherEntity, obStrikeListRef ) )
	{
		// Check distance between me and my target
		CDirection obMeToTarget( pobOtherEntity->GetPosition() - m_pobParentEntity->GetPosition() );
		float fDistance = obMeToTarget.Length();

		// Add this entity to the strike list
		obStrikeListRef.push_back( pobOtherEntity );

		// Generate the actual strike object
		// If target is within strike distance and exclusion distance, mark this as a strike within the exclusion distance
		MakeStrike( pobOtherEntity, bPreStrike, bSyncronise, false, m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckExclusionDistance > 0.0f && fDistance < m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckExclusionDistance && fDistance < m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckDistance );

		// We have sent a strike
		return true;
	}
	
	// If we are here then we haven't passed a strike to anyone
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::MakeStrike
*
*	DESCRIPTION		Actually builds and sends a strike
*
***************************************************************************************************/
void CAttackComponent::MakeStrike( const CEntity* pobOtherEntity, bool bPreStrike, bool bSyncronise, bool bIncidental, bool bWithinExclusionDistance )
{
	if ( pobOtherEntity->GetAttackComponent() )
	{
		CStrike* pobGeneratedStrike = NT_NEW_CHUNK( Mem::MC_MISC ) CStrike(	m_pobParentEntity, 
													pobOtherEntity, m_pobMyCurrentStrike->GetAttackDataP(), 
													m_fAttackScalar, 
													m_pobMyCurrentStrike->GetAttackRange(), 
													bPreStrike,
													m_pobMyCurrentStrike->IsCounter(),
													bIncidental,
													bWithinExclusionDistance,
													bSyncronise,
													false,
													0,
													m_pobParentEntity->GetPosition() );

		// Send to the opponent
		SyncdCombat::Get().PostStrike( pobGeneratedStrike );

		// Capture who we have hit incase we need to do some 'post-string' preocessing
		// Sorry about the const cast but i am going to have to rethink how we communicate in the future
		m_obAttackeeList.push_back( const_cast<CEntity*>( pobOtherEntity ) );
	}
	else
	{
		// If the target entity doesn't have an attack component then its likely to be a 
		// a background object or something similar. Send it a combat struck msg to 
		// inform it of the collision.

		if( pobOtherEntity->GetMessageHandler() )
		{
			// Create a message to send to the entity
			Message msg( msg_combat_struck );

			// Add params to the message. 
			msg.SetEnt( "Sender", m_pobParentEntity );		

			// Post the message
			pobOtherEntity->GetMessageHandler()->Receive( msg );
		}

	}

	// Send the AI a message telling it of the incoming attack
	if( pobOtherEntity->IsAI()  ) //&& bPreStrike )
	{
		// 
		const AICombatComponent& aiCombat = ((AI*)pobOtherEntity)->GetAIComponent()->GetCombatComponent();

		//
		aiCombat.Event_OnAttacked( AC_2_AAT[m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass], 
									m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike.GetFirstValue( m_fAttackScalar ),
									m_pobParentEntity,
									bPreStrike, bIncidental );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StrikeLanded
*
*	DESCRIPTION		Called when a strike lands successfully
*
***************************************************************************************************/
void CAttackComponent::StrikeLanded() const
{
	// Increment our strike landed flag
	m_bStrikeLanded = true;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::ProximityTestStrikeTarget
*
*	DESCRIPTION		Test a strike target just before strike creation.
*
***************************************************************************************************/
const CEntity* CAttackComponent::ProximityTestStrikeTarget( const CEntity* pobOtherEntity, float fRangeMultiplier )
{
// #define _DEBUG_PROX_CHECK
#ifdef _DEBUG_PROX_CHECK
	// Find the distance of the target from us
	CPoint obRelativePosition = pobOtherEntity->GetPosition();
	obRelativePosition -= m_pobParentEntity->GetPosition();
	float fDistance = obRelativePosition.Length();

	// Find the angle of the attacker from us
	float fAngle = MovementControllerUtilities::RotationAboutY( m_pobParentEntity->GetMatrix().GetZAxis(), CDirection( obRelativePosition ) );

	// Print out these values because they are useful
	ntPrintf( "Distance of target: %f \n", fDistance );
	ntPrintf( "Angle of target: %f \n", fAngle * RAD_TO_DEG_VALUE );

#endif

	// If we are in aerial combo mode we need to check height differently to usual
	if ( m_bAerialCombo )
	{
		CEQCHeightRange obHeightSub;
		obHeightSub.SetRelativeY( m_pobParentEntity->GetPosition().Y() );
		obHeightSub.SetTop( m_pobAttackDefinition->m_fAerialStrikeUpperHeight );
		obHeightSub.SetBottom( m_pobAttackDefinition->m_fAerialStrikeLowerHeight );

		// Directly query our target
		if ( !obHeightSub.Visit( *pobOtherEntity ) )
			return 0;
	}
	else
	{
		CEQCHeightRange obHeightSub;
		obHeightSub.SetRelativeY( m_pobParentEntity->GetPosition().Y() );
		obHeightSub.SetTop( m_pobAttackDefinition->m_fStrikeUpperHeight );
		obHeightSub.SetBottom( m_pobAttackDefinition->m_fStrikeLowerHeight );

		// Directly query our target
		if ( !obHeightSub.Visit( *pobOtherEntity ) )
			return 0;
	}

	// Find all that is in the strike range
	CEQCProximitySegment obProximitySub;
	obProximitySub.SetRadius( m_pobMyCurrentStrike->GetAttackDataP()->m_fStrikeProximityCheckDistance * fRangeMultiplier );
	// As this is used for direct strikes, need to use the Sweep in CAttackDefinition
	obProximitySub.SetAngle( GetAttackStrikeProximityCheckSweep(1) * DEG_TO_RAD_VALUE * fRangeMultiplier );

	// Set the matrix for the proximity check
	CMatrix obStrikeTestMatrix( m_pobParentEntity->GetMatrix() );
	obStrikeTestMatrix = CMatrix( CVecMath::GetYAxis(), GetAttackStrikeProximityCheckAngle(1) * DEG_TO_RAD_VALUE ) * obStrikeTestMatrix;
	obProximitySub.SetMatrix( obStrikeTestMatrix );

	// Directly query our target
	if ( obProximitySub.Visit( *pobOtherEntity ) )
		return pobOtherEntity;

	// If we are here we have failed
	return 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::ReceiveStrike const
*
*	DESCRIPTION		Register a strike with the stack
*
***************************************************************************************************/
void CAttackComponent::ReceiveStrike( CStrike* pobStrike ) const
{
	// We may have been disabled - if so drop out
	if ( m_bExternallyDisabled )
		return;

	// Add the strike to the strike stack
	m_obStrikeStack.push_back( pobStrike );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateStrikeStack const
*
*	DESCRIPTION		Update the strike stack
*
***************************************************************************************************/
void CAttackComponent::UpdateStrikeStack()
{
	//CGatso::Start("CAttackComponent::UpdateStrikeStack");

	// Get strikes from the list
	while(!m_obStrikeStack.empty())
	{
		CStrike* pobStrike = m_obStrikeStack.front();
		if ( pobStrike->IsPreStrike() )
		{
			COMBAT_STATE eStateBefore = m_eCombatState;
			// Deal with the prestrike
			if ( ProcessPreStrike( pobStrike ) )
			{
				//If our combat state hasn't changed, but ProcessPreStrike has returned true, then assert... we need to find this!
				if(eStateBefore == m_eCombatState && (m_eCombatState == CS_ATTACKING))
				{
//					user_warn_p(false, ("This shouldn't have happened (we've taken a strike in ProcessPreStrike, but we're still in attacking state). Tell GavC how you did this."));
					ntError_p(false, ("This shouldn't have happened (we've taken a strike in ProcessPreStrike, but we're still in attacking state). Tell GavC how you did this and show TTY!"));
				}

				// Reset any current attack we may be doing
				m_obAttackTracker.Reset();
				if (m_pobMyCurrentStrike)
					ntPrintf("%s: resetting attack %s from UpdateStrikeStack ProcessPreStrike.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString());
				ResetAttack(true);
				EndString();

				// Let the Lua state know that we have dropped out
				m_pobParentEntity->GetMessageHandler()->ReceiveMsg<msg_combat_struck>();
			}

			else
			{
				// Clean up the pre strike and move onto the next
				NT_DELETE_CHUNK( Mem::MC_MISC, m_obStrikeStack.front() );
			}

			// Move through the strike stack	
			m_obStrikeStack.pop_front();
		}
		else
		{
			COMBAT_STATE eStateBefore = m_eCombatState;
			// Deal with the proper strike
			if ( ProcessStrike( pobStrike ) )
			{
				//If our combat state hasn't changed, but ProcessStrike has returned true, then assert... we need to find this!
				if(eStateBefore == m_eCombatState && (m_eCombatState == CS_ATTACKING))
				{
//					user_warn_p(false, ("This shouldn't have happened (we've taken a strike in ProcessStrike, but we're still in attacking state). Tell GavC how you did this."));
					ntError_p(false, ("This shouldn't have happened (we've taken a strike in ProcessStrike, but we're still in attacking state). Tell GavC how you did this and show TTY."));
				}

				// If we're in a combat cam triggered by a move this character started before being hit then cancel it
				if ( m_iCombatCam >= 0 )
					m_iCombatCam = m_pobAttackDefinition->m_pobCameras->DeactivateCombatCam( m_iCombatCam, true );

				// If we were starting an aerial on someone, and we're accepting this strike, we need to drop them
				if ( m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_AERIAL_COMBO)
				{
					CEntity* pobEnt = const_cast< CEntity* >(m_pobMyCurrentStrike->GetTargetP());
					pobEnt->GetAttackComponent()->AerialTargetingCancel();
				}

				// Reset any current attack we may be doing
				m_obAttackTracker.Reset();
				if (m_pobMyCurrentStrike)
					ntPrintf("%s: resetting attack %s from UpdateStrikeStack ProcessStrike.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString());
				ResetAttack(true);
				EndString();

				// Tell the originator that they hit us
				if ( ( pobStrike->GetOriginatorP() ) 
					 && 
					 ( pobStrike->GetOriginatorP()->GetAttackComponent() ) )
				{
					pobStrike->GetOriginatorP()->GetAttackComponent()->StrikeLanded();
				}

				// Let the Lua state know that we have dropped out
				m_pobParentEntity->GetMessageHandler()->ReceiveMsg<msg_combat_struck>();
			}
			else
			{
				// Clean up the pre strike and move onto the next
				NT_DELETE_CHUNK( Mem::MC_MISC, m_obStrikeStack.front() );
			}

			// Move through the strike stack		
			m_obStrikeStack.pop_front();
		}
	}

	//CGatso::Stop("CAttackComponent::UpdateStrikeStack");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::SetState
*
*	DESCRIPTION		Change the combat to a new state
*
***************************************************************************************************/
void CAttackComponent::SetState( COMBAT_STATE eCombatState, const CStrike * pobStrike )
{
	//CGatso::Start("CAttackComponent::SetState - 1");

	// Set our ragdoll state according to new state change
	SetRagdollState(eCombatState, pobStrike);

	// Call exit function for the current state
	if ( m_eCombatState != eCombatState )
	{
		// Set the state flag
		m_eCombatState = eCombatState;

		// Reset this flag in STANDARD in case we're respawned
		if (m_eCombatState == CS_STANDARD)
			m_bNotifiedToDieOutOfCurrentMovement = false;

		// There are many other areas interseted in the combat state
		if( m_pobParentEntity->IsAI() )
			((AI*)m_pobParentEntity)->GetAIComponent()->CombatStateChanged( m_eCombatState );
	}

	// Clear the state time
	m_fStateTime = 0.0f;

	//CGatso::Stop("CAttackComponent::SetState - 2");
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::SetRagdollState
*
*	DESCRIPTION		Change the ragdoll state whenever we change combat state
*
***************************************************************************************************/
#define PROJECTILE_IMPACT_MASS 5.0f
//#define PROJECTILE_ADD_UP_VELOCITY 2.5f

void CAttackComponent::SetRagdollState( COMBAT_STATE eTo, const CStrike * pobStrike )
{
	Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				
	switch ( eTo )
	{
	case CS_KO:
		{
			if (m_bUseProceduralKODeath)
			{
				// procedural ragdoll means go directly to ragdoll, no animation
				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
					Physics::System::RAGDOLL_DEAD,
					Physics::System::RAGDOLL_IGNORE_CONTACT );

				// should we add impulse from strike? let's say yes for a moment
				if (pobStrike && pobStrike->GetProjectile() && pobStrike->GetHitArea() >= 0)
				{
					const CEntity * proj = pobStrike->GetProjectile();
					const CPoint& pos = proj->GetMatrix().GetTranslation();
					CDirection dir = proj->GetCalcVelocity();
					dir *= PROJECTILE_IMPACT_MASS;
					pobAdvCC->RagdollBodyApplyImpulse(pobStrike->GetHitArea(), pos, dir);

					// add some up velocity to have more cool result
					//pobAdvCC->AddRagdollLinearVelocity(CDirection(0,PROJECTILE_ADD_UP_VELOCITY,0));

				}

			}
			else if (!m_bCannotUseRagdoll
				&&
				!(m_pobAttackDefinition->m_bExemptFromRagdollKOs || m_bEntityExemptFromRagdollKOs) 
				&& 
				!( m_pobStruckStrike && (m_pobStruckStrike->GetAttackDataP()->m_bJuggleKO || (m_pobStruckStrike->GetAttackDataP()->m_bAerialComboStartKO) || m_pobStruckStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_AERIAL_COMBO || m_pobStruckStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_DOWN) )
				&&
				!m_pobParentEntity->IsBoss() )
			{				
				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
					Physics::System::RAGDOLL_ANIMATED_PHYSICALLY,
					Physics::System::RAGDOLL_DEAD_ON_CONTACT );
			}
			else
			{				
				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
					Physics::System::CHARACTER_CONTROLLER,
					Physics::System::CHARACTER_CONTROLLER_COLLIDE_WITH_EVERYTHING );
			}

			m_pobParentEntity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Enable();

			// only power attacks produce BIG_KOs
			if (m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST || 
				m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM)
				m_pobParentEntity->GetPhysicsSystem()->SetKOState(Physics::BIG_KO_BIT);
			else
				m_pobParentEntity->GetPhysicsSystem()->SetKOState(Physics::SMALL_KO_BIT);

			break;
		}
	case CS_DEAD:
		{
			if (m_bCannotUseRagdoll)
			{
				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
							Physics::System::CHARACTER_CONTROLLER,
							Physics::System::CHARACTER_CONTROLLER_COLLIDE_WITH_EVERYTHING );
			}
			// If we're doing a movement of some kind, set to turn raggy on contact so we do as much of the movement as possible
			else if (!m_bUseProceduralKODeath && m_pobParentEntity->GetMovement()->GetNumberOfActiveControllers() > 0)
			{
				// Ground attack?
				if (m_pobStruckStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_DOWN)
				{
					m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
							Physics::System::CHARACTER_CONTROLLER,
							Physics::System::CHARACTER_CONTROLLER_COLLIDE_WITH_EVERYTHING );
				}
				else
				{
					if (pobAdvCC && pobAdvCC->GetRagdollState() != Physics::ANIMATED)
					{
						if (!m_pobParentEntity->IsBoss())
							m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
								Physics::System::RAGDOLL_ANIMATED_PHYSICALLY,
								Physics::System::RAGDOLL_DEAD_ON_CONTACT );
					}
				}
			}
			else // Otherwise, kill us dead now
			{
				if (pobAdvCC && pobAdvCC->GetRagdollState() != Physics::DEAD)
				{
					if (!m_pobParentEntity->IsBoss())
					{						
						m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
							Physics::System::RAGDOLL_DEAD,
							Physics::System::RAGDOLL_IGNORE_CONTACT );

						// should we add impulse from strike? let's say yes for a moment
						if (pobStrike && pobStrike->GetProjectile() && pobStrike->GetHitArea() >= 0)
						{
							const CEntity * proj = pobStrike->GetProjectile();
							const CPoint& pos = proj->GetMatrix().GetTranslation();
							CDirection dir = proj->GetCalcVelocity();
							dir *= PROJECTILE_IMPACT_MASS;
							pobAdvCC->RagdollBodyApplyImpulse(pobStrike->GetHitArea(), pos, dir);

							// add some up velocity to have more cool result
							// pobAdvCC->AddRagdollLinearVelocity(CDirection(0,PROJECTILE_ADD_UP_VELOCITY,0));
						}
					}
				}
			}

			m_pobParentEntity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable();

			break;
		}
	case CS_HELD:
		{
			m_pobParentEntity->GetPhysicsSystem()->SetKOState(Physics::KO_States(0));

			//Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if (m_pobStruckStrike && !m_pobStruckStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull())
			{
				// Need to set to char control first, then switch, incase we were in ragdoll
				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
					Physics::System::CHARACTER_CONTROLLER,
					Physics::System::CHARACTER_CONTROLLER_DO_NOT_COLLIDE );
			}
			
			m_pobParentEntity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable();

			break;
		}
	default: // Everything else we shouldn't have a ragdoll, otherwise we'll get bugs/slowness
		{
			// Only set to non collidable CC when we're attacking with a synchronised attack, all other times it should be a standard collidable CC
			if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->ShouldSync() && eTo == CS_ATTACKING)
			{
				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
					Physics::System::CHARACTER_CONTROLLER,
					Physics::System::CHARACTER_CONTROLLER_DO_NOT_COLLIDE );
			}
			else
			{
				m_pobParentEntity->GetPhysicsSystem()->SetKOState(Physics::KO_States(0));
				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
						Physics::System::CHARACTER_CONTROLLER,
						Physics::System::CHARACTER_CONTROLLER_COLLIDE_WITH_EVERYTHING );
			}

				m_pobParentEntity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable();
			}
	}

	// A little hacktastic - because rigid body char controller isn't robust enough for general game characters, check if this one has been specifically flagged as being able to use it
	// If we're not sync'd or aerial, use the cheapo controller
	if ( m_pobParentEntity->IsCharacter() && m_pobParentEntity->ToCharacter()->GetCanUseCheapCC() && !pobAdvCC->IsRagdollActive() )		
	{
		// In syncs and aerials we should be full, else rigid
		if (m_pobStruckStrike && m_pobStruckStrike->ShouldSync())
			pobAdvCC->SwitchCharacterControllerType(Physics::CharacterController::FULL_CONTROLLER);
		else if ( m_pobStruckStrike && m_pobStruckStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_AERIAL_COMBO )
			pobAdvCC->SwitchCharacterControllerType(Physics::CharacterController::FULL_CONTROLLER);
		else
			pobAdvCC->SwitchCharacterControllerType(Physics::CharacterController::RIGID_BODY);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::ProcessStrike
*
*	DESCRIPTION		Process the strike - returns true if the strike is successful
*
***************************************************************************************************/
bool CAttackComponent::ProcessStrike( CStrike* pobStrike )
{
	//CGatso::Start("CAttackComponent::ProcessStrike");

	//CGatso::Start("CAttackComponent::ProcessStrike - 1");
	// Quick special handling for bosses
	if (m_pobParentEntity->IsBoss())
	{
		Boss* pobBoss = (Boss*)m_pobParentEntity;

		//Hack for demon, no strikes thanks!
		if(pobBoss->GetBossType() == Boss::BT_DEMON)
		{
			//Still reset the timer though...
			pobBoss->ResetTimeSinceLastPlayerStrike();
			return false;
		}

		// Notify the boss that he's under attack, even if we're gonna reject the strike
		m_fTimeTillNotifyNotUnderAttack = pobStrike->GetAttackTime();
		pobBoss->NotifyUnderAttack(true);

		if ( !pobBoss->IsVulnerableTo(pobStrike) )
		{
			//Even if the boss is about to reject this strike, we need to perform certain checks for effect-purposes (uninterruptable
			//and invulnerable hit-effects).
			//This is a heavily cut-down version of the checks below for non-boss-rejected use of the same effects.
			if(m_eCombatState == CS_ATTACKING)
			{
				//If we're in an uninterruptable attack and not in invulnerable window then we want an uninterruptable effect.
				if ((RobustIsInWindow(m_pobMyCurrentStrike->GetAttackDataP()->m_obUninterruptibleWindow)) && (IsInInvulnerabilityWindow() <= 0))
				{
					if (pobStrike->GetOriginatorP() && m_pobParentEntity)
					{
						CombatEffectsTrigger* pobCombatEffectsTrigger = CombatEffectsDefinition::GetEffectsTrigger( pobStrike->GetOriginatorP()->GetAttackComponent()->m_iCurrEffectsTrigger );
						if (pobCombatEffectsTrigger)
						{
							//Hit but uninterruptable effect (still taking damage).
							pobCombatEffectsTrigger->TriggerHitUninterruptableEffect(pobStrike->GetOriginatorP(), m_pobParentEntity);
						}
					}
				}

				// If we are invulnerable and the incoming attack is not a grab or evade then we want the invulnerable effect.
				if ( ( IsInInvulnerabilityWindow() > 0 ) && 
					( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_GOTO && m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD && m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE ) &&
					!pobStrike->GetAttackDataP()->m_bOverrideInvulnerabilityWindow )
				{
					CombatEffectsTrigger* pobCombatEffectsTrigger = CombatEffectsDefinition::GetEffectsTrigger( pobStrike->GetOriginatorP()->GetAttackComponent()->m_iCurrEffectsTrigger );
					if (pobCombatEffectsTrigger)
					{
						//We don't want to play the invulnerable effect if you're just evading nearby.
						if(pobStrike->GetAttackDataP()->m_eAttackClass != AC_EVADE) 
						{
							//Invulnerable effect (uninterruptable and taking no damage).
							pobCombatEffectsTrigger->TriggerInvulnerableEffect(pobStrike->GetOriginatorP(), m_pobParentEntity);
						}
					}

					//Send struck invulnerable message
					Message obMessage(msg_bosscombat_struck_invulnerable);
					m_pobParentEntity->GetMessageHandler()->QueueMessage(obMessage);
				}
			}
			
			return false;
		}
		//If we're recovering, and invulnerable/uninterruptable during recovery, then we want to just play the effect and resume.
		else
		{
			//Check we've got all our components first!
			if(pobBoss && pobBoss->GetCurrentAttackPhase() && pobBoss->GetCurrentAttackPhase()->GetCurrentBossAttackPhaseState())
			{
				//Now get a pointer to the current attack (so that we can query it for various devious flag purposes).
				const BossAttack* pobBossAttack = pobBoss->GetCurrentAttackPhase()->GetCurrentBossAttackPhaseState()->GetCurrentAttack();
				if(pobBossAttack)
				{
					if((m_eCombatState == CS_RECOVERING) && pobBossAttack->IsInvulnerableDuringRecovery())
					{
						//Play invulnerable effect.
						if (pobStrike && pobStrike->GetOriginatorP() && m_pobParentEntity)
						{
							CombatEffectsTrigger* pobCombatEffectsTrigger = CombatEffectsDefinition::GetEffectsTrigger( pobStrike->GetOriginatorP()->GetAttackComponent()->m_iCurrEffectsTrigger );
							if (pobCombatEffectsTrigger)
							{
								//Invulnerable effect (taking damage no damage).
								pobCombatEffectsTrigger->TriggerInvulnerableEffect(pobStrike->GetOriginatorP(), m_pobParentEntity);
							}
						}

						//If the unblocked reaction is RT_HELD then start that reaction and return true.
						if(pobStrike->GetAttackDataP()->m_eUnBlockedReaction == RT_HELD)
						{
							StartReaction( RT_HELD, pobStrike, RML_UNBLOCKED );
							ntPrintf("PROCESSSTRIKE-RETURNCHECK-1\n");
							return true;
						}

						//Return false (no reaction).
						return false;
					}
					else if((m_eCombatState == CS_RECOVERING) && pobBossAttack->IsUnInterruptableDuringRecovery())
					{
						//Play uninterruptable effect.
						if (pobStrike && pobStrike->GetOriginatorP() && m_pobParentEntity)
						{
							CombatEffectsTrigger* pobCombatEffectsTrigger = CombatEffectsDefinition::GetEffectsTrigger( pobStrike->GetOriginatorP()->GetAttackComponent()->m_iCurrEffectsTrigger );
							if (pobCombatEffectsTrigger)
							{
								//Hit but uninterruptable effect (still taking damage).
								pobCombatEffectsTrigger->TriggerHitUninterruptableEffect(pobStrike->GetOriginatorP(), m_pobParentEntity);
							}
						}

						//If the unblocked reaction is RT_HELD then start that reaction and return true.
						if(pobStrike->GetAttackDataP()->m_eUnBlockedReaction == RT_HELD)
						{
							StartReaction( RT_HELD, pobStrike, RML_UNBLOCKED );
							ntPrintf("PROCESSSTRIKE-RETURNCHECK-2\n");
							return true;
						}

						//Return false (no reaction).
						return false;
					}
				}
			}
		}
		// else we go through the motions as below
	}

	//CGatso::Stop("CAttackComponent::ProcessStrike - 1");
	//CGatso::Start("CAttackComponent::ProcessStrike - 2");

	// Make sure that we are not being complete monkeys
	ntAssert( pobStrike->GetOriginatorP() != m_pobParentEntity );
	ntAssert( pobStrike->GetAttackDataP() );

	//if (pobStrike->GetOriginatorP()) 
	//	ntPrintf("%s in %s: Processing Strike - from %s in %s type %s, %f\n", ObjectDatabase::Get().GetNameFromPointer(this->m_pobParentEntity), g_apcCombatStateTable[m_eCombatState], ObjectDatabase::Get().GetNameFromPointer(pobStrike->GetOriginatorP()), g_apcCombatStateTable[pobStrike->GetOriginatorP()->GetAttackComponent()->m_eCombatState],g_apcClassNameTable[pobStrike->GetAttackDataP()->m_eAttackClass], m_fStateTime);

	// If I'm in a safety transition, don't want to be interrupted
	if ( IsInSuperStyleSafetyTransition() && pobStrike->GetOriginatorP() != m_pobSuperStyleSafetyAttacker )
	{	
		return false;
	}

	// If I'm currently doing a sync attack, reject this strike
	// If I'm currently being abused with a syncronised attack from someone, ignore the strike if it's from someone else
	if ( ( m_pobMyCurrentStrike && m_pobMyCurrentStrike->ShouldSync() && !pobStrike->IsCounter() ) ||
		 ( m_pobStruckStrike && m_pobStruckStrike->ShouldSync() && (pobStrike->GetOriginatorP() != m_pobStruckStrike->GetOriginatorP() || pobStrike->IsIncidental()) ) )
	{
		return false;
	}

	// If we're being struck by someone doing a ledge recover, get hit always
	if ( pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent()->IsDoingLedgeRecover() )
	{
		ntPrintf("BASH.\n");
		StartReaction( pobStrike->GetAttackDataP()->m_eUnBlockedReaction, pobStrike, RML_UNBLOCKED );
		ntPrintf("PROCESSSTRIKE-RETURNCHECK-3\n");
		return true;
	}

	// If I'm within the strike exclusion distance specified, and there's no reaction type for being within the exclusion zone, then just reject it 
	if (pobStrike->IsWithinExclusionDistance() && m_pobAttackDefinition->m_eStrikeProximityCheckExclusionDistanceReaction == RT_COUNT)
	{
		return false;
	}

#ifdef REMOVED_ENTITY_INFO
	// Don't process strikes on remotes...
	if ( ( pobStrike->GetTargetP() ) 
		 && 
		 ( pobStrike->GetTargetP()->GetEntityInfo() )
		 && 
		 ( !pobStrike->GetTargetP()->GetEntityInfo()->Net_IsLocal() ) )
	{
		return false;
	}
#endif

	//CGatso::Stop("CAttackComponent::ProcessStrike - 2");
	//CGatso::Start("CAttackComponent::ProcessStrike - 3");

	// Assume we've not got a reaction override
	REACTION_TYPE eUnblockedReaction = pobStrike->GetAttackDataP()->m_eUnBlockedReaction;
	REACTION_TYPE eSpeedBlockedReaction = pobStrike->GetAttackDataP()->m_eSpeedBlockedReaction;
	REACTION_TYPE ePowerBlockedReaction = pobStrike->GetAttackDataP()->m_ePowerBlockedReaction;
	REACTION_TYPE eRangeBlockedReaction = pobStrike->GetAttackDataP()->m_eRangeBlockedReaction;

	// Override them with exlusion distance reaction if neccessary
	if (pobStrike->IsWithinExclusionDistance())
	{
		eUnblockedReaction = eSpeedBlockedReaction = ePowerBlockedReaction = eRangeBlockedReaction = m_pobAttackDefinition->m_eStrikeProximityCheckExclusionDistanceReaction;
	}

	// Check if we need a final strike override
	if (pobStrike->GetAttackDataP()->m_obStrike.GetNumFlippers() > 1)
	{
		// Find the strike flipper with the latest start time
		float fLatestStart = 0.0f;
		CFlipFlop::FlipperContainerType::const_iterator obEndIt = pobStrike->GetAttackDataP()->m_obStrike.End();
		for ( CFlipFlop::FlipperContainerType::const_iterator obIt = pobStrike->GetAttackDataP()->m_obStrike.Begin(); obIt != obEndIt; ++obIt )
		{
			if (obIt->X1(pobStrike->GetAttackTime()) > fLatestStart)
				fLatestStart = obIt->X1(pobStrike->GetAttackTime());
		}

		// If the opponents attack time is > that this, i.e. they're striking us with this last strike window
		if ( pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent()->m_fStateTime >= fLatestStart && pobStrike->GetAttackDataP()->m_eUnBlockedReactionFinalStrikeOverride != RT_COUNT)
		{
			// Use the last strike override reaction type
			eUnblockedReaction = pobStrike->GetAttackDataP()->m_eUnBlockedReactionFinalStrikeOverride;
	}

		// If the blocked reaction type is a stagger, and we're not on the final strike, use deflect until the final strike
		if ( pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent()->m_fStateTime < fLatestStart)
		{
			if (eSpeedBlockedReaction == RT_BLOCK_STAGGER || eSpeedBlockedReaction == RT_IMPACT_STAGGER)
			{
				eSpeedBlockedReaction = RT_DEFLECT;
			} 
			else if (ePowerBlockedReaction == RT_BLOCK_STAGGER || ePowerBlockedReaction == RT_IMPACT_STAGGER)
			{
				ePowerBlockedReaction = RT_DEFLECT;
			}
			else if (eRangeBlockedReaction == RT_BLOCK_STAGGER || eRangeBlockedReaction == RT_IMPACT_STAGGER)
			{
				eRangeBlockedReaction = RT_DEFLECT;
			}
		}
	}

	//CGatso::Stop("CAttackComponent::ProcessStrike - 3");
	//CGatso::Start("CAttackComponent::ProcessStrike - 4");

	// If the character is not in a 'safe' state then ther are a few incoming attacks where they will definately get hit
	if ( ( m_eCombatState != CS_DEAD ) && ( m_eCombatState != CS_DYING ) && ( m_eCombatState != CS_FLOORED ) && ( m_eCombatState != CS_RISE_WAIT ) )
	{
		// If the incoming attack is any of these then we will be hit
		if ( pobStrike->IsCounter() && !pobStrike->IsIncidental() )
		{
			// Else we just get hit unblocked
			StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-4\n");
			return true;
		}

		// If we are being hit by the secondary strike of a syncronised attack
		if ( ( !pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() ) && ( !pobStrike->ShouldSync() ) )
		{
			StartReaction( pobStrike->GetAttackDataP()->m_eSyncdSecondaryReaction, pobStrike, RML_SYNCDSECONDARY );
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-5\n");
			return true;
		}
	}

	// To replicate prestrike blocking, we're allowed to decide to block when the main strike comes in, otherwise if we start blocking between pre and real strike, we wont block when we should do
	// Only players need this, AIs and Bosses should have already decided to block or not basec on pre-strike
	if ((m_eCombatState != CS_BLOCKING) && !pobStrike->ShouldSync() && m_pobParentEntity->IsPlayer() && MayAutoBlockAttack(pobStrike->GetAttackDataP()->m_eAttackClass))
	{
		m_eCombatState = CS_BLOCKING;
	}

	//CGatso::Stop("CAttackComponent::ProcessStrike - 4");

	// Handle the strike according to our current state	
	switch( m_eCombatState )
	{
	case CS_ATTACKING:
		{
			//CGatso::Start("CAttackComponent::ProcessStrike - CS_ATTACKING");

			// If we're in an uninterruptible attack then we can be damaged, but the strike is ignored
			if (m_pobMyCurrentStrike && (RobustIsInWindow(m_pobMyCurrentStrike->GetAttackDataP()->m_obUninterruptibleWindow))
				&& (IsInInvulnerabilityWindow() <= 0))
			{
				bool bContinueProcessingUninterruptible = true; // Assume this is definately an uninterruptible
				if (m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails)
				{
					// Check the stance of the attack against our uninterruptible details to see if we should make this attack interruptible
					
					if (CharacterUninterruptibleVulnerableTo(pobStrike->GetAttackDataP()->m_eAttackClass))
						bContinueProcessingUninterruptible = false; // Actually we are vulnerable
				}

				//We also want to be interruptable even during an uninterruptable window if our outgoing strike AND the incoming strike
				//are both in/near their strike window. They have to overlap within the first (x)% (see below) of this strike for both
				//to be pushed into a stale-mate where both react accordingly and neither gets hit.
				//We force a block-reaction so that both will parry/block... better than the player just ignoring the strike and the enemy
				//being uninterruptable with an effect.
				//NOTE: If this incoming strike is from a projectile then it will have no originator, so we just carry on.
				if(pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent() &&
					(pobStrike->GetOriginatorP()->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING)
					&& m_pobMyCurrentStrike)	//If we are also in a strike when we got hit while both of us are attacking...
				{
					//Rather than checking if we're both in our strike window, we want to see if the strike-window of the opponent's strike
					//is going to start before we're (x)% through our strike window. If so, then they overlap quick enough that we can
					//just push both of us into a stale-mate collision and both block/deflect/stagger/etc appropriately.

					bool bStrikeWindowsOverlapClosely = false;

					//When this incoming strike started (should be negative value).
					float fIncomingStrikeStart = 0.0f;
					//When this incoming strike's strike-window ends (should be positive!)
					float fIncomingStrikeFinish = 0.0f;
					//How long till our strike hits its strike-window, to see if we're quick/close enough.
					float fOurStrikeStart = 0.0f;

					float fIncomingAttackTime = pobStrike->GetAttackTime();
					const CFlipFlop* pIncomingStrikeFlipFlop = &pobStrike->GetAttackDataP()->m_obStrike;
					float fIncomingStateTime = pobStrike->GetOriginatorP()->GetAttackComponent()->GetStateTime();

					float fMyCurrAttackTime = m_pobMyCurrentStrike->GetAttackTime();
					const CFlipFlop* pMyStrikeFlipFlop = &m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike;
					float fMyStateTime = GetStateTime();

					CFlipFlop::FlipperContainerType::const_iterator obEndIt = pIncomingStrikeFlipFlop->End();
					for(CFlipFlop::FlipperContainerType::const_iterator obIt = pIncomingStrikeFlipFlop->Begin() ; obIt != obEndIt ; ++obIt)
					{
						//Find the start point of the incoming window.
						//Find the end point of the incoming window.
						fIncomingStrikeStart = obIt->X1(fIncomingAttackTime);
						fIncomingStrikeFinish = obIt->X2(fIncomingAttackTime);
						break;
					}

					obEndIt = pMyStrikeFlipFlop->End();
					for(CFlipFlop::FlipperContainerType::const_iterator obIt = pMyStrikeFlipFlop->Begin() ; obIt != obEndIt ; ++obIt)
					{
						fOurStrikeStart = obIt->X1(fMyCurrAttackTime);
						break;
					}

					//Now that we've got all of the facts, we need to put them together from relative-times to absolute times
					//so that we can compare them.
					fIncomingStrikeStart -= fIncomingStateTime;
					fIncomingStrikeFinish -= fIncomingStateTime;
					fOurStrikeStart -= fMyStateTime;
					
					//Nice self-explanatory (and extremely bloaty) variable name.
					float fPercentageThroughFirstStrikeThatOpponentStrikeStarts = 101.0f;	//The impossible is real! :P
					if(fOurStrikeStart < fIncomingStrikeFinish)		//Important to be < not <=, avoids divide by 0 then too.
					{
						//Make sure we have no negative values (put everything relative to the incoming strike).
						if(fIncomingStrikeStart < 0.0f)
						{
							fIncomingStrikeFinish -= fIncomingStrikeStart;
							fOurStrikeStart -= fIncomingStrikeStart;
							fIncomingStrikeStart = 0.0f;
						}
						//If we're the second strike, we may need to flip the timings around.
						if(fOurStrikeStart >= fIncomingStrikeStart)
						{
							fPercentageThroughFirstStrikeThatOpponentStrikeStarts = ((fOurStrikeStart - fIncomingStrikeStart) /
																					(fIncomingStrikeFinish - fIncomingStrikeStart)) * 100.0f;	//Percent.
						}
						else
						{
							fPercentageThroughFirstStrikeThatOpponentStrikeStarts = ((fIncomingStrikeStart - fOurStrikeStart) /
																					(fIncomingStrikeFinish - fOurStrikeStart)) * 100.0f;	//Percent.
						}
					}

					if(fPercentageThroughFirstStrikeThatOpponentStrikeStarts < 40.0f)	//Within the first 40%.
					{
						bStrikeWindowsOverlapClosely = true;
					}

					if(bStrikeWindowsOverlapClosely)
					{
						//We're both in our strike window, so we're actually interruptable for this special case.
						if (m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_FAST
							|| m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_MEDIUM)
						{
							//React as if we speed-blocked their attack.
							REACTION_TYPE eReaction = pobStrike->GetAttackDataP()->m_eSpeedBlockedReaction;
							StartReaction(eReaction, pobStrike, RML_SPEEDBLOCKED);
						}
						else if (m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST ||
							m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM)
						{
							//React as if we power-blocked their attack.
							REACTION_TYPE eReaction = pobStrike->GetAttackDataP()->m_ePowerBlockedReaction;
							StartReaction(eReaction, pobStrike, RML_POWERBLOCKED);
						}
						else if (m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST ||
							m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM)
						{
							//React as if we range-blocked their attack.
							REACTION_TYPE eReaction = pobStrike->GetAttackDataP()->m_eRangeBlockedReaction;
							StartReaction(eReaction, pobStrike, RML_RANGEBLOCKED);
						}

						ntPrintf("PROCESSSTRIKE-RETURNCHECK-6\n");
						return true;
					}
				}

				if (bContinueProcessingUninterruptible)
				{
					// Take damage
					m_pobParentEntity->ChangeHealth(-GetDamageFromStrike(pobStrike), "Was struck");

					// Do a sparkley effect
					if (pobStrike->GetOriginatorP() && m_pobParentEntity)
					{
						CombatEffectsTrigger* pobCombatEffectsTrigger = CombatEffectsDefinition::GetEffectsTrigger( pobStrike->GetOriginatorP()->GetAttackComponent()->m_iCurrEffectsTrigger );
						if (pobCombatEffectsTrigger)
						{
							//Hit but uninterruptable effect (still taking damage).
							pobCombatEffectsTrigger->TriggerHitUninterruptableEffect(pobStrike->GetOriginatorP(), m_pobParentEntity);
						}
					}

					// Send struck uninterruptible message
					Message obMessage(msg_combat_struck_uninterruptible);
					m_pobParentEntity->GetMessageHandler()->QueueMessage(obMessage);
					
					// If we're not dead after this damage, ignore the strike
					if (!m_pobParentEntity->IsDead())
					{						
						//CGatso::Stop("CAttackComponent::ProcessStrike - CS_ATTACKING");
						//CGatso::Stop("CAttackComponent::ProcessStrike");
						return false;
					}
					else
					{
						// If in sync attack, drop other player
						if ( m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetAttackDataP() && !m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() )
						{
							if ( m_pobMyCurrentStrike->GetTargetP() && m_pobMyCurrentStrike->GetTargetP()->GetMessageHandler() )
							{
								CEntity* pobNonConst = const_cast<CEntity*>(m_pobMyCurrentStrike->GetTargetP());
								// Make sure they've got a record of the strike that's hit them
								ntError(pobNonConst->GetAttackComponent()->m_pobStruckStrike && !pobNonConst->GetAttackComponent()->m_pobStruckStrike->IsIncidental());
								pobNonConst->GetAttackComponent()->InterruptSyncAttack();
							}
						}
					}
				}
			}

			// If we are invulnerable and the incoming attack is not a grab then we can't be hit
			if ( ( IsInInvulnerabilityWindow() > 0 ) && 
				( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_GOTO && m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD && m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE ) &&
				!pobStrike->GetAttackDataP()->m_bOverrideInvulnerabilityWindow )
			{
				//We check that this strike has an originator with an attack component (in-case it came from a projectile perhaps?)
				if(pobStrike && pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent())
				{
					CombatEffectsTrigger* pobCombatEffectsTrigger = CombatEffectsDefinition::GetEffectsTrigger( pobStrike->GetOriginatorP()->GetAttackComponent()->m_iCurrEffectsTrigger );
					if (pobCombatEffectsTrigger)
					{
						//We don't want to play the invulnerable effect if we're just evading nearby.
						if(m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass != AC_EVADE)
						{
							//Invulnerable effect (uninterruptable and taking no damage).
							pobCombatEffectsTrigger->TriggerInvulnerableEffect(pobStrike->GetOriginatorP(), m_pobParentEntity);
						}
					}
				}

				//If this strike came from a projectile and we're evading or invulnerable to it (not deflecting), then send a
				//msg_combat_missedprojectilecounter to the projectile just so that it knows it had a strike but was not deflected
				//(and therefore not countered).
				//In most cases, projectiles will just want to do the same thing here as if they were deflected but not countered.
				if(pobStrike && pobStrike->GetProjectile() && pobStrike->GetProjectile()->IsProjectile())
				{
					Message obMessage(msg_combat_missedprojectilecounter);
					obMessage.SetEnt("Counterer",m_pobParentEntity);
					CEntity* pobProjectile = const_cast<CEntity*>(pobStrike->GetProjectile());
					if(pobProjectile && pobProjectile->GetMessageHandler())
					{
						pobProjectile->GetMessageHandler()->Receive(obMessage);
					}
				}

				//CGatso::Stop("CAttackComponent::ProcessStrike - CS_ATTACKING");
				//CGatso::Stop("CAttackComponent::ProcessStrike");
				return false;
			}

			// If i am doing a counter attack and the target is the character striking me now then return false
			if ( ( m_pobMyCurrentStrike->IsCounter() ) && ( m_pobMyCurrentStrike->GetTargetP() == pobStrike->GetOriginatorP() ) )
			{
				// If we are about to counter them we can ignore the strike
				//CGatso::Stop("CAttackComponent::ProcessStrike - CS_ATTACKING");
				//CGatso::Stop("CAttackComponent::ProcessStrike");
				return false;
			}

			// If we are evading there are some special circumstances
			if ( m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_EVADE )
			{
				// If the incoming attack is of the range variety we completely ignore it
				if ( ( pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST )
					 ||
					 ( pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM ) )
				{
					// They can't hit us at all
					//CGatso::Stop("CAttackComponent::ProcessStrike - CS_ATTACKING");
					//CGatso::Stop("CAttackComponent::ProcessStrike");
					return false;
				}
			}

/*
			//REMOVED: Player-Priority only takes effect when trying to throw now (see below).
			// If I'm the player, and I'm in my strike window for my attack, I can ignore this incoming attack
			if (m_pobMyCurrentStrike && IsInStrikeWindow() && m_pobParentEntity->IsPlayer())
			{
				// If the incoming is a grab, or an evade combo, we can't ignore it
				if (pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_GOTO && 
					pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD && 
					pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE &&
					(pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent() && !pobStrike->GetOriginatorP()->GetAttackComponent()->m_bEvadeCombo ) )
				{
					//CGatso::Stop("CAttackComponent::ProcessStrike - CS_ATTACKING");
					//CGatso::Stop("CAttackComponent::ProcessStrike");
					return false;
				}
			}
*/
			if (m_pobMyCurrentStrike && IsInStrikeWindow() && m_pobParentEntity->IsPlayer())
			{
				// If the incoming isn't a grab, or an evade combo, AND we're currently performing a grab-hold or
				// grab-strike, then we can ignore the incoming strike, ours takes priority.
				if (pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_GOTO && 
					pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD && 
					pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE &&
					(pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent() && !pobStrike->GetOriginatorP()->GetAttackComponent()->m_bEvadeCombo) &&
					(m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE))
				{
					//CGatso::Stop("CAttackComponent::ProcessStrike - CS_ATTACKING");
					//CGatso::Stop("CAttackComponent::ProcessStrike");
					return false;
				}
			}

			// From here on in we are going to get hit - if we are currently doing a syncronised attack 
			// then we need to make sure that the character we are doing it on is dropped
			if ( m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetAttackDataP() && !m_pobMyCurrentStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() )
			{
				// We should definately have a target here but just to be safe
				if ( m_pobMyCurrentStrike->GetTargetP() && m_pobMyCurrentStrike->GetTargetP()->GetMessageHandler() )
				{
					CEntity* pobNonConst = const_cast<CEntity*>(m_pobMyCurrentStrike->GetTargetP());
					pobNonConst->GetAttackComponent()->InterruptSyncAttack();
				}	
			}

			// If we are in our intercept window and the incoming attack is a range attack then we can be KOd
			// Also if we're in our regular intercept window, we'll get hit
			if ( ( ( IsInRangeInterceptWindow() > 0 )
				 &&
				 ( ( pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST )
				   ||
				   ( pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM ) ) )
				 ||
				 ( IsInInterceptWindow() ) )
			{
				StartReaction( RT_KO, pobStrike, RML_UNBLOCKED );
			}

			// If this character is autoblocking we take the current stance
			else if ( !m_pobParentEntity->IsBoss() && MayAutoBlockAttack(pobStrike->GetAttackDataP()->m_eAttackClass) )
			{
				switch( m_eCurrentStance )
				{
				case ST_SPEED:	StartReaction( eSpeedBlockedReaction, pobStrike, RML_SPEEDBLOCKED );	break;
				case ST_RANGE:	StartReaction( eRangeBlockedReaction, pobStrike, RML_RANGEBLOCKED );	break;
				case ST_POWER:	StartReaction( ePowerBlockedReaction, pobStrike, RML_POWERBLOCKED );	break;
				default:		ntAssert( 0 );																		break;
				}
			}
			else if ( m_pobParentEntity->IsBoss() )
			{
				Boss* pobBoss = (Boss*)m_pobParentEntity;
				if ( MayAutoBlockAttack(pobStrike->GetAttackDataP()->m_eAttackClass) && pobBoss->ShouldBlock(pobStrike) )
				{
					if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_MEDIUM)
					{
						StartReaction( eSpeedBlockedReaction, pobStrike, RML_SPEEDBLOCKED );
					}
					else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM)
					{
						StartReaction( ePowerBlockedReaction, pobStrike, RML_POWERBLOCKED );
					}
					else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM)
					{
						StartReaction( eRangeBlockedReaction, pobStrike, RML_RANGEBLOCKED );
					}	
					else
					{
						// Otherwise we just start an unblocked reaction
						StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
					}
				}
				else
				{
					// Otherwise we just start an unblocked reaction
					StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
				}
			}	

			// Otherwise we just start an unblocked reaction
			else
			{
				//ntPrintf("%s: ProcessStrike - Starting reaction %i\n", ObjectDatabase::Get().GetNameFromPointer(this->m_pobParentEntity), pobStrike->GetAttackDataP()->m_eUnBlockedReaction);

				StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
			}

			// We have reacted
			//CGatso::Stop("CAttackComponent::ProcessStrike - CS_ATTACKING");
			//CGatso::Stop("CAttackComponent::ProcessStrike");
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-7\n");
			return true;
		}
		break;

	case CS_FLOORED:
	case CS_RISE_WAIT:
		{
			//CGatso::Start("CAttackComponent::ProcessStrike - CS_FLOORED/CS_RISE_WAIT");

			// If this is not a strike designed to hit folk on the floor
			if ( pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST || pobStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_DOWN )
			{
				// Start an unblocked reaction
				StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
				//CGatso::Stop("CAttackComponent::ProcessStrike - CS_FLOORED/CS_RISE_WAIT");
				//CGatso::Stop("CAttackComponent::ProcessStrike");
				ntPrintf("PROCESSSTRIKE-RETURNCHECK-8\n");
				return true;
			}
			else 
			{
				//CGatso::Stop("CAttackComponent::ProcessStrike - CS_FLOORED/CS_RISE_WAIT");
				//CGatso::Stop("CAttackComponent::ProcessStrike");

				//If this strike came from a projectile and we're coming up from KO etc (not deflecting), then send a
				//msg_combat_missedprojectilecounter to the projectile just so that it knows it had a strike but was not deflected
				//(and therefore not countered).
				//In most cases, projectiles will just want to do the same thing here as if they were deflected but not countered.
				if(pobStrike && pobStrike->GetProjectile() && pobStrike->GetProjectile()->IsProjectile())
				{
					Message obMessage(msg_combat_missedprojectilecounter);
					obMessage.SetEnt("Counterer",m_pobParentEntity);
					CEntity* pobProjectile = const_cast<CEntity*>(pobStrike->GetProjectile());
					if(pobProjectile && pobProjectile->GetMessageHandler())
					{
						pobProjectile->GetMessageHandler()->Receive(obMessage);
					}
				}

				return false;
			}
		}
		break;

	case CS_KO:
		{
			//CGatso::Start("CAttackComponent::ProcessStrike - CS_KO");

			// If this strike is from an object - it will have no orignator - do not react to these
			if ( !pobStrike->GetOriginatorP() )
			{
				//CGatso::Stop("CAttackComponent::ProcessStrike - CS_KO");
				//CGatso::Stop("CAttackComponent::ProcessStrike");

				//If this strike came from a projectile and we're currently KO'd (so not deflecting), then send a
				//msg_combat_missedprojectilecounter to the projectile just so that it knows it had a strike but was not deflected
				//(and therefore not countered).
				//In most cases, projectiles will just want to do the same thing here as if they were deflected but not countered.
				if(pobStrike && pobStrike->GetProjectile() && pobStrike->GetProjectile()->IsProjectile())
				{
					Message obMessage(msg_combat_missedprojectilecounter);
					obMessage.SetEnt("Counterer",m_pobParentEntity);
					CEntity* pobProjectile = const_cast<CEntity*>(pobStrike->GetProjectile());
					if(pobProjectile && pobProjectile->GetMessageHandler())
					{
						pobProjectile->GetMessageHandler()->Receive(obMessage);
					}
				}

				return false;
			}

			// If we're ragdolled, we don't want to react
			Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if (pobAdvCC)
			{
				if (pobAdvCC->IsRagdollActive())
				{
					//CGatso::Stop("CAttackComponent::ProcessStrike - CS_KO");
					//CGatso::Stop("CAttackComponent::ProcessStrike");
					return false;
			}
			}

			// Clear the data that describes what we are currently being hit by
			StartReaction( RT_KO, pobStrike, RML_UNBLOCKED );
			//CGatso::Stop("CAttackComponent::ProcessStrike - CS_KO");
			//CGatso::Stop("CAttackComponent::ProcessStrike");
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-9\n");
			return true;
		}
		break;

	case CS_HELD:
		{
			//CGatso::Start("CAttackComponent::ProcessStrike - CS_HELD");

			if (!IsInSuperStyleSafetyTransition())
			{
				ntError(m_pobStruckStrike);

				// When held we shouldn't take any other strikes except those from whoever's holding us
				if (m_pobStruckStrike && m_pobStruckStrike->GetOriginatorP() == pobStrike->GetOriginatorP())
				{
					// Clear the data that describes what we are currently being hit by
					StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
					//CGatso::Stop("CAttackComponent::ProcessStrike - CS_HELD");
					//CGatso::Stop("CAttackComponent::ProcessStrike");
					ntPrintf("PROCESSSTRIKE-RETURNCHECK-10\n");
					return true;
				}
				else
				{
					ntError(0);
				}
			}
			else
			{

				ntError(m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD);
			}

			//CGatso::Stop("CAttackComponent::ProcessStrike - CS_HELD");
			//CGatso::Stop("CAttackComponent::ProcessStrike");
			return false;
		}
		break;

	case CS_INSTANTRECOVER:
		{
			// This state is obselete
			ntError(0);

			// We can only be deflected if we are in the instant recovery state
			StartReaction( RT_DEFLECT, pobStrike, RML_SPEEDBLOCKED );
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-11\n");
			return true;
		}
		break;

	case CS_STANDARD:
	case CS_RECOVERING:
	case CS_DEFLECTING:
		{
			//CGatso::Start("CAttackComponent::ProcessStrike - CS_STANDARD/CS_RECOVERING/CS_DEFLECTING");

			// If we are in a rising recovery we can only be deflected
			if ( ( m_eCombatState == CS_RECOVERING ) && ( m_eRecoveryType == RC_RISING ) )
			{
				switch( m_eCurrentStance )
				{
				case ST_SPEED:	StartReaction( RT_DEFLECT, pobStrike, RML_SPEEDBLOCKED );	break;
				case ST_RANGE:	StartReaction( RT_DEFLECT, pobStrike, RML_RANGEBLOCKED );	break;
				case ST_POWER:	StartReaction( RT_DEFLECT, pobStrike, RML_POWERBLOCKED );	break;
				default:		ntAssert( 0 );																		break;
				}
			}
			// If we are doing a specific grab recovery then we get hit
			else if ( ( m_eCombatState == CS_RECOVERING ) && ( m_eRecoveryType == RC_GRAB ) )
				StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
			// Bosses block using their specialness
			else if (m_pobParentEntity->IsBoss())
			{
				Boss* pobBoss = (Boss*)m_pobParentEntity;
				if ( MayAutoBlockAttack(pobStrike->GetAttackDataP()->m_eAttackClass) && pobBoss->ShouldBlock(pobStrike) )
				{
					if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_MEDIUM)
					{
						StartReaction( eSpeedBlockedReaction, pobStrike, RML_SPEEDBLOCKED );
					}
					else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM)
					{
						StartReaction( ePowerBlockedReaction, pobStrike, RML_POWERBLOCKED );
					}
					else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM)
					{
						StartReaction( eRangeBlockedReaction, pobStrike, RML_RANGEBLOCKED );
					}	
					else
					{
						// Otherwise we just start an unblocked reaction
						StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
					}
				}
				else
				{
					// Otherwise we just start an unblocked reaction
					StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
				}
			}
			// Characters block acc to stance
			else if ( MayAutoBlockAttack(pobStrike->GetAttackDataP()->m_eAttackClass) )
			{
				switch( m_eCurrentStance )
				{
				case ST_SPEED:	StartReaction( eSpeedBlockedReaction, pobStrike, RML_SPEEDBLOCKED );	break;
				case ST_RANGE:	StartReaction( eRangeBlockedReaction, pobStrike, RML_RANGEBLOCKED );	break;
				case ST_POWER:	StartReaction( ePowerBlockedReaction, pobStrike, RML_POWERBLOCKED );	break;
				default:		ntAssert( 0 );																		break;
				}
			}
			else
			{
				// Otherwise we just start an unblocked reaction
				StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
			}

			//CGatso::Stop("CAttackComponent::ProcessStrike - CS_STANDARD/CS_RECOVERING/CS_DEFLECTING");
			//CGatso::Stop("CAttackComponent::ProcessStrike");
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-12\n");
			return true;
		}
		break;

	case CS_DYING:
		{
			//CGatso::Start("CAttackComponent::ProcessStrike - CS_DYING");

			// If this is a grab, just do it, but only if it's from who was attacking us before
			// Last time I saw this used, it caused a crash on PC, so I'm taking it out for E3
			// BUT, if you see the hero superstyling by herself often, it may well be because the person she wanted to superstyle was stuck in a DYING state and couldn't handle the strike here
			// So try uncommenting this if you want a potential fix, if it crashes again though, comment it right out again!
			/*if ( (pobStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_GOTO || pobStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || pobStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE) && m_pobStruckStrike && m_pobStruckStrike->GetOriginatorP() == pobStrike->GetOriginatorP() )				
				StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );			
			// If it's an aerial hit, KO again
			else*/ if ( pobStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_AERIAL_COMBO )
				StartReaction( RT_KO, pobStrike, RML_UNBLOCKED );
			// ...otherwise we just do it again
			else
				StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );

			// Mark their success
			//CGatso::Stop("CAttackComponent::ProcessStrike - CS_DYING");
			//CGatso::Stop("CAttackComponent::ProcessStrike");
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-13\n");
			return true;
		}
		break;

	case CS_BLOCKING:
		{
			//CGatso::Start("CAttackComponent::ProcessStrike - CS_BLOCKING");

			// If this is a player then we've already done an autoblocking check above, we take the current stance and block using that
			if ( m_pobParentEntity->IsPlayer() )
			{
				switch( m_eCurrentStance )
				{
				case ST_SPEED:	StartReaction( eSpeedBlockedReaction, pobStrike, RML_SPEEDBLOCKED );	break;
				case ST_RANGE:	StartReaction( eRangeBlockedReaction, pobStrike, RML_RANGEBLOCKED );	break;
				case ST_POWER:	StartReaction( ePowerBlockedReaction, pobStrike, RML_POWERBLOCKED );	break;
				default:		ntAssert( 0 );																		break;
				}
			}
			else if ( m_pobParentEntity->IsBoss() )
			{
				// If we're boos and we're blocking, then we can assume the logic of whether we should block is done, and we should just match the incoming attack with a block
				// Ignore stance or block type, we just do choose reaction from the stance of the incoming attack
				if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_MEDIUM)
				{
					StartReaction( eSpeedBlockedReaction, pobStrike, RML_SPEEDBLOCKED );
				}
				else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM)
				{
					StartReaction( ePowerBlockedReaction, pobStrike, RML_POWERBLOCKED );
				}
				else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM)
				{
					StartReaction( eRangeBlockedReaction, pobStrike, RML_RANGEBLOCKED );
				}	
				else
					ntAssert( 0 );
			}
			// Otherwise AIs go off the current block type that they have set themselves
			else
			{
				switch( m_eBlockType )
				{
				case BT_SPEED:	StartReaction( eSpeedBlockedReaction, pobStrike, RML_SPEEDBLOCKED );	break;
				case BT_RANGE:	StartReaction( eRangeBlockedReaction, pobStrike, RML_RANGEBLOCKED );	break;
				case BT_POWER:	StartReaction( ePowerBlockedReaction, pobStrike, RML_POWERBLOCKED );	break;
				case BT_GRAB:	
					ntPrintf("%s: You were trying a grab block here but it should have been dealt with earlier, tell Duncan what you were doing to make this happen!\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity)));
					StartReaction( eSpeedBlockedReaction, pobStrike, RML_SPEEDBLOCKED );	
					break;
				default:		ntAssert( 0 );																		break;
				}
			}

			// We always react
			//CGatso::Stop("CAttackComponent::ProcessStrike - CS_BLOCKING");
			//CGatso::Stop("CAttackComponent::ProcessStrike");
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-14\n");
			return true;
		}
		break;

	case CS_DEAD:
		{
			// We don't do anything
			return false;
		}
		break;

	default: // Recoiling, staggering
		{
			//CGatso::Start("CAttackComponent::ProcessStrike - default");

			// ...an unblocked reaction
			StartReaction( eUnblockedReaction, pobStrike, RML_UNBLOCKED );
			//CGatso::Stop("CAttackComponent::ProcessStrike - default");
			//CGatso::Stop("CAttackComponent::ProcessStrike");
			ntPrintf("PROCESSSTRIKE-RETURNCHECK-15\n");
			return true;
		}
		break;
	}

	// Nothing to do if we are here
	//CGatso::Stop("CAttackComponent::ProcessStrike");
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartReaction
*
*	DESCRIPTION		Start a reaction to a strike - the reaction type that should be used needs to
*					be specified.
*
***************************************************************************************************/
void CAttackComponent::StartReaction( REACTION_TYPE eReactionType, const CStrike* pobStrike, REACTION_MATRIX_LOOKUP eReactionMatrixLookup )
{
	// Clear any held button attacks
	m_bNewButtonHeldAttack = false;
	m_obAttackTracker.SetIsWaitingForHeld(false);
	m_fAttackButtonTime = -1.0f;

	// Make sure any current strike is cleared up
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;

	ntAssert(pobStrike);

	// Find the attack zone
	m_eStruckReactionZone = ChooseReactionZone( m_pobParentEntity, pobStrike );

	// If we have a reaction matrix, use it to look up a specific reaction
	if (m_pobAttackDefinition->m_pobReactionMatrix && !pobStrike->ShouldSync()) 
	{
		// Get the correct column from our matrix and check it
		ReactionMatrixColumn* pobReactionMatrixColumn = m_pobAttackDefinition->m_pobReactionMatrix->GetReactionMatrixColumn(eReactionMatrixLookup);
		if (!pobReactionMatrixColumn)
		{
			user_warn_p( false, ("No ReactionMatrixColumn found. Falling back onto non-matrix lookup reaction.") );
		}
		else
		{
			// Get the correct column entry from our column and check it
			ReactionMatrixColumnEntry* pobReactionMatrixColumnEntry = pobReactionMatrixColumn->GetReactionMatrixColumnEntry(pobStrike->GetAttackDataP()->m_eAttackClass);
			if (!pobReactionMatrixColumnEntry)
			{
				user_warn_p( false, ("No ReactionMatrixColumnEntry found. Falling back onto non-matrix lookup reaction.") );
			}
			else
			{
				// Get the new reaction type and check it
				REACTION_TYPE eTempReactionType = pobReactionMatrixColumnEntry->GetReactionType(eReactionType);
				if (eTempReactionType == RT_COUNT)
				{
					user_warn_p( false, ("Bad reaction type (RT_COUNT) in ReactionMatrixColumnEntry. Falling back onto non-matrix lookup reaction.") );
				}
				else 
				{ // Use this reaction
					eReactionType = eTempReactionType;
				}
			}
		}
	}

	// If we're in a special debug mode, kill everything not human!
	if (g_ShellOptions->m_bOneHitKills && this->m_pobParentEntity->IsAI())
	{
		eReactionType = RT_KILL;
	}

	CHashedString m_obChosenReceiverAnim;
	if(pobStrike && pobStrike->GetAttackDataP())
	{
		const CAttackData* pAttackData = pobStrike->GetAttackDataP();
		switch(eReactionMatrixLookup)
		{
		case RML_UNBLOCKED:
			m_obChosenReceiverAnim = pAttackData->m_obReceiverAnim;
			break;
		case RML_SPEEDBLOCKED:
			m_obChosenReceiverAnim = pAttackData->m_obReceiverAnimSpeedBlocked;
			break;
		case RML_RANGEBLOCKED:
			m_obChosenReceiverAnim = pAttackData->m_obReceiverAnimRangeBlocked;
			break;
		case RML_POWERBLOCKED:
			m_obChosenReceiverAnim = pAttackData->m_obReceiverAnimPowerBlocked;
			break;
		case RML_SYNCDSECONDARY:
			m_obChosenReceiverAnim = pAttackData->m_obReceiverAnimSyncdSecondaryReaction;
			break;
		default:
			ntAssert(0);	//Could be bad data in ReactionMatrix.
			break;
		}

		// Taking default to receiver anim out because we need anims to be specific only to their reaction types
		/*if(ntStr::IsNull(m_obChosenReceiverAnim))
		{
			m_obChosenReceiverAnim = pAttackData->m_obReceiverAnim;
		}*/
	}

	switch ( eReactionType )
	{
	case RT_DEFLECT:			StartDeflecting( pobStrike, m_obChosenReceiverAnim);		break;
	case RT_BLOCK_STAGGER:		StartBlockStagger( pobStrike, m_obChosenReceiverAnim );		break;
	case RT_IMPACT_STAGGER:		StartImpactStagger( pobStrike, m_obChosenReceiverAnim );	break;
	case RT_RECOIL:				StartRecoil( pobStrike, m_obChosenReceiverAnim );			break;
	case RT_HELD:				StartHeld( pobStrike, m_obChosenReceiverAnim );				break;
	case RT_KO:					StartKO( pobStrike, m_obChosenReceiverAnim );				break;
	case RT_KILL:				
		if(pobStrike && pobStrike->GetProjectile() && pobStrike->GetProjectile()->IsProjectile())
			StartKill( pobStrike, m_obChosenReceiverAnim, false );
		else
			StartKill( pobStrike, m_obChosenReceiverAnim, true );
		break;
	default:					ntAssert( 0 );												break; // Could be bad data in ReactionMatrix
	}

	//If this strike came from a projectile and we're not deflecting, then send a msg_combat_missedprojectilecounter to the
	//projectile just so that it knows it had a strike but was not deflected (and therefore not countered).
	//In most cases, projectiles will just want to do the same thing here as if they were deflected but not countered.
	if(pobStrike && pobStrike->GetProjectile() && pobStrike->GetProjectile()->IsProjectile())
	{
		if(eReactionType != RT_DEFLECT)
		{
			Message obMessage(msg_combat_missedprojectilecounter);
			obMessage.SetEnt("Counterer",m_pobParentEntity);
			CEntity* pobProjectile = const_cast<CEntity*>(pobStrike->GetProjectile());
			if(pobProjectile && pobProjectile->GetMessageHandler())
			{
				pobProjectile->GetMessageHandler()->Receive(obMessage);
			}
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AttackIsValidCounter
*
*	DESCRIPTION		Defines the counter rules of the game.  Fast attacks can currently counter both
*					medium and fast attacks of the same stance.
*
***************************************************************************************************/
bool CAttackComponent::AttackIsValidCounter( ATTACK_CLASS eCurrentAttack, ATTACK_CLASS ePossibleCounter )
{
	// Just dealing with the simple cases for now
	switch( eCurrentAttack )
	{
	case AC_POWER_MEDIUM:
	case AC_POWER_FAST:
		if ( ePossibleCounter == AC_POWER_FAST )	return true;
		break;

	case AC_SPEED_MEDIUM:
	case AC_SPEED_FAST:
		if ( ePossibleCounter == AC_SPEED_FAST )	return true;
		break;

	case AC_RANGE_MEDIUM:
	case AC_RANGE_FAST:
		if ( ePossibleCounter == AC_RANGE_FAST )	return true;
		break;

	case AC_GRAB_GOTO:
	case AC_GRAB_STRIKE:
	case AC_GRAB_HOLD:
		if ( ePossibleCounter == AC_GRAB_GOTO || ePossibleCounter == AC_GRAB_HOLD || ePossibleCounter == AC_GRAB_STRIKE )
			return true;
		break;

	default:
		break;
	}

	// If we are here then the counter is not valid
	return false;
}

	
/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::MayAutoBlockAttack
*
*	DESCRIPTION		Would it be possible for this character to auto block an attack at the moment?
*
***************************************************************************************************/
bool CAttackComponent::MayAutoBlockAttack( ATTACK_CLASS eAttackClass ) const
{
	// If we have turned off auto blocking globally then we can't auto block
	if ( !m_bGlobablAutoBlockEnable )
		return false;

	if( m_pobParentEntity->IsPlayer() || m_pobParentEntity->IsBoss() )
	{
		// If we are not in a state in which blocking can take place
		if ( !CanBlock(AttackClassToBlockType(eAttackClass)) )
			return false;
	}
	else
	{
		// If not blocking.. return false. 
		if ( m_eCombatState != CS_BLOCKING )
			return false;

		// If we are not in a state in which blocking can take place
		if ( !CanBlock(AttackClassToBlockType(eAttackClass)) )
			return false;
	}



	// If we are here then all is well
	return true;
}

BLOCK_TYPE CAttackComponent::AttackClassToBlockType(ATTACK_CLASS eAttackClass) const
{
	if (eAttackClass == AC_POWER_FAST || eAttackClass == AC_POWER_MEDIUM)
	{
		return BT_POWER;
	}
	else if (eAttackClass == AC_RANGE_FAST || eAttackClass == AC_RANGE_MEDIUM) 
	{
		return BT_RANGE;
	} // Grabs revert to speed at the moment
	else if (eAttackClass == AC_SPEED_FAST || eAttackClass == AC_SPEED_MEDIUM) 
	{
		return BT_SPEED;
	}
	else if (eAttackClass == AC_GRAB_GOTO || eAttackClass == AC_GRAB_HOLD || eAttackClass == AC_GRAB_STRIKE) 
	{
		return BT_GRAB;
	}

	// Shouldn't get here
	ntAssert(0);
	return BT_COUNT;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::ProcessPreStrike
*
*	DESCRIPTION		Process the _PRE_ strike
*
***************************************************************************************************/
bool CAttackComponent::ProcessPreStrike(CStrike* pobStrike)
{
	// Quick special handling for bosses
	if (m_pobParentEntity->IsBoss())
	{
		Boss* pobBoss = (Boss*)m_pobParentEntity;

		//Hack for demon, no strikes thanks!
		if(pobBoss->GetBossType() == Boss::BT_DEMON)
		{
			return false;
		}

		bool bVuln = pobBoss->IsVulnerableTo(pobStrike);

		// If we're not totally invulnerable to this attack, if we can block it, start a block
		if (bVuln && MayAutoBlockAttack(pobStrike->GetAttackDataP()->m_eAttackClass) && pobBoss->ShouldBlock(pobStrike))
		{
			if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_MEDIUM)
			{
				StartBlock( pobStrike, BT_SPEED );
			}
			else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM)
			{
				StartBlock( pobStrike, BT_POWER );
			}
			else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM)
			{
				StartBlock( pobStrike, BT_RANGE );
			}
			else
			{
				ntError_p(false, ("Boss is vulnerable to strike, but attack-class matches none of those checked for... possibly bad data. Tell GavC how you did this"));
			}

			pobBoss->NotifyWillBlockIncomingStrike(pobStrike);

			return true;
		}	
		// If we're completely invulnerable to it's effects, ignore it
		else if (!bVuln)
		{
			pobBoss->NotifyInvulnerableToIncomingStrike(pobStrike);
			return false;
		}
		// else notify that we're gonna get hit
		else
		{
			pobBoss->NotifyVulnerableToIncomingStrike(pobStrike);
			return false;
		}
	}
	// For everyone else...
	else
	{
		// Ensure that if we're within exclusion distance, we have some reaction type
		if (pobStrike->IsWithinExclusionDistance() && m_pobAttackDefinition->m_eStrikeProximityCheckExclusionDistanceReaction == RT_COUNT)
		{
			return false;
		}

		// Make sure I'm not a ragdoll for incoming ground attacks
		if (pobStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_DOWN &&
			(m_eCombatState == CS_FLOORED || m_eCombatState == CS_RISE_WAIT) )
		{
			m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
				Physics::System::CHARACTER_CONTROLLER,
				Physics::System::CHARACTER_CONTROLLER_COLLIDE_WITH_EVERYTHING );

			return true;
		}

		// Check the global auto block settings
		if ( !m_bGlobablAutoBlockEnable )
			return false;

		// If we can't autoblock the incoming stance of attack, do not respond here
		if ( !MayAutoBlockAttack(pobStrike->GetAttackDataP()->m_eAttackClass) )
		{
			return false;
		}

		// If I'm currently doing a sync attack, reject this strike
		// If I'm currently being abused with a syncronised attack from someone, ignore the strike if it's from someone else
		if ( ( m_pobMyCurrentStrike && m_pobMyCurrentStrike->ShouldSync() && (pobStrike->GetOriginatorP() != m_pobMyCurrentStrike->GetTargetP() || pobStrike->IsIncidental()) ) ||
			( m_pobStruckStrike && m_pobStruckStrike->ShouldSync() && (pobStrike->GetOriginatorP() != m_pobStruckStrike->GetOriginatorP() || pobStrike->IsIncidental()) ) )
		{
			return false;
		}

		// If we are already blocking then that is where we stay
		if ( m_eCombatState == CS_BLOCKING )
		{
			// Reset the state time
			m_fStateTime = 0.0f;

			// Update the current prestrike we are reacting to
			if ( m_pobStruckStrike )
			{
				NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
				m_pobStruckStrike = pobStrike;
			}

			// We have effectively restarted so return true
			return true;
		}

		// If we are in the standard state, the recovery state, or the popout 
		// section of the attack state then we can simply start a new block
		if ( ( m_eCombatState == CS_STANDARD )
			||
			( ( m_eCombatState == CS_RECOVERING ) && ( ( m_eRecoveryType != RC_RISING ) ) ) // != RC_DEFLECT taken out to allow deflecting from a previous attack to deflect again
			||
			( ( m_eCombatState == CS_ATTACKING ) && ( IsInBlockWindow() > 0 ) ) 
			||
			( m_eCombatState == CS_DEFLECTING ) ) // Added because prestrikes were not being processed mid attack string if you were deflecting the entire string
		{
			// Update the current prestrike we are reacting to
			if ( m_pobStruckStrike )
			{
				NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
				m_pobStruckStrike = pobStrike;
			}

			// Block based on stance
			if ( m_eCurrentStance == ST_POWER )
				StartBlock( pobStrike, BT_POWER );
			else if ( m_eCurrentStance == ST_RANGE )
				StartBlock( pobStrike, BT_RANGE );
			else 
				StartBlock( pobStrike, BT_SPEED );
			return true;
		}

		// If we are here we have failed to block
		return false;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartRecoil
*
*	DESCRIPTION		Start a recoil
*
***************************************************************************************************/
void CAttackComponent::StartRecoil( const CStrike* pobStrike, CHashedString obChosenReceiverAnim )
{
	//CGatso::Start("CAttackComponent::StartRecoil");

	// If we're already recoiuling update how deep we get in repeated recoils
	if (m_eCombatState == CS_RECOILING)
	{
		m_iDepthInRecoiling++;
	}

	// Save a pointer to the strike
	m_pobStruckStrike = pobStrike;

	// If we have a hit counter - kill the current string
	if ( GetHitCounter() )
		GetHitCounter()->PlayerStruck();

	// Log being recoiled
	m_pobCombatEventLogManager->AddEvent(CE_GOT_RECOILED, m_pobStruckStrike->GetOriginatorP(), (void*)m_pobStruckStrike->GetAttackDataP());
	if (m_pobStruckStrike->GetOriginatorP())
		m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_RECOIL, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP());
	else if ( m_pobStruckStrike->GetProjectile() && m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter() )
		m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_RECOIL, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );

	// Inform the other guy what has happened
	if ( pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent() )
		pobStrike->GetOriginatorP()->GetAttackComponent()->PlayerCausedRecoil( pobStrike );

	// Update our health
	m_pobParentEntity->ChangeHealth( -GetDamageFromStrike(pobStrike), "Was struck" );

	// If this has killed the character then they actually need to start the dying reaction
	if ( m_pobParentEntity->IsDead() )
	{
		m_iDepthInRecoiling = 0;
		return StartKO( pobStrike, obChosenReceiverAnim );
	}

	// Save a pointer to the strike
	m_pobStruckStrike = pobStrike;

	// Update our time based on the timings of the attacker
	if ( pobStrike->GetOriginatorP() )
		m_pobParentEntity->SetTimeMultiplier( pobStrike->GetOriginatorP()->GetTimeMultiplier() );

	// Set up the time during which we will be incapacitated - from the attack
	m_fIncapacityTime = m_pobStruckStrike->GetAttackDataP()->m_fRecoilTime;

	// Do we have a specific response to this attack?
//	CHashedString obSpecificRecoil;// = GetSpecificResponse( pobStrike->GetAttackDataP()->m_obReceiverAnim );
	CHashedString obSpecificRecoil = obChosenReceiverAnim;

	if (pobStrike->IsWithinExclusionDistance())
	{
		obSpecificRecoil = m_pobAttackDefinition->m_obStrikeProximityCheckExclusionDistanceReactionAnim;
	}

	// If the incoming attack is not syncronised - in which case this movement will be sorted
	if ( pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() || !pobStrike->ShouldSync())
	{
		// Otherwise - if we do have a specific recoil animation
		if ( !obSpecificRecoil.IsNull() )
		{
			// Kick off the specific response
			// If this strike came from a projectile then we don't want to do ANY yaw-rotation during the recoil movement (no snap-to-face-target etc).
			if(pobStrike->GetProjectile())
			{
				m_obAttackMovement.StartRecoilMovement( obSpecificRecoil, RZ_FRONT, pobStrike->GetAttackTimeScalar(), false);
			}
			else
			{
				//TODO: Rather than assuming front, shouldn't we pass m_eStruckReactionZone ???
				m_obAttackMovement.StartRecoilMovement( obSpecificRecoil, RZ_FRONT, pobStrike->GetAttackTimeScalar() );
			}
		}

		// Otherwise we just look to the recoil pool
		else
		{
			// Select from the pool for the front
			if ( m_eStruckReactionZone == RZ_FRONT )
			{
				//Another king-specific hack to allow for in-air recoil (forces to power recoil where we store the in-air ones or speed for ground).
				if(this->m_pobParentEntity && this->m_pobParentEntity->IsBoss() && (((Boss*)m_pobParentEntity)->GetBossType() == Boss::BT_KING_BOHAN))
				{
					KingBohan* pobKing = (KingBohan*)m_pobParentEntity;
					if(pobKing->HasAttachedDemon())
					{
						ntPrintf("--> King in-air front recoil\n");
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobPowerRecoilsFront->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
					}
					else
					{
						ntPrintf("--> King on-ground front recoil\n");
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobSpeedRecoilsFront->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
					}
				}
				else
				{
					// Need to get the stance right
					switch( m_eCurrentStance )
					{
					case ST_POWER:
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobPowerRecoilsFront->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
						break;

					case ST_RANGE:
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobRangeRecoilsFront->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
						break;

					case ST_SPEED:
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobSpeedRecoilsFront->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
						break;

					default:
						ntAssert( 0 );
						break;
					}
				}
			}

			// Select from the pool for the back
			else if ( m_eStruckReactionZone == RZ_BACK )
			{
				//Another king-specific hack to allow for in-air recoil (forces to power recoil where we store the in-air ones or speed for ground).
				if(this->m_pobParentEntity && this->m_pobParentEntity->IsBoss() && (((Boss*)m_pobParentEntity)->GetBossType() == Boss::BT_KING_BOHAN))
				{
					KingBohan* pobKing = (KingBohan*)m_pobParentEntity;
					if(pobKing->HasAttachedDemon())
					{
						ntPrintf("--> King in-air back recoil\n");
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobPowerRecoilsBack->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
					}
					else
					{
						ntPrintf("--> King on-ground back recoil\n");
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobSpeedRecoilsBack->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
					}
				}
				else
				{
					// Need to get the stance right
					switch( m_eCurrentStance )
					{
					case ST_POWER:
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobPowerRecoilsBack->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
						break;

					case ST_RANGE:
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobRangeRecoilsBack->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
						break;

					case ST_SPEED:
						m_obAttackMovement.StartRecoilMovement(	m_pobAttackDefinition->m_pobSpeedRecoilsBack->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], m_eStruckReactionZone, pobStrike->GetAttackTimeScalar() );
						break;

					default:
						ntAssert( 0 );
						break;
					}
				}
			}

			// The range of reaction directions has been extended?
			else
				ntAssert( 0 );
		}
	}

	// Otherwise if we know that there is a syncronised animamation started...
	else
	{
		ntAssert_p( 0, ( "Recoils are not valid for syncronised movement because there is no recover period \n" ) );
	}

	// Set to recoil state
	SetState( CS_RECOILING );

	// Give the movement system a message to pass to the state if this movement is completed fully - except if held
	if ( pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_GOTO && pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD && pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE )
		m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );

	//////////////////////////////////////////////////////////////////////////

	// Show that a Recoil has occurred
	DoHitEffect( pobStrike->GetOriginatorP() );

	// Trigger sound effects
	DoHitSound( pobStrike );

	//CGatso::Stop("CAttackComponent::StartRecoil");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateRecoil
*
*	DESCRIPTION		Updates whilst in the recoil state
*
***************************************************************************************************/
void CAttackComponent::UpdateRecoil( float )
{
	//CGatso::Start("CAttackComponent::UpdateRecoil");

	// Wait till we have reached the end of the reaction time then drop out
	if ( m_fStateTime > m_fIncapacityTime )
		EndRecoil();

	//CGatso::Stop("CAttackComponent::UpdateRecoil");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndRecoil
*
*	DESCRIPTION		End a recoil
*
***************************************************************************************************/
void CAttackComponent::EndRecoil()
{
	m_iDepthInRecoiling = 0;

	// Set to recoil state
	SetState( CS_RECOVERING );
	m_fIncapacityTime = 0.0f;
	m_eRecoveryType = RC_STANDARD;

	// This allows us to react to characters doing a special correctly and the smoothly return to speed
	m_pobParentEntity->SetTimeMultiplier( 1.0f );

	// Clear the strike we reacted to
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndSyncdReaction
*
*	DESCRIPTION		Finish up a reaction that has used a syncronised aniamtion
*
***************************************************************************************************/
void CAttackComponent::EndSyncdReaction( void )
{
	// If we are in a block stagger then we need to end it...
	if ( m_eCombatState == CS_BLOCK_STAGGERING )
	{
		EndBlockStagger();
	}

	// ...if this was an impact stagger then end that...
	else if ( m_eCombatState == CS_IMPACT_STAGGERING )
	{
		EndImpactStagger();
	}

	// ...otherwise we shouldn't really be here at all - warn someone
	else
	{
		ntAssert_p( 0, ( "'CAttackComponent::EndSyncdReaction' called in the wrong combat state." ) );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartHeld
*
*	DESCRIPTION		Start the held state
*
***************************************************************************************************/
void CAttackComponent::StartHeld( const CStrike* pobStrike, CHashedString obChosenReceiverAnim )
{
	//CGatso::Start("CAttackComponent::StartHeld");

	// Reuse the the general functionality here
	StartKO( pobStrike, obChosenReceiverAnim, true );

	// Save the countering time so we can decide whether to counter
	m_fCounteringTime = m_pobAttackDefinition->m_fCounterTime;
	m_fQuickCounteringTime = m_pobAttackDefinition->m_fQuickCounterTime;
	
	if (m_eCombatState == CS_DYING) 
	// If we were killed we'll be in DYING here, set up the ragdoll states we usually use for HELD because they woud have got set for DYING
	{
		m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
			Physics::System::CHARACTER_CONTROLLER,
			Physics::System::CHARACTER_CONTROLLER_DO_NOT_COLLIDE );
	}

	//CGatso::Stop("CAttackComponent::StartHeld");
	//ntPrintf("%s: StartHeld\n", ObjectDatabase::Get().GetNameFromPointer(this->m_pobParentEntity));
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::NotifyHeldVictimRagdolled
*
*	DESCRIPTION		Called from the RagdollContinuationController, stop our attack as our victim 
*					has flopped out.
*
***************************************************************************************************/
void CAttackComponent::NotifyHeldVictimRagdolled()
{
	// Just make sure we're not doing this at time we shouldn't be
	if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetTargetP() && m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState == CS_HELD)
	{
		// Victim will take care of themselves, all I need to do is end my attack
		EndAttack();
	}
	else if (IsInSuperStyleSafetyTransition())
	{
		ntAssert(m_pobSuperStyleSafetyReceiver);
		EndAttack();
	}

	// In all other cases, I've finished my attack and the victim will either be safely in some ragdoll transition controller, or in a normally animated FLOORED state
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateHeld
*
*	DESCRIPTION		Update the held state - someone is grabbing us
*
***************************************************************************************************/
void CAttackComponent::UpdateHeld( float )
{
	//CGatso::Start("CAttackComponent::UpdateHeld");

	//ntPrintf("%s: UpdateHeld\n", ObjectDatabase::Get().GetNameFromPointer(this->m_pobParentEntity));

	// At certain points in being attacked, we can be set to be ragdollable to allow physically safe execution of the grab/super/counter
	// Check our attacker to see what ragdoll state we should be in
	// If we don't have an attacker any more, then it's automatically ok to be ragdolled
	if (m_pobStruckStrike && m_pobStruckStrike->GetOriginatorP() && m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->m_eCombatState == CS_ATTACKING)
	{
		if (!(m_pobAttackDefinition->m_bExemptFromRagdollKOs || m_bEntityExemptFromRagdollKOs))
		{
			if (m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->IsInVictimRagdollableWindow())
			{
				// I need to be ready to ragdoll
				// Make sure we're not resetting the ragdoll
				Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if (pobAdvCC && pobAdvCC->GetRagdollState() != Physics::ANIMATED && pobAdvCC->GetRagdollState() != Physics::DEAD)
				{
					m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
						Physics::System::RAGDOLL_ANIMATED_PHYSICALLY,
						Physics::System::RAGDOLL_DEAD_ON_CONTACT );
				}
			}
			else
			{
				// Like normal in HELD, I ignore everything and don't do any ragdollery
				Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if (pobAdvCC && !pobAdvCC->IsCharacterControllerActive())
				{			
					m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
						Physics::System::CHARACTER_CONTROLLER,
						Physics::System::CHARACTER_CONTROLLER_DO_NOT_COLLIDE );
				}
			}
		}
	}
	else if (!m_bIsDoingSuperStyleSafetyTransition)
	{
		if (!(m_pobAttackDefinition->m_bExemptFromRagdollKOs || m_bEntityExemptFromRagdollKOs || m_bCannotUseRagdoll))
		{
			// I need to be ready to ragdoll
			// Make sure we're not resetting the ragdoll
			Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if (pobAdvCC && pobAdvCC->GetRagdollState() != Physics::ANIMATED && pobAdvCC->GetRagdollState() != Physics::DEAD)
			{
				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
					Physics::System::RAGDOLL_ANIMATED_PHYSICALLY,
					Physics::System::RAGDOLL_DEAD_ON_CONTACT );
			}
		}
	}


	// Use this state for SuperStyleSafetyTransition-ing, but that's a special case where we don't have a struck strike yet, so can't counter
	if (!m_bIsDoingSuperStyleSafetyTransition && m_pobStruckStrike)
	{
		// If we are within the countering state (HOLD) let the character select a counter attack
		// If they're not already being punished for a previous bad counter
		if (!m_bBadCounterBeingPunished && m_pobStruckStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD)
		{
			// See if we can counter here
			if ( !m_pobParentEntity->IsAI() && m_obAttackTracker.GetRequestedAttackType(m_pobParentEntity->GetInputComponent(), m_eCurrentStance) != AM_NONE )
			{
				CPoint obCheckPosition = m_pobParentEntity->GetPosition();
				obCheckPosition.Y() += 0.5f;
				if ( m_fStateTime <= m_fQuickCounteringTime ) // Check for a kill counter if we're quick enough
				{
					// Check if we're too close to environment
					if (Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition,m_pobAttackDefinition->m_obCounterAttackEnvironmentCheckHalfExtents,0.0f) || Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition,m_pobAttackDefinition->m_obCounterAttackEnvironmentCheckHalfExtents,45.0f))
						m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificSmallKillCounterIndex, m_bAutoCounter);
					else
						m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificKillCounterIndex, m_bAutoCounter);
				}
				else // Otherwise select a normal one
				{
					// Check if we're too close to environment
					if (Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition,m_pobAttackDefinition->m_obCounterAttackEnvironmentCheckHalfExtents,0.0f) || Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition,m_pobAttackDefinition->m_obCounterAttackEnvironmentCheckHalfExtents,45.0f))
						m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificSmallCounterIndex, m_bAutoCounter);
					else
						m_obAttackTracker.SelectSpecificCounterAttack( m_pobParentEntity, m_eCurrentStance, m_pobStruckStrike->GetAttackDataP()->m_pobSpecificCounterIndex, m_bAutoCounter);
				}

				// If we have a input pad save the secondar pad direction at this point - only for evades
				if ( ( m_pobParentEntity->GetInputComponent() ) && ( m_obAttackTracker.RequestIsReady() ) && ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
					m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();
			}

			// We let the player request attacks here
			if ( m_obAttackTracker.RequestIsReady() )
			{
				// Check that the requested attack is a valid counter for the attack we have just received
				if ( AttackIsValidCounter(	m_pobStruckStrike->GetAttackDataP()->m_eAttackClass,
											m_obAttackTracker.GetRequestedAttackClass() ) )
				{
					// If we successfully generated a strike...
					if ( GenerateMyStrike( true, false ) && StartAttack() ) 
					{
						// Let the Lua state know that we have countered
						CMessageSender::SendEmptyMessage( "msg_combat_countered", m_pobParentEntity->GetMessageHandler() );

						// Drop out here
						return;
					}
				}

				// Otherwise reset our invalid request
				else
					m_obAttackTracker.ClearRequested();
			}
		}
	}

	//CGatso::Stop("CAttackComponent::UpdateHeld");
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::AI_Command_SetStaggerTime							JML ADDED FOR AI
//! Set the time that we want to be staggered (or floored for)
//!
//------------------------------------------------------------------------------------------
bool CAttackComponent::AI_Command_SetStaggerTime( float fTime )
{
	// Make sure our input is sensible
	ntAssert( fTime >= 0.0f);

	// If we are not currently staggering then this will fail
	if ( ( m_eCombatState != CS_BLOCK_STAGGERING ) || ( m_eCombatState != CS_IMPACT_STAGGERING ) )
		return false;

	// If the requested time is lower than our current state time we fail
	if ( fTime < m_fStateTime )
		m_fIncapacityTime = m_fStateTime + EPSILON;

	// Otherwise set our incapacity time to the requested time
	else
		m_fIncapacityTime = fTime;

	// Show success
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Command_RequestBlock
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackComponent::AI_Command_RequestBlock( BLOCK_TYPE eBlockType, float fBlockTime )
{
	// We can only react to this if we are currently doing nothing
	if ( CanBlock( eBlockType ) )
	{
		// Go into our block state
		StartBlock( m_pobStruckStrike, eBlockType );

		// Confirm that the action was completed
		return true;
	}

	//ntPrintf("Request block failed\n");

	// We couldn't deal with the request
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Command_LeaveBlock
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackComponent::AI_Command_LeaveBlock( void ) 
{
	// We can only react to this is we are currently defending
	if ( m_eCombatState == CS_BLOCKING )
	{
		// Do whats required to leave the state
		EndBlock();

		// Confirm that the action was completed
		return true;
	}

	// The request was inappropriate
	return false;
}



/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Command_DoInstantKORecovery
*
*	DESCRIPTION		Returns true if this was successful
*
***************************************************************************************************/
bool CAttackComponent::AI_Command_DoInstantKORecovery( void ) 
{ 
	// We have to be in a KO state
	if ( m_eCombatState != CS_KO )
		return false;

	// We have to be in the instant KO recovery window
	if ( m_fStateTime >= m_pobAttackDefinition->m_fKOInstantRecoverTime )
		return false;

	// Otherwise set the flag
	m_bQuickRecover = true; 

	// Show success
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::SetDisabled
*
*	DESCRIPTION		Set the external disabled flag - cleans up if necessary
*
***************************************************************************************************/
void CAttackComponent::SetDisabled( bool bDisabled ) 
{ 
	// Store the state flag
	m_bExternallyDisabled = bDisabled; 

	// I'm not going to bother to check for state here - just blat over everything
	// try and clean up any attack stuff
	if (m_pobMyCurrentStrike)
		ntPrintf("%s: resetting attack %s from SetDisabled.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString());
	ResetAttack(true);
	EndString();

	// ...clean up any reaction stuff
	if ( m_pobStruckStrike )
	{
		NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
		m_pobStruckStrike = 0;
	}

	if( !bDisabled )
	{
		// Before switching state, make sure the current state is cleared up
		switch( m_eCombatState )
		{
			case CS_RECOVERING: EndRecovery();	break;
			default: break;
		}
	}

	// Write over the current state
	SetState( CS_STANDARD );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Command_RequestAttack
*
*	DESCRIPTION		This really needs looking at - JML
*
*					It's true.  The AI - combat interface has become really messy, it needs to be 
*					sorted out.  I'll have to have a good look at how the combat system can be best
*					structured to cater for both the AI and the player code.  I think that there is 
*					probably too many assumptions that the combat component is being used by a human
*					player.  Code that deals with these assumptions should be moved outside the 
*					component - GH.
*
***************************************************************************************************/
bool CAttackComponent::AI_Command_RequestAttack( ATTACK_MOVE_TYPE eMoveType, bool bForceToLockon, const CAttackComponent* pAttackTarget, const CDirection& obEvadeDirection )
{
	//ntPrintf( "CAttackComponent::AI_Command_RequestAttack %s, (%d) %f\n", m_pobParentEntity->GetName().c_str(), eMoveType, CTimer::Get().GetGameTime() );

	if( AM_NONE == eMoveType )
	{
		m_obAttackTracker.Reset();
		return true;
	}

	// Set up the evade direction - will not necessarily be used - for example if the requested attack is not an evade
	m_obEvadeDirection = obEvadeDirection; // * m_pobParentEntity->GetMatrix();

	// By request of the combat master - I hereby remove the following lines of code. 
/* // In this case we can't do anything
	if ( m_pobStruckStrike )
	{
		return false;
	} */


	// If the character is currently attacking already then we request a further attack in the string
	if ( m_pobMyCurrentStrike )
	{
		// Make sure the user hasn't tried to do a to-lockon here
		ntAssert_p( !bForceToLockon, ( "The AI has made a request which does not make sense" ) );
		if( m_obAttackTracker.AISelectNextAttack( eMoveType ) )
		{
			if( pAttackTarget && m_obAttackTracker.GetCurrentAttackDataP() )
			{
				pAttackTarget->AddAIAttacker( m_pobParentEntity, m_obAttackTracker.GetCurrentAttackDataP()->GetAttackTime( 1.0f ) );
			}
			return true;
		}
		return false;
	}

	// If we need to force a short attack that matches the timing fo the longer attacks...
	if ( bForceToLockon )
	{
		//float fLockOnDist = (m_pobParentEntity->GetPosition() - CEntityManager::Get().GetPlayer()->GetPosition()).Length();
		//if ( m_obAttackTracker.AISelectLockAttack( eMoveType, fLockOnDist ) )
		if ( m_obAttackTracker.AISelectShortLockAttack( eMoveType ) )
		{
			StartNewAttack();
			if( pAttackTarget && m_obAttackTracker.GetCurrentAttackDataP() )
			{
				pAttackTarget->AddAIAttacker( m_pobParentEntity, m_obAttackTracker.GetCurrentAttackDataP()->GetAttackTime( 1.0f ) );
			}
			return true;
		}

		// Otherwise we have failed
		return false;
	}

	// Otherwise if we are in a neutral mode we can kick off a new string
	if ( m_obAttackTracker.AISelectStartAttack( eMoveType ) && 
		// Extra protection, even through we have more informed AI attack selection, just make sure they're not breaking basic rules
		m_eCombatState != CS_HELD && m_eCombatState != CS_DEAD && m_eCombatState != CS_DYING && m_eCombatState != CS_KO && m_eCombatState != CS_FLOORED && m_eCombatState != CS_RECOILING )
	{
		StartNewAttack();
		if( pAttackTarget && m_obAttackTracker.GetCurrentAttackDataP() )
		{
			pAttackTarget->AddAIAttacker( m_pobParentEntity, m_obAttackTracker.GetCurrentAttackDataP()->GetAttackTime( 1.0f ) );
		}

		return true;
	}

	// If we are here then we haven't managed to do anything - indicate that fact
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Command_RequestDirectAttack
*
*	DESCRIPTION		Method used by AI characters that require direct access to attack processing
*
*	PARAM			pcName points to a data object (CAttackLink) - 
*	TODO:			GUID could also be used.
*
***************************************************************************************************/
bool CAttackComponent::AI_Command_RequestDirectAttack( CHashedString pcName )
{
	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromName( pcName );

	// Does the data object exist?
	if( !pDataObject )
		return false;

	// Is there a pointer to a non-attack lick type of object.
	if( !strstr( "CAttackLink", pDataObject->GetClassName() ) )
		return false;

	// Looks like the name is valid - select the attack
	if( m_obAttackTracker.AISelectAttack( (CAttackLink*) pDataObject->GetBasePtr() ) )
	{
		// Fire the attack off
		if( !m_pobMyCurrentStrike )
			return StartNewAttack();

		return true;
	}

	return false;
}

bool CAttackComponent::Boss_Command_RequestDirectAttack( const CAttackLink* pobLink, bool bForce )
{
	ntError( m_pobParentEntity->IsBoss() );
	ntError( pobLink );

	if( m_obAttackTracker.AISelectAttack( pobLink ) )
	{
		if (bForce)
			return StartNewAttack();
		else
			return true;
	}

	return false;
}

bool CAttackComponent::Boss_Command_RequestDirectNextAttack( const CAttackLink* pobLink )
{
	ntError( m_pobParentEntity->IsBoss() );
	ntError( pobLink );

	if( m_obAttackTracker.AISelectNextAttack( pobLink ) )
	{
		return true;
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CharacterUninterruptibleVulnerableTo
*
*	DESCRIPTION		Can our uninterruptible be interrupted by this?
*
***************************************************************************************************/
bool CAttackComponent::CharacterUninterruptibleVulnerableTo( ATTACK_CLASS eAttackClass ) const 
{
	if (m_eCombatState == CS_ATTACKING && m_pobMyCurrentStrike && IsInUninterruptibleWindow() && m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails)
	{
		// Mediums
		if ( eAttackClass == AC_SPEED_MEDIUM 
			&& !m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails->m_bUninterruptibleForSpeedAttackMedium )
			return true; // Actually, I can be hit by this type of attack
		if ( eAttackClass == AC_RANGE_MEDIUM
			&& !m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails->m_bUninterruptibleForRangeAttackMedium )
			return true; // Actually, I can be hit by this type of attack
		if ( eAttackClass == AC_POWER_MEDIUM
			&& !m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails->m_bUninterruptibleForPowerAttackMedium )
			return true; // Actually, I can be hit by this type of attack
		
		// Fasts
		if ( eAttackClass == AC_SPEED_FAST 
			&& !m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails->m_bUninterruptibleForSpeedAttackFast )
			return true; // Actually, I can be hit by this type of attack
		if ( eAttackClass == AC_RANGE_FAST 
			&& !m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails->m_bUninterruptibleForRangeAttackFast )
			return true; // Actually, I can be hit by this type of attack
		if ( eAttackClass == AC_POWER_FAST 
			&& !m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails->m_bUninterruptibleForPowerAttackFast )
			return true; // Actually, I can be hit by this type of attack

		// Grab
		if ( (eAttackClass == AC_GRAB_GOTO || eAttackClass == AC_GRAB_HOLD || eAttackClass == AC_GRAB_STRIKE)
			&& !m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails->m_bUninterruptibleForGrab )
			return true; // Actually, I can be hit by this type of attack
		
		return false; // We're not vulnerable
	} else if (m_eCombatState == CS_ATTACKING && m_pobMyCurrentStrike && IsInUninterruptibleWindow() && !m_pobMyCurrentStrike->GetAttackDataP()->m_pobUninterruptibleWindowDetails) {
		// No attack class specific details about it, we're just uninterruptible
		return false;
	} else {
		// If we're not in our uninterruptible window or we're not attacking, then by default we're gonna get abused
		return true;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartBlockStagger
*
*	DESCRIPTION		Starts a block-stagger believe it or not
*
***************************************************************************************************/
void CAttackComponent::StartBlockStagger( const CStrike* pobStrike, CHashedString obChosenReceiverAnim )
{
	//CGatso::Start("CAttackComponent::StartBlockStagger");

	// Save a pointer to the strike
	m_pobStruckStrike = pobStrike;

	// Log my stagger
	m_pobCombatEventLogManager->AddEvent(CE_GOT_BLOCK_STAGGERED, m_pobStruckStrike->GetOriginatorP(), (void*)m_pobStruckStrike->GetAttackDataP());
	if (m_pobStruckStrike->GetOriginatorP())
		m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_BLOCK_STAGGER, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP());
	else if ( m_pobStruckStrike->GetProjectile() && m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter() )
		m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_RECOIL, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );

	// Update our time based on the timings of the attacker
	if ( pobStrike->GetOriginatorP() )
		m_pobParentEntity->SetTimeMultiplier( pobStrike->GetOriginatorP()->GetTimeMultiplier() );

	// Set up the reaction time - for how long are we actually incapacitated
	m_fIncapacityTime = m_pobAttackDefinition->m_fBlockStaggerTime;

	// Make sure we zero the wiggle get back time
	m_fWiggleGetBack = 0.0f;

	// Do we have a specific response to this attack?
//	CHashedString obSpecificStagger;// = GetSpecificResponse( pobStrike->GetAttackDataP()->m_obReceiverAnim );
	CHashedString obSpecificStagger = obChosenReceiverAnim;

	if (pobStrike->IsWithinExclusionDistance())
	{
		obSpecificStagger = m_pobAttackDefinition->m_obStrikeProximityCheckExclusionDistanceReactionAnim;
	}

	// If the incoming attack is not syncronised - in which case this movement will be sorted
	if ( pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() || !pobStrike->ShouldSync() )
	{
		// If we do have a specific stagger for this attack
		if ( !obSpecificStagger.IsNull() )
		{
			// Kick off the specific response with normal movement
			m_obAttackMovement.StartStaggerMovement( obSpecificStagger, pobStrike->GetAttackTimeScalar() );
		}

		// Otherwise we just look to the stagger pool
		else
		{
			// Need to get the stance right
			switch( m_eCurrentStance )
			{
			case ST_POWER:
				if (m_pobAttackDefinition->m_pobPowerBlockStaggers)
				m_obAttackMovement.StartStaggerMovement( m_pobAttackDefinition->m_pobPowerBlockStaggers->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], pobStrike->GetAttackTimeScalar() );
				break;

			case ST_SPEED:
				if (m_pobAttackDefinition->m_pobSpeedBlockStaggers)
				m_obAttackMovement.StartStaggerMovement( m_pobAttackDefinition->m_pobSpeedBlockStaggers->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], pobStrike->GetAttackTimeScalar() );
				break;

			case ST_RANGE:
				if (m_pobAttackDefinition->m_pobRangeBlockStaggers)
				m_obAttackMovement.StartStaggerMovement( m_pobAttackDefinition->m_pobRangeBlockStaggers->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], pobStrike->GetAttackTimeScalar() );
				break;

			default:
				ntAssert( 0 );
				break;
			}
		}
	}

	// Otherwise if we know that there is a syncronised animamation started...
	else
	{
		// Give us some feedback when the movement is done
		m_obAttackMovement.SetMovementMessage( "msg_combatsyncdreactionend" );
	}

	// Set to a stagger state
	SetState( CS_BLOCK_STAGGERING );

	//////////////////////////////////////////////////////////////////////////

	// Show that a Stagger has occurred
	DoHitEffect( pobStrike->GetOriginatorP() );

	// Trigger sound effects
	DoHitSound( pobStrike );

	//CGatso::Stop("CAttackComponent::StartBlockStagger");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateBlockStagger
*
*	DESCRIPTION		Updates in block-stagger mode
*
***************************************************************************************************/
void CAttackComponent::UpdateBlockStagger( float )
{
	//CGatso::Start("CAttackComponent::UpdateBlockStagger");

	// If we are NOT 'staggering' in a syncronised animation
	if ( !( ( !m_pobStruckStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() )
		    &&
			( m_pobStruckStrike->ShouldSync() ) ) )
	{
		// See if we can reduce the effective time of this state
		CheckWiggleAction( m_fIncapacityTime, m_fWiggleGetBack );

		// Wait till we have reached the end of the reaction time then drop out
		// We have may have a minimum amount of time during which we can never escape
		if ( ( ( m_fStateTime + m_fWiggleGetBack ) > m_fIncapacityTime )
			&&
			( m_fStateTime > m_pobAttackDefinition->m_fMinBlockStaggerTime ) )
		{
			EndBlockStagger();
		}
	}

	//CGatso::Stop("CAttackComponent::UpdateBlockStagger");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndBlockStagger
*
*	DESCRIPTION		Finish a block-stagger
*
***************************************************************************************************/
void CAttackComponent::EndBlockStagger( void )
{
	// Set to recoil state
	SetState( CS_RECOVERING );
	m_fIncapacityTime = 0.0f;
	m_eRecoveryType = RC_STANDARD;

	// Kick off the stagger recovery movement
	if (m_eCurrentStance == ST_SPEED)
		m_obAttackMovement.StartStaggerRecoverMovement( m_pobAttackDefinition->m_obSpeedBlockStaggerRecoverAnim );
	if (m_eCurrentStance == ST_RANGE)
		m_obAttackMovement.StartStaggerRecoverMovement( m_pobAttackDefinition->m_obRangeBlockStaggerRecoverAnim );
	if (m_eCurrentStance == ST_POWER)
		m_obAttackMovement.StartStaggerRecoverMovement( m_pobAttackDefinition->m_obPowerBlockStaggerRecoverAnim );

	// Give the movement system a message to pass to the state if this movement is completed fully
	m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );

	// This allows us to react to characters doing a special correctly and the smoothly return to speed
	m_pobParentEntity->SetTimeMultiplier( 1.0f );

	// Clear the strike we reacted to
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartImpactStagger
*
*	DESCRIPTION		Starts an impact-stagger believe it or not
*
***************************************************************************************************/
void CAttackComponent::StartImpactStagger( const CStrike* pobStrike, CHashedString obChosenReceiverAnim )
{
	//CGatso::Start("CAttackComponent::StartImpactStagger");

	// Save a pointer to the strike
	m_pobStruckStrike = pobStrike;

	// If we have a hit counter - kill the current string
	if ( GetHitCounter() )
		GetHitCounter()->PlayerStruck();

	// Log my stagger
	m_pobCombatEventLogManager->AddEvent(CE_GOT_IMPACT_STAGGERED, m_pobStruckStrike->GetOriginatorP(), (void*)m_pobStruckStrike->GetAttackDataP() );
	if (m_pobStruckStrike->GetOriginatorP())
		m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_IMPACT_STAGGER, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );
	else if ( m_pobStruckStrike->GetProjectile() && m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter() )
		m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_IMPACT_STAGGER, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );

	// Inform the other guy what has happened
	if ( pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent() )
		pobStrike->GetOriginatorP()->GetAttackComponent()->PlayerCausedImpactStagger( pobStrike );

	// Update the health of the character...
	m_pobParentEntity->ChangeHealth( -GetDamageFromStrike(pobStrike), "Was struck");

	// If this has killed the character then they actually need to start the dying reaction
	if ( m_pobParentEntity->IsDead() )
	{
		//DGF need to fix this for final strike override
		/*if ( pobStrike->GetAttackDataP()->m_eUnBlockedReaction == RT_KO )
		{*/
			return StartKO( pobStrike, obChosenReceiverAnim );
		/*}
		else
		{
			return StartDying( pobStrike );		
		}*/
	}

	// Update our time based on the timings of the attacker
	if ( pobStrike->GetOriginatorP() )
		m_pobParentEntity->SetTimeMultiplier( pobStrike->GetOriginatorP()->GetTimeMultiplier() );

	// Set up the reaction time - for how long are we actually incapacitated
	m_fIncapacityTime = m_pobAttackDefinition->m_fImpactStaggerTime;

	// Make sure we zero the wiggle get back time
	m_fWiggleGetBack = 0.0f;

	// Do we have a specific response to this attack?
//	CHashedString obSpecificStagger;// = GetSpecificResponse( pobStrike->GetAttackDataP()->m_obReceiverAnim );
	CHashedString obSpecificStagger = obChosenReceiverAnim;

	if (pobStrike->IsWithinExclusionDistance())
	{
		obSpecificStagger = m_pobAttackDefinition->m_obStrikeProximityCheckExclusionDistanceReactionAnim;
	}

	// If the incoming attack is not syncronised - in which case this movement will be sorted
	if ( pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() || !pobStrike->ShouldSync() )
	{
		// If we do have a specific stagger for this attack
		if ( !obSpecificStagger.IsNull() )
		{
			// Kick off the specific response with normal movement
			m_obAttackMovement.StartStaggerMovement( obSpecificStagger, pobStrike->GetAttackTimeScalar() );
		}

		// Otherwise we just look to the stagger pool
		else
		{
			// Need to get the stance right
			switch( m_eCurrentStance )
			{
			case ST_POWER:
				if (m_pobAttackDefinition->m_pobPowerImpactStaggers)	
				m_obAttackMovement.StartStaggerMovement( m_pobAttackDefinition->m_pobPowerImpactStaggers->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], pobStrike->GetAttackTimeScalar() );
				break;

			case ST_SPEED:
				if (m_pobAttackDefinition->m_pobSpeedImpactStaggers)
				m_obAttackMovement.StartStaggerMovement( m_pobAttackDefinition->m_pobSpeedImpactStaggers->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], pobStrike->GetAttackTimeScalar() );
				break;

			case ST_RANGE:
				if (m_pobAttackDefinition->m_pobRangeImpactStaggers)
				m_obAttackMovement.StartStaggerMovement( m_pobAttackDefinition->m_pobRangeImpactStaggers->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance], pobStrike->GetAttackTimeScalar() );
				break;

			default:
				ntAssert( 0 );
				break;
			}
		}
	}

	// Otherwise if we know that there is a syncronised animamation started...
	else
	{
		// Give us some feedback when the movement is done
		m_obAttackMovement.SetMovementMessage( "msg_combatsyncdreactionend" );
	}

	// Set to a stagger state
	SetState( CS_IMPACT_STAGGERING );

	// Show that a Stagger has occurred
	DoHitEffect( pobStrike->GetOriginatorP() );

	// Trigger sound effects
	DoHitSound( pobStrike );

	//CGatso::Stop("CAttackComponent::StartImpactStagger");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateImpactStagger
*
*	DESCRIPTION		Updates an impact stagger
*
***************************************************************************************************/
void CAttackComponent::UpdateImpactStagger( float )
{
	//CGatso::Start("CAttackComponent::UpdateImpactStagger");

	// If we are NOT 'staggering' in a syncronised animation
	if ( !( ( !m_pobStruckStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() )
		    &&
			( m_pobStruckStrike->ShouldSync() ) ) )
	{
		// See if we can reduce the effective time of this state
		CheckWiggleAction( m_fIncapacityTime, m_fWiggleGetBack );

		// Wait till we have reached the end of the reaction time then drop out
		// We have may have a minimum amount of time during which we can never escape
		if ( ( ( m_fStateTime + m_fWiggleGetBack ) > m_fIncapacityTime )
			&&
			( m_fStateTime > m_pobAttackDefinition->m_fMinImpactStaggerTime ) )
		{
			EndImpactStagger();
		}
	}

	//CGatso::Stop("CAttackComponent::UpdateImpactStagger");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndImpactStagger
*
*	DESCRIPTION		Clear up an impact stagger
*
***************************************************************************************************/
void CAttackComponent::EndImpactStagger( void )
{
	// Set to recoil state
	SetState( CS_RECOVERING );
	m_fIncapacityTime = 0.0f;
	m_eRecoveryType = RC_STANDARD;

	// Kick off the stagger recovery movement
	// Kick off the stagger recovery movement
	if (m_eCurrentStance == ST_SPEED)
		m_obAttackMovement.StartStaggerRecoverMovement( m_pobAttackDefinition->m_obSpeedImpactStaggerRecoverAnim );
	if (m_eCurrentStance == ST_RANGE)
		m_obAttackMovement.StartStaggerRecoverMovement( m_pobAttackDefinition->m_obRangeImpactStaggerRecoverAnim );
	if (m_eCurrentStance == ST_POWER)
		m_obAttackMovement.StartStaggerRecoverMovement( m_pobAttackDefinition->m_obPowerImpactStaggerRecoverAnim );

	// Give the movement system a message to pass to the state if this movement is completed fully
	m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );

	// This allows us to react to characters doing a special correctly and the smoothly return to speed
	m_pobParentEntity->SetTimeMultiplier( 1.0f );

	// Clear the strike we reacted to
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::PlayerCausedKO
*
*	DESCRIPTION		To be called on a character if they have caused someone to be KO'd with a strike
*
***************************************************************************************************/
void CAttackComponent::PlayerCausedKO( const CStrike* pobStrike ) const
{
	// We only count each individual attack once and we do not count superstyles
	if ( GetHitCounter() && ( pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_GOTO || pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD || pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE ) )
	{
		GetHitCounter()->SuccessfulHit();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::PlayerCausedRecoil
*
*	DESCRIPTION		To be called on a character if they have caused someone to be recoiled with a 
*					strike
*
***************************************************************************************************/
void CAttackComponent::PlayerCausedRecoil( const CStrike* pobStrike ) const
{
	// We only count each individual attack once and we do not count superstyles
	if ( GetHitCounter() && ( pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_GOTO || pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD || pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE ) )
	{
		GetHitCounter()->SuccessfulHit();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::PlayerCausedImpactStagger
*
*	DESCRIPTION		To be called on a character if they have caused someone to react with an
*					'impact stagger'
*
***************************************************************************************************/
void CAttackComponent::PlayerCausedImpactStagger( const CStrike* pobStrike ) const
{
	// We only count each individual attack once and we do not count superstyles
	if ( GetHitCounter() && ( pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_GOTO || pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD || pobStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE ) )
	{
		GetHitCounter()->SuccessfulHit();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::PlayerCausedDeath
*
*	DESCRIPTION		To be called on a character if they have caused someone to die
*
***************************************************************************************************/
void CAttackComponent::PlayerCausedDeath( const CStrike* pobStrike ) const
{
	// Not sure what we want here _ should the player be rewarded for hitting someone who is dead?
	UNUSED( pobStrike );

}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::InterruptSyncAttack
*
*	DESCRIPTION		Gracefully handle getting dumped from a sync attack, start a KO
*
***************************************************************************************************/
void CAttackComponent::InterruptSyncAttack()
{
	if (!m_pobStruckStrike)
	{
		//ntPrintf("No struck strike when interrupting sync attack! %s\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity));
		return;
	}

	// If I am not the target of this strike, I'm being sync secondary struck, so I shouldn't do anything here
	if (m_pobStruckStrike->GetTargetP() != m_pobParentEntity || m_pobStruckStrike->IsIncidental() || m_eCombatState == CS_DEAD)
		return;

	// Make sure we're not calling this at a completely wrong time
	ntError(m_eCombatState != CS_STANDARD);

	// Set the flag that tells StartKO what not to do
	m_bSyncAttackInterrupted = true;

	// Start a KO (really this is just a ragdoll)
    StartKO(m_pobStruckStrike, NULL);

	// m_bSyncAttackInterrupted flag cleared in StartKO
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartKO
*
*	DESCRIPTION		Start a KO
*
***************************************************************************************************/
void CAttackComponent::StartKO( const CStrike* pobStrike, CHashedString obChosenReceiverAnim, bool bFromHeld )
{
	//CGatso::Start("CAttackComponent::StartKO - 1");

	// Reset this flag when we start each new KO
	m_bNeedLedgeRecover = false;
	m_obLastKOPosition = m_pobParentEntity->GetPosition();

	// Check if we can use a ragdoll, counters/supers must be able to use a ragdoll for collision safety reasons
	if (!Physics::RagdollPerformanceManager::Get().CanAnimated() && !pobStrike->ShouldSync())
		m_bCannotUseRagdoll = true;
	 else
		m_bCannotUseRagdoll = false;

	// Save a pointer to the strike
	m_pobStruckStrike = pobStrike;

	// Log my KO
	m_pobCombatEventLogManager->AddEvent(CE_GOT_KOED, m_pobStruckStrike->GetOriginatorP(), (void*)m_pobStruckStrike->GetAttackDataP() );
	if (m_pobStruckStrike->GetOriginatorP())
		m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_KO, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );
	else if ( m_pobStruckStrike->GetProjectile() && m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter() )
		m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_KO, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );

	// If we have a hit counter - kill the current string
	if ( GetHitCounter() )
		GetHitCounter()->PlayerStruck();

	// Inform the other guy what has happened
	if ( pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetAttackComponent() )
		pobStrike->GetOriginatorP()->GetAttackComponent()->PlayerCausedKO( pobStrike );

	// Clear the quick recover flag and update health flag
	m_bQuickRecover = false;

	// Else just do it the normal way
	m_pobParentEntity->ChangeHealth( -GetDamageFromStrike(pobStrike), "Was KO'ed" );

	// This should be centralised rather than branching at the beginning of all reaction functions
	// StartDying uses this for stuff, and we don't want to recurse
	if ( !m_pobParentEntity->IsBoss() && m_pobParentEntity->IsDead() && m_eCombatState != CS_DYING)
	{		
		if ((m_pobStruckStrike->ShouldSync() && !m_bSyncAttackInterrupted) || m_pobStruckStrike->GetAttackDataP()->m_eAttackMovementType == AMT_AIR_TO_AIR)
		{
			// Not going through SetState to avoid resetting any collision settings
			m_fStateTime = 0.0f;			
			m_eCombatState = CS_DYING;
		}
		else return StartKill( m_pobStruckStrike, obChosenReceiverAnim );
	}

	// Update our time based on the timings of the attacker
	if ( pobStrike->GetOriginatorP() )
		m_pobParentEntity->SetTimeMultiplier( m_pobStruckStrike->GetOriginatorP()->GetTimeMultiplier() );

	// Is it OK to use a specific animation?
	bool bUseSpecific = !( !pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() && !pobStrike->ShouldSync() );

	// Find the hit reaction 
	CHashedString obHitReaction = FindKOHitAnim( m_eStruckReactionZone, m_pobStruckStrike, m_eCombatState, bUseSpecific, obChosenReceiverAnim );

	// Send a message to the entities that a KO has started. 
	if( m_pobParentEntity->GetMessageHandler() && pobStrike->GetOriginatorP() )
	{
		m_pobParentEntity->GetMessageHandler()->Receive( CMessageHandler::Make( pobStrike->GetOriginatorP(), "msg_combat_being_ko" ) );
		pobStrike->GetOriginatorP()->GetMessageHandler()->Receive( CMessageHandler::Make( m_pobParentEntity, "msg_combat_started_ko" ) );
	}

	//CGatso::Stop("CAttackComponent::StartKO - 1");

	m_bUseProceduralKODeath = false;  
	// If the incoming attack is not syncronised - in which case this movement will be sorted
	// Don't do this if we're sync attack interupted or going to be aerialed (which means we've got to only start a falling movement)
	if ( (pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() || !pobStrike->ShouldSync()) && !m_bSyncAttackInterrupted && !m_bGoingToBeAerialed)
	{
		//CGatso::Start("CAttackComponent::StartKO - 2");

		if(!m_bCannotUseRagdoll && pobStrike->GetAttackDataP()->m_bUseProceduralKO && 
			!m_pobParentEntity->IsBoss() && !bFromHeld)
		{
			// clear movement controllers. porsedural KO will be started in SetRagdollState
			//m_pobParentEntity->GetMovement()->ClearControllers();		
			m_bUseProceduralKODeath = true; 
		}
		else
		{
			// Kick off the movement - front or for the specific animation case
			if ( ( m_eStruckReactionZone == RZ_FRONT ) || ( obHitReaction == m_pobStruckStrike->GetAttackDataP()->m_obReceiverAnim ) )
			{
				// Make sure the rest of the KO plays out correctly
				m_eStruckReactionZone = RZ_FRONT;

				bool bUseSpecificAngleToTarget = false;
				// If this is a ground attack, orient to our attacker cos we could be in a weird position
				if (pobStrike->GetOriginatorP() && pobStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_DOWN)
				{
					m_obKOTargetVector = CDirection(pobStrike->GetOriginatorP()->GetPosition()) - CDirection(this->m_pobParentEntity->GetPosition());
					m_obKOTargetVector[1] = 0.0f;
					m_obKOTargetVector.Normalise();
					CDirection obUp(0.0,1.0,0.0);
					m_fKOSpecificAngleToTarget = PI;//obUp.CalculateRotationAxis(pobStrike->GetOriginatorP()->GetMatrix().GetZAxis(),m_pobParentEntity->GetMatrix().GetZAxis());
					bUseSpecificAngleToTarget = true;
					m_pobParentEntity->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;
					m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint = this->m_pobParentEntity->GetPosition() + (pobStrike->GetOriginatorP()->GetMatrix().GetZAxis() * -5) ;
				}

				// If this is not aerial stuff and we have an attack we allow aftertouch
				if ( ( m_pobStruckStrike->GetAttackDataP()->m_eAttackMovementType != AMT_AIR_TO_AIR ) && ( pobStrike->GetOriginatorP() ) && !m_bSyncAttackInterrupted)
				{
					if (m_pobStruckStrike->GetAttackDataP()->m_bAftertouchableKO && !m_pobStruckStrike->IsIncidental())
					{
						// Messages for KO aftertouch
						if (pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetMessageHandler() && pobStrike->GetOriginatorP()->GetInputComponent())
						{
							const float fBUTTON_HELD_DURATION = 0.1f;

							float fAttackFastTime=pobStrike->GetOriginatorP()->GetInputComponent()->GetVHeldTime(AB_ATTACK_FAST);
							float fAttackMediumTime=pobStrike->GetOriginatorP()->GetInputComponent()->GetVHeldTime(AB_ATTACK_MEDIUM);

							float fHeldTime=( fAttackFastTime > fAttackMediumTime ? fAttackFastTime : fAttackMediumTime );
		
							if (fHeldTime>fBUTTON_HELD_DURATION)
							{
								// Send a message to the attacker (the player) that we are ready to start the ko aftertouch
								Message obAttackerMessage(msg_combat_enemy_ko);
								obAttackerMessage.SetEnt( "Sender", m_pobParentEntity ); // Sender is this entity (the one being KOed)

								pobStrike->GetOriginatorP()->GetMessageHandler()->QueueMessage( obAttackerMessage );
							}
						}

						m_obAttackMovement.StartKOAftertouchMovement(	obHitReaction,
																		m_pobAttackDefinition->m_pobFrontKODefinition,
																		m_eStruckReactionZone,
																		m_pobStruckStrike->GetAttackTimeScalar(),
																		false,
																		0.0f,
																		pobStrike->GetOriginatorP(),
																		!m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement );
					}
					else
					{
						m_obAttackMovement.StartKOMovement( obHitReaction,
														m_pobAttackDefinition->m_pobFrontKODefinition,
														m_eStruckReactionZone,
														m_pobStruckStrike->GetAttackTimeScalar(),
														( m_pobStruckStrike->GetAttackDataP()->m_eAttackMovementType == AMT_AIR_TO_AIR ),
														false,
														0.0f, 
														!m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement );
					}
				}
				else
				{
					// Start the movement
					m_obAttackMovement.StartKOMovement( obHitReaction,
														m_pobAttackDefinition->m_pobFrontKODefinition,
														m_eStruckReactionZone,
														m_pobStruckStrike->GetAttackTimeScalar(),
														( m_pobStruckStrike->GetAttackDataP()->m_eAttackMovementType == AMT_AIR_TO_AIR ),
														false,
														0.0f,
														!m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement );
				}
			}

			// ...and for the back
			else if ( m_eStruckReactionZone == RZ_BACK )
			{
				// If this is a ground attack, orient to our attacker cos we could be in a weird position
				if (pobStrike->GetOriginatorP() && pobStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_DOWN)
				{
					m_pobParentEntity->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;
					m_pobParentEntity->GetMovement()->m_obMovementInput.m_obTargetPoint = this->m_pobParentEntity->GetPosition() + (pobStrike->GetOriginatorP()->GetMatrix().GetZAxis() * 5) ;
					//char pcMsg[500] = "Ground KO Back: %f\n";
					//ntPrintf(pcMsg, m_fKOSpecificAngleToTarget*RAD_TO_DEG_VALUE);
				}

				// If this is not aerial stuff and we have an attack we allow aftertouch
				if ( ( m_pobStruckStrike->GetAttackDataP()->m_eAttackMovementType != AMT_AIR_TO_AIR ) && ( pobStrike->GetOriginatorP() ) && !m_bSyncAttackInterrupted)
				{
					if (m_pobStruckStrike->GetAttackDataP()->m_bAftertouchableKO && !m_pobStruckStrike->IsIncidental())
					{
						// Messages for KO aftertouch
						if (pobStrike->GetOriginatorP() && pobStrike->GetOriginatorP()->GetMessageHandler() && pobStrike->GetOriginatorP()->GetInputComponent())
						{
							if (pobStrike->GetOriginatorP()->GetInputComponent()->GetVHeld() &  ( ( 1 << AB_ATTACK_FAST ) | ( 1 << AB_ATTACK_MEDIUM )))
							{
								// Send a message to the attacker (the player) that we are ready to start the ko aftertouch
								Message obAttackerMessage(msg_combat_enemy_ko);
								obAttackerMessage.SetEnt( "Sender", m_pobParentEntity ); // Sender is this entity (the one being KOed)

								pobStrike->GetOriginatorP()->GetMessageHandler()->QueueMessage( obAttackerMessage );

								//ntPrintf("Sending msg_ko_aftertouch to %s\n",pobStrike->GetOriginatorP()->GetName().c_str());
							}
						}

						m_obAttackMovement.StartKOAftertouchMovement(	obHitReaction,
																		m_pobAttackDefinition->m_pobFrontKODefinition,
																		m_eStruckReactionZone,
																		m_pobStruckStrike->GetAttackTimeScalar(),
																		false,
																		0.0f,
																		( pobStrike->GetOriginatorP() ),
																		!m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement );
					}
					else
					{
						m_obAttackMovement.StartKOMovement( obHitReaction,
															m_pobAttackDefinition->m_pobBackKODefinition,
															m_eStruckReactionZone,
															m_pobStruckStrike->GetAttackTimeScalar(),
															( m_pobStruckStrike->GetAttackDataP()->m_eAttackMovementType == AMT_AIR_TO_AIR ),
															false,
															0.0f,
															!m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement);
					}
					
				}
				else
				{
					m_obAttackMovement.StartKOMovement( obHitReaction,
														m_pobAttackDefinition->m_pobBackKODefinition,
														m_eStruckReactionZone,
														m_pobStruckStrike->GetAttackTimeScalar(),
														( m_pobStruckStrike->GetAttackDataP()->m_eAttackMovementType == AMT_AIR_TO_AIR ),
														false,
														0.0f,
														!m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement );
				}
			}
			// Do we have more directions to be hit in now?
			else
				ntAssert( 0 );
		}
		
		// Set up the time during which we will be incapacitated - different for grab stuff
		m_fIncapacityTime = m_pobAttackDefinition->m_fKOTime + ((float)(grand()%21)-10)*0.1f;

		// Give the movement system a message to pass to the state if this movement is completed fully
		if (m_eCombatState != CS_DYING) // StartDying uses this for stuff so make sure we dont revive our dead person
		{
			if (!m_bUseProceduralKODeath) 
			m_obAttackMovement.SetMovementMessage( "msg_combat_floored" );
		}
		else if (m_pobStruckStrike->GetAttackDataP()->m_eTargetType != AT_TYPE_AERIAL_COMBO && !IsInSuperStyleSafetyTransition() && m_pobStruckStrike->GetAttackDataP()->m_eAttackClass != AC_GRAB_HOLD) // Only set this to be sent on interruption if it's not in an aerial (which we want to continue as long as possible)
			m_obAttackMovement.SetMovementCallback( Character::KillEnt, true );
		else if (m_pobStruckStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_AERIAL_COMBO )
		{
			if (m_bCannotUseRagdoll)
			{
				// Chain on an impact movement
				if ( m_eStruckReactionZone == RZ_FRONT )
					m_obAttackMovement.StartImpactMovement(m_pobAttackDefinition->m_pobFrontKODefinition->m_obFlooredAnimName, true);
				else if ( m_eStruckReactionZone == RZ_BACK )
					m_obAttackMovement.StartImpactMovement(m_pobAttackDefinition->m_pobBackKODefinition->m_obFlooredAnimName, true);
			}
			m_obAttackMovement.SetMovementCallback( Character::KillEnt, false );
		}

		//CGatso::Stop("CAttackComponent::StartKO - 2");
	}

	// If we are syncronised we need to add some final recovery movement - this is where interrupted syncs also end up, just adding a fall
	else
	{
		// All synchronised moves have to be able to ragdoll, this is cos it's the easiest way of nicely falling out of them
		// We could have an animated alternative, but the animators would need to make end pose to fall cycles for all synchronised moves
		// To be honest, I think we can afford adding another single ragdoll ragardless of performance manager every sync move, it won't hurt us that much - DGF
		m_bCannotUseRagdoll = false;

		//CGatso::Start("CAttackComponent::StartKO - 3");

		if (!m_bSyncAttackInterrupted)
		{
			// We come out of this as we would a normal KO
			// Only add a falling movement if we're not gonna die, OR we can't afford to use another ragdoll cos PS3 sux0r5
			if ( (m_eCombatState != CS_DYING ) && !pobStrike->GetAttackDataP()->m_bSkipFlooredAndRise )
			{
				if (!m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement && m_pobAttackDefinition->m_pobFrontKODefinition)
					m_obAttackMovement.StartKOFallMovement( m_pobAttackDefinition->m_pobFrontKODefinition, m_bSyncAttackInterrupted );
				m_fIncapacityTime = m_pobAttackDefinition->m_fKOTime + ((float)(grand()%21)-10)*0.1f;
			}
			// If we're DYING in a sync aerial, set a message to kill us
			else if (m_eCombatState == CS_DYING && m_pobStruckStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_AERIAL_COMBO)
			{
				m_obAttackMovement.SetMovementCallback( Character::KillEnt, true );
			}
		}
		else
		{
			if (!m_bCannotUseRagdoll)
			{
			// Activate my ragdoll so I gracefully physically exit the move
			m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
				Physics::System::RAGDOLL_DEAD,
				Physics::System::RAGDOLL_IGNORE_CONTACT );
		}
		}

		if ( m_eCombatState != CS_DYING && !pobStrike->GetAttackDataP()->m_bSkipFlooredAndRise ) // StartDying uses this for stuff so make sure we dont revive our dead person
			m_obAttackMovement.SetMovementMessage( "msg_combat_floored" );
		else if (m_eCombatState != CS_DYING && pobStrike->GetAttackDataP()->m_bSkipFlooredAndRise ) // If we're not chaining on a fall movement, start a recover as soon as sync anim is done
			m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );
		// If we're dying, kill us, but not if we're mid aerial combo (let the combos complete and we'll get killed at the end)
		else if (m_pobStruckStrike->GetAttackDataP()->m_eTargetType != AT_TYPE_AERIAL_COMBO )
			m_obAttackMovement.SetMovementCallback( Character::KillEnt, true );

		//CGatso::Stop("CAttackComponent::StartKO - 3");
	}

	//CGatso::Start("CAttackComponent::StartKO - 4");

	// Make sure that we always set the state when we're not dying
	if (m_eCombatState != CS_DYING)
	{
		if (!bFromHeld)
			SetState( CS_KO, pobStrike );
		else
			SetState( CS_HELD );
	}

	//CGatso::Stop("CAttackComponent::StartKO - 4");

	//CGatso::Start("CAttackComponent::StartKO - 6");
	// Show that a KO has occurred
	DoHitEffect( pobStrike->GetOriginatorP() );

	// Trigger sound effects
	DoHitSound( pobStrike );

	// Set this to false here ready for any notifies
	m_bGoingToBeAerialed = false;

	// Clear this flag as we've used it
	m_bSyncAttackInterrupted = false;

	//CGatso::Stop("CAttackComponent::StartKO - 6");
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateDying
*
*	DESCRIPTION		This is now not a proper state, CS_DYING is a kind of middle state between
*					something and being dead. At the moment it's used to keep the entity alive
*					during combos and then to kill them when they're over.
*
***************************************************************************************************/
void CAttackComponent::UpdateDying( float fTimeDelta )
{
	//CGatso::Start("CAttackComponent::UpdateDying");

	UNUSED( fTimeDelta );

	// At certain points in being syncronised attacked, we can be set to be ragdollable to allow physically safe execution of the grab/super/counter
	// Check our attacker to see what ragdoll state we should be in
	// If we don't have an attacker any more, then it's automatically ok to be ragdolled
	if (m_pobStruckStrike && m_pobStruckStrike->ShouldSync() && m_pobStruckStrike->GetOriginatorP() && m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->m_eCombatState == CS_ATTACKING)
	{
		if (!(m_pobAttackDefinition->m_bExemptFromRagdollKOs || m_bEntityExemptFromRagdollKOs))
		{
			if (m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->IsInVictimRagdollableWindow())
			{
				// Clear this flag because we use this victim ragdollable window for collision safety, so we rely on there being a ragdoll
				m_bCannotUseRagdoll = false;

				// I need to be ready to ragdoll
				// Make sure we're not resetting the ragdoll
				Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if (pobAdvCC && pobAdvCC->GetRagdollState() != Physics::ANIMATED && pobAdvCC->GetRagdollState() != Physics::DEAD)
				{
					m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
						Physics::System::RAGDOLL_ANIMATED_PHYSICALLY,
						Physics::System::RAGDOLL_DEAD_ON_CONTACT );
				}
			}
			else
			{
				// Like normal in HELD, I ignore everything and don't do any ragdollery
				Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if (pobAdvCC && !pobAdvCC->IsCharacterControllerActive())
				{			
					m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
						Physics::System::CHARACTER_CONTROLLER,
						Physics::System::CHARACTER_CONTROLLER_COLLIDE_WITH_STATIC_ONLY );
				}
			}
		}
	}
	else if (!m_bIsDoingSuperStyleSafetyTransition && m_pobStruckStrike->ShouldSync() )
	{
		if (!(m_pobAttackDefinition->m_bExemptFromRagdollKOs || m_bEntityExemptFromRagdollKOs))
		{
			// I need to be ready to ragdoll
			// Make sure we're not resetting the ragdoll
			Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
			if (pobAdvCC && pobAdvCC->GetRagdollState() != Physics::ANIMATED && pobAdvCC->GetRagdollState() != Physics::DEAD)
			{
				// Clear this flag because we use this for collision safety, so we rely on there being a ragdoll
				m_bCannotUseRagdoll = false;

				m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
					Physics::System::RAGDOLL_ANIMATED_PHYSICALLY,
					Physics::System::RAGDOLL_DEAD_ON_CONTACT );
			}
		}
	}

	// Hacktastic - DGF/JL
	if (m_fStateTime > 10.0f)
	{
		ntPrintf("%s was dying for too long, he/she/it's getting killed now.\n", m_pobParentEntity->GetName().c_str() );
		m_pobParentEntity->ToCharacter()->Kill();
	}

	//CGatso::Stop("CAttackComponent::UpdateDying");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::IsDoingSpecial
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackComponent::IsDoingSpecial( void ) const 
{ 
	return ( m_pobAttackSpecial && m_pobAttackSpecial->IsActive() );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartSpecial
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackComponent::StartSpecial( void )
{ 
	// Try and kick one off if we have the relevant hardware
	if ( m_pobAttackSpecial )
	{
		if (m_pobAttackSpecial->StartSpecial(m_eCurrentStance))
		{
			// Log it
			m_pobCombatEventLogManager->AddEvent(CE_STARTED_SPECIAL, 0);
			return true;
		}
		else return false;
	}
	else
		return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::FindKOHitAnim
*
*	DESCRIPTION		
*
***************************************************************************************************/
CHashedString CAttackComponent::FindKOHitAnim(	REACTION_ZONE	eReactionZone, 
											const CStrike*	pobStrike, 
											COMBAT_STATE	eCurrentState,
											bool			bUseSpecific,
											CHashedString	obChosenReceiverAnim)
{
	// We made need to use a specific KO reaction for the incoming attack
//	CHashedString obSpecificKO = GetSpecificResponse( pobStrike->GetAttackDataP()->m_obReceiverAnim );
	CHashedString obSpecificKO = (ntStr::IsNull(pobStrike->GetAttackDataP()->m_obSpecificKOAnimation) == false) ? 
		pobStrike->GetAttackDataP()->m_obSpecificKOAnimation : obChosenReceiverAnim;

	if (pobStrike->IsWithinExclusionDistance())
	{
		obSpecificKO = m_pobAttackDefinition->m_obStrikeProximityCheckExclusionDistanceReactionAnim;
	}

	if ( !obSpecificKO.IsNull() && bUseSpecific )
	{
		// We only have a front KO for this
		return obSpecificKO;
	}

	// Otherwise if we are already in KO we do the air KO
	else if ( ( eCurrentState == CS_KO ) )
	{
		// Do the air KO for the front
		if ( eReactionZone == RZ_FRONT )
			return m_pobAttackDefinition->m_obFrontAirKOAnim;

		// Do the air KO for the back
		else if ( eReactionZone == RZ_BACK )
			return m_pobAttackDefinition->m_obBackAirKOAnim;

		// The range of reaction directions has been extended?
		else
		{
			ntAssert( 0 );
			return CHashedString();
		}
	}

	// After all that we can just resort to the standard KO animations
	else
	{
		// Choose from the front
		if ( eReactionZone == RZ_FRONT )
		{
			// Need to get the stance right
			switch( m_eCurrentStance )
			{
			case ST_POWER:
				if (m_pobAttackDefinition->m_pobPowerKOsFront)
					return m_pobAttackDefinition->m_pobPowerKOsFront->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance];
				break;

			case ST_RANGE:
				if (m_pobAttackDefinition->m_pobRangeKOsFront)
					return m_pobAttackDefinition->m_pobRangeKOsFront->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance];
				break;

			case ST_SPEED:
				if (m_pobAttackDefinition->m_pobSpeedKOsFront)
					return m_pobAttackDefinition->m_pobSpeedKOsFront->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance];
				break;

			default:
				ntAssert( 0 );
				return CHashedString();
				break;
			}
		}

		// Choose from the back
		else if ( eReactionZone == RZ_BACK )
		{
			// Need to get the stance right
			switch( m_eCurrentStance )
			{
			case ST_POWER:
				if (m_pobAttackDefinition->m_pobPowerKOsBack)
					return m_pobAttackDefinition->m_pobPowerKOsBack->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance];
				break;

			case ST_RANGE:
				if (m_pobAttackDefinition->m_pobRangeKOsBack)
					return m_pobAttackDefinition->m_pobRangeKOsBack->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance];
				break;

			case ST_SPEED:
				if (m_pobAttackDefinition->m_pobSpeedKOsBack)
					return m_pobAttackDefinition->m_pobSpeedKOsBack->m_obAnimation[pobStrike->GetAttackDataP()->m_eReactionAppearance];
				break;

			default:
				ntAssert( 0 );
				return CHashedString();
				break;
			}
		}

		// The range of reaction directions has been extended?
		else
		{
			ntAssert( 0 );
			return CHashedString();
		}
	}

	return CHashedString();
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateKO
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::UpdateKO( float )
{
	//CGatso::Start("CAttackComponent::UpdateKO");

	if ( m_bNeedLedgeRecover )
	{
		// Something wrong if this is the case
		ntError( CanLedgeRecover() );

		// See if we can instant ko recover attack
		m_obAttackTracker.SelectLedgeRecoverAttack();

		const CEntity* pobBlokeWhoKnockedMeOffLedge = m_pobStruckStrike->GetOriginatorP();

		// If we successfully generated a strike...
		if ( m_obAttackTracker.RequestIsReady() && GenerateMyStrike( false, false ) && StartAttack() ) 
		{
			GenerateDirectStrike( pobBlokeWhoKnockedMeOffLedge, false, false, false );
			return;
		}

		// This gets cleared here if we failed to do the ledge recover, otherwise it'll be cleared in EndAttack
		m_bNeedLedgeRecover = false;
	}

	// This is the only state in which this bool makes sense, so only handling it here
	if ( m_bGoingToBeAerialed )
	{		
		m_pobParentEntity->GetPhysicsSystem()->SetCharacterCollisionRepresentation( 
				Physics::System::CHARACTER_CONTROLLER,
				Physics::System::CHARACTER_CONTROLLER_COLLIDE_WITH_EVERYTHING );

		// Mmm hack.
		Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
		if (pobAdvCC && pobAdvCC->GetCharacterControllerType() == Physics::CharacterController::RIGID_BODY)
		{
			pobAdvCC->SetLinearVelocity(CDirection(CONSTRUCT_CLEAR));
		}

		if ( m_eStruckReactionZone == RZ_FRONT )
		{
			m_obAttackMovement.StartPausedKOFallMovement( m_pobAttackDefinition->m_pobFrontKODefinition );
		}
		else
		{
			m_obAttackMovement.StartPausedKOFallMovement( m_pobAttackDefinition->m_pobBackKODefinition );
		}

		m_bGoingToBeAerialed = false;
	}

	// If we are within the KO recover time then set the instant recover flag and prevent rest of health being deducted
	if ( ( m_fStateTime < m_pobAttackDefinition->m_fKOInstantRecoverTime ) )
	{		
		// See if we can instant ko recover attack
		m_obAttackTracker.SelectInstantKORecoverAttack( m_pobParentEntity, m_eCurrentStance, m_eHitLevel );

		m_pobInstantKORecoverTargetEntity = m_pobStruckStrike->GetOriginatorP();

		// If that succeeded...
		CPoint obCheckPosition = m_pobParentEntity->GetPosition();
		obCheckPosition.Y() += 0.6f; // Shift this up a bit so we don't collide with the floor	
		if ( m_pobInstantKORecoverTargetEntity && m_obAttackTracker.RequestIsReady() && !Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition, m_obCounterAttackCheckVolumeHalfExtents,0.0f) && !Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition, m_obCounterAttackCheckVolumeHalfExtents,PI*0.25f) )
		{
			// If we successfully generated a strike...
			if ( GenerateMyStrike( false, false ) ) 
			{
				StartAttack();
			}
			else
			{			
				// If we are here then we should clear the requested attack, cos it failed.
				m_obAttackTracker.ClearRequested();
			}
		}

		// Done with this now, null it out so we're don't spanner up next time
		m_pobInstantKORecoverTargetEntity = 0;
	}

	//CGatso::Stop("CAttackComponent::UpdateKO");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartFlooredState
*
*	DESCRIPTION		Called from state system when we've hit the ground
*
***************************************************************************************************/
void CAttackComponent::StartFlooredState()
{
	//CGatso::Start("CAttackComponent::StartFlooredState");

	// Make sure we zero the wiggle get back time
	m_fWiggleGetBack = 0.0f;
	
	// Set the state to our floored state
	SetState( CS_FLOORED );

	// Start the movement - front
	if ( m_eStruckReactionZone == RZ_FRONT )
	{
		m_obAttackMovement.StartFlooredMovement(	m_pobAttackDefinition->m_pobFrontKODefinition->m_obFlooredAnimName,
													m_pobAttackDefinition->m_pobFrontKODefinition->m_obWaitAnimName,
													m_bWasFullyRagdolledInKO,
													m_pobStruckStrike && !m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement,
													false );
	}
	// ...and for the back
	else if ( m_eStruckReactionZone == RZ_BACK )
	{
		m_obAttackMovement.StartFlooredMovement(	m_pobAttackDefinition->m_pobBackKODefinition->m_obFlooredAnimName,
													m_pobAttackDefinition->m_pobBackKODefinition->m_obWaitAnimName,
													m_bWasFullyRagdolledInKO,
													m_pobStruckStrike && !m_pobStruckStrike->GetAttackDataP()->m_bSkipFallingMovement,
													false );
	}
	else
	{
		ntAssert( 0 );
	}

	//CGatso::Stop("CAttackComponent::StartFlooredState");
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateFloored
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackComponent::UpdateFloored( float )
{
	//CGatso::Start("CAttackComponent::UpdateFloored");

	// See if we can reduce the effective time of this state
	CheckWiggleAction( m_fIncapacityTime, m_fWiggleGetBack );

	// Wait till we have reached the end of the reaction time then drop out
	if ( ( m_fStateTime + m_fWiggleGetBack ) > m_fIncapacityTime )
		EndFloored();

	//CGatso::Stop("CAttackComponent::UpdateFloored");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::EndFloored
*
*	DESCRIPTION		Ends a KO - or rather the floored state
*
***************************************************************************************************/
void CAttackComponent::EndFloored()
{
	// Set to the rise wait state
	SetState( CS_RISE_WAIT );

	// This allows us to react to characters doing a special correctly and the smoothly return to speed
	m_pobParentEntity->SetTimeMultiplier( 1.0f );

	// Send a message indicating we have reached a point where we can get up
	CMessageSender::SendEmptyMessage( "msg_combat_rise_wait", m_pobParentEntity->GetMessageHandler() );

	// Flag that we need a ground attack recovery
	m_bGroundAttackRecover = true;

	// Clear the strike we reacted to
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;

	m_bWasFullyRagdolledInKO = false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::UpdateRiseWait
*
*	DESCRIPTION		This is when we are waiting on the floor - we can attack or just get up.
*
***************************************************************************************************/
void CAttackComponent::UpdateRiseWait( float fTimeDelta )
{
	//CGatso::Start("CAttackComponent::UpdateRiseWait");

	// Deal with the unused bits
	UNUSED( fTimeDelta );

	// We need to check if a request has been made
	bool bButtonPressed = false;
	m_obAttackTracker.SelectRisingAttack( m_pobParentEntity, m_eCurrentStance, bButtonPressed, m_eHitLevel );

	// If we have a input pad save the secondar pad direction at this point - only for evades
	if ( ( m_pobParentEntity->GetInputComponent() ) && ( m_obAttackTracker.RequestIsReady() ) && ( m_obAttackTracker.GetRequestedAttackClass() == AC_EVADE ) )
		m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();

	// If there is a request ready then we do it...
	if ( m_obAttackTracker.RequestIsReady() )
	{
		// If we successfully generated a strike...
		if ( GenerateMyStrike( false, false ) && StartAttack() ) 
		{
			// The counter message lets the lua state know we have attacked out of a reaction state
			CMessageSender::SendEmptyMessage( "msg_combat_countered", m_pobParentEntity->GetMessageHandler() );

			// Get out of here
			return;
		}
	}

	// Otherwise if there was a failed attack attempt or the character wishes to run away...
	else if ( (m_pobParentEntity->IsPlayer() && !m_pobParentEntity->GetPhysicsSystem()->IsActive()) 
			  ||
			  ( bButtonPressed )
		      ||
			  ( ( m_pobParentEntity->GetInputComponent() ) && ( m_pobParentEntity->GetInputComponent()->IsDirectionHeld() ) )
			  || // AIs don't get up until they're told to...
			  ( m_pobParentEntity->IsAI() && ((AI*)m_pobParentEntity)->GetAIComponent()->GetCombatComponent().IsConscious() )
			  || // Auto get up at the mo because we can't get them up any other way
			  ( ( m_pobParentEntity->GetInputComponent() ) && ( m_fStateTime > m_fAutoRiseTime ) ) 
			  ||
			  m_bGroundAttackRecover )
	{
		// Set to recover state
		SetState( CS_RECOVERING );
		m_fIncapacityTime = 0.0f;
		m_eRecoveryType = RC_RISING;

		// Start the KO recovery movement - for the front...
		if ( m_eStruckReactionZone == RZ_FRONT )
		{
			if (m_bGroundAttackRecover && !m_pobAttackDefinition->m_pobFrontKODefinition->m_obGroundAttackRiseAnimName.IsNull())
				m_obAttackMovement.StartRiseMovement( m_pobAttackDefinition->m_pobFrontKODefinition->m_obRiseAnimName );
			else
				m_obAttackMovement.StartRiseMovement( m_pobAttackDefinition->m_pobFrontKODefinition->m_obRiseAnimName );
		}
		// ...or for the back
		else if ( m_eStruckReactionZone == RZ_BACK )
		{
			if (m_bGroundAttackRecover && !m_pobAttackDefinition->m_pobBackKODefinition->m_obGroundAttackRiseAnimName.IsNull())
				m_obAttackMovement.StartRiseMovement( m_pobAttackDefinition->m_pobBackKODefinition->m_obRiseAnimName );
			else
				m_obAttackMovement.StartRiseMovement( m_pobAttackDefinition->m_pobBackKODefinition->m_obRiseAnimName );
		}
		// Otherwise perhaps the fidelity of reactions has increased?
		else
			ntAssert( 0 );

		// Clear flag
		m_bGroundAttackRecover = false;

		// Give the movement system a message to pass to the state if this movement is completed fully
		m_obAttackMovement.SetMovementMessage( "msg_combat_recovered" );
	}

	//CGatso::Stop("CAttackComponent::UpdateRiseWait");
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AddAIAttacker
*
*	DESCRIPTION		Each Attack component maintains a list of entities that are attacking. 
*					This list is used by AI attacker to determine if they should attack another
*					on the grounds of their number of attackers.
*
***************************************************************************************************/
bool CAttackComponent::AddAIAttacker( const CEntity* pAttacker, float fAttackTime, bool bAdd ) const
{
	ntstd::List< ntstd::pair<const CEntity*, float> >::iterator obIt;

	for( obIt = m_obAIAttackList.begin(); obIt != m_obAIAttackList.end(); ++obIt )
	{
		if( obIt->first == pAttacker )
		{
			if( bAdd )
				obIt->second = fAttackTime;
			else
				obIt->second += fAttackTime;

			return true;
		}
	}

	m_obAIAttackList.push_back( ntstd::pair<const CEntity*, float>( pAttacker, fAttackTime ) );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::StartKill
*
*	DESCRIPTION		Start a Kill
*
***************************************************************************************************/
void CAttackComponent::StartKill( const CStrike* pobStrike, CHashedString obChosenReceiverAnim, bool bIsReaction )
{
	// See if we can use a ragdoll, counters/supers must be able to use a ragdoll for collision safety reasons
	 if (!Physics::RagdollPerformanceManager::Get().CanAnimated() && !pobStrike->ShouldSync())
		m_bCannotUseRagdoll = true;
	 else
		m_bCannotUseRagdoll = false;

	//CGatso::Start("CAttackComponent::StartKill");

	// Check on our input quality
	ntAssert( pobStrike->GetAttackDataP() );	

	// Save a pointer to the strike
	m_pobStruckStrike = pobStrike;

	// Log being killed
	m_pobCombatEventLogManager->AddEvent(CE_GOT_KILLED, m_pobStruckStrike->GetOriginatorP(), (void*)m_pobStruckStrike->GetAttackDataP() );
	if (m_pobStruckStrike->GetOriginatorP())
		m_pobStruckStrike->GetOriginatorP()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_KILL, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );
	else if ( m_pobStruckStrike->GetProjectile() && m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter() )
		m_pobStruckStrike->GetProjectile()->ToProjectile()->GetShooter()->GetAttackComponent()->m_pobCombatEventLogManager->AddEvent(CE_CAUSED_KILL, m_pobParentEntity, (void*)m_pobStruckStrike->GetAttackDataP() );

	m_bUseProceduralKODeath = false;

	// If this is a reaction, then we really need to go through our lua state system to kill us off properly
	if (bIsReaction)
	{
		// Just send a message to lua and we should drop our swords and everything as we need to, then come back in here when it's not a reaction type
	//	ntError(0);
		m_pobParentEntity->Kill();
	}
	else
	{
		// Is it OK to use a specific animation?
		bool bUseSpecific = !( !pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() && !pobStrike->ShouldSync() );

		// If the incoming attack is not syncronised - in which case this movement will be sorted
		if ( pobStrike->GetAttackDataP()->m_obSyncReceiverAnim.IsNull() || !pobStrike->ShouldSync() )
		{
			// Update our time based on the timings of the attacker
			if ( pobStrike->GetOriginatorP() )
				m_pobParentEntity->SetTimeMultiplier( m_pobStruckStrike->GetOriginatorP()->GetTimeMultiplier() );

			if(!m_bCannotUseRagdoll && pobStrike->GetAttackDataP()->m_bUseProceduralKO)
			{
				// clear movement controllers. porsedural KO will be started in SetRagdollState
				//m_pobParentEntity->GetMovement()->ClearControllers();	
				m_bUseProceduralKODeath = true;
			}
			else
			{
				// Find the hit reaction
				CHashedString obHitReaction = FindKOHitAnim( m_eStruckReactionZone, m_pobStruckStrike, m_eCombatState, bUseSpecific, obChosenReceiverAnim );

				// Kick off the movement - for the specific animation case
				if ( obHitReaction == m_pobStruckStrike->GetAttackDataP()->m_obReceiverAnim )
				{
					if ( m_eStruckReactionZone == RZ_FRONT )
						m_obAttackMovement.StartKillMovement( obHitReaction, m_eStruckReactionZone, m_pobStruckStrike->GetAttackTimeScalar(), m_bCannotUseRagdoll, m_pobAttackDefinition->m_pobFrontKODefinition );
					else if ( m_eStruckReactionZone == RZ_BACK )
						m_obAttackMovement.StartKillMovement( obHitReaction, m_eStruckReactionZone, m_pobStruckStrike->GetAttackTimeScalar(), m_bCannotUseRagdoll, m_pobAttackDefinition->m_pobBackKODefinition );
					else
						ntAssert( 0 );
				}
				// ...or for the front general case
				else if ( m_eStruckReactionZone == RZ_FRONT )
					m_obAttackMovement.StartKillMovement( obHitReaction, m_eStruckReactionZone, m_pobStruckStrike->GetAttackTimeScalar(), m_bCannotUseRagdoll, m_pobAttackDefinition->m_pobFrontKODefinition );

				// ...and for the back
				else if ( m_eStruckReactionZone == RZ_BACK )
					m_obAttackMovement.StartKillMovement( obHitReaction, m_eStruckReactionZone, m_pobStruckStrike->GetAttackTimeScalar(), m_bCannotUseRagdoll, m_pobAttackDefinition->m_pobBackKODefinition );

				// Do we have more directions to be hit in now?
				else
					ntAssert( 0 );
			}
		}
		
		// Give the movement system a message to pass on KO completion
		m_obAttackMovement.SetMovementCallback( Character::KillEnt, true );

		// Set to dead state
		SetState( CS_DEAD, pobStrike );

		// Show that a KO has occurred
		DoHitEffect( pobStrike->GetOriginatorP() );

		// Trigger sound effects
		DoHitSound( pobStrike );

		// Clear this flag as we've used it
		m_bSyncAttackInterrupted = false;
	}

	//CGatso::Stop("CAttackComponent::StartKill");
}

void CAttackComponent::NotifyDieOutOfCurrentMovement()
{
	//FIXME HACK ALPHA EVILNESS (that should be enough for people to find it).
	//To stop flying fox from being killed accidently in walkways, we need to make sure this message does nothing on him.
	//Post-alpha we can add a flag for this, but we decided not to for alpha (45mins from now) to avoid changing the memory map at all.
	if ( m_pobParentEntity && (strcmp(m_pobParentEntity->GetName().c_str(), "Nme_AerialGeneral") == 0) )
	{
		//Do nothing for this character.
		ntPrintf("**** EVIL ALPHA HACK - Don't do NotifyDieOutOfCurrentMovement() on Aerial General in walkways ****\n");
		return;
	}

	// At the moment as far as I know this should only be used when in KO
	if ( m_eCombatState == CS_KO )
	{
		if (m_bNotifiedToDieOutOfCurrentMovement)
			return;

		if (m_bCannotUseRagdoll)
		{
			// Chain on an impact movement
			if ( m_eStruckReactionZone == RZ_FRONT )
				m_obAttackMovement.StartImpactMovement(m_pobAttackDefinition->m_pobFrontKODefinition->m_obFlooredAnimName, true);
			else if ( m_eStruckReactionZone == RZ_BACK )
				m_obAttackMovement.StartImpactMovement(m_pobAttackDefinition->m_pobBackKODefinition->m_obFlooredAnimName, true);
		}

		// Make sure a kill takes place
		m_obAttackMovement.SetMovementCallback( Character::KillEnt, true );

		m_bNotifiedToDieOutOfCurrentMovement = true;

		// Set that we're dying, this'll make us untargetable so we'll be left to complete our movements in peace, then die tidily
		// Not using set state to avoid resetting ragdoll/collision settings unnecessarily
		m_fStateTime = 0.0f;
		m_eCombatState = CS_DYING;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::MakeDead
*
*	DESCRIPTION		Clean up struck strike, set dead
*
***************************************************************************************************/
void CAttackComponent::MakeDead( void ) 
{
	// Make sure any current strike is cleared up
	NT_DELETE_CHUNK( Mem::MC_MISC, m_pobStruckStrike );
	m_pobStruckStrike = 0;

	SetState( CS_DEAD ); 
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Access_CanBreakAway
*
*	DESCRIPTION		Is it possible for an AI character to break out of the current attack string
*					it is in?
*
***************************************************************************************************/
bool CAttackComponent::AI_Access_CanBreakAway( void ) const
{
	// We'll say yes if they are in standard state
	if ( m_eCombatState == CS_STANDARD )
		return true;

	// Otherwise if they are in the attack state then they can breakaway if the movement
	// window is active
	if ( m_eCombatState == CS_ATTACKING )
	{
		// As soon as the movement popout is reached a character may run away if they wish
		if ( ( IsInMovementWindow() ) 
			&&
			( m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike.GetNumFlippers() > 0 ) )
		{
			return true;
		}
	}

	// If we are here then we can't escape
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Access_GetPostStrikeRecovery
*
*	DESCRIPTION		If in a post stirke recover window return the amount of time remaining, else
*					just 0
*
***************************************************************************************************/
float CAttackComponent::AI_Access_GetPostStrikeRecovery(void) const
{
	// Current strike not valid - just return 0.0
	if( !m_pobMyCurrentStrike )
		return 0.0f;

	if( !m_pobMyCurrentStrike->GetAttackDataP() )
		return 0.0f;

	if( m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike.GetNumFlippers() <= 0 )
		return 0.0f;

	// Find the time at the end of the last strike window
	float fPostStrikeTime = (--m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike.End())->X2( m_fAttackScalar );

	// Get the block popput time window
	float fBlockPopoutTime = m_pobMyCurrentStrike->GetAttackDataP()->m_obBlockPopOut.GetNumFlippers() > 0 ? 
								m_pobMyCurrentStrike->GetAttackDataP()->m_obBlockPopOut.GetFirstValue( m_fAttackScalar ) :
								m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime( m_fAttackScalar );

	// If valid - return the remaining time
	if( fPostStrikeTime < m_fStateTime && m_fStateTime < fBlockPopoutTime )
		return fBlockPopoutTime - m_fStateTime;
	
	// return 0.0 as the time can't be found
	return 0.0f;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Command_Breakout
*
*	DESCRIPTION		Early exit from an attack
*
***************************************************************************************************/
bool CAttackComponent::AI_Command_Breakout( void )
{
	// Make sure we can do as bid
	if ( !AI_Access_CanBreakAway() )
		return false;

	ProcessAttackeeList();
	CMessageSender::SendEmptyMessage( "msg_combat_breakout", m_pobParentEntity->GetMessageHandler() );
	EndRecovery();
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Access_CanRequestAttack
*
*	DESCRIPTION		Checks whether now would be a sensible time for the AI to request an attack
*					from us.
*
***************************************************************************************************/
bool CAttackComponent::AI_Access_CanRequestAttack( void ) const 
{ 
	// What states may an attack be requested in? Definately not
	if ( ( m_eCombatState == CS_RECOILING )
		 ||
		 ( m_eCombatState == CS_FLOORED )
		 ||
		 ( m_eCombatState == CS_BLOCK_STAGGERING )
		 ||
		 ( m_eCombatState == CS_IMPACT_STAGGERING )
		 ||
		 ( m_eCombatState == CS_DEAD )
		 ||
		 ( m_eCombatState == CS_KO )
		 ||
		 ( m_eCombatState == CS_DYING )
		 || 
		 ( m_eCombatState == CS_INSTANTRECOVER ) )
	{
		return false;
	}

	// Not possible if an attack has already been requested
	if ( !m_obAttackTracker.WillTakeAttackRequests() )
		return false;

	// If we are deflecting or held we can only hit within the counter point
	if ( ( ( m_eCombatState == CS_HELD ) 
		   ||
		   ( m_eCombatState == CS_DEFLECTING ) )
		 &&
		 ( m_fStateTime > m_fCounteringTime ) )
	{
		return false;
	}

	// If we are attacking we can only request in the next move window
	if ( ( m_eCombatState == CS_ATTACKING ) && ( IsInNextMoveWindow() == 0 ) )
		return false;

	// Everything else is good - go for it
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::AI_Access_GetCurrentAttackDataP
*
*	DESCRIPTION		
*
***************************************************************************************************/
const CAttackData* CAttackComponent::AI_Access_GetCurrentAttackDataP() const 
{ 
	if ( m_pobMyCurrentStrike )
        return m_pobMyCurrentStrike->GetAttackDataP();
	else
		return 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::DoHitEffect
*
*	DESCRIPTION		This must be called after the new state has been set.  i.e. if an attack makes
*					one recoil then the CS_RECOIL state must be set before we are here, because the
*					effect is chosen based on state.
*
***************************************************************************************************/
void CAttackComponent::DoHitEffect( const CEntity* pAttacker )
{
	// Its possible we don't have an attacker
	if ( !pAttacker )
		return;

	// The attackee is our parent entity - make sure we have one
	const CEntity* pAttackee = m_pobParentEntity;
	ntAssert( pAttackee );

	// find the attackers current effect trigger object
	CombatEffectsTrigger* pTriggerObj = CombatEffectsDefinition::GetEffectsTrigger( pAttacker->GetAttackComponent()->m_iCurrEffectsTrigger );
	if (pTriggerObj)
	{
		switch( m_eCombatState )
		{
			case CS_RECOILING:			pTriggerObj->TriggerHitEffect(pAttacker, pAttackee);		break;
			case CS_DEFLECTING:			pTriggerObj->TriggerBlockEffect(pAttacker, pAttackee);		break;
			case CS_KO:					pTriggerObj->TriggerKOEffect(pAttacker, pAttackee);			break;
			case CS_DEAD:				pTriggerObj->TriggerKOEffect(pAttacker, pAttackee);			break;
			case CS_BLOCK_STAGGERING:	pTriggerObj->TriggerStaggerEffect(pAttacker, pAttackee);	break;
			case CS_IMPACT_STAGGERING:	pTriggerObj->TriggerStaggerEffect(pAttacker, pAttackee);	break;
			case CS_DYING:				pTriggerObj->TriggerDeathEffect(pAttacker, pAttackee);		break;

			default: break;
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::DoHitSound
*
*	DESCRIPTION		This must be called after the new state has been set.  i.e. if an attack makes
*					one recoil then the CS_RECOIL state must be set before we are here, because the
*					effect is chosen based on state.
*
***************************************************************************************************/
void CAttackComponent::DoHitSound ( const CStrike* pobStrike )
{
	if (pobStrike && pobStrike->GetOriginatorP())
	{
	if (pobStrike->GetAttackDataP()->GetStyleType()==STYLE_TYPE_SPEED ||
		pobStrike->GetAttackDataP()->GetStyleType()==STYLE_TYPE_POWER ||
		pobStrike->GetAttackDataP()->GetStyleType()==STYLE_TYPE_RANGE)
	{
		if (pobStrike->GetOriginatorP())
		GameAudioManager::Get().TriggerCombatHitSound(pobStrike->GetOriginatorP(),m_pobParentEntity,m_eCombatState,pobStrike->GetAttackDataP()->m_eAttackClass);
		else if (pobStrike->GetProjectile())
			GameAudioManager::Get().TriggerCombatHitSound(pobStrike->GetProjectile(),m_pobParentEntity,m_eCombatState,pobStrike->GetAttackDataP()->m_eAttackClass);
		else
		{
			// No projectile or attacker to do sound with
			ntError( 0 );
		}
	}
}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::BuildStrikeForTarget
*
*	DESCRIPTION		
*
***************************************************************************************************/
const CStrike* CAttackComponent::BuildStrikeForTarget( const CAttackData* pobAttackData, const CEntity* pobTarget, bool bCounter )
{
	// Save it's current value
	float fOldAttackScalar = m_fAttackScalar;

	// Update attack scalar from special if it's a range attack
	if (IsDoingSpecial() && (pobAttackData->m_eAttackClass == AC_RANGE_FAST || pobAttackData->m_eAttackClass == AC_RANGE_MEDIUM) )
		m_fAttackScalar = fOldAttackScalar * m_pobAttackSpecial->GetAttackSpeedMultiplier();

	// Build a strike with what we know
	const CStrike* pobStrike = NT_NEW_CHUNK( Mem::MC_MISC ) CStrike(	m_pobParentEntity, 
											pobTarget, 
											pobAttackData, 
											m_fAttackScalar, 
											pobAttackData->m_fMaxDistance, 
											true,
											bCounter,
											false,
											false,
											!pobAttackData->m_obSyncReceiverAnim.IsNull(),
											false,
											0,
											m_pobParentEntity->GetPosition() );

	// Restore old value 
	m_fAttackScalar = fOldAttackScalar;

	// Send it on it's way
	return pobStrike;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::BuildStrikeFromInput
*
*	DESCRIPTION		
*
***************************************************************************************************/
const CStrike* CAttackComponent::BuildStrikeFromInput( const CAttackData* pobAttackData )
{	
	// Use the pad direction to choose an attack
	float fAngle = 0.0f;
	if ( m_pobParentEntity->GetInputComponent() )
		fAngle = MovementControllerUtilities::RotationAboutY(	m_pobParentEntity->GetMatrix().GetZAxis(), 
																m_pobParentEntity->GetInputComponent()->GetInputDir() );

	// Make sure we have got some valid data
	ntAssert( pobAttackData );

	// Find the attack distance - if we are doing a speed extra attack with no target it need truncating
	float fMaxRange = pobAttackData->m_fMaxDistance;

	// Build and return a strike
	const CStrike* pobStrike = NT_NEW_CHUNK( Mem::MC_MISC ) CStrike(	m_pobParentEntity, 
											0, 
											pobAttackData, 
											m_fAttackScalar, 
											fMaxRange, 
											true,
											false,
											false,
											false,
											false,
											false,
											0,
											m_pobParentEntity->GetPosition() );

	// Send it on it's way
	return pobStrike;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CheckSpecificAttackVulnerabilityZones
*
*	DESCRIPTION		Check zones of specific attack vulnerability.
*
***************************************************************************************************/
const CAttackLink* CAttackComponent::CheckSpecificAttackVulnerabilityZones(CEntity* pobAttacker, SpecificAttackVulnerabilityZone** pobVulnerabilityZone)
{
	CAttackLink* pobRet = 0;

	if ( m_pobAttackDefinition && m_pobAttackDefinition->m_obSpecificVulnerabilities.size() > 0 )
	{
		for (ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY>::const_iterator obIt = m_pobAttackDefinition->m_obSpecificVulnerabilities.begin();
			obIt != m_pobAttackDefinition->m_obSpecificVulnerabilities.end();
			obIt++)
		{
			//No need to check disabled zones.
			if((*obIt)->IsDisabled())
			{
				continue;
			}

			pobRet = (*obIt)->IsInZone(m_pobParentEntity,pobAttacker);
			if (pobRet)
			{
				*pobVulnerabilityZone = *obIt;
				break;
			}
		}
	}

	return pobRet;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::BuildStrikeFromData
*
*	DESCRIPTION		This function just gets more and more knotted.  But it works.  So i don't really
*					want to touch it anymore.
*
***************************************************************************************************/
const CStrike* CAttackComponent::BuildStrikeFromData( bool bCounter, bool bAutoLink )
{
	// Make sure that we have a request ready
	ntAssert( m_obAttackTracker.RequestIsReady() );

	// Find our current target
	const CEntity* pobInitialTarget = GetCurrentTargetP();

	// If this attack is only suitable to execute on someone we have just hit
	if ( m_obAttackTracker.GetRequestedAttackDataP()->m_bHoldPreviousTarget && !m_bForcedModeEnabled && !bCounter )
	{
		// Make sure we have an existing target
		if ( pobInitialTarget )
		{
			// Get our strike using the target
			return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobInitialTarget, false );
		}

		// Otherwise the requested attack is invalid
		return 0;
	}

	// If the attack is an autolinking attack then we stay with the initial target
	if ( bAutoLink )
	{
		// If we have an initial target then use it
		if ( pobInitialTarget )
		{	
			return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobInitialTarget, false );
		}

		// Otherwise carry out the attack with no target
		else
			return BuildStrikeFromInput( m_obAttackTracker.GetRequestedAttackDataP() );
	}

	// Look for a new target
	const CEntity* pobNewTarget = 0;

	// If this is a counter we must have a strike that we are countering
	if ( bCounter )
	{
		// Warn if we are doing a counter at the wrong time
		ntError_p( m_pobStruckStrike, ( "We are trying to counter with out an incoming attack.\n" ) );

		// Use the originator of the attack we are countering
		pobNewTarget = m_pobStruckStrike->GetOriginatorP();
		// If it's a counter and it's from a projectile strike, our initial target should be the thrower of the projectile
		if (!pobNewTarget && m_pobStruckStrike->GetProjectile())
		{
			// If this isn't a safe cast then there's something wrong with the world.
			pobNewTarget = ((Object_Projectile*)m_pobStruckStrike->GetProjectile())->GetShooter();
		}

		if (!pobNewTarget)
		{
			ntPrintf("Tried a counter, but had no target from previous strike.\n");
		}
	}

	// If our attack is grab we do things slightly differently. 
	else if ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_GOTO || m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE )
	{
		// Use Sweep value from AttackDefinition as grabs are always a direct attack
		
		// If this is exactly the same grab attack as one we are currently doing, reject it
		if (m_pobMyCurrentStrike && (m_pobMyCurrentStrike->GetAttackDataP() == m_obAttackTracker.GetRequestedAttackDataP()) )
		{
			m_obAttackTracker.ClearRequested();
			return 0;
		}

		// Directly implement a targeting query
		if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetTargetP() && m_obAttackTracker.GetRequestedAttackDataP()->m_bExemptPreviousTarget )
			pobNewTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( m_obAttackTracker.GetRequestedAttackDataP()->m_eTargetType, m_pobParentEntity, false, false, CDirection( 0.0f, 0.0f, 1.0f ), m_pobMyCurrentStrike->GetTargetP() );
		else
			pobNewTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( m_obAttackTracker.GetRequestedAttackDataP()->m_eTargetType, m_pobParentEntity );

		// See if we're trying to grab a boss - need to ask them specifically
		if (pobNewTarget && pobNewTarget->IsBoss())
		{
			Boss* pobBoss = (Boss*)pobNewTarget;
			if (!pobBoss->IsVulnerableTo(m_obAttackTracker.GetRequestedAttackDataP()))
			{
				pobNewTarget = 0;
			}
		}

		// If the chosen target is not susceptible to grab attacks then we need to reselect
		if (pobNewTarget && (pobNewTarget->GetAttackComponent()->m_eCombatState == CS_KO || pobNewTarget->GetAttackComponent()->m_eCombatState == CS_HELD 
			|| !pobNewTarget->GetAttackComponent()->CharacterUninterruptibleVulnerableTo( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass )) )
		{
			// Then we can't attack them, do a failed grab
			pobNewTarget = 0;
		}

		//If we're actually attempting a skill-evade, then replace our target with the one chosen when selecting to perform a skill evade.
		if(m_pobSkillEvadeTargetEntity)
		{
			pobNewTarget = m_pobSkillEvadeTargetEntity;
		}

		//if (pobNewTarget)
		//	ntPrintf("Grab strike: Targeting %s, in %s.\n",ObjectDatabase::Get().GetNameFromPointer(pobNewTarget),g_apcCombatStateTable[pobNewTarget->GetAttackComponent()->m_eCombatState]);
	}
	else
	{
		// See if we can improve on our initial lockon - need to use a specific direction for evades
		if ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_EVADE )
		{
			// Get latest stick input
			if (m_pobParentEntity->GetInputComponent())
				m_obEvadeDirection = m_pobParentEntity->GetInputComponent()->GetInputDirAlt();

			// Targeted evades are dead.
			pobNewTarget = 0;

			// See if we can detect that we've evaded an incoming attack
			/*if (pobNewTarget && pobNewTarget->GetAttackComponent()->m_pobMyCurrentStrike)
			{
				// Check if he's already struck us, if he hasn't, then we've escaped it
				if (m_pobStruckStrike != pobNewTarget->GetAttackComponent()->m_pobMyCurrentStrike)
					m_pobCombatEventLogManager->AddEvent(CE_EVADED_INCOMING_ATTACK, pobNewTarget, (void*)pobNewTarget->GetAttackComponent()->m_pobMyCurrentStrike->GetAttackDataP());
			}*/
		}
		else
		{
			// Try a ground attack first
			// Check for a ground target, if there is one, reselect a ground attack
			if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetTargetP() && m_obAttackTracker.GetRequestedAttackDataP()->m_bExemptPreviousTarget )
				pobNewTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_DOWN, m_pobParentEntity, true, false, m_obEvadeDirection, m_pobMyCurrentStrike->GetTargetP() );
			else
				pobNewTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_DOWN, m_pobParentEntity, true, false, m_obEvadeDirection );

			if (pobNewTarget && pobNewTarget->GetAttackComponent()->m_pobAttackDefinition->m_bIsGroundAttackable)
			{
				// If this is a range fast attack, do it
				if (m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass == AC_RANGE_FAST)
				{
					return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobNewTarget, bCounter );
				}
				// Try to reselect a ground attack
				if (m_obAttackTracker.SelectGroundAttack(m_pobParentEntity, m_eCurrentStance, m_eHitLevel, pobNewTarget->GetAttackComponent()->m_eStruckReactionZone ))
				{
					return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobNewTarget, bCounter );
				}

				// Null this out at this point because we've tried to do a ground attack and failed, so we don't want later lines of code to think it's ok to start one
				pobNewTarget = 0;
			}			

			if (!pobNewTarget)
			{
				// Normal attack targeting 
				// But if we have an instant ko bloke, use him over anyone else
				if ( m_pobInstantKORecoverTargetEntity )
				{
					pobNewTarget = m_pobInstantKORecoverTargetEntity;
				}
				// If we have a juggled target, check if we can safely carry on hitting them, if we're gonna try to lock on, then we have to do a full targetting check regardless
				else if ( m_pobLastJuggledEntity && !m_pobParentEntity->GetAwarenessComponent()->IsGoingToLockonTargeting() )
				{	
					// Need to check special juggle ko height bounds
					float fYDiff = m_pobLastJuggledEntity->GetPosition().Y() - m_pobParentEntity->GetPosition().Y();
					if (fYDiff > m_pobParentEntity->GetAwarenessComponent()->GetAttackTargetingData()->m_fLowerJuggleHeight && fYDiff < m_pobParentEntity->GetAwarenessComponent()->GetAttackTargetingData()->m_fUpperJuggleHeight)
					{
					//ntPrintf("*** Sucessfully automatically targeted juggle bloke.\n");
					pobNewTarget = m_pobLastJuggledEntity;
				}
				else
				{
						//ntPrintf("*** Automatically targeted juggle bloke rejected because of height.\n");
					}
				}
				else
				{
					if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetTargetP() && m_obAttackTracker.GetRequestedAttackDataP()->m_bExemptPreviousTarget )
						pobNewTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( m_obAttackTracker.GetRequestedAttackDataP()->m_eTargetType, m_pobParentEntity, false, false, CDirection( 0.0f, 0.0f, 1.0f ), m_pobMyCurrentStrike->GetTargetP() );
					else
						pobNewTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( m_obAttackTracker.GetRequestedAttackDataP()->m_eTargetType, m_pobParentEntity );				
				
					if (pobNewTarget && m_pobLastJuggledEntity && pobNewTarget == m_pobLastJuggledEntity)
					{
						//ntPrintf("*** Sucessfully normally targeted juggle bloke.\n");
					}
				} 
			}
		}
	}

	// Check if we can swap it out with a specific zone attack
	if (pobNewTarget)
	{
		// Cocking constness being a pain
		CAttackComponent* pobAttack = const_cast< CAttackComponent* >(pobNewTarget->GetAttackComponent());
		// Check for specific attack vulnerability zones in my position for the target and swap out attack link if we're in one

		SpecificAttackVulnerabilityZone* pobVulnerabilityZone = NULL;
		const CAttackLink* pobNewLink = pobAttack->CheckSpecificAttackVulnerabilityZones(m_pobParentEntity, &pobVulnerabilityZone);
		if (pobNewLink && pobVulnerabilityZone)
		{
			// If we can select something from this link, this will do it
			// If we can't, our requested attack data will not be changed and we'll carry on as before
			if(m_obAttackTracker.SelectSpecificAttackFromVulnerabilityZone(pobNewLink))
			{
				//If we successfully selected an attack from the vulnerability zone, then iterate the 'use' count on that zone.
				//This will stop it being used next time round if it is flagged as 'remove if successful'.
				pobVulnerabilityZone->IncrementNumUses();
			}
		}
	}

	// If this attack is not a counter and we have changed targets
	if ( ( pobNewTarget != pobInitialTarget ) && ( !bCounter ) && ( ( m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass != AC_GRAB_GOTO && m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass != AC_GRAB_HOLD && m_obAttackTracker.GetRequestedAttackDataP()->m_eAttackClass != AC_GRAB_STRIKE ) ) )
	{
		// No longer restarting at the root of the attack tree, just continuing our attack on the new target
			if ( pobNewTarget ) 
				return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobNewTarget, bCounter );
	}

	// Find the best strike to get at the given target
	if ( pobNewTarget )
	{
		// If the new target is attacking us and currently within its intercept window then
		// we may want to carry out an alternative version of the attack...
		if ( ( pobNewTarget->GetAttackComponent()->IsInInterceptWindow() > 0 )
			 &&
			 ( pobNewTarget->GetAttackComponent()->GetCurrentTargetP() == m_pobParentEntity ) 
			 && // If we're autocountering, best to forget about intercepts
			 !m_bAutoCounter )
		{
			// If we find a new attack then use that instead
			if ( m_obAttackTracker.SelectInterceptEquivalent(m_eHitLevel) )
			{
				return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobNewTarget, bCounter );
			}
		}

		// Get our strike using the target
		return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobNewTarget, bCounter );
	}

	// It might be that we can try to replace this attack with one with a longer range
	if ( SuitableForToLockon( m_obAttackTracker.GetRequestedAttackDataP(), bCounter ) )
	{
		// Try for an attack from the medium cluster
		// Can we find a target for the new attack?
		CEntity* pobTestTarget = 0;
		if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetTargetP() && m_obAttackTracker.GetRequestedAttackDataP()->m_bExemptPreviousTarget )
			pobTestTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_MEDIUM_ATTACK, m_pobParentEntity, false, false, CDirection( 0.0f, 0.0f, 1.0f ), m_pobMyCurrentStrike->GetTargetP() );
		else
			pobTestTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_MEDIUM_ATTACK, m_pobParentEntity );

		if ( pobTestTarget && m_pobAttackDefinition->m_pobClusterStructure->m_pobMediumRangeCluster )
		{
			// See if there is a medium range equivalent of the current attack
			if ( m_obAttackTracker.SelectMediumRangeEquivalent() )
			{
				// If that was successful - use this attack
				if ( pobNewTarget )
				{
					return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobNewTarget, bCounter );
				}

				// Otherwise we must clear out the unused requested attack
				else if ( pobInitialTarget )
					m_obAttackTracker.SelectInitialAttackEquivalent();
				else
					m_obAttackTracker.SelectNextAttackEquivalent();
			}
		}

		// Try for an attack from the long cluster

		// Can we find a target for the new attack?
		if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetTargetP() && m_obAttackTracker.GetRequestedAttackDataP()->m_bExemptPreviousTarget )
			pobTestTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_LONG_ATTACK, m_pobParentEntity, false, false, CDirection( 0.0f, 0.0f, 1.0f ), m_pobMyCurrentStrike->GetTargetP() );
		else
			pobTestTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( AT_TYPE_LONG_ATTACK, m_pobParentEntity );

		if ( pobTestTarget && m_pobAttackDefinition->m_pobClusterStructure->m_pobLongRangeCluster )
		{
			// See if there is a long range equivalent of the current attack
			if ( m_obAttackTracker.SelectLongRangeEquivalent() )
			{
				// Can we find a target for the new attack?
				if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetTargetP() && m_obAttackTracker.GetRequestedAttackDataP()->m_bExemptPreviousTarget )
					pobNewTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( m_obAttackTracker.GetRequestedAttackDataP()->m_eTargetType, m_pobParentEntity, false, false, CDirection( 0.0f, 0.0f, 1.0f ), m_pobMyCurrentStrike->GetTargetP() );
				else
					pobNewTarget = m_pobParentEntity->GetAwarenessComponent()->GetTarget( m_obAttackTracker.GetRequestedAttackDataP()->m_eTargetType, m_pobParentEntity);

				// If that was successful - use this attack
				if ( pobNewTarget )
				{
					return BuildStrikeForTarget( m_obAttackTracker.GetRequestedAttackDataP(), pobNewTarget, bCounter );
				}

				// Otherwise we must clear out the unused requested attack
				else if ( pobInitialTarget )
					m_obAttackTracker.SelectInitialAttackEquivalent();
				else
					m_obAttackTracker.SelectNextAttackEquivalent();
			}
		}	
	}

	// Just make an attack using the input details as a parameter 
	return BuildStrikeFromInput( m_obAttackTracker.GetRequestedAttackDataP() );
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::SuitableForToLockon
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackComponent::SuitableForToLockon( const CAttackData* pobAttackData, bool bCounter )
{
	// If this is a counter then we shouldn't try for a longer move
	if ( bCounter )
		return false;

	// If we are in the rise wait state then we can't do a to-lockon
	if ( m_eCombatState == CS_RISE_WAIT )
		return false;

	// If we are currently within an aerial attack then no
	if ( m_bAerialCombo )
		return false;

	// If the attack is not of our standard attack type then no...
	if (	( pobAttackData->m_eAttackClass != AC_SPEED_FAST )
			&&
			( pobAttackData->m_eAttackClass != AC_SPEED_MEDIUM )
			&&
			( pobAttackData->m_eAttackClass != AC_POWER_FAST )
			&&
			( pobAttackData->m_eAttackClass != AC_POWER_MEDIUM )
			&&
			( pobAttackData->m_eAttackClass != AC_RANGE_FAST )
			&&
			( pobAttackData->m_eAttackClass != AC_RANGE_MEDIUM ) )
	{
		return false;
	}

	// Otherwise we shall give it a shot
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::GenerateMyStrike
*
*	DESCRIPTION		Generate a strike from a local attacktracker and send a request to remote
*					attackcomponents
*
***************************************************************************************************/
bool CAttackComponent::GenerateMyStrike( bool bCounter, bool bAutoLink )
{
	// Make sure that we have a request ready
	ntAssert( m_obAttackTracker.RequestIsReady() );

	// Generate a new Strike
	const CStrike* pobNewStrike = BuildStrikeFromData( bCounter, bAutoLink );

	// If that was a success
	if ( pobNewStrike )
	{
		/*if (pobNewStrike->GetTargetP())
			ntPrintf("%s: Generating New Strike - targeting %s, in %s.\n",ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity),ObjectDatabase::Get().GetNameFromPointer(pobNewStrike->GetTargetP()),g_apcCombatStateTable[pobNewStrike->GetTargetP()->GetAttackComponent()->m_eCombatState]);
		else
			ntPrintf("%s: Generating New Strike - targetless - %s.\n",ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity), ObjectDatabase::Get().GetNameFromPointer(pobNewStrike->GetAttackDataP()));*/

		if (!pobNewStrike->GetTargetP() && !CanAttackProceedTargetless(pobNewStrike->GetAttackDataP()))
		{
			// We need to bail completely here
			m_obAttackTracker.ClearRequested();
			NT_DELETE_CHUNK( Mem::MC_MISC, pobNewStrike );
			//ntPrintf("GenerateMyStrike: Can't do %s targetless.\n",ObjectDatabase::Get().GetNameFromPointer( pobNewStrike->GetAttackDataP() ));
			return false;
		}

		// If this is an aerial, check if we're going through an invisible wall...
		if (pobNewStrike->GetTargetP() && (pobNewStrike->GetAttackDataP()->m_eTargetType == AT_TYPE_AERIAL_COMBO || pobNewStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || pobNewStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE) )
		{
			if (!pobNewStrike->GetTargetP()->GetAttackComponent()->m_pobAttackDefinition->m_bIsAerialable)
			{
				// We can't aerial this character, bail
				m_obAttackTracker.ClearRequested();
				NT_DELETE_CHUNK( Mem::MC_MISC, pobNewStrike );
				return false;
			}
			else
			{
				// Raycast for an invisible wall
				CPoint obRayStart( m_pobParentEntity->GetPosition() );
				CPoint obRayEnd( pobNewStrike->GetTargetP()->GetPosition() );
				float fHitFraction = -1.0f;
				CDirection obHitNormal( CONSTRUCT_CLEAR );

				Physics::RaycastCollisionFlag obCollision;
				obCollision.base = 0;
				obCollision.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
				obCollision.flags.i_collide_with = Physics::AI_WALL_BIT;
				if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayEnd, fHitFraction, obHitNormal, obCollision ) )
				{
					// ...and bail completely if we are
					m_obAttackTracker.ClearRequested();
					NT_DELETE_CHUNK( Mem::MC_MISC, pobNewStrike );
					return false;
				}		
			}
		}

		if (pobNewStrike->IsCounter())
		{
			// Log that I managed to counter
			m_pobCombatEventLogManager->AddEvent(CE_COUNTER_ATTACK, pobNewStrike->GetTargetP(), (void*)pobNewStrike->GetAttackDataP() );
		}

		// Couple of super safety checks
		// Can't do a an attack that needs super safety without a super safety manager
		if ( pobNewStrike->GetAttackDataP()->m_bNeedsSuperStyleSafety && !SuperStyleSafetyManager::Exists() )
		{
			m_obAttackTracker.ClearRequested();
			NT_DELETE_CHUNK( Mem::MC_MISC, pobNewStrike );
			return false;			
		}
		// If this needs to be in a continue volume, make sure we are in a continue volume
		if (pobNewStrike->GetAttackDataP()->m_bNeedsToBeInContinueVolume && !(SuperStyleSafetyManager::Exists() && SuperStyleSafetyManager::Get().PointInSuperStyleContinueVolume(m_pobParentEntity->GetPosition())))
		{
			m_obAttackTracker.ClearRequested();
			NT_DELETE_CHUNK( Mem::MC_MISC, pobNewStrike );
			return false;			
		}

		m_bPreviousAttackWasSynchronised = m_pobMyCurrentStrike && m_pobMyCurrentStrike->ShouldSync();

		// Reset all the data associated with the current attack, only reset camera if our last attack wasn't autlinked
		bool bResetCamera = true;
		if (m_pobMyCurrentStrike && m_pobMyCurrentStrike->GetAttackDataP()->m_bAutoLink)
			bResetCamera = false;
		if (m_pobMyCurrentStrike)
			ntPrintf("%s: resetting attack %s from GenerateMyStrike.\n", ObjectDatabase::Get().GetNameFromPointer(m_pobParentEntity).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobMyCurrentStrike->GetAttackDataP()).GetString());
		ResetAttack(bResetCamera);
	
		// Update our tracker
		m_obAttackTracker.MoveToRequested();

		// Make this our current strike
		m_pobMyCurrentStrike = pobNewStrike;

		// Say we have succeeded
		return true;
	}

	// If we are here we have not generated a new strike - clear the strike
	m_obAttackTracker.ClearRequested();

	// Get out of here
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackComponent::CanAttackProceedTargetless
*
*	DESCRIPTION		Can we do this attack without a target?
*
***************************************************************************************************/
bool CAttackComponent::CanAttackProceedTargetless(const CAttackData* pobAttackData)
{
	if ((pobAttackData->m_eAttackMovementType == AMT_GROUND_TO_AIR || 
		pobAttackData->m_eAttackMovementType == AMT_AIR_TO_AIR)
		&& !m_bForcedModeEnabled )
	{
		return false;
	}

	if (pobAttackData->m_eAttackClass == AC_GRAB_HOLD || pobAttackData->m_eAttackClass == AC_GRAB_STRIKE)
	{
		// If we're ever here with a targetless grab, assert about it cos it shouldn't happen
		ntAssert( 0 );
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::AI_Access_GetTimeBeforeStrike
//! Return the time before our strike lands
//!
//------------------------------------------------------------------------------------------
float CAttackComponent::AI_Access_GetTimeBeforeStrike() const
{
	if(!m_pobMyCurrentStrike)
		return -1.0f;

	ntAssert(m_pobMyCurrentStrike->GetAttackDataP());

	float fStrikeTime = m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike.GetFirstValue(m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime(m_fAttackScalar));
	return fStrikeTime - m_fStateTime;
}

//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::AI_Access_GetTimeUntilCurrentAttackComplete
//! Return the time until the current attack has finished
//!
//------------------------------------------------------------------------------------------
float CAttackComponent::AI_Access_GetTimeUntilCurrentAttackComplete() const
{
	if(!m_pobMyCurrentStrike)
		return -1.0f;

	ntAssert(m_pobMyCurrentStrike->GetAttackDataP());

	float fAttackTime = m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime(m_fAttackScalar);
	return fAttackTime - m_fStateTime;
}

//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::AI_Access_GetLandedStrikeResponse
//! Return the targets response to our strike.
//!
//------------------------------------------------------------------------------------------
COMBAT_STATE CAttackComponent::AI_Access_GetLandedStrikeResponse( void ) const 
{
	// Make sure we have useful data to return
	if ( ( !m_bStrikeLanded )
		 || 
		 ( !m_pobMyCurrentStrike )
		 || 
		 ( !m_pobMyCurrentStrike->GetTargetP() )
		 ||
		 ( !m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent() ) )
	{
		return CS_COUNT;
	}

	// Return the combat state of the entity we have just struck
	return m_pobMyCurrentStrike->GetTargetP()->GetAttackComponent()->m_eCombatState;
}



//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::AI_Access_GetStruckStrikeAttackType					
//! Get the type of attack that we have been hit by
//!
//------------------------------------------------------------------------------------------
ATTACK_CLASS CAttackComponent::AI_Access_GetStruckStrikeAttackType( void ) const
{
	// Return a default if we are not currently struck
	if( !m_pobStruckStrike )
		return AC_COUNT;

	// Otherwise tell them
	return m_pobStruckStrike->GetAttackDataP()->m_eAttackClass;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::AI_Access_GetStaggerTime							JML ADDED FOR AI
//! Return the time we're staggered for
//!
//------------------------------------------------------------------------------------------
float CAttackComponent::AI_Access_GetStaggerTime( void ) const 
{
	ntAssert( ( m_eCombatState == CS_BLOCK_STAGGERING ) || ( m_eCombatState == CS_IMPACT_STAGGERING ) || ( m_eCombatState == CS_FLOORED ) );

	if ( ( m_eCombatState != CS_BLOCK_STAGGERING ) && ( m_eCombatState != CS_IMPACT_STAGGERING ) && ( m_eCombatState != CS_FLOORED ) )
		return 0.0f;

	return m_fIncapacityTime - m_fStateTime;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::ChooseReactionZone
//! A static helper function that allows us to work out how to react to a particular strike.
//! The fidelity of the reactions will increase as the collision solution advances.
//!
//------------------------------------------------------------------------------------------
REACTION_ZONE CAttackComponent::ChooseReactionZone( const CEntity* pobEntFrom, const CStrike* pobStrike )
{
	// Get the details of the character that issued the strike
	const CEntity* pobEntTo = pobStrike->GetOriginatorP();
	// No character? Maybe we got hit by a projectile?
	if ( !pobEntTo )
		pobEntTo = pobStrike->GetProjectile();

	// If there is no originator or projectile, default to the front
	if ( !pobEntTo )
		return RZ_FRONT;

	// Find the relative position of the person hitting me
	CDirection obRelativePosition = CDirection( pobEntTo->GetPosition() - pobEntFrom->GetPosition() );
	obRelativePosition.Normalise();

	// Compare the facing direction with the relative one - facing, hit in the front
	float fDot = pobEntFrom->GetMatrix().GetZAxis().Dot( obRelativePosition );
	if ( fDot > 0.0f )
		return RZ_FRONT;

	// Else we are facing in the same direction as the opponent so we hit them in the back
	else
		return RZ_BACK;
}

//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::ChooseReactionZone
//! A static helper function that allows us to work out how to react to a particular strike.
//! The fidelity of the reactions will increase as the collision solution advances.
//!
//------------------------------------------------------------------------------------------
CHashedString CAttackComponent::GetSpecificResponse( const CHashedString& obResponse )
{
	// If the requested response is null return a NULL string
	if ( obResponse.IsNull() )
		return CHashedString();

	// If the requested response is of zero length - return a NULL string
	if ( ntStr::IsNull(obResponse) )
		return CHashedString();

	// If we can find the animation then return the string
	if ( m_pobParentEntity->FindAnimHeader( obResponse, false ) )
		return obResponse;

	// If we are here then it is all rubbish
	return CHashedString();
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::CheckWiggleAction
//! Increases the state time to reduce the effective time of a state
//!
//------------------------------------------------------------------------------------------
void CAttackComponent::CheckWiggleAction( float fIncapacityTime, float& m_fWiggleGetBack )
{
	// Only human controlled characters can do this - AI should use a more direct method
	if ( !m_pobParentEntity->GetInputComponent() )
		return;

	// Has some sort of wiggle been performed this frame?
	if ( m_pobParentEntity->GetInputComponent()->GetGeneralButtonPressed() )
	{
		// Add a percentage of the incapacity time to the state time
		m_fWiggleGetBack += ( m_pobAttackDefinition->m_fWiggleReductionFactor * fIncapacityTime );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::Audio_Access_HitByMainCharacter
//! 
//!
//------------------------------------------------------------------------------------------
bool CAttackComponent::Audio_Access_HitByMainCharacter( void ) const
{
	// We need to have been struck
	if ( !m_pobStruckStrike )
		return false;

	// We need to have been hit directly by another character
	if ( !m_pobStruckStrike->GetOriginatorP() )
		return false;

	// And we must have been hit by the main character
	if ( m_pobStruckStrike->GetOriginatorP() != CEntityManager::Get().GetPlayer() )
		return false;

	// If we passed all those tests we are good to go
	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::Audio_Access_HitByPowerAttack
//! 
//!
//------------------------------------------------------------------------------------------
bool CAttackComponent::Audio_Access_HitByPowerAttack( void ) const
{
	// We need to have been struck
	if ( !m_pobStruckStrike )
		return false;

	// We need to have been hit directly by another character
	if ( !m_pobStruckStrike->GetOriginatorP() )
		return false;

	// We need to have an attack data
	if ( !m_pobStruckStrike->GetAttackDataP() )
		return false;

	// The attack has to be of the right class
	if ( ( m_pobStruckStrike->GetAttackDataP()->m_eAttackClass != AC_POWER_FAST )
		 &&
		 ( m_pobStruckStrike->GetAttackDataP()->m_eAttackClass != AC_POWER_MEDIUM ) )
		 return false;

	// If we passed all these attacks we are good to go
	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::Audio_Access_HitByRangeAttack
//! 
//!
//------------------------------------------------------------------------------------------
bool CAttackComponent::Audio_Access_HitByRangeAttack( void ) const
{
	// We need to have been struck
	if ( !m_pobStruckStrike )
		return false;

	// We need to have been hit directly by another character
	if ( !m_pobStruckStrike->GetOriginatorP() )
		return false;

	// We need to have an attack data
	if ( !m_pobStruckStrike->GetAttackDataP() )
		return false;

	// The attack has to be of the right class
	if ( ( m_pobStruckStrike->GetAttackDataP()->m_eAttackClass != AC_RANGE_FAST )
		 &&
		 ( m_pobStruckStrike->GetAttackDataP()->m_eAttackClass != AC_RANGE_MEDIUM ) )
		 return false;

	// If we passed all these attacks we are good to go
	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::Audio_Access_HitBySpeedAttack
//! 
//!
//------------------------------------------------------------------------------------------
bool CAttackComponent::Audio_Access_HitBySpeedAttack( void ) const
{
	// We need to have been struck
	if ( !m_pobStruckStrike )
		return false;

	// We need to have been hit directly by another character
	if ( !m_pobStruckStrike->GetOriginatorP() )
		return false;

	// We need to have an attack data
	if ( !m_pobStruckStrike->GetAttackDataP() )
		return false;

	// The attack has to be of the right class
	if ( ( m_pobStruckStrike->GetAttackDataP()->m_eAttackClass != AC_SPEED_FAST )
		 &&
		 ( m_pobStruckStrike->GetAttackDataP()->m_eAttackClass != AC_SPEED_MEDIUM ) )
		 return false;

	// If we passed all these attacks we are good to go
	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::Audio_Access_GetHitByKick
//! This is really bad stuff for an audio test.  In the future the collision sounds should
//!	be chosen based on data from the physics system - GH
//!
//------------------------------------------------------------------------------------------
bool CAttackComponent::Audio_Access_GetHitByKick( void ) const
{
	// We need to have been struck
	if ( !m_pobStruckStrike )
		return false;

	// We need to have been hit directly by another character
	if ( !m_pobStruckStrike->GetOriginatorP() )
		return false;

	// We need to have an attack data
	if ( !m_pobStruckStrike->GetAttackDataP() )
		return false;

	// Now the really nasty bit - directly check the strings
	if ( ( m_pobStruckStrike->GetAttackDataP()->m_obAttackAnimName != "hero_spd_ff" )
		 &&
		 ( m_pobStruckStrike->GetAttackDataP()->m_obAttackAnimName != "hero_spd_fmf" )
		 &&
		 ( m_pobStruckStrike->GetAttackDataP()->m_obAttackAnimName != "hero_spd_ender_f" )
		 &&
		 ( m_pobStruckStrike->GetAttackDataP()->m_obAttackAnimName != "hero_spd_ender_m" ) )
		 return false;

	// If we passed all the tests return true
	return true;

}

//------------------------------------------------------------------------------------------
//!
//!	CAttackComponent::ForceAttack
//! Horrible method that overrides anything the attackcomponent is doing and forces a new attack
//! ONLY FOR WELDER!
//------------------------------------------------------------------------------------------
bool CAttackComponent::ForceAttack( const CAttackLink* pobAttackLink )
{
	ntError( pobAttackLink );

	m_bForcedModeEnabled = true;

	m_obAttackTracker.ForceRequestedAttackToBe(pobAttackLink);
	if ( GenerateMyStrike(false,false) )
	{
		return StartAttack();
	}
	else
	{
		m_bForcedModeEnabled = false;
	}

	return false;
}

bool CAttackComponent::GetNeedsHintForButton(VIRTUAL_BUTTON_TYPE eButton) const
{
	return m_abHintForButton[eButton];
}

void CAttackComponent::SetNeedsHintForButton(VIRTUAL_BUTTON_TYPE eButton, bool bNeedsHint)
{
	m_abHintForButton[eButton] = bNeedsHint;
}

void CAttackComponent::ChangeLeadClusterTo(CClusterStructure* pobCluster)
{
	ntError( pobCluster );

	m_bLeadClusterChanged = true;
	m_obAttackTracker.InitialiseAttackStructure( pobCluster );
}

void CAttackComponent::SetDefaultLeadClusterTo(CClusterStructure* pobCluster)
{
	ntError( pobCluster );

	m_pobDefaultLeadCluster = pobCluster;
}

void CAttackComponent::ResetLeadCluster()
{
	ntError( m_pobDefaultLeadCluster );

	m_bLeadClusterChanged = false;
	m_obAttackTracker.InitialiseAttackStructure( m_pobDefaultLeadCluster );
}

bool CAttackComponent::HasSelectedANextMove() const
{
	return m_obAttackTracker.RequestIsReady();
}

float CAttackComponent::GetDistanceToClosestStrikeVolume( CPoint& obPosition ) const
{
	float fClosest = 10000.0f;
	for (ntstd::List<CombatPhysicsStrikeVolume*>::const_iterator obItVol = m_obActiveStrikeVolumes.begin();
			obItVol != m_obActiveStrikeVolumes.end();
			obItVol++ )
	{
		float fDistance = (*obItVol)->GetDistanceFrom(obPosition);
		if (fDistance < fClosest)
			fClosest = fDistance;
	}

	return fClosest;
}
