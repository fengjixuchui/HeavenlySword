//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aigroupmove.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aigroupmove.h"
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
#include "ai/ainavnodes.h"
#include "ai/ainavfollowpoint.h"

#include "game/aicomponent.h"
#include "game/entityinfo.h"
#include "game/randmanager.h"

bool
AIGroupMove::TooFarAhead() const
{
	// get our distance to the target
	CPoint obDiff = m_obTargetNodeCentre - m_pobEnt->GetPosition();
	float fDist = obDiff.Length();

	// get number in group
	int iNumInGroup = CAINavGraphManager::Get().GetRepulsion()->GetNumRepellers();
    
	float fMaxDiff = -999999.0f;

	for (int i = 0; i < iNumInGroup; i++)
	{
		// get distance to target
		CPoint	obGroupMemberDiff = m_obTargetNodeCentre - CAINavGraphManager::Get().GetRepulsion()->GetPos( i );
		float	fGroupMemberDist = obGroupMemberDiff.Length();

		// store greatest difference
		if ( (fGroupMemberDist - fDist) > fMaxDiff )
		{
			fMaxDiff = (fGroupMemberDist - fDist);
		}
	}

	// if difference is greater than threshold value, return true
	if (fMaxDiff > 8.0f)
	{
        return true;
	}
	
	return false;
}

float
AIGroupMove::DistFromGroupCentre() const
{
	// get number in group
	int iNumInGroup = CAINavGraphManager::Get().GetRepulsion()->GetNumRepellers();
    
	CPoint obCentre( CONSTRUCT_CLEAR );

	for (int i = 0; i < iNumInGroup; i++)
	{
		/*
		if (m_pobAIComp->GetAvoidanceID() != i)
		{
			obCentre += CAINavGraphManager::Get().GetRepulsion()->GetPos( i );
		}
		*/
	}

	float fOneOverNum = 1.0f / iNumInGroup;

	obCentre *= fOneOverNum; 

	// get our distance to the target
	CPoint obDiff = obCentre - m_pobEnt->GetPosition();

	// get dp of to centre vector and facing vector
	CDirection	obFacing;
	CDirection	obToCentre( obDiff );
	m_pobEnt->GetLookDirection( obFacing );

	if (obToCentre.Dot( obFacing ) > 0.0f)
	{
        return obDiff.Length();
	}
	else
	{
		return -obDiff.Length();
	}
}



void
AIGroupMove::SelectTargetNode()
{
	CPoint obTargetPos;
	if (m_iTripCycle % 2)
	{
		ntPrintf( "going to start pos, trip cycle = %d\n", m_iTripCycle );
		obTargetPos = m_obStartPoint;
	}
	else
	{
		ntPrintf( "going to end node, trip cycle = %d\n", m_iTripCycle );

		// the target is hardcoded at the moment
		static const CPoint obGoalPos( -72.22f, -14.0f, -76.78f );
		//static const CPoint obGoalPos( -76.0f, -14.0f, -7.0f );

		obTargetPos = obGoalPos;
	}

	m_obTargetNodeCentre = CAINavGraphManager::Get().GetRandomPosInGraph();
}


void
AIGroupMove::SetupGroupPosition()
{
	const float rangeOffset = 2.0f;
	const float minOffset = rangeOffset * -0.5f;

	// generate an offset within a range
	CPoint obOffset( minOffset + grandf( rangeOffset ), 0.0f, minOffset + grandf( rangeOffset ) );

	// set this as the follow point offset
	m_pobAIComp->GetFollowPoint().SetOffset( obOffset );

}


bool
AIGroupMove::InTargetNode( void ) const
{
#if 0
	CPoint obPos( m_pobAIComp->GetParentInfo()->GetPosition() );

	ntAssert( m_pobTargetNode );
	return m_pobTargetNode->Contains( obPos );
#else
	CPoint obPos( m_pobEnt->GetPosition() );

	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	CPoint obDiff = obPos - m_obTargetNodeCentre;
	obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() < 25.0f)
	{
		return true;
	}

	return false;
#endif
}



bool
AIGroupMove::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	float fInitTime = grandf( 1.0f ) + 0.3f;

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				m_iTripCycle = 1; 
				SetupGroupPosition();
				m_obStartPoint = m_pobEnt->GetPosition();
				SetState( STATE_GETTARGET );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_GETTARGET )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			SelectTargetNode();

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_GOTOTARGET );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_GOTOTARGET )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_WALK );
			m_pobAIComp->SetActionDest( m_obTargetNodeCentre );
			ntPrintf("set action to run, going to %f, %f, %f\n", m_obTargetNodeCentre.X(), m_obTargetNodeCentre.Y(), m_obTargetNodeCentre.Z() );

			// we're not at our goal if we're going to it
			//CAINavGraphManager::Get().GetRepulsion()->SetAtGoal( m_pobAIComp->GetAvoidanceID(), false );

		OnUpdate
			m_fTimer += fTimeChange;

			// if we've reached the target then go to the wait for group state
			if (InTargetNode())
			{
                SetState( STATE_WAITFORGROUP );                
			}

			float fNormalSpeed = 1.0f;
			float fMinSpeed = 0.5f;			
			float fMaxSpeed = 1.5f;
			float fDistFactor = 0.6f;

			float fSpeedAdjuster = DistFromGroupCentre() * fDistFactor;
			float fSpeed = fNormalSpeed + fSpeedAdjuster;
			fSpeed = max( fSpeed, fMinSpeed );
			fSpeed = min( fSpeed, fMaxSpeed );

			m_pobAIComp->SetActionMoveSpeed( fSpeed );
			//m_pobAIComp->SetActionMoveSpeed( 1.0f );
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_WAITFORGROUP )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );

			// tell the group brain that we're at our goal
			//CAINavGraphManager::Get().GetRepulsion()->SetAtGoal( m_pobAIComp->GetAvoidanceID(), true );
			m_iTripCycle++;

			//ntPrintf( "character %d at target\n", m_pobAIComp->GetAvoidanceID() );

		OnUpdate
			m_fTimer += fTimeChange;

/*
			if (m_pobAIComp->IsSimpleActionComplete())
			{
				m_pobAIComp->SetAction( ACTION_PATROL_LOOK );
			}
*/
			// check to see if everyone else got here
			if( 0 ) //CAINavGraphManager::Get().GetRepulsion()->AllAtGoal() )
			{
				ntPrintf( "everyone's here!\n" );
				SetState( STATE_GETTARGET );
			}

		OnExit
			m_fTimer = 0.0f;


EndStateMachine
}

