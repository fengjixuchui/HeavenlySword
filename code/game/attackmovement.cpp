/***************************************************************************************************
*
*	DESCRIPTION		Provides a simple interface for combat related movement requests
*
*	NOTES
*
***************************************************************************************************/

// Necessary includes
#include "attackmovement.h"
#include "game/attacks.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/movement.h"
#include "targetedtransition.h"
#include "strafecontroller.h"
#include "continuationtransition.h"
#include "evadetransition.h"
#include "core/exportstruct_anim.h"

#include "anim/animator.h"

#include "game/messagehandler.h"

#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

START_CHUNKED_INTERFACE	( CAttackMovementBlendInTimes, Mem::MC_ENTITY )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fAttackBlendInTime, 0.15f, AttackBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fAttackRecoverBlendInTime, 0.15f, AttackRecoverBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBlockBlendInTime, 0.15f, BlockBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fEndBlockBlendInTime, 0.15f, EndBlockBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fStanceSwitchBlendInTime, 0.15f, StanceSwitchBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fDeflectBlendInTime, 0.0f, DeflectBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fRecoilBlendInTime, 0.0f, RecoilBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fStaggerBlendInTime, 0.0f, StaggerBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fStaggerRecoverBlendInTime, 0.15f, StaggerRecoverBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fKOBlendInTime, 0.0f, KOBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fKOFallBlendInTime, 0.15f, KOFallBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fKOImpactBlendInTime, 0.075f, KOImpactBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fFlooredBlendInTime, 0.6f, FlooredBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fRiseBlendInTime, 0.15f, RiseBlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fKillBlendInTime, 0.0f, KillBlendInTime )
END_STD_INTERFACE

/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::CAttackMovement
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CAttackMovement::CAttackMovement( CEntity* pobParent, CAttackMovementBlendInTimes* pobBlendInTimes ) :
	m_pobBlendInTimes( pobBlendInTimes ),
	m_bOwnsBlendInTimes( false ),
	m_pobParentEntity( pobParent )
{
	if (!m_pobBlendInTimes)
	{
		m_pobBlendInTimes = NT_NEW_CHUNK( Mem::MC_ENTITY ) CAttackMovementBlendInTimes();
		m_bOwnsBlendInTimes = true;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::CAttackMovement
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
CAttackMovement::~CAttackMovement( void )
{
	if (m_bOwnsBlendInTimes)
	{
		NT_DELETE_CHUNK( Mem::MC_ENTITY, m_pobBlendInTimes );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::SetMovementMessage
*
*	DESCRIPTION		Set a message on the movement system to be broadcast on movement completion
*
***************************************************************************************************/
void CAttackMovement::SetMovementMessage( const char* pcMessage, bool bSetInterruptToo )
{
	// Check the data
	ntAssert( pcMessage );

	// Use it - set an interrupted message too so attacks get all info 
	m_pobParentEntity->GetMovement()->SetCompletionMessage( pcMessage, m_pobParentEntity );
	if ( bSetInterruptToo )
		m_pobParentEntity->GetMovement()->SetInterruptMessage( pcMessage, m_pobParentEntity );
}

/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::SetMovementCallback
*
*	DESCRIPTION		Set a callback on the movement system to be broadcast on movement completion
*
***************************************************************************************************/
void CAttackMovement::SetMovementCallback( void ( *CompletionCallback )( CEntity* ), bool bSetInterruptToo )
{
	// Check the data
	ntAssert( CompletionCallback );

	// Use it - set an interrupted message too so attacks get all info 
	m_pobParentEntity->GetMovement()->SetCompletionCallback( CompletionCallback, m_pobParentEntity );
	if ( bSetInterruptToo )
		m_pobParentEntity->GetMovement()->SetInterruptCallback( CompletionCallback, m_pobParentEntity );
}

/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartAttackMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartAttackMovement(	const CStrike&		obStrike, 
											const CDirection&	obEvadeDirection,
											bool				bNoBlend )
{
	// Get our attack class
	ATTACK_CLASS eAttackClass = obStrike.GetAttackDataP()->m_eAttackClass;

	// How long does the attack animation last
	const CHashedString* pobAttackAnimName = &obStrike.GetAttackDataP()->m_obAttackAnimName;
	const CAnimationHeader* pHeader = m_pobParentEntity->FindAnimHeader( *pobAttackAnimName, false );

	ntAssert_p( pHeader, ("Missing animation %s in StartAttackMovement", ntStr::GetString(*pobAttackAnimName) ) );

	float fAttackAnimDuration = pHeader->GetDuration();
	float fAttackDuration = obStrike.GetAttackTime();

	// We need different movement based on the class - these are standard...
	if ( eAttackClass == AC_EVADE )
	{
		// Create our movement definition
		EvadeTransitionDef obEvadeDefinition;
		obEvadeDefinition.SetDebugNames( "Evade", "EvadeTransitionDef" );

		// Set up the evading parameters
		obEvadeDefinition.m_fAvoidanceRadius = 0.5f;
		obEvadeDefinition.m_fMovementDuration = fAttackDuration;
		obEvadeDefinition.m_obEvadeAnim = *pobAttackAnimName;
		obEvadeDefinition.m_obDirection = obEvadeDirection;
		obEvadeDefinition.m_bDoTargetedEvade = false;
		obEvadeDefinition.m_fMaximumRotationSpeed = 0.0f;
		obEvadeDefinition.m_fMaximumTargetingRadius = 0.0f;

		// Push the controller on to the movement component
		m_pobParentEntity->GetMovement()->BringInNewController( obEvadeDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fAttackBlendInTime );
	}

	// ...these are scaled
	else 
	{
		// Define our strike phase
		ScaledTargetedTransitionDef obAttackDefinition;
		obAttackDefinition.SetDebugNames( "Attack", "ScaledTargetedTransitionDef" );

		// Set up the parameters
		obAttackDefinition.m_bApplyGravity = ( obStrike.GetAttackDataP()->m_eAttackMovementType == AMT_GROUND_TO_GROUND ) && !obStrike.GetAttackDataP()->m_bTurnOffGravity;
		obAttackDefinition.m_fMaxRange = obStrike.GetAttackRange();
		obAttackDefinition.m_fMovementOffset = obStrike.GetAttackDataP()->m_fAttackScaledOffset;
		obAttackDefinition.m_obAnimationName = *pobAttackAnimName;
		obAttackDefinition.m_fMovementDuration = fAttackDuration;
		obAttackDefinition.m_bTrackAfterScale = obStrike.GetAttackDataP()->m_bTrackTargetThroughout;
		obAttackDefinition.m_b3DScaling = ( obStrike.GetAttackDataP()->m_eAttackMovementType == AMT_GROUND_TO_AIR ) || obStrike.GetAttackDataP()->m_bUse3DScaling;
		obAttackDefinition.m_bScaleDown = false;
		obAttackDefinition.m_bNoDirectionCorrectionScaling = !obStrike.GetAttackDataP()->m_bCorrectDirectionWhenScaling;
		obAttackDefinition.m_bNoRotateIfTargetBehind = obStrike.GetAttackDataP()->m_bNoRotateIfTargetBehind;
		// Calculate the scale to time
		if ( obStrike.GetAttackDataP()->m_obStrike.GetNumFlippers() > 0)
		{
			const CFlipFlop& obStrikeWindow = obStrike.GetAttackDataP()->m_obStrike;
			obAttackDefinition.m_fScaleToTime = obStrikeWindow.GetFirstValue( fAttackAnimDuration );

			if (obStrikeWindow.GetNumFlippers() > 1)
				obAttackDefinition.m_bTrackAfterScale = true;
		}
		else
		{
			obAttackDefinition.m_fScaleToTime = fAttackAnimDuration;
		}

		// Push the controller on to the movement component
		if (!bNoBlend)
			m_pobParentEntity->GetMovement()->BringInNewController( obAttackDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fAttackBlendInTime );
		else
		{
			m_pobParentEntity->GetMovement()->ClearControllers();
			m_pobParentEntity->GetMovement()->BringInNewController( obAttackDefinition, CMovement::DMM_STANDARD, 0.0f );
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartAttackRecoverMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartAttackRecoverMovement( const CStrike& obStrike, const AerialDetailsDef* pobAerialDetails, bool bStrikeLanded )
{
	// Create our movement definition
	StandardTargetedTransitionDef obRecoveryDefinition;
	obRecoveryDefinition.SetDebugNames( "Attack Recovery", "StandardTargetedTransitionDef" );

	// Do we want to apply gravity?
	bool bApplyGravity = !obStrike.GetAttackDataP()->m_bTurnOffGravity;

	// Set up the parameters
	obRecoveryDefinition.m_bApplyGravity = bApplyGravity;
	obRecoveryDefinition.m_bReversed = false;
	obRecoveryDefinition.m_bTrackTarget = false;
	obRecoveryDefinition.m_fMaximumRotationSpeed = 0.0f;
	obRecoveryDefinition.m_fMaximumTargetingRadius = 0.1f;
	obRecoveryDefinition.m_fMovementDuration = obStrike.GetAttackRecoverTime();
	if (!bStrikeLanded && !obStrike.GetAttackDataP()->m_obStrikeFailedRecoveryAnimName.IsNull())
	{
		Message obAttackMessage(msg_combat_startstrikefailedrecovery);
		m_pobParentEntity->GetMessageHandler()->QueueMessage(obAttackMessage);
		obRecoveryDefinition.m_obAnimationName = obStrike.GetAttackDataP()->m_obStrikeFailedRecoveryAnimName;
	}
	else
	{
		Message obAttackMessage(msg_combat_startrecovery);
		m_pobParentEntity->GetMessageHandler()->QueueMessage(obAttackMessage);
		obRecoveryDefinition.m_obAnimationName = obStrike.GetAttackDataP()->m_obRecoveryAnimName;
	}
	obRecoveryDefinition.m_bLooping = false;

	// If we are recovering from up in the air then we need to stop when we hit the ground - otherwise we'll skid
	if ( ( obStrike.GetAttackDataP()->m_eAttackMovementType == AMT_AIR_TO_AIR )
		 ||
		 ( obStrike.GetAttackDataP()->m_eAttackMovementType == AMT_GROUND_TO_AIR ) )
	{
		obRecoveryDefinition.m_bStopWhenOnGround = true;
	}

	float fBlend = m_pobBlendInTimes->m_fAttackRecoverBlendInTime;
	if (obStrike.ShouldSync())
		fBlend = 0.0f;

	// Push the controller on to the movement component
	m_pobParentEntity->GetMovement()->BringInNewController( obRecoveryDefinition, CMovement::DMM_STANDARD, fBlend );

	// If this is an attack from which we want to drop to the floor afterwards - add that movement
	if ( ( obStrike.GetAttackDataP()->m_eAttackMovementType == AMT_AIR_TO_AIR )
		 ||
		 ( obStrike.GetAttackDataP()->m_eAttackMovementType == AMT_GROUND_TO_AIR ) )
	{
		// Make sure we have all the data that we need
		if ( !pobAerialDetails )
		{
			user_warn_p( 0, ( "The character does not have the data required to perform aerial moves" ) );
			return;
		}

		// Build the definition for dropping
		ContinuationTransitionDef obDropDefinitionDef;
		obDropDefinitionDef.SetDebugNames( "Aerial Drop Loop", "ContinuationTransitionDef" );
		obDropDefinitionDef.m_obAnimationName = pobAerialDetails->m_obAerialWinDrop;
		obDropDefinitionDef.m_bHorizontalVelocity = true;
		obDropDefinitionDef.m_bVerticalVelocity = true;
		obDropDefinitionDef.m_bVerticalAcceleration = !true;
		obDropDefinitionDef.m_bLooping = true;
		obDropDefinitionDef.m_bEndOnGround = true;
		obDropDefinitionDef.m_bApplyGravity = true;

		// Push the controller on to the movement component
		m_pobParentEntity->GetMovement()->AddChainedController( obDropDefinitionDef, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fAttackRecoverBlendInTime );

		// Build the definition for landing
		StandardTargetedTransitionDef obLandDefinitionDef;
		obLandDefinitionDef.SetDebugNames( "Aerial Land", "StandardTargetedTransitionDef" );
		obLandDefinitionDef.m_bApplyGravity = true;
		obLandDefinitionDef.m_bLooping = false;
		obLandDefinitionDef.m_bReversed = false;
		obLandDefinitionDef.m_bTrackTarget = false;
		obLandDefinitionDef.m_obAnimationName = pobAerialDetails->m_obAerialWinLand;

		// Push the controller on to the movement component
		m_pobParentEntity->GetMovement()->AddChainedController( obLandDefinitionDef, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fAttackRecoverBlendInTime );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartSwitchMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartSwitchMovement( const CHashedString& obAnimName )
{
	// Create our movement definition
	StandardTargetedTransitionDef obSwitchDefinition;
	obSwitchDefinition.SetDebugNames( "Stance Switch", "StandardTargetedTransitionDef" );

	// Set up the parameters
	obSwitchDefinition.m_bApplyGravity = true;
	obSwitchDefinition.m_bReversed = false;
	obSwitchDefinition.m_bTrackTarget = false;
	obSwitchDefinition.m_fMaximumRotationSpeed = 0.0f;
	obSwitchDefinition.m_fMaximumTargetingRadius = 0.1f;
	obSwitchDefinition.m_obAnimationName = obAnimName;
	obSwitchDefinition.m_bLooping = false;

	// Push the controller on to the movement component
	m_pobParentEntity->GetMovement()->BringInNewController( obSwitchDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fStanceSwitchBlendInTime );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartRecoilMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartRecoilMovement( const CHashedString& obAnimName, REACTION_ZONE eReactionZone, float fTimeScalar, bool bYawRotation )
{
	// Create our movement definition
	BlockTargetedTransitionDef obRecoilDefinition;
	obRecoilDefinition.SetDebugNames( "Recoil", "BlockTargetedTransitionDef" );

	// Set up the parameters
	obRecoilDefinition.m_bApplyGravity = true;
	obRecoilDefinition.m_bStayAligned = false;
	obRecoilDefinition.m_fAngleToTargetTime = 0.0f;
	obRecoilDefinition.m_fMovementOffset = 0.0f;
	obRecoilDefinition.m_obAnimationName = obAnimName;
	obRecoilDefinition.m_fMovementDuration = fTimeScalar * m_pobParentEntity->FindAnimHeader( obRecoilDefinition.m_obAnimationName, false )->GetDuration();

	if(bYawRotation)
	{
		switch ( eReactionZone )
		{
		case RZ_FRONT:	obRecoilDefinition.m_fAngleToTarget = 0.0f;	break;
		case RZ_BACK:	obRecoilDefinition.m_fAngleToTarget = PI;	break;
		default:		ntAssert( 0 );								break;
		}

		obRecoilDefinition.m_bTurnToFaceTarget = true;
	}
	else
	{
		obRecoilDefinition.m_fAngleToTarget = 0.0f;
		obRecoilDefinition.m_bTurnToFaceTarget = false;
	}

	// Push the controller on to the movement component - no blend, recoils snap
	m_pobParentEntity->GetMovement()->BringInNewController( obRecoilDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fRecoilBlendInTime );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartStaggerMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartStaggerMovement( const CHashedString& obAnimName, float fTimeScalar )
{
	// Create our movement definition
	BlockTargetedTransitionDef obStaggerDefinition;
	obStaggerDefinition.SetDebugNames( "Stagger", "BlockTargetedTransitionDef" );

	// Set up the parameters
	obStaggerDefinition.m_bApplyGravity = true;
	obStaggerDefinition.m_bStayAligned = false;
	obStaggerDefinition.m_fAngleToTarget = 0.0f;
	obStaggerDefinition.m_fAngleToTargetTime = 0.0f;
	obStaggerDefinition.m_obAnimationName = obAnimName;
	obStaggerDefinition.m_fMovementDuration = fTimeScalar * m_pobParentEntity->FindAnimHeader( obStaggerDefinition.m_obAnimationName, false )->GetDuration();

	// Check some of our data
	ntAssert( obStaggerDefinition.m_fMovementDuration > 0.0f );

	// Push the controller on to the movement component - no blend
	m_pobParentEntity->GetMovement()->BringInNewController( obStaggerDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fStaggerBlendInTime );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartStaggerRecoverMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartStaggerRecoverMovement( const CHashedString& obAnimName )
{
	// Create our movement definition
	StandardTargetedTransitionDef obStaggerRecoverDefinition;
	obStaggerRecoverDefinition.SetDebugNames( "Stagger Recover", "StandardTargetedTransitionDef" );

	// Set up the parameters
	obStaggerRecoverDefinition.m_bApplyGravity = true;
	obStaggerRecoverDefinition.m_bReversed = false;
	obStaggerRecoverDefinition.m_bTrackTarget = false;
	obStaggerRecoverDefinition.m_fMaximumRotationSpeed = 0.0f;
	obStaggerRecoverDefinition.m_fMaximumTargetingRadius = 0.1f;
	obStaggerRecoverDefinition.m_obAnimationName = obAnimName;
	obStaggerRecoverDefinition.m_bLooping = false;

	// Push the controller on to the movement component
	m_pobParentEntity->GetMovement()->BringInNewController( obStaggerRecoverDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fStaggerRecoverBlendInTime );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartKillMovement
*
*	DESCRIPTION		Currently the same as the first section of the KO movement
*
***************************************************************************************************/
void CAttackMovement::StartKillMovement( const CHashedString& obAnimName, REACTION_ZONE eReactionZone, float fTimeScalar, bool bChainOnFall, CAttackKO* pobKO )
{
	// Create our movement definition
	BlockTargetedTransitionDef obHitDefinition;
	obHitDefinition.SetDebugNames( "Killed Anim", "BlockTargetedTransitionDef" );

	// Set up the parameters
	obHitDefinition.m_bApplyGravity = false;
	obHitDefinition.m_bStayAligned = false;
	obHitDefinition.m_fAngleToTargetTime = 0.0f;
	obHitDefinition.m_obAnimationName = obAnimName;
	obHitDefinition.m_fMovementDuration = fTimeScalar * m_pobParentEntity->FindAnimHeader( obHitDefinition.m_obAnimationName, false )->GetDuration();

	// Check some of our data
	ntAssert( obHitDefinition.m_fMovementDuration > 0.0f );

	// The angle to target depends on the reaction zone
	switch ( eReactionZone )
	{
	case RZ_FRONT:	obHitDefinition.m_fAngleToTarget = 0.0f;	break;
	case RZ_BACK:	obHitDefinition.m_fAngleToTarget = PI;		break;
	default:		ntAssert( 0 );								break;
	}

	// Push the controller on to the movement component - no blend
	m_pobParentEntity->GetMovement()->BringInNewController( obHitDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKillBlendInTime );

	if (bChainOnFall)
	{
		// Create our drop section definition
		ContinuationTransitionDef obDropDefinition;
		obDropDefinition.SetDebugNames( "Kill Fall", "ContinuationTransitionDef" );

		// Set up the parameters
		obDropDefinition.m_obAnimationName = pobKO->m_obFallAnimName;
		obDropDefinition.m_bHorizontalVelocity = true;
		obDropDefinition.m_bHorizontalAcceleration = false;
		obDropDefinition.m_bVerticalAcceleration = true;
		obDropDefinition.m_bVerticalVelocity = true;
		obDropDefinition.m_bLooping = true;
		obDropDefinition.m_bEndOnGround = true;
		obDropDefinition.m_bApplyGravity = true;

		m_pobParentEntity->GetMovement()->AddChainedController( obDropDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOFallBlendInTime );
			
		// Create our floored section definition
		StandardTargetedTransitionDef obFlooredDefinition;
		obFlooredDefinition.SetDebugNames( "Kill Impact", "StandardTargetedTransitionDef" );

		// Set up the parameters
		obFlooredDefinition.m_bApplyGravity = true;
		obFlooredDefinition.m_bTrackTarget = false;
		obFlooredDefinition.m_obAnimationName = pobKO->m_obFlooredAnimName;
		obFlooredDefinition.m_bLooping = false;

		// Push the controller on to the movement component to chain it
		m_pobParentEntity->GetMovement()->AddChainedController( obFlooredDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOImpactBlendInTime );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartKOAftertouchMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartKOAftertouchMovement(	const CHashedString&	obAnimName, 
													const CAttackKO*	pobKO, 
													REACTION_ZONE		eReactionZone, 
													float				fTimeScalar,
													bool				bUseSpecificAngleToTarget, 
													float				fSpecificAngleToTarget, 
													const CEntity*		pobAttacker,
													bool				bNeedFallAndFlooredImpact )
{
	ntError_p( pobKO, ("Null KO Definition for %s\n", m_pobParentEntity->GetName().c_str()) );

	// Create our movement definition
	KOAftertouchTransitionDef obHitDefinition;
	obHitDefinition.SetDebugNames( "KO Anim", "KOAftertouchTransitionDef" );

	// Set up the parameters
	obHitDefinition.m_obAnimationName = obAnimName;
	obHitDefinition.m_fMovementDuration = fTimeScalar * m_pobParentEntity->FindAnimHeader( obHitDefinition.m_obAnimationName, false )->GetDuration();
	obHitDefinition.m_pobAttacker = pobAttacker;

	// Check some of our data
	ntAssert( obHitDefinition.m_fMovementDuration > 0.0f );

	// The angle to target depends on the reaction zone
	if ( bUseSpecificAngleToTarget )
	{
		obHitDefinition.m_fAngleToTarget = fSpecificAngleToTarget;
	}
	else
	{
		switch ( eReactionZone )
		{
		case RZ_FRONT:	obHitDefinition.m_fAngleToTarget = 0.0;		break;
		case RZ_BACK:	obHitDefinition.m_fAngleToTarget = PI;		break;
		default:		ntAssert( 0 );								break;
		}
	}

	// Push the controller on to the movement component
	m_pobParentEntity->GetMovement()->BringInNewController( obHitDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOBlendInTime );

	// Add on the falling movement
	if ( bNeedFallAndFlooredImpact )
		StartKOFallMovement( pobKO, false );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartKOMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartKOMovement(	const CHashedString&	obAnimName, 
										const CAttackKO*	pobKO, 
										REACTION_ZONE		eReactionZone, 
										float				fTimeScalar,
										bool				bSyncInitialHeight,
										bool				bUseSpecificAngleToTarget,
										float				fSpecificAngleToTarget,
										bool				bNeedFallAndFlooredImpact )
{
	ntError_p( pobKO, ("Null KO Definition for %s\n", m_pobParentEntity->GetName().c_str()) );

	// Create our movement definition
	BlockTargetedTransitionDef obHitDefinition;
	obHitDefinition.SetDebugNames( "KO Anim", "BlockTargetedTransitionDef" );

	// Set up the parameters
	obHitDefinition.m_bApplyGravity = false;
	obHitDefinition.m_bStayAligned = false;
	obHitDefinition.m_fAngleToTargetTime = 0.0f;
	obHitDefinition.m_obAnimationName = obAnimName;
	obHitDefinition.m_fMovementDuration = fTimeScalar * m_pobParentEntity->FindAnimHeader( obHitDefinition.m_obAnimationName, false )->GetDuration();
	obHitDefinition.m_bSyncInitialHeight = bSyncInitialHeight;

	// Check some of our data
	ntAssert( obHitDefinition.m_fMovementDuration > 0.0f );

	// The angle to target depends on the reaction zone
	if (bUseSpecificAngleToTarget)
	{
		obHitDefinition.m_fAngleToTarget = fSpecificAngleToTarget;
	}
	else
	{
		switch ( eReactionZone )
		{
		case RZ_FRONT:	obHitDefinition.m_fAngleToTarget = 0.0;		break;
		case RZ_BACK:	obHitDefinition.m_fAngleToTarget = PI;		break;
		default:		ntAssert( 0 );								break;
		}
	}

	// Push the controller on to the movement component
	m_pobParentEntity->GetMovement()->BringInNewController( obHitDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOBlendInTime );

	// Add on the falling movement
	if ( bNeedFallAndFlooredImpact )
		StartKOFallMovement( pobKO, false );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartKOFallMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartKOFallMovement( const CAttackKO* pobKO, bool bInterruptedSync )
{
	ntError_p( pobKO, ("Null KO Definition for %s\n", m_pobParentEntity->GetName().c_str()) );

	// Create our drop section definition
	ContinuationTransitionDef obDropDefinition;
	obDropDefinition.SetDebugNames( "KO Fall", "ContinuationTransitionDef" );

	// Set up the parameters
	obDropDefinition.m_obAnimationName = pobKO->m_obFallAnimName;
	obDropDefinition.m_bHorizontalVelocity = true;
	obDropDefinition.m_bHorizontalAcceleration = false;
	obDropDefinition.m_bVerticalAcceleration = true;
	obDropDefinition.m_bVerticalVelocity = true;
	obDropDefinition.m_bLooping = true;
	obDropDefinition.m_bEndOnGround = true;
	obDropDefinition.m_bApplyGravity = true;

	// Push the controller on to the movement component to chain it - blend in
	if (bInterruptedSync)
		m_pobParentEntity->GetMovement()->BringInNewController( obDropDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOFallBlendInTime );
	else
		m_pobParentEntity->GetMovement()->AddChainedController( obDropDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOFallBlendInTime );
}

/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartPausedKOFallMovement
*
*	DESCRIPTION		Do a fall but with no vertical movement, used to pause the fall when targetted 
*					for an aerial attack.
*
***************************************************************************************************/
void CAttackMovement::StartPausedKOFallMovement( const CAttackKO* pobKO )
{
	ntError_p( pobKO, ("Null KO Definition for %s\n", m_pobParentEntity->GetName().c_str()) );

	// Create our drop section definition
	ContinuationTransitionDef obDropDefinition;
	obDropDefinition.SetDebugNames( "Paused KO Fall", "ContinuationTransitionDef" );

	// Set up the parameters
	obDropDefinition.m_obAnimationName = pobKO->m_obFallAnimName;
	obDropDefinition.m_bHorizontalVelocity = false;
	obDropDefinition.m_bVerticalVelocity = false;
	obDropDefinition.m_bVerticalAcceleration = false;
	obDropDefinition.m_bHorizontalAcceleration = false;
	obDropDefinition.m_bApplyGravity = false;
	obDropDefinition.m_bLooping = true;
	obDropDefinition.m_bEndOnGround = true;

	// Push the controller on to the movement component to chain it - blend in
	m_pobParentEntity->GetMovement()->BringInNewController( obDropDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOBlendInTime );
}

void CAttackMovement::StartImpactMovement( const CHashedString& obFlooredAnimName, bool bChainIt )
{
	// Create our floored section definition
	StandardTargetedTransitionDef obFlooredDefinition;
	obFlooredDefinition.SetDebugNames( "Floor Impact", "StandardTargetedTransitionDef" );

	// Set up the parameters
	obFlooredDefinition.m_bApplyGravity = true;
	obFlooredDefinition.m_bTrackTarget = false;
	obFlooredDefinition.m_obAnimationName = obFlooredAnimName;
	obFlooredDefinition.m_bLooping = false;

	// Push the controller on to the movement component to chain it
	if (bChainIt)
		m_pobParentEntity->GetMovement()->AddChainedController( obFlooredDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOImpactBlendInTime );
	else
		m_pobParentEntity->GetMovement()->BringInNewController( obFlooredDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOImpactBlendInTime );
}

/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartFlooredMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartFlooredMovement( const CHashedString& obFlooredAnimName, 
										   const CHashedString& obWaitAnimName, 
										   bool bWasFullyRagdolledInKO,
										   bool bNeedFallAndFlooredImpact,
										   bool bChainIt )
{	
	if (!bWasFullyRagdolledInKO)
	{
		if (bNeedFallAndFlooredImpact)
		{
			// Create our floored section definition
			StandardTargetedTransitionDef obFlooredDefinition;
			obFlooredDefinition.SetDebugNames( "Floor Impact", "StandardTargetedTransitionDef" );

			// Set up the parameters
			obFlooredDefinition.m_bApplyGravity = true;
			obFlooredDefinition.m_bTrackTarget = false;
			obFlooredDefinition.m_obAnimationName = obFlooredAnimName;
			obFlooredDefinition.m_bLooping = false;

			// Push the controller on to the movement component to chain it
			if (bChainIt)
				m_pobParentEntity->GetMovement()->AddChainedController( obFlooredDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOImpactBlendInTime );
			else
				m_pobParentEntity->GetMovement()->BringInNewController( obFlooredDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fKOImpactBlendInTime );
		}

		// Create our movement definition for the looping section
		StandardTargetedTransitionDef obWaitDefinition;
		obWaitDefinition.SetDebugNames( "Floor Loop", "StandardTargetedTransitionDef" );

		// Set up the parameters
		obWaitDefinition.m_bApplyGravity = true;
		obWaitDefinition.m_bReversed = false;
		obWaitDefinition.m_bTrackTarget = false;
		obWaitDefinition.m_obAnimationName = obWaitAnimName;
		obWaitDefinition.m_bLooping = true;
	
		// Chain this one so it happens after the impact? Quite a long blend for it
		if (bNeedFallAndFlooredImpact || bChainIt)
			m_pobParentEntity->GetMovement()->AddChainedController( obWaitDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fFlooredBlendInTime );
		else
			m_pobParentEntity->GetMovement()->BringInNewController( obWaitDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fFlooredBlendInTime );
	}
	else
	{
		// Will blend from the ragdoll hierarchy to the anim gradually
		RagdollFlooredTransitionDef obRagdollFlooredTransitionDef;
		obRagdollFlooredTransitionDef.SetDebugNames( "Ragdoll Floor Loop", "RagdollFlooredTransitionDef" );
		obRagdollFlooredTransitionDef.m_obFlooredAnimationName = obWaitAnimName;

		m_pobParentEntity->GetMovement()->BringInNewController( obRagdollFlooredTransitionDef, CMovement::DMM_STANDARD, 0.0f );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartDeflectingMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartDeflectingMovement( const CHashedString& obAnimName, float fTimeScalar )
{
	const CAnimationHeader* pobHeader = m_pobParentEntity->FindAnimHeader( obAnimName, false );

	// Create our movement definition
	BlockTargetedTransitionDef obDeflectDefinition;
	obDeflectDefinition.SetDebugNames( "Deflect", "BlockTargetedTransitionDef" );

	// Set up the parameters
	obDeflectDefinition.m_bApplyGravity = true;
	obDeflectDefinition.m_bStayAligned = false;
	obDeflectDefinition.m_fAngleToTargetTime = 0.0f;
	obDeflectDefinition.m_fAngleToTarget = 0.0f;
	
	// How far do we need to move from the attacks
	obDeflectDefinition.m_fMovementOffset = 0.0f;

	// We need to pick a random animation to from the handed pool
	obDeflectDefinition.m_obAnimationName = obAnimName;

	// Set the movement duration and check its valid
	if (pobHeader)
	{

		obDeflectDefinition.m_fMovementDuration = fTimeScalar * pobHeader->GetDuration();
	}
	else
	{	
		user_warn_p( false, ("Anim %s not found on entity %s.\n", obAnimName.GetDebugString(), m_pobParentEntity->GetName().c_str() ));
		obDeflectDefinition.m_fMovementDuration = 1.0f;
	}

	ntAssert( obDeflectDefinition.m_fMovementDuration > 0.0f );

	// Push the controller on to the movement component - no blend
	m_pobParentEntity->GetMovement()->BringInNewController( obDeflectDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fDeflectBlendInTime );
}

/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartRiseMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartRiseMovement( const CHashedString& obAnimName )
{
	UNUSED( obAnimName );

	// Create our movement definition
	StandardTargetedTransitionDef obKORecoverDefinition;
	obKORecoverDefinition.SetDebugNames( "Rise", "StandardTargetedTransitionDef" );

	// Set up the parameters
	obKORecoverDefinition.m_bApplyGravity = true;
	obKORecoverDefinition.m_bReversed = false;
	obKORecoverDefinition.m_bTrackTarget = false;
	obKORecoverDefinition.m_fMaximumRotationSpeed = 0.0f;
	obKORecoverDefinition.m_fMaximumTargetingRadius = 0.0f;
	obKORecoverDefinition.m_bLooping = false;
	obKORecoverDefinition.m_obAnimationName = obAnimName;

	// Push the controller on to the movement component
	m_pobParentEntity->GetMovement()->BringInNewController( obKORecoverDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fRiseBlendInTime );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartBlockMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartBlockMovement( const CAttackBlock* pobBlock, float fTimeTillStrike )
{
	// Make sure that we have useful information here
	ntAssert_p( pobBlock, ( "The character does not have the correct definition data to perform a block" ) );

	// Create our movement definition
	BlockTargetedTransitionDef obEnterBlockDefinition;
	obEnterBlockDefinition.SetDebugNames( "Start Block", "BlockTargetedTransitionDef" );

	// Set up the parameters
	obEnterBlockDefinition.m_bApplyGravity = true;
	obEnterBlockDefinition.m_bStayAligned = false;
	obEnterBlockDefinition.m_fAngleToTargetTime = fTimeTillStrike;
	obEnterBlockDefinition.m_fAngleToTarget = 0.0f;
	obEnterBlockDefinition.m_fMovementOffset = 0.0f;
	obEnterBlockDefinition.m_obAnimationName = pobBlock->m_obEnterAnimName;

	// Push the controller on to the movement component
	m_pobParentEntity->GetMovement()->BringInNewController( obEnterBlockDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fBlockBlendInTime );

	// Create our movement definition for the looping section
	StandardTargetedTransitionDef obBlockLoopDefinition;
	obBlockLoopDefinition.SetDebugNames( "Block Loop", "StandardTargetedTransitionDef" );

	// Set up the parameters
	obBlockLoopDefinition.m_bApplyGravity = true;
	obBlockLoopDefinition.m_bReversed = false;
	obBlockLoopDefinition.m_bTrackTarget = true;
	obBlockLoopDefinition.m_fMaximumRotationSpeed = 0.0f;
	obBlockLoopDefinition.m_fMaximumTargetingRadius = 0.0f;
	obBlockLoopDefinition.m_fMovementDuration = 0.0f;
	obBlockLoopDefinition.m_obAnimationName = pobBlock->m_obLoopAnimName;
	obBlockLoopDefinition.m_bLooping = true;

	// Push the controller on to the movement component as a chained item
	m_pobParentEntity->GetMovement()->AddChainedController( obBlockLoopDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fBlockBlendInTime );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackMovement::StartBlockEndMovement
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAttackMovement::StartBlockEndMovement( const CAttackBlock* pobBlock )
{
	// Make sure that we have useful information here
	ntAssert_p( pobBlock, ( "The character does not have the correct definition data to perform a block" ) );

	// Create our movement definition
	StandardTargetedTransitionDef obEndBlockDefinition;
	obEndBlockDefinition.SetDebugNames( "End Block", "StandardTargetedTransitionDef" );

	// Set up the parameters
	obEndBlockDefinition.m_bApplyGravity = true;
	obEndBlockDefinition.m_bReversed = false;
	obEndBlockDefinition.m_bTrackTarget = false;
	obEndBlockDefinition.m_fMaximumRotationSpeed = 0.0f;
	obEndBlockDefinition.m_fMaximumTargetingRadius = 0.0f;
	obEndBlockDefinition.m_obAnimationName = pobBlock->m_obExitAnimName;
	obEndBlockDefinition.m_bLooping = false;

	// Push the controller on to the movement component
	m_pobParentEntity->GetMovement()->BringInNewController( obEndBlockDefinition, CMovement::DMM_STANDARD, m_pobBlendInTimes->m_fEndBlockBlendInTime );
}

