
#include "ai/aicover.h"
#include "ai/ainavgraphmanager.h"

#include "core/osddisplay.h"

#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/randmanager.h"
#include "game/messagehandler.h"

#include "physics/world.h"


/***************************************************************************************************
****************************************************************************************************
****************************************************************************************************
*
*	CLASS			CAICover
*
****************************************************************************************************
****************************************************************************************************
***************************************************************************************************/

bool
CAICover::GenerateCoverPoint()
{
	bool	bSuccess( false );
	int		iAttempts( 0 );

	// XXX: get the player's position in the usual extremely nasty way
	CEntity* pobPlayer = CEntityManager::Get().GetPlayer(); // Is this the best way?
	if(!pobPlayer)
	{
		ntAssert( !"Cover behaviour couldn't find the player\n" );
		//pobPlayer = NetPlayerMan::GetLocalPlayerP() ? NetPlayerMan::GetLocalPlayerP()->GetEntity() : 0;
	}

	CPoint obPlayerPos(CONSTRUCT_CLEAR);
	if(pobPlayer)
	{
		obPlayerPos = pobPlayer->GetPosition();
	}

	// get the AI's position
	CPoint	obPos( m_pobEnt->GetPosition() );
 
	while (!bSuccess && iAttempts < 30000000)
	{
		// create random offset in x and z from AI position
		CPoint		obTestPos( obPos );
		const float	fRange( 30.0f );

		obTestPos.X() +=  grandf(fRange * 2.0f) - fRange;
		obTestPos.Z() +=  grandf(fRange * 2.0f) - fRange;
		obTestPos.Y() +=  1.0f;

		if (CAINavGraphManager::Get().InGraph( obTestPos ))
		{
			// test line of sight to player
			// do a raycast check
			Physics::TRACE_LINE_QUERY stQuery;

			Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
			// [Mus] - What settings for this cast ?
			obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
			obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
											Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
											Physics::RAGDOLL_BIT						|
											Physics::SMALL_INTERACTABLE_BIT				|
											Physics::LARGE_INTERACTABLE_BIT				);

			bool bCollision = Physics::CPhysicsWorld::Get().TraceLine( obTestPos, obPlayerPos, pobPlayer, stQuery, obFlag );

			// if there's a collision, offset back along the collision ray by the radius of the character
			if (bCollision)
			{
				m_obCoverPos = obTestPos;
				bSuccess = true;
			}

			// use this as our cover point
		}

		++iAttempts;
	}

	return bSuccess;
}

bool
CAICover::InCover( void ) const
{
	CPoint obPos( m_pobEnt->GetPosition() );

	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	CPoint obDiff = obPos - m_obCoverPos;
	obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() < 4.0f)
	{
		return true;
	}

	return false;
}



bool CAICover::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.3f);
	const float	fHideTime			(13.0f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				// if there are friendlies nearby, go to alerting state

				// else go to alerted state


				SetState( STATE_RUN );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_ALERT )
		OnEnter
			m_fTimer = 0.0f;
			
			// begin the "alerting" behaviour

		OnUpdate
			m_fTimer += fTimeChange;
			
			// continue alerting

			// if we've finished alerting our allies, go to the approach state
			if (0)
			{
				SetState( STATE_RUN );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_RUN )
		OnEnter
			OSD::Add( OSD::AISTATES, 0xffffffff, "Take cover!" );
			m_fTimer = 0.0f;

			GenerateCoverPoint();
			m_pobAIComp->SetAction( ACTION_RUN );
			m_pobAIComp->SetActionDest( m_obCoverPos );

		OnUpdate
			m_fTimer += fTimeChange;

			// continue alerted
			if (InCover())
			{
				SetState( STATE_HIDE );
			}

		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_HIDE )
		OnEnter
			OSD::Add( OSD::AISTATES, 0xffffffff, "I'm staying here!" );
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fHideTime )
			{
				OSD::Add( OSD::AISTATES, 0xffffffff, "I think it's safe" );
				AI_BEHAVIOUR_SEND_MSG( COVER_FINISHEDHIDING );
			}
			
		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}
