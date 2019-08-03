//------------------------------------------------------------------------------------------
//!
//!	\file game/entityplayer.cpp
//!	Definition of the Player entity object
//!
//------------------------------------------------------------------------------------------


#include "game/entityplayer.h"
#include "game/attacks.h"
#include "game/archermovementparams.h"
#include "game/archer3rdpcontroller.h"
#include "game/entitymanager.h"
#include "game/inputcomponent.h"
#include "game/interactiontransitions.h"
#include "game/movement.h"				
#include "game/entityinteractable.h"
#include "game/entityrangedweapon.h"
#include "core/visualdebugger.h"

#include "objectdatabase/dataobject.h"


void ForceLinkFunction24()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction24() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
// Player Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START_INHERITED(Player, Character)
	LUA_EXPOSED_METHOD(SetPrimaryPlayer, SetPrimaryPlayer, "Switch controls to this player", "", "")
LUA_EXPOSED_END(Player)


//------------------------------------------------------------------------------------------
// Player XML Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(Player)
	COPY_INTERFACE_FROM(Character)
	DEFINE_INTERFACE_INHERITANCE(Character)

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)

	OVERRIDE_DEFAULT(ConstructionScript, "Player_Construct")
	OVERRIDE_DEFAULT(DestructionScript, "Player_Destroy")
	OVERRIDE_DEFAULT(Description, "player")
	OVERRIDE_DEFAULT(Important, "true")

	PUBLISH_VAR_WITH_DEFAULT_AS(m_uiPad, 0, Pad)
	OVERRIDE_DEFAULT(SceneElementDef, "ImportantAISceneElement")
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obInitialSystemState, "Player_DefaultState", InitialSystemState)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obWalkingController, "HeroWalkRun", WalkingController)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obSpeedWalkingController, "HeroSpeedWalkRun", SpeedWalkingController)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPowerWalkingController, "HeroPowerWalkRun", PowerWalkingController)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obRangeWalkingController, "HeroRangeWalkRun", RangeWalkingController)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obSpeedStrafeController, "SpeedStrafe", SpeedStrafeController)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPowerStrafeController, "PowerStrafe", PowerStrafeController)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obRangeStrafeController, "RangeStrafe", RangeStrafeController)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bInstantRefill, false, HealthInstantRefill)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDamageCooldownTime, 10.0f, DamageCooldownTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRefillRate, 10.0f, HealthRefillRate)
	
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fTimeFiringDelay, 0.3f, FireDelay )

END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	Player::Player()
//!	Default constructor
//!
//------------------------------------------------------------------------------------------
Player::Player()
:	m_pobInputComponent(0),
	m_uiPad(0),
	m_bArcher(false),
	m_State(ARC_IDLE),
	m_StateTime(0.f),
	m_fAfterTouchStartDelay(0.1f),
	m_Mag1Time(0.f),
	m_CurrentAngle(0.f),
	m_LastAngle(0.f),
	m_CurrentAngleSmoothed(0.f),
	m_LastAngleSmoothed(0.f),
	m_pLockOnTarget(0),
	m_bReadingPadInput(false),
	m_FiredCount(0),
	m_bFireRequest(false),
	m_bReloadRequest(false),
	m_bFireRequestCompleted(false),
	m_bFireRequestQueued(false),
	m_bDisableFireFeedback(false),
	m_fShotCharge(0.f),
	m_dTimeSinceLastShot(0.f),
	m_fTimeFiringDelay(0.3f),
	m_AfterTouchState(false),
	m_bBreakWaitState(false),
	m_CurrentState(0),
	m_CurrentTransition(0),
	m_bInstantRefill(false),
	m_fDamageCooldownTime(10.0f),
	m_fRefillRate(10.0f),
	m_fCurrDamageTime(0.f),
	m_eDeadZoneMode(DZ_STD)
{
	ATTACH_LUA_INTERFACE(Player);

	m_eType					=	EntType_Player;
	m_eCharacterType		=	Character::CT_Player;

	// Set the primary player on the entity manager if there isn't already one specified
	if(!CEntityManager::Get().GetPlayer())
	{
		CEntityManager::Get().SetPrimaryPlayer(this);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Player::~Player()
//!	Default destructor
//!
//------------------------------------------------------------------------------------------
Player::~Player()
{
	NT_DELETE_CHUNK(Mem::MC_ENTITY, m_pobInputComponent );
	m_pobInputComponent = 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Player::OnPostConstruct()
//!	Post construction
//!
//------------------------------------------------------------------------------------------
void Player::OnPostPostConstruct()
{
	// Call the base postpost construction on the entity first
	CEntity::OnPostPostConstruct();

	// By default our input is disabled so that only one primary player is controlled
	// Playable characters on other pads can stay so combat levels continue to work as normal
	ntAssert(m_pobInputComponent);
	if(m_pobInputComponent->GetPadNumber() == PAD_0)
	{
		m_pobInputComponent->SetDisabled(true);

		// If there isn't a primary player already then set us as the primary
		Player* pPrimary = CEntityManager::Get().GetPlayer();

		if(pPrimary == this || pPrimary->GetInputComponent()->GetPadNumber() != PAD_0)
		{
			CEntityManager::Get().SetPrimaryPlayer(0);
			SetPrimaryPlayer();
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Player::SetPrimaryPlayer
//!	Set this as the primary player entity, input works off this entity.
//!
//------------------------------------------------------------------------------------------
void Player::SetPrimaryPlayer()
{
	// Disable the input on the old player
	CEntity* pOldPlayer = CEntityManager::Get().GetPlayer();
	if(pOldPlayer)
		pOldPlayer->GetInputComponent()->SetDisabled(true);

	// Enable the input on this player
	m_pobInputComponent->SetDisabled(false);

	// Set this player as the primary player on the entity manager
	CEntityManager::Get().SetPrimaryPlayer(this);
}

//------------------------------------------------------------------------------------------
//!  public  ThirdPersonAttackTransitionStarted
//!
//!  @param [in]        const ThirdPersonAttackTransition *    
//!
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
void Player::ThirdPersonAttackTransitionStarted( const ThirdPersonAttackTransition* pTransition )
{
	// Has a transition started and there archer isn't even ready for it?
	if( m_State == ARC_IDLE || m_State == ARC_VAULTING || m_State == ARC_TRANSITION )
	{
		m_State			= ARC_TRANSITION;
		m_StateTime		= 0.0f;
		m_Mag1Time		= 0.0f;
		m_CurrentState	= 0;
		m_CurrentTransition = pTransition;

		GetMovement()->SetCompletionCallback( &MovementCompletedCB );
		GetMovement()->SetInterruptCallback( &MovementInterruptedCB );
	}
}

//------------------------------------------------------------------------------------------
//!
//! Movement callbacks
//!
//------------------------------------------------------------------------------------------

void Player::MovementCompletedCB( CEntity* pEntity )		{  pEntity->ToPlayer()->MovementCompleted(); }
void Player::MovementInterruptedCB( CEntity* pEntity )		{  pEntity->ToPlayer()->MovementInterrupted(); }

//------------------------------------------------------------------------------------------
//!  private  MovementCompleted
//!
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
void Player::MovementCompleted( void )
{
	if( m_State == ARC_TRANSITION )
	{
		// Set the current state of the controller to the 
		m_State				= ARC_AIMING;
		m_StateTime			= 0.0f;
		m_CurrentState		= m_CurrentTransition->m_TransitionTo;
	}
}

//------------------------------------------------------------------------------------------
//!  private  MovementInterrupted
//!
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
void Player::MovementInterrupted( void )
{
	if( m_State == ARC_TRANSITION )
	{
		m_State				= ARC_IDLE;
		m_StateTime			= 0.0f;
		m_CurrentState		= 0;
	}
}

//------------------------------------------------------------------------------------------
//!  private  ProcessTransition
//!
//!  @param [in]       pState const ThirdPersonAttackState *    
//!
//!  @return bool 
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
bool Player::ProcessTransition( const ThirdPersonAttackState* pState )
{
	// If there isn't a valid, then return now.
	if( !pState || AfterTouchState() || RequestReload() )
		return false;

	if( pState->m_Targeting )
	{
		CMatrix matDirection = CMatrix( CDirection(0.0f, 1.0f, 0.0f), m_CurrentAngle * DEG_TO_RAD_VALUE ) * GetMatrix();

		// Check if the current target is valid. 
		if( m_pLockOnTarget && !pState->m_Targeting->IsActiveTargetValid( matDirection, m_pLockOnTarget ) )
		{
			m_pLockOnTarget = 0;
		}

		// If there isn't a current target then...
		if( !m_pLockOnTarget )
		{
			// Ask the targeting system for another target. 
			ntstd::List<CEntity*> obEntList;
			pState->m_Targeting->GetTargetList( matDirection, obEntList );

			float		fBestDistance = FLT_MAX;
			CEntity*	pBestEntity	= 0;

			// Cycle through the list of entities to find the closest
			for( ntstd::List<CEntity*>::iterator obIt = obEntList.begin(); obIt != obEntList.end(); ++obIt )
			{
				CEntity*	pNewTarget	= *obIt;
				float		fTestDist	= (pNewTarget->GetPosition() - GetPosition()).LengthSquared();

				if( fTestDist < fBestDistance )
				{
					fBestDistance	= fTestDist;
					pBestEntity		= pNewTarget;
				}
			}

			// Get the first entity from the list as the new active target. 
			if( pBestEntity )
			{
				m_pLockOnTarget = pBestEntity;
			}
		}

#ifdef ARCHER_DEBUG_RENDER
		// Show the lock on target. 
		if( m_pLockOnTarget )
		{
			g_VisualDebug->RenderSphere( CQuat(0.0f, 0.0f, 0.0f, 1.0f), m_pLockOnTarget->GetPosition(), 0.3f, DC_RED );
		}
#endif // ARCHER_DEBUG_RENDER

	}

	CInputComponent* pInput = GetInputComponent();
	ntAssert( pInput );

	// Obtain the angle in 'Easy to Think About'(tm) angles. 
	CPoint ptPlayer = GetPosition();

	// debug print
#ifdef ARCHER_DEBUG_RENDER
	g_VisualDebug->Printf3D( ptPlayer + pInput->GetInputDir(), 0.0f, 0.25f, DC_GREEN, 0, "%f", m_CurrentAngle );
	float fRadius = 1.0f;
#endif // ARCHER_DEBUG_RENDER

	// Iterate over all the of the transitions. 
	for( ntstd::List<ThirdPersonAttackTransition*>::const_iterator obIt( pState->m_Transitions.begin() );
			obIt != pState->m_Transitions.end();
				++obIt )
	{
		// Get the data we're interested in. 
		ThirdPersonAttackTransition* pTrans = *obIt;


		//// DEBUG render Code. 
#ifdef ARCHER_DEBUG_RENDER
		// Create a matrix on which to render the arc
		CMatrix matTemp = CMatrix( CDirection(0.0f,1.0f,0.0f), pTrans->m_HorizontalAngleOffset * DEG_TO_RAD_VALUE ) * GetMatrix();
		matTemp.SetTranslation( matTemp.GetTranslation() + CPoint(0.0f,0.1f,0.0f ) );

		// Render the arc
		g_VisualDebug->RenderArc(	CMatrix( CDirection(0.0f,1.0f,0.0f), pTrans->m_HorizontalAngleOffset * DEG_TO_RAD_VALUE ) * GetMatrix(), 
									fRadius, 
									2.0f * pTrans->m_HorizontalAngleRange * DEG_TO_RAD_VALUE, 
									DC_RED);

		// Move the radius out a little for each pass of the render
		fRadius += 0.05f;
#endif // ARCHER_DEBUG_RENDER

		// Don't process this transition if the state hasn't run for long enough
		if( pTrans->m_StateUpTimeCheck > 0.0f && pTrans->m_StateUpTimeCheck > m_StateTime )
			continue;

		// Only process transitions that have a required directional speed.
		if( pInput->GetInputSpeed() >= pTrans->m_InputMagnitude )
		{
			float fPhaseShift1	= m_CurrentAngleSmoothed + 180.0f;
			float fPhaseShift2	= pTrans->m_HorizontalAngleOffset + 180.0f;
			float fAngleDiff1	= fabs(fPhaseShift1 - fPhaseShift2);
			float fAngleDiff2	= 360.0f - fAngleDiff1;

			if( fAngleDiff1 < pTrans->m_HorizontalAngleRange || fAngleDiff2 < pTrans->m_HorizontalAngleRange )
			{
				// Reset the start time
				m_StateTime = 0.0f;

				// If there isn't a transition anim the transition striaght into another aim pose. 
				if( !pTrans->m_TransitionAnim.length() )
				{
					m_CurrentTransition = pTrans;
					m_CurrentState = pTrans->m_TransitionTo;
				}
				else
				{	
					m_State = ARC_TRANSITION;

					ntAssert( pTrans->m_TransitionTo );

					ThirdPersonAimControllerTransDef obDef;
					obDef.m_pTransition = pTrans;
					m_CurrentTransition = pTrans;

					// Push the controller onto our movement component
					GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, pTrans->m_BlendInTime );
					GetMovement()->SetCompletionCallback( &MovementCompletedCB );
					GetMovement()->SetInterruptCallback( &MovementInterruptedCB );

					/*
					if( GetRangedWeapon() )
					{
						SimpleTransitionDef obDef;
						
						obDef.m_bApplyGravity			= true;
						obDef.m_bLooping				= false;
						obDef.m_obAnimationName			= ;
						obDef.m_fSpeed					= 1.0f;
						obDef.m_fTimeOffsetPercentage	= 0.0f;
					}
					*/

				}

				return true;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!  public  UpdateAiming
//!
//!  @param [in]        float    
//!
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
void Player::UpdateAiming( float fTimeStep )
{
	// Clear out some member varaibles that are updated each frame
	m_LastAngle = m_CurrentAngle;
	m_LastAngleSmoothed = m_CurrentAngleSmoothed;

	// Cache a pointer to the input component
	CInputComponent* pInput = GetInputComponent();

	// Obtain the test angle for the transitions
	CPoint ptPlayer = GetPosition();
	CDirection dirPlayerFacing = GetMatrix().GetZAxis();

	// Decide if the pad should be read. 
	m_bReadingPadInput = m_bReadingPadInput ? pInput->GetInputSpeed() > 0.7f : pInput->GetInputSpeed() > 0.4f;

	// If the pad input is to be read from, then process the current pad input and apply it a archer relative angle
	if( m_bReadingPadInput )
	{
		CDirection dirHeading = GetMatrix().GetZAxis();
		CDirection dirTo = pInput->GetInputDir();
		
		m_CurrentAngle = (atan2f(dirTo.X(), dirTo.Z()) - atan2f(dirHeading.X(), dirHeading.Z())) * RAD_TO_DEG_VALUE;

		if( m_CurrentAngle > 180.0f ) 
			m_CurrentAngle = -180.0f + (m_CurrentAngle - 180.0f);

		if( m_CurrentAngle < -180.0f ) 
			m_CurrentAngle = 180.0f - (-m_CurrentAngle - 180.0f);
	}

	// Get the current input directed angle and save it to the lock on angle, this will allow
	// a target lock angle (if available) to be computed. 
	float fLockOnAngle = m_CurrentAngle;

	// Compute the lock on target angle
	if( m_pLockOnTarget )
	{
		CDirection dirHeading = CDirection(m_pLockOnTarget->GetPosition() - ptPlayer);
		fLockOnAngle = (atan2f(dirHeading.X(), dirHeading.Z()) - atan2f(dirPlayerFacing.X(), dirPlayerFacing.Z())) * RAD_TO_DEG_VALUE;
	}


	// Is there a lock on target threshold? This will lock the archer to a target within a angle threshold given
	float fLockOnThreshold = m_CurrentState ? m_CurrentState->m_LockOnThreshold : -1.0f;

	if( fabs(fLockOnAngle - m_CurrentAngle) < fLockOnThreshold )
	{
		m_CurrentAngle = fLockOnAngle;
	}


	// Create a speed limit if a state isn't currently defined. 
	float fSpeedLimit = m_CurrentState ? m_CurrentState->m_RotationSpeedLimit : 500.0f;

	// Clip the speed to an ammount specified in data
	float fAngleSpeedLimit = fSpeedLimit * fTimeStep;
	float fAngleDiff = fabsf( m_CurrentAngle - m_LastAngleSmoothed );
	
	// only apply smoothing if needed. 
	if( fAngleDiff > fAngleSpeedLimit )
	{
		// Only apply the smoothing if the entity is in a good state
		if( m_State == ARC_AIMING && !AfterTouchState() )
		{
			if( m_LastAngleSmoothed < m_CurrentAngle )
			{
				m_CurrentAngleSmoothed = m_LastAngleSmoothed + fAngleSpeedLimit;
			}
			else
			{
				m_CurrentAngleSmoothed = m_LastAngleSmoothed - fAngleSpeedLimit;
			}
		}
	}
	else
	{
		m_CurrentAngleSmoothed = m_CurrentAngle;
	}

	// For a given state, call the appropriate update function
	switch( m_State )
	{
		case ARC_VAULTING:		// Drop through...
		case ARC_IDLE:			UpdateIdle( fTimeStep );		break;
		case ARC_TRANSITION:	UpdateTransition( fTimeStep );	break;
		case ARC_AIMING:		// Drop through...
		case ARC_FIRING:		UpdateAimingAndFiring( fTimeStep );		break;
		case ARC_1ST_AIMING:	// Drop through...
		case ARC_1ST_FIRING:	Update1stFiring( fTimeStep );			break;
	}
}

/***************************************************************************************************
*
*	FUNCTION		Player::UpdateHealth
*
*	DESCRIPTION		Per frame update for refilling health system 
*
***************************************************************************************************/
void Player::UpdateHealth( float fTimeStep )
{
	// Health - per frame updates
	if ( m_fCurrDamageTime > 0.0f )
	{
		m_fCurrDamageTime -= fTimeStep;
	}
	else
	{
		// Am I on reduced health, but not dead
		if ( ( m_fCurrHealth < m_fStartHealth ) && ( ! IsDead() ) )
		{
			if ( m_bInstantRefill )
			{
				// Instanly refill health - renderable will deal with blending
				ChangeHealth( m_fStartHealth - m_fCurrHealth, "Health Recharge" );
			}
			else
			{
				float fHealthDelta = m_fRefillRate * fTimeStep;
				ChangeHealth( fHealthDelta, "Health Recharge" );
			}
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		Player::ChangeHealth
*
*	DESCRIPTION		
*
***************************************************************************************************/
void Player::ChangeHealth( float fDelta, const char* pcReason )
{ 
	// Super hero mode?
	if( m_bIsInvulnerable )
		return;

	UNUSED( pcReason );

	m_fLastHealth = m_fCurrHealth; 
	m_fCurrHealth += fDelta; 

	// Set off/reset the damage time cooldown
	if ( fDelta < 0.0f )
		m_fCurrDamageTime = m_fDamageCooldownTime;
	
	if ( m_fCurrHealth > m_fStartHealth )
		m_fCurrHealth = m_fStartHealth;

#ifndef _RELEASE
	DebugUpdateHealthHistory( fDelta, pcReason );
#endif // _RELEASE
}

//------------------------------------------------------------------------------------------
//!  private  UpdateIdle
//!
//!  @param [in]        float    
//!
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
void Player::UpdateIdle(float)
{
	// Clear the lock on target. 
	m_pLockOnTarget = 0;

	// Clear the current angle whilst in the idle state
	m_CurrentAngle = 0.0f;

}



//------------------------------------------------------------------------------------------
//!  private  UpdateAimingAndFiring
//!
//!  @param [in]        float    
//!
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
void Player::UpdateAimingAndFiring(float fTimeStep)
{
	// If for any reason the entity enters a combat state - then just return and place the entity
	// back into an idle state
	if( !GetAttackComponent()->AI_Access_IsInCSStandard() )
	{
		ResetToIdleState();
		return;
	}

	// If there is a current transition being played, then bring in the new controller for the aiming. 
	if( m_CurrentTransition )
	{
		ThirdPersonAimControllerDef obDef;
		obDef.m_pCurrentState = m_CurrentState;

		// Push the controller onto our movement component
		GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.15f );
		GetMovement()->SetCompletionCallback( &MovementCompletedCB );
		GetMovement()->SetInterruptCallback( &MovementInterruptedCB );

		m_CurrentTransition = 0;
}

	// Sanity Check
	ntAssert( m_CurrentState );

	// If the state timer has run it course, then exit out now. 
	if( (m_StateTime > m_CurrentState->m_PopoutTime || m_Mag1Time >= m_CurrentState->m_PopoutTimeMag1) && m_State != ARC_FIRING )
	{
		ThirdPersonAttackTransition* pTrans = m_CurrentState->m_PopoutTransition;
		m_StateTime = 0.0f;
		m_Mag1Time	= 0.0f;


		if( !pTrans )
		{
			// 
			if( !m_CurrentState->m_PopoutAnim.IsNull() )
			{
				// Define our movement0
				FacingTransitionDef obDef;

				obDef.m_bApplyGravity		= true;
				obDef.m_fAngularSpeed		= 0.0f; // Degrees rotation per second
				obDef.m_fEarlyOut			= 0.2f; // Allows a transition to opt out slightly earlier (for blending)
				obDef.m_obAnimationName		= m_CurrentState->m_PopoutAnim;
				obDef.m_fAnimSpeed			= 2.0f;
				obDef.m_fStartTurnControl	= 0.0f; // Time offset in the animation when they player can control the character
				obDef.m_fEndTurnControl		= 0.0f; // The last time offset in the animation when the player can control the character
				obDef.m_fAlignToThrowTarget = 0.0f;

				// Push the controller onto our movement component
				GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.15f );
				GetMovement()->SetCompletionMessage( "msg_combat_finished" );
				GetMovement()->SetInterruptMessage( "msg_combat_finished" );
			}
			else
			{
				CMessageSender::SendEmptyMessage( "msg_combat_finished", GetMessageHandler() );
			}

			m_State = ARC_IDLE;
			return;
		}

		m_State = ARC_TRANSITION;
		ThirdPersonAimControllerTransDef obDef;
		obDef.m_pTransition = pTrans;
		m_CurrentTransition = pTrans;

		// Push the controller onto our movement component
		GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, pTrans->m_BlendInTime );
		GetMovement()->SetCompletionCallback( &MovementCompletedCB );
		GetMovement()->SetInterruptCallback( &MovementInterruptedCB );

		return;
	}

	// Cache a pointer to the input
	CInputComponent* pInput = GetInputComponent();

	// local copy of the players position.
	CPoint ptPlayer = GetPosition();

	// Has a there been an external fire request?
	bool bStartFiring = m_bReloadRequest || (ARC_FIRING == m_State) ? false : m_bFireRequest | m_bFireRequestQueued;

	// If there is a reload request but the weapon hasn't fired anything - then request to fire
	if( m_bReloadRequest && !bStartFiring && !m_FiredCount)
	{
		bStartFiring = true;
	}
	
	// Clear out the fire request
	m_bFireRequestQueued = false;

	// Word needs to be sent down to the archer 3rd person controller, but this easier said than
	// done. Flag the fire request inside the component, this will then get picked up on the next
	// movement controller update. 
	m_bFireRequest = bStartFiring;

	// 
	if( m_bFireRequest )
	{
		// As the request has been isssued, mark the completed flag as false
		m_bFireRequestCompleted = false;

		// Put the archer component into a firing state. 
		m_State			= ARC_FIRING;
		m_StateTime		= 0.0f;
		m_Mag1Time		= 0.0f;
	}

	// Has the state time elasped?
	if( ARC_FIRING == m_State && m_bFireRequestCompleted )
	{
		m_State		= ARC_AIMING;
		m_StateTime	= 0.0f;
		m_Mag1Time	= 0.0f;
	}

	// Process the transitions of the current state.
	if( m_State == ARC_AIMING )
		ProcessTransition( m_CurrentState );

	// Whilst reloading, pause the timestep
	if( m_bReloadRequest )
		fTimeStep = 0.0f;

	// Update the time in the state.. 
	m_StateTime += fTimeStep;

	// Update the magnitude 1 time
	if( pInput->GetInputSpeed() > 0.9f )
	{
		m_Mag1Time += fTimeStep;
	}
	else
	{
		m_Mag1Time = 0.0f;
	}

}



//------------------------------------------------------------------------------------------
//!  private  UpdateTransition
//!
//!  @param [in]        float    
//!
//!
//!  @author GavB @date 01/06/2006
//------------------------------------------------------------------------------------------
void Player::UpdateTransition(float)
{
	CInputComponent* pInput = GetInputComponent();
	ntAssert( pInput );

	// 
	if( pInput->GetVPressed() & ((1<<AB_ATTACK_FAST)|(1<<AB_ATTACK_MEDIUM)) && m_State != ARC_FIRING )
	{
		m_bFireRequestCompleted = false;
		m_bFireRequest = true;
	}
}


//------------------------------------------------------------------------------------------
//!  private  Update1stFiring
//!
//!  @param [in]        float    Time step for the frame. 
//!
//!  @author GavB @date 14/07/2006
//------------------------------------------------------------------------------------------

void Player::Update1stFiring( float fTimeStep )
{
	UNUSED( fTimeStep );

	// If for any reason the entity enters a combat state - then just return and place the entity
	// back into an idle state
	if( !GetAttackComponent()->AI_Access_IsInCSStandard() )
	{
		ResetToIdleState();
		return;
	}

	// Cache a pointer to the input component
	CInputComponent* pInput = GetInputComponent();

	// Default the reload count to a big number
	u_int uiReloadCount = 0xFFFFFFFF;

	// Some weapons define the number of shots before a reload - test whether this is one of those weapon types. 
	if( GetInteractionTarget() && 
		(GetInteractionTarget()->GetEntType() & CEntity::EntType_Interactable) && 
		((Interactable*)GetInteractionTarget())->GetInteractableType() & Interactable::EntTypeInteractable_Object_Ranged_Weapon )
	{
		uiReloadCount = ((Object_Ranged_Weapon*)GetInteractionTarget())->GetSharedAttributes()->m_bReloadAfterShot;

		if( uiReloadCount <= 0 )
			uiReloadCount = 6;
	}

	// Is there a firing request
	m_bFireRequest = (pInput->GetVPressed() & ( 1 << AB_ATTACK_MEDIUM ));

	// Cache the current game time as it's used in a couple of places. 
	double dCurrentGameTime = CTimer::Get().GetGameTime();

	// Can the archer fire again. 
	bool bNotAllowedToFireAgain = (float)(dCurrentGameTime - m_dTimeSinceLastShot) < m_fTimeFiringDelay;

	// Has a there been an external fire request?
	bool bStartFiring = m_bReloadRequest || (ARC_FIRING == m_State) || bNotAllowedToFireAgain ? false : (pInput->GetVPressed() & ( 1 << AB_ATTACK_MEDIUM )) ? true : m_bFireRequestQueued;

	// If there is a reload request but the weapon hasn't fired anything - then request to fire
	if( m_bReloadRequest && !bStartFiring && !m_FiredCount)
	{
		m_bReloadRequest = false;
		bStartFiring = true;
	}

	// Only allow a weapon reload when not in a firing state. 
	if( m_bReloadRequest && m_State == ARC_FIRING )
		m_bReloadRequest = false;

	// Clear out the fire request
	m_bFireRequestQueued = false;

	// Save the time the shot was fired (if firing)
	if( bStartFiring )
	{
		// Mark the fire request. 
		m_bFireRequest = bStartFiring;
	}

}

//------------------------------------------------------------------------------------------
//!  public  EnableAimingPadDeadZone
//!
//!  @author GavB @date 29/09/2006
//------------------------------------------------------------------------------------------
void Player::EnableAimingPadDeadZone( void )
{
	m_eDeadZoneMode = CInputHardware::Get().DeadZoneMode();
	CInputHardware::Get().DeadZoneMode( DZ_NONE );
}

//------------------------------------------------------------------------------------------
//!  public  RestoreAimingPadDeadZone
//!
//!  @author GavB @date 29/09/2006
//------------------------------------------------------------------------------------------
void Player::RestoreAimingPadDeadZone( void )
{
	CInputHardware::Get().DeadZoneMode( m_eDeadZoneMode );
}
