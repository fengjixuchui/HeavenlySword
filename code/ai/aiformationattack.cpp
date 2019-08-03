//!                                                                                         
//!	\file aiformationattack.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------

#include "ai/aiformationattack.h"
#include "ai/aiformation.h"
#include "ai/aiformationslot.h"
#include "ai/aiformationcomponent.h"
#include "ai/aiformationmanager.h"
#include "ai/aiformationxml.h"

#include "game/aicomponent.h"
#include "game/attacks.h"
#include "game/entityinfo.h"
#include "game/entitymanager.h"
#include "game/messagehandler.h"
#include "game/luaattrtable.h"
#include "game/luaexptypes.h"
#include "game/randmanager.h"

#include "core/visualdebugger.h"
#include "objectdatabase/dataobject.h"

#include "camera/camman.h"
#include "camera/camview.h"

#if defined(USER_gavin) && !defined(_RELEASE)
#define gromboPrintf	ntPrintf
#else
void gromboPrintf( ... ) { }
#endif

//------------------------------------------------------------------------------------------
// Constants                                                                                
//------------------------------------------------------------------------------------------

static const char * const EVENT_START_FORMATION_ATTACK = "StartFormationAttack";
static const char * const EVENT_COMBOENDED = "ComboEnded";
static const char * const EVENT_DOIT = "DoIt";
static const float		  DEFAULT_ONEONONE_TIME	= 15.0f;
static const float		  DEFAULT_ONEONONE_LOWNRG = 0.25f;

//------------------------------------------------------------------------------------------
// Start exposing the element to Lua
//------------------------------------------------------------------------------------------

LUA_EXPOSED_START(AIFormationAttack)
	LUA_EXPOSED_METHOD_GET(Paused, IsPaused,  "")
	LUA_EXPOSED_METHOD_SET(Paused, SetPaused, "")
LUA_EXPOSED_END(AIFormationAttack)


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIFormationAttackString::FindValid                                                    
//! From a given list of entities, find a match that'll suit the 
//! needs of the current attack query
//!                                                                                         
//------------------------------------------------------------------------------------------
AI* CAIFormationAttackString::FindValid( const ntstd::List<AI*>& obEntList, CEntity* pobPlayer, CEntity* pobLastFoundEnt )
{
//	if( !pobPlayer->GetKeywords()->ContainsAny( m_obTargetType ) )
//		return (0);

	int iWrongType = 0;

	for( ntstd::List<AI*>::const_iterator obIt = obEntList.begin();
			obIt != obEntList.end();
				++obIt )

	{
		AI* pobEnt = *obIt;

		if( !pobEnt->GetAIComponent() 
			|| !pobEnt->GetAIComponent()->GetAIFormationComponent()
			|| !pobEnt->GetAIComponent()->GetAIFormationComponent()->GetFormation() )
		{
			continue;
		}

		// Get the formation for this entity
		const AIFormation* pFormation = pobEnt->GetAIComponent()->GetAIFormationComponent()->GetFormation();

		// Is the type a match? If so, continue
		if( !pobEnt->GetKeywords().ContainsAny( m_obType ) )
		{
			++iWrongType;
			continue;
		}

		// Is the entity dead - if so, move on!
		if( pobEnt->ToCharacter()->IsDead() )
		{
			continue;
		}


		COMBAT_STATE eState = pobEnt->GetAttackComponent()->AI_Access_GetState();

		// make sure the entity is in a state to start an attack, and that the player isn't performing a strike
		if(	 !((CS_STANDARD == eState) || (CS_ATTACKING == eState))
			|| (pobPlayer->GetAttackComponent() == 0) 
			|| (pobPlayer->GetAttackComponent()->GetDisabled())
			|| (pobPlayer->GetAttackComponent()->AI_Access_GetCurrentAttackDataP() &&  pobPlayer->GetAttackComponent()->AI_Access_GetCurrentAttackDataP()->m_eAttackClass == AC_GRAB_GOTO)
			|| (pobPlayer->GetAttackComponent()->AI_Access_GetCurrentAttackDataP() &&  pobPlayer->GetAttackComponent()->AI_Access_GetCurrentAttackDataP()->m_eAttackClass == AC_GRAB_HOLD)
			|| (pobPlayer->GetAttackComponent()->AI_Access_GetCurrentAttackDataP() &&  pobPlayer->GetAttackComponent()->AI_Access_GetCurrentAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE) )
		{
			continue;
		}

		// 
		//if( !pobEnt->GetAIComponent()->IsSimpleActionComplete() )
		//	continue;
/*
		if( m_InGrombo && !pobEnt->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack()  )
			continue;
*/
		if( m_TimeNotInGrombo != 0.0f && m_TimeNotInGrombo > pobEnt->GetAIComponent()->GetAIFormationComponent()->GetTimeNotInGrombo()  )
				continue;

		
		const AIFormationSlot* pSlot = pFormation->FindSlot( *pobEnt );

		if( m_FormationPositionOffset > 0.0f )
		{
			// If there isn't a slot or the entity isn't in position, then contine to the next entity
			if( !pSlot || (m_FormationPositionOffset < ((pSlot->GetWorldPoint() - pobEnt->GetPosition()).Length()) ) )
				continue;
		}
		else if( !pSlot->IsInPosition() )
		{
			continue;
		}

		// Range check
		if( pobPlayer )
		{
			float DistanceToTarget = (pobEnt->GetPosition() - pobPlayer->GetPosition()).Length();
			if( DistanceToTarget > m_Distance  )
			{
				continue;
			}
		}

		if( m_AngleBits )
		{
			// No Player
			if( !pobPlayer ) continue;

			CDirection obEOf = CDirection(pobEnt->GetPosition() - pobPlayer->GetPosition());
			CDirection obDir = pobPlayer->GetMatrix().GetZAxis();

			// Normalise the vector
			obEOf.Normalise();

			bool	bFound	= false;
			float	fAngleD = TWO_PI / 8.0f;
            CMatrix obRot	= CMatrix( CDirection(0.0f, 1.0f, 0.0f), -fAngleD );
			float	fDotC = fcosf(fAngleD / 2.0f);

			for( int iAngleBitIndex = 0; iAngleBitIndex < 8; ++iAngleBitIndex, obDir = obDir * obRot )
			{
				// Angle not good, continue then
				if( !(m_AngleBits & (1 << iAngleBitIndex)) )
					continue;

				// Work out the angle from the current 
				float fDot = obDir.Dot( obEOf );

				if( fDot < 0.0f ) 
					continue;

				// Test the dot angle, check that it's within the bounds. If not continue
				if( fDot < fDotC )
					continue;

				bFound = true;
				break;
			}

			// Entity not found. just continue;
			if( !bFound ) continue;
		} 
		else if( m_CameraBits )
		{
			// No Player
			if( !pobPlayer ) continue;

			CDirection obEOf = CDirection(pobEnt->GetPosition() - pobPlayer->GetPosition());
			CDirection obDir = CamMan::GetPrimaryView()->GetCurrMatrix().GetZAxis();

			// Normalise the vector
			obEOf.Normalise();

			bool	bFound	= false;
			float	fAngleD = TWO_PI / 8.0f;
            CMatrix obRot	= CMatrix( CDirection(0.0f, 1.0f, 0.0f), -fAngleD );
			float	fDotC = fcosf(fAngleD / 2.0f);

			for( int iCameraBitIndex = 0; iCameraBitIndex < 8; ++iCameraBitIndex, obDir = obDir * obRot )
			{
				// Angle not good, continue then
				if( !(m_CameraBits & (1 << iCameraBitIndex)) )
					continue;

				// Work out the angle from the current 
				float fDot = obDir.Dot( obEOf );

				if( fDot < 0.0f ) 
					continue;

				// Test the dot angle, check that it's within the bounds. If not continue
				if( fDot < fDotC )
					continue;

				bFound = true;
				break;
			}

			// Entity not found. just continue;
			if( !bFound ) continue;
		} 
		else if( m_RelativeBits )
		{
			// No Player
			if( !pobPlayer || !pobLastFoundEnt ) continue;

			CDirection obEOf = CDirection(pobEnt->GetPosition() - pobPlayer->GetPosition());
			CDirection obDir = CDirection(pobPlayer->GetPosition() - pobLastFoundEnt->GetPosition());
			
			// Normalise the vector
			obEOf.Normalise();
			obDir.Normalise();

			bool	bFound	= false;
			float	fRelativeD = TWO_PI / 8.0f;
            CMatrix obRot	= CMatrix( CDirection(0.0f, 1.0f, 0.0f), -fRelativeD );
			float	fDotC = fcosf(fRelativeD / 2.0f);

			for( int iRelativeBitIndex = 0; iRelativeBitIndex < 8; ++iRelativeBitIndex, obDir = obDir * obRot )
			{
				// Relative not good, continue then
				if( !(m_RelativeBits & (1 << iRelativeBitIndex)) )
					continue;

				// Work out the angle from the current 
				float fDot = obDir.Dot( obEOf );

				if( fDot < 0.0f ) 
					continue;

				// Test the dot angle, check that it's within the bounds. If not continue
				if( fDot < fDotC )
					continue;

				bFound = true;
				break;
			}

			// Entity not found. just continue;
			if( !bFound ) continue;
		}

		// This entity seems to be a great match, return it
		return pobEnt;
	}

//	m_WrongType = iWrongType == (int)obEntList.size();

	return (0);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::AIFormationAttack                                                    
//! Constructor.                                                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationAttack::AIFormationAttack( FormationComponent* pParent, const CGromboInstance* pGromboInstance, const ntstd::String& rName )
{
	// Save the name of the attack
	m_Name = rName;

	// Save the parent
	m_pobParent = pParent;

	// Setup the grombo instance
	m_pGromboInstance = pGromboInstance;

	// Back in the old days of Lua, this 'AIFormationAttack' was built at run-time from Lua data but now the data is just built from 
	// XML. Therefore, this class 'AIFormationAttack' should be the item exposed to XML, but time and hassel has kept me from 
	// doing it. 
	m_Incidental					= pGromboInstance->m_bIncidental; 
	m_bAllowedToRunSimultaneously	= pGromboInstance->m_bSimultaneousAllowed; 
	m_Priority						= pGromboInstance->m_fPriority; 
	m_fWeighting					= pGromboInstance->m_fWeight;
	m_Interruptible					= pGromboInstance->m_fInterruptible; 
	m_obTargetType					= pGromboInstance->m_obTargetType; 
	m_OneOnOneAttacker				= CHashedString(pGromboInstance->m_obOneOnOneAttacker);
	m_obMetadata					= CKeywords( pGromboInstance->m_obMetadata.c_str() );

	// Zero out the delays
	m_bValid		= false;
	m_bPaused		= false;
	m_StartDelay	= 0.0f;
	m_EndDelay		= 0.0f;

	// State machine for the 
	m_pGromboStateMachine = 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::~AIFormationAttack                                                          
//! Destructor.                                                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationAttack::~AIFormationAttack()
{
	ATTACH_LUA_INTERFACE(AIFormationAttack);

	while(!m_obQuerys.empty())
	{
		CAIFormationAttackString* pQuery = m_obQuerys.back();
		m_obQuerys.pop_back();
		NT_DELETE_CHUNK( Mem::MC_AI, pQuery );
	}

	NT_DELETE_CHUNK( Mem::MC_AI, m_pGromboStateMachine );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine::AIGromboStateMachine
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
AIGromboStateMachine::AIGromboStateMachine( AIFormationAttack* pParent, const ntstd::List<AI*>& obEntities, const CGromboInstance* pGromboInstance )
	:	m_pFormationAttack( pParent ),
		m_pGromboInstance( pGromboInstance ),
		m_Entities( obEntities )
{
	gromboPrintf("------------------- AIGromboStateMachine (%s) --------------------\n", GetFormationAttack()->GetName().c_str() );
	m_ActorCount = m_Entities.size();

	// Create the grombo actors
	m_pActors = NT_NEW_ARRAY_CHUNK( Mem::MC_AI ) AIGromboActor[ m_ActorCount ];


	ntstd::List<AI*>::const_iterator obIt(m_Entities.begin());
	ntstd::List<CGromboEntity*, Mem::MC_AI>::const_iterator obGromboIt(pGromboInstance->m_obEntities.begin());

	// Set the grombo stage for each of the actors. 
	for( int Index = 0; Index < m_ActorCount; ++Index, ++obIt, ++obGromboIt )
	{
		m_pActors[Index].m_pEntity			= *obIt;
		m_pActors[Index].m_pGromboState		= this;
		m_pActors[Index].m_Id				= Index;
		m_pActors[Index].m_pGromboEntity	= *obGromboIt;

		// Initialise the grombo
		m_pActors[Index].Initialise();
	}

	// 
	m_bClosingDown = false;

	// Start and end delays
	m_StartDelay = 0.0f;
	m_EndDelay = 0.0f;

	// Running time
	m_RunningTime = 0.0f;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine::Update
//! Method for updating the grombo
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIGromboStateMachine::Update( float fTimeDelta )
{
	// If there is a start delay, use it. 
	if( m_StartDelay > 0.0f )
	{
		m_StartDelay -= fTimeDelta;

		return false;
	}

	// Is the grombo closing down?
	if( m_bClosingDown )
	{
		// If there is an end delay, use it. 
		if( m_EndDelay > 0.0f )
		{
			m_EndDelay -= fTimeDelta;
			return false;
		}

		// Formation attack finished
		return true;
	}

	// Update teh running time
	m_RunningTime += fTimeDelta;

	// counter up the number of finished actors. 
	int		FinishedCount = 0;
	bool	bError = false;

	// Test the players state.
	CEntity* pPlayer = CEntityManager::Get().GetPlayer();
	if( pPlayer )
	{
		COMBAT_STATE ePlayerState = pPlayer->GetAttackComponent()->AI_Access_GetState();
		
		if( CS_KO == ePlayerState
			|| CS_DEAD == ePlayerState
			|| CS_FLOORED == ePlayerState
			|| CS_RISE_WAIT == ePlayerState )
		{
			bError = true;
		}
	}

	// By default don't allow simultaneous attacks, only allow them if the one on attacker is koed
	m_AllowSimultaneousAttack = false;

	// Update each of the actors. 
	for( int Index = 0; Index < m_ActorCount; ++Index )
	{
		AIGromboActor* pActor = m_pActors + Index;

		if( bError )
		{
			pActor->SetEarlyExit();
		}

		if( !pActor->IsFinished() ) 
		{
			pActor->Update( fTimeDelta );
		}

		// Is there an error
		if( pActor->HasError() )
			bError = true;

		if( pActor->IsFinished() ) 
		{
			++FinishedCount;
		}

		// Allow simultaneous attacks if the entity is in one on one and is koed. The idea behind this is
		// allowing the combat scripter to make other 
		if( pActor->IsInOneOnOne() && pActor->m_pEntity->GetAttackComponent()->AI_Access_GetState() == CS_KO )
			m_AllowSimultaneousAttack = true;
	}


	// If all the actors are finished, then 
	if ( FinishedCount == m_ActorCount )
	{
		m_bClosingDown = true;
	}

	// 
	return false;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine::AttackStateEnded
//! 
//! Called when the attack for the given entity has finished
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIGromboStateMachine::AttackStateEnded( CEntity* pEntity )
{
	// Find the entity in the list of actors and call its AttackStateEnded
	for( int Index = 0; Index < m_ActorCount; ++Index )
	{
		if( m_pActors[Index].m_pEntity ==  pEntity )
		{
			m_pActors[Index].AttackStateEnded( );
			break;
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine::CombatStateChange
//! 
//! Called when the state of any of the attacking entities combat state changes. 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIGromboStateMachine::CombatStateChange( CEntity* pEntity, COMBAT_STATE eNewState )
{
	// Find the entity in the list of actors and call its AttackStateEnded
	for( int Index = 0; Index < m_ActorCount; ++Index )
	{
		if( m_pActors[Index].m_pEntity ==  pEntity )
		{
			m_pActors[Index].CombatStateChange( eNewState );
			break;
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine::RemoveEntity
//! 
//! Called when the state of any of the attacking entities combat state changes. 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIGromboStateMachine::RemoveEntity( CEntity* pEntity )
{
	// Find the entity in the list of actors and call its AttackStateEnded
	for( int Index = 0; Index < m_ActorCount; ++Index )
	{
		if( m_pActors[Index].m_pEntity ==  pEntity )
		{
			m_pActors[Index].Exit(false);
			break;
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine::OneOnOneAttackTest
//! 
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
CEntity* AIGromboStateMachine::OneOnOneAttackTest( const CHashedString& robAttackName ) const
{
	// Find the entity in the list of actors and call its AttackStateEnded
	for( int Index = 0; Index < m_ActorCount; ++Index )
	{
		if( m_pActors[Index].IsInOneOnOne() )
		{
			CEntity* pEntity = m_pActors[Index].m_pEntity;
			const CAttackData* pAttackData = m_pActors[Index].m_pEntity->GetAttackComponent()->AI_Access_GetCurrentAttackDataP();

			if( pAttackData && robAttackName == pAttackData->m_obAttackAnimName )
				return pEntity;
		}
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine::CanEntityAttack
//! 
//! Is the entity in a state of being able to attack in a non-formation context?
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIGromboStateMachine::CanEntityAttack( CEntity* pEntity )
{
	// Find the entity in the list of actors and call its AttackStateEnded
	for( int Index = 0; Index < m_ActorCount; ++Index )
	{
		if( m_pActors[Index].m_pEntity ==  pEntity )
		{
			return m_pActors[Index].CanAttack();
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboStateMachine::CanEntityBlock
//! 
//! Is the entity in a state of being able to attack in a non-formation context?
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIGromboStateMachine::CanEntityBlock( CEntity* pEntity )
{
	// Find the entity in the list of actors and call its AttackStateEnded
	for( int Index = 0; Index < m_ActorCount; ++Index )
	{
		if( m_pActors[Index].m_pEntity ==  pEntity )
		{
			return m_pActors[Index].CanBlock();
		}
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboActor::Initialise
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIGromboActor::Initialise(void)
{
	// Set the delay until the attack starts. 
	m_fDelay = m_pGromboEntity->m_fDelay; // m_obGromboStage.GetOpt<float>("Delay", 0.0f);

	// Nope not finished yet - in fact, just started
	m_bFinished = false;
	m_bError = false;
	m_bEarlyExit = false;

	// Set the next state
	ntAssert( m_pGromboEntity );
	m_pNextAttack = m_pGromboEntity->m_pobStartState;
	m_pAttackData = 0;
	ntAssert( m_pNextAttack != NULL );

	// One on One setup
	m_OneOnOneState = false;
	m_UntilKO = false;
	m_UntilRecoiling = false;
	m_Attacking = false;
	m_FaceTarget = false;
	m_Timeout = 0.0f;
	m_UntilRangeSqrd = FLT_MAX;

	// 
	m_ActiveAttack = 0;

	// Blocking State. 
	m_Blocking = false;

	// Set up the early attack range. 
	m_EarlyAttackRange = -1.f;

	// Check the state is valid. 
	//ntAssert_p( m_obGromboStage[ m_State.c_str() ].IsTable(), ("Table %s is not defined for current grombo %s\n", m_State.c_str(), m_pGromboState->GetFormationAttack()->GetName().c_str() ) );

	// Get the message handler
	ntAssert( m_pEntity && m_pEntity->GetMessageHandler() );

	if( m_fDelay == 0.0f )
	{
		m_pEntity->GetMessageHandler()->Receive( CMessageHandler::Make( m_pEntity, "msg_formation_enter_attack" ) );
		m_Attacking = true;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboActor::Exit
//! 
//! bool bError: Is the exit due to an error (default false)
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIGromboActor::Exit(bool bError)
{
	// Save the error flag
	m_bError = m_bError ? m_bError : bError;

	if( m_bFinished )
		return;

	m_bFinished = true;

	// If still in 1n1 combat
	if( m_OneOnOneState )
	{
		m_OneOnOneState = false;
		m_UntilKO = false;
		m_UntilRecoiling = false;
		m_UntilRangeSqrd = FLT_MAX;

		m_pEntity->GetAIComponent()->GetAIFormationComponent()->SetNoFormationUpdate( false );
		m_pEntity->GetMessageHandler()->Receive( CMessageHandler::Make( m_pEntity, "msg_formation_exit_1n1" ) );
	}

	// Get the message handler
	ntAssert( m_pEntity && m_pEntity->GetMessageHandler() );

	// Send the entity to enter a formation attack state
	if( m_Attacking )
	{
		m_pEntity->GetMessageHandler()->Receive( CMessageHandler::Make( m_pEntity, "msg_formation_exit_attack" ) );
		m_Attacking = false;
	}

	//Sanity checks... 
	ntAssert( m_pEntity->GetAIComponent() );
	ntAssert( m_pEntity->GetAIComponent()->GetAIFormationComponent() );

	const AIFormation* pFormation = m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation();
//	ntAssert( pFormation );// Sanity check..

	if (pFormation)
	{
		const AIFormationSlot* pFormationSlot = pFormation->FindSlot(*m_pEntity);
//		ntAssert( pFormationSlot );// Sanity check..
		
		// Mark that the entity isn't in position any longer.
		if (pFormationSlot)
			pFormationSlot->SetInPosition(false);
	}

	// Clear out the formation attack
	m_pEntity->GetAIComponent()->GetAIFormationComponent()->SetFormationAttack( 0 );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboActor::Update
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIGromboActor::Update(float fTimeDelta)
{
	// Sanity checks
	ntError( m_pEntity != NULL );
	ntError( m_pEntity->GetAIComponent() != NULL );
	ntError( m_pEntity->GetAttackComponent() != NULL );
	ntError( m_pEntity->GetAIComponent()->GetAIFormationComponent() != NULL );

	// If the entity is blocking an attack, where a blocking state can only have occured during the 
	// course of the grombo, then wait until the block has compelted and exit the grombo attack
	// for the actor. 
	if( m_Blocking )
	{
		// If not blocking any longer, exit the attack. 
		if( m_pEntity->GetAttackComponent()->AI_Access_GetState() != CS_BLOCKING )
		{
			m_Blocking = false;
			m_pNextAttack = 0;
			Exit();
		}

		return;
	}

	if( m_FaceTarget 
		&& m_pEntity->GetAIComponent()->GetAIFormationComponent()
		&& m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation() )
	{
		CDirection dirFacing( m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation()->GetLockonTarget() - m_pEntity->GetPosition() );
		dirFacing.Normalise();
		m_pEntity->GetAIComponent()->SetActionFacing( dirFacing );
	}


	// If there is a delay, use it. 
	if( m_fDelay > 0.0f )
	{
		if( (m_fDelay -= fTimeDelta) > 0.0f )
		{
			return;
		}

		if( !m_Attacking )
		{
			m_pEntity->GetMessageHandler()->Receive( CMessageHandler::Make( m_pEntity, "msg_formation_enter_attack" ) );
			m_Attacking = true;
		}
	}

	// Is the entity in a one one one state?
	if( m_OneOnOneState )
	{
		COMBAT_STATE eCombatState = m_pEntity->GetAttackComponent()->AI_Access_GetState();

		// Get the distance of the entity to the target. 
		float fDistToTargetSqrd = 0.0f;
		if( m_pEntity->GetAIComponent()->GetAIFormationComponent() 
			&& m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation() )
		{
			fDistToTargetSqrd = (m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation()->GetLockonTarget() - m_pEntity->GetPosition()).LengthSquared();
		}

		// Has the one on one finished?
		if( ((m_Timeout -= fTimeDelta) <= 0.0f)
			|| m_bError
			|| m_bEarlyExit
			|| fDistToTargetSqrd >= m_UntilRangeSqrd
			|| (m_UntilKO && ((CS_KO == eCombatState)||(CS_FLOORED == eCombatState)||(CS_RISE_WAIT == eCombatState)) ) 
			|| (m_UntilRecoiling && (CS_RECOILING == eCombatState) ) 
			|| ((CS_DYING == eCombatState) || (CS_DEAD == eCombatState)) )
		{
			m_OneOnOneState = false;
			m_UntilKO = false;
			m_UntilRecoiling = false;
			fDistToTargetSqrd = FLT_MAX;

			// Should be set from the script. 
			m_pEntity->GetAIComponent()->GetAIFormationComponent()->SetNoFormationUpdate( false );

			// Send the entity an exit one on one message
			m_pEntity->GetMessageHandler()->Receive( CMessageHandler::Make( m_pEntity, "msg_formation_exit_1n1" ) );

			// 
			if(	   (m_Timeout -= fTimeDelta) <= 0.0f 
				&&	m_pAttackData
				&&  m_pAttackData->m_pobNextState
				&& !m_bError 
				&& !((CS_DYING == eCombatState) || (CS_DEAD == eCombatState))
				&& !m_pEntity->ToCharacter()->IsDead() )
			{
				m_pNextAttack = m_pAttackData->m_pobNextState;
				ProcessStateChange();
			}
			else
			{
				// End the attack
				AttackStateEnded();
			}
		}
	}
	else
	{

	}
	
	// Is there an error?
	if( m_bError )
	{
		if( !m_pAttackData )
		{
			m_pAttackData = 0;
			m_pNextAttack = 0;
			Exit();
			return;
		}
	}
	else if( m_bEarlyExit )
	{
		m_pAttackData = 0;
		m_pNextAttack = 0;
		Exit();
		return;
	}

	// If setup for an early attack range, poll the distance to the target 
	/*
	if( m_EarlyAttackRange >= 0.0f )
	{
		if( m_State == GC_GOTO_STATE )
		{
			ntAssert( CEntityManager::Get().GetPlayer() );
			float fDistanceToTarget = (m_pEntity->GetPosition() - CEntityManager::Get().GetPlayer()->GetPosition()).Length();

			if( fDistanceToTarget <= m_EarlyAttackRange )
			{
				gromboPrintf("Grombo message: m_EarlyAttackRange:%f, Distance:%f\n", m_EarlyAttackRange, fDistanceToTarget );
				AttackStateEnded();
			}
		}
	}
	*/

	if( !m_pAttackData )
		ProcessStateChange();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboActor::ProcessStateChange
//! 
//! Process the next stage of the actors grombo
//!
//------------------------------------------------------------------------------------------

void AIGromboActor::ProcessStateChange(void)
{
	// Possible reasons for an early exit... 
	// 1, dead, 
	// 2, ko'od

	COMBAT_STATE eCombatState = m_pEntity->GetAttackComponent()->AI_Access_GetState();

	// Don't let the grombo continue if... 
	if( (CS_DYING == eCombatState) || (CS_DEAD == eCombatState) )
	{
		Exit();
		return;
	}

	// Don't let the grombo continue if.. 
	if( !m_pAttackData && CS_KO == eCombatState )
	{
		Exit();
		return;
	}

	if( !m_OneOnOneState && m_pAttackData && ( CS_ATTACKING != eCombatState ) )
	{
		if( !((CS_KO == eCombatState && m_pAttackData->m_pobNextStateIfKoed) ||
			(CS_RECOILING == eCombatState && m_pAttackData->m_pobNextStateIfRecoiled ) ||
			(CS_BLOCKING == eCombatState && m_pAttackData->m_pobNextStateIfBlocked )) )
		{
			// Don't let the grombo continue
			Exit();
			return;
		}
	}

	// Don't let the grombo continue if a special exit state is defined.
	if( m_pNextAttack == 0 )
	{
		Exit();
		return;
	}

	// Obtain the defined lua state
	m_pAttackData = m_pNextAttack;
	m_pNextAttack = 0;

	// Make sure the state used is a table, if not there is likely a script problem.
	//ntError_p( m_DefState.IsTable(), ("Table %s is not defined for current grombo %s\n", m_State.c_str(), m_pGromboState->GetFormationAttack()->GetName().c_str() ) );

	ntAssert( CEntityManager::Get().GetPlayer() );
	float fDistanceToTarget = (m_pEntity->GetPosition() - CEntityManager::Get().GetPlayer()->GetPosition()).Length();

	// Set up the early attack range. 
	m_EarlyAttackRange = m_pAttackData->m_fEarlyAttackRange; // m_DefState.GetOpt( "early_attack_range", -1.0f) ;

	// Turn to face the target
	m_FaceTarget = m_pAttackData->m_bFaceTarget; // m_DefState.GetOpt( "face_target", false );

	// 
	m_ActiveAttack = 0;
	GromboAttackList::const_iterator obItEnd = m_pAttackData->m_obAttacks.end();
	
	GromboAttackList::const_iterator obIt( m_pAttackData->m_obAttacks.begin() );

#ifndef _RELEASE
	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromPointer(m_pAttackData);
	one_time_assert_p( 0xBAD102, pDataObject != NULL, ("Not valid formation attack") );
	gromboPrintf("New grombo state: %s\n",  ntStr::GetString(ObjectDatabase::Get().GetDataObjectFromPointer(m_pAttackData)->GetName()) );
#endif

	if( obIt != obItEnd )
	{
		int AnimCount = (int) m_pAttackData->m_obAttacks.size();
		Array<float, 32> Weights;
		float WeightTotal = 0.0f;

		// Find the anim that we'll be playing
		for( int Index = 0; obIt != obItEnd; ++obIt, ++Index )
		{
			// Clear the first weight
			Weights[ Index ] = 0.0f;
		
			const CGromboAttack* pAttackData = *obIt;

			// Get the name of the animation to test. 
			const ntstd::String& robAnimName = pAttackData->m_obAnimName; // rAnimEntry["anim"].GetString();

			// Is the animation a failure animation?
			if( strstr( robAnimName.c_str(), "fail") )
			{
				// If no attack is picked, then choose the failure
				if( WeightTotal == 0.0f )
				{
					m_ActiveAttack = obIt;
					break;
				}
				
				continue;
			}

			if( !m_bError )
			{
				// Get the range and tolerance for the anim to play
				float Range = pAttackData->m_fRange; // rAnimEntry.GetOpt<float>( "range", 0.0f );
				float Tolerance = pAttackData->m_fTolerance; // rAnimEntry.GetOpt<float>( "tolerance", 1000.0f );

				if( (Range - Tolerance) <= fDistanceToTarget && (Range + Tolerance) > fDistanceToTarget )
				{
					ntAssert( pAttackData->m_fWeight > 0.0f );
					Weights[ Index ] = pAttackData->m_fWeight; 
					WeightTotal	+= Weights[ Index ];
				}
			}
		}

		// 
		if( WeightTotal > 0.0f )
		{
			float ChoosenWeight = grandf( WeightTotal );
			WeightTotal = 0.0f;
			obIt = m_pAttackData->m_obAttacks.begin();

			for( int Index = 0; Index <= AnimCount; ++Index, ++obIt )
			{
				WeightTotal += Weights[ Index ];
				if( ChoosenWeight <= WeightTotal )
				{
					m_ActiveAttack = obIt;
					break;
				}
			}
		}

		// Did we find a valid animation to play? 
		if( m_ActiveAttack == 0 || m_ActiveAttack == obItEnd )
		{
			// Nope, don't let the formation start
			gromboPrintf("Couldn't find a valid anim for grombo: %s\n", m_pGromboState->GetFormationAttack()->GetName().c_str() );
			Exit(true);
			return;
		}

		// Get the animation name to play
		const ntstd::String& robAnimName = (*m_ActiveAttack)->m_obAnimName;

		// Get the player entity pointer
		CEntity* pTarget = CEntityManager::Get().GetPlayer();

		// Get the targets current state
		COMBAT_STATE eTargetState = pTarget->GetAttackComponent()->AI_Access_GetState();

		// The targets current state is one of the following... then return now.
		if (eTargetState == CS_KO 
			|| eTargetState == CS_FLOORED 
			|| eTargetState == CS_RISE_WAIT 
			|| eTargetState == CS_HELD 
			|| eTargetState == CS_DYING 
			|| eTargetState == CS_DEAD 
			|| (m_pEntity->GetAttackComponent()->AI_Access_GetStruckStrike() && m_pEntity->GetAttackComponent()->AI_Access_GetStruckStrike()->IsCounter())
//			|| (pTarget->GetAttackComponent() && pTarget->GetAttackComponent()->AI_Access_IsPerformingCounter())
			|| (pTarget->GetAttackComponent() && pTarget->GetAttackComponent()->AI_Access_GetCurrentAttackDataP() && ( (pTarget->GetAttackComponent()->AI_Access_GetCurrentAttackDataP()->m_eAttackClass == AC_GRAB_GOTO)
																													|| (pTarget->GetAttackComponent()->AI_Access_GetCurrentAttackDataP()->m_eAttackClass == AC_GRAB_HOLD)
																													|| (pTarget->GetAttackComponent()->AI_Access_GetCurrentAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE) ) ) )
		{
			Exit();
			return;
		}
		
		// 
		gromboPrintf("Choosing grombo stage:%s Distance:%f weight:%f on:%s\n", (*m_ActiveAttack)->m_obAnimName.c_str(), (m_pEntity->GetPosition() - pTarget->GetPosition()).Length(),  WeightTotal, m_pEntity->GetName().c_str() );

		// Direct attack.. was it successful?
		if( !m_pEntity->GetAIComponent()->GetCombatComponent().DirectAttack( robAnimName.c_str() ) )
		{
			// Nope - post a failure back to to the formation.. 
			Exit(true);
			return;
		}

		if( strstr( robAnimName.c_str(), "fail") )
		{
			gromboPrintf("Grombo Animname: %s marked as a failure\n", robAnimName.c_str() );
			m_bError = true;
		}
		else
		{
			// Is the anim marked as a failure?
			//m_bError = rAnimList[m_ActiveAnimIndex].GetOpt( "Failed", false );
		}
	}
	else if( m_pAttackData->m_bEnter1on1 )
	{
		// If there is a new ai attack tree
		if( m_pAttackData->m_pNewAIAttackTree )
		{
			// Set the new attack tree.
			m_pEntity->GetAIComponent()->GetCombatComponent().SetNewAttackTree(m_pAttackData->m_pNewAIAttackTree);
		}

		ntError_p( m_fDelay <= 0.0f, ("Entered one on one whilst in the delayed stage could leed to problems!" ) );

		// If not currently in formation attack, do so to prevent some problems that could occur
		if( m_fDelay > 0.0f )
		{
			m_fDelay = 0.0f;
			m_pEntity->GetMessageHandler()->Receive( CMessageHandler::Make( m_pEntity , "msg_formation_enter_attack" ) );
			m_Attacking = true;
		}

		// Get the one on one state
		m_OneOnOneState = true; //m_DefState.Get<bool>("enter1n1");
		
		// Should the NPC 1n1 until Koed?
		m_UntilKO = m_pAttackData->m_bUntilKoed; // m_DefState.GetOpt("untilKO", false);
		m_UntilRecoiling = m_pAttackData->m_bUntilRecoiling; //m_DefState.GetOpt("untilRecoiling", false);
		m_UntilRangeSqrd = m_pAttackData->m_fUntilDist * m_pAttackData->m_fUntilDist; //m_DefState.GetOpt("untilRange", FLT_MAX );
		m_Timeout = m_pAttackData->m_fTimeout; // m_DefState.GetOpt("timeout", 15.0f);

		// Should be set from the script. 
		m_pEntity->GetAIComponent()->GetAIFormationComponent()->SetNoFormationUpdate( true );

		// Enter a 1n1 state... 
		m_pEntity->GetMessageHandler()->Receive( CMessageHandler::Make( m_pEntity, "msg_formation_enter_1n1" ) );
	}
	else if( m_pAttackData->m_obSendMsg.length() )
	{
		// Get the message subject and send the message on
		m_pEntity->GetMessageHandler()->Receive( CMessageHandler::Make( m_pEntity, m_pAttackData->m_obSendMsg.c_str() ) );

		// Clear the current state, set the next state for the grombo
		m_pNextAttack = m_pAttackData->m_pobNextState;
		m_pAttackData = NULL;
		
		if( !m_pNextAttack )
		{
			Exit();
		}
	}
	else if( m_pAttackData->m_obSimpleAnim.length() )
	{
		// Get the message subject and send the message on...
		if( m_pEntity->GetAIComponent()->ActivateSingleAnim( m_pAttackData->m_obSimpleAnim.c_str() ) )
		{
			m_fDelay = m_pEntity->GetAIComponent()->TimeRemainingOnSingleAnim();
		}

		// Clear the current state, set the next state for the grombo
		m_pNextAttack = m_pAttackData->m_pobNextState;
		m_pAttackData = NULL;
	}
	else
	{
		m_fDelay = m_pAttackData->m_fDelay; // m_DefState.GetOpt<float>("delay", 0.0f);
		
		// Clear the current state, set the next state for the grombo
		m_pNextAttack = m_pAttackData->m_pobNextState;
		m_pAttackData = NULL;
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboActor::AttackStateEnded
//! 
//! Called when the attack for the actor has completed
//!
//------------------------------------------------------------------------------------------

void AIGromboActor::AttackStateEnded( )
{
	if( m_fDelay > 0.0f )
		return;

	if( m_OneOnOneState )
		return;

	// Attack completed... choose the next phase in the attack.
	if( m_ActiveAttack != 0 && *m_ActiveAttack )
	{
		// Clear the current state, set the next state for the grombo
		m_pNextAttack = (*m_ActiveAttack)->m_pobNextState;
		m_pAttackData = NULL;
	}
	else
	{
		m_pNextAttack = NULL;
		m_pAttackData = NULL;
	}

	// No next state, mark the actor as finished
	if( m_pNextAttack == NULL )
	{
		Exit();
		return;
	}

	// Process the next stage of the actors grombo
	ProcessStateChange();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIGromboActor::CombatStateChange
//! 
//! Called when the state of the attack entities combat state changes
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIGromboActor::CombatStateChange( COMBAT_STATE eNewState )
{
	if( m_OneOnOneState )
		return;

	// 
	if( eNewState == CS_BLOCKING )
	{
		m_Blocking = true;
		return;
	}

	if( (m_fDelay <= 0.0f) && !(eNewState == CS_ATTACKING || eNewState == CS_BLOCKING) && !m_bFinished )
	{
		if( !m_pAttackData )
		{
			ProcessStateChange();
			
			if( !m_pAttackData )
			{
				m_pNextAttack = NULL;
				Exit();
			}
		}

		if( CS_KO == eNewState && m_pAttackData && m_pAttackData->m_pobNextStateIfKoed )
		{
			m_pNextAttack = m_pAttackData->m_pobNextStateIfKoed;
			ProcessStateChange();
		}
		else if( CS_RECOILING == eNewState && m_pAttackData && m_pAttackData->m_pobNextStateIfBlocked )
		{
			m_pNextAttack = m_pAttackData->m_pobNextStateIfBlocked;
			ProcessStateChange();
		} 
		else if( CS_BLOCKING == eNewState && m_pAttackData && m_pAttackData->m_pobNextStateIfRecoiled )
		{
			m_pNextAttack = m_pAttackData->m_pobNextStateIfRecoiled;
			ProcessStateChange();
		} 
		else
		{
			m_pNextAttack = NULL;
			Exit();
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::IsInterruptible                                                               
//! Can this aifromation attack be interrupted?
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIFormationAttack::IsInterruptible(void) const 
{ 
	// If the attack isn't interruptible, then return false now. 
	if( m_Interruptible == FLT_MAX )
		return false;

	// If there isn't a state machine running, then return true now.
	if( !m_pGromboStateMachine )
		return true;

	return m_pGromboStateMachine->GetUpTime() >= m_Interruptible;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::Initiate                                                               
//! Begin this attack
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIFormationAttack::Initiate( void )
{
	// 
	if( (int) m_ValidEntList.size() != StringCount() )
		return;

	// 
	for( ntstd::List<AI*>::const_iterator obIt( m_ValidEntList.begin() ); obIt != m_ValidEntList.end(); ++obIt )
	{
		AI* pEntity = *obIt;

		//Sanity checks... 
		ntAssert( pEntity );
		ntAssert( pEntity->GetAIComponent() );
		ntAssert( pEntity->GetAIComponent()->GetAIFormationComponent() );


		// Mark the entity with the entity attack.
		pEntity->GetAIComponent()->GetAIFormationComponent()->SetFormationAttack( this );
	}


	// Send word to the formation entity that an attack has started. 
	m_pobParent->GetParent()->GetMessageHandler()->Receive( CMessageHandler::Make( m_pobParent->GetParent(), "msg_formation_attack_started", GetName().c_str() ) );

	// Create a new grombo state machine
	ntAssert( m_pGromboStateMachine == NULL );
	m_pGromboStateMachine = NT_NEW_CHUNK( Mem::MC_AI ) AIGromboStateMachine( this, m_ValidEntList, m_pGromboInstance );

	m_pGromboStateMachine->SetStartDelay(m_StartDelay);
	m_pGromboStateMachine->SetEndDelay(m_EndDelay);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	 AIFormationAttack::RemoveEntity
//! 
//!  Remove an entity from the list.                                                                                         
//! 
//------------------------------------------------------------------------------------------
void AIFormationAttack::RemoveEntity( CEntity* pEnt )
{
	if( m_pGromboStateMachine )
		m_pGromboStateMachine->RemoveEntity( pEnt );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	 AIFormationAttack::EndCurrentAttack
//! 
//!  Remove an entity from the list.                                                                                         
//! 
//------------------------------------------------------------------------------------------
void AIFormationAttack::EndCurrentAttack( void )
{
	// Send word to the formation entity that an attack has ended 
	m_pobParent->GetParent()->GetMessageHandler()->Receive( CMessageHandler::Make( m_pobParent->GetParent(), "msg_formation_attack_ended", GetName().c_str() ) );

	NT_DELETE_CHUNK( Mem::MC_AI, m_pGromboStateMachine );
	m_pGromboStateMachine = 0;
}

//------------------------------------------------------------------------------------------
//!	 AIFormationAttack::Validate
//! 
//!  Passed a list of entities that allows the attack to check if it can run
//------------------------------------------------------------------------------------------
bool AIFormationAttack::Validate( ntstd::List<AI*>& rEntList, const AIFormationAttack::List& rActiveAttacks )
{
	// Mark the attack as invalid to start with.
	m_bValid = false;
	m_pcValidFailedReason = "";

	// Clear out the valid entities list. 
	m_ValidEntList.clear();

	// Check the players combat state
	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	if( !pobPlayer )
	{
		m_pcValidFailedReason = "No Player";
		//SetAttackState( AFAS_WAITING_FOR_VALID_PLAYER );
		return false;
	}

	// If the metadata is a complete match
	if( /* m_obMetadata.GetKeywordCount() && */ m_pobParent->GetMetadta().GetKeywordCount() && m_obMetadata.ContainsAll( m_pobParent->GetMetadta() ) )
	{
		m_pcValidFailedReason = "metadata match failed. ";
		return false;
	}

	COMBAT_STATE eState = pobPlayer->GetAttackComponent()->AI_Access_GetState();
	if(    eState != CS_ATTACKING 
		&& eState != CS_STANDARD
		&& eState != CS_DEFLECTING
		&& eState != CS_RECOVERING
		&& eState != CS_BLOCKING )
	{
		// SetAttackState( AFAS_WAITING_FOR_PLAYER_TO_GETUP );
		m_pcValidFailedReason = "Player down";
		return false;
	}

	// If this attack is active, then ignore it. 
	if( m_pGromboStateMachine )
	{
		m_pcValidFailedReason = "Already playing";
		return false;
	}

	// Assume that if no attacks are running then 
	bool bOnlySimultaneousRunning = true;

	// Check the simultaneous state of the current running attacks.
	for( AIFormationAttackList::const_iterator obIt = rActiveAttacks.begin(); 
			obIt != rActiveAttacks.end();
				++obIt )
	{
		if( !(*obIt)->AllowedToRunSimultaneously() )
		{
			bOnlySimultaneousRunning = false;
			break;
		}
	}

	if( rActiveAttacks.size() > 2 || ((!AllowedToRunSimultaneously() && rActiveAttacks.size()) && !bOnlySimultaneousRunning ) )
	{
		m_pcValidFailedReason = "Not Simultaneous";
		// Nope, the other attack(s) must be stopped. 
		return false;
	}

	// Can the attack run simultaneously with another?
	if( rActiveAttacks.size() > 2 && !m_Incidental )
	{
		m_pcValidFailedReason = "Too many attacks";
		// Nope, the other attack(s) must be stopped. 
		return false;
	}

	if( !m_Incidental )
	{
		// Check the priorities of the current attacks.
		for( AIFormationAttackList::const_iterator obIt = rActiveAttacks.begin(); 
				obIt != rActiveAttacks.end();
					++obIt )
		{
			AIFormationAttack* pActiveAttack = *obIt;

			// Don't even consider the formation attack if it's already playing. 
			if( pActiveAttack == this )
				return false;

			// If the attack is an incidental, then ingore this entry
			if( pActiveAttack->m_Incidental )
				continue;

			// If the current attack isn't interruptable or the priority of the testing attack is less  100 then don't check this attack
			if( !pActiveAttack->IsInterruptible() ) //  && (m_bAllowedToRunSimultaneously && GetPriority() < 100.0f)
			{
				m_pcValidFailedReason = "NonInterruptible Attack";
				return false;
			}

			// If there is a current attack, and the priority of the test attack is less than or equal to the 
			// current attack, then just return now. 
			if( GetPriority() < pActiveAttack->GetPriority() )
			{
				m_pcValidFailedReason = "Priority low";
				return false;
			}

			/*
			if( pActiveAttack->m_pGromboStateMachine && !pActiveAttack->m_pGromboStateMachine->AllowSimultaneousAttack() )
			{
				m_pcValidFailedReason = "Not simultaneous";
				return false;
			}
			*/
		}
	}

	// Test the debug attack context - debug
	if( (AIFormationManager::Get().GetDebugContext() != -1) && (GetDebugContext() != AIFormationManager::Get().GetDebugContext()) )
	{
		m_pcValidFailedReason = "Not active context";
		return false;
	}

	// Is there a One on One attack requirement?
	if( IsOneOnOneAttackRequirement()  )
	{
		if( !rActiveAttacks.size() )
		{
			m_pcValidFailedReason = "1on1 check missing data";
			return false;
		}

		// Obtain the attack requirement test. 
		const CHashedString& robAttackReq = OneOnOneAttackRequirement();

		for( AIFormationAttackList::const_iterator obIt = rActiveAttacks.begin(); 
				obIt != rActiveAttacks.end();
					++obIt )
		{
			// Test the attack of any of the current attackers in the grombo, if there is 
			// match, then the return value is the entity that match was found on
			CEntity* pEntity = (*obIt)->OneOnOneAttackTest( robAttackReq );

			// If there wasn't a match, then return now. 
			if( !pEntity )
			{
				m_pcValidFailedReason = "1on1 no valid entity";
				return false;
			}
		}
	}
/*
	// Is there a player combo requirment?
	if( !GetPlayerComboReq().IsNil() )
	{
		const AttackHistory& rAttackHistory = pobPlayer->GetAttackComponent()->AI_Access_History();
		const AttackHistory::AttackHistoryData* pHistoryData;
		const NinjaLua::LuaObject& rComboReq = GetPlayerComboReq();
		int ComboReqSize = rComboReq.GetSize();

		// Iterate over all the elements in the history checking for matches
		for( int Index = -1; ComboReqSize && (pHistoryData = rAttackHistory[Index]) != 0; --Index, --ComboReqSize )
		{
			// If history element isn't an attack start, then there wont be any combo string match
			if( pHistoryData->m_eType != AttackHistory::AH_START_ATTACK )
			{
				break;
			}

			// Sanity check
			ntError( pHistoryData->m_pAttackData != NULL );

			// Is the string valid?
			if( !rComboReq[ ComboReqSize ].IsString() )
			{
				break;
			}

			// Get the attack name
			const char* pcAttackName = rComboReq[ ComboReqSize ].GetString();

			// Compare the two, if there isn't a match, then break out now
			if( pHistoryData->m_pAttackData->m_obAttackAnimName != pcAttackName )
			{
				break;
			}
		}

		if( ComboReqSize )
			return false;
	}
	*/

	// The entity list obtain should be checked quickly that there are enough valid entities to satisfy the attack
	ntstd::List<AI*> obCheckList = rEntList;

	// Iterate over the list of entities. 
	for( AIFormationAttackStringList::const_iterator obIt(GetStringList().begin()); obIt != GetStringList().end(); ++obIt )
	{
		// Cache out the attack string
		const CAIFormationAttackString* pAttackString = *obIt;

		// Check each of the available entities. 
		for( ntstd::List<AI*>::iterator obCheckIt = obCheckList.begin(); obCheckIt != obCheckList.end(); ++obCheckIt )
		{
			CEntity* pEnt = *obCheckIt;

			// Is there a type match? If so, ...
			if( pEnt->GetKeywords().ContainsAny( pAttackString->m_obType ) )
			{
				// Remove the entity and move on to the next entity in the list. 
				obCheckList.erase( obCheckIt );
				pAttackString = 0;
				
				break;
			}
		}

		// If the attack string pointer is still valid then an entity wasn't found to fill the void and
		// the attack is not possible. 
		if( pAttackString )
		{
			// If the attack was once possible, then raise an event
			if( !IsImpossible() )
			{
				//RaiseEvent("NowImpossible");
			}

			// The attack is impossible to trigger. 
			SetImpossible( true );

			m_pcValidFailedReason = "not enough ents";
			return false;
		}

	}
	// Mark the attack as possible. 
	SetImpossible( false );


	for( u_int iCount = 0; iCount < rEntList.size(); ++iCount )
	{
		CEntity* pobLastFound = 0;
		bool WrongType = false;

		for( AIFormationAttackStringList::const_iterator obIt = m_obQuerys.begin();
				obIt != m_obQuerys.end(); 
					++obIt )
		{
			// Obtain the query
			CAIFormationAttackString* pobQuery = *obIt;
			
			// Find the first entity
			AI* pobFoundEnt = pobQuery->FindValid( rEntList, pobPlayer, pobLastFound );

			// If no entity is found, then break out of the attack query loop
			if( !pobFoundEnt ) 
			{
				//WrongType = pobQuery->m_WrongType;
				break;
			}

			// remember the last entity found
			pobLastFound = pobFoundEnt;

			// Remove the found entity from the found list, this will prevent the entity from being picked again
			rEntList.remove(pobFoundEnt);

			// Add the entity to the found list
			m_ValidEntList.push_back(pobFoundEnt);
		}

		// not a single good entity found, return now
		if( !m_ValidEntList.size() )
		{
			//SetAttackState( AFAS_NO_VALID_ENTITIES );
			break;
		}
		// Some entities were ok, but not enough. 
		else if( m_ValidEntList.size() < GetStringList().size() )
		{
			// Return the entities back into the ent list
			while( m_ValidEntList.size() )
			{
				AI* pobEnt = m_ValidEntList.back();
				m_ValidEntList.pop_back();
				rEntList.push_back( pobEnt );
			}

			// 
			//SetAttackState( AFAS_NOT_ENOUGH_VALID_ENTITIES );

			// This is a serious problem, there arn't enough entities
			if( WrongType )
			{
				//SetAttackState( AFAS_NOT_ENOUGH_ENTITIES );
				break;
			}
		}
		else
		{
			// Return the entities back into the ent list
			for( ntstd::List<AI*>::iterator obIt( m_ValidEntList.begin() ); obIt != m_ValidEntList.end(); ++obIt )
			{
				rEntList.push_back( *obIt );
			}

			m_bValid = true;
			break;
		}
	}

	if( !m_bValid )
	{
		m_pcValidFailedReason = "No matching ents";
	}

	return m_bValid;
}


//------------------------------------------------------------------------------------------
//!	 AIFormationAttack::Update
//! 
//!  Update the formation attack
//------------------------------------------------------------------------------------------
bool AIFormationAttack::Update(float fTimeDelta)
{
	if( m_pGromboStateMachine )
	{
		// Update the state machine controlling the actors. Has the state machine finished?
		if( m_pGromboStateMachine->Update( fTimeDelta ) )
		{
			// Clear out the attack.
			// GetFormation()->ClearAttack();
			CEntity* pEnt = m_pobParent->GetParent();

			if( m_OnCompleteMsg.length() && pEnt && pEnt->GetMessageHandler() )
				pEnt->GetMessageHandler()->Receive( CMessageHandler::Make( pEnt, m_OnCompleteMsg.c_str() ) );

			NT_DELETE_CHUNK( Mem::MC_AI,  m_pGromboStateMachine );
			m_pGromboStateMachine = 0;

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::CombatStateChanged
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIFormationAttack::CombatStateChanged( CEntity* pEntity, COMBAT_STATE eNewState )
{
	if( m_pGromboStateMachine )
		m_pGromboStateMachine->CombatStateChange( pEntity, eNewState );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::AttackStateEnd
//! 
//! Called when the entity has completed an attack 
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIFormationAttack::AttackStateEnded( CEntity* pEntity )
{
	if( m_pGromboStateMachine )
		m_pGromboStateMachine->AttackStateEnded( pEntity );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::CanEntityAttack
//! 
//! Called when the entity has completed an attack 
//!                                                                                         
//------------------------------------------------------------------------------------------

bool AIFormationAttack::CanEntityAttack( CEntity* pEntity )
{
	if( m_pGromboStateMachine )
		return m_pGromboStateMachine->CanEntityAttack( pEntity );

	return true;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::CanEntityBlock
//! 
//! Called when the entity has completed an attack 
//!                                                                                         
//------------------------------------------------------------------------------------------

bool AIFormationAttack::CanEntityBlock( CEntity* pEntity )
{
	if( m_pGromboStateMachine )
		return m_pGromboStateMachine->CanEntityBlock( pEntity );

	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationAttack::OneOnOneAttackTest
//! 
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

CEntity* AIFormationAttack::OneOnOneAttackTest( const CHashedString& robAttackName ) const
{
	if( m_pGromboStateMachine )
		return m_pGromboStateMachine->OneOnOneAttackTest( robAttackName );

	return (0);
}
