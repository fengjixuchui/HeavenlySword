
#include "ai/aichase.h"
#include "ai/ainavpath.h"
#include "ai/ainavgraphmanager.h"
#include "core/osddisplay.h"
#include "game/entityinfo.h"
#include "game/entitymanager.h"
#include "game/aicomponent.h"
#include "game/messagehandler.h"

/***************************************************************************************************
****************************************************************************************************
****************************************************************************************************
*
*	CLASS			CAIChase
*
****************************************************************************************************
****************************************************************************************************
***************************************************************************************************/

bool
CAIChase::AtLastPlayerPos( void ) const
{
	const float threshold = 8.0f;
	CPoint obPos( m_pobEnt->GetPosition() );

	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	//CPoint obDiff = obPos - m_pobAIComp->GetLastKnownPlayerPos();
	CPoint obDiff = obPos - m_pobAIComp->GetActualPlayerPos();
	obDiff.Y() = 0.0f;

	//if (obDiff.LengthSquared() < 16.0f)
	if (obDiff.LengthSquared() < (threshold * threshold))
	{
		return true;
	}

	return false;
}


bool CAIChase::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.3f);
	const float	fWalkTowardsTime	(2.0f);
	const float	fReAcquireTime		(20.0f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			//ntPrintf("Chase::STATE_INITIALISE\n");
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;

			if( !m_pobAIComp->GetParent()->IsInNinjaSequence() && 
				!m_pobAIComp->GetDisabled() )
			{
				if( m_fTimer > fInitTime )
				{
					if( !m_pobAIComp->HasSeenPlayer() )
					{
						m_pobAIComp->SetActualPlayerPos( CEntityManager::Get().GetPlayer()->GetPosition() );
					}

					//SetState( STATE_CHASE_ENEMY );                
					// All this AI really needs looking at... E3HACK
					AI_BEHAVIOUR_SEND_MSG( CHASE_INATTACKRANGE );
				}
			}

			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_CHASE_ENEMY )
		OnEnter
			//ntPrintf("Chase::STATE_CHASE_ENEMY\n");
			OSD::Add( OSD::AISTATES, 0xffffffff, "After her!\n" );	
			m_fTimer = 0.0f;
			
			// set action to run
			m_pobAIComp->SetAction( ACTION_RUN );
			//m_pobAIComp->SetActionDest( m_pobAIComp->GetLastKnownPlayerPos() );
			m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );
			//m_pobAIComp->SetAction( ACTION_WALK );
			//m_pobAIComp->SetActionMoveSpeed( 0.0f );

			m_bPlayerLeftPsychicRadius = false;
			m_obLastReachablePlayerPos = m_pobEnt->GetPosition();

		OnUpdate
			m_fTimer += fTimeChange;

			if (m_pobAIComp->GetPathfindStatus() == CAIComponent::PF_SUCCESS)
			{
				//m_obLastReachablePlayerPos = m_pobAIComp->GetLastKnownPlayerPos();
				m_obLastReachablePlayerPos = m_pobAIComp->GetActualPlayerPos();
			}

			if (m_pobAIComp->GetPathfindStatus() == CAIComponent::PF_FAILED)
			{
				// pathfind to enemy destination failed, taunt them instead
				//SetState( STATE_MOVE_TO_TAUNT );
				m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );
			}
	
			// set destination to last player pos
			//if (m_pobAIComp->GetPathfindStatus() == CAIComponent::PF_SUCCESS)
			{
				//m_pobAIComp->SetActionDest( m_pobAIComp->GetLastKnownPlayerPos() );
				m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );
			}

			// if the player is too far away from us at any point during the chase, then we
			// rely on visual to track them
			const float fPsychicRadiusSquared = 6.0f * 6.0f;
			//CPoint obDiff = m_pobAIComp->GetLastKnownPlayerPos() - m_pobEnt->GetPosition();
			CPoint obDiff = m_pobAIComp->GetActualPlayerPos() - m_pobEnt->GetPosition();
			if (obDiff.LengthSquared() > fPsychicRadiusSquared)
			{
				m_bPlayerLeftPsychicRadius = true;
			}

			// if we've reached the last known player position, then we have a few options
			if (AtLastPlayerPos())
			{
				// if we can still see the player, then start attacking
				//if (m_pobAIComp->CanSeePlayer())
				if (1)
				{
					//if (obDiff.LengthSquared() < 9.0f)
					if (1)
					{
						AI_BEHAVIOUR_SEND_MSG( CHASE_INATTACKRANGE );
						m_pobAIComp->SetActionMoveSpeed( 0.0f );
					}
				}
				else if (!m_bPlayerLeftPsychicRadius)
				{
					// if the player stayed inside the "psychic radius" (!) of the AI then
					// reaquire them
					SetState( STATE_REACQUIRE_ENEMY );
				}
				else
				{
					// if we've really lost them, walk a short way towards them just
					// in case they're hiding round the corner :)
					SetState( STATE_WALK_TOWARDS_ENEMY );
				}
			}
			else
			{
				m_pobAIComp->SetAction( ACTION_RUN );
				m_pobAIComp->SetActionMoveSpeed( 1.0f );
				m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );
			}

		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_WALK_TOWARDS_ENEMY )
		OnEnter
			//ntPrintf("Chase::STATE_WALK_TOWARDS_ENEMY\n");
			m_fTimer = 0.0f;
			
			// set action to walk
			m_pobAIComp->SetAction( ACTION_WALK );

		OnUpdate
			m_fTimer += fTimeChange;

			m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );

			if (m_pobAIComp->CanSeeSuspicious())
			{
				SetState( STATE_CHASE_ENEMY );
			}

			// if we have lost track of the player, investigate the surrounding area
			if (m_fTimer > fWalkTowardsTime)
			{
				SetState( STATE_LOST_ENEMY );
			}
			
		OnExit
			m_fTimer = 0.0f;


	/***************************************************************/
	AIState( STATE_LOST_ENEMY )
		OnEnter
			//ntPrintf("Chase::STATE_LOST_ENEMY\n");
			OSD::Add( OSD::AISTATES, 0xffffffff, "Where did she go?" );	
			m_fTimer = 0.0f;
			
			// set action to look
			m_pobAIComp->SetAction( ACTION_PATROL_LOOK );

		OnUpdate
			m_fTimer += fTimeChange;

			if (m_pobAIComp->CanSeePlayer())
			{
				SetState( STATE_CHASE_ENEMY );
			}

			// if we have lost track of the player, investigate the surrounding area
			if (m_pobAIComp->IsSimpleActionComplete())
			{
				AI_BEHAVIOUR_SEND_MSG( CHASE_LOSTTARGET );
			}
			
		OnExit
			m_fTimer = 0.0f;


	/***************************************************************/
	AIState( STATE_REACQUIRE_ENEMY )
		OnEnter
			//ntPrintf("Chase::STATE_REACQUIRE_ENEMY\n");
			//OSD::Add( OSD::AISTATES, 0xffffffff, "Where did she go?" );	
			m_fTimer = 0.0f;
			
			// set action to run
			m_pobAIComp->SetAction( ACTION_RUN );

		OnUpdate
			m_fTimer += fTimeChange;

			if (m_pobAIComp->GetPathfindStatus() == CAIComponent::PF_FAILED)
			{
				// pathfind to enemy destination failed, taunt them instead
				SetState( STATE_TAUNT_ENEMY );
			}

			if (m_pobAIComp->CanSeePlayer())
			{
				SetState( STATE_CHASE_ENEMY );
			}

			// if we have really lost the player, move to the lost state
			if (m_fTimer > fReAcquireTime)
			{
				SetState( STATE_LOST_ENEMY );
			}

			m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );
			
		OnExit
			m_fTimer = 0.0f;


	/***************************************************************/
	AIState( STATE_START_ATTACKING )
		OnEnter
			//ntPrintf("Chase::STATE_START_ATTACKING\n");
			OSD::Add( OSD::AISTATES, 0xffffffff, "Fighting time!" );	
			m_fTimer = 0.0f;
			
			// on entry, set the action to idle
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fInitTime )
			{
				AI_BEHAVIOUR_SEND_MSG( CHASE_INATTACKRANGE );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_MOVE_TO_TAUNT )
		OnEnter
			//ntPrintf("Chase::STATE_MOVE_TO_TAUNT\n");
			m_fTimer = 0.0f;
			
			CPoint diff = m_obLastReachablePlayerPos - m_pobEnt->GetPosition();
			m_obLastReachablePlayerPos -= (diff / diff.Length() );

			// on entry, set the action to idle
			//m_pobAIComp->SetAction( ACTION_TAUNT );
			m_pobAIComp->SetAction( ACTION_WALK );
			m_pobAIComp->SetActionMoveSpeed( 0.5f );
			m_pobAIComp->SetActionDest( m_obLastReachablePlayerPos );

		OnUpdate
			m_fTimer += fTimeChange;

			CPoint obDiff = m_obLastReachablePlayerPos - m_pobEnt->GetPosition();
			if (obDiff.LengthSquared() < 2.0f)
			{
				//ntPrintf( "Arrived at last reachable player pos\n" );
				SetState( STATE_TAUNT_ENEMY );
			}		
			
			if( m_fTimer > 3.0f )
				SetState( STATE_INITIALISE );
			//ntAssert( m_pobAIComp->GetPathfindStatus() != CAIComponent::PF_FAILED );

		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_TAUNT_ENEMY )
		OnEnter
			m_fTimer = 0.0f;

			// on entry, set the action to idle
			CDirection obFacing( m_pobAIComp->GetActualPlayerPos() - m_pobEnt->GetPosition() );
			m_pobAIComp->SetActionFacing( obFacing );
			m_pobAIComp->SetAction( ACTION_TAUNT );

		OnUpdate
			m_fTimer += fTimeChange;

			// if the player moves back to a reachable position, chase him again
			
			// make a path
			CAINavPath* testPath = CAINavGraphManager::Get().MakePath(
									m_pobEnt->GetPosition(), m_pobAIComp->GetActualPlayerPos(),
									&m_pobEnt->GetKeywords() );

			// if it's non-NULL, delete it and go back to STATE_WALK_TOWARDS_ENEMY
			if (testPath)
			{
				NT_DELETE_CHUNK( Mem::MC_AI, testPath );
				SetState( STATE_WALK_TOWARDS_ENEMY );
			}

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_TAUNT_LOOPBACK );
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_TAUNT_LOOPBACK )
		OnEnter
			m_fTimer = 0.0f;

			// on entry, set the action to idle
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );

		OnUpdate
			m_fTimer += fTimeChange;
			SetState( STATE_TAUNT_ENEMY );
		
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_IDLE )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );

		OnUpdate
			// m_fTimer += fTimeChange;

		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}


