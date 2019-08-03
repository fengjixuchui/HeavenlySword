
#include "ai/aiinvestigate.h"
#include "ai/ainavnodes.h"
#include "ai/ainavgraphmanager.h"

#include "core/osddisplay.h"

#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/randmanager.h"
#include "game/messagehandler.h"

/***************************************************************************************************
****************************************************************************************************
****************************************************************************************************
*
*	CLASS			CAIInvestigate
*
****************************************************************************************************
****************************************************************************************************
***************************************************************************************************/

void
CAIInvestigate::GenerateSearchPos()
{
	bool	bDestValid( false );

	while (!bDestValid)
	{
		CPoint	obPos( m_pobEnt->GetPosition() );

		if (m_bCheckPlayerPos)
		{
			m_obDest = m_pobAIComp->GetLastKnownPlayerPos();
			m_bCheckPlayerPos = false;
		}
		else
		{
			// XXX: get the player's position in the usual extremely nasty way
			CEntity* pobPlayer = CEntityManager::Get().GetPlayer(); // Is this the best way?
			if(!pobPlayer)
			{
				ntAssert( !"Investigate behaviour couldn't find the player\n" );
				//pobPlayer = NetPlayerMan::GetLocalPlayerP() ? NetPlayerMan::GetLocalPlayerP()->GetEntity() : 0;
			}

			CPoint obPlayerPos(CONSTRUCT_CLEAR);
			if(pobPlayer)
			{
				obPlayerPos = pobPlayer->GetPosition();
			}

			// create the vector from the AI to the player
			CPoint	obDiff( obPlayerPos - obPos );

			// generate random angular and distance modifiers
			float fAngularRange = HALF_PI;
			float fAngularOffsetBase = 0 - (fAngularRange * 0.5f);
			float fAngularModifier = fAngularOffsetBase + grandf( fAngularRange );

			float fDistRange = 1.5f;
			float fDistOffsetBase = 0.5f;
			float fDistModifier = fDistOffsetBase + grandf( fDistRange );

			// apply modifiers
			obDiff *= fDistModifier;

			float fCos = cos( fAngularModifier );
			float fSin = sin( fAngularModifier );
			float fNewX = fCos * obDiff.X() + fSin * obDiff.Z();
			float fNewZ = fCos * obDiff.Z() - fSin * obDiff.X();

			obDiff.X() = fNewX;
			obDiff.Z() = fNewZ;	

			// store our new destination
			m_obDest = obPos + obDiff;
		}

		// check to see if the destination is valid
		// this pathfinder call will be made again by the movement layer if the path is valid,
		// however we don't have to worry too much about performance implications, as the result is cached
		// and will be reused
#if 0
		CAINavPath*	pobTempPath = CAINavGraphManager::Get().MakePath( obPos, m_obDest );
		if (pobTempPath)
		{
			bDestValid = true;
			NT_DELETE_CHUNK( Mem::MC_AI, pobTempPath );
		}
#else
		// if our destination isn't contained within the nearest navgraph node,
		// then just path to that node's centre point
		m_obDest = CAINavGraphManager::Get().GetNearestPosInGraph( m_obDest );
		bDestValid = true;

#endif
	}

	// if it's not then generate a new search location
	// XXX: this needs to be done by finding the nearest node to the invalid destination as just generating
	// new values over and over could get stuck long enough to frame out
}


bool
CAIInvestigate::AtDest( void ) const
{
	CPoint obPos( m_pobEnt->GetPosition() );

	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	CPoint obDiff = obPos - m_obDest;
	obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() < 4.0f)
	{
		return true;
	}

	return false;
}


bool CAIInvestigate::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );
 	const float	iInitTime			(0.3f);
	const float	iInvestigateTime	(20.0f);
	const float	iSeenTime			(0.4f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			ntPrintf("Investigate::STATE_INITIALISE\n");
			//OSD::Add( OSD::AISTATES, 0xffffffff, "Investigate::STATE_INITIALISE\n" );	
			m_fTimer = 0;

			// if we've seen the player (ever) then we need to act like it
			m_bFromChase = m_pobAIComp->HasSeenPlayer();

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > iInitTime )
			{
 				// if a message was received from a friendly go to alerted state

				// else if there are friendlies nearby, go to alerting state
                
				// else go to approach state
				SetState( STATE_APPROACH );
				
				// set a flag to indicate that, because we have just entered the search state,
				// we should investigate the last known player position first
				if (m_bFromChase)
				{
					// except if we've just been chasing them, in which case we can expect to already
					// be at their last known position
					m_bCheckPlayerPos = false;
				}
				else
				{
					m_bCheckPlayerPos = true;
				}
				m_iSearchTimer = 0;				
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_ALERTING )
		OnEnter
			ntPrintf("Investigate::STATE_ALERTING\n");
			OSD::Add( OSD::AISTATES, 0xffffffff, "Hey, I think I saw\n" );	
			OSD::Add( OSD::AISTATES, 0xffffffff, "something over there\n" );	
			//ntPrintf("Hey, I think I saw something over there\n");			
			m_fTimer = 0;
			
			// begin the "alerting" behaviour

		OnUpdate
			m_fTimer += fTimeChange;
			
			// continue alerting

			// if we've finished alerting our allies, go to the approach state
			if (0)
			{
				SetState( STATE_APPROACH );
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_ALERTED )
		OnEnter
			ntPrintf("Investigate::STATE_ALERTED\n");
			m_fTimer = 0;

			// begin the alerted behaviour
		OnUpdate
			m_fTimer += fTimeChange;

			// continue alerted

			// if the alerted behaviour is done, pop the investigate behaviour off the stack
			if (0)
			{
				// pop
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_APPROACH )
		OnEnter
			// output state info
			ntPrintf("Investigate::STATE_APPROACH\n");
			if (m_bFromChase)
			{
				OSD::Add( OSD::AISTATES, 0xffffffff, "Maybe she's hiding" );
				OSD::Add( OSD::AISTATES, 0xffffffff, "over here" );
			}
			else if (m_bCheckPlayerPos)
			{
				OSD::Add( OSD::AISTATES, 0xffffffff, "I'll go check it out\n" );
			}
			else
			{
				OSD::Add( OSD::AISTATES, 0xffffffff, "I'll search" );
				OSD::Add( OSD::AISTATES, 0xffffffff, "over here" );
			}
			m_fTimer = 0;

			// create a position on the level to search
			GenerateSearchPos();

			if (m_bFromChase)
			{
				m_pobAIComp->SetAction( ACTION_RUN );
			}
			else
			{
				m_pobAIComp->SetAction( ACTION_INVESTIGATE_WALK );
			}

			m_pobAIComp->SetActionDest( m_obDest );

		OnUpdate
			m_fTimer += fTimeChange;
			m_iSearchTimer++;

			if (m_pobAIComp->CanSeePlayer())
			{
				SetState( STATE_ENEMYSEEN );
			}
			if (AtDest())
			{
                SetState( STATE_LOOK );
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_LOOK )
		OnEnter
			ntPrintf("Investigate::STATE_LOOK\n");
			if (m_bFromChase)
			{
				OSD::Add( OSD::AISTATES, 0xffffffff, "She's got to be" );
				OSD::Add( OSD::AISTATES, 0xffffffff, "around here somewhere" );
			}
			else
			{
				OSD::Add( OSD::AISTATES, 0xffffffff, "Can't see anyone" );	
				OSD::Add( OSD::AISTATES, 0xffffffff, "around here" );	
			}
			m_fTimer = 0;
			m_pobAIComp->SetAction( ACTION_INVESTIGATE_LOOK );

		OnUpdate
			m_fTimer += fTimeChange;

			if (m_pobAIComp->CanSeePlayer())
			{
				SetState( STATE_ENEMYSEEN );
			}
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				// once we've had a good look around
				// if the investigate time is over then shrug
				if( m_iSearchTimer > iInvestigateTime )
				{
					SetState( STATE_SHRUG );
				}
				else
				{
					// otherwise, search somewhere else
					SetState( STATE_APPROACH );
				}
			}	
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_SHRUG )
		OnEnter
			ntPrintf("Investigate::STATE_SHRUG\n");
			if (m_bFromChase)
			{
				OSD::Add( OSD::AISTATES, 0xffffffff, "I'll get you" );
				OSD::Add( OSD::AISTATES, 0xffffffff, "next time, sucka!" );
			}
			else
			{
				OSD::Add( OSD::AISTATES, 0xffffffff, "I guess it was nothing\n" );	
			}
			m_fTimer = 0;
			m_pobAIComp->SetAction( ACTION_INVESTIGATE_SHRUG );

		OnUpdate
			m_fTimer += fTimeChange;
			if (m_pobAIComp->CanSeePlayer())
			{
				SetState( STATE_ENEMYSEEN );
			}
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				AI_BEHAVIOUR_SEND_MSG( INVESTIGATE_FOUNDNOTHING );
			}
			
		OnExit	
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_ENEMYSEEN )
		OnEnter
			ntPrintf("Investigate::STATE_ENEMYSEEN\n");
			OSD::Add( OSD::AISTATES, 0xffffffff, "Hey you! Intruder!\n" );	
			m_fTimer = 0;
			m_pobAIComp->SetAction( ACTION_NONE );

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > iSeenTime )
			{
				AI_BEHAVIOUR_SEND_MSG( INVESTIGATE_FOUNDTARGET );
			}
			
		OnExit	
			m_fTimer = 0;

EndStateMachine
}


