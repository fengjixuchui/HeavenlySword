
#include "ai/aipatrol.h"
#include "ai/ainavnodes.h"
#include "ai/ainavgraphmanager.h"

#include "ai/aipatrolmanager.h"
#include "ai/aipatrolpath.h"
#include "ai/aipatrolnode.h"

#include "core/osddisplay.h"

#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/randmanager.h"
#include "game/messagehandler.h"

//--------------------------------------------------
//!
//!	CAIPatrol::Initialise
//!
//--------------------------------------------------

void
CAIPatrol::Initialise( void )
{
	// get the character's patrol path, and make an iterator pointing to its start
	PatrolNode*	pobStartNode = AIPatrolManager::Get().GetNearestNodeWithinRange( m_pobEnt->GetPosition(), 10.0f );

	if (!pobStartNode)
	{
		m_iPathNum = -1;
	}
	else
	{
		m_iPathNum = pobStartNode->GetPath();
		m_iStartNodeNum = pobStartNode->GetNum();
		m_iCurrentNodeNum = pobStartNode->GetNum();
	}
}


//--------------------------------------------------
//!
//!	CAIPatrol::WalkInitialise
//!
//--------------------------------------------------

void
CAIPatrol::WalkInitialise( void )
{
	if (m_iPathNum > -1)
	{				
		ntAssert( m_iPathNum <= AIPatrolManager::Get().GetLargestPathNum() ) ;
		ntAssert( m_iCurrentNodeNum < AIPatrolManager::Get().GetPath( m_iPathNum )->GetNumNodes() ) ;

		m_obDest = AIPatrolManager::Get().GetPath( m_iPathNum )->GetNode( m_iCurrentNodeNum )->GetPos();

		// set the action to walk. in theory, this is only being done from the behaviours,
		// so we should only have to set it this one time
		//m_pobAIComp->SetAction( ACTION_PATROL_WALK );
		m_pobAIComp->SetAction( ACTION_WALK );
		m_pobAIComp->SetActionMoveSpeed( 0.5f );

		// give the action a destination
		m_pobAIComp->SetActionDest( m_obDest );
	}
	else
	{
		// just play some idle anims
		m_pobAIComp->SetAction( ACTION_PATROL_IDLE );
	}

}


//--------------------------------------------------
//!
//!	CAIPatrol::Walk
//!	Checks distance from current dest, and picks new
//! dest when the threshold distance is reached
//!
//--------------------------------------------------

void
CAIPatrol::Walk( void )
{
	// Am i at the dest node
	CPoint obPos( m_pobEnt->GetPosition() );

	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	CPoint obDiff = obPos - m_obDest;
	//obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() < 3.0f)
	{
		// increment the path iterator. if we're at the end, get the start
		if (++m_iCurrentNodeNum == AIPatrolManager::Get().GetPath( m_iPathNum )->GetNumNodes())
		{
			m_iCurrentNodeNum = 0;
		}

		PatrolNode*	pobNode = AIPatrolManager::Get().GetPath( m_iPathNum )->GetNode( m_iCurrentNodeNum );
		// if the patrol node exists
		if (pobNode)
		{
			// set the node pos as our dest
			m_obDest = pobNode->GetPos();
			m_pobAIComp->SetActionDest( m_obDest );
		}
	}
}


bool
CAIPatrol::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fChanceOfIdleStart	(0.5f);
	const float	fInitTime			(0.5f);
	const float	fWalkTime			(150000.0f);
	const float	fApproachTime		(0.1f);
	const float	fAlertedDelayTime	(0.7f);
	const float	fSeenTime			(1.0f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			//ntPrintf("Patrol::STATE_INITIALISE\n");
			//OSD::Add( OSD::AISTATES, 0xffffffff, "Patrol::STATE_INITIALISE\n" );	
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				// moved initialise here, because all the patrol paths have to be loaded
				// before it gets called
				Initialise();

				// randomly initialise into walk or idle
                if( grandf( 1.0f ) > fChanceOfIdleStart )
				{
					SetState( STATE_IDLE );
				}
				else
				{
					SetState( STATE_WALK );
				}
			}
			
		OnExit
			m_fTimer = 0.f;

	/***************************************************************/
	AIState( STATE_WALK )
		OnEnter
			//ntPrintf("Patrol::STATE_WALK\n");
//			OSD::EnableChannel( OSD::AISTATES );
			OSD::Add( OSD::AISTATES, 0xffffffff, "Back on patrol... again\n" );	
			m_fTimer = 0.0f;

			WalkInitialise();

		OnUpdate
			Walk();
			m_fTimer += fTimeChange;
			
			CPoint obAlertPlayerPos;
			if (AIAlertManager::Get().ActiveAlertInRange( m_pobEnt->GetPosition(), obAlertPlayerPos ))
			{
				SetState( STATE_ALERTED );
			}
			if (m_pobAIComp->CanSeeSuspicious())
			{
				SetState( STATE_SPOTTED );
			}
			if( m_fTimer > fWalkTime )
			{
				SetState( STATE_IDLE );
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_IDLE )
		OnEnter
			//ntPrintf("Patrol::STATE_IDLE\n");
			OSD::Add( OSD::AISTATES, 0xffffffff, "Man, patrolling sure" );	
			OSD::Add( OSD::AISTATES, 0xffffffff, "gets tiring" );	
			m_fTimer = 0.0f;

			// on entry, set the action to idle
			m_pobAIComp->SetAction( ACTION_PATROL_IDLE );

		OnUpdate
			m_fTimer += fTimeChange;

			CPoint obAlertPlayerPos;
			if (AIAlertManager::Get().ActiveAlertInRange( m_pobEnt->GetPosition(), obAlertPlayerPos ))
			{
				SetState( STATE_ALERTED );
			}
			if (m_pobAIComp->CanSeeSuspicious())
			{
				SetState( STATE_SPOTTED );
			}
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_LOOK );
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_LOOK )
		OnEnter
			//ntPrintf("Patrol::STATE_LOOK\n");
			OSD::Add( OSD::AISTATES, 0xffffffff, "Area looks secure\n" );	
			m_fTimer = 0.0f;

			// on entry, set the action to look
			m_pobAIComp->SetAction( ACTION_PATROL_LOOK );

		OnUpdate
			m_fTimer += fTimeChange;

			CPoint obAlertPlayerPos;
			if (AIAlertManager::Get().ActiveAlertInRange( m_pobEnt->GetPosition(), obAlertPlayerPos ))
			{
				SetState( STATE_ALERTED );				
			}
			if (m_pobAIComp->CanSeeSuspicious())
			{
				SetState( STATE_SPOTTED );
			}
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_WALK );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_SPOTTED )
		OnEnter
			ntPrintf("Patrol::STATE_SPOTTED\n");
			m_fTimer = 0.0f;

			// on entry, play the spotted player anim
			m_pobAIComp->CompleteSimpleAction();

			CDirection obFacing( m_pobAIComp->GetActualPlayerPos() - m_pobEnt->GetPosition() );
			m_pobAIComp->SetActionFacing( obFacing );
			m_pobAIComp->SetAction( ACTION_PATROL_SPOTTED );

			// and play some VO
			m_pobAIComp->PlayVO( "voice_sb", "vo_shld_enter_chase" );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_ALERT );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_ALERT )
		OnEnter
			ntPrintf("Patrol::STATE_ALERT\n");
			m_fTimer = 0.0f;

			// on entry, set the action to alert
			m_pobAIComp->CompleteSimpleAction();

			CDirection obFacing( m_pobAIComp->GetActualPlayerPos() - m_pobEnt->GetPosition() );
			m_pobAIComp->SetActionFacing( obFacing );
			m_pobAIComp->SetAction( ACTION_PATROL_ALERT );
			AIAlertManager::Get().SendAlert( m_pobEnt->GetPosition(), m_pobAIComp->GetActualPlayerPos() );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_SEENSOMETHING );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_ALERTED )
		OnEnter
			ntPrintf("Patrol::STATE_ALERTED\n");
			m_fTimer = 0.0f;

			// on entry, set the action to alerted
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetAction( ACTION_PATROL_ALERTED );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				m_pobAIComp->CanAlwaysSeePlayer( true );
				SetState( STATE_SEENSOMETHING );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_SEENSOMETHING )
		OnEnter
			//ntPrintf("Patrol::STATE_SEENSOMETHING\n");
			OSD::Add( OSD::AISTATES, 0xffffffff, "Hey, what's that" );	
			OSD::Add( OSD::AISTATES, 0xffffffff, "over there?" );	
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			//m_pobAIComp->SetAction( ACTION_PATROL_WALK );
			//m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );
			//m_pobAIComp->SetActionMoveSpeed( 1.0f );

			AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fApproachTime )
			{
				m_pobAIComp->SetAction( ACTION_NONE );
			}
	
			if( m_fTimer > fAlertedDelayTime )
			{
				m_pobAIComp->SetAlerted( true );
			}

			if( m_fTimer > fSeenTime )
			{
				// raise an alert for other AIs
				AIAlertManager::Get().SendAlert( m_pobEnt->GetPosition(), m_pobAIComp->GetActualPlayerPos() );

				if (m_pobAIComp->CanSeePlayer())
				{
					AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
				}
				else
				{
					AI_BEHAVIOUR_SEND_MSG( PATROL_SEENSOMETHING );
				}

				// set the state for when we get back
				SetState( STATE_RETURN );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_RETURN )
		OnEnter
			//ntPrintf("Patrol::STATE_RETURN\n");
			//OSD::Add( OSD::AISTATES, 0xffffffff, "Patrol::STATE_RETURN\n" );	
		OnUpdate
			SetState( STATE_WALK );
		OnExit

EndStateMachine
}

