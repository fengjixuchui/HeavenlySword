//! -------------------------------------------
//! aivision.cpp
//!
//! AI Vision
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------


#include "core/visualdebugger.h"

#include "ai/aivision.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "editable/enums_ai.h"
#include "game/movementcontrollerinterface.h"		// For MovementControllerUtilities::RotationAboutY
#include "game/entitymanager.h"
#include "game/entitybrowser.h"
#include "game/entityinfo.h"						// For IsDead()
#include "game/query.h"


//------------------------------------------------------------------------------------------
//!  public constant  CalculateScore
//!
//!  @param [in]       pRelativeTo const AI *    <TODO: insert parameter description here>
//!
//!  @return float <TODO: insert return value description here>
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 22/08/2006
//------------------------------------------------------------------------------------------
float	CAIFact::CalculateScore	( const AI* pRelativeTo ) const
{
	// Obtain the score weights asigned to this entity
	const AIFactScoreWeights* pScoreWeights = pRelativeTo->GetVisibleScoreWeights();

	// Sanity check null pointer badness
	ntAssert( pScoreWeights );

	// Create a score for the best entity using just the distance to the entity.
	float fScore = (m_obLastKnownPosition - pRelativeTo->GetPosition()).LengthSquared() * pScoreWeights->m_FactDistanceWeight;

	// adjust the game score with the age of the fact
	fScore	*= (m_fAge + 1.0f) * pScoreWeights->m_FactAgeWeight;

	// If the entity is an AI type
	if( m_pEnt->IsAI() )
	{
		// Give a better score to entities that don't have any current attackers
		const CAIComponent* pAIComp = m_pEnt->ToAI()->GetAIComponent();
		u_int uiAttackCount			= pAIComp ? (pAIComp->GetCombatComponent().GetAttackerCount() + 1) * 2 : 1;

		// adjust the score with the number of attacks.
		fScore *= (float)uiAttackCount * pScoreWeights->m_FactEntAttackedWeight;
	}


	// adjust the score with the size of the visibility flags.
	fScore *= m_uiVisibilityFlags & INNER_RANGE		? pScoreWeights->m_InnerRangeWeight					: 1.0f;
	fScore *= m_uiVisibilityFlags & CLOSE_RANGE		? pScoreWeights->m_CloseRangeWeight					: 1.0f;
	fScore *= m_uiVisibilityFlags & LINE_OF_SIGHT	? pScoreWeights->m_LineOfSightRangeWeight			: 1.0f;
	fScore *= m_uiVisibilityFlags & MAIN_RANGE		? pScoreWeights->m_MainRangeWeight					: 1.0f;
	fScore *= m_uiVisibilityFlags & SHOOT_RANGE		? pScoreWeights->m_ShootRangeWeight					: 1.0f;
	fScore *= m_uiVisibilityFlags & OPP_RANGE		? pScoreWeights->m_OtherPersonsProblemRangeWeight	: 1.0f;
	
	return fScore;
}

//! -------------------------------------------
//! CAIVision()
//! -------------------------------------------
CAIVision::CAIVision()
	: m_LastKnownEnemyPos(CONSTRUCT_CLEAR)
	, m_pEnt(NULL)
	, m_pTarget(NULL)
	, m_pCAIComp(NULL)
	, m_fDist2EnemySQR(FAR_FAR_AWAY)
	, m_Active(true)
	, m_bAlerted(false)
	, m_bEverSeenEnemy(false)
	, m_VisionResults(VIS_RESULT_NOTHING_SEEN)
	, m_EnemyType(0)
	, m_VisionUpdateTime(0.0f)
	, m_Facts() 
{
	m_Facts.reserve(FACT_CACHE_LIMIT);
}


//! -------------------------------------------
//! SetVisionParam
//! -------------------------------------------
void CAIVision::SetVisionParam	( unsigned int uiParam, float fValue)
{ 
	if (fValue < 0.0f) return;

	switch (uiParam)
	{
		// View distances
		case VP_CLOSE_DISTANCE_SQR		: m_Params.fCloseDistSQR			= (fValue < 1.0f) ? 1.0f : fValue; break;
		case VP_MID_DISTANCE_SQR		: m_Params.fMaxViewDistSQR			= (fValue < m_Params.fCloseDistSQR) ? m_Params.fCloseDistSQR : fValue; break;
		case VP_MAX_DISTANCE_SQR		: m_Params.fOPPMaxViewDistSQR		= (fValue < m_Params.fMaxViewDistSQR) ? m_Params.fMaxViewDistSQR : fValue; break;
		case VP_INNER_RADIUS_SQR		: m_Params.fInnerRadiusSQR			= (fValue < 1.0f) ? 1.0f : fValue; break;
		case VP_SHOOTING_RADIUS			: SetShootingDistance(fValue); break;
		// View angles
		case VP_HALF_CLOSE_DIST_ANGLE	: m_Params.fHalfCloseViewAngle		= (fValue > PI) ? PI : fValue; break;
		case VP_HALF_MID_DIST_ANGLE		: m_Params.fHalfViewAngle			= (fValue < m_Params.fHalfCloseViewAngle) ? m_Params.fHalfCloseViewAngle : fValue; break;
		case VP_HALF_MAX_DIST_ANGLE		: m_Params.fOPPHalfViewAngle		= (fValue < m_Params.fHalfViewAngle) ? m_Params.fHalfViewAngle : fValue; break;
		case VP_SHOOTING_ANGLE			: SetShootingAngle(DEG_TO_RAD_VALUE*fValue); break;
		// Eyes height
		case VP_EYES_HEIGHT				: m_Params.fVisionYOffset			= (fValue < 0.1f) ? 0.1f : fValue; break;

		// See All
		case VP_SEE_ALL_IN_RANGE		:	if (fValue<1.0f) fValue = 1.0f; else fValue = fValue*fValue;
											m_Params.fInnerRadiusSQR = fValue; m_Params.fMaxViewDistSQR = fValue+1.0f; m_Params.fOPPMaxViewDistSQR = fValue+2.0f; 
											m_Params.fCloseDistSQR = fValue+0.5f;
											m_Params.fHalfCloseViewAngle = PI; m_Params.fHalfViewAngle = PI; m_Params.fOPPHalfViewAngle = PI;
											break;

		default							: ntPrintf("SetVisionParam -> Unknown Parameter : %d\n",uiParam);
	}
}

//!--------------------------------------------
//! IsTargetInAttackRange
//!--------------------------------------------
bool CAIVision::IsTargetInAttackRange( void ) const	
{ 
	if (!m_pTarget) 
		return false;

	if ( m_pTarget->IsPlayer() )
	{
		return ( m_Active && (m_VisionResults==VIS_RESULT_ENEMY_SEEN) && m_pCAIComp->GetCAIMovement()->GetAttackRangeSQR()> m_fDist2EnemySQR); 
	}
	else
	{
		CPoint obTargetPos = m_pTarget->GetPosition();
		bool bVisible = CAINavigationSystemMan::Get().HasLineOfSight( m_pEnt->GetPosition(), obTargetPos );
		
		if (!bVisible) 
			return false;

		float fDist2Entity = ( obTargetPos - m_pEnt->GetPosition() ).LengthSquared();
		return ( m_Active && m_pCAIComp->GetCAIMovement()->GetAttackRangeSQR()> fDist2Entity ); 
	}
}

//!--------------------------------------------
//! Update
//!--------------------------------------------
void CAIVision::Update( float fTimeStep )
{
	// If the component isn't active - then return now. 
	if (!m_Active || m_pEnt->IsPaused())
		return;

	// Update the ages of the facts and cull any facts are too old. 
	for ( TFactCache::iterator obIt = m_Facts.begin(); obIt != m_Facts.end(); )
	{
		CAIFact& rFact = *obIt;

		// Remove the line of sight 
		rFact.m_uiVisibilityFlags &= ~CAIFact::LINE_OF_SIGHT;

		// Update the fact age. 
		rFact.m_fAge += fTimeStep;

		// If the fact is too old, then remove it. 
		if( rFact.m_fAge > 30.0f || (rFact.m_pEnt && rFact.m_pEnt->ToCharacter()->IsDead()) )
		{
			if( &(*obIt) != &m_Facts.back() )
				*obIt = m_Facts.back();

			m_Facts.pop_back();
		}
		else
		{
			++obIt;
		}
	}
 
	// Only update vision cache on entities that have been mapped to do so. 
	if( m_pEnt->GetVisionUpdateRate() > 0.0f )
	{
		// Update the current update time for the vision
		m_VisionUpdateTime += fTimeStep;

		// If the time is greater than the required update rate - then process an update. 
		if( m_VisionUpdateTime >= m_pEnt->GetVisionUpdateRate() )
		{
			// Generate a list of visible enemy entities, this could be time sliced if performance requires it. 
			GenerateListOfVisibleEnemies();

			// Reset the vision update rate
			m_VisionUpdateTime -= m_pEnt->GetVisionUpdateRate();
		}

		// If there isn't an active attack target...
		if( !GetTarget() && m_pEnt->GetMessageHandler() )
		{
			// Find a good target.
			const CEntity* pBestTarget = GetBestAttackTarget();

			// If there is a good target
			if( pBestTarget )
			{
				// Send a message saying that there is an attack target available.
				Message msg( msg_new_attack_target );
				msg.AddParam( pBestTarget );
				m_pEnt->GetMessageHandler()->Receive( msg );
			}
		}
	}

	// Draw View Cones
	// Don't include a DebugRender() here, otherwise ALL entities get their cones drawn
	// To see the vision cone of an entity, press V in the AI context and then select the entity
	// in the GAME context with Ctrl+E.
	// The same process for stop rendering the view cones. - Dario -
	// DebugRender(); 
	
		// Get the target
	m_pTarget = m_pCAIComp->GetCAIMovement()->GetEntityToAttack();

	// If vision is not activated or there is no target, the AI does not see anything
	if (!m_Active || !m_pTarget || m_pTarget->ToCharacter()->IsDead())
	{
		m_VisionResults = VIS_RESULT_NOTHING_SEEN;
		m_LastKnownEnemyPos = CPoint(CONSTRUCT_CLEAR);
		return;
	}

	// ===================================================================
	// ===================         UPDATE      ===========================
	// ===================================================================

	// Update stored Player's position
	CPoint obEnemyPos = m_pTarget->GetPosition();
	
	// Calculate Distance to Player
	CPoint		obMyPos = m_pEnt->GetPosition();
	CDirection	obLineAI2Enemy( obEnemyPos - obMyPos );
	
	// Calculate the SQR of the (Straight) Distance to the player
	m_fDist2EnemySQR = obLineAI2Enemy.LengthSquared();

	if ( m_fDist2EnemySQR < EPSILON )
	{
		// Something is not too good here ... so just in case
		m_VisionResults = VIS_RESULT_NOTHING_SEEN;
		return;
	}
    
	// Get Angle to Player
	// HeadFacingUpdate() -- Needed ? (Dario)

	CDirection	obAIFacingDir = m_pEnt->GetMatrix().GetZAxis();

	float fAngle = fabs (MovementControllerUtilities::RotationAboutY( obAIFacingDir, obLineAI2Enemy ));

	// Is the AI carring a shooting weapon?
	//if ( m_pCAIComp->GetControllerModifier() != AMCM_NONE )
	if ((Character*)m_pEnt->GetRangedWeapon() != NULL )
	{
		// =============================================
		// ====  SHOOTER GUY CARRYING RANGE WEAPON   ===
		// =============================================

		// Is the enemy out of MELE range and withing shooting range?
		if ( (m_fDist2EnemySQR > m_Params.fCloseDistSQR) &&
			 (m_fDist2EnemySQR < m_Params.fShootAimDistSQR) &&
			 (fAngle<m_Params.fShootAimHalfAngle)
			)
		{
			// In Shooting Range
			if (CAINavigationSystemMan::Get().HasShootingLineOfSight( obMyPos, obEnemyPos ) &&
				!AreAIsInShootingRange())
			{
				// In shooting range

				// Is there any AI in between ?

				m_VisionResults			= VIS_RESULT_SHOOTING_RANGE;
				m_LastKnownEnemyPos		= obEnemyPos;
				m_bEverSeenEnemy		= true;
				return;
			}
		}
		else if ( m_fDist2EnemySQR < m_Params.fCloseDistSQR &&
				  CAINavigationSystemMan::Get().HasLineOfSight( obMyPos, obEnemyPos )
				)
		{
			// In Mele Range
			m_VisionResults			= VIS_RESULT_MELE_RANGE;
			m_LastKnownEnemyPos		= obEnemyPos;
			m_bEverSeenEnemy		= true;
			return;
		}
	}

	// Is the enemy in my "Circle of Trust"?
	if ( (m_fDist2EnemySQR < m_Params.fInnerRadiusSQR) ||
			( (fAngle < m_Params.fHalfCloseViewAngle) && (m_fDist2EnemySQR < m_Params.fCloseDistSQR) )
		)
	{
		if (CAINavigationSystemMan::Get().HasLineOfSight( obMyPos, obEnemyPos ))
		{
			// I can see you in my circle of trust!!!
			m_VisionResults			= VIS_RESULT_ENEMY_SEEN;
			m_LastKnownEnemyPos		= obEnemyPos;
			m_bEverSeenEnemy		= true;
			return;
		}
	}

	// Do I see the player? (and how?)
	if ( (fAngle > m_Params.fOPPHalfViewAngle) || (m_fDist2EnemySQR > m_Params.fOPPMaxViewDistSQR) )
	{
		// The enemy is either too far or not in my FOV 
		m_VisionResults = VIS_RESULT_NOTHING_SEEN;
		return;
	}
	else 
	{
		// In a way or another I am in a good position to see the enemy
		// But is there any wall in between?

	//	bool bHasLOS = CAINavigationSystemMan::Get().HasLineOfSight( obMyPos, obEnemyPos );
		bool bHasLOS = CAINavigationSystemMan::Get().HasLineOfSightExcludingGoAroundVolumes( obMyPos, obEnemyPos );
		if ( !bHasLOS )
		{
			// I cannot see the player.
			m_VisionResults = VIS_RESULT_NOTHING_SEEN;
			return;
		}
		else
		{
			bool bInsideMainConeOfView = false;
			if ( (fAngle < m_Params.fHalfViewAngle) && (m_fDist2EnemySQR < m_Params.fMaxViewDistSQR) )
			{
				// The enemy is clearly visible
				m_VisionResults = VIS_RESULT_ENEMY_SEEN;
				bInsideMainConeOfView = true;
			}
			else
			{
				// The enemy is somewhere around, but I cannot properly locate it (this vision of mine...)
				m_VisionResults = m_bAlerted ? VIS_RESULT_ENEMY_SEEN : VIS_RESULT_SOMETHING_SEEN;
				bInsideMainConeOfView = true; // I think it will be best not to use so many cones (Dario), at least in combat
			}

			// ==================================
			// Let's check the GoAround Volumes
			// ==================================
			bool bIntersectsGoAroundVols = CAINavigationSystemMan::Get().HasLineOfSightThroughGoAroundVolumes( obMyPos, obEnemyPos );
			if ( bInsideMainConeOfView && bIntersectsGoAroundVols )
			{
				// The enemy is visible BUT behind Go Around Volumes 
				m_VisionResults = VIS_RESULT_GO_AROUND;
			}
			
			// Store player's position
			m_LastKnownEnemyPos	= obEnemyPos;
			m_bEverSeenEnemy		= true;
		}
	}
}

//------------------------------------------------------------------------------------------
//!  AreAIsInShootingRange
//------------------------------------------------------------------------------------------
bool CAIVision::AreAIsInShootingRange ( void )
{
	static const float NARROW_ANGLE_COS = 0.98f;
	CEntityQuery obQuery;

	// The entities should at least be alive and be an AI
	CEQCHealthLTE obHealth(0.0f);
	obQuery.AddUnClause( obHealth );
	CEQCIsEnemy obIsEnemy;
    obQuery.AddClause( obIsEnemy );

	CPoint		obAIPos = m_pEnt->GetPosition();
	CPoint		obTargetPos = m_pTarget->GetPosition();
	CDirection	obLine2Enemy(obTargetPos - obAIPos);

	CEQCProximitySphere obSphere;
	obSphere.Set(m_pEnt->GetPosition(), obLine2Enemy.Length());
	obQuery.AddClause(obSphere);

	// Exclude this AI
	obQuery.AddExcludedEntity( *m_pEnt );

    CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AI );

	// Get the list of all the found entities.
	QueryResultsContainerType& rEntList = obQuery.GetResults();

	obLine2Enemy.Normalise();

	QueryResultsContainerType::const_iterator obIt	= rEntList.begin();
	QueryResultsContainerType::const_iterator obItEnd = rEntList.end();
	for( ; obIt != obItEnd;	++obIt )
	{
		CEntity* pMyAIFriends = *obIt;
		//if (pMyAIFriends->IsPaused()) 
		//{
		//	ntAssert_p(0,("AIVISION: CEQCProximitySphere returned a paused entity!!"));
		//}

		CPoint		obMyFriendPos = pMyAIFriends->GetPosition();
		CDirection	obLine2Friend(obMyFriendPos - obAIPos);

		obLine2Friend.Normalise();

		float fCosAngle = obLine2Enemy.Dot(obLine2Friend);

		if ( fCosAngle > NARROW_ANGLE_COS )
			return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!  public  GetBestAttackTarget
//!
//!  @return CEntity * An entity pointer that would make the best next attack target. 
//!
//!  @remarks This function can return NULL, so watch out!
//!
//!  @author GavB @date 29/06/2006
//------------------------------------------------------------------------------------------
const CEntity* CAIVision::GetBestAttackTarget(void) const
{
	float			fScore = 0.0f;
	const CAIFact*	pBestFact = 0;

	// Loop through the cache of facts looking for an old match
	for ( TFactCache::const_iterator obIt = m_Facts.begin(); obIt != m_Facts.end(); ++obIt)
	{
		// Get the fact score
		float fTestScore = obIt->CalculateScore( m_pEnt );

		// If the test score is lower than the current best then assign a new best fact. 
		if( !pBestFact || fTestScore < fScore )
		{
			pBestFact	= &*obIt;
			fScore		= fTestScore;
			continue;
		}
	}

	return pBestFact ? pBestFact->m_pEnt : 0;
}

//------------------------------------------------------------------------------------------
//!  public constant  GetFact
//!
//!  @param [in]	pEnt const CEntity *    Pointer the entity that the knownledge of is 
//!											required
//!
//!  @return const CAIFact * Pointer the fact - may be NULL
//!
//!  @author GavB @date 29/06/2006
//------------------------------------------------------------------------------------------
const CAIFact* CAIVision::GetFact( const CEntity* pEnt ) const
{
	// If no entity is given - then return none
	if(!pEnt)
	{
		// Loop through the cache of facts looking for an old match
		for ( TFactCache::const_iterator obIt = m_Facts.begin(); obIt != m_Facts.end(); ++obIt)
		{
			// Return the fact if this is correct entity
			if( obIt->m_pEnt == pEnt )
				return &*obIt;
		}
	}
	return NULL;
}


//------------------------------------------------------------------------------------------
//!  private  GenerateListOfVisibleEnemies
//!
//!  @remarks	The list created within can be otained via the function GetVisableEntities
//!				This is a private function	
//!
//!  @author GavB @date 29/06/2006
//------------------------------------------------------------------------------------------
void CAIVision::GenerateListOfVisibleEnemies( void )
{
	// If the enemy type isn't set, the don't bother scanning entities
	if( !m_EnemyType )
		return;

	// Create a query to scan all the known entities and extract the required ones
	CEntityQuery obQuery;

	// The entities should at least be alive. 
	CEQCHealthLTE obHealth(0.0f);
	obQuery.AddUnClause( obHealth );

	// Is Enemy clause. 
	CEQCIsEnemy obIsEnemy;

	// If this is an enemy entity then don't pick other enemies
	if( m_pEnt->IsEnemy() )
	{
		if( !m_pEnt->AttackAI() )
		{
			obQuery.AddUnClause( obIsEnemy );
		}
	}
	else
	{
		obQuery.AddClause( obIsEnemy );
	}
		

	// Create a matrix for the segments tests
	CMatrix matEnt = m_pEnt->GetMatrix();
	matEnt.SetTranslation( matEnt.GetTranslation() + CPoint( 0.0f, GetParams()->fVisionYOffset, 0.0f ) );
	matEnt.SetYAxis( matEnt.GetYAxis() * 20.0f );

	// Create a segment volume to check within
	CEQCProximitySegment obSegMaxView;
	obSegMaxView.SetRadius( fsqrtf(GetParams()->fMaxViewDistSQR) );
	obSegMaxView.SetAngle( GetParams()->fHalfViewAngle * 2.0f );
	obSegMaxView.SetMatrix( matEnt );

	CEQCProximitySegment obSegOPPMaxView;
	obSegOPPMaxView.SetRadius( fsqrtf(GetParams()->fOPPMaxViewDistSQR) );
	obSegOPPMaxView.SetAngle( GetParams()->fOPPHalfViewAngle * 2.0f );
	obSegOPPMaxView.SetMatrix( matEnt );

	CEQCProximitySegment obSegClose;
	obSegClose.SetRadius( fsqrtf(GetParams()->fCloseDistSQR) );
	obSegClose.SetAngle( GetParams()->fHalfCloseViewAngle * 2.0f );
	obSegClose.SetMatrix( matEnt );


	CEQCProximitySegment obSegShoot;
	obSegShoot.SetRadius( fsqrtf(GetParams()->fShootAimDistSQR) );
	obSegShoot.SetAngle( GetParams()->fShootAimHalfAngle * 2.0f );
	obSegShoot.SetMatrix( matEnt );


	// Add add all the segment searches into an Or clause
	CEQCOr obOrClauseSet;
	obOrClauseSet.AddClause( obSegMaxView );
	obOrClauseSet.AddClause( obSegOPPMaxView );
	obOrClauseSet.AddClause( obSegClose );

	if( m_pCAIComp->GetControllerModifier() != AMCM_NONE )
		obOrClauseSet.AddClause( obSegShoot );


	// Create a local search for the inner range. 
	CEQCProximitySphere obInnerRange;
	obInnerRange.Set( m_pEnt->GetPosition(), fsqrtf(GetParams()->fInnerRadiusSQR) );
	obOrClauseSet.AddClause( obInnerRange );


	// Add the clause for the range checks
	obQuery.AddClause( obOrClauseSet );

	// Don't add this entity
	obQuery.AddExcludedEntity( *m_pEnt );

	// Don't scan for any entities that are marked a invisible to this entity.
	for( AI::InvisibleList::const_iterator obIt = m_pEnt->GetInvisibleList().begin(); obIt != m_pEnt->GetInvisibleList().end(); ++obIt )
	{
		obQuery.AddExcludedEntity( *m_pEnt );
	}

	// Only check for entities in this area
	CEQCIsEntityInArea obAreaCheck( AreaManager::Get().GetCurrActiveArea() );
	obQuery.AddClause( obAreaCheck );

	// Find all the AI entities that match our needs
	CEntityManager::Get().FindEntitiesByType( obQuery, m_EnemyType );

	// Get the list of all the found entities.
	QueryResultsContainerType& rEntList = obQuery.GetResults();

	// Run through the list of entities found
	for( QueryResultsContainerType::const_iterator obIt = rEntList.begin(); 
			obIt != rEntList.end();
				++obIt )
	{
		CEntity* pBaseEnt = (*obIt);

		if( !pBaseEnt->IsCharacter() )
			continue;

		Character* pEnt = pBaseEnt->ToCharacter();

		// Sanity check
		ntAssert( pEnt );

		// Create a fact locally
		CAIFact obFact;

		// Save the entity pointer in to the fact
		obFact.m_pEnt = pEnt;

		// Save the current position of the entity
		obFact.m_obLastKnownPosition = pEnt->GetPosition();

		// Clear the age of the fact to zero
		obFact.m_fAge = 0.0f;

		// Clear the visibility flags
		obFact.m_uiVisibilityFlags = 0;

		if( obSegOPPMaxView.Visit( *pEnt ) )
			obFact.m_uiVisibilityFlags |= CAIFact::OPP_RANGE;

		if( obSegMaxView.Visit( *pEnt ) )
			obFact.m_uiVisibilityFlags |= CAIFact::MAIN_RANGE;

		if( m_pCAIComp->GetControllerModifier() != AMCM_NONE && obSegShoot.Visit( *pEnt ) )
			obFact.m_uiVisibilityFlags |= CAIFact::SHOOT_RANGE;

		if( obSegClose.Visit( *pEnt ) )
			obFact.m_uiVisibilityFlags |= CAIFact::CLOSE_RANGE;

		if( obInnerRange.Visit( *pEnt ) )
			obFact.m_uiVisibilityFlags |= CAIFact::INNER_RANGE;

		if( CAINavigationSystemMan::Get().HasLineOfSight( m_pEnt->GetPosition(), obFact.m_obLastKnownPosition ) )
			obFact.m_uiVisibilityFlags |= CAIFact::LINE_OF_SIGHT;


		// Add the entity to the list.
		UpdateFact( obFact );
	}

}


//------------------------------------------------------------------------------------------
//!  private  UpdateFact
//!
//!  @param [in]   rFact  const CAIFact &    
//!
//!  @remarks	Updates a fact for a given entity. The fact might not get added to the entity
//!				is the cache is full and the priority isn't enough.
//!
//!  @author GavB @date 29/06/2006
//------------------------------------------------------------------------------------------

void CAIVision::UpdateFact( const CAIFact& rFact )
{
	// Loop through the cache of facts looking for an old match
	for ( TFactCache::iterator obIt = m_Facts.begin(); obIt != m_Facts.end(); ++obIt)
	{
		const CEntity* pEnt = obIt->m_pEnt;

		// If the fact to update points to the same entity, then just update it and return
		if( pEnt == rFact.m_pEnt )
		{
			*obIt = rFact;
			return;
		}
	}

	// If the fact cache isn't yet full, then place the new fact in the cache
	if( m_Facts.capacity() > 0)
	{
		m_Facts.push_back( rFact );
		return;
	}

	// Calculate a score for the fact
	float	bWorstScore = 0.0f;
	TFactCache::iterator obWorst = m_Facts.end();

	// Loop through the cache of facts ready to add a new fact
	for ( TFactCache::iterator obIt = m_Facts.begin(); obIt != m_Facts.end(); ++obIt)
	{
		float fScore = obIt->CalculateScore(m_pEnt);

		if( fScore > bWorstScore )
		{
			bWorstScore = fScore;
			obWorst = obIt;
		}
	}

	float fScore = rFact.CalculateScore( m_pEnt );

	// If this score is better than that of the worst, replace it. 
	if( fScore < bWorstScore )
	{
		*obWorst = rFact;
	}
}

//------------------------------------------------------------------------------------------
//!  public  SetParent
//!
//!  @param [in, out]  pEnt CEntity *    <TODO: insert parameter description here>
//!
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 29/06/2006
//------------------------------------------------------------------------------------------
void CAIVision::SetParent( CEntity*	pEnt )	
{ 
	// Make sure the entity is valid before assigning it. 
	if (pEnt) 
	{ 
		// Make sure the vision is only attached to AI entities
		ntError( pEnt->IsAI() );

		m_pEnt = static_cast<AI*>(pEnt); 

		// Null out the current enemy type
		m_EnemyType = 0;

		// If this is an enemy entity - then make the player and boss type entities targets
		if( m_pEnt->IsEnemy() )
			m_EnemyType |= CEntity::EntType_Player | CEntity::EntType_Boss;
		
		if( m_pEnt->AttackAI() )
			m_EnemyType |= CEntity::EntType_AI;
	}

}


//!--------------------------------------------
//! DebugRender
//!--------------------------------------------
void CAIVision::DebugRender (void) const
{
#ifndef _GOLD_MASTER
	static float fLasPlayerPosRadius = 0.0f;
	const static CPoint DBG_Y_OFFSET = CPoint(0,0.1f,0);

	// Show facts that this current entity knowns about others. 
	if( CAINavigationSystemMan::Get().m_bRenderKnowledge && CEntityBrowser::Get().GetCurrentEntity() == m_pEnt )
	{
		g_VisualDebug->Printf3D( m_pEnt->GetPosition(), 0.0f, 0.0f, DC_RED, 0, "SOURCE" );

		for ( TFactCache::const_iterator obIt = m_Facts.begin(); obIt != m_Facts.end(); ++obIt)
		{
			g_VisualDebug->Printf3D( obIt->m_obLastKnownPosition, 0.0f, 0.0f, DC_GREEN, 0, "VisFlags: %X", obIt->m_uiVisibilityFlags );
			g_VisualDebug->Printf3D( obIt->m_obLastKnownPosition, 0.0f, 10.0f, DC_GREEN, 0, "Age: %f", obIt->m_fAge );
			g_VisualDebug->Printf3D( obIt->m_obLastKnownPosition, 0.0f, 20.0f, DC_GREEN, 0, "Score: %f", obIt->CalculateScore(m_pEnt) );
		}
	}

	// draw the view cones
	if(	(CAINavigationSystemMan::Get().m_bRenderViewCones) && !m_pEnt->ToCharacter()->IsDead() )
	{
		CPoint		obPos			= m_pEnt->GetPosition()+CPoint(0,1.5f,0);
		CDirection	obAIFacingDir	= m_pEnt->GetMatrix().GetZAxis();

		DrawViewCone( m_pEnt, 0 );
		DrawViewCone( m_pEnt, 1 );
		DrawViewCone( m_pEnt, 2 );
		DrawViewCone( m_pEnt, 3 );
		//if ( m_pCAIComp->Lua_GetControllerModifier() != AMCM_NONE )	DrawViewCone( m_pEnt, 4 );
		if ((Character*)m_pEnt->GetRangedWeapon() != NULL ) DrawViewCone( m_pEnt, 4 );

		// Mark the last known player position

		fLasPlayerPosRadius +=0.1f;
		if ( fLasPlayerPosRadius>1.5f ) fLasPlayerPosRadius = 0.0f;

		unsigned int uiPosColor =  (m_VisionResults == VIS_RESULT_NOTHING_SEEN) ? DC_CYAN : DC_BLUE;
		CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
		ArcMatrix.SetTranslation(m_LastKnownEnemyPos+DBG_Y_OFFSET);
		g_VisualDebug->RenderArc(ArcMatrix, fLasPlayerPosRadius , TWO_PI,  uiPosColor);
	}
#endif
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIComponent::DrawViewCone
//! Draws the AI's cone of vision
//!                                                                                         
//------------------------------------------------------------------------------------------

void CAIVision::DrawViewCone( AI* pEnt, unsigned int uiType )
{
#ifndef _GOLD_MASTER

	CPoint		obPosition	= pEnt->GetPosition()+CPoint(0,1.5f,0);
	CDirection	obFacing	= pEnt->GetMatrix().GetZAxis();

	float fViewAngle	= 0.0f;
	float fViewDist		= 0.0f;
	unsigned int uiColor= 0;

	switch (uiType)
	{
	case 0: 
		fViewAngle	= pEnt->GetAIComponent()->GetAIVision()->GetParams()->fHalfViewAngle;
		fViewDist	= sqrt(pEnt->GetAIComponent()->GetAIVision()->GetParams()->fMaxViewDistSQR);
		uiColor		= DC_GREEN;
		break;
	case 1:
		fViewAngle	= pEnt->GetAIComponent()->GetAIVision()->GetParams()->fOPPHalfViewAngle;
		fViewDist	= sqrt(pEnt->GetAIComponent()->GetAIVision()->GetParams()->fOPPMaxViewDistSQR);
		uiColor		= DC_CYAN;
		break;
	case 2:
		fViewAngle	= pEnt->GetAIComponent()->GetAIVision()->GetParams()->fHalfCloseViewAngle;
		fViewDist	= sqrt(pEnt->GetAIComponent()->GetAIVision()->GetParams()->fCloseDistSQR);
		uiColor		= DC_BLUE;
		break;
	case 3:
		fViewAngle	= PI;
		fViewDist	= sqrt(pEnt->GetAIComponent()->GetAIVision()->GetParams()->fInnerRadiusSQR);
		uiColor		= DC_RED;
		break;
	case 4:
		fViewAngle	= pEnt->GetAIComponent()->GetAIVision()->GetParams()->fShootAimHalfAngle;
		fViewDist	= sqrt(pEnt->GetAIComponent()->GetAIVision()->GetParams()->fShootAimDistSQR);
		uiColor		= DC_RED;
		break;
	}
	
	// debug draw the view cone
	CPoint	obDiff1( obFacing );
	obDiff1 /= obDiff1.Length();
	obDiff1 *= fViewDist;

	CPoint	obDiff2( obDiff1 );

	// make one side of the cone
	float fAngularModifier = fViewAngle;

	float fCos = cos( fAngularModifier );
	float fSin = sin( fAngularModifier );
	float fNewX = fCos * obDiff1.X() + fSin * obDiff1.Z();
	float fNewZ = fCos * obDiff1.Z() - fSin * obDiff1.X();

	obDiff1.X() = fNewX;
	obDiff1.Z() = fNewZ;

	// and the other
	fAngularModifier *= -1.0f;

	fCos = cos( fAngularModifier );
	fSin = sin( fAngularModifier );
	fNewX = fCos * obDiff2.X() + fSin * obDiff2.Z();
	fNewZ = fCos * obDiff2.Z() - fSin * obDiff2.X();

	obDiff2.X() = fNewX;
	obDiff2.Z() = fNewZ;

	g_VisualDebug->RenderLine( obPosition, obPosition + obDiff1, uiColor, 8192 /*DPF_NOCULLING*/ );
	g_VisualDebug->RenderLine( obPosition, obPosition + obDiff2, uiColor, 8192 /*DPF_NOCULLING*/ );

	CMatrix ArcMatrix = pEnt->GetMatrix();
	ArcMatrix.SetTranslation(obPosition);
	g_VisualDebug->RenderArc(ArcMatrix, fViewDist , fViewAngle*2.0f,  uiColor);
#endif
}

//------------------------------------------------------------------------------------------
//!  public  ForceVisionScanNextUpdate
//!
//!
//!  @remarks Forces a vision fact scan on the next update. 
//!
//!  @author GavB @date 01/08/2006
//------------------------------------------------------------------------------------------
void CAIVision::ForceVisionScanNextUpdate(void) 
{
	m_VisionUpdateTime += m_pEnt->GetVisionUpdateRate();
}

