#include "entityaerialgeneral.h"
#include "editable/enumlist.h"
#include "game/entitymanager.h"
#include "objectdatabase/dataobject.h"
#include "core/timer.h"
#include "game/attacks.h"
#include "game/awareness.h"	
#include "Physics/system.h"
#include "Physics/compoundlg.h"
#include "physics/world.h"
#include "physics/advancedcharactercontroller.h"
#include "movement.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "anim/animator.h"
#include "game/strike.h"
#include "game/randmanager.h"
#include "messagehandler.h"
#include "game/interactioncomponent.h"
#include "core/visualdebugger.h"
#include "targetedtransition.h"
#include "simpletransition.h"
#include "continuationtransition.h"
#include "inputcomponent.h"
#include "camera/camutils.h"
#include "hud/hudmanager.h"
#include "game/syncdmovement.h"
#include "game/entityinteractablethrown.h"

START_STD_INTERFACE(AerialGeneral)
	COPY_INTERFACE_FROM(Boss)
	DEFINE_INTERFACE_INHERITANCE(Boss)

	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[0], DoppelgangerAGenOneEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[1], DoppelgangerAGenTwoEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[2], DoppelgangerAGenThreeEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[3], DoppelgangerAGenFourEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[4], DoppelgangerAGenFiveEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[5], DoppelgangerAGenSixEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[6], DoppelgangerAGenSevenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[7], DoppelgangerAGenEightEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[8], DoppelgangerAGenNineEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[9], DoppelgangerAGenTenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[10], DoppelgangerAGenElevenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[11], DoppelgangerAGenTwelveEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[12], DoppelgangerAGenThirteenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[13], DoppelgangerAGenFourteenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[14], DoppelgangerAGenFifteenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[15], DoppelgangerAGenSixteenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[16], DoppelgangerAGenSeventeenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[17], DoppelgangerAGenEighteenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[18], DoppelgangerAGenNineteenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[19], DoppelgangerAGenTwentyEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[20], DoppelgangerAGenTwentyOneEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[21], DoppelgangerAGenTwentyTwoEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[22], DoppelgangerAGenTwentyThreeEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[23], DoppelgangerAGenTwentyFourEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[24], DoppelgangerAGenTwentyFiveEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[25], DoppelgangerAGenTwentySixEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[26], DoppelgangerAGenTwentySevenEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[27], DoppelgangerAGenTwentyEightEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[28], DoppelgangerAGenTwentyNineEntityName)
	PUBLISH_VAR_AS(m_aobDoppelgangerAGenEntityNames[29], DoppelgangerAGenThirtyEntityName)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_obAGenSplitAnim, "agen_spcl_split_left", AGenSplitAnim) 
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDoppelgangerSplitAnim, "agen_spcl_split_right", DoppelgangerSplitAnim) 

	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumberOfDoppelgangersTillEnd, 12, NumberOfDoppelgangersTillEnd)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obAllDoppelgangersActiveMessage, "Trigger", AllDoppelgangersActiveMessage)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obAllDoppelgangersActiveMessageEntityName, "Seq_BossBattle_Fox_Battle", AllDoppelgangersActiveMessageEntityName)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_obArenaCentre, CPoint(CONSTRUCT_CLEAR), ArenaCentre)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fArenaRadius, 5.0f, ArenaRadius)

	//Note: These defaults are most definitely NOT final values, need tweaking so that the swords that disappear/appear when unholstering
	//or holstering a sword match up well with the animations. Exposed here so that they can be tweaked easier.
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iRightHandSwordBackTransform1, 1, RightHandSwordBackTransformRef1)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iRightHandSwordBackTransform2, 2, RightHandSwordBackTransformRef2)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iLeftHandSwordBackTransform1, 3, LeftHandSwordBackTransformRef1)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iLeftHandSwordBackTransform2, 4, LeftHandSwordBackTransformRef2)

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralPlayerRelativeVectorMovement)
	COPY_INTERFACE_FROM(BossWalkingMovement)
	DEFINE_INTERFACE_INHERITANCE(BossWalkingMovement)
	
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obVector, CDirection( CONSTRUCT_CLEAR ), Vector)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fStopDistance, 1.5f, StopDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDistanceToFormAroundPlayer, 2.5f, DistanceToFormAroundPlayer)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFormationTolerance, 0.25f, FormationTolerance)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)

	PUBLISH_VAR_AS(m_bInAir, InAir)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralMovementSelector)
	COPY_INTERFACE_FROM(BossMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossMovementSelector)

	PUBLISH_VAR_AS(m_bInAir, InAir)
END_STD_INTERFACE

START_STD_INTERFACE(RandomAerialGeneralAttackSelector)
	COPY_INTERFACE_FROM(AerialGeneralAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(AerialGeneralAttackSelector)
END_STD_INTERFACE

START_STD_INTERFACE(RandomAerialGeneralMovementSelector)
	COPY_INTERFACE_FROM(AerialGeneralMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(AerialGeneralMovementSelector)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralIntoAirTransitionMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obIntoAirAnim, IntoAirAnim)
	PUBLISH_VAR_AS(m_fHoverHeight, HoverHeight)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralOntoGroundTransitionMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obFallStartAnim, FallStartAnim)
	PUBLISH_VAR_AS(m_obFallCycleAnim, FallCycleAnim)
	PUBLISH_VAR_AS(m_obHitGroundAnim, HitGroundAnim)
END_STD_INTERFACE

START_STD_INTERFACE(DistanceSuitabilityAerialGeneralAttackSelector)
	COPY_INTERFACE_FROM(AerialGeneralAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(AerialGeneralAttackSelector)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralBoomerangSpecialAttack)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_PTR_AS(m_pobBeginningMovement, BeginningMovement)
	PUBLISH_PTR_AS(m_pobThrowAttackLeft, ThrowAttackLeft)
	PUBLISH_PTR_AS(m_pobThrowAttackRight, ThrowAttackRight)
	PUBLISH_PTR_AS(m_pobMovementInBetween, MovementInBetween)
	PUBLISH_PTR_AS(m_pobCatchAttack, CatchAttack)
	PUBLISH_PTR_AS(m_pobProjectileAttributes, ProjectileAttributes)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fCatchDistance, 1.0f, CatchDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRandomAugmentAngle, 180.0f, RandomAugmentAngle)
	PUBLISH_PTR_AS(m_pobGoadingMovement, GoadingMovement)
	PUBLISH_PTR_AS(m_pobDiveLeftMovement, DiveLeftMovement)
	PUBLISH_PTR_AS(m_pobDiveRightMovement, DiveRightMovement)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obCounteredSwordSharedAttribs, "Att_Thrown_Countered_AGen_Sword", CounteredSwordSharedAttribs)
    PUBLISH_VAR_WITH_DEFAULT_AS(m_obCounteredSwordClump, "entities\\characters\\aerialgeneral\\agen_throwablesingleblade.clump", CounteredSwordClump)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralSwoopMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obTurnLeftAnim,		TurnLeftAnim)
	PUBLISH_VAR_AS(m_obTurnRightAnim,		TurnRightAnim)
	PUBLISH_VAR_AS(m_obSwoopAnim,			SwoopAnim)
	PUBLISH_VAR_AS(m_fRandomAugmentAngle,	RandomAugmentAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fHeightAdjustMinimum, 3.0f, HeightAdjustMinimum)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fHeightAdjust, 10.0f, HeightAdjust)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fToDistanceInFrontOfPlayer, 0.0f, ToDistanceInFrontOfPlayer)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralMachineGunSpecialAttack)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_PTR_AS(m_pobBeginningMovement, BeginningMovement)
	PUBLISH_PTR_AS(m_pobThrowAttack, ThrowAttack)
	PUBLISH_PTR_AS(m_pobMovementInBetween, MovementInBetween)
	PUBLISH_PTR_AS(m_pobCatchAttack, CatchAttack)
	PUBLISH_PTR_AS(m_pobProjectileAttributes, ProjectileAttributes)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeTillSwordReclaim, 2.5f, TimeTillSwordReclaim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumAttacksToDo, 3, NumAttacksToDo)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumAttacksToDoAdjust, 0, NumAttacksToDoAdjust)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeBetweenAttacks, 1.0f, TimeBetweenAttacks)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fStartCatchDistance, 5.0f, StartCatchDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSwordDestroyDistance, 0.5f, SwordDestroyDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumberOfThrowsTillOnTarget, 6, NumberOfThrowsTillOnTarget)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDownishVector, CDirection( 0.0f, -2.0f, 0.0f ), DownishVector) 
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralTeleportingMeleeAttack)
	COPY_INTERFACE_FROM(BossMeleeAttack)
	DEFINE_INTERFACE_INHERITANCE(BossMeleeAttack)

	PUBLISH_VAR_AS(m_obTeleportString, TeleportString)
END_STD_INTERFACE

START_STD_INTERFACE(AerialGeneralDoppelgangerSpawningBossAttackPhase)
	COPY_INTERFACE_FROM(BossAttackPhase)
	DEFINE_INTERFACE_INHERITANCE(BossAttackPhase)
END_STD_INTERFACE

START_STD_INTERFACE( AerialGeneralDoppelgangerSpawningStartTransition )
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumToWakeUp, 10, NumToWakeUp) 
END_STD_INTERFACE

//Lua
LUA_EXPOSED_START(DoppelgangerManager)
	LUA_EXPOSED_METHOD(RegisterDoppelganger, RegisterDoppelganger, "", "", "") 
	LUA_EXPOSED_METHOD(SetActive, SetActive, "", "", "") 
	LUA_EXPOSED_METHOD(IsActive, IsActive, "", "", "") 
	LUA_EXPOSED_METHOD(CanIAttack, CanIAttack, "", "", "") 
	LUA_EXPOSED_METHOD(SetMaxNumberOfAttackers, SetMaxNumberOfAttackers, "", "", "") 
	LUA_EXPOSED_METHOD(SetAttackerSwitchFrequency, SetAttackerSwitchFrequency, "", "", "") 
	LUA_EXPOSED_METHOD(SetArenaFormationStartDistance, SetArenaFormationStartDistance, "", "", "") 
LUA_EXPOSED_END(DoppelgangerManager)

//--------------------------------------------------
//!
//! DoppelgangerManager 
//!
//--------------------------------------------------
DoppelgangerManager::DoppelgangerManager()
{
	ATTACH_LUA_INTERFACE(DoppelgangerManager);

	NinjaLua::LuaState& luaState = CLuaGlobal::Get().State();
	NinjaLua::LuaValue::Push( luaState, this );
	luaState.GetGlobals().Set( "DoppelgangerManager", NinjaLua::LuaObject( -1, luaState, false) );

	m_bActive = true;
	m_iAttackers = 1;
	m_fMaxFrequency = 30.0f;
	m_fMaxFrequencyAdjust = 20.0f; 
	m_bFirstFrame = true;
	m_pobMaster = 0;
	m_fArenaFormationStartDistance = 1.0f;
	m_fTimeSinceLastWakeUp = 0.0f;
}

float DoppelgangerManager::GetArenaFormationDistance( AerialGeneral* pobAGen )
{
	// AGens that are allowed to attack don't need to obey any movements
	if ( CanIAttack(pobAGen) )
		return -1.0f;

	// We want a place in formation for each general in the arena - this is crap I know, there isn't enough space in the arena, but as a quick first pass this'll do. Someone else can break it down into nice pie segments
	float fIncrementValue = ( GetMaster()->GetArenaRadius() - m_fArenaFormationStartDistance ) / m_apobNonAttackers.size();

	// Get my place in non-attacker array/vector, use that index to get a distance
	int i = 0;
	for ( ntstd::Vector< AerialGeneral* >::iterator obIt = m_apobNonAttackers.begin(); obIt != m_apobNonAttackers.end(); obIt++ )
	{
		if ((*obIt) == pobAGen)
		{
			break;
		}

		i++;
	}

	return m_fArenaFormationStartDistance + ( fIncrementValue * i );
}

void DoppelgangerManager::RegisterDoppelganger( CEntity* pobAGen ) 
{ 
	m_apobNonAttackers.push_back((AerialGeneral*)pobAGen); 
}

void DoppelgangerManager::RegisterMaster( CEntity* pobAGen ) 
{ 
	m_pobMaster = (AerialGeneral*)pobAGen; 
	m_apobNonAttackers.push_back((AerialGeneral*)pobAGen); 
}	

void DoppelgangerManager::SetMaxNumberOfAttackers( int iAttackers ) 
{
	// Delete all our current slots
	for ( ntstd::Vector< DoppelgangerAttackingSlot* >::iterator obIt = m_apobAttackers.begin(); obIt != m_apobAttackers.end(); ++obIt )
	{
		m_apobNonAttackers.push_back( (*obIt)->m_pobAGen );

		NT_DELETE_CHUNK( Mem::MC_MISC, (*obIt) );
		m_apobAttackers.erase( obIt );
		obIt--;
	} 

	ntError( m_apobAttackers.empty() );

	m_iAttackers = iAttackers; 
	m_bFirstFrame = true; // Set this so we refill attacker collection next frame
}

void DoppelgangerManager::Update(float fTimeDelta)
{
	if (!m_bActive) return;

	// HACK for alpha - as soon as there are more than one aerial generals, render a blob on the master
	if (m_apobNonAttackers.size() + m_apobAttackers.size() > 1)
	{
		#ifndef _GOLD_MASTER
		CPoint obPos(GetMaster()->GetPosition());
		obPos.Y() += 2.0f;
		g_VisualDebug->RenderSphere(GetMaster()->GetRotation(),obPos, 0.5f, DC_RED);
		#endif
	}

	//ntPrintf("AGen: %i non, %i att.\n", m_apobNonAttackers.size(), m_apobAttackers.size());

	if (m_bFirstFrame)
	{
		// If we have more slots than generals, all generals are ok to attack
		if ( (unsigned int)m_iAttackers >= m_apobNonAttackers.size() )
		{
			for ( ntstd::Vector< AerialGeneral* >::iterator obIt = m_apobNonAttackers.begin(); obIt != m_apobNonAttackers.end(); obIt++ )
			{
				DoppelgangerAttackingSlot* pobSlot = NT_NEW_CHUNK( Mem::MC_MISC ) DoppelgangerAttackingSlot;

				pobSlot->m_pobAGen = (*obIt);
				pobSlot->m_fTimeAttackingSoFar = 0.0f;
				pobSlot->m_fTimeAllowedAttacking = m_fMaxFrequency - BOSS_RAND_F( m_fMaxFrequencyAdjust );
				
				// Add him to the attacks
                m_apobAttackers.push_back(pobSlot);

				// Remove him from the non attackers
				m_apobNonAttackers.erase(obIt);
				obIt--;
			}
		}
		else
		{
			// Just take the first however many
			for ( ntstd::Vector< AerialGeneral* >::iterator obIt = m_apobNonAttackers.begin(); m_apobNonAttackers.size() < (unsigned int)m_iAttackers; ++obIt )
			{
				DoppelgangerAttackingSlot* pobSlot = NT_NEW_CHUNK( Mem::MC_MISC ) DoppelgangerAttackingSlot;

				pobSlot->m_pobAGen = (*obIt);
				pobSlot->m_fTimeAttackingSoFar = 0.0f;
				pobSlot->m_fTimeAllowedAttacking = m_fMaxFrequency - BOSS_RAND_F( m_fMaxFrequencyAdjust );
				
				// Add him to the attacks
                m_apobAttackers.push_back(pobSlot);

				// Remove him from the non attackers
				m_apobNonAttackers.erase(obIt);
				obIt--;
			}
		}

		m_bFirstFrame = false;
	}

	// Sanity check
	ntError( m_apobAttackers.size() <= (unsigned int)m_iAttackers );

	m_fTimeSinceLastWakeUp += fTimeDelta;

	// If we have enough AGens available to swap around at all
	if ( m_apobNonAttackers.size() > 0 )
	{
		// We've got non-attackers, if we've got space for them in attackers, immediately put them in a slot
		while( m_apobAttackers.size() < (unsigned int)m_iAttackers && m_apobNonAttackers.size() > 0 )
		{
			// This AGen can't attack anymore, get another one
			AerialGeneral* pobAGen = 0;
			if ( m_apobNonAttackers.size() == 1 )
			{
				pobAGen = m_apobNonAttackers[0];
				m_apobNonAttackers.clear();
			}
			else
			{
				int iRand = BOSS_RAND() % m_apobNonAttackers.size();
				int i = 0;
				ntstd::Vector< AerialGeneral* >::iterator obIt2 = m_apobNonAttackers.begin(); 
				while ( i != iRand )
				{
					++i;
					++obIt2;
				}

				pobAGen = (*obIt2);
				m_apobNonAttackers.erase(obIt2);
			}	

			// Create a new slot, push it on
			DoppelgangerAttackingSlot* pobSlot = NT_NEW_CHUNK( Mem::MC_MISC ) DoppelgangerAttackingSlot;

			pobSlot->m_pobAGen = pobAGen;
			pobSlot->m_fTimeAttackingSoFar = 0.0f;
			pobSlot->m_fTimeAllowedAttacking = m_fMaxFrequency - BOSS_RAND_F( m_fMaxFrequencyAdjust );
			
			// Add him to the attacks
			m_apobAttackers.push_back(pobSlot);
		}

		// Loop through all attackers, updating time and checking if they're out of time to attack
		for ( ntstd::Vector< DoppelgangerAttackingSlot* >::iterator obIt = m_apobAttackers.begin(); obIt != m_apobAttackers.end(); ++obIt )
		{
			(*obIt)->m_fTimeAttackingSoFar += fTimeDelta;

			if ( (*obIt)->m_fTimeAttackingSoFar >= (*obIt)->m_fTimeAllowedAttacking )
			{				
				// This AGen can't attack anymore, get another one
				AerialGeneral* pobAGen = 0;
				if ( m_apobNonAttackers.size() == 1 )
				{
					pobAGen = m_apobNonAttackers[0];
					m_apobNonAttackers.clear();
				}
				else
				{
					int iRand = BOSS_RAND() % m_apobNonAttackers.size();
					int i = 0;
					ntstd::Vector< AerialGeneral* >::iterator obIt2 = m_apobNonAttackers.begin(); 
					while ( i != iRand )
					{
						++i;
						++obIt2;
					}

					pobAGen = (*obIt2);
					m_apobNonAttackers.erase(obIt2);
				}	

				// Just replace this one with our new AGen
				m_apobNonAttackers.push_back((*obIt)->m_pobAGen);

				// Reusing the slot pointer to keep allocation only to when we really need it
				(*obIt)->m_pobAGen = pobAGen;
				(*obIt)->m_fTimeAttackingSoFar = 0.0f;
				(*obIt)->m_fTimeAllowedAttacking = m_fMaxFrequency - BOSS_RAND_F( m_fMaxFrequencyAdjust );
			}
		}
	}
}

bool DoppelgangerManager::CanIAttack( CEntity* pobAGen ) 
{ 
	for ( ntstd::Vector< DoppelgangerAttackingSlot* >::iterator obIt = m_apobAttackers.begin(); obIt != m_apobAttackers.end(); ++obIt )
	{
		if( (*obIt)->m_pobAGen == (AerialGeneral*)pobAGen )
			return true;
	}

	return false;
}

//--------------------------------------------------
//!
//! AerialGeneral 
//! Any specific construction or updating for the aerial general
//!
//--------------------------------------------------
AerialGeneral::AerialGeneral()
: Boss()
{
	m_eBossType = BT_AERIAL_GENERAL;
	m_bInAir = false;

	m_iDoppelganger = 0;

    m_pLeftSword = 0;
	m_pRightSword = 0;
	m_pLeftPowerSword = 0;
	m_pRightPowerSword = 0;

	m_bDoppelgangerPaused = false;

	if ( !DoppelgangerManager::Exists() )
		NT_NEW DoppelgangerManager;

	m_bSplitting = false;
}

AerialGeneral::~AerialGeneral()
{
	// Assuming all AGens get deconstructed at the same time
	if ( DoppelgangerManager::Exists() )
		DoppelgangerManager::Kill();
}

void AerialGeneral::NotifyBoomerangSwordCountered(CEntity* pobCounterer)
{
	Boss::NotifyInteractionWith(pobCounterer);
}


CEntity* AerialGeneral::CreateInHandSword(bool bRightHand, bool bPowerSword, const char* pcName)
{
	LuaAttributeTable* pobTable = LuaAttributeTable::Create();
	DataObject* pDO = ObjectDatabase::Get().ConstructObject( "CEntity", pcName, GameGUID(), 0, true, false );
	CEntity* pobNewEntity = 0;

	if(pDO)
	{
		pobNewEntity  = (CEntity*) pDO->GetBasePtr();
		if(pobNewEntity)
		{
			pobNewEntity->SetAttributeTable( pobTable );
			pobNewEntity->GetAttributeTable()->SetDataObject( pDO );
			pobTable->SetAttribute("Name", pcName );
			//Set the correct clump (single sword or double/power sword)
			if(bPowerSword)
			{
				pobTable->SetAttribute("Clump", "entities/characters/aerialgeneral/agen_blade_double.clump" );
			}
			else
			{
				pobTable->SetAttribute("Clump", "entities/characters/aerialgeneral/agen_blade_single.clump" );
			}

			ObjectDatabase::Get().DoPostLoadDefaults( pDO );

			// Parent the sword to the correct hand.
			Transform* pobParentTransform = 0;
			if(bRightHand)
			{
				pobParentTransform = this->GetHierarchy()->GetTransform( CHashedString("r_weapon") );
			}
			else
			{
				pobParentTransform = this->GetHierarchy()->GetTransform( CHashedString("l_weapon") );
			}
			Transform* pobTargTransform = pobNewEntity->GetHierarchy()->GetRootTransform();
			pobNewEntity->SetParentEntity( this );
			pobTargTransform->RemoveFromParent();
			pobParentTransform->AddChild( pobTargTransform );

			if(pobNewEntity->GetPhysicsSystem())
			{
				pobNewEntity->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
			}

			//Hide by default.
			pobNewEntity->Hide();
		}
	}

	return pobNewEntity;
}


void AerialGeneral::OnPostPostConstruct()
{
	this->GetAttackComponent()->SetCanHeadshotThisEntity(true);

	// Only the main AGen (with the name of the other AGens specified) should do this doppelganger setup
	if (!m_aobDoppelgangerAGenEntityNames[0].IsNull())
	{
		// Add me to the doppelganger manager, I'll be the only one in there till the phase where they start waking up
		DoppelgangerManager::Get().RegisterMaster(this);

		for (int i = 0; i < AGEN_DOPPELGANGER_COUNT; i++)
		{
			if (!m_aobDoppelgangerAGenEntityNames[i].IsNull())
			{
				CEntity* pobEnt = CEntityManager::Get().FindEntity(m_aobDoppelgangerAGenEntityNames[i]);
				if (pobEnt && pobEnt->IsBoss())
				{
					m_apobDoppelgangers[i] = (AerialGeneral*)pobEnt;
					m_apobDoppelgangers[i]->SetIsDoppelganger(i+1);
					m_apobDoppelgangers[i]->Pause(true);
					m_apobDoppelgangers[i]->SetIsDoppelgangerPaused(true);
					m_apobDoppelgangers[i]->Hide();
					m_apobDoppelgangers[i]->GetPhysicsSystem()->Deactivate();
				}
				else
					m_apobDoppelgangers[i] = 0;
			}
			else
				m_apobDoppelgangers[i] = 0;
		}
	}
	// Null it out in all the other AGens
	else
	{
		for (int i = 0; i < AGEN_DOPPELGANGER_COUNT; i++)
		{
			m_aobDoppelgangerAGenEntityNames[i] = m_aobDoppelgangerAGenEntityNames[i].nullString;
			m_apobDoppelgangers[i] = 0;
		}
	}

	// Swords
	char* apcTransforms[26] = {	"RFBlade_J01", "RFBlade_J02", "RFBlade_J03", "RFBlade_J04", "RFBlade_J05", "RFBlade_J06", "RFBlade_J07", "RBBlade_J01",
								"RBBlade_J02", "RBBlade_J03", "RBBlade_J04", "RBBlade_J05", "RBBlade_J06", "LFBlade_J01", "LFBlade_J02", "LFBlade_J03",
								"LFBlade_J04", "LFBlade_J05", "LFBlade_J06", "LFBlade_J07", "LBBlade_J01", "LBBlade_J02", "LBBlade_J03", "LBBlade_J04",
								"LBBlade_J05", "LBBlade_J06" };

	const char* pcClump = "entities/characters/aerialgeneral/agen_blade_single.clump";

	for (int i = 0; i < 26; i++)
	{
		char pcName[128];
		sprintf(pcName,"%s_BackSword_%s",GetName().c_str(),apcTransforms[i]);
		
		LuaAttributeTable* pobTable = LuaAttributeTable::Create();
		DataObject* pDO;
		pDO = ObjectDatabase::Get().ConstructObject( "CEntity", pcName, GameGUID(), 0, true, false );

		CEntity* pobNewEntity = (CEntity*) pDO->GetBasePtr();
		pobNewEntity->SetAttributeTable( pobTable );
		pobNewEntity->GetAttributeTable()->SetDataObject( pDO );
		pobTable->SetAttribute("Name", pcName );
		pobTable->SetAttribute("Clump", pcClump );

		ObjectDatabase::Get().DoPostLoadDefaults( pDO );

		Transform* pobParentTransform = GetHierarchy()->GetTransform( apcTransforms[i] );
		Transform* pobTargTransform = pobNewEntity->GetHierarchy()->GetRootTransform();
		pobNewEntity->SetParentEntity( this );
		pobTargTransform->RemoveFromParent();
		pobParentTransform->AddChild( pobTargTransform );

		m_obBackSwords.push_back(pobNewEntity);

		if (GetIsDoppelganger())
			pobNewEntity->Hide();
	}

	//Create and parent the in-hand swords here. This way we don't need to reparent the ones on the back, but can rather just show/hide them.
	char pcName[128];
	sprintf(pcName,"%s_LeftHandSpeedSword",GetName().c_str());
	m_pLeftSword = CreateInHandSword(false, false, pcName);
	sprintf(pcName,"%s_RightHandSpeedSword",GetName().c_str());
	m_pRightSword = CreateInHandSword(true, false, pcName);
	sprintf(pcName,"%s_LeftHandPowerSword",GetName().c_str());
	m_pLeftPowerSword = CreateInHandSword(false, true, pcName);
	sprintf(pcName,"%s_RightHandPowerSword",GetName().c_str());
	m_pRightPowerSword = CreateInHandSword(true, true, pcName);
    
	ntError_p(m_pLeftSword && m_pRightSword && m_pLeftPowerSword && m_pRightPowerSword, ("Failed to create one or more in-hand swords"));
}

AerialGeneral* AerialGeneral::GetNextDoppelgangerToWakeUp()
{
	// Wake up next inactive agen
	for (int i = 0; i < AGEN_DOPPELGANGER_COUNT && i < m_iNumberOfDoppelgangersTillEnd; i++)
	{
		if (m_apobDoppelgangers[i] && m_apobDoppelgangers[i]->GetIsDoppelgangerPaused())
		{
			return m_apobDoppelgangers[i];
		}
	}

	// If we get here, all dopps have been woken up and we should send our message
	// Try to find the entity to tell
	CEntity* pobEntity = CEntityManager::Get().FindEntity(m_obAllDoppelgangersActiveMessageEntityName);
	if (pobEntity)
	{
		ntPrintf("All doppelgangers active message sent.\n");
		pobEntity->GetMessageHandler()->Receive( CMessageHandler::Make( this, m_obAllDoppelgangersActiveMessage ) );
	}

	return 0;
}

void AerialGeneral::WakeUpDoppelganger( bool bDoSplit, bool bForce )
{
	if (DoppelgangerManager::Get().GetTimeSinceLastWakeUp() > 5.0f || bForce)
	{
		AerialGeneral* pobSleepyDop = DoppelgangerManager::Get().GetMaster()->GetNextDoppelgangerToWakeUp();

		if (pobSleepyDop)
		{
			ntPrintf("Doppelganger (%s) activating.\n", pobSleepyDop->GetName().c_str());

			DoppelgangerManager::Get().RegisterDoppelganger(pobSleepyDop);

			pobSleepyDop->Pause(false);
			pobSleepyDop->SetIsDoppelgangerPaused(false);
			pobSleepyDop->Show();
			pobSleepyDop->GetPhysicsSystem()->Activate();
			pobSleepyDop->SetPosition(this->GetPosition());
			pobSleepyDop->SetRotation(this->GetRotation());

			if (bDoSplit)
			{			
				pobSleepyDop->GetInteractionComponent()->ExcludeCollisionWith(DoppelgangerManager::Get().GetMaster());
				
				SimpleRelativeTransitionDef obAGenMoveDef;
				obAGenMoveDef.SetDebugNames( "AGen split sync", "SimpleRelativeTransitionDef" );
				obAGenMoveDef.m_bOwnsTransform = true;
				obAGenMoveDef.m_pobAnimation = this->GetAnimator()->CreateAnimation(m_obAGenSplitAnim);
				obAGenMoveDef.m_pobRelativeTransform = NT_NEW_CHUNK( Mem::MC_MISC ) Transform();
				obAGenMoveDef.m_pobRelativeTransform->SetLocalMatrix( this->GetMatrix() );
				CHierarchy::GetWorld()->GetRootTransform()->AddChild( obAGenMoveDef.m_pobRelativeTransform );

				SimpleRelativeTransitionDef obDoppelgangerMoveDef;
				obDoppelgangerMoveDef.SetDebugNames( "Doppelganger split sync", "SimpleRelativeTransitionDef" );
				obDoppelgangerMoveDef.m_bOwnsTransform = true;
				obDoppelgangerMoveDef.m_pobAnimation = pobSleepyDop->GetAnimator()->CreateAnimation(m_obDoppelgangerSplitAnim);
				obDoppelgangerMoveDef.m_pobRelativeTransform = NT_NEW_CHUNK( Mem::MC_MISC ) Transform();
				obDoppelgangerMoveDef.m_pobRelativeTransform->SetLocalMatrix( this->GetMatrix() );
				CHierarchy::GetWorld()->GetRootTransform()->AddChild( obDoppelgangerMoveDef.m_pobRelativeTransform );

				this->GetAnimator()->RemoveAllAnimations();
				this->GetAnimator()->ClearAnimWeights();
				this->GetMovement()->ClearControllers();
				pobSleepyDop->GetAnimator()->RemoveAllAnimations();
				pobSleepyDop->GetAnimator()->ClearAnimWeights();
				pobSleepyDop->GetMovement()->ClearControllers();

				this->GetMovement()->BringInNewController(obAGenMoveDef, CMovement::DMM_SOFT_RELATIVE, 0.0f);
				pobSleepyDop->GetMovement()->BringInNewController(obDoppelgangerMoveDef, CMovement::DMM_SOFT_RELATIVE, 0.0f);
			
				// AGen being woken up needs to start boss battle when done
				Message obWakeMessage(msg_startbossbattle);
				pobSleepyDop->GetMovement()->SetCompletionMessage(obWakeMessage,pobSleepyDop);
				Message obContinueMessage(msg_combat_recovered);
				pobSleepyDop->GetMovement()->SetCompletionMessage(obContinueMessage,pobSleepyDop);
				// But this Agen just needs to carry on out of react state
				this->GetMovement()->SetCompletionMessage(obContinueMessage,this);

				this->SetSplitting(true);
				pobSleepyDop->SetSplitting(true);
			}
			else
			{
				Message obWakeMessage(msg_startbossbattle);
				pobSleepyDop->GetMessageHandler()->Receive(obWakeMessage);
			}

			DoppelgangerManager::Get().ResetTimeSinceLastWakeUp();
		}
	}
}

void AerialGeneral::UnholsterSword(bool bRightHand, bool bPowerSword)
{
	//Bounds check our transform-ref values before attempting to use them!
	ntError_p((m_iRightHandSwordBackTransform1 >= 0) && (m_iRightHandSwordBackTransform1 < (int)(m_obBackSwords.size())),
		("Out of bounds value [%d] on m_iRightHandSwordBackTransform1", m_iRightHandSwordBackTransform1));
	ntError_p((m_iRightHandSwordBackTransform2 >= 0) && (m_iRightHandSwordBackTransform2 < (int)(m_obBackSwords.size())),
		("Out of bounds value [%d] on m_iRightHandSwordBackTransform2", m_iRightHandSwordBackTransform2));
	ntError_p((m_iLeftHandSwordBackTransform1 >= 0) && (m_iLeftHandSwordBackTransform1 < (int)(m_obBackSwords.size())),
		("Out of bounds value [%d] on m_iLeftHandSwordBackTransform1", m_iLeftHandSwordBackTransform1));
	ntError_p((m_iLeftHandSwordBackTransform2 >= 0) && (m_iLeftHandSwordBackTransform2 < (int)(m_obBackSwords.size())),
		("Out of bounds value [%d] on m_iLeftHandSwordBackTransform2", m_iLeftHandSwordBackTransform2));

	if(bPowerSword)
	{
		//With power-swords we hide the two on his back and show a power-sword in the appropriate hand.
		CEntity* pSword1 = 0;
		CEntity* pSword2 = 0;
		CEntity* pPowerSword = 0;
		if(bRightHand)
		{
			pSword1 = m_obBackSwords[m_iRightHandSwordBackTransform1];
			pSword2 = m_obBackSwords[m_iRightHandSwordBackTransform2];
			pPowerSword = m_pRightPowerSword;
		}
		else
		{
			pSword1 = m_obBackSwords[m_iLeftHandSwordBackTransform1];
			pSword2 = m_obBackSwords[m_iLeftHandSwordBackTransform2];
			pPowerSword = m_pLeftPowerSword;
		}

		//Hide the ones on the back.
		ntError_p(pSword1 != NULL, ("One or more of the back-swords appears to be missing"));
		ntError_p(pSword2 != NULL, ("One or more of the back-swords appears to be missing"));
		if(pSword1){ pSword1->Hide(); }
		if(pSword2){ pSword2->Hide(); }

		//Show the in-hand power sword.
		ntError_p(pPowerSword != NULL, ("Missing power-sword on the aerial general (should've been created on post construct)"));
		if(pPowerSword)
		{
			pPowerSword->Show();
		}
	}
	else
	{
		//With normal swords, we hide one on the back and show the one in the general's hand.
		CEntity* pSword = 0;
		CEntity* pHandSword = 0;
		if(bRightHand)
		{
			pSword = m_obBackSwords[m_iRightHandSwordBackTransform1];
			pHandSword = m_pRightSword;
		}
		else
		{
			pSword = m_obBackSwords[m_iLeftHandSwordBackTransform1];
			pHandSword = m_pLeftSword;
		}

		ntError_p(pSword != NULL, ("One or more of the back-swords appears to be missing"));
		if(pSword) { pSword->Hide(); }
		ntError_p(pHandSword != NULL, ("Missing in-hand sword (should've been created on post construct)"));
		if(pHandSword) { pHandSword->Show(); }
	}
}


void AerialGeneral::HolsterSword(bool bRightHand, bool bPowerSword)
{
	//Bounds check our transform-ref values before attempting to use them!
	ntError_p((m_iRightHandSwordBackTransform1 >= 0) && (m_iRightHandSwordBackTransform1 < (int)(m_obBackSwords.size())),
		("Out of bounds value [%d] on m_iRightHandSwordBackTransform1", m_iRightHandSwordBackTransform1));
	ntError_p((m_iRightHandSwordBackTransform2 >= 0) && (m_iRightHandSwordBackTransform2 < (int)(m_obBackSwords.size())),
		("Out of bounds value [%d] on m_iRightHandSwordBackTransform2", m_iRightHandSwordBackTransform2));
	ntError_p((m_iLeftHandSwordBackTransform1 >= 0) && (m_iLeftHandSwordBackTransform1 < (int)(m_obBackSwords.size())),
		("Out of bounds value [%d] on m_iLeftHandSwordBackTransform1", m_iLeftHandSwordBackTransform1));
	ntError_p((m_iLeftHandSwordBackTransform2 >= 0) && (m_iLeftHandSwordBackTransform2 < (int)(m_obBackSwords.size())),
		("Out of bounds value [%d] on m_iLeftHandSwordBackTransform2", m_iLeftHandSwordBackTransform2));

	if(bPowerSword)
	{
		//With power-swords we hide the two on his back and show a power-sword in the appropriate hand.
		CEntity* pSword1 = 0;
		CEntity* pSword2 = 0;
		CEntity* pPowerSword = 0;
		if(bRightHand)
		{
			pSword1 = m_obBackSwords[m_iRightHandSwordBackTransform1];
			pSword2 = m_obBackSwords[m_iRightHandSwordBackTransform2];
			pPowerSword = m_pRightPowerSword;
		}
		else
		{
			pSword1 = m_obBackSwords[m_iLeftHandSwordBackTransform1];
			pSword2 = m_obBackSwords[m_iLeftHandSwordBackTransform2];
			pPowerSword = m_pLeftPowerSword;
		}

		//Show the ones on the back.
		ntError_p(pSword1 != NULL, ("One or more of the back-swords appears to be missing"));
		ntError_p(pSword2 != NULL, ("One or more of the back-swords appears to be missing"));
		if(pSword1){ pSword1->Show(); }
		if(pSword2){ pSword2->Show(); }

		//Hide the in-hand power sword.
		ntError_p(pPowerSword != NULL, ("Missing power-sword on the aerial general (should've been created on post construct)"));
		if(pPowerSword)
		{
			pPowerSword->Hide();
		}
	}
	else
	{
		//With normal swords, we hide one in-hand and show the one on the general's hand.
		CEntity* pSword = 0;
		CEntity* pHandSword = 0;
		if(bRightHand)
		{
			pSword = m_obBackSwords[m_iRightHandSwordBackTransform1];
			pHandSword = m_pRightSword;
		}
		else
		{
			pSword = m_obBackSwords[m_iLeftHandSwordBackTransform1];
			pHandSword = m_pLeftSword;
		}

		ntError_p(pSword != NULL, ("One or more of the back-swords appears to be missing"));
		if(pSword) { pSword->Show(); }
		ntError_p(pHandSword != NULL, ("Missing in-hand sword (should've been created on post construct)"));
		if(pHandSword) { pHandSword->Hide(); }
	}
}

bool AerialGeneral::CanStartAnAttack()
{
	bool bDoppelCheck = true;
	if (DoppelgangerManager::Exists())
		bDoppelCheck = DoppelgangerManager::Get().CanIAttack(this);
	return GetAttackComponent()->AI_Access_GetState() == CS_STANDARD && bDoppelCheck;
}

bool AerialGeneral::CanStartALinkedAttack()
{
	return GetAttackComponent()->IsInNextMoveWindow();
}

void AerialGeneral::DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset )
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderPoint(DoppelgangerManager::Get().GetMaster()->GetArenaCentre(),10.0f,DC_GREEN);
	CMatrix obMtx( CONSTRUCT_IDENTITY );
	obMtx.SetTranslation(DoppelgangerManager::Get().GetMaster()->GetArenaCentre());
	g_VisualDebug->RenderArc(obMtx, DoppelgangerManager::Get().GetMaster()->GetArenaRadius(), 360.0f * DEG_TO_RAD_VALUE, DC_CYAN);
#endif
}

void AerialGeneral::UpdateBossSpecifics( float fTimeDelta )
{
	// If this is a doppelganger, and it's paused, we need to ensure the entity remains paused
	if (m_bDoppelgangerPaused && !IsPaused())
	{
		Pause(true);
	}

	if (DoppelgangerManager::Exists() && DoppelgangerManager::Get().GetMaster() == this)
		DoppelgangerManager::Get().Update(fTimeDelta);

	if (m_iDoppelganger == 0)
	{
		float fHealth = GetCurrHealth();
	
		for (int i = 0; i < AGEN_DOPPELGANGER_COUNT; i++)
		{
			if (m_apobDoppelgangers[i])
			{
				m_apobDoppelgangers[i]->SetHealth(fHealth,"AGen updating health in line with doppelgangers");
			}
		}
	}

	/*if (DoppelgangerManager::Get().CanIAttack(this))
		g_VisualDebug->RenderSphere(CQuat(GetMatrix()),GetMatrix().GetTranslation(),1.0f,DC_RED);*/
}

//--------------------------------------------------
//!
//! RandomAerialGeneralAttackSelector
//!
//--------------------------------------------------
float RandomAerialGeneralAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );
	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
	
	if (pobAGen->IsInAir() == m_bInAir && m_obAttacks.size() > 0)
		return m_fMaxPriority;
	else
		return 0.0f;
}

BossAttack* RandomAerialGeneralAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );
	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
	if (pobAGen->IsInAir() != m_bInAir)
	{
		ntError( 0 );
	}

	if (m_obAttacks.size() > 0)
	{
		return ChooseRandomAttackWithWeighting(pobBoss, pobPlayer);
	}

	return 0;
}

//--------------------------------------------------
//!
//! RandomAerialGeneralMovementSelector 
//!
//--------------------------------------------------
float RandomAerialGeneralMovementSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) 
{ 
	UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); 
	
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );
	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
	if (pobAGen->IsInAir() == m_bInAir && m_obMovements.size() > 0)
	{
		m_iSelectedMovement = BOSS_RAND() % m_obMovements.size();
		return m_fMaxPriority;
	}
	else
		return 0.0f;
};

BossMovement* RandomAerialGeneralMovementSelector::GetSelectedMovement() 
{
	if (m_obMovements.size() > 0)
	{
		ntstd::List<BossMovement*>::iterator obIt = m_obMovements.begin();
		for (int i = 0; i < m_iSelectedMovement; i++)
			obIt++;
		return (*obIt); 
	}
	else
		return 0; 
};

BossMovement* RandomAerialGeneralMovementSelector::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) 
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );
	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
	if (pobAGen->IsInAir() != m_bInAir)
	{
		ntError( 0 );
	}

	if (m_obMovements.size() > 0)
	{
		ntstd::List<BossMovement*>::iterator obIt = m_obMovements.begin();
		for (int i = 0; i < m_iSelectedMovement; i++)
			obIt++;
		return (*obIt)->DoMovement(fTimeDelta,pobBoss,pobPlayer); 
	}
	else
		return 0;
};

//--------------------------------------------------
//!
//! AerialGeneralIntoAirTransitionMovement
//!
//--------------------------------------------------
BossMovement* AerialGeneralIntoAirTransitionMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );
	
	m_fTimeInMovement = 0.0f;

	ZAxisAlignTargetedTransitionDef obStartAlignDef;
	obStartAlignDef.SetDebugNames("AGen Into Air","ZAxisAlignTargetedTransitionDef");
	obStartAlignDef.m_pobEntityAlignZTowards = pobPlayer;
	obStartAlignDef.m_obAnimationName = m_obIntoAirAnim;
	obStartAlignDef.m_bApplyGravity = false;
	obStartAlignDef.m_obScaleToCoverDistance.Y() = m_fHoverHeight;

	pobBoss->GetMovement()->BringInNewController( obStartAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

	m_bDone = false;

	return this;
}

BossMovement* AerialGeneralIntoAirTransitionMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );

	m_fTimeInMovement += fTimeDelta;
	
	if (!m_bDone)
	{
		return this;
	}
	else
	{
		AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
		pobAGen->SetIsInAir(true);
		return 0;
	}
}

//--------------------------------------------------
//!
//! AerialGeneralOntoGroundTransitionMovement
//!
//--------------------------------------------------
BossMovement* AerialGeneralOntoGroundTransitionMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );

	m_fTimeInMovement = 0.0f;

	ZAxisAlignTargetedTransitionDef obStartFallDef;
	obStartFallDef.SetDebugNames("AGen Into Air","ZAxisAlignTargetedTransitionDef");
	obStartFallDef.m_pobEntityAlignZTowards = pobPlayer;
	obStartFallDef.m_obAnimationName = m_obFallStartAnim;
	obStartFallDef.m_bApplyGravity = false;

	ContinuationTransitionDef obDropDefinitionDef;
	obDropDefinitionDef.SetDebugNames( "AGen Drop Loop", "ContinuationTransitionDef" );
	obDropDefinitionDef.m_obAnimationName = m_obFallCycleAnim;
	obDropDefinitionDef.m_bHorizontalVelocity = true;
	obDropDefinitionDef.m_bVerticalVelocity = true;
	obDropDefinitionDef.m_bVerticalAcceleration = !true;
	obDropDefinitionDef.m_bLooping = true;
	obDropDefinitionDef.m_bEndOnGround = true;
	obDropDefinitionDef.m_bApplyGravity = true;

	ZAxisAlignTargetedTransitionDef obHitGroundDef;
	obHitGroundDef.SetDebugNames("AGen Hit Grounf","ZAxisAlignTargetedTransitionDef");
	obHitGroundDef.m_pobEntityAlignZTowards = pobPlayer;
	obHitGroundDef.m_obAnimationName = m_obHitGroundAnim;
	obHitGroundDef.m_bApplyGravity = false;

	pobBoss->GetMovement()->BringInNewController( obStartFallDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobBoss->GetMovement()->AddChainedController( obDropDefinitionDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobBoss->GetMovement()->AddChainedController( obHitGroundDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

	m_bDone = false;

	return this;
}

BossMovement* AerialGeneralOntoGroundTransitionMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );

	m_fTimeInMovement += fTimeDelta;
	
	if (!m_bDone)
	{
		return this;
	}
	else
	{
		AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
		pobAGen->SetIsInAir(false);
		return 0;
	}
}

//--------------------------------------------------
//!
//! DistanceSuitabilityBossAttackSelector
//!
//--------------------------------------------------
float DistanceSuitabilityAerialGeneralAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );
	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;

	m_pobSelectedAttack = 0;

	if (pobAGen->IsInAir() == m_bInAir)
	{
		// Get the average distance of our attacks, if we're within this average distance, select randomly from our pool
		CPoint obPlayerPosition = pobPlayer->GetPosition();
		CPoint obBossPosition = pobBoss->GetPosition();
		CDirection obBossToPlayer = CDirection( obPlayerPosition - obBossPosition );
		float fDistance = obBossToPlayer.Length();

		// Get average strike proxy for all our attacks
		ntstd::List<BossAttack*>::iterator obIt = m_obAttacks.begin();
		m_obInRangeAttacks.clear();
		while (obIt != m_obAttacks.end())
		{
			if (fDistance < (*obIt)->GetMaxDistance())
			{
				m_obInRangeAttacks.push_back((*obIt));
			}
			obIt++;
		}

		// If we're in range...
		if (m_obInRangeAttacks.size() == 1)
		{
			m_pobSelectedAttack = *m_obInRangeAttacks.begin();
			return m_fMaxPriority;
		}
		else if (m_obInRangeAttacks.size() > 1)
		{
			m_pobSelectedAttack = 0;
			return m_fMaxPriority;
		}
	}

	// Else we can't do anything at the moment
	m_pobSelectedAttack = 0;
	return m_fMinPriority;
}

BossAttack* DistanceSuitabilityAerialGeneralAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	if (!m_pobSelectedAttack)
		m_pobSelectedAttack = ChooseRandomAttackWithWeighting(pobBoss,pobPlayer,&m_obInRangeAttacks);

	// If we managed to select an attack whilst deciding priority...
	if (m_pobSelectedAttack && m_pobSelectedAttack->Initialise(pobBoss,pobPlayer))
	{
		m_pobLastSelection = m_pobSelectedAttack;
		return m_pobSelectedAttack;
	}
	else
	{
		return 0;
	}
}

//--------------------------------------------------
//!
//! AerialGeneralBoomerangSpecialAttack
//!
//--------------------------------------------------
AerialGeneralBoomerangSpecialAttack::AerialGeneralBoomerangSpecialAttack()
{
	m_pobThrowAttackLeft = 0;
	m_pobThrowAttackRight = 0;
	m_pobSword = 0;
	m_pobProjectileAttributes = 0;
	m_bSwordCreatedForThisStrikeWindow = false;
	m_bDone = false;
	m_pobMovementInBetween = 0;
	m_bMovementInBetweenInitialised = false;
}

bool AerialGeneralBoomerangSpecialAttack::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );

	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;

	m_pobSword = 0;
	m_pobCounteredSword = 0;
	m_bSwordCreatedForThisStrikeWindow = false;
	m_bDone = false;
	m_bMovementInBetweenInitialised = false;
	m_bSwordComingBack = false;
	m_bDoneAttack = false;
	m_bCatching = false;
	m_bNotifyNotInAir = false;
	m_bGoading = false;
	m_bDiving = false;
	m_bStartDiving = false;
	m_bGotStruck = false;

	if (m_pobBeginningMovement)
	{
		m_pobBeginningMovement->Initialise(pobBoss,pobPlayer);
	}

	// Get a point somewhere opposite in the arena to aim for while attacking, set it as the target point
	CPoint obArenaCentre = DoppelgangerManager::Get().GetMaster()->GetArenaCentre();
	float fArenaRadius = DoppelgangerManager::Get().GetMaster()->GetArenaRadius();
	
	CPoint obPlayerPosition( pobPlayer->GetPosition() );
	CPoint obBossPosition( pobAGen->GetPosition() );

	obPlayerPosition.Y() = obBossPosition.Y();
	obArenaCentre.Y() = obBossPosition.Y();

	CDirection obBossToCentre( obArenaCentre - obBossPosition );
	CDirection obBossToPlayer( obArenaCentre - obPlayerPosition );
	obBossToCentre.Normalise();
	obBossToPlayer.Normalise();
	if (m_fRandomAugmentAngle > 0.0f )
	{
		CMatrix obRot( CONSTRUCT_IDENTITY );
		float fRandomAugmentAngle = BOSS_RAND_F(m_fRandomAugmentAngle) - (m_fRandomAugmentAngle * 0.5f);
		obRot.SetFromAxisAndAngle(CDirection( 0.0f, 1.0f, 0.0f), fRandomAugmentAngle * DEG_TO_RAD_VALUE);
		obBossToCentre = obBossToCentre * obRot;
	}

	obBossToCentre *= fArenaRadius;
	m_obTargetPoint = obArenaCentre + obBossToCentre;
	
	CDirection obBossToTargetPoint( m_obTargetPoint - obBossPosition );
	obBossToTargetPoint.Normalise();

	if ( MovementControllerUtilities::RotationAboutY(obBossToTargetPoint, obBossToPlayer) < 0.0f )	
		m_pobAttackToUse = m_pobThrowAttackRight;
	else
		m_pobAttackToUse = m_pobThrowAttackLeft;

	// Orient the AGen to the centre of the arena throughout this attack
	m_obTargetPoint = DoppelgangerManager::Get().GetMaster()->GetArenaCentre();
	m_obTargetPoint.Y() += BOSS_RAND_F(7.0f);

	m_bBeingSyncCountered = m_bBeingThrownCountered = false;

	return true;
}

bool AerialGeneralBoomerangSpecialAttack::IsVulnerableTo(CStrike* pobStrike)
{
	return IsVulnerableTo(pobStrike->GetAttackDataP());
}

bool AerialGeneralBoomerangSpecialAttack::IsVulnerableTo(const CAttackData* pobAttackData)
{
	return m_bBeingSyncCountered || m_bBeingThrownCountered;
}

void AerialGeneralBoomerangSpecialAttack::NotifyMovementDone() 
{
	if (!m_bDoneAttack)
		m_pobBeginningMovement->NotifyMovementDone();
	else
	{
		if (m_bBeingSyncCountered)
		{
			m_bNotifyNotInAir = true;
		}
		else if (m_bBeingThrownCountered)
		{
			if (m_bGoading && !m_bDiving)
			{
				if (m_pobGoadingMovement)
					m_pobGoadingMovement->NotifyMovementDone();
			}
			else if (m_bGoading && m_bDiving)
			{
				if (m_pobDive)
					m_pobDive->NotifyMovementDone();
			}
		}

		m_bDone = true; 
	}
}

void AerialGeneralBoomerangSpecialAttack::NotifyPlayerInteracting(bool bState)
{
	// She's stopped interacting with her sword, so we're done with this special
	if (!bState)
		m_bDone = true;
}

void AerialGeneralBoomerangSpecialAttack::NotifyPlayerInteractionAction()
{
	m_bStartDiving = true;
}

BossAttack* AerialGeneralBoomerangSpecialAttack::NotifyAttackInterrupted()
{
	m_bGotStruck = true;
	// Return this cos even though we won't be able to do anything as we KO, we want one more update ot tidy up after ourselves
	return this;
}

// Sorry, quickest way to do this
static int s_iNumCounteredSwords = 0;
void AerialGeneralBoomerangSpecialAttack::NotifyProjectileCountered(Object_Projectile* pobProj)
{
	// Power countered swords mean I'm being sync attacked, just finish the attack
	if (m_pobAttackToUse->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM || m_pobAttackToUse->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST)
		m_bBeingSyncCountered = true;
	else
	{
		m_bBeingThrownCountered = true;

		// Get player directly
		Player* pobPlayer = const_cast< Player* >(CEntityManager::Get().GetPlayer());

		// Get somewhere sensible
		CPoint obPosition(pobPlayer->GetPosition() + pobPlayer->GetMatrix().GetZAxis());
		char pcPos[256];
		sprintf(pcPos, "%f,%f,%f", obPosition.X(), obPosition.Y(), obPosition.Z());

		// Construct a throwable sword with the same clump
		char pcName[256];
		sprintf(pcName, "Countered_Boomerang_%i", s_iNumCounteredSwords);
		DataObject* pDO = ObjectDatabase::Get().ConstructObject("Interactable_Thrown", pcName, GameGUID(), 0, true, false);
		m_pobCounteredSword = (Interactable_Thrown*)pDO->GetBasePtr();
		s_iNumCounteredSwords++;

		// Set it up like a normal thrown weapon
		LuaAttributeTable* pWeaponAttrs = LuaAttributeTable::Create();
		m_pobCounteredSword->SetAttributeTable(pWeaponAttrs);
		m_pobCounteredSword->GetAttributeTable()->SetDataObject(pDO);
		pWeaponAttrs->SetString("Name", pcName);
		pWeaponAttrs->SetString("Position", pcPos);
		pWeaponAttrs->SetBool("Attached", false);
		pWeaponAttrs->SetString("Clump", m_obCounteredSwordClump.c_str());
		pWeaponAttrs->SetString("SharedAttributes", m_obCounteredSwordSharedAttribs.c_str()); // Copy the sword
		pWeaponAttrs->SetInteger("SectorBits", pobProj->GetMappedAreaInfo());		
		ObjectDatabase::Get().DoPostLoadDefaults(pDO);

		// Send it over to the hero to play with
		Message obMessage(msg_aerialgeneralhack_forceherotointeracting);
		obMessage.SetEnt("Sword",m_pobCounteredSword);
		pobPlayer->GetMessageHandler()->Receive(obMessage);
		
		// Force player out of attacking hacktastically		
		pobPlayer->GetAttackComponent()->CompleteRecovery();
		// Clear the controllers so we don't get blendy weirdness
		//pobPlayer->GetMovement()->ClearControllers();

		// Don't want it to collide with player
		pobPlayer->GetInteractionComponent()->ExcludeCollisionWith(m_pobCounteredSword);

		// Force a recovery on the boss so he's ready to be hit
		Message obMessage2(msg_combat_recovered);
		pobProj->GetShooter()->GetMessageHandler()->Receive(obMessage2);

		// Lose the projectile
		pobProj->Destroy();
	}
}

BossAttack* AerialGeneralBoomerangSpecialAttack::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;

	pobAGen->GetBossMovement()->m_bTargetPointSet = true; 
	pobAGen->GetBossMovement()->m_obTargetPoint = m_obTargetPoint;

	if (!m_bDoneAttack && (!m_pobBeginningMovement || !m_pobBeginningMovement->DoMovement(fTimeDelta,pobBoss,pobPlayer)))
	{
		if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackToUse))
		{			
			pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bDoneAttack = true;
		}
	} 
	else if (!m_bMovementInBetweenInitialised && !m_bCatching && pobBoss->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING)
	{
		if ( !m_bSwordCreatedForThisStrikeWindow && pobBoss->GetAttackComponent()->IsInStrikeWindow() )
		{
			// Throw a sword now
			m_pobSword = Object_Projectile::CreateAGenSword(pobBoss, 0, m_pobProjectileAttributes, pobPlayer);

			CDirection obSwordToAGen( pobBoss->GetPosition() - m_pobSword->GetPosition() );
			m_fLastSwordToAGenDistance = obSwordToAGen.Length();

			m_bSwordCreatedForThisStrikeWindow = true;
		}
		else if ( m_bSwordCreatedForThisStrikeWindow && !pobBoss->GetAttackComponent()->IsInStrikeWindow() )
		{
			// Clear it for the next strike window
			m_bSwordCreatedForThisStrikeWindow = false;
		}
	}
	else if ( m_bDoneAttack )
	{
		if (!m_bMovementInBetweenInitialised && pobBoss->CanStartAnAttack())
		{
			if (m_pobMovementInBetween)
				m_pobMovementInBetween->Initialise(pobBoss,pobPlayer);
			m_bMovementInBetweenInitialised = true;
		}

		if (m_pobMovementInBetween)
			m_pobMovementInBetween->DoMovement(fTimeDelta,pobBoss,pobPlayer);

		if (m_pobSword)
		{
			// Power sword counter? Just set that we're done and return out, he'll carry on once he leaves his held state
			if (m_bBeingSyncCountered)
			{
				m_bNotifyNotInAir = true;
				m_bDone = true;
			}
			// Bit more involved, manage his 'come on hit me' and 'oh no dive away'
			else if (m_bBeingThrownCountered)
			{
				// Need to start a goading anim?
				if (!m_bGoading && !m_bDiving)
				{
					pobAGen->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();

					if (m_pobGoadingMovement)
						m_pobGoadingMovement->Initialise(pobBoss,pobPlayer);
					
					m_pobGoadingMovement->DoMovement(fTimeDelta, pobBoss,pobPlayer);

					m_bGoading = true;
				}
				// Then wait for the player to throw
				else if (m_bGoading && !m_bDiving)
				{
					pobAGen->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();

					if (m_pobGoadingMovement)
						m_pobGoadingMovement->DoMovement(fTimeDelta, pobBoss,pobPlayer);
					
					if (m_bStartDiving)
					{
						int iRand = BOSS_RAND() % 2;
						if ( iRand > 0 )
							m_pobDive = m_pobDiveLeftMovement;
						else
							m_pobDive = m_pobDiveRightMovement;

						if (m_pobDive)
						{
							m_pobDive->Initialise(pobBoss,pobPlayer);
							m_pobDive->DoMovement(fTimeDelta, pobBoss,pobPlayer);
						}
						else
						{
							m_bDone = true;
						}

						m_bDiving = true;
					}
				}
				else if (m_bGoading && m_bDiving)
				{
					pobAGen->GetBossMovement()->m_obTargetPoint = m_pobCounteredSword->GetPosition();

					if (m_bGotStruck)
					{
						m_bDone = true;
						m_bNotifyNotInAir = true;
					}
					else if (m_pobDive)
					{
						if (!m_pobDive->DoMovement(fTimeDelta, pobBoss, pobPlayer))
						{
							m_bDone = true;
						}
					}
					else
					{
						m_bDone = true;
					}
				}
			}
			else
			{
				CPoint obSwordOffsetAGenPosition( pobBoss->GetPosition() );
				obSwordOffsetAGenPosition.Y() += 1.0f;
				CDirection obSwordToAGen( obSwordOffsetAGenPosition - m_pobSword->GetPosition() );
				CDirection obAGenToPlayer( pobPlayer->GetPosition() - pobBoss->GetPosition() );
				float fSwordToAGenDistance = obSwordToAGen.Length();

				if (fSwordToAGenDistance <= m_fCatchDistance && !m_bCatching)
				{
					if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobCatchAttack))
					{			
						pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
						m_bCatching = true;
					}
				}
				else
				{
					if ( (m_pobSword->AGenSwordHasMissedTarget() && !m_pobSword->AGenSwordIsFrozen() && !m_bSwordComingBack)
						||
						( (m_pobSword->AGenSwordIsFrozen() || !m_pobSword->AGenSwordIsMoving()) && m_bSwordComingBack ))
					{
						m_pobSword->AGenSwordBringBack();
						m_bSwordComingBack = true;
					}					
				}

				if (m_pobSword && m_bCatching && pobBoss->GetAttackComponent()->IsInStrikeWindow())
				{
					// Destroy sword
					m_pobSword->AGenSwordDestroy();
					m_pobSword = 0;

					m_bMovementInBetweenInitialised = false; // We'll be pushing this off our movement
				}

				m_fLastSwordToAGenDistance = fSwordToAGenDistance;
			
				m_bDone = m_bDoneAttack && m_bCatching && !m_pobSword && pobBoss->CanStartAnAttack();
			}
		}
		else
		{
			m_bDone = m_bDoneAttack && m_bCatching && !m_pobSword && pobBoss->CanStartAnAttack();
		}
	}

	if (m_bDone)
	{
		if (m_bNotifyNotInAir)
		{
			pobAGen->SetIsInAir(false);
		}

		if (m_pobSword)
			m_pobSword->Destroy();

		return 0;
	}
	else
		return this;
}

void AerialGeneralBoomerangSpecialAttack::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderPoint(m_obTargetPoint, 5.0f, DC_RED);

	if (m_bBeingSyncCountered)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Being sync countered.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	}
	else if (m_bBeingThrownCountered)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Being thrown countered.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	}
	else if (m_bMovementInBetweenInitialised && m_bDoneAttack && !m_bCatching)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Waiting.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	}
	else if (m_bDoneAttack && !m_bCatching && !m_bMovementInBetweenInitialised)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Throwing.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	}
	else if (m_bDoneAttack && m_bCatching && m_bMovementInBetweenInitialised)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Catching.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	}
	else
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Swooping.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
		if (m_pobBeginningMovement)
		m_pobBeginningMovement->DebugRender(obScreenLocation,fXOffset,fYOffset + DEBUG_SHIFT_AMOUNT);
	}
#endif
}

//--------------------------------------------------
//!
//! AerialGeneralSwoopMovement
//!
//--------------------------------------------------
BossMovement* AerialGeneralSwoopMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );
	
	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
	if (pobAGen->IsInAir())
	{
		m_fTimeInMovement = 0.0f;

		CPoint obArenaCentre = DoppelgangerManager::Get().GetMaster()->GetArenaCentre();
		float fArenaRadius = DoppelgangerManager::Get().GetMaster()->GetArenaRadius();

		CPoint obPlayerPosition( pobPlayer->GetPosition() );
		obPlayerPosition.Y() = pobBoss->GetPosition().Y();
		obArenaCentre.Y() = pobBoss->GetPosition().Y();
		CDirection obPlayerToCentre( obArenaCentre - obPlayerPosition );
		obPlayerToCentre.Normalise();
		m_obFromCentreVector = obPlayerToCentre;
		
		bool bGoLeft = true;
		if (m_fToDistanceInFrontOfPlayer > 0.0f)
		{
			obPlayerToCentre.Normalise();
			obPlayerToCentre *= m_fToDistanceInFrontOfPlayer;
			m_obSwoopPoint = obPlayerPosition + obPlayerToCentre;
		}
		else 
		{
			obPlayerToCentre *= fArenaRadius;

			if (m_fRandomAugmentAngle > 0.0f )
			{
				CMatrix obRot( CONSTRUCT_IDENTITY );
				float fRandomAugmentAngle = BOSS_RAND_F(m_fRandomAugmentAngle) - (m_fRandomAugmentAngle * 0.5f);
				if (fRandomAugmentAngle > 0.0f)
					bGoLeft = false;
				CCamUtil::MatrixFromEuler_XYZ(obRot, 0.0f, fRandomAugmentAngle * DEG_TO_RAD_VALUE, 0.0f);
				obPlayerToCentre = obPlayerToCentre * obRot;
			}

			m_obSwoopPoint = obArenaCentre + obPlayerToCentre;
			m_obSwoopPoint.Y() += m_fHeightAdjustMinimum + BOSS_RAND_F(m_fHeightAdjust);
		}

		CDirection obAGenToSwoopPoint( m_obSwoopPoint - pobBoss->GetPosition() );
		CDirection obAGenToSwoopPointN( obAGenToSwoopPoint );
		obAGenToSwoopPointN.Normalise();

		ZAxisAlignTargetedTransitionDef obStartAlignDef;
		obStartAlignDef.SetDebugNames("AGen Turn to Swoop","ZAxisAlignTargetedTransitionDef");
		obStartAlignDef.m_obAlignZTo = obAGenToSwoopPointN;
		if (bGoLeft)
			obStartAlignDef.m_obAnimationName = m_obTurnLeftAnim;
		else
			obStartAlignDef.m_obAnimationName = m_obTurnRightAnim;
		obStartAlignDef.m_bApplyGravity = false;
		
		ZAxisAlignTargetedTransitionDef obSwoop;
		obSwoop.SetDebugNames("AGen Swoop","ZAxisAlignTargetedTransitionDef");
		obSwoop.m_obAlignZTo = obAGenToSwoopPointN;
		obSwoop.m_obAnimationName = m_obSwoopAnim;
		obSwoop.m_bApplyGravity = false;
		obSwoop.m_obScaleToCoverDistance = CPoint( obAGenToSwoopPoint );
		obSwoop.m_obScaleToCoverDistance.X() = fabs(obSwoop.m_obScaleToCoverDistance.X());
		obSwoop.m_obScaleToCoverDistance.Y() = fabs(obSwoop.m_obScaleToCoverDistance.Y());
		obSwoop.m_obScaleToCoverDistance.Z() = fabs(obSwoop.m_obScaleToCoverDistance.Z());

		ZAxisAlignTargetedTransitionDef obEndAlignDef;
		obEndAlignDef.SetDebugNames("AGen Turn to target","ZAxisAlignTargetedTransitionDef");
		obEndAlignDef.m_pobEntityAlignZTowards = pobPlayer;
		// Turn the opposite way to before
		if (bGoLeft)
			obEndAlignDef.m_obAnimationName = m_obTurnRightAnim;
		else
			obEndAlignDef.m_obAnimationName = m_obTurnLeftAnim;
		obEndAlignDef.m_bApplyGravity = false;

		pobBoss->GetMovement()->ClearControllers();
		pobBoss->GetMovement()->BringInNewController( obStartAlignDef, CMovement::DMM_STANDARD, 0.0f );
		pobBoss->GetMovement()->AddChainedController( obSwoop, CMovement::DMM_STANDARD, 0.0f );
		pobBoss->GetMovement()->AddChainedController( obEndAlignDef, CMovement::DMM_STANDARD, 0.0f );

		Message obMovementMessage(msg_movementdone);
		pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

		m_bDone = false;

		return this;
	}
	else
	{
		return 0;
	}
}

BossMovement* AerialGeneralSwoopMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );

	m_fTimeInMovement += fTimeDelta;
	
	if (!m_bDone)
	{
		return this;
	}
	else
	{
		return 0;
	}
}

void AerialGeneralSwoopMovement::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderPoint(m_obSwoopPoint,10.0f,DC_RED);
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Throwing/catching.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
#endif
}

//--------------------------------------------------
//!
//! AerialGeneralMachineGunSpecialAttack
//!
//--------------------------------------------------
AerialGeneralMachineGunSpecialAttack::AerialGeneralMachineGunSpecialAttack()
{
	m_pobThrowAttack = 0;
	m_pobProjectileAttributes = 0;
	m_bSwordCreatedForThisStrikeWindow = false;
	m_bDone = false;
	m_pobMovementInBetween = 0;
	m_bMovementInBetweenInitialised = false;
}

void AerialGeneralMachineGunSpecialAttack::NotifyMovementDone() 
{
	if (m_pobBeginningMovement)
	{
		m_pobBeginningMovement->NotifyMovementDone();
		m_bBeginningMovementDone = true;
	}
	if (m_pobMovementInBetween)
		m_pobMovementInBetween->NotifyMovementDone();
}

bool AerialGeneralMachineGunSpecialAttack::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{	
	m_bSwordCreatedForThisStrikeWindow = false;
	m_bDone = false;
	m_bCatching = false;
	m_bMovementInBetweenInitialised = false;
	m_bStartedCatchAttack = false;
	m_fAttackTimer = 0.0f;
	m_iSwordsThrowSoFarThisAttack = 0;

	if (m_iNumAttacksToDoAdjust > 0)
		m_iRemainingAttacks = m_iNumAttacksToDo - BOSS_RAND() % m_iNumAttacksToDoAdjust;
	else
		m_iRemainingAttacks = m_iNumAttacksToDo;
	if ( m_iRemainingAttacks <= 0 )
		m_iRemainingAttacks = 1;

	if (m_pobBeginningMovement)
	{
		m_pobBeginningMovement->Initialise(pobBoss,pobPlayer);
		m_bBeginningMovementDone = false;
	}
	else
		m_bBeginningMovementDone = true;
	
	return true;
}

BossAttack* AerialGeneralMachineGunSpecialAttack::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{	
	if (m_bBeginningMovementDone)
	{
		ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );

		// Update our inbetween movement if needed
		BossMovement* pobInBetweenMovementNotDone = 0;
		if (m_pobMovementInBetween && m_bMovementInBetweenInitialised)
			pobInBetweenMovementNotDone = m_pobMovementInBetween->DoMovement(fTimeDelta,pobBoss,pobPlayer);

		m_fAttackTimer += fTimeDelta;

		// If we're not catching and we have some attacks to do
		if ( !m_bCatching )
		{
			// If we have an attack left, and it's time to start it
			if ( m_iRemainingAttacks > 0 && m_fAttackTimer >= m_fTimeBetweenAttacks && pobBoss->CanStartAnAttack() && !pobInBetweenMovementNotDone)
			{
				if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobThrowAttack))
				{			
					pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
					
					m_fAttackTimer = 0.0f;
					m_iRemainingAttacks--;
					m_bMovementInBetweenInitialised = false;
					m_iSwordsThrowSoFarThisAttack = 0;
				}
			} 
			// If we're attacking we need to spawn lots of swords
			else if ( pobBoss->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING )
			{
				// Keep zeroing this so wehn we stop attacking, it's zero
				m_fAttackTimer = 0.0f;

				if ( !m_bSwordCreatedForThisStrikeWindow && pobBoss->GetAttackComponent()->IsInStrikeWindow() )
				{
					// Throw a sword now
					CPoint obPlayerPosition( pobPlayer->GetPosition() );
					obPlayerPosition.Y() = DoppelgangerManager::Get().GetMaster()->GetArenaCentre().Y() + 1.0f;
					CDirection obToPlayer( obPlayerPosition - pobBoss->GetPosition() );
					obToPlayer.Normalise();
					CDirection obDownish( obToPlayer + m_obDownishVector );
					obDownish.Normalise();
					float fLerpValue = (float)m_iSwordsThrowSoFarThisAttack / (float)m_iNumberOfThrowsTillOnTarget;
					fLerpValue > 1.0f ? fLerpValue = 1.0f : fLerpValue = fLerpValue;
					CDirection obThrowVector = CDirection::Lerp( obDownish, obToPlayer, fLerpValue );

					Object_Projectile* pobSword = Object_Projectile::CreateAGenSword(pobBoss, 0, m_pobProjectileAttributes, pobPlayer, &obThrowVector );
					m_iSwordsThrowSoFarThisAttack++;
					pobSword->AGenSwordSetBringBackOnImpact(false); // Won't come back on impact with heroine, assuming data set up to stick in ground
					m_apobSwords.push_back(pobSword);
					m_bSwordCreatedForThisStrikeWindow = true;
				}
				else if ( m_bSwordCreatedForThisStrikeWindow && !pobBoss->GetAttackComponent()->IsInStrikeWindow() )
				{
					// Clear it for the next strike window
					m_bSwordCreatedForThisStrikeWindow = false;
				}
			}
			// If we're waiting to start another attack and need therefore to have our in-between movement started
			// Or our catching started
			else if ( m_fAttackTimer < m_fTimeBetweenAttacks && !m_bMovementInBetweenInitialised && pobBoss->CanStartAnAttack() )
			{
				if (m_pobMovementInBetween)
					m_pobMovementInBetween->Initialise(pobBoss,pobPlayer);
				m_bMovementInBetweenInitialised = true;
			}
			else if ( m_iRemainingAttacks == 0 )
			{
				// Wait until the time limit is up and then start bringing them in
				if (m_fAttackTimer >= m_fTimeTillSwordReclaim)
				{
					m_bCatching = true;
				}
			}
		}
		else
		{
			bool bSkipEndCheckThisFrame = false;

			// While catching, loop through all of our swords to check distances away to either start the catch anim, or make them disappear
			for ( ntstd::Vector<Object_Projectile*, Mem::MC_MISC>::iterator obIt = m_apobSwords.begin(); 
					obIt != m_apobSwords.end(); 
					obIt++ )
			{
				CPoint obSwordOffsetAGenPosition( pobBoss->GetPosition() );
				obSwordOffsetAGenPosition.Y() += 1.0f;
				CDirection obSwordToAGen( obSwordOffsetAGenPosition - (*obIt)->GetPosition() );
				CDirection obAGenToPlayer( pobPlayer->GetPosition() - pobBoss->GetPosition() );
				float fSwordToAGenDistance = obSwordToAGen.Length();

				// Keep doing this so it just goes back to the Agen perfectly
				(*obIt)->AGenSwordBringBack();

				if ( fSwordToAGenDistance <= m_fStartCatchDistance && pobBoss->CanStartAnAttack() && pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobCatchAttack) )
				{	
					pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
					m_bStartedCatchAttack = true;
					bSkipEndCheckThisFrame = true; // Needed so the check below waits a frame for the message to get across
				}

				if (fSwordToAGenDistance <= m_fSwordDestroyDistance)
				{
					(*obIt)->AGenSwordDestroy();
					m_apobSwords.erase( obIt );
					obIt--;
				}
			}

			// If we've started our catch attack, and we're able to catch again...
			if ( !bSkipEndCheckThisFrame && m_apobSwords.size() == 0 && pobBoss->CanStartAnAttack() )
			{
				// ...then we're done overall.
				m_bDone = true;
			}

			if ( !bSkipEndCheckThisFrame && !m_bMovementInBetweenInitialised && pobBoss->CanStartAnAttack() )
			{			
				if (m_pobMovementInBetween)
					m_pobMovementInBetween->Initialise(pobBoss,pobPlayer);
				m_bMovementInBetweenInitialised = true;
			}
		}
	}
	else
	{
		m_pobBeginningMovement->DoMovement(fTimeDelta,pobBoss,pobPlayer);
	}

	if (m_bDone)
		return 0;
	else
		return this;
}

void AerialGeneralMachineGunSpecialAttack::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	if (m_bMovementInBetweenInitialised)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Waiting.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
		if (m_pobMovementInBetween)
			m_pobMovementInBetween->DebugRender(obScreenLocation,fXOffset,fYOffset + DEBUG_SHIFT_AMOUNT);
	}
	else if (m_bCatching)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Catching swords.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	}
	else if (m_iRemainingAttacks > 0)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Attacking, %i left.", ObjectDatabase::Get().GetNameFromPointer(this).GetString(), m_iRemainingAttacks );
	}
	else if (m_pobBeginningMovement)
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Doing beginning movement.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
		m_pobBeginningMovement->DebugRender(obScreenLocation,fXOffset,fYOffset + DEBUG_SHIFT_AMOUNT);
	}
	else
	{
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Boomerang %s: Is doing... something.", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	}
#endif
}

//--------------------------------------------------
//!
//! AerialGeneralTeleportingMeleeAttack
//!
//--------------------------------------------------
BossAttack* AerialGeneralTeleportingMeleeAttack::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntAssert( m_pobCurrentAttackLink );

	if (pobBoss->CanStartALinkedAttack() && !m_bDecidedOnLink)
	{
		// Decide to link or not
		float fRand = BOSS_RAND_F(1.0f);

		// Tokenise probability string using , as a delimeter
		ntstd::Vector<ntstd::String> obProbabilities;
		char cBuffer[256];
		ntAssert(ntStr::GetLength(m_obProbabilityString.c_str()) < 256);
		strcpy(cBuffer, ntStr::GetString(m_obProbabilityString.c_str()));
		char* pcNext = strtok(cBuffer, ",");	
		while (pcNext != 0)
		{
			obProbabilities.push_back(ntstd::String(pcNext));
			pcNext = strtok(0, ",");
		}

		// Find the one for the current depth
		float fProbability = 0.0f;
		if ((int)obProbabilities.size() > m_iDepth)
			sscanf(obProbabilities[m_iDepth].c_str(),"%f",&fProbability);
		else
			ntPrintf("AerialGeneralTeleportingMeleeAttack link probability for depth %i not found in string. Attack will not link.\n",m_iDepth);

		if (fProbability > fRand)
		{
			// Is it a teleporter?
			bool bTeleport = false;
			// Tokenise teleportation string using , as a delimeter
			ntstd::Vector<ntstd::String> obTeleports;
			char cBuffer[256];
			ntAssert(ntStr::GetLength(m_obTeleportString.c_str()) < 256);
			strcpy(cBuffer, ntStr::GetString(m_obTeleportString.c_str()));
			char* pcNext = strtok(cBuffer, ",");	
			while (pcNext != 0)
			{
				obTeleports.push_back(ntstd::String(pcNext));
				pcNext = strtok(0, ",");
			}
			// Find the one for the current depth
			char cTeleport = '-';
			if ((int)obTeleports.size() > m_iDepth)
				sscanf(obTeleports[m_iDepth].c_str(),"%c",&cTeleport);
			else
				ntPrintf("AerialGeneralTeleportingMeleeAttack teleport for depth %i not found in string. Attack will not teleport.\n",m_iDepth);
			bTeleport = cTeleport == 'T';

			if (bTeleport)
			{
				// Reflect the AGens position about the player
				CPoint obBossPlayerRelativePosition( pobBoss->GetPosition() - pobPlayer->GetPosition() );
				obBossPlayerRelativePosition *= -1;
				CMatrix obMtx = pobBoss->GetMatrix();
				obMtx.SetTranslation( CPoint(CONSTRUCT_CLEAR) );
				obMtx.SetXAxis( obMtx.GetXAxis() * -1 );
				obMtx.SetZAxis( obMtx.GetZAxis() * -1 );
				CPoint obNewPosition( pobPlayer->GetPosition() + obBossPlayerRelativePosition );
				float fBudge = 0.41f;
				obNewPosition.Y() += fBudge;
				ntstd::List<CEntity*> obEntityList;
				if(!Physics::CPhysicsWorld::Get().FindIntersectingRigidEntities(obNewPosition, 0.4f, obEntityList))
				{
					obNewPosition.Y() -= fBudge;
					pobBoss->SetRotation( CQuat(obMtx) );
					pobBoss->SetPosition( obNewPosition );
				}
			}

			// We link to the next attack
			const CAttackLink* pobNextAttack = BossAttackPhase::GetNextAttackLink(m_pobCurrentAttackLink);
			if ( pobNextAttack )
			{
				// This'll push on our attack
				if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectNextAttack(pobNextAttack))
				{
					// This'll start it 
					pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
					m_pobCurrentAttackLink = pobNextAttack;
					m_iDepth++;
				}
				else
				{
					m_bFinished = true;
				}
			}
		}

		m_bDecidedOnLink = true;
	}

	if (m_bFinished)
		return 0;
	else
		return this;
}

void AerialGeneralDoppelgangerSpawningBossAttackPhase::NotifyGotStruck(Boss* pobBoss)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );
	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;

	if (pobBoss->GetAttackComponent()->AI_Access_GetState() == CS_KO)
		pobAGen->WakeUpDoppelganger();
}

//--------------------------------------------------
//!
//! BossPlayerRelativeVectorMovement 
//! Class to do player relative movement at a specified speed
//!
//--------------------------------------------------
BossMovement* AerialGeneralPlayerRelativeVectorMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );

	float fArenaCentreDistance = DoppelgangerManager::Get().GetArenaFormationDistance((AerialGeneral*)pobBoss);

	CDirection obToPlayer = CDirection( pobPlayer->GetPosition() - pobBoss->GetPosition() );
	float fDistanceSquared = obToPlayer.LengthSquared();
	obToPlayer.Normalise();

	// Do we need to obey the doppelganger maanger in whether we orbit around the centre of the arena?
	if ( fArenaCentreDistance > 0.0f )
	{
		AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
		bool bFormingAroundPlayer = false;
		CPoint obPointOfInterest( DoppelgangerManager::Get().GetMaster()->GetArenaCentre() );
		// Are we so close to the player that we need to move away from them?
		if ( fDistanceSquared < m_fDistanceToFormAroundPlayer*m_fDistanceToFormAroundPlayer )
		{
			bFormingAroundPlayer = true;
			obPointOfInterest = pobPlayer->GetPosition();
		}

		CDirection obToPointOfInterest( obPointOfInterest - pobAGen->GetPosition() );
		obToPointOfInterest.Normalise();

		//g_VisualDebug->RenderPoint(obPointOfInterest, 10, DC_RED);
		//g_VisualDebug->RenderLine(obPointOfInterest,obPointOfInterest+obToPointOfInterest, DC_RED);

		// We form up around the point acc to our assigned distance
		CPoint obGetToPosition( obPointOfInterest + (obToPointOfInterest * -fArenaCentreDistance) );
		CDirection obToGetToPoint( obGetToPosition - pobBoss->GetPosition() );
		float fToGetToPointDistanceSquared = obToGetToPoint.LengthSquared();
		obToGetToPoint.Normalise();
		
		//g_VisualDebug->RenderPoint(obGetToPosition, 10, DC_GREEN);
		//g_VisualDebug->RenderLine(obGetToPosition,obGetToPosition+obToGetToPoint, DC_GREEN);

		// Are we close enough to where we need to fall in?
		if ( bFormingAroundPlayer || fToGetToPointDistanceSquared < m_fFormationTolerance*m_fFormationTolerance )
		{
			// If so don't need to walk towards the get to point, start strafing
			// See which direction out vector takes us, left or right
			CMatrix obMtx;
			if (bFormingAroundPlayer)
				obMtx.SetXAxis( obToPlayer.Cross( CDirection( 0.0f, 1.0f, 0.0f ) ) );
			else
				obMtx.SetXAxis( obToGetToPoint.Cross( CDirection( 0.0f, 1.0f, 0.0f ) ) );
			obMtx.SetYAxis( CDirection( 0.0f, 1.0f, 0.0f ) );
			if (bFormingAroundPlayer)
				obMtx.SetZAxis( obToPlayer );
			else
                obMtx.SetZAxis( obToGetToPoint );
			CDirection obReferenceVector( m_obVector * obMtx );
			obReferenceVector.Normalise();

			//g_VisualDebug->RenderLine(pobAGen->GetPosition(),pobAGen->GetPosition()+obReferenceVector, DC_YELLOW);

			float fAngle = MovementControllerUtilities::RotationAboutY( obToPointOfInterest, obReferenceVector );

			// Set to strafe either left or right depending on the angle
			pobBoss->GetBossMovement()->m_obFacingDirection = obToPlayer;
			if ( fAngle < 5.0f )
				pobBoss->GetBossMovement()->m_obMoveDirection = obMtx.GetXAxis() * -1;
			else
				pobBoss->GetBossMovement()->m_obMoveDirection = obMtx.GetXAxis();
			//g_VisualDebug->RenderLine(pobAGen->GetPosition(),pobAGen->GetPosition()+pobBoss->GetBossMovement()->m_obMoveDirection, DC_CYAN);
			pobBoss->GetBossMovement()->m_fMoveSpeed = m_fSpeed;
			pobBoss->GetBossMovement()->m_obTargetPoint = obPointOfInterest;
			pobBoss->GetBossMovement()->m_bTargetPointSet = true;
		}		
		else
		{
			// Walk towards where we need to fall in
			pobBoss->GetBossMovement()->m_obFacingDirection = obToPlayer;
			pobBoss->GetBossMovement()->m_obMoveDirection = obToGetToPoint;
			pobBoss->GetBossMovement()->m_fMoveSpeed = m_fSpeed;
			pobBoss->GetBossMovement()->m_obTargetPoint = obGetToPosition;
			pobBoss->GetBossMovement()->m_bTargetPointSet = true;
		}
	}
	else
	{
		m_fTimeInMovement += fTimeDelta;

		CMatrix obMtx;
		obMtx.SetXAxis( obToPlayer.Cross( CDirection( 0.0f, 1.0f, 0.0f ) ) );
		obMtx.SetYAxis( CDirection( 0.0f, 1.0f, 0.0f ) );
		obMtx.SetZAxis( obToPlayer );

		pobBoss->GetBossMovement()->m_obFacingDirection = obToPlayer;
		pobBoss->GetBossMovement()->m_obMoveDirection = m_obVector * obMtx;
		pobBoss->GetBossMovement()->m_fMoveSpeed = m_fSpeed;
		pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		if (m_fTimeInMovement - fTimeDelta > 0.0f)
		pobBoss->GetBossMovement()->m_bTargetPointSet = true;
		else
			pobBoss->GetBossMovement()->m_bTargetPointSet = false;

		bool bAugmentDirection = false;
		if (pobBoss->GetNavMan() && pobBoss->GetNavMan()->IsPointInAvoidanceArea(pobBoss->GetBossMovement()->m_obTargetPoint))
		{
			//Attempt a ray-cast between the boss and the player to see if it hits any static geometry. This check will only occur when
			//both the boss and player are in avoidance areas, and determines whether or not to still avoid the area (something in the
			//avoidance area between them, stopping a direct path from one to the other).
			Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
			obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
			obFlag.flags.i_collide_with = ( Physics::CHARACTER_CONTROLLER_ENEMY_BIT | Physics::RAGDOLL_BIT | Physics::SMALL_INTERACTABLE_BIT |
				Physics::LARGE_INTERACTABLE_BIT );

			ntstd::List<CEntity*> obIgnoreEntityList;
			obIgnoreEntityList.push_back(pobBoss);
			obIgnoreEntityList.push_back(pobPlayer);

			CPoint obStart(pobBoss->GetPosition() + CPoint(0.0f, 0.25f, 0.0f));
			CPoint obEnd(pobPlayer->GetPosition() + CPoint(0.0f, 0.25f, 0.0f));

			Physics::TRACE_LINE_QUERY stActualRaycast;

			bAugmentDirection = Physics::CPhysicsWorld::Get().TraceLine(obStart, obEnd, obIgnoreEntityList, stActualRaycast, obFlag);

	#ifndef _GOLD_MASTER
			//Render the ray
			g_VisualDebug->RenderLine(obStart, obEnd, (bAugmentDirection ? 0xffff0000 : 0xffffffff));
	#endif
		}
	//	else if (pobBoss->GetNavMan() && !pobBoss->GetNavMan()->IsPointInAvoidanceArea(pobBoss->GetBossMovement()->m_obTargetPoint))
		if (pobBoss->GetNavMan() && bAugmentDirection)
		{
			pobBoss->GetNavMan()->AugmentVector(pobBoss->GetPosition(), pobBoss->GetBossMovement()->m_obMoveDirection, pobBoss->GetBossMovement()->m_fMoveSpeed, 5.0f, false );
		}
		pobBoss->GetBossMovement()->m_obMoveDirection.Normalise();
		if (fDistanceSquared < m_fStopDistance*m_fStopDistance)
			pobBoss->GetBossMovement()->m_fMoveSpeed = 0.0f;
	}

	if (m_fTimeInMovement < m_fTimeToDoMovementExclusivelyThisTime)
		return this;
	else
		return 0;
}

void AerialGeneralDoppelgangerSpawningStartTransition::BeginStartTransition(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError( pobBoss->GetBossType() == Boss::BT_AERIAL_GENERAL );

	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
	for (int i = 0; i < m_iNumToWakeUp; i++)
	{
		pobAGen->WakeUpDoppelganger(false,true);
	}
}

bool AerialGeneralDoppelgangerSpawningStartTransition::Update(float /*fTimeDelta*/, Boss* /*pobBoss*/, CEntity* /*pobPlayer*/)
{
	return true;
}
