#include "entityboss.h"
#include "fsm.h"

#include "game/entitymanager.h"
#include "objectdatabase/dataobject.h"
#include "core/timer.h"
#include "game/attacks.h"
#include "game/awareness.h"	
#include "Physics/system.h"
#include "movement.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "anim/animator.h"
#include "game/strike.h"
#include "game/randmanager.h"
#include "messagehandler.h"
#include "core/visualdebugger.h" 
#include "simpletransition.h"
#include "targetedtransition.h"
#include "game/interactioncomponent.h"
#include "game/nsmanager.h"
#include "simpletransition.h"
#include "camera/camutils.h"

#include "entitygladiatorgeneral.h"
#include "entitywatergeneral.h"
#include "entitykingbohan.h"
#include "entitydemon.h"
#include "entityaerialgeneral.h"

#include "game/combatstyle.h"
#include "physics/world.h"

//------------------------------------------------------------------------------------------
// Boss XML Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(Boss)
	COPY_INTERFACE_FROM(Character)
	DEFINE_INTERFACE_INHERITANCE(Character)

	PUBLISH_VAR_AS(m_obDefaultWalkingController, DefaultWalkingController)
	PUBLISH_VAR_AS(m_obInitialAttackPhase, InitialAttackPhase)
	PUBLISH_PTR_CONTAINER_AS(m_obAvoidanceAreas, AvoidanceAreas)
	OVERRIDE_DEFAULT(Description, "boss")
	OVERRIDE_DEFAULT(SceneElementDef, "ImportantAISceneElement")
	OVERRIDE_DEFAULT(HairConstruction, "")
	OVERRIDE_DEFAULT(ReactionMatrix, "NULL")
	OVERRIDE_DEFAULT(WeaponConstruction, "")

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
END_STD_INTERFACE

// Attack selectors exposed:
START_STD_INTERFACE(BossAttackSelector)
	PUBLISH_PTR_CONTAINER_AS(m_obAttacks, Attacks)
	PUBLISH_VAR_AS(m_fMaxPriority, MaxPriority)
	PUBLISH_VAR_AS(m_fMinPriority, MinPriority)
END_STD_INTERFACE

START_STD_INTERFACE(RandomBossAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)
END_STD_INTERFACE

START_STD_INTERFACE(HealthBossAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)

	PUBLISH_VAR_AS(m_fHealthLostThreshold, HealthLostThreshold)
	PUBLISH_VAR_AS(m_fHealthSampleRate, HealthSampleRate)
END_STD_INTERFACE

START_STD_INTERFACE(UnderAttackBossAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)
END_STD_INTERFACE

START_STD_INTERFACE(DistanceSuitabilityBossAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)
END_STD_INTERFACE

// End conditions exposed:
START_STD_INTERFACE(BossAttackPhaseEndCondition)
	PUBLISH_VAR_AS(m_bNotCondition, NotCondition)
END_STD_INTERFACE

START_STD_INTERFACE(HealthBossAttackPhaseEndCondition)
	COPY_INTERFACE_FROM(BossAttackPhaseEndCondition)
	DEFINE_INTERFACE_INHERITANCE(BossAttackPhaseEndCondition)
	PUBLISH_VAR_AS(m_fHealthThreshold, HealthThreshold)
END_STD_INTERFACE

// Start/end transitions exposed:
START_STD_INTERFACE(BossAttackPhaseEndTransition)
	PUBLISH_PTR_AS(m_pobToPhase, ToPhase)
END_STD_INTERFACE

START_STD_INTERFACE(BossAttackPhaseEndTransitionSpecialAttacking)
	COPY_INTERFACE_FROM(BossAttackPhaseEndTransition)
	DEFINE_INTERFACE_INHERITANCE(BossAttackPhaseEndTransition)

	PUBLISH_PTR_AS(m_pobEndAttack, EndAttack)
END_STD_INTERFACE

START_STD_INTERFACE(BossAttackPhaseEndTransitionAnimated)
	COPY_INTERFACE_FROM(BossAttackPhaseEndTransition)
	DEFINE_INTERFACE_INHERITANCE(BossAttackPhaseEndTransition)

	PUBLISH_VAR_AS(m_obEndAnimName, EndAnimName)
END_STD_INTERFACE

START_STD_INTERFACE(BossAttackPhaseEndTransitionWithMovement)
	COPY_INTERFACE_FROM(BossAttackPhaseEndTransition)
	DEFINE_INTERFACE_INHERITANCE(BossAttackPhaseEndTransition)

	PUBLISH_PTR_AS(m_pobMovementToDo, MovementToDo)
END_STD_INTERFACE

START_STD_INTERFACE(BossAttackPhaseStartTransitionSpecialAttacking)

	PUBLISH_PTR_AS(m_pobStartAttack, StartAttack)
END_STD_INTERFACE

START_STD_INTERFACE(BossAttackPhaseStartTransitionAnimated)
	PUBLISH_VAR_AS(m_obStartAnimName, StartAnimName)
END_STD_INTERFACE

START_STD_INTERFACE(BossMeleeAttack)
	PUBLISH_VAR_AS(m_fAttackProbability, ProbabilityOfSelection)
	PUBLISH_PTR_AS(m_pobRootAttackLink, RootAttackLink)
	PUBLISH_VAR_AS(m_bVulnerableDuring, VulnerableDuring)
	PUBLISH_VAR_AS(m_bVulnerableDuringFailedStrikeRecovery, VulnerableDuringFailedStrikeRecovery)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bInvulnerableDuringRecovery, 0, InvulnerableDuringRecovery)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUnInterruptableDuringRecovery, 0, UnInterruptableDuringRecovery)
	PUBLISH_VAR_AS(m_obProbabilityString,ProbabilityString)
END_STD_INTERFACE

START_STD_INTERFACE(BossSpecialAttack)
//	PUBLISH_VAR_AS(m_fAttackProbability, ProbabilityOfSelection)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fAttackProbability, 1.0f, ProbabilityOfSelection)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bInvulnerableDuringRecovery, 0, InvulnerableDuringRecovery)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUnInterruptableDuringRecovery, 0, UnInterruptableDuringRecovery)
END_STD_INTERFACE

// Attack phase stuff exposed:
START_STD_INTERFACE(BossAttackPhase)	
	PUBLISH_PTR_AS(m_pobStartTransition, StartTransition)
	PUBLISH_PTR_AS(m_pobEndTransition, EndTransition)

	PUBLISH_VAR_AS(m_obSuperStyleNinjaSequenceEntityName, SuperStyleNinjaSequenceEntityName)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSuperStyleNinjaSequenceHealthThreshold, 10.0f, SuperStyleNinjaSequenceHealthThreshold)

	PUBLISH_PTR_CONTAINER_AS(m_obAttackPhaseStates, AttackPhaseStates)
	PUBLISH_PTR_CONTAINER_AS(m_obEndConditions, EndConditions)
END_STD_INTERFACE

START_STD_INTERFACE(BossAttackPhaseState)
	PUBLISH_VAR_AS(m_fMaxTimeInState, MaxTimeInState)
	PUBLISH_VAR_AS(m_fMaxTimeInStateAdjust, MaxTimeInStateAdjust)
	PUBLISH_VAR_AS(m_fProbabilityOfSwitchingToState, ProbabilityOfSwitchingToState)
	PUBLISH_VAR_AS(m_bVulnerableWhileNotAttacking,VulnerableWhileNotAttacking)
	PUBLISH_VAR_AS(m_fTimeToRememberStrikes,TimeToRememberStrikes)
	PUBLISH_VAR_AS(m_iNumberOfStrikesTillInvulnerable,NumberOfStrikesTillInvulnerable)
	PUBLISH_VAR_AS(m_fTimeToBeInvulnerable,TimeToBeInvulnerable)
	PUBLISH_VAR_AS(m_obRecoilingDodgeProbabilityString, RecoilingDodgeProbabilityString)
	PUBLISH_VAR_AS(m_obDeflectingDodgeProbabilityString, DeflectingDodgeProbabilityString)
	PUBLISH_PTR_AS(m_pobDodgeMovementSelector, DodgeMovementSelector)
	PUBLISH_PTR_AS(m_pobDodgeAttackSelector, DodgeAttackSelector)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fProbabilityOfDodgeMovement, 0.5f, ProbabilityOfDodgeMovement)
	PUBLISH_VAR_AS(m_fSpeedBlockProbability,m_fSpeedBlockProbability)
	PUBLISH_VAR_AS(m_fPowerBlockProbability,m_fPowerBlockProbability)
	PUBLISH_VAR_AS(m_fRangeBlockProbability,m_fRangeBlockProbability)

	PUBLISH_PTR_CONTAINER_AS(m_obAttackSelectors, AttackSelectors)
	PUBLISH_PTR_CONTAINER_AS(m_obMovementSelectors, MovementSelectors)
END_STD_INTERFACE

START_STD_INTERFACE(BossTauntMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTauntAnim, "NULL", TauntAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bGravity, false, ApplyGravity)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUseSimpleTransition, false, UseSimpleTransition)
END_STD_INTERFACE

START_STD_INTERFACE(BossMovementSelector)
	PUBLISH_PTR_CONTAINER_AS(m_obMovements, Movements)
	PUBLISH_VAR_AS(m_fMinPriority, MinPriority)
	PUBLISH_VAR_AS(m_fMaxPriority, MaxPriority)
END_STD_INTERFACE

START_STD_INTERFACE(BossTauntMovementSelector)
	COPY_INTERFACE_FROM(BossMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossMovementSelector)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeSinceAttackBeforeTaunt, 10.0f, MinTimeSinceAttack)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinDistanceToPlayer, 5.0f, MinDistanceToPlayer)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxDistanceToPlayer, 15.0f, MaxDistanceToPlayer)
END_STD_INTERFACE

START_STD_INTERFACE(SingleBossMovementSelector)
	COPY_INTERFACE_FROM(BossMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossMovementSelector)
END_STD_INTERFACE

START_STD_INTERFACE(VulnerableToIncomingAttackBossMovementSelector)
	COPY_INTERFACE_FROM(BossMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossMovementSelector)

	PUBLISH_VAR_AS(m_fProbabilityOfSelecting, ProbabilityOfSelecting)
END_STD_INTERFACE

START_STD_INTERFACE(RandomBossMovementSelector)
	COPY_INTERFACE_FROM(BossMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossMovementSelector)
END_STD_INTERFACE

START_STD_INTERFACE(BossMovement)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMovementProbability, 1.0f, MovementProbability)
	PUBLISH_VAR_AS(m_fSpeed, Speed)
	PUBLISH_VAR_AS(m_bCanBeInterruptedToAttack, CanBeInterruptedToAttack)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bVulnerableDuring, true, VulnerableDuring)
END_STD_INTERFACE

START_STD_INTERFACE(BossWalkingMovement)
	COPY_INTERFACE_FROM(BossMovement)
	DEFINE_INTERFACE_INHERITANCE(BossMovement)

	PUBLISH_VAR_AS(m_fTimeToDoMovementExclusively, TimeToDoMovementExclusively)
	PUBLISH_VAR_AS(m_fTimeToDoMovementExclusivelyAdjust, TimeToDoMovementExclusivelyAdjust)
	PUBLISH_VAR_AS(m_obWalkingController, WalkingController)
END_STD_INTERFACE

START_STD_INTERFACE(BossTransitioningMovement)
	COPY_INTERFACE_FROM(BossMovement)
	DEFINE_INTERFACE_INHERITANCE(BossMovement)
END_STD_INTERFACE

START_STD_INTERFACE(BossPlayerRelativeVectorMovement)
	COPY_INTERFACE_FROM(BossWalkingMovement)
	DEFINE_INTERFACE_INHERITANCE(BossWalkingMovement)
	
	PUBLISH_VAR_AS(m_obVector, Vector)
	PUBLISH_VAR_AS(m_fStopDistance, StopDistance)
END_STD_INTERFACE

START_STD_INTERFACE(BossPlayerRelativeTransitioningMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)
	
	PUBLISH_VAR_AS(m_obVector, Vector)
	PUBLISH_VAR_AS(m_obStartMove, StartMove)
	PUBLISH_VAR_AS(m_obCycleMove, CycleMove)
	PUBLISH_VAR_AS(m_obEndMove, EndMove)
	PUBLISH_VAR_AS(m_fCycleTime, CycleTime)

	PUBLISH_VAR_AS(m_bAlignToPlayerAtEnd, AlignToPlayerAtEnd)
END_STD_INTERFACE

START_STD_INTERFACE(BossSimpleNavigationAvoidanceArea)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obCentre, CPoint(0.0f, 0.0f, 0.0f), Centre)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRadius, 1.0f, Radius)
END_STD_INTERFACE

START_STD_INTERFACE(BossAttackPhaseEndTransitionNinjaSequence)
	COPY_INTERFACE_FROM(BossAttackPhaseEndTransition)
	DEFINE_INTERFACE_INHERITANCE(BossAttackPhaseEndTransition)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_obNinjaSequenceEntityName, "NULL", NinjaSequenceEntityName)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossAttackSelectorPlayerCombatStateCondition)
	IENUM_d		(GeneralBossAttackSelectorPlayerCombatStateCondition, RequiredCombatState, COMBAT_STATE, CS_STANDARD)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bNegateCheck, false, NegateCheck)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossAttackSelectorPlayerDistanceCondition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fInnerDistance, 0.0f, InnerDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fOuterDistance, 10.0f, OuterDistance)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossAttackSelectorPlayerTimeSinceLastAttackCondition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fGracePeriod, 1.0f, GracePeriod)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fGracePeriodAdjust, 0.1f, GracePeriodAdjust)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossAttackSelectorBossTimeSinceLastAttackCondition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fGracePeriod, 1.0f, GracePeriod)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fGracePeriodAdjust, 0.1f, GracePeriodAdjust)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossAttackSelectorBossDistanceFromStaticGeometryBox)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obOffset, CPoint(0.0f, 0.0f, 0.0f), BoxOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRotationY, 0.0f, RotationY)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fWidth, 1.5f, BoxTestWidth)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fLength, 1.5f, BoxTestLength)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossAttackSelectorMinTimeSinceLastSelected)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinTimeBetweenSelectionLower, 5.0f, MinTimeLowerLimit)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinTimeBetweenSelectionUpper, 15.0f, MinTimeUpperLimit)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossMovementSelectorAGenInAir)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bRequiredInAirState, true, RequiredInAirState)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossAttackSelectorAGenInAir)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bRequiredInAirState, true, RequiredInAirState)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)

	PUBLISH_PTR_CONTAINER_AS(m_obConditions, Conditions)
END_STD_INTERFACE



START_STD_INTERFACE(GeneralBossMovementSelectorPlayerCombatStateCondition)
	IENUM_d		(GeneralBossAttackSelectorPlayerCombatStateCondition, RequiredCombatState, COMBAT_STATE, CS_STANDARD)	
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bNegateCheck, false, NegateCheck)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossMovementSelectorPlayerDistanceCondition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fInnerDistance, 0.0f, InnerDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fOuterDistance, 10.0f, OuterDistance)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossMovementSelectorPlayerTimeSinceLastAttackCondition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fGracePeriod, 1.0f, GracePeriod)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fGracePeriodAdjust, 0.1f, GracePeriodAdjust)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossMovementSelectorBossTimeSinceLastAttackCondition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fGracePeriod, 1.0f, GracePeriod)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fGracePeriodAdjust, 0.1f, GracePeriodAdjust)
END_STD_INTERFACE

START_STD_INTERFACE(GeneralBossMovementSelector)
	COPY_INTERFACE_FROM(BossMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossMovementSelector)

	PUBLISH_PTR_CONTAINER_AS(m_obConditions, Conditions)
END_STD_INTERFACE

//--------------------------------------------------
//!
//! Boss State Machine - as simple as possible
//!
//--------------------------------------------------
STATEMACHINE(BOSS_FSM, Boss)
	BOSS_FSM(CKeyString& obInitState)
	{
		if ( obInitState == "Boss_DefaultState" )	
			SET_INITIAL_STATE( BOSS_DEFAULTSTATE );
		else
			ntError_p(false, ("Boss Initial State should be Boss_DefaultState"));
	}

	STATE(BOSS_DEFAULTSTATE)
		BEGIN_EVENTS
			ON_UPDATE
				ME->Update(CTimer::Get().GetGameTimeChange());
			END_EVENT(true)

			ON_ENTER
				
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				if (ME->GetBossType() == Boss::BT_AERIAL_GENERAL)
					((AerialGeneral*)ME)->SetSplitting(false);
				
				ME->NotifyIsInFailedStrikeRecovery(false);
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( BOSS_EXTERNALCONTROLSTATE );
			END_EVENT(true)

			EVENT(msg_combat_killed)
				SET_STATE( BOSS_DEADSTATE );
			END_EVENT(true)

			EVENT(msg_movementdone)
				ME->NotifyMovementDone();
			END_EVENT(true)

			EVENT(msg_startbossbattle)
					ME->SetInitialAttackPhase();
				
				if (StyleManager::Exists() && StyleManager::Get().GetHitCounter() && !StyleManager::Get().GetHitCounter()->GetBossMode())
					StyleManager::Get().GetHitCounter()->SetBossMode(ME);

				if (ME->GetBossType() == Boss::BT_AERIAL_GENERAL)
				{
					if (DoppelgangerManager::Exists())
						ME->GetInteractionComponent()->AllowCollisionWith(DoppelgangerManager::Get().GetMaster());
					((AerialGeneral*)ME)->SetSplitting(false);
				}
			END_EVENT(true)

			EVENT(msg_combat_struck)
				SET_STATE( BOSS_REACTSTATE );
			END_EVENT(true)

			EVENT(msg_buttonattack)
				// Check for combat action
				if ( ME->GetAttackComponent()->StartNewAttack() )
				{
					SET_STATE( BOSS_COMBATSTATE );
				}
			END_EVENT(true)

			EVENT(msg_bosscombat_struck_invulnerable)
				ME->ResetTimeSinceLastPlayerStrike();	//A strike's a strike, even if we're invulnerable... reset the timer.
			END_EVENT(true)

			EVENT(msg_demon_playanim)
			{
				//First retrieve our AnimName parameter.
				CHashedString obAnimation = msg.GetHashedString("AnimName");
				//Check that we've been given an animation to play.
				if(ntStr::IsNull(obAnimation))
				{
					ntError_p(false, ("msg_demon_playanim retrieved with no animation name"));
					END_EVENT(true)
				}

				//Now check that this message is actually being received on the king as it should be!
				ntError_p( ME->GetBossType() == Boss::BT_KING_BOHAN,
					("Only the king should be sending anim-control messages to the demon, attempting to send [%s] to boss-type %d",
					ntStr::GetString(obAnimation), ME->GetBossType()));
				if(ME->GetBossType() != Boss::BT_KING_BOHAN)
				{
					END_EVENT(true);
				}

				//Get a pointer to our demon object, so we can just play the animation on it from here.
				KingBohan* pobKing = static_cast<KingBohan*>(ME);
				Demon* pobDemon = static_cast<Demon*>(pobKing->GetDemon());

				//Check that it's a demon (paranoia!)
				ntError_p( pobDemon->GetBossType() == Boss::BT_DEMON, ("Why on earth is the demon object in the king not of type BT_DEMON?") );
				//Only synchronise animations if the demon isn't off doing it's own thing.
				if(pobDemon->IsAttachedToKing())
				{
					pobDemon->PlayAnimation(obAnimation);
				}
			}
			END_EVENT(true)


			//Aerial-general sword bits.
			EVENT(msg_agen_unholster_sword_left)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_unholster_sword_left") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->UnholsterSword(false, false);
			}
			END_EVENT(true)

			EVENT(msg_agen_unholster_sword_right)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_unholster_sword_right") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->UnholsterSword(true, false);
			}
			END_EVENT(true)

			EVENT(msg_agen_holster_sword_left)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_holster_sword_left") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->HolsterSword(false, false);
			}
			END_EVENT(true)

			EVENT(msg_agen_holster_sword_right)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_holster_sword_right") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->HolsterSword(true, false);
			}
			END_EVENT(true)

			EVENT(msg_agen_unholster_powersword_left)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_unholster_powersword_left") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->UnholsterSword(false, true);
			}
			END_EVENT(true)

			EVENT(msg_agen_unholster_powersword_right)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_unholster_powersword_right") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->UnholsterSword(true, true);
			}
			END_EVENT(true)

			EVENT(msg_agen_holster_powersword_left)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_holster_powersword_left") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->HolsterSword(false, true);
			}
			END_EVENT(true)

			EVENT(msg_agen_holster_powersword_right)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_holster_powersword_right") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->HolsterSword(true, true);
			}
			END_EVENT(true)


			ON_EXIT

			END_EVENT(true)
		END_EVENTS
	END_STATE // BOSS_DEFAULTSTATE

	STATE(BOSS_COMBATSTATE)
		BEGIN_EVENTS
			ON_UPDATE
				ME->Update(CTimer::Get().GetGameTimeChange());
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( BOSS_EXTERNALCONTROLSTATE );
			END_EVENT(true)

			EVENT(msg_combat_killed)
				SET_STATE( BOSS_DEADSTATE );
			END_EVENT(true)

			EVENT(msg_movementdone)
				ME->NotifyMovementDone();
			END_EVENT(true)

			EVENT(msg_combat_struck)
				ME->NotifyAttackInterrupted();
				SET_STATE( BOSS_REACTSTATE );
			END_EVENT(true)

			EVENT(msg_combat_struck_uninterruptible)
				ME->ResetTimeSinceLastPlayerStrike();	//A strike's a strike... reset the timer.
				//For the king and the gladiator general, we need to set a flag on specific attacks to show we've been hit so that
				//we can force on a new attack-link as a recoil anim (when the GGen has hit the pillar, or the king is stunned,
				//and a normal snap-to for recoil anim would look silly).
				if(ME->GetBossType() == Boss::BT_KING_BOHAN)
				{
					KingBohan* pobKing = (KingBohan*)ME;
					if(pobKing->NeedsHitWhileStunnedNotification())
					{
						pobKing->SetHitWhileStunned(true);
					}
				}
				if(ME->GetBossType() == Boss::BT_GLADIATOR_GENERAL)
				{
					GladiatorGeneral* pobGGen = (GladiatorGeneral*)ME;
					if(pobGGen->NeedsHitWhileStunnedNotification())
					{
						pobGGen->SetHitWhileStunned(true);
					}
				}
				// Sound reaction? Something? ANYTHING?!
			END_EVENT(true)

			EVENT(msg_bosscombat_struck_invulnerable)
				ME->ResetTimeSinceLastPlayerStrike();	//A strike's a strike, even if we're invulnerable... reset the timer.
			END_EVENT(true)

			EVENT(msg_buttonattack)
				// Check for next combat action
				ME->GetAttackComponent()->SelectNextAttack();
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				if (ME->GetBossType() == Boss::BT_AERIAL_GENERAL)
					((AerialGeneral*)ME)->SetSplitting(false);
				ME->NotifyIsInFailedStrikeRecovery(false);
				ME->GetAttackComponent()->CompleteRecovery();
				ME->NotifyAttackFinished();
				SET_STATE( BOSS_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_combat_autolinked)				
				ME->NotifyAttackAutoLinked();
			END_EVENT(true)

			EVENT(msg_combat_startstrikefailedrecovery)

				ME->NotifyIsInFailedStrikeRecovery(true);
			END_EVENT(true)

			ON_EXIT		
			END_EVENT(true)

			EVENT( msg_ggen_stop_pillar_notification )
			{
				ntError_p(ME->GetBossType() == Boss::BT_GLADIATOR_GENERAL, ("msg_ggen_stop_pillar_notification should only ever be received on GGen"));

				GladiatorGeneral* pGGen = static_cast<GladiatorGeneral*>(ME);
				pGGen->StopNotificationWithPillars();
			}
			END_EVENT(true)

			EVENT( msg_ggen_stomplanded )
			{
				ntError_p(ME->GetBossType() == Boss::BT_GLADIATOR_GENERAL, ("msg_ggen_stomplanded should only ever be received on GGen"));
				GladiatorGeneral* pGGen = static_cast<GladiatorGeneral*>(ME);
				pGGen->NotifyJustFinishedStomp();
			}
			END_EVENT(true)

			EVENT(msg_demon_playanim)
			{
				//First retrieve our AnimName parameter.
				CHashedString obAnimation = msg.GetHashedString("AnimName");
				//Check that we've been given an animation to play.
				if(ntStr::IsNull(obAnimation))
				{
					ntError_p(false, ("msg_demon_playanim retrieved with no animation name"));
					END_EVENT(true)
				}

				//Now check that this message is actually being received on the king as it should be!
				ntError_p( ME->GetBossType() == Boss::BT_KING_BOHAN,
					("Only the king should be sending anim-control messages to the demon, attempting to send [%s] to boss-type %d",
					ntStr::GetString(obAnimation), ME->GetBossType()));
				if(ME->GetBossType() != Boss::BT_KING_BOHAN)
				{
					END_EVENT(true);
				}

				//Get a pointer to our demon object, so we can just play the animation on it from here.
				KingBohan* pobKing = static_cast<KingBohan*>(ME);
				Demon* pobDemon = static_cast<Demon*>(pobKing->GetDemon());

				//Check that it's a demon (paranoia!)
				ntError_p( pobDemon->GetBossType() == Boss::BT_DEMON, ("Why on earth is the demon object in the king not of type BT_DEMON?") );
				//Only synchronise animations if the demon isn't off doing it's own thing.
				if(pobDemon->IsAttachedToKing())
				{
					pobDemon->PlayAnimation(obAnimation);
				}
			}
			END_EVENT(true)

			//Aerial-general sword bits.
			EVENT(msg_agen_unholster_sword_left)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_unholster_sword_left") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->UnholsterSword(false, false);
			}
			END_EVENT(true)

			EVENT(msg_agen_unholster_sword_right)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_unholster_sword_right") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->UnholsterSword(true, false);
			}
			END_EVENT(true)

			EVENT(msg_agen_holster_sword_left)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_holster_sword_left") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->HolsterSword(false, false);
			}
			END_EVENT(true)

			EVENT(msg_agen_holster_sword_right)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_holster_sword_right") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->HolsterSword(true, false);
			}
			END_EVENT(true)

			EVENT(msg_agen_unholster_powersword_left)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_unholster_powersword_left") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->UnholsterSword(false, true);
			}
			END_EVENT(true)

			EVENT(msg_agen_unholster_powersword_right)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_unholster_powersword_right") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->UnholsterSword(true, true);
			}
			END_EVENT(true)

			EVENT(msg_agen_holster_powersword_left)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_holster_powersword_left") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->HolsterSword(false, true);
			}
			END_EVENT(true)

			EVENT(msg_agen_holster_powersword_right)
			{
				ntError_p( ME->GetBossType() == Boss::BT_AERIAL_GENERAL, ("Only the aerial general should recieve msg_agen_holster_powersword_right") );
				AerialGeneral* pAGen = static_cast<AerialGeneral*>(ME);
				pAGen->HolsterSword(true, true);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE // BOSS_COMBATSTATE

	STATE(BOSS_REACTSTATE)
		BEGIN_EVENTS
			ON_UPDATE
				ME->Update(CTimer::Get().GetGameTimeChange());
#ifdef DEBUG_HORRIBLE_REACTSTATE_TIMEOUT
				if((ME->GetTimeInReactState() > 20.0f) && (ME->GetTimeSinceLastPlayerStrike() > 20.0f))
				{
					//Do what we do with msg_combatrecovered... but output a message first.
					ntPrintf("*** BOSS_REACTSTATE HANG, SAVED BY SUPERHAX! ***\n");
					ME->NotifyIsInFailedStrikeRecovery(false);
					ME->NotifyAttackFinished();
					ME->GetAttackComponent()->CompleteRecovery();
					ME->NotifyRecovered();
					SET_STATE( BOSS_DEFAULTSTATE );
				}
				ME->IncrementTimeInReactState(CTimer::Get().GetGameTimeChange());
#endif
			END_EVENT(true)

			ON_ENTER
				ME->ResetTimeSinceLastPlayerStrike();	//Reset last-strike timer so we don't want to taunt having just been hit.
				ME->GetAttackComponent()->Lua_SendRecoilMessage();
#ifdef DEBUG_HORRIBLE_REACTSTATE_TIMEOUT
				ME->ResetTimeInReactState();
#endif
				// HACK another last min change to the agen, this will send a trigger as soon as he gets struck and he's run out of doppelgangers
				if ( ME->GetBossType() == Boss::BT_AERIAL_GENERAL )
				{
					AerialGeneral* pobAGen = (AerialGeneral*)ME;
					if (DoppelgangerManager::Get().GetMaster() == ME)
						pobAGen->GetNextDoppelgangerToWakeUp(); // Assuming we're in the final stage of the battle, this'll send another message like the end of the doppelgangers
				}
			
			END_EVENT(true)
			END_EVENT(true)

			EVENT(msg_combat_superstyledone_receiver)
				ME->NotifySuperStyleDone();
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( BOSS_EXTERNALCONTROLSTATE );
			END_EVENT(true)

			EVENT(msg_combat_killed)
				SET_STATE( BOSS_DEADSTATE );
			END_EVENT(true)

			EVENT(msg_movementdone)
				ME->NotifyMovementDone();
			END_EVENT(true)

			EVENT(msg_combat_breakout)
				SET_STATE( BOSS_DEFAULTSTATE );
			END_EVENT(true)

			//Note: NotifyAttackFinished() here is "semi-dodgy". Currently it's fine as the propogation basically ends up just
			//calling NotifyAttackFinished on the attack itself (which is what we want here, i.e. if m_pobCurrentAttack, notify it).
			//We should be mindful of it's use here though just in-case anything is added in the tree of NotifyAttackFinished()
			//calls later on that isn't suited to react-state.
			EVENT(msg_combat_recovered)
				if (ME->GetBossType() == Boss::BT_AERIAL_GENERAL)
					((AerialGeneral*)ME)->SetSplitting(false);

				ME->NotifyIsInFailedStrikeRecovery(false);
				ME->NotifyAttackFinished();
				ME->GetAttackComponent()->CompleteRecovery();
				ME->NotifyRecovered();
				SET_STATE( BOSS_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_combatsyncdreactionend)
				ME->GetAttackComponent()->EndSyncdReaction();
			END_EVENT(true)

			EVENT(msg_combat_countered)
				SET_STATE( BOSS_COMBATSTATE );
			END_EVENT(true)

			EVENT(msg_combat_floored)
				ME->GetAttackComponent()->StartFlooredState();
			END_EVENT(true)

			EVENT( msg_ggen_shellbreak_01 )
			{
				ntError( ME->GetBossType() == Boss::BT_GLADIATOR_GENERAL );

				GladiatorGeneral* pGGen = static_cast<GladiatorGeneral*>(ME);
				pGGen->BreakShell_1();
			}
			END_EVENT(true)

			EVENT( msg_ggen_shellbreak_02 )
			{
				ntError( ME->GetBossType() == Boss::BT_GLADIATOR_GENERAL );

				GladiatorGeneral* pGGen = static_cast<GladiatorGeneral*>(ME);
				pGGen->BreakShell_2();
			}
			END_EVENT(true)

			EVENT( msg_ggen_shellbreak_03 )
			{
				ntError( ME->GetBossType() == Boss::BT_GLADIATOR_GENERAL );

				GladiatorGeneral* pGGen = static_cast<GladiatorGeneral*>(ME);
				pGGen->BreakShell_3();
			}
			END_EVENT(true)

			EVENT(msg_demon_playanim)
			{
				//First retrieve our AnimName parameter.
				CHashedString obAnimation = msg.GetHashedString("AnimName");
				//Check that we've been given an animation to play.
				if(ntStr::IsNull(obAnimation))
				{
					ntError_p(false, ("msg_demon_playanim retrieved with no animation name"));
					END_EVENT(true)
				}

				//Now check that this message is actually being received on the king as it should be!
				ntError_p( ME->GetBossType() == Boss::BT_KING_BOHAN,
					("Only the king should be sending anim-control messages to the demon, attempting to send [%s] to boss-type %d",
					ntStr::GetString(obAnimation), ME->GetBossType()));
				if(ME->GetBossType() != Boss::BT_KING_BOHAN)
				{
					END_EVENT(true);
				}

				//Get a pointer to our demon object, so we can just play the animation on it from here.
				KingBohan* pobKing = static_cast<KingBohan*>(ME);
				Demon* pobDemon = static_cast<Demon*>(pobKing->GetDemon());

				//Check that it's a demon (paranoia!)
				ntError_p( pobDemon->GetBossType() == Boss::BT_DEMON, ("Why on earth is the demon object in the king not of type BT_DEMON?") );
				//Only synchronise animations if the demon isn't off doing it's own thing.
				if(pobDemon->IsAttachedToKing())
				{
					pobDemon->PlayAnimation(obAnimation);
				}
			}
			END_EVENT(true)

			ON_EXIT
				ME->ResetTimeSinceLastPlayerStrike();	//Reset it again as we leave react-state, start the counter from that point.
			END_EVENT(true)

		END_EVENTS
	END_STATE // BOSS_REACTSTATE

	STATE(BOSS_DEADSTATE)
		BEGIN_EVENTS
			ON_ENTER			
				// Destroy any projectiles that may be parented to this object
				ME->GetPhysicsSystem()->Lua_RemoveChildEntities();

				// Make the character into a corpse
				ME->GetMovement()->ClearControllers();
				ME->GetMovement()->SetEnabled( false );
				ME->GetAnimator()->Disable();
				ME->GetPhysicsSystem()->Deactivate();
				ME->GetAttackComponent()->MakeDead();
				ME->Lua_MakeDead();
			END_EVENT(true)

			EVENT(msg_demon_playanim)
			{
				//First retrieve our AnimName parameter.
				CHashedString obAnimation = msg.GetHashedString("AnimName");
				//Check that we've been given an animation to play.
				if(ntStr::IsNull(obAnimation))
				{
					ntError_p(false, ("msg_demon_playanim retrieved with no animation name"));
					END_EVENT(true)
				}

				//Now check that this message is actually being received on the king as it should be!
				ntError_p( ME->GetBossType() == Boss::BT_KING_BOHAN,
					("Only the king should be sending anim-control messages to the demon, attempting to send [%s] to boss-type %d",
					ntStr::GetString(obAnimation), ME->GetBossType()));
				if(ME->GetBossType() != Boss::BT_KING_BOHAN)
				{
					END_EVENT(true);
				}

				//Get a pointer to our demon object, so we can just play the animation on it from here.
				KingBohan* pobKing = static_cast<KingBohan*>(ME);
				Demon* pobDemon = static_cast<Demon*>(pobKing->GetDemon());

				//Check that it's a demon (paranoia!)
				ntError_p( pobDemon->GetBossType() == Boss::BT_DEMON, ("Why on earth is the demon object in the king not of type BT_DEMON?") );
				//Only synchronise animations if the demon isn't off doing it's own thing.
				if(pobDemon->IsAttachedToKing())
				{
					pobDemon->PlayAnimation(obAnimation);
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE // BOSS_DEADSTATE

	STATE(BOSS_EXTERNALCONTROLSTATE)
		BEGIN_EVENTS
			ON_ENTER
				if(ME->GetBossType() == Boss::BT_DEMON)
				{
					//If we're the demon, then we need to detach from the king at this point.
					//Absolute movement only sets the local-transform matrix on the entity, so we need it to be parented only
					//to the world. This means that on msg_external_control_end we'll need to handle all reparenting too.
					Demon* pobDemon = (Demon*)ME;
					pobDemon->DetachFromKingForNS();
				}
				ME->GetAttackComponent()->SetDisabled(true);
				ME->GetMovement()->SetInputDisabled(true);

				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(true);
			END_EVENT(true)

			EVENT(msg_external_control_end)
				ME->GetAttackComponent()->SetDisabled(false);
				ME->GetMovement()->SetInputDisabled(false);

				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(false);
				if(ME->GetBossType() == Boss::BT_DEMON)
				{
					//If we're the demon, then we now need to re-attach to the king. The demon was detached for the externally-controlled
					//movement because absolute-movement only sets local transform matrix, so the demon needed to be parented only to the
					//world.
					Demon* pobDemon = (Demon*)ME;
					pobDemon->AttachToKingAfterNS();
				}
				SET_STATE( BOSS_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_combat_killed)
				SET_STATE( BOSS_DEADSTATE );
			END_EVENT(true)

			EVENT(msg_demon_playanim)
			{
				//First retrieve our AnimName parameter.
				CHashedString obAnimation = msg.GetHashedString("AnimName");
				//Check that we've been given an animation to play.
				if(ntStr::IsNull(obAnimation))
				{
					ntError_p(false, ("msg_demon_playanim retrieved with no animation name"));
					END_EVENT(true)
				}

				//Now check that this message is actually being received on the king as it should be!
				ntError_p( ME->GetBossType() == Boss::BT_KING_BOHAN,
					("Only the king should be sending anim-control messages to the demon, attempting to send [%s] to boss-type %d",
					ntStr::GetString(obAnimation), ME->GetBossType()));
				if(ME->GetBossType() != Boss::BT_KING_BOHAN)
				{
					END_EVENT(true);
				}

				//Get a pointer to our demon object, so we can just play the animation on it from here.
				KingBohan* pobKing = static_cast<KingBohan*>(ME);
				Demon* pobDemon = static_cast<Demon*>(pobKing->GetDemon());

				//Check that it's a demon (paranoia!)
				ntError_p( pobDemon->GetBossType() == Boss::BT_DEMON, ("Why on earth is the demon object in the king not of type BT_DEMON?") );
				//Only synchronise animations if the demon isn't off doing it's own thing.
				if(pobDemon->IsAttachedToKing())
				{
					pobDemon->PlayAnimation(obAnimation);
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE // BOSS_EXTERNALCONTROLSTATE

	//All-states
	BEGIN_EVENTS
		EVENT(msg_boss_gotophase)
		{
			if (msg.IsHash("Name"))
			{
				ME->GoToPhase(msg.GetHashedString("Name"));
			}
			else if (msg.IsInt("PhaseNumber"))
			{
				ntPrintf("msg_boss_gotophase message received\n");
				int iPhaseNumber = msg.GetInt("PhaseNumber");
				ME->GoToPhase(iPhaseNumber);
			}
		}
		END_EVENT(true)

		EVENT(msg_headshot)
		{
			if ( ME->GetBossType() == Boss::BT_AERIAL_GENERAL )
			{
				AerialGeneral* pobAGen = (AerialGeneral*)ME;
				pobAGen->WakeUpDoppelganger(true, true); // Assuming we're in the final stage of the battle, this'll send another message like the end of the doppelgangers
			}
		}
		END_EVENT(true)
	END_EVENTS

END_STATEMACHINE

//--------------------------------------------------
//!
//! Boss Implementation
//!
//--------------------------------------------------
Boss::Boss()
: Character()
{
	// I am boss.
	m_eType = EntType_Boss;
	m_eCharacterType = Character::CT_Boss;

	m_obMovementInput.m_obMoveDirection = m_obMovementInput.m_obFacingDirection = CDirection(0.0f,0.0f,0.0f);
	m_obMovementInput.m_fMoveSpeed = 0.0f;
	m_obMovementInput.m_bTargetPointSet = false;

	m_bIsUnderAttack = false;

	m_eBossType = BT_COUNT;

	m_pobCurrentAttackPhase = 0;

	m_pobCombatEventLog = NT_NEW CombatEventLog( CE_GOT_DEFLECT | CE_GOT_RECOILED | CE_GOT_IMPACT_STAGGERED | CE_GOT_BLOCK_STAGGERED | CE_STARTED_ATTACK | CE_GOT_KOED );

	m_bVulnerableToIncomingStrike = false;

	m_bForceRemoveAttackPhase = false;
	m_bAttackPhaseSetManually = false;
	m_pobForcedCurrentAttackPhase = 0;
	m_fTimeSinceLastPlayerStrike = 0.0f;
	m_fTimeSinceBossLastAttacked = 0.0f;
#ifdef DEBUG_HORRIBLE_REACTSTATE_TIMEOUT
	m_fTimeInReactState = 0.0f;
#endif
}

void Boss::GoToPhase(CHashedString obName)
{
	m_obInitialAttackPhase = obName;
	SetInitialAttackPhase();
}

Boss::~Boss()
{
	NT_DELETE( m_pobCombatEventLog );
	if (m_pobNavMan)
		NT_DELETE_CHUNK(Mem::MC_ENTITY,m_pobNavMan);
}

bool Boss::ShouldBlock(CStrike* pobStrike) 
{ 
	bool bRet = true;
	if (m_pobCurrentAttackPhase)
		bRet = m_pobCurrentAttackPhase->ShouldBlock(pobStrike);			
	return bRet;
}

bool Boss::IsVulnerableTo(CStrike* pobStrike) 
{ 
	// Mmm hack - DGF
	if ( GetBossType() == Boss::BT_AERIAL_GENERAL )
	{
		if (((AerialGeneral*)this)->GetSplitting())
			return false;
	}

	bool bRet = true;
	if (m_pobCurrentAttackPhase)
		bRet = m_pobCurrentAttackPhase->IsVulnerableTo(pobStrike);			
	return bRet;
}

bool Boss::IsVulnerableTo(const CAttackData* pobAttackData) 
{ 
	// Mmm hack - DGF
	if ( GetBossType() == Boss::BT_AERIAL_GENERAL )
	{
		if (((AerialGeneral*)this)->GetSplitting())
			return false;
	}

	bool bRet = true;
	if (m_pobCurrentAttackPhase)
		bRet = m_pobCurrentAttackPhase->IsVulnerableTo(pobAttackData);			
	return bRet;
}

// Psuedo-virtualised functions for sub-classes
bool Boss::CanStartAnAttack() 
{ 
	switch ( m_eBossType )
	{
	case BT_GLADIATOR_GENERAL:
		{
			GladiatorGeneral* pobGGen = (GladiatorGeneral*)this;
			return pobGGen->CanStartAnAttack();
			break;
		}
	case BT_WATER_GENERAL:
		{
			WaterGeneral* pobWGen = (WaterGeneral*)this;
			return pobWGen->CanStartAnAttack();
			break;
		}
	case BT_KING_BOHAN:
		{
			KingBohan* pobKing = (KingBohan*)this;
			return pobKing->CanStartAnAttack();
			break;
		}
	case BT_DEMON:
		{
			Demon* pobDemon = (Demon*)this;
			return pobDemon->CanStartAnAttack();
			break;
		}
	case BT_AERIAL_GENERAL:
		{
			AerialGeneral* pobAGen = (AerialGeneral*)this;
			return pobAGen->CanStartAnAttack();
			break;
		}
	case BT_COUNT:
		{
			ntError_p(false, ("CanStartAnAttack() - No Boss-type specified"));
			break;
		}
	}

	return false;
}

bool Boss::CanStartALinkedAttack() 
{ 
	switch ( m_eBossType )
	{
	case BT_GLADIATOR_GENERAL:
		{
			GladiatorGeneral* pobGGen = (GladiatorGeneral*)this;
			return pobGGen->CanStartALinkedAttack();
			break;
		}
	case BT_WATER_GENERAL:
		{
			WaterGeneral* pobWGen = (WaterGeneral*)this;
			return pobWGen->CanStartALinkedAttack();
			break;
		}
	case BT_KING_BOHAN:
		{
			KingBohan* pobKing = (KingBohan*)this;
			return pobKing->CanStartALinkedAttack();
			break;
		}
	case BT_DEMON:
		{
			Demon* pobDemon = (Demon*)this;
			return pobDemon->CanStartALinkedAttack();
			break;
		}
	case BT_AERIAL_GENERAL:
		{
			AerialGeneral* pobAGen = (AerialGeneral*)this;
			return pobAGen->CanStartALinkedAttack();
			break;
		}
	case BT_COUNT:
		{
			ntError_p(false, ("CanStartALinkedAttack() - No Boss-type specified"));
			break;
		}
	}

	return false;
}

void Boss::UpdateBossSpecifics( float fTimeDelta ) 
{ 
	switch ( m_eBossType )
	{
	case BT_GLADIATOR_GENERAL:
		{
			GladiatorGeneral* pobGGen = (GladiatorGeneral*)this;
			pobGGen->UpdateBossSpecifics(fTimeDelta);
			break;
		}
	case BT_WATER_GENERAL:
		{
			WaterGeneral* pobWGen = (WaterGeneral*)this;
			pobWGen->UpdateBossSpecifics(fTimeDelta);
			break;
		}
	case BT_KING_BOHAN:
		{
			KingBohan* pobKing = (KingBohan*)this;
			pobKing->UpdateBossSpecifics(fTimeDelta);
			break;
		}
	case BT_DEMON:
		{
			Demon* pobDemon = (Demon*)this;
			pobDemon->UpdateBossSpecifics(fTimeDelta);
			break;
		}
	case BT_AERIAL_GENERAL:
		{
			AerialGeneral* pobAGen = (AerialGeneral*)this;
			return pobAGen->UpdateBossSpecifics(fTimeDelta);
			break;
		}
	case BT_COUNT:
		{
			ntError_p(false, ("UpdateBossSpecifics() - No Boss-type specified"));
			break;
		}
	}

	//If we're nearly dead and the player is close, then circle will trigger the superstyle to start the boss-fight end ninja sequence.
	float fCurrentHealth = ToCharacter()->GetCurrHealth();
	if(fCurrentHealth < 25)	//25 is an okay default value? Or we set it higher for some? Needs to be over 20 to account for 20-damage hits.
	{
		CEntity* pPlayer = CEntityManager::Get().GetPlayer();
		ntError_p(pPlayer != NULL, ("Could not get player pointer"));
		if(pPlayer)
		{
			CDirection obBetween = CDirection(GetPosition() - pPlayer->GetPosition());
			float fBetweenLenSquared = obBetween.LengthSquared();
			if(fBetweenLenSquared <= 4.0f)	//2metre radius.
			{
				CAttackComponent* pobAttackComponent = pPlayer->GetAttackComponent();
				ntError_p(pobAttackComponent, ("Why doesn't the player have an attack component?"));

				if(pobAttackComponent)
				{
					//Don't show this DURING the superstyle, so check to see if the player is currently in one.
					bool bDisplayHint = true;

					const CStrike* pStrike = pobAttackComponent->GetCurrentStrike();
					if(pStrike)
					{
						//Basically any strike that needs to synchronise probably doesn't need our button-press there, including
						//superstyles.
						if(pStrike->ShouldSync())
						{
							bDisplayHint = false;
						}
					}
					if(bDisplayHint)
					{
						pobAttackComponent->SetNeedsHintForButton(AB_GRAB, true);
					}
				}
			}
		}
	}
}

void Boss::DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset )
{ 
#ifndef _GOLD_MASTER

	switch ( m_eBossType )
	{
	case BT_GLADIATOR_GENERAL:
		{
			g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_YELLOW,0,
				"Gladiator General: %s, TimeSinceAttacked[%f]",GetName().c_str(), m_fTimeSinceLastPlayerStrike);
			GladiatorGeneral* pobGGen = (GladiatorGeneral*)this;
			fXOffset += DEBUG_SHIFT_AMOUNT;
			fYOffset += DEBUG_SHIFT_AMOUNT;
			pobGGen->DebugRenderBossSpecifics(obScreenLocation,fXOffset,fYOffset);
			break;
		}
	case BT_WATER_GENERAL:
		{
			g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_YELLOW,0,
				"Water General: %s, TimeSinceAttacked[%f]",GetName().c_str(), m_fTimeSinceLastPlayerStrike);
			WaterGeneral* pobWGen = (WaterGeneral*)this;
			fXOffset += DEBUG_SHIFT_AMOUNT;
			fYOffset += DEBUG_SHIFT_AMOUNT;
			pobWGen->DebugRenderBossSpecifics(obScreenLocation,fXOffset,fYOffset);
			break;
		}
	case BT_KING_BOHAN:
		{
			g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_YELLOW,0,
				"King Bohan: %s, TimeSinceAttacked[%f]", GetName().c_str(), m_fTimeSinceLastPlayerStrike);
			KingBohan* pobKing = (KingBohan*)this;
			fXOffset += DEBUG_SHIFT_AMOUNT;
			fYOffset += DEBUG_SHIFT_AMOUNT;
			pobKing->DebugRenderBossSpecifics(obScreenLocation,fXOffset,fYOffset);
			break;
		}
	case BT_DEMON:
		{
			g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_YELLOW,0,
				"Demon: %s, TimeSinceAttacked[%f]", GetName().c_str(), m_fTimeSinceLastPlayerStrike);
			Demon* pobDemon = (Demon*)this;
			fXOffset += DEBUG_SHIFT_AMOUNT;
			fYOffset += DEBUG_SHIFT_AMOUNT;
			pobDemon->DebugRenderBossSpecifics(obScreenLocation,fXOffset,fYOffset);
			break;
		}
	case BT_AERIAL_GENERAL:
		{
			g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_YELLOW,0,
				"Aerial General: %s, TimeSinceAttacked[%f]",GetName().c_str(), m_fTimeSinceLastPlayerStrike);
			AerialGeneral* pobAGen = (AerialGeneral*)this;
			fXOffset += DEBUG_SHIFT_AMOUNT;
			fYOffset += DEBUG_SHIFT_AMOUNT;
			return pobAGen->DebugRenderBossSpecifics(obScreenLocation,fXOffset,fYOffset);
			break;
		}
	case BT_COUNT:
		{
			ntError_p(false, ("DebugRenderBossSpecifics() - No Boss-type specified"));
			break;
		}
	}

	if(m_pobNavMan)
	{
		m_pobNavMan->DebugRender(obScreenLocation,fXOffset,fYOffset);
	}
#endif
}

void Boss::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{ 
#ifndef _GOLD_MASTER
	DebugRenderBossSpecifics(obScreenLocation,fXOffset,fYOffset);

	fXOffset += DEBUG_SHIFT_AMOUNT;
	fYOffset += DEBUG_SHIFT_AMOUNT;

	if (m_pobCurrentAttackPhase)
	{		
		m_pobCurrentAttackPhase->DebugRender(obScreenLocation, fXOffset + DEBUG_SHIFT_AMOUNT, fYOffset + DEBUG_SHIFT_AMOUNT);
	}
#endif
}

void Boss::Update(float fTimeDelta)
{
	// Update our player/boss attack timers
	m_fTimeSinceLastPlayerStrike += fTimeDelta;
	m_fTimeSinceBossLastAttacked += fTimeDelta;

	for (int i = 0; i < m_pobCombatEventLog->GetEventCount(); i++)
	{
		if (m_pobCombatEventLog->GetEvents()[i].m_eEventType == CE_GOT_RECOILED || 
			m_pobCombatEventLog->GetEvents()[i].m_eEventType == CE_GOT_IMPACT_STAGGERED ||
			m_pobCombatEventLog->GetEvents()[i].m_eEventType == CE_GOT_BLOCK_STAGGERED ||
			m_pobCombatEventLog->GetEvents()[i].m_eEventType == CE_GOT_KOED )
		{
			NotifyGotStruck();
		}
		else if (m_pobCombatEventLog->GetEvents()[i].m_eEventType == CE_STARTED_ATTACK)
		{
			NotifyAttackStarted();
		}
		else if (m_pobCombatEventLog->GetEvents()[i].m_eEventType == CE_GOT_DEFLECT)
		{
			NotifyDeflected();
		}
	}

	m_pobCombatEventLog->ClearEvents();

	// Update what attacks we should be doing and suchlike
	if (m_pobCurrentAttackPhase || m_pobForcedCurrentAttackPhase)
	{
		CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
		ntError( pobPlayer );

		// Update, and see if we're done
		if(m_pobCurrentAttackPhase)
		{
			m_pobCurrentAttackPhase = m_pobCurrentAttackPhase->Update( fTimeDelta, this, pobPlayer );
		}

		//If during that update we forced a new attack phase (SetNamedAttackPhase()) then switch to it here.
		if(m_bAttackPhaseSetManually && m_pobForcedCurrentAttackPhase)
		{
			m_pobCurrentAttackPhase = m_pobForcedCurrentAttackPhase;
			m_pobForcedCurrentAttackPhase = 0;
			m_bAttackPhaseSetManually = false;
		}
		//If we've forced the removal of the attack phase in this instance (SetNoAttackPhase()), then make sure it's now null.
		if(m_bForceRemoveAttackPhase)
		{
			m_pobCurrentAttackPhase = 0;
			m_bForceRemoveAttackPhase = false;
		}
	}

	UpdateBossSpecifics(fTimeDelta);

	// Clear this flag every frame so there is only 1 opportunity to react
	m_bVulnerableToIncomingStrike = false;
}

void Boss::OnPostConstruct()
{
	Character::OnPostConstruct();

	m_obAttributeTable->SetBool("IsBoss", true);

	// Create and attach the statemachine
	CKeyString obInitialState("Boss_DefaultState");
	BOSS_FSM* pobFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) BOSS_FSM( obInitialState );
	ATTACH_FSM(pobFSM);

	GetAttackComponent()->RegisterCombatEventLog(m_pobCombatEventLog);

	m_pobNavMan = 0;
	if (m_obAvoidanceAreas.size() > 0)
		m_pobNavMan = NT_NEW_CHUNK(Mem::MC_ENTITY) BossSimpleNavigationManager(m_obAvoidanceAreas); 
}

void Boss::SetMovementController()
{
	if (!m_pobCurrentAttackPhase && !m_obDefaultWalkingController.IsNull() )
		GetMovement()->Lua_StartMovementFromXMLDef( m_obDefaultWalkingController );
}

void Boss::SetInitialAttackPhase()
{
	GetAttackComponent()->AI_Setup_SetToDirectRequestMode();

	DataObject* pobDOb = ObjectDatabase::Get().GetDataObjectFromName(m_obInitialAttackPhase);
	
	if (pobDOb)
	{
		ntError( strcmp( pobDOb->GetClassName(), "BossAttackPhase" ) == 0 || strcmp( pobDOb->GetClassName(), "AerialGeneralDoppelgangerSpawningBossAttackPhase" ) == 0 );
	
		m_pobCurrentAttackPhase = (BossAttackPhase*)pobDOb->GetBasePtr();
	}
	else
	{
		m_pobCurrentAttackPhase = 0;
	}
}


void Boss::SetNamedAttackPhase(CHashedString obAttackPhaseName)
{
	if(ntStr::IsNull(obAttackPhaseName))
	{
		ntError_p(false, ("SetNamedAttackPhase requested with no phase name"));
	}

	GetAttackComponent()->AI_Setup_SetToDirectRequestMode();

	DataObject* pobDOb = ObjectDatabase::Get().GetDataObjectFromName(obAttackPhaseName);

	if( pobDOb )
	{
		ntError_p( strcmp( pobDOb->GetClassName(), "BossAttackPhase" ) == 0, ("SetNamedAttackPhase: Named object is not a boss attack phase!") );
		//Get a pointer to the attack phase we want to switch to, and store it (so we can switch to it after phase update).
		m_pobForcedCurrentAttackPhase = (BossAttackPhase*)pobDOb->GetBasePtr();
		ntError_p(m_pobForcedCurrentAttackPhase, ("SetNamedAttackPhase: Failed to GetBasePtr() into Current Attack Phase") );
		//Now clear out the current attack phase data so that it starts fresh.
		m_pobForcedCurrentAttackPhase->m_pobCurrentState = 0;
		m_pobForcedCurrentAttackPhase->m_bFirstUpdate = true;
		m_bAttackPhaseSetManually = true;
	}
	else
	{
		ntError_p( false, ("Failed to find requested boss attack phase, does it exist? Name typo?") );
		m_pobCurrentAttackPhase = 0;
	}
}


void Boss::SetNoAttackPhase()
{
	GetAttackComponent()->AI_Setup_SetToDirectRequestMode();

	m_bForceRemoveAttackPhase = true;
}

void Boss::NotifyPlayerInteracting(bool bState)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyPlayerInteracting(bState);
}

void Boss::NotifyPlayerInteractionAction()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyPlayerInteractionAction();
}

void Boss::NotifyProjectileCountered(Object_Projectile* pobProj)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyProjectileCountered(pobProj);
}

void Boss::NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyCreatedStrikeVolume(pobVol);
}

void Boss::NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyRemovedStrikeVolume(pobVol);
}

void Boss::NotifyInvulnerableToIncomingStrike(CStrike* pobStrike)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyInvulnerableToIncomingStrike(pobStrike);
}

void Boss::NotifyWillBlockIncomingStrike(CStrike* pobStrike)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyWillBlockIncomingStrike(pobStrike);
}

void Boss::NotifyVulnerableToIncomingStrike(CStrike* pobStrike)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyVulnerableToIncomingStrike(pobStrike);

	m_bVulnerableToIncomingStrike = true;
}

void Boss::NotifySuperStyleDone()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifySuperStyleDone(this);
}

void Boss::NotifyAttackStarted()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyAttackStarted(this);
}

void Boss::NotifyAttackFinished()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyAttackFinished(this);
}

void Boss::NotifyMovementDone()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyMovementDone(this);
}

void Boss::NotifyAttackAutoLinked()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyAttackAutoLinked(this);
}

void Boss::NotifyAttackInterrupted()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyAttackInterrupted(this);
}

void Boss::NotifyInteractionWith(CEntity* pobEntity)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyInteractionWith(pobEntity);
}

void Boss::NotifyUnderAttack(bool bUnderAttack)
{
	m_bIsUnderAttack = bUnderAttack;
}

void Boss::NotifyIsInFailedStrikeRecovery(bool bFailedStrike)
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyIsInFailedStrikeRecovery(bFailedStrike);
}

void Boss::NotifyGotStruck()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyGotStruck(this);
}

void Boss::NotifyDeflected()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyDeflected(this);
}

void Boss::NotifyRecovered()
{
	if (m_pobCurrentAttackPhase)
		m_pobCurrentAttackPhase->NotifyRecovered(this);
}

bool Boss::IsUnderAttack()
{
	return m_bIsUnderAttack;
}

BossAttackPhase* Boss::GetCurrentAttackPhase()
{
	return m_pobCurrentAttackPhase;
}

CMovementInput* Boss::GetBossMovement()
{
	return &m_obMovementInput;
}


BossSpecialAttack::BossSpecialAttack()
{
	m_fAttackProbability = 0.5f;
}

//--------------------------------------------------
//!
//! BossAttackPhase 
//! Class to handle a phase of of the boss battle
//!
//--------------------------------------------------
BossAttackPhase::BossAttackPhase()
{
	// Start with a few seconds of idling
	m_eCurrentMode = BAPM_IN_ATTACK_STATE;

	m_pobCurrentState = 0;

	m_pobStartTransition = 0;
	m_pobEndTransition = 0;

	m_bFirstUpdate = true;
}

BossAttackPhase::~BossAttackPhase()
{
}

BossAttackPhaseEndTransition* BossAttackPhase::GetEndTransition()
{
	return m_pobEndTransition;
}

bool BossAttackPhase::CanSwitchState(Boss* pobBoss)
{
	return 
		!m_pobCurrentState->IsAttacking() && 
		pobBoss->GetAttackComponent()->AI_Access_GetState() == CS_STANDARD;
}

BossAttackPhase* BossAttackPhase::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	if (m_bFirstUpdate)
	{
		if (m_pobStartTransition)
		{
			// Begin start transition
			m_eCurrentMode = BAPM_START_PHASE_TRANSITION;
			m_pobStartTransition->BeginStartTransition(pobBoss,pobPlayer);
		}
		else
		{
			// Force a state change
			m_pobCurrentState = 0;
			m_eCurrentMode = BAPM_IN_ATTACK_STATE;
		}

		// Set state, this will put the highest probablity one on for our first update
		SwitchStateTo(GetNextState(),pobBoss,pobPlayer);

		m_bFirstUpdate = false;
	}

	switch ( m_eCurrentMode )
	{
	case BAPM_COUNT:
		{
			ntError_p(false, ("Invalid BossAttackPhaseMode"));
			break;
		}	
	case BAPM_IN_ATTACK_STATE:
		{
			if ( m_pobCurrentState )
			{
				BossAttackPhaseState* pobState = m_pobCurrentState->Update(fTimeDelta,pobBoss,pobPlayer);
				
				// Check if we need to switch to another state
				if (CanSwitchState(pobBoss))
				{
					// If previous state completely finished
					if (!pobState)
					{
						pobBoss->GetBossMovement()->ClearMovementInput();

						// Then we need a new state
						SwitchStateTo(GetNextState(),pobBoss,pobPlayer);
					}
					// If we got a different state back
					else if (pobState != m_pobCurrentState)
					{
						pobBoss->GetBossMovement()->ClearMovementInput();

						// We've chained states
						SwitchStateTo(pobState,pobBoss,pobPlayer);
					}
				}
			}
			else
			{
				SwitchStateTo(GetNextState(),pobBoss,pobPlayer);
			}

			// Have we finished?
			bool bIsFinished = m_obEndConditions.size() > 0;
			ntstd::List<BossAttackPhaseEndCondition*>::iterator obIt = m_obEndConditions.begin();
			while (obIt != m_obEndConditions.end())
			{
				bIsFinished &= (*obIt)->IsSatisfied(fTimeDelta,pobBoss,pobPlayer);
				obIt++;
			}
			if (bIsFinished && m_pobEndTransition && pobBoss->CanStartAnAttack() && !m_pobCurrentState->GetCurrentAttack())
			{
				// Begin end transition
				if(m_pobEndTransition->BeginEndTransition(pobBoss, pobPlayer))
				{
					m_eCurrentMode = BAPM_END_PHASE_TRANSITION;
				}
				else
				{
					//If we failed to initialise our end transition then we stay in our current phase. We'll try again soon.
					//NOTE: If the end transition fails to initialise after successfully pushing on a movement, then the boss
					//could become stuck.
					//NOTE2: If the end transition can NEVER successfully initialise for any reason, then the bossfight will
					//not progress any further phase-wise... it's important to make sure this never happens really.
					user_warn_msg(("End transition failed to initialise for some reason Potential badness. Please tell GavC how you did this"));
//					user_error_p(false, ("End transition failed to initialise for some reason. Please tell GavC how you did this"));
				}
			}

			break;
		}	
	case BAPM_START_PHASE_TRANSITION:
		{
			ntError( m_pobStartTransition );

			// Keep updating the transition till it's finished
			if (m_pobStartTransition->Update(fTimeDelta,pobBoss,pobPlayer))
			{
				// Change some vars to force a state change next frame
				m_pobCurrentState = 0;
				m_eCurrentMode = BAPM_IN_ATTACK_STATE;
			}
			break;
		}
	case BAPM_END_PHASE_TRANSITION:
		{
			ntError( m_pobEndTransition );

			// Keep updating the transition till it's finished
			if (m_pobEndTransition->Update(fTimeDelta,pobBoss,pobPlayer))
				return m_pobEndTransition->GetToPhase(); // Return the next phase so we switch
			break;
		}
	}

	// Keep returning this attack phase until we're ready to switch
	return this;
}

void BossAttackPhase::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_RED,0,"Attack Phase: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );

	switch ( m_eCurrentMode )
	{
	case BAPM_COUNT:
		{
			ntAssert( 0 );
			break;
		}	
	case BAPM_IN_ATTACK_STATE:
		{
			if ( m_pobCurrentState )
			{
				m_pobCurrentState->DebugRender(obScreenLocation, fXOffset + DEBUG_SHIFT_AMOUNT, fYOffset + DEBUG_SHIFT_AMOUNT);
			}
			else
			{
				g_VisualDebug->Printf3D( obScreenLocation, fXOffset + DEBUG_SHIFT_AMOUNT, fYOffset + DEBUG_SHIFT_AMOUNT, DC_GREEN, 0, "No active state." );
			}

			break;
		}	
	case BAPM_START_PHASE_TRANSITION:
		{
			m_pobStartTransition->DebugRender(obScreenLocation, fXOffset + DEBUG_SHIFT_AMOUNT, fYOffset + DEBUG_SHIFT_AMOUNT);
			break;
		}
	case BAPM_END_PHASE_TRANSITION:
		{
			m_pobEndTransition->DebugRender(obScreenLocation, fXOffset + DEBUG_SHIFT_AMOUNT, fYOffset + DEBUG_SHIFT_AMOUNT);
			break;
		}
	}
#endif
}

const CAttackLink* BossAttackPhase::GetNextAttackLink(const CAttackLink* pobLink)
{
	if (!pobLink)
		return pobLink;
	
	// Run through and look down the list until we find one
	for (int i = 0; i < AM_NONE; i++)
	{
		if (pobLink->m_pobLinks[i])
			return pobLink->m_pobLinks[i];
	}

	return 0;
}

class comparatorAscending
{
public:
	bool operator()( ntstd::pair<float, BossAttackPhaseState*>& obFirst, ntstd::pair<float, BossAttackPhaseState*>& obSecond ) const
	{
		return obFirst.first < obSecond.first;
	}
};


BossAttackPhaseState* BossAttackPhase::GetNextState()
{
	// Save some hassle if possible
	if (m_obAttackPhaseStates.size() == 1)
	{
		return *m_obAttackPhaseStates.begin();
	}

	float fTotal = 0.0f;
	for (ntstd::List<BossAttackPhaseState*>::iterator obIt = m_obAttackPhaseStates.begin(); obIt != m_obAttackPhaseStates.end(); obIt++)
	{
		if ((*obIt) != m_pobCurrentState)
			fTotal += (*obIt)->GetProbabilityOfSwitchingTo();
	}

	if ( fTotal == 0.0f )
	{
		return 0;
	}
		
	float fLastWeight = 0.0f;
	ntstd::Vector< ntstd::pair<float, BossAttackPhaseState*> > aobNormalisedProbabilities;
	for (ntstd::List<BossAttackPhaseState*>::iterator obIt = m_obAttackPhaseStates.begin(); obIt != m_obAttackPhaseStates.end(); obIt++)
	{
		if ((*obIt) != m_pobCurrentState)
		{
			ntstd::pair<float, BossAttackPhaseState*> obPair;
			obPair.first = ((*obIt)->GetProbabilityOfSwitchingTo() / fTotal) + fLastWeight;
			obPair.second = (*obIt);
			aobNormalisedProbabilities.push_back(obPair);
			fLastWeight = obPair.first;
		}
	}

	ntstd::sort(aobNormalisedProbabilities.begin(),aobNormalisedProbabilities.end(),comparatorAscending());

	// If this is our first update, need to return the highest probability state
	if (m_bFirstUpdate)
	{
		return aobNormalisedProbabilities[aobNormalisedProbabilities.size()-1].second;
	}
	else
	{
		float fRand = BOSS_RAND_F(1.0f);
		for (int i = 0; i <= (int)aobNormalisedProbabilities.size(); i++)
		{
			if ( i == 0 )
			{
				float fProb = aobNormalisedProbabilities[i].first;

				if (fRand >= 0.0f && fRand < fProb)
					return aobNormalisedProbabilities[i].second;
			}
			else if ( i == (int)aobNormalisedProbabilities.size() )
			{
				float fProb = aobNormalisedProbabilities[i-1].first;

				if (fRand > fProb && fRand <= 1.0f)
					return aobNormalisedProbabilities[i-1].second;
			}
			else
			{
				float fPreviousProb = aobNormalisedProbabilities[i-1].first;
				float fNextProb = aobNormalisedProbabilities[i].first;

				if (fRand > fPreviousProb && fRand < fNextProb)
					return aobNormalisedProbabilities[i].second;
			}
		}
	}

	// No state to switch to
	return 0;
}

bool BossAttackPhase::ShouldBlock(CStrike* pobStrike) 
{ 
	switch ( m_eCurrentMode )
	{
	case BAPM_IN_ATTACK_STATE:
		{			
			if (m_pobCurrentState)
				return m_pobCurrentState->ShouldBlock(pobStrike);
			else
				return true;
			break;
		}
	default:
		{
			return true;
			break;
		}
	}

	return true;
}

bool BossAttackPhase::IsVulnerableTo(CStrike* pobStrike)
{
	switch ( m_eCurrentMode )
	{
	case BAPM_IN_ATTACK_STATE:
		{			
			if (m_pobCurrentState)
				return m_pobCurrentState->IsVulnerableTo(pobStrike);
			else
				return false;
			break;
		}
	default:
		{
			return false;
			break;
		}
	}

	return false;
}

bool BossAttackPhase::IsVulnerableTo(const CAttackData* pobAttackData)
{
	switch ( m_eCurrentMode )
	{
	case BAPM_IN_ATTACK_STATE:
		{			
			if (m_pobCurrentState)
				return m_pobCurrentState->IsVulnerableTo(pobAttackData);
			else
				return false;
			break;
		}
	default:
		{
			return false;
			break;
		}
	}
	return false;
}

void BossAttackPhase::NotifyPlayerInteracting(bool bState)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyPlayerInteracting(bState);
}

void BossAttackPhase::NotifyPlayerInteractionAction()
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyPlayerInteractionAction();
}

void BossAttackPhase::NotifyProjectileCountered(Object_Projectile* pobProj)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyProjectileCountered(pobProj);
}

void BossAttackPhase::NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyCreatedStrikeVolume(pobVol);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyCreatedStrikeVolume(pobVol);

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyCreatedStrikeVolume(pobVol);
}

void BossAttackPhase::NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyRemovedStrikeVolume(pobVol);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyRemovedStrikeVolume(pobVol);

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyRemovedStrikeVolume(pobVol);
}

void BossAttackPhase::NotifySuperStyleDone(Boss* pobBoss)
{
	// Use this as an indicator that we might need to play a final ninja sequence
	if (!m_obSuperStyleNinjaSequenceEntityName.IsNull() && pobBoss->GetCurrHealth() < m_fSuperStyleNinjaSequenceHealthThreshold)
	{
		// Try to find the NS
		CEntity* pobNinjaSequenceEntity = CEntityManager::Get().FindEntity(m_obSuperStyleNinjaSequenceEntityName);

		if (pobNinjaSequenceEntity)
		{
			pobNinjaSequenceEntity->GetMessageHandler()->Receive( CMessageHandler::Make( pobBoss, "Trigger" ) );
		}
	}
}

void BossAttackPhase::NotifyInvulnerableToIncomingStrike(CStrike* pobStrike)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyInvulnerableToIncomingStrike(pobStrike);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyInvulnerableToIncomingStrike(pobStrike);

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyInvulnerableToIncomingStrike(pobStrike);
}

void BossAttackPhase::NotifyWillBlockIncomingStrike(CStrike* pobStrike)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyWillBlockIncomingStrike(pobStrike);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyWillBlockIncomingStrike(pobStrike);

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyWillBlockIncomingStrike(pobStrike);
}

void BossAttackPhase::NotifyVulnerableToIncomingStrike(CStrike* pobStrike)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyVulnerableToIncomingStrike(pobStrike);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyVulnerableToIncomingStrike(pobStrike);

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyVulnerableToIncomingStrike(pobStrike);
}

void BossAttackPhase::NotifyIsInFailedStrikeRecovery(bool bFailedStrike)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyIsInFailedStrikeRecovery(bFailedStrike);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyIsInFailedStrikeRecovery(bFailedStrike);

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyIsInFailedStrikeRecovery(bFailedStrike);
}

void BossAttackPhase::NotifyInteractionWith(CEntity* pobEntity)
{	
	// Delegate down to state
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyInteractionWith(pobEntity);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyInteractionWith(pobEntity);

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyInteractionWith(pobEntity);
}

void BossAttackPhase::NotifyAttackStarted(Boss* pobBoss)
{
	// Delegate down to state
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyAttackStarted(pobBoss);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyAttackStarted();

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyAttackStarted();
}

void BossAttackPhase::NotifyMovementDone(Boss* pobBoss)
{
	// Delegate down to state
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyMovementDone(pobBoss);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyMovementDone();

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyMovementDone();
}

void BossAttackPhase::NotifyGotStruck(Boss* pobBoss)
{
	// Delegate down to state
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyGotStruck(pobBoss);

	// Don't need to tell start/end transitions because we're invulnerable in them
}

void BossAttackPhase::NotifyAttackFinished(Boss* pobBoss)
{
	// Delegate down to state
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyAttackFinished(pobBoss);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyAttackFinished();

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyAttackFinished();
}

void BossAttackPhase::NotifyAttackAutoLinked(Boss* pobBoss)
{
	// Delegate down to state
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyAttackAutoLinked(pobBoss);

	// And to start transitions
	if (m_eCurrentMode == BAPM_START_PHASE_TRANSITION && m_pobStartTransition)
		m_pobStartTransition->NotifyAttackAutoLinked();

	// And to end transitions
	if (m_eCurrentMode == BAPM_END_PHASE_TRANSITION && m_pobEndTransition)
		m_pobEndTransition->NotifyAttackAutoLinked();
}

void BossAttackPhase::NotifyAttackInterrupted(Boss* pobBoss)
{
	// Might need to do something special
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyAttackInterrupted(pobBoss);
}

void BossAttackPhase::NotifyDeflected(Boss* pobBoss)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyDeflected(pobBoss);
}

void BossAttackPhase::NotifyRecovered(Boss* pobBoss)
{
	if (m_pobCurrentState)
		m_pobCurrentState->NotifyRecovered(pobBoss);
}

void BossAttackPhase::SwitchStateTo(BossAttackPhaseState* pobTo, Boss* pobBoss, CEntity* pobPlayer)
{
	if (pobTo)
	{
		m_pobCurrentState = pobTo;
		m_pobCurrentState->Initialise(pobBoss,pobPlayer);
	}
}

//--------------------------------------------------
//!
//! HealthBossAttackPhaseEndCondition 
//! Class to decide if a phase is over based on health value
//!
//--------------------------------------------------
bool HealthBossAttackPhaseEndCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	return pobBoss->GetCurrHealth() < m_fHealthThreshold;
}

//--------------------------------------------------
//!
//! BossAttackPhaseEndTransition 
//! Base class to handle transition into another phase
//!
//--------------------------------------------------
BossAttackPhase* BossAttackPhaseEndTransition::GetToPhase()
{
	return m_pobToPhase;
}

//--------------------------------------------------
//!
//! BossAttackPhaseEndTransitionAnimated 
//! Class to handle animated transition into another phase
//!
//--------------------------------------------------
bool BossAttackPhaseEndTransitionAnimated::BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer)
{
	SimpleTransitionDef obDef;
	obDef.SetDebugNames("BeginEndTransition","SimpleTransitionDef");
	obDef.m_bApplyGravity = true;
	obDef.m_bLooping = false;
	obDef.m_obAnimationName = m_obEndAnimName;

	pobBoss->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

	m_bFinished = false;
	return true;
}

void BossAttackPhaseEndTransitionAnimated::NotifyMovementDone()
{
	m_bFinished = true;
}

bool BossAttackPhaseEndTransitionAnimated::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer);
	return m_bFinished; 
}

void BossAttackPhaseEndTransitionAnimated::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"End Transition Animated: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString());
#endif
}

//--------------------------------------------------
//!
//! BossAttackPhaseEndTransitionWithMovement 
//! Class to handle animated transition into another phase
//!
//--------------------------------------------------
bool BossAttackPhaseEndTransitionWithMovement::BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer)
{
	m_bFinished = m_pobMovementToDo->Initialise(pobBoss,pobPlayer) == 0;
	return m_bFinished;
}

void BossAttackPhaseEndTransitionWithMovement::NotifyMovementDone()
{
	if (!m_bFinished)
		m_pobMovementToDo->NotifyMovementDone();
}

bool BossAttackPhaseEndTransitionWithMovement::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	if (!m_bFinished)
		m_bFinished = m_pobMovementToDo->DoMovement(fTimeDelta,pobBoss,pobPlayer) == 0;

	return m_bFinished; 
}

void BossAttackPhaseEndTransitionWithMovement::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"End Transition With Movement: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString());
#endif
}


//--------------------------------------------------
//!
//! BossAttackPhaseEndTransitionNinjaSequence 
//! Class to handle animated transition into another phase
//!
//--------------------------------------------------
bool BossAttackPhaseEndTransitionNinjaSequence::BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer)
{
	// Try to find the NS
	m_pobNinjaSequenceEntity = CEntityManager::Get().FindEntity(m_obNinjaSequenceEntityName);

	if (m_pobNinjaSequenceEntity)
	{
		m_pobNinjaSequenceEntity->GetMessageHandler()->Receive( CMessageHandler::Make( pobBoss, "Trigger" ) );
	}

	// If we have no ns entity, we're done
	m_bFinished = m_pobNinjaSequenceEntity != 0;

	m_bFirstFrame = true;
	return true;
}

bool BossAttackPhaseEndTransitionNinjaSequence::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	return true;
}

void BossAttackPhaseEndTransitionNinjaSequence::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"End Transition Ninja Sequence: %s", m_obNinjaSequenceEntityName.GetDebugString());
#endif
}

//--------------------------------------------------
//!
//! BossAttackPhaseEndTransitionSpecialAttacking 
//! Class to handle attacking transition into another phase
//!
//--------------------------------------------------
BossSpecialAttack* BossAttackPhaseEndTransitionSpecialAttacking::GetBossSpecialAttack()
{
	return m_pobEndAttack;
}


void BossAttackPhaseEndTransitionSpecialAttacking::NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) 
{ 
	m_pobEndAttack->NotifyCreatedStrikeVolume(pobVol); 
};

void BossAttackPhaseEndTransitionSpecialAttacking::NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) 
{ 
	m_pobEndAttack->NotifyRemovedStrikeVolume(pobVol); 
};

bool BossAttackPhaseEndTransitionSpecialAttacking::BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer)
{
	if(m_pobEndAttack->Initialise(pobBoss,pobPlayer))
	{
		return true;
	}

	return false;
}

bool BossAttackPhaseEndTransitionSpecialAttacking::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	return !m_pobEndAttack->Update(fTimeDelta,pobBoss,pobPlayer);
}

void BossAttackPhaseEndTransitionSpecialAttacking::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"End Transition Special Attacking: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString());
	fYOffset += DEBUG_SHIFT_AMOUNT;
	m_pobEndAttack->DebugRender(obScreenLocation,fXOffset,fYOffset);
#endif
}

//--------------------------------------------------
//!
//! BossAttackPhaseStartTransitionAnimated 
//! Class to handle animated transition into a phase
//!
//--------------------------------------------------
void BossAttackPhaseStartTransitionAnimated::BeginStartTransition(Boss* pobBoss, CEntity* pobPlayer)
{
	SimpleTransitionDef obDef;
	obDef.SetDebugNames("BeginStartTransition","SimpleTransitionDef");
	obDef.m_bApplyGravity = true;
	obDef.m_bLooping = false;
	obDef.m_obAnimationName = m_obStartAnimName;

	pobBoss->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

	m_bFinished = false;
}

void BossAttackPhaseStartTransitionAnimated::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Start Transition Animated: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString());
#endif
}

void BossAttackPhaseStartTransitionAnimated::NotifyMovementDone()
{
	m_bFinished = true;
}

bool BossAttackPhaseStartTransitionAnimated::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer);
	return m_bFinished; 
}

void BossAttackPhaseStartTransitionSpecialAttacking::NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) 
{ 
	m_pobStartAttack->NotifyCreatedStrikeVolume(pobVol); 
};

void BossAttackPhaseStartTransitionSpecialAttacking::NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) 
{ 
	m_pobStartAttack->NotifyRemovedStrikeVolume(pobVol); 
};
	

//--------------------------------------------------
//!
//! BossAttackPhaseStartTransitionSpecialAttacking 
//! Class to handle attacking transition into a phase
//!
//--------------------------------------------------
BossSpecialAttack* BossAttackPhaseStartTransitionSpecialAttacking::GetBossSpecialAttack()
{
	return m_pobStartAttack;
}

void BossAttackPhaseStartTransitionSpecialAttacking::BeginStartTransition(Boss* pobBoss, CEntity* pobPlayer)
{
	m_pobStartAttack->Initialise(pobBoss,pobPlayer);
}

bool BossAttackPhaseStartTransitionSpecialAttacking::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	return !m_pobStartAttack->Update(fTimeDelta,pobBoss,pobPlayer);
}

void BossAttackPhaseStartTransitionSpecialAttacking::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Start Transition Special Attacking: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString());
	fYOffset += DEBUG_SHIFT_AMOUNT;
	m_pobStartAttack->DebugRender(obScreenLocation,fXOffset,fYOffset);
#endif
}

//--------------------------------------------------
//!
//! BossAttackPhaseEndCondition 
//! Base class to decide if a phase is over
//!
//--------------------------------------------------
// Can't do this inline for GCC/Linker weirdness reasons
bool BossAttackPhaseEndCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{ 
	UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); return true; 
}

class attackCompareAscending
{
public:
	bool operator()( ntstd::pair<float, BossAttack*>& obFirst, ntstd::pair<float, BossAttack*>& obSecond) const
	{
		return obFirst.first < obSecond.first;
	}
};

class movementCompareAscending
{
public:
	bool operator()( ntstd::pair<float, BossMovement*>& obFirst, ntstd::pair<float, BossMovement*>& obSecond) const
	{
		return obFirst.first < obSecond.first;
	}
};

//--------------------------------------------------
//!
//! ChooseRandomAttackWithWeighting 
//! Takes all the attacks in a current selector, and randomly chooses one giving each of
//! them it's own weighting in how likely it is to be chosen.
//!
//--------------------------------------------------
BossAttack* BossAttackSelector::ChooseRandomAttackWithWeighting(Boss* pobBoss, CEntity* pobPlayer, ntstd::List<BossAttack*>* pobAlternateList)
{
	// Use alternate list if possible, otherwise default to member list
	ntstd::List<BossAttack*>* pobAttacks = pobAlternateList;
	if (!pobAttacks)
		pobAttacks = &m_obAttacks;

	if(pobAttacks->size() == 1)
	{
		BossAttack* pReturn = *pobAttacks->begin();
		if(pReturn->Initialise(pobBoss, pobPlayer))
		{
			return *pobAttacks->begin();
		}
		
		return 0;
	}

	float fTotal = 0.0f;
	for (ntstd::List<BossAttack*>::iterator obIt = pobAttacks->begin(); obIt != pobAttacks->end(); obIt++)
	{
		fTotal += (*obIt)->GetAttackProbability();
	}

	if(fTotal == 0.0f)
	{
		return 0;
	}

	BossAttack* pAttack = 0;

	float fLastWeight = 0.0f;
	ntstd::Vector< ntstd::pair<float, BossAttack*> > aobNormalisedAttackProbabilities;
	for(ntstd::List<BossAttack*>::iterator obIt = pobAttacks->begin() ; obIt != pobAttacks->end() ; obIt++)
	{
		ntstd::pair<float, BossAttack*> obPair;
		float fAttackWeight = (float)((*obIt)->GetAttackProbability() / fTotal) + fLastWeight;
		obPair.first = fAttackWeight;
		obPair.second = (*obIt);
		aobNormalisedAttackProbabilities.push_back(obPair);
		fLastWeight = fAttackWeight;

		if(fLastWeight > 1.0f)
		{
			ntAssert_p(false, ("This should never happen with normalised weights... have Gavin C look into it!"));
			return 0;
		}
	}

	ntstd::sort(aobNormalisedAttackProbabilities.begin(), aobNormalisedAttackProbabilities.end(), attackCompareAscending());

	float fRand = BOSS_RAND_F(1.0f);
	for(int i = 0 ; i <= (int)aobNormalisedAttackProbabilities.size() ; i++)
	{
		if ( i == 0 )
		{
			float fProb = aobNormalisedAttackProbabilities[i].first;

			if (fRand >= 0.0f && fRand <= fProb)
			{
				pAttack = aobNormalisedAttackProbabilities[i].second;
				break;
			}
		}
		else if ( i == (int)aobNormalisedAttackProbabilities.size() )
		{
			float fProb = aobNormalisedAttackProbabilities[i-1].first;

			if (fRand >= fProb && fRand <= 1.0f)
			{
				pAttack = aobNormalisedAttackProbabilities[i-1].second;
				break;
			}
		}
		else
		{
			float fPreviousProb = aobNormalisedAttackProbabilities[i-1].first;
			float fNextProb = aobNormalisedAttackProbabilities[i].first;

			if (fRand > fPreviousProb && fRand < fNextProb)
			{
				pAttack = aobNormalisedAttackProbabilities[i].second;
				break;
			}
		}
	}

	if(!pAttack)
	{
		ntAssert_p(false, ("We should never get here, we should have chosen an attack above"));
		return 0;
	}

	//If our selected attack fails to initialise, then we just iterate backwards from most-likely attack to least-likely until
	//one of them works!
	if(pAttack->Initialise(pobBoss, pobPlayer))
	{
		return pAttack;
	}
	else
	{
		return 0;
	}

	return pAttack;
}

//--------------------------------------------------
//!
//! ChooseRandomMovementWithWeighting 
//!
//--------------------------------------------------
BossMovement* BossMovementSelector::ChooseRandomMovementWithWeighting(ntstd::List<BossMovement*>* pobAlternateList)
{
	// Use alternate list if possible, otherwise default to member list
	ntstd::List<BossMovement*>* pobMovements = pobAlternateList;
	if (!pobMovements)
		pobMovements = &m_obMovements;

	if(pobMovements->size() == 1)
	{
		return *pobMovements->begin();
	}

	float fTotal = 0.0f;
	for (ntstd::List<BossMovement*>::iterator obIt = pobMovements->begin(); obIt != pobMovements->end(); obIt++)
	{
		fTotal += (*obIt)->GetMovementProbability();
	}

	if(fTotal == 0.0f)
	{
		return 0;
	}

	BossMovement* pobMovement = 0;

	float fLastWeight = 0.0f;
	ntstd::Vector< ntstd::pair<float, BossMovement*> > aobNormalisedMovementProbabilities;
	for(ntstd::List<BossMovement*>::iterator obIt = pobMovements->begin() ; obIt != pobMovements->end() ; obIt++)
	{
		ntstd::pair<float, BossMovement*> obPair;
		float fMovementWeight = (float)((*obIt)->GetMovementProbability() / fTotal) + fLastWeight;
		obPair.first = fMovementWeight;
		obPair.second = (*obIt);
		aobNormalisedMovementProbabilities.push_back(obPair);
		fLastWeight = fMovementWeight;

		if(fLastWeight > 1.0f)
		{
			ntAssert_p(false, ("This should never happen with normalised weights... have Gavin C look into it!"));
			return 0;
		}
	}

	ntstd::sort(aobNormalisedMovementProbabilities.begin(), aobNormalisedMovementProbabilities.end(), movementCompareAscending());

	float fRand = BOSS_RAND_F(1.0f);
	for(int i = 0 ; i <= (int)aobNormalisedMovementProbabilities.size() ; i++)
	{
		if ( i == 0 )
		{
			float fProb = aobNormalisedMovementProbabilities[i].first;

			if (fRand >= 0.0f && fRand <= fProb)
			{
				pobMovement = aobNormalisedMovementProbabilities[i].second;
				break;
			}
		}
		else if ( i == (int)aobNormalisedMovementProbabilities.size() )
		{
			float fProb = aobNormalisedMovementProbabilities[i-1].first;

			if (fRand >= fProb && fRand <= 1.0f)
			{
				pobMovement = aobNormalisedMovementProbabilities[i-1].second;
				break;
			}
		}
		else
		{
			float fPreviousProb = aobNormalisedMovementProbabilities[i-1].first;
			float fNextProb = aobNormalisedMovementProbabilities[i].first;

			if (fRand > fPreviousProb && fRand < fNextProb)
			{
				pobMovement = aobNormalisedMovementProbabilities[i].second;
				break;
			}
		}
	}

	if(!pobMovement)
	{
		ntAssert_p(false, ("We should never get here, we should have chosen an movement above"));
		return 0;
	}

	return pobMovement;
}

//--------------------------------------------------
//!
//! UnderAttackBossAttackSelector 
//! Class to select an attack in under attack
//!
//--------------------------------------------------
float UnderAttackBossAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	if (pobBoss->IsUnderAttack())
	{
		return m_fMaxPriority;
	}
	else
	{
		return m_fMinPriority;
	}
}

BossAttack* UnderAttackBossAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	if (m_obAttacks.size() > 0)
	{
		return ChooseRandomAttackWithWeighting(pobBoss, pobPlayer);
	}

	return 0;
}

//--------------------------------------------------
//!
//! DistanceSuitabilityBossAttackSelector 
//!
//--------------------------------------------------
float DistanceSuitabilityBossAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	m_pobSelectedAttack = 0;

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

	m_pobSelectedAttack = 0;
	return m_fMinPriority;
}

BossAttack* DistanceSuitabilityBossAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
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
//! RandomBossAttackSelector 
//! Class to select an attack randomly
//!
//--------------------------------------------------
float RandomBossAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	return (m_fMinPriority + m_fMaxPriority) / 2.0f;
}


//--------------------------------------------------
//!
//! BeginAttack 
//! Randomly selects an attack but gives each attack in the selector it's own weighting.
//!
//--------------------------------------------------
BossAttack* RandomBossAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	if (m_obAttacks.size() > 0)
	{
		return ChooseRandomAttackWithWeighting(pobBoss, pobPlayer);
	}

	return 0;
}

//--------------------------------------------------
//!
//! HealthBossAttackSelector 
//! Class to select an attack based on health lost over a period of time
//!
//--------------------------------------------------
HealthBossAttackSelector::HealthBossAttackSelector()
{
	m_fTimeOfLastSample = 0.0f;
	m_bFirstUpdate = true;

	m_fHealthLostThreshold = 25.0f;
	m_fHealthSampleRate = 2.0f;
}

void HealthBossAttackSelector::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	m_fTimeOfLastSample = pobBoss->GetCurrentAttackPhase()->GetCurrentBossAttackPhaseState()->GetTimeSpentIn();
	
	if (m_bFirstUpdate)
	{
		m_afHealthSamples[0] = pobBoss->GetCurrHealth();
		m_afHealthSamples[1] = pobBoss->GetCurrHealth();	
		m_bFirstUpdate = false;
	}
	else
	{
		m_afHealthSamples[1] = m_afHealthSamples[0];			
		m_afHealthSamples[0] = pobBoss->GetCurrHealth();
	}
}

float HealthBossAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	float fTimeInState = pobBoss->GetCurrentAttackPhase()->GetCurrentBossAttackPhaseState()->GetTimeSpentIn();

	// Is it time for another sample
    if (fTimeInState > m_fTimeOfLastSample + m_fHealthSampleRate)
	{
		m_afHealthSamples[1] = m_afHealthSamples[0];			
		m_afHealthSamples[0] = pobBoss->GetCurrHealth();

		m_fTimeOfLastSample = fTimeInState;
	}
	
	if (m_afHealthSamples[1] - m_afHealthSamples[0] > m_fHealthLostThreshold)
	{
		// We've lost our threshold amount of health, we absolutely must select the next attack
		return m_fMaxPriority;
	}

	return m_fMinPriority;
}

BossAttack* HealthBossAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	// If we have more than one attack, select it randomly
	if (m_obAttacks.size() > 0)
	{
		return ChooseRandomAttackWithWeighting(pobBoss, pobPlayer);
	}
	return 0;
}


//--------------------------------------------------
//!
//! BossMeleeAttack 
//! Class to wrap standard melee attacks
//!
//--------------------------------------------------
BossMeleeAttack::BossMeleeAttack()
{
	m_pobRootAttackLink = m_pobCurrentAttackLink = 0;

	m_fAttackProbability = 0.5f;
	
	m_bFinished = m_bDecidedOnLink = false;

	m_bInFailedStrikeRecovery = false;

	m_bVulnerableDuring = m_bVulnerableDuringFailedStrikeRecovery = true;
}

void BossMeleeAttack::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Melee Attack: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString(), ObjectDatabase::Get().GetNameFromPointer(m_pobCurrentAttackLink).GetString() );
#endif
}

float BossMeleeAttack::GetMaxDistance() 
{ 
	ntError(m_pobRootAttackLink); 
	return m_pobRootAttackLink->GetAttackDataP()->m_fMaxDistance; 
}

bool BossMeleeAttack::IsVulnerableTo(CStrike* pobStrike) 
{ 
	UNUSED( pobStrike ); 

	if ( m_bInFailedStrikeRecovery )
		return m_bVulnerableDuringFailedStrikeRecovery;
	else
		return m_bVulnerableDuring; 
}
	
bool BossMeleeAttack::IsVulnerableTo(const CAttackData* pobAttackData) 
{ 
	UNUSED( pobAttackData ); 

	if ( m_bInFailedStrikeRecovery )
		return m_bVulnerableDuringFailedStrikeRecovery;
	else
	return m_bVulnerableDuring; 
}

bool BossMeleeAttack::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	m_bInFailedStrikeRecovery = false;
	m_bFinished = false;
	m_pobCurrentAttackLink = m_pobRootAttackLink;
	m_iDepth = 0;

	// This'll push on our attack
	if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobCurrentAttackLink))
	{
		// This'll start it 
		pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
	}
	else
	{
		m_bFinished = true;
		return false;
	}

	return true;
}

BossAttack* BossMeleeAttack::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
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
			ntPrintf("BossMeleeAttack link probability for depth %i not found in string. Attack will not link.\n",m_iDepth);

		if (fProbability > fRand)
		{
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

void BossMeleeAttack::NotifyAttackFinished()
{ 
	m_bFinished = true;
}

void BossMeleeAttack::NotifyAttackStarted()
{ 
	m_bDecidedOnLink = false;
}

/***************************************************************************************************
*
*	FUNCTION		Boss::ChangeHealth
*
*	DESCRIPTION		
*
***************************************************************************************************/
void Boss::ChangeHealth( float fDelta, const char* pcReason )
{ 
	// Bosses cannot die.
	if( m_bIsInvulnerable || m_fCurrHealth + fDelta <= 0.0f )
		return;

	UNUSED( pcReason );

	m_fLastHealth = m_fCurrHealth; 
	m_fCurrHealth += fDelta; 

#ifndef _RELEASE
	DebugUpdateHealthHistory( fDelta, pcReason );
#endif // _RELEASE
}

//--------------------------------------------------
//!
//! BossPlayerRelativeVectorMovement 
//! Class to do player relative movement at a specified speed
//!
//--------------------------------------------------
BossMovement* BossPlayerRelativeVectorMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	m_fTimeInMovement += fTimeDelta;

	CDirection obToPlayer = CDirection( pobPlayer->GetPosition() - pobBoss->GetPosition() );
	float fDistanceSquared = obToPlayer.LengthSquared();
	obToPlayer.Normalise();

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

	if (m_fTimeInMovement < m_fTimeToDoMovementExclusivelyThisTime)
		return this;
	else
		return 0;
}

BossMovement* BossWalkingMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	//ntPrintf("%s initialised a walking movement.\n", pobBoss->GetName().c_str() );

	m_fTimeInMovement = 0.0f;
	m_fTimeToDoMovementExclusivelyThisTime = m_fTimeToDoMovementExclusively - BOSS_RAND_F(m_fTimeToDoMovementExclusivelyAdjust);

	if (!m_obWalkingController.IsNull())
	{
		pobBoss->GetMovement()->ClearControllers();
		pobBoss->GetMovement()->Lua_StartMovementFromXMLDef( m_obWalkingController );
	}
	return this;
}

BossMovement* BossWalkingMovement::Reinitialise(Boss* pobBoss, CEntity* pobPlayer)
{
	if (!m_obWalkingController.IsNull())
	{
		pobBoss->GetMovement()->Lua_StartMovementFromXMLDef( m_obWalkingController );
		return this;
	}
	else
		return 0;
}

void BossWalkingMovement::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_PURPLE,0,"Boss Walking Movement: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
#endif
}

BossMovement* BossTransitioningMovement::Reinitialise(Boss* pobBoss, CEntity* pobPlayer)
{
	// By default transitioning movements cannot be reinitialised (unless the subclass overrides this and does it itself)
	return 0;
}

//--------------------------------------------------
//!
//! BossPlayerRelativeTransitioningMovement 
//! Class to do player relative movement transition
//!
//--------------------------------------------------
BossMovement* BossPlayerRelativeTransitioningMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	m_fTimeInMovement = 0.0f;

	CDirection obToPlayer = CDirection( pobPlayer->GetPosition() - pobBoss->GetPosition() );
	obToPlayer.Normalise();

	CMatrix obMtx;
	obMtx.SetXAxis( obToPlayer.Cross( CDirection( 0.0f, 1.0f, 0.0f ) ) );
	obMtx.SetYAxis( CDirection( 0.0f, 1.0f, 0.0f ) );
	obMtx.SetZAxis( obToPlayer );

	m_obMoveDirection = m_obVector * obMtx;
	m_obMoveDirection.Y() = 0.0f;
	m_obMoveDirection.Normalise();

	ZAxisAlignTargetedTransitionDef obStartAlignDef;
	obStartAlignDef.SetDebugNames("Boss Move Start","ZAxisAlignTargetedTransitionDef");
	obStartAlignDef.m_bApplyGravity = true;
	obStartAlignDef.m_obAlignZTo = m_obMoveDirection;
	obStartAlignDef.m_obAnimationName = m_obStartMove;

	SimpleTimedTransitionDef obCycle;
	obCycle.SetDebugNames("Boss Move Cycle","SimpleTimedTransitionDef");
	obCycle.m_bApplyGravity = true;
	obCycle.m_bLooping = true;
	obCycle.m_fExtraMovementSpeed = m_fSpeed;
	obCycle.m_obAnimationName = m_obCycleMove;
	obCycle.m_fTime = m_fCycleTime;

	ZAxisAlignTargetedTransitionDef obEndAlignDef;
	obEndAlignDef.SetDebugNames("Boss End","ZAxisAlignTargetedTransitionDef");
	obEndAlignDef.m_bApplyGravity = true;
	obEndAlignDef.m_obAlignZTo = m_obMoveDirection;
	if (m_bAlignToPlayerAtEnd)
	{
		obEndAlignDef.m_pobEntityAlignZTowards = pobPlayer;
	}
	obEndAlignDef.m_obAnimationName = m_obEndMove;

	pobBoss->GetMovement()->BringInNewController( obStartAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobBoss->GetMovement()->AddChainedController( obCycle, CMovement::DMM_STANDARD, m_fCycleTime * 0.25f );
	pobBoss->GetMovement()->AddChainedController( obEndAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

	m_bDone = false;

	ntPrintf("Excluding.\n");
	m_pobAllowEntityCollisionOnCompletion = pobPlayer;
	pobBoss->GetInteractionComponent()->ExcludeCollisionWith(pobPlayer);

	return this;
}

BossMovement* BossPlayerRelativeTransitioningMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderLine(pobBoss->GetPosition(), pobBoss->GetPosition() + m_obMoveDirection, DC_RED);
#endif

	m_fTimeInMovement += fTimeDelta;
	if (!m_bDone)
		return this;
	else
	{
		ntPrintf("Allowing.\n");
		pobBoss->GetInteractionComponent()->AllowCollisionWith(m_pobAllowEntityCollisionOnCompletion);
		return 0;
	}
}

void BossTransitioningMovement::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_PURPLE,0,"Boss Transitioning Movement: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
#endif
}


//===================== BOSS TAUNT MOVEMENT =====================

BossTauntMovement::BossTauntMovement()
{
	m_bDone = false;
	m_bGravity = false;
	m_pobBoss = 0;
}

BossMovement* BossTauntMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	//Store the boss pointer (in-case we need it during update later).
	m_pobBoss = pobBoss;
	m_bDone = false;

	if(!ntStr::IsNull(m_obTauntAnim))
	{
		if (m_bUseSimpleTransition)
		{
			// Simple transition plays whatever's in the anim
			SimpleTransitionDef obTauntDef;
			obTauntDef.SetDebugNames("Boss Taunt", "SimpleTransitionDef");
			obTauntDef.m_bApplyGravity = m_bGravity;
			obTauntDef.m_obAnimationName = m_obTauntAnim;

			pobBoss->GetMovement()->BringInNewController( obTauntDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
		}
		else
		{
		//When we taunt, we want to make sure to face the player at the same time.
		//This continuously updates the desired z-align to point at the player.
		ZAxisAlignTargetedTransitionDef obTauntDef;
		obTauntDef.SetDebugNames("Boss Taunt", "ZAxisAlignTargetedTransitionDef");
		obTauntDef.m_bApplyGravity = m_bGravity;
		obTauntDef.m_pobEntityAlignZTowards = pobPlayer;
		obTauntDef.m_obAnimationName = m_obTauntAnim;

		pobBoss->GetMovement()->BringInNewController( obTauntDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
		}

		Message obMovementMessage(msg_movementdone);
		pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);

		//Successfully initialised.
		return this;
	}

	//Failed to initialise.
	return 0;
}

BossMovement* BossTauntMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer);
	if(!m_bDone)
	{
		return this;
	}

	return 0;
}

//===================== BOSS TAUNT MOVEMENT SELECTOR =====================

BossTauntMovementSelector::BossTauntMovementSelector()
{
	m_fTimeSinceAttackBeforeTaunt = 10.0f;
	m_fMinDistanceToPlayer = 5.0f;
	m_fMaxDistanceToPlayer = 15.0f;
}

float BossTauntMovementSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	if(pobBoss->GetTimeSinceLastPlayerStrike() >= m_fTimeSinceAttackBeforeTaunt)
	{
		//Get the boss and player's position to check for range.
		CPoint obBossPos = pobBoss->GetPosition();
		CPoint obPlayerPos = pobPlayer->GetPosition();
		CDirection obBetween = CDirection(obBossPos - obPlayerPos);
		float fLengthSquared = obBetween.LengthSquared();
		if((fLengthSquared >= (m_fMinDistanceToPlayer * m_fMinDistanceToPlayer)) &&
			(fLengthSquared <= (m_fMaxDistanceToPlayer * m_fMaxDistanceToPlayer)))
		{
			//We're going to taunt, so clear our timer!
			pobBoss->ResetTimeSinceLastPlayerStrike();
			//Everything checks out, taunt!
			return m_fMaxPriority;
		}
	}

	return m_fMinPriority;
}

BossMovement* BossTauntMovementSelector::GetSelectedMovement()
{
	if(m_obMovements.size() > 0)
	{
		return (*m_obMovements.begin());
	}
	return 0;
}

//--------------------------------------------------
//!
//! VulnerableToIncomingAttackBossMovementSelector 
//! A class to do a movement when an incoming strike will hit us
//!
//--------------------------------------------------
float VulnerableToIncomingAttackBossMovementSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) 
{ 
	if (pobBoss->IsVulnerableToIncomingStrike())
	{
		if (BOSS_RAND_F(1.0f) <= m_fProbabilityOfSelecting)
			return 1.0f;
	}
	
	return 0.0f;
}

BossMovement* VulnerableToIncomingAttackBossMovementSelector::Initialise(Boss* pobBoss, CEntity* pobPlayer) 
{
	if (m_obMovements.size() > 0)
		return (*m_obMovements.begin())->Initialise(pobBoss,pobPlayer); 
	else
		return 0;
}

BossMovement* VulnerableToIncomingAttackBossMovementSelector::GetSelectedMovement() 
{
	if (m_obMovements.size() > 0)
		return (*m_obMovements.begin()); 
	else
		return 0;
}

//--------------------------------------------------
//!
//! BossAttackPhaseState 
//! A class representing a state within an attack phase, holds attack and movement selectors, and state timing information
//!
//--------------------------------------------------
BossAttackPhaseState::BossAttackPhaseState()
{
	m_eCurrentMode = BAPSM_ATTACKING;
	m_pobCurrentAttack = 0;
	m_pobCurrentMovement = 0;

	m_fMaxTimeInState = 0.0f;
	m_fMaxTimeInStateAdjust = 0.0f;
	m_fProbabilityOfSwitchingToState = 0.0f;
	m_bVulnerableWhileNotAttacking = true;
	m_fTimeToRememberStrikes = 0.0f;
	m_fTimeToBeInvulnerable = 0.0f;
	m_bGotStruck = false;
	m_iNumberOfStrikesInLastSeconds = 0;
	m_iNumberOfStrikesTillInvulnerable = 0;
	m_bInvulnerableBecauseOfStrikeCountTrigger = false;
	m_fTimeToBeVulnerableAgain = 0.0f;
	m_pobDodgeMovementSelector = 0;
	m_pobDodgeAttackSelector = 0;
	m_bNeedsDodge = false;
	m_bIsDodging = false;

	m_pobLastWinningAttackSelector = 0;
	m_fLastWinningPriority = 0.0f;

    m_fSpeedBlockProbability = m_fRangeBlockProbability = m_fPowerBlockProbability = 0.5f;
}

BossAttackPhaseState::~BossAttackPhaseState()
{
}

bool BossAttackPhaseState::ShouldBlock(CStrike* pobStrike) 
{ 
	//We may have it set on our current attack that we want our recovery to be uninterruptable or invulnerable.
	//In this event, we return false early to prevent the boss from going into auto-block or reacting!
	if(m_pobCurrentAttack)
	{
		if((pobStrike->GetTargetP()->GetAttackComponent()->AI_Access_GetState() == CS_RECOVERING)
			&& (m_pobCurrentAttack->IsUnInterruptableDuringRecovery() || m_pobCurrentAttack->IsInvulnerableDuringRecovery()) )
		{
			return false;
		}
	}

	// Look at probabilities
	float fRand = BOSS_RAND_F(1.0f);
	if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_SPEED_MEDIUM)
	{
		if ( fRand < m_fSpeedBlockProbability )
			return true;
	}
	else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_POWER_MEDIUM)
	{
		if ( fRand < m_fPowerBlockProbability )
			return true;
	}
	else if (pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST || pobStrike->GetAttackDataP()->m_eAttackClass == AC_RANGE_MEDIUM)
	{
		if ( fRand < m_fRangeBlockProbability )
			return true;
	}	

	return false;
}

bool BossAttackPhaseState::IsVulnerableTo(CStrike* pobStrike)
{
	// If we've decided on dodging, we're not vulnerable
	if (m_bNeedsDodge || m_bIsDodging)
		return false;

	bool bRet = false;

	switch ( m_eCurrentMode )
	{
	case BAPSM_ATTACKING:
		{			
			if (m_pobCurrentAttack)
				bRet = m_pobCurrentAttack->IsVulnerableTo(pobStrike);
			else
				bRet = m_bVulnerableWhileNotAttacking && !m_bInvulnerableBecauseOfStrikeCountTrigger;
			break;
		}
	default:
		{
			if (m_pobCurrentMovement)
				bRet = m_pobCurrentMovement->GetVulnerableDuring() && m_bVulnerableWhileNotAttacking && !m_bInvulnerableBecauseOfStrikeCountTrigger;
			else
				bRet = m_bVulnerableWhileNotAttacking && !m_bInvulnerableBecauseOfStrikeCountTrigger;
			break;
		}
	}

	return bRet;
}

bool BossAttackPhaseState::IsVulnerableTo(const CAttackData* pobAttackData)
{
	// If we've decided on dodging, we're not vulnerable
	if (m_bNeedsDodge || m_bIsDodging)
		return false;

	switch ( m_eCurrentMode )
	{
	case BAPSM_ATTACKING:
		{			
			if (m_pobCurrentAttack)
				return m_pobCurrentAttack->IsVulnerableTo(pobAttackData);
			else
				return m_bVulnerableWhileNotAttacking && !m_bInvulnerableBecauseOfStrikeCountTrigger;
			break;
		}
	default:
		{
			if (m_pobCurrentMovement)
				return m_pobCurrentMovement->GetVulnerableDuring() && m_bVulnerableWhileNotAttacking && !m_bInvulnerableBecauseOfStrikeCountTrigger;
			else
				return m_bVulnerableWhileNotAttacking && !m_bInvulnerableBecauseOfStrikeCountTrigger;
			break;
		}
	}
	return false;
}

void BossAttackPhaseState::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	m_fTimeToSpendIn = m_fMaxTimeInState;
	m_fTimeToSpendIn -= BOSS_RAND_F(m_fMaxTimeInStateAdjust);
	m_fTimeSpentIn = 0.0f;
	m_bGotStruck = false;
	m_iNumberOfStrikesInLastSeconds = 0;
	m_bInvulnerableBecauseOfStrikeCountTrigger = false;
	m_fTimeToBeVulnerableAgain = 0.0f;
	m_obTimesToDecrement.clear();
	m_bNeedsDodge = false;
	m_bIsDodging = false;
	
	m_pobCurrentMovement = 0;
	m_pobCurrentAttack = 0;

	ntstd::List<BossAttackSelector*>::iterator obIt = m_obAttackSelectors.begin();
	while (obIt != m_obAttackSelectors.end())
	{
		(*obIt)->Initialise(pobBoss,pobPlayer);
		obIt++;
	}

	m_pobLastWinningAttackSelector = 0;
	m_fLastWinningPriority = 0.0f;
}

void BossAttackPhaseState::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_GREEN,0,"Attack Phase State: %s", ObjectDatabase::Get().GetNameFromPointer(this).GetString() );
	fYOffset += DEBUG_SHIFT_AMOUNT;
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_GREEN,0,"%f seconds remain for this state", m_fTimeToSpendIn - m_fTimeSpentIn );
	if (m_iNumberOfStrikesTillInvulnerable)
	{
		fYOffset += DEBUG_SHIFT_AMOUNT;
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_GREEN,0,"%i strikes in last %f seconds", m_iNumberOfStrikesInLastSeconds, m_fTimeToRememberStrikes );
	}
	if (m_bInvulnerableBecauseOfStrikeCountTrigger)
	{
		fYOffset += DEBUG_SHIFT_AMOUNT;
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_GREEN,0,"Struck >= %i times so I'm INVULNERABLE!", m_iNumberOfStrikesTillInvulnerable );
	}

	switch( m_eCurrentMode )
	{
		case BAPSM_MOVING:
		{
			if (m_pobCurrentMovement)
			{
				m_pobCurrentMovement->DebugRender(obScreenLocation, fXOffset + DEBUG_SHIFT_AMOUNT, fYOffset + DEBUG_SHIFT_AMOUNT);
			}
			else
			{
				g_VisualDebug->Printf3D(obScreenLocation,fXOffset + DEBUG_SHIFT_AMOUNT,fYOffset + DEBUG_SHIFT_AMOUNT,DC_GREEN,0,"Moving with no movement.");
			}

			break;
		}
		case BAPSM_ATTACKING:
		{
			if (m_pobCurrentAttack)
			{
				m_pobCurrentAttack->DebugRender(obScreenLocation, fXOffset + DEBUG_SHIFT_AMOUNT, fYOffset + DEBUG_SHIFT_AMOUNT);
			}
			else
			{
				g_VisualDebug->Printf3D(obScreenLocation,fXOffset + DEBUG_SHIFT_AMOUNT,fYOffset + DEBUG_SHIFT_AMOUNT,DC_GREEN,0,"Attacking with no attack.");
			}

			break;
		}
	}
#endif
}

BossAttackPhaseState* BossAttackPhaseState::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	m_fTimeSpentIn += fTimeDelta;

	// If we need to keep track of strikes that've hit us
	if (m_fTimeToRememberStrikes > 0.0f)
	{
		// Keep track of strikes
		if (m_bGotStruck)
		{
			m_iNumberOfStrikesInLastSeconds++;

			float fTimeTillDecrement = m_fTimeSpentIn + m_fTimeToRememberStrikes;
			m_obTimesToDecrement.push_back(fTimeTillDecrement);

			m_bGotStruck = false;
		}

		if (m_obTimesToDecrement.size() > 0)
		{
			if (m_fTimeSpentIn > *(m_obTimesToDecrement.begin()))
			{
				m_iNumberOfStrikesInLastSeconds--;
				m_obTimesToDecrement.erase(m_obTimesToDecrement.begin());
			}
		}

		if (m_iNumberOfStrikesInLastSeconds >= m_iNumberOfStrikesTillInvulnerable && !m_bInvulnerableBecauseOfStrikeCountTrigger)
		{
			m_bInvulnerableBecauseOfStrikeCountTrigger = true;
			m_fTimeToBeVulnerableAgain = m_fTimeSpentIn + m_fTimeToBeInvulnerable;
		}

		if (m_bInvulnerableBecauseOfStrikeCountTrigger && m_fTimeSpentIn > m_fTimeToBeVulnerableAgain)
		{
			m_bInvulnerableBecauseOfStrikeCountTrigger = false;
		}
	}

	// Deal with a dodge move/attack if it's needed
	// Always update the selectors so they're up to date with the state of the world when they're needed
	if (m_pobDodgeMovementSelector)
		m_pobDodgeMovementSelector->GetPriority(fTimeDelta,pobBoss,pobPlayer);

	if (m_pobDodgeAttackSelector)
		m_pobDodgeAttackSelector->GetPriority(fTimeDelta,pobBoss,pobPlayer);

	if (m_bNeedsDodge)
	{
		ntAssert(m_pobDodgeMovementSelector || m_pobDodgeAttackSelector);

		// By default select the dodge movement, and see if the attack is do-able
		bool bUseAttack = false;
		if (!m_pobDodgeMovementSelector && m_pobDodgeAttackSelector)
			bUseAttack = true;
		else if (m_pobDodgeMovementSelector && m_pobDodgeAttackSelector)
		{
			float fRand = BOSS_RAND_F(1.0f);
			if (fRand > m_fProbabilityOfDodgeMovement)
				bUseAttack = true;
		}

		if (!bUseAttack)
		{
			float fPriority = m_pobDodgeMovementSelector->GetPriority(fTimeDelta,pobBoss,pobPlayer);
			if (fPriority > 0)
			{
				// This'll break the boss out of react state if he's in it 
				Message obAttackMessage(msg_combat_breakout);
				pobBoss->GetMessageHandler()->QueueMessage(obAttackMessage);

				BossMovement* pobMovement = m_pobDodgeMovementSelector->GetSelectedMovement();
				
				// John Li, use good movements.
				// (We need to make sure all dodge attacks will definately initialise, otherwise we get stuck)
				ntError(pobMovement);

				if (pobMovement)
			{
					m_fLastWinningPriority = fPriority;

					pobBoss->GetMovement()->ClearControllers();

					// Stop the current attack/movement
					if (m_pobCurrentAttack)
				{
						m_pobCurrentAttack->NotifyAttackInterrupted();
						m_pobCurrentAttack = 0;
					}

					m_pobCurrentMovement = pobMovement;

			pobBoss->GetMovement()->ClearControllers();
					m_pobCurrentMovement = m_pobCurrentMovement->Initialise(pobBoss,pobPlayer);
			
			m_eCurrentMode = BAPSM_MOVING;

					m_bIsDodging = true;
				}
			}
		}
		else
		{
			// Check if there's a doppelganger manager, if there is, we can upcast and check
			bool bVeto = false;
			if (DoppelgangerManager::Exists())
				bVeto = DoppelgangerManager::Get().CanIAttack((AerialGeneral*)pobBoss);

			float fPriority = m_pobDodgeAttackSelector->GetPriority(fTimeDelta,pobBoss,pobPlayer);
			if (!bVeto && fPriority > 0)
			{
				// This'll break the boss out of react state if he's in it 
				Message obAttackMessage(msg_combat_breakout);
				pobBoss->GetMessageHandler()->QueueMessage(obAttackMessage);

				BossAttack* pobAttack = m_pobDodgeAttackSelector->BeginAttack(pobBoss,pobPlayer);
				
				// John Li, use good attacks.
				// (We need to make sure all dodge attacks will definately initialise, otherwise we get stuck)
				user_warn_p(pobAttack, ("Dodge Attack failed to initialise. Only attacks guaranteed to initialise should be used. Check data!"));
//				ntError(pobAttack);

				if (pobAttack)
				{
					m_fLastWinningPriority = fPriority;

					pobBoss->GetMovement()->ClearControllers();
				
					// Stop the current attack/movement
					if (m_pobCurrentAttack)
					{					
						m_pobCurrentAttack->NotifyAttackInterrupted();
						m_pobCurrentAttack = 0;
					}

					if (m_pobCurrentMovement)
					{
						m_pobCurrentMovement = 0;
					}

					m_pobCurrentAttack = pobAttack;

					pobBoss->ResetTimeSinceBossLastAttacked();

					m_eCurrentMode = BAPSM_ATTACKING;

					m_bIsDodging = true;
				}
			}
		}

		m_bNeedsDodge = false;
	}

	// Sanity check on movement vars, if we have no current movement then there's no need to keep track of whether it's done or not
	if (!m_pobCurrentMovement)
		m_pobCurrentMovementNotDone = 0;

	// If we have time to continue, or we're still attacking/moving
	if ( m_fTimeToSpendIn > m_fTimeSpentIn || m_pobCurrentAttack || (m_pobCurrentMovementNotDone && ! m_pobCurrentMovementNotDone->GetCanBeInterruptedToAttack()) || m_bNeedsDodge || m_bIsDodging )
	{
		switch( m_eCurrentMode )
		{
			case BAPSM_MOVING:
			{
				ntAssert( !m_pobCurrentAttack );

				// Is it appropriate to start looking for new movements?
				m_pobCurrentMovementNotDone = 0;
				if (m_pobCurrentMovement)
					m_pobCurrentMovementNotDone = m_pobCurrentMovement->DoMovement(fTimeDelta,pobBoss,pobPlayer);
				// Shouldn't start movement's in anything other than combat standard
				if ( !m_pobCurrentMovementNotDone && pobBoss->GetAttackComponent()->AI_Access_GetState() == CS_STANDARD )
				{
					// If we've finished a movement then we're definately not dodging any more
					m_bIsDodging = false;

					float fHighestPrioritySoFar = 0.0f;
					BossMovementSelector* pobWinningSelector = 0;

					for (ntstd::List<BossMovementSelector*>::iterator obIt = m_obMovementSelectors.begin();
						obIt != m_obMovementSelectors.end();
						obIt++)
					{
						float fPriority = (*obIt)->GetPriority(fTimeDelta,pobBoss,pobPlayer);
						if (fPriority > fHighestPrioritySoFar)
						{
							fHighestPrioritySoFar = fPriority;
							pobWinningSelector = *obIt;
						}
					}

					if (fHighestPrioritySoFar > 0.0f)
					{
						// Have we switched movement from something we were doing before?
						BossMovement* pobNewMovement = pobWinningSelector->GetSelectedMovement();
						if (pobNewMovement && (pobWinningSelector->GetNumberOfMovements() == 1 || pobNewMovement != m_pobCurrentMovement || !pobBoss->CanStartAnAttack() ) )
						{
							// If so, call initialise on the movement
							m_pobCurrentMovement = pobNewMovement->Initialise(pobBoss, pobPlayer);
							m_pobCurrentMovementNotDone = m_pobCurrentMovement;
						}
					}
				}

				// Breaking out of moving to attack?
				if ( ( ( m_pobCurrentMovement && !m_pobCurrentMovementNotDone ) || (m_pobCurrentMovement && m_pobCurrentMovement->GetCanBeInterruptedToAttack()) ) && pobBoss->CanStartAnAttack())
				{
					float fHighestPrioritySoFar = 0.0f;
					BossAttackSelector* pobWinningSelector = 0;

					ntstd::List<BossAttackSelector*>::iterator obIt = m_obAttackSelectors.begin();
					while (obIt != m_obAttackSelectors.end())
					{
						float fPriority = (*obIt)->GetPriority(fTimeDelta,pobBoss,pobPlayer);
						if (fPriority > fHighestPrioritySoFar)
						{
							fHighestPrioritySoFar = fPriority;
							pobWinningSelector = *obIt;
						}
						obIt++;
					}

					BossAttack* pobSuccess = 0;
					if (fHighestPrioritySoFar > 0.0f)
						pobSuccess = pobWinningSelector->BeginAttack(pobBoss,pobPlayer);
					
					if (pobSuccess)
					{
						m_fLastWinningPriority = fHighestPrioritySoFar;
						m_pobLastWinningAttackSelector = pobWinningSelector;
						m_pobCurrentAttack = pobSuccess;
						m_eCurrentMode = BAPSM_ATTACKING;
						m_pobCurrentMovement = 0;
						m_pobCurrentMovementNotDone = 0;
						pobBoss->ResetTimeSinceBossLastAttacked();
						//ntPrintf("%s initialised an attack.\n", pobBoss->GetName().c_str() );
					}
				}
				
				break;
			}
			case BAPSM_ATTACKING:
			{
				ntAssert(!m_pobCurrentMovement);

				// Keep facing player
				CPoint obPlayerPosition = pobPlayer->GetPosition();
				CPoint obBossPosition = pobBoss->GetPosition();
				pobBoss->GetBossMovement()->m_obFacingDirection = CDirection(obPlayerPosition - obBossPosition);
				pobBoss->GetBossMovement()->m_obFacingDirection.Normalise();
				pobBoss->GetBossMovement()->m_obMoveDirection = pobBoss->GetBossMovement()->m_obFacingDirection;
				pobBoss->GetBossMovement()->m_obTargetPoint = obPlayerPosition;
				pobBoss->GetBossMovement()->m_bTargetPointSet = true;
				pobBoss->GetBossMovement()->m_fMoveSpeed = 1.0f;

				// If we're already attacking...
				if (m_pobCurrentAttack)
				{
					// Go through attack selectors and if any return a higher priority than the priority we got for the current one, then we need to interrupt the current one with the new higher priority one
					float fHighestPrioritySoFar = 0.0f;
					BossAttackSelector* pobWinningSelector = 0;

					ntstd::List<BossAttackSelector*>::iterator obIt = m_obAttackSelectors.begin();
					while (obIt != m_obAttackSelectors.end())
					{
						float fPriority = (*obIt)->GetPriority(fTimeDelta,pobBoss,pobPlayer);
						if (fPriority > fHighestPrioritySoFar)
						{
							fHighestPrioritySoFar = fPriority;
							pobWinningSelector = *obIt;
						}
						obIt++;
					}

					// Should we switch from our current attack?
					if (fHighestPrioritySoFar > m_fLastWinningPriority)
					{
						m_fLastWinningPriority = fHighestPrioritySoFar;
						m_pobLastWinningAttackSelector = pobWinningSelector;

						BossAttack* pNewAttack = pobWinningSelector->BeginAttack(pobBoss, pobPlayer);

						if(pNewAttack)
						{
							pobBoss->ResetTimeSinceBossLastAttacked();
							NotifyAttackInterrupted(pobBoss);
							m_pobCurrentAttack = pNewAttack;
						}
					}

					// Update till it's finished
					if(m_pobCurrentAttack)
						m_pobCurrentAttack = m_pobCurrentAttack->Update(fTimeDelta,pobBoss,pobPlayer);
//					else
//						ntAssert_p(0, ("If the next attack is a special (and a roll) it'll likely break here (attacking with no attack, loop forever)"));
				}
				// If we're not attacking but we could be...
				else if (pobBoss->CanStartAnAttack())
				{
					// If we've finished an attack then we're definately not dodging any more
					m_bIsDodging = false;

					// Loop though selectors seeing who wins...
					float fHighestPrioritySoFar = 0.0f;
					BossAttackSelector* pobWinningSelector = 0;

					ntstd::List<BossAttackSelector*>::iterator obIt = m_obAttackSelectors.begin();
					while (obIt != m_obAttackSelectors.end())
					{
						float fPriority = (*obIt)->GetPriority(fTimeDelta,pobBoss,pobPlayer);
						if (fPriority > fHighestPrioritySoFar)
						{
							fHighestPrioritySoFar = fPriority;
							pobWinningSelector = *obIt;
						}
						obIt++;
					}

					BossAttack* pobSuccess = 0;
					if (fHighestPrioritySoFar > 0.0f)
						pobSuccess = pobWinningSelector->BeginAttack(pobBoss,pobPlayer);
					
					if (pobSuccess)
					{
						m_fLastWinningPriority = fHighestPrioritySoFar;
						m_pobLastWinningAttackSelector = pobWinningSelector;
						m_pobCurrentAttack = pobSuccess;
						m_eCurrentMode = BAPSM_ATTACKING;
						m_pobCurrentMovement = 0;
						m_pobCurrentMovementNotDone = 0;
							pobBoss->ResetTimeSinceBossLastAttacked();
						//ntPrintf("%s initialised an attack.\n", pobBoss->GetName().c_str() );
					}
				}

				// If we didn't start an attack, then we should move around a bit till we can
				if (!m_pobCurrentAttack)
					m_eCurrentMode = BAPSM_MOVING;

				break;
			}
		}

		return this;
	}
	else
	{
		// This state is OVER.
		m_pobCurrentAttack = 0;
		return 0;	
	}
}

void BossAttackPhaseState::NotifyPlayerInteracting(bool bState)
{
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyPlayerInteracting(bState);
}

void BossAttackPhaseState::NotifyPlayerInteractionAction()
{
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyPlayerInteractionAction();
}

void BossAttackPhaseState::NotifyProjectileCountered(Object_Projectile* pobProj)
{
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyProjectileCountered(pobProj);
}

void BossAttackPhaseState::NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyCreatedStrikeVolume(pobVol);
}

void BossAttackPhaseState::NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol)
{
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyRemovedStrikeVolume(pobVol);
}

void BossAttackPhaseState::NotifyVulnerableToIncomingStrike(CStrike* pobStrike)
{
	ntAssert( pobStrike->IsPreStrike() );
	
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyVulnerableToIncomingStrike(pobStrike);

	// Set up a default, and decide if we need to use anything state specific
	int iDepth = 0;
	ntstd::String obProbabilityString = "0.0";
	if ( pobStrike->GetTargetP()->GetAttackComponent()->AI_Access_GetState() == CS_RECOILING )
	{
		obProbabilityString = m_obRecoilingDodgeProbabilityString;
		iDepth = pobStrike->GetTargetP()->GetAttackComponent()->GetDepthInRecoiling();
	}

	// Tokenise dodge probability string using , as a delimeter
	ntstd::Vector<ntstd::String> obProbabilities;
	char cBuffer[256];
	ntAssert(ntStr::GetLength(obProbabilityString.c_str()) < 256);
	strcpy(cBuffer, ntStr::GetString(obProbabilityString.c_str()));
	char* pcNext = strtok(cBuffer, ",");	
	while (pcNext != 0)
	{
		obProbabilities.push_back(ntstd::String(pcNext));
		pcNext = strtok(0, ",");
	}

	float fDodgeProbability = 0.0f;
	if ((int)obProbabilities.size() > iDepth)
		sscanf(obProbabilities[iDepth].c_str(),"%f",&fDodgeProbability);
	else if ((int)obProbabilities.size() > 0)
		sscanf(obProbabilities[(int)obProbabilities.size()-1].c_str(),"%f",&fDodgeProbability);

	if (fDodgeProbability > 0.0f && (m_pobDodgeMovementSelector || m_pobDodgeAttackSelector) )
	{
		if (pobStrike->GetTargetP()->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING && ( !pobStrike->GetTargetP()->GetAttackComponent()->IsInBlockWindow() || !pobStrike->GetTargetP()->GetAttackComponent()->IsInMovementWindow() ) )
		{
			// Can't dodge
		}
		else
		{
			float fRand = BOSS_RAND_F(1.0f);
			if (fRand <= fDodgeProbability)
			{
				// Bit messy, need to const cast to get access to the bosses attack component
				CAttackComponent* pobAttack = const_cast<CAttackComponent*>(pobStrike->GetTargetP()->GetAttackComponent());
				// Manually complete the recovery so that after our dodge all's well with the combat state system
				pobAttack->CompleteRecovery();

				m_bNeedsDodge = true;
			}
		}
	}
}

void BossAttackPhaseState::NotifyInvulnerableToIncomingStrike(CStrike* pobStrike)
{
	ntAssert( pobStrike->IsPreStrike() );
	
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyInvulnerableToIncomingStrike(pobStrike);
}

void BossAttackPhaseState::NotifyWillBlockIncomingStrike(CStrike* pobStrike)
{
	ntAssert( pobStrike->IsPreStrike() );
	
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyWillBlockIncomingStrike(pobStrike);

	// Set up a default, and decide if we need to use anything state specific
	int iDepth = 0;
	ntstd::String obProbabilityString = "0.0";
	// We include deflecting and blocking as they're both 'block' states that we want to take notice of
	if ( pobStrike->GetTargetP()->GetAttackComponent()->AI_Access_GetState() == CS_DEFLECTING || pobStrike->GetTargetP()->GetAttackComponent()->AI_Access_GetState() == CS_BLOCKING )
	{
		obProbabilityString = m_obDeflectingDodgeProbabilityString;
		iDepth = pobStrike->GetTargetP()->GetAttackComponent()->GetDepthInDeflecting();
	}

	// Tokenise dodge probability string using , as a delimeter
	ntstd::Vector<ntstd::String> obProbabilities;
	char cBuffer[256];
	ntAssert(ntStr::GetLength(obProbabilityString.c_str()) < 256);
	strcpy(cBuffer, ntStr::GetString(obProbabilityString.c_str()));
	char* pcNext = strtok(cBuffer, ",");	
	while (pcNext != 0)
	{
		obProbabilities.push_back(ntstd::String(pcNext));
		pcNext = strtok(0, ",");
	}

	float fDodgeProbability = 0.0f;
	if ((int)obProbabilities.size() > iDepth)
		sscanf(obProbabilities[iDepth].c_str(),"%f",&fDodgeProbability);
	else if ((int)obProbabilities.size() > 0)
		sscanf(obProbabilities[(int)obProbabilities.size()-1].c_str(),"%f",&fDodgeProbability);

	if (fDodgeProbability > 0.0f && (m_pobDodgeMovementSelector || m_pobDodgeAttackSelector) )
	{
		if (pobStrike->GetTargetP()->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING && ( !pobStrike->GetTargetP()->GetAttackComponent()->IsInBlockWindow() || !pobStrike->GetTargetP()->GetAttackComponent()->IsInMovementWindow() ) )
		{
			// Can't dodge
		}
		else
		{
			float fRand = BOSS_RAND_F(1.0f);
			if (fRand <= fDodgeProbability)
			{
				// Bit messy, need to const cast to get access to the bosses attack component
				CAttackComponent* pobAttack = const_cast<CAttackComponent*>(pobStrike->GetTargetP()->GetAttackComponent());
				// Manually complete the recovery so that after our dodge all's well with the combat state system
				pobAttack->CompleteRecovery();

				m_bNeedsDodge = true;
			}
		}
	}
}

void BossAttackPhaseState::NotifyIsInFailedStrikeRecovery(bool bFailedStrike)
{
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyIsInFailedStrikeRecovery(bFailedStrike);
}

void BossAttackPhaseState::NotifyInteractionWith(CEntity* pobEntity)
{	
	// Delegate down to special attacks
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyInteractionWith(pobEntity);
}

void BossAttackPhaseState::NotifyAttackStarted(Boss* pobBoss)
{
	// Delegate down to attacks
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyAttackStarted();
}

void BossAttackPhaseState::NotifyMovementDone(Boss* pobBoss)
{
	if (m_pobCurrentMovement)
	{
		m_pobCurrentMovement->NotifyMovementDone();

		// Need to send this in case we were in a combat state
		Message obMovementMessage(msg_combat_recovered);
		pobBoss->GetMovement()->SetCompletionMessage(obMovementMessage,pobBoss);
	}

	// Delegate down to attacks
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyMovementDone();
}

void BossAttackPhaseState::NotifyAttackFinished(Boss* pobBoss)
{
	// Delegate down to attacks
	if (m_pobCurrentAttack)
	{
		if (m_pobCurrentMovement)
		{
			ntError( 0 );
		}

		ntPrintf("%s finished an attack.\n", pobBoss->GetName().c_str() );
		m_pobCurrentAttack->NotifyAttackFinished();
	}
}

void BossAttackPhaseState::NotifyAttackAutoLinked(Boss* pobBoss)
{
	// Delegate down to attacks
	if (m_pobCurrentAttack)
		m_pobCurrentAttack->NotifyAttackAutoLinked();
}

void BossAttackPhaseState::NotifyAttackInterrupted(Boss* pobBoss)
{
	// Delegate down to attacks
	if (m_pobCurrentAttack)
		m_pobCurrentAttack = m_pobCurrentAttack->NotifyAttackInterrupted();
}

void BossAttackPhaseState::NotifyGotStruck(Boss* pobBoss)
{    
	NotifyAttackInterrupted( pobBoss );
	m_bGotStruck = true;

	// Don't need to delegate down to current attack because if we got struck then the attack would be interrupted
}

void BossAttackPhaseState::NotifyDeflected(Boss* pobBoss)
{
	NotifyAttackInterrupted( pobBoss );
	// Something to do with likelyhood of further blocking?
}

// This should only get called from our REACTSTATE, which implies we were doing something in DEFAULTSTATE and we were interrupted
void BossAttackPhaseState::NotifyRecovered(Boss* pobBoss)
{
	// If we had a movement that didn't finish
	if (m_pobCurrentMovement)
	{
		//ntError( m_pobCurrentMovement == m_pobCurrentMovementNotDone );

		CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
		ntError( pobPlayer );
		m_pobCurrentMovement = m_pobCurrentMovement->Reinitialise(pobBoss,pobPlayer);
	}

	// Could do something with attacks here? NotifyAttackInterrupted should have got the attack to either clear itself or prepare itself to be interrupted, if the pointer is still around here we could reinit it?
}

BossSimpleNavigationManager::BossSimpleNavigationManager(ntstd::List<BossSimpleNavigationAvoidanceArea*>& obAvoidanceAreas)
: m_obAvoidanceAreas(obAvoidanceAreas)
{
}

BossSimpleNavigationManager::~BossSimpleNavigationManager()
{
}

bool BossSimpleNavigationManager::IsPointInAvoidanceArea(CPoint& obPoint)
{
	if (m_obAvoidanceAreas.size() == 0)
		return false;

	for (ntstd::List<BossSimpleNavigationAvoidanceArea*>::iterator obIt = m_obAvoidanceAreas.begin();
		obIt != m_obAvoidanceAreas.end();
		obIt++)
	{
		CDirection obVector = CDirection( (*obIt)->m_obCentre - obPoint );
		if (obVector.Length() < (*obIt)->m_fRadius)
		{
			return true;
		}
	}

	return false;
}

bool BossSimpleNavigationManager::AugmentPath(CPoint* paobPoints, int iNumPoints)
{
	if (m_obAvoidanceAreas.size() == 0)
		return false;

	bool bRet = false;
	for (int i = 0; i < iNumPoints; ++i)
	{
		for (ntstd::List<BossSimpleNavigationAvoidanceArea*>::iterator obIt = m_obAvoidanceAreas.begin();
			obIt != m_obAvoidanceAreas.end();
			obIt++)
		{
			CDirection obVector = CDirection( (*obIt)->m_obCentre - paobPoints[i] );
			if (obVector.Length() < (*obIt)->m_fRadius)
			{
				obVector *= (*obIt)->m_fRadius;
				paobPoints[i] = (*obIt)->m_obCentre + obVector;
				bRet = true;
			}
		}
	}

	return bRet;
}

bool BossSimpleNavigationManager::AugmentVector(const CPoint& obCurrentPosition, CDirection& obTravelVector, float& fSpeed, float fLookAhead,
												bool bChangeSpeed)
{
	if (m_obAvoidanceAreas.size() == 0)
		return false;

	CPoint obNextPosition = obCurrentPosition + ( obTravelVector * fLookAhead );
//	g_VisualDebug->RenderPoint(obNextPosition,10.0f,DC_RED);

	for (ntstd::List<BossSimpleNavigationAvoidanceArea*>::iterator obIt = m_obAvoidanceAreas.begin();
		obIt != m_obAvoidanceAreas.end();
		obIt++)
	{
		// Get a couple of useful vectors
		CDirection obVectorToCurrent = CDirection( (*obIt)->m_obCentre - obCurrentPosition );
		CDirection obVectorToNext = CDirection( (*obIt)->m_obCentre - obNextPosition );
		// And their normalised equivalents
		CDirection obVectorToNextNorm = obVectorToNext;
		obVectorToNextNorm.Normalise();
		CDirection obVectorToCurrentNorm = obVectorToCurrent;
		obVectorToCurrentNorm.Normalise();

		// Get the 2 points on the circluar volume that are on perpendicular vectors to the current vector
		CDirection obPerp = obVectorToCurrent.Cross( CDirection( 0.0f, 1.0f, 0.0f ) );
		obPerp.Normalise();
		obPerp *= (*obIt)->m_fRadius;
		CPoint obPerp1 = (*obIt)->m_obCentre + obPerp;
		CPoint obPerp2 = (*obIt)->m_obCentre - obPerp;
		// Get the vectors to each of these points from the current position
		CDirection obVectorToPerp1( obPerp1 - obCurrentPosition);
		CDirection obVectorToPerp2( obPerp2 - obCurrentPosition);
		obVectorToPerp1.Normalise();
		obVectorToPerp2.Normalise();
		// What is the angle between them?
		float fAngleCoverageForThis = facosf(obVectorToPerp1.Dot(obVectorToPerp2));
		CDirection obPlayerToThis = obVectorToCurrent;
		obPlayerToThis.Normalise(); 
		// What is the angle between position now and where travel is wanted
		float fAngleToTravelVector = facosf(obPlayerToThis.Dot(obTravelVector));
		// If I'm inside the volume, move opposite way *now*
		if (obVectorToCurrent.Length() < (*obIt)->m_fRadius)
		{
//			g_VisualDebug->RenderLine(obCurrentPosition,obCurrentPosition+obTravelVector,DC_RED);
			obTravelVector = obVectorToCurrent * -1;
			obTravelVector.Normalise();
//			g_VisualDebug->RenderLine(obCurrentPosition,obCurrentPosition+obTravelVector,DC_GREEN);

			return true;
		}
		// If I'm heading towards it - and the next point is either in the volume or past the volume - push vector away a bit
		else if ( fAngleToTravelVector <= fAngleCoverageForThis*0.5f && ( obVectorToNext.Length() < (*obIt)->m_fRadius || obVectorToNextNorm.Dot(obVectorToCurrentNorm) < 0.0f ))
		{
//			g_VisualDebug->RenderLine(obCurrentPosition,obCurrentPosition+obTravelVector,DC_RED);

			// Unneccessary fancy way to construct a vector to avoid
			/*CDirection obAway(CONSTRUCT_CLEAR);
			CDirection obCentreToPerp1( obPerp1 - (*obIt)->m_obCentre);
			CDirection obCentreToPerp2( obPerp2 - (*obIt)->m_obCentre);
			if (obCentreToPerp1.Dot(obTravelVector) < 0.0f)
				obAway = obCentreToPerp2;
			else
				obAway = obCentreToPerp1;*/

			float fLength = obVectorToCurrent.Length();
			obVectorToCurrent.Normalise();
			//g_VisualDebug->RenderLine(obCurrentPosition,obCurrentPosition+obVectorToCurrent * -1,DC_YELLOW);
			float fProximity = 1.0f - (fLength / fLookAhead);
			obTravelVector = CDirection::Lerp(obTravelVector,obVectorToCurrent * -1,fProximity); // Lerp between directly away and our current travel to get away vector...
			if(bChangeSpeed)
			{
				fProximity < 0.1f ? fSpeed = 0.25f : fSpeed = fProximity; // ...combined with slowing the speed of the movement this works quite nicely
			}
			
			obTravelVector.Normalise();

//			g_VisualDebug->RenderLine(obCurrentPosition,obCurrentPosition+obTravelVector,DC_GREEN);

			return true;
		}
	}

	return false;
}

void BossSimpleNavigationManager::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	for (ntstd::List<BossSimpleNavigationAvoidanceArea*>::iterator obIt = m_obAvoidanceAreas.begin();
		obIt != m_obAvoidanceAreas.end();
		obIt++)
	{
		g_VisualDebug->RenderPoint( (*obIt)->m_obCentre, 10.0f, DC_RED );
		CMatrix obMtx(CONSTRUCT_IDENTITY);
		obMtx.SetTranslation( (*obIt)->m_obCentre );
		obMtx.SetTranslation( CPoint( obMtx.GetTranslation().X(), obMtx.GetTranslation().Y() + 0.1f, obMtx.GetTranslation().Z() ) );
		g_VisualDebug->RenderArc( obMtx, (*obIt)->m_fRadius, 360.0f, DC_RED );
	}
#endif
}

//--------------------------------------------------
//!
//! GeneralBossAttackSelector 
//!
//--------------------------------------------------
float GeneralBossAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	bool bSatisfied = true;
	for (	ntstd::List<GeneralBossAttackSelectorCondition*>::iterator obIt = m_obConditions.begin();
			obIt != m_obConditions.end();
			obIt++ )
	{
		bSatisfied &= (*obIt)->IsSatisfied( fTimeDelta, pobBoss, pobPlayer );

		if (!bSatisfied)
			break;
	}

	if (bSatisfied) 
		return m_fMaxPriority; 
	else
		return m_fMinPriority;
}

//--------------------------------------------------
//!
//! GeneralBossAttackSelector 
//! Class to select an attack randomly
//!
//--------------------------------------------------
void GeneralBossAttackSelector::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	for (	ntstd::List<GeneralBossAttackSelectorCondition*>::iterator obIt = m_obConditions.begin();
			obIt != m_obConditions.end();
			obIt++ )
	{
		(*obIt)->Initialise( pobBoss, pobPlayer );
	}
}

//--------------------------------------------------
//!
//! GeneralBossAttackSelector::BeginAttack 
//! Randomly selects an attack but gives each attack in the selector it's own weighting.
//!
//--------------------------------------------------
BossAttack* GeneralBossAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	if (m_obAttacks.size() > 0)
	{
		//If we've selected this selector, then go through and reset any conditions that need it.
		for(ntstd::List<GeneralBossAttackSelectorCondition*>::iterator obIt = m_obConditions.begin();
			obIt != m_obConditions.end() ; obIt++)
		{
			(*obIt)->Reset();
		}

		return ChooseRandomAttackWithWeighting(pobBoss, pobPlayer);
	}

	return 0;
}

bool GeneralBossAttackSelectorPlayerCombatStateCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	bool bIsState = (pobPlayer->GetAttackComponent()->AI_Access_GetState() == m_eRequiredCombatState);

	return (m_bNegateCheck == true) ? !bIsState : bIsState;
}

bool GeneralBossAttackSelectorPlayerDistanceCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	CDirection obBossToPlayer( pobPlayer->GetPosition() - pobBoss->GetPosition() );
	if (obBossToPlayer.LengthSquared() < m_fOuterDistance*m_fOuterDistance &&
		obBossToPlayer.LengthSquared() > m_fInnerDistance*m_fInnerDistance)
		return true;
	else
		return false;
}

bool GeneralBossAttackSelectorPlayerTimeSinceLastAttackCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	float fGracePeriod = m_fGracePeriod;
	fGracePeriod -= BOSS_RAND_F(m_fGracePeriodAdjust);

	if (pobBoss->GetTimeSinceLastPlayerStrike() > fGracePeriod)
		return true;
	else
		return false;
}

bool GeneralBossAttackSelectorBossTimeSinceLastAttackCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	float fGracePeriod = m_fGracePeriod;
	fGracePeriod -= BOSS_RAND_F(m_fGracePeriodAdjust);

	if (pobBoss->GetTimeSinceBossLastAttacked() > fGracePeriod)
		return true;
	else
		return false;
}

bool GeneralBossAttackSelectorBossDistanceFromStaticGeometryBox::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	//Get the y-rotation of the boss (which will be the rotation for our box phantom).
	float fBossXRot, fBossYRot, fBossZRot;
	CCamUtil::EulerFromMat_XYZ(pobBoss->GetMatrix(), fBossXRot, fBossYRot, fBossZRot);

	//Setup the box extents.
	CPoint obBoxHalfExtents(m_fWidth, 0.6f, m_fLength);

	//Set up the point that will be the centre of this box (raised up off the ground so as to not collide with it) offset relative to the boss.
	CPoint obCheckPosition = m_obOffset * pobBoss->GetMatrix();
	obCheckPosition.Y() += 0.9f;	//This box will go from 0.3f to 1.5f on the y-axis for the boss.

	bool bNoIntersection = false;	//False by default.
	float fFinalRotation = fBossYRot + (float)(m_fRotationY * (float)(3.14159265358979323846f / 180.0f));	//Keep it in radians.

	//If we're not intersecting any geometry within this box, then return true.
	if(!Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obCheckPosition, obBoxHalfExtents, fFinalRotation))
	{
		bNoIntersection = true;
	}

#ifndef _RELEASE
	//Draw the box so we can see the check. Render in a different colour if there was an intersection than if not.
	CQuat obDebugRenderOBBOrientation(CDirection(0.0f, 1.0f, 0.0f), fFinalRotation);
	g_VisualDebug->RenderOBB(obDebugRenderOBBOrientation, obCheckPosition, CDirection(obBoxHalfExtents),
		(bNoIntersection) ? 0xffffffff : 0xffff0000, DPF_WIREFRAME);
#endif

	//Return whether or not we've intersected the geometry with the test phantom.
	return bNoIntersection;
}

bool GeneralBossMovementSelectorAGenInAir::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	if (pobBoss->GetBossType() != Boss::BT_AERIAL_GENERAL)
		return false;

	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
	
	return pobAGen->IsInAir() == m_bRequiredInAirState;
}

bool GeneralBossAttackSelectorMinTimeSinceLastSelected::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	m_fTimeSinceLastSelected += fTimeDelta;

	if(m_fTimeSinceLastSelected >= m_fMinTimeBetweenSelection)
	{
		return true;
	}

	return false;
}


void GeneralBossAttackSelectorMinTimeSinceLastSelected::Reset()
{
	//Recalculate the new timer value.
	float fRange = m_fMinTimeBetweenSelectionUpper - m_fMinTimeBetweenSelectionLower;
	m_fMinTimeBetweenSelection = BOSS_RAND_F(fRange) + m_fMinTimeBetweenSelectionLower;
	//Reset how long since this was last selected.
	m_fTimeSinceLastSelected = 0.0f;
}

bool GeneralBossAttackSelectorAGenInAir::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	if (pobBoss->GetBossType() != Boss::BT_AERIAL_GENERAL)
		return false;

	AerialGeneral* pobAGen = (AerialGeneral*)pobBoss;
	
	return pobAGen->IsInAir() == m_bRequiredInAirState;
}

//--------------------------------------------------
//!
//! GeneralBossAttackSelector 
//!
//--------------------------------------------------
float GeneralBossMovementSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	bool bSatisfied = true;
	for (	ntstd::List<GeneralBossMovementSelectorCondition*>::iterator obIt = m_obConditions.begin();
			obIt != m_obConditions.end();
			obIt++ )
	{
		bSatisfied &= (*obIt)->IsSatisfied( fTimeDelta, pobBoss, pobPlayer );

		if (!bSatisfied)
			break;
	}

	if (bSatisfied) 
		return m_fMaxPriority; 
	else
		return m_fMinPriority;
}

BossMovement* GeneralBossMovementSelector::GetSelectedMovement()
{
	if (m_obMovements.size() > 0)
	{
		return ChooseRandomMovementWithWeighting();
	}

	return 0;
}

bool GeneralBossMovementSelectorPlayerCombatStateCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	bool bIsState = (pobPlayer->GetAttackComponent()->AI_Access_GetState() == m_eRequiredCombatState);

	return (m_bNegateCheck == true) ? !bIsState : bIsState;
}

bool GeneralBossMovementSelectorPlayerDistanceCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	CDirection obBossToPlayer( pobPlayer->GetPosition() - pobBoss->GetPosition() );
	if (obBossToPlayer.LengthSquared() < m_fOuterDistance*m_fOuterDistance &&
		obBossToPlayer.LengthSquared() > m_fInnerDistance*m_fInnerDistance)
		return true;
	else
		return false;
}

bool GeneralBossMovementSelectorPlayerTimeSinceLastAttackCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	float fGracePeriod = m_fGracePeriod;
	fGracePeriod -= BOSS_RAND_F(m_fGracePeriodAdjust);

	if (pobBoss->GetTimeSinceLastPlayerStrike() > fGracePeriod)
		return true;
	else
		return false;
}

bool GeneralBossMovementSelectorBossTimeSinceLastAttackCondition::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	float fGracePeriod = m_fGracePeriod;
	fGracePeriod -= BOSS_RAND_F(m_fGracePeriodAdjust);

	if (pobBoss->GetTimeSinceBossLastAttacked() > fGracePeriod)
		return true;
	else
		return false;
}
