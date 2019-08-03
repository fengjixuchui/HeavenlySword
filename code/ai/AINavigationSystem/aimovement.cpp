//! -------------------------------------------
//! AIMovement.cpp
//!
//! Movement AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aimovement.h"
#include "ainavigsystemman.h"
#include "game/aicomponent.h"
#include "ai/aiformationcomponent.h"
#include "ai/aiformation.h"
#include "game/entityplayer.h"
#include "aiworld.h"
#include "game/chatterboxman.h"

//! -------------------------------------------
//! SetMovementParam
//! -------------------------------------------
void CAIMovement::SetMovementParam	( unsigned int uiParam, float fValue)
{ 
	if (fValue < 0.0f) return;

	switch (uiParam)
	{
		// Speed related
		case MP_MAX_SPEED				: m_SMovementParams.m_fMaxSpeed					= (fValue > 1.0f) ? 1.0f : fValue; break;
		case MP_WALK_SPEED				: m_SMovementParams.m_fWalkSpeed				= (fValue > 1.0f) ? 1.0f : fValue; break;
		case MP_CHASE_SPEED				: m_SMovementParams.m_fChaseSpeed				= (fValue > 1.0f) ? 1.0f : fValue; break;
		case MP_APPROACH_SPEED			: m_SMovementParams.m_fApproachSpeed			= (fValue > 1.0f) ? 1.0f : fValue; break;
		case MP_FACE_DIR_SPEED			: m_SMovementParams.m_fFaceDirectionSpeed		= (fValue < 0.3f) ? 0.3f : fValue; break;
		
		case MP_PANIC_DST_SQR			: m_SMovementParams.m_fPanicDistSQR				= (fValue < 0.0f) ? 1.0f : fValue; break;
		
		// AI Avoidance related
		case MP_AI_AVOID_RADIUS			: m_SMovementParams.m_fAIAvoidanceRadius		= (fValue < 0.5f) ? 0.5f : fValue; m_SMovementParams.m_fAIAvoidanceRadiusSQR	= fValue*fValue; break;
		case MP_AI_AVOID_RADIUS_STD		: m_SMovementParams.m_fNormalAIAvoidanceRadius	= (fValue < 0.5f) ? 0.5f : fValue; break;
		case MP_AI_AVOID_RADIUS_COMBAT	: m_SMovementParams.m_fCombatAIAvoidanceRadius	= (fValue < 0.5f) ? 0.5f : fValue; break;
		
		case MP_AI_FLOCKING_RADIUS		: m_SMovementParams.m_fAIFlockRadius			= (fValue < 0.0f) ? 0.0f : fValue; break;
		
		// Attack related
		case MP_ATTACK_RANGE			: m_SMovementParams.m_fAttackRangeSQR			= (fValue < 1.0f) ? 1.0f : fValue*fValue; break;
		case MP_ATTACK_RANGE_SQR		: m_SMovementParams.m_fAttackRangeSQR			= (fValue < 1.0f) ? 1.0f : fValue; break;
		case MP_ATTACK_POINT_RADIUS_SQR	: m_SMovementParams.m_fAttackPointRadiusSQR		= (fValue < 0.25f) ? 0.25f : fValue; break;

		// Obstacle detection relates
		case MP_MIN_WALL_DET_RADIUS		: m_SMovementParams.m_fMinWallDetectionRadius		= (fValue < 0.75f) ? 0.75f : fValue; break;	// To be parametrised realted to the character's radius
		case MP_MIN_DYN_OBST_DET_RADIUS	: m_SMovementParams.m_fMinObstacleDetectionRadius	= (fValue < 0.75f) ? 0.75f : fValue; break;
		case MP_SIDE_DETECTION_LENGTH	: m_SMovementParams.m_fSideDetectionLength			= (fValue < 0.5f)  ? 0.5f : fValue; break;

		// Entity following related
		case MP_FOLLOW_ENTITY_RADIUS	: m_SMovementParams.m_fFollowEntityRadiusSQR		= (fValue < 0.0f) ? 0.0f : fValue*fValue; break;

		// Threshold for go_to_entity
		case MP_GO_TO_ENTITY_RADIUS		: m_SAIIntentions.fDestPointRadiusSQR				= (fValue < 0.0f) ? 0.0f : fValue*fValue; break;

		// Follow Path Cover related
		case MP_FPC_WIDTH_PERCENTAGE	: m_SMovementParams.m_fFollowPathRadiusPercentage	= (fValue > 0.9f) ? 0.9f : ( fValue > 0.0f ? fValue : 0.0f ); break;
		case MP_FPC_MAX_RECOVER_TIME_AFTER_PNF : m_SMovementParams.m_fMaxWaitingTimeAfterPathNotFound = (fValue < 0.0f) ? 0.0f : fValue; break;
		// Cover

		case MP_COVER_TIME					: m_SCoverParams.fCoverTime = (fValue > 0.0f ) ? fValue : 0.0f; break;
		case MP_IN_COVER_PEEK_PROBABILITY	: m_SCoverParams.fPeekProbability = (fValue < 0.0f  || fValue > 1.0f ) ? 1.0f : fValue ; break;

		// Diving
		case MP_DIVE_COOLING_TIME		: m_SAIDiving.fCoolingTime = ( fValue > 0.0f ) ? fValue : 0.0f;
		case MP_DIVE_CONE_LENGTH		: m_SAIDiving.fBoltsConeLength = ( fValue > 1.0f ) ? fValue : 1.0f;
										  m_SAIDiving.fBoltsConeLengthSQR = m_SAIDiving.fBoltsConeLength*m_SAIDiving.fBoltsConeLength; break;
		case MP_DIVE_CONE_HALFRADIUS	: m_SAIDiving.fBoltsConeHalfRadius = ( fValue > 0.0f ) ? fValue : 0.0f; break;
		case MP_DIVE_PROBABILITY		: m_SAIDiving.fDivingProbability = (fValue < 0.0f || fValue > 1.0f) ? 0.0f : fValue ; break;
		case MP_DIVE_PANIC_PROBABILITY	: m_SAIDiving.fPanicProbability = (fValue < 0.0f || fValue > 1.0f) ? 0.0f : fValue ; break;
		case MP_DIVE_AFTERTOUCH_PROBABILITY : m_SAIDiving.fAftertouchDivingProbability = (fValue < 0.0f || fValue > 1.0f) ? 0.0f : fValue ; break;
		case MP_DIVE_AFTERTOUCH_PANIC_PROBABILITY	: m_SAIDiving.fAfterTouchPanicProbability = (fValue < 0.0f || fValue > 1.0f) ? 0.0f : fValue ; break;
            
		default							: ntPrintf("SetMovementParam -> Unknown Parameter : %d\n",uiParam);
	}
}

//! -------------------------------------------
//! SetRangedParam
//! -------------------------------------------
float CAIMovement::GetRangedParameter( unsigned int uiParam )
{
	switch (uiParam)
	{
		// Ranged Attack related

		case RANGED_TIME_BETWEEN_SHOTS	: return GetTimeBetweenShoots(false); break;
		case RANGED_CONSECUTIVE_SHOTS	: return (float)GetNumberOfConsecShoots(false); break;
		case RANGED_MISS_RADIUS			: return GetOffsetRadius(); break;
		case RANGED_SHOOTING_ACCURACY	: return GetShootingAccuracy(false); break;
		case RANGED_MIN_HIDDING_TIME	: return m_SWhackAMoleData.fMinHiddingTime; break;
		case RANGED_MAX_HIDDING_TIME	: return m_SWhackAMoleData.fMaxHiddingTime; break;
		case RANGED_ALWAYS_MISS			: return m_SShootingParams.bAlwaysMiss; break;
		case RANGED_MIN_AIMING_TIME		: return m_SWhackAMoleData.fAimingMinTime; break;
		case RANGED_MAX_AIMING_TIME		: return m_SWhackAMoleData.fAimingMaxTime; break;
		case RANGED_MIN_RANGE_RADIUS	: return fsqrtf(m_SShootingParams.fMinDistanceToEnemySQR); break;
		case RANGED_MAX_RANGE_RADIUS	: return fsqrtf(m_SShootingParams.fMaxDistanceToEnemySQR); break;

		default							: ntPrintf("GetRangedParameter -> Unknown Parameter : %d\n",uiParam);
										  return -1.0f;
	}
}

//! -------------------------------------------
//! SetRangedParam
//! -------------------------------------------
void CAIMovement::SetRangedParam	( unsigned int uiParam, float fValue)
{ 
	if (fValue < 0.0f) return;

	switch (uiParam)
	{
		// Ranged Attack related

		case RANGED_TIME_BETWEEN_SHOTS	: SetTimeBetweenShoots(fValue); break;
		case RANGED_CONSECUTIVE_SHOTS	: SetNumberOfConsecShots( (int)fValue ); break;
		case RANGED_MISS_RADIUS			: SetOffsetRadius(fValue); break;
		case RANGED_SHOOTING_ACCURACY	: SetShootingAccuracy(fValue); break;
		case RANGED_MIN_HIDDING_TIME	: m_SWhackAMoleData.fMinHiddingTime = (fValue > MIN_WHACKAMOLE_HIDDING_TIME) ? fValue : MIN_WHACKAMOLE_HIDDING_TIME; break;
		case RANGED_MAX_HIDDING_TIME	: if (fValue>0) {m_SWhackAMoleData.fMaxHiddingTime = (fValue < MAX_WHACKAMOLE_HIDDING_TIME) ? fValue : MAX_WHACKAMOLE_HIDDING_TIME;} break;
		case RANGED_ALWAYS_MISS			: m_SShootingParams.bAlwaysMiss = (fValue>0.0f) ? true : false; break;
		case RANGED_MIN_AIMING_TIME		: m_SWhackAMoleData.fAimingMinTime = (fValue>0.0f) ? fValue : 0.0f; break;
		case RANGED_MAX_AIMING_TIME		: m_SWhackAMoleData.fAimingMaxTime = (fValue>0.0f) ? fValue : 0.0f; break;

		default							: ntPrintf("SetMovementParam -> Unknown Parameter : %d\n",uiParam);
	}
}

//! -------------------------------------------
//! SetCoverAttitude
//! -------------------------------------------
void CAIMovement::SetCoverAttitude	( unsigned int uiParam )
{ 
	switch (uiParam)
	{
		// Speed related
		case COVER_DEFAULT	: m_SCoverParams.fMultiplier	= 1.0f; break;
		case COVER_NONE		: m_SCoverParams.fMultiplier	= 0.0f; break;
		case COVER_SELDOM	: m_SCoverParams.fMultiplier	= 0.5f; break;
		case COVER_OFTEN	: m_SCoverParams.fMultiplier	= 1.5f; break;
		case COVER_ALWAYS	: m_SCoverParams.fMultiplier	= 10.0f; break;
		default				: ntAssert_p(0,("SetCoverAttitude -> Unknown Parameter passed "));
	}
}

//! -------------------------------------------
//! SetCoverAttitude
//! -------------------------------------------
void CAIMovement::SetIntention	( unsigned int eIntentions, float fSpeed, unsigned int eMovFlag ) 
{ 
	m_SAIIntentions.eNavIntentions = eIntentions;
	SetMaxSpeed(fSpeed);

	if (eMovFlag == NF_DEFAULT)
	{
		switch (eIntentions)
		{
			case NAVINT_FOLLOWPATHCOVER_TO_ENEMY: eMovFlag = CAINavigationSystemMan::NF_DEF_FOLLOW_PATH_COVER; break;
			case NAVINT_FOLLOWPATHCOVER_TO_NODE	: eMovFlag = CAINavigationSystemMan::NF_DEF_FOLLOW_PATH_COVER; break;
			case NAVINT_FOLLOWPATHCOVER_MINMAX	: eMovFlag = CAINavigationSystemMan::NF_DEF_FOLLOW_PATH_DYN_COVER; break;
			case NAVINT_GOTOENTITY				: eMovFlag = CAINavigationSystemMan::NF_DEF_STEER_TO_ENTITY; break;
			case NAVINT_GOTOLOCATORNODE			: eMovFlag = CAINavigationSystemMan::NF_DEF_STEER_TO_LOCATOR_NODE; break;
			case NAVINT_PATROL					: eMovFlag = CAINavigationSystemMan::NF_DEF_PATROL_WALK; break;
			case NAVINT_FOLLOW_ENTITY			: eMovFlag = CAINavigationSystemMan::NF_DEF_FOLLOW_ENEMY; break;
			case NAVINT_WHACKAMOLE				: eMovFlag = NF_NO_MOVEMENT; break;
			case NAVINT_GO_AROUND_VOLUMES		: eMovFlag = NF_GO_AROUND_VOLUMES | NF_S_OBSTACLE | NF_AI_OBSTACLE; break;
			
			//default: user_error_p(0,("AI: [%s] received an unknown Intention: %d",ntStr::GetString(m_pEnt->GetName()),eIntentions));
			//		 return;
		}
	}
	
	SetSteeringFlags(eMovFlag);
}

//! -------------------------------------------
//! SetBoolParam
//! -------------------------------------------
void CAIMovement::SetAIBoolParam	( unsigned int eParam, bool bValue ) 
{ 
	switch (eParam)
	{
		case AIBOOL_IDLE_PLAYS_ANIM	:	m_SAIIntentions.bIdlePlaysAnims = bValue; break;
		default : return;
	}
}

//! -------------------------------------------
//! GetTimeBetweenShoots
//! -------------------------------------------
float CAIMovement::GetTimeBetweenShoots	( bool bRand ) const 
{ 
	if (!bRand) 
		return m_SShootingParams.fMaxTimeBetweenShots; 
	else
	{ 
		float fTime = grandf(m_SShootingParams.fMaxTimeBetweenShots);
		if (fTime > MIN_TIME_BETWEEN_SHOTS)
			return (fTime); 
		else
			return MIN_TIME_BETWEEN_SHOTS;
	}
}

//! -------------------------------------------
//! IsPointWithinMinMaxRange
//! -------------------------------------------
bool CAIMovement::IsPointWithinMinMaxRange	( const CPoint& obPos ) const 
{ 
	const CEntity* pEnemy	= GetEntityToAttack();
	
	if (!pEnemy || !pEnemy->IsPlayer() || !pEnemy->ToPlayer()->IsArcher() )
		return false;

	CPoint obEnemyPos = pEnemy->GetPosition();
	CDirection LineEnemy2CoverPint(obEnemyPos - obPos);

	float fDistEnemy2CoverPintSQR	= LineEnemy2CoverPint.LengthSquared();
	bool bFarThanMinRange		= (fDistEnemy2CoverPintSQR > GetRangeMinDistSQR());
	bool bCloserThanMaxRange	= (fDistEnemy2CoverPintSQR < GetRangeMaxDistSQR());

	return (bFarThanMinRange && bCloserThanMaxRange);
}

//! -------------------------------------------
//! IsPointCloserThanMinRange
//! -------------------------------------------
bool CAIMovement::IsPointCloserThanMinRange	( const CPoint& obPos ) const 
{ 
	const CEntity* pEnemy	= GetEntityToAttack();
	
	if (!pEnemy || !pEnemy->IsPlayer() || !pEnemy->ToPlayer()->IsArcher() )
		return false;

	CPoint obEnemyPos = pEnemy->GetPosition();
	CDirection LineEnemy2CoverPint(obEnemyPos - obPos);

	float fDistEnemy2CoverPintSQR	= LineEnemy2CoverPint.LengthSquared();
	bool bCloserThanMinRange		= (fDistEnemy2CoverPintSQR < GetRangeMinDistSQR());

	return (bCloserThanMinRange);
}

//! -------------------------------------------
//! IsLeader
//! -------------------------------------------
bool CAIMovement::IsLeader	( void )
{ 
	return CAINavigationSystemMan::Get().IsLeader(m_pEnt); 
}

//! -------------------------------------------
//! IsSimpleActionComplete
//! -------------------------------------------
bool CAIMovement::IsSimpleActionComplete ( void )
{ 
	return (m_pCAIComp->IsSimpleActionComplete());
}

//! -------------------------------------------
//! SetDestinationNode
//! -------------------------------------------
bool CAIMovement::SetDestinationNode ( const char* psN )
{
	// Invalid Specific Start Node field
	m_SAIIntentions.bValidStartNode = false;
	m_SAIIntentions.ksDestNodeName = CHashedString();

	// Check if the node exists
	m_SAIIntentions.bValidNode = false;

	if (!psN) return false;

	bool	bFound	= false;
	float	fRadius	= 0;

	// Set the Node's Location
	m_SAIIntentions.obDestPos = CAINavigationSystemMan::Get().GetNavigGraphManager()->GetNodePosWithName(CHashedString(psN),&bFound,&fRadius);
    
	if (bFound)
	{
		// Set Name
		m_SAIIntentions.ksDestNodeName = CHashedString(psN);
		// Set Radius
		SetDestinationRadiusSQR(fRadius);
	}
	m_SAIIntentions.bValidNode = bFound;

	return bFound;
}

//! -------------------------------------------
//! SetDestinationNode
//! -------------------------------------------
bool CAIMovement::SetStartEndNodes ( const char* psStartNode, const char* psEndNode )
{
	// Check if the node exists
	m_SAIIntentions.bValidNode = false;
	m_SAIIntentions.bValidStartNode = false;

	if (!psStartNode || !psEndNode) return false;

	bool	bFoundStart		= false;
	bool	bFoundDest		= false;
	float	fRadiusStart	= 0;
	float	fRadiusEnd		= 0;

	// Set the End Node's Location
	m_SAIIntentions.obDestPos = CAINavigationSystemMan::Get().GetNavigGraphManager()->GetNodePosWithName(CHashedString(psEndNode),&bFoundDest,&fRadiusEnd);
    
	// Set the Start Node's Location
	m_SAIIntentions.obStartPos = CAINavigationSystemMan::Get().GetNavigGraphManager()->GetNodePosWithName(CHashedString(psStartNode),&bFoundStart,&fRadiusStart);
	
	if (bFoundStart && bFoundDest)
	{
		// Set End Name
		m_SAIIntentions.ksDestNodeName = CHashedString(psEndNode);
		// Set End Radius
		SetDestinationRadiusSQR(fRadiusEnd);

		// Set Start Name
		m_SAIIntentions.ksStartNodeName = CHashedString(psStartNode);
		// Set Start Radius
		//	SetDestinationRadiusSQR(fRadiusEnd);
		
		// Flag nodes as valid
		m_SAIIntentions.bValidNode = true;
		m_SAIIntentions.bValidStartNode = true;

		return true;
	}

	return false;
}

//! -------------------------------------------
//! SetDestinationNode
//! -------------------------------------------
bool CAIMovement::SetStartEndNodes ( CAINavigNode* pStartNode, CAINavigNode* pEndNode )
{
	// Check if the node exists
	m_SAIIntentions.bValidNode = false;
	m_SAIIntentions.bValidStartNode = false;

	if (!pStartNode || !pEndNode) return false;

	// Set the Start and End Node's Location
	m_SAIIntentions.obStartPos = pStartNode->GetPos();
	m_SAIIntentions.obDestPos = pEndNode->GetPos();
    
	// Set Nodes Names
	m_SAIIntentions.ksStartNodeName = pStartNode->GetName();
	m_SAIIntentions.ksDestNodeName = pEndNode->GetName();

	SetDestinationRadiusSQR(pEndNode->GetRadiusSQR());
		
	// Flag nodes as valid
	m_SAIIntentions.bValidNode = true;
	m_SAIIntentions.bValidStartNode = true;

	return true;
}

//! -------------------------------------------
//! SetDestination
//! -------------------------------------------
void CAIMovement::SetDestination ( E_DESTINATION eIntention, CAINavigNode* pNode , CEntity* pLeader)
{
	// Set Flags
	switch (eIntention)
	{
		case DEST_IDLE	: SetSteeringFlags(NF_NO_MOVEMENT); break;
	//	case DEST_LEADER: SetSteeringFlags(CAINavigationSystemMan::NF_DEF_FOLLOW_LEADER); break;
		case DEST_NODE	: SetSteeringFlags(CAINavigationSystemMan::NF_DEF_FOLLOW_PATH); break;
		default			: SetSteeringFlags(NF_NO_MOVEMENT); // Unknown Intention!!!
	}
	// Set Intentions
	m_SDestination.Set(eIntention,pNode,pLeader);
}

void CAIMovement::SetDestination ( SDestination* pD )
{
	// Set Flags
	switch (pD->eType)
	{
		case DEST_IDLE	: SetSteeringFlags(NF_NO_MOVEMENT); break;
		//case DEST_LEADER: SetSteeringFlags(CAINavigationSystemMan::NF_DEF_FOLLOW_LEADER); break;
		case DEST_NODE	: SetSteeringFlags(CAINavigationSystemMan::NF_DEF_FOLLOW_PATH); break;
		default			: SetSteeringFlags(NF_NO_MOVEMENT); // Unknown Intention!!!
	}
	// Set Intentions
	m_SDestination.Set(pD);
}

//! -------------------------------------------
//! CancelSingleAnim
//! -------------------------------------------
void CAIMovement::CancelSingleAnim(void)
{
	m_pCAIComp->CancelSingleAnim();
}

//! -------------------------------------------
//! SetClaimCoverPoint
//! -------------------------------------------
void CAIMovement::SetClaimCoverPoint( CAINavigCoverPoint * pCP )
{
	// If AI was claiming another CP, free the other CP
	if (m_SCoverParams.pBookedCoverPoint) 
	{
		m_SCoverParams.pBookedCoverPoint->SetClaimer(NULL);
		m_SCoverParams.pBookedCoverPoint->SetAvailabe(true);
	}
	
	m_SCoverParams.pBookedCoverPoint = pCP; 

	// Inform the new CP that it is booked
	if (pCP) 
	{
		pCP->SetClaimer(m_pEnt);
		pCP->SetAvailabe(false);
	}
}

//! -------------------------------------------
//! Update
//! -------------------------------------------
void CAIMovement::Update( float fTimeChange )
{
	CDirection SteeringAction(CONSTRUCT_CLEAR);
	
	// Update the Diving Timer
	UpdateTimeSinceLastDive(fTimeChange);

	// Check if AI is under external control or the AI is disabled
	if ( ( GetExternalControlState() == true ) || ( m_pCAIComp->GetDisabled() == true ) ) 
            return;

	if ( IsUsingCannon() )
	{
		// Calculate Yaw and Pitch
		CDirection Dir = m_obFacingActionDir;
		float fDistance = Dir.Length();
		Dir.Normalise();
		
		
		float fSin_2Pitch = fDistance * GetCannonBallG()/(GetCannonBallV0_SQR());
		if ( fSin_2Pitch > 1.0f )
			fSin_2Pitch = 1.0f;
		else if ( fSin_2Pitch < -1.0f )
			fSin_2Pitch = -1.0f;
		float fPitch	= -asinf(fSin_2Pitch)/2;

		float fSafeZDir =	Dir.Z() > 0.999f	? 1.0f :
							Dir.Z() < -0.999f	? -1.0f : Dir.Z();

		float fYaw		= (Dir.X() < 0.0f) ? -facosf(fSafeZDir) : facosf(fSafeZDir);

		// Set Movement controller parameters
		m_pCAIComp->SetUseObjectPitch(fPitch);
		m_pCAIComp->SetUseObjectYaw(fYaw);

		m_pCAIComp->SetMovementMagnitude(1.0f);
		m_pCAIComp->SetMovementDirection(m_obFacingActionDir);
		m_pCAIComp->SetMovementFacing(m_obFacingActionDir);

		return;
	}

	// Update movement only if not in facing-action
	if ( m_pCAIComp->IsPlayingFacingAction() )
	{
		m_pCAIComp->SetMovementMagnitude(m_SMovementParams.m_fFaceDirectionSpeed);
		m_pCAIComp->SetMovementDirection(m_obFacingActionDir);
		m_pCAIComp->SetMovementFacing(m_obFacingActionDir);
		return;
	}

	// Update movement only if a SingleAnimation is not active
	if ( m_pCAIComp->IsSimpleActionComplete() )
	{
		if (  GetSteeringFlags() != NF_NO_MOVEMENT )
		{
			SteeringAction = CAINavigationSystemMan::Get().CalculateSteeringAction(this, fTimeChange);				
			if (IsSlowingDown())
			{
				// Spend the showdown time at breakspeed
				m_SpeedMag = GetBreakSpeed();
				DecRemainingSlowDownTime(fTimeChange);
			}
			else
			{
				m_SpeedMag	= SteeringAction.Length();
			}
			m_Speed	= SteeringAction;

			if ( m_SpeedMag > 1.0f ) m_SpeedMag = 1.0f;
			if ( m_SpeedMag > GetMaxSpeed() ) m_SpeedMag = GetMaxSpeed(); //m_SpeedMag*=GetMaxSpeed();
		}
		else
		{
			m_Speed		= SteeringAction;
			m_SpeedMag	= 0;
		}

		m_pCAIComp->SetMovementDirection(SteeringAction);
		m_pCAIComp->SetMovementMagnitude(m_SpeedMag);

		if ( IsMovingWhileFacingTgt() )
		{
			m_pCAIComp->SetMovementFacing(m_obFacingActionDir);
		}
		else
		{
			m_pCAIComp->SetMovementFacing(SteeringAction);
		}

	}
}
//! -------------------------------------------
//! DebugRender
//! -------------------------------------------
void CAIMovement::DebugRender()
{
	m_Path.RenderPath();

}

//! -------------------------------------------
//! PlayAnimation
//! -------------------------------------------
bool CAIMovement::PlayAnimation(int eAnim)
{
	CHashedString sAnim;
	float fAnimSpeedMulti = 1.0f;
	bool bRet = false;
	
	m_pCAIComp->CancelSingleAnim();
	switch (eAnim)
	{
		case ACTION_SPOTTED_FIRST		: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolSpotted")); break;
		case ACTION_SPOTTED_INFORMED	: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolAlert")); break;
		case ACTION_INVESTIGATE_LOOK	: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("InvestigateCheck")); break;
		case ACTION_INVESTIGATE_SHRUG	: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("InvestigateFoundNothing")); break;
		case ACTION_TAUNT				: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("FormationJeer01")); break;
		case ACTION_SCRIPTANIM			: bRet = m_pCAIComp->ActivateSingleAnim(m_pCAIComp->GetScriptAnimName()); break;
		case ACTION_PATROL_SPOTTED		: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolSpotted")); break;
		case ACTION_PATROL_ALERT		: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolAlert")); break;
		case ACTION_PATROL_ALERTED		: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolAlerted")); break;
		case ACTION_WHACK_A_MOLE_CROUCH	: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("crouching_idle_with_crossbow")); break;
		case ACTION_DUCK_LONG_LEFT		: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("KaisEntrance_Shieldman_ArrowDuck")); break;
		case ACTION_IDLE_AIMING_CROSSBOWMAN : bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("crossbowman_aiming_idle")); break;
		case ACTION_PATROL_LOOK			: 
			if (grandf(1.0f) > 0.5f) { bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolLook1"));} else
									{ bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolLook2"));} break;
		case ACTION_PATROL_IDLE			: 
			if (grandf(1.0f) > 0.5f) { bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolLook1"));} else
									{ bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("PatrolLook2"));} break;
		case ACTION_INFORMATION_ANIM	: 
			sAnim = m_pCAIComp->GetFormationIdleAnim();
			fAnimSpeedMulti = m_pCAIComp->GetAIFormationComponent()->GetFormation() ? m_pCAIComp->GetAIFormationComponent()->GetFormation()->GetIdleAnimSpeed() : 1.0f;

			// Set a default formation strafe if one isn't set. 
			if(ntStr::IsNull(sAnim))
				sAnim = CHashedString("formationclose_inc_strafe");

			bRet = m_pCAIComp->ActivateSingleAnim(sAnim, ((grandf(.5f)+.75f)/1.5f) * fAnimSpeedMulti, grandf(0.6f), 0.4f, false);

			if(!bRet)
				return false;

			m_pCAIComp->PlaySingleAnimUntilComplete(true);
			break;

		default: return false;
	}

	return bRet;
}

//! -------------------------------------------
//! PlayDiveAnimation
//! -------------------------------------------
bool CAIMovement::PlayDiveAnimation(int eDiveAction)
{
	bool bRet = false;
	
	m_pCAIComp->CancelSingleAnim();
	switch (eDiveAction)
	{
		case E_DIVE_LONG_LEFT	: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("KaisEntrance_Shieldman_ArrowDive_LongLeft")); break;
		case E_DIVE_LONG_RIGHT	: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("KaisEntrance_Shieldman_ArrowDive_LongRight")); break;
		case E_DIVE_SHORT_LEFT	: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("KaisEntrance_Shieldman_ArrowDive_ShortLeft")); break;
		case E_DIVE_SHORT_RIGHT : bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("KaisEntrance_Shieldman_ArrowDive_ShortRight")); break;
		case E_DIVE_CROUCH		: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("KaisEntrance_Shieldman_ArrowDuck")); break;
		case E_DIVE_PANIC		: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("KaisEntrance_Shieldman_ArrowDive_Scared")); break;
		
		default					: bRet = m_pCAIComp->ActivateSingleAnim(CHashedString("KaisEntrance_Shieldman_ArrowDuck"));
	}

	return bRet;

}

//! -------------------------------------------
//! SetWhackAMoleNode
//! -------------------------------------------
void CAIMovement::SetWhackAMoleNode	( CHashedString hsNode )
{ 
	m_SWhackAMoleData.pNode = CAINavigationSystemMan::Get().GetNodeByName(hsNode); 
}

//! -------------------------------------------
//! SetActionStyle
//! -------------------------------------------
void CAIMovement::SetActionStyle ( unsigned int uiStyle )
{
	// For compatibility with old AI system
	m_pCAIComp->SetActionStyle((CAIState_ACTIONSTYLE)uiStyle);

	m_uiActionStyle = uiStyle;
}

