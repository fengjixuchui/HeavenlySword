//------------------------------------------------------
//!
//!	\file army/armydef.cpp
//!
//------------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "army/armymanager.h"
#include "army/battlefield.h"

#include "army/armydef.h"

START_CHUNKED_INTERFACE(ArmyBattlefield, Mem::MC_ARMY)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_SectionName, "Chapter1_1", SectionName );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_MinCorner, CPoint(0.0f, 0.0f, 0.0f), MinCorner );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_MaxCorner, CPoint(0.0f, 0.0f, 0.0f), MaxCorner );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_HeightfieldFilename, "", HeightfieldFilename );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_HFWidth, 512, HeightfieldWidth );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_HFHeight, 512, HeightfieldHeight );
	PUBLISH_VAR_AS( m_ContainmentFieldAIVol0, StaticAIVolume );
	PUBLISH_VAR_AS( m_ContainmentFieldAIVol1, StaticAIVolume1 );
	PUBLISH_VAR_AS( m_ContainmentFieldAIVol2, StaticAIVolume2 );
	PUBLISH_VAR_AS( m_ContainmentFieldAIVol3, StaticAIVolume3 );
	PUBLISH_PTR_CONTAINER_AS( m_ArmyBattalionDefs, Battalions );
	PUBLISH_PTR_CONTAINER_AS( m_ArmyObstacleDefs, Obstacles );
	PUBLISH_VAR_AS( m_obEventSoundExplosion, EventSoundExplosion );
	PUBLISH_VAR_AS( m_obEventSoundRocketFire, EventSoundRocketFire );
	PUBLISH_VAR_AS( m_obEventSoundJeer, EventSoundJeer );
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(ArmyBattalionSoundDef, Mem::MC_ARMY)
	PUBLISH_VAR_AS( m_obStateAdvanceSound,	StateAdvanceSound );
	PUBLISH_VAR_AS( m_obStateHoldSound,		StateHoldSound );
	PUBLISH_VAR_AS( m_obStateRetreatSound,	StateRetreatSound );
	PUBLISH_VAR_AS( m_obStateFleeSound,		StateFleeSound );
	PUBLISH_VAR_AS( m_obStateAttackSound,	StateAttackSound );
	PUBLISH_VAR_AS( m_obStateChargeSound,	StateChargeSound );
END_STD_INTERFACE

START_CHUNKED_INTERFACE(ArmyBattalion, Mem::MC_ARMY)
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_Command,				"Default",							Command );
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_MinCorner,			CPoint(0.0f, 0.0f, 0.0f),			MinCorner );
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_MaxCorner,			CPoint(0.0f, 0.0f, 0.0f),			MaxCorner );
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_Orientation,			CQuat(0.0f, 0.0f, 0.0f, 1.0f),		Orientation );
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_SoundDef,				"",									SoundDef );
	PUBLISH_PTR_CONTAINER_AS(		m_ArmyBattalionCommands,									Commands );
	PUBLISH_PTR_AS(					m_pBattalionToCopyCommandsFrom,								BattalionToCopyCommandsFrom );
	PUBLISH_PTR_AS(					m_pParentEntity,											ParentEntity );
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bPlayerTracking,		true,								PlayerTracking );
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bPlayerCircle,		false,								PlayerCircling );
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bPlayerAttacking,		true,								PlayerAttacking );
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bHasFlag,				false,								HasFlags );
END_STD_INTERFACE

START_CHUNKED_INTERFACE(ArmyObstacle, Mem::MC_ARMY)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_Centre, CPoint(0.0f, 0.0f, 0.0f), Centre );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_Radius, 1.0f, Radius );
END_STD_INTERFACE

START_CHUNKED_INTERFACE( ArmyUnitParameters, Mem::MC_ARMY )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fRunSpeed,					4.5f,			RunSpeed );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fWalkSpeed,					1.7f,			WalkSpeed );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fDiveSpeed,					2.f,			DiveSpeed );

	PUBLISH_VAR_WITH_DEFAULT_AS( m_iPersonalSpace,				1,			PersonalSpace );
	PUBLISH_PTR_AS(				 m_SpawnPool,								SpawnPool );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iMaxPoolSize,				50,			MaxPoolSize );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fHalfHeight,					0.9f,		HalfHeight );

	PUBLISH_VAR_WITH_DEFAULT_AS( m_IdleColourSprite,			"imp_shieldman_idle_colour_alpha.tai",		IdleColourSprite );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_IdleNormalSprite,			"imp_shieldman_idle_normal_mono.tai",		IdleNormalSprite );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_IdleCycleTime,				0.5f,										IdleCycleTime );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_WalkColourSprite,			"imp_shieldman_walk_colour_alpha.tai",		WalkColourSprite );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_WalkNormalSprite,			"imp_shieldman_walk_normal_mono.tai",		WalkNormalSprite );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_WalkCycleTime,				0.5f,										WalkCycleTime );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_RunColourSprite,				"imp_shieldman_run_colour_alpha.tai",		RunColourSprite );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_RunNormalSprite,				"imp_shieldman_run_normal_mono.tai",		RunNormalSprite );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_RunCycleTime,				0.5f,										RunCycleTime );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_DeadColourSprite,			"imp_shieldman_dead_colour_alpha.tai",		DeadColourSprite );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_DeadNormalSprite,			"imp_shieldman_dead_normal_mono.tai",		DeadNormalSprite );

	PUBLISH_VAR_WITH_DEFAULT_AS( m_BombName,					"entities\\Resources\\Objects\\Generic\\Interactive_props\\obj_bomb_barrel.clump", BombName );

	PUBLISH_VAR_WITH_DEFAULT_AS( m_OuterPlayerTrackRadius,		60,											OuterPlayerTrackRadius );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_InnerPlayerTrackRadius,		10,											InnerPlayerTrackRadius );

	PUBLISH_VAR_WITH_DEFAULT_AS( m_CirclePlayerRadius,			15,											CirclePlayerRadius );

END_STD_INTERFACE

START_CHUNKED_INTERFACE( ArmyGenericParameters, Mem::MC_ARMY )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fPlayerInRadius,					20.f,		PlayerRadius );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBazookaInnerThreatRadius,		15.f,		BazookaInnerThreatRadius );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBazookaThreatRadius,			20.f,		BazookaThreatRadius );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBazookaExplodeRadius,			8.f,		BazookaExplodeRadius );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fCamaraFarRadius,				100,		CamaraFarRadius );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iMaxPeeps,						600,		MaxPeeps );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iMaxAIDudes,						20,			MaxAIDudes );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iVisibleFrameLimit,				10,			VisibleFrameLimit );

	PUBLISH_VAR_WITH_DEFAULT_AS( m_iPlayerTrackWallInset,			1000,		PlayerTrackWallInset );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iPlayerTrackWallMultipler,		100,		PlayerTrackWallMultipler );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iPlayerTrackWallRandomishFactor,	10,			PlayerTrackWallRandomishFactor);

	PUBLISH_VAR_WITH_DEFAULT_AS( m_iStagePostBodyCountWithBazooka,			30,			StagePostBodyCountWithBazooka );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iStagePostBodyCountWithSwords,			5,			StagePostBodyCountWithSwords );

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMaxRandomDurationVariationAnim, 0.1f,		MaxRandomDurationVariationAnim );

	PUBLISH_VAR_WITH_DEFAULT_AS( m_FlagTemplateName,				"",			FlagTemplate );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_FlagPoleClumpName,				"",			FlagPoleClump );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fFlagPoleVerticalOffset,			10.f,		FlagPoleVerticalOffset );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fFlagPoleHeight,					10.f,		FlagPoleHeight );
END_STD_INTERFACE

START_CHUNKED_INTERFACE( ArmyBattalionCommand, Mem::MC_ARMY )
	PUBLISH_VAR_WITH_DEFAULT_AS(		m_vPos,			CPoint(0.0f, 0.0f, 0.0f),	Position );
	PUBLISH_VAR_WITH_DEFAULT_AS(		m_iSequence,	0,							Sequence );
	PUBLISH_VAR_WITH_DEFAULT_AS(		m_iCommand,		0,							Command );
	PUBLISH_VAR_WITH_DEFAULT_AS(		m_iParam0,		0,							Param0 );
	PUBLISH_VAR_WITH_DEFAULT_AS(		m_iParam1,		0,							Param1 );
END_STD_INTERFACE

void 	ForceLinkFunctionArmy()
{
	ntPrintf("!ATTN! Calling 	ForceLinkFunctionArmy(); !ATTN!\n");
}


