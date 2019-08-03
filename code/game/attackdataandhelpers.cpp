/***************************************************************************************************
*
*	Attack data and helper classes
*
*	CHANGES
*
*	20/03/06	Duncan	Created from mammoth attacks.cpp
*
***************************************************************************************************/

// Necessary includes
#include "game/attacks.h" 
#include "core/visualdebugger.h" 

#include "core/osddisplay.h"

#include "game/hitcounter.h"
#include "game/awareness.h"
#include "game/movement.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/query.h"
#include "game/special.h"
#include "game/renderablecomponent.h"
#include "audio/gameaudiocomponents.h"
#include "game/messagehandler.h"
#include "game/attackdebugger.h"
#include "camera/coolcam_maya.h"
#include "camera/coolcam_aerialcombo.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camtrans_poirot.h"

#include "game/syncdcombat.h"
#include "anim/hierarchy.h"
#include "core/exportstruct_anim.h"

#include "physics/advancedcharactercontroller.h"
#include "physics/system.h"
#include "physics/world.h"

// For the syncronised movement
#include "syncdmovement.h"
#include "relativetransitions.h"
#include "continuationtransition.h"
#include "anim/animator.h"

//
//// this should go as soon as we have a good way of requesting announcements
#include "game/aicomponent.h"
#include "ai/aiformationcomponent.h"
#include "ai/aiformationattack.h"
//#include "aistates.h"
//
//// Networking
#include "jamnet/netman.h"
//
//// Combat Cameras
#include "game/attackcameras.h"
#include "objectdatabase/dataobject.h"
//
//// effects
#include "effect/combateffects_trigger.h"
#include "effect/combateffect.h"
#include "effect/effect_shims.h"
#include "effect/effect_manager.h"

#include "lua_enum_list.h"
// To get some data properties off the entity
#include "game/luaattrtable.h"

#include "game/randmanager.h"

#include "camera/timescalarcurve.h"

#include "physics/shapephantom.h"

// For some printf debug spew
extern char* g_apcCombatStateTable[]; // Get from attack debugger
extern char* g_apcClassNameTable[];

START_CHUNKED_INTERFACE	(SpecificAttackVulnerabilityZone, Mem::MC_ENTITY)
	IFLOAT	(SpecificAttackVulnerabilityZone, InnerDistance)
	IFLOAT	(SpecificAttackVulnerabilityZone, OuterDistance)
	IFLOAT	(SpecificAttackVulnerabilityZone, ZoneAngle)
	IFLOAT	(SpecificAttackVulnerabilityZone, ZoneSweep)
	IREFERENCE	(SpecificAttackVulnerabilityZone, SpecificAttack)
	IBOOL	(SpecificAttackVulnerabilityZone, RemoveIfUsedSuccessfully)
END_STD_INTERFACE

START_CHUNKED_INTERFACE	( ReactionAnimList, Mem::MC_ENTITY )
	ISTRING		( ReactionAnimList, Animation[RA_SPEED_HIGH_LEFT] )
	ISTRING		( ReactionAnimList, Animation[RA_SPEED_HIGH_RIGHT] )
	ISTRING		( ReactionAnimList, Animation[RA_SPEED_LOW_LEFT] )
	ISTRING		( ReactionAnimList, Animation[RA_SPEED_LOW_RIGHT] )
	ISTRING		( ReactionAnimList, Animation[RA_SPEED_UP] )
	ISTRING		( ReactionAnimList, Animation[RA_SPEED_DOWN] )
	ISTRING		( ReactionAnimList, Animation[RA_POWER_HIGH_LEFT] )
	ISTRING		( ReactionAnimList, Animation[RA_POWER_HIGH_RIGHT] )
	ISTRING		( ReactionAnimList, Animation[RA_POWER_LOW_LEFT] )
	ISTRING		( ReactionAnimList, Animation[RA_POWER_LOW_RIGHT] )
	ISTRING		( ReactionAnimList, Animation[RA_POWER_UP] )
	ISTRING		( ReactionAnimList, Animation[RA_POWER_DOWN] )
	ISTRING		( ReactionAnimList, Animation[RA_RANGE_HIGH_LEFT] )
	ISTRING		( ReactionAnimList, Animation[RA_RANGE_HIGH_RIGHT] )
	ISTRING		( ReactionAnimList, Animation[RA_RANGE_LOW_LEFT] )
	ISTRING		( ReactionAnimList, Animation[RA_RANGE_LOW_RIGHT] )
	ISTRING		( ReactionAnimList, Animation[RA_RANGE_UP] )
	ISTRING		( ReactionAnimList, Animation[RA_RANGE_DOWN] )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	( CClusterStructure, Mem::MC_ENTITY )
	IREFERENCE	( CClusterStructure, LeadCluster )
	IREFERENCE	( CClusterStructure, GroundClusterFront )
	IREFERENCE	( CClusterStructure, GroundClusterBack )
	IREFERENCE	( CClusterStructure, InstantKORecoverAttackCluster )
	IREFERENCE	( CClusterStructure, InterceptCluster )
	IREFERENCE	( CClusterStructure, RisingCluster )
	
	PUBLISH_PTR_CONTAINER_AS( m_aobSuperStyleLevelLinks[0], SuperStyleLevelCluster[HL_ONE] );
	PUBLISH_PTR_CONTAINER_AS( m_aobSuperStyleLevelLinks[1], SuperStyleLevelCluster[HL_TWO] );
	PUBLISH_PTR_CONTAINER_AS( m_aobSuperStyleLevelLinks[2], SuperStyleLevelCluster[HL_THREE] );

	PUBLISH_PTR_CONTAINER_AS( m_aobOnTheSpotStyleLevelLinks[0], OnTheSpotStyleLevelLinks[HL_ONE] );
	PUBLISH_PTR_CONTAINER_AS( m_aobOnTheSpotStyleLevelLinks[1], OnTheSpotStyleLevelLinks[HL_TWO] );
	PUBLISH_PTR_CONTAINER_AS( m_aobOnTheSpotStyleLevelLinks[2], OnTheSpotStyleLevelLinks[HL_THREE] );

	IREFERENCE	( CClusterStructure, StyleLevelSpecialCluster )
	IREFERENCE	( CClusterStructure, ShortRangeCluster )
	IREFERENCE	( CClusterStructure, MediumRangeCluster )
	IREFERENCE	( CClusterStructure, LongRangeCluster )
	IREFERENCE	( CClusterStructure, BlockedGrab )
	IREFERENCE	( CClusterStructure, OpenFreeEvades )
	IREFERENCE	( CClusterStructure, ComboFreeEvades )

	PUBLISH_PTR_AS(m_pobOpenSkillEvades, OpenSkillEvades)
	PUBLISH_PTR_AS(m_pobCloseSkillEvades, CloseSkillEvades)
	PUBLISH_PTR_AS(m_pobOpenSuperSkillEvades, OpenSuperSkillEvades)
	PUBLISH_PTR_AS(m_pobCloseSuperSkillEvades, CloseSuperSkillEvades)

	PUBLISH_PTR_AS( m_pobSpecificCounterCollection, SpecificCounterCollection)

	PUBLISH_PTR_AS(m_pobLedgeRecoverAttack, LedgeRecoverAttack)
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAttackLink, Mem::MC_ENTITY)
	PUBLISH_PTR_AS	(m_pobAttackData, AttackData)
	
	PUBLISH_PTR_AS(m_pobLinks[AM_SPEED_FAST], SpeedFast)
	PUBLISH_PTR_AS(m_pobLinks[AM_SPEED_MEDIUM], SpeedMedium)
	PUBLISH_PTR_AS(m_pobLinks[AM_SPEED_GRAB], SpeedGrab)
	PUBLISH_PTR_AS(m_pobLinks[AM_POWER_FAST], PowerFast)
	PUBLISH_PTR_AS(m_pobLinks[AM_POWER_MEDIUM], PowerMedium)
	PUBLISH_PTR_AS(m_pobLinks[AM_POWER_GRAB], PowerGrab)
	PUBLISH_PTR_AS(m_pobLinks[AM_RANGE_FAST], RangeFast)
	PUBLISH_PTR_AS(m_pobLinks[AM_RANGE_MEDIUM], RangeMedium)
	PUBLISH_PTR_AS(m_pobLinks[AM_RANGE_GRAB], RangeGrab)
	PUBLISH_PTR_AS(m_pobLinks[AM_ACTION], Action)

	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_SPEED_FAST], MashSpeedFast)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_SPEED_MEDIUM], MashSpeedMedium)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_SPEED_GRAB], MashSpeedGrab)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_POWER_FAST], MashPowerFast)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_POWER_MEDIUM], MashPowerMedium)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_POWER_GRAB], MashPowerGrab)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_RANGE_FAST], MashRangeFast)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_RANGE_MEDIUM], MashRangeMedium)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_RANGE_GRAB], MashRangeGrab)
	PUBLISH_PTR_AS(m_pobLinks[AM_MASH_ACTION], MashAction)

	PUBLISH_PTR_AS(m_pobLinks[AM_DODGE_LEFT], DodgeLeft)
	PUBLISH_PTR_AS(m_pobLinks[AM_DODGE_RIGHT], DodgeRight)
	PUBLISH_PTR_AS(m_pobLinks[AM_DODGE_FORWARD], DodgeForward)
	PUBLISH_PTR_AS(m_pobLinks[AM_DODGE_BACK], DodgeBack)

	PUBLISH_PTR_AS(m_pobButtonHeldAttack, ButtonHeldAttack)
END_STD_INTERFACE

// Declare editable interface for CAttackData
START_CHUNKED_INTERFACE	(CAttackData, Mem::MC_ENTITY)
	IENUM		(CAttackData, AttackClass, ATTACK_CLASS)
	ISTRING		(CAttackData, AttackAnimName)
	ISTRING		(CAttackData, RecoveryAnimName)
	ISTRING		(CAttackData, StrikeFailedRecoveryAnimName)
	IFLOAT		(CAttackData, AttackTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fAttackRecoverTime, 0.0f, AttackRecoverTime)
	IFLOAT		(CAttackData, StrikeProximityCheckDistance)
	IFLOAT		(CAttackData, StrikeProximityCheckExclusionDistance)
	IFLOAT		(CAttackData, MaxDistance)
	IFLOAT		(CAttackData, AttackScaledOffset )
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fStrikeProximityCheckAngle, 0.0f, StrikeProximityCheckAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fStrikeProximityCheckSweep, 90.0f, StrikeProximityCheckSweep)
	IFLOAT		(CAttackData, Strike2ProximityCheckAngle)
	IFLOAT		(CAttackData, Strike2ProximityCheckAngleCoverage)
	IFLOAT		(CAttackData, Strike2ProximityCheckSweep)		
	//IINT		(CAttackData, RequiredSwordLevel)
	IFLIPFLOP	(CAttackData, InvulnerabilityWindow)
	IFLIPFLOP	(CAttackData, Strike)
	IFLIPFLOP	(CAttackData, Strike2)
	IFLIPFLOP	(CAttackData, AttackPopOut)
	IFLIPFLOP	(CAttackData, BlockPopOut)
	IFLIPFLOP	(CAttackData, NoLockWindows)
	IFLIPFLOP	(CAttackData, MovementPopOut)
	IFLIPFLOP	(CAttackData, UninterruptibleWindow)
	IFLIPFLOP	(CAttackData, NoCollideWindows)
	IFLIPFLOP	(CAttackData, NextMove)
	IENUM_d		(CAttackData, HintInNextMove, VIRTUAL_BUTTON_TYPE, AB_NUM)	
	IFLIPFLOP	(CAttackData, RangeInterceptWindow)
	IFLIPFLOP	(CAttackData, InterceptWindow)
	IFLIPFLOP	(CAttackData, EvadeAndGrabDenyWindow)
	IINT		(CAttackData, Damage)
	IINT		(CAttackData, Damage2)
	IFLOAT		(CAttackData, RecoilTime)
	IFLOAT		(CAttackData, DeflectionTime)
	IENUM		(CAttackData, ReactionAppearance, REACTION_APPEARANCE)		
	IENUM		(CAttackData, UnBlockedReaction, REACTION_TYPE)
	IENUM		(CAttackData, SpeedBlockedReaction, REACTION_TYPE)
	IENUM		(CAttackData, RangeBlockedReaction, REACTION_TYPE)
	IENUM		(CAttackData, PowerBlockedReaction, REACTION_TYPE)
	IENUM		(CAttackData, SyncdSecondaryReaction, REACTION_TYPE)
	IENUM		(CAttackData, UnBlockedReactionFinalStrikeOverride, REACTION_TYPE)
	ISTRING		(CAttackData, ReceiverAnim)
	ISTRING		(CAttackData, ReceiverAnimSpeedBlocked)
	ISTRING		(CAttackData, ReceiverAnimRangeBlocked)
	ISTRING		(CAttackData, ReceiverAnimPowerBlocked)
	ISTRING		(CAttackData, ReceiverAnimSyncdSecondaryReaction)
	ISTRING		(CAttackData, SpecificKOAnimation)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUseProceduralKO, false, UseProceeduralKO)
	PUBLISH_VAR_AS(m_bOverrideInvulnerabilityWindow, OverrideInvulnerabilityWindow)
	PUBLISH_VAR_AS(m_bTurnOffGravity, TurnOffGravity)
	IBOOL		(CAttackData, AftertouchableKO)
	IBOOL		(CAttackData, AerialComboStartKO)
	IBOOL		(CAttackData, JuggleKO)
	IBOOL		(CAttackData, HoldPreviousTarget)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bExemptPreviousTarget, false, ExemptPreviousTarget )
	IBOOL		(CAttackData, AutoLink)	
	ISTRING		(CAttackData, SyncReceiverAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bSkipFlooredAndRise, false, SkipFlooredAndRise )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bSkipFallingMovement, false, SkipFallingMovement )
	PUBLISH_VAR_AS(	m_bInteractWithSyncTransform, InteractWithSyncTransform)
	PUBLISH_VAR_AS(	m_obInteractiveSyncTranslateAbsoluteSpeed, InteractiveSyncTranslateAbsoluteSpeed)
	PUBLISH_VAR_AS(	m_obMaxInteractiveSyncTranslateSpeed, MaxInteractiveSyncTranslateSpeed)
	PUBLISH_VAR_AS(	m_obInteractiveSyncRotationAbsoluteSpeed,InteractiveSyncRotationAbsoluteSpeed)
	PUBLISH_VAR_AS(	m_obMaxInteractiveSyncRotationSpeed, MaxInteractiveSyncRotationSpeed)
	PUBLISH_VAR_AS(	m_bReverseInteractiveStickInput, ReverseInteractiveStickInput)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fInteractiveCollisionCheckDistance, 3.0f, InteractiveCollisionCheckDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fInteractiveCollisionCheckStartHeight, 0.3f, InteractiveCollisionCheckStartHeight)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iInteractiveCollisionCheckHeightCount, 5, InteractiveCollisionCheckHeightCount)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fInteractiveCollisionCheckHeightInterval, 0.4f, InteractiveCollisionCheckHeightInterval)
	IENUM		(CAttackData, SyncTransform, COMBAT_REL_MOVE_TYPE)
	PUBLISH_VAR_AS( m_obSyncToLocatorName, SyncToLocatorName )
	IBOOL		(CAttackData, NeedsSuperStyleSafety)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_bNeedsToBeInContinueVolume, false, NeedsNeedsToBeInContinueVolume)
	ISTRING		(CAttackData, SuperSafeRotationAnimAttacker)
	ISTRING		(CAttackData, SuperSafeRotationAnimReceiver)
	ISTRING		(CAttackData, SuperSafeTranslationAnimAttacker)
	ISTRING		(CAttackData, SuperSafeTranslationAnimReceiver)
	ISTRING		(CAttackData, SuperSafeLinkupAnimAttacker)
	ISTRING		(CAttackData, SuperSafeLinkupAnimReceiver)
	IFLIPFLOP	(CAttackData, VictimRagdollableWindow)
	IREFERENCE	(CAttackData, AttackCamera)
	IREFERENCE	(CAttackData, TSCurve)
	ISTRING		(CAttackData, EffectsScript)
	IREFERENCE	(CAttackData, UninterruptibleWindowDetails)
	IREFERENCE	(CAttackData, SpecificCounterIndex)	
	IREFERENCE	(CAttackData, SpecificKillCounterIndex)
	IREFERENCE	(CAttackData, SpecificSmallCounterIndex)	
	IREFERENCE	(CAttackData, SpecificSmallKillCounterIndex)
	IBOOL		(CAttackData, GroupCombatLockOnMove)
	IENUM		(CAttackData, TargetType, ATTACK_TARGET_TYPE)
	IENUM		(CAttackData, AttackMovementType, ATTACK_MOVEMENT_TYPE)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bNoAnimationBlend, false, NoAnimationBlend)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bTrackTargetThroughout, false, TrackTargetThroughout)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bCorrectDirectionWhenScaling, true, CorrectDirectionWhenScaling)
	PUBLISH_PTR_CONTAINER_AS(m_obStrikeVolumeDescriptors, StrikeVolumeDescriptors)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUse3DScaling, false, Use3DScaling)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bNoRotateIfTargetBehind, false, NoRotateIfTargetBehind)
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )
#endif
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAttackDefinition, Mem::MC_ENTITY)
	IREFERENCE	(CAttackDefinition, ClusterStructure)
	IFLOAT		(CAttackDefinition, StrikeToBlockFactor)
	IFLOAT		(CAttackDefinition, WiggleReductionFactor)
	IREFERENCE	(CAttackDefinition, SpeedBlock)
	IREFERENCE	(CAttackDefinition, PowerBlock)
	IREFERENCE	(CAttackDefinition, RangeBlock)
	ISTRING		(CAttackDefinition, RiseDeflectionAnim)
	IREFERENCE	(CAttackDefinition, PowerDeflections)
	IREFERENCE	(CAttackDefinition, SpeedDeflections)
	IREFERENCE	(CAttackDefinition, RangeDeflections)
	ISTRING		(CAttackDefinition, SpeedImpactStaggerRecoverAnim)
	ISTRING		(CAttackDefinition, RangeImpactStaggerRecoverAnim)
	ISTRING		(CAttackDefinition, PowerImpactStaggerRecoverAnim)
	IREFERENCE	(CAttackDefinition, PowerImpactStaggers)
	IREFERENCE	(CAttackDefinition, SpeedImpactStaggers)
	IREFERENCE	(CAttackDefinition, RangeImpactStaggers)
	ISTRING		(CAttackDefinition, SpeedBlockStaggerRecoverAnim)
	ISTRING		(CAttackDefinition, RangeBlockStaggerRecoverAnim)
	ISTRING		(CAttackDefinition, PowerBlockStaggerRecoverAnim)
	IREFERENCE	(CAttackDefinition, PowerBlockStaggers)
	IREFERENCE	(CAttackDefinition, SpeedBlockStaggers)
	IREFERENCE	(CAttackDefinition, RangeBlockStaggers)
	IFLOAT		(CAttackDefinition, BlockStaggerTime)
	IFLOAT		(CAttackDefinition, ImpactStaggerTime)
	IFLOAT		(CAttackDefinition, MinBlockStaggerTime)
	IFLOAT		(CAttackDefinition, MinImpactStaggerTime)
	IREFERENCE	(CAttackDefinition, PowerRecoilsFront)
	IREFERENCE	(CAttackDefinition, PowerRecoilsBack)
	IREFERENCE	(CAttackDefinition, SpeedRecoilsFront)
	IREFERENCE	(CAttackDefinition, SpeedRecoilsBack)
	IREFERENCE	(CAttackDefinition, RangeRecoilsFront)
	IREFERENCE	(CAttackDefinition, RangeRecoilsBack)
	IREFERENCE	(CAttackDefinition, PowerDeathsFront)
	IREFERENCE	(CAttackDefinition, PowerDeathsBack)
	IREFERENCE	(CAttackDefinition, SpeedDeathsFront)
	IREFERENCE	(CAttackDefinition, SpeedDeathsBack)
	IREFERENCE	(CAttackDefinition, RangeDeathsFront)
	IREFERENCE	(CAttackDefinition, RangeDeathsBack)
	ISTRING		(CAttackDefinition, FrontAirKOAnim)
	ISTRING		(CAttackDefinition, BackAirKOAnim)
	IREFERENCE	(CAttackDefinition, FrontKODefinition)
	IREFERENCE	(CAttackDefinition, BackKODefinition)
	IREFERENCE	(CAttackDefinition, PowerKOsFront)
	IREFERENCE	(CAttackDefinition, PowerKOsBack)
	IREFERENCE	(CAttackDefinition, SpeedKOsFront)
	IREFERENCE	(CAttackDefinition, SpeedKOsBack)
	IREFERENCE	(CAttackDefinition, RangeKOsFront)
	IREFERENCE	(CAttackDefinition, RangeKOsBack)
	IFLOAT		(CAttackDefinition, KOTime)
	IFLOAT		(CAttackDefinition, KOInstantRecoverTime)
	IREFERENCE	(CAttackDefinition, StanceSwitchingDef)
	IREFERENCE	(CAttackDefinition, AerialDetails)
	IREFERENCE	(CAttackDefinition, HitCounterDef)
	IREFERENCE	(CAttackDefinition, Cameras)
	IFLOAT		(CAttackDefinition, StrikeUpperHeight)
	IFLOAT		(CAttackDefinition, StrikeLowerHeight)
	IFLOAT		(CAttackDefinition, AerialStrikeUpperHeight)
	IFLOAT		(CAttackDefinition, AerialStrikeLowerHeight)
	IREFERENCE	(CAttackDefinition, SpecialDefinition )
	IFLOAT		(CAttackDefinition, BadCounterDetectTime)
	IFLOAT		(CAttackDefinition, BadCounterPunishTime)
	IFLOAT		(CAttackDefinition, CounterTime)
	IFLOAT		(CAttackDefinition, QuickCounterTime)
	IFLOAT		(CAttackDefinition, GrabProximityCheckSweep)
	IREFERENCE	(CAttackDefinition, ReactionMatrix)
	IFLOAT		(CAttackDefinition, DeflectionTimeOverrideSpeed)
	IFLOAT		(CAttackDefinition, DeflectionTimeOverrideRange)
	IFLOAT		(CAttackDefinition, DeflectionTimeOverridePower)
	IBOOL		(CAttackDefinition, CanAutoBlockGrabs)	
	IBOOL		(CAttackDefinition, CanAutoBlockPower)
	IBOOL		(CAttackDefinition, CanAutoBlockRange)
	IBOOL		(CAttackDefinition, CanAutoBlockSpeed)
	IFLOAT		(CAttackDefinition, HeldAttackThresholdSpeed)
	IFLOAT		(CAttackDefinition, HeldAttackThresholdRange)
	IFLOAT		(CAttackDefinition, HeldAttackThresholdPower)
	IENUM		(CAttackDefinition, StrikeProximityCheckExclusionDistanceReaction, REACTION_TYPE)
	ISTRING		(CAttackDefinition, StrikeProximityCheckExclusionDistanceReactionAnim)
	IBOOL		(CAttackDefinition, IsGroundAttackable)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bIsAerialable, true, m_bIsAerialable )
	IBOOL		(CAttackDefinition, ExemptFromRagdollKOs)
	IBOOL		(CAttackDefinition, OnlyHeadShots)
	PUBLISH_PTR_AS( m_pCombatEffectsDef, CombatEffectsDefinition )
	IREFERENCE	(CAttackDefinition, HCMultiplierDef)
	IREFERENCE	(CAttackDefinition, CombatPhysicsPushVolumesAttacking)
	IREFERENCE	(CAttackDefinition, CombatPhysicsPushVolumesNonAttacking)
	PUBLISH_PTR_CONTAINER_AS(m_obSpecificVulnerabilities, SpecificVulnerabilities)
	IFLOAT		(CAttackDefinition, JuggleTargetMemoryTime)
	PUBLISH_VAR_AS( m_obCounterAttackEnvironmentCheckHalfExtents, CounterAttackEnvironmentCheckHalfExtents )
	PUBLISH_PTR_AS( m_pobAttackMovementBlendInTimes, AttackMovementBlendInTimes )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fSkillEvadeRadius, 3.0f, SkillEvadeQueryRadius )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fSkillEvadeMargin, 0.7f, SkillEvadeMargin )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fSuperSkillEvadeMargin, 0.25f, SuperSkillEvadeMargin )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obSkillEvadeAttackEnvironmentCheckHalfExtents, CPoint(2.5f, 0.3f, 2.5f), SkillEvadeAttackEnvironmentCheckHalfExtents )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bCanLedgeRecover, false, CanLedgeRecover )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fLedgeRecoverRequiredHeight, 0.8f ,LedgeRecoverRequiredHeight )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAttackBlock, Mem::MC_ENTITY)
	ISTRING		(CAttackBlock, LoopAnimName) 
	ISTRING		(CAttackBlock, EnterAnimName)
	ISTRING		(CAttackBlock, ExitAnimName)
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAttackKO, Mem::MC_ENTITY)
	ISTRING		(CAttackKO, FallAnimName) 
	ISTRING		(CAttackKO, FlooredAnimName)
	ISTRING		(CAttackKO, WaitAnimName)
	ISTRING		(CAttackKO, RiseAnimName)
	ISTRING		(CAttackKO, IRecoverAnimName)
	ISTRING		(CAttackKO, GroundAttackRiseAnimName)
END_STD_INTERFACE

START_CHUNKED_INTERFACE	( StanceSwitchingDef, Mem::MC_ENTITY )
	ISTRING		( StanceSwitchingDef, SpeedToPowerAnim )
	ISTRING		( StanceSwitchingDef, SpeedToRangeAnim )
	ISTRING		( StanceSwitchingDef, RangeToSpeedAnim )
	ISTRING		( StanceSwitchingDef, RangeToPowerAnim )
	ISTRING		( StanceSwitchingDef, PowerToSpeedAnim )
	ISTRING		( StanceSwitchingDef, PowerToRangeAnim )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	( AerialDetailsDef, Mem::MC_ENTITY )
	ISTRING		( AerialDetailsDef, AerialWinDrop )
	ISTRING		( AerialDetailsDef, AerialWinLand )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obAerialGrappleTargetOffset, CDirection( 0.0f, -1.34f, -1.59f ), AerialGrappleTargetOffset )
END_STD_INTERFACE

START_CHUNKED_INTERFACE (SpecificCounterCollection, Mem::MC_ENTITY)
	PUBLISH_PTR_CONTAINER_AS( m_obSpecificCounters, SpecificCounters )
END_STD_INTERFACE

START_CHUNKED_INTERFACE (SkillEvadeCollection, Mem::MC_ENTITY)
	PUBLISH_PTR_CONTAINER_AS( m_obSkillEvades[0], FrontSkillEvades )
	PUBLISH_PTR_CONTAINER_AS( m_obSkillEvades[1], BackSkillEvades )
	PUBLISH_PTR_CONTAINER_AS( m_obSkillEvades[2], LeftSkillEvades )
	PUBLISH_PTR_CONTAINER_AS( m_obSkillEvades[3], RightSkillEvades )
END_STD_INTERFACE

START_CHUNKED_INTERFACE (SpecificCounterIndexEntry, Mem::MC_ENTITY)
	PUBLISH_VAR_AS( m_obSpecificCounterAttackName, SpecificCounterAttackName )
END_STD_INTERFACE

START_CHUNKED_INTERFACE (SpecificCounterIndex, Mem::MC_ENTITY)
	PUBLISH_PTR_CONTAINER_AS( m_obEntries, Entries )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(SpecificCounter, Mem::MC_ENTITY)
	ISTRING		(m_obSpecificCounterAttackName, SpecificCounterAttackName)
	IREFERENCE	(m_obSpecificCounterAttackLink, SpecificCounterAttackLink)

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(ReactionMatrixColumnEntry, Mem::MC_ENTITY)
	IENUM		(ReactionMatrixColumnEntry, Deflect,		REACTION_TYPE)
	IENUM		(ReactionMatrixColumnEntry, BlockStagger,	REACTION_TYPE)
	IENUM		(ReactionMatrixColumnEntry, ImpactStagger,	REACTION_TYPE)
	IENUM		(ReactionMatrixColumnEntry, Recoil,			REACTION_TYPE)
	IENUM		(ReactionMatrixColumnEntry, Held,			REACTION_TYPE)
	IENUM		(ReactionMatrixColumnEntry, KO,				REACTION_TYPE)
	IENUM		(ReactionMatrixColumnEntry, Kill,			REACTION_TYPE)
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(ReactionMatrixColumn, Mem::MC_ENTITY)
	IREFERENCE	(ReactionMatrixColumn, SpeedAttacks)
	IREFERENCE	(ReactionMatrixColumn, PowerAttacks)
	IREFERENCE	(ReactionMatrixColumn, RangeAttacks)
	IREFERENCE	(ReactionMatrixColumn, OtherAttacks)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(ReactionMatrix, Mem::MC_ENTITY)
	IREFERENCE	(ReactionMatrix, UnBlocked)
	IREFERENCE	(ReactionMatrix, SpeedBlocked)
	IREFERENCE	(ReactionMatrix, PowerBlocked)
	IREFERENCE	(ReactionMatrix, RangeBlocked)
	IREFERENCE	(ReactionMatrix, SyncdSecondary)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(UninterruptibleWindowDetails, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(m_bUninterruptibleForSpeedAttackMedium, ForSpeedAttackMedium)
	PUBLISH_VAR_AS(m_bUninterruptibleForRangeAttackMedium, ForRangeAttackMedium)
	PUBLISH_VAR_AS(m_bUninterruptibleForPowerAttackMedium, ForPowerAttackMedium)
	PUBLISH_VAR_AS(m_bUninterruptibleForSpeedAttackFast, ForSpeedAttackFast)
	PUBLISH_VAR_AS(m_bUninterruptibleForRangeAttackFast, ForRangeAttackFast)
	PUBLISH_VAR_AS(m_bUninterruptibleForPowerAttackFast, ForPowerAttackFast)
	PUBLISH_VAR_AS(m_bUninterruptibleForGrab, ForGrab)
END_STD_INTERFACE

START_CHUNKED_INTERFACE( CombatPhysicsPushVolumes, Mem::MC_ENTITY )
	PUBLISH_PTR_CONTAINER_AS(m_obSpeedPhysicsVolumes, SpeedPhysicsVolumes);
	PUBLISH_PTR_CONTAINER_AS(m_obRangePhysicsVolumes, RangePhysicsVolumes);
	PUBLISH_PTR_CONTAINER_AS(m_obPowerPhysicsVolumes, PowerPhysicsVolumes);
END_STD_INTERFACE

// Toggle auto block
bool CAttackComponent::m_bGlobablAutoBlockEnable = true;

// Some general combat settings
const float CAttackComponent::m_fAutoBlockLeadTime = 0.1f;
const float CAttackComponent::m_fAutoRiseTime = 3.0f;

/***************************************************************************************************
*
*	FUNCTION		SpecificAttackVulnerabilityZone::IsInZone
*
*	DESCRIPTION		Is an attacker in an owners zone of vulnerability?
*
***************************************************************************************************/
CAttackLink* SpecificAttackVulnerabilityZone::IsInZone(CEntity* pobZoneOwner, CEntity* pobAttacker)
{
	// Some zones should only be used once.
	if (m_bRemoveIfUsedSuccessfully && (m_iNumUses > 0))
		return 0;

	if (pobZoneOwner->GetAwarenessComponent()->IsEntityInZone(pobZoneOwner,pobAttacker,m_fZoneAngle,m_fZoneSweep,m_fInnerDistance,m_fOuterDistance))
		return m_pobSpecificAttack;
	else
		return 0;
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrix::ReactionMatrix
*
*	DESCRIPTION		Constructs a reaction matrix for reaction lookups.
*
***************************************************************************************************/
ReactionMatrix::ReactionMatrix() :
	m_pobUnBlocked ( NULL ),
	m_pobSpeedBlocked ( NULL ),
	m_pobPowerBlocked ( NULL ),
	m_pobRangeBlocked ( NULL ),
	m_pobSyncdSecondary ( NULL )
{
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrix::GetReactionMatrixColumn
*
*	DESCRIPTION		Return the correct ReactionMatrixColumn according to specified REACTION_MATRIX_LOOKUP 
*					type.
*
***************************************************************************************************/
ReactionMatrixColumn* ReactionMatrix::GetReactionMatrixColumn(REACTION_MATRIX_LOOKUP eReactionMatrixLookup)
{
	switch (eReactionMatrixLookup)
	{
		case RML_UNBLOCKED :		return m_pobUnBlocked;		break;
		case RML_SPEEDBLOCKED :		return m_pobSpeedBlocked;	break;
		case RML_POWERBLOCKED :		return m_pobPowerBlocked;	break;
		case RML_RANGEBLOCKED :		return m_pobRangeBlocked;	break;
		case RML_SYNCDSECONDARY :	return m_pobSyncdSecondary; break;
		default:
		{
			// Should never be here
			ntAssert( 0 );
			return 0;
		}
		break;
	}
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrix::PostConstruct
*
*	DESCRIPTION		Do some post construct type checks to avoid anything silly.
*
***************************************************************************************************/
void ReactionMatrix::PostConstruct()
{
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrix::EditorChangeValue
*
*	DESCRIPTION		Trigger a PostConstruct whenever a value is changed, to check types.
*
***************************************************************************************************/
bool ReactionMatrix::EditorChangeValue(CallBackParameter, CallBackParameter)
{
	PostConstruct();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrix::~ReactionMatrix
*
*	DESCRIPTION		Deconstructor
*
***************************************************************************************************/
ReactionMatrix::~ReactionMatrix()
{
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrixColumn::ReactionMatrixColumn
*
*	DESCRIPTION		Constructs a ReactionMatrixColumn entry in a ReactionMatrix
*
***************************************************************************************************/
ReactionMatrixColumn::ReactionMatrixColumn() :
	m_pobSpeedAttacks ( NULL ),
	m_pobPowerAttacks ( NULL ),
	m_pobRangeAttacks ( NULL ),
	m_pobOtherAttacks ( NULL )
{
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrixColumn::GetReactionMatrixColumnEntry
*
*	DESCRIPTION		Get the correct ReactionMatrixColumnEntry for the specified ATTACK_CLASS.
*
***************************************************************************************************/
ReactionMatrixColumnEntry* ReactionMatrixColumn::GetReactionMatrixColumnEntry(ATTACK_CLASS eAttackClass)
{
	switch (eAttackClass)
	{
		case AC_SPEED_MEDIUM:
		case AC_SPEED_FAST:		return m_pobSpeedAttacks;		break;
		case AC_POWER_MEDIUM:
		case AC_POWER_FAST:		return m_pobPowerAttacks;		break;
		case AC_RANGE_MEDIUM:
		case AC_RANGE_FAST:		return m_pobRangeAttacks;		break;
		default:				return m_pobOtherAttacks;		break;
	}
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrixColumn::PostConstruct
*
*	DESCRIPTION		Do some post construct type checks do avoid anything silly.
*
***************************************************************************************************/
void ReactionMatrixColumn::PostConstruct()
{
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrixColumn::EditorChangeValue
*
*	DESCRIPTION		Trigger a post construct type check.
*
***************************************************************************************************/
bool ReactionMatrixColumn::EditorChangeValue(CallBackParameter, CallBackParameter)
{
	PostConstruct();
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrixColumn::~ReactionMatrixColumn
*
*	DESCRIPTION		Deconstructor.
*
***************************************************************************************************/
ReactionMatrixColumn::~ReactionMatrixColumn()
{
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrixColumnEntry::ReactionMatrixColumnEntry
*
*	DESCRIPTION		Construct an entry in a ReactionMatrixColumn.
*
***************************************************************************************************/
ReactionMatrixColumnEntry::ReactionMatrixColumnEntry() :
	m_eDeflect ( RT_DEFLECT ),
	m_eBlockStagger ( RT_BLOCK_STAGGER ),
	m_eImpactStagger ( RT_IMPACT_STAGGER ),
	m_eRecoil ( RT_RECOIL ),
	m_eHeld( RT_HELD ),
	m_eKO ( RT_KO ),
	m_eKill ( RT_KILL )
{
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrixColumnEntry::GetReactionType
*
*	DESCRIPTION		Get the new REACTION_TYPE for the specified REACTION_TYPE.
*
***************************************************************************************************/
REACTION_TYPE ReactionMatrixColumnEntry::GetReactionType(REACTION_TYPE eReactionType)
{
	switch ( eReactionType )
	{
	case RT_DEFLECT:			return m_eDeflect;			break;
	case RT_BLOCK_STAGGER:		return m_eBlockStagger;		break;
	case RT_IMPACT_STAGGER:		return m_eImpactStagger;	break;
	case RT_RECOIL:				return m_eRecoil;			break;
	case RT_HELD:				return m_eHeld;				break;
	case RT_KO:					return m_eKO;				break;
	case RT_KILL:				return m_eKill;				break;
	default:	
		{
			// Should never be here
			ntAssert( 0 );
			return RT_COUNT;
		}
		break;
	}
}

/***************************************************************************************************
*
*	FUNCTION		ReactionMatrixColumnEntry::~ReactionMatrixColumnEntry
*
*	DESCRIPTION		Deconstructor.
*
***************************************************************************************************/
ReactionMatrixColumnEntry::~ReactionMatrixColumnEntry()
{
}


//------------------------------------------------------------------------------------------
//!
//!	StanceSwitchingDef::StanceSwitchingDef
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
StanceSwitchingDef::StanceSwitchingDef( void )
{
}


//------------------------------------------------------------------------------------------
//!
//!	AerialDetailsDef::AerialDetailsDef
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
AerialDetailsDef::AerialDetailsDef( void )
:	m_obAerialWinDrop(),
	m_obAerialWinLand()
{
}


//------------------------------------------------------------------------------------------
//!
//!	ReactionAnimList::ReactionAnimList
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
ReactionAnimList::ReactionAnimList( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		SpecificCounter::PostConstruct
*
*	DESCRIPTION		Post construct to make sure the data type being passed in is a CAttackLink,
*					if it's not it will raise a window warning the user.
*
***************************************************************************************************/
void SpecificCounter::PostConstruct()
{
	#ifdef _DEBUG
	
	if (m_pobSpecificCounterAttackLink)
	{
		DataObject* pobInterface = ObjectDatabase::Get().GetDataObjectFromPointer( (const void*) m_pobSpecificCounterAttackLink );
		if (pobInterface && strcmp( pobInterface->GetClassName(), "CAttackLink" ) != 0)
		{
			user_warn_msg(("Wrong data type passed into SpecificCounter! Must be a CAttackLink, NOT a CAttackData"));
		}
	}

	#endif
}

/***************************************************************************************************
*
*	FUNCTION		SpecificCounter::EditorChangeValue
*
*	DESCRIPTION		Checks to see if the new data being specified is a CAttackLink, and if it's not
*					raises a window warning the user.
*
***************************************************************************************************/
bool SpecificCounter::EditorChangeValue(CallBackParameter, CallBackParameter)
{
	#ifdef _DEBUG
	if (m_pobSpecificCounterAttackLink)
	{
	DataObject* pobInterface = ObjectDatabase::Get().GetDataObjectFromPointer( (const void*) m_pobSpecificCounterAttackLink );
	if (strcmp( pobInterface->GetClassName(), "CAttackLink" ) != 0)
	{
		user_warn_msg(("Wrong data type passed into SpecificCounter! Must be a CAttackLink, NOT a CAttackData"));
		return false;
	}
	else 
	{
		return true;
	}
	}
	#endif

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		SpecificCounterIndex::GetEntryAt
*
***************************************************************************************************/
SpecificCounterIndexEntry* SpecificCounterIndex::GetEntryAt(int iIndex) const
{
	ntError( iIndex >= 0 && iIndex < (int)m_obEntries.size());

	SpecificCounterIndexEntry* pobSpecificCounterIndexEntry = 0;
	
	ntstd::List<SpecificCounterIndexEntry*, Mem::MC_ENTITY>::const_iterator obIt = m_obEntries.begin();
	int i = 0;
	while (i <= iIndex) 
	{
		if (i == iIndex)
		{
			pobSpecificCounterIndexEntry = (*obIt);
			break;
		}

		++i;
		++obIt;
	}
    
	return pobSpecificCounterIndexEntry;
}


/***************************************************************************************************
*
*	FUNCTION		SkillEvadeCollection::GetRandomSkillEvade
*
*	DESCRIPTION		Will retrieve a random skill evade from our list, but not the same one that we
*					retrieved last time (unless we only have one).
*
***************************************************************************************************/
const CAttackLink* SkillEvadeCollection::GetRandomSkillEvade(ATTACK_EVADE_SECTOR eEvadeSector)
{
	if((eEvadeSector < AES_FRONT) || (eEvadeSector >= AES_NUM_EVADESECTORS))
	{
		return 0;
	}

	int iNumEvades = m_obSkillEvades[eEvadeSector].size();

	if(iNumEvades == 0)
	{
		return 0;
	}
	else if(iNumEvades == 1)
	{
		return *m_obSkillEvades[eEvadeSector].begin();
	}
	else
	{
		int iNumToChoose = grand() % iNumEvades;
		//If we've generated the same random number as the last one used, then just plus or minus one (depending on position)
		if(iNumToChoose == m_iLastSkillEvadeNum[eEvadeSector])
		{
			if(iNumToChoose == (iNumEvades - 1))
			{
				iNumToChoose--;
				m_iLastSkillEvadeNum[eEvadeSector] = iNumToChoose;
			}
			else
			{
				iNumToChoose++;
				m_iLastSkillEvadeNum[eEvadeSector] = iNumToChoose;
			}
		}
		else
		{
			m_iLastSkillEvadeNum[eEvadeSector] = iNumToChoose;
		}

		CAttackLink* pAttackLink = 0;
		ntstd::List<CAttackLink*, Mem::MC_ENTITY>::const_iterator obIt = m_obSkillEvades[eEvadeSector].begin();
		int iLoopNum = 0;
		for( ; obIt != m_obSkillEvades[eEvadeSector].end() ; obIt++, iLoopNum++ )
		{
			if(iLoopNum != iNumToChoose)
			{
				continue;
			}

			pAttackLink = *obIt;
			break;
		}

		return pAttackLink;
	}

	return 0;
}


/***************************************************************************************************
*
*	FUNCTION		SpecificCounterCollection::GetSpecificCounter
*
*	DESCRIPTION		Will search collection of SpecificCounter objects for the given name and return
*					a pointer to the CAttackLink if found. Otherwise it will return NULL.
*
***************************************************************************************************/
CAttackLink* SpecificCounterCollection::GetSpecificCounter(CHashedString obSpecificCounterName) const
{
	ntError(!obSpecificCounterName.IsNull());

	//just linear search for it, there isn't going to be that many?
	ntstd::List<SpecificCounter*, Mem::MC_ENTITY>::const_iterator itCounters = m_obSpecificCounters.begin();
	while (itCounters != m_obSpecificCounters.end()) 
	{
		if ((*itCounters)->GetSpecificCounterAttackName() == obSpecificCounterName)
		{
			CAttackLink* pobTempAttackLink = (*itCounters)->GetSpecificCounterAttackLink();
			#ifdef _DEBUG
			DataObject* pobInterface = ObjectDatabase::Get().GetDataObjectFromPointer( (const void*) pobTempAttackLink );
			if (strcmp( pobInterface->GetClassName(), "CAttackLink" ) != 0)
			{
				// Stop a crash later on and warn the user that the data is invalid
				user_warn_msg(("Wrong data type in SpecificCounter! Must be a CAttackLink, NOT a CAttackData or anything else"));
				return NULL;
			}
			else 
			#endif
				return pobTempAttackLink;
		}		

		++itCounters;
	}
	
	return NULL;
}


/***************************************************************************************************
*
*	FUNCTION		CClusterStructure::CClusterStructure
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/
CClusterStructure::CClusterStructure()
:	m_pobLeadCluster( 0 ),
	m_pobGroundClusterFront( 0 ),
	m_pobGroundClusterBack( 0 ),
	m_pobInstantKORecoverAttackCluster( 0 ),
	m_pobInterceptCluster( 0 ),
	m_pobRisingCluster( 0 ),
	m_pobStyleLevelSpecialCluster( 0 ),
	m_pobShortRangeCluster( 0 ),
	m_pobMediumRangeCluster( 0 ),
	m_pobLongRangeCluster( 0 ),
	m_pobOpenFreeEvades( 0 ),
	m_pobComboFreeEvades( 0 ),
	m_pobOpenSkillEvades( 0 ),
	m_pobCloseSkillEvades( 0 ),
	m_pobOpenSuperSkillEvades( 0 ),
	m_pobCloseSuperSkillEvades( 0 )
{
}


/***************************************************************************************************
*
*	FUNCTION		CClusterStructure::~CClusterStructure
*
*	DESCRIPTION		Detruction
*
***************************************************************************************************/
CClusterStructure::~CClusterStructure()
{
}

//------------------------------------------------------------------------------------------
//!
//!	CAttackBlock::CAttackBlock
//! Construction
//!
//------------------------------------------------------------------------------------------
CAttackBlock::CAttackBlock()
:	m_obLoopAnimName(),
	m_obEnterAnimName(),
	m_obExitAnimName()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackBlock::~CAttackBlock
//! Destruction
//!
//------------------------------------------------------------------------------------------
CAttackBlock::~CAttackBlock()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackKO::CAttackKO
//! Construction
//!
//------------------------------------------------------------------------------------------
CAttackKO::CAttackKO( void )
:  	m_obFallAnimName(),
	m_obFlooredAnimName(),
	m_obWaitAnimName(),
	m_obRiseAnimName(),
	m_obIRecoverAnimName(),
	m_obGroundAttackRiseAnimName()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackKO::~CAttackKO
//! Destruction
//!
//------------------------------------------------------------------------------------------
CAttackKO::~CAttackKO()
{
}


/***************************************************************************************************
*
*	FUNCTION		CAttackLink::CAttackLink
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/
CAttackLink::CAttackLink()
{
	m_pobAttackData = 0;

	for (int i = 0; i < AM_NONE; i++)
	{
		m_pobLinks[i] = 0;
	}

	m_pobButtonHeldAttack = 0;

	m_pobForceSwapWith = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackLink::HasAttackType
*
*	DESCRIPTION		Checks to see whether this link has an attack of the particular type.  For now
*					it makes the assumption that the attack type is linked to the Link structure. 
*					I'm not sure that this assumption will always remain valid - especially for AI.
*
***************************************************************************************************/
bool CAttackLink::HasAttackType( ATTACK_MOVE_TYPE eAttackType ) const
{
	return m_pobLinks[eAttackType] != 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackLink::GetNextAttackNode
*
*	DESCRIPTION		Returns a pointer to the next attack node based on the requested attack type.
*					Will return a null pointer if there is no valid attack.
*
***************************************************************************************************/
const CAttackLink* CAttackLink::GetNextAttackNode( ATTACK_MOVE_TYPE eAttackType ) const
{
	if (m_pobForceSwapWith)
		return m_pobForceSwapWith->GetNextAttackNode(eAttackType);

	return m_pobLinks[eAttackType];
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackData::CAttackData
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
CAttackData::CAttackData()
:	
	m_obStrike(),
	m_obStrike2(),
	m_obAttackPopOut(),
	m_obMovementPopOut(),
	m_obBlockPopOut(),
	m_obNextMove(),
	m_obInvulnerabilityWindow(),
	m_obNoLockWindows(),
	m_obNoCollideWindows(),
	m_obRangeInterceptWindow(),
	m_obInterceptWindow(),
	m_pobUninterruptibleWindowDetails( 0 ),
	m_obEvadeAndGrabDenyWindow(),
	m_obVictimRagdollableWindow(),
	m_eAttackClass( AC_COUNT ),
	m_obAttackAnimName(),
	m_fAttackScaledOffset( 0.7f ),
	m_iDamage( 0 ),
	m_iDamage2( 0 ),
	m_eUnBlockedReaction( RT_RECOIL ),
	m_eSpeedBlockedReaction( RT_DEFLECT ),
	m_eRangeBlockedReaction( RT_IMPACT_STAGGER ),
	m_ePowerBlockedReaction( RT_IMPACT_STAGGER ),
	m_eSyncdSecondaryReaction( RT_IMPACT_STAGGER ),
	m_eUnBlockedReactionFinalStrikeOverride( RT_COUNT ),
	m_obReceiverAnim(),
	m_obReceiverAnimSpeedBlocked(),
	m_obReceiverAnimRangeBlocked(),
	m_obReceiverAnimPowerBlocked(),
	m_obReceiverAnimSyncdSecondaryReaction(),
	m_obSpecificKOAnimation(),
	m_bUseProceduralKO(false),
	m_pobSpecificCounterIndex( 0 ),
	m_pobSpecificKillCounterIndex( 0 ),
	m_pobSpecificSmallCounterIndex( 0 ),
	m_pobSpecificSmallKillCounterIndex( 0 ),
	m_fDeflectionTime( 0.3f ),
	m_fStrike2ProximityCheckAngle( 0.0f ),
	m_fStrike2ProximityCheckAngleCoverage( 0.0f ),
	m_fStrike2ProximityCheckSweep( 0.0f ),
	m_fStrikeProximityCheckDistance( 2.0f ),
	m_pobTSCurve( 0 ),
	m_obRecoveryAnimName(),
	m_fRecoilTime( 0.75f ),
	m_eReactionAppearance( RA_SPEED_UP ),
	m_bHoldPreviousTarget( false ),
	m_bGroupCombatLockOnMove( false ),
	m_eAttackMovementType( AMT_GROUND_TO_GROUND ),
	m_fMaxDistance( 1.5f ),
	m_eTargetType( AT_TYPE_ATTACK ),
	m_bAutoLink( false ),
	m_eSyncTransform( CRM_ATTACKER_ROOT_INIT ),
	m_obSyncReceiverAnim(),
	m_bInteractWithSyncTransform( false ),
	m_obInteractiveSyncTranslateAbsoluteSpeed( CONSTRUCT_CLEAR ),
	m_obMaxInteractiveSyncTranslateSpeed( CONSTRUCT_CLEAR ),
	m_obInteractiveSyncRotationAbsoluteSpeed( 0.0f ),
	m_obMaxInteractiveSyncRotationSpeed( 90.0f ),
	m_bReverseInteractiveStickInput( false ),
	m_bAftertouchableKO( false ),
	m_bAerialComboStartKO( false ),
	m_bJuggleKO( false ),
	m_bTurnOffGravity( false ),
	m_fStrikeProximityCheckExclusionDistance( 0.0f ),
	m_pobAttackCamera( 0 ),
	m_obSuperSafeRotationAnimAttacker(),
	m_obSuperSafeRotationAnimReceiver(),
	m_obSuperSafeTranslationAnimAttacker(),
	m_obSuperSafeTranslationAnimReceiver(),
	m_obSuperSafeLinkupAnimAttacker(),
	m_obSuperSafeLinkupAnimReceiver(),
	m_bNeedsSuperStyleSafety( false ),
	m_bNeedsToBeInContinueVolume( false ),
	m_obStrikeVolumeDescriptors(),
	m_bOverrideInvulnerabilityWindow( false ),
	m_bSkipFlooredAndRise( false ),
	m_bSkipFallingMovement( false ),
	m_fStrikeProximityCheckAngle( 0.0f ),
	m_fStrikeProximityCheckSweep( 60.0f ),
	m_bNoAnimationBlend( false ),
	m_bTrackTargetThroughout( false ),
	m_fAttackTime( 1.0f ),
	m_fAttackRecoverTime( 0.0f ) // If left at 0.0 will just mean anim plays at it's own length
{
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackData::~CAttackData
//! Destruction
//!
//------------------------------------------------------------------------------------------
CAttackData::~CAttackData()
{
}

#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!
//!	CAttackData::PostConstruct
//! Checks through the serialised data on import
//!
//------------------------------------------------------------------------------------------
void CAttackData::PostConstruct( void )
{
	if (m_pobUninterruptibleWindowDetails)
	{
		DataObject* pobInterface = ObjectDatabase::Get().GetDataObjectFromPointer( (const void*) m_pobUninterruptibleWindowDetails );
		if (strcmp( pobInterface->GetClassName(), "UninterruptibleWindowDetails" ) != 0)
		{
			// Stop a crash later on and warn the user that the data is invalid
			user_warn_msg(("Wrong data type in CAttackData::m_pobUninterruptibleWindowDetails! Must be a UninterruptibleWindowDetails"));
		}
	}

	/*if ( (m_eAttackClass == AC_GRAB || m_eAttackClass == AC_SUPER) && this->m_obNextMove.GetNumFlippers() > 0)
	{
		user_warn_msg(("%s: Grabs and supers with NextMove windows will cause button mashing badness!", ObjectDatabase::Get().GetNameFromPointer(this)));
	}*/

	/*if (this->m_bSyncronisedAttack && 
		(this->m_ePowerBlockedReaction == CS_KO || this->m_eRangeBlockedReaction == CS_KO || this->m_eSpeedBlockedReaction == CS_KO || this->m_eUnBlockedReaction == CS_KO) )
	{
		user_warn_msg(("%s: KO Reaction from a sync attack is likely to cause badness!", ObjectDatabase::Get().GetNameFromPointer(this)));
	}*/

	/*//if this is a grab or super and it is auto linking and not synchronised, it should have no next move window
	if ( (this->m_eAttackClass == AC_GRAB || this->m_eAttackClass == AC_SUPER) &&
		this->m_bAutoLink && !this->m_bSyncronisedAttack &&
		this->m_obNextMove.GetNumFlippers() > 0)
	{
		user_warn_p(false, ("%s: Autolinking grabs and supers should not have a next move window.", GetNameC()));
	}

	// Check all of the timings will work at least 15fps
	float fRobustToFPS = 15.0f;
	float fRobustnessCheck = 1.0f/fRobustToFPS;

	//m_fDeflectionTime, m_fRecoilTime, m_fAttackTime
	//m_obStrike, m_obStrike2, m_obAttackPopOut, m_obMovementPopOut, 
	//m_obBlockPopOut, m_obNextMove, m_obInvulnerabilityWindow, m_obNoLockWindows, 
	//m_obNoCollideWindows, m_obRangeInterceptWindow, m_obInterceptWindow, m_obUninterruptibleWindow
	
	//not just a straight time check, look at AIs and how they request attacks
	//check if it's possible at <15fps to end up being in 2 windows with conflicting purposes

	if (m_fDeflectionTime != 0.0f &&
		m_fDeflectionTime < fRobustnessCheck)
		user_warn_p(false,("DeflectionTime of %f in %s is too small to be robust to %fFPS",	m_fDeflectionTime,	this->GetNameC(), fRobustToFPS));
	if (m_fRecoilTime != 0.0f &&
		m_fRecoilTime < fRobustnessCheck)	
		user_warn_p(false,("RecoilTime of %f in %s is too small to be robust to %fFPS",		m_fRecoilTime,		this->GetNameC(), fRobustToFPS));
	if (m_fAttackTime != 0.0f &&
		m_fAttackTime < fRobustnessCheck)
		user_warn_p(false,("AttackTime of %f in %s is too small to be robust to %fFPS",	m_fAttackTime,		this->GetNameC(), fRobustToFPS));
	
	CheckFlipFlopRobustness(m_obStrike,"Strike",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obStrike2,"Strike2",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obAttackPopOut,"AttackPopOut",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obBlockPopOut,"BlockPopOut",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obNextMove,"NextMove",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obInvulnerabilityWindow,"InvulnerabilityWindow",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obNoLockWindows,"NoLockWindows",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obNoCollideWindows,"NoCollideWindows",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obRangeInterceptWindow,"RangeInterceptWindow",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obInterceptWindow,"InterceptWindow",fRobustToFPS,fRobustnessCheck);
	CheckFlipFlopRobustness(m_obUninterruptibleWindow,"UninterruptibleWindow",fRobustToFPS,fRobustnessCheck);
	*/
}

void CAttackData::CheckFlipFlopRobustness(CFlipFlop& obFlipFlop, const char* pcName, float fRobustToFPS, float fRobustnessCheck)
{
	float fLength;
	if (obFlipFlop.GetNumFlippers() == 1)
	{
		fLength = obFlipFlop.GetFirstValueLength(m_fAttackTime);
		if (fLength != 0.0f && fLength < fRobustnessCheck)
			user_warn_p(false,("%s FlipFlop length of %f in %s is too small to be robust to %fFPS", pcName, fLength, ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )), fRobustToFPS));
	}
	else if (obFlipFlop.GetNumFlippers() > 1)
	{
		CFlipFlop::FlipperContainerType::const_iterator itStart = obFlipFlop.Begin();
		while (itStart != obFlipFlop.End()) 
		{
			fLength = itStart->X2(m_fAttackTime) - itStart->X1(m_fAttackTime);
			if (fLength != 0.0f && fLength < fRobustnessCheck)
				user_warn_p(false,("%s FlipFlop length of %f in %s is too small to be robust to %fFPS", pcName, fLength, ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )), fRobustToFPS));

			itStart++;
		}
	}
}

bool CAttackData::EditorChangeValue(CallBackParameter, CallBackParameter)
{
	PostConstruct();
	return true;
}
#endif


//--------------------------------------------------
//!
//!	CAttackData::RenderSprite
//!
//--------------------------------------------------
#ifndef _RELEASE
void RenderSprite( float fX, float fY, float fWidth, float fHeight, uint32_t col )
{
	ScreenSprite* pSprite = NT_NEW ScreenSprite;

	pSprite->SetPosition( CPoint( fX, fY, 0.0f ) );
	pSprite->SetWidth( fWidth );
	pSprite->SetHeight( fHeight );
	pSprite->SetColour( col );
	pSprite->SetTexture( "white.bmp" );
	ScreenSpriteShim* pShim = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) ScreenSpriteShim( pSprite );
	pShim->m_bHDR = false;
	EffectManager::Get().AddEffect( pShim );
}
#endif


//--------------------------------------------------
//!
//!	CAttackData::RenderFlipFlop
//!
//--------------------------------------------------
#ifndef _RELEASE
void RenderFlipFlop( CFlipFlop& flipFlop, float fScale, float fStartX, float fMaxWidth, float fY, float fHeight, uint32_t col )
{
	UNUSED(fScale);

	for (	CFlipFlop::FlipperContainerType::const_iterator it = flipFlop.Begin();
			it != flipFlop.End(); ++it )
	{
		float fStart = fStartX + it->X1( fMaxWidth );
		float fEnd = fStartX + it->X2( fMaxWidth );

		RenderSprite( (fStart + fEnd) * 0.5f, fY, fEnd - fStart, fHeight, col );
//		g_VisualDebug->Printf2D( fStart, fY - (fHeight * 0.5f), 0xffffffff, DEBUG_TEXT_ALIGN_BOTTOM, "%.2f", (*it)->X1( fScale ) );
//		g_VisualDebug->Printf2D( fEnd, fY + (fHeight * 0.5f), 0xffffffff, 0, "%.2f", (*it)->X2( fScale ) );
	}
}
#endif


//--------------------------------------------------
//!
//!	CAttackData::DebugRender
//!
//--------------------------------------------------
#ifndef _RELEASE
void CAttackData::DebugRender( void )
{
	static bool sDebugRender = false;

	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_COMMA ) )
		sDebugRender = !sDebugRender;

	if (!sDebugRender)
		return;

	float fDisplayWidth = g_VisualDebug->GetDebugDisplayWidth();
	float fDisplayHeight = g_VisualDebug->GetDebugDisplayHeight();

	RenderSprite(	fDisplayWidth*0.5f,
					fDisplayHeight*0.5f,
					fDisplayWidth,
					fDisplayHeight, 0xffffffff);

#define TEXT_HEIGHT (14.0f)
#define PIXELS_PER_SECOND (8 * 30) // assuming 30 fps
#define BAR_HEIGHT (16.0f)
#define LINE_HEIGHT (BAR_HEIGHT + TEXT_HEIGHT)	
#define OFFSET ((BAR_HEIGHT * 0.5f)+TEXT_HEIGHT)

	float fCurrY = 20.0f;
	float fMinX = 45.0f;
	float fMaxX = fMinX + (m_fAttackTime * PIXELS_PER_SECOND);
	float fDisplayLength = fMaxX - fMinX;

	float fStrikeTime = 0.0f;
	if (m_obStrike.GetNumFlippers())
		fStrikeTime = m_obStrike.GetFirstValue( m_fAttackTime );

	// name
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "Attack: %s (%.2f sec)", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )), m_fAttackTime );
	fCurrY += TEXT_HEIGHT * 2.0f;

	float fStartY = fCurrY;

	// Strike
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "Strike:" );
	RenderFlipFlop( m_obStrike, m_fAttackTime, fMinX, fDisplayLength, fCurrY + OFFSET, BAR_HEIGHT, 0xffff0000 );
	fCurrY += LINE_HEIGHT;

	// NoLockWindows
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "NoLockon:" );
	RenderFlipFlop( m_obNoLockWindows, m_fAttackTime, fMinX, fDisplayLength, fCurrY + OFFSET, BAR_HEIGHT, 0xffff0000 );
	fCurrY += LINE_HEIGHT;

	// AttackPopOut
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "AttackPopOut:" );
	RenderFlipFlop( m_obAttackPopOut, m_fAttackTime, fMinX, fDisplayLength, fCurrY + OFFSET, BAR_HEIGHT, 0xffff0000 );
	fCurrY += LINE_HEIGHT;

	// MovementPopOut
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "MovementPopOut:" );
	RenderFlipFlop( m_obMovementPopOut, m_fAttackTime, fMinX, fDisplayLength, fCurrY + OFFSET, BAR_HEIGHT, 0xffff0000 );
	fCurrY += LINE_HEIGHT;

	// BlockPopOut
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "BlockPopOut:" );
	RenderFlipFlop( m_obBlockPopOut, m_fAttackTime, fMinX, fDisplayLength, fCurrY + OFFSET, BAR_HEIGHT, 0xffff0000 );
	fCurrY += LINE_HEIGHT;
	
	// DeflectionTime
	float fDeflectionWidth = PIXELS_PER_SECOND * m_fDeflectionTime;
	float fDeflectionStart = PIXELS_PER_SECOND * fStrikeTime;
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "DeflectionTime:", m_fDeflectionTime );

	RenderSprite(	fMinX + fDeflectionStart + (fDeflectionWidth*0.5f),
					fCurrY + OFFSET,
					fDeflectionWidth,
					BAR_HEIGHT, 0xffff0000);
	fCurrY += LINE_HEIGHT;
	
	// RecoilTime
	float fRecoilWidth = PIXELS_PER_SECOND * m_fRecoilTime;
	float fRecoilStart = PIXELS_PER_SECOND * fStrikeTime;
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "RecoilTime:", m_fRecoilTime );

	RenderSprite(	fMinX + fRecoilStart + (fRecoilWidth*0.5f),
					fCurrY + OFFSET,
					fRecoilWidth,
					BAR_HEIGHT, 0xffff0000);
	fCurrY += LINE_HEIGHT;
	
	// Invul
	g_VisualDebug->Printf2D( fMinX, fCurrY, 0xffffffff, 0, "InvulnerabilityWindow:" );
	RenderFlipFlop( m_obInvulnerabilityWindow, m_fAttackTime, fMinX, fDisplayLength, fCurrY + OFFSET, BAR_HEIGHT, 0xffff0000 );
	fCurrY += LINE_HEIGHT;

	// borders
	g_VisualDebug->RenderLine(	CPoint( fMinX, fStartY, 0.0f ),
										CPoint( fMinX, fCurrY, 0.0f ),
										0x800000ff, DPF_DISPLAYSPACE );

	g_VisualDebug->RenderLine(	CPoint( fMaxX, fStartY, 0.0f ),
										CPoint( fMaxX, fCurrY, 0.0f ),
										0x800000ff, DPF_DISPLAYSPACE );

	// 1, 2, 3 sec intervals
	float fX = PIXELS_PER_SECOND * 1.0f;
	g_VisualDebug->RenderLine(	CPoint( fMinX + fX, fStartY, 0.0f ),
										CPoint( fMinX + fX, fCurrY, 0.0f ),
										0x80ff0000, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine(	CPoint( fMinX + fX*2.0f, fStartY, 0.0f ),
										CPoint( fMinX + fX*2.0f, fCurrY, 0.0f ),
										0x80ff0000, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine(	CPoint( fMinX + fX*3.0f, fStartY, 0.0f ),
										CPoint( fMinX + fX*3.0f, fCurrY, 0.0f ),
										0x80ff0000, DPF_DISPLAYSPACE );

	for (float fsub = 0.0f; fsub < 3.0f; fsub += 0.1f)
	{
		g_VisualDebug->RenderLine(	CPoint( fMinX + fsub*PIXELS_PER_SECOND, fStartY, 0.0f ),
											CPoint( fMinX + fsub*PIXELS_PER_SECOND, fCurrY, 0.0f ),
											0x40ff0000, DPF_DISPLAYSPACE );
	}
}
#endif

//--------------------------------------------------
//!
//!	CAttackData::GetStyleType
//!	Interogates the CAttackData for its style type
//! Adds no new data, so no data overhead.
//!
//--------------------------------------------------
STYLE_TYPE CAttackData::GetStyleType( void ) const
{
	if ( AT_TYPE_AERIAL_COMBO == m_eTargetType )
	{
		return STYLE_TYPE_AERIAL;
	}

	switch ( m_eAttackClass )
	{
	case AC_SPEED_FAST:
	case AC_SPEED_MEDIUM:
		return STYLE_TYPE_SPEED;
		break;

	case AC_POWER_FAST: 
	case AC_POWER_MEDIUM:
		return STYLE_TYPE_POWER;
		break;

	case AC_RANGE_FAST:
	case AC_RANGE_MEDIUM: 
		return STYLE_TYPE_RANGE;
		break;

	case AC_GRAB_STRIKE:
	case AC_GRAB_HOLD:
	case AC_GRAB_GOTO:
		switch ( m_eReactionAppearance )
		{
			case RA_SPEED_HIGH_LEFT:
			case RA_SPEED_HIGH_RIGHT:
			case RA_SPEED_LOW_LEFT:
			case RA_SPEED_LOW_RIGHT:
			case RA_SPEED_UP:
			case RA_SPEED_DOWN:
				return STYLE_TYPE_SPEED;
				break;

			case RA_POWER_HIGH_LEFT:
			case RA_POWER_HIGH_RIGHT:
			case RA_POWER_LOW_LEFT:
			case RA_POWER_LOW_RIGHT:
			case RA_POWER_UP:
			case RA_POWER_DOWN:
				return STYLE_TYPE_POWER;
				break;

			case RA_RANGE_HIGH_LEFT:
			case RA_RANGE_HIGH_RIGHT:
			case RA_RANGE_LOW_LEFT:
			case RA_RANGE_LOW_RIGHT:
			case RA_RANGE_UP:
			case RA_RANGE_DOWN:
				return STYLE_TYPE_RANGE;
				break;

			default:
				break;
		} // switch ( m_eReactionAppearance )
		break;

	default:
		break;
	} // switch ( m_eAttackClass )

	ntPrintf ( "Unexpected style for CAttackData %s\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this ) ) );

	return STYLE_TYPE_MISC;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackDefinition::CAttackDefinition
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
CAttackDefinition::CAttackDefinition()
:	m_pobClusterStructure( 0 ),
	m_fStrikeToBlockFactor( 0.0f ),
	m_fWiggleReductionFactor( 0.0f ),
	m_pobSpeedBlock( 0 ),
	m_pobPowerBlock( 0 ),
	m_pobRangeBlock( 0 ),
	m_pobPowerDeflections( 0 ),
	m_pobSpeedDeflections( 0 ),
	m_pobRangeDeflections( 0 ),
	m_obRiseDeflectionAnim(),
	m_obSpeedImpactStaggerRecoverAnim(),
	m_obRangeImpactStaggerRecoverAnim(),
	m_obPowerImpactStaggerRecoverAnim(),
	m_pobPowerImpactStaggers( 0 ),
	m_pobSpeedImpactStaggers( 0 ),
	m_pobRangeImpactStaggers( 0 ),
	m_obSpeedBlockStaggerRecoverAnim(),
	m_obRangeBlockStaggerRecoverAnim(),
	m_obPowerBlockStaggerRecoverAnim(),
	m_pobPowerBlockStaggers( 0 ),
	m_pobSpeedBlockStaggers( 0 ),
	m_pobRangeBlockStaggers( 0 ),
	m_fBlockStaggerTime( 0.0f ),
	m_fImpactStaggerTime( 0.0f ),
	m_fMinBlockStaggerTime( 0.0f ),
	m_fMinImpactStaggerTime( 0.0f ),
	m_pobPowerRecoilsFront( 0 ),
	m_pobPowerRecoilsBack( 0 ),
	m_pobSpeedRecoilsFront( 0 ),
	m_pobSpeedRecoilsBack( 0 ),
	m_pobRangeRecoilsFront( 0 ),
	m_pobRangeRecoilsBack( 0 ),
	m_pobPowerDeathsFront( 0 ),
	m_pobPowerDeathsBack( 0 ),
	m_pobSpeedDeathsFront( 0 ),
	m_pobSpeedDeathsBack( 0 ),
	m_pobRangeDeathsFront( 0 ),
	m_pobRangeDeathsBack( 0 ),
	m_obFrontAirKOAnim(),
	m_obBackAirKOAnim(),
	m_pobFrontKODefinition( 0 ),
	m_pobBackKODefinition( 0 ),
	m_pobPowerKOsFront( 0 ),
	m_pobPowerKOsBack( 0 ),
	m_pobSpeedKOsFront( 0 ),
	m_pobSpeedKOsBack( 0 ),
	m_pobRangeKOsFront( 0 ),
	m_pobRangeKOsBack( 0 ),
	m_fKOTime( 0.0f ),
	m_fKOInstantRecoverTime( 0.0f ),
	m_pobStanceSwitchingDef( 0 ),
	m_pobAerialDetails( 0 ),
	m_pobHitCounterDef( 0 ),
	m_pobCameras( 0 ),
	m_fStrikeUpperHeight( 1.0f ),
	m_fStrikeLowerHeight( 0.0f ),
	m_fAerialStrikeUpperHeight( 1.0f ),
	m_fAerialStrikeLowerHeight( 0.5f ),
	m_pobSpecialDefinition( 0 ),
	m_fBadCounterDetectTime( 0.2f ),
	m_fBadCounterPunishTime( 0.2f ),
	m_fCounterTime( 0.15f ),
	m_fQuickCounterTime( 0.0f ),
	m_fGrabProximityCheckSweep( 120.0f ),
	m_pobReactionMatrix( 0 ),
	m_fDeflectionTimeOverrideSpeed( 0.0f ),
	m_fDeflectionTimeOverrideRange( 0.0f ),
	m_fDeflectionTimeOverridePower( 0.0f ),
	m_bCanAutoBlockGrabs( true ),
	m_bCanAutoBlockSpeed( true ),
	m_bCanAutoBlockPower( true ),
	m_bCanAutoBlockRange( true ),
	m_fHeldAttackThresholdSpeed( 0.25f ),
	m_fHeldAttackThresholdRange( 0.25f ),
	m_fHeldAttackThresholdPower( 0.25f ),
	m_eStrikeProximityCheckExclusionDistanceReaction( RT_COUNT ),
	m_bIsGroundAttackable( true ),
	m_bExemptFromRagdollKOs( false ),
	m_bOnlyHeadShots( false ),
	m_pobHCMultiplierDef( 0 ),
	m_pobCombatPhysicsPushVolumesAttacking( 0 ),
	m_pobCombatPhysicsPushVolumesNonAttacking( 0 ),
	m_obSpecificVulnerabilities(),
	m_fJuggleTargetMemoryTime( 2.0f ),
	m_obCounterAttackEnvironmentCheckHalfExtents( 1.0f, 0.225f, 1.0f ),
	m_pobAttackMovementBlendInTimes( 0 ),
	m_fSkillEvadeRadius( 3.0f ),
	m_fSkillEvadeMargin( 0.7f ),
	m_fSuperSkillEvadeMargin( 0.25f ),
	m_obSkillEvadeAttackEnvironmentCheckHalfExtents( 2.5f, 0.3f, 2.5f )
{
}


/***************************************************************************************************
*
*	FUNCTION		CAttackDefinition::~CAttackDefinition
*
*	DESCRIPTION		Destructor
*
***************************************************************************************************/
CAttackDefinition::~CAttackDefinition()
{
}
